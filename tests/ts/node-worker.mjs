import { parentPort } from "node:worker_threads";
import { DirectClient } from "../../dist/direct-client.js";

let client;

parentPort.on("message", async (message) => {
  if (message.type === "init") {
    client = await DirectClient.create(message.wasm);
    parentPort.postMessage({ type: "ready" });
    return;
  }

  try {
    const result = await client.request(message.op, message.args, {
      ...(message.outputBuffers === undefined ? {} : { outputBuffers: message.outputBuffers }),
    });
    const transfers = [];
    const collect = (value) => {
      if (value instanceof ArrayBuffer) transfers.push(value);
      else if (Array.isArray(value)) value.forEach(collect);
      else if (value !== null && typeof value === "object") Object.values(value).forEach(collect);
    };
    collect(result);
    parentPort.postMessage({ type: "response", id: message.id, ok: true, result }, transfers);
  } catch (error) {
    parentPort.postMessage({
      type: "response",
      id: message.id,
      ok: false,
      error: {
        code: error instanceof Error && "code" in error ? error.code : "KernelError",
        message: error instanceof Error ? error.message : String(error),
      },
    });
  }
});
