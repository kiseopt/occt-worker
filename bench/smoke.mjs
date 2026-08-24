import { createHash } from "node:crypto";
import { performance } from "node:perf_hooks";
import { readFile } from "node:fs/promises";
import { DirectClient } from "../dist/direct-client.js";

const argument = process.argv.find((value) => value.startsWith("--iterations="));
const iterations = argument === undefined ? 5 : Number(argument.split("=")[1]);
if (!Number.isInteger(iterations) || iterations < 1) throw new Error("iterations must be a positive integer");
const baselineArgument = process.argv.find((value) => value.startsWith("--baseline="));

const wasm = await readFile(new URL("../wasm/occt-worker.wasm", import.meta.url));
const client = await DirectClient.create(wasm);
const scope = await client.beginScope();
const samples = { booleanCutMs: [], filletMs: [], tessellateMs: [] };

for (let index = 0; index < iterations; index++) {
  const plate = await scope.makeBox([100, 60, 4]);
  const hole = await scope.makeCylinder(8, 4, { origin: [50, 30, 0] });
  let started = performance.now();
  const cut = await scope.booleanCut(plate, [hole]);
  samples.booleanCutMs.push(performance.now() - started);

  const box = await scope.makeBox([40, 40, 40]);
  started = performance.now();
  const fillet = await scope.fillet(box, [0, 1, 2, 3], 2);
  samples.filletMs.push(performance.now() - started);

  started = performance.now();
  await client.tessellate(fillet, { linearDeflection: 0.1, angularDeflection: 0.3 });
  samples.tessellateMs.push(performance.now() - started);
  await client.release(cut.shape);
}

await scope.end();
const stats = await client.stats();
if (stats.liveShapeHandles !== 0 || stats.liveBufferBytes !== 0) {
  throw new Error(`benchmark leaked resources: ${JSON.stringify(stats)}`);
}
const summarize = (values) => ({
  minimum: Math.min(...values),
  median: [...values].sort((a, b) => a - b)[Math.floor(values.length / 2)],
  maximum: Math.max(...values),
});
const summary = Object.fromEntries(Object.entries(samples).map(([name, values]) => [name, summarize(values)]));
if (baselineArgument !== undefined) {
  const baseline = JSON.parse(await readFile(baselineArgument.slice("--baseline=".length), "utf8"));
  const artifactSha256 = createHash("sha256").update(wasm).digest("hex");
  if (artifactSha256 !== baseline.artifactSha256) {
    throw new Error(`benchmark artifact hash ${artifactSha256} does not match baseline ${baseline.artifactSha256}`);
  }
  for (const [name, expectedMedian] of Object.entries(baseline.medianMs)) {
    const actualMedian = summary[name]?.median;
    if (actualMedian === undefined) throw new Error(`baseline references unknown benchmark: ${name}`);
    const ratio = actualMedian / expectedMedian;
    summary[name].baselineRatio = ratio;
    if (ratio > baseline.maximumRegressionFactor) {
      throw new Error(`${name} median regressed by ${ratio.toFixed(2)}x (limit ${baseline.maximumRegressionFactor}x)`);
    }
  }
}
console.log(JSON.stringify(summary, null, 2));
