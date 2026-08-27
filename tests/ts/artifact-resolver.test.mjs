// Stage-7 resolver tests: priority rules, URL joining, custom resolvers.

import assert from "node:assert/strict";
import test from "node:test";
import { pathToFileURL } from "node:url";
import {
  DEFAULT_ARTIFACT_BASE_URL,
  resolveArtifact,
  verifyArtifact,
} from "../../dist/artifact-resolver.js";

test("resolver honors custom resolver over baseUrl over defaultBase", async () => {
  const descriptor = { name: "mesh.side.wasm" };

  const implicit = await resolveArtifact(descriptor);
  assert.equal(implicit, `${DEFAULT_ARTIFACT_BASE_URL}mesh.side.wasm`);

  const byManifest = await resolveArtifact({
    name: "mesh.side.wasm",
    url: "https://cdn.example.com/occt/v1/mesh.side.wasm",
  });
  assert.equal(byManifest, "https://cdn.example.com/occt/v1/mesh.side.wasm");

  const byDefault = await resolveArtifact(descriptor, {
    defaultBase: "https://release.example.com/occt-worker/v1/",
  });
  assert.equal(byDefault, "https://release.example.com/occt-worker/v1/mesh.side.wasm");

  const byUserBase = await resolveArtifact(descriptor, {
    defaultBase: "https://default.invalid/",
    baseUrl: "https://private.corp/mirrors/occt",
  });
  assert.equal(byUserBase, "https://private.corp/mirrors/occt/mesh.side.wasm");

  const byCustom = await resolveArtifact(descriptor, {
    baseUrl: "https://ignored.invalid/",
    resolve: (d) => `file:///artifacts/${d.name}?v=2`,
  });
  assert.equal(byCustom, "file:///artifacts/mesh.side.wasm?v=2");

  // Node file URLs are supported for local/private deployments.
  const fileUrl = await resolveArtifact(descriptor, {
    defaultBase: pathToFileURL("/tmp/artifacts/").href,
  });
  assert.ok(fileUrl.endsWith("/artifacts/mesh.side.wasm"));
});

test("artifact integrity accepts the declared SHA-256 and rejects mismatches", async () => {
  const bytes = new TextEncoder().encode("abc").buffer;
  await verifyArtifact({
    name: "profile.wasm",
    sha256: "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
  }, bytes);
  await assert.rejects(
    verifyArtifact({ name: "profile.wasm", sha256: "00".repeat(32) }, bytes),
    /SHA-256 mismatch/,
  );
});
