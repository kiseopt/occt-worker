const RUN_COUNT = 3;
const workerUrl = new URL("./measure-worker.mjs", import.meta.url);
const status = document.querySelector("#status");
const device = document.querySelector("#device");
const previewButton = document.querySelector("#measure-preview");
const fullButton = document.querySelector("#measure-full");
const copyButton = document.querySelector("#copy-results");
const report = document.querySelector("#report");
const results = { preview: undefined, full: undefined };

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
  previewButton.disabled = true;
  fullButton.disabled = true;
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
    previewButton.disabled = false;
    fullButton.disabled = false;
  }
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
    if (navigator.clipboard?.writeText !== undefined) {
      await navigator.clipboard.writeText(report.textContent);
    } else {
      const field = document.createElement("textarea");
      field.value = report.textContent;
      field.style.position = "fixed";
      field.style.opacity = "0";
      document.body.append(field);
      field.select();
      if (!document.execCommand("copy")) throw new Error("浏览器拒绝复制结果");
      field.remove();
    }
    status.dataset.state = "complete";
    status.textContent = "结果已复制";
  } catch (error) {
    status.dataset.state = "error";
    status.textContent = error instanceof Error ? error.message : String(error);
  }
}

previewButton.addEventListener("click", () => measure("preview"));
fullButton.addEventListener("click", () => measure("full"));
copyButton.addEventListener("click", copyResults);
device.addEventListener("input", updateReport);
