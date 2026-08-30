import { WorkerClient } from "./worker-client.js";
import type { ArtifactLoadAttemptTracker } from "./artifact-load-attempt.js";
import { assertOperationSet } from "./capability-assertion.js";
import type { ProfileRuntime, ProfileRuntimeFactory } from "./engine.js";
import type { OperationName } from "./generated.js";
import { PROFILE_OPERATIONS, type ProfileClientMap } from "./profile-clients.generated.js";
import type { RequestOptions } from "./types.js";
import { loadArtifactBytes, resolveArtifact, verifyArtifact, type ArtifactDescriptor, type ResolveOptions } from "./artifact-resolver.js";
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
  loadAttempt?: ArtifactLoadAttemptTracker;
  onLoadStage?(stage: "fetching" | "compiling" | "instantiating" | "ready"): void;
}

export type WorkerProfileOptions = WorkerProfileHostOptions & (
  | { profile: RuntimeProfileId; artifact?: never }
  | { profile?: never; artifact: ArtifactDescriptor }
);

async function createWorkerClient(options: WorkerProfileOptions): Promise<WorkerClient> {
  const artifact = options.profile === undefined
    ? options.artifact
    : RUNTIME_CONFIG.profiles[options.profile].artifact;
  options.loadAttempt?.begin(artifact.name, "fetching");
  options.onLoadStage?.("fetching");
  try {
    const url = await resolveArtifact(artifact, options);
    const bytes = options.loadArtifact === undefined
      ? await loadArtifactBytes(artifact, url)
      : await options.loadArtifact(url);
    if (options.loadArtifact !== undefined) await verifyArtifact(artifact, bytes);
    const client = await WorkerClient.create(options.createWorker, bytes, {
      onInitializationStage: (stage) => {
        if (stage === "ready") return;
        options.loadAttempt?.update(stage);
        options.onLoadStage?.(stage);
      },
    });
    if (options.profile !== undefined) {
      try {
        const capabilities = await client.initialize();
        assertOperationSet(
          `isolated profile '${options.profile}'`,
          PROFILE_OPERATIONS[options.profile],
          capabilities.ops,
        );
      } catch (error) {
        client.close();
        throw error;
      }
    }
    options.loadAttempt?.update("ready");
    options.onLoadStage?.("ready");
    options.loadAttempt?.complete({ status: "success" });
    return client;
  } catch (error) {
    options.loadAttempt?.fail(error);
    throw error;
  }
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
