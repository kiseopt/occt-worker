// Focused geometry/topology construction and validation handlers.

#include "kernel_geometry_topology_operations.hpp"
#include "kernel_geometry_helpers.hpp"
#include "kernel_operation_context.hpp"
#include "kernel_protocol_helpers.hpp"

namespace occt_worker {

json h_makeVertex(KernelOperationContext& theContext, const json& theArgs)
{
  const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
  const auto aPoint = requiredVec3(theArgs, "point");
  BRepBuilderAPI_MakeVertex aBuilder(gp_Pnt(aPoint[0], aPoint[1], aPoint[2]));
  if (!aBuilder.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct vertex");
  return {{"shape", theContext.arena().add(aBuilder.Shape(), aScope)}};
}

json h_makePolygon(KernelOperationContext& theContext, const json& theArgs)
{
  const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
  if (!theArgs.contains("points") || !theArgs.at("points").is_array() || theArgs.at("points").size() < 2)
    throw KernelFailure(ErrorCode::InvalidArgs, "makePolygon requires at least two points");
  BRepBuilderAPI_MakePolygon aBuilder;
  for (const json& aPoint : theArgs.at("points"))
  {
    if (!aPoint.is_array() || aPoint.size() != 3)
      throw KernelFailure(ErrorCode::InvalidArgs, "Polygon points must be vec3");
    aBuilder.Add(gp_Pnt(aPoint[0].get<double>(), aPoint[1].get<double>(), aPoint[2].get<double>()));
  }
  if (theArgs.value("close", true)) aBuilder.Close();
  if (!aBuilder.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct polygon");
  return {{"shape", theContext.arena().add(aBuilder.Wire(), aScope)}};
}

json h_makeWire(KernelOperationContext& theContext, const json& theArgs)
{
  const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
  if (!theArgs.contains("edges") || !theArgs.at("edges").is_array() || theArgs.at("edges").empty())
    throw KernelFailure(ErrorCode::InvalidArgs, "makeWire requires edges");
  BRepBuilderAPI_MakeWire aBuilder;
  for (const json& anEdge : theArgs.at("edges"))
    aBuilder.Add(TopoDS::Edge(theContext.arena().get(anEdge.get<std::uint32_t>())));
  if (!aBuilder.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct wire");
  return {{"shape", theContext.arena().add(aBuilder.Wire(), aScope)}};
}

json h_makeFace(KernelOperationContext& theContext, const json& theArgs)
{
  const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
  const TopoDS_Wire anOuter = TopoDS::Wire(theContext.arena().get(requiredU32(theArgs, "outer")));
  std::vector<TopoDS_Wire> aHoles;
  if (theArgs.contains("holes"))
  {
    if (!theArgs.at("holes").is_array()) throw KernelFailure(ErrorCode::InvalidArgs, "Face holes must be an array");
    for (const json& aHoleValue : theArgs.at("holes"))
    {
      TopoDS_Wire aHole = TopoDS::Wire(theContext.arena().get(aHoleValue.get<std::uint32_t>()));
      auto isValidWithHole = [&](const TopoDS_Wire& theCandidate) {
        BRepBuilderAPI_MakeFace aCandidate(anOuter, true);
        for (const TopoDS_Wire& anAcceptedHole : aHoles) aCandidate.Add(anAcceptedHole);
        aCandidate.Add(theCandidate);
        return aCandidate.IsDone() && !aCandidate.Face().IsNull()
          && BRepCheck_Analyzer(aCandidate.Face(), true).IsValid();
      };
      if (!isValidWithHole(aHole))
      {
        aHole.Reverse();
        if (!isValidWithHole(aHole))
          throw KernelFailure(ErrorCode::ConstructionFailed, "Face hole does not define a valid planar inner wire");
      }
      aHoles.push_back(aHole);
    }
  }
  BRepBuilderAPI_MakeFace aBuilder(anOuter, true);
  for (const TopoDS_Wire& aHole : aHoles) aBuilder.Add(aHole);
  if (!aBuilder.IsDone() || aBuilder.Face().IsNull() || !BRepCheck_Analyzer(aBuilder.Face(), true).IsValid())
    throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct a valid planar face");
  return {{"shape", theContext.arena().add(aBuilder.Face(), aScope)}};
}

json h_isValid(KernelOperationContext& theContext, const json& theArgs)
{
  BRepCheck_Analyzer anAnalyzer(theContext.arena().get(requiredU32(theArgs, "shape")), true);
  return {{"valid", anAnalyzer.IsValid()}};
}

json h_diagnoseShape(KernelOperationContext& theContext, const json& theArgs)
{
  const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
  BRepCheck_Analyzer anAnalyzer(aShape, theArgs.value("geomControls", true), false, theArgs.value("exact", false));
  json anIssues = json::array();
  for (const auto& aType : std::vector<std::pair<TopAbs_ShapeEnum, const char*>>{
         {TopAbs_COMPOUND, "compound"}, {TopAbs_COMPSOLID, "compsolid"}, {TopAbs_SOLID, "solid"},
         {TopAbs_SHELL, "shell"}, {TopAbs_FACE, "face"}, {TopAbs_WIRE, "wire"},
         {TopAbs_EDGE, "edge"}, {TopAbs_VERTEX, "vertex"}})
  {
    TopTools_IndexedMapOfShape aShapes;
    TopExp::MapShapes(aShape, aType.first, aShapes);
    for (int anIndex = 1; anIndex <= aShapes.Extent(); ++anIndex)
    {
      const occ::handle<BRepCheck_Result>& aResult = anAnalyzer.Result(aShapes(anIndex));
      if (aResult.IsNull()) continue;
      for (const BRepCheck_Status aStatus : aResult->Status())
      {
        if (aStatus == BRepCheck_NoError) continue;
        anIssues.push_back({{"type", aType.second}, {"index", anIndex - 1}, {"status", checkStatusName(aStatus)}});
      }
    }
  }
  return {{"valid", anAnalyzer.IsValid()}, {"issues", anIssues}};
}

json h_inspectTolerances(KernelOperationContext& theContext, const json& theArgs)
{
  const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
  json aResult;
  for (const auto& aType : std::vector<std::pair<TopAbs_ShapeEnum, const char*>>{
         {TopAbs_FACE, "faces"}, {TopAbs_EDGE, "edges"}, {TopAbs_VERTEX, "vertices"}})
  {
    TopTools_IndexedMapOfShape aShapes;
    TopExp::MapShapes(aShape, aType.first, aShapes);
    json aValues = json::array();
    for (int anIndex = 1; anIndex <= aShapes.Extent(); ++anIndex)
    {
      double aTolerance = 0.0;
      if (aType.first == TopAbs_FACE) aTolerance = BRep_Tool::Tolerance(TopoDS::Face(aShapes(anIndex)));
      else if (aType.first == TopAbs_EDGE) aTolerance = BRep_Tool::Tolerance(TopoDS::Edge(aShapes(anIndex)));
      else aTolerance = BRep_Tool::Tolerance(TopoDS::Vertex(aShapes(anIndex)));
      aValues.push_back({{"index", anIndex - 1}, {"tolerance", aTolerance}});
    }
    aResult[aType.second] = std::move(aValues);
  }
  return aResult;
}

json h_setTolerance(KernelOperationContext& theContext, const json& theArgs)
{
  const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
  const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
  const double aTolerance = requiredNumber(theArgs, "tolerance");
  if (aTolerance <= 0.0) throw KernelFailure(ErrorCode::InvalidArgs, "Shape tolerance must be positive");
  const std::string aTypeName = theArgs.value("type", "all");
  TopAbs_ShapeEnum aType = TopAbs_SHAPE;
  if (aTypeName == "face") aType = TopAbs_FACE;
  else if (aTypeName == "edge") aType = TopAbs_EDGE;
  else if (aTypeName == "vertex") aType = TopAbs_VERTEX;
  else if (aTypeName != "all") throw KernelFailure(ErrorCode::InvalidArgs, "Shape tolerance type is invalid");
  BRepBuilderAPI_Copy aCopy(aShape, true, false);
  if (!aCopy.IsDone() || aCopy.Shape().IsNull())
    throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to copy shape for tolerance editing");
  TopoDS_Shape aResult = aCopy.Shape();
  ShapeFix_ShapeTolerance().SetTolerance(aResult, aTolerance, aType);
  return {{"shape", theContext.arena().add(aResult, aScope)}};
}

} // namespace occt_worker
