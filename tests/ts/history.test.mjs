import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { DirectClient } from "../../dist/direct-client.js";

const wasm = await readFile(new URL("../../wasm/occt-worker.wasm", import.meta.url));

function assertFullHistory(history, inputCounts, outputCounts) {
  assert.deepEqual(Object.keys(history).sort(), ["deleted", "generated", "modified", "retained"]);
  const covered = new Set();
  const sourceKey = (source) => `${source.input}:${source.type}:${source.index}`;
  const assertSource = (source) => {
    assert.ok(Number.isInteger(source.input) && source.input >= 0 && source.input < inputCounts.length);
    assert.ok(source.type === "face" || source.type === "edge");
    assert.ok(Number.isInteger(source.index) && source.index >= 0 && source.index < inputCounts[source.input][source.type]);
    covered.add(sourceKey(source));
  };
  const assertTarget = (target) => {
    assert.ok(target.type === "face" || target.type === "edge");
    assert.ok(Number.isInteger(target.index) && target.index >= 0 && target.index < outputCounts[target.type]);
  };
  for (const entry of history.retained) {
    assertSource(entry.from);
    assertTarget(entry.to);
  }
  for (const category of [history.generated, history.modified]) {
    for (const entry of category) {
      assertSource(entry.from);
      assert.ok(entry.to.length > 0);
      entry.to.forEach(assertTarget);
    }
  }
  history.deleted.forEach(assertSource);
  inputCounts.forEach((counts, input) => {
    for (const type of ["face", "edge"]) {
      for (let index = 0; index < counts[type]; index++) {
        assert.ok(covered.has(`${input}:${type}:${index}`), `missing ${input}:${type}:${index}`);
      }
    }
  });
}

test("all full-history boolean operations cover every input face and edge", async () => {
  const client = await DirectClient.create(wasm);
  const scope = await client.beginScope();
  for (const operation of ["booleanCut", "booleanFuse", "booleanCommon"]) {
    const base = await scope.makeBox([4, 4, 4]);
    const tool = await scope.makeBox([4, 4, 4], [2, 0, 0]);
    const inputCounts = [await client.topologyCounts(base), await client.topologyCounts(tool)];
    const result = await scope[operation](base, [tool], { includeHistory: true });
    assert.ok(result.history);
    assertFullHistory(result.history, inputCounts, await client.topologyCounts(result.shape));
  }
  await scope.end();
});

test("fillet and chamfer histories expose OCCT replacements and cover every input face and edge", async () => {
  const client = await DirectClient.create(wasm);
  const capabilities = await client.initialize();
  assert.equal(capabilities.historySupport.fillet, "full");
  assert.equal(capabilities.historySupport.chamfer, "full");

  const scope = await client.beginScope();
  for (const operation of ["fillet", "chamfer"]) {
    const box = await scope.makeBox([10, 10, 10]);
    const inputCounts = await client.topologyCounts(box);
    const result = operation === "fillet"
      ? await scope.fillet(box, [0], 0.5, { includeHistory: true })
      : await scope.chamfer(box, [0], 0.5, { includeHistory: true });
    const outputCounts = await client.topologyCounts(result.shape);

    assert.ok(result.history);
    assertFullHistory(result.history, [inputCounts], outputCounts);
    assert.ok(result.history.generated.some((entry) =>
      entry.from.type === "edge" && entry.from.index === 0
      && entry.to.some((target) => target.type === "face")));
    assert.ok(result.history.modified.some((entry) =>
      entry.from.type === "face" && entry.to.some((target) => target.type === "face")));
    assert.ok(result.history.deleted.some((source) => source.type === "edge" && source.index === 0));
    assert.ok(result.history.generated.every((entry) => entry.from.type === "edge"));
    assert.ok(result.history.modified.every((entry) => entry.from.type === "face"));
  }
  await scope.end();
});

test("transform histories cover every input face and edge", async () => {
  const client = await DirectClient.create(wasm);
  const capabilities = await client.initialize();
  for (const operation of ["transform", "generalTransform", "translate", "rotate", "scale", "mirror"]) {
    assert.equal(capabilities.historySupport[operation], "full");
  }

  const scope = await client.beginScope();
  const calls = [
    (shape) => scope.transform(shape, { translation: [1, 2, 3], includeHistory: true }),
    (shape) => scope.generalTransform(shape, [1, 0.2, 0, 1, 0, 1, 0, 2, 0, 0, 1, 3], { includeHistory: true }),
    (shape) => scope.translate(shape, [1, 2, 3], { includeHistory: true }),
    (shape) => scope.rotate(shape, { direction: [0, 0, 1], angle: Math.PI / 4 }, { includeHistory: true }),
    (shape) => scope.scale(shape, 1.5, undefined, { includeHistory: true }),
    (shape) => scope.mirror(shape, [1, 0, 0], { includeHistory: true }),
  ];
  for (const call of calls) {
    const box = await scope.makeBox([2, 3, 4]);
    const inputCounts = await client.topologyCounts(box);
    const result = await call(box);
    assertFullHistory(result.history, [inputCounts], await client.topologyCounts(result.shape));
  }
  await scope.end();
});

