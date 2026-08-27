// Generated from protocol/errors.json, protocol/operations.json, and protocol/modules.json.
#pragma once

#include <array>
#include <string_view>

namespace occt_worker {
inline constexpr std::string_view kProtocolVersion = "1.2.0";
enum class ErrorCode {
  ConstructionFailed,
  BooleanFailed,
  FilletFailed,
  TessellationFailed,
  ImportExportFailed,
  HealingFailed,
  Cancelled,
  InvalidHandle,
  InvalidArgs,
  OutOfMemory,
  KernelError,
  ProtocolError,
};
inline constexpr std::array<const char*, 161> kOperationNames = {
  "approximateCurveBSpline",
  "approximateSurfaceBSpline",
  "batch",
  "bbox",
  "beginScope",
  "booleanCommon",
  "booleanCut",
  "booleanFuse",
  "classifyPoint",
  "capabilities",
  "convertCurveToBSpline",
  "convertSurfaceToBSpline",
  "createBuffer",
  "curveContinuity",
  "curveControlData",
  "curveDomain",
  "curveGeometry",
  "cylindricalHole",
  "editCurveBSpline",
  "editSurfaceBSpline",
  "endScope",
  "evaluateCurve",
  "evaluateSurface",
  "exportBREP",
  "exportGLTF",
  "exportIGES",
  "exportIGESDocument",
  "exportOBJ",
  "exportPLY",
  "exportSTEP",
  "exportSTEPDocument",
  "exportVRML",
  "exportXCAF",
  "extendCurve",
  "extendSurface",
  "freeBuffer",
  "getSubShape",
  "getSubShapes",
  "distance",
  "diagnoseShape",
  "defeature",
  "draftAngle",
  "extremaCurveCurve",
  "extremaCurveSurface",
  "extremaSurfaceSurface",
  "extrude",
  "fillet",
  "fixShape",
  "generalFuse",
  "generalTransform",
  "getAdjacency",
  "glue",
  "hollow",
  "importBREP",
  "importGLTF",
  "importIGES",
  "importIGESDocument",
  "importOBJ",
  "importPLY",
  "importSTEP",
  "importSTEPDocument",
  "importSTL",
  "importVRML",
  "importXCAF",
  "inspectTolerances",
  "intersectCurveCurve",
  "intersectCurveSurface",
  "intersectSurfaceSurface",
  "isSameShape",
  "isValid",
  "makeBox",
  "makeCompSolid",
  "makeCompound",
  "makeCone",
  "makeCylinder",
  "makeEdgeArc",
  "makeEdgeBezier",
  "makeEdgeBSpline",
  "makeEdgeCircle",
  "makeEdgeEllipse",
  "makeEdgeHelix",
  "makeEdgeHyperbola",
  "makeEdgeLine",
  "makeEdgeOffset",
  "makeEdgeParabola",
  "makeFace",
  "makeFaceOnSurface",
  "makeHalfSpace",
  "makePolygon",
  "makeSurfaceBezier",
  "makeSurfaceBSpline",
  "makeSurfaceExtrusion",
  "makeSurfaceFace",
  "makeSurfaceFilling",
  "makeSurfaceOffset",
  "makeSurfaceRevolution",
  "makeSurfaceRuled",
  "makeShell",
  "makeSolidFromShell",
  "makeSphere",
  "makeTorus",
  "makeVertex",
  "makeWedge",
  "makeWire",
  "massProps",
  "middlePath",
  "loft",
  "localPrism",
  "localRevolution",
  "linearForm",
  "revolutionForm",
  "obb",
  "offsetShape",
  "offsetWire2D",
  "probeFormat",
  "projectPointCurve",
  "projectPointSurface",
  "release",
  "releaseAll",
  "reparameterizeCurve",
  "reparameterizeSurface",
  "reduceCurveDegree",
  "reduceSurfaceDegree",
  "revolve",
  "section",
  "sectionAnalysis",
  "projectHLR",
  "selectGeneralFuseCells",
  "setTolerance",
  "sew",
  "shapeType",
  "split",
  "stats",
  "sweepPipe",
  "surfaceContinuity",
  "surfaceControlData",
  "surfaceDomain",
  "surfaceGeometry",
  "surfaceIsoCurve",
  "sweepPipeShell",
  "tessellate",
  "tessellateEdges",
  "topologyCounts",
  "triangulationData",
  "replaceTriangulation",
  "validateTriangulation",
  "repairTriangulation",
  "transform",
  "trimCurve",
  "trimSurface",
  "unifySameDomain",
  "updateCurvePole",
  "updateSurfacePole",
  "batchTransformCopy",
  "exportSTL",
  "chamfer",
  "shapeUpgrade",
  "translate",
  "rotate",
  "scale",
  "mirror",
};
#define OCCT_RUNTIME_OPERATION_TABLE(X) \
  X(capabilities) \
  X(beginScope) \
  X(endScope) \
  X(release) \
  X(releaseAll) \
  X(createBuffer) \
  X(freeBuffer) \
  X(stats) \
  X(batch) \
  X(importBREP) \
  X(exportBREP)

#define OCCT_OPERATION_NAME(theOperation) #theOperation,
inline constexpr std::array<const char*, 11> kRuntimeOperationNames = {
  OCCT_RUNTIME_OPERATION_TABLE(OCCT_OPERATION_NAME)
};
#undef OCCT_OPERATION_NAME
#define OCCT_GEOMETRY_TOPOLOGY_OPERATION_TABLE(X) \
  X(approximateCurveBSpline) \
  X(approximateSurfaceBSpline) \
  X(convertCurveToBSpline) \
  X(convertSurfaceToBSpline) \
  X(curveContinuity) \
  X(curveControlData) \
  X(curveDomain) \
  X(curveGeometry) \
  X(editCurveBSpline) \
  X(editSurfaceBSpline) \
  X(evaluateCurve) \
  X(evaluateSurface) \
  X(extendCurve) \
  X(extendSurface) \
  X(reparameterizeCurve) \
  X(reparameterizeSurface) \
  X(reduceCurveDegree) \
  X(reduceSurfaceDegree) \
  X(trimCurve) \
  X(trimSurface) \
  X(updateCurvePole) \
  X(updateSurfacePole) \
  X(surfaceContinuity) \
  X(surfaceControlData) \
  X(surfaceDomain) \
  X(surfaceGeometry) \
  X(surfaceIsoCurve) \
  X(makeEdgeArc) \
  X(makeEdgeBezier) \
  X(makeEdgeBSpline) \
  X(makeEdgeCircle) \
  X(makeEdgeEllipse) \
  X(makeEdgeHelix) \
  X(makeEdgeHyperbola) \
  X(makeEdgeLine) \
  X(makeEdgeOffset) \
  X(makeEdgeParabola) \
  X(makeVertex) \
  X(makeWire) \
  X(makePolygon) \
  X(makeFace) \
  X(makeFaceOnSurface) \
  X(makeSurfaceBezier) \
  X(makeSurfaceBSpline) \
  X(makeSurfaceExtrusion) \
  X(makeSurfaceFace) \
  X(makeSurfaceFilling) \
  X(makeSurfaceOffset) \
  X(makeSurfaceRevolution) \
  X(makeSurfaceRuled) \
  X(getSubShape) \
  X(getSubShapes) \
  X(shapeType) \
  X(topologyCounts) \
  X(getAdjacency) \
  X(isSameShape) \
  X(inspectTolerances) \
  X(setTolerance) \
  X(isValid) \
  X(diagnoseShape) \
  X(bbox) \
  X(obb)

#define OCCT_OPERATION_NAME(theOperation) #theOperation,
inline constexpr std::array<const char*, 62> kGeometryTopologyOperationNames = {
  OCCT_GEOMETRY_TOPOLOGY_OPERATION_TABLE(OCCT_OPERATION_NAME)
};
#undef OCCT_OPERATION_NAME
#define OCCT_MODELING_OPERATION_TABLE(X) \
  X(makeBox) \
  X(makeCone) \
  X(makeCylinder) \
  X(makeSphere) \
  X(makeTorus) \
  X(makeWedge) \
  X(makeShell) \
  X(makeSolidFromShell) \
  X(makeCompSolid) \
  X(makeCompound) \
  X(makeHalfSpace) \
  X(extrude) \
  X(revolve) \
  X(loft) \
  X(sweepPipe) \
  X(fillet) \
  X(chamfer) \
  X(hollow) \
  X(draftAngle) \
  X(defeature) \
  X(cylindricalHole) \
  X(linearForm) \
  X(revolutionForm) \
  X(localPrism) \
  X(localRevolution) \
  X(transform) \
  X(translate) \
  X(rotate) \
  X(scale) \
  X(mirror) \
  X(batchTransformCopy) \
  X(generalTransform)

#define OCCT_OPERATION_NAME(theOperation) #theOperation,
inline constexpr std::array<const char*, 32> kModelingOperationNames = {
  OCCT_MODELING_OPERATION_TABLE(OCCT_OPERATION_NAME)
};
#undef OCCT_OPERATION_NAME
#define OCCT_ALGORITHMS_OPERATION_TABLE(X) \
  X(booleanCommon) \
  X(booleanCut) \
  X(booleanFuse) \
  X(section) \
  X(split) \
  X(generalFuse) \
  X(selectGeneralFuseCells) \
  X(glue) \
  X(sew) \
  X(distance) \
  X(extremaCurveCurve) \
  X(extremaCurveSurface) \
  X(extremaSurfaceSurface) \
  X(intersectCurveCurve) \
  X(intersectCurveSurface) \
  X(intersectSurfaceSurface) \
  X(projectPointCurve) \
  X(projectPointSurface) \
  X(projectHLR) \
  X(classifyPoint) \
  X(sectionAnalysis) \
  X(massProps) \
  X(middlePath) \
  X(offsetShape) \
  X(offsetWire2D) \
  X(fixShape) \
  X(unifySameDomain) \
  X(shapeUpgrade) \
  X(sweepPipeShell)

#define OCCT_OPERATION_NAME(theOperation) #theOperation,
inline constexpr std::array<const char*, 29> kAlgorithmsOperationNames = {
  OCCT_ALGORITHMS_OPERATION_TABLE(OCCT_OPERATION_NAME)
};
#undef OCCT_OPERATION_NAME
#define OCCT_MESH_OPERATION_TABLE(X) \
  X(tessellate) \
  X(tessellateEdges) \
  X(triangulationData) \
  X(replaceTriangulation) \
  X(validateTriangulation) \
  X(repairTriangulation)

#define OCCT_OPERATION_NAME(theOperation) #theOperation,
inline constexpr std::array<const char*, 6> kMeshOperationNames = {
  OCCT_MESH_OPERATION_TABLE(OCCT_OPERATION_NAME)
};
#undef OCCT_OPERATION_NAME
#define OCCT_EXCHANGE_MESH_OPERATION_TABLE(X) \
  X(importSTL) \
  X(exportSTL) \
  X(importPLY) \
  X(exportPLY) \
  X(importOBJ) \
  X(exportOBJ) \
  X(importGLTF) \
  X(exportGLTF) \
  X(importVRML) \
  X(exportVRML)

#define OCCT_OPERATION_NAME(theOperation) #theOperation,
inline constexpr std::array<const char*, 10> kExchangeMeshOperationNames = {
  OCCT_EXCHANGE_MESH_OPERATION_TABLE(OCCT_OPERATION_NAME)
};
#undef OCCT_OPERATION_NAME
#define OCCT_EXCHANGE_CAD_OPERATION_TABLE(X) \
  X(importSTEP) \
  X(exportSTEP) \
  X(importSTEPDocument) \
  X(exportSTEPDocument) \
  X(importIGES) \
  X(exportIGES) \
  X(importIGESDocument) \
  X(exportIGESDocument) \
  X(importXCAF) \
  X(exportXCAF) \
  X(probeFormat)

#define OCCT_OPERATION_NAME(theOperation) #theOperation,
inline constexpr std::array<const char*, 11> kExchangeCadOperationNames = {
  OCCT_EXCHANGE_CAD_OPERATION_TABLE(OCCT_OPERATION_NAME)
};
#undef OCCT_OPERATION_NAME
struct OperationFailurePolicy { const char* operation; ErrorCode code; };
inline constexpr std::array<OperationFailurePolicy, 107> kOperationFailurePolicies = {{
  {"booleanCut", ErrorCode::BooleanFailed},
  {"booleanFuse", ErrorCode::BooleanFailed},
  {"booleanCommon", ErrorCode::BooleanFailed},
  {"generalFuse", ErrorCode::BooleanFailed},
  {"selectGeneralFuseCells", ErrorCode::BooleanFailed},
  {"section", ErrorCode::BooleanFailed},
  {"split", ErrorCode::BooleanFailed},
  {"defeature", ErrorCode::BooleanFailed},
  {"fillet", ErrorCode::FilletFailed},
  {"chamfer", ErrorCode::FilletFailed},
  {"tessellate", ErrorCode::TessellationFailed},
  {"tessellateEdges", ErrorCode::TessellationFailed},
  {"triangulationData", ErrorCode::TessellationFailed},
  {"validateTriangulation", ErrorCode::TessellationFailed},
  {"repairTriangulation", ErrorCode::HealingFailed},
  {"hollow", ErrorCode::HealingFailed},
  {"offsetShape", ErrorCode::HealingFailed},
  {"sew", ErrorCode::HealingFailed},
  {"fixShape", ErrorCode::HealingFailed},
  {"unifySameDomain", ErrorCode::HealingFailed},
  {"shapeUpgrade", ErrorCode::HealingFailed},
  {"exportIGES", ErrorCode::ImportExportFailed},
  {"importIGES", ErrorCode::ImportExportFailed},
  {"exportSTEP", ErrorCode::ImportExportFailed},
  {"importSTEP", ErrorCode::ImportExportFailed},
  {"exportSTEPDocument", ErrorCode::ImportExportFailed},
  {"importSTEPDocument", ErrorCode::ImportExportFailed},
  {"exportIGESDocument", ErrorCode::ImportExportFailed},
  {"importIGESDocument", ErrorCode::ImportExportFailed},
  {"exportXCAF", ErrorCode::ImportExportFailed},
  {"importXCAF", ErrorCode::ImportExportFailed},
  {"exportBREP", ErrorCode::ImportExportFailed},
  {"importBREP", ErrorCode::ImportExportFailed},
  {"exportSTL", ErrorCode::ImportExportFailed},
  {"importSTL", ErrorCode::ImportExportFailed},
  {"exportGLTF", ErrorCode::ImportExportFailed},
  {"importGLTF", ErrorCode::ImportExportFailed},
  {"exportVRML", ErrorCode::ImportExportFailed},
  {"importVRML", ErrorCode::ImportExportFailed},
  {"exportOBJ", ErrorCode::ImportExportFailed},
  {"importOBJ", ErrorCode::ImportExportFailed},
  {"exportPLY", ErrorCode::ImportExportFailed},
  {"importPLY", ErrorCode::ImportExportFailed},
  {"replaceTriangulation", ErrorCode::ConstructionFailed},
  {"approximateCurveBSpline", ErrorCode::ConstructionFailed},
  {"approximateSurfaceBSpline", ErrorCode::ConstructionFailed},
  {"reduceCurveDegree", ErrorCode::ConstructionFailed},
  {"reduceSurfaceDegree", ErrorCode::ConstructionFailed},
  {"extendCurve", ErrorCode::ConstructionFailed},
  {"extendSurface", ErrorCode::ConstructionFailed},
  {"makeBox", ErrorCode::ConstructionFailed},
  {"makeCylinder", ErrorCode::ConstructionFailed},
  {"makeSphere", ErrorCode::ConstructionFailed},
  {"makeCone", ErrorCode::ConstructionFailed},
  {"makeTorus", ErrorCode::ConstructionFailed},
  {"makeWedge", ErrorCode::ConstructionFailed},
  {"makeHalfSpace", ErrorCode::ConstructionFailed},
  {"makeVertex", ErrorCode::ConstructionFailed},
  {"makePolygon", ErrorCode::ConstructionFailed},
  {"makeWire", ErrorCode::ConstructionFailed},
  {"makeFace", ErrorCode::ConstructionFailed},
  {"makeCompound", ErrorCode::ConstructionFailed},
  {"makeShell", ErrorCode::ConstructionFailed},
  {"makeCompSolid", ErrorCode::ConstructionFailed},
  {"makeSurfaceFace", ErrorCode::ConstructionFailed},
  {"makeSurfaceBezier", ErrorCode::ConstructionFailed},
  {"makeSurfaceBSpline", ErrorCode::ConstructionFailed},
  {"makeSurfaceExtrusion", ErrorCode::ConstructionFailed},
  {"makeSurfaceOffset", ErrorCode::ConstructionFailed},
  {"makeSurfaceRevolution", ErrorCode::ConstructionFailed},
  {"makeSurfaceRuled", ErrorCode::ConstructionFailed},
  {"surfaceIsoCurve", ErrorCode::ConstructionFailed},
  {"makeSolidFromShell", ErrorCode::ConstructionFailed},
  {"makeEdgeArc", ErrorCode::ConstructionFailed},
  {"makeEdgeBezier", ErrorCode::ConstructionFailed},
  {"makeEdgeBSpline", ErrorCode::ConstructionFailed},
  {"makeEdgeCircle", ErrorCode::ConstructionFailed},
  {"makeEdgeEllipse", ErrorCode::ConstructionFailed},
  {"makeEdgeHelix", ErrorCode::ConstructionFailed},
  {"makeEdgeHyperbola", ErrorCode::ConstructionFailed},
  {"makeEdgeLine", ErrorCode::ConstructionFailed},
  {"makeEdgeOffset", ErrorCode::ConstructionFailed},
  {"makeEdgeParabola", ErrorCode::ConstructionFailed},
  {"trimCurve", ErrorCode::ConstructionFailed},
  {"trimSurface", ErrorCode::ConstructionFailed},
  {"convertCurveToBSpline", ErrorCode::ConstructionFailed},
  {"convertSurfaceToBSpline", ErrorCode::ConstructionFailed},
  {"updateCurvePole", ErrorCode::ConstructionFailed},
  {"updateSurfacePole", ErrorCode::ConstructionFailed},
  {"extrude", ErrorCode::ConstructionFailed},
  {"revolve", ErrorCode::ConstructionFailed},
  {"loft", ErrorCode::ConstructionFailed},
  {"draftAngle", ErrorCode::ConstructionFailed},
  {"localPrism", ErrorCode::ConstructionFailed},
  {"localRevolution", ErrorCode::ConstructionFailed},
  {"linearForm", ErrorCode::ConstructionFailed},
  {"cylindricalHole", ErrorCode::ConstructionFailed},
  {"sweepPipe", ErrorCode::ConstructionFailed},
  {"sweepPipeShell", ErrorCode::ConstructionFailed},
  {"offsetWire2D", ErrorCode::ConstructionFailed},
  {"transform", ErrorCode::ConstructionFailed},
  {"generalTransform", ErrorCode::ConstructionFailed},
  {"batchTransformCopy", ErrorCode::ConstructionFailed},
  {"translate", ErrorCode::ConstructionFailed},
  {"rotate", ErrorCode::ConstructionFailed},
  {"scale", ErrorCode::ConstructionFailed},
  {"mirror", ErrorCode::ConstructionFailed},
}};
inline constexpr ErrorCode operationFailureCode(const std::string_view theOperation)
{
  for (const auto& aPolicy : kOperationFailurePolicies)
    if (theOperation == aPolicy.operation) return aPolicy.code;
  return ErrorCode::KernelError;
}
enum class DocumentDispatchKind { None, Obj, Gltf };
struct DocumentDispatchPolicy { const char* operation; const char* argument; DocumentDispatchKind kind; };
inline constexpr std::array<DocumentDispatchPolicy, 2> kDocumentDispatchPolicies = {{
  {"exportOBJ", "positions", DocumentDispatchKind::Obj},
  {"exportGLTF", "document", DocumentDispatchKind::Gltf},
}};
inline constexpr DocumentDispatchPolicy documentDispatchPolicy(const std::string_view theOperation)
{
  for (const auto& aPolicy : kDocumentDispatchPolicies)
    if (theOperation == aPolicy.operation) return aPolicy;
  return {"", "", DocumentDispatchKind::None};
}
struct HistorySupportEntry { const char* operation; const char* support; };
inline constexpr std::array<HistorySupportEntry, 161> kHistorySupport = {{
  {"approximateCurveBSpline", "unsupported"},
  {"approximateSurfaceBSpline", "unsupported"},
  {"batch", "unsupported"},
  {"bbox", "unsupported"},
  {"beginScope", "unsupported"},
  {"booleanCommon", "full"},
  {"booleanCut", "full"},
  {"booleanFuse", "full"},
  {"classifyPoint", "unsupported"},
  {"capabilities", "unsupported"},
  {"convertCurveToBSpline", "unsupported"},
  {"convertSurfaceToBSpline", "unsupported"},
  {"createBuffer", "unsupported"},
  {"curveContinuity", "unsupported"},
  {"curveControlData", "unsupported"},
  {"curveDomain", "unsupported"},
  {"curveGeometry", "unsupported"},
  {"cylindricalHole", "full"},
  {"editCurveBSpline", "unsupported"},
  {"editSurfaceBSpline", "unsupported"},
  {"endScope", "unsupported"},
  {"evaluateCurve", "unsupported"},
  {"evaluateSurface", "unsupported"},
  {"extendCurve", "unsupported"},
  {"extendSurface", "unsupported"},
  {"extremaCurveCurve", "unsupported"},
  {"extremaCurveSurface", "unsupported"},
  {"extremaSurfaceSurface", "unsupported"},
  {"exportBREP", "unsupported"},
  {"exportGLTF", "unsupported"},
  {"exportIGES", "unsupported"},
  {"exportIGESDocument", "unsupported"},
  {"exportOBJ", "unsupported"},
  {"exportPLY", "unsupported"},
  {"exportSTEP", "unsupported"},
  {"exportSTEPDocument", "unsupported"},
  {"exportVRML", "unsupported"},
  {"exportXCAF", "unsupported"},
  {"freeBuffer", "unsupported"},
  {"getSubShape", "unsupported"},
  {"getSubShapes", "unsupported"},
  {"distance", "unsupported"},
  {"diagnoseShape", "unsupported"},
  {"defeature", "full"},
  {"draftAngle", "unsupported"},
  {"extrude", "full"},
  {"fillet", "full"},
  {"fixShape", "full"},
  {"generalFuse", "unsupported"},
  {"generalTransform", "full"},
  {"getAdjacency", "unsupported"},
  {"glue", "unsupported"},
  {"hollow", "unsupported"},
  {"importBREP", "unsupported"},
  {"importGLTF", "unsupported"},
  {"importIGES", "unsupported"},
  {"importIGESDocument", "unsupported"},
  {"importOBJ", "unsupported"},
  {"importPLY", "unsupported"},
  {"importSTEP", "unsupported"},
  {"importSTEPDocument", "unsupported"},
  {"importSTL", "unsupported"},
  {"importVRML", "unsupported"},
  {"importXCAF", "unsupported"},
  {"inspectTolerances", "unsupported"},
  {"intersectCurveCurve", "unsupported"},
  {"intersectCurveSurface", "unsupported"},
  {"intersectSurfaceSurface", "unsupported"},
  {"isSameShape", "unsupported"},
  {"isValid", "unsupported"},
  {"makeBox", "unsupported"},
  {"makeCompSolid", "unsupported"},
  {"makeCompound", "unsupported"},
  {"makeCone", "unsupported"},
  {"makeCylinder", "unsupported"},
  {"makeEdgeArc", "unsupported"},
  {"makeEdgeBezier", "unsupported"},
  {"makeEdgeBSpline", "unsupported"},
  {"makeEdgeCircle", "unsupported"},
  {"makeEdgeEllipse", "unsupported"},
  {"makeEdgeHelix", "unsupported"},
  {"makeEdgeHyperbola", "unsupported"},
  {"makeEdgeLine", "unsupported"},
  {"makeEdgeOffset", "unsupported"},
  {"makeEdgeParabola", "unsupported"},
  {"makeFace", "unsupported"},
  {"makeFaceOnSurface", "unsupported"},
  {"makeHalfSpace", "unsupported"},
  {"makePolygon", "unsupported"},
  {"makeSurfaceBezier", "unsupported"},
  {"makeSurfaceBSpline", "unsupported"},
  {"makeSurfaceExtrusion", "unsupported"},
  {"makeSurfaceFace", "unsupported"},
  {"makeSurfaceFilling", "unsupported"},
  {"makeSurfaceOffset", "unsupported"},
  {"makeSurfaceRevolution", "unsupported"},
  {"makeSurfaceRuled", "unsupported"},
  {"makeShell", "unsupported"},
  {"makeSolidFromShell", "unsupported"},
  {"makeSphere", "unsupported"},
  {"makeTorus", "unsupported"},
  {"makeVertex", "unsupported"},
  {"makeWedge", "unsupported"},
  {"makeWire", "unsupported"},
  {"massProps", "unsupported"},
  {"middlePath", "unsupported"},
  {"loft", "full"},
  {"localPrism", "unsupported"},
  {"localRevolution", "unsupported"},
  {"linearForm", "unsupported"},
  {"revolutionForm", "unsupported"},
  {"obb", "unsupported"},
  {"offsetShape", "unsupported"},
  {"offsetWire2D", "unsupported"},
  {"probeFormat", "unsupported"},
  {"projectPointCurve", "unsupported"},
  {"projectPointSurface", "unsupported"},
  {"projectHLR", "unsupported"},
  {"release", "unsupported"},
  {"releaseAll", "unsupported"},
  {"reparameterizeCurve", "unsupported"},
  {"reparameterizeSurface", "unsupported"},
  {"reduceCurveDegree", "unsupported"},
  {"reduceSurfaceDegree", "unsupported"},
  {"revolve", "full"},
  {"section", "unsupported"},
  {"sectionAnalysis", "unsupported"},
  {"selectGeneralFuseCells", "unsupported"},
  {"setTolerance", "unsupported"},
  {"sew", "full"},
  {"shapeUpgrade", "unsupported"},
  {"shapeType", "unsupported"},
  {"split", "unsupported"},
  {"stats", "unsupported"},
  {"surfaceControlData", "unsupported"},
  {"surfaceDomain", "unsupported"},
  {"surfaceContinuity", "unsupported"},
  {"surfaceGeometry", "unsupported"},
  {"surfaceIsoCurve", "unsupported"},
  {"sweepPipe", "full"},
  {"sweepPipeShell", "partial"},
  {"tessellate", "unsupported"},
  {"tessellateEdges", "unsupported"},
  {"topologyCounts", "unsupported"},
  {"triangulationData", "unsupported"},
  {"replaceTriangulation", "unsupported"},
  {"validateTriangulation", "unsupported"},
  {"repairTriangulation", "unsupported"},
  {"transform", "full"},
  {"trimCurve", "unsupported"},
  {"trimSurface", "unsupported"},
  {"unifySameDomain", "full"},
  {"updateCurvePole", "unsupported"},
  {"updateSurfacePole", "unsupported"},
  {"batchTransformCopy", "unsupported"},
  {"exportSTL", "unsupported"},
  {"chamfer", "full"},
  {"translate", "full"},
  {"rotate", "full"},
  {"scale", "full"},
  {"mirror", "full"},
}};
}
