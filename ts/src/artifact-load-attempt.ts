export interface AttemptStorage {
  getItem(key: string): string | null;
  setItem(key: string, value: string): void;
  removeItem(key: string): void;
}

export interface ArtifactLoadAttempt {
  subject: string;
  stage: string;
  [key: string]: unknown;
}

export interface ArtifactLoadAttemptTrackerOptions {
  attemptKey?: string;
  resultKey?: string;
}

const DEFAULT_ATTEMPT_KEY = "occt-worker:attempting-full";
const DEFAULT_RESULT_KEY = "occt-worker:full-load-result";

/** A leftover attempt proves only that the previous load did not complete, not why it stopped. */
export class ArtifactLoadAttemptTracker {
  readonly #storage: AttemptStorage;
  readonly #attemptKey: string;
  readonly #resultKey: string;
  #current: ArtifactLoadAttempt | undefined;

  constructor(storage: AttemptStorage, options: ArtifactLoadAttemptTrackerOptions = {}) {
    this.#storage = storage;
    this.#attemptKey = options.attemptKey ?? DEFAULT_ATTEMPT_KEY;
    this.#resultKey = options.resultKey ?? DEFAULT_RESULT_KEY;
    this.#current = this.unfinished();
  }

  unfinished(): ArtifactLoadAttempt | undefined {
    const value = this.#storage.getItem(this.#attemptKey);
    return value === null ? undefined : JSON.parse(value) as ArtifactLoadAttempt;
  }

  lastResult<T extends Record<string, unknown> = Record<string, unknown>>(): T | undefined {
    const value = this.#storage.getItem(this.#resultKey);
    return value === null ? undefined : JSON.parse(value) as T;
  }

  begin(subject: string, stage: string, details: Record<string, unknown> = {}): void {
    this.#current = { subject, stage, ...details };
    this.#storage.removeItem(this.#resultKey);
    this.#storage.setItem(this.#attemptKey, JSON.stringify(this.#current));
  }

  update(stage: string): void {
    if (this.#current === undefined) throw new Error("artifact load attempt has not started");
    this.#current = { ...this.#current, stage };
    this.#storage.setItem(this.#attemptKey, JSON.stringify(this.#current));
  }

  complete(result: Record<string, unknown>): void {
    if (this.#current === undefined) throw new Error("artifact load attempt has not started");
    this.#storage.setItem(this.#resultKey, JSON.stringify({
      subject: this.#current.subject,
      stage: this.#current.stage,
      ...result,
    }));
    this.#storage.removeItem(this.#attemptKey);
    this.#current = undefined;
  }

  fail(reason: unknown): void {
    this.complete({
      status: "failed",
      reason: reason instanceof Error ? `${reason.name}: ${reason.message}` : String(reason),
    });
  }

  clear(): void {
    this.#current = undefined;
    this.#storage.removeItem(this.#attemptKey);
    this.#storage.removeItem(this.#resultKey);
  }
}