test("sweep builders cover every input face and edge", async () => {
  const client = await DirectClient.create(wasm);
  const capabilities = await client.initialize();
  for (const operation of ["extrude", "revolve", "loft", "sweepPipe"]) {
    assert.equal(capabilities.historySupport[operation], "full");
  }
  assert.equal(capabilities.historySupport.sweepPipeShell, "partial");

  const scope = await client.beginScope();
  const square = await scope.makePolygon([[1, -1, 0], [3, -1, 0], [3, 1, 0], [1, 1, 0]]);
  const face = await scope.makeFace(square);
  const faceCounts = await client.topologyCounts(face);
  const extruded = await scope.extrude(face, [0, 0, 4], { includeHistory: true });
  assertFullHistory(
    extruded.history,
    [faceCounts],
    await client.topologyCounts(extruded.shape),
  );
  assert.ok(extruded.history.generated
    .filter(({ from }) => from.type === "edge")
    .every(({ to }) => to.every(({ type }) => type === "face")));
  const revolved = await scope.revolve(face, [0, 0, 0], [0, 1, 0], Math.PI, { includeHistory: true });
  assertFullHistory(
    revolved.history,
    [faceCounts],
    await client.topologyCounts(revolved.shape),
  );

  const lower = await scope.makePolygon([[0, 0, 0], [4, 0, 0], [4, 4, 0], [0, 4, 0]]);
  const upper = await scope.makePolygon([[1, 1, 4], [3, 1, 4], [3, 3, 4], [1, 3, 4]]);
  const lofted = await scope.loft([lower, upper], { solid: true, includeHistory: true });
  assertFullHistory(
    lofted.history,
    [await client.topologyCounts(lower), await client.topologyCounts(upper)],
    await client.topologyCounts(lofted.shape),
  );

  const spine = await scope.makePolygon([[0, 0, 0], [0, 0, 8]], false);
  const profile = await scope.makePolygon([[1, 0, 0], [0, 1, 0], [-1, 0, 0], [0, -1, 0]]);
  const pipe = await scope.sweepPipe(spine, profile, { includeHistory: true });
  const pipeInputCounts = [
    await client.topologyCounts(spine),
    await client.topologyCounts(profile),
  ];
  assertFullHistory(
    pipe.history,
    pipeInputCounts,
    await client.topologyCounts(pipe.shape),
  );
  assert.ok(pipe.history.generated
    .filter(({ from }) => from.type === "edge")
    .every(({ to }) => to.every(({ type }) => type === "face")));
  const pipeShell = await scope.sweepPipeShell(spine, [profile], {
    mode: "fixedAxis",
    axis: { direction: [0, 0, 1], xDirection: [1, 0, 0] },
    includeHistory: true,
  });
  assertFullHistory(
    pipeShell.history,
    pipeInputCounts,
    await client.topologyCounts(pipeShell.shape),
  );

  const auxiliarySpine = await scope.makePolygon([[2, 0, 0], [2, 0, 8]], false);
  const guidedPipeShell = await scope.sweepPipeShell(spine, [profile], {
    mode: "auxiliarySpine",
    auxiliarySpine,
    includeHistory: true,
  });
  assertFullHistory(
    guidedPipeShell.history,
    pipeInputCounts,
    await client.topologyCounts(guidedPipeShell.shape),
  );
  const historySources = [
    ...guidedPipeShell.history.retained.map(({ from }) => from),
    ...guidedPipeShell.history.generated.map(({ from }) => from),
    ...guidedPipeShell.history.modified.map(({ from }) => from),
    ...guidedPipeShell.history.deleted,
  ];
  assert.ok((await client.topologyCounts(auxiliarySpine)).edge > 0);
  assert.equal(historySources.some(({ input }) => input === 2), false);
  await scope.end();
});

test("healing histories cover every input face and edge", async () => {
  const client = await DirectClient.create(wasm);
  const capabilities = await client.initialize();
  for (const operation of ["fixShape", "sew", "unifySameDomain"]) {
    assert.equal(capabilities.historySupport[operation], "full");
  }

  const scope = await client.beginScope();
  const box = await scope.makeBox([2, 3, 4]);
  const fixed = await scope.fixShape(box, undefined, { includeHistory: true });
  assertFullHistory(
    fixed.history,
    [await client.topologyCounts(box)],
    await client.topologyCounts(fixed.shape),
  );

  const facePointSets = [
    [[0, 0, 0], [0, 1, 0], [1, 1, 0], [1, 0, 0]],
    [[0, 0, 1], [1, 0, 1], [1, 1, 1], [0, 1, 1]],
    [[0, 0, 0], [0, 0, 1], [0, 1, 1], [0, 1, 0]],
    [[1, 0, 0], [1, 1, 0], [1, 1, 1], [1, 0, 1]],
    [[0, 0, 0], [1, 0, 0], [1, 0, 1], [0, 0, 1]],
    [[0, 1, 0], [0, 1, 1], [1, 1, 1], [1, 1, 0]],
  ];
  const faces = [];
  for (const points of facePointSets) {
    faces.push(await scope.makeFace(await scope.makePolygon(points)));
  }
  const sewn = await scope.sew(faces, undefined, { includeHistory: true });
  assertFullHistory(
    sewn.history,
    await Promise.all(faces.map((face) => client.topologyCounts(face))),
    await client.topologyCounts(sewn.shape),
  );

  const left = await scope.makeBox([2, 2, 2]);
  const right = await scope.makeBox([2, 2, 2], [2, 0, 0]);
  const fused = await scope.booleanFuse(left, [right]);
  const before = await client.topologyCounts(fused.shape);
  const unified = await scope.unifySameDomain(fused.shape, { includeHistory: true });
  const after = await client.topologyCounts(unified.shape);
  assert.ok(after.face < before.face);
  assertFullHistory(unified.history, [before], after);

  await scope.end();
});
