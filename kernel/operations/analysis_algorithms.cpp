// Geometry analysis algorithm handlers.

#include "kernel_algorithms_operations.hpp"
#include "kernel_geometry_helpers.hpp"
#include "kernel_operation_context.hpp"
#include "kernel_protocol_helpers.hpp"

namespace occt_worker {

json h_massProps(KernelOperationContext& theContext, const json& theArgs)
{
      const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
      GProp_GProps aProps;
      const std::string aKind = theArgs.value("kind", "volume");
      if (aKind == "volume") BRepGProp::VolumeProperties(aShape, aProps);
      else if (aKind == "surface") BRepGProp::SurfaceProperties(aShape, aProps);
      else if (aKind == "linear") BRepGProp::LinearProperties(aShape, aProps);
      else throw KernelFailure(ErrorCode::InvalidArgs, "massProps kind must be volume, surface, or linear");
      const gp_Pnt aCenter = aProps.CentreOfMass();
      const gp_Mat anInertia = aProps.MatrixOfInertia();
      return {{"mass", aProps.Mass()}, {"center", {aCenter.X(), aCenter.Y(), aCenter.Z()}},
              {"inertia", {anInertia.Value(1, 1), anInertia.Value(1, 2), anInertia.Value(1, 3),
                           anInertia.Value(2, 1), anInertia.Value(2, 2), anInertia.Value(2, 3),
                           anInertia.Value(3, 1), anInertia.Value(3, 2), anInertia.Value(3, 3)}}};
    }

json h_distance(KernelOperationContext& theContext, const json& theArgs)
{
      const TopoDS_Shape& aFirstShape = theContext.arena().get(requiredU32(theArgs, "first"));
      const TopoDS_Shape& aSecondShape = theContext.arena().get(requiredU32(theArgs, "second"));
      BRepExtrema_DistShapeShape aDistance(aFirstShape, aSecondShape);
      if (!aDistance.IsDone() || aDistance.NbSolution() == 0)
        throw KernelFailure(ErrorCode::KernelError, "OCCT distance computation failed");

      TopTools_IndexedMapOfShape aFirstVertices;
      TopTools_IndexedMapOfShape aFirstEdges;
      TopTools_IndexedMapOfShape aFirstFaces;
      TopTools_IndexedMapOfShape aSecondVertices;
      TopTools_IndexedMapOfShape aSecondEdges;
      TopTools_IndexedMapOfShape aSecondFaces;
      TopExp::MapShapes(aFirstShape, TopAbs_VERTEX, aFirstVertices);
      TopExp::MapShapes(aFirstShape, TopAbs_EDGE, aFirstEdges);
      TopExp::MapShapes(aFirstShape, TopAbs_FACE, aFirstFaces);
      TopExp::MapShapes(aSecondShape, TopAbs_VERTEX, aSecondVertices);
      TopExp::MapShapes(aSecondShape, TopAbs_EDGE, aSecondEdges);
      TopExp::MapShapes(aSecondShape, TopAbs_FACE, aSecondFaces);

      json aSolutions = json::array();
      for (int anIndex = 1; anIndex <= aDistance.NbSolution(); ++anIndex)
      {
        const auto makeSupport = [&](const bool theFirst) {
          const BRepExtrema_SupportType aType = theFirst
            ? aDistance.SupportTypeShape1(anIndex)
            : aDistance.SupportTypeShape2(anIndex);
          const TopoDS_Shape aSupportShape = theFirst
            ? aDistance.SupportOnShape1(anIndex)
            : aDistance.SupportOnShape2(anIndex);
          json aSupport;
          if (aType == BRepExtrema_IsVertex)
          {
            aSupport["type"] = "vertex";
            const int aShapeIndex = (theFirst ? aFirstVertices : aSecondVertices).FindIndex(aSupportShape);
            if (aShapeIndex > 0) aSupport["index"] = aShapeIndex - 1;
          }
          else if (aType == BRepExtrema_IsOnEdge)
          {
            aSupport["type"] = "edge";
            const int aShapeIndex = (theFirst ? aFirstEdges : aSecondEdges).FindIndex(aSupportShape);
            if (aShapeIndex > 0) aSupport["index"] = aShapeIndex - 1;
            double aParameter = 0.0;
            if (theFirst) aDistance.ParOnEdgeS1(anIndex, aParameter);
            else aDistance.ParOnEdgeS2(anIndex, aParameter);
            aSupport["parameter"] = aParameter;
          }
          else
          {
            aSupport["type"] = "face";
            const int aShapeIndex = (theFirst ? aFirstFaces : aSecondFaces).FindIndex(aSupportShape);
            if (aShapeIndex > 0) aSupport["index"] = aShapeIndex - 1;
            double aU = 0.0;
            double aV = 0.0;
            if (theFirst) aDistance.ParOnFaceS1(anIndex, aU, aV);
            else aDistance.ParOnFaceS2(anIndex, aU, aV);
            aSupport["u"] = aU;
            aSupport["v"] = aV;
          }
          return aSupport;
        };
        const gp_Pnt& aFirst = aDistance.PointOnShape1(anIndex);
        const gp_Pnt& aSecond = aDistance.PointOnShape2(anIndex);
        aSolutions.push_back({
          {"distance", aFirst.Distance(aSecond)},
          {"pointOnFirst", {aFirst.X(), aFirst.Y(), aFirst.Z()}},
          {"pointOnSecond", {aSecond.X(), aSecond.Y(), aSecond.Z()}},
          {"supportOnFirst", makeSupport(true)},
          {"supportOnSecond", makeSupport(false)}});
      }
      const gp_Pnt& aFirst = aDistance.PointOnShape1(1);
      const gp_Pnt& aSecond = aDistance.PointOnShape2(1);
      return {{"distance", aDistance.Value()}, {"pointOnFirst", {aFirst.X(), aFirst.Y(), aFirst.Z()}},
              {"pointOnSecond", {aSecond.X(), aSecond.Y(), aSecond.Z()}},
              {"innerSolution", aDistance.InnerSolution()}, {"solutions", std::move(aSolutions)}};
    }

json h_classifyPoint(KernelOperationContext& theContext, const json& theArgs)
{
      const auto aPoint = requiredVec3(theArgs, "point");
      const double aTolerance = theArgs.value("tolerance", 1.0e-7);
      if (aTolerance <= 0.0) throw KernelFailure(ErrorCode::InvalidArgs, "Classification tolerance must be positive");
      BRepClass3d_SolidClassifier aClassifier(theContext.arena().get(requiredU32(theArgs, "shape")),
                                              gp_Pnt(aPoint[0], aPoint[1], aPoint[2]), aTolerance);
      const TopAbs_State aState = aClassifier.State();
      return {{"state", aState == TopAbs_IN ? "inside" : aState == TopAbs_OUT ? "outside" :
                         aState == TopAbs_ON ? "on" : "unknown"}};
    }

json h_projectPointCurve(KernelOperationContext& theContext, const json& theArgs)
{
      double aFirst = 0.0;
      double aLast = 0.0;
      const occ::handle<Geom_Curve> aCurve = curveForEdge(
        theContext.arena().get(requiredU32(theArgs, "shape")), aFirst, aLast);
      const auto aPoint = requiredVec3(theArgs, "point");
      GeomAPI_ProjectPointOnCurve aProjection(
        gp_Pnt(aPoint[0], aPoint[1], aPoint[2]), aCurve, aFirst, aLast);
      json aSolutions = json::array();
      for (int anIndex = 1; anIndex <= aProjection.NbPoints(); ++anIndex)
      {
        const gp_Pnt aProjected = aProjection.Point(anIndex);
        aSolutions.push_back({
          {"parameter", aProjection.Parameter(anIndex)},
          {"point", {aProjected.X(), aProjected.Y(), aProjected.Z()}},
          {"distance", aProjection.Distance(anIndex)}});
      }
      return {{"solutions", aSolutions}};
    }

json h_projectPointSurface(KernelOperationContext& theContext, const json& theArgs)
{
      double aUFirst = 0.0;
      double aULast = 0.0;
      double aVFirst = 0.0;
      double aVLast = 0.0;
      const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
      const occ::handle<Geom_Surface> aSurface = surfaceForFace(
        aShape, aUFirst, aULast, aVFirst, aVLast);
      const TopoDS_Face aFace = TopoDS::Face(aShape);
      const auto aPoint = requiredVec3(theArgs, "point");
      const double aTolerance = theArgs.value("tolerance", 1.0e-7);
      if (aTolerance <= 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs, "Projection tolerance must be positive");
      GeomAPI_ProjectPointOnSurf aProjection(
        gp_Pnt(aPoint[0], aPoint[1], aPoint[2]), aSurface,
        aUFirst, aULast, aVFirst, aVLast, aTolerance);
      json aSolutions = json::array();
      for (int anIndex = 1; anIndex <= aProjection.NbPoints(); ++anIndex)
      {
        const gp_Pnt aProjected = aProjection.Point(anIndex);
        double aU = 0.0;
        double aV = 0.0;
        aProjection.Parameters(anIndex, aU, aV);
        BRepClass_FaceClassifier aClassifier(aFace, gp_Pnt2d(aU, aV), aTolerance);
        if (aClassifier.State() != TopAbs_IN && aClassifier.State() != TopAbs_ON) continue;
        aSolutions.push_back({
          {"u", aU}, {"v", aV},
          {"point", {aProjected.X(), aProjected.Y(), aProjected.Z()}},
          {"distance", aProjection.Distance(anIndex)}});
      }
      return {{"solutions", aSolutions}};
    }

json h_extremaCurveCurve(KernelOperationContext& theContext, const json& theArgs)
{
      double aFirst1 = 0.0;
      double aLast1 = 0.0;
      double aFirst2 = 0.0;
      double aLast2 = 0.0;
      const TopoDS_Shape& aShape1 = theContext.arena().get(requiredU32(theArgs, "first"));
      const TopoDS_Shape& aShape2 = theContext.arena().get(requiredU32(theArgs, "second"));
      const occ::handle<Geom_Curve> aCurve1 = curveForEdge(
        aShape1, aFirst1, aLast1);
      const occ::handle<Geom_Curve> aCurve2 = curveForEdge(
        aShape2, aFirst2, aLast2);
      GeomAPI_ExtremaCurveCurve anExtrema(
        aCurve1, aCurve2, aFirst1, aLast1, aFirst2, aLast2);
      json aSolutions = json::array();
      if (anExtrema.IsParallel())
      {
        BRepExtrema_DistShapeShape aDistance(aShape1, aShape2);
        if (!aDistance.IsDone() || aDistance.NbSolution() == 0)
          throw KernelFailure(ErrorCode::KernelError, "OCCT curve-curve extrema failed");
        for (int anIndex = 1; anIndex <= aDistance.NbSolution(); ++anIndex)
        {
          const gp_Pnt aPoint1 = aDistance.PointOnShape1(anIndex);
          const gp_Pnt aPoint2 = aDistance.PointOnShape2(anIndex);
          GeomAPI_ProjectPointOnCurve aProjection1(aPoint1, aCurve1, aFirst1, aLast1);
          GeomAPI_ProjectPointOnCurve aProjection2(aPoint2, aCurve2, aFirst2, aLast2);
          aSolutions.push_back({
            {"parameter1", aProjection1.LowerDistanceParameter()},
            {"parameter2", aProjection2.LowerDistanceParameter()},
            {"point1", {aPoint1.X(), aPoint1.Y(), aPoint1.Z()}},
            {"point2", {aPoint2.X(), aPoint2.Y(), aPoint2.Z()}},
            {"distance", aPoint1.Distance(aPoint2)}});
        }
      }
      else
      {
        for (int anIndex = 1; anIndex <= anExtrema.NbExtrema(); ++anIndex)
        {
          gp_Pnt aPoint1;
          gp_Pnt aPoint2;
          double aParameter1 = 0.0;
          double aParameter2 = 0.0;
          anExtrema.Points(anIndex, aPoint1, aPoint2);
          anExtrema.Parameters(anIndex, aParameter1, aParameter2);
          aSolutions.push_back({
            {"parameter1", aParameter1}, {"parameter2", aParameter2},
            {"point1", {aPoint1.X(), aPoint1.Y(), aPoint1.Z()}},
            {"point2", {aPoint2.X(), aPoint2.Y(), aPoint2.Z()}},
            {"distance", anExtrema.Distance(anIndex)}});
        }
      }
      return {{"solutions", aSolutions}, {"parallel", anExtrema.IsParallel()}};
    }

json h_extremaCurveSurface(KernelOperationContext& theContext, const json& theArgs)
{
      double aFirst = 0.0;
      double aLast = 0.0;
      double aUFirst = 0.0;
      double aULast = 0.0;
      double aVFirst = 0.0;
      double aVLast = 0.0;
      const TopoDS_Shape& aCurveShape = theContext.arena().get(requiredU32(theArgs, "curve"));
      const TopoDS_Shape& aSurfaceShape = theContext.arena().get(requiredU32(theArgs, "surface"));
      const occ::handle<Geom_Curve> aCurve = curveForEdge(
        aCurveShape, aFirst, aLast);
      const occ::handle<Geom_Surface> aSurface = surfaceForFace(
        aSurfaceShape, aUFirst, aULast, aVFirst, aVLast);
      GeomAPI_ExtremaCurveSurface anExtrema(
        aCurve, aSurface, aFirst, aLast, aUFirst, aULast, aVFirst, aVLast);
      json aSolutions = json::array();
      if (anExtrema.IsParallel())
      {
        BRepExtrema_DistShapeShape aDistance(aCurveShape, aSurfaceShape);
        if (!aDistance.IsDone() || aDistance.NbSolution() == 0)
          throw KernelFailure(ErrorCode::KernelError, "OCCT curve-surface extrema failed");
        for (int anIndex = 1; anIndex <= aDistance.NbSolution(); ++anIndex)
        {
          const gp_Pnt aPointOnCurve = aDistance.PointOnShape1(anIndex);
          const gp_Pnt aPointOnSurface = aDistance.PointOnShape2(anIndex);
          GeomAPI_ProjectPointOnCurve aCurveProjection(aPointOnCurve, aCurve, aFirst, aLast);
          GeomAPI_ProjectPointOnSurf aSurfaceProjection(
            aPointOnSurface, aSurface, aUFirst, aULast, aVFirst, aVLast, 1.0e-7);
          double aU = 0.0;
          double aV = 0.0;
          aSurfaceProjection.LowerDistanceParameters(aU, aV);
          aSolutions.push_back({
            {"curveParameter", aCurveProjection.LowerDistanceParameter()}, {"u", aU}, {"v", aV},
            {"pointOnCurve", {aPointOnCurve.X(), aPointOnCurve.Y(), aPointOnCurve.Z()}},
            {"pointOnSurface", {aPointOnSurface.X(), aPointOnSurface.Y(), aPointOnSurface.Z()}},
            {"distance", aPointOnCurve.Distance(aPointOnSurface)}});
        }
      }
      else
      {
        const TopoDS_Face aFace = TopoDS::Face(aSurfaceShape);
        for (int anIndex = 1; anIndex <= anExtrema.NbExtrema(); ++anIndex)
        {
          gp_Pnt aPointOnCurve;
          gp_Pnt aPointOnSurface;
          double aCurveParameter = 0.0;
          double aU = 0.0;
          double aV = 0.0;
          anExtrema.Points(anIndex, aPointOnCurve, aPointOnSurface);
          anExtrema.Parameters(anIndex, aCurveParameter, aU, aV);
          BRepClass_FaceClassifier aClassifier(aFace, gp_Pnt2d(aU, aV), 1.0e-7);
          if (aClassifier.State() != TopAbs_IN && aClassifier.State() != TopAbs_ON) continue;
          aSolutions.push_back({
            {"curveParameter", aCurveParameter}, {"u", aU}, {"v", aV},
            {"pointOnCurve", {aPointOnCurve.X(), aPointOnCurve.Y(), aPointOnCurve.Z()}},
            {"pointOnSurface", {aPointOnSurface.X(), aPointOnSurface.Y(), aPointOnSurface.Z()}},
            {"distance", anExtrema.Distance(anIndex)}});
        }
      }
      return {{"solutions", aSolutions}, {"parallel", anExtrema.IsParallel()}};
    }

json h_extremaSurfaceSurface(KernelOperationContext& theContext, const json& theArgs)
{
      double aUFirst1 = 0.0;
      double aULast1 = 0.0;
      double aVFirst1 = 0.0;
      double aVLast1 = 0.0;
      double aUFirst2 = 0.0;
      double aULast2 = 0.0;
      double aVFirst2 = 0.0;
      double aVLast2 = 0.0;
      const TopoDS_Shape& aShape1 = theContext.arena().get(requiredU32(theArgs, "first"));
      const TopoDS_Shape& aShape2 = theContext.arena().get(requiredU32(theArgs, "second"));
      const occ::handle<Geom_Surface> aSurface1 = surfaceForFace(
        aShape1, aUFirst1, aULast1, aVFirst1, aVLast1);
      const occ::handle<Geom_Surface> aSurface2 = surfaceForFace(
        aShape2, aUFirst2, aULast2, aVFirst2, aVLast2);
      GeomAPI_ExtremaSurfaceSurface anExtrema(
        aSurface1, aSurface2,
        aUFirst1, aULast1, aVFirst1, aVLast1,
        aUFirst2, aULast2, aVFirst2, aVLast2);
      json aSolutions = json::array();
      if (anExtrema.IsParallel())
      {
        BRepExtrema_DistShapeShape aDistance(aShape1, aShape2);
        if (!aDistance.IsDone() || aDistance.NbSolution() == 0)
          throw KernelFailure(ErrorCode::KernelError, "OCCT surface-surface extrema failed");
        for (int anIndex = 1; anIndex <= aDistance.NbSolution(); ++anIndex)
        {
          const gp_Pnt aPoint1 = aDistance.PointOnShape1(anIndex);
          const gp_Pnt aPoint2 = aDistance.PointOnShape2(anIndex);
          GeomAPI_ProjectPointOnSurf aProjection1(
            aPoint1, aSurface1, aUFirst1, aULast1, aVFirst1, aVLast1, 1.0e-7);
          GeomAPI_ProjectPointOnSurf aProjection2(
            aPoint2, aSurface2, aUFirst2, aULast2, aVFirst2, aVLast2, 1.0e-7);
          double aU1 = 0.0;
          double aV1 = 0.0;
          double aU2 = 0.0;
          double aV2 = 0.0;
          aProjection1.LowerDistanceParameters(aU1, aV1);
          aProjection2.LowerDistanceParameters(aU2, aV2);
          aSolutions.push_back({
            {"u1", aU1}, {"v1", aV1}, {"u2", aU2}, {"v2", aV2},
            {"point1", {aPoint1.X(), aPoint1.Y(), aPoint1.Z()}},
            {"point2", {aPoint2.X(), aPoint2.Y(), aPoint2.Z()}},
            {"distance", aPoint1.Distance(aPoint2)}});
        }
      }
      else
      {
        const TopoDS_Face aFace1 = TopoDS::Face(aShape1);
        const TopoDS_Face aFace2 = TopoDS::Face(aShape2);
        for (int anIndex = 1; anIndex <= anExtrema.NbExtrema(); ++anIndex)
        {
          gp_Pnt aPoint1;
          gp_Pnt aPoint2;
          double aU1 = 0.0;
          double aV1 = 0.0;
          double aU2 = 0.0;
          double aV2 = 0.0;
          anExtrema.Points(anIndex, aPoint1, aPoint2);
          anExtrema.Parameters(anIndex, aU1, aV1, aU2, aV2);
          BRepClass_FaceClassifier aClassifier1(aFace1, gp_Pnt2d(aU1, aV1), 1.0e-7);
          BRepClass_FaceClassifier aClassifier2(aFace2, gp_Pnt2d(aU2, aV2), 1.0e-7);
          if ((aClassifier1.State() != TopAbs_IN && aClassifier1.State() != TopAbs_ON)
              || (aClassifier2.State() != TopAbs_IN && aClassifier2.State() != TopAbs_ON)) continue;
          aSolutions.push_back({
            {"u1", aU1}, {"v1", aV1}, {"u2", aU2}, {"v2", aV2},
            {"point1", {aPoint1.X(), aPoint1.Y(), aPoint1.Z()}},
            {"point2", {aPoint2.X(), aPoint2.Y(), aPoint2.Z()}},
            {"distance", anExtrema.Distance(anIndex)}});
        }
      }
      return {{"solutions", aSolutions}, {"parallel", anExtrema.IsParallel()}};
    }

json h_intersectCurveCurve(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aFirstShape = theContext.arena().get(requiredU32(theArgs, "first"));
      const TopoDS_Shape& aSecondShape = theContext.arena().get(requiredU32(theArgs, "second"));
      if (aFirstShape.ShapeType() != TopAbs_EDGE || aSecondShape.ShapeType() != TopAbs_EDGE)
        throw KernelFailure(ErrorCode::InvalidArgs, "Curve intersection requires edge shapes");
      const double aTolerance = theArgs.value("tolerance", 1.0e-7);
      if (aTolerance <= 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs, "Curve intersection tolerance must be positive");
      BRepAlgoAPI_Section anIntersection(aFirstShape, aSecondShape, false);
      anIntersection.SetNonDestructive(true);
      anIntersection.SetRunParallel(false);
      anIntersection.SetFuzzyValue(aTolerance);
      anIntersection.Approximation(true);
      anIntersection.Build();
      if (anIntersection.HasErrors())
        throw KernelFailure(ErrorCode::KernelError, "OCCT curve-curve intersection failed");
      json aShapes = json::array();
      TopTools_IndexedMapOfShape anEdges;
      TopExp::MapShapes(anIntersection.Shape(), TopAbs_EDGE, anEdges);
      TopTools_IndexedMapOfShape anEdgeVertices;
      for (int anIndex = 1; anIndex <= anEdges.Extent(); ++anIndex)
      {
        const TopoDS_Shape& anEdge = anEdges(anIndex);
        TopExp::MapShapes(anEdge, TopAbs_VERTEX, anEdgeVertices);
        aShapes.push_back(theContext.arena().add(anEdge, aScope));
      }
      TopTools_IndexedMapOfShape allVertices;
      TopExp::MapShapes(anIntersection.Shape(), TopAbs_VERTEX, allVertices);
      for (int anIndex = 1; anIndex <= allVertices.Extent(); ++anIndex)
        if (!anEdgeVertices.Contains(allVertices(anIndex)))
          aShapes.push_back(theContext.arena().add(allVertices(anIndex), aScope));
      return {{"shapes", aShapes}};
    }

json h_intersectCurveSurface(KernelOperationContext& theContext, const json& theArgs)
{
      double aFirst = 0.0;
      double aLast = 0.0;
      double aUFirst = 0.0;
      double aULast = 0.0;
      double aVFirst = 0.0;
      double aVLast = 0.0;
      const TopoDS_Shape& aCurveShape = theContext.arena().get(requiredU32(theArgs, "curve"));
      const TopoDS_Shape& aSurfaceShape = theContext.arena().get(requiredU32(theArgs, "surface"));
      const occ::handle<Geom_Curve> aCurve = curveForEdge(aCurveShape, aFirst, aLast);
      const occ::handle<Geom_Surface> aSurface = surfaceForFace(
        aSurfaceShape, aUFirst, aULast, aVFirst, aVLast);
      const double aTolerance = theArgs.value("tolerance", 1.0e-7);
      if (aTolerance <= 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs, "Curve-surface intersection tolerance must be positive");
      BRepAlgoAPI_Section anIntersection(aCurveShape, aSurfaceShape, false);
      anIntersection.SetNonDestructive(true);
      anIntersection.SetRunParallel(false);
      anIntersection.SetFuzzyValue(aTolerance);
      anIntersection.Approximation(true);
      anIntersection.ComputePCurveOn2(true);
      anIntersection.Build();
      if (anIntersection.HasErrors())
        throw KernelFailure(ErrorCode::KernelError, "OCCT curve-surface intersection failed");

      TopTools_IndexedMapOfShape anEdges;
      TopExp::MapShapes(anIntersection.Shape(), TopAbs_EDGE, anEdges);
      TopTools_IndexedMapOfShape anEdgeVertices;
      for (int anIndex = 1; anIndex <= anEdges.Extent(); ++anIndex)
        TopExp::MapShapes(anEdges(anIndex), TopAbs_VERTEX, anEdgeVertices);

      json aPoints = json::array();
      TopTools_IndexedMapOfShape allVertices;
      TopExp::MapShapes(anIntersection.Shape(), TopAbs_VERTEX, allVertices);
      for (int anIndex = 1; anIndex <= allVertices.Extent(); ++anIndex)
      {
        const TopoDS_Shape& aVertex = allVertices(anIndex);
        if (anEdgeVertices.Contains(aVertex)) continue;
        const gp_Pnt aPoint = BRep_Tool::Pnt(TopoDS::Vertex(aVertex));
        GeomAPI_ProjectPointOnCurve aCurveProjection(aPoint, aCurve, aFirst, aLast);
        GeomAPI_ProjectPointOnSurf aSurfaceProjection(
          aPoint, aSurface, aUFirst, aULast, aVFirst, aVLast, aTolerance);
        if (aCurveProjection.NbPoints() == 0 || aSurfaceProjection.NbPoints() == 0)
          throw KernelFailure(ErrorCode::KernelError, "OCCT failed to parameterize an intersection point");
        double aU = 0.0;
        double aV = 0.0;
        aSurfaceProjection.LowerDistanceParameters(aU, aV);
        aPoints.push_back({
          {"point", {aPoint.X(), aPoint.Y(), aPoint.Z()}},
          {"curveParameter", aCurveProjection.LowerDistanceParameter()}, {"u", aU}, {"v", aV}});
      }
      json aSegments = json::array();
      for (int anIndex = 1; anIndex <= anEdges.Extent(); ++anIndex)
      {
        double aU1 = 0.0;
        double aV1 = 0.0;
        double aU2 = 0.0;
        double aV2 = 0.0;
        const TopoDS_Edge aSectionEdge = TopoDS::Edge(anEdges(anIndex));
        double aPCurveFirst = 0.0;
        double aPCurveLast = 0.0;
        const occ::handle<Geom2d_Curve> aPCurve = BRep_Tool::CurveOnSurface(
          aSectionEdge, TopoDS::Face(aSurfaceShape), aPCurveFirst, aPCurveLast);
        if (!aPCurve.IsNull())
        {
          const gp_Pnt2d aStartUV = aPCurve->Value(aPCurveFirst);
          const gp_Pnt2d anEndUV = aPCurve->Value(aPCurveLast);
          aU1 = aStartUV.X();
          aV1 = aStartUV.Y();
          aU2 = anEndUV.X();
          aV2 = anEndUV.Y();
        }
        else
        {
          double aSegmentFirst = 0.0;
          double aSegmentLast = 0.0;
          const occ::handle<Geom_Curve> aSegment = BRep_Tool::Curve(
            aSectionEdge, aSegmentFirst, aSegmentLast);
          if (aSegment.IsNull()) continue;
          GeomAPI_ProjectPointOnSurf aStartProjection(
            aSegment->Value(aSegmentFirst), aSurface,
            aUFirst, aULast, aVFirst, aVLast, aTolerance);
          GeomAPI_ProjectPointOnSurf anEndProjection(
            aSegment->Value(aSegmentLast), aSurface,
            aUFirst, aULast, aVFirst, aVLast, aTolerance);
          if (aStartProjection.NbPoints() == 0 || anEndProjection.NbPoints() == 0)
            throw KernelFailure(ErrorCode::KernelError, "OCCT failed to parameterize an intersection segment");
          aStartProjection.LowerDistanceParameters(aU1, aV1);
          anEndProjection.LowerDistanceParameters(aU2, aV2);
        }
        aSegments.push_back({{"u1", aU1}, {"v1", aV1}, {"u2", aU2}, {"v2", aV2}});
      }
      return {{"points", aPoints}, {"segments", aSegments}};
    }

json h_intersectSurfaceSurface(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aFirstShape = theContext.arena().get(requiredU32(theArgs, "first"));
      const TopoDS_Shape& aSecondShape = theContext.arena().get(requiredU32(theArgs, "second"));
      if (aFirstShape.ShapeType() != TopAbs_FACE || aSecondShape.ShapeType() != TopAbs_FACE)
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface intersection requires face shapes");
      const double aTolerance = theArgs.value("tolerance", 1.0e-7);
      if (aTolerance <= 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface intersection tolerance must be positive");
      BRepAlgoAPI_Section anIntersection(aFirstShape, aSecondShape, false);
      anIntersection.SetNonDestructive(true);
      anIntersection.SetRunParallel(false);
      anIntersection.SetFuzzyValue(aTolerance);
      anIntersection.Approximation(true);
      anIntersection.ComputePCurveOn1(true);
      anIntersection.ComputePCurveOn2(true);
      anIntersection.Build();
      if (anIntersection.HasErrors())
        throw KernelFailure(ErrorCode::KernelError, "OCCT surface-surface intersection failed");
      json aShapes = json::array();
      TopTools_IndexedMapOfShape anEdges;
      TopExp::MapShapes(anIntersection.Shape(), TopAbs_EDGE, anEdges);
      TopTools_IndexedMapOfShape anEdgeVertices;
      for (int anIndex = 1; anIndex <= anEdges.Extent(); ++anIndex)
      {
        const TopoDS_Shape& anEdge = anEdges(anIndex);
        TopExp::MapShapes(anEdge, TopAbs_VERTEX, anEdgeVertices);
        aShapes.push_back(theContext.arena().add(anEdge, aScope));
      }
      TopTools_IndexedMapOfShape allVertices;
      TopExp::MapShapes(anIntersection.Shape(), TopAbs_VERTEX, allVertices);
      for (int anIndex = 1; anIndex <= allVertices.Extent(); ++anIndex)
        if (!anEdgeVertices.Contains(allVertices(anIndex)))
          aShapes.push_back(theContext.arena().add(allVertices(anIndex), aScope));
      return {{"shapes", aShapes}};
    }

} // namespace occt_worker
