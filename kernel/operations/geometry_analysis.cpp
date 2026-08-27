// Focused geometry/topology operation handlers.

#include "kernel_geometry_topology_operations.hpp"
#include "kernel_geometry_helpers.hpp"
#include "kernel_operation_context.hpp"
#include "kernel_protocol_helpers.hpp"

namespace occt_worker {

namespace {
enum class CurveQueryMode { Continuity, Domain, Geometry, Evaluate };

json queryCurve(KernelOperationContext& theContext, const json& theArgs,
                CurveQueryMode theMode)
{
      const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
      if (aShape.ShapeType() != TopAbs_EDGE)
        throw KernelFailure(ErrorCode::InvalidArgs, "Curve evaluation requires an edge shape");
      BRepAdaptor_Curve anAdaptor(TopoDS::Edge(aShape));
      const bool isPeriodic = anAdaptor.IsPeriodic();
      if (theMode == CurveQueryMode::Continuity)
        return {{"continuity", continuityName(anAdaptor.Continuity())}};
      if (theMode == CurveQueryMode::Geometry)
      {
        json aResult;
        auto setAxis = [&](const gp_Ax2& theAxis) {
          const gp_Pnt& aCenter = theAxis.Location();
          const gp_Dir& aNormal = theAxis.Direction();
          const gp_Dir& anXDirection = theAxis.XDirection();
          aResult["center"] = {aCenter.X(), aCenter.Y(), aCenter.Z()};
          aResult["normal"] = {aNormal.X(), aNormal.Y(), aNormal.Z()};
          aResult["xDirection"] = {anXDirection.X(), anXDirection.Y(), anXDirection.Z()};
        };
        switch (anAdaptor.GetType())
        {
          case GeomAbs_Line:
          {
            const gp_Lin aLine = anAdaptor.Line();
            const gp_Pnt& anOrigin = aLine.Location();
            const gp_Dir& aDirection = aLine.Direction();
            aResult = {{"type", "line"},
                       {"origin", {anOrigin.X(), anOrigin.Y(), anOrigin.Z()}},
                       {"direction", {aDirection.X(), aDirection.Y(), aDirection.Z()}}};
            break;
          }
          case GeomAbs_Circle:
          {
            const gp_Circ aCircle = anAdaptor.Circle();
            aResult["type"] = "circle";
            setAxis(aCircle.Position());
            aResult["radius"] = aCircle.Radius();
            break;
          }
          case GeomAbs_Ellipse:
          {
            const gp_Elips anEllipse = anAdaptor.Ellipse();
            aResult["type"] = "ellipse";
            setAxis(anEllipse.Position());
            aResult["majorRadius"] = anEllipse.MajorRadius();
            aResult["minorRadius"] = anEllipse.MinorRadius();
            break;
          }
          case GeomAbs_Hyperbola:
          {
            const gp_Hypr aHyperbola = anAdaptor.Hyperbola();
            aResult["type"] = "hyperbola";
            setAxis(aHyperbola.Position());
            aResult["majorRadius"] = aHyperbola.MajorRadius();
            aResult["minorRadius"] = aHyperbola.MinorRadius();
            break;
          }
          case GeomAbs_Parabola:
          {
            const gp_Parab aParabola = anAdaptor.Parabola();
            aResult["type"] = "parabola";
            setAxis(aParabola.Position());
            aResult["focal"] = aParabola.Focal();
            break;
          }
          case GeomAbs_BezierCurve:
            aResult = {{"type", "bezier"}, {"degree", anAdaptor.Degree()},
                       {"poleCount", anAdaptor.NbPoles()}, {"rational", anAdaptor.IsRational()}};
            break;
          case GeomAbs_BSplineCurve:
            aResult = {{"type", "bspline"}, {"degree", anAdaptor.Degree()},
                       {"poleCount", anAdaptor.NbPoles()}, {"knotCount", anAdaptor.NbKnots()},
                       {"rational", anAdaptor.IsRational()}};
            break;
          case GeomAbs_OffsetCurve:
          {
            const occ::handle<Geom_OffsetCurve> anOffset = anAdaptor.OffsetCurve();
            const gp_Dir& aDirection = anOffset->Direction();
            aResult = {{"type", "offset"}, {"offset", anOffset->Offset()},
                       {"direction", {aDirection.X(), aDirection.Y(), aDirection.Z()}}};
            break;
          }
          case GeomAbs_OtherCurve:
          default:
            aResult = {{"type", "other"}};
            break;
        }
        return aResult;
      }
      if (theMode == CurveQueryMode::Domain)
      {
        return {{"first", anAdaptor.FirstParameter()}, {"last", anAdaptor.LastParameter()},
                {"periodic", isPeriodic}, {"period", isPeriodic ? anAdaptor.Period() : 0.0}};
      }
      const double aParameter = requiredNumber(theArgs, "parameter");
      const double aResolution = theArgs.value("resolution", 1.0e-12);
      if (aResolution <= 0.0) throw KernelFailure(ErrorCode::InvalidArgs, "Curve resolution must be positive");
      if (aParameter < anAdaptor.FirstParameter() || aParameter > anAdaptor.LastParameter())
        throw KernelFailure(ErrorCode::InvalidArgs, "Curve parameter is outside the edge domain");
      BRepLProp_CLProps aProperties(anAdaptor, aParameter, 2, aResolution);
      const gp_Pnt& aPoint = aProperties.Value();
      const gp_Vec& aD1 = aProperties.D1();
      const gp_Vec& aD2 = aProperties.D2();
      const auto isCurveIntervalBoundary = [&](const GeomAbs_Shape theContinuity) {
        const int anIntervalCount = anAdaptor.NbIntervals(theContinuity);
        if (anIntervalCount <= 1) return false;
        TColStd_Array1OfReal anIntervals(1, anIntervalCount + 1);
        anAdaptor.Intervals(anIntervals, theContinuity);
        const double aTolerance = std::max(aResolution, Precision::PConfusion());
        for (int anIndex = 2; anIndex <= anIntervalCount; ++anIndex)
          if (std::abs(aParameter - anIntervals(anIndex)) <= aTolerance) return true;
        return false;
      };
      const bool hasTangent = aProperties.IsTangentDefined()
        && !isCurveIntervalBoundary(GeomAbs_C1);
      const bool hasCurvature = hasTangent;
      json aResult{{"parameter", aParameter},
                   {"point", {aPoint.X(), aPoint.Y(), aPoint.Z()}},
                   {"d1", {aD1.X(), aD1.Y(), aD1.Z()}},
                   {"d2", {aD2.X(), aD2.Y(), aD2.Z()}},
                   {"tangentDefined", hasTangent},
                   {"curvatureDefined", hasCurvature}};
      if (hasTangent)
      {
        gp_Dir aTangent;
        aProperties.Tangent(aTangent);
        aResult["tangent"] = {aTangent.X(), aTangent.Y(), aTangent.Z()};
      }
      if (hasCurvature)
      {
        aResult["curvature"] = aProperties.Curvature();
      }
      return aResult;
    }

} // namespace

json h_curveContinuity(KernelOperationContext& theContext, const json& theArgs)
{
  return queryCurve(theContext, theArgs, CurveQueryMode::Continuity);
}

json h_curveDomain(KernelOperationContext& theContext, const json& theArgs)
{
  return queryCurve(theContext, theArgs, CurveQueryMode::Domain);
}

json h_curveGeometry(KernelOperationContext& theContext, const json& theArgs)
{
  return queryCurve(theContext, theArgs, CurveQueryMode::Geometry);
}

json h_evaluateCurve(KernelOperationContext& theContext, const json& theArgs)
{
  return queryCurve(theContext, theArgs, CurveQueryMode::Evaluate);
}

namespace {
enum class SurfaceQueryMode { Continuity, Domain, Geometry, Evaluate };

json querySurface(KernelOperationContext& theContext, const json& theArgs,
                  SurfaceQueryMode theMode)
{
      const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
      if (aShape.ShapeType() != TopAbs_FACE)
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface evaluation requires a face shape");
      const TopoDS_Face aFace = TopoDS::Face(aShape);
      BRepAdaptor_Surface anAdaptor(aFace, true);
      const bool isUPeriodic = anAdaptor.IsUPeriodic();
      const bool isVPeriodic = anAdaptor.IsVPeriodic();
      if (theMode == SurfaceQueryMode::Continuity)
        return {{"uContinuity", continuityName(anAdaptor.UContinuity())},
                {"vContinuity", continuityName(anAdaptor.VContinuity())}};
      if (theMode == SurfaceQueryMode::Geometry)
      {
        json aResult;
        auto setAxis = [&](const gp_Ax3& theAxis) {
          const gp_Pnt& anOrigin = theAxis.Location();
          const gp_Dir& aDirection = theAxis.Direction();
          const gp_Dir& anXDirection = theAxis.XDirection();
          aResult["origin"] = {anOrigin.X(), anOrigin.Y(), anOrigin.Z()};
          aResult["direction"] = {aDirection.X(), aDirection.Y(), aDirection.Z()};
          aResult["xDirection"] = {anXDirection.X(), anXDirection.Y(), anXDirection.Z()};
        };
        switch (anAdaptor.GetType())
        {
          case GeomAbs_Plane:
            aResult["type"] = "plane";
            setAxis(anAdaptor.Plane().Position());
            break;
          case GeomAbs_Cylinder:
          {
            const gp_Cylinder aCylinder = anAdaptor.Cylinder();
            aResult["type"] = "cylinder";
            setAxis(aCylinder.Position());
            aResult["radius"] = aCylinder.Radius();
            break;
          }
          case GeomAbs_Cone:
          {
            const gp_Cone aCone = anAdaptor.Cone();
            aResult["type"] = "cone";
            setAxis(aCone.Position());
            aResult["referenceRadius"] = aCone.RefRadius();
            aResult["semiAngle"] = aCone.SemiAngle();
            break;
          }
          case GeomAbs_Sphere:
          {
            const gp_Sphere aSphere = anAdaptor.Sphere();
            aResult["type"] = "sphere";
            setAxis(aSphere.Position());
            aResult["radius"] = aSphere.Radius();
            break;
          }
          case GeomAbs_Torus:
          {
            const gp_Torus aTorus = anAdaptor.Torus();
            aResult["type"] = "torus";
            setAxis(aTorus.Position());
            aResult["majorRadius"] = aTorus.MajorRadius();
            aResult["minorRadius"] = aTorus.MinorRadius();
            break;
          }
          case GeomAbs_BezierSurface:
            aResult = {{"type", "bezier"}, {"uDegree", anAdaptor.UDegree()},
                       {"vDegree", anAdaptor.VDegree()}, {"uPoleCount", anAdaptor.NbUPoles()},
                       {"vPoleCount", anAdaptor.NbVPoles()}};
            break;
          case GeomAbs_BSplineSurface:
            aResult = {{"type", "bspline"}, {"uDegree", anAdaptor.UDegree()},
                       {"vDegree", anAdaptor.VDegree()}, {"uPoleCount", anAdaptor.NbUPoles()},
                       {"vPoleCount", anAdaptor.NbVPoles()}, {"uKnotCount", anAdaptor.NbUKnots()},
                       {"vKnotCount", anAdaptor.NbVKnots()}};
            break;
          case GeomAbs_SurfaceOfRevolution:
          {
            const gp_Ax1 anAxis = anAdaptor.AxeOfRevolution();
            const gp_Pnt& anOrigin = anAxis.Location();
            const gp_Dir& aDirection = anAxis.Direction();
            aResult = {{"type", "revolution"},
                       {"origin", {anOrigin.X(), anOrigin.Y(), anOrigin.Z()}},
                       {"direction", {aDirection.X(), aDirection.Y(), aDirection.Z()}}};
            break;
          }
          case GeomAbs_SurfaceOfExtrusion:
          {
            const gp_Dir aDirection = anAdaptor.Direction();
            aResult = {{"type", "extrusion"},
                       {"direction", {aDirection.X(), aDirection.Y(), aDirection.Z()}}};
            break;
          }
          case GeomAbs_OffsetSurface:
            aResult = {{"type", "offset"}, {"offset", anAdaptor.OffsetValue()}};
            break;
          case GeomAbs_OtherSurface:
          default:
            aResult = {{"type", "other"}};
            break;
        }
        return aResult;
      }
      if (theMode == SurfaceQueryMode::Domain)
      {
        return {{"uFirst", anAdaptor.FirstUParameter()}, {"uLast", anAdaptor.LastUParameter()},
                {"vFirst", anAdaptor.FirstVParameter()}, {"vLast", anAdaptor.LastVParameter()},
                {"uPeriodic", isUPeriodic}, {"uPeriod", isUPeriodic ? anAdaptor.UPeriod() : 0.0},
                {"vPeriodic", isVPeriodic}, {"vPeriod", isVPeriodic ? anAdaptor.VPeriod() : 0.0}};
      }
      const double aU = requiredNumber(theArgs, "u");
      const double aV = requiredNumber(theArgs, "v");
      const double aResolution = theArgs.value("resolution", 1.0e-12);
      if (aResolution <= 0.0) throw KernelFailure(ErrorCode::InvalidArgs, "Surface resolution must be positive");
      if (aU < anAdaptor.FirstUParameter() || aU > anAdaptor.LastUParameter()
          || aV < anAdaptor.FirstVParameter() || aV > anAdaptor.LastVParameter())
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface parameters are outside the face domain");
      BRepLProp_SLProps aProperties(anAdaptor, aU, aV, 2, aResolution);
      const gp_Pnt& aPoint = aProperties.Value();
      const gp_Vec& aD1U = aProperties.D1U();
      const gp_Vec& aD1V = aProperties.D1V();
      const gp_Vec& aD2U = aProperties.D2U();
      const gp_Vec& aD2V = aProperties.D2V();
      const gp_Vec& aDUV = aProperties.DUV();
      const auto isSurfaceIntervalBoundary = [&](const bool isU,
                                                  const GeomAbs_Shape theContinuity) {
        const int anIntervalCount = isU
          ? anAdaptor.NbUIntervals(theContinuity)
          : anAdaptor.NbVIntervals(theContinuity);
        if (anIntervalCount <= 1) return false;
        TColStd_Array1OfReal anIntervals(1, anIntervalCount + 1);
        if (isU) anAdaptor.UIntervals(anIntervals, theContinuity);
        else anAdaptor.VIntervals(anIntervals, theContinuity);
        const double aParameter = isU ? aU : aV;
        const double aTolerance = std::max(aResolution, Precision::PConfusion());
        for (int anIndex = 2; anIndex <= anIntervalCount; ++anIndex)
          if (std::abs(aParameter - anIntervals(anIndex)) <= aTolerance) return true;
        return false;
      };
      const bool hasNormal = aProperties.IsNormalDefined()
        && !isSurfaceIntervalBoundary(true, GeomAbs_C1)
        && !isSurfaceIntervalBoundary(false, GeomAbs_C1);
      const bool hasCurvature = hasNormal && aProperties.IsCurvatureDefined();
      json aResult{{"u", aU}, {"v", aV},
                   {"point", {aPoint.X(), aPoint.Y(), aPoint.Z()}},
                   {"d1u", {aD1U.X(), aD1U.Y(), aD1U.Z()}},
                   {"d1v", {aD1V.X(), aD1V.Y(), aD1V.Z()}},
                   {"d2u", {aD2U.X(), aD2U.Y(), aD2U.Z()}},
                   {"d2v", {aD2V.X(), aD2V.Y(), aD2V.Z()}},
                   {"duv", {aDUV.X(), aDUV.Y(), aDUV.Z()}},
                   {"normalDefined", hasNormal}, {"curvatureDefined", hasCurvature}};
      const bool isReversed = aFace.Orientation() == TopAbs_REVERSED;
      if (hasNormal)
      {
        gp_Dir aNormal = aProperties.Normal();
        if (isReversed) aNormal.Reverse();
        aResult["normal"] = {aNormal.X(), aNormal.Y(), aNormal.Z()};
      }
      if (hasCurvature)
      {
        const double aRawMinimum = aProperties.MinCurvature();
        const double aRawMaximum = aProperties.MaxCurvature();
        aResult["minimumCurvature"] = isReversed ? -aRawMaximum : aRawMinimum;
        aResult["maximumCurvature"] = isReversed ? -aRawMinimum : aRawMaximum;
        aResult["meanCurvature"] = isReversed ? -aProperties.MeanCurvature() : aProperties.MeanCurvature();
        aResult["gaussianCurvature"] = aProperties.GaussianCurvature();
        if (!aProperties.IsUmbilic())
        {
          gp_Dir aMaximumDirection;
          gp_Dir aMinimumDirection;
          aProperties.CurvatureDirections(aMaximumDirection, aMinimumDirection);
          const gp_Dir& aResultMinimumDirection = isReversed ? aMaximumDirection : aMinimumDirection;
          const gp_Dir& aResultMaximumDirection = isReversed ? aMinimumDirection : aMaximumDirection;
          aResult["minimumDirection"] = {aResultMinimumDirection.X(), aResultMinimumDirection.Y(), aResultMinimumDirection.Z()};
          aResult["maximumDirection"] = {aResultMaximumDirection.X(), aResultMaximumDirection.Y(), aResultMaximumDirection.Z()};
        }
      }
      return aResult;
    }

} // namespace

json h_surfaceContinuity(KernelOperationContext& theContext, const json& theArgs)
{
  return querySurface(theContext, theArgs, SurfaceQueryMode::Continuity);
}

json h_surfaceDomain(KernelOperationContext& theContext, const json& theArgs)
{
  return querySurface(theContext, theArgs, SurfaceQueryMode::Domain);
}

json h_surfaceGeometry(KernelOperationContext& theContext, const json& theArgs)
{
  return querySurface(theContext, theArgs, SurfaceQueryMode::Geometry);
}

json h_evaluateSurface(KernelOperationContext& theContext, const json& theArgs)
{
  return querySurface(theContext, theArgs, SurfaceQueryMode::Evaluate);
}

json h_surfaceIsoCurve(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      double aUFirst = 0.0;
      double aULast = 0.0;
      double aVFirst = 0.0;
      double aVLast = 0.0;
      const TopoDS_Shape& aSurfaceShape = theContext.arena().get(requiredU32(theArgs, "surface"));
      const occ::handle<Geom_Surface> aSurface = surfaceForFace(
        aSurfaceShape, aUFirst, aULast, aVFirst, aVLast);
      const std::string aDirection = theArgs.value("direction", "");
      const double aParameter = requiredNumber(theArgs, "parameter");
      const double aTolerance = theArgs.value("tolerance", 1.0e-7);
      if (aTolerance <= 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs, "Iso-curve tolerance must be positive");

      occ::handle<Geom_Curve> anIsoCurve;
      double aFirst = 0.0;
      double aLast = 0.0;
      if (aDirection == "u")
      {
        if (aParameter < aUFirst || aParameter > aULast)
          throw KernelFailure(ErrorCode::InvalidArgs, "U iso-curve parameter is outside the face domain");
        anIsoCurve = aSurface->UIso(aParameter);
        aFirst = aVFirst;
        aLast = aVLast;
      }
      else if (aDirection == "v")
      {
        if (aParameter < aVFirst || aParameter > aVLast)
          throw KernelFailure(ErrorCode::InvalidArgs, "V iso-curve parameter is outside the face domain");
        anIsoCurve = aSurface->VIso(aParameter);
        aFirst = aUFirst;
        aLast = aULast;
      }
      else
      {
        throw KernelFailure(ErrorCode::InvalidArgs, "Iso-curve direction must be u or v");
      }
      BRepBuilderAPI_MakeEdge anIsoEdge(anIsoCurve, aFirst, aLast);
      if (!anIsoEdge.IsDone())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct the surface iso-curve");

      BRepAlgoAPI_Section aClip(anIsoEdge.Edge(), aSurfaceShape, false);
      aClip.SetNonDestructive(true);
      aClip.SetRunParallel(false);
      aClip.SetFuzzyValue(aTolerance);
      aClip.Approximation(true);
      aClip.ComputePCurveOn2(true);
      aClip.Build();
      if (aClip.HasErrors())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to clip the surface iso-curve");

      json aShapes = json::array();
      TopTools_IndexedMapOfShape anEdges;
      TopExp::MapShapes(aClip.Shape(), TopAbs_EDGE, anEdges);
      for (int anIndex = 1; anIndex <= anEdges.Extent(); ++anIndex)
      {
        const TopoDS_Shape& anEdge = anEdges(anIndex);
        aShapes.push_back(theContext.arena().add(anEdge, aScope));
      }
      return {{"shapes", aShapes}};
    }

} // namespace occt_worker
