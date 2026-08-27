// Transform operation handlers.

#include "kernel_modeling_operations.hpp"
#include "kernel_history_helpers.hpp"
#include "kernel_operation_context.hpp"
#include "kernel_protocol_helpers.hpp"

namespace occt_worker {

namespace {
enum class SimpleTransformMode { Translate, Rotate, Scale, Mirror };

json applySimpleTransform(KernelOperationContext& theContext, const json& theArgs,
                          SimpleTransformMode theMode)
{
      json aTransformArgs = theArgs;
      if (theMode == SimpleTransformMode::Translate)
        aTransformArgs["translation"] = requiredVec3(theArgs, "translation");
      else if (theMode == SimpleTransformMode::Rotate)
        aTransformArgs["rotation"] = theArgs.at("rotation");
      else if (theMode == SimpleTransformMode::Scale)
      {
        aTransformArgs["scale"] = requiredNumber(theArgs, "factor");
        if (theArgs.contains("origin")) aTransformArgs["scaleOrigin"] = theArgs.at("origin");
      }
      else aTransformArgs["mirror"] = requiredVec3(theArgs, "normal");
      return theContext.dispatch("transform", aTransformArgs);
    }

} // namespace

json h_translate(KernelOperationContext& theContext, const json& theArgs)
{
  return applySimpleTransform(theContext, theArgs, SimpleTransformMode::Translate);
}

json h_rotate(KernelOperationContext& theContext, const json& theArgs)
{
  return applySimpleTransform(theContext, theArgs, SimpleTransformMode::Rotate);
}

json h_scale(KernelOperationContext& theContext, const json& theArgs)
{
  return applySimpleTransform(theContext, theArgs, SimpleTransformMode::Scale);
}

json h_mirror(KernelOperationContext& theContext, const json& theArgs)
{
  return applySimpleTransform(theContext, theArgs, SimpleTransformMode::Mirror);
}

json h_transform(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
      gp_Trsf aTrsf;
      if (theArgs.contains("matrix"))
      {
        const json& aMatrix = theArgs.at("matrix");
        if (!aMatrix.is_array() || aMatrix.size() != 12) throw KernelFailure(ErrorCode::InvalidArgs, "transform matrix must contain 12 numbers");
        aTrsf.SetValues(aMatrix[0].get<double>(), aMatrix[1].get<double>(), aMatrix[2].get<double>(), aMatrix[3].get<double>(),
                        aMatrix[4].get<double>(), aMatrix[5].get<double>(), aMatrix[6].get<double>(), aMatrix[7].get<double>(),
                        aMatrix[8].get<double>(), aMatrix[9].get<double>(), aMatrix[10].get<double>(), aMatrix[11].get<double>());
      }
      else
      {
        if (theArgs.contains("translation"))
        {
          const auto aTranslation = requiredVec3(theArgs, "translation");
          gp_Trsf aPart;
          aPart.SetTranslationPart(gp_Vec(aTranslation[0], aTranslation[1], aTranslation[2]));
          aTrsf.Multiply(aPart);
        }
        if (theArgs.contains("rotation"))
        {
          const json& aRotation = theArgs.at("rotation");
          if (!aRotation.is_object()) throw KernelFailure(ErrorCode::InvalidArgs, "rotation must be an object");
          const auto anOrigin = aRotation.contains("origin") ? requiredVec3(aRotation, "origin") : std::array<double, 3>{0, 0, 0};
          const auto aDirection = aRotation.contains("direction") ? requiredDirection(aRotation, "direction") : std::array<double, 3>{0, 0, 1};
          const double anAngle = aRotation.at("angle").get<double>();
          gp_Trsf aPart;
          aPart.SetRotation(gp_Ax1(gp_Pnt(anOrigin[0], anOrigin[1], anOrigin[2]), gp_Dir(aDirection[0], aDirection[1], aDirection[2])), anAngle);
          aTrsf.Multiply(aPart);
        }
        if (theArgs.contains("scale"))
        {
          const double aScale = theArgs.at("scale").get<double>();
          if (aScale == 0.0) throw KernelFailure(ErrorCode::InvalidArgs, "Scale cannot be zero");
          const auto anOrigin = theArgs.contains("scaleOrigin") ? requiredVec3(theArgs, "scaleOrigin") : std::array<double, 3>{0, 0, 0};
          gp_Trsf aPart;
          aPart.SetScale(gp_Pnt(anOrigin[0], anOrigin[1], anOrigin[2]), aScale);
          aTrsf.Multiply(aPart);
        }
        if (theArgs.contains("mirror"))
        {
          const auto aMirror = requiredDirection(theArgs, "mirror");
          gp_Trsf aPart;
          aPart.SetMirror(gp_Ax2(gp_Pnt(0, 0, 0), gp_Dir(aMirror[0], aMirror[1], aMirror[2])));
          aTrsf.Multiply(aPart);
        }
      }
      const bool isCopy = theArgs.value("copy", true);
      BRepBuilderAPI_Transform aBuilder(aShape, aTrsf, isCopy, false);
      if (!aBuilder.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT transform failed");
      json aHistory;
      if (theArgs.value("includeHistory", false))
        aHistory = buildHistory(aBuilder, {aShape}, aBuilder.Shape());
      json aResult{{"shape", theContext.arena().add(aBuilder.Shape(), aScope)}};
      if (theArgs.value("includeHistory", false))
        aResult["history"] = std::move(aHistory);
      return aResult;
    }

json h_batchTransformCopy(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
      const std::uint32_t aCount = requiredU32(theArgs, "count");
      if (aCount == 0) throw KernelFailure(ErrorCode::InvalidArgs, "batchTransformCopy count must be positive");
      const std::string aMode = theArgs.value("mode", "linear");
      std::array<double, 3> aStep{};
      std::array<double, 3> anOrigin{};
      std::array<double, 3> aDirection{};
      double anAngle = 0.0;
      if (aMode == "linear")
      {
        aStep = requiredVec3(theArgs, "translation");
      }
      else if (aMode == "circular")
      {
        anOrigin = theArgs.contains("origin")
          ? requiredVec3(theArgs, "origin") : std::array<double, 3>{0.0, 0.0, 0.0};
        aDirection = requiredDirection(theArgs, "direction");
        anAngle = requiredNumber(theArgs, "angle");
        if (anAngle == 0.0) throw KernelFailure(ErrorCode::InvalidArgs, "Circular array angle cannot be zero");
      }
      else throw KernelFailure(ErrorCode::InvalidArgs, "batchTransformCopy mode must be linear or circular");
      json aShapes = json::array();
      for (std::uint32_t anIndex = 1; anIndex <= aCount; ++anIndex)
      {
        gp_Trsf aTrsf;
        if (aMode == "linear")
        {
          gp_Vec aTranslation(aStep[0] * anIndex, aStep[1] * anIndex, aStep[2] * anIndex);
          aTrsf.SetTranslation(aTranslation);
        }
        else
        {
          aTrsf.SetRotation(gp_Ax1(gp_Pnt(anOrigin[0], anOrigin[1], anOrigin[2]),
                                    gp_Dir(aDirection[0], aDirection[1], aDirection[2])),
                            anAngle * static_cast<double>(anIndex) / static_cast<double>(aCount));
        }
        BRepBuilderAPI_Transform aBuilder(aShape, aTrsf, true, false);
        if (!aBuilder.IsDone() || aBuilder.Shape().IsNull())
          throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT batch transform failed");
        aShapes.push_back(theContext.arena().add(aBuilder.Shape(), aScope));
      }
      return {{"shapes", aShapes}};
    }

json h_generalTransform(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
      if (!theArgs.contains("matrix") || !theArgs.at("matrix").is_array() || theArgs.at("matrix").size() != 12)
        throw KernelFailure(ErrorCode::InvalidArgs, "generalTransform matrix must contain 12 numbers");
      const json& aMatrix = theArgs.at("matrix");
      gp_GTrsf aTrsf;
      for (int aRow = 1; aRow <= 3; ++aRow)
        for (int aColumn = 1; aColumn <= 4; ++aColumn)
          aTrsf.SetValue(aRow, aColumn, aMatrix[(aRow - 1) * 4 + aColumn - 1].get<double>());
      if (aTrsf.IsSingular()) throw KernelFailure(ErrorCode::InvalidArgs, "generalTransform matrix must be non-singular");
      aTrsf.SetForm();
      BRepBuilderAPI_GTransform aBuilder(aShape, aTrsf, true);
      if (!aBuilder.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT general transform failed");
      json aResult{{"shape", theContext.arena().add(aBuilder.Shape(), aScope)}};
      if (theArgs.value("includeHistory", false))
        aResult["history"] = buildHistory(aBuilder, {aShape}, aBuilder.Shape());
      return aResult;
    }

} // namespace occt_worker
