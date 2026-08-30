import { ArtifactLoadAttemptTracker } from "../../dist/artifact-load-attempt.js";

const RUN_COUNT = 3;
const workerUrl = new URL("./measure-worker.mjs", import.meta.url);
const status = document.querySelector("#status");
const device = document.querySelector("#device");
const previewButton = document.querySelector("#measure-preview");
const fullButton = document.querySelector("#measure-full");
const copyButton = document.querySelector("#copy-results");
const report = document.querySelector("#report");
const longRunButton = document.querySelector("#run-long-run");
const longRunStatus = document.querySelector("#long-run-status");
const longRunReport = document.querySelector("#long-run-report");
const limitCandidate = document.querySelector("#limit-candidate");
const limitButton = document.querySelector("#run-limit-test");
const limitStatus = document.querySelector("#limit-status");
const limitReport = document.querySelector("#limit-report");
const copyLimitButton = document.querySelector("#copy-limit-results");
const pressureCandidate = document.querySelector("#pressure-candidate");
const pressureButton = document.querySelector("#run-pressure-test");
const pressureStatus = document.querySelector("#pressure-status");
const pressureReport = document.querySelector("#pressure-report");
const copyPressureButton = document.querySelector("#copy-pressure-results");
const results = { preview: undefined, full: undefined };
const limitAttempts = new ArtifactLoadAttemptTracker(localStorage, {
  attemptKey: "occt-worker-device-limit-attempt",
  resultKey: "occt-worker-device-limit-result",
});
const pressureAttempts = new ArtifactLoadAttemptTracker(localStorage, {
  attemptKey: "occt-worker-device-pressure-attempt",
  resultKey: "occt-worker-device-pressure-result",
});
let limitResult;
let pressureResult;

const rows = [
  ["compileMs", "compileStreaming 耗时", "ms"],
  ["instantiateMb", "实例化后 wasm 线性内存容量", "MB"],
  ["linmemMaxMb", "wasm 线性内存容量", "MB"],
  ["tessellateMs", "三角化耗时", "ms"],
  ["booleanMs", "布尔运算耗时", "ms"],
  ["jsHeapMb", "JS 堆已用容量", "MB"],
  ["uaMemoryMb", "User agent memory", "MB"],
];

for (const profile of ["preview", "full"]) {
  const body = document.querySelector(`#${profile}-results`);
  for (const [key, label, unit] of rows) {
    const row = document.createElement("tr");
    const heading = document.createElement("th");
    const value = document.createElement("td");
    heading.scope = "row";
    heading.textContent = label;
    value.id = `${profile}-${key}`;
    value.textContent = key === "booleanMs" && profile === "preview" ? "n/a" : `待测 ${unit}`;
    row.append(heading, value);
    body.append(row);
  }
}

function median(values) {
  const sorted = [...values].sort((left, right) => left - right);
  return sorted[Math.floor(sorted.length / 2)];
}

function runOnce(profile) {
  return new Promise((resolve, reject) => {
    const worker = new Worker(workerUrl, { type: "module" });
    worker.addEventListener("message", async (event) => {
      if (event.data.type === "error") {
        worker.terminate();
        reject(new Error(event.data.error));
        return;
      }
      if (event.data.type !== "result") return;
      let uaMemoryBytes;
      if (typeof performance.measureUserAgentSpecificMemory === "function") {
        try {
          const measurement = await performance.measureUserAgentSpecificMemory();
          uaMemoryBytes = measurement.bytes;
        } catch {
          uaMemoryBytes = undefined;
        }
      }
      const jsHeapBytes = Number.isFinite(performance.memory?.usedJSHeapSize)
        ? performance.memory.usedJSHeapSize
        : event.data.result.jsHeapBytes;
      worker.terminate();
      resolve({ ...event.data.result, jsHeapBytes, uaMemoryBytes });
    }, { once: true });
    worker.addEventListener("error", (event) => {
      worker.terminate();
      reject(new Error(event.message));
    }, { once: true });
    worker.postMessage({ profile });
  });
}

function formatNumber(value) {
  return value.toFixed(2);
}

function setBusy(busy) {
  previewButton.disabled = busy;
  fullButton.disabled = busy;
  longRunButton.disabled = busy;
  limitButton.disabled = busy;
  pressureButton.disabled = busy;
}

