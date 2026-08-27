// occt-worker dylink spike verifier.
//
// Builds the Main+Side dynamic linking spike (unless --no-build) and verifies
// the required checklist on both hosts:
//   1. Node worker_threads (module worker)
//   2. Browser Worker (Playwright)
//
// Usage: node scripts/verify-dylink-spike.mjs [--no-build] [--browser=chromium]

import assert from "node:assert/strict";
import { spawn } from "node:child_process";
import { createServer } from "node:http";
import { readFile, stat } from "node:fs/promises";
import { extname, join, normalize } from "node:path";
import { fileURLToPath, pathToFileURL } from "node:url";
import { Worker } from "node:worker_threads";

const repoRoot = fileURLToPath(new URL("..", import.meta.url));
const spikeDir = join(repoRoot, "spikes", "dylink");
const artifactDir = join(repoRoot, "build", "release", "dylink-spike");

const args = process.argv.slice(2);
const noBuild = args.includes("--no-build");
const browserName = args.find((argument) => argument.startsWith("--browser="))?.split("=")[1] ?? "chromium";

function run(command, commandArgs) {
  return new Promise((resolve, reject) => {
    const child = spawn(command, commandArgs, { stdio: "inherit", cwd: repoRoot, shell: process.platform === "win32" });
    child.on("exit", (code) => (code === 0 ? resolve() : reject(new Error(`${command} exited with ${code}`))));
    child.on("error", reject);
  });
}

async function buildSpike() {
  await run("powershell.exe", [
    "-NoProfile",
    "-ExecutionPolicy",
    "Bypass",
    "-File",
    join("spikes", "dylink", "build.ps1"),
  ]);
}

if (noBuild) {
  for (const name of ["dylink-main.mjs", "dylink-main.wasm", "dylink-side.wasm"]) {
    try {
      await stat(join(artifactDir, name));
    } catch {
      throw new Error(`missing spike artifact ${name}; run without --no-build`);
    }
  }
} else {
  await buildSpike();
}

const { runSpikeSequence } = await import(new URL("../spikes/dylink/spike-sequence.mjs", import.meta.url).href);

// ---------------------------------------------------------------------------
// Host 1: Node worker_threads
// ---------------------------------------------------------------------------

function createNodeHost() {
  const worker = new Worker(join(spikeDir, "worker-entry.mjs"), { type: "module" });
  return {
    urls: {
      mainJsUrl: pathToFileURL(join(artifactDir, "dylink-main.mjs")).href,
      mainWasmUrl: join(artifactDir, "dylink-main.wasm"),
      sideUrl: join(artifactDir, "dylink-side.wasm"),
      badSideUrl: join(artifactDir, "missing-side.wasm"),
    },
    onMessage: (handler) => worker.on("message", handler),
    onError: (handler) => {
      worker.on("error", (error) => handler(String(error.message ?? error)));
      worker.on("exit", (code) => {
        if (code !== 0) handler(`worker exited with code ${code}`);
      });
    },
    post: (message) => worker.postMessage(message),
    terminate: async () => {
      await worker.terminate();
    },
  };
}

