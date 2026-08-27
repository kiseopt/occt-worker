import type { ArtifactDescriptor } from "./artifact-resolver.js";
import type { OperationName } from "./generated.js";

export interface ModulesManifest {
  readonly semanticModules: Readonly<Record<string, readonly OperationName[]>>;
  readonly artifactModuleCandidates: Readonly<Record<string, readonly string[]>>;
  readonly profiles: Readonly<Record<string, {
    readonly artifact: string;
    readonly artifacts: readonly string[];
  }>>;
  readonly transferOperations?: readonly OperationName[];
}

export interface RuntimeArtifactDescriptor extends ArtifactDescriptor {
  readonly url: string;
  readonly sha256: string;
  readonly sizeBytes: number;
  readonly buildFamily: string;
  readonly kind: "shared-glue" | "shared-main" | "shared-side" | "isolated-profile";
  readonly semanticModule?: string;
  readonly profile?: string;
}

export interface SharedRuntimeConfig {
  readonly buildFamily: string;
  readonly mainJs: RuntimeArtifactDescriptor;
  readonly mainWasm: RuntimeArtifactDescriptor;
  readonly sides: readonly RuntimeArtifactDescriptor[];
  readonly operationSides: Readonly<Partial<Record<OperationName, string>>>;
}

export interface IsolatedProfileConfig {
  readonly profileId: string;
  readonly buildFamily: string;
  readonly artifact: RuntimeArtifactDescriptor;
}

export interface RuntimeConfig {
  readonly modules: ModulesManifest;
  readonly shared: SharedRuntimeConfig;
  readonly profiles: Readonly<Record<string, IsolatedProfileConfig>>;
}
