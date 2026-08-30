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

  console.log(JSON.stringify({ browser: browserName, triangles: value.triangles, workerSoftFailure: true }));
} finally {
  await browser.close();
  await new Promise((resolve, reject) => server.close((error) => error ? reject(error) : resolve()));
}
