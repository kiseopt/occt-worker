#include "kernel_exchange_helpers.hpp"

namespace occt_worker {

void restoreXmlDocumentMetadata(
  const json& theMetadata,
  const std::vector<TDF_Label>& theOccurrenceLabels,
  const occ::handle<XCAFDoc_DimTolTool>& theDimTolTool)
{
  std::vector<TDF_Label> aXmlDatumLabels;
  if (theMetadata.empty())
    return;

  auto markerPresentation = [](const json& theItem) {
    TopoDS_Shape aShape;
    occ::handle<TCollection_HAsciiString> aName;
    if (!theItem.contains("presentationBrep"))
      return std::make_pair(aShape, aName);
    if (!theItem.at("presentationBrep").is_string()
        || (theItem.contains("presentationName")
            && !theItem.at("presentationName").is_string()))
      throw KernelFailure(ErrorCode::ImportExportFailed,
                          "XCAF XML annotation presentation metadata is invalid");
    std::istringstream aStream(theItem.at("presentationBrep").get<std::string>(),
                               std::ios::in | std::ios::binary);
    BRep_Builder aBuilder;
    BRepTools::Read(aShape, aStream, aBuilder);
    if (aShape.IsNull())
      throw KernelFailure(ErrorCode::ImportExportFailed,
                          "XCAF XML annotation presentation metadata is invalid");
    if (theItem.contains("presentationName"))
      aName = new TCollection_HAsciiString(
        theItem.at("presentationName").get<std::string>().c_str());
    return std::make_pair(aShape, aName);
  };

  const json& aDatums = theMetadata.at("datums");
  NCollection_Sequence<TDF_Label> aExistingDatumLabels;
  theDimTolTool->GetDatumLabels(aExistingDatumLabels);
  for (const json& anItem : aDatums)
  {
    if (!anItem.is_object() || !anItem.contains("node") || !anItem.at("node").is_number_integer()
        || !anItem.contains("name") || !anItem.at("name").is_string()
        || !anItem.contains("identification") || !anItem.at("identification").is_string())
      throw KernelFailure(ErrorCode::ImportExportFailed, "XCAF XML datum metadata is invalid");
    const std::int64_t aNode = anItem.at("node").get<std::int64_t>();
    if (aNode < 0 || static_cast<std::size_t>(aNode) >= theOccurrenceLabels.size())
      throw KernelFailure(ErrorCode::ImportExportFailed, "XCAF XML datum metadata is invalid");
    const std::string aName = anItem.at("name").get<std::string>();
    const std::string anIdentification = anItem.at("identification").get<std::string>();
    TDF_Label aDatumLabel;
    for (int anIndex = 1; anIndex <= aExistingDatumLabels.Size(); ++anIndex)
    {
      occ::handle<TCollection_HAsciiString> anExistingName;
      occ::handle<TCollection_HAsciiString> anExistingDescription;
      occ::handle<TCollection_HAsciiString> anExistingIdentification;
      if (theDimTolTool->GetDatum(aExistingDatumLabels.Value(anIndex), anExistingName,
                                   anExistingDescription, anExistingIdentification)
          && !anExistingName.IsNull() && !anExistingIdentification.IsNull()
          && aName == anExistingName->ToCString()
          && anIdentification == anExistingIdentification->ToCString())
      {
        aDatumLabel = aExistingDatumLabels.Value(anIndex);
        break;
      }
    }
    if (aDatumLabel.IsNull())
    {
      const std::string aDescription = anItem.value("description", "");
      aDatumLabel = theDimTolTool->AddDatum(
        new TCollection_HAsciiString(aName.c_str()),
        new TCollection_HAsciiString(aDescription.c_str()),
        new TCollection_HAsciiString(anIdentification.c_str()));
    }
    occ::handle<XCAFDoc_Datum> aDatumAttribute;
    if (!aDatumLabel.FindAttribute(XCAFDoc_Datum::GetID(), aDatumAttribute)
        || aDatumAttribute.IsNull())
      throw KernelFailure(ErrorCode::ImportExportFailed, "XCAF XML datum metadata is invalid");
    const auto aDatumObject = aDatumAttribute->GetObject();
    if (anItem.contains("semanticName"))
    {
      if (!anItem.at("semanticName").is_string())
        throw KernelFailure(ErrorCode::ImportExportFailed, "XCAF XML datum metadata is invalid");
      aDatumObject->SetSemanticName(new TCollection_HAsciiString(
        anItem.at("semanticName").get<std::string>().c_str()));
    }
    const auto aPresentation = markerPresentation(anItem);
    if (!aPresentation.first.IsNull())
      aDatumObject->SetPresentation(aPresentation.first, aPresentation.second);
    aDatumAttribute->SetObject(aDatumObject);
    NCollection_Sequence<TDF_Label> aShapeLabels;
    aShapeLabels.Append(theOccurrenceLabels[static_cast<std::size_t>(aNode)]);
    theDimTolTool->SetDatum(aShapeLabels, aDatumLabel);
    aXmlDatumLabels.push_back(aDatumLabel);
  }

  const json& aTolerances = theMetadata.at("geometricTolerances");
  NCollection_Sequence<TDF_Label> aExistingToleranceLabels;
  theDimTolTool->GetGeomToleranceLabels(aExistingToleranceLabels);
  for (const json& anItem : aTolerances)
  {
    if (!anItem.is_object() || !anItem.contains("node") || !anItem.at("node").is_number_integer()
        || !anItem.contains("type") || !anItem.at("type").is_string()
        || !anItem.contains("value") || !anItem.at("value").is_number())
      throw KernelFailure(ErrorCode::ImportExportFailed,
                          "XCAF XML geometric tolerance metadata is invalid");
    const std::int64_t aNode = anItem.at("node").get<std::int64_t>();
    const double aValue = anItem.at("value").get<double>();
    if (aNode < 0 || static_cast<std::size_t>(aNode) >= theOccurrenceLabels.size()
        || !std::isfinite(aValue) || aValue <= 0.0)
      throw KernelFailure(ErrorCode::ImportExportFailed,
                          "XCAF XML geometric tolerance metadata is invalid");
    XCAFDimTolObjects_GeomToleranceType aType;
    if (!documentGeomToleranceTypeFromName(anItem.at("type").get<std::string>(), aType))
      throw KernelFailure(ErrorCode::ImportExportFailed,
                          "XCAF XML geometric tolerance metadata is invalid");
    XCAFDimTolObjects_GeomToleranceTypeValue aValueType =
      XCAFDimTolObjects_GeomToleranceTypeValue_None;
    if (anItem.contains("valueType")
        && (!anItem.at("valueType").is_string()
            || !documentGeomToleranceValueTypeFromName(
                 anItem.at("valueType").get<std::string>(), aValueType)))
      throw KernelFailure(ErrorCode::ImportExportFailed,
                          "XCAF XML geometric tolerance metadata is invalid");
    const TDF_Label aShapeLabel = theOccurrenceLabels[static_cast<std::size_t>(aNode)];
    TDF_Label aToleranceLabel;
    TDF_Label anUnlinkedCandidate;
    bool hasAmbiguousUnlinkedCandidates = false;
    for (int anIndex = 1; anIndex <= aExistingToleranceLabels.Size(); ++anIndex)
    {
      occ::handle<XCAFDoc_GeomTolerance> anAttribute;
      if (!aExistingToleranceLabels.Value(anIndex).FindAttribute(
            XCAFDoc_GeomTolerance::GetID(), anAttribute) || anAttribute.IsNull())
        continue;
      const auto anObject = anAttribute->GetObject();
      if (anObject.IsNull() || anObject->GetType() != aType
          || std::abs(anObject->GetValue() - aValue) > 1e-12)
        continue;
      NCollection_Sequence<TDF_Label> aRefs;
      NCollection_Sequence<TDF_Label> anUnused;
      if (!XCAFDoc_DimTolTool::GetRefShapeLabel(
            aExistingToleranceLabels.Value(anIndex), aRefs, anUnused)
          || aRefs.IsEmpty())
      {
        if (anUnlinkedCandidate.IsNull())
          anUnlinkedCandidate = aExistingToleranceLabels.Value(anIndex);
        else
          hasAmbiguousUnlinkedCandidates = true;
        continue;
      }
      for (int aRefIndex = 1; aRefIndex <= aRefs.Size(); ++aRefIndex)
        if (aRefs.Value(aRefIndex).IsEqual(aShapeLabel))
        {
          aToleranceLabel = aExistingToleranceLabels.Value(anIndex);
          break;
        }
      if (!aToleranceLabel.IsNull())
        break;
    }
    if (aToleranceLabel.IsNull() && !hasAmbiguousUnlinkedCandidates)
      aToleranceLabel = anUnlinkedCandidate;
    if (aToleranceLabel.IsNull())
      aToleranceLabel = theDimTolTool->AddGeomTolerance();
    TopoDS_Shape aPresentation;
    occ::handle<TCollection_HAsciiString> aPresentationName;
    occ::handle<XCAFDoc_GeomTolerance> anExistingAttribute;
    if (aToleranceLabel.FindAttribute(XCAFDoc_GeomTolerance::GetID(), anExistingAttribute)
        && !anExistingAttribute.IsNull())
    {
      const auto anExistingObject = anExistingAttribute->GetObject();
      if (!anExistingObject.IsNull())
      {
        aPresentation = anExistingObject->GetPresentation();
        aPresentationName = anExistingObject->GetPresentationName();
      }
    }
    const auto aToleranceObject = new XCAFDimTolObjects_GeomToleranceObject();
    aToleranceObject->SetType(aType);
    aToleranceObject->SetValue(aValue);
    aToleranceObject->SetTypeOfValue(aValueType);
    if (anItem.contains("materialRequirement"))
    {
      if (!anItem.at("materialRequirement").is_string())
        throw KernelFailure(ErrorCode::ImportExportFailed,
                            "XCAF XML geometric tolerance metadata is invalid");
      XCAFDimTolObjects_GeomToleranceMatReqModif aModifier;
      if (!documentGeomToleranceMaterialRequirementFromName(
            anItem.at("materialRequirement").get<std::string>(), aModifier))
        throw KernelFailure(ErrorCode::ImportExportFailed,
                            "XCAF XML geometric tolerance metadata is invalid");
      aToleranceObject->SetMaterialRequirementModifier(aModifier);
    }
    if (anItem.contains("zoneModifier"))
    {
      if (!anItem.at("zoneModifier").is_string())
        throw KernelFailure(ErrorCode::ImportExportFailed,
                            "XCAF XML geometric tolerance metadata is invalid");
      XCAFDimTolObjects_GeomToleranceZoneModif aModifier;
      if (!documentGeomToleranceZoneModifierFromName(
            anItem.at("zoneModifier").get<std::string>(), aModifier))
        throw KernelFailure(ErrorCode::ImportExportFailed,
                            "XCAF XML geometric tolerance metadata is invalid");
      aToleranceObject->SetZoneModifier(aModifier);
    }
    if (anItem.contains("zoneModifierValue"))
    {
      if (!anItem.at("zoneModifierValue").is_number())
        throw KernelFailure(ErrorCode::ImportExportFailed,
                            "XCAF XML geometric tolerance metadata is invalid");
      const double aModifierValue = anItem.at("zoneModifierValue").get<double>();
      if (!std::isfinite(aModifierValue) || aModifierValue <= 0.0)
        throw KernelFailure(ErrorCode::ImportExportFailed,
                            "XCAF XML geometric tolerance metadata is invalid");
      aToleranceObject->SetValueOfZoneModifier(aModifierValue);
    }
    if (anItem.contains("modifiers"))
    {
      if (!anItem.at("modifiers").is_array())
        throw KernelFailure(ErrorCode::ImportExportFailed,
                            "XCAF XML geometric tolerance metadata is invalid");
      std::unordered_set<std::string> aSeenModifiers;
      for (const json& aModifierValue : anItem.at("modifiers"))
      {
        if (!aModifierValue.is_string())
          throw KernelFailure(ErrorCode::ImportExportFailed,
                              "XCAF XML geometric tolerance metadata is invalid");
        const std::string aModifier = aModifierValue.get<std::string>();
        if (!aSeenModifiers.insert(aModifier).second)
          throw KernelFailure(ErrorCode::ImportExportFailed,
                              "XCAF XML geometric tolerance metadata is invalid");
        XCAFDimTolObjects_GeomToleranceModif anOcctModifier;
        if (!documentGeomToleranceModifierFromName(aModifier, anOcctModifier))
          throw KernelFailure(ErrorCode::ImportExportFailed,
                              "XCAF XML geometric tolerance metadata is invalid");
        aToleranceObject->AddModifier(anOcctModifier);
      }
    }
    if (anItem.contains("maxValueModifier"))
    {
      if (!anItem.at("maxValueModifier").is_number())
        throw KernelFailure(ErrorCode::ImportExportFailed,
                            "XCAF XML geometric tolerance metadata is invalid");
      const double aMaxValue = anItem.at("maxValueModifier").get<double>();
      if (!std::isfinite(aMaxValue) || aMaxValue <= 0.0)
        throw KernelFailure(ErrorCode::ImportExportFailed,
                            "XCAF XML geometric tolerance metadata is invalid");
      aToleranceObject->SetMaxValueModifier(aMaxValue);
    }
    if (anItem.contains("semanticName"))
    {
      if (!anItem.at("semanticName").is_string())
        throw KernelFailure(ErrorCode::ImportExportFailed,
                            "XCAF XML geometric tolerance metadata is invalid");
      aToleranceObject->SetSemanticName(new TCollection_HAsciiString(
        anItem.at("semanticName").get<std::string>().c_str()));
    }
    const auto aMarkerPresentation = markerPresentation(anItem);
    if (!aMarkerPresentation.first.IsNull())
      aToleranceObject->SetPresentation(aMarkerPresentation.first,
                                        aMarkerPresentation.second);
    else if (!aPresentation.IsNull())
      aToleranceObject->SetPresentation(aPresentation, aPresentationName);
    XCAFDoc_GeomTolerance::Set(aToleranceLabel)->SetObject(aToleranceObject);
    theDimTolTool->SetGeomTolerance(aShapeLabel, aToleranceLabel);
    if (anItem.contains("datumIndices"))
    {
      if (!anItem.at("datumIndices").is_array())
        throw KernelFailure(ErrorCode::ImportExportFailed,
                            "XCAF XML geometric tolerance metadata is invalid");
      for (const json& aDatumIndex : anItem.at("datumIndices"))
      {
        if (!aDatumIndex.is_number_integer() || aDatumIndex.get<std::int64_t>() < 0
            || static_cast<std::size_t>(aDatumIndex.get<std::uint64_t>()) >= aXmlDatumLabels.size())
          throw KernelFailure(ErrorCode::ImportExportFailed,
                              "XCAF XML geometric tolerance metadata is invalid");
        theDimTolTool->SetDatumToGeomTol(
          aXmlDatumLabels[aDatumIndex.get<std::size_t>()], aToleranceLabel);
      }
    }
  }
}

} // namespace occt_worker
