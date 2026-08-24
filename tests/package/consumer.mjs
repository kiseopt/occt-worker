import assert from "node:assert/strict";
import { execFileSync } from "node:child_process";
import { createServer } from "node:http";
import { mkdir, mkdtemp, readFile, rm, stat, writeFile } from "node:fs/promises";
import { tmpdir } from "node:os";
import { extname, join, normalize } from "node:path";
import { fileURLToPath } from "node:url";
import { chromium } from "playwright";

const repository = fileURLToPath(new URL("../..", import.meta.url));
const temporary = await mkdtemp(join(tmpdir(), "occt-worker-package-"));
const npmCli = process.env.npm_execpath;
if (npmCli === undefined) throw new Error("Run this test through npm run test:package");
const npm = (args, options = {}) => execFileSync(process.execPath, [npmCli, ...args], options);

try {
  const pack = JSON.parse(npm(["pack", "--json", "--pack-destination", temporary], {
    cwd: repository,
    encoding: "utf8",
  }))[0];
  const tarball = join(temporary, pack.filename);
  const unpacked = join(temporary, "unpacked");
  await mkdir(unpacked);
  execFileSync("tar", ["-xf", tarball, "-C", unpacked]);
  npm(["ci", "--ignore-scripts", "--no-audit", "--no-fund"], {
    cwd: join(unpacked, "package"),
    stdio: "ignore",
  });
  npm(["init", "-y"], { cwd: temporary, stdio: "ignore" });
  npm(["install", "--ignore-scripts", "--no-audit", "--no-fund", tarball], { cwd: temporary, stdio: "ignore" });
  await writeFile(join(temporary, "smoke.mjs"), `
    import assert from "node:assert/strict";
    import { readFile } from "node:fs/promises";
    import { DirectClient } from "occt-worker";
    const wasmUrl = import.meta.resolve("occt-worker/wasm");
    assert.ok(import.meta.resolve("occt-worker/worker").endsWith("/dist/worker-entry.js"));
    const client = await DirectClient.create(await readFile(new URL(wasmUrl)));
    const scope = await client.beginScope();
    const box = await scope.makeBox([2, 3, 4]);
    assert.ok(Math.abs((await client.massProps(box)).mass - 24) < 1e-9);
    await scope.end();
    assert.equal((await client.stats()).liveShapeHandles, 0);
  `);
  execFileSync(process.execPath, ["smoke.mjs"], { cwd: temporary, stdio: "inherit" });
  await writeFile(join(temporary, "browser-smoke.mjs"), `
    import { WorkerClient } from "/node_modules/occt-worker/dist/index.js";
    const result = document.querySelector("#result");
    try {
      const wasm = await (await fetch("/node_modules/occt-worker/wasm/occt-worker.wasm")).arrayBuffer();
      const kernel = await WorkerClient.create(
        () => new Worker("/node_modules/occt-worker/dist/worker-entry.js", { type: "module" }),
        wasm,
      );
      const scope = await kernel.beginScope();
      const box = await scope.makeBox([2, 3, 4]);
      const bbox = await kernel.bbox(box);
      await scope.end();
      const stats = await kernel.stats();
      kernel.close();
      result.dataset.state = "passed";
      result.textContent = JSON.stringify({ bbox, stats });
    } catch (error) {
      result.dataset.state = "failed";
      result.textContent = error instanceof Error ? error.stack ?? error.message : String(error);
    }
  `);
  await writeFile(join(temporary, "browser-smoke.html"), `<!doctype html><pre id="result" data-state="running"></pre><script type="module" src="./browser-smoke.mjs"></script>`);
  const contentTypes = { ".html": "text/html", ".js": "text/javascript", ".mjs": "text/javascript", ".wasm": "application/wasm" };
  const server = createServer(async (request, response) => {
    try {
      const pathname = decodeURIComponent(new URL(request.url, "http://127.0.0.1").pathname);
      const candidate = normalize(join(temporary, pathname));
      if (!candidate.startsWith(normalize(temporary)) || !(await stat(candidate)).isFile()) throw new Error("not found");
      response.writeHead(200, { "Content-Type": contentTypes[extname(candidate)] ?? "application/octet-stream" });
      response.end(await readFile(candidate));
    } catch {
      response.writeHead(404).end("Not found");
    }
  });
  await new Promise((resolve) => server.listen(0, "127.0.0.1", resolve));
  const address = server.address();
  const browser = await chromium.launch({ headless: true });
  try {
    const page = await browser.newPage();
    await page.goto(`http://127.0.0.1:${address.port}/browser-smoke.html`);
    const result = page.locator("#result");
    await result.waitFor({ state: "visible" });
    await page.waitForFunction(() => document.querySelector("#result")?.dataset.state !== "running");
    assert.equal(await result.getAttribute("data-state"), "passed", await result.textContent());
    const value = JSON.parse(await result.textContent());
    assert.deepEqual(value.bbox.max.map(Math.round), [2, 3, 4]);
    assert.equal(value.stats.liveShapeHandles, 0);
    assert.equal(value.stats.liveBufferBytes, 0);
  } finally {
    await browser.close();
    await new Promise((resolve, reject) => server.close((error) => error ? reject(error) : resolve()));
  }
  const installedPackage = JSON.parse(await readFile(join(temporary, "node_modules/occt-worker/package.json"), "utf8"));
  assert.equal(installedPackage.version, "1.0.0");
  assert.ok(pack.files.some(({ path }) => path === "SOURCE.md"));
  assert.ok(pack.files.some(({ path }) => path === "npm-shrinkwrap.json"));
  assert.ok(pack.files.some(({ path }) => path === "THIRD-PARTY-NOTICES.txt"));
  assert.ok(!pack.files.some(({ path }) => path.endsWith("debug.wasm")));
  console.log(JSON.stringify({ files: pack.files.length, packageSize: pack.size }));
} finally {
  await rm(temporary, { recursive: true, force: true });
}
