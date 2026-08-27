// Shared spike sequence used by both hosts. Drives worker-entry.mjs through
// the required verification checklist and returns a result summary.
//
// host must expose:
//   onMessage(handler), onError(handler), post(message), terminate()

const CALL_TIMEOUT_MS = 30000;

export async function runSpikeSequence(host, urls) {
  const pending = new Map();
  let nextCallId = 1;
  let readyHandler = undefined;
  let fatalHandler = undefined;

  host.onMessage((data) => {
    if (!data || typeof data !== "object") return;
    if (data.type === "ready" && readyHandler) {
      const handler = readyHandler;
      readyHandler = undefined;
      handler.resolve(data);
      return;
    }
    if (data.type === "fatal" && fatalHandler) {
      const handler = fatalHandler;
      fatalHandler = undefined;
      clearTimeout(handler.timer);
      handler.resolve(data);
    }
    if (data.type === "fatal" && data.callId !== null && data.callId !== undefined && pending.has(data.callId)) {
      const entry = pending.get(data.callId);
      pending.delete(data.callId);
      clearTimeout(entry.timer);
      entry.reject(new Error(`fatal: ${data.error}`));
    }
    if (data.type === "response" && pending.has(data.callId)) {
      const entry = pending.get(data.callId);
      pending.delete(data.callId);
      clearTimeout(entry.timer);
      if (data.ok) entry.resolve(data);
      else entry.reject(new Error(data.error ?? "spike call failed"));
    }
  });

  host.onError((error) => {
    for (const [, entry] of pending) {
      clearTimeout(entry.timer);
      entry.reject(new Error(`worker error: ${error}`));
    }
    pending.clear();
    if (readyHandler) {
      const handler = readyHandler;
      readyHandler = undefined;
      clearTimeout(handler.timer);
      handler.reject(new Error(`worker error during init: ${error}`));
    }
    if (fatalHandler) {
      const handler = fatalHandler;
      fatalHandler = undefined;
      clearTimeout(handler.timer);
      handler.reject(new Error(`worker error while waiting for trap: ${error}`));
    }
  });

  const send = (message) => new Promise((resolve, reject) => {
    const callId = message.callId ?? nextCallId++;
    const timer = setTimeout(() => {
      if (pending.has(callId)) {
        pending.delete(callId);
        reject(new Error(`timeout waiting for response to ${JSON.stringify(message).slice(0, 120)}`));
      }
    }, CALL_TIMEOUT_MS);
    pending.set(callId, { resolve, reject, timer });
    host.post({ ...message, callId });
  });

  const results = {};

  // 1. Cold load: main glue locates main.wasm and async-loads the side.
  const ready1 = await new Promise((resolve, reject) => {
    const timer = setTimeout(() => reject(new Error("timeout waiting for init")), CALL_TIMEOUT_MS);
    readyHandler = { resolve, reject, timer };
    send({ type: "init", epoch: 1, ...urls }).catch((error) => {
      clearTimeout(timer);
      reject(error);
    });
  });
  results.ready1 = { ...ready1.info };
  if (ready1.info.registrations !== 1) throw new Error(`expected exactly one registration, got ${ready1.info.registrations}`);
  if (ready1.info.abiVersion !== 1) throw new Error("descriptor abiVersion mismatch");
  if (ready1.info.descriptorName !== "dylink-spike") throw new Error(`unexpected descriptor name ${ready1.info.descriptorName}`);
  if (!ready1.info.buildFamilyMatches) throw new Error("build family mismatch");

  // 2. Side operation calls Main's host API through the shared heap.
  const identity = await send({ type: "call", op: "identity", input: 41 });
  if (identity.value !== 42) throw new Error(`identity returned ${identity.value}`);
  results.identityAfterLoad = identity.value;

  // 3. Exceptions are caught inside the C ABI boundary; shared EH runtime.
  const eh = await send({ type: "call", op: "eh_roundtrip", input: 7 });
  if (eh.value !== 107) throw new Error(`eh_roundtrip returned ${eh.value}; exception crossed C ABI or was not caught`);
  results.ehRoundtrip = eh.value;

  // 4. Memory growth keeps side operations valid.
  const pagesBefore = (await send({ type: "call", op: "host_pages", input: 0 })).value;
  const grow = await send({ type: "grow", pages: 4 });
  if (!grow.ok) throw new Error("memory growth failed");
  const pagesAfterGrow = (await send({ type: "call", op: "host_pages", input: 0 })).value;
  if (pagesAfterGrow !== pagesBefore + 4) throw new Error(`pages ${pagesBefore} -> ${pagesAfterGrow}, expected +4`);
  const identityAfterGrowth = (await send({ type: "call", op: "identity", input: 41 })).value;
  if (identityAfterGrowth !== 42) throw new Error("identity broke after memory growth");
  results.memoryGrowth = { before: pagesBefore, after: pagesAfterGrow, identityStillValid: true };

  // 5. Repeat load must not register twice.
  const reload = await send({ type: "reload-side", sideUrl: urls.sideUrl });
  if (reload.registrations !== 1) throw new Error(`repeat load registered again: registrations=${reload.registrations}`);
  results.repeatLoadRegistrations = reload.registrations;

  // 6. Load failure is captured by JS.
  const badLoad = await send({ type: "bad-load", sideUrl: urls.badSideUrl });
  if (badLoad.ok !== true || !badLoad.errorMessage) throw new Error("load failure was not captured");
  results.loadFailure = { kind: badLoad.errorKind, message: badLoad.errorMessage };

  // 7. Trap escapes as WebAssembly.RuntimeError and rejects the caller.
  let trapFatal;
  const fatalPromise = new Promise((resolve, reject) => {
    const timer = setTimeout(() => reject(new Error("trap did not produce a fatal error within timeout")), CALL_TIMEOUT_MS);
    fatalHandler = { resolve, reject, timer };
  });
  let trapRejectedPending = false;
  try {
    await send({ type: "call", op: "trap", input: 0 });
    throw new Error("trap operation unexpectedly succeeded");
  } catch (error) {
    if (String(error.message ?? "").startsWith("timeout waiting for response")) throw error;
    trapRejectedPending = true;
  }
  trapFatal = await fatalPromise;
  if (!trapFatal.error) throw new Error("trap produced no error message");
  results.trapCaptured = { rejectedPending: trapRejectedPending, fatal: trapFatal.error };

  return results;
}
