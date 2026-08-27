// Focused geometry/topology operation handlers.

#include "kernel_geometry_topology_operations.hpp"
#include "kernel_geometry_helpers.hpp"
#include "kernel_operation_context.hpp"
#include "kernel_protocol_helpers.hpp"

namespace occt_worker {

static json makeEdgeLineOrArc(KernelOperationContext& theContext,
                              const bool isArc,
                              const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      if (isArc && theArgs.contains("center"))
      {
        const auto aCenter = requiredVec3(theArgs, "center");
        const auto aNormal = requiredDirection(theArgs, "normal");
        const double aRadius = requiredNumber(theArgs, "radius");
        const double aStartParameter = requiredNumber(theArgs, "startAngle");
        const double anEndParameter = requiredNumber(theArgs, "endAngle");
        if (aRadius <= 0.0 || aStartParameter == anEndParameter)
          throw KernelFailure(ErrorCode::InvalidArgs, "Arc radius must be positive and angles must be distinct");
        gp_Ax2 anAxis(gp_Pnt(aCenter[0], aCenter[1], aCenter[2]),
                      gp_Dir(aNormal[0], aNormal[1], aNormal[2]));
        if (theArgs.contains("xDirection"))
        {
          const auto anXDirection = requiredDirection(theArgs, "xDirection");
          if (gp_Dir(aNormal[0], aNormal[1], aNormal[2]).IsParallel(
                gp_Dir(anXDirection[0], anXDirection[1], anXDirection[2]), 1.0e-12))
            throw KernelFailure(ErrorCode::InvalidArgs, "Arc normal and xDirection must not be parallel");
          anAxis = gp_Ax2(gp_Pnt(aCenter[0], aCenter[1], aCenter[2]),
                          gp_Dir(aNormal[0], aNormal[1], aNormal[2]),
                          gp_Dir(anXDirection[0], anXDirection[1], anXDirection[2]));
        }
        BRepBuilderAPI_MakeEdge aBuilder(gp_Circ(anAxis, aRadius), aStartParameter, anEndParameter);
        if (!aBuilder.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct parameterized arc edge");
        return {{"shape", theContext.arena().add(aBuilder.Edge(), aScope)}};
      }
      const auto aStart = requiredVec3(theArgs, "start");
      const auto anEnd = requiredVec3(theArgs, "end");
      const gp_Pnt aStartPoint(aStart[0], aStart[1], aStart[2]);
      const gp_Pnt anEndPoint(anEnd[0], anEnd[1], anEnd[2]);
      if (aStartPoint.Distance(anEndPoint) <= 1.0e-12)
        throw KernelFailure(ErrorCode::InvalidArgs, "Edge endpoints must be distinct");
      if (!isArc)
      {
        BRepBuilderAPI_MakeEdge aBuilder(aStartPoint, anEndPoint);
        if (!aBuilder.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct line edge");
        return {{"shape", theContext.arena().add(aBuilder.Edge(), aScope)}};
      }
      const auto aPoint = requiredVec3(theArgs, "point");
      GC_MakeArcOfCircle anArc(aStartPoint, gp_Pnt(aPoint[0], aPoint[1], aPoint[2]), anEndPoint);
      if (!anArc.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct arc");
      BRepBuilderAPI_MakeEdge aBuilder(anArc.Value());
      if (!aBuilder.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct arc edge");
      return {{"shape", theContext.arena().add(aBuilder.Edge(), aScope)}};
    }

json h_makeEdgeLine(KernelOperationContext& theContext, const json& theArgs)
{
  return makeEdgeLineOrArc(theContext, false, theArgs);
}

json h_makeEdgeArc(KernelOperationContext& theContext, const json& theArgs)
{
  return makeEdgeLineOrArc(theContext, true, theArgs);
}

static json makeEdgeCircleOrEllipse(KernelOperationContext& theContext,
                                    const bool isEllipse,
                                    const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const auto aCenter = optionalVec3(theArgs, "center", {0.0, 0.0, 0.0});
      const auto aNormal = optionalDirection(theArgs, "normal", {0.0, 0.0, 1.0});
      const gp_Ax2 anAxis(gp_Pnt(aCenter[0], aCenter[1], aCenter[2]), gp_Dir(aNormal[0], aNormal[1], aNormal[2]));
      if (!isEllipse)
      {
        const double aRadius = requiredNumber(theArgs, "radius");
        if (aRadius <= 0.0) throw KernelFailure(ErrorCode::InvalidArgs, "Circle radius must be positive");
        BRepBuilderAPI_MakeEdge aBuilder(gp_Circ(anAxis, aRadius));
        if (!aBuilder.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct circle edge");
        return {{"shape", theContext.arena().add(aBuilder.Edge(), aScope)}};
      }
      const double aMajor = requiredNumber(theArgs, "majorRadius");
      const double aMinor = requiredNumber(theArgs, "minorRadius");
      if (aMajor <= 0.0 || aMinor <= 0.0 || aMajor < aMinor)
        throw KernelFailure(ErrorCode::InvalidArgs, "Ellipse radii must be positive with majorRadius >= minorRadius");
      BRepBuilderAPI_MakeEdge aBuilder(gp_Elips(anAxis, aMajor, aMinor));
      if (!aBuilder.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct ellipse edge");
      return {{"shape", theContext.arena().add(aBuilder.Edge(), aScope)}};
    }

json h_makeEdgeCircle(KernelOperationContext& theContext, const json& theArgs)
{
  return makeEdgeCircleOrEllipse(theContext, false, theArgs);
}

json h_makeEdgeEllipse(KernelOperationContext& theContext, const json& theArgs)
{
  return makeEdgeCircleOrEllipse(theContext, true, theArgs);
}

static json makeEdgeHyperbolaOrParabola(KernelOperationContext& theContext,
                                        const bool isParabola,
                                        const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const auto aCenter = optionalVec3(theArgs, "center", {0.0, 0.0, 0.0});
      const auto aNormal = optionalDirection(theArgs, "normal", {0.0, 0.0, 1.0});
      gp_Ax2 anAxis(gp_Pnt(aCenter[0], aCenter[1], aCenter[2]),
                    gp_Dir(aNormal[0], aNormal[1], aNormal[2]));
      if (theArgs.contains("xDirection"))
      {
        const auto anXDirection = requiredDirection(theArgs, "xDirection");
        if (gp_Dir(aNormal[0], aNormal[1], aNormal[2]).IsParallel(
              gp_Dir(anXDirection[0], anXDirection[1], anXDirection[2]), 1.0e-12))
          throw KernelFailure(ErrorCode::InvalidArgs,
                              "Conic normal and xDirection must not be parallel");
        anAxis = gp_Ax2(gp_Pnt(aCenter[0], aCenter[1], aCenter[2]),
                        gp_Dir(aNormal[0], aNormal[1], aNormal[2]),
                        gp_Dir(anXDirection[0], anXDirection[1], anXDirection[2]));
      }
      const double aFirst = requiredNumber(theArgs, "firstParameter");
      const double aLast = requiredNumber(theArgs, "lastParameter");
      if (aFirst >= aLast)
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "Conic parameters must be finite and increasing");
      if (!isParabola)
      {
        const double aMajorRadius = requiredNumber(theArgs, "majorRadius");
        const double aMinorRadius = requiredNumber(theArgs, "minorRadius");
        if (aMajorRadius <= 0.0 || aMinorRadius <= 0.0)
          throw KernelFailure(ErrorCode::InvalidArgs,
                              "Hyperbola radii must be positive");
        BRepBuilderAPI_MakeEdge aBuilder(
          gp_Hypr(anAxis, aMajorRadius, aMinorRadius), aFirst, aLast);
        if (!aBuilder.IsDone())
          throw KernelFailure(ErrorCode::ConstructionFailed,
                              "OCCT failed to construct hyperbola edge");
        return {{"shape", theContext.arena().add(aBuilder.Edge(), aScope)}};
      }
      const double aFocal = requiredNumber(theArgs, "focal");
      if (aFocal <= 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs, "Parabola focal length must be positive");
      BRepBuilderAPI_MakeEdge aBuilder(gp_Parab(anAxis, aFocal), aFirst, aLast);
      if (!aBuilder.IsDone())
        throw KernelFailure(ErrorCode::ConstructionFailed,
                            "OCCT failed to construct parabola edge");
      return {{"shape", theContext.arena().add(aBuilder.Edge(), aScope)}};
    }

json h_makeEdgeHyperbola(KernelOperationContext& theContext, const json& theArgs)
{
  return makeEdgeHyperbolaOrParabola(theContext, false, theArgs);
}

json h_makeEdgeParabola(KernelOperationContext& theContext, const json& theArgs)
{
  return makeEdgeHyperbolaOrParabola(theContext, true, theArgs);
}

json h_makeEdgeOffset(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
      double aFirst = 0.0;
      double aLast = 0.0;
      const occ::handle<Geom_Curve> aCurve = curveForEdge(aShape, aFirst, aLast);
      const double anOffset = requiredNumber(theArgs, "offset");
      const auto aDirection = requiredDirection(theArgs, "direction");
      const occ::handle<Geom_OffsetCurve> anOffsetCurve = new Geom_OffsetCurve(
        aCurve, anOffset, gp_Dir(aDirection[0], aDirection[1], aDirection[2]));
      BRepBuilderAPI_MakeEdge aBuilder(anOffsetCurve, aFirst, aLast);
      if (!aBuilder.IsDone())
        throw KernelFailure(ErrorCode::ConstructionFailed,
                            "OCCT failed to construct offset curve edge");
      return {{"shape", theContext.arena().add(aBuilder.Edge(), aScope)}};
    }

namespace {
std::vector<gp_Pnt> readCurvePoles(KernelOperationContext& theContext, const json& theArgs)
{
      if (!theArgs.contains("poles"))
        throw KernelFailure(ErrorCode::InvalidArgs, "Bezier/BSpline poles are required");
      std::vector<gp_Pnt> aPointValues;
      if (theArgs.at("poles").is_array())
      {
        for (const json& aPointValue : theArgs.at("poles"))
        {
          if (!aPointValue.is_array() || aPointValue.size() != 3 || !aPointValue[0].is_number()
              || !aPointValue[1].is_number() || !aPointValue[2].is_number())
            throw KernelFailure(ErrorCode::InvalidArgs, "Curve poles must be vec3");
          aPointValues.emplace_back(aPointValue[0].get<double>(), aPointValue[1].get<double>(), aPointValue[2].get<double>());
        }
      }
      else if (theArgs.at("poles").is_object())
      {
        const std::string aPoleBytes = inputBufferData(theContext.buffers(), theArgs, "poles");
        if (aPoleBytes.size() % (3 * sizeof(double)) != 0)
          throw KernelFailure(ErrorCode::InvalidArgs, "Curve pole buffer must use the f64x3 layout");
        for (std::size_t anOffset = 0; anOffset < aPoleBytes.size(); anOffset += 3 * sizeof(double))
        {
          std::array<double, 3> aPoint{};
          std::memcpy(aPoint.data(), aPoleBytes.data() + anOffset, 3 * sizeof(double));
          aPointValues.emplace_back(aPoint[0], aPoint[1], aPoint[2]);
        }
      }
      else
      {
        throw KernelFailure(ErrorCode::InvalidArgs, "Curve poles must be an array or f64x3 buffer reference");
      }
      if (aPointValues.size() < 2)
        throw KernelFailure(ErrorCode::InvalidArgs, "Bezier/BSpline poles require at least two points");
      return aPointValues;
}
} // namespace

json h_makeEdgeBezier(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const std::vector<gp_Pnt> aPointValues = readCurvePoles(theContext, theArgs);
      TColgp_Array1OfPnt aPoles(1, static_cast<int>(aPointValues.size()));
      for (int anIndex = 1; anIndex <= aPoles.Upper(); ++anIndex)
        aPoles.SetValue(anIndex, aPointValues[static_cast<std::size_t>(anIndex - 1)]);
      occ::handle<Geom_Curve> aCurve;
      if (theArgs.contains("weights"))
      {
        if (!theArgs.at("weights").is_array() || theArgs.at("weights").size() != aPointValues.size())
          throw KernelFailure(ErrorCode::InvalidArgs, "Bezier weights must match the pole count");
        TColStd_Array1OfReal aWeights(1, aPoles.Length());
        for (int aWeightIndex = 1; aWeightIndex <= aWeights.Upper(); ++aWeightIndex)
        {
          const json& aWeightValue = theArgs.at("weights").at(aWeightIndex - 1);
          if (!aWeightValue.is_number())
            throw KernelFailure(ErrorCode::InvalidArgs, "Bezier weights must be positive and finite");
          const double aWeight = aWeightValue.get<double>();
          if (aWeight <= 0.0 || !std::isfinite(aWeight))
            throw KernelFailure(ErrorCode::InvalidArgs, "Bezier weights must be positive and finite");
          aWeights.SetValue(aWeightIndex, aWeight);
        }
        aCurve = new Geom_BezierCurve(aPoles, aWeights);
      }
      else
      {
        aCurve = new Geom_BezierCurve(aPoles);
      }
      BRepBuilderAPI_MakeEdge anEdge(aCurve);
      if (!anEdge.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct curve edge");
      return {{"shape", theContext.arena().add(anEdge.Edge(), aScope)}};
}

json h_makeEdgeBSpline(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const std::vector<gp_Pnt> aPointValues = readCurvePoles(theContext, theArgs);
      TColgp_Array1OfPnt aPoles(1, static_cast<int>(aPointValues.size()));
      for (int anIndex = 1; anIndex <= aPoles.Upper(); ++anIndex)
        aPoles.SetValue(anIndex, aPointValues[static_cast<std::size_t>(anIndex - 1)]);
      occ::handle<Geom_Curve> aCurve;
      const std::string aMode = theArgs.value("mode", "interpolate");
      if (aMode != "interpolate" && aMode != "controlPoints")
        throw KernelFailure(ErrorCode::InvalidArgs, "BSpline mode must be interpolate or controlPoints");
      if (aMode == "controlPoints")
      {
        const int aDegree = theArgs.value("degree", std::min(3, aPoles.Length() - 1));
        if (aDegree < 1 || aDegree >= aPoles.Length())
          throw KernelFailure(ErrorCode::InvalidArgs, "BSpline degree must be between 1 and poleCount - 1");
        const bool isPeriodic = theArgs.value("periodic", false);
        if (isPeriodic && (!theArgs.contains("knots") || !theArgs.contains("multiplicities")))
          throw KernelFailure(ErrorCode::InvalidArgs, "Periodic control-point BSplines require explicit knots and multiplicities");
        const int aKnotCount = theArgs.contains("knots")
          ? static_cast<int>(theArgs.at("knots").size()) : aPoles.Length() - aDegree + 1;
        if (aKnotCount < 2 || (theArgs.contains("multiplicities")
            && static_cast<int>(theArgs.at("multiplicities").size()) != aKnotCount))
          throw KernelFailure(ErrorCode::InvalidArgs, "BSpline knots and multiplicities must have equal length of at least two");
        TColStd_Array1OfReal aKnots(1, aKnotCount);
        TColStd_Array1OfInteger aMultiplicities(1, aKnotCount);
        for (int aKnotIndex = 1; aKnotIndex <= aKnotCount; ++aKnotIndex)
        {
          aKnots.SetValue(aKnotIndex, theArgs.contains("knots")
            ? theArgs.at("knots").at(aKnotIndex - 1).get<double>() : static_cast<double>(aKnotIndex - 1));
          aMultiplicities.SetValue(aKnotIndex, theArgs.contains("multiplicities")
            ? theArgs.at("multiplicities").at(aKnotIndex - 1).get<int>()
            : (aKnotIndex == 1 || aKnotIndex == aKnotCount ? aDegree + 1 : 1));
        }
        if (theArgs.contains("weights"))
        {
          if (!theArgs.at("weights").is_array() || theArgs.at("weights").size() != aPointValues.size())
            throw KernelFailure(ErrorCode::InvalidArgs, "BSpline weights must match the pole count");
          TColStd_Array1OfReal aWeights(1, aPoles.Length());
          for (int aWeightIndex = 1; aWeightIndex <= aWeights.Upper(); ++aWeightIndex)
            aWeights.SetValue(aWeightIndex, theArgs.at("weights").at(aWeightIndex - 1).get<double>());
          aCurve = new Geom_BSplineCurve(aPoles, aWeights, aKnots, aMultiplicities, aDegree, isPeriodic);
        }
        else
        {
          aCurve = new Geom_BSplineCurve(aPoles, aKnots, aMultiplicities, aDegree, isPeriodic);
        }
      }
      else
      {
        const occ::handle<TColgp_HArray1OfPnt> aInterpolationPoints = new TColgp_HArray1OfPnt(1, aPoles.Length());
        for (int aPointIndex = 1; aPointIndex <= aPoles.Length(); ++aPointIndex)
          aInterpolationPoints->SetValue(aPointIndex, aPoles.Value(aPointIndex));
        GeomAPI_Interpolate anInterpolation(aInterpolationPoints,
                                            theArgs.value("periodic", false),
                                            theArgs.value("tolerance", 1.0e-7));
        anInterpolation.Perform();
        if (!anInterpolation.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to interpolate BSpline");
        aCurve = anInterpolation.Curve();
      }
      BRepBuilderAPI_MakeEdge anEdge(aCurve);
      if (!anEdge.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct curve edge");
      return {{"shape", theContext.arena().add(anEdge.Edge(), aScope)}};
}

json h_makeEdgeHelix(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const double aRadius = requiredNumber(theArgs, "radius");
      const double aPitch = requiredNumber(theArgs, "pitch");
      const double aTurns = requiredNumber(theArgs, "turns");
      if (aRadius <= 0.0 || aPitch == 0.0 || aTurns <= 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs, "Helix radius and turns must be positive and pitch non-zero");
      const auto anOrigin = optionalVec3(theArgs, "origin", {0.0, 0.0, 0.0});
      const auto aDirection = optionalDirection(theArgs, "direction", {0.0, 0.0, 1.0});
      const gp_Ax2 anAxis(gp_Pnt(anOrigin[0], anOrigin[1], anOrigin[2]),
                          gp_Dir(aDirection[0], aDirection[1], aDirection[2]));
      const double aPi = 3.14159265358979323846;
      const occ::handle<Geom_Surface> aSurface =
        new Geom_CylindricalSurface(gp_Ax3(anAxis), aRadius);
      const std::string aHandedness = theArgs.value("handedness", "right");
      if (aHandedness != "right" && aHandedness != "left")
        throw KernelFailure(ErrorCode::InvalidArgs, "Helix handedness must be right or left");
      const double aSign = aHandedness == "right" ? 1.0 : -1.0;
      const double aSlope = aPitch / (2.0 * aPi);
      const gp_Dir2d aLineDirection(aSign, aSlope);
      const occ::handle<Geom2d_Line> aCurve =
        new Geom2d_Line(gp_Pnt2d(0.0, 0.0), aLineDirection);
      const double aEndU = aSign * 2.0 * aPi * aTurns;
      const double aEndParameter = aEndU / aLineDirection.X();
      BRepBuilderAPI_MakeEdge anEdge(aCurve, aSurface, 0.0, aEndParameter);
      if (!anEdge.IsDone())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct helix edge");
      if (!BRepLib::BuildCurves3d(anEdge.Edge()))
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to build the helix 3D curve");
      return {{"shape", theContext.arena().add(anEdge.Edge(), aScope)}};
}

json h_curveControlData(KernelOperationContext& theContext, const json& theArgs)
{
      const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
      if (aShape.ShapeType() != TopAbs_EDGE)
        throw KernelFailure(ErrorCode::InvalidArgs, "Curve control data requires an edge shape");
      BRepAdaptor_Curve anAdaptor(TopoDS::Edge(aShape));
      json aResult{{"poles", json::array()}, {"weights", json::array()},
                   {"knots", json::array()}, {"multiplicities", json::array()},
                   {"periodic", false}};
      if (anAdaptor.GetType() == GeomAbs_BezierCurve)
      {
        const occ::handle<Geom_BezierCurve> aBezier = anAdaptor.Bezier();
        aResult["type"] = "bezier";
        aResult["degree"] = aBezier->Degree();
        for (int anIndex = 1; anIndex <= aBezier->NbPoles(); ++anIndex)
        {
          const gp_Pnt aPole = aBezier->Pole(anIndex);
          aResult["poles"].push_back({aPole.X(), aPole.Y(), aPole.Z()});
          aResult["weights"].push_back(aBezier->Weight(anIndex));
        }
        return aResult;
      }
      if (anAdaptor.GetType() == GeomAbs_BSplineCurve)
      {
        const occ::handle<Geom_BSplineCurve> aBSpline = anAdaptor.BSpline();
        aResult["type"] = "bspline";
        aResult["degree"] = aBSpline->Degree();
        aResult["periodic"] = aBSpline->IsPeriodic();
        for (int anIndex = 1; anIndex <= aBSpline->NbPoles(); ++anIndex)
        {
          const gp_Pnt aPole = aBSpline->Pole(anIndex);
          aResult["poles"].push_back({aPole.X(), aPole.Y(), aPole.Z()});
          aResult["weights"].push_back(aBSpline->Weight(anIndex));
        }
        for (int anIndex = 1; anIndex <= aBSpline->NbKnots(); ++anIndex)
        {
          aResult["knots"].push_back(aBSpline->Knot(anIndex));
          aResult["multiplicities"].push_back(aBSpline->Multiplicity(anIndex));
        }
        return aResult;
      }
      throw KernelFailure(ErrorCode::InvalidArgs, "Curve control data is available only for Bezier and BSpline edges");
    }

json h_updateCurvePole(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
      if (aShape.ShapeType() != TopAbs_EDGE)
        throw KernelFailure(ErrorCode::InvalidArgs, "Curve pole editing requires an edge shape");
      const TopoDS_Edge aEdge = TopoDS::Edge(aShape);
      BRepAdaptor_Curve anAdaptor(aEdge);
      const std::uint32_t anIndex = requiredU32(theArgs, "index");
      const auto aPoint = requiredVec3(theArgs, "point");
      const gp_Pnt aPole(aPoint[0], aPoint[1], aPoint[2]);
      occ::handle<Geom_Curve> anEditedCurve;
      if (anAdaptor.GetType() == GeomAbs_BezierCurve)
      {
        const occ::handle<Geom_BezierCurve> aBezier =
          occ::down_cast<Geom_BezierCurve>(anAdaptor.Bezier()->Copy());
        if (anIndex >= static_cast<std::uint32_t>(aBezier->NbPoles()))
          throw KernelFailure(ErrorCode::InvalidArgs, "Curve pole index is out of range");
        if (theArgs.contains("weight"))
        {
          const double aWeight = requiredNumber(theArgs, "weight");
          if (aWeight <= 0.0) throw KernelFailure(ErrorCode::InvalidArgs, "Curve pole weight must be positive");
          aBezier->SetPole(static_cast<int>(anIndex + 1), aPole, aWeight);
        }
        else
        {
          aBezier->SetPole(static_cast<int>(anIndex + 1), aPole);
        }
        anEditedCurve = aBezier;
      }
      else if (anAdaptor.GetType() == GeomAbs_BSplineCurve)
      {
        const occ::handle<Geom_BSplineCurve> aBSpline =
          occ::down_cast<Geom_BSplineCurve>(anAdaptor.BSpline()->Copy());
        if (anIndex >= static_cast<std::uint32_t>(aBSpline->NbPoles()))
          throw KernelFailure(ErrorCode::InvalidArgs, "Curve pole index is out of range");
        if (theArgs.contains("weight"))
        {
          const double aWeight = requiredNumber(theArgs, "weight");
          if (aWeight <= 0.0) throw KernelFailure(ErrorCode::InvalidArgs, "Curve pole weight must be positive");
          aBSpline->SetPole(static_cast<int>(anIndex + 1), aPole, aWeight);
        }
        else
        {
          aBSpline->SetPole(static_cast<int>(anIndex + 1), aPole);
        }
        anEditedCurve = aBSpline;
      }
      else
      {
        throw KernelFailure(ErrorCode::InvalidArgs, "Curve pole editing is available only for Bezier and BSpline edges");
      }
      BRepBuilderAPI_MakeEdge aBuilder(
        anEditedCurve, anAdaptor.FirstParameter(), anAdaptor.LastParameter());
      if (!aBuilder.IsDone() || aBuilder.Edge().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct the edited curve edge");
      TopoDS_Edge anEditedEdge = aBuilder.Edge();
      anEditedEdge.Orientation(aEdge.Orientation());
      return {{"shape", theContext.arena().add(anEditedEdge, aScope)}};
    }

json h_editCurveBSpline(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
      if (aShape.ShapeType() != TopAbs_EDGE)
        throw KernelFailure(ErrorCode::InvalidArgs, "BSpline curve editing requires an edge shape");
      const TopoDS_Edge aEdge = TopoDS::Edge(aShape);
      BRepAdaptor_Curve anAdaptor(aEdge);
      if (anAdaptor.GetType() != GeomAbs_BSplineCurve)
        throw KernelFailure(ErrorCode::InvalidArgs, "BSpline curve editing requires a BSpline edge");
      const occ::handle<Geom_BSplineCurve> aBSpline =
        occ::down_cast<Geom_BSplineCurve>(anAdaptor.BSpline()->Copy());
      const std::string anAction = theArgs.value("action", "");
      const double aTolerance = theArgs.value("tolerance", 1.0e-7);
      if (aTolerance <= 0.0 || !std::isfinite(aTolerance))
        throw KernelFailure(ErrorCode::InvalidArgs, "BSpline curve editing tolerance must be positive and finite");

      auto readArray = [&](const char* theKey) {
        if (!theArgs.contains(theKey) || !theArgs.at(theKey).is_array() || theArgs.at(theKey).empty())
          throw KernelFailure(ErrorCode::InvalidArgs, std::string("BSpline curve ") + theKey + " must be a non-empty array");
        return theArgs.at(theKey);
      };
      if (anAction == "insertKnot")
      {
        const double aKnot = requiredNumber(theArgs, "knot");
        const int aMultiplicity = theArgs.value("multiplicity", 1);
        if (!std::isfinite(aKnot) || aMultiplicity <= 0)
          throw KernelFailure(ErrorCode::InvalidArgs, "BSpline knot and multiplicity must be valid");
        aBSpline->InsertKnot(aKnot, aMultiplicity, aTolerance, theArgs.value("add", true));
      }
      else if (anAction == "insertKnots")
      {
        const json& aKnots = readArray("knots");
        const json& aMultiplicities = readArray("multiplicities");
        if (aKnots.size() != aMultiplicities.size())
          throw KernelFailure(ErrorCode::InvalidArgs, "BSpline knots and multiplicities must have equal length");
        TColStd_Array1OfReal aKnotArray(1, static_cast<int>(aKnots.size()));
        TColStd_Array1OfInteger aMultiplicityArray(1, static_cast<int>(aMultiplicities.size()));
        for (int anIndex = 1; anIndex <= aKnotArray.Upper(); ++anIndex)
        {
          const double aKnot = aKnots.at(static_cast<std::size_t>(anIndex - 1)).get<double>();
          const int aMultiplicity = aMultiplicities.at(static_cast<std::size_t>(anIndex - 1)).get<int>();
          if (!std::isfinite(aKnot) || aMultiplicity <= 0)
            throw KernelFailure(ErrorCode::InvalidArgs, "BSpline knot and multiplicity must be valid");
          aKnotArray.SetValue(anIndex, aKnot);
          aMultiplicityArray.SetValue(anIndex, aMultiplicity);
        }
        aBSpline->InsertKnots(aKnotArray, aMultiplicityArray, aTolerance, theArgs.value("add", false));
      }
      else if (anAction == "removeKnot")
      {
        const std::uint32_t aKnotIndex = requiredU32(theArgs, "knotIndex");
        const int aMultiplicity = static_cast<int>(requiredU32(theArgs, "multiplicity"));
        if (aKnotIndex >= static_cast<std::uint32_t>(aBSpline->NbKnots()) || aMultiplicity < 0)
          throw KernelFailure(ErrorCode::InvalidArgs, "BSpline knot index or multiplicity is out of range");
        if (!aBSpline->RemoveKnot(static_cast<int>(aKnotIndex + 1), aMultiplicity, aTolerance))
          throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT could not remove the BSpline knot within tolerance");
      }
      else if (anAction == "increaseDegree")
      {
        const std::uint32_t aDegree = requiredU32(theArgs, "degree");
        if (aDegree > static_cast<std::uint32_t>(Geom_BSplineCurve::MaxDegree()))
          throw KernelFailure(ErrorCode::InvalidArgs, "BSpline curve degree exceeds the OCCT maximum");
        aBSpline->IncreaseDegree(static_cast<int>(aDegree));
      }
      else if (anAction == "setPeriodic")
      {
        aBSpline->SetPeriodic();
      }
      else if (anAction == "setNotPeriodic")
      {
        aBSpline->SetNotPeriodic();
      }
      else if (anAction == "setControlNet")
      {
        const json& aPoles = readArray("poles");
        if (aPoles.size() != static_cast<std::size_t>(aBSpline->NbPoles()))
          throw KernelFailure(ErrorCode::InvalidArgs, "BSpline curve poles must match the control net size");
        const bool hasWeights = theArgs.contains("weights");
        if (hasWeights && (!theArgs.at("weights").is_array()
                           || theArgs.at("weights").size() != aPoles.size()))
          throw KernelFailure(ErrorCode::InvalidArgs, "BSpline curve weights must match the pole count");
        for (int anIndex = 1; anIndex <= aBSpline->NbPoles(); ++anIndex)
        {
          const json& aPoint = aPoles.at(static_cast<std::size_t>(anIndex - 1));
          if (!aPoint.is_array() || aPoint.size() != 3 || !aPoint.at(0).is_number()
              || !aPoint.at(1).is_number() || !aPoint.at(2).is_number())
            throw KernelFailure(ErrorCode::InvalidArgs, "BSpline curve poles must be vec3 values");
          const gp_Pnt aPole(aPoint.at(0).get<double>(), aPoint.at(1).get<double>(), aPoint.at(2).get<double>());
          if (hasWeights)
          {
            const double aWeight = theArgs.at("weights").at(static_cast<std::size_t>(anIndex - 1)).get<double>();
            if (aWeight <= 0.0 || !std::isfinite(aWeight))
              throw KernelFailure(ErrorCode::InvalidArgs, "BSpline curve weights must be positive and finite");
            aBSpline->SetPole(anIndex, aPole, aWeight);
          }
          else
          {
            aBSpline->SetPole(anIndex, aPole);
          }
        }
      }
      else
      {
        throw KernelFailure(ErrorCode::InvalidArgs, "Unknown BSpline curve edit action");
      }

      BRepBuilderAPI_MakeEdge aBuilder(aBSpline, aBSpline->FirstParameter(), aBSpline->LastParameter());
      if (!aBuilder.IsDone() || aBuilder.Edge().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct the edited BSpline curve edge");
      TopoDS_Edge anEditedEdge = aBuilder.Edge();
      anEditedEdge.Orientation(aEdge.Orientation());
      return {{"shape", theContext.arena().add(anEditedEdge, aScope)}};
    }

json h_reparameterizeCurve(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
      if (aShape.ShapeType() != TopAbs_EDGE)
        throw KernelFailure(ErrorCode::InvalidArgs, "Curve reparameterization requires an edge shape");
      const TopoDS_Edge aEdge = TopoDS::Edge(aShape);
      BRepAdaptor_Curve anAdaptor(aEdge);
      if (anAdaptor.GetType() != GeomAbs_BSplineCurve)
        throw KernelFailure(ErrorCode::InvalidArgs, "Curve reparameterization requires a BSpline edge");
      const double aFirst = requiredNumber(theArgs, "first");
      const double aLast = requiredNumber(theArgs, "last");
      if (!std::isfinite(aFirst) || !std::isfinite(aLast) || aFirst >= aLast)
        throw KernelFailure(ErrorCode::InvalidArgs, "Curve parameter domain must be finite and increasing");
      const occ::handle<Geom_BSplineCurve> aBSpline =
        occ::down_cast<Geom_BSplineCurve>(anAdaptor.BSpline()->Copy());
      TColStd_Array1OfReal aKnots(1, aBSpline->NbKnots());
      for (int anIndex = 1; anIndex <= aBSpline->NbKnots(); ++anIndex)
        aKnots.SetValue(anIndex, aBSpline->Knot(anIndex));
      BSplCLib::Reparametrize(aFirst, aLast, aKnots);
      aBSpline->SetKnots(aKnots);
      BRepBuilderAPI_MakeEdge aBuilder(aBSpline, aFirst, aLast);
      if (!aBuilder.IsDone() || aBuilder.Edge().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed,
                            "OCCT failed to construct the reparameterized curve edge");
      TopoDS_Edge aResult = aBuilder.Edge();
      aResult.Orientation(aEdge.Orientation());
      return {{"shape", theContext.arena().add(aResult, aScope)}};
    }

json h_trimCurve(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      double aFirst = 0.0;
      double aLast = 0.0;
      const occ::handle<Geom_Curve> aCurve = curveForEdge(
        theContext.arena().get(requiredU32(theArgs, "shape")), aFirst, aLast);
      const double aTrimFirst = requiredNumber(theArgs, "first");
      const double aTrimLast = requiredNumber(theArgs, "last");
      if (aTrimFirst < aFirst || aTrimLast > aLast || aTrimFirst >= aTrimLast)
        throw KernelFailure(ErrorCode::InvalidArgs, "Trim parameters must define an increasing subset of the edge domain");
      BRepBuilderAPI_MakeEdge aBuilder(aCurve, aTrimFirst, aTrimLast);
      if (!aBuilder.IsDone() || aBuilder.Edge().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to trim the curve");
      return {{"shape", theContext.arena().add(aBuilder.Edge(), aScope)}};
}

json h_convertCurveToBSpline(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      double aFirst = 0.0;
      double aLast = 0.0;
      const occ::handle<Geom_Curve> aCurve = curveForEdge(
        theContext.arena().get(requiredU32(theArgs, "shape")), aFirst, aLast);
      Convert_ParameterisationType aParameterization = Convert_TgtThetaOver2;
      const std::string aParameterizationName = theArgs.value("parameterization", "tgtThetaOver2");
      if (aParameterizationName == "quasiAngular") aParameterization = Convert_QuasiAngular;
      else if (aParameterizationName == "rationalC1") aParameterization = Convert_RationalC1;
      else if (aParameterizationName == "polynomial") aParameterization = Convert_Polynomial;
      else if (aParameterizationName != "tgtThetaOver2")
        throw KernelFailure(ErrorCode::InvalidArgs, "Unknown curve conversion parameterization");
      const occ::handle<Geom_TrimmedCurve> aTrimmed = new Geom_TrimmedCurve(aCurve, aFirst, aLast);
      const occ::handle<Geom_BSplineCurve> aBSpline =
        GeomConvert::CurveToBSplineCurve(aTrimmed, aParameterization);
      if (aBSpline.IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to convert the curve to BSpline");
      BRepBuilderAPI_MakeEdge aBuilder(aBSpline);
      if (!aBuilder.IsDone() || aBuilder.Edge().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct the converted BSpline edge");
      return {{"shape", theContext.arena().add(aBuilder.Edge(), aScope)}};
}

} // namespace occt_worker
