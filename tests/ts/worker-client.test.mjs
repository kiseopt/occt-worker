import assert from "node:assert/strict";
import test from "node:test";
import { WorkerClient } from "../../dist/worker-client.js";

const capabilities = {
  protocolVersion: "1.0.0",
  kernelVersion: "test",
  occtVersion: "test",
  ops: [],
  historySupport: {},
  buildFlags: { threads: false, simd: false, wasmExceptions: true },
};

class FakeWorker {
  listeners = { message: [], error: [] };
  terminated = false;

  constructor(handleCall, initialization = "ready") {
    this.handleCall = handleCall;
    this.initialization = initialization;
  }

  addEventListener(type, listener) {
    this.listeners[type].push(listener);
  }

  postMessage(message, transfer) {
    if (message.type === "init") {
      queueMicrotask(() => {
        if (this.terminated) return;
        if (this.initialization === "ready") this.emitMessage({ type: "ready" });
        else this.emitError(new Error("persistent initialization failure"));
      });
      return;
    }
    this.handleCall(this, message, transfer);
  }

  terminate() {
    this.terminated = true;
  }

  emitMessage(data) {
    for (const listener of this.listeners.message) listener({ data });
  }

  emitError(error) {
    for (const listener of this.listeners.error) listener({ error, message: error.message });
  }
}

function respond(worker, message, result) {
  worker.emitMessage({ type: "response", id: message.id, ok: true, result });
}

function handleCapabilities(worker, message) {
  if (message.op !== "capabilities") return false;
  queueMicrotask(() => respond(worker, message, capabilities));
  return true;
}

test("WorkerClient starts timeout only when a queued call is dispatched", async () => {
  const dispatches = [];
  const factory = () => {
    const pending = [];
    let busy = false;
    const processNext = (worker) => {
      if (busy || pending.length === 0) return;
      busy = true;
      const message = pending.shift();
      dispatches.push(message.op);
      setTimeout(() => {
        respond(worker, message, message.op);
        busy = false;
        processNext(worker);
      }, message.op === "slow" ? 30 : 0);
    };
    return new FakeWorker((worker, message) => {
      if (handleCapabilities(worker, message)) return;
      pending.push(message);
      processNext(worker);
    });
  };

  const client = await WorkerClient.create(factory, new ArrayBuffer(1));
  try {
    const slow = client.request("slow", {}, 80);
    const queued = client.request("queued", {}, 10);
    assert.deepEqual(await Promise.all([slow, queued]), ["slow", "queued"]);
    assert.deepEqual(dispatches, ["slow", "queued"]);
  } finally {
    client.close();
  }
});

test("WorkerClient shares caller input and requests shared output buffers", async () => {
  const input = new SharedArrayBuffer(16);
  const output = new SharedArrayBuffer(32);
  const factory = () => new FakeWorker((worker, message, transfers) => {
    if (handleCapabilities(worker, message)) return;
    assert.equal(message.outputBuffers, "shared");
    assert.strictEqual(message.args.data.$inputBuffer, input);
    assert.deepEqual(transfers, []);
    queueMicrotask(() => respond(worker, message, {
      data: { layout: "test", data: output },
    }));
  });

  const client = await WorkerClient.create(factory, new ArrayBuffer(1));
  try {
    const result = await client.requestShared("shared", { data: { $inputBuffer: input } });
    assert.strictEqual(result.data.data, output);
    assert.equal(input.byteLength, 16);
  } finally {
    client.close();
  }
});

test("WorkerClient removes an aborted queued call without restarting", async () => {
  const dispatches = [];
  let releaseSlow;
  let factoryCalls = 0;
  const factory = () => {
    factoryCalls++;
    return new FakeWorker((worker, message) => {
      if (handleCapabilities(worker, message)) return;
      dispatches.push(message.op);
      if (message.op === "slow") {
        releaseSlow = () => respond(worker, message, "slow");
      } else {
        queueMicrotask(() => respond(worker, message, message.op));
      }
    });
  };

  const client = await WorkerClient.create(factory, new ArrayBuffer(1));
  try {
    const controller = new AbortController();
    const slow = client.request("slow", {});
    const queued = client.request("queued", {}, { signal: controller.signal });
    controller.abort(new Error("queued request cancelled"));
    await assert.rejects(queued, /queued request cancelled/);
    releaseSlow();
    assert.equal(await slow, "slow");
    assert.deepEqual(dispatches, ["slow"]);
    assert.equal(factoryCalls, 1);
  } finally {
    client.close();
  }
});

