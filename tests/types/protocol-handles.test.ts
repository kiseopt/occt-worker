import type {
  ClientRequestOwner,
  ProtocolRequestArgs,
  ProtocolResult,
  ScopeHandle,
  ShapeHandle,
} from "../../ts/src/client-contract.js";
import { RUNTIME_CONFIG } from "../../ts/src/index.js";

type Equal<Left, Right> =
  (<Value>() => Value extends Left ? 1 : 2) extends
  (<Value>() => Value extends Right ? 1 : 2) ? true : false;
type Assert<Value extends true> = Value;

type BeginScopeIsRawNumber = Assert<Equal<ProtocolResult<"beginScope">, { scopeId: number }>>;
type ShapeInputIsNominal = Assert<Equal<ProtocolRequestArgs<"bbox">["shape"], ShapeHandle>>;
type ScopeInputIsNominal = Assert<Equal<ProtocolRequestArgs<"endScope">["scopeId"], ScopeHandle>>;
type BufferInputIsNumeric = Assert<Equal<ProtocolRequestArgs<"freeBuffer">["bufferId"], number>>;
type ModelingSideIsCanonical = Assert<Equal<
  typeof RUNTIME_CONFIG.shared.operationSides.makeBox,
  "modeling.side.wasm"
>>;
type CoreProfileArtifactIsCanonical = Assert<Equal<
  typeof RUNTIME_CONFIG.profiles["core-modeling"]["artifact"]["name"],
  "core-modeling.wasm"
>>;

declare const client: ClientRequestOwner;
declare const shape: ShapeHandle;
declare const scope: ScopeHandle;

void client.request("bbox", { shape });
void client.request("endScope", { scopeId: scope });
void client.request("freeBuffer", { bufferId: 1 });
void client.request("exportSTEP", { shape });
void client.request("exportGLTF", { shape });
void client.request("batchTransformCopy", {
  scopeId: scope,
  shape,
  mode: "linear",
  count: 2,
  translation: [1, 0, 0],
});
void client.request("getAdjacency", { shape, from: "face", to: "edge" });

// @ts-expect-error Scope handles cannot be used as shape handles.
void client.request("bbox", { shape: scope });
// @ts-expect-error Shape handles cannot be used as scope handles.
void client.request("endScope", { scopeId: shape });
// @ts-expect-error Buffer ids cannot be shape handles.
void client.request("freeBuffer", { bufferId: shape });
// @ts-expect-error STEP export requires either shape or shapes.
void client.request("exportSTEP", {});
// @ts-expect-error STEP export accepts one shape source form, not both.
void client.request("exportSTEP", { shape, shapes: [shape] });
// @ts-expect-error glTF export accepts a shape or a document with buffers, not both.
void client.request("exportGLTF", { shape, document: {}, buffers: [] });
// @ts-expect-error Linear batch copies require a translation.
void client.request("batchTransformCopy", { scopeId: scope, shape, mode: "linear", count: 2 });
// @ts-expect-error Face adjacency can only target edges.
void client.request("getAdjacency", { shape, from: "face", to: "vertex" });

export type ProtocolHandleAssertions =
  | BeginScopeIsRawNumber
  | ShapeInputIsNominal
  | ScopeInputIsNominal
  | BufferInputIsNumeric
  | ModelingSideIsCanonical
  | CoreProfileArtifactIsCanonical;