function updateReport() {
  if (results.preview === undefined || results.full === undefined) {
    report.textContent = "preview 和 full 完成后生成";
    copyButton.disabled = true;
    return;
  }
  const jsHeapSupported = results.preview.jsHeapMb !== undefined || results.full.jsHeapMb !== undefined;
  report.textContent = [
    `device: ${device.value.trim() || "<未填写>"}`,
    `preview.compile.ms: ${formatNumber(results.preview.compileMs)}`,
    `preview.instantiate.mb: ${formatNumber(results.preview.instantiateMb)}`,
    `preview.linmem.max.mb: ${formatNumber(results.preview.linmemMaxMb)}`,
    `preview.tessellate.ms: ${formatNumber(results.preview.tessellateMs)}`,
    "preview.boolean.ms: n/a",
    `full.compile.ms: ${formatNumber(results.full.compileMs)}`,
    `full.instantiate.mb: ${formatNumber(results.full.instantiateMb)}`,
    `full.linmem.max.mb: ${formatNumber(results.full.linmemMaxMb)}`,
    `full.tessellate.ms: ${formatNumber(results.full.tessellateMs)}`,
    `full.boolean.ms: ${formatNumber(results.full.booleanMs)}`,
    `jsheap.supported: ${jsHeapSupported ? "yes" : "no"}`,
  ].join("\n");
  copyButton.disabled = false;
}

function render(profile, value) {
  for (const [key, , unit] of rows) {
    const cell = document.querySelector(`#${profile}-${key}`);
    if (key === "booleanMs" && profile === "preview") {
      cell.textContent = "n/a";
    } else if (value[key] === undefined) {
      cell.textContent = "不支持";
    } else {
      cell.textContent = `${formatNumber(value[key])} ${unit}`;
    }
  }
}

async function measure(profile) {
  setBusy(true);
  status.dataset.state = "running";
  const samples = [];
  try {
    for (let run = 1; run <= RUN_COUNT; run++) {
      status.textContent = `${profile}: 第 ${run}/${RUN_COUNT} 次`;
      samples.push(await runOnce(profile));
    }
    const value = {
      compileMs: median(samples.map((sample) => sample.compileMs)),
      instantiateMb: median(samples.map((sample) => sample.instantiateMb)),
      linmemMaxMb: median(samples.map((sample) => sample.linmemMaxMb)),
      tessellateMs: median(samples.map((sample) => sample.tessellateMs)),
      ...(profile === "full" ? { booleanMs: median(samples.map((sample) => sample.booleanMs)) } : {}),
      ...(samples.every((sample) => Number.isFinite(sample.jsHeapBytes))
        ? { jsHeapMb: median(samples.map((sample) => sample.jsHeapBytes / 1024 / 1024)) }
        : {}),
      ...(samples.every((sample) => Number.isFinite(sample.uaMemoryBytes))
        ? { uaMemoryMb: median(samples.map((sample) => sample.uaMemoryBytes / 1024 / 1024)) }
        : {}),
    };
    results[profile] = value;
    render(profile, value);
    updateReport();
    status.dataset.state = "complete";
    status.textContent = `${profile}: 完成`;
  } catch (error) {
    status.dataset.state = "error";
    status.textContent = error instanceof Error ? error.message : String(error);
  } finally {
    setBusy(false);
  }
}

async function copyText(value) {
  if (navigator.clipboard?.writeText !== undefined) {
    await navigator.clipboard.writeText(value);
    return;
  }
  const field = document.createElement("textarea");
  field.value = value;
  field.style.position = "fixed";
  field.style.opacity = "0";
  document.body.append(field);
  field.select();
  const copied = document.execCommand("copy");
  field.remove();
  if (!copied) throw new Error("浏览器拒绝复制结果");
}

async function copyResults() {
  if (device.value.trim() === "") {
    status.dataset.state = "error";
    status.textContent = "请先填写设备、系统版本和浏览器";
    device.focus();
    return;
  }
  updateReport();
  try {
    await copyText(report.textContent);
    status.dataset.state = "complete";
    status.textContent = "结果已复制";
  } catch (error) {
    status.dataset.state = "error";
    status.textContent = error instanceof Error ? error.message : String(error);
  }
}

