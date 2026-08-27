// Focused geometry/topology operation handlers.

#include "kernel_geometry_topology_operations.hpp"
#include "kernel_geometry_helpers.hpp"
#include "kernel_operation_context.hpp"
#include "kernel_protocol_helpers.hpp"

namespace occt_worker {

namespace {
enum class ApproximationMode { Curve, Surface };

json approximateBSpline(KernelOperationContext& theContext, const json& theArgs,
                        ApproximationMode theMode)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const int aDegreeMin = theArgs.value("degreeMin", 3);
      const int aDegreeMax = theArgs.value("degreeMax", 8);
      const double aTolerance = theArgs.value("tolerance", 1.0e-3);
      const int aMaximumDegree = theMode == ApproximationMode::Curve
        ? Geom_BSplineCurve::MaxDegree() : Geom_BSplineSurface::MaxDegree();
      if (aDegreeMin < 1 || aDegreeMax < aDegreeMin || aDegreeMax > aMaximumDegree
          || aTolerance <= 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs, "BSpline approximation degree range or tolerance is invalid");

      Approx_ParametrizationType aParameterization = Approx_ChordLength;
      const std::string aParameterizationName = theArgs.value("parameterization", "chordLength");
      if (aParameterizationName == "centripetal") aParameterization = Approx_Centripetal;
      else if (aParameterizationName == "uniform") aParameterization = Approx_IsoParametric;
      else if (aParameterizationName != "chordLength")
        throw KernelFailure(ErrorCode::InvalidArgs, "Unknown BSpline approximation parameterization");

      GeomAbs_Shape aContinuity = GeomAbs_C2;
      const std::string aContinuityName = theArgs.value("continuity", "c2");
      if (aContinuityName == "c0") aContinuity = GeomAbs_C0;
      else if (aContinuityName == "c1") aContinuity = GeomAbs_C1;
      else if (aContinuityName == "c3") aContinuity = GeomAbs_C3;
      else if (aContinuityName != "c2")
        throw KernelFailure(ErrorCode::InvalidArgs, "Unknown BSpline approximation continuity");

      std::array<double, 3> aSmoothingWeights{};
      const bool hasVariationalSmoothing = theArgs.contains("variationalSmoothing");
      if (hasVariationalSmoothing)
      {
        if (theArgs.contains("parameterization") || theArgs.contains("degreeMin"))
          throw KernelFailure(ErrorCode::InvalidArgs,
                              "Variational smoothing does not use parameterization or degreeMin");
        const json& aSmoothing = theArgs.at("variationalSmoothing");
        if (!aSmoothing.is_object())
          throw KernelFailure(ErrorCode::InvalidArgs, "Variational smoothing weights are invalid");
        for (std::size_t anIndex = 0; anIndex < aSmoothingWeights.size(); ++anIndex)
        {
          const char* aName = anIndex == 0 ? "length" : (anIndex == 1 ? "curvature" : "torsion");
          if (!aSmoothing.contains(aName) || !aSmoothing.at(aName).is_number())
            throw KernelFailure(ErrorCode::InvalidArgs, "Variational smoothing weights are invalid");
          aSmoothingWeights[anIndex] = aSmoothing.at(aName).get<double>();
          if (!std::isfinite(aSmoothingWeights[anIndex]) || aSmoothingWeights[anIndex] < 0.0)
            throw KernelFailure(ErrorCode::InvalidArgs, "Variational smoothing weights must be non-negative");
        }
        if (aSmoothingWeights[0] + aSmoothingWeights[1] + aSmoothingWeights[2] <= 0.0)
          throw KernelFailure(ErrorCode::InvalidArgs, "At least one variational smoothing weight must be positive");
      }

