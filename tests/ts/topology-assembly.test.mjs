import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { DirectClient } from "../../dist/direct-client.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));

test("direct compound, shell, and compsolid construction produces reusable topology", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  const box = await scope.makeBox([2, 3, 4]);
  const sphere = await scope.makeSphere(1, { origin: [5, 0, 0] });

  const compound = await scope.makeCompound([box, sphere]);
  assert.equal(await client.shapeType(compound), "compound");
  assert.equal((await client.topologyCounts(compound)).solid, 2);

  const faces = await Promise.all(
    Array.from({ length: 6 }, (_, index) => scope.getSubShape(box, "face", index)),
  );
  const shell = await scope.makeShell(faces);
  assert.equal(await client.shapeType(shell), "shell");
  assert.equal(await client.isValid(shell), true);
  const rebuiltSolid = await scope.makeSolidFromShell(shell);
  assert.equal(await client.shapeType(rebuiltSolid), "solid");
  assert.ok(Math.abs((await client.massProps(rebuiltSolid)).mass - 24) < 1e-8);

  const compSolid = await scope.makeCompSolid([box]);
  assert.equal(await client.shapeType(compSolid), "compsolid");
  assert.equal((await client.topologyCounts(compSolid)).solid, 1);

  await assert.rejects(
    scope.makeShell([box]),
    (error) => error.code === "InvalidArgs",
  );
  await assert.rejects(
    scope.makeCompSolid([faces[0]]),
    (error) => error.code === "InvalidArgs",
  );
  await assert.rejects(
    scope.makeCompound([]),
    (error) => error.code === "InvalidArgs",
  );
  await scope.end();
  assert.equal((await client.stats()).liveShapeHandles, 0);
});