function runLongRunWorker() {
  return new Promise((resolve, reject) => {
    const worker = new Worker(workerUrl, { type: "module" });
    worker.addEventListener("message", (event) => {
      if (event.data.type === "long-run-progress") {
        longRunStatus.textContent = `full: 第 ${event.data.round}/${event.data.total} 轮`;
        return;
      }
      worker.terminate();
      if (event.data.type === "long-run-result") {
        resolve(event.data.result);
      } else if (event.data.type === "error") {
        reject(new Error(event.data.error));
      }
    });
    worker.addEventListener("error", (event) => {
      worker.terminate();
      reject(new Error(event.message));
    }, { once: true });
    worker.postMessage({ mode: "long-run", profile: "full" });
  });
}

async function runLongRunTest() {
  if (device.value.trim() === "") {
    longRunStatus.dataset.state = "error";
    longRunStatus.textContent = "请先填写设备型号、系统版本和浏览器";
    device.focus();
    return;
  }
  setBusy(true);
  longRunStatus.dataset.state = "running";
  longRunStatus.textContent = "full: 启动";
  longRunReport.textContent = "测试进行中";
  try {
    const result = await runLongRunWorker();
    longRunReport.textContent = JSON.stringify({
      device: device.value.trim(),
      profile: "full",
      rounds: result.rounds.length,
      baseline: {
        liveShapeHandles: result.baseline.liveShapeHandles,
        liveBufferBytes: result.baseline.liveBufferBytes,
      },
      scopeReturnedToBaseline: true,
      wasmLinearMemoryCapacityMbByRound: result.rounds.map((round) => round.linearMemoryMb),
      postWarmupStable: result.postWarmupStable,
      observation: "wasm 线性内存容量仅用于同设备、同负载横向观测，不是系统内存峰值",
    }, null, 2);
    longRunStatus.dataset.state = "complete";
    longRunStatus.textContent = "5 轮完成，scope 统计已回到基线";
  } catch (error) {
    longRunStatus.dataset.state = "error";
    longRunStatus.textContent = error instanceof Error ? error.message : String(error);
    longRunReport.textContent = "测试失败";
  } finally {
    setBusy(false);
  }
}

function renderLimitResult() {
  if (limitResult === undefined) {
    limitReport.textContent = "测试完成或页面恢复后生成";
    copyLimitButton.disabled = true;
    return;
  }
  limitReport.textContent = [
    `device: ${device.value.trim() || limitResult.device || "<未填写>"}`,
    `candidate: ${limitResult.candidate}`,
    `outcome: ${limitResult.outcome}`,
    `classification: ${limitResult.classification ?? (limitResult.outcome === "range-error"
      ? Number(limitResult.declaredMaximumMb) === 4096 ? "wasm32-address-space-maximum" : "candidate-declared-maximum"
      : "unknown")}`,
    `candidate.declared.maximum.mb: ${limitResult.declaredMaximumMb === undefined ? "unknown" : formatNumber(limitResult.declaredMaximumMb)}`,
    `candidate.declared.maximum.pages: ${limitResult.declaredMaximumPages === undefined ? "unknown" : limitResult.declaredMaximumPages}`,
    `wasm.last.observed.capacity.mb: ${limitResult.lastObservedCapacityMb === undefined ? "unknown" : formatNumber(limitResult.lastObservedCapacityMb)}`,
    `wasm.last.observed.capacity.pages: ${limitResult.lastObservedCapacityPages === undefined ? "unknown" : limitResult.lastObservedCapacityPages}`,
    `wasm.touch.checksum: ${limitResult.wasmTouchChecksum === undefined ? "unknown" : limitResult.wasmTouchChecksum}`,
    `wasm32.intrinsic.maximum.mb: ${limitResult.wasm32MaximumMb === undefined ? "4096.00" : formatNumber(limitResult.wasm32MaximumMb)}`,
    `detail: ${limitResult.detail}`,
  ].join("\n");
  copyLimitButton.disabled = false;
}

function storeLimitResult(value) {
  limitResult = value;
  limitAttempts.complete({ ...value, status: value.outcome });
  renderLimitResult();
}

