#include "kernel_geometry_helpers.hpp"

namespace occt_worker {

const char* continuityName(const GeomAbs_Shape theContinuity)
{
  switch (theContinuity)
  {
    case GeomAbs_C0: return "c0";
    case GeomAbs_G1: return "g1";
    case GeomAbs_C1: return "c1";
    case GeomAbs_G2: return "g2";
    case GeomAbs_C2: return "c2";
    case GeomAbs_C3: return "c3";
    case GeomAbs_CN: return "cn";
  }
  return "c0";
}

TopAbs_ShapeEnum shapeTypeFromName(const std::string& theName)
{
  if (theName == "compound") return TopAbs_COMPOUND;
  if (theName == "compsolid") return TopAbs_COMPSOLID;
  if (theName == "solid") return TopAbs_SOLID;
  if (theName == "shell") return TopAbs_SHELL;
  if (theName == "face") return TopAbs_FACE;
  if (theName == "wire") return TopAbs_WIRE;
  if (theName == "edge") return TopAbs_EDGE;
  if (theName == "vertex") return TopAbs_VERTEX;
  throw KernelFailure(ErrorCode::InvalidArgs, "Unknown topology type");
}

const char* shapeTypeName(const TopAbs_ShapeEnum theType)
{
  switch (theType)
  {
    case TopAbs_COMPOUND: return "compound";
    case TopAbs_COMPSOLID: return "compsolid";
    case TopAbs_SOLID: return "solid";
    case TopAbs_SHELL: return "shell";
    case TopAbs_FACE: return "face";
    case TopAbs_WIRE: return "wire";
    case TopAbs_EDGE: return "edge";
    case TopAbs_VERTEX: return "vertex";
    case TopAbs_SHAPE: return "shape";
    default: return "unknown";
  }
}

const char* checkStatusName(const BRepCheck_Status theStatus)
{
  switch (theStatus)
  {
    case BRepCheck_NoError: return "noError";
    case BRepCheck_InvalidPointOnCurve: return "invalidPointOnCurve";
    case BRepCheck_InvalidPointOnCurveOnSurface: return "invalidPointOnCurveOnSurface";
    case BRepCheck_InvalidPointOnSurface: return "invalidPointOnSurface";
    case BRepCheck_No3DCurve: return "no3DCurve";
    case BRepCheck_Multiple3DCurve: return "multiple3DCurve";
    case BRepCheck_Invalid3DCurve: return "invalid3DCurve";
    case BRepCheck_NoCurveOnSurface: return "noCurveOnSurface";
    case BRepCheck_InvalidCurveOnSurface: return "invalidCurveOnSurface";
    case BRepCheck_InvalidCurveOnClosedSurface: return "invalidCurveOnClosedSurface";
    case BRepCheck_InvalidSameRangeFlag: return "invalidSameRangeFlag";
    case BRepCheck_InvalidSameParameterFlag: return "invalidSameParameterFlag";
    case BRepCheck_InvalidDegeneratedFlag: return "invalidDegeneratedFlag";
    case BRepCheck_FreeEdge: return "freeEdge";
    case BRepCheck_InvalidMultiConnexity: return "invalidMultiConnexity";
    case BRepCheck_InvalidRange: return "invalidRange";
    case BRepCheck_EmptyWire: return "emptyWire";
    case BRepCheck_RedundantEdge: return "redundantEdge";
    case BRepCheck_SelfIntersectingWire: return "selfIntersectingWire";
    case BRepCheck_NoSurface: return "noSurface";
    case BRepCheck_InvalidWire: return "invalidWire";
    case BRepCheck_RedundantWire: return "redundantWire";
    case BRepCheck_IntersectingWires: return "intersectingWires";
    case BRepCheck_InvalidImbricationOfWires: return "invalidImbricationOfWires";
    case BRepCheck_EmptyShell: return "emptyShell";
    case BRepCheck_RedundantFace: return "redundantFace";
    case BRepCheck_InvalidImbricationOfShells: return "invalidImbricationOfShells";
    case BRepCheck_UnorientableShape: return "unorientableShape";
    case BRepCheck_NotClosed: return "notClosed";
    case BRepCheck_NotConnected: return "notConnected";
    case BRepCheck_SubshapeNotInShape: return "subshapeNotInShape";
    case BRepCheck_BadOrientation: return "badOrientation";
    case BRepCheck_BadOrientationOfSubshape: return "badOrientationOfSubshape";
    case BRepCheck_InvalidPolygonOnTriangulation: return "invalidPolygonOnTriangulation";
    case BRepCheck_InvalidToleranceValue: return "invalidToleranceValue";
    case BRepCheck_EnclosedRegion: return "enclosedRegion";
    case BRepCheck_CheckFail: return "checkFail";
  }
  return "unknown";
}

occ::handle<Geom_Curve> curveForEdge(const TopoDS_Shape& theShape,
                                     double& theFirst,
                                     double& theLast)
{
  if (theShape.ShapeType() != TopAbs_EDGE)
    throw KernelFailure(ErrorCode::InvalidArgs, "Curve analysis requires an edge shape");
  occ::handle<Geom_Curve> aCurve = BRep_Tool::Curve(TopoDS::Edge(theShape), theFirst, theLast);
  if (aCurve.IsNull())
    throw KernelFailure(ErrorCode::InvalidArgs, "Edge has no 3D curve");
  return aCurve;
}

occ::handle<Geom_Surface> surfaceForFace(const TopoDS_Shape& theShape,
                                         double& theUFirst,
                                         double& theULast,
                                         double& theVFirst,
                                         double& theVLast)
{
  if (theShape.ShapeType() != TopAbs_FACE)
    throw KernelFailure(ErrorCode::InvalidArgs, "Surface analysis requires a face shape");
  const TopoDS_Face aFace = TopoDS::Face(theShape);
  occ::handle<Geom_Surface> aSurface = BRep_Tool::Surface(aFace);
  if (aSurface.IsNull())
    throw KernelFailure(ErrorCode::InvalidArgs, "Face has no geometric surface");
  BRepTools::UVBounds(aFace, theUFirst, theULast, theVFirst, theVLast);
  if (!(theUFirst <= theULast && theVFirst <= theVLast))
    throw KernelFailure(ErrorCode::InvalidArgs, "Face has an invalid UV domain");
  return aSurface;
}

TopoDS_Face makeRectangularFace(const occ::handle<Geom_Surface>& theSurface,
                                const double theUFirst,
                                const double theULast,
                                const double theVFirst,
                                const double theVLast,
                                const double theTolerance,
                                const TopAbs_Orientation theOrientation)
{
  if (theSurface.IsNull() || !std::isfinite(theUFirst) || !std::isfinite(theULast)
      || !std::isfinite(theVFirst) || !std::isfinite(theVLast)
      || theUFirst >= theULast || theVFirst >= theVLast || theTolerance <= 0.0)
    throw KernelFailure(ErrorCode::InvalidArgs,
                        "Surface bounds must be increasing, finite, and tolerance positive");
  BRepBuilderAPI_MakeFace aBuilder(
    theSurface, theUFirst, theULast, theVFirst, theVLast, theTolerance);
  if (!aBuilder.IsDone() || aBuilder.Face().IsNull()
      || !BRepCheck_Analyzer(aBuilder.Face(), true).IsValid())
    throw KernelFailure(ErrorCode::ConstructionFailed,
                        "OCCT failed to construct a valid bounded surface face");
  TopoDS_Face aFace = aBuilder.Face();
  aFace.Orientation(theOrientation);
  return aFace;
}

TopoDS_Face makeSurfaceFacePreservingWires(
  const TopoDS_Face& theSourceFace,
  const occ::handle<Geom_Surface>& theSurface,
  const double theTolerance,
  const TopAbs_Orientation theOrientation,
  const std::array<double, 4>* theOuterBounds)
{
  if (theSurface.IsNull() || theTolerance <= 0.0)
    throw KernelFailure(ErrorCode::InvalidArgs,
                        "Surface bounds must be valid and tolerance positive");

  BRepBuilderAPI_Copy aCopy(theSourceFace, true, false);
  if (!aCopy.IsDone() || aCopy.Shape().IsNull())
    throw KernelFailure(ErrorCode::ConstructionFailed,
                        "OCCT failed to copy the surface face topology");
  const TopoDS_Face aCopiedFace = TopoDS::Face(aCopy.Shape());

  BRep_Builder aShapeBuilder;
  for (TopExp_Explorer anExplorer(aCopiedFace, TopAbs_EDGE);
       anExplorer.More(); anExplorer.Next())
  {
    const TopoDS_Edge anEdge = TopoDS::Edge(anExplorer.Current());
    double aFirst = 0.0;
    double aLast = 0.0;
    if (BRep_Tool::IsClosed(anEdge, aCopiedFace))
    {
      occ::handle<Geom2d_Curve> aFirstPCurve;
      occ::handle<Geom2d_Curve> aSecondPCurve;
      occ::handle<Geom_Surface> anOldSurface;
      TopLoc_Location anOldLocation;
      BRep_Tool::CurveOnSurface(anEdge, aFirstPCurve, anOldSurface,
                                anOldLocation, aFirst, aLast, 1);
      BRep_Tool::CurveOnSurface(anEdge, aSecondPCurve, anOldSurface,
                                anOldLocation, aFirst, aLast, 2);
      if (aFirstPCurve.IsNull() || aSecondPCurve.IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed,
                            "OCCT could not preserve a closed-surface boundary pcurve");
      aShapeBuilder.UpdateEdge(anEdge, aFirstPCurve, aSecondPCurve, theSurface,
                               TopLoc_Location(), theTolerance);
    }
    else
    {
      const occ::handle<Geom2d_Curve> aPCurve =
        BRep_Tool::CurveOnSurface(anEdge, aCopiedFace, aFirst, aLast);
      if (aPCurve.IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed,
                            "OCCT could not preserve a surface boundary pcurve");
      aShapeBuilder.UpdateEdge(anEdge, aPCurve, theSurface,
                               TopLoc_Location(), theTolerance);
    }
    aShapeBuilder.UpdateEdge(anEdge, occ::handle<Geom_Curve>(), theTolerance);
    if (!BRepLib::BuildCurve3d(anEdge, theTolerance))
      throw KernelFailure(ErrorCode::ConstructionFailed,
                          "OCCT could not rebuild a surface boundary curve");
    BRepLib::SameParameter(anEdge, theTolerance);
  }

  const TopoDS_Wire anOuter = BRepTools::OuterWire(aCopiedFace);
  if (anOuter.IsNull())
    throw KernelFailure(ErrorCode::ConstructionFailed, "Surface face has no outer wire");
  TopoDS_Face aResult;
  if (theOuterBounds != nullptr)
  {
    BRepBuilderAPI_MakeFace aBuilder(theSurface, (*theOuterBounds)[0], (*theOuterBounds)[1],
                                     (*theOuterBounds)[2], (*theOuterBounds)[3], theTolerance);
    if (!aBuilder.IsDone() || aBuilder.Face().IsNull())
      throw KernelFailure(ErrorCode::ConstructionFailed,
                          "OCCT failed to rebuild the extended surface boundary");
    aResult = aBuilder.Face();
  }
  else
  {
    BRepBuilderAPI_MakeFace aBuilder(theSurface, anOuter, true);
    if (!aBuilder.IsDone() || aBuilder.Face().IsNull())
      throw KernelFailure(ErrorCode::ConstructionFailed,
                          "OCCT failed to rebuild the surface face boundary");
    aResult = aBuilder.Face();
  }
  BRep_Builder aResultBuilder;
  for (TopExp_Explorer anExplorer(aCopiedFace, TopAbs_WIRE);
       anExplorer.More(); anExplorer.Next())
  {
    const TopoDS_Wire aWire = TopoDS::Wire(anExplorer.Current());
    if (!aWire.IsSame(anOuter))
      aResultBuilder.Add(aResult, aWire);
  }
  ShapeFix_Edge aEdgeFixer;
  for (TopExp_Explorer anExplorer(aResult, TopAbs_EDGE);
       anExplorer.More(); anExplorer.Next())
  {
    const TopoDS_Edge anEdge = TopoDS::Edge(anExplorer.Current());
    aEdgeFixer.FixVertexTolerance(anEdge, aResult);
    aEdgeFixer.FixSameParameter(anEdge, theTolerance);
  }
  if (!BRepCheck_Analyzer(aResult, true).IsValid())
    throw KernelFailure(ErrorCode::ConstructionFailed,
                        "OCCT failed to validate the rebuilt surface face");
  aResult.Orientation(theOrientation);
  return aResult;
}

} // namespace occt_worker
