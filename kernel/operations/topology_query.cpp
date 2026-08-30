#include "kernel_topology_query_operations.hpp"
#include "kernel_geometry_helpers.hpp"
#include "kernel_operation_context.hpp"
#include "kernel_protocol_helpers.hpp"
#include "side_registration.hpp"

namespace occt_worker {

json h_topologyCounts(KernelOperationContext& theContext, const json& theArgs)
{
  const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
  json aCounts = json::object();
  for (const auto& aType : std::vector<std::pair<TopAbs_ShapeEnum, const char*>>{
         {TopAbs_COMPOUND, "compound"}, {TopAbs_COMPSOLID, "compsolid"}, {TopAbs_SOLID, "solid"},
         {TopAbs_SHELL, "shell"}, {TopAbs_FACE, "face"}, {TopAbs_WIRE, "wire"}, {TopAbs_EDGE, "edge"}, {TopAbs_VERTEX, "vertex"}})
  {
    TopTools_IndexedMapOfShape aMap;
    TopExp::MapShapes(aShape, aType.first, aMap);
    aCounts[aType.second] = aMap.Extent();
  }
  return aCounts;
}

json h_getSubShapes(KernelOperationContext& theContext, const json& theArgs)
{
  const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
  const std::string aTypeName = theArgs.value("type", "face");
  const auto aType = shapeTypeFromName(aTypeName);
  TopTools_IndexedMapOfShape aMap;
  TopExp::MapShapes(aShape, aType, aMap);
  json aShapes = json::array();
  for (int anIndex = 1; anIndex <= aMap.Extent(); ++anIndex)
    aShapes.push_back({{"type", aTypeName}, {"index", anIndex - 1}});
  return {{"shapes", aShapes}};
}

json h_getSubShape(KernelOperationContext& theContext, const json& theArgs)
{
  const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
  const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
  const TopAbs_ShapeEnum aType = shapeTypeFromName(theArgs.at("type").get<std::string>());
  const std::uint32_t anIndex = requiredU32(theArgs, "index");
  TopTools_IndexedMapOfShape aMap;
  TopExp::MapShapes(aShape, aType, aMap);
  if (anIndex >= static_cast<std::uint32_t>(aMap.Extent()))
    throw KernelFailure(ErrorCode::InvalidArgs, "Subshape index is out of range");
  return {{"shape", theContext.arena().add(aMap(static_cast<int>(anIndex + 1)), aScope)}};
}

json h_getAdjacency(KernelOperationContext& theContext, const json& theArgs)
{
  const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
  const std::string aFromName = theArgs.value("from", "edge");
  const std::string aToName = theArgs.value("to", aFromName == "face" || aFromName == "vertex" ? "edge" : "face");
  const TopAbs_ShapeEnum aFrom = shapeTypeFromName(aFromName);
  const TopAbs_ShapeEnum aTo = shapeTypeFromName(aToName);
  const bool isAncestorDirection = (aFrom == TopAbs_EDGE && aTo == TopAbs_FACE)
    || (aFrom == TopAbs_VERTEX && aTo == TopAbs_EDGE);
  const bool isChildDirection = (aFrom == TopAbs_FACE && aTo == TopAbs_EDGE)
    || (aFrom == TopAbs_EDGE && aTo == TopAbs_VERTEX);
  if (!isAncestorDirection && !isChildDirection)
    throw KernelFailure(ErrorCode::InvalidArgs,
                        "getAdjacency supports edge-to-face, face-to-edge, vertex-to-edge, and edge-to-vertex");
  TopTools_IndexedMapOfShape aFromShapes;
  TopTools_IndexedMapOfShape aToShapes;
  TopExp::MapShapes(aShape, aFrom, aFromShapes);
  TopExp::MapShapes(aShape, aTo, aToShapes);
  NCollection_IndexedDataMap<TopoDS_Shape, TopTools_ListOfShape, TopTools_ShapeMapHasher> aMap;
  if (isAncestorDirection) TopExp::MapShapesAndUniqueAncestors(aShape, aFrom, aTo, aMap);
  json anItems = json::array();
  for (int anIndex = 1; anIndex <= aFromShapes.Extent(); ++anIndex)
  {
    std::vector<int> aTargetIndices;
    if (isAncestorDirection)
    {
      TopTools_ListOfShape aList;
      if (aMap.FindFromKey(aFromShapes(anIndex), aList))
        for (const TopoDS_Shape& aTarget : aList)
        {
          const int aTargetIndex = aToShapes.FindIndex(aTarget);
          if (aTargetIndex > 0) aTargetIndices.push_back(aTargetIndex - 1);
        }
    }
    else
    {
      TopTools_IndexedMapOfShape aChildren;
      TopExp::MapShapes(aFromShapes(anIndex), aTo, aChildren);
      for (int aChildIndex = 1; aChildIndex <= aChildren.Extent(); ++aChildIndex)
      {
        const int aTargetIndex = aToShapes.FindIndex(aChildren(aChildIndex));
        if (aTargetIndex > 0) aTargetIndices.push_back(aTargetIndex - 1);
      }
    }
    std::sort(aTargetIndices.begin(), aTargetIndices.end());
    aTargetIndices.erase(std::unique(aTargetIndices.begin(), aTargetIndices.end()), aTargetIndices.end());
    json aTargets = json::array();
    for (const int aTargetIndex : aTargetIndices) aTargets.push_back(aTargetIndex);
    anItems.push_back({{"index", anIndex - 1}, {"adjacent", aTargets}});
  }
  return {{"from", aFromName}, {"to", aToName}, {"items", anItems}};
}

json h_shapeType(KernelOperationContext& theContext, const json& theArgs)
{
  return {{"type", shapeTypeName(theContext.arena().get(requiredU32(theArgs, "shape")).ShapeType())}};
}

json h_isSameShape(KernelOperationContext& theContext, const json& theArgs)
{
  return {{"same", theContext.arena().get(requiredU32(theArgs, "first")).IsSame(theContext.arena().get(requiredU32(theArgs, "second")))}};
}

json h_bbox(KernelOperationContext& theContext, const json& theArgs)
{
  Bnd_Box aBox;
  BRepBndLib::Add(theContext.arena().get(requiredU32(theArgs, "shape")), aBox, true);
  if (aBox.IsVoid()) throw KernelFailure(ErrorCode::KernelError, "Shape has an empty bounding box");
  double xMin, yMin, zMin, xMax, yMax, zMax;
  aBox.Get(xMin, yMin, zMin, xMax, yMax, zMax);
  return {{"min", {xMin, yMin, zMin}}, {"max", {xMax, yMax, zMax}}};
}

json h_obb(KernelOperationContext& theContext, const json& theArgs)
{
  Bnd_OBB aBox;
  BRepBndLib::AddOBB(theContext.arena().get(requiredU32(theArgs, "shape")), aBox,
                     theArgs.value("useTriangulation", true), theArgs.value("optimal", false),
                     theArgs.value("useShapeTolerance", true));
  if (aBox.IsVoid()) throw KernelFailure(ErrorCode::KernelError, "Shape has an empty oriented bounding box");
  const gp_XYZ& aCenter = aBox.Center();
  const gp_XYZ& anX = aBox.XDirection();
  const gp_XYZ& aY = aBox.YDirection();
  const gp_XYZ& aZ = aBox.ZDirection();
  return {{"center", {aCenter.X(), aCenter.Y(), aCenter.Z()}},
          {"axes", {{anX.X(), anX.Y(), anX.Z()}, {aY.X(), aY.Y(), aY.Z()}, {aZ.X(), aZ.Y(), aZ.Z()}}},
          {"halfSizes", {aBox.XHSize(), aBox.YHSize(), aBox.ZHSize()}}, {"axisAligned", aBox.IsAABox()}};
}

void register_topology_query_operations()
{
  auto& aRegistry = OperationRegistry::instance();
#define OCCT_REGISTER_HANDLER(theOperation) aRegistry.add(#theOperation, &h_##theOperation);
  OCCT_TOPOLOGY_QUERY_OPERATION_TABLE(OCCT_REGISTER_HANDLER)
#undef OCCT_REGISTER_HANDLER
}

OCCT_DEFINE_SIDE_PLUGIN(topology_query, kTopologyQueryOperationNames, register_topology_query_operations)

} // namespace occt_worker