console.log("[dylink-spike] running Node worker sequence...");
{
  const host = createNodeHost();
  const nodeResults = await runSpikeSequence(host, host.urls);
  console.log("[dylink-spike] node:", JSON.stringify(nodeResults));

  // Epoch rebuild: the terminated worker must reject further traffic, and a
  // rebuilt worker must start from an entirely fresh plugin registry (its own
  // side load registers pluginId 0 exactly once).
  const terminated = host.terminate();
  let staleResponseReceived = false;
  host.onMessage((message) => {
    if (message?.callId === "stale") staleResponseReceived = true;
  });
  try {
    await terminated;
    host.post({ type: "call", callId: "stale", op: "identity", input: 1 });
  } catch {}
  await new Promise((resolve) => setTimeout(resolve, 25));
  assert.equal(staleResponseReceived, false, "terminated worker must not answer stale traffic");
  console.log("[dylink-spike] stale worker rejected traffic: true");

  const second = createNodeHost();
  const fresh = await new Promise((resolve, reject) => {
    const timer = setTimeout(() => reject(new Error("timeout waiting for rebuilt worker init")), 30000);
    second.onMessage((data) => {
      if (data && data.type === "ready") {
        clearTimeout(timer);
        resolve(data);
      }
    });
    second.onError((message) => {
      clearTimeout(timer);
      reject(new Error(message));
    });
    second.post({ type: "init", epoch: 2, ...second.urls });
  });
  assert.equal(fresh.info.registrations, 1, "rebuilt worker must register the plugin exactly once");
  assert.equal(fresh.info.pluginId, 0, "rebuilt worker registry must start empty");
  console.log("[dylink-spike] epoch rebuild ok:", JSON.stringify(fresh.info));
  await second.terminate();
}

// ---------------------------------------------------------------------------
// Host 2: browser Worker
// ---------------------------------------------------------------------------

async function runBrowserPhase() {
  const root = normalize(repoRoot);
  const contentTypes = { ".html": "text/html", ".js": "text/javascript", ".mjs": "text/javascript", ".wasm": "application/wasm" };
  const server = createServer(async (request, response) => {
    try {
      const pathname = decodeURIComponent(new URL(request.url, "http://127.0.0.1").pathname);
      const candidate = normalize(join(root, pathname));
      if (!candidate.startsWith(root) || !(await stat(candidate)).isFile()) throw new Error("not found");
      response.writeHead(200, {
        "Content-Type": contentTypes[extname(candidate)] ?? "application/octet-stream",
        "Cross-Origin-Opener-Policy": "same-origin",
        "Cross-Origin-Embedder-Policy": "require-corp",
      });
      response.end(await readFile(candidate));
    } catch {
      response.writeHead(404).end("Not found");
    }
  });
  await new Promise((resolve) => server.listen(0, "127.0.0.1", resolve));
  const address = server.address();
  let browser;
  try {
    const playwright = await import("playwright");
    const browserType = { chromium: playwright.chromium, firefox: playwright.firefox, webkit: playwright.webkit }[browserName];
    if (browserType === undefined) throw new Error(`unsupported browser ${browserName}`);
    browser = await browserType.launch({ headless: true });
    const page = await browser.newPage();
    page.on("console", (message) => {
      if (message.type() === "error") console.log(`[browser:${browserName}]`, message.text());
    });
    page.on("pageerror", (error) => console.log(`[browser:${browserName}] pageerror:`, String(error)));
    await page.goto(`http://127.0.0.1:${address.port}/spikes/dylink/browser-driver.html`);
    await page.waitForFunction(
      () => document.querySelector("#result") !== null && document.querySelector("#result").dataset.state !== "running",
      undefined,
      { timeout: 120000 },
    );
    assert.equal(await page.getAttribute("#result", "data-state"), "passed", await page.textContent("#result"));
    return JSON.parse(await page.textContent("#result"));
  } finally {
    if (browser) await browser.close();
    await new Promise((resolve, reject) => server.close((error) => (error ? reject(error) : resolve())));
  }
}

const browserResults = await runBrowserPhase();
console.log(`[dylink-spike] browser(${browserName}):`, JSON.stringify(browserResults));

console.log(JSON.stringify({
  spike: "dylink",
  node: "pass",
  browser: browserName,
  checklist: {
    glueLocatesMainWasm: true,
    asyncSideLoad: true,
    singleRegistration: true,
    sharedMemoryTableAllocator: true,
    hostCallbackFromSide: true,
    exceptionsContained: true,
    callsValidAfterGrowth: true,
    trapAndLoadFailureCaptured: true,
    epochRebuildInvalidatesOldState: true,
  },
}, null, 2));
