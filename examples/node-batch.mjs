import { readFile } from "node:fs/promises";
import { DirectClient } from "../dist/direct-client.js";

const wasm = await readFile(new URL("../wasm/occt-worker.wasm", import.meta.url));
const kernel = await DirectClient.create(wasm);
const scope = await kernel.beginScope();
const result = await scope.batch([
    { op: "makeBox", args: { size: [10, 10, 4] } },
    { op: "makeCylinder", args: { radius: 2, height: 4, origin: [5, 5, 0] } },
    { op: "booleanCut", args: { base: { $ref: 0 }, tools: [{ $ref: 1 }] } },
]);
if (result.error !== undefined || result.results[2]?.shape === undefined) {
  throw new Error(result.error?.message ?? "Batch did not return a shape");
}
console.log(await kernel.bbox(result.results[2].shape));
await scope.end();
