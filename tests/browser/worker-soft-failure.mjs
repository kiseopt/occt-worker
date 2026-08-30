import { WorkerClient } from "../../dist/worker-client.js";

const failure = document.querySelector("#failure");
const uiProbe = document.querySelector("#ui-probe");
const uiResult = document.querySelector("#ui-result");
uiProbe.addEventListener("click", () => { uiResult.textContent = "alive"; });

let kernel;
try {
  const wasm = await (await fetch("../../wasm/occt-worker.wasm")).arrayBuffer();
  const factory = () => new Worker(new URL("./worker-soft-failure-worker.mjs", import.meta.url), { type: "module" });
  kernel = await WorkerClient.create(factory, wasm);
  await kernel.stats();
  throw new Error("Worker request unexpectedly succeeded");
} catch (error) {
  const detail = error instanceof Error ? `${error.name}: ${error.message}` : String(error);
  failure.dataset.state = detail.includes("Worker memory limit reached") ? "passed" : "failed";
  failure.textContent = detail;
} finally {
  kernel?.close();
}