      if (!theArgs.contains("points"))
        throw KernelFailure(ErrorCode::InvalidArgs, "BSpline approximation points are required");
      if (theMode == ApproximationMode::Curve)
      {
        std::vector<gp_Pnt> aPointValues;
        if (theArgs.at("points").is_array())
        {
          for (const json& aPointValue : theArgs.at("points"))
          {
            if (!aPointValue.is_array() || aPointValue.size() != 3 || !aPointValue[0].is_number()
                || !aPointValue[1].is_number() || !aPointValue[2].is_number())
              throw KernelFailure(ErrorCode::InvalidArgs, "Curve approximation points must be vec3");
            aPointValues.emplace_back(aPointValue[0].get<double>(), aPointValue[1].get<double>(),
                                      aPointValue[2].get<double>());
          }
        }
        else if (theArgs.at("points").is_object())
        {
          const std::string aPointBytes = inputBufferData(theContext.buffers(), theArgs, "points");
          if (aPointBytes.size() % (3 * sizeof(double)) != 0)
            throw KernelFailure(ErrorCode::InvalidArgs, "Curve approximation point buffer must use the f64x3 layout");
          for (std::size_t anOffset = 0; anOffset < aPointBytes.size(); anOffset += 3 * sizeof(double))
          {
            std::array<double, 3> aPoint{};
            std::memcpy(aPoint.data(), aPointBytes.data() + anOffset, 3 * sizeof(double));
            aPointValues.emplace_back(aPoint[0], aPoint[1], aPoint[2]);
          }
        }
        else
        {
          throw KernelFailure(ErrorCode::InvalidArgs, "Curve approximation points must be an array or f64x3 buffer reference");
        }
        if (aPointValues.size() < 2)
          throw KernelFailure(ErrorCode::InvalidArgs, "Curve approximation requires at least two points");
        NCollection_Array1<gp_Pnt> aPoints(1, static_cast<int>(aPointValues.size()));
        for (int anIndex = 1; anIndex <= aPoints.Upper(); ++anIndex)
          aPoints.SetValue(anIndex, aPointValues[static_cast<std::size_t>(anIndex - 1)]);
        GeomAPI_PointsToBSpline anApproximation;
        if (hasVariationalSmoothing)
          anApproximation.Init(aPoints, aSmoothingWeights[0], aSmoothingWeights[1],
                               aSmoothingWeights[2], aDegreeMax, aContinuity, aTolerance);
        else
          anApproximation.Init(
            aPoints, aParameterization, aDegreeMin, aDegreeMax, aContinuity, aTolerance);
        if (!anApproximation.IsDone() || anApproximation.Curve().IsNull())
          throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to approximate the BSpline curve");
        BRepBuilderAPI_MakeEdge aBuilder(anApproximation.Curve());
        if (!aBuilder.IsDone() || aBuilder.Edge().IsNull())
          throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct the approximated curve edge");
        return {{"shape", theContext.arena().add(aBuilder.Edge(), aScope)}};
      }