test("WorkerClient aborts an active call by rebuilding the worker", async () => {
  let factoryCalls = 0;
  let markSlowDispatched;
  const slowDispatched = new Promise((resolve) => { markSlowDispatched = resolve; });
  const factory = () => {
    factoryCalls++;
    const generation = factoryCalls;
    return new FakeWorker((worker, message) => {
      if (handleCapabilities(worker, message)) return;
      if (message.op === "beginScope") {
        queueMicrotask(() => respond(worker, message, { scopeId: 1 }));
      } else if (message.op === "makeBox") {
        queueMicrotask(() => respond(worker, message, { shape: 1 }));
      } else if (generation === 1 && message.op === "slow") {
        markSlowDispatched();
      } else {
        queueMicrotask(() => respond(worker, message, message.op));
      }
    });
  };

  const client = await WorkerClient.create(factory, new ArrayBuffer(1));
  const scope = await client.beginScope();
  const oldHandle = await scope.makeBox([1, 1, 1]);
  const controller = new AbortController();
  const active = client.request("slow", {}, { signal: controller.signal });
  await slowDispatched;
  const queued = client.request("queued", {});
  await Promise.resolve();
  controller.abort(new Error("active request cancelled"));
  await assert.rejects(active, /active request cancelled/);
  await assert.rejects(queued, /active request cancelled/);
  assert.throws(() => oldHandle.encode(client), /expired kernel instance/);
  assert.equal(await client.request("after", {}), "after");
  assert.equal(factoryCalls, 2);
  client.close();
});

test("WorkerClient cooperatively cancels STEP transfer without rebuilding the worker", async () => {
  let factoryCalls = 0;
  let sawCancellation = false;
  const factory = () => {
    factoryCalls++;
    return new FakeWorker((worker, message) => {
      if (handleCapabilities(worker, message)) return;
      if (message.op === "exportSTEP") {
        assert.ok(message.cancelBuffer instanceof SharedArrayBuffer);
        queueMicrotask(() => worker.emitMessage({
          type: "progress",
          id: message.id,
          fraction: 0.25,
        }));
        setTimeout(() => {
          sawCancellation = Atomics.load(new Int32Array(message.cancelBuffer), 0) === 1;
          worker.emitMessage({
            type: "response",
            id: message.id,
            ok: false,
            error: { code: "ImportExportFailed", message: "STEP transfer cancelled" },
          });
        }, 0);
        return;
      }
      queueMicrotask(() => respond(worker, message, message.op));
    });
  };

  const client = await WorkerClient.create(factory, new ArrayBuffer(1));
  try {
    const controller = new AbortController();
    const progress = [];
    const active = client.request("exportSTEP", {}, {
      signal: controller.signal,
      onProgress: (event) => {
        progress.push(event);
        controller.abort(new Error("STEP request cancelled"));
      },
    });
    const queued = client.request("after", {});
    await assert.rejects(active, /STEP request cancelled/);
    assert.equal(await queued, "after");
    assert.equal(sawCancellation, true);
    assert.equal(factoryCalls, 1);
    assert.deepEqual(progress, [{ operation: "exportSTEP", fraction: 0.25 }]);
  } finally {
    client.close();
  }
});

test("WorkerClient cooperatively cancels tessellation without rebuilding the worker", async () => {
  let factoryCalls = 0;
  let sawCancellation = false;
  const factory = () => {
    factoryCalls++;
    return new FakeWorker((worker, message) => {
      if (handleCapabilities(worker, message)) return;
      if (message.op === "tessellate") {
        assert.ok(message.cancelBuffer instanceof SharedArrayBuffer);
        queueMicrotask(() => worker.emitMessage({ type: "progress", id: message.id, fraction: 0.25 }));
        setTimeout(() => {
          sawCancellation = Atomics.load(new Int32Array(message.cancelBuffer), 0) === 1;
          worker.emitMessage({
            type: "response",
            id: message.id,
            ok: false,
            error: { code: "TessellationFailed", message: "OCCT tessellation cancelled" },
          });
        }, 0);
        return;
      }
      queueMicrotask(() => respond(worker, message, message.op));
    });
  };

  const client = await WorkerClient.create(factory, new ArrayBuffer(1));
  try {
    const controller = new AbortController();
    const active = client.request("tessellate", {}, {
      signal: controller.signal,
      onProgress: () => controller.abort(new Error("tessellation request cancelled")),
    });
    const queued = client.request("after", {});
    await assert.rejects(active, /tessellation request cancelled/);
    assert.equal(await queued, "after");
    assert.equal(sawCancellation, true);
    assert.equal(factoryCalls, 1);
  } finally {
    client.close();
  }
});

