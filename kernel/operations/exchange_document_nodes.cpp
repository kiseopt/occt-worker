#include "kernel_exchange_helpers.hpp"
#include "kernel_operation_context.hpp"
#include "kernel_protocol_helpers.hpp"

namespace occt_worker {

bool documentDimensionTypeFromName(
  const std::string& theName, XCAFDimTolObjects_DimensionType& theResult)
{
  static const std::pair<const char*, XCAFDimTolObjects_DimensionType> aNames[] = {
    {"curveLength", XCAFDimTolObjects_DimensionType_Size_CurveLength},
    {"diameter", XCAFDimTolObjects_DimensionType_Size_Diameter},
    {"sphericalDiameter", XCAFDimTolObjects_DimensionType_Size_SphericalDiameter},
    {"radius", XCAFDimTolObjects_DimensionType_Size_Radius},
    {"sphericalRadius", XCAFDimTolObjects_DimensionType_Size_SphericalRadius},
    {"toroidalMinorDiameter", XCAFDimTolObjects_DimensionType_Size_ToroidalMinorDiameter},
    {"toroidalMajorDiameter", XCAFDimTolObjects_DimensionType_Size_ToroidalMajorDiameter},
    {"toroidalMinorRadius", XCAFDimTolObjects_DimensionType_Size_ToroidalMinorRadius},
    {"toroidalMajorRadius", XCAFDimTolObjects_DimensionType_Size_ToroidalMajorRadius},
    {"toroidalHighMajorDiameter", XCAFDimTolObjects_DimensionType_Size_ToroidalHighMajorDiameter},
    {"toroidalLowMajorDiameter", XCAFDimTolObjects_DimensionType_Size_ToroidalLowMajorDiameter},
    {"toroidalHighMajorRadius", XCAFDimTolObjects_DimensionType_Size_ToroidalHighMajorRadius},
    {"toroidalLowMajorRadius", XCAFDimTolObjects_DimensionType_Size_ToroidalLowMajorRadius},
    {"thickness", XCAFDimTolObjects_DimensionType_Size_Thickness},
    {"angular", XCAFDimTolObjects_DimensionType_Size_Angular},
  };
  for (const auto& aEntry : aNames)
    if (theName == aEntry.first) { theResult = aEntry.second; return true; }
  return false;
}

const char* documentDimensionTypeName(const XCAFDimTolObjects_DimensionType theType)
{
  switch (theType)
  {
    case XCAFDimTolObjects_DimensionType_Size_CurveLength: return "curveLength";
    case XCAFDimTolObjects_DimensionType_Size_Diameter: return "diameter";
    case XCAFDimTolObjects_DimensionType_Size_SphericalDiameter: return "sphericalDiameter";
    case XCAFDimTolObjects_DimensionType_Size_Radius: return "radius";
    case XCAFDimTolObjects_DimensionType_Size_SphericalRadius: return "sphericalRadius";
    case XCAFDimTolObjects_DimensionType_Size_ToroidalMinorDiameter: return "toroidalMinorDiameter";
    case XCAFDimTolObjects_DimensionType_Size_ToroidalMajorDiameter: return "toroidalMajorDiameter";
    case XCAFDimTolObjects_DimensionType_Size_ToroidalMinorRadius: return "toroidalMinorRadius";
    case XCAFDimTolObjects_DimensionType_Size_ToroidalMajorRadius: return "toroidalMajorRadius";
    case XCAFDimTolObjects_DimensionType_Size_ToroidalHighMajorDiameter: return "toroidalHighMajorDiameter";
    case XCAFDimTolObjects_DimensionType_Size_ToroidalLowMajorDiameter: return "toroidalLowMajorDiameter";
    case XCAFDimTolObjects_DimensionType_Size_ToroidalHighMajorRadius: return "toroidalHighMajorRadius";
    case XCAFDimTolObjects_DimensionType_Size_ToroidalLowMajorRadius: return "toroidalLowMajorRadius";
    case XCAFDimTolObjects_DimensionType_Size_Thickness: return "thickness";
    case XCAFDimTolObjects_DimensionType_Size_Angular: return "angular";
    default: return nullptr;
  }
}

bool documentGeomToleranceTypeFromName(
  const std::string& theName, XCAFDimTolObjects_GeomToleranceType& theResult)
{
  static const std::pair<const char*, XCAFDimTolObjects_GeomToleranceType> aNames[] = {
    {"angularity", XCAFDimTolObjects_GeomToleranceType_Angularity},
    {"circularRunout", XCAFDimTolObjects_GeomToleranceType_CircularRunout},
    {"circularity", XCAFDimTolObjects_GeomToleranceType_CircularityOrRoundness},
    {"coaxiality", XCAFDimTolObjects_GeomToleranceType_Coaxiality},
    {"concentricity", XCAFDimTolObjects_GeomToleranceType_Concentricity},
    {"cylindricity", XCAFDimTolObjects_GeomToleranceType_Cylindricity},
    {"flatness", XCAFDimTolObjects_GeomToleranceType_Flatness},
    {"parallelism", XCAFDimTolObjects_GeomToleranceType_Parallelism},
    {"perpendicularity", XCAFDimTolObjects_GeomToleranceType_Perpendicularity},
    {"position", XCAFDimTolObjects_GeomToleranceType_Position},
    {"profileOfLine", XCAFDimTolObjects_GeomToleranceType_ProfileOfLine},
    {"profileOfSurface", XCAFDimTolObjects_GeomToleranceType_ProfileOfSurface},
    {"straightness", XCAFDimTolObjects_GeomToleranceType_Straightness},
    {"symmetry", XCAFDimTolObjects_GeomToleranceType_Symmetry},
    {"totalRunout", XCAFDimTolObjects_GeomToleranceType_TotalRunout},
  };
  for (const auto& aEntry : aNames)
    if (theName == aEntry.first) { theResult = aEntry.second; return true; }
  return false;
}

const char* documentGeomToleranceTypeName(
  const XCAFDimTolObjects_GeomToleranceType theType)
{
  switch (theType)
  {
    case XCAFDimTolObjects_GeomToleranceType_Angularity: return "angularity";
    case XCAFDimTolObjects_GeomToleranceType_CircularRunout: return "circularRunout";
    case XCAFDimTolObjects_GeomToleranceType_CircularityOrRoundness: return "circularity";
    case XCAFDimTolObjects_GeomToleranceType_Coaxiality: return "coaxiality";
    case XCAFDimTolObjects_GeomToleranceType_Concentricity: return "concentricity";
    case XCAFDimTolObjects_GeomToleranceType_Cylindricity: return "cylindricity";
    case XCAFDimTolObjects_GeomToleranceType_Flatness: return "flatness";
    case XCAFDimTolObjects_GeomToleranceType_Parallelism: return "parallelism";
    case XCAFDimTolObjects_GeomToleranceType_Perpendicularity: return "perpendicularity";
    case XCAFDimTolObjects_GeomToleranceType_Position: return "position";
    case XCAFDimTolObjects_GeomToleranceType_ProfileOfLine: return "profileOfLine";
    case XCAFDimTolObjects_GeomToleranceType_ProfileOfSurface: return "profileOfSurface";
    case XCAFDimTolObjects_GeomToleranceType_Straightness: return "straightness";
    case XCAFDimTolObjects_GeomToleranceType_Symmetry: return "symmetry";
    case XCAFDimTolObjects_GeomToleranceType_TotalRunout: return "totalRunout";
    default: return nullptr;
  }
}

bool documentGeomToleranceValueTypeFromName(
  const std::string& theName, XCAFDimTolObjects_GeomToleranceTypeValue& theResult)
{
  if (theName == "none") theResult = XCAFDimTolObjects_GeomToleranceTypeValue_None;
  else if (theName == "diameter") theResult = XCAFDimTolObjects_GeomToleranceTypeValue_Diameter;
  else if (theName == "sphericalDiameter") theResult = XCAFDimTolObjects_GeomToleranceTypeValue_SphericalDiameter;
  else return false;
  return true;
}

const char* documentGeomToleranceValueTypeName(
  const XCAFDimTolObjects_GeomToleranceTypeValue theType)
{
  switch (theType)
  {
    case XCAFDimTolObjects_GeomToleranceTypeValue_None: return "none";
    case XCAFDimTolObjects_GeomToleranceTypeValue_Diameter: return "diameter";
    case XCAFDimTolObjects_GeomToleranceTypeValue_SphericalDiameter: return "sphericalDiameter";
    default: return nullptr;
  }
}

bool documentGeomToleranceMaterialRequirementFromName(
  const std::string& theName,
  XCAFDimTolObjects_GeomToleranceMatReqModif& theResult)
{
  if (theName == "none") theResult = XCAFDimTolObjects_GeomToleranceMatReqModif_None;
  else if (theName == "maximum") theResult = XCAFDimTolObjects_GeomToleranceMatReqModif_M;
  else if (theName == "least") theResult = XCAFDimTolObjects_GeomToleranceMatReqModif_L;
  else return false;
  return true;
}

const char* documentGeomToleranceMaterialRequirementName(
  const XCAFDimTolObjects_GeomToleranceMatReqModif theType)
{
  switch (theType)
  {
    case XCAFDimTolObjects_GeomToleranceMatReqModif_None: return "none";
    case XCAFDimTolObjects_GeomToleranceMatReqModif_M: return "maximum";
    case XCAFDimTolObjects_GeomToleranceMatReqModif_L: return "least";
    default: return nullptr;
  }
}

bool documentGeomToleranceZoneModifierFromName(
  const std::string& theName,
  XCAFDimTolObjects_GeomToleranceZoneModif& theResult)
{
  if (theName == "none") theResult = XCAFDimTolObjects_GeomToleranceZoneModif_None;
  else if (theName == "projected") theResult = XCAFDimTolObjects_GeomToleranceZoneModif_Projected;
  else if (theName == "runout") theResult = XCAFDimTolObjects_GeomToleranceZoneModif_Runout;
  else if (theName == "nonUniform") theResult = XCAFDimTolObjects_GeomToleranceZoneModif_NonUniform;
  else return false;
  return true;
}

const char* documentGeomToleranceZoneModifierName(
  const XCAFDimTolObjects_GeomToleranceZoneModif theType)
{
  switch (theType)
  {
    case XCAFDimTolObjects_GeomToleranceZoneModif_None: return "none";
    case XCAFDimTolObjects_GeomToleranceZoneModif_Projected: return "projected";
    case XCAFDimTolObjects_GeomToleranceZoneModif_Runout: return "runout";
    case XCAFDimTolObjects_GeomToleranceZoneModif_NonUniform: return "nonUniform";
    default: return nullptr;
  }
}

bool documentGeomToleranceModifierFromName(
  const std::string& theName,
  XCAFDimTolObjects_GeomToleranceModif& theResult)
{
  static const std::pair<const char*, XCAFDimTolObjects_GeomToleranceModif> aNames[] = {
    {"anyCrossSection", XCAFDimTolObjects_GeomToleranceModif_Any_Cross_Section},
    {"commonZone", XCAFDimTolObjects_GeomToleranceModif_Common_Zone},
    {"eachRadialElement", XCAFDimTolObjects_GeomToleranceModif_Each_Radial_Element},
    {"freeState", XCAFDimTolObjects_GeomToleranceModif_Free_State},
    {"leastMaterialRequirement", XCAFDimTolObjects_GeomToleranceModif_Least_Material_Requirement},
    {"lineElement", XCAFDimTolObjects_GeomToleranceModif_Line_Element},
    {"majorDiameter", XCAFDimTolObjects_GeomToleranceModif_Major_Diameter},
    {"maximumMaterialRequirement", XCAFDimTolObjects_GeomToleranceModif_Maximum_Material_Requirement},
    {"minorDiameter", XCAFDimTolObjects_GeomToleranceModif_Minor_Diameter},
    {"notConvex", XCAFDimTolObjects_GeomToleranceModif_Not_Convex},
    {"pitchDiameter", XCAFDimTolObjects_GeomToleranceModif_Pitch_Diameter},
    {"reciprocityRequirement", XCAFDimTolObjects_GeomToleranceModif_Reciprocity_Requirement},
    {"separateRequirement", XCAFDimTolObjects_GeomToleranceModif_Separate_Requirement},
    {"statisticalTolerance", XCAFDimTolObjects_GeomToleranceModif_Statistical_Tolerance},
    {"tangentPlane", XCAFDimTolObjects_GeomToleranceModif_Tangent_Plane},
    {"allAround", XCAFDimTolObjects_GeomToleranceModif_All_Around},
    {"allOver", XCAFDimTolObjects_GeomToleranceModif_All_Over},
  };
  for (const auto& aEntry : aNames)
    if (theName == aEntry.first) { theResult = aEntry.second; return true; }
  return false;
}

const char* documentGeomToleranceModifierName(
  const XCAFDimTolObjects_GeomToleranceModif theType)
{
  switch (theType)
  {
    case XCAFDimTolObjects_GeomToleranceModif_Any_Cross_Section: return "anyCrossSection";
    case XCAFDimTolObjects_GeomToleranceModif_Common_Zone: return "commonZone";
    case XCAFDimTolObjects_GeomToleranceModif_Each_Radial_Element: return "eachRadialElement";
    case XCAFDimTolObjects_GeomToleranceModif_Free_State: return "freeState";
    case XCAFDimTolObjects_GeomToleranceModif_Least_Material_Requirement: return "leastMaterialRequirement";
    case XCAFDimTolObjects_GeomToleranceModif_Line_Element: return "lineElement";
    case XCAFDimTolObjects_GeomToleranceModif_Major_Diameter: return "majorDiameter";
    case XCAFDimTolObjects_GeomToleranceModif_Maximum_Material_Requirement: return "maximumMaterialRequirement";
    case XCAFDimTolObjects_GeomToleranceModif_Minor_Diameter: return "minorDiameter";
    case XCAFDimTolObjects_GeomToleranceModif_Not_Convex: return "notConvex";
    case XCAFDimTolObjects_GeomToleranceModif_Pitch_Diameter: return "pitchDiameter";
    case XCAFDimTolObjects_GeomToleranceModif_Reciprocity_Requirement: return "reciprocityRequirement";
    case XCAFDimTolObjects_GeomToleranceModif_Separate_Requirement: return "separateRequirement";
    case XCAFDimTolObjects_GeomToleranceModif_Statistical_Tolerance: return "statisticalTolerance";
    case XCAFDimTolObjects_GeomToleranceModif_Tangent_Plane: return "tangentPlane";
    case XCAFDimTolObjects_GeomToleranceModif_All_Around: return "allAround";
    case XCAFDimTolObjects_GeomToleranceModif_All_Over: return "allOver";
    default: return nullptr;
  }
}

void validateDocumentTree(KernelOperationContext& theContext,
                          const json& theNodes,
                          const json& theRoots)
{
  if (!theNodes.is_array() || theNodes.empty())
    throw KernelFailure(ErrorCode::InvalidArgs, "STEP document requires at least one node");
  if (!theRoots.is_array() || theRoots.empty())
    throw KernelFailure(ErrorCode::InvalidArgs, "STEP document requires at least one root");

  const std::size_t aNodeCount = theNodes.size();
  std::vector<std::size_t> aParentCounts(aNodeCount, 0);
  std::vector<bool> isRoot(aNodeCount, false);
  for (const json& aRootValue : theRoots)
  {
    if (!aRootValue.is_number_unsigned() && !aRootValue.is_number_integer())
      throw KernelFailure(ErrorCode::InvalidArgs, "STEP document root indices must be integers");
    const std::int64_t aRoot = aRootValue.get<std::int64_t>();
    if (aRoot < 0 || static_cast<std::size_t>(aRoot) >= aNodeCount || isRoot[aRoot])
      throw KernelFailure(ErrorCode::InvalidArgs, "STEP document root index is invalid or duplicated");
    isRoot[aRoot] = true;
  }
  for (std::size_t aNodeIndex = 0; aNodeIndex < aNodeCount; ++aNodeIndex)
  {
    const json& aNode = theNodes[aNodeIndex];
    if (!aNode.is_object() || !aNode.contains("kind") || !aNode.at("kind").is_string())
      throw KernelFailure(ErrorCode::InvalidArgs, "STEP document node kind is required");
    const std::string aKind = aNode.at("kind").get<std::string>();
    const json aChildren = aNode.value("children", json::array());
    if (!aChildren.is_array())
      throw KernelFailure(ErrorCode::InvalidArgs, "STEP document node children must be an array");
    if (aKind == "part")
    {
      if (!aNode.contains("shape") || !aChildren.empty())
        throw KernelFailure(ErrorCode::InvalidArgs, "STEP part nodes require shape and cannot have children");
      theContext.arena().get(requiredU32(aNode, "shape"));
    }
    else if (aKind == "assembly")
    {
      if (aChildren.empty())
        throw KernelFailure(ErrorCode::InvalidArgs, "STEP assembly nodes require children");
    }
    else
      throw KernelFailure(ErrorCode::InvalidArgs, "STEP document node kind must be part or assembly");
    for (const json& aChildValue : aChildren)
    {
      if (!aChildValue.is_number_unsigned() && !aChildValue.is_number_integer())
        throw KernelFailure(ErrorCode::InvalidArgs, "STEP document child indices must be integers");
      const std::int64_t aChild = aChildValue.get<std::int64_t>();
      if (aChild < 0 || static_cast<std::size_t>(aChild) >= aNodeCount
          || static_cast<std::size_t>(aChild) == aNodeIndex)
        throw KernelFailure(ErrorCode::InvalidArgs, "STEP document child index is invalid");
      ++aParentCounts[aChild];
    }
  }
  for (std::size_t aNodeIndex = 0; aNodeIndex < aNodeCount; ++aNodeIndex)
  {
    const bool shouldBeRoot = aParentCounts[aNodeIndex] == 0;
    if (isRoot[aNodeIndex] != shouldBeRoot || aParentCounts[aNodeIndex] > 1)
      throw KernelFailure(ErrorCode::InvalidArgs, "STEP document nodes must form rooted trees");
  }
  std::vector<int> aVisitState(aNodeCount, 0);
  std::function<void(std::size_t)> visitNode = [&](const std::size_t theNodeIndex) {
    if (aVisitState[theNodeIndex] == 1)
      throw KernelFailure(ErrorCode::InvalidArgs, "STEP document contains a cycle");
    if (aVisitState[theNodeIndex] == 2) return;
    aVisitState[theNodeIndex] = 1;
    for (const json& aChild : theNodes[theNodeIndex].value("children", json::array()))
      visitNode(aChild.get<std::size_t>());
    aVisitState[theNodeIndex] = 2;
  };
  for (std::size_t aNodeIndex = 0; aNodeIndex < aNodeCount; ++aNodeIndex)
    if (isRoot[aNodeIndex]) visitNode(aNodeIndex);
  for (const int aState : aVisitState)
    if (aState != 2)
      throw KernelFailure(ErrorCode::InvalidArgs, "STEP document contains nodes outside its rooted trees");
}

DocumentNodeData extractDocumentNodes(
  KernelOperationContext& theContext,
    const bool theRestoreIgesHierarchy,
    const json& aIgesMetadata,
    const std::uint32_t aScope,
    const occ::handle<XCAFDoc_ShapeTool>& aShapeTool,
    const occ::handle<XCAFDoc_ColorTool>& aColorTool,
    const occ::handle<XCAFDoc_LayerTool>& aLayerTool,
    const occ::handle<XCAFDoc_VisMaterialTool>& aVisMaterialTool,
    const NCollection_Sequence<TDF_Label>& aRootLabels)
{
  auto nameForLabel = [](const TDF_Label& theLabel) {
    occ::handle<TDataStd_Name> aName;
    if (!theLabel.FindAttribute(TDataStd_Name::GetID(), aName) || aName.IsNull())
      return std::string();
    return std::string(TCollection_AsciiString(aName->Get()).ToCString());
  };
  auto colorForLabel = [](const TDF_Label& theLabel, Quantity_ColorRGBA& theColor) {
    return XCAFDoc_ColorTool::GetColor(theLabel, XCAFDoc_ColorGen, theColor)
           || XCAFDoc_ColorTool::GetColor(theLabel, XCAFDoc_ColorSurf, theColor)
           || XCAFDoc_ColorTool::GetColor(theLabel, XCAFDoc_ColorCurv, theColor);
  };
  auto layersForLabel = [&](const TDF_Label& theLabel) {
    json aLayers = json::array();
    std::unordered_set<std::string> aSeen;
    std::function<void(const TDF_Label&)> appendLayers = [&](const TDF_Label& theCurrent) {
      NCollection_Sequence<TDF_Label> aLayerLabels;
      if (aLayerTool->GetLayers(theCurrent, aLayerLabels))
        for (int anIndex = 1; anIndex <= aLayerLabels.Size(); ++anIndex)
        {
          TCollection_ExtendedString aLayerName;
          if (!aLayerTool->GetLayer(aLayerLabels.Value(anIndex), aLayerName)) continue;
          const std::string aName(TCollection_AsciiString(aLayerName).ToCString());
          if (aSeen.insert(aName).second) aLayers.push_back(aName);
        }
      NCollection_Sequence<TDF_Label> aSubshapeLabels;
      if (!XCAFDoc_ShapeTool::GetSubShapes(theCurrent, aSubshapeLabels)) return;
      for (int anIndex = 1; anIndex <= aSubshapeLabels.Size(); ++anIndex)
        appendLayers(aSubshapeLabels.Value(anIndex));
    };
    appendLayers(theLabel);
    return aLayers;
  };
  auto materialForLabel = [](const TDF_Label& theLabel, json& theMaterial) {
    occ::handle<TDataStd_TreeNode> aReference;
    if (!theLabel.FindAttribute(XCAFDoc::MaterialRefGUID(), aReference)
        || aReference.IsNull() || !aReference->HasFather())
      return false;
    occ::handle<TCollection_HAsciiString> aName;
    occ::handle<TCollection_HAsciiString> aDescription;
    occ::handle<TCollection_HAsciiString> aDensityName;
    occ::handle<TCollection_HAsciiString> aDensityValueType;
    double aDensity = 0.0;
    if (!XCAFDoc_MaterialTool::GetMaterial(aReference->Father()->Label(), aName,
                                           aDescription, aDensity, aDensityName,
                                           aDensityValueType))
      return false;
    theMaterial = {{"name", aName.IsNull() ? "" : aName->ToCString()},
                   {"density", aDensity}};
    if (!aDescription.IsNull() && aDescription->Length() > 0)
      theMaterial["description"] = aDescription->ToCString();
    if (!aDensityName.IsNull() && aDensityName->Length() > 0)
      theMaterial["densityName"] = aDensityName->ToCString();
    if (!aDensityValueType.IsNull() && aDensityValueType->Length() > 0)
      theMaterial["densityValueType"] = aDensityValueType->ToCString();
    return true;
  };
  auto visualMaterialForLabel = [&](const TDF_Label& theLabel, json& theMaterial) {
    TDF_Label aMaterialLabel;
    if (!aVisMaterialTool->GetShapeMaterial(theLabel, aMaterialLabel)) return false;
    const occ::handle<XCAFDoc_VisMaterial> aMaterial =
      XCAFDoc_VisMaterialTool::GetMaterial(aMaterialLabel);
    if (aMaterial.IsNull() || !aMaterial->HasPbrMaterial()) return false;
    const XCAFDoc_VisMaterialPBR& aPbr = aMaterial->PbrMaterial();
    const Quantity_Color& aBaseColor = aPbr.BaseColor.GetRGB();
    theMaterial = {
      {"name", nameForLabel(aMaterialLabel)},
      {"baseColor", {aBaseColor.Red(), aBaseColor.Green(), aBaseColor.Blue(),
                     aPbr.BaseColor.Alpha()}},
      {"metallic", aPbr.Metallic},
      {"roughness", aPbr.Roughness},
      {"emissive", {aPbr.EmissiveFactor.x(), aPbr.EmissiveFactor.y(),
                    aPbr.EmissiveFactor.z()}},
      {"refractionIndex", aPbr.RefractionIndex}
    };
    return true;
  };
  auto validationPropertiesForLabel = [](const TDF_Label& theLabel, json& theProperties) {
    double anArea = 0.0;
    double aVolume = 0.0;
    gp_Pnt aCentroid;
    if (!theProperties.contains("area") && XCAFDoc_Area::Get(theLabel, anArea))
      theProperties["area"] = anArea;
    if (!theProperties.contains("volume") && XCAFDoc_Volume::Get(theLabel, aVolume))
      theProperties["volume"] = aVolume;
    if (!theProperties.contains("centroid") && XCAFDoc_Centroid::Get(theLabel, aCentroid))
      theProperties["centroid"] = {aCentroid.X(), aCentroid.Y(), aCentroid.Z()};
  };
  auto subshapeStylesForLabels = [&](const TDF_Label& theOccurrence,
                                     const TDF_Label& theDefinition,
                                     const TopoDS_Shape& theShape) {
    TopTools_IndexedMapOfShape aFaces;
    TopTools_IndexedMapOfShape anEdges;
    TopExp::MapShapes(theShape, TopAbs_FACE, aFaces);
    TopExp::MapShapes(theShape, TopAbs_EDGE, anEdges);
    std::unordered_map<std::string, json> aStyles;
    auto appendLabelStyles = [&](const TDF_Label& theLabel) {
      NCollection_Sequence<TDF_Label> aSubshapeLabels;
      if (!XCAFDoc_ShapeTool::GetSubShapes(theLabel, aSubshapeLabels)) return;
      for (int aLabelIndex = 1; aLabelIndex <= aSubshapeLabels.Size(); ++aLabelIndex)
      {
        const TDF_Label& aSubshapeLabel = aSubshapeLabels.Value(aLabelIndex);
        const TopoDS_Shape aSubshape = XCAFDoc_ShapeTool::GetShape(aSubshapeLabel);
        const TopTools_IndexedMapOfShape* aMap = nullptr;
        XCAFDoc_ColorType aColorType = XCAFDoc_ColorSurf;
        const char* aTopology = nullptr;
        if (aSubshape.ShapeType() == TopAbs_FACE)
        {
          aMap = &aFaces;
          aTopology = "face";
        }
        else if (aSubshape.ShapeType() == TopAbs_EDGE)
        {
          aMap = &anEdges;
          aColorType = XCAFDoc_ColorCurv;
          aTopology = "edge";
        }
        else
        {
          continue;
        }
        const int aSubshapeIndex = aMap->FindIndex(aSubshape);
        if (aSubshapeIndex == 0) continue;
        const std::string aKey = std::string(aTopology) + ":"
                                 + std::to_string(aSubshapeIndex - 1);
        if (aStyles.find(aKey) != aStyles.end()) continue;
        Quantity_ColorRGBA aColor;
        if (!XCAFDoc_ColorTool::GetColor(aSubshapeLabel, aColorType, aColor)) continue;
        const Quantity_Color& anRgb = aColor.GetRGB();
        aStyles.emplace(aKey, json{{"topology", aTopology},
                                  {"index", aSubshapeIndex - 1},
                                  {"color", {anRgb.Red(), anRgb.Green(), anRgb.Blue(),
                                             aColor.Alpha()}}});
      }
    };
    appendLabelStyles(theOccurrence);
    if (theDefinition != theOccurrence) appendLabelStyles(theDefinition);

    json aResult = json::array();
    for (const char* aTopology : {"face", "edge"})
    {
      const int aCount = std::strcmp(aTopology, "face") == 0
                           ? aFaces.Extent() : anEdges.Extent();
      for (int anIndex = 0; anIndex < aCount; ++anIndex)
      {
        const auto anItem = aStyles.find(std::string(aTopology) + ":"
                                         + std::to_string(anIndex));
        if (anItem != aStyles.end()) aResult.push_back(anItem->second);
      }
    }
    return aResult;
  };

  json aNodes = json::array();
  std::vector<TDF_Label> aNodeOccurrenceLabels;
  std::vector<TDF_Label> aNodeDefinitionLabels;
  std::function<std::size_t(const TDF_Label&)> appendNode = [&](const TDF_Label& theOccurrence) {
    TDF_Label aDefinition = theOccurrence;
    XCAFDoc_ShapeTool::GetReferredShape(theOccurrence, aDefinition);
    const TopoDS_Shape aShape = XCAFDoc_ShapeTool::GetShape(aDefinition);
    if (aShape.IsNull())
      throw KernelFailure(ErrorCode::ImportExportFailed, "STEP document node has no shape");
    gp_Trsf aTransform = XCAFDoc_ShapeTool::GetLocation(theOccurrence).Transformation();
    const std::size_t aNodeIndex = aNodes.size();
    if (theRestoreIgesHierarchy && aIgesMetadata.contains("nodes")
        && aNodeIndex < aIgesMetadata.at("nodes").size())
    {
      const json& aMetadataNode = aIgesMetadata.at("nodes").at(aNodeIndex);
      if (!aMetadataNode.is_object() || !aMetadataNode.value("layers", json::array()).is_array())
        throw KernelFailure(ErrorCode::ImportExportFailed, "IGES XCAF metadata marker is invalid");
      for (const json& aLayer : aMetadataNode.value("layers", json::array()))
      {
        if (!aLayer.is_string())
          throw KernelFailure(ErrorCode::ImportExportFailed, "IGES XCAF metadata marker is invalid");
        const TCollection_ExtendedString aLayerName(aLayer.get<std::string>().c_str(), true);
        aLayerTool->SetLayer(theOccurrence, aLayerName);
        aLayerTool->SetLayer(aShape, aLayerName);
      }
      if (aMetadataNode.contains("color") && aMetadataNode.at("color").is_array()
          && aMetadataNode.at("color").size() == 4)
      {
        const auto& aColor = aMetadataNode.at("color");
        const Quantity_ColorRGBA aValue(aColor[0].get<float>(), aColor[1].get<float>(),
                                         aColor[2].get<float>(), aColor[3].get<float>());
        aColorTool->SetColor(theOccurrence, aValue, XCAFDoc_ColorGen);
        aColorTool->SetColor(theOccurrence, aValue, XCAFDoc_ColorSurf);
        aColorTool->SetColor(theOccurrence, aValue, XCAFDoc_ColorCurv);
      }
      if (aMetadataNode.contains("transform") && aMetadataNode.at("transform").is_array()
          && aMetadataNode.at("transform").size() == 12)
      {
        const auto& aMatrix = aMetadataNode.at("transform");
        gp_Trsf aTransform;
        aTransform.SetValues(aMatrix[0].get<double>(), aMatrix[1].get<double>(), aMatrix[2].get<double>(), aMatrix[3].get<double>(),
                             aMatrix[4].get<double>(), aMatrix[5].get<double>(), aMatrix[6].get<double>(), aMatrix[7].get<double>(),
                             aMatrix[8].get<double>(), aMatrix[9].get<double>(), aMatrix[10].get<double>(), aMatrix[11].get<double>());
        TDF_Label aLocatedRoot;
        aShapeTool->SetLocation(theOccurrence, TopLoc_Location(aTransform), aLocatedRoot);
        aTransform = XCAFDoc_ShapeTool::GetLocation(theOccurrence).Transformation();
      }
    }
    json aNode = {
      {"kind", XCAFDoc_ShapeTool::IsAssembly(aDefinition) ? "assembly" : "part"},
      {"shape", theContext.arena().add(aShape, aScope)},
      {"children", json::array()},
      {"transform", {
        aTransform.Value(1, 1), aTransform.Value(1, 2), aTransform.Value(1, 3), aTransform.Value(1, 4),
        aTransform.Value(2, 1), aTransform.Value(2, 2), aTransform.Value(2, 3), aTransform.Value(2, 4),
        aTransform.Value(3, 1), aTransform.Value(3, 2), aTransform.Value(3, 3), aTransform.Value(3, 4)}},
      {"layers", layersForLabel(theOccurrence)},
      {"subshapeStyles", subshapeStylesForLabels(theOccurrence, aDefinition, aShape)},
      {"visible", XCAFDoc_ColorTool::IsVisible(theOccurrence)
                  && XCAFDoc_ColorTool::IsVisible(aDefinition)}};
    std::string aName = nameForLabel(theOccurrence);
    if (aName.empty()) aName = nameForLabel(aDefinition);
    if (!aName.empty()) aNode["name"] = aName;
    Quantity_ColorRGBA aColor;
    if (colorForLabel(theOccurrence, aColor) || colorForLabel(aDefinition, aColor))
    {
      const Quantity_Color& anRgb = aColor.GetRGB();
      aNode["color"] = {anRgb.Red(), anRgb.Green(), anRgb.Blue(), aColor.Alpha()};
    }
    if (aNode.at("layers").empty()) aNode["layers"] = layersForLabel(aDefinition);
    json aMaterial;
    if (materialForLabel(theOccurrence, aMaterial)
        || materialForLabel(aDefinition, aMaterial))
      aNode["material"] = std::move(aMaterial);
    json aVisualMaterial;
    if (visualMaterialForLabel(theOccurrence, aVisualMaterial)
        || visualMaterialForLabel(aDefinition, aVisualMaterial))
      aNode["visualMaterial"] = std::move(aVisualMaterial);
    json aValidationProperties;
    validationPropertiesForLabel(theOccurrence, aValidationProperties);
    validationPropertiesForLabel(aDefinition, aValidationProperties);
    if (!aValidationProperties.empty())
      aNode["validationProperties"] = std::move(aValidationProperties);

    aNodes.push_back(std::move(aNode));
    aNodeOccurrenceLabels.push_back(theOccurrence);
    aNodeDefinitionLabels.push_back(aDefinition);
    NCollection_Sequence<TDF_Label> aComponents;
    if (XCAFDoc_ShapeTool::GetComponents(aDefinition, aComponents, false))
    {
      for (int aComponentIndex = 1; aComponentIndex <= aComponents.Size(); ++aComponentIndex)
        aNodes[aNodeIndex]["children"].push_back(appendNode(aComponents.Value(aComponentIndex)));
    }
    return aNodeIndex;
  };

  json aRoots = json::array();
  for (int aRootIndex = 1; aRootIndex <= aRootLabels.Size(); ++aRootIndex)
    aRoots.push_back(appendNode(aRootLabels.Value(aRootIndex)));

  DocumentNodeData aResult;
  aResult.roots = std::move(aRoots);
  aResult.nodes = std::move(aNodes);
  aResult.occurrenceLabels = std::move(aNodeOccurrenceLabels);
  aResult.definitionLabels = std::move(aNodeDefinitionLabels);
  return aResult;
}

} // namespace occt_worker
