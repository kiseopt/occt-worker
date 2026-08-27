import type { Matrix12, RGBA, Vec3 } from "./types-base.js";

export interface XCAFMaterial {
  name: string;
  description?: string;
  density: number;
  densityName?: string;
  densityValueType?: string;
}

export interface XCAFVisualMaterial {
  name: string;
  baseColor?: RGBA;
  metallic?: number;
  roughness?: number;
  emissive?: Vec3;
  refractionIndex?: number;
}

export interface XCAFValidationProperties {
  area?: number;
  volume?: number;
  centroid?: Vec3;
}

export interface XCAFSubshapeStyle {
  topology: "face" | "edge";
  index: number;
  color: RGBA;
}

export type XCAFViewProjection = "none" | "parallel" | "central";

export interface XCAFClippingPlaneDefinition {
  name: string;
  origin: Vec3;
  normal: Vec3;
  capping?: boolean;
}

export interface XCAFViewDefinition {
  name: string;
  projection?: XCAFViewProjection;
  projectionPoint?: Vec3;
  viewDirection: Vec3;
  upDirection: Vec3;
  zoomFactor?: number;
  windowHorizontalSize?: number;
  windowVerticalSize?: number;
  frontPlaneDistance?: number;
  backPlaneDistance?: number;
  viewVolumeSidesClipping?: boolean;
  nodeIndices: readonly number[];
  clippingPlanes?: readonly XCAFClippingPlaneDefinition[];
}

export interface XCAFShuoDefinition {
  /** Component node indices from an upper usage to a next usage. */
  nodeIndices: readonly number[];
  /** Optional per-occurrence RGBA style stored on the SHUO graph node. */
  color?: RGBA;
}

export interface XCAFAnnotationPresentation {
  /** Scoped shape containing the annotation's graphical curves and symbols. */
  shape: import("./client.js").ShapeHandle;
  name?: string;
}

export interface XCAFGDTDimension {
  node: number;
  type:
    | "curveLength"
    | "diameter"
    | "sphericalDiameter"
    | "radius"
    | "sphericalRadius"
    | "toroidalMinorDiameter"
    | "toroidalMajorDiameter"
    | "toroidalMinorRadius"
    | "toroidalMajorRadius"
    | "toroidalHighMajorDiameter"
    | "toroidalLowMajorDiameter"
    | "toroidalHighMajorRadius"
    | "toroidalLowMajorRadius"
    | "thickness"
    | "angular";
  value: number;
  semanticName?: string;
  presentation?: XCAFAnnotationPresentation;
}

export interface XCAFGDTDatum {
  node: number;
  name: string;
  description?: string;
  identification: string;
  semanticName?: string;
  presentation?: XCAFAnnotationPresentation;
}

export interface XCAFGeomTolerance {
  node: number;
  type:
    | "angularity"
    | "circularRunout"
    | "circularity"
    | "coaxiality"
    | "concentricity"
    | "cylindricity"
    | "flatness"
    | "parallelism"
    | "perpendicularity"
    | "position"
    | "profileOfLine"
    | "profileOfSurface"
    | "straightness"
    | "symmetry"
    | "totalRunout";
  value: number;
  valueType?: "none" | "diameter" | "sphericalDiameter";
  semanticName?: string;
  datumIndices?: readonly number[];
  materialRequirement?: "none" | "maximum" | "least";
  zoneModifier?: "none" | "projected" | "runout" | "nonUniform";
  zoneModifierValue?: number;
  modifiers?: readonly (
    | "anyCrossSection"
    | "commonZone"
    | "eachRadialElement"
    | "freeState"
    | "leastMaterialRequirement"
    | "lineElement"
    | "majorDiameter"
    | "maximumMaterialRequirement"
    | "minorDiameter"
    | "notConvex"
    | "pitchDiameter"
    | "reciprocityRequirement"
    | "separateRequirement"
    | "statisticalTolerance"
    | "tangentPlane"
    | "allAround"
    | "allOver"
  )[];
  maxValueModifier?: number;
  presentation?: XCAFAnnotationPresentation;
}

export interface STEPDocumentNodeDefinition {
  kind: "part" | "assembly";
  shape?: import("./client.js").ShapeHandle;
  name?: string;
  children?: readonly number[];
  transform?: Matrix12;
  color?: RGBA;
  layers?: readonly string[];
  visible?: boolean;
  material?: XCAFMaterial;
  visualMaterial?: XCAFVisualMaterial;
  validationProperties?: XCAFValidationProperties;
  subshapeStyles?: readonly XCAFSubshapeStyle[];
}

export interface STEPDocumentDefinition {
  nodes: readonly STEPDocumentNodeDefinition[];
  roots: readonly number[];
  gdt?: readonly XCAFGDTDimension[];
  datums?: readonly XCAFGDTDatum[];
  geometricTolerances?: readonly XCAFGeomTolerance[];
  views?: readonly XCAFViewDefinition[];
  shuo?: readonly XCAFShuoDefinition[];
}

export interface STEPDocumentNode {
  kind: "part" | "assembly";
  shape: import("./client.js").ShapeHandle;
  name?: string;
  children: number[];
  transform: Matrix12;
  color?: RGBA;
  layers: string[];
  visible: boolean;
  material?: XCAFMaterial;
  visualMaterial?: XCAFVisualMaterial;
  validationProperties?: XCAFValidationProperties;
  subshapeStyles: XCAFSubshapeStyle[];
}

export interface STEPDocument {
  shape: import("./client.js").ShapeHandle;
  rootCount: number;
  roots: number[];
  nodes: STEPDocumentNode[];
  gdt: XCAFGDTDimension[];
  datums: XCAFGDTDatum[];
  geometricTolerances: XCAFGeomTolerance[];
  views: XCAFViewDefinition[];
  shuo: XCAFShuoDefinition[];
}