test("WorkerClient cooperatively cancels VRML transfer without rebuilding the worker", async () => {
  let factoryCalls = 0;
  let sawCancellation = false;
  const factory = () => {
    factoryCalls++;
    return new FakeWorker((worker, message) => {
      if (handleCapabilities(worker, message)) return;
      if (message.op === "exportVRML" || message.op === "importVRML") {
        assert.ok(message.cancelBuffer instanceof SharedArrayBuffer);
        queueMicrotask(() => worker.emitMessage({ type: "progress", id: message.id, fraction: 0.5 }));
        setTimeout(() => {
          sawCancellation = Atomics.load(new Int32Array(message.cancelBuffer), 0) === 1;
          worker.emitMessage({
            type: "response",
            id: message.id,
            ok: false,
            error: { code: "ImportExportFailed", message: "VRML transfer cancelled" },
          });
        }, 0);
        return;
      }
      queueMicrotask(() => respond(worker, message, message.op));
    });
  };

  const client = await WorkerClient.create(factory, new ArrayBuffer(1));
  try {
    const controller = new AbortController();
    const active = client.request("exportVRML", {}, {
      signal: controller.signal,
      onProgress: () => controller.abort(new Error("VRML request cancelled")),
    });
    const queued = client.request("after", {});
    await assert.rejects(active, /VRML request cancelled/);
    assert.equal(await queued, "after");
    assert.equal(sawCancellation, true);
    sawCancellation = false;
    const importController = new AbortController();
    const imported = client.request("importVRML", {}, {
      signal: importController.signal,
      onProgress: () => importController.abort(new Error("VRML import request cancelled")),
    });
    await assert.rejects(imported, /VRML import request cancelled/);
    assert.equal(sawCancellation, true);
    assert.equal(factoryCalls, 1);
  } finally {
    client.close();
  }
});

test("WorkerClient equips mesh exchange with cooperative cancellation", async () => {
  let factoryCalls = 0;
  let sawCancellation = false;
  const factory = () => {
    factoryCalls++;
    return new FakeWorker((worker, message) => {
      if (handleCapabilities(worker, message)) return;
      if (message.op === "exportOBJ") {
        assert.ok(message.cancelBuffer instanceof SharedArrayBuffer);
        queueMicrotask(() => worker.emitMessage({ type: "progress", id: message.id, fraction: 0.5 }));
        setTimeout(() => {
          sawCancellation = Atomics.load(new Int32Array(message.cancelBuffer), 0) === 1;
          worker.emitMessage({
            type: "response",
            id: message.id,
            ok: false,
            error: { code: "ImportExportFailed", message: "OBJ transfer cancelled" },
          });
        }, 0);
        return;
      }
      queueMicrotask(() => respond(worker, message, message.op));
    });
  };

  const client = await WorkerClient.create(factory, new ArrayBuffer(1));
  try {
    const controller = new AbortController();
    const active = client.request("exportOBJ", {}, {
      signal: controller.signal,
      onProgress: () => controller.abort(new Error("OBJ request cancelled")),
    });
    const queued = client.request("after", {});
    await assert.rejects(active, /OBJ request cancelled/);
    assert.equal(await queued, "after");
    assert.equal(sawCancellation, true);
    assert.equal(factoryCalls, 1);
  } finally {
    client.close();
  }
});

