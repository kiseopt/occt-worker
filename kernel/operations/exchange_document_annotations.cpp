#include "kernel_exchange_helpers.hpp"
#include "kernel_operation_context.hpp"
#include "kernel_protocol_helpers.hpp"

namespace occt_worker {

DocumentAnnotationData extractDocumentAnnotations(
  KernelOperationContext& theContext,
  const bool theIsStepDocument,
  const std::vector<std::string>& aStepGdtSemanticNames,
  const std::uint32_t aScope,
  const occ::handle<XCAFDoc_ViewTool>& aViewTool,
  const occ::handle<XCAFDoc_ClippingPlaneTool>& aClippingPlaneTool,
  const occ::handle<XCAFDoc_DimTolTool>& aDimTolTool,
  const std::vector<TDF_Label>& aNodeOccurrenceLabels,
  const std::vector<TDF_Label>& aNodeDefinitionLabels)
{
        json aGdt = json::array();
      auto appendAnnotationPresentation = [&](json& theItem, const TopoDS_Shape& theShape,
                                              const occ::handle<TCollection_HAsciiString>& theName) {
        if (theShape.IsNull()) return;
        json aPresentation = {{"shape", theContext.arena().add(theShape, aScope)}};
        if (!theName.IsNull() && theName->Length() > 0)
          aPresentation["name"] = theName->ToCString();
        theItem["presentation"] = std::move(aPresentation);
      };
      std::size_t aGdtIndex = 0;
      NCollection_Sequence<TDF_Label> aDimensionLabels;
      aDimTolTool->GetDimensionLabels(aDimensionLabels);
      for (int aDimensionIndex = 1; aDimensionIndex <= aDimensionLabels.Size(); ++aDimensionIndex)
      {
        const TDF_Label& aDimensionLabel = aDimensionLabels.Value(aDimensionIndex);
        occ::handle<XCAFDoc_Dimension> aDimensionAttribute;
        if (!aDimensionLabel.FindAttribute(XCAFDoc_Dimension::GetID(), aDimensionAttribute)
            || aDimensionAttribute.IsNull())
          continue;
        const occ::handle<XCAFDimTolObjects_DimensionObject> aDimension =
          aDimensionAttribute->GetObject();
        const char* aTypeName = aDimension.IsNull() ? nullptr : documentDimensionTypeName(aDimension->GetType());
        if (aTypeName == nullptr)
          continue;
        NCollection_Sequence<TDF_Label> aFirstLabels;
        NCollection_Sequence<TDF_Label> aSecondLabels;
        if (!XCAFDoc_DimTolTool::GetRefShapeLabel(aDimensionLabel, aFirstLabels, aSecondLabels)
            || aFirstLabels.IsEmpty())
          continue;
        std::size_t aTargetNode = aNodeOccurrenceLabels.size();
        for (int aRefIndex = 1; aRefIndex <= aFirstLabels.Size() && aTargetNode == aNodeOccurrenceLabels.size(); ++aRefIndex)
        {
          for (std::size_t aNodeIndex = 0; aNodeIndex < aNodeOccurrenceLabels.size(); ++aNodeIndex)
          {
            if (aFirstLabels.Value(aRefIndex).IsEqual(aNodeOccurrenceLabels[aNodeIndex])
                || aFirstLabels.Value(aRefIndex).IsEqual(aNodeDefinitionLabels[aNodeIndex]))
            {
              aTargetNode = aNodeIndex;
              break;
            }
          }
        }
        if (aTargetNode == aNodeOccurrenceLabels.size()) continue;
        double aDimensionValue = aDimension->GetValue();
        if (theIsStepDocument
            && aDimension->GetType() == XCAFDimTolObjects_DimensionType_Size_Angular)
          aDimensionValue *= 3.14159265358979323846 / 180.0;
        json anItem = {{"node", aTargetNode}, {"type", aTypeName}, {"value", aDimensionValue}};
        const occ::handle<TCollection_HAsciiString> aName = aDimension->GetSemanticName();
        if (aGdtIndex < aStepGdtSemanticNames.size() && !aStepGdtSemanticNames[aGdtIndex].empty())
          anItem["semanticName"] = aStepGdtSemanticNames[aGdtIndex];
        else if (!aName.IsNull() && aName->Length() > 0)
          anItem["semanticName"] = aName->ToCString();
        appendAnnotationPresentation(anItem, aDimension->GetPresentation(),
                                     aDimension->GetPresentationName());
        ++aGdtIndex;
        aGdt.push_back(std::move(anItem));
      }
      json aDatums = json::array();
      std::vector<TDF_Label> aOutputDatumLabels;
      NCollection_Sequence<TDF_Label> aDatumLabels;
      aDimTolTool->GetDatumLabels(aDatumLabels);
      for (int aDatumIndex = 1; aDatumIndex <= aDatumLabels.Size(); ++aDatumIndex)
      {
        const TDF_Label& aDatumLabel = aDatumLabels.Value(aDatumIndex);
        NCollection_Sequence<TDF_Label> aShapeLabels;
        NCollection_Sequence<TDF_Label> anUnusedSecond;
        if (!XCAFDoc_DimTolTool::GetRefShapeLabel(aDatumLabel, aShapeLabels, anUnusedSecond)
            || aShapeLabels.IsEmpty())
          continue;
        std::size_t aTargetNode = aNodeOccurrenceLabels.size();
        for (int aRefIndex = 1; aRefIndex <= aShapeLabels.Size(); ++aRefIndex)
        {
          for (std::size_t aNodeIndex = 0; aNodeIndex < aNodeOccurrenceLabels.size(); ++aNodeIndex)
          {
            if (aShapeLabels.Value(aRefIndex).IsEqual(aNodeOccurrenceLabels[aNodeIndex])
                || aShapeLabels.Value(aRefIndex).IsEqual(aNodeDefinitionLabels[aNodeIndex]))
            {
              aTargetNode = aNodeIndex;
              break;
            }
          }
          if (aTargetNode < aNodeOccurrenceLabels.size()) break;
        }
        if (aTargetNode == aNodeOccurrenceLabels.size()) continue;
        occ::handle<TCollection_HAsciiString> aName;
        occ::handle<TCollection_HAsciiString> aDescription;
        occ::handle<TCollection_HAsciiString> anIdentification;
        if (!aDimTolTool->GetDatum(aDatumLabel, aName, aDescription, anIdentification)
            || aName.IsNull() || anIdentification.IsNull())
          continue;
        json anItem = {
          {"node", aTargetNode},
          {"name", aName->ToCString()},
          {"identification", anIdentification->ToCString()}
        };
        if (!aDescription.IsNull() && aDescription->Length() > 0)
          anItem["description"] = aDescription->ToCString();
        occ::handle<XCAFDoc_Datum> aDatumAttribute;
        if (aDatumLabel.FindAttribute(XCAFDoc_Datum::GetID(), aDatumAttribute)
            && !aDatumAttribute.IsNull())
        {
          const auto aDatumObject = aDatumAttribute->GetObject();
          if (!aDatumObject.IsNull() && !aDatumObject->GetSemanticName().IsNull()
              && aDatumObject->GetSemanticName()->Length() > 0)
            anItem["semanticName"] = aDatumObject->GetSemanticName()->ToCString();
          if (!aDatumObject.IsNull())
            appendAnnotationPresentation(anItem, aDatumObject->GetPresentation(),
                                         aDatumObject->GetPresentationName());
        }
        aDatums.push_back(std::move(anItem));
        aOutputDatumLabels.push_back(aDatumLabel);
      }
      auto nodeIndexForLabel = [&](const TDF_Label& theLabel) {
        for (std::size_t anIndex = 0; anIndex < aNodeOccurrenceLabels.size(); ++anIndex)
          if (theLabel.IsEqual(aNodeOccurrenceLabels[anIndex])
              || theLabel.IsEqual(aNodeDefinitionLabels[anIndex]))
            return anIndex;
        return aNodeOccurrenceLabels.size();
      };
      json aGeometricTolerances = json::array();
      NCollection_Sequence<TDF_Label> aGeomToleranceLabels;
      aDimTolTool->GetGeomToleranceLabels(aGeomToleranceLabels);
      for (int aToleranceIndex = 1; aToleranceIndex <= aGeomToleranceLabels.Size(); ++aToleranceIndex)
      {
        const TDF_Label& aToleranceLabel = aGeomToleranceLabels.Value(aToleranceIndex);
        occ::handle<XCAFDoc_GeomTolerance> aToleranceAttribute;
        if (!aToleranceLabel.FindAttribute(XCAFDoc_GeomTolerance::GetID(), aToleranceAttribute)
            || aToleranceAttribute.IsNull())
          continue;
        const occ::handle<XCAFDimTolObjects_GeomToleranceObject> aTolerance =
          aToleranceAttribute->GetObject();
        const char* aTypeName = aTolerance.IsNull() ? nullptr : documentGeomToleranceTypeName(aTolerance->GetType());
        const char* aValueTypeName = aTolerance.IsNull() ? nullptr
          : documentGeomToleranceValueTypeName(aTolerance->GetTypeOfValue());
        if (aTypeName == nullptr || aValueTypeName == nullptr || aTolerance->GetValue() <= 0.0)
          continue;
        NCollection_Sequence<TDF_Label> aShapeLabels;
        NCollection_Sequence<TDF_Label> anUnusedSecond;
        if (!XCAFDoc_DimTolTool::GetRefShapeLabel(aToleranceLabel, aShapeLabels, anUnusedSecond)
            || aShapeLabels.IsEmpty())
          continue;
        std::size_t aTargetNode = aNodeOccurrenceLabels.size();
        for (int aRefIndex = 1; aRefIndex <= aShapeLabels.Size() && aTargetNode == aNodeOccurrenceLabels.size(); ++aRefIndex)
          aTargetNode = nodeIndexForLabel(aShapeLabels.Value(aRefIndex));
        if (aTargetNode >= aNodeOccurrenceLabels.size()) continue;
        json anItem = {{"node", aTargetNode}, {"type", aTypeName},
                       {"value", aTolerance->GetValue()}};
        if (std::string(aValueTypeName) != "none") anItem["valueType"] = aValueTypeName;
        const occ::handle<TCollection_HAsciiString> aSemanticName = aTolerance->GetSemanticName();
        if (!aSemanticName.IsNull() && aSemanticName->Length() > 0)
          anItem["semanticName"] = aSemanticName->ToCString();
        appendAnnotationPresentation(anItem, aTolerance->GetPresentation(),
                                     aTolerance->GetPresentationName());
        const char* aMaterialRequirement = documentGeomToleranceMaterialRequirementName(
          aTolerance->GetMaterialRequirementModifier());
        if (aMaterialRequirement != nullptr && std::string(aMaterialRequirement) != "none")
          anItem["materialRequirement"] = aMaterialRequirement;
        const char* aZoneModifier = documentGeomToleranceZoneModifierName(
          aTolerance->GetZoneModifier());
        if (aZoneModifier != nullptr && std::string(aZoneModifier) != "none")
          anItem["zoneModifier"] = aZoneModifier;
        if (anItem.contains("zoneModifier") && aTolerance->GetValueOfZoneModifier() > 0.0)
          anItem["zoneModifierValue"] = aTolerance->GetValueOfZoneModifier();
        const NCollection_Sequence<XCAFDimTolObjects_GeomToleranceModif> aModifiers =
          aTolerance->GetModifiers();
        if (!aModifiers.IsEmpty())
        {
          anItem["modifiers"] = json::array();
          for (int aModifierIndex = 1; aModifierIndex <= aModifiers.Size(); ++aModifierIndex)
          {
            const char* aModifierName = documentGeomToleranceModifierName(
              aModifiers.Value(aModifierIndex));
            if (aModifierName != nullptr) anItem["modifiers"].push_back(aModifierName);
          }
          if (anItem["modifiers"].empty()) anItem.erase("modifiers");
        }
        if (aTolerance->GetMaxValueModifier() > 0.0)
          anItem["maxValueModifier"] = aTolerance->GetMaxValueModifier();
        anItem["datumIndices"] = json::array();
        NCollection_Sequence<TDF_Label> aDatumOfTolerance;
        if (XCAFDoc_DimTolTool::GetDatumOfTolerLabels(aToleranceLabel, aDatumOfTolerance))
        {
          for (int aDatumRefIndex = 1; aDatumRefIndex <= aDatumOfTolerance.Size(); ++aDatumRefIndex)
          {
            for (std::size_t anOutputIndex = 0; anOutputIndex < aOutputDatumLabels.size(); ++anOutputIndex)
            {
              if (aDatumOfTolerance.Value(aDatumRefIndex).IsEqual(aOutputDatumLabels[anOutputIndex]))
              {
                anItem["datumIndices"].push_back(anOutputIndex);
                break;
              }
            }
          }
        }
        if (anItem["datumIndices"].empty()) anItem.erase("datumIndices");
        aGeometricTolerances.push_back(std::move(anItem));
      }
      json aViews = json::array();
      NCollection_Sequence<TDF_Label> aViewLabels;
      aViewTool->GetViewLabels(aViewLabels);
      for (int aViewIndex = 1; aViewIndex <= aViewLabels.Size(); ++aViewIndex)
      {
        occ::handle<XCAFDoc_View> aViewAttribute;
        if (!aViewLabels.Value(aViewIndex).FindAttribute(XCAFDoc_View::GetID(), aViewAttribute)
            || aViewAttribute.IsNull()) continue;
        const occ::handle<XCAFView_Object> aViewObject = aViewAttribute->GetObject();
        if (aViewObject.IsNull()) continue;
        json aView = {
          {"name", aViewObject->Name().IsNull() ? "" : aViewObject->Name()->ToCString()},
          {"projection", aViewObject->Type() == XCAFView_ProjectionType_Parallel ? "parallel"
                        : aViewObject->Type() == XCAFView_ProjectionType_Central ? "central" : "none"},
          {"projectionPoint", {aViewObject->ProjectionPoint().X(), aViewObject->ProjectionPoint().Y(), aViewObject->ProjectionPoint().Z()}},
          {"viewDirection", {aViewObject->ViewDirection().X(), aViewObject->ViewDirection().Y(), aViewObject->ViewDirection().Z()}},
          {"upDirection", {aViewObject->UpDirection().X(), aViewObject->UpDirection().Y(), aViewObject->UpDirection().Z()}},
          {"zoomFactor", aViewObject->ZoomFactor()},
          {"windowHorizontalSize", aViewObject->WindowHorizontalSize()},
          {"windowVerticalSize", aViewObject->WindowVerticalSize()},
          {"viewVolumeSidesClipping", aViewObject->HasViewVolumeSidesClipping()},
          {"nodeIndices", json::array()}
        };
        if (aViewObject->HasFrontPlaneClipping()) aView["frontPlaneDistance"] = aViewObject->FrontPlaneDistance();
        if (aViewObject->HasBackPlaneClipping()) aView["backPlaneDistance"] = aViewObject->BackPlaneDistance();
        aView["clippingPlanes"] = json::array();
        NCollection_Sequence<TDF_Label> aClippingPlaneLabels;
        if (aViewTool->GetRefClippingPlaneLabel(aViewLabels.Value(aViewIndex),
                                                aClippingPlaneLabels))
        {
          for (int aPlaneIndex = 1; aPlaneIndex <= aClippingPlaneLabels.Size(); ++aPlaneIndex)
          {
            gp_Pln aPlane;
            TCollection_ExtendedString aPlaneName;
            bool aCapping = false;
            if (!aClippingPlaneTool->GetClippingPlane(
                  aClippingPlaneLabels.Value(aPlaneIndex), aPlane, aPlaneName, aCapping))
              continue;
            const gp_Pnt& anOrigin = aPlane.Location();
            const gp_Dir& aNormal = aPlane.Axis().Direction();
            aView["clippingPlanes"].push_back({
              {"name", TCollection_AsciiString(aPlaneName).ToCString()},
              {"origin", {anOrigin.X(), anOrigin.Y(), anOrigin.Z()}},
              {"normal", {aNormal.X(), aNormal.Y(), aNormal.Z()}},
              {"capping", aCapping}});
          }
        }
        NCollection_Sequence<TDF_Label> aShapeLabels;
        if (aViewTool->GetRefShapeLabel(aViewLabels.Value(aViewIndex), aShapeLabels))
          for (int aShapeIndex = 1; aShapeIndex <= aShapeLabels.Size(); ++aShapeIndex)
          {
            const std::size_t aNodeIndex = nodeIndexForLabel(aShapeLabels.Value(aShapeIndex));
            if (aNodeIndex < aNodeOccurrenceLabels.size()) aView["nodeIndices"].push_back(aNodeIndex);
          }
        if (!aView["nodeIndices"].empty()) aViews.push_back(std::move(aView));
      }
      json aShuo = json::array();
      std::unordered_set<std::string> aShuoPaths;
      for (const TDF_Label& aNodeLabel : aNodeOccurrenceLabels)
      {
        NCollection_Sequence<occ::handle<TDF_Attribute>> aAttributes;
        if (!XCAFDoc_ShapeTool::GetAllComponentSHUO(aNodeLabel, aAttributes)) continue;
        for (int anAttributeIndex = 1; anAttributeIndex <= aAttributes.Size(); ++anAttributeIndex)
        {
          const occ::handle<XCAFDoc_GraphNode> aStart = occ::down_cast<XCAFDoc_GraphNode>(aAttributes.Value(anAttributeIndex));
          if (aStart.IsNull() || aStart->NbFathers() != 0) continue;
          json aPath = json::array();
          std::string aPathKey;
          occ::handle<XCAFDoc_GraphNode> aCurrent = aStart;
          while (!aCurrent.IsNull())
          {
            const std::size_t aNodeIndex = nodeIndexForLabel(aCurrent->Label().Father());
            if (aNodeIndex >= aNodeOccurrenceLabels.size()) break;
            if (!aPathKey.empty()) aPathKey += "/";
            aPathKey += std::to_string(aNodeIndex);
            aPath.push_back(aNodeIndex);
            if (aCurrent->NbChildren() != 1) break;
            aCurrent = aCurrent->GetChild(1);
          }
          if (aPath.size() >= 2 && aShuoPaths.insert(aPathKey).second)
          {
            json aDefinition = {{"nodeIndices", std::move(aPath)}};
            Quantity_ColorRGBA aColor;
            if (XCAFDoc_ColorTool::GetColor(aStart->Label(), XCAFDoc_ColorGen, aColor))
            {
              const Quantity_Color& anRgb = aColor.GetRGB();
              aDefinition["color"] = {anRgb.Red(), anRgb.Green(), anRgb.Blue(), aColor.Alpha()};
            }
            aShuo.push_back(std::move(aDefinition));
          }
        }
      }
  return {std::move(aGdt), std::move(aDatums), std::move(aGeometricTolerances),
          std::move(aViews), std::move(aShuo)};
}


void applyDocumentAnnotations(
  KernelOperationContext& theContext,
  const bool theIsStepDocument,
  const json& theArgs,
  const json& aNodes,
  const std::vector<TDF_Label>& aLabels,
  const std::size_t aNodeCount,
  const occ::handle<XCAFDoc_DimTolTool>& aDimTolTool)
{
      auto applyAnnotationPresentation = [&](const json& theItem, const auto& theObject,
                                             const char* theKind) {
        if (!theItem.contains("presentation")) return;
        const json& aPresentation = theItem.at("presentation");
        if (!aPresentation.is_object() || !aPresentation.contains("shape")
            || (!aPresentation.at("shape").is_number_unsigned()
                && !aPresentation.at("shape").is_number_integer())
            || (aPresentation.contains("name") && !aPresentation.at("name").is_string()))
          throw KernelFailure(ErrorCode::InvalidArgs,
                              std::string("XCAF ") + theKind
                              + " presentation requires a shape and optional name");
        occ::handle<TCollection_HAsciiString> aName;
        if (aPresentation.contains("name"))
          aName = new TCollection_HAsciiString(
            aPresentation.at("name").get<std::string>().c_str());
        theObject->SetPresentation(
          theContext.arena().get(requiredU32(aPresentation, "shape")), aName);
      };
      if (theArgs.contains("gdt"))
      {
        const json& aGdt = theArgs.at("gdt");
        if (!aGdt.is_array())
          throw KernelFailure(ErrorCode::InvalidArgs, "STEP document gdt must be an array");
        for (const json& anItem : aGdt)
        {
          if (!anItem.is_object() || !anItem.contains("node") || !anItem.at("node").is_number_integer()
              || !anItem.contains("type") || !anItem.at("type").is_string()
              || !anItem.contains("value") || !anItem.at("value").is_number())
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "STEP document gdt requires a supported type, node, and value");
          XCAFDimTolObjects_DimensionType aType;
          if (!documentDimensionTypeFromName(anItem.at("type").get<std::string>(), aType))
            throw KernelFailure(ErrorCode::InvalidArgs, "STEP document gdt type is unsupported");
          const std::int64_t aNode = anItem.at("node").get<std::int64_t>();
          const double aValue = anItem.at("value").get<double>();
          if (aNode < 0 || static_cast<std::size_t>(aNode) >= aNodeCount
              || aNodes[static_cast<std::size_t>(aNode)].at("kind") != "part"
              || !std::isfinite(aValue) || aValue <= 0.0)
            throw KernelFailure(ErrorCode::InvalidArgs, "STEP document gdt entry is invalid");
          const auto aDimension = new XCAFDimTolObjects_DimensionObject();
          aDimension->SetType(aType);
          aDimension->SetValue(aValue);
          if (anItem.contains("semanticName"))
          {
            if (!anItem.at("semanticName").is_string())
              throw KernelFailure(ErrorCode::InvalidArgs, "STEP document gdt semanticName must be a string");
            aDimension->SetSemanticName(new TCollection_HAsciiString(
              anItem.at("semanticName").get<std::string>().c_str()));
          }
          applyAnnotationPresentation(anItem, aDimension, "dimension");
          const TDF_Label aDimensionLabel = aDimTolTool->AddDimension();
          XCAFDoc_Dimension::Set(aDimensionLabel)->SetObject(aDimension);
          aDimTolTool->SetDimension(aLabels[static_cast<std::size_t>(aNode)], aDimensionLabel);
        }
      }

