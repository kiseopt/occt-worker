import {
  ArtifactLoadAttemptTracker,
  createWorkerProfileRuntime,
} from "../../dist/index.js";

const result = document.querySelector("#result");
const mode = new URLSearchParams(location.search).get("mode");
const tracker = new ArtifactLoadAttemptTracker(localStorage, {
  attemptKey: "occt-worker-test:attempting-full",
  resultKey: "occt-worker-test:full-load-result",
});

function finish(value, passed) {
  result.dataset.state = passed ? "passed" : "failed";
  result.textContent = JSON.stringify(value);
}

if (mode === "success") {
  tracker.clear();
  const stages = [];
  let runtime;
  try {
    runtime = await createWorkerProfileRuntime({
      profile: "full-profile",
      createWorker: () => new Worker(new URL("../../dist/worker-entry.js", import.meta.url), { type: "module" }),
      resolve: () => new URL("../../artifacts/full.wasm", import.meta.url),
      loadAttempt: tracker,
      onLoadStage: (stage) => stages.push(stage),
    })();
    const stats = await runtime.request("stats", {});
    const lastResult = tracker.lastResult();
    finish({ stages, stats, unfinished: tracker.unfinished(), lastResult },
      stages.join(",") === "fetching,compiling,instantiating,ready"
      && tracker.unfinished() === undefined
      && lastResult?.status === "success");
  } catch (error) {
    finish({ error: error instanceof Error ? error.message : String(error), stages }, false);
  } finally {
    await runtime?.close?.();
  }
} else if (mode === "failure") {
  tracker.clear();
  try {
    await createWorkerProfileRuntime({
      profile: "full-profile",
      createWorker: () => new Worker(new URL("../../dist/worker-entry.js", import.meta.url), { type: "module" }),
      loadArtifact: async () => { throw new Error("capturable load failure"); },
      loadAttempt: tracker,
    })();
    finish({ error: "load unexpectedly succeeded" }, false);
  } catch (error) {
    const lastResult = tracker.lastResult();
    finish({
      error: error instanceof Error ? error.message : String(error),
      unfinished: tracker.unfinished(),
      lastResult,
    }, tracker.unfinished() === undefined
      && lastResult?.status === "failed"
      && String(lastResult.reason).includes("capturable load failure"));
  }
} else if (mode === "reload") {
  const reloadKey = "occt-worker-test:reload-seeded";
  if (sessionStorage.getItem(reloadKey) === null) {
    tracker.clear();
    tracker.begin("full.wasm", "instantiating");
    sessionStorage.setItem(reloadKey, "yes");
    location.reload();
  } else {
    sessionStorage.removeItem(reloadKey);
    const unfinished = tracker.unfinished();
    const userMessage = unfinished === undefined ? "" : "上次加载完整功能未能完成";
    finish({ unfinished, userMessage },
      unfinished?.subject === "full.wasm"
      && unfinished.stage === "instantiating"
      && userMessage === "上次加载完整功能未能完成");
    tracker.clear();
  }
} else {
  finish({ error: `Unknown mode '${mode}'` }, false);
}