function runLimitWorker(profile, candidateMb) {
  return new Promise((resolve, reject) => {
    const worker = new Worker(workerUrl, { type: "module" });
    let lastMemoryMb;
    let heartbeat = setTimeout(() => {
      worker.terminate();
      resolve({
        outcome: "worker-lost",
        lastObservedCapacityMb: lastMemoryMb,
        detail: "Worker 失联或停止响应",
      });
    }, 30000);
    const resetHeartbeat = () => {
      clearTimeout(heartbeat);
      heartbeat = setTimeout(() => {
        worker.terminate();
        resolve({
          outcome: "worker-lost",
          lastObservedCapacityMb: lastMemoryMb,
          detail: "Worker 失联或停止响应",
        });
      }, 30000);
    };
    worker.addEventListener("message", (event) => {
      resetHeartbeat();
      if (event.data.type === "limit-progress") {
        lastMemoryMb = event.data.memoryMb;
        if (event.data.attemptStage !== undefined) limitAttempts.update(event.data.attemptStage);
        limitStatus.textContent = `${profile} / ${candidateMb} MiB: ${event.data.stage}${lastMemoryMb === undefined ? "" : ` (${formatNumber(lastMemoryMb)} MB)`}`;
        return;
      }
      clearTimeout(heartbeat);
      worker.terminate();
      if (event.data.type === "limit-result") {
        resolve(event.data.result);
      } else if (event.data.type === "error") {
        reject(new Error(event.data.error));
      }
    });
    worker.addEventListener("error", (event) => {
      clearTimeout(heartbeat);
      worker.terminate();
      resolve({ outcome: "worker-lost", lastObservedCapacityMb: lastMemoryMb, detail: event.message });
    }, { once: true });
    worker.postMessage({ mode: "limit", profile, candidateMb });
  });
}

async function runLimitTest() {
  if (device.value.trim() === "") {
    limitStatus.dataset.state = "error";
    limitStatus.textContent = "请先填写设备型号、系统版本和浏览器";
    device.focus();
    return;
  }
  const [profile, candidateMbText] = limitCandidate.value.split(":");
  const candidateMb = Number(candidateMbText);
  const candidate = `${profile}-${candidateMb}`;
  limitAttempts.begin(candidate, "fetching", {
    device: device.value.trim(),
    candidateMb,
  });
  limitResult = undefined;
  renderLimitResult();
  limitStatus.dataset.state = "running";
  limitStatus.textContent = `${profile} / ${candidateMb} MiB: 启动`;
  setBusy(true);
  try {
    const outcome = await runLimitWorker(profile, candidateMb);
    storeLimitResult({
      ...outcome,
      candidate,
      declaredMaximumMb: candidateMb,
      device: device.value.trim(),
    });
    limitStatus.dataset.state = outcome.outcome === "range-error" ? "complete" : "error";
    limitStatus.textContent = outcome.outcome === "range-error"
      ? outcome.classification === "wasm32-address-space-maximum"
        ? "已到达 wasm32 固有 4 GiB 地址空间上限（不代表设备上限）"
        : "已到达候选 wasm 声明上限（不代表设备上限）"
      : outcome.outcome === "allocation-error"
        ? "浏览器在候选声明上限前拒绝增长（不代表设备上限）"
        : "Worker 失联或停止响应";
  } catch (error) {
    const detail = error instanceof Error ? error.message : String(error);
    storeLimitResult({ candidate, device: device.value.trim(), outcome: "error", detail });
    limitStatus.dataset.state = "error";
    limitStatus.textContent = detail;
  } finally {
    setBusy(false);
  }
}

async function copyLimitResults() {
  try {
    await copyText(limitReport.textContent);
    limitStatus.dataset.state = "complete";
    limitStatus.textContent = "二分结果已复制";
  } catch (error) {
    limitStatus.dataset.state = "error";
    limitStatus.textContent = error instanceof Error ? error.message : String(error);
  }
}

function renderPressureResult() {
  if (pressureResult === undefined) {
    pressureReport.textContent = "测试完成或页面恢复后生成";
    copyPressureButton.disabled = true;
    return;
  }
  pressureReport.textContent = [
    `device: ${device.value.trim() || pressureResult.device || "<未填写>"}`,
    `candidate: ${pressureResult.candidate}`,
    `outcome: ${pressureResult.outcome}`,
    `pressure.target.mb: ${pressureResult.targetMb === undefined ? "unknown" : formatNumber(pressureResult.targetMb)}`,
    `pressure.wasm.capacity.mb: ${pressureResult.wasmCapacityMb === undefined ? "unknown" : formatNumber(pressureResult.wasmCapacityMb)}`,
    `pressure.allocated.mb: ${pressureResult.allocatedMb === undefined ? "unknown" : formatNumber(pressureResult.allocatedMb)}`,
    `pressure.touch.checksum: ${pressureResult.touchChecksum === undefined ? "unknown" : pressureResult.touchChecksum}`,
    `detail: ${pressureResult.detail ?? "none"}`,
  ].join("\n");
  copyPressureButton.disabled = false;
}

