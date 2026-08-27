#include "kernel_exchange_cad_operations.hpp"
#include "kernel_exchange_helpers.hpp"
#include "kernel_operation_context.hpp"
#include "kernel_protocol_helpers.hpp"

namespace occt_worker {

std::array<float, 4> parseNormalizedColor(const json& theValue,
                                                  const char* theError)
{
  if (!theValue.is_array() || theValue.size() != 4)
    throw KernelFailure(ErrorCode::InvalidArgs, theError);
  std::array<float, 4> aValues;
  for (std::size_t anIndex = 0; anIndex < aValues.size(); ++anIndex)
  {
    if (!theValue[anIndex].is_number())
      throw KernelFailure(ErrorCode::InvalidArgs, theError);
    const double aValue = theValue[anIndex].get<double>();
    if (!std::isfinite(aValue) || aValue < 0.0 || aValue > 1.0)
      throw KernelFailure(ErrorCode::InvalidArgs, theError);
    aValues[anIndex] = static_cast<float>(aValue);
  }
  return aValues;
}

namespace {
struct ExportDocumentData
{
  occ::handle<TDocStd_Document> document;
  occ::handle<XCAFDoc_ColorTool> colorTool;
  occ::handle<XCAFDoc_LayerTool> layerTool;
  occ::handle<XCAFDoc_DimTolTool> dimTolTool;
  std::vector<TDF_Label> labels;
};

ExportDocumentData buildExportDocument(KernelOperationContext& theContext,
                                       const json& theArgs,
                                       const char* theDocumentFormat)
{
      const json& aNodes = theArgs.at("nodes");
      const std::size_t aNodeCount = aNodes.size();
      validateDocumentTree(theContext, aNodes, theArgs.at("roots"));
      std::vector<bool> isRoot(aNodeCount, false);
      for (const json& aRoot : theArgs.at("roots"))
        isRoot[aRoot.get<std::size_t>()] = true;
      auto nodeTransform = [](const json& theNode) {
        gp_Trsf aTransform;
        if (!theNode.contains("transform")) return aTransform;
        const json& aMatrix = theNode.at("transform");
        if (!aMatrix.is_array() || aMatrix.size() != 12)
          throw KernelFailure(ErrorCode::InvalidArgs, "STEP document transform must contain 12 numbers");
        for (const json& aValue : aMatrix)
          if (!aValue.is_number() || !std::isfinite(aValue.get<double>()))
            throw KernelFailure(ErrorCode::InvalidArgs, "STEP document transform must be finite");
        aTransform.SetValues(
          aMatrix[0].get<double>(), aMatrix[1].get<double>(), aMatrix[2].get<double>(), aMatrix[3].get<double>(),
          aMatrix[4].get<double>(), aMatrix[5].get<double>(), aMatrix[6].get<double>(), aMatrix[7].get<double>(),
          aMatrix[8].get<double>(), aMatrix[9].get<double>(), aMatrix[10].get<double>(), aMatrix[11].get<double>());
        return aTransform;
      };

      const occ::handle<TDocStd_Document> aDocument =
        new TDocStd_Document(theDocumentFormat);
      XCAFDoc_DocumentTool::Set(aDocument->Main());
      const occ::handle<XCAFDoc_ShapeTool> aShapeTool =
        XCAFDoc_DocumentTool::ShapeTool(aDocument->Main());
      const occ::handle<XCAFDoc_ColorTool> aColorTool =
        XCAFDoc_DocumentTool::ColorTool(aDocument->Main());
      const occ::handle<XCAFDoc_LayerTool> aLayerTool =
        XCAFDoc_DocumentTool::LayerTool(aDocument->Main());
      const occ::handle<XCAFDoc_MaterialTool> aMaterialTool =
        XCAFDoc_DocumentTool::MaterialTool(aDocument->Main());
      const occ::handle<XCAFDoc_VisMaterialTool> aVisMaterialTool =
        XCAFDoc_DocumentTool::VisMaterialTool(aDocument->Main());
      const occ::handle<XCAFDoc_ClippingPlaneTool> aClippingPlaneTool =
        XCAFDoc_DocumentTool::ClippingPlaneTool(aDocument->Main());
      occ::handle<XCAFDoc_ViewTool> aViewTool;
      if (theArgs.contains("views"))
        aViewTool = XCAFDoc_DocumentTool::ViewTool(aDocument->Main());
      const occ::handle<XCAFDoc_DimTolTool> aDimTolTool =
        XCAFDoc_DocumentTool::DimTolTool(aDocument->Main());
      std::vector<TDF_Label> aLabels(aNodeCount);
      std::vector<TDF_Label> aComponentLabels(aNodeCount);

      auto applyMetadata = [&](const TDF_Label& theLabel, const json& theNode) {
        if (theNode.contains("visible") && !theNode.at("visible").is_boolean())
          throw KernelFailure(ErrorCode::InvalidArgs, "STEP document visibility must be boolean");
        if (theNode.contains("name"))
        {
          if (!theNode.at("name").is_string())
            throw KernelFailure(ErrorCode::InvalidArgs, "STEP document node name must be a string");
          const std::string aName = theNode.at("name").get<std::string>();
          TDataStd_Name::Set(theLabel, TCollection_ExtendedString(aName.c_str(), true));
        }
        if (theNode.contains("color"))
        {
          const std::array<float, 4> aValues = parseNormalizedColor(
            theNode.at("color"), "STEP document color must contain four finite values between zero and one");
          aColorTool->SetColor(theLabel,
            Quantity_ColorRGBA(aValues[0], aValues[1], aValues[2], aValues[3]), XCAFDoc_ColorGen);
        }
        if (theNode.contains("layers"))
        {
          if (!theNode.at("layers").is_array())
            throw KernelFailure(ErrorCode::InvalidArgs, "STEP document layers must be an array");
          for (const json& aLayer : theNode.at("layers"))
          {
            if (!aLayer.is_string())
              throw KernelFailure(ErrorCode::InvalidArgs, "STEP document layer names must be strings");
            const std::string aName = aLayer.get<std::string>();
            aLayerTool->SetLayer(theLabel, TCollection_ExtendedString(aName.c_str(), true));
          }
        }
        if (theNode.contains("material"))
        {
          const json& aMaterial = theNode.at("material");
          if (!aMaterial.is_object() || !aMaterial.contains("name")
              || !aMaterial.at("name").is_string() || !aMaterial.contains("density")
              || !aMaterial.at("density").is_number())
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "STEP document material requires name and density");
          const double aDensity = aMaterial.at("density").get<double>();
          if (!std::isfinite(aDensity) || aDensity < 0.0)
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "STEP document material density must be finite and non-negative");
          auto materialString = [&](const char* theKey, const char* theDefault) {
            if (!aMaterial.contains(theKey))
              return occ::handle<TCollection_HAsciiString>(new TCollection_HAsciiString(theDefault));
            if (!aMaterial.at(theKey).is_string())
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  std::string("STEP document material ") + theKey + " must be a string");
            return occ::handle<TCollection_HAsciiString>(
              new TCollection_HAsciiString(aMaterial.at(theKey).get<std::string>().c_str()));
          };
          const auto aName = materialString("name", "");
          const auto aDescription = materialString("description", "");
          const auto aDensityName = materialString("densityName", "kg/m^3");
          const auto aDensityValueType = materialString("densityValueType", "mass density");
          aMaterialTool->SetMaterial(theLabel, aName, aDescription, aDensity,
                                     aDensityName, aDensityValueType);
        }
        if (theNode.contains("visualMaterial"))
        {
          const json& aMaterial = theNode.at("visualMaterial");
          if (!aMaterial.is_object() || !aMaterial.contains("name")
              || !aMaterial.at("name").is_string())
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "STEP document visual material requires a name");

