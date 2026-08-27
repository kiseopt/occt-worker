// Kernel modeling operations: one out-of-line ModelingOperations::h_* handler per dispatch branch.
// Handler bodies are byte-identical fragments of the original dispatchOperation.
// As a shared Side module the constructor registers into the Main registry
// right after Emscripten runs __wasm_call_ctors.

#include "kernel_modeling_operations.hpp"
#include "kernel_history_helpers.hpp"
#include "kernel_operation_context.hpp"
#include "kernel_protocol_helpers.hpp"
#include "side_registration.hpp"

namespace occt_worker {

json h_makeBox(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const auto aSize = requiredVec3(theArgs, "size");
      const auto anOrigin = optionalVec3(theArgs, "origin", {0.0, 0.0, 0.0});
      if (aSize[0] <= 0.0 || aSize[1] <= 0.0 || aSize[2] <= 0.0)
      {
        throw KernelFailure(ErrorCode::InvalidArgs, "Box dimensions must be positive");
      }
      BRepPrimAPI_MakeBox aBuilder(gp_Pnt(anOrigin[0], anOrigin[1], anOrigin[2]),
                                  aSize[0], aSize[1], aSize[2]);
      aBuilder.Build();
      if (!aBuilder.IsDone())
      {
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct box");
      }
      return {{"shape", theContext.arena().add(aBuilder.Shape(), aScope)}};
    }

namespace {
enum class AggregateMode { Compound, Shell, CompSolid };

json makeAggregate(KernelOperationContext& theContext, const json& theArgs, AggregateMode theMode)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const char* aKey = theMode == AggregateMode::Compound ? "shapes"
        : (theMode == AggregateMode::Shell ? "faces" : "solids");
      if (!theArgs.contains(aKey) || !theArgs.at(aKey).is_array() || theArgs.at(aKey).empty())
        throw KernelFailure(ErrorCode::InvalidArgs, std::string(aKey) + " must contain at least one shape");
      BRep_Builder aBuilder;
      TopoDS_Shape aResult;
      if (theMode == AggregateMode::Compound)
      {
        TopoDS_Compound aCompound;
        aBuilder.MakeCompound(aCompound);
        aResult = aCompound;
      }
      else if (theMode == AggregateMode::Shell)
      {
        TopoDS_Shell aShell;
        aBuilder.MakeShell(aShell);
        aResult = aShell;
      }
      else
      {
        TopoDS_CompSolid aCompSolid;
        aBuilder.MakeCompSolid(aCompSolid);
        aResult = aCompSolid;
      }
      for (const json& aHandle : theArgs.at(aKey))
      {
        if ((!aHandle.is_number_integer() && !aHandle.is_number_unsigned())
            || aHandle.get<std::int64_t>() < 0 || aHandle.get<std::uint64_t>() > UINT32_MAX)
          throw KernelFailure(ErrorCode::InvalidArgs, std::string(aKey) + " must contain shape handles");
        const TopoDS_Shape& aShape = theContext.arena().get(aHandle.get<std::uint32_t>());
        if (theMode == AggregateMode::Shell && aShape.ShapeType() != TopAbs_FACE)
          throw KernelFailure(ErrorCode::InvalidArgs, "makeShell accepts faces only");
        if (theMode == AggregateMode::CompSolid && aShape.ShapeType() != TopAbs_SOLID)
          throw KernelFailure(ErrorCode::InvalidArgs, "makeCompSolid accepts solids only");
        aBuilder.Add(aResult, aShape);
      }
      return {{"shape", theContext.arena().add(aResult, aScope)}};
    }

enum class RoundedPrimitiveMode { Sphere, Cone, Torus };

json makeRoundedPrimitive(KernelOperationContext& theContext, const json& theArgs,
                          RoundedPrimitiveMode theMode)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const auto anOrigin = optionalVec3(theArgs, "origin", {0.0, 0.0, 0.0});
      const auto aDirection = optionalDirection(theArgs, "direction", {0.0, 0.0, 1.0});
      const gp_Ax2 anAxis(gp_Pnt(anOrigin[0], anOrigin[1], anOrigin[2]),
                          gp_Dir(aDirection[0], aDirection[1], aDirection[2]));
      if (theMode == RoundedPrimitiveMode::Sphere)
      {
        const double aRadius = requiredNumber(theArgs, "radius");
        if (aRadius <= 0.0) throw KernelFailure(ErrorCode::InvalidArgs, "Sphere radius must be positive");
        BRepPrimAPI_MakeSphere aBuilder(anAxis, aRadius);
        aBuilder.Build();
        if (!aBuilder.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct sphere");
        return {{"shape", theContext.arena().add(aBuilder.Shape(), aScope)}};
      }
      if (theMode == RoundedPrimitiveMode::Cone)
      {
        const double aRadius1 = requiredNumber(theArgs, "radius1");
        const double aRadius2 = requiredNumber(theArgs, "radius2");
        const double aHeight = requiredNumber(theArgs, "height");
        if (aRadius1 < 0.0 || aRadius2 < 0.0 || aHeight <= 0.0 || (aRadius1 == 0.0 && aRadius2 == 0.0)
            || aRadius1 == aRadius2)
          throw KernelFailure(ErrorCode::InvalidArgs, "Cone radii must be non-negative, distinct, and height positive");
        BRepPrimAPI_MakeCone aBuilder(anAxis, aRadius1, aRadius2, aHeight);
        aBuilder.Build();
        if (!aBuilder.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct cone");
        return {{"shape", theContext.arena().add(aBuilder.Shape(), aScope)}};
      }
      const double aMajor = requiredNumber(theArgs, "majorRadius");
      const double aMinor = requiredNumber(theArgs, "minorRadius");
      if (aMajor <= 0.0 || aMinor <= 0.0 || aMajor <= aMinor)
        throw KernelFailure(ErrorCode::InvalidArgs, "Torus radii must be positive with majorRadius > minorRadius");
      BRepPrimAPI_MakeTorus aBuilder(anAxis, aMajor, aMinor);
      aBuilder.Build();
      if (!aBuilder.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct torus");
      return {{"shape", theContext.arena().add(aBuilder.Shape(), aScope)}};
}
} // namespace

json h_makeCompound(KernelOperationContext& theContext, const json& theArgs)
{
  return makeAggregate(theContext, theArgs, AggregateMode::Compound);
}

json h_makeShell(KernelOperationContext& theContext, const json& theArgs)
{
  return makeAggregate(theContext, theArgs, AggregateMode::Shell);
}

json h_makeCompSolid(KernelOperationContext& theContext, const json& theArgs)
{
  return makeAggregate(theContext, theArgs, AggregateMode::CompSolid);
}

json h_makeCylinder(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const double aRadius = requiredNumber(theArgs, "radius");
      const double aHeight = requiredNumber(theArgs, "height");
      const auto anOrigin = optionalVec3(theArgs, "origin", {0.0, 0.0, 0.0});
      const auto aDirection = optionalDirection(theArgs, "direction", {0.0, 0.0, 1.0});
      if (aRadius <= 0.0 || aHeight <= 0.0)
      {
        throw KernelFailure(ErrorCode::InvalidArgs, "Cylinder radius and height must be positive");
      }
      BRepPrimAPI_MakeCylinder aBuilder(
        gp_Ax2(gp_Pnt(anOrigin[0], anOrigin[1], anOrigin[2]),
               gp_Dir(aDirection[0], aDirection[1], aDirection[2])),
        aRadius,
        aHeight);
      aBuilder.Build();
      if (!aBuilder.IsDone())
      {
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct cylinder");
      }
      return {{"shape", theContext.arena().add(aBuilder.Shape(), aScope)}};
    }

json h_makeSphere(KernelOperationContext& theContext, const json& theArgs)
{
  return makeRoundedPrimitive(theContext, theArgs, RoundedPrimitiveMode::Sphere);
}

json h_makeCone(KernelOperationContext& theContext, const json& theArgs)
{
  return makeRoundedPrimitive(theContext, theArgs, RoundedPrimitiveMode::Cone);
}

json h_makeTorus(KernelOperationContext& theContext, const json& theArgs)
{
  return makeRoundedPrimitive(theContext, theArgs, RoundedPrimitiveMode::Torus);
}

json h_makeWedge(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const auto aSize = requiredVec3(theArgs, "size");
      const double aLtx = requiredNumber(theArgs, "ltx");
      const auto anOrigin = optionalVec3(theArgs, "origin", {0.0, 0.0, 0.0});
      const auto aDirection = optionalDirection(theArgs, "direction", {0.0, 0.0, 1.0});
      if (aSize[0] <= 0.0 || aSize[1] <= 0.0 || aSize[2] <= 0.0 || aLtx < 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs, "Wedge dimensions must be positive and ltx non-negative");
      BRepPrimAPI_MakeWedge aBuilder(
        gp_Ax2(gp_Pnt(anOrigin[0], anOrigin[1], anOrigin[2]),
               gp_Dir(aDirection[0], aDirection[1], aDirection[2])),
        aSize[0], aSize[1], aSize[2], aLtx);
      aBuilder.Build();
      if (!aBuilder.IsDone())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct wedge");
      return {{"shape", theContext.arena().add(aBuilder.Shape(), aScope)}};
    }

json h_makeHalfSpace(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aBoundary = theContext.arena().get(requiredU32(theArgs, "face"));
      if (aBoundary.ShapeType() != TopAbs_FACE)
        throw KernelFailure(ErrorCode::InvalidArgs, "Half-space boundary must be a face");
      const auto aReference = requiredVec3(theArgs, "referencePoint");
      BRepPrimAPI_MakeHalfSpace aBuilder(
        TopoDS::Face(aBoundary), gp_Pnt(aReference[0], aReference[1], aReference[2]));
      if (!aBuilder.IsDone() || aBuilder.Shape().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct half-space");
      return {{"shape", theContext.arena().add(aBuilder.Shape(), aScope)}};
    }

json h_makeSolidFromShell(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShellShape = theContext.arena().get(requiredU32(theArgs, "shell"));
      if (aShellShape.ShapeType() != TopAbs_SHELL)
        throw KernelFailure(ErrorCode::InvalidArgs, "makeSolidFromShell requires a shell");
      const TopoDS_Shell aShell = TopoDS::Shell(aShellShape);
      if (!BRep_Tool::IsClosed(aShell) || !BRepCheck_Analyzer(aShell, true).IsValid())
        throw KernelFailure(ErrorCode::InvalidArgs, "makeSolidFromShell requires a closed, valid shell");
      BRepBuilderAPI_MakeSolid aBuilder(aShell);
      if (!aBuilder.IsDone() || aBuilder.Solid().IsNull()
          || !BRepCheck_Analyzer(aBuilder.Solid(), true).IsValid())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct a valid solid from shell");
      return {{"shape", theContext.arena().add(aBuilder.Solid(), aScope)}};
    }

json h_extrude(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
      const auto aVector = requiredVec3(theArgs, "vector");
      const double aSquaredLength = aVector[0] * aVector[0] + aVector[1] * aVector[1] + aVector[2] * aVector[2];
      if (aSquaredLength <= 1.0e-30)
        throw KernelFailure(ErrorCode::InvalidArgs, "Extrusion vector must be non-zero");
      BRepPrimAPI_MakePrism aBuilder(aShape, gp_Vec(aVector[0], aVector[1], aVector[2]), true);
      aBuilder.Build();
      if (!aBuilder.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to extrude shape");
      json aHistory;
      if (theArgs.value("includeHistory", false))
        aHistory = buildHistory(aBuilder, {aShape}, aBuilder.Shape());
      json aResult{{"shape", theContext.arena().add(aBuilder.Shape(), aScope)}};
      if (theArgs.value("includeHistory", false))
        aResult["history"] = std::move(aHistory);
      return aResult;
    }

json h_revolve(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
      const auto anOrigin = requiredVec3(theArgs, "origin");
      const auto aDirection = requiredDirection(theArgs, "direction");
      const double anAngle = theArgs.value("angle", 2.0 * 3.14159265358979323846);
      if (anAngle <= 0.0 || anAngle > 2.0 * 3.14159265358979323846)
        throw KernelFailure(ErrorCode::InvalidArgs, "Revolve angle must be in (0, 2pi]");
      BRepPrimAPI_MakeRevol aBuilder(aShape,
                                     gp_Ax1(gp_Pnt(anOrigin[0], anOrigin[1], anOrigin[2]),
                                            gp_Dir(aDirection[0], aDirection[1], aDirection[2])), anAngle, true);
      aBuilder.Build();
      if (!aBuilder.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to revolve shape");
      json aHistory;
      if (theArgs.value("includeHistory", false))
        aHistory = buildHistory(aBuilder, {aShape}, aBuilder.Shape());
      json aResult{{"shape", theContext.arena().add(aBuilder.Shape(), aScope)}};
      if (theArgs.value("includeHistory", false))
        aResult["history"] = std::move(aHistory);
      return aResult;
    }

json h_localPrism(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aBase = theContext.arena().get(requiredU32(theArgs, "base"));
      if (!theArgs.contains("faceIndices") || !theArgs.at("faceIndices").is_array()
          || theArgs.at("faceIndices").empty())
        throw KernelFailure(ErrorCode::InvalidArgs, "localPrism requires faceIndices");
      const auto aDirection = requiredDirection(theArgs, "direction");
      const std::string aMode = theArgs.value("mode", "length");
      if (aMode != "length" && aMode != "until" && aMode != "fromUntil"
          && aMode != "untilEnd" && aMode != "fromEnd" && aMode != "thruAll")
        throw KernelFailure(ErrorCode::InvalidArgs, "Unknown localPrism limit mode");
      const std::string anOperation = theArgs.value("operation", "add");
      if (anOperation != "add" && anOperation != "cut")
        throw KernelFailure(ErrorCode::InvalidArgs, "localPrism operation must be add or cut");

      TopTools_IndexedMapOfShape aFaces;
      TopExp::MapShapes(aBase, TopAbs_FACE, aFaces);
      std::vector<TopoDS_Face> aSelectedFaces;
      std::vector<bool> aSeen(static_cast<std::size_t>(aFaces.Extent()), false);
      for (const json& anIndexValue : theArgs.at("faceIndices"))
      {
        if (!anIndexValue.is_number_integer())
          throw KernelFailure(ErrorCode::InvalidArgs, "localPrism face indices must be integers");
        const std::int64_t anIndex = anIndexValue.get<std::int64_t>();
        if (anIndex < 0 || anIndex >= aFaces.Extent())
          throw KernelFailure(ErrorCode::InvalidArgs, "localPrism face index is out of range");
        if (aSeen[static_cast<std::size_t>(anIndex)])
          throw KernelFailure(ErrorCode::InvalidArgs, "localPrism face indices must be unique");
        aSeen[static_cast<std::size_t>(anIndex)] = true;
        aSelectedFaces.push_back(TopoDS::Face(aFaces(static_cast<int>(anIndex) + 1)));
      }

      TopoDS_Shape aProfile;
      TopoDS_Face aSketchFace;
      bool toModify = false;
      if (aSelectedFaces.size() == 1)
      {
        aProfile = aSelectedFaces.front();
        aSketchFace = aSelectedFaces.front();
        toModify = true;
      }
      else
      {
        BRep_Builder aShapeBuilder;
        TopoDS_Shell aShell;
        aShapeBuilder.MakeShell(aShell);
        for (const TopoDS_Face& aFace : aSelectedFaces) aShapeBuilder.Add(aShell, aFace);
        aShell.Closed(BRep_Tool::IsClosed(aShell));
        aProfile = aShell;
      }

      BRepFeat_MakePrism aBuilder;
      aBuilder.Init(aBase,
                    aProfile,
                    aSketchFace,
                    gp_Dir(aDirection[0], aDirection[1], aDirection[2]),
                    anOperation == "add" ? 1 : 0,
                    toModify);
      if (aMode == "length")
      {
        const double aLength = requiredNumber(theArgs, "length");
        if (aLength <= 0.0)
          throw KernelFailure(ErrorCode::InvalidArgs, "localPrism length must be positive");
        aBuilder.Perform(aLength);
      }
      else if (aMode == "until")
      {
        aBuilder.Perform(theContext.arena().get(requiredU32(theArgs, "until")));
      }
      else if (aMode == "fromUntil")
      {
        aBuilder.Perform(theContext.arena().get(requiredU32(theArgs, "from")),
                         theContext.arena().get(requiredU32(theArgs, "until")));
      }
      else if (aMode == "untilEnd")
      {
        aBuilder.PerformUntilEnd();
      }
      else if (aMode == "fromEnd")
      {
        aBuilder.PerformFromEnd(theContext.arena().get(requiredU32(theArgs, "from")));
      }
      else
      {
        aBuilder.PerformThruAll();
      }
      if (!aBuilder.IsDone() || aBuilder.Shape().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT local prism feature failed");
      return {{"shape", theContext.arena().add(aBuilder.Shape(), aScope)}};
    }

json h_localRevolution(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aBase = theContext.arena().get(requiredU32(theArgs, "base"));
      if (!theArgs.contains("faceIndices") || !theArgs.at("faceIndices").is_array()
          || theArgs.at("faceIndices").empty())
        throw KernelFailure(ErrorCode::InvalidArgs, "localRevolution requires faceIndices");
      const auto anOrigin = requiredVec3(theArgs, "origin");
      const auto aDirection = requiredDirection(theArgs, "direction");
      const std::string aMode = theArgs.value("mode", "angle");
      if (aMode != "angle" && aMode != "until" && aMode != "fromUntil" && aMode != "thruAll")
        throw KernelFailure(ErrorCode::InvalidArgs, "Unknown localRevolution limit mode");
      const std::string anOperation = theArgs.value("operation", "add");
      if (anOperation != "add" && anOperation != "cut")
        throw KernelFailure(ErrorCode::InvalidArgs, "localRevolution operation must be add or cut");

      TopTools_IndexedMapOfShape aFaces;
      TopExp::MapShapes(aBase, TopAbs_FACE, aFaces);
      std::vector<TopoDS_Face> aSelectedFaces;
      std::vector<bool> aSeen(static_cast<std::size_t>(aFaces.Extent()), false);
      for (const json& anIndexValue : theArgs.at("faceIndices"))
      {
        if (!anIndexValue.is_number_integer())
          throw KernelFailure(ErrorCode::InvalidArgs, "localRevolution face indices must be integers");
        const std::int64_t anIndex = anIndexValue.get<std::int64_t>();
        if (anIndex < 0 || anIndex >= aFaces.Extent())
          throw KernelFailure(ErrorCode::InvalidArgs, "localRevolution face index is out of range");
        if (aSeen[static_cast<std::size_t>(anIndex)])
          throw KernelFailure(ErrorCode::InvalidArgs, "localRevolution face indices must be unique");
        aSeen[static_cast<std::size_t>(anIndex)] = true;
        aSelectedFaces.push_back(TopoDS::Face(aFaces(static_cast<int>(anIndex) + 1)));
      }

      TopoDS_Shape aProfile;
      TopoDS_Face aSketchFace;
      bool toModify = false;
      if (aSelectedFaces.size() == 1)
      {
        aProfile = aSelectedFaces.front();
        aSketchFace = aSelectedFaces.front();
        toModify = true;
      }
      else
      {
        BRep_Builder aShapeBuilder;
        TopoDS_Shell aShell;
        aShapeBuilder.MakeShell(aShell);
        for (const TopoDS_Face& aFace : aSelectedFaces) aShapeBuilder.Add(aShell, aFace);
        aShell.Closed(BRep_Tool::IsClosed(aShell));
        aProfile = aShell;
      }

      BRepFeat_MakeRevol aBuilder;
      aBuilder.Init(aBase, aProfile, aSketchFace,
                    gp_Ax1(gp_Pnt(anOrigin[0], anOrigin[1], anOrigin[2]),
                           gp_Dir(aDirection[0], aDirection[1], aDirection[2])),
                    anOperation == "add" ? 1 : 0, toModify);
      if (aMode == "angle")
      {
        const double anAngle = requiredNumber(theArgs, "angle");
        if (!std::isfinite(anAngle) || anAngle == 0.0 || std::abs(anAngle) > 2.0 * 3.14159265358979323846)
          throw KernelFailure(ErrorCode::InvalidArgs, "localRevolution angle must be non-zero and within +/-2pi");
        aBuilder.Perform(anAngle);
      }
      else if (aMode == "until")
      {
        aBuilder.Perform(theContext.arena().get(requiredU32(theArgs, "until")));
      }
      else if (aMode == "fromUntil")
      {
        aBuilder.Perform(theContext.arena().get(requiredU32(theArgs, "from")),
                         theContext.arena().get(requiredU32(theArgs, "until")));
      }
      else
      {
        aBuilder.PerformThruAll();
      }
      if (!aBuilder.IsDone() || aBuilder.Shape().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT local revolution feature failed");
      return {{"shape", theContext.arena().add(aBuilder.Shape(), aScope)}};
    }

json h_linearForm(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aBase = theContext.arena().get(requiredU32(theArgs, "base"));
      const TopoDS_Shape& aProfileShape = theContext.arena().get(requiredU32(theArgs, "profile"));
      if (aProfileShape.ShapeType() != TopAbs_WIRE)
        throw KernelFailure(ErrorCode::InvalidArgs, "linearForm profile must be a wire");

      const auto aPlaneOrigin = requiredVec3(theArgs, "planeOrigin");
      const auto aPlaneNormal = requiredDirection(theArgs, "planeNormal");
      const auto aDirection = requiredVec3(theArgs, "direction");
      const auto aDirection1 = optionalVec3(theArgs, "direction1", {0.0, 0.0, 0.0});
      const gp_Vec aFirstDirection(aDirection[0], aDirection[1], aDirection[2]);
      if (aFirstDirection.Magnitude() <= Precision::Confusion())
        throw KernelFailure(ErrorCode::InvalidArgs, "linearForm direction must be non-zero");
      const gp_Vec aSecondDirection(aDirection1[0], aDirection1[1], aDirection1[2]);
      if (aSecondDirection.Magnitude() > Precision::Confusion())
      {
        const double aFirstMagnitude = aFirstDirection.Magnitude();
        const double aSecondMagnitude = aSecondDirection.Magnitude();
        const double aCosine = aFirstDirection.Dot(aSecondDirection)
                             / (aFirstMagnitude * aSecondMagnitude);
        const double aCrossMagnitude = aFirstDirection.Crossed(aSecondDirection).Magnitude()
                                     / (aFirstMagnitude * aSecondMagnitude);
        if (aCosine >= -1.0 + 1e-9 || aCrossMagnitude > 1e-9)
          throw KernelFailure(ErrorCode::InvalidArgs,
                              "linearForm direction1 must be zero or opposite to direction");
      }

      const std::string anOperation = theArgs.value("operation", "add");
      if (anOperation != "add" && anOperation != "cut")
        throw KernelFailure(ErrorCode::InvalidArgs, "linearForm operation must be add or cut");
      const occ::handle<Geom_Plane> aPlane = new Geom_Plane(
        gp_Pln(gp_Pnt(aPlaneOrigin[0], aPlaneOrigin[1], aPlaneOrigin[2]),
               gp_Dir(aPlaneNormal[0], aPlaneNormal[1], aPlaneNormal[2])));

      BRepFeat_MakeLinearForm aBuilder;
      aBuilder.Init(aBase, TopoDS::Wire(aProfileShape), aPlane, aFirstDirection,
                    aSecondDirection,
                    anOperation == "add" ? 1 : 0, theArgs.value("modify", true));
      if (!aBuilder.IsDone())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to initialize linearForm");

      aBuilder.Perform();
      if (!aBuilder.IsDone() || aBuilder.Shape().IsNull()
          || !BRepCheck_Analyzer(aBuilder.Shape(), true).IsValid())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT linearForm failed");
      return {{"shape", theContext.arena().add(aBuilder.Shape(), aScope)}};
    }

json h_revolutionForm(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aBase = theContext.arena().get(requiredU32(theArgs, "base"));
      const TopoDS_Shape& aProfileShape = theContext.arena().get(requiredU32(theArgs, "profile"));
      if (aProfileShape.ShapeType() != TopAbs_WIRE)
        throw KernelFailure(ErrorCode::InvalidArgs, "revolutionForm profile must be a wire");

      const auto aPlaneOrigin = requiredVec3(theArgs, "planeOrigin");
      const auto aPlaneNormal = requiredDirection(theArgs, "planeNormal");
      const auto anOrigin = requiredVec3(theArgs, "origin");
      const auto aDirection = requiredDirection(theArgs, "direction");
      const double aHeight1 = requiredNumber(theArgs, "height1");
      const double aHeight2 = requiredNumber(theArgs, "height2");
      if (!std::isfinite(aHeight1) || !std::isfinite(aHeight2)
          || aHeight1 < 0.0 || aHeight2 < 0.0 || (aHeight1 == 0.0 && aHeight2 == 0.0))
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "revolutionForm heights must be non-negative and not both zero");
      const std::string anOperation = theArgs.value("operation", "add");
      if (anOperation != "add" && anOperation != "cut")
        throw KernelFailure(ErrorCode::InvalidArgs, "revolutionForm operation must be add or cut");

      const occ::handle<Geom_Plane> aPlane = new Geom_Plane(
        gp_Pln(gp_Pnt(aPlaneOrigin[0], aPlaneOrigin[1], aPlaneOrigin[2]),
               gp_Dir(aPlaneNormal[0], aPlaneNormal[1], aPlaneNormal[2])));
      bool toModify = theArgs.value("modify", true);
      BRepFeat_MakeRevolutionForm aBuilder;
      aBuilder.Init(aBase,
                    TopoDS::Wire(aProfileShape),
                    aPlane,
                    gp_Ax1(gp_Pnt(anOrigin[0], anOrigin[1], anOrigin[2]),
                           gp_Dir(aDirection[0], aDirection[1], aDirection[2])),
                    aHeight1,
                    aHeight2,
                    anOperation == "add" ? 1 : 0,
                    toModify);
      if (!aBuilder.IsDone())
        throw KernelFailure(ErrorCode::ConstructionFailed,
                            "OCCT failed to initialize revolutionForm");

      aBuilder.Perform();
      if (!aBuilder.IsDone() || aBuilder.Shape().IsNull()
          || !BRepCheck_Analyzer(aBuilder.Shape(), true).IsValid())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT revolutionForm failed");
      return {{"shape", theContext.arena().add(aBuilder.Shape(), aScope)}};
    }

json h_cylindricalHole(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
      const auto anOrigin = requiredVec3(theArgs, "origin");
      const auto aDirection = requiredDirection(theArgs, "direction");
      const double aRadius = requiredNumber(theArgs, "radius");
      if (aRadius <= 0.0 || !std::isfinite(aRadius))
        throw KernelFailure(ErrorCode::InvalidArgs, "Cylindrical hole radius must be positive");
      const gp_Pnt anAxisOrigin(anOrigin[0], anOrigin[1], anOrigin[2]);
      const gp_Dir anAxisDirection(aDirection[0], aDirection[1], aDirection[2]);
      const gp_Lin anAxis(anAxisOrigin, anAxisDirection);

      Bnd_Box aBounds;
      BRepBndLib::Add(aShape, aBounds);
      if (aBounds.IsVoid())
        throw KernelFailure(ErrorCode::InvalidArgs, "Cylindrical hole requires a bounded shape");
      double aXMin = 0.0;
      double aYMin = 0.0;
      double aZMin = 0.0;
      double aXMax = 0.0;
      double aYMax = 0.0;
      double aZMax = 0.0;
      aBounds.Get(aXMin, aYMin, aZMin, aXMax, aYMax, aZMax);
      double aShapeFirst = std::numeric_limits<double>::max();
      double aShapeLast = std::numeric_limits<double>::lowest();
      for (const double anX : {aXMin, aXMax})
        for (const double aY : {aYMin, aYMax})
          for (const double aZ : {aZMin, aZMax})
          {
            const double aParameter = gp_Vec(anAxisOrigin, gp_Pnt(anX, aY, aZ))
                                        .Dot(gp_Vec(anAxisDirection));
            aShapeFirst = std::min(aShapeFirst, aParameter);
            aShapeLast = std::max(aShapeLast, aParameter);
          }
      const double aShapeSpan = std::max({aShapeLast - aShapeFirst, aRadius * 2.0, 1.0});
      const double aPad = std::max(aShapeSpan * 1.0e-7, 1.0e-6);
      double aToolFirst = 0.0;
      double aToolLast = 0.0;
      const std::string aMode = theArgs.value("mode", "throughAll");
      const bool withControl = theArgs.value("withControl", true);
      if (aMode == "throughAll")
      {
        aToolFirst = aShapeFirst - aShapeSpan;
        aToolLast = aShapeLast + aShapeSpan;
      }
      else if (aMode == "throughNext" || aMode == "untilEnd")
      {
        IntCurvesFace_ShapeIntersector anIntersector;
        anIntersector.Load(aShape, 1.0e-7);
        anIntersector.Perform(anAxis, aShapeFirst - aShapeSpan, aShapeLast + aShapeSpan);
        if (!anIntersector.IsDone())
          throw KernelFailure(ErrorCode::ConstructionFailed,
                              "OCCT failed to intersect the cylindrical hole axis");
        anIntersector.SortResult();
        int aFirstForward = 0;
        for (int anIndex = 1; anIndex <= anIntersector.NbPnt(); ++anIndex)
          if (anIntersector.Transition(anIndex) != IntCurveSurface_Tangent
              && anIntersector.WParameter(anIndex) >= -aPad)
          {
            aFirstForward = anIndex;
            break;
          }
        if (aFirstForward == 0)
          throw KernelFailure(ErrorCode::ConstructionFailed,
                              "Cylindrical hole axis does not enter the shape after its origin");
        int anEntry = 0;
        int anExit = 0;
        if (anIntersector.Transition(aFirstForward) == IntCurveSurface_In)
        {
          anEntry = aFirstForward;
        }
        else
        {
          for (int anIndex = aFirstForward - 1; anIndex >= 1; --anIndex)
            if (anIntersector.Transition(anIndex) == IntCurveSurface_In)
            {
              anEntry = anIndex;
              break;
            }
        }
        if (anEntry == 0)
          throw KernelFailure(ErrorCode::ConstructionFailed,
                              "Cylindrical hole axis has no entry intersection");
        for (int anIndex = anEntry + 1; anIndex <= anIntersector.NbPnt(); ++anIndex)
          if (anIntersector.Transition(anIndex) == IntCurveSurface_Out)
          {
            anExit = anIndex;
            if (aMode == "throughNext") break;
          }
        if (anExit == 0)
          throw KernelFailure(ErrorCode::ConstructionFailed,
                              "Cylindrical hole axis has no exit intersection");
        aToolFirst = anIntersector.WParameter(anEntry) - aPad;
        aToolLast = anIntersector.WParameter(anExit) + aPad;
      }
      else if (aMode == "blind")
      {
        const double aLength = requiredNumber(theArgs, "length");
        if (aLength <= 0.0 || !std::isfinite(aLength))
          throw KernelFailure(ErrorCode::InvalidArgs, "Blind hole length must be positive");
        aToolFirst = -aPad;
        aToolLast = aLength;
      }
      else if (aMode == "between")
      {
        const double aFrom = requiredNumber(theArgs, "from");
        const double aTo = requiredNumber(theArgs, "to");
        if (aFrom == aTo || !std::isfinite(aFrom) || !std::isfinite(aTo))
          throw KernelFailure(ErrorCode::InvalidArgs, "Cylindrical hole interval must have non-zero length");
        aToolFirst = std::min(aFrom, aTo);
        aToolLast = std::max(aFrom, aTo);
      }
      else
      {
        throw KernelFailure(ErrorCode::InvalidArgs, "Unknown cylindrical hole mode");
      }
      const gp_Pnt aToolOrigin = anAxisOrigin.Translated(gp_Vec(anAxisDirection) * aToolFirst);
      BRepPrimAPI_MakeCylinder aToolBuilder(
        gp_Ax2(aToolOrigin, anAxisDirection), aRadius, aToolLast - aToolFirst);
      aToolBuilder.Build();
      if (!aToolBuilder.IsDone() || aToolBuilder.Shape().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct the cylindrical hole tool");
      NCollection_List<TopoDS_Shape> anArguments;
      NCollection_List<TopoDS_Shape> aTools;
      anArguments.Append(aShape);
      aTools.Append(aToolBuilder.Shape());
      BRepAlgoAPI_Cut aBuilder;
      aBuilder.SetArguments(anArguments);
      aBuilder.SetTools(aTools);
      aBuilder.SetNonDestructive(true);
      aBuilder.SetRunParallel(false);
      aBuilder.SetToFillHistory(theArgs.value("includeHistory", false));
      aBuilder.Build();
      if (!aBuilder.IsDone() || aBuilder.HasErrors() || aBuilder.Shape().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT cylindrical hole feature failed");
      const TopoDS_Shape anOutput = aBuilder.Shape();
      json aHistory;
      if (theArgs.value("includeHistory", false))
        aHistory = buildHistory(aBuilder, {aShape}, anOutput);
      if (withControl && !BRepCheck_Analyzer(anOutput, true).IsValid())
        throw KernelFailure(ErrorCode::ConstructionFailed,
                            "OCCT cylindrical hole feature failed validation");
      json aResult{{"shape", theContext.arena().add(anOutput, aScope)}};
      if (theArgs.value("includeHistory", false))
        aResult["history"] = std::move(aHistory);
      return aResult;
    }

json h_defeature(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
      if (aShape.ShapeType() != TopAbs_SOLID && aShape.ShapeType() != TopAbs_COMPSOLID
          && aShape.ShapeType() != TopAbs_COMPOUND)
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "Defeaturing requires a solid, compsolid, or compound of solids");
      if (!theArgs.contains("faceIndices") || !theArgs.at("faceIndices").is_array()
          || theArgs.at("faceIndices").empty())
        throw KernelFailure(ErrorCode::InvalidArgs, "Defeaturing requires faceIndices");
      TopTools_IndexedMapOfShape aFaces;
      TopExp::MapShapes(aShape, TopAbs_FACE, aFaces);
      std::vector<bool> aSeen(static_cast<std::size_t>(aFaces.Extent()), false);
      BRepAlgoAPI_Defeaturing aBuilder;
      aBuilder.SetShape(aShape);
      aBuilder.SetRunParallel(false);
      aBuilder.SetToFillHistory(theArgs.value("includeHistory", false));
      for (const json& anIndexValue : theArgs.at("faceIndices"))
      {
        if (!anIndexValue.is_number_integer())
          throw KernelFailure(ErrorCode::InvalidArgs, "Defeaturing face indices must be integers");
        const std::int64_t anIndex = anIndexValue.get<std::int64_t>();
        if (anIndex < 0 || anIndex >= aFaces.Extent())
          throw KernelFailure(ErrorCode::InvalidArgs, "Defeaturing face index is out of range");
        if (aSeen[static_cast<std::size_t>(anIndex)])
          throw KernelFailure(ErrorCode::InvalidArgs, "Defeaturing face indices must be unique");
        aSeen[static_cast<std::size_t>(anIndex)] = true;
        aBuilder.AddFaceToRemove(aFaces(static_cast<int>(anIndex) + 1));
      }
      aBuilder.Build();
      if (!aBuilder.IsDone() || aBuilder.HasErrors() || aBuilder.Shape().IsNull())
        throw KernelFailure(ErrorCode::BooleanFailed, "OCCT defeaturing failed");
      json aHistory;
      if (theArgs.value("includeHistory", false))
        aHistory = buildHistory(aBuilder, {aShape}, aBuilder.Shape());
      json aResult{{"shape", theContext.arena().add(aBuilder.Shape(), aScope)}};
      if (theArgs.value("includeHistory", false))
        aResult["history"] = std::move(aHistory);
      return aResult;
    }

json h_loft(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      if (!theArgs.contains("sections") || !theArgs.at("sections").is_array() || theArgs.at("sections").size() < 2)
        throw KernelFailure(ErrorCode::InvalidArgs, "loft requires at least two wire sections");
      BRepOffsetAPI_ThruSections aBuilder(theArgs.value("solid", false), theArgs.value("ruled", false), theArgs.value("precision", 1.0e-6));
      aBuilder.CheckCompatibility(true);
      std::vector<TopoDS_Shape> aSections;
      for (const json& aSection : theArgs.at("sections"))
      {
        aSections.push_back(theContext.arena().get(aSection.get<std::uint32_t>()));
        aBuilder.AddWire(TopoDS::Wire(aSections.back()));
      }
      aBuilder.Build();
      if (!aBuilder.IsDone() || aBuilder.Shape().IsNull()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT loft failed");
      json aHistory;
      if (theArgs.value("includeHistory", false))
        aHistory = buildHistory(aBuilder, aSections, aBuilder.Shape());
      json aResult{{"shape", theContext.arena().add(aBuilder.Shape(), aScope)}};
      if (theArgs.value("includeHistory", false))
        aResult["history"] = std::move(aHistory);
      return aResult;
    }

json h_sweepPipe(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape aSpine = theContext.arena().get(requiredU32(theArgs, "spine"));
      const TopoDS_Shape aProfile = theContext.arena().get(requiredU32(theArgs, "profile"));
      BRepOffsetAPI_MakePipe aBuilder(TopoDS::Wire(aSpine), aProfile);
      aBuilder.Build();
      if (!aBuilder.IsDone() || aBuilder.Shape().IsNull()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT pipe sweep failed");
      json aHistory;
      if (theArgs.value("includeHistory", false))
        aHistory = buildHistory(aBuilder, {aSpine, aProfile}, aBuilder.Shape());
      json aResult{{"shape", theContext.arena().add(aBuilder.Shape(), aScope)}};
      if (theArgs.value("includeHistory", false))
        aResult["history"] = std::move(aHistory);
      return aResult;
    }


json h_draftAngle(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
      if (!theArgs.contains("faceIndices") || !theArgs.at("faceIndices").is_array()
          || theArgs.at("faceIndices").empty())
        throw KernelFailure(ErrorCode::InvalidArgs, "draftAngle requires faceIndices");
      if (!theArgs.contains("neutralPlane") || !theArgs.at("neutralPlane").is_object())
        throw KernelFailure(ErrorCode::InvalidArgs, "draftAngle requires neutralPlane");

      const auto aDirection = requiredDirection(theArgs, "direction");
      const double anAngle = requiredNumber(theArgs, "angle");
      if (std::abs(anAngle) <= 1.0e-4)
        throw KernelFailure(ErrorCode::InvalidArgs, "draftAngle angle magnitude must exceed 1e-4 radians");
      const json& aPlaneValue = theArgs.at("neutralPlane");
      const auto aPlaneOrigin = requiredVec3(aPlaneValue, "origin");
      const auto aPlaneNormal = requiredDirection(aPlaneValue, "normal");

      TopTools_IndexedMapOfShape aFaces;
      TopExp::MapShapes(aShape, TopAbs_FACE, aFaces);
      std::vector<bool> aSeen(static_cast<std::size_t>(aFaces.Extent()), false);
      BRepOffsetAPI_DraftAngle aBuilder(aShape);
      for (const json& anIndexValue : theArgs.at("faceIndices"))
      {
        if (!anIndexValue.is_number_integer())
          throw KernelFailure(ErrorCode::InvalidArgs, "draftAngle face indices must be integers");
        const std::int64_t anIndex = anIndexValue.get<std::int64_t>();
        if (anIndex < 0 || anIndex >= aFaces.Extent())
          throw KernelFailure(ErrorCode::InvalidArgs, "draftAngle face index is out of range");
        if (aSeen[static_cast<std::size_t>(anIndex)])
          throw KernelFailure(ErrorCode::InvalidArgs, "draftAngle face indices must be unique");
        aSeen[static_cast<std::size_t>(anIndex)] = true;
        aBuilder.Add(TopoDS::Face(aFaces(static_cast<int>(anIndex) + 1)),
                     gp_Dir(aDirection[0], aDirection[1], aDirection[2]),
                     anAngle,
                     gp_Pln(gp_Pnt(aPlaneOrigin[0], aPlaneOrigin[1], aPlaneOrigin[2]),
                            gp_Dir(aPlaneNormal[0], aPlaneNormal[1], aPlaneNormal[2])),
                     theArgs.value("flag", true));
        if (!aBuilder.AddDone())
          throw KernelFailure(ErrorCode::ConstructionFailed,
                              "OCCT draft failed while adding face index " + std::to_string(anIndex));
      }
      aBuilder.Build();
      if (!aBuilder.IsDone() || aBuilder.Shape().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT draft build failed");
      return {{"shape", theContext.arena().add(aBuilder.Shape(), aScope)}};
    }

json h_hollow(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const double anOffset = requiredNumber(theArgs, "thickness");
      if (anOffset == 0.0) throw KernelFailure(ErrorCode::InvalidArgs, "Hollow thickness cannot be zero");
      const double aTolerance = theArgs.value("tolerance", 1.0e-6);
      if (aTolerance <= 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs, "Hollow tolerance must be positive");
      const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
      BRepOffsetAPI_MakeThickSolid aBuilder;
      NCollection_List<TopoDS_Shape> aClosingFaces;
      if (theArgs.contains("closingFaceIndices"))
      {
        TopTools_IndexedMapOfShape aFaces;
        TopExp::MapShapes(aShape, TopAbs_FACE, aFaces);
        for (const json& anIndex : theArgs.at("closingFaceIndices"))
        {
          const std::int64_t aFaceIndex = anIndex.get<std::int64_t>();
          if (aFaceIndex < 0 || aFaceIndex >= aFaces.Extent()) throw KernelFailure(ErrorCode::InvalidArgs, "Hollow face index out of range");
          aClosingFaces.Append(aFaces(static_cast<int>(aFaceIndex) + 1));
        }
      }
      aBuilder.MakeThickSolidByJoin(aShape, aClosingFaces, anOffset, aTolerance);
      if (!aBuilder.IsDone() || aBuilder.Shape().IsNull()) throw KernelFailure(ErrorCode::HealingFailed, "OCCT hollow failed");
      return {{"shape", theContext.arena().add(aBuilder.Shape(), aScope)}};
    }





void register_modeling_operations()
{
  auto& aRegistry = OperationRegistry::instance();
#define OCCT_REGISTER_HANDLER(theOperation) aRegistry.add(#theOperation, &h_##theOperation);
  OCCT_MODELING_OPERATION_TABLE(OCCT_REGISTER_HANDLER)
#undef OCCT_REGISTER_HANDLER
}

OCCT_DEFINE_SIDE_PLUGIN(modeling, kModelingOperationNames, register_modeling_operations)

} // namespace occt_worker
