import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import { DirectClient } from "../../dist/direct-client.js";

const bytes = await readFile(new URL("../../build/debug/occt-worker.debug.wasm", import.meta.url));
const client = await DirectClient.create(bytes);
const scope = await client.beginScope();

const plate = await scope.makeBox([20, 10, 2]);
const hole = await scope.makeCylinder(2, 2, { origin: [10, 5, 0] });
const cut = await scope.booleanCut(plate, [hole]);
const bbox = await client.bbox(cut.shape);
const mesh = await client.tessellate(cut.shape, { linearDeflection: 0.2, angularDeflection: 0.3 });

assert.deepEqual(bbox.min, [-1e-7, -1e-7, -1e-7]);
assert.ok(mesh.indices.byteLength > 0);
await scope.end();
const stats = await client.stats();
assert.equal(stats.liveShapeHandles, 0);
assert.equal(stats.liveBufferBytes, 0);

console.log(JSON.stringify({ debugBytes: bytes.byteLength, triangles: mesh.indices.byteLength / 12 }));