      if (!theArgs.at("points").is_array() || theArgs.at("points").size() < 2)
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface approximation requires a point grid");
      const std::size_t aUCount = theArgs.at("points").size();
      const std::size_t aVCount = theArgs.at("points").at(0).is_array()
        ? theArgs.at("points").at(0).size() : 0;
      if (aVCount < 2)
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface approximation requires at least two points in each direction");
      NCollection_Array2<gp_Pnt> aPoints(1, static_cast<int>(aUCount), 1, static_cast<int>(aVCount));
      for (std::size_t aU = 0; aU < aUCount; ++aU)
      {
        const json& aRow = theArgs.at("points").at(aU);
        if (!aRow.is_array() || aRow.size() != aVCount)
          throw KernelFailure(ErrorCode::InvalidArgs, "Surface approximation point grid must be rectangular");
        for (std::size_t aV = 0; aV < aVCount; ++aV)
        {
          const json& aPoint = aRow.at(aV);
          if (!aPoint.is_array() || aPoint.size() != 3 || !aPoint[0].is_number()
              || !aPoint[1].is_number() || !aPoint[2].is_number())
            throw KernelFailure(ErrorCode::InvalidArgs, "Surface approximation points must be vec3");
          aPoints.SetValue(static_cast<int>(aU + 1), static_cast<int>(aV + 1),
                           gp_Pnt(aPoint[0].get<double>(), aPoint[1].get<double>(),
                                  aPoint[2].get<double>()));
        }
      }
      GeomAPI_PointsToBSplineSurface anApproximation;
      if (hasVariationalSmoothing)
        anApproximation.Init(aPoints, aSmoothingWeights[0], aSmoothingWeights[1],
                             aSmoothingWeights[2], aDegreeMax, aContinuity, aTolerance);
      else
        anApproximation.Init(
          aPoints, aParameterization, aDegreeMin, aDegreeMax, aContinuity, aTolerance);
      if (!anApproximation.IsDone() || anApproximation.Surface().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to approximate the BSpline surface");
      double aUFirst = 0.0;
      double aULast = 0.0;
      double aVFirst = 0.0;
      double aVLast = 0.0;
      anApproximation.Surface()->Bounds(aUFirst, aULast, aVFirst, aVLast);
      const TopoDS_Face aFace = makeRectangularFace(
        anApproximation.Surface(), aUFirst, aULast, aVFirst, aVLast,
        1.0e-7, TopAbs_FORWARD);
      return {{"shape", theContext.arena().add(aFace, aScope)}};
    }

} // namespace

json h_approximateCurveBSpline(KernelOperationContext& theContext,
                               const json& theArgs)
{
  return approximateBSpline(theContext, theArgs, ApproximationMode::Curve);
}

json h_approximateSurfaceBSpline(KernelOperationContext& theContext,
                                 const json& theArgs)
{
  return approximateBSpline(theContext, theArgs, ApproximationMode::Surface);
}

json h_reduceCurveDegree(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
      if (aShape.ShapeType() != TopAbs_EDGE)
        throw KernelFailure(ErrorCode::InvalidArgs, "Curve degree reduction requires an edge shape");
      const TopoDS_Edge aEdge = TopoDS::Edge(aShape);
      BRepAdaptor_Curve anAdaptor(aEdge);
      if (anAdaptor.GetType() != GeomAbs_BezierCurve
          && anAdaptor.GetType() != GeomAbs_BSplineCurve)
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "Curve degree reduction requires a Bezier or BSpline edge");
      const int aTargetDegree = static_cast<int>(requiredU32(theArgs, "degree"));
      const int aMaxSegments = static_cast<int>(theArgs.value("maxSegments", 100u));
      const double aTolerance = theArgs.value("tolerance", 1.0e-3);
      if (aTargetDegree < 1 || aTargetDegree >= anAdaptor.Degree()
          || aMaxSegments < 1 || !std::isfinite(aTolerance) || aTolerance <= 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "Curve target degree must be lower than the input degree, with positive tolerance and maxSegments");
      GeomAbs_Shape aContinuity = GeomAbs_C1;
      const std::string aContinuityName = theArgs.value("continuity", "c1");
      if (aContinuityName == "c0") aContinuity = GeomAbs_C0;
      else if (aContinuityName == "c2") aContinuity = GeomAbs_C2;
      else if (aContinuityName == "c3") aContinuity = GeomAbs_C3;
      else if (aContinuityName != "c1")
        throw KernelFailure(ErrorCode::InvalidArgs, "Unknown curve degree reduction continuity");
      const int aContinuityOrder = aContinuity == GeomAbs_C0 ? 0
        : aContinuity == GeomAbs_C1 ? 1
        : aContinuity == GeomAbs_C2 ? 2 : 3;
      if (aContinuityOrder > aTargetDegree - 1)
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "Curve target degree is too low for the requested continuity");

      double aFirst = 0.0;
      double aLast = 0.0;
      const occ::handle<Geom_Curve> aCurve = curveForEdge(aShape, aFirst, aLast);
      const occ::handle<Geom_Curve> aTrimmedCurve =
        new Geom_TrimmedCurve(aCurve, aFirst, aLast);
      GeomConvert_ApproxCurve anApproximation(
        aTrimmedCurve, aTolerance, aContinuity, aMaxSegments, aTargetDegree);
      if (!anApproximation.IsDone() || !anApproximation.HasResult()
          || anApproximation.Curve().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed,
                            "OCCT could not reduce the curve degree within tolerance");
      BRepBuilderAPI_MakeEdge aBuilder(anApproximation.Curve(), aFirst, aLast);
      if (!aBuilder.IsDone() || aBuilder.Edge().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed,
                            "OCCT failed to construct the reduced-degree curve edge");
      TopoDS_Edge aResult = aBuilder.Edge();
      aResult.Orientation(aEdge.Orientation());
      return {{"shape", theContext.arena().add(aResult, aScope)},
              {"maxError", anApproximation.MaxError()}};
    }