          XCAFDoc_VisMaterialPBR aPbr;
          if (aMaterial.contains("baseColor"))
          {
            const std::array<float, 4> aValues = parseNormalizedColor(
              aMaterial.at("baseColor"),
              "STEP document visual material baseColor must contain four finite values between zero and one");
            aPbr.BaseColor = Quantity_ColorRGBA(aValues[0], aValues[1], aValues[2], aValues[3]);
          }
          auto unitValue = [&](const char* theKey, const float theDefault) {
            if (!aMaterial.contains(theKey)) return theDefault;
            if (!aMaterial.at(theKey).is_number())
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  std::string("STEP document visual material ") + theKey
                                  + " must be a number");
            const double aValue = aMaterial.at(theKey).get<double>();
            if (!std::isfinite(aValue) || aValue < 0.0 || aValue > 1.0)
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  std::string("STEP document visual material ") + theKey
                                  + " must be between zero and one");
            return static_cast<float>(aValue);
          };
          aPbr.Metallic = unitValue("metallic", aPbr.Metallic);
          aPbr.Roughness = unitValue("roughness", aPbr.Roughness);
          if (aMaterial.contains("emissive"))
          {
            const json& anEmissive = aMaterial.at("emissive");
            if (!anEmissive.is_array() || anEmissive.size() != 3)
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "STEP document visual material emissive must contain RGB values");
            std::array<float, 3> aValues;
            for (std::size_t anIndex = 0; anIndex < aValues.size(); ++anIndex)
            {
              if (!anEmissive[anIndex].is_number())
                throw KernelFailure(ErrorCode::InvalidArgs,
                                    "STEP document visual material emissive values must be numbers");
              const double aValue = anEmissive[anIndex].get<double>();
              if (!std::isfinite(aValue) || aValue < 0.0 || aValue > 1.0)
                throw KernelFailure(ErrorCode::InvalidArgs,
                                    "STEP document visual material emissive values must be between zero and one");
              aValues[anIndex] = static_cast<float>(aValue);
            }
            aPbr.EmissiveFactor = NCollection_Vec3<float>(aValues[0], aValues[1], aValues[2]);
          }
          if (aMaterial.contains("refractionIndex"))
          {
            if (!aMaterial.at("refractionIndex").is_number())
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "STEP document visual material refractionIndex must be a number");
            const double aValue = aMaterial.at("refractionIndex").get<double>();
            if (!std::isfinite(aValue) || aValue < 1.0 || aValue > 3.0)
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "STEP document visual material refractionIndex must be between one and three");
            aPbr.RefractionIndex = static_cast<float>(aValue);
          }
          const occ::handle<XCAFDoc_VisMaterial> aVisMaterial = new XCAFDoc_VisMaterial();
          aVisMaterial->SetPbrMaterial(aPbr);
          const TDF_Label aMaterialLabel = aVisMaterialTool->AddMaterial(
            aVisMaterial, TCollection_AsciiString(aMaterial.at("name").get<std::string>().c_str()));
          aVisMaterialTool->SetShapeMaterial(theLabel, aMaterialLabel);
        }
        if (theNode.contains("validationProperties"))
        {
          const json& aProperties = theNode.at("validationProperties");
          if (!aProperties.is_object())
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "STEP document validationProperties must be an object");
          auto nonNegativeProperty = [&](const char* theKey) {
            if (!aProperties.at(theKey).is_number())
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  std::string("STEP document ") + theKey + " must be a number");
            const double aValue = aProperties.at(theKey).get<double>();
            if (!std::isfinite(aValue) || aValue < 0.0)
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  std::string("STEP document ") + theKey
                                  + " must be finite and non-negative");
            return aValue;
          };
          if (aProperties.contains("area"))
            XCAFDoc_Area::Set(theLabel, nonNegativeProperty("area"));
          if (aProperties.contains("volume"))
            XCAFDoc_Volume::Set(theLabel, nonNegativeProperty("volume"));
          if (aProperties.contains("centroid"))
          {
            const json& aCentroid = aProperties.at("centroid");
            if (!aCentroid.is_array() || aCentroid.size() != 3)
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "STEP document centroid must contain three numbers");
            std::array<double, 3> aValues;
            for (std::size_t anIndex = 0; anIndex < aValues.size(); ++anIndex)
            {
              if (!aCentroid[anIndex].is_number())
                throw KernelFailure(ErrorCode::InvalidArgs,
                                    "STEP document centroid values must be numbers");
              aValues[anIndex] = aCentroid[anIndex].get<double>();
              if (!std::isfinite(aValues[anIndex]))
                throw KernelFailure(ErrorCode::InvalidArgs,
                                    "STEP document centroid values must be finite");
            }
            XCAFDoc_Centroid::Set(theLabel, gp_Pnt(aValues[0], aValues[1], aValues[2]));
          }
        }
        aColorTool->SetVisibility(theLabel, theNode.value("visible", true));
      };

      auto applySubshapeStyles = [&](const TDF_Label& theLabel, const json& theNode) {
        if (!theNode.contains("subshapeStyles")) return;
        const json& aStyles = theNode.at("subshapeStyles");
        if (!aStyles.is_array())
          throw KernelFailure(ErrorCode::InvalidArgs,
                              "STEP document subshapeStyles must be an array");
        if (theNode.at("kind") != "part" && !aStyles.empty())
          throw KernelFailure(ErrorCode::InvalidArgs,
                              "STEP document subshapeStyles are only valid on part nodes");

        const TopoDS_Shape aShape = XCAFDoc_ShapeTool::GetShape(theLabel);
        std::unordered_set<std::string> aStyledSubshapes;
        for (const json& aStyle : aStyles)
        {
          if (!aStyle.is_object() || !aStyle.contains("topology")
              || !aStyle.at("topology").is_string() || !aStyle.contains("index")
              || (!aStyle.at("index").is_number_unsigned()
                  && !aStyle.at("index").is_number_integer())
              || !aStyle.contains("color"))
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "STEP document subshape style requires topology, index, and color");
          const std::string aTopology = aStyle.at("topology").get<std::string>();
          if (aTopology != "face" && aTopology != "edge")
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "STEP document subshape style topology must be face or edge");
          const std::int64_t anIndex = aStyle.at("index").get<std::int64_t>();
          if (anIndex < 0)
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "STEP document subshape style index must be non-negative");
          const std::string aKey = aTopology + ":" + std::to_string(anIndex);
          if (!aStyledSubshapes.insert(aKey).second)
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "STEP document subshape styles must not contain duplicates");

          const std::array<float, 4> aValues = parseNormalizedColor(
            aStyle.at("color"),
            "STEP document subshape style color must contain four finite values between zero and one");

          TopTools_IndexedMapOfShape aSubshapes;
          const TopAbs_ShapeEnum aShapeType = aTopology == "face" ? TopAbs_FACE : TopAbs_EDGE;
          TopExp::MapShapes(aShape, aShapeType, aSubshapes);
          if (anIndex >= aSubshapes.Extent())
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "STEP document subshape style index is out of range");
          const TDF_Label aSubshapeLabel =
            aShapeTool->AddSubShape(theLabel, aSubshapes(static_cast<int>(anIndex + 1)));
          if (aSubshapeLabel.IsNull())
            throw KernelFailure(ErrorCode::ImportExportFailed,
                                "OCCT could not create an XCAF subshape label");
          aColorTool->SetColor(
            aSubshapeLabel,
            Quantity_ColorRGBA(aValues[0], aValues[1], aValues[2], aValues[3]),
            aTopology == "face" ? XCAFDoc_ColorSurf : XCAFDoc_ColorCurv);
        }
      };

      for (std::size_t aNodeIndex = 0; aNodeIndex < aNodeCount; ++aNodeIndex)
      {
        const json& aNode = aNodes[aNodeIndex];
        aLabels[aNodeIndex] = aShapeTool->NewShape();
        if (aNode.at("kind") == "part")
        {
          aShapeTool->SetShape(aLabels[aNodeIndex], theContext.arena().get(requiredU32(aNode, "shape")));
        }
        else
        {
          BRep_Builder aBuilder;
          TopoDS_Compound anAssembly;
          aBuilder.MakeCompound(anAssembly);
          aShapeTool->SetShape(aLabels[aNodeIndex], anAssembly);
        }
        applyMetadata(aLabels[aNodeIndex], aNode);
        applySubshapeStyles(aLabels[aNodeIndex], aNode);
      }
      for (std::size_t aNodeIndex = 0; aNodeIndex < aNodeCount; ++aNodeIndex)
      {
        if (aNodes[aNodeIndex].at("kind") != "assembly") continue;
        for (const json& aChildValue : aNodes[aNodeIndex].value("children", json::array()))
        {
          const std::size_t aChild = aChildValue.get<std::size_t>();
          const TDF_Label aComponent = aShapeTool->AddComponent(
            aLabels[aNodeIndex], aLabels[aChild], TopLoc_Location(nodeTransform(aNodes[aChild])));
          if (aComponent.IsNull())
            throw KernelFailure(ErrorCode::ImportExportFailed, "OCCT could not create STEP assembly component");
          applyMetadata(aComponent, aNodes[aChild]);
          aComponentLabels[aChild] = aComponent;
        }
      }

      auto finiteXCAFVec3 = [&](const json& theValue, const char* theName) {
        if (!theValue.is_array() || theValue.size() != 3)
          throw KernelFailure(ErrorCode::InvalidArgs,
                              std::string("XCAF ") + theName + " must contain three numbers");
        std::array<double, 3> aResult;
        for (std::size_t anIndex = 0; anIndex < 3; ++anIndex)
        {
          if (!theValue[anIndex].is_number()
              || !std::isfinite(theValue[anIndex].get<double>()))
            throw KernelFailure(ErrorCode::InvalidArgs,
                                std::string("XCAF ") + theName + " must be finite");
          aResult[anIndex] = theValue[anIndex].get<double>();
        }
        return aResult;
      };
      if (theArgs.contains("views"))
      {
        const json& aViews = theArgs.at("views");
        if (!aViews.is_array())
          throw KernelFailure(ErrorCode::InvalidArgs, "XCAF views must be an array");
        for (const json& aView : aViews)
        {
          if (!aView.is_object() || !aView.contains("name") || !aView.at("name").is_string()
              || !aView.contains("viewDirection") || !aView.contains("upDirection")
              || !aView.contains("nodeIndices") || !aView.at("nodeIndices").is_array()
              || aView.at("nodeIndices").empty())
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "XCAF view requires name, directions, and nodeIndices");
          const auto aDirection = finiteXCAFVec3(aView.at("viewDirection"), "viewDirection");
          const auto anUp = finiteXCAFVec3(aView.at("upDirection"), "upDirection");
          if (aDirection[0] * aDirection[0] + aDirection[1] * aDirection[1]
                + aDirection[2] * aDirection[2] < 1e-24
              || anUp[0] * anUp[0] + anUp[1] * anUp[1] + anUp[2] * anUp[2] < 1e-24)
            throw KernelFailure(ErrorCode::InvalidArgs, "XCAF view directions must be non-zero");
          const std::string aProjection = aView.value("projection", "none");
          XCAFView_ProjectionType aProjectionType = XCAFView_ProjectionType_NoCamera;
          if (aProjection == "parallel") aProjectionType = XCAFView_ProjectionType_Parallel;
          else if (aProjection == "central") aProjectionType = XCAFView_ProjectionType_Central;
          else if (aProjection != "none")
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "XCAF view projection must be none, parallel, or central");
          auto positiveViewValue = [&](const char* theKey, double theDefault) {
            if (!aView.contains(theKey)) return theDefault;
            if (!aView.at(theKey).is_number())
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  std::string("XCAF view ") + theKey + " must be a number");
            const double aValue = aView.at(theKey).get<double>();
            if (!std::isfinite(aValue) || aValue <= 0.0)
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  std::string("XCAF view ") + theKey + " must be positive");
            return aValue;
          };
          const occ::handle<XCAFView_Object> aViewObject = new XCAFView_Object();
          aViewObject->SetName(new TCollection_HAsciiString(aView.at("name").get<std::string>().c_str()));
          aViewObject->SetType(aProjectionType);
          if (aView.contains("projectionPoint"))
          {
            const auto aPoint = finiteXCAFVec3(aView.at("projectionPoint"), "projectionPoint");
            aViewObject->SetProjectionPoint(gp_Pnt(aPoint[0], aPoint[1], aPoint[2]));
          }
          aViewObject->SetViewDirection(gp_Dir(aDirection[0], aDirection[1], aDirection[2]));
          aViewObject->SetUpDirection(gp_Dir(anUp[0], anUp[1], anUp[2]));
          aViewObject->SetZoomFactor(positiveViewValue("zoomFactor", 1.0));
          aViewObject->SetWindowHorizontalSize(positiveViewValue("windowHorizontalSize", 1.0));
          aViewObject->SetWindowVerticalSize(positiveViewValue("windowVerticalSize", 1.0));
          if (aView.contains("frontPlaneDistance"))
            aViewObject->SetFrontPlaneDistance(positiveViewValue("frontPlaneDistance", 1.0));
          if (aView.contains("backPlaneDistance"))
            aViewObject->SetBackPlaneDistance(positiveViewValue("backPlaneDistance", 1.0));
          if (aView.contains("viewVolumeSidesClipping")
              && !aView.at("viewVolumeSidesClipping").is_boolean())
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "XCAF view viewVolumeSidesClipping must be boolean");
          aViewObject->SetViewVolumeSidesClipping(aView.value("viewVolumeSidesClipping", false));
          NCollection_Sequence<TDF_Label> aViewShapes;
          for (const json& aNodeValue : aView.at("nodeIndices"))
          {
            if ((!aNodeValue.is_number_unsigned() && !aNodeValue.is_number_integer())
                || aNodeValue.get<std::int64_t>() < 0
                || static_cast<std::size_t>(aNodeValue.get<std::uint64_t>()) >= aNodeCount)
              throw KernelFailure(ErrorCode::InvalidArgs, "XCAF view node index is invalid");
            aViewShapes.Append(aLabels[aNodeValue.get<std::size_t>()]);
          }
          NCollection_Sequence<TDF_Label> aViewClippingPlanes;
          if (aView.contains("clippingPlanes"))
          {
            const json& aClippingPlanes = aView.at("clippingPlanes");
            if (!aClippingPlanes.is_array())
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "XCAF view clippingPlanes must be an array");
            for (const json& aClippingPlane : aClippingPlanes)
            {
              if (!aClippingPlane.is_object() || !aClippingPlane.contains("name")
                  || !aClippingPlane.at("name").is_string()
                  || !aClippingPlane.contains("origin")
                  || !aClippingPlane.contains("normal"))
                throw KernelFailure(ErrorCode::InvalidArgs,
                                    "XCAF clipping plane requires name, origin, and normal");
              const auto anOrigin = finiteXCAFVec3(aClippingPlane.at("origin"), "clippingPlane origin");
              const auto aNormal = finiteXCAFVec3(aClippingPlane.at("normal"), "clippingPlane normal");
              if (aNormal[0] * aNormal[0] + aNormal[1] * aNormal[1]
                    + aNormal[2] * aNormal[2] < 1e-24)
                throw KernelFailure(ErrorCode::InvalidArgs,
                                    "XCAF clipping plane normal must be non-zero");
              if (aClippingPlane.contains("capping")
                  && !aClippingPlane.at("capping").is_boolean())
                throw KernelFailure(ErrorCode::InvalidArgs,
                                    "XCAF clipping plane capping must be boolean");
              const bool aCapping = aClippingPlane.value("capping", false);
              const TCollection_ExtendedString aName(
                aClippingPlane.at("name").get<std::string>().c_str(), true);
              aViewClippingPlanes.Append(aClippingPlaneTool->AddClippingPlane(
                gp_Pln(gp_Pnt(anOrigin[0], anOrigin[1], anOrigin[2]),
                       gp_Dir(aNormal[0], aNormal[1], aNormal[2])),
                aName, aCapping));
            }
          }
          const TDF_Label aViewLabel = aViewTool->AddView();
          XCAFDoc_View::Set(aViewLabel)->SetObject(aViewObject);
          aViewTool->SetView(aViewShapes, NCollection_Sequence<TDF_Label>(),
                             aViewClippingPlanes, aViewLabel);
        }
      }
      if (theArgs.contains("shuo"))
      {
        const json& aShuo = theArgs.at("shuo");
        if (!aShuo.is_array())
          throw KernelFailure(ErrorCode::InvalidArgs, "XCAF shuo must be an array");
        std::unordered_set<std::string> aPaths;
        for (const json& aDefinition : aShuo)
        {
          if (!aDefinition.is_object() || !aDefinition.contains("nodeIndices")
              || !aDefinition.at("nodeIndices").is_array()
              || aDefinition.at("nodeIndices").size() < 2)
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "XCAF shuo requires at least two nodeIndices");
          NCollection_Sequence<TDF_Label> aPath;
          std::string aPathKey;
          std::size_t aPrevious = 0;
          bool isFirst = true;
          for (const json& aNodeValue : aDefinition.at("nodeIndices"))
          {
            if ((!aNodeValue.is_number_unsigned() && !aNodeValue.is_number_integer())
                || aNodeValue.get<std::int64_t>() < 0
                || static_cast<std::size_t>(aNodeValue.get<std::uint64_t>()) >= aNodeCount)
              throw KernelFailure(ErrorCode::InvalidArgs, "XCAF shuo node index is invalid");
            const std::size_t aNode = aNodeValue.get<std::size_t>();
            if (!isFirst)
            {
              bool isChild = false;
              for (const json& aChild : aNodes[aPrevious].value("children", json::array()))
                if (aChild.get<std::size_t>() == aNode) isChild = true;
              if (!isChild)
                throw KernelFailure(ErrorCode::InvalidArgs,
                                    "XCAF shuo nodeIndices must follow assembly components");
            }
            isFirst = false;
            if (aComponentLabels[aNode].IsNull())
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "XCAF shuo nodeIndices must identify component occurrences");
            if (!aPathKey.empty()) aPathKey += "/";
            aPathKey += std::to_string(aNode);
            aPath.Append(aComponentLabels[aNode]);
            aPrevious = aNode;
          }
          if (!aPaths.insert(aPathKey).second)
            throw KernelFailure(ErrorCode::InvalidArgs, "XCAF shuo paths must be unique");
          occ::handle<XCAFDoc_GraphNode> aMainShuo;
          if (!aShapeTool->SetSHUO(aPath, aMainShuo) || aMainShuo.IsNull())
            throw KernelFailure(ErrorCode::ImportExportFailed, "OCCT could not create XCAF SHUO");
          if (aDefinition.contains("color"))
          {
            const std::array<float, 4> aValues = parseNormalizedColor(
              aDefinition.at("color"),
              "XCAF SHUO color must contain four finite values between zero and one");
            aColorTool->SetColor(aMainShuo->Label(),
                                 Quantity_ColorRGBA(aValues[0], aValues[1], aValues[2], aValues[3]),
                                 XCAFDoc_ColorGen);
          }
        }
      }
      aShapeTool->UpdateAssemblies();
      for (std::size_t aNodeIndex = 0; aNodeIndex < aNodeCount; ++aNodeIndex)
      {
        if (!isRoot[aNodeIndex] || !aNodes[aNodeIndex].contains("transform")) continue;
        const gp_Trsf aTransform = nodeTransform(aNodes[aNodeIndex]);
        if (aTransform.Form() == gp_Identity) continue;
        TDF_Label aLocatedRoot;
        if (!aShapeTool->SetLocation(aLabels[aNodeIndex], TopLoc_Location(aTransform), aLocatedRoot))
          throw KernelFailure(ErrorCode::ImportExportFailed, "OCCT could not locate STEP document root");
        aLabels[aNodeIndex] = aLocatedRoot;
        applyMetadata(aLocatedRoot, aNodes[aNodeIndex]);
      }

      return {aDocument, aColorTool, aLayerTool, aDimTolTool, std::move(aLabels)};
    }

} // namespace

