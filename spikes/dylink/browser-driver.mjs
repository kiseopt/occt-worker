// Browser-side driver: runs the same spike protocol as the Node phase via a
// module Worker, then publishes the result for the Node-side verifier.

const base = new URL("./", import.meta.url);
const urls = {
  mainJsUrl: new URL("../../../build/release/dylink-spike/dylink-main.mjs", base).href,
  mainWasmUrl: new URL("../../../build/release/dylink-spike/dylink-main.wasm", base).href,
  sideUrl: new URL("../../../build/release/dylink-spike/dylink-side.wasm", base).href,
  badSideUrl: new URL("../../../build/release/dylink-spike/missing-side.wasm", base).href,
};

function createHost() {
  const worker = new Worker(new URL("./worker-entry.mjs", import.meta.url), { type: "module" });
  return {
    onMessage: (handler) => worker.addEventListener("message", (event) => handler(event.data)),
    onError: (handler) => worker.addEventListener("error", (event) => handler(event.message ?? String(event))),
    post: (message) => worker.postMessage(message),
    terminate: () => worker.terminate(),
  };
}

try {
  const { runSpikeSequence } = await import("./spike-sequence.mjs");
  const host = createHost();
  const result = await runSpikeSequence(host, urls);
  await host.terminate();
  document.querySelector("#result").dataset.state = "passed";
  document.querySelector("#result").textContent = JSON.stringify(result);
} catch (error) {
  document.querySelector("#result").dataset.state = "failed";
  document.querySelector("#result").textContent = JSON.stringify({ error: String(error && error.message ? error.message : error) });
}