json h_reduceSurfaceDegree(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
      if (aShape.ShapeType() != TopAbs_FACE)
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface degree reduction requires a face shape");
      const TopoDS_Face aFace = TopoDS::Face(aShape);
      BRepAdaptor_Surface anAdaptor(aFace, true);
      if (anAdaptor.GetType() != GeomAbs_BezierSurface
          && anAdaptor.GetType() != GeomAbs_BSplineSurface)
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "Surface degree reduction requires a Bezier or BSpline face");
      const int aUTargetDegree = static_cast<int>(requiredU32(theArgs, "uDegree"));
      const int aVTargetDegree = static_cast<int>(requiredU32(theArgs, "vDegree"));
      const int aMaxSegments = static_cast<int>(theArgs.value("maxSegments", 100u));
      const double aTolerance = theArgs.value("tolerance", 1.0e-3);
      if (aUTargetDegree < 1 || aVTargetDegree < 1
          || aUTargetDegree > anAdaptor.UDegree() || aVTargetDegree > anAdaptor.VDegree()
          || (aUTargetDegree == anAdaptor.UDegree() && aVTargetDegree == anAdaptor.VDegree())
          || aMaxSegments < 1 || !std::isfinite(aTolerance) || aTolerance <= 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "Surface target degrees must not exceed the input degrees and at least one must be lower, with positive tolerance and maxSegments");
      auto readContinuity = [&](const char* theKey) {
        const std::string aName = theArgs.value(theKey, "c1");
        if (aName == "c0") return GeomAbs_C0;
        if (aName == "c1") return GeomAbs_C1;
        if (aName == "c2") return GeomAbs_C2;
        if (aName == "c3") return GeomAbs_C3;
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "Unknown surface degree reduction continuity");
      };
      const GeomAbs_Shape aUContinuity = readContinuity("uContinuity");
      const GeomAbs_Shape aVContinuity = readContinuity("vContinuity");
      auto continuityOrder = [](const GeomAbs_Shape theContinuity) {
        return theContinuity == GeomAbs_C0 ? 0
          : theContinuity == GeomAbs_C1 ? 1
          : theContinuity == GeomAbs_C2 ? 2 : 3;
      };
      if (continuityOrder(aUContinuity) > aUTargetDegree - 1
          || continuityOrder(aVContinuity) > aVTargetDegree - 1)
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "Surface target degree is too low for the requested continuity");

      double aUFirst = 0.0;
      double aULast = 0.0;
      double aVFirst = 0.0;
      double aVLast = 0.0;
      const occ::handle<Geom_Surface> aSurface = surfaceForFace(
        aShape, aUFirst, aULast, aVFirst, aVLast);
      const occ::handle<Geom_Surface> aTrimmedSurface =
        new Geom_RectangularTrimmedSurface(
          aSurface, aUFirst, aULast, aVFirst, aVLast);
      GeomConvert_ApproxSurface anApproximation(
        aTrimmedSurface, aTolerance, aUContinuity, aVContinuity,
        aUTargetDegree, aVTargetDegree, aMaxSegments, 1);
      if (!anApproximation.IsDone() || !anApproximation.HasResult()
          || anApproximation.Surface().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed,
                            "OCCT could not reduce the surface degree within tolerance");
      const TopoDS_Face aResult = makeSurfaceFacePreservingWires(
        aFace, anApproximation.Surface(), aTolerance, aFace.Orientation());
      return {{"shape", theContext.arena().add(aResult, aScope)},
              {"maxError", anApproximation.MaxError()}};
    }