json h_exportSTEPDocument(KernelOperationContext& theContext, const json& theArgs)
{
  if (theArgs.contains("gdt") && !theArgs.at("gdt").empty()
      && theArgs.at("roots").size() != 1)
    throw KernelFailure(ErrorCode::InvalidArgs,
                        "STEP GDT export requires exactly one document root");
  ExportDocumentData aData = buildExportDocument(theContext, theArgs, "BinXCAF");
  const json& aNodes = theArgs.at("nodes");
  applyDocumentAnnotations(theContext, 
    true, theArgs, aNodes, aData.labels, aNodes.size(), aData.dimTolTool);
  return writeSTEPDocument(theContext, theArgs, aData.document);
}

json h_exportXCAF(KernelOperationContext& theContext, const json& theArgs)
{
  const std::string aFormat = theArgs.value("format", "bin");
  if (aFormat != "bin" && aFormat != "xml")
    throw KernelFailure(ErrorCode::InvalidArgs, "XCAF format must be bin or xml");
  if (aFormat == "xml")
  {
    for (const json& aNode : theArgs.at("nodes"))
    {
      if (aNode.contains("color"))
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "XCAF XML cannot preserve node RGBA color; use format bin");
      if (aNode.contains("subshapeStyles") && !aNode.at("subshapeStyles").empty())
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "XCAF XML cannot preserve subshape RGBA styles; use format bin");
    }
    if (theArgs.contains("shuo"))
    {
      for (const json& aShuo : theArgs.at("shuo"))
        if (aShuo.contains("color"))
          throw KernelFailure(ErrorCode::InvalidArgs,
                              "XCAF XML cannot preserve SHUO RGBA color; use format bin");
    }
  }
  ExportDocumentData aData = buildExportDocument(
    theContext, theArgs, aFormat == "xml" ? "XmlXCAF" : "BinXCAF");
  const json& aNodes = theArgs.at("nodes");
  applyDocumentAnnotations(theContext, 
    false, theArgs, aNodes, aData.labels, aNodes.size(), aData.dimTolTool);
  return writeXCAFDocument(theContext, theArgs, aData.document, aFormat);
}

