import { BUILD_IDENTITY, RUNTIME_CONFIG, createWorkerProfileRuntime } from "../../dist/index.js";

const result = document.querySelector("#result");
const workerFactory = () => new Worker(new URL("../../dist/worker-entry.js", import.meta.url), { type: "module" });
const official = RUNTIME_CONFIG.profiles["full-profile"].artifact;

function finish(value, passed) {
  result.dataset.state = passed ? "passed" : "failed";
  result.textContent = JSON.stringify(value);
}

let runtime;
try {
  runtime = await createWorkerProfileRuntime({
    artifact: {
      name: "vendor-full.wasm",
      sha256: official.sha256,
      protocolVersion: BUILD_IDENTITY.protocolVersion,
      abiVersion: BUILD_IDENTITY.pluginAbiVersion,
      buildFamily: BUILD_IDENTITY.buildFamilies.isolated,
    },
    resolve: () => new URL("../../artifacts/full.wasm", import.meta.url),
    createWorker: workerFactory,
  })();
  const capabilities = await runtime.request("capabilities", {});
  let mismatchMessage = "";
  try {
    await createWorkerProfileRuntime({
      artifact: { name: "invalid.wasm", buildFamily: "vendor-family" },
      resolve: () => new URL("../../artifacts/full.wasm", import.meta.url),
      createWorker: workerFactory,
    })();
  } catch (error) {
    mismatchMessage = error instanceof Error ? error.message : String(error);
  }
  finish({
    protocolVersion: capabilities.protocolVersion,
    operationCount: capabilities.ops.length,
    resolvedOperation: capabilities.ops.includes("makeBox") && capabilities.ops.includes("importSTEP"),
    mismatchMessage,
  }, capabilities.protocolVersion === BUILD_IDENTITY.protocolVersion
    && capabilities.ops.includes("makeBox")
    && capabilities.ops.includes("importSTEP")
    && /declares build family/.test(mismatchMessage));
} catch (error) {
  finish({ error: error instanceof Error ? error.message : String(error) }, false);
} finally {
  await runtime?.close?.();
}
