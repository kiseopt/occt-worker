import { WorkerClient } from "./worker-client.js";
import type { ProfileRuntime, ProfileRuntimeFactory } from "./engine.js";
import type { OperationName } from "./generated.js";
import type { ProfileClientMap } from "./profile-clients.generated.js";
import type { RequestOptions } from "./types.js";
import { resolveArtifact, verifyArtifact, type ArtifactDescriptor, type ResolveOptions } from "./artifact-resolver.js";
import { RUNTIME_CONFIG, type RuntimeProfileId } from "./runtime-manifest.generated.js";

interface WorkerLike {
  postMessage(message: unknown, transfer?: Transferable[]): void;
  terminate(): void;
  addEventListener(type: "message", listener: (event: MessageEvent) => void): void;
  addEventListener(type: "error", listener: (event: ErrorEvent) => void): void;
}

export interface WorkerProfileHostOptions extends ResolveOptions {
  createWorker(): WorkerLike;
  loadArtifact?(url: string): Promise<ArrayBuffer>;
}

export type WorkerProfileOptions = WorkerProfileHostOptions & (
  | { profile: RuntimeProfileId; artifact?: never }
  | { profile?: never; artifact: ArtifactDescriptor }
);

async function createWorkerClient(options: WorkerProfileOptions): Promise<WorkerClient> {
  const artifact = options.profile === undefined
    ? options.artifact
    : RUNTIME_CONFIG.profiles[options.profile].artifact;
  const url = await resolveArtifact(artifact, options);
  const bytes = options.loadArtifact === undefined
    ? await fetch(url).then((response) => {
        if (!response.ok) throw new Error(`failed to load '${artifact.name}': HTTP ${response.status}`);
        return response.arrayBuffer();
      })
    : await options.loadArtifact(url);
  await verifyArtifact(artifact, bytes);
  return WorkerClient.create(options.createWorker, bytes);
}

export function createWorkerProfileRuntime(options: WorkerProfileOptions): ProfileRuntimeFactory {
  return async (): Promise<ProfileRuntime> => {
    const client = await createWorkerClient(options);
    return {
      get epoch() { return client.epoch; },
      request: <T>(operation: OperationName, args: Record<string, unknown>, requestOptions?: RequestOptions) => (
        client.requestUnsafe<T>(operation, args, requestOptions)
      ),
      close: () => client.close(),
    };
  };
}

export async function createProfileClient<P extends keyof ProfileClientMap>(
  profile: P,
  options: WorkerProfileHostOptions,
): Promise<ProfileClientMap[P]> {
  const client = await createWorkerClient({ ...options, profile });
  return client as unknown as ProfileClientMap[P];
}