      std::vector<TDF_Label> aInputDatumLabels;
      if (theArgs.contains("datums"))
      {
        const json& aDatums = theArgs.at("datums");
        if (!aDatums.is_array())
          throw KernelFailure(ErrorCode::InvalidArgs, "STEP document datums must be an array");
        if (theIsStepDocument && !aDatums.empty())
          throw KernelFailure(ErrorCode::ImportExportFailed,
                              "STEP transfer cannot preserve XCAF datums; use exportXCAF");
        for (const json& anItem : aDatums)
        {
          if (!anItem.is_object() || !anItem.contains("node")
              || !anItem.at("node").is_number_integer()
              || !anItem.contains("name") || !anItem.at("name").is_string()
              || !anItem.contains("identification") || !anItem.at("identification").is_string())
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "STEP document datum requires node, name, and identification");
          const std::int64_t aNode = anItem.at("node").get<std::int64_t>();
          if (aNode < 0 || static_cast<std::size_t>(aNode) >= aNodeCount
              || aNodes[static_cast<std::size_t>(aNode)].at("kind") != "part")
            throw KernelFailure(ErrorCode::InvalidArgs, "STEP document datum node is invalid");
          auto datumString = [&](const char* theKey, const char* theDefault) {
            if (!anItem.contains(theKey))
              return occ::handle<TCollection_HAsciiString>(new TCollection_HAsciiString(theDefault));
            if (!anItem.at(theKey).is_string())
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  std::string("STEP document datum ") + theKey + " must be a string");
            return occ::handle<TCollection_HAsciiString>(
              new TCollection_HAsciiString(anItem.at(theKey).get<std::string>().c_str()));
          };
          const auto aName = datumString("name", "");
          const auto aDescription = datumString("description", "");
          const auto anIdentification = datumString("identification", "");
          const TDF_Label aDatumLabel = aDimTolTool->AddDatum(aName, aDescription, anIdentification);
          occ::handle<XCAFDoc_Datum> aDatumAttribute;
          if (!aDatumLabel.FindAttribute(XCAFDoc_Datum::GetID(), aDatumAttribute)
              || aDatumAttribute.IsNull())
            throw KernelFailure(ErrorCode::ImportExportFailed, "OCCT could not create XCAF datum");
          const auto aDatumObject = aDatumAttribute->GetObject();
          if (anItem.contains("semanticName"))
          {
            if (!anItem.at("semanticName").is_string())
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "STEP document datum semanticName must be a string");
            aDatumObject->SetSemanticName(new TCollection_HAsciiString(
              anItem.at("semanticName").get<std::string>().c_str()));
          }
          applyAnnotationPresentation(anItem, aDatumObject, "datum");
          aDatumAttribute->SetObject(aDatumObject);
          NCollection_Sequence<TDF_Label> aDatumShapeLabels;
          aDatumShapeLabels.Append(aLabels[static_cast<std::size_t>(aNode)]);
          aDimTolTool->SetDatum(aDatumShapeLabels, aDatumLabel);
          aInputDatumLabels.push_back(aDatumLabel);
        }
      }
      if (theArgs.contains("geometricTolerances"))
      {
        const json& aTolerances = theArgs.at("geometricTolerances");
        if (!aTolerances.is_array())
          throw KernelFailure(ErrorCode::InvalidArgs, "XCAF geometricTolerances must be an array");
        if (theIsStepDocument && !aTolerances.empty())
          throw KernelFailure(ErrorCode::ImportExportFailed,
                              "STEP transfer cannot preserve XCAF geometric tolerance links; use exportXCAF");
        for (const json& anItem : aTolerances)
        {
          if (!anItem.is_object() || !anItem.contains("node")
              || !anItem.at("node").is_number_integer()
              || !anItem.contains("type") || !anItem.at("type").is_string()
              || !anItem.contains("value") || !anItem.at("value").is_number())
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "XCAF geometric tolerance requires node, type, and value");
          const std::int64_t aNode = anItem.at("node").get<std::int64_t>();
          const double aValue = anItem.at("value").get<double>();
          if (aNode < 0 || static_cast<std::size_t>(aNode) >= aNodeCount
              || aNodes[static_cast<std::size_t>(aNode)].at("kind") != "part"
              || !std::isfinite(aValue) || aValue <= 0.0)
            throw KernelFailure(ErrorCode::InvalidArgs, "XCAF geometric tolerance entry is invalid");
          XCAFDimTolObjects_GeomToleranceType aType;
          if (!documentGeomToleranceTypeFromName(anItem.at("type").get<std::string>(), aType))
            throw KernelFailure(ErrorCode::InvalidArgs, "XCAF geometric tolerance type is unsupported");
          XCAFDimTolObjects_GeomToleranceTypeValue aValueType = XCAFDimTolObjects_GeomToleranceTypeValue_None;
          if (anItem.contains("valueType"))
          {
            if (!anItem.at("valueType").is_string()
                || !documentGeomToleranceValueTypeFromName(anItem.at("valueType").get<std::string>(), aValueType))
              throw KernelFailure(ErrorCode::InvalidArgs, "XCAF geometric tolerance valueType is unsupported");
          }
          const auto aTolerance = new XCAFDimTolObjects_GeomToleranceObject();
          aTolerance->SetType(aType);
          aTolerance->SetValue(aValue);
          aTolerance->SetTypeOfValue(aValueType);
          if (anItem.contains("materialRequirement"))
          {
            if (!anItem.at("materialRequirement").is_string())
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "XCAF geometric tolerance materialRequirement is unsupported");
            XCAFDimTolObjects_GeomToleranceMatReqModif aModifier;
            if (!documentGeomToleranceMaterialRequirementFromName(
                  anItem.at("materialRequirement").get<std::string>(), aModifier))
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "XCAF geometric tolerance materialRequirement is unsupported");
            aTolerance->SetMaterialRequirementModifier(aModifier);
          }
          if (anItem.contains("zoneModifier"))
          {
            if (!anItem.at("zoneModifier").is_string())
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "XCAF geometric tolerance zoneModifier is unsupported");
            XCAFDimTolObjects_GeomToleranceZoneModif aModifier;
            if (!documentGeomToleranceZoneModifierFromName(
                  anItem.at("zoneModifier").get<std::string>(), aModifier))
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "XCAF geometric tolerance zoneModifier is unsupported");
            aTolerance->SetZoneModifier(aModifier);
          }
          if (anItem.contains("zoneModifierValue"))
          {
            if (!anItem.at("zoneModifierValue").is_number())
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "XCAF geometric tolerance zoneModifierValue must be positive");
            const double aModifierValue = anItem.at("zoneModifierValue").get<double>();
            if (!std::isfinite(aModifierValue) || aModifierValue <= 0.0)
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "XCAF geometric tolerance zoneModifierValue must be positive");
            aTolerance->SetValueOfZoneModifier(aModifierValue);
          }
          if (anItem.contains("modifiers"))
          {
            if (!anItem.at("modifiers").is_array())
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "XCAF geometric tolerance modifiers must be an array");
            std::unordered_set<std::string> aSeenModifiers;
            for (const json& aModifierValue : anItem.at("modifiers"))
            {
              if (!aModifierValue.is_string())
                throw KernelFailure(ErrorCode::InvalidArgs,
                                    "XCAF geometric tolerance modifier is unsupported");
              const std::string aModifier = aModifierValue.get<std::string>();
              if (!aSeenModifiers.insert(aModifier).second)
                throw KernelFailure(ErrorCode::InvalidArgs,
                                    "XCAF geometric tolerance modifiers must be unique");
              XCAFDimTolObjects_GeomToleranceModif anOcctModifier;
              if (!documentGeomToleranceModifierFromName(aModifier, anOcctModifier))
                throw KernelFailure(ErrorCode::InvalidArgs,
                                    "XCAF geometric tolerance modifier is unsupported");
              aTolerance->AddModifier(anOcctModifier);
            }
          }
          if (anItem.contains("maxValueModifier"))
          {
            if (!anItem.at("maxValueModifier").is_number())
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "XCAF geometric tolerance maxValueModifier must be positive");
            const double aMaxValue = anItem.at("maxValueModifier").get<double>();
            if (!std::isfinite(aMaxValue) || aMaxValue <= 0.0)
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "XCAF geometric tolerance maxValueModifier must be positive");
            aTolerance->SetMaxValueModifier(aMaxValue);
          }
          if (anItem.contains("semanticName"))
          {
            if (!anItem.at("semanticName").is_string())
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "XCAF geometric tolerance semanticName must be a string");
            aTolerance->SetSemanticName(new TCollection_HAsciiString(
              anItem.at("semanticName").get<std::string>().c_str()));
          }
          applyAnnotationPresentation(anItem, aTolerance, "geometric tolerance");
          const TDF_Label aToleranceLabel = aDimTolTool->AddGeomTolerance();
          XCAFDoc_GeomTolerance::Set(aToleranceLabel)->SetObject(aTolerance);
          aDimTolTool->SetGeomTolerance(aLabels[static_cast<std::size_t>(aNode)], aToleranceLabel);
          if (anItem.contains("datumIndices"))
          {
            if (!anItem.at("datumIndices").is_array())
              throw KernelFailure(ErrorCode::InvalidArgs, "XCAF geometric tolerance datumIndices must be an array");
            for (const json& aDatumIndexValue : anItem.at("datumIndices"))
            {
              if (!aDatumIndexValue.is_number_integer() || aDatumIndexValue.get<std::int64_t>() < 0
                  || static_cast<std::size_t>(aDatumIndexValue.get<std::uint64_t>()) >= aInputDatumLabels.size())
                throw KernelFailure(ErrorCode::InvalidArgs,
                                    "XCAF geometric tolerance datum index is invalid");
              aDimTolTool->SetDatumToGeomTol(
                aInputDatumLabels[aDatumIndexValue.get<std::size_t>()], aToleranceLabel);
            }
          }
        }
      }
}


} // namespace occt_worker
