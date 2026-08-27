// Focused geometry/topology operation handlers.

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
        if (!aPoint.is_array() || aPoint.size() != 3) throw KernelFailure(ErrorCode::InvalidArgs, "Polygon points must be vec3");
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
      for (const json& anEdge : theArgs.at("edges")) aBuilder.Add(TopoDS::Edge(theContext.arena().get(anEdge.get<std::uint32_t>())));
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
      if (!aBuilder.IsDone() || aBuilder.Face().IsNull()
          || !BRepCheck_Analyzer(aBuilder.Face(), true).IsValid())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct a valid planar face");
      return {{"shape", theContext.arena().add(aBuilder.Face(), aScope)}};
    }

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

json h_isValid(KernelOperationContext& theContext, const json& theArgs)
{
      BRepCheck_Analyzer anAnalyzer(theContext.arena().get(requiredU32(theArgs, "shape")), true);
      return {{"valid", anAnalyzer.IsValid()}};
    }

json h_diagnoseShape(KernelOperationContext& theContext, const json& theArgs)
{
      const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
      BRepCheck_Analyzer anAnalyzer(
        aShape, theArgs.value("geomControls", true), false, theArgs.value("exact", false));
      json anIssues = json::array();
      for (const auto& aType : std::vector<std::pair<TopAbs_ShapeEnum, const char*>>{
             {TopAbs_COMPOUND, "compound"}, {TopAbs_COMPSOLID, "compsolid"},
             {TopAbs_SOLID, "solid"}, {TopAbs_SHELL, "shell"}, {TopAbs_FACE, "face"},
             {TopAbs_WIRE, "wire"}, {TopAbs_EDGE, "edge"}, {TopAbs_VERTEX, "vertex"}})
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
            anIssues.push_back({
              {"type", aType.second}, {"index", anIndex - 1},
              {"status", checkStatusName(aStatus)}});
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
          if (aType.first == TopAbs_FACE)
            aTolerance = BRep_Tool::Tolerance(TopoDS::Face(aShapes(anIndex)));
          else if (aType.first == TopAbs_EDGE)
            aTolerance = BRep_Tool::Tolerance(TopoDS::Edge(aShapes(anIndex)));
          else
            aTolerance = BRep_Tool::Tolerance(TopoDS::Vertex(aShapes(anIndex)));
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
      if (aTolerance <= 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs, "Shape tolerance must be positive");
      const std::string aTypeName = theArgs.value("type", "all");
      TopAbs_ShapeEnum aType = TopAbs_SHAPE;
      if (aTypeName == "face") aType = TopAbs_FACE;
      else if (aTypeName == "edge") aType = TopAbs_EDGE;
      else if (aTypeName == "vertex") aType = TopAbs_VERTEX;
      else if (aTypeName != "all")
        throw KernelFailure(ErrorCode::InvalidArgs, "Shape tolerance type is invalid");
      BRepBuilderAPI_Copy aCopy(aShape, true, false);
      if (!aCopy.IsDone() || aCopy.Shape().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to copy shape for tolerance editing");
      TopoDS_Shape aResult = aCopy.Shape();
      ShapeFix_ShapeTolerance().SetTolerance(aResult, aTolerance, aType);
      return {{"shape", theContext.arena().add(aResult, aScope)}};
    }

json h_bbox(KernelOperationContext& theContext, const json& theArgs)
{
      Bnd_Box aBox;
      BRepBndLib::Add(theContext.arena().get(requiredU32(theArgs, "shape")), aBox, true);
      if (aBox.IsVoid())
      {
        throw KernelFailure(ErrorCode::KernelError, "Shape has an empty bounding box");
      }
      double xMin, yMin, zMin, xMax, yMax, zMax;
      aBox.Get(xMin, yMin, zMin, xMax, yMax, zMax);
      return {{"min", {xMin, yMin, zMin}}, {"max", {xMax, yMax, zMax}}};
    }

json h_obb(KernelOperationContext& theContext, const json& theArgs)
{
      Bnd_OBB aBox;
      BRepBndLib::AddOBB(
        theContext.arena().get(requiredU32(theArgs, "shape")), aBox,
        theArgs.value("useTriangulation", true),
        theArgs.value("optimal", false),
        theArgs.value("useShapeTolerance", true));
      if (aBox.IsVoid())
        throw KernelFailure(ErrorCode::KernelError, "Shape has an empty oriented bounding box");
      const gp_XYZ& aCenter = aBox.Center();
      const gp_XYZ& anX = aBox.XDirection();
      const gp_XYZ& aY = aBox.YDirection();
      const gp_XYZ& aZ = aBox.ZDirection();
      return {
        {"center", {aCenter.X(), aCenter.Y(), aCenter.Z()}},
        {"axes", {{anX.X(), anX.Y(), anX.Z()},
                  {aY.X(), aY.Y(), aY.Z()},
                  {aZ.X(), aZ.Y(), aZ.Z()}}},
        {"halfSizes", {aBox.XHSize(), aBox.YHSize(), aBox.ZHSize()}},
        {"axisAligned", aBox.IsAABox()}};
    }

} // namespace occt_worker
