import assert from "node:assert/strict";
import { join } from "node:path";
import { fileURLToPath, pathToFileURL } from "node:url";
import test from "node:test";
import { EngineCompatClient } from "../../dist/engine-compat.js";
import { GeometryEngine } from "../../dist/engine.js";
import { ParametricModel } from "../../dist/parametric.js";
import { SharedClient } from "../../dist/shared-client.js";
import { RUNTIME_CONFIG } from "../../dist/index.js";

const root = fileURLToPath(new URL("../..", import.meta.url));

test("shared Main/Side is usable through EngineCompatClient and ParametricModel", async () => {
  const shared = await SharedClient.create({
    baseUrl: pathToFileURL(join(root, "artifacts")),
    importFactory: async (url) => (await import(url)).default,
  });
  const engine = new GeometryEngine();
  engine.registerProfile("core-modeling", RUNTIME_CONFIG.shared.buildFamily, async () => ({
    request: (operation, args) => shared.request(operation, args),
    close: () => shared.close(),
  }));
  const compat = new EngineCompatClient(engine);
  const model = new ParametricModel(compat, {
    parameters: { depth: 5 },
    features: [
      {
        id: "profile",
        type: "sketch",
        entities: [
          { id: "a", type: "line", start: [0, 0], end: [10, 0] },
          { id: "b", type: "line", start: [10, 0], end: [10, 8] },
          { id: "c", type: "line", start: [10, 8], end: [0, 8] },
          { id: "d", type: "line", start: [0, 8], end: [0, 0] },
        ],
        constraints: [],
      },
      { id: "face", type: "face", outer: "profile" },
      { id: "solid", type: "extrude", input: "face", vector: [0, 0, "depth"] },
    ],
  });

  try {
    await model.recompute();
    assert.equal(await compat.shapeType(model.getShape("solid")), "solid");
    assert.ok(Math.abs((await compat.massProps(model.getShape("solid"))).mass - 400) < 1e-6);
  } finally {
    await model.dispose();
    await compat.close();
  }
});
