import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { DirectClient } from "../../dist/direct-client.js";

const wasm = await readFile(
  process.env.OCCT_P1_WASM_PATH ?? new URL("../../artifacts/full.wasm", import.meta.url),
);

test("full-profile tessellates a multi-section loft after multi-edge filleting", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const section = (z, x, y, corner) => scope.makePolygon([
    [-x, -y, z], [x, -y, z], [x + corner, -y / 2, z], [x + corner, y / 2, z],
    [x, y, z], [-x, y, z], [-x - corner, y / 2, z], [-x - corner, -y / 2, z],
  ], true);

  const lower = await section(0, 8, 5, 2);
  const middle = await section(8, 7, 4, 2);
  const upper = await section(16, 5, 3, 2);
  const loft = await scope.loft([lower, middle, upper], { solid: true });
  const fillet = await scope.fillet(loft, [0, 2, 4, 6], 0.6);
  const counts = await client.topologyCounts(fillet);
  const mesh = await client.tessellate(fillet, {
    linearDeflection: 0.1,
    angularDeflection: 0.3,
  });

  assert.equal(await client.isValid(fillet), true);
  assert.equal(counts.solid, 1);
  assert.ok(counts.face >= 14);
  assert.ok(mesh.indices.length / 3 >= 200);
  assert.ok([...mesh.positions].every(Number.isFinite));
  await scope.end();
});
