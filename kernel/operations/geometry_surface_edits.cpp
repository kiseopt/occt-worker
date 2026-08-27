// Focused geometry/topology operation handlers.

#include "kernel_geometry_topology_operations.hpp"
#include "kernel_geometry_helpers.hpp"
#include "kernel_operation_context.hpp"
#include "kernel_protocol_helpers.hpp"

namespace occt_worker {

json h_trimSurface(KernelOperationContext& theContext, const json& theArgs)
{
  const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
  const TopoDS_Shape& aShape =
    theContext.arena().get(requiredU32(theArgs, "shape"));
  if (aShape.ShapeType() != TopAbs_FACE)
    throw KernelFailure(ErrorCode::InvalidArgs, "Surface editing requires a face shape");
  const TopoDS_Face aFace = TopoDS::Face(aShape);
  const occ::handle<Geom_Surface> aSurface = BRep_Tool::Surface(aFace);
  if (aSurface.IsNull())
    throw KernelFailure(ErrorCode::InvalidArgs, "Face has no geometric surface");
  BRepAdaptor_Surface anAdaptor(aFace, true);
  const double aUFirst = requiredNumber(theArgs, "uFirst");
  const double aULast = requiredNumber(theArgs, "uLast");
  const double aVFirst = requiredNumber(theArgs, "vFirst");
  const double aVLast = requiredNumber(theArgs, "vLast");
  const double aTolerance = theArgs.value("tolerance", 1.0e-7);
  if (aTolerance <= 0.0)
    throw KernelFailure(ErrorCode::InvalidArgs, "Surface editing tolerance must be positive");
  if (!std::isfinite(aUFirst) || !std::isfinite(aULast) || !std::isfinite(aVFirst)
      || !std::isfinite(aVLast) || aUFirst >= aULast || aVFirst >= aVLast
      || aUFirst < anAdaptor.FirstUParameter() || aULast > anAdaptor.LastUParameter()
      || aVFirst < anAdaptor.FirstVParameter() || aVLast > anAdaptor.LastVParameter())
    throw KernelFailure(ErrorCode::InvalidArgs,
                        "Surface trim must be an increasing finite subset of the face UV domain");
  const TopoDS_Wire anOuter = BRepTools::OuterWire(aFace);
  int aWireCount = 0;
  int anOuterEdgeCount = 0;
  for (TopExp_Explorer anExplorer(aFace, TopAbs_WIRE); anExplorer.More(); anExplorer.Next())
  {
    ++aWireCount;
    if (TopoDS::Wire(anExplorer.Current()).IsSame(anOuter))
    {
      for (TopExp_Explorer anEdgeExplorer(anExplorer.Current(), TopAbs_EDGE);
           anEdgeExplorer.More(); anEdgeExplorer.Next())
        ++anOuterEdgeCount;
    }
  }
  if (aWireCount != 1 || anOuter.IsNull() || anOuterEdgeCount != 4)
    throw KernelFailure(ErrorCode::InvalidArgs,
                        "Surface trim requires a single four-edge face boundary");
  const TopoDS_Face aTrimmedFace = makeRectangularFace(
    aSurface, aUFirst, aULast, aVFirst, aVLast, aTolerance, aFace.Orientation());
  return {{"shape", theContext.arena().add(aTrimmedFace, aScope)}};
}

json h_updateSurfacePole(KernelOperationContext& theContext,
                         const json& theArgs)
{
  const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
  const TopoDS_Shape& aShape =
    theContext.arena().get(requiredU32(theArgs, "shape"));
  if (aShape.ShapeType() != TopAbs_FACE)
    throw KernelFailure(ErrorCode::InvalidArgs, "Surface editing requires a face shape");
  const TopoDS_Face aFace = TopoDS::Face(aShape);
  const occ::handle<Geom_Surface> aSurface = BRep_Tool::Surface(aFace);
  if (aSurface.IsNull())
    throw KernelFailure(ErrorCode::InvalidArgs, "Face has no geometric surface");
  const double aTolerance = theArgs.value("tolerance", 1.0e-7);
  if (aTolerance <= 0.0)
    throw KernelFailure(ErrorCode::InvalidArgs, "Surface editing tolerance must be positive");
  const std::uint32_t anUIndex = requiredU32(theArgs, "uIndex");
  const std::uint32_t anVIndex = requiredU32(theArgs, "vIndex");
  const auto aPoint = requiredVec3(theArgs, "point");
  occ::handle<Geom_Surface> aCopySurface;
  if (aSurface->IsKind(STANDARD_TYPE(Geom_BezierSurface)))
  {
    const occ::handle<Geom_BezierSurface> aBezier =
      occ::down_cast<Geom_BezierSurface>(aSurface->Copy());
    if (anUIndex >= static_cast<std::uint32_t>(aBezier->NbUPoles())
        || anVIndex >= static_cast<std::uint32_t>(aBezier->NbVPoles()))
      throw KernelFailure(ErrorCode::InvalidArgs, "Surface pole index is out of range");
    if (theArgs.contains("weight"))
    {
      const double aWeight = requiredNumber(theArgs, "weight");
      if (aWeight <= 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface pole weight must be positive");
      aBezier->SetPole(static_cast<int>(anUIndex + 1), static_cast<int>(anVIndex + 1),
                       gp_Pnt(aPoint[0], aPoint[1], aPoint[2]), aWeight);
    }
    else
    {
      aBezier->SetPole(static_cast<int>(anUIndex + 1), static_cast<int>(anVIndex + 1),
                       gp_Pnt(aPoint[0], aPoint[1], aPoint[2]));
    }
    aCopySurface = aBezier;
  }
  else if (aSurface->IsKind(STANDARD_TYPE(Geom_BSplineSurface)))
  {
    const occ::handle<Geom_BSplineSurface> aBSpline =
      occ::down_cast<Geom_BSplineSurface>(aSurface->Copy());
    if (anUIndex >= static_cast<std::uint32_t>(aBSpline->NbUPoles())
        || anVIndex >= static_cast<std::uint32_t>(aBSpline->NbVPoles()))
      throw KernelFailure(ErrorCode::InvalidArgs, "Surface pole index is out of range");
    if (theArgs.contains("weight"))
    {
      const double aWeight = requiredNumber(theArgs, "weight");
      if (aWeight <= 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface pole weight must be positive");
      aBSpline->SetPole(static_cast<int>(anUIndex + 1), static_cast<int>(anVIndex + 1),
                        gp_Pnt(aPoint[0], aPoint[1], aPoint[2]), aWeight);
    }
    else
    {
      aBSpline->SetPole(static_cast<int>(anUIndex + 1), static_cast<int>(anVIndex + 1),
                        gp_Pnt(aPoint[0], aPoint[1], aPoint[2]));
    }
    aCopySurface = aBSpline;
  }
  else
  {
    throw KernelFailure(ErrorCode::InvalidArgs,
                        "Surface pole editing is available only for Bezier and BSpline faces");
  }
  const TopoDS_Face anEditedFace = makeSurfaceFacePreservingWires(
    aFace, aCopySurface, aTolerance, aFace.Orientation());
  return {{"shape", theContext.arena().add(anEditedFace, aScope)}};
}

json h_convertSurfaceToBSpline(KernelOperationContext& theContext,
                               const json& theArgs)
{
  const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
  const TopoDS_Shape& aShape =
    theContext.arena().get(requiredU32(theArgs, "shape"));
  if (aShape.ShapeType() != TopAbs_FACE)
    throw KernelFailure(ErrorCode::InvalidArgs, "Surface editing requires a face shape");
  const TopoDS_Face aFace = TopoDS::Face(aShape);
  const occ::handle<Geom_Surface> aSurface = BRep_Tool::Surface(aFace);
  if (aSurface.IsNull())
    throw KernelFailure(ErrorCode::InvalidArgs, "Face has no geometric surface");
  BRepAdaptor_Surface anAdaptor(aFace, true);
  const double aTolerance = theArgs.value("tolerance", 1.0e-7);
  if (aTolerance <= 0.0)
    throw KernelFailure(ErrorCode::InvalidArgs, "Surface editing tolerance must be positive");
  occ::handle<Geom_BSplineSurface> aBSpline;
  if (aSurface->IsKind(STANDARD_TYPE(Geom_BSplineSurface)))
  {
    aBSpline = GeomConvert::SplitBSplineSurface(
      occ::down_cast<Geom_BSplineSurface>(aSurface), anAdaptor.FirstUParameter(),
      anAdaptor.LastUParameter(), anAdaptor.FirstVParameter(),
      anAdaptor.LastVParameter(), aTolerance, true, true);
  }
  else
  {
    const occ::handle<Geom_Surface> aTrimmedSurface = new Geom_RectangularTrimmedSurface(
      aSurface, anAdaptor.FirstUParameter(), anAdaptor.LastUParameter(),
      anAdaptor.FirstVParameter(), anAdaptor.LastVParameter());
    aBSpline = GeomConvert::SurfaceToBSplineSurface(aTrimmedSurface);
  }
  if (aBSpline.IsNull())
    throw KernelFailure(ErrorCode::ConstructionFailed,
                        "OCCT failed to convert the surface to BSpline");
  const TopoDS_Face aConvertedFace = makeSurfaceFacePreservingWires(
    aFace, aBSpline, aTolerance, aFace.Orientation());
  return {{"shape", theContext.arena().add(aConvertedFace, aScope)}};
}

} // namespace occt_worker