function storePressureResult(value) {
  pressureResult = value;
  pressureAttempts.complete({ ...value, status: value.outcome });
  renderPressureResult();
}

function runPressureWorker(pressureMb) {
  return new Promise((resolve, reject) => {
    const worker = new Worker(workerUrl, { type: "module" });
    let lastAllocatedMb;
    let heartbeat = setTimeout(() => {
      worker.terminate();
      resolve({
        outcome: "worker-lost",
        allocatedMb: lastAllocatedMb,
        detail: "Worker 失联或停止响应",
      });
    }, 30000);
    const resetHeartbeat = () => {
      clearTimeout(heartbeat);
      heartbeat = setTimeout(() => {
        worker.terminate();
        resolve({
          outcome: "worker-lost",
          allocatedMb: lastAllocatedMb,
          detail: "Worker 失联或停止响应",
        });
      }, 30000);
    };
    worker.addEventListener("message", (event) => {
      resetHeartbeat();
      if (event.data.type === "pressure-progress") {
        lastAllocatedMb = event.data.allocatedMb;
        if (event.data.attemptStage !== undefined) pressureAttempts.update(event.data.attemptStage);
        pressureStatus.textContent = `full + ${pressureMb} MiB: ${event.data.stage ?? "分配并触碰额外内存"}${lastAllocatedMb === undefined ? "" : ` (${formatNumber(lastAllocatedMb)} MB)`}`;
        return;
      }
      clearTimeout(heartbeat);
      worker.terminate();
      if (event.data.type === "pressure-result") {
        resolve(event.data.result);
      } else if (event.data.type === "error") {
        reject(new Error(event.data.error));
      }
    });
    worker.addEventListener("error", (event) => {
      clearTimeout(heartbeat);
      worker.terminate();
      resolve({ outcome: "worker-lost", allocatedMb: lastAllocatedMb, detail: event.message });
    }, { once: true });
    worker.postMessage({ mode: "pressure", profile: "full", pressureMb });
  });
}

async function runPressureTest() {
  if (device.value.trim() === "") {
    pressureStatus.dataset.state = "error";
    pressureStatus.textContent = "请先填写设备型号、系统版本和浏览器";
    device.focus();
    return;
  }
  const pressureMb = Number(pressureCandidate.value);
  const candidate = `full-pressure-${pressureMb}`;
  pressureAttempts.begin(candidate, "fetching", {
    device: device.value.trim(),
    candidateMb: pressureMb,
  });
  pressureResult = undefined;
  renderPressureResult();
  pressureStatus.dataset.state = "running";
  pressureStatus.textContent = `full + ${pressureMb} MiB: 启动`;
  setBusy(true);
  try {
    const outcome = await runPressureWorker(pressureMb);
    storePressureResult({
      ...outcome,
      candidate,
      targetMb: pressureMb,
      device: device.value.trim(),
    });
    pressureStatus.dataset.state = outcome.outcome === "pressure-complete" ? "complete" : "error";
    pressureStatus.textContent = outcome.outcome === "pressure-complete"
      ? "目标压力已触碰；这不是设备安全上限"
      : outcome.outcome === "allocation-error"
        ? "分配失败；这不是设备安全上限"
        : "Worker 失联或停止响应";
  } catch (error) {
    const detail = error instanceof Error ? error.message : String(error);
    storePressureResult({ candidate, targetMb: pressureMb, device: device.value.trim(), outcome: "error", detail });
    pressureStatus.dataset.state = "error";
    pressureStatus.textContent = detail;
  } finally {
    setBusy(false);
  }
}

async function copyPressureResults() {
  try {
    await copyText(pressureReport.textContent);
    pressureStatus.dataset.state = "complete";
    pressureStatus.textContent = "压力测试结果已复制";
  } catch (error) {
    pressureStatus.dataset.state = "error";
    pressureStatus.textContent = error instanceof Error ? error.message : String(error);
  }
}

