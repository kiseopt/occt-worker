import assert from "node:assert/strict";
import { join } from "node:path";
import test from "node:test";
import { fileURLToPath } from "node:url";

const root = new URL("../..", import.meta.url);
const artifactPath = (name) => new URL(`../../artifacts/${name}`, import.meta.url);

test("shared Main loads all six Sides and runs a modeling smoke", async () => {
  const { default: createMain } = await import(artifactPath("shared-main.mjs"));
  const repoPath = fileURLToPath(root);
  const module = await createMain({ locateFile: (path) => join(repoPath, "artifacts", path) });
  const sides = [
    "geometry-topology.side.wasm",
    "modeling.side.wasm",
    "algorithms.side.wasm",
    "mesh.side.wasm",
    "exchange-mesh.side.wasm",
    "exchange-cad.side.wasm",
  ];
  for (const side of sides) {
    assert.equal(await module.loadDynamicLibrary(side, { loadAsync: true, global: true, nodelete: true }), true);
  }
  assert.equal(module.UTF8ToString(module._occt_host_build_family()), "occt-worker-shared-v1");
  assert.equal(module._occt_host_plugin_count(), 6);
  assert.equal(module._occt_host_plugin_error(), 0);
  assert.equal(module.UTF8ToString(module._occt_host_plugin_name()), "exchange-cad");

  let nextId = 1;
  const call = (op, args = {}) => {
    const request = new TextEncoder().encode(JSON.stringify({ id: nextId++, op, args }));
    const pointer = module._k_alloc(request.byteLength);
    module.HEAPU8.set(request, pointer);
    const length = module._k_handle(pointer, request.byteLength);
    module._k_free(pointer);
    const responsePointer = module._k_response_ptr();
    const response = JSON.parse(new TextDecoder().decode(module.HEAPU8.slice(responsePointer, responsePointer + length)));
    if (!response.ok) throw new Error(response.error?.message ?? "shared runtime request failed");
    return response.result;
  };

  const scopeId = call("beginScope").scopeId;
  const box = call("makeBox", { scopeId, size: [2, 3, 4] });
  const mesh = call("tessellate", { scopeId, shape: box.shape, deflection: 0.1 });
  assert.equal(box.shape > 0, true);
  assert.equal(mesh.indices.byteLength % 12, 0);
  assert.ok(mesh.positions.byteLength > 0);
  for (const value of Object.values(mesh)) {
    if (value !== null && typeof value === "object" && "bufferId" in value) {
      assert.ok(module._k_buffer_ptr(value.bufferId) > 0);
      call("freeBuffer", { bufferId: value.bufferId });
    }
  }
  call("endScope", { scopeId });
});
