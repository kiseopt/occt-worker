// Focused geometry/topology operation handlers.

#include "kernel_geometry_topology_operations.hpp"
#include "kernel_geometry_helpers.hpp"
#include "kernel_operation_context.hpp"
#include "kernel_protocol_helpers.hpp"

namespace occt_worker {

json h_makeSurfaceExtrusion(KernelOperationContext& theContext,
                            const json& theArgs)
{
  const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
  const TopoDS_Shape& aProfile =
    theContext.arena().get(requiredU32(theArgs, "profile"));
  double aCurveFirst = 0.0;
  double aCurveLast = 0.0;
  const occ::handle<Geom_Curve> aCurve =
    curveForEdge(aProfile, aCurveFirst, aCurveLast);
  const occ::handle<Geom_TrimmedCurve> aProfileCurve =
    new Geom_TrimmedCurve(aCurve, aCurveFirst, aCurveLast);
  const auto aDirection = requiredDirection(theArgs, "direction");
  const double aVFirst = requiredNumber(theArgs, "vFirst");
  const double aVLast = requiredNumber(theArgs, "vLast");
  const double aTolerance = theArgs.value("tolerance", 1.0e-7);
  const occ::handle<Geom_SurfaceOfLinearExtrusion> aSurface =
    new Geom_SurfaceOfLinearExtrusion(
      aProfileCurve, gp_Dir(aDirection[0], aDirection[1], aDirection[2]));
  const TopoDS_Face aFace = makeRectangularFace(
    aSurface, aCurveFirst, aCurveLast, aVFirst, aVLast, aTolerance,
    TopAbs_FORWARD);
  return {{"shape", theContext.arena().add(aFace, aScope)}};
}

json h_makeSurfaceRevolution(KernelOperationContext& theContext,
                             const json& theArgs)
{
  const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
  const TopoDS_Shape& aProfile =
    theContext.arena().get(requiredU32(theArgs, "profile"));
  double aCurveFirst = 0.0;
  double aCurveLast = 0.0;
  const occ::handle<Geom_Curve> aCurve =
    curveForEdge(aProfile, aCurveFirst, aCurveLast);
  const occ::handle<Geom_TrimmedCurve> aProfileCurve =
    new Geom_TrimmedCurve(aCurve, aCurveFirst, aCurveLast);
  const auto anOrigin = optionalVec3(theArgs, "origin", {0.0, 0.0, 0.0});
  const auto aDirection =
    optionalDirection(theArgs, "direction", {0.0, 0.0, 1.0});
  const double anAngle = theArgs.value("angle", 6.28318530717958647693);
  if (anAngle <= 0.0 || anAngle > 6.28318530717958647693)
    throw KernelFailure(ErrorCode::InvalidArgs,
                        "Revolution surface angle must be in (0, 2*pi]");
  const double aTolerance = theArgs.value("tolerance", 1.0e-7);
  const occ::handle<Geom_SurfaceOfRevolution> aSurface =
    new Geom_SurfaceOfRevolution(
      aProfileCurve,
      gp_Ax1(gp_Pnt(anOrigin[0], anOrigin[1], anOrigin[2]),
             gp_Dir(aDirection[0], aDirection[1], aDirection[2])));
  const TopoDS_Face aFace = makeRectangularFace(
    aSurface, 0.0, anAngle, aCurveFirst, aCurveLast, aTolerance,
    TopAbs_FORWARD);
  return {{"shape", theContext.arena().add(aFace, aScope)}};
}

} // namespace occt_worker
