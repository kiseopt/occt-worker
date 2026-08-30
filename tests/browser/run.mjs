import assert from "node:assert/strict";
import { createServer } from "node:http";
import { readFile, stat } from "node:fs/promises";
import { extname, join, normalize } from "node:path";
import { chromium, firefox, webkit } from "playwright";

const browserName = process.argv.find((argument) => argument.startsWith("--browser="))?.split("=")[1] ?? "chromium";
const browserType = { chromium, firefox, webkit }[browserName];
if (browserType === undefined) throw new Error(`Unsupported browser: ${browserName}`);

const root = new URL("../../", import.meta.url).pathname.replace(/^\/(.:)/, "$1");
const contentTypes = { ".html": "text/html", ".js": "text/javascript", ".mjs": "text/javascript", ".wasm": "application/wasm" };
const server = createServer(async (request, response) => {
  try {
    const pathname = decodeURIComponent(new URL(request.url, "http://127.0.0.1").pathname);
    const candidate = normalize(join(root, pathname));
    if (!candidate.startsWith(normalize(root)) || !(await stat(candidate)).isFile()) throw new Error("not found");
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
const browser = await browserType.launch({ headless: true });
try {
  const page = await browser.newPage();
  await page.goto(`http://127.0.0.1:${address.port}/examples/browser/index.html`);
  const result = page.locator("#result");
  await result.waitFor({ state: "visible" });
  await page.waitForFunction(() => document.querySelector("#result")?.dataset.state !== "running");
  assert.equal(await result.getAttribute("data-state"), "passed", await result.textContent());
  const value = JSON.parse(await result.textContent());
  assert.deepEqual(value.bbox, value.restoredBBox);
  assert.ok(value.triangles > 0);
  assert.equal(value.sharedTransport, true);
  assert.equal(value.stats.liveShapeHandles, 0);
  assert.equal(value.stats.liveBufferBytes, 0);

  const failurePage = await browser.newPage();
  await failurePage.goto(`http://127.0.0.1:${address.port}/tests/browser/worker-soft-failure.html`);
  const failure = failurePage.locator("#failure");
  await failure.waitFor({ state: "visible" });
  await failurePage.waitForFunction(() => document.querySelector("#failure")?.dataset.state !== "running");
  assert.equal(await failure.getAttribute("data-state"), "passed", await failure.textContent());
  assert.match(await failure.textContent(), /Worker memory limit reached/);
  await failurePage.locator("#ui-probe").click();
  assert.equal(await failurePage.locator("#ui-result").textContent(), "alive");

  const memoryPage = await browser.newPage();
  await memoryPage.goto(`http://127.0.0.1:${address.port}/tests/device/index.html`);
  await memoryPage.locator("#device").fill(`${browserName} desktop memory regression`);
  await memoryPage.locator("#run-long-run").click();
  await memoryPage.waitForFunction(
    () => document.querySelector("#long-run-status")?.dataset.state !== "running",
    undefined,
    { timeout: 180000 },
  );
  const memoryStatus = memoryPage.locator("#long-run-status");
  assert.equal(await memoryStatus.getAttribute("data-state"), "complete", await memoryStatus.textContent());
  const memoryResult = JSON.parse(await memoryPage.locator("#long-run-report").textContent());
  assert.equal(memoryResult.scopeReturnedToBaseline, true);
  assert.equal(memoryResult.baseline.liveShapeHandles, 0);
  assert.equal(memoryResult.baseline.liveBufferBytes, 0);
  assert.equal(memoryResult.postWarmupStable, true);

  const attemptPage = await browser.newPage();
  const attemptUrl = `http://127.0.0.1:${address.port}/tests/browser/artifact-load-attempt.html`;
  for (const mode of ["success", "failure", "reload"]) {
    await attemptPage.goto(`${attemptUrl}?mode=${mode}`);
    const attemptResult = attemptPage.locator("#result");
    await attemptPage.waitForFunction(
      () => document.querySelector("#result")?.dataset.state !== "running",
      undefined,
      { timeout: 120000 },
    );
    assert.equal(await attemptResult.getAttribute("data-state"), "passed", await attemptResult.textContent());
  }

  console.log(JSON.stringify({
    browser: browserName,
    triangles: value.triangles,
    workerSoftFailure: true,
    longRunRounds: memoryResult.rounds,
    longRunLinearMemoryMb: memoryResult.wasmLinearMemoryCapacityMbByRound,
    fullLoadAttemptStates: ["success", "failure", "reload"],
  }));
} finally {
  await browser.close();
  await new Promise((resolve, reject) => server.close((error) => error ? reject(error) : resolve()));
}
