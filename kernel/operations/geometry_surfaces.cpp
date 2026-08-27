// Focused geometry/topology operation handlers.

#include "kernel_geometry_topology_operations.hpp"
#include "kernel_geometry_helpers.hpp"
#include "kernel_operation_context.hpp"
#include "kernel_protocol_helpers.hpp"

namespace occt_worker {

json h_makeSurfaceFilling(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aBoundaryShape = theContext.arena().get(requiredU32(theArgs, "boundary"));
      if (aBoundaryShape.ShapeType() != TopAbs_WIRE)
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface filling boundary must be a wire");
      const TopoDS_Wire aBoundary = TopoDS::Wire(aBoundaryShape);
      if (!BRep_Tool::IsClosed(aBoundary))
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface filling boundary wire must be closed");

      const int aDegree = theArgs.value("degree", 3);
      const int aPointsOnCurve = theArgs.value("pointsOnCurve", 15);
      const int anIterations = theArgs.value("iterations", 2);
      const bool isAnisotropic = theArgs.value("anisotropic", false);
      const double aTolerance2d = theArgs.value("tolerance2d", 1.0e-5);
      const double aTolerance3d = theArgs.value("tolerance3d", 1.0e-4);
      const double anAngularTolerance = theArgs.value("angularTolerance", 1.0e-2);
      const double aCurvatureTolerance = theArgs.value("curvatureTolerance", 1.0e-1);
      const int aMaxDegree = theArgs.value("maxDegree", 8);
      const int aMaxSegments = theArgs.value("maxSegments", 9);
      if (aDegree < 2 || aPointsOnCurve < 2 || anIterations < 1 || aMaxDegree < 1
          || aMaxSegments < 1 || aTolerance2d <= 0.0 || aTolerance3d <= 0.0
          || anAngularTolerance <= 0.0 || aCurvatureTolerance <= 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface filling settings must be positive and use degree >= 2");

      BRepOffsetAPI_MakeFilling aBuilder(
        aDegree, aPointsOnCurve, anIterations, isAnisotropic, aTolerance2d,
        aTolerance3d, anAngularTolerance, aCurvatureTolerance, aMaxDegree,
        aMaxSegments);
      std::vector<TopoDS_Edge> aBoundaryEdges;
      int anEdgeCount = 0;
      for (BRepTools_WireExplorer anExplorer(aBoundary); anExplorer.More(); anExplorer.Next())
      {
        aBoundaryEdges.push_back(TopoDS::Edge(anExplorer.Current()));
        ++anEdgeCount;
      }
      if (anEdgeCount < 2)
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface filling boundary must contain at least two edges");
      std::vector<GeomAbs_Shape> aConstraintOrders(aBoundaryEdges.size(), GeomAbs_C0);
      std::vector<TopoDS_Face> aConstraintSupports(aBoundaryEdges.size());
      if (theArgs.contains("constraints"))
      {
        const json& aConstraints = theArgs.at("constraints");
        if (!aConstraints.is_array())
          throw KernelFailure(ErrorCode::InvalidArgs, "Surface filling constraints must be an array");
        std::unordered_set<std::uint32_t> aConstrainedEdges;
        for (const json& aConstraint : aConstraints)
        {
          if (!aConstraint.is_object() || !aConstraint.contains("edgeIndex")
              || !aConstraint.contains("support") || !aConstraint.contains("continuity")
              || !aConstraint.at("edgeIndex").is_number_unsigned()
              || !aConstraint.at("support").is_number_unsigned()
              || !aConstraint.at("continuity").is_string())
            throw KernelFailure(ErrorCode::InvalidArgs, "Surface filling constraints require edgeIndex, support, and continuity");
          const std::uint32_t anEdgeIndex = aConstraint.at("edgeIndex").get<std::uint32_t>();
          if (anEdgeIndex >= aBoundaryEdges.size() || !aConstrainedEdges.insert(anEdgeIndex).second)
            throw KernelFailure(ErrorCode::InvalidArgs, "Surface filling constraint edgeIndex must be unique and in wire range");
          const TopoDS_Shape& aSupportShape = theContext.arena().get(aConstraint.at("support").get<std::uint32_t>());
          if (aSupportShape.ShapeType() != TopAbs_FACE)
            throw KernelFailure(ErrorCode::InvalidArgs, "Surface filling constraint support must be a face");
          const std::string aContinuity = aConstraint.at("continuity").get<std::string>();
          // BRepFill interprets this enum value as a raw G-order (0..2), while
          // GeomAbs_G2 is 3 because GeomAbs_C1 occupies value 2.
          const GeomAbs_Shape anOrder = aContinuity == "g1" ? GeomAbs_G1
            : aContinuity == "g2" ? static_cast<GeomAbs_Shape>(2) : GeomAbs_C0;
          if (anOrder == GeomAbs_C0)
            throw KernelFailure(ErrorCode::InvalidArgs, "Surface filling constraint continuity must be g1 or g2");
          double aFirst = 0.0;
          double aLast = 0.0;
          if (BRep_Tool::CurveOnSurface(aBoundaryEdges[anEdgeIndex], TopoDS::Face(aSupportShape), aFirst, aLast).IsNull())
            throw KernelFailure(ErrorCode::InvalidArgs, "Surface filling support face has no pcurve for the constrained edge");
          aConstraintOrders[anEdgeIndex] = anOrder;
          aConstraintSupports[anEdgeIndex] = TopoDS::Face(aSupportShape);
        }
      }
      for (std::size_t anIndex = 0; anIndex < aBoundaryEdges.size(); ++anIndex)
      {
        if (aConstraintOrders[anIndex] == GeomAbs_C0)
          aBuilder.Add(aBoundaryEdges[anIndex], GeomAbs_C0, true);
        else
          aBuilder.Add(aBoundaryEdges[anIndex], aConstraintSupports[anIndex], aConstraintOrders[anIndex], true);
      }
      if (theArgs.contains("points"))
      {
        const json& aPoints = theArgs.at("points");
        if (!aPoints.is_array())
          throw KernelFailure(ErrorCode::InvalidArgs, "Surface filling points must be an array");
        for (const json& aPointValue : aPoints)
        {
          if (!aPointValue.is_array() || aPointValue.size() != 3
              || !aPointValue.at(0).is_number() || !aPointValue.at(1).is_number()
              || !aPointValue.at(2).is_number())
            throw KernelFailure(ErrorCode::InvalidArgs, "Surface filling points must be vec3 values");
          aBuilder.Add(gp_Pnt(aPointValue.at(0).get<double>(),
                              aPointValue.at(1).get<double>(),
                              aPointValue.at(2).get<double>()));
        }
      }
      aBuilder.Build();
      if (!aBuilder.IsDone() || aBuilder.Shape().IsNull()
          || aBuilder.Shape().ShapeType() != TopAbs_FACE
          || !BRepCheck_Analyzer(aBuilder.Shape(), true).IsValid())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct a valid filling surface");
      return {{"shape", theContext.arena().add(aBuilder.Shape(), aScope)}};
    }

json h_makeFaceOnSurface(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aSupportShape = theContext.arena().get(requiredU32(theArgs, "surface"));
      if (aSupportShape.ShapeType() != TopAbs_FACE)
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface support must be a face");
      const TopoDS_Face aSupportFace = TopoDS::Face(aSupportShape);
      const occ::handle<Geom_Surface> aSurface = BRep_Tool::Surface(aSupportFace);
      if (aSurface.IsNull())
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface support face has no geometry");
      const double aTolerance = theArgs.value("tolerance", 1.0e-7);
      if (aTolerance <= 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface face tolerance must be positive");

      auto projectWire = [&](const std::uint32_t theHandle) {
        const TopoDS_Shape& aWireShape = theContext.arena().get(theHandle);
        if (aWireShape.ShapeType() != TopAbs_WIRE)
          throw KernelFailure(ErrorCode::InvalidArgs, "Surface face boundaries must be wires");
        BRepBuilderAPI_Copy aCopy(aWireShape, true, false);
        if (!aCopy.IsDone() || aCopy.Shape().IsNull())
          throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to copy a surface boundary wire");
        TopoDS_Wire aWire = TopoDS::Wire(aCopy.Shape());
        ShapeFix_Edge aFixer;
        for (TopExp_Explorer anExplorer(aWire, TopAbs_EDGE); anExplorer.More(); anExplorer.Next())
        {
          const TopoDS_Edge anEdge = TopoDS::Edge(anExplorer.Current());
          double aFirst = 0.0;
          double aLast = 0.0;
          occ::handle<Geom2d_Curve> aPCurve = BRep_Tool::CurveOnSurface(
            anEdge, aSurface, TopLoc_Location(), aFirst, aLast);
          if (aPCurve.IsNull())
          {
            aFixer.FixAddPCurve(anEdge, aSurface, TopLoc_Location(), false, aTolerance);
            aPCurve = BRep_Tool::CurveOnSurface(
              anEdge, aSurface, TopLoc_Location(), aFirst, aLast);
          }
          if (aPCurve.IsNull())
            throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to project a boundary edge onto the surface");
          aFixer.FixSameParameter(anEdge, aTolerance);
        }
        return aWire;
      };

      const TopoDS_Wire anOuter = projectWire(requiredU32(theArgs, "outer"));
      std::vector<TopoDS_Wire> aHoles;
      if (theArgs.contains("holes"))
      {
        if (!theArgs.at("holes").is_array())
          throw KernelFailure(ErrorCode::InvalidArgs, "Surface face holes must be an array");
        for (const json& aHoleValue : theArgs.at("holes"))
        {
          if (!aHoleValue.is_number_unsigned() && !aHoleValue.is_number_integer())
            throw KernelFailure(ErrorCode::InvalidArgs, "Surface face hole handles must be integers");
          aHoles.push_back(projectWire(aHoleValue.get<std::uint32_t>()));
        }
      }

      auto buildFace = [&](const std::vector<TopoDS_Wire>& theHoles, TopoDS_Face& theFace) {
        BRepBuilderAPI_MakeFace aBuilder(aSurface, anOuter, true);
        for (const TopoDS_Wire& aHole : theHoles) aBuilder.Add(aHole);
        if (!aBuilder.IsDone() || aBuilder.Face().IsNull()
            || !BRepCheck_Analyzer(aBuilder.Face(), true).IsValid())
          return false;
        theFace = aBuilder.Face();
        return true;
      };
      TopoDS_Face aFace;
      if (!buildFace(aHoles, aFace))
      {
        for (TopoDS_Wire& aHole : aHoles) aHole.Reverse();
        if (!buildFace(aHoles, aFace))
          throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct a valid wire-trimmed surface face");
      }
      aFace.Orientation(aSupportFace.Orientation());
      return {{"shape", theContext.arena().add(aFace, aScope)}};
    }

namespace {
struct SurfacePoleData
{
  SurfacePoleData(int theUCount, int theVCount)
    : uCount(theUCount), vCount(theVCount), poles(1, theUCount, 1, theVCount),
      weights(1, theUCount, 1, theVCount)
  {}

  int uCount;
  int vCount;
  TColgp_Array2OfPnt poles;
  TColStd_Array2OfReal weights;
  bool hasWeights = false;
};

SurfacePoleData readSurfacePoles(const json& theArgs)
{
      if (!theArgs.contains("poles") || !theArgs.at("poles").is_array()
          || theArgs.at("poles").size() < 2)
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface poles must be a rectangular U-major grid with at least two rows");
      const json& aPoleGrid = theArgs.at("poles");
      if (!aPoleGrid.at(0).is_array() || aPoleGrid.at(0).size() < 2)
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface poles require at least two V poles");
      const int aUCount = static_cast<int>(aPoleGrid.size());
      const int aVCount = static_cast<int>(aPoleGrid.at(0).size());
      SurfacePoleData aData(aUCount, aVCount);
      for (int aU = 0; aU < aUCount; ++aU)
      {
        const json& aRow = aPoleGrid.at(aU);
        if (!aRow.is_array() || static_cast<int>(aRow.size()) != aVCount)
          throw KernelFailure(ErrorCode::InvalidArgs, "Surface pole grid must be rectangular");
        for (int aV = 0; aV < aVCount; ++aV)
        {
          const json& aPoint = aRow.at(aV);
          if (!aPoint.is_array() || aPoint.size() != 3 || !aPoint.at(0).is_number()
              || !aPoint.at(1).is_number() || !aPoint.at(2).is_number())
            throw KernelFailure(ErrorCode::InvalidArgs, "Surface poles must be vec3 values");
          aData.poles.SetValue(aU + 1, aV + 1,
                               gp_Pnt(aPoint.at(0).get<double>(), aPoint.at(1).get<double>(),
                                      aPoint.at(2).get<double>()));
        }
      }

      aData.hasWeights = theArgs.contains("weights");
      if (aData.hasWeights)
      {
        const json& aWeightGrid = theArgs.at("weights");
        if (!aWeightGrid.is_array() || static_cast<int>(aWeightGrid.size()) != aUCount)
          throw KernelFailure(ErrorCode::InvalidArgs, "Surface weights must match the pole grid");
        for (int aU = 0; aU < aUCount; ++aU)
        {
          const json& aRow = aWeightGrid.at(aU);
          if (!aRow.is_array() || static_cast<int>(aRow.size()) != aVCount)
            throw KernelFailure(ErrorCode::InvalidArgs, "Surface weights must match the pole grid");
          for (int aV = 0; aV < aVCount; ++aV)
          {
            if (!aRow.at(aV).is_number() || aRow.at(aV).get<double>() <= 0.0)
              throw KernelFailure(ErrorCode::InvalidArgs, "Surface weights must be positive numbers");
            aData.weights.SetValue(aU + 1, aV + 1, aRow.at(aV).get<double>());
          }
        }
      }

      return aData;
}

} // namespace

json h_makeSurfaceBezier(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const SurfacePoleData aData = readSurfacePoles(theArgs);
      if (aData.uCount - 1 > Geom_BezierSurface::MaxDegree()
          || aData.vCount - 1 > Geom_BezierSurface::MaxDegree())
        throw KernelFailure(ErrorCode::InvalidArgs, "Bezier surface degree exceeds the OCCT maximum");
      const occ::handle<Geom_Surface> aSurface = aData.hasWeights
        ? occ::handle<Geom_Surface>(new Geom_BezierSurface(aData.poles, aData.weights))
        : occ::handle<Geom_Surface>(new Geom_BezierSurface(aData.poles));
      const double aTolerance = theArgs.value("tolerance", 1.0e-7);
      double aUFirst = 0.0;
      double aULast = 0.0;
      double aVFirst = 0.0;
      double aVLast = 0.0;
      aSurface->Bounds(aUFirst, aULast, aVFirst, aVLast);
      const TopoDS_Face aFace = makeRectangularFace(
        aSurface, aUFirst, aULast, aVFirst, aVLast, aTolerance, TopAbs_FORWARD);
      return {{"shape", theContext.arena().add(aFace, aScope)}};
}

json h_makeSurfaceBSpline(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const SurfacePoleData aData = readSurfacePoles(theArgs);
      const int aUCount = aData.uCount;
      const int aVCount = aData.vCount;
      const TColgp_Array2OfPnt& aPoles = aData.poles;
      const TColStd_Array2OfReal& aWeights = aData.weights;
      const bool hasWeights = aData.hasWeights;
      occ::handle<Geom_Surface> aSurface;
      {
        const int aUDegree = theArgs.value("uDegree", std::min(3, aUCount - 1));
        const int aVDegree = theArgs.value("vDegree", std::min(3, aVCount - 1));
        if (aUDegree < 1 || aUDegree >= aUCount || aUDegree > Geom_BSplineSurface::MaxDegree()
            || aVDegree < 1 || aVDegree >= aVCount || aVDegree > Geom_BSplineSurface::MaxDegree())
          throw KernelFailure(ErrorCode::InvalidArgs, "BSpline surface degrees must be between 1 and poleCount - 1");
        const bool isUPeriodic = theArgs.value("uPeriodic", false);
        const bool isVPeriodic = theArgs.value("vPeriodic", false);
        std::vector<double> aUKnots;
        std::vector<double> aVKnots;
        std::vector<int> aUMultiplicities;
        std::vector<int> aVMultiplicities;
        auto readDirection = [&](const char* theKnotKey,
                                 const char* theMultiplicityKey,
                                 const int thePoleCount,
                                 const int theDegree,
                                 const bool isPeriodic,
                                 std::vector<double>& theKnots,
                                 std::vector<int>& theMultiplicities) {
          const bool hasKnots = theArgs.contains(theKnotKey);
          const bool hasMultiplicities = theArgs.contains(theMultiplicityKey);
          if (hasKnots != hasMultiplicities)
            throw KernelFailure(ErrorCode::InvalidArgs, "BSpline knots and multiplicities must be provided together");
          if (isPeriodic && !hasKnots)
            throw KernelFailure(ErrorCode::InvalidArgs, "Periodic BSpline surfaces require explicit knots and multiplicities");
          if (!hasKnots)
          {
            const int aKnotCount = thePoleCount - theDegree + 1;
            for (int anIndex = 0; anIndex < aKnotCount; ++anIndex)
            {
              theKnots.push_back(static_cast<double>(anIndex));
              theMultiplicities.push_back(
                anIndex == 0 || anIndex == aKnotCount - 1 ? theDegree + 1 : 1);
            }
          }
          else
          {
            const json& aKnots = theArgs.at(theKnotKey);
            const json& aMultiplicities = theArgs.at(theMultiplicityKey);
            if (!aKnots.is_array() || !aMultiplicities.is_array() || aKnots.size() < 2
                || aKnots.size() != aMultiplicities.size())
              throw KernelFailure(ErrorCode::InvalidArgs, "BSpline knots and multiplicities must have equal length of at least two");
            for (std::size_t anIndex = 0; anIndex < aKnots.size(); ++anIndex)
            {
              if (!aKnots.at(anIndex).is_number()
                  || (!aMultiplicities.at(anIndex).is_number_integer()
                      && !aMultiplicities.at(anIndex).is_number_unsigned()))
                throw KernelFailure(ErrorCode::InvalidArgs, "BSpline knot data must be numeric with integer multiplicities");
              const double aKnot = aKnots.at(anIndex).get<double>();
              const int aMultiplicity = aMultiplicities.at(anIndex).get<int>();
              if (!std::isfinite(aKnot) || aMultiplicity <= 0
                  || (!theKnots.empty() && aKnot <= theKnots.back()))
                throw KernelFailure(ErrorCode::InvalidArgs, "BSpline knots must increase and multiplicities must be positive");
              theKnots.push_back(aKnot);
              theMultiplicities.push_back(aMultiplicity);
            }
          }
          TColStd_Array1OfInteger aMultiplicityArray(1, static_cast<int>(theMultiplicities.size()));
          for (int anIndex = 1; anIndex <= aMultiplicityArray.Upper(); ++anIndex)
            aMultiplicityArray.SetValue(anIndex, theMultiplicities[static_cast<std::size_t>(anIndex - 1)]);
          if (BSplCLib::NbPoles(theDegree, isPeriodic, aMultiplicityArray) != thePoleCount)
            throw KernelFailure(ErrorCode::InvalidArgs, "BSpline multiplicities do not match the degree and pole count");
        };
        readDirection("uKnots", "uMultiplicities", aUCount, aUDegree, isUPeriodic,
                      aUKnots, aUMultiplicities);
        readDirection("vKnots", "vMultiplicities", aVCount, aVDegree, isVPeriodic,
                      aVKnots, aVMultiplicities);

        TColStd_Array1OfReal aUKnotArray(1, static_cast<int>(aUKnots.size()));
        TColStd_Array1OfReal aVKnotArray(1, static_cast<int>(aVKnots.size()));
        TColStd_Array1OfInteger aUMultiplicityArray(1, static_cast<int>(aUMultiplicities.size()));
        TColStd_Array1OfInteger aVMultiplicityArray(1, static_cast<int>(aVMultiplicities.size()));
        for (int anIndex = 1; anIndex <= aUKnotArray.Upper(); ++anIndex)
        {
          aUKnotArray.SetValue(anIndex, aUKnots[static_cast<std::size_t>(anIndex - 1)]);
          aUMultiplicityArray.SetValue(anIndex, aUMultiplicities[static_cast<std::size_t>(anIndex - 1)]);
        }
        for (int anIndex = 1; anIndex <= aVKnotArray.Upper(); ++anIndex)
        {
          aVKnotArray.SetValue(anIndex, aVKnots[static_cast<std::size_t>(anIndex - 1)]);
          aVMultiplicityArray.SetValue(anIndex, aVMultiplicities[static_cast<std::size_t>(anIndex - 1)]);
        }
        aSurface = hasWeights
          ? occ::handle<Geom_Surface>(new Geom_BSplineSurface(
              aPoles, aWeights, aUKnotArray, aVKnotArray, aUMultiplicityArray,
              aVMultiplicityArray, aUDegree, aVDegree, isUPeriodic, isVPeriodic))
          : occ::handle<Geom_Surface>(new Geom_BSplineSurface(
              aPoles, aUKnotArray, aVKnotArray, aUMultiplicityArray,
              aVMultiplicityArray, aUDegree, aVDegree, isUPeriodic, isVPeriodic));
      }

      const double aTolerance = theArgs.value("tolerance", 1.0e-7);
      double aUFirst = 0.0;
      double aULast = 0.0;
      double aVFirst = 0.0;
      double aVLast = 0.0;
      aSurface->Bounds(aUFirst, aULast, aVFirst, aVLast);
      const TopoDS_Face aFace = makeRectangularFace(
        aSurface, aUFirst, aULast, aVFirst, aVLast, aTolerance, TopAbs_FORWARD);
      return {{"shape", theContext.arena().add(aFace, aScope)}};
    }

json h_makeSurfaceRuled(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aFirstShape = theContext.arena().get(requiredU32(theArgs, "first"));
      const TopoDS_Shape& aSecondShape = theContext.arena().get(requiredU32(theArgs, "second"));
      double aFirstStart = 0.0;
      double aFirstEnd = 0.0;
      double aSecondStart = 0.0;
      double aSecondEnd = 0.0;
      curveForEdge(aFirstShape, aFirstStart, aFirstEnd);
      curveForEdge(aSecondShape, aSecondStart, aSecondEnd);
      const TopoDS_Face aFace = BRepFill::Face(
        TopoDS::Edge(aFirstShape), TopoDS::Edge(aSecondShape));
      if (aFace.IsNull() || !BRepCheck_Analyzer(aFace, true).IsValid())
        throw KernelFailure(ErrorCode::ConstructionFailed,
                            "OCCT failed to construct ruled surface");
      return {{"shape", theContext.arena().add(aFace, aScope)}};
    }

json h_makeSurfaceOffset(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
      if (aShape.ShapeType() != TopAbs_FACE)
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "Offset surface construction requires a face shape");
      const TopoDS_Face aSourceFace = TopoDS::Face(aShape);
      const occ::handle<Geom_Surface> aSourceSurface = BRep_Tool::Surface(aSourceFace);
      if (aSourceSurface.IsNull())
        throw KernelFailure(ErrorCode::InvalidArgs, "Face has no geometric surface");
      const occ::handle<Geom_OffsetSurface> aSurface = new Geom_OffsetSurface(
        aSourceSurface, requiredNumber(theArgs, "offset"));
      const TopoDS_Face aFace = makeSurfaceFacePreservingWires(
        aSourceFace, aSurface, theArgs.value("tolerance", 1.0e-7),
        aSourceFace.Orientation());
      return {{"shape", theContext.arena().add(aFace, aScope)}};
    }

json h_makeSurfaceFace(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      if (!theArgs.contains("surface") || !theArgs.at("surface").is_object())
        throw KernelFailure(ErrorCode::InvalidArgs, "makeSurfaceFace requires a surface definition");
      const json& aDefinition = theArgs.at("surface");
      const std::string aType = aDefinition.value("type", "");
      const auto anOrigin = optionalVec3(aDefinition, "origin", {0.0, 0.0, 0.0});
      const auto aDirection = optionalDirection(aDefinition, "direction", {0.0, 0.0, 1.0});
      gp_Ax3 anAxis(gp_Pnt(anOrigin[0], anOrigin[1], anOrigin[2]),
                    gp_Dir(aDirection[0], aDirection[1], aDirection[2]));
      if (aDefinition.contains("xDirection"))
      {
        const auto anXDirection = requiredDirection(aDefinition, "xDirection");
        if (gp_Dir(aDirection[0], aDirection[1], aDirection[2]).IsParallel(
              gp_Dir(anXDirection[0], anXDirection[1], anXDirection[2]), 1.0e-12))
          throw KernelFailure(ErrorCode::InvalidArgs, "Surface direction and xDirection must not be parallel");
        anAxis = gp_Ax3(gp_Pnt(anOrigin[0], anOrigin[1], anOrigin[2]),
                        gp_Dir(aDirection[0], aDirection[1], aDirection[2]),
                        gp_Dir(anXDirection[0], anXDirection[1], anXDirection[2]));
      }

      occ::handle<Geom_Surface> aSurface;
      if (aType == "plane")
      {
        aSurface = new Geom_Plane(anAxis);
      }
      else if (aType == "cylinder" || aType == "sphere")
      {
        const double aRadius = requiredNumber(aDefinition, "radius");
        if (aRadius <= 0.0)
          throw KernelFailure(ErrorCode::InvalidArgs, "Surface radius must be positive");
        aSurface = aType == "cylinder"
          ? occ::handle<Geom_Surface>(new Geom_CylindricalSurface(anAxis, aRadius))
          : occ::handle<Geom_Surface>(new Geom_SphericalSurface(anAxis, aRadius));
      }
      else if (aType == "cone")
      {
        const double aReferenceRadius = requiredNumber(aDefinition, "referenceRadius");
        const double aSemiAngle = requiredNumber(aDefinition, "semiAngle");
        if (aReferenceRadius < 0.0 || std::abs(aSemiAngle) <= 1.0e-12
            || std::abs(aSemiAngle) >= 1.57079632679489661923)
          throw KernelFailure(ErrorCode::InvalidArgs, "Cone referenceRadius must be non-negative and semiAngle must be between -pi/2 and pi/2 excluding zero");
        aSurface = new Geom_ConicalSurface(anAxis, aSemiAngle, aReferenceRadius);
      }
      else if (aType == "torus")
      {
        const double aMajorRadius = requiredNumber(aDefinition, "majorRadius");
        const double aMinorRadius = requiredNumber(aDefinition, "minorRadius");
        if (aMajorRadius <= 0.0 || aMinorRadius <= 0.0 || aMajorRadius <= aMinorRadius)
          throw KernelFailure(ErrorCode::InvalidArgs, "Torus radii must be positive with majorRadius > minorRadius");
        aSurface = new Geom_ToroidalSurface(anAxis, aMajorRadius, aMinorRadius);
      }
      else
      {
        throw KernelFailure(ErrorCode::InvalidArgs, "Unknown analytic surface type");
      }

      const double aUFirst = requiredNumber(theArgs, "uFirst");
      const double aULast = requiredNumber(theArgs, "uLast");
      const double aVFirst = requiredNumber(theArgs, "vFirst");
      const double aVLast = requiredNumber(theArgs, "vLast");
      const double aTolerance = theArgs.value("tolerance", 1.0e-7);
      if (aUFirst >= aULast || aVFirst >= aVLast || aTolerance <= 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface bounds must be increasing and tolerance positive");
      BRepBuilderAPI_MakeFace aBuilder(aSurface, aUFirst, aULast, aVFirst, aVLast, aTolerance);
      if (!aBuilder.IsDone() || aBuilder.Face().IsNull()
          || !BRepCheck_Analyzer(aBuilder.Face(), true).IsValid())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct a valid bounded surface face");
      return {{"shape", theContext.arena().add(aBuilder.Face(), aScope)}};
    }

json h_surfaceControlData(KernelOperationContext& theContext, const json& theArgs)
{
      const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
      if (aShape.ShapeType() != TopAbs_FACE)
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface control data requires a face shape");
      const occ::handle<Geom_Surface> aSurface = BRep_Tool::Surface(TopoDS::Face(aShape));
      if (aSurface.IsNull())
        throw KernelFailure(ErrorCode::InvalidArgs, "Face has no geometric surface");
      json aResult{{"poles", json::array()}, {"weights", json::array()},
                   {"uKnots", json::array()}, {"vKnots", json::array()},
                   {"uMultiplicities", json::array()}, {"vMultiplicities", json::array()},
                   {"uPeriodic", false}, {"vPeriodic", false}};
      if (aSurface->IsKind(STANDARD_TYPE(Geom_BezierSurface)))
      {
        const occ::handle<Geom_BezierSurface> aBezier = occ::down_cast<Geom_BezierSurface>(aSurface);
        aResult["type"] = "bezier";
        aResult["uDegree"] = aBezier->UDegree();
        aResult["vDegree"] = aBezier->VDegree();
        for (int aU = 1; aU <= aBezier->NbUPoles(); ++aU)
        {
          json aPoleRow = json::array();
          json aWeightRow = json::array();
          for (int aV = 1; aV <= aBezier->NbVPoles(); ++aV)
          {
            const gp_Pnt aPole = aBezier->Pole(aU, aV);
            aPoleRow.push_back({aPole.X(), aPole.Y(), aPole.Z()});
            aWeightRow.push_back(aBezier->Weight(aU, aV));
          }
          aResult["poles"].push_back(aPoleRow);
          aResult["weights"].push_back(aWeightRow);
        }
        return aResult;
      }
      if (aSurface->IsKind(STANDARD_TYPE(Geom_BSplineSurface)))
      {
        const occ::handle<Geom_BSplineSurface> aBSpline = occ::down_cast<Geom_BSplineSurface>(aSurface);
        aResult["type"] = "bspline";
        aResult["uDegree"] = aBSpline->UDegree();
        aResult["vDegree"] = aBSpline->VDegree();
        aResult["uPeriodic"] = aBSpline->IsUPeriodic();
        aResult["vPeriodic"] = aBSpline->IsVPeriodic();
        for (int aU = 1; aU <= aBSpline->NbUPoles(); ++aU)
        {
          json aPoleRow = json::array();
          json aWeightRow = json::array();
          for (int aV = 1; aV <= aBSpline->NbVPoles(); ++aV)
          {
            const gp_Pnt aPole = aBSpline->Pole(aU, aV);
            aPoleRow.push_back({aPole.X(), aPole.Y(), aPole.Z()});
            aWeightRow.push_back(aBSpline->Weight(aU, aV));
          }
          aResult["poles"].push_back(aPoleRow);
          aResult["weights"].push_back(aWeightRow);
        }
        for (int anIndex = 1; anIndex <= aBSpline->NbUKnots(); ++anIndex)
        {
          aResult["uKnots"].push_back(aBSpline->UKnot(anIndex));
          aResult["uMultiplicities"].push_back(aBSpline->UMultiplicity(anIndex));
        }
        for (int anIndex = 1; anIndex <= aBSpline->NbVKnots(); ++anIndex)
        {
          aResult["vKnots"].push_back(aBSpline->VKnot(anIndex));
          aResult["vMultiplicities"].push_back(aBSpline->VMultiplicity(anIndex));
        }
        return aResult;
      }
      throw KernelFailure(ErrorCode::InvalidArgs, "Surface control data is available only for Bezier and BSpline faces");
    }

json h_editSurfaceBSpline(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
      if (aShape.ShapeType() != TopAbs_FACE)
        throw KernelFailure(ErrorCode::InvalidArgs, "BSpline surface editing requires a face shape");
      const TopoDS_Face aFace = TopoDS::Face(aShape);
      const occ::handle<Geom_Surface> aSurface = BRep_Tool::Surface(aFace);
      if (aSurface.IsNull() || !aSurface->IsKind(STANDARD_TYPE(Geom_BSplineSurface)))
        throw KernelFailure(ErrorCode::InvalidArgs, "BSpline surface editing requires a BSpline face");
      const occ::handle<Geom_BSplineSurface> aBSpline =
        occ::down_cast<Geom_BSplineSurface>(aSurface->Copy());
      const std::string anAction = theArgs.value("action", "");
      const double aTolerance = theArgs.value("tolerance", 1.0e-7);
      if (aTolerance <= 0.0 || !std::isfinite(aTolerance))
        throw KernelFailure(ErrorCode::InvalidArgs, "BSpline surface editing tolerance must be positive and finite");

      auto readArray = [&](const char* theKey) -> const json& {
        if (!theArgs.contains(theKey) || !theArgs.at(theKey).is_array() || theArgs.at(theKey).empty())
          throw KernelFailure(ErrorCode::InvalidArgs, std::string("BSpline surface ") + theKey + " must be a non-empty array");
        return theArgs.at(theKey);
      };
      auto insertKnots = [&](const char* theKnotsKey, const char* theMultiplicitiesKey,
                             const bool isU) {
        const json& aKnots = readArray(theKnotsKey);
        const json& aMultiplicities = readArray(theMultiplicitiesKey);
        if (aKnots.size() != aMultiplicities.size())
          throw KernelFailure(ErrorCode::InvalidArgs, "BSpline surface knots and multiplicities must have equal length");
        TColStd_Array1OfReal aKnotArray(1, static_cast<int>(aKnots.size()));
        TColStd_Array1OfInteger aMultiplicityArray(1, static_cast<int>(aMultiplicities.size()));
        for (int anIndex = 1; anIndex <= aKnotArray.Upper(); ++anIndex)
        {
          const double aKnot = aKnots.at(static_cast<std::size_t>(anIndex - 1)).get<double>();
          const int aMultiplicity = aMultiplicities.at(static_cast<std::size_t>(anIndex - 1)).get<int>();
          if (!std::isfinite(aKnot) || aMultiplicity <= 0)
            throw KernelFailure(ErrorCode::InvalidArgs, "BSpline surface knot and multiplicity must be valid");
          aKnotArray.SetValue(anIndex, aKnot);
          aMultiplicityArray.SetValue(anIndex, aMultiplicity);
        }
        if (isU)
          aBSpline->InsertUKnots(aKnotArray, aMultiplicityArray, aTolerance, theArgs.value("add", true));
        else
          aBSpline->InsertVKnots(aKnotArray, aMultiplicityArray, aTolerance, theArgs.value("add", true));
      };

      if (anAction == "insertUKnot")
      {
        const double aKnot = requiredNumber(theArgs, "uKnot");
        const int aMultiplicity = static_cast<int>(requiredU32(theArgs, "uMultiplicity"));
        if (!std::isfinite(aKnot) || aMultiplicity <= 0)
          throw KernelFailure(ErrorCode::InvalidArgs, "BSpline U knot and multiplicity must be valid");
        aBSpline->InsertUKnot(aKnot, aMultiplicity, aTolerance, theArgs.value("add", true));
      }
      else if (anAction == "insertVKnot")
      {
        const double aKnot = requiredNumber(theArgs, "vKnot");
        const int aMultiplicity = static_cast<int>(requiredU32(theArgs, "vMultiplicity"));
        if (!std::isfinite(aKnot) || aMultiplicity <= 0)
          throw KernelFailure(ErrorCode::InvalidArgs, "BSpline V knot and multiplicity must be valid");
        aBSpline->InsertVKnot(aKnot, aMultiplicity, aTolerance, theArgs.value("add", true));
      }
      else if (anAction == "insertUKnots")
      {
        insertKnots("uKnots", "uMultiplicities", true);
      }
      else if (anAction == "insertVKnots")
      {
        insertKnots("vKnots", "vMultiplicities", false);
      }
      else if (anAction == "removeUKnot" || anAction == "removeVKnot")
      {
        const char* anIndexKey = anAction == "removeUKnot" ? "uKnotIndex" : "vKnotIndex";
        const char* aMultiplicityKey = anAction == "removeUKnot" ? "uMultiplicity" : "vMultiplicity";
        const std::uint32_t aKnotIndex = requiredU32(theArgs, anIndexKey);
        const int aMultiplicity = static_cast<int>(requiredU32(theArgs, aMultiplicityKey));
        const int aKnotCount = anAction == "removeUKnot" ? aBSpline->NbUKnots() : aBSpline->NbVKnots();
        if (aKnotIndex >= static_cast<std::uint32_t>(aKnotCount))
          throw KernelFailure(ErrorCode::InvalidArgs, "BSpline surface knot index is out of range");
        const bool isRemoved = anAction == "removeUKnot"
          ? aBSpline->RemoveUKnot(static_cast<int>(aKnotIndex + 1), aMultiplicity, aTolerance)
          : aBSpline->RemoveVKnot(static_cast<int>(aKnotIndex + 1), aMultiplicity, aTolerance);
        if (!isRemoved)
          throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT could not remove the BSpline surface knot within tolerance");
      }
      else if (anAction == "increaseDegree")
      {
        const std::uint32_t aUDegree = requiredU32(theArgs, "uDegree");
        const std::uint32_t aVDegree = requiredU32(theArgs, "vDegree");
        if (aUDegree > static_cast<std::uint32_t>(Geom_BSplineSurface::MaxDegree())
            || aVDegree > static_cast<std::uint32_t>(Geom_BSplineSurface::MaxDegree()))
          throw KernelFailure(ErrorCode::InvalidArgs, "BSpline surface degree exceeds the OCCT maximum");
        aBSpline->IncreaseDegree(static_cast<int>(aUDegree), static_cast<int>(aVDegree));
      }
      else if (anAction == "setUPeriodic")
      {
        aBSpline->SetUPeriodic();
      }
      else if (anAction == "setVPeriodic")
      {
        aBSpline->SetVPeriodic();
      }
      else if (anAction == "setUNotPeriodic")
      {
        aBSpline->SetUNotPeriodic();
      }
      else if (anAction == "setVNotPeriodic")
      {
        aBSpline->SetVNotPeriodic();
      }
      else if (anAction == "exchangeUV")
      {
        aBSpline->ExchangeUV();
      }
      else if (anAction == "reverseU")
      {
        aBSpline->UReverse();
      }
      else if (anAction == "reverseV")
      {
        aBSpline->VReverse();
      }
      else if (anAction == "setControlNet")
      {
        const json& aPoles = readArray("poles");
        if (aPoles.size() != static_cast<std::size_t>(aBSpline->NbUPoles()))
          throw KernelFailure(ErrorCode::InvalidArgs, "BSpline surface pole grid U size does not match");
        const bool hasWeights = theArgs.contains("weights");
        if (hasWeights && (!theArgs.at("weights").is_array()
                           || theArgs.at("weights").size() != aPoles.size()))
          throw KernelFailure(ErrorCode::InvalidArgs, "BSpline surface weight grid U size does not match");
        for (int anUIndex = 1; anUIndex <= aBSpline->NbUPoles(); ++anUIndex)
        {
          const json& aPoleRow = aPoles.at(static_cast<std::size_t>(anUIndex - 1));
          if (!aPoleRow.is_array() || aPoleRow.size() != static_cast<std::size_t>(aBSpline->NbVPoles()))
            throw KernelFailure(ErrorCode::InvalidArgs, "BSpline surface pole grid V size does not match");
          const json* aWeightRow = nullptr;
          if (hasWeights)
          {
            aWeightRow = &theArgs.at("weights").at(static_cast<std::size_t>(anUIndex - 1));
            if (!aWeightRow->is_array() || aWeightRow->size() != aPoleRow.size())
              throw KernelFailure(ErrorCode::InvalidArgs, "BSpline surface weight grid V size does not match");
          }
          for (int anVIndex = 1; anVIndex <= aBSpline->NbVPoles(); ++anVIndex)
          {
            const json& aPoint = aPoleRow.at(static_cast<std::size_t>(anVIndex - 1));
            if (!aPoint.is_array() || aPoint.size() != 3 || !aPoint.at(0).is_number()
                || !aPoint.at(1).is_number() || !aPoint.at(2).is_number())
              throw KernelFailure(ErrorCode::InvalidArgs, "BSpline surface poles must be vec3 values");
            const gp_Pnt aPole(aPoint.at(0).get<double>(), aPoint.at(1).get<double>(), aPoint.at(2).get<double>());
            if (aWeightRow != nullptr)
            {
              const double aWeight = aWeightRow->at(static_cast<std::size_t>(anVIndex - 1)).get<double>();
              if (aWeight <= 0.0 || !std::isfinite(aWeight))
                throw KernelFailure(ErrorCode::InvalidArgs, "BSpline surface weights must be positive and finite");
              aBSpline->SetPole(anUIndex, anVIndex, aPole, aWeight);
            }
            else
            {
              aBSpline->SetPole(anUIndex, anVIndex, aPole);
            }
          }
        }
      }
      else
      {
        throw KernelFailure(ErrorCode::InvalidArgs, "Unknown BSpline surface edit action");
      }

      const TopoDS_Face anEditedFace = makeSurfaceFacePreservingWires(
        aFace, aBSpline, aTolerance, aFace.Orientation());
      return {{"shape", theContext.arena().add(anEditedFace, aScope)}};
    }

json h_reparameterizeSurface(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
      if (aShape.ShapeType() != TopAbs_FACE)
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "Surface reparameterization requires a face shape");
      const TopoDS_Face aFace = TopoDS::Face(aShape);
      const occ::handle<Geom_Surface> aSurface = BRep_Tool::Surface(aFace);
      if (aSurface.IsNull() || !aSurface->IsKind(STANDARD_TYPE(Geom_BSplineSurface)))
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "Surface reparameterization requires a BSpline face");
      const double aUFirst = requiredNumber(theArgs, "uFirst");
      const double aULast = requiredNumber(theArgs, "uLast");
      const double aVFirst = requiredNumber(theArgs, "vFirst");
      const double aVLast = requiredNumber(theArgs, "vLast");
      const double aTolerance = theArgs.value("tolerance", 1.0e-7);
      if (!std::isfinite(aUFirst) || !std::isfinite(aULast)
          || !std::isfinite(aVFirst) || !std::isfinite(aVLast)
          || aUFirst >= aULast || aVFirst >= aVLast
          || !std::isfinite(aTolerance) || aTolerance <= 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "Surface parameter domains must be finite and increasing");
      const occ::handle<Geom_BSplineSurface> aBSpline =
        occ::down_cast<Geom_BSplineSurface>(aSurface->Copy());
      TColStd_Array1OfReal aUKnots(1, aBSpline->NbUKnots());
      TColStd_Array1OfReal aVKnots(1, aBSpline->NbVKnots());
      for (int anIndex = 1; anIndex <= aBSpline->NbUKnots(); ++anIndex)
        aUKnots.SetValue(anIndex, aBSpline->UKnot(anIndex));
      for (int anIndex = 1; anIndex <= aBSpline->NbVKnots(); ++anIndex)
        aVKnots.SetValue(anIndex, aBSpline->VKnot(anIndex));
      BSplCLib::Reparametrize(aUFirst, aULast, aUKnots);
      BSplCLib::Reparametrize(aVFirst, aVLast, aVKnots);
      aBSpline->SetUKnots(aUKnots);
      aBSpline->SetVKnots(aVKnots);

      BRepBuilderAPI_Copy aCopy(aFace, true, false);
      if (!aCopy.IsDone() || aCopy.Shape().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed,
                            "OCCT failed to copy the surface boundary topology");
      const TopoDS_Face aCopiedFace = TopoDS::Face(aCopy.Shape());
      ShapeFix_Edge aFixer;
      for (TopExp_Explorer anExplorer(aCopiedFace, TopAbs_EDGE);
           anExplorer.More(); anExplorer.Next())
      {
        const TopoDS_Edge anEdge = TopoDS::Edge(anExplorer.Current());
        aFixer.FixAddPCurve(anEdge, aBSpline, TopLoc_Location(),
                            BRep_Tool::IsClosed(anEdge, aCopiedFace), aTolerance);
        double aFirst = 0.0;
        double aLast = 0.0;
        if (BRep_Tool::CurveOnSurface(
              anEdge, aBSpline, TopLoc_Location(), aFirst, aLast).IsNull())
          throw KernelFailure(ErrorCode::ConstructionFailed,
                              "OCCT failed to reparameterize a surface boundary pcurve");
        aFixer.FixSameParameter(anEdge, aTolerance);
      }
      const TopoDS_Wire anOuter = BRepTools::OuterWire(aCopiedFace);
      if (anOuter.IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed,
                            "Surface reparameterization requires an outer wire");
      BRepBuilderAPI_MakeFace aBuilder(aBSpline, anOuter, true);
      for (TopExp_Explorer anExplorer(aCopiedFace, TopAbs_WIRE);
           anExplorer.More(); anExplorer.Next())
      {
        const TopoDS_Wire aWire = TopoDS::Wire(anExplorer.Current());
        if (!aWire.IsSame(anOuter)) aBuilder.Add(aWire);
      }
      if (!aBuilder.IsDone() || aBuilder.Face().IsNull()
          || !BRepCheck_Analyzer(aBuilder.Face(), true).IsValid())
        throw KernelFailure(ErrorCode::ConstructionFailed,
                            "OCCT failed to construct the reparameterized surface face");
      TopoDS_Face aResult = aBuilder.Face();
      aResult.Orientation(aFace.Orientation());
      return {{"shape", theContext.arena().add(aResult, aScope)}};
    }

} // namespace occt_worker
