import { loadArtifactBytes, RUNTIME_CONFIG } from "../../dist/index.js";

const result = document.querySelector("#result");
const descriptor = RUNTIME_CONFIG.profiles.preview.artifact;
const url = new URL("../../artifacts/preview.wasm", import.meta.url).toString();
const cacheName = "occt-worker-artifact-cache-test";
const nativeFetch = window.fetch.bind(window);
let fetchCount = 0;

window.fetch = (...args) => {
  fetchCount++;
  return nativeFetch(...args);
};

try {
  await caches.delete(cacheName);
  const first = await loadArtifactBytes(descriptor, url, { cacheName });
  const second = await loadArtifactBytes(descriptor, url, { cacheName });
  const cache = await caches.open(cacheName);
  const keys = await cache.keys();
  const keyUrl = new URL(keys[0].url);
  const hitWorked = fetchCount === 1 && first.byteLength === second.byteLength;
  const keyIsVersioned = keyUrl.searchParams.get("occt-worker-artifact") === descriptor.name
    && keyUrl.searchParams.get("occt-worker-sha256") === descriptor.sha256;

  await cache.put(keys[0], new Response(new Uint8Array([0])));
  const recovered = await loadArtifactBytes(descriptor, url, { cacheName });
  const invalidationWorked = fetchCount === 2 && recovered.byteLength === first.byteLength;
  result.dataset.state = hitWorked && keyIsVersioned && invalidationWorked ? "passed" : "failed";
  result.textContent = JSON.stringify({ fetchCount, hitWorked, keyIsVersioned, invalidationWorked });
} catch (error) {
  result.dataset.state = "failed";
  result.textContent = error instanceof Error ? `${error.name}: ${error.message}` : String(error);
} finally {
  window.fetch = nativeFetch;
  await caches.delete(cacheName);
}
