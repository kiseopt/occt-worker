import { PROTOCOL_VERSION } from "../../dist/generated.js";

self.addEventListener("message", (event) => {
  const message = event.data;
  if (message.type === "init") {
    WebAssembly.compile(message.wasm).then(
      () => self.postMessage({ type: "ready" }),
      (error) => { throw error; },
    );
    return;
  }
  if (message.type !== "call") return;
  if (message.op === "capabilities") {
    self.postMessage({
      type: "response",
      id: message.id,
      ok: true,
      result: { protocolVersion: PROTOCOL_VERSION },
    });
    return;
  }

  const memory = new WebAssembly.Memory({ initial: 1, maximum: 1 });
  try {
    memory.grow(1);
  } catch (error) {
    throw new RangeError(`Worker memory limit reached: ${error instanceof Error ? error.message : String(error)}`);
  }
});
