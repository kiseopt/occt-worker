// Stage-7 artifact resolver.
//
// Side/Profile WASM artifacts are not bundled with the npm package; they are
// fetched from a Release/CDN location determined by, in priority order:
//   1. an explicit user baseUrl / custom resolve function
//   2. the descriptor URL from protocol/artifacts.json
//   3. the project's versioned GitHub Release URL

import { BUILD_IDENTITY } from "./build-identity.generated.js";
export { BUILD_IDENTITY };

export interface ArtifactDescriptor {
  /** Stable file name, e.g. "mesh.side.wasm". */
  name: string;
  /** Release/CDN URL recorded in the artifact manifest. */
  url?: string;
  /** Content hash used for integrity verification when provided by manifest. */
  sha256?: string;
  protocolVersion?: string;
  abiVersion?: number;
  buildFamily?: string;
}

export type CustomArtifactResolver = (
  descriptor: ArtifactDescriptor,
) => string | URL | Promise<string | URL>;

export interface ResolveOptions {
  /** User-provided base URL for private deployments / mirrors. */
  baseUrl?: string | URL;
  /** Fully custom resolver overriding every default rule. */
  resolve?: CustomArtifactResolver;
  /**
   * Default base used when no user input is given. In browsers this should be
   * derived from import.meta.url of the host bundle; Node may pass a file URL.
   */
  defaultBase?: string | URL;
}

export const DEFAULT_ARTIFACT_BASE_URL =
  BUILD_IDENTITY.releaseBaseUrl;

function joinUrl(base: string | URL, name: string): string {
  const baseUrl = base instanceof URL ? base : new URL(base, import.meta.url);
  return new URL(name, baseUrl.toString().endsWith("/") ? baseUrl : new URL(`${baseUrl}/`)).toString();
}

/**
 * Resolves an artifact descriptor to a fetchable URL string.
 * Never performs the fetch itself; transport stays caller-owned.
 */
export async function resolveArtifact(
  descriptor: ArtifactDescriptor,
  options: ResolveOptions = {},
): Promise<string> {
  if (options.resolve) {
    const resolved = await options.resolve(descriptor);
    return resolved.toString();
  }
  if (options.baseUrl === undefined && options.defaultBase === undefined && descriptor.url !== undefined) {
    return new URL(descriptor.url, import.meta.url).toString();
  }
  const base = options.baseUrl ?? options.defaultBase ?? DEFAULT_ARTIFACT_BASE_URL;
  return joinUrl(base, descriptor.name);
}

export async function verifyArtifact(descriptor: ArtifactDescriptor, bytes: ArrayBuffer): Promise<void> {
  if (descriptor.sha256 === undefined) return;
  const digest = await crypto.subtle.digest("SHA-256", bytes);
  const actual = [...new Uint8Array(digest)].map((byte) => byte.toString(16).padStart(2, "0")).join("");
  if (actual !== descriptor.sha256.toLowerCase()) {
    throw new Error(`artifact '${descriptor.name}' SHA-256 mismatch`);
  }
}