json h_exportIGESDocument(KernelOperationContext& theContext, const json& theArgs)
{
  auto hasEntries = [&](const char* theKey) {
    return theArgs.contains(theKey) && theArgs.at(theKey).is_array()
           && !theArgs.at(theKey).empty();
  };
  if (hasEntries("gdt") || hasEntries("datums") || hasEntries("geometricTolerances")
      || hasEntries("views") || hasEntries("shuo"))
    throw KernelFailure(ErrorCode::ImportExportFailed,
                        "IGES transfer cannot preserve XCAF semantic annotations, views, or SHUO");

  ExportDocumentData aData = buildExportDocument(theContext, theArgs, "BinXCAF");
  const json& aNodes = theArgs.at("nodes");
  for (std::size_t aNodeIndex = 0; aNodeIndex < aNodes.size(); ++aNodeIndex)
  {
    const json& aNode = aNodes[aNodeIndex];
    const TDF_Label& aLabel = aData.labels[aNodeIndex];
    if (aNode.contains("color"))
    {
      const std::array<float, 4> aValues = parseNormalizedColor(
        aNode.at("color"),
        "STEP document color must contain four finite values between zero and one");
      const Quantity_ColorRGBA aColor(aValues[0], aValues[1], aValues[2], aValues[3]);
      aData.colorTool->SetColor(aLabel, aColor, XCAFDoc_ColorSurf);
      aData.colorTool->SetColor(aLabel, aColor, XCAFDoc_ColorCurv);
      std::function<void(const TDF_Label&)> applySubshapeColor =
        [&](const TDF_Label& theCurrent) {
          NCollection_Sequence<TDF_Label> aSubshapeLabels;
          if (!XCAFDoc_ShapeTool::GetSubShapes(theCurrent, aSubshapeLabels)) return;
          for (int anIndex = 1; anIndex <= aSubshapeLabels.Size(); ++anIndex)
          {
            const TDF_Label& aSubshapeLabel = aSubshapeLabels.Value(anIndex);
            aData.colorTool->SetColor(aSubshapeLabel, aColor, XCAFDoc_ColorSurf);
            aData.colorTool->SetColor(aSubshapeLabel, aColor, XCAFDoc_ColorCurv);
            applySubshapeColor(aSubshapeLabel);
          }
        };
      applySubshapeColor(aLabel);
    }
    for (const json& aLayer : aNode.value("layers", json::array()))
    {
      const TopoDS_Shape aNodeShape = XCAFDoc_ShapeTool::GetShape(aLabel);
      if (!aNodeShape.IsNull())
        aData.layerTool->SetLayer(
          aNodeShape, TCollection_ExtendedString(aLayer.get<std::string>().c_str(), true));
    }
  }
  applyDocumentAnnotations(theContext, 
    false, theArgs, aNodes, aData.labels, aNodes.size(), aData.dimTolTool);
  return writeIGESDocument(theContext, theArgs, aData.document);
}

} // namespace occt_worker