try {
  const previousAttempt = limitAttempts.unfinished();
  const previousResult = limitAttempts.lastResult();
  if (previousAttempt !== undefined) {
    const attemptedMaximumMb = Number(
      previousAttempt.candidateMb ?? previousAttempt.subject?.split("-").at(-1),
    );
    storeLimitResult({
      candidate: previousAttempt.subject,
      device: previousAttempt.device,
      outcome: "incomplete",
      declaredMaximumMb: Number.isFinite(attemptedMaximumMb) ? attemptedMaximumMb : undefined,
      detail: `上次尝试未完成（阶段：${previousAttempt.stage}）；请结合测试时是否出现白屏或页面重载判断`,
    });
    limitStatus.dataset.state = "error";
    limitStatus.textContent = "上次尝试未完成";
  } else if (previousResult?.candidate !== undefined) {
    const recordedMaximumMb = Number(
      previousResult.declaredMaximumMb ?? previousResult.candidate.split("-").at(-1),
    );
    limitResult = {
      candidate: previousResult.candidate,
      device: previousResult.device,
      outcome: previousResult.outcome ?? previousResult.status,
      classification: previousResult.classification
        ?? ((previousResult.outcome ?? previousResult.status) === "range-error" && Number(recordedMaximumMb) === 4096
          ? "wasm32-address-space-maximum"
          : undefined),
      declaredMaximumMb: Number.isFinite(recordedMaximumMb) ? recordedMaximumMb : undefined,
      declaredMaximumPages: previousResult.declaredMaximumPages,
      lastObservedCapacityMb: previousResult.lastObservedCapacityMb ?? previousResult.memoryMb,
      lastObservedCapacityPages: previousResult.lastObservedCapacityPages,
      wasm32MaximumMb: previousResult.wasm32MaximumMb,
      detail: previousResult.detail,
    };
    renderLimitResult();
    limitStatus.dataset.state = limitResult.outcome === "range-error" ? "complete" : "error";
    limitStatus.textContent = limitResult.outcome === "range-error"
      ? limitResult.classification === "wasm32-address-space-maximum"
        ? "上次结果：已到达 wasm32 固有 4 GiB 地址空间上限"
        : "上次结果：已到达候选 wasm 声明上限"
      : `上次结果：${limitResult.outcome}`;
  }
} catch (error) {
  limitStatus.dataset.state = "error";
  limitStatus.textContent = error instanceof Error ? error.message : String(error);
}

try {
  const previousAttempt = pressureAttempts.unfinished();
  const previousResult = pressureAttempts.lastResult();
  if (previousAttempt !== undefined) {
    const attemptedTargetMb = Number(
      previousAttempt.candidateMb ?? previousAttempt.subject?.split("-").at(-1),
    );
    storePressureResult({
      candidate: previousAttempt.subject,
      device: previousAttempt.device,
      outcome: "incomplete",
      targetMb: Number.isFinite(attemptedTargetMb) ? attemptedTargetMb : undefined,
      detail: `上次压力测试未完成（阶段：${previousAttempt.stage}）；请结合测试时是否出现白屏或页面重载判断`,
    });
    pressureStatus.dataset.state = "error";
    pressureStatus.textContent = "上次压力测试未完成";
  } else if (previousResult?.candidate !== undefined) {
    const recordedTargetMb = Number(
      previousResult.targetMb ?? previousResult.candidate.split("-").at(-1),
    );
    pressureResult = {
      candidate: previousResult.candidate,
      device: previousResult.device,
      outcome: previousResult.outcome ?? previousResult.status,
      targetMb: Number.isFinite(recordedTargetMb) ? recordedTargetMb : undefined,
      wasmCapacityMb: previousResult.wasmCapacityMb,
      allocatedMb: previousResult.allocatedMb,
      touchChecksum: previousResult.touchChecksum,
      detail: previousResult.detail,
    };
    renderPressureResult();
    pressureStatus.dataset.state = pressureResult.outcome === "pressure-complete" ? "complete" : "error";
    pressureStatus.textContent = `上次压力测试结果：${pressureResult.outcome}`;
  }
} catch (error) {
  pressureStatus.dataset.state = "error";
  pressureStatus.textContent = error instanceof Error ? error.message : String(error);
}

previewButton.addEventListener("click", () => measure("preview"));
fullButton.addEventListener("click", () => measure("full"));
copyButton.addEventListener("click", copyResults);
longRunButton.addEventListener("click", runLongRunTest);
limitButton.addEventListener("click", runLimitTest);
copyLimitButton.addEventListener("click", copyLimitResults);
pressureButton.addEventListener("click", runPressureTest);
copyPressureButton.addEventListener("click", copyPressureResults);
device.addEventListener("input", () => {
  updateReport();
  renderLimitResult();
  renderPressureResult();
});
