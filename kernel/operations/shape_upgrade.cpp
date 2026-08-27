#include "kernel_algorithms_operations.hpp"
#include "kernel_operation_context.hpp"
#include "kernel_protocol_helpers.hpp"

namespace occt_worker {
namespace {

double readPositive(const json& theArgs, const char* theName, const double theDefault)
{
  const double aValue = theArgs.value(theName, theDefault);
  if (!std::isfinite(aValue) || aValue <= 0.0)
  {
    throw KernelFailure(ErrorCode::InvalidArgs,
                        std::string("shapeUpgrade ") + theName + " must be positive and finite");
  }
  return aValue;
}

int readPositiveInt(const json& theArgs, const char* theName)
{
  const std::uint32_t aValue = requiredU32(theArgs, theName);
  if (aValue == 0 || aValue > static_cast<std::uint32_t>(std::numeric_limits<int>::max()))
  {
    throw KernelFailure(ErrorCode::InvalidArgs,
                        std::string("shapeUpgrade ") + theName + " must be a positive integer");
  }
  return static_cast<int>(aValue);
}

GeomAbs_Shape readCriterion(const json& theArgs,
                            const char* theName,
                            const GeomAbs_Shape theDefault)
{
  if (!theArgs.contains(theName)) return theDefault;
  const std::string aValue = theArgs.at(theName).get<std::string>();
  if (aValue == "c0") return GeomAbs_C0;
  if (aValue == "c1") return GeomAbs_C1;
  if (aValue == "c2") return GeomAbs_C2;
  if (aValue == "c3") return GeomAbs_C3;
  if (aValue == "cn") return GeomAbs_CN;
  throw KernelFailure(ErrorCode::InvalidArgs,
                      std::string("shapeUpgrade ") + theName
                        + " must be c0, c1, c2, c3, or cn");
}

void configureDivide(ShapeUpgrade_ShapeDivide& theBuilder, const json& theArgs)
{
  const double aMinTolerance = readPositive(theArgs, "minTolerance", Precision::Confusion());
  const double aMaxTolerance = readPositive(theArgs, "maxTolerance", 1.0e-2);
  if (aMinTolerance > aMaxTolerance)
  {
    throw KernelFailure(ErrorCode::InvalidArgs,
                        "shapeUpgrade minTolerance must not exceed maxTolerance");
  }
  const int anEdgeMode = theArgs.value("edgeMode", 0);
  if (anEdgeMode < 0 || anEdgeMode > 2)
  {
    throw KernelFailure(ErrorCode::InvalidArgs,
                        "shapeUpgrade edgeMode must be 0, 1, or 2");
  }
  theBuilder.SetPrecision(readPositive(theArgs, "precision", Precision::Confusion()));
  theBuilder.SetMinTolerance(aMinTolerance);
  theBuilder.SetMaxTolerance(aMaxTolerance);
  theBuilder.SetSurfaceSegmentMode(theArgs.value("surfaceSegmentMode", false));
  theBuilder.SetEdgeMode(anEdgeMode);
}

TopoDS_Shape performDivide(ShapeUpgrade_ShapeDivide& theBuilder,
                           const json& theArgs,
                           const char* theDescription)
{
  configureDivide(theBuilder, theArgs);
  theBuilder.Perform(true);
  const TopoDS_Shape aResult = theBuilder.Result();
  if (theBuilder.Status(ShapeExtend_FAIL) || aResult.IsNull())
  {
    throw KernelFailure(ErrorCode::HealingFailed,
                        std::string("OCCT ") + theDescription + " produced no result");
  }
  return aResult;
}

json addResult(KernelOperationContext& theContext,
               const std::uint32_t theScope,
               const TopoDS_Shape& theShape)
{
  return {{"shape", theContext.arena().add(theShape, theScope)}};
}

json upgradeContinuity(KernelOperationContext& theContext,
                       const json& theArgs,
                       const TopoDS_Shape& theShape,
                       const std::uint32_t theScope)
{
  ShapeUpgrade_ShapeDivideContinuity aBuilder(theShape);
  aBuilder.SetTolerance(readPositive(theArgs, "tolerance", Precision::Confusion()));
  aBuilder.SetTolerance2d(readPositive(theArgs, "tolerance2d", Precision::PConfusion()));
  aBuilder.SetBoundaryCriterion(readCriterion(theArgs, "boundaryCriterion", GeomAbs_C1));
  aBuilder.SetPCurveCriterion(readCriterion(theArgs, "pcurveCriterion", GeomAbs_C1));
  aBuilder.SetSurfaceCriterion(readCriterion(theArgs, "surfaceCriterion", GeomAbs_C1));
  return addResult(theContext, theScope,
                   performDivide(aBuilder, theArgs, "continuity shape upgrade"));
}

json upgradeAngle(KernelOperationContext& theContext,
                  const json& theArgs,
                  const TopoDS_Shape& theShape,
                  const std::uint32_t theScope)
{
  const double aMaxAngle = readPositive(theArgs, "maxAngle", 0.0);
  if (aMaxAngle > 2.0 * std::acos(-1.0))
  {
    throw KernelFailure(ErrorCode::InvalidArgs,
                        "shapeUpgrade maxAngle must not exceed 2*pi radians");
  }
  ShapeUpgrade_ShapeDivideAngle aBuilder(aMaxAngle, theShape);
  return addResult(theContext, theScope,
                   performDivide(aBuilder, theArgs, "angle shape upgrade"));
}

json upgradeArea(KernelOperationContext& theContext,
                 const json& theArgs,
                 const TopoDS_Shape& theShape,
                 const std::uint32_t theScope)
{
  ShapeUpgrade_ShapeDivideArea aBuilder(theShape);
  const std::string anAreaMode = theArgs.value("areaMode", "");
  if (anAreaMode == "maxArea")
  {
    aBuilder.MaxArea() = readPositive(theArgs, "maxArea", 0.0);
  }
  else if (anAreaMode == "parts")
  {
    const int aPartCount = readPositiveInt(theArgs, "nbParts");
    if (aPartCount < 2)
      throw KernelFailure(ErrorCode::InvalidArgs, "shapeUpgrade nbParts must be at least 2");
    aBuilder.SetSplittingByNumber(true);
    aBuilder.NbParts() = aPartCount;
  }
  else if (anAreaMode == "uv")
  {
    aBuilder.SetNumbersUVSplits(readPositiveInt(theArgs, "uSplits"),
                                readPositiveInt(theArgs, "vSplits"));
  }
  else
  {
    throw KernelFailure(ErrorCode::InvalidArgs,
                        "shapeUpgrade areaMode must be maxArea, parts, or uv");
  }
  return addResult(theContext, theScope,
                   performDivide(aBuilder, theArgs, "area shape upgrade"));
}

json upgradeClosedFaces(KernelOperationContext& theContext,
                        const json& theArgs,
                        const TopoDS_Shape& theShape,
                        const std::uint32_t theScope)
{
  ShapeUpgrade_ShapeDivideClosed aBuilder(theShape);
  if (theArgs.contains("splitPoints"))
    aBuilder.SetNbSplitPoints(readPositiveInt(theArgs, "splitPoints"));
  return addResult(theContext, theScope,
                   performDivide(aBuilder, theArgs, "closed-face shape upgrade"));
}

json upgradeClosedEdges(KernelOperationContext& theContext,
                        const json& theArgs,
                        const TopoDS_Shape& theShape,
                        const std::uint32_t theScope)
{
  ShapeUpgrade_ShapeDivideClosedEdges aBuilder(theShape);
  return addResult(theContext, theScope,
                   performDivide(aBuilder, theArgs, "closed-edge shape upgrade"));
}

json convertToBezier(KernelOperationContext& theContext,
                     const json& theArgs,
                     const TopoDS_Shape& theShape,
                     const std::uint32_t theScope)
{
  const bool aConvert2d = theArgs.value("convert2d", true);
  const bool aConvert3d = theArgs.value("convert3d", true);
  const bool aConvertSurfaces = theArgs.value("convertSurfaces", true);
  if (!aConvert2d && !aConvert3d && !aConvertSurfaces)
  {
    throw KernelFailure(ErrorCode::InvalidArgs,
                        "shapeUpgrade convertToBezier requires at least one conversion mode");
  }
  ShapeUpgrade_ShapeConvertToBezier aBuilder(theShape);
  aBuilder.Set2dConversion(aConvert2d);
  aBuilder.Set3dConversion(aConvert3d);
  aBuilder.SetSurfaceConversion(aConvertSurfaces);
  aBuilder.Set3dLineConversion(theArgs.value("convertLines", true));
  aBuilder.Set3dCircleConversion(theArgs.value("convertCircles", true));
  aBuilder.Set3dConicConversion(theArgs.value("convertConics", true));
  aBuilder.SetPlaneMode(theArgs.value("convertPlanes", true));
  aBuilder.SetRevolutionMode(theArgs.value("convertRevolutions", true));
  aBuilder.SetExtrusionMode(theArgs.value("convertExtrusions", true));
  aBuilder.SetBSplineMode(theArgs.value("convertBSplines", true));
  return addResult(theContext, theScope,
                   performDivide(aBuilder, theArgs, "Bezier conversion"));
}

json removeInternalWires(KernelOperationContext& theContext,
                         const json& theArgs,
                         const TopoDS_Shape& theShape,
                         const std::uint32_t theScope)
{
  ShapeUpgrade_RemoveInternalWires aBuilder(theShape);
  aBuilder.MinArea() = readPositive(theArgs, "minArea", 0.0);
  aBuilder.RemoveFaceMode() = theArgs.value("removeFaces", true);
  aBuilder.Perform();
  const TopoDS_Shape aResult = aBuilder.GetResult();
  if (aBuilder.Status(ShapeExtend_FAIL) || aResult.IsNull())
  {
    throw KernelFailure(ErrorCode::HealingFailed,
                        "OCCT internal-wire removal produced no result");
  }
  return addResult(theContext, theScope, aResult);
}

json removeLocations(KernelOperationContext& theContext,
                     const json& theArgs,
                     const TopoDS_Shape& theShape,
                     const std::uint32_t theScope)
{
  const std::string aLevel = theArgs.value("removeLevel", "shape");
  TopAbs_ShapeEnum aRemoveLevel = TopAbs_SHAPE;
  if (aLevel == "compound") aRemoveLevel = TopAbs_COMPOUND;
  else if (aLevel == "solid") aRemoveLevel = TopAbs_SOLID;
  else if (aLevel == "shell") aRemoveLevel = TopAbs_SHELL;
  else if (aLevel == "face") aRemoveLevel = TopAbs_FACE;
  else if (aLevel != "shape")
  {
    throw KernelFailure(ErrorCode::InvalidArgs,
                        "shapeUpgrade removeLevel must be shape, compound, solid, shell, or face");
  }
  ShapeUpgrade_RemoveLocations aBuilder;
  aBuilder.SetRemoveLevel(aRemoveLevel);
  aBuilder.Remove(theShape);
  const TopoDS_Shape aResult = aBuilder.GetResult();
  if (aResult.IsNull())
    throw KernelFailure(ErrorCode::HealingFailed, "OCCT location removal produced no result");
  return addResult(theContext, theScope, aResult);
}

} // namespace

json h_shapeUpgrade(KernelOperationContext& theContext, const json& theArgs)
{
  const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
  const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
  const std::string aMode = theArgs.value("mode", "");
  if (aMode == "continuity") return upgradeContinuity(theContext, theArgs, aShape, aScope);
  if (aMode == "angle") return upgradeAngle(theContext, theArgs, aShape, aScope);
  if (aMode == "area") return upgradeArea(theContext, theArgs, aShape, aScope);
  if (aMode == "closedFaces") return upgradeClosedFaces(theContext, theArgs, aShape, aScope);
  if (aMode == "closedEdges") return upgradeClosedEdges(theContext, theArgs, aShape, aScope);
  if (aMode == "convertToBezier") return convertToBezier(theContext, theArgs, aShape, aScope);
  if (aMode == "removeInternalWires") return removeInternalWires(theContext, theArgs, aShape, aScope);
  if (aMode == "removeLocations") return removeLocations(theContext, theArgs, aShape, aScope);
  throw KernelFailure(ErrorCode::InvalidArgs, "shapeUpgrade mode is not supported");
}

} // namespace occt_worker
