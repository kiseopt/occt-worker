/// <reference lib="webworker" />

import { DirectClient } from "./direct-client.js";
import { OPERATIONS, type OperationName } from "./generated.js";
import { collectArrayBuffers } from "./protocol-codec.js";

declare const self: DedicatedWorkerGlobalScope;

let client: DirectClient | undefined;
let activeCall: { id: number; cancelFlag?: Int32Array } | undefined;

self.addEventListener("message", async (event) => {
  const message = event.data as {
    type: "init" | "call";
    wasm?: ArrayBuffer;
    id?: number;
    op?: string;
    args?: Record<string, unknown>;
    cancelBuffer?: SharedArrayBuffer;
    outputBuffers?: "array" | "shared";
  };
  if (message.type === "init") {
    try {
      self.postMessage({ type: "init-progress", stage: "compiling" });
      const module = await WebAssembly.compile(message.wasm!);
      self.postMessage({ type: "init-progress", stage: "instantiating" });
      client = await DirectClient.create(module, {}, {
        isCancelled: () => activeCall?.cancelFlag !== undefined
          && Atomics.load(activeCall.cancelFlag, 0) !== 0,
        onProgress: (fraction) => {
          if (activeCall !== undefined) {
            self.postMessage({ type: "progress", id: activeCall.id, fraction });
          }
        },
      });
      self.postMessage({ type: "ready" });
    } catch (error) {
      client = undefined;
      self.postMessage({
        type: "init-error",
        error: {
          code: error instanceof Error && "code" in error ? error.code : "KernelError",
          message: error instanceof Error ? error.message : String(error),
        },
      });
    }
    return;
  }
  try {
    if (client === undefined) throw new Error("Worker kernel is not initialized");
    activeCall = {
      id: message.id!,
      ...(message.cancelBuffer === undefined
        ? {}
        : { cancelFlag: new Int32Array(message.cancelBuffer) }),
    };
    if (!(OPERATIONS as readonly string[]).includes(message.op!)) {
      throw new TypeError(`Unknown kernel operation '${message.op!}'`);
    }
    const result = await client.request(message.op! as OperationName, message.args!, {
      ...(message.outputBuffers === undefined ? {} : { outputBuffers: message.outputBuffers }),
    });
    const transfers: Transferable[] = collectArrayBuffers(result);
    self.postMessage({ type: "response", id: message.id, ok: true, result }, transfers);
  } catch (error) {
    if (error instanceof WebAssembly.RuntimeError) {
      self.postMessage({
        type: "fatal",
        error: {
          code: "KernelError",
          message: error.message,
        },
      });
      return;
    }
    self.postMessage({
      type: "response",
      id: message.id,
      ok: false,
      error: {
        code: error instanceof Error && "code" in error ? error.code : "KernelError",
        message: error instanceof Error ? error.message : String(error),
      },
    });
  } finally {
    activeCall = undefined;
  }
});
