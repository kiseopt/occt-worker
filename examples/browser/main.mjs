import { WorkerClient } from "../../dist/worker-client.js";

const result = document.querySelector("#result");

try {
  const wasm = await (await fetch("../../wasm/occt-worker.wasm")).arrayBuffer();
  const factory = () => new Worker(new URL("../../dist/worker-entry.js", import.meta.url), { type: "module" });
  const kernel = await WorkerClient.create(factory, wasm);
  const scope = await kernel.beginScope();
  const plate = await scope.makeBox([100, 60, 4]);
  const hole = await scope.makeCylinder(8, 4, { origin: [50, 30, 0] });
  const cut = await scope.booleanCut(plate, [hole], { includeHistory: true });
  const bbox = await kernel.bbox(cut.shape);
  const mesh = await kernel.tessellate(cut.shape, { linearDeflection: 0.5 });
  const sharedBREP = await kernel.requestShared("exportBREP", { shape: cut.shape });
  const brep = sharedBREP.data.data;
  if (!(brep instanceof SharedArrayBuffer)) throw new Error("Shared output was not materialized into SharedArrayBuffer");
  const restored = await scope.importBREP(brep);
  const restoredBBox = await kernel.bbox(restored);
  await scope.end();
  const stats = await kernel.stats();
  kernel.close();
  result.dataset.state = "passed";
  result.textContent = JSON.stringify({
    bbox,
    restoredBBox,
    triangles: mesh.indices.length / 3,
    sharedTransport: brep instanceof SharedArrayBuffer,
    stats,
  }, null, 2);
} catch (error) {
  result.dataset.state = "failed";
  result.textContent = error instanceof Error ? `${error.name}: ${error.message}` : String(error);
}