json h_extendCurve(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
      if (aShape.ShapeType() != TopAbs_EDGE)
        throw KernelFailure(ErrorCode::InvalidArgs, "Curve extension requires an edge shape");
      const TopoDS_Edge aEdge = TopoDS::Edge(aShape);
      BRepAdaptor_Curve anAdaptor(aEdge);
      occ::handle<Geom_BoundedCurve> aCurve;
      if (anAdaptor.GetType() == GeomAbs_BezierCurve)
      {
        aCurve = occ::down_cast<Geom_BoundedCurve>(anAdaptor.Bezier()->Copy());
      }
      else if (anAdaptor.GetType() == GeomAbs_BSplineCurve)
      {
        aCurve = occ::down_cast<Geom_BoundedCurve>(anAdaptor.BSpline()->Copy());
      }
      else
      {
        throw KernelFailure(ErrorCode::InvalidArgs, "Curve extension is available only for Bezier and BSpline edges");
      }
      const auto aPoint = requiredVec3(theArgs, "point");
      int aContinuity = 1;
      const std::string aContinuityName = theArgs.value("continuity", "c1");
      if (aContinuityName == "c2") aContinuity = 2;
      else if (aContinuityName == "c3") aContinuity = 3;
      else if (aContinuityName != "c1")
        throw KernelFailure(ErrorCode::InvalidArgs, "Unknown curve extension continuity");
      GeomLib::ExtendCurveToPoint(
        aCurve, gp_Pnt(aPoint[0], aPoint[1], aPoint[2]), aContinuity,
        theArgs.value("after", true));
      const double aFirst = aCurve->FirstParameter();
      const double aLast = aCurve->LastParameter();
      BRepBuilderAPI_MakeEdge aBuilder(aCurve, aFirst, aLast);
      if (!aBuilder.IsDone() || aBuilder.Edge().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct the extended curve edge");
      TopoDS_Edge anExtendedEdge = aBuilder.Edge();
      anExtendedEdge.Orientation(aEdge.Orientation());
      return {{"shape", theContext.arena().add(anExtendedEdge, aScope)}};
    }

json h_extendSurface(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
      if (aShape.ShapeType() != TopAbs_FACE)
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface extension requires a face shape");
      const TopoDS_Face aFace = TopoDS::Face(aShape);
      const occ::handle<Geom_Surface> aSurface = BRep_Tool::Surface(aFace);
      if (aSurface.IsNull())
        throw KernelFailure(ErrorCode::InvalidArgs, "Face has no geometric surface");
      double aUFirst = 0.0;
      double aULast = 0.0;
      double aVFirst = 0.0;
      double aVLast = 0.0;
      BRepTools::UVBounds(aFace, aUFirst, aULast, aVFirst, aVLast);
      if (!(aUFirst < aULast && aVFirst < aVLast))
        throw KernelFailure(ErrorCode::InvalidArgs, "Face has an invalid finite UV domain");
      const double aLength = requiredNumber(theArgs, "length");
      if (aLength <= 0.0) throw KernelFailure(ErrorCode::InvalidArgs, "Surface extension length must be positive");
      const std::string aDirection = theArgs.at("direction").get<std::string>();
      const std::string aSide = theArgs.at("side").get<std::string>();
      if ((aDirection != "u" && aDirection != "v") || (aSide != "before" && aSide != "after"))
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface extension direction or side is invalid");
      int aContinuity = 1;
      const std::string aContinuityName = theArgs.value("continuity", "c1");
      if (aContinuityName == "c2") aContinuity = 2;
      else if (aContinuityName == "c3") aContinuity = 3;
      else if (aContinuityName != "c1")
        throw KernelFailure(ErrorCode::InvalidArgs, "Unknown surface extension continuity");
      occ::handle<Geom_BoundedSurface> anExtendedSurface;
      if (aSurface->IsKind(STANDARD_TYPE(Geom_BoundedSurface)))
      {
        anExtendedSurface = occ::down_cast<Geom_BoundedSurface>(aSurface->Copy());
      }
      else
      {
        anExtendedSurface = new Geom_RectangularTrimmedSurface(
          aSurface, aUFirst, aULast, aVFirst, aVLast);
      }
      GeomLib::ExtendSurfByLength(
        anExtendedSurface, aLength, aContinuity, aDirection == "u", aSide == "after");
      anExtendedSurface->Bounds(aUFirst, aULast, aVFirst, aVLast);
      const std::array<double, 4> anExtendedBounds{aUFirst, aULast, aVFirst, aVLast};
      const TopoDS_Face anExtendedFace = makeSurfaceFacePreservingWires(
        aFace, anExtendedSurface, 1.0e-7, aFace.Orientation(), &anExtendedBounds);
      return {{"shape", theContext.arena().add(anExtendedFace, aScope)}};
    }

} // namespace occt_worker
