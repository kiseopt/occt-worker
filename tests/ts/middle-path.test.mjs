import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { DirectClient } from "../../dist/direct-client.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));
const near = (actual, expected, tolerance = 1e-5) => {
  assert.ok(Math.abs(actual - expected) <= tolerance, `${actual} != ${expected}`);
};

test("middlePath extracts the centerline between pipe-like end faces", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const box = await scope.makeBox([10, 100, 10]);
  const faces = await Promise.all(
    Array.from({ length: 6 }, (_, index) => scope.getSubShape(box, "face", index)),
  );
  const bounds = await Promise.all(faces.map((face) => client.bbox(face)));
  const startIndex = bounds.findIndex(({ min, max }) => Math.abs(min[1]) < 1e-5 && Math.abs(max[1]) < 1e-5);
  const endIndex = bounds.findIndex(({ min, max }) => Math.abs(min[1] - 100) < 1e-5 && Math.abs(max[1] - 100) < 1e-5);
  assert.notEqual(startIndex, -1);
  assert.notEqual(endIndex, -1);

  const path = await scope.middlePath(box, faces[startIndex], faces[endIndex]);
  assert.equal(await client.isValid(path), true);
  assert.equal(await client.shapeType(path), "wire");
  near((await client.massProps(path, "linear")).mass, 100);
  const pathBounds = await client.bbox(path);
  near(pathBounds.min[0], 5);
  near(pathBounds.max[0], 5);
  near(pathBounds.min[1], 0);
  near(pathBounds.max[1], 100);
  near(pathBounds.min[2], 5);
  near(pathBounds.max[2], 5);

  const otherBox = await scope.makeBox([10, 100, 10], [20, 0, 0]);
  const otherFace = await scope.getSubShape(otherBox, "face", startIndex);
  await assert.rejects(
    scope.middlePath(box, otherFace, faces[endIndex]),
    (error) => error.code === "InvalidArgs",
  );

  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});