test("WorkerClient equips IGES transfer with cooperative cancellation", async () => {
  let factoryCalls = 0;
  let sawCancellation = false;
  const factory = () => {
    factoryCalls++;
    return new FakeWorker((worker, message) => {
      if (handleCapabilities(worker, message)) return;
      if (message.op === "importIGES") {
        assert.ok(message.cancelBuffer instanceof SharedArrayBuffer);
        queueMicrotask(() => worker.emitMessage({
          type: "progress",
          id: message.id,
          fraction: 0.5,
        }));
        setTimeout(() => {
          sawCancellation = Atomics.load(new Int32Array(message.cancelBuffer), 0) === 1;
          worker.emitMessage({
            type: "response",
            id: message.id,
            ok: false,
            error: { code: "ImportExportFailed", message: "IGES transfer cancelled" },
          });
        }, 0);
        return;
      }
      queueMicrotask(() => respond(worker, message, message.op));
    });
  };

  const client = await WorkerClient.create(factory, new ArrayBuffer(1));
  try {
    const controller = new AbortController();
    const active = client.request("importIGES", {}, {
      signal: controller.signal,
      onProgress: () => controller.abort(new Error("IGES request cancelled")),
    });
    const queued = client.request("after", {});
    await assert.rejects(active, /IGES request cancelled/);
    assert.equal(await queued, "after");
    assert.equal(sawCancellation, true);
    assert.equal(factoryCalls, 1);
  } finally {
    client.close();
  }
});

test("WorkerClient rejects persistent initialization failure without restarting", async () => {
  let factoryCalls = 0;
  const factory = () => {
    factoryCalls++;
    return new FakeWorker(() => undefined, "error");
  };

  await assert.rejects(
    WorkerClient.create(factory, new ArrayBuffer(1)),
    /persistent initialization failure/,
  );
  await new Promise((resolve) => setTimeout(resolve, 20));
  assert.equal(factoryCalls, 1);
});

test("WorkerClient fails the old queue and rebuilds once after a runtime error", async () => {
  let factoryCalls = 0;
  const factory = () => {
    factoryCalls++;
    const generation = factoryCalls;
    return new FakeWorker((worker, message) => {
      if (handleCapabilities(worker, message)) return;
      if (message.op === "beginScope") {
        queueMicrotask(() => respond(worker, message, { scopeId: 1 }));
        return;
      }
      if (message.op === "makeBox") {
        queueMicrotask(() => respond(worker, message, { shape: 1 }));
        return;
      }
      if (generation === 1 && message.op === "crash") {
        queueMicrotask(() => worker.emitMessage({
          type: "fatal",
          error: { code: "KernelError", message: "runtime trap" },
        }));
        return;
      }
      queueMicrotask(() => respond(worker, message, message.op));
    });
  };

  const client = await WorkerClient.create(factory, new ArrayBuffer(1));
  const scope = await client.beginScope();
  const oldHandle = await scope.makeBox([1, 1, 1]);
  const crashing = client.request("crash", {});
  const queued = client.request("queued", {});
  await assert.rejects(crashing, /runtime trap/);
  await assert.rejects(queued, /runtime trap/);
  assert.throws(() => oldHandle.encode(client), /expired kernel instance/);
  assert.equal(await client.request("after", {}), "after");
  await assert.rejects(scope.makeBox([2, 2, 2]), /expired kernel instance/);
  await assert.rejects(scope.end(), /expired kernel instance/);
  const replacementScope = await client.beginScope();
  await replacementScope.end();
  assert.equal(factoryCalls, 2);
  client.close();
});

test("ShapeScope shares concurrent end requests and retries a failed end", async () => {
  let endAttempts = 0;
  const factory = () => new FakeWorker((worker, message) => {
    if (handleCapabilities(worker, message)) return;
    if (message.op === "beginScope") {
      queueMicrotask(() => respond(worker, message, { scopeId: 1 }));
      return;
    }
    if (message.op === "endScope") {
      endAttempts++;
      queueMicrotask(() => {
        if (endAttempts === 1) {
          worker.emitMessage({
            type: "response",
            id: message.id,
            ok: false,
            error: { code: "KernelError", message: "temporary end failure" },
          });
        } else {
          respond(worker, message, {});
        }
      });
    }
  });

  const client = await WorkerClient.create(factory, new ArrayBuffer(1));
  const scope = await client.beginScope();
  const first = scope.end();
  const concurrent = scope.end();
  await assert.rejects(Promise.all([first, concurrent]), /temporary end failure/);
  assert.equal(endAttempts, 1);
  await scope.end();
  assert.equal(endAttempts, 2);
  client.close();
});
