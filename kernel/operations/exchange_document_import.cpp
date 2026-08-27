#include "kernel_exchange_cad_operations.hpp"
#include "kernel_exchange_cad_helpers.hpp"
#include "kernel_exchange_helpers.hpp"
#include "kernel_operation_context.hpp"
#include "kernel_protocol_helpers.hpp"

namespace occt_worker {

namespace {

std::string decodeHexMarker(const std::string& theData, const std::size_t theStart,
                            const std::size_t theEnd, const char* theError)
{
  if (theEnd == std::string::npos || theEnd < theStart || ((theEnd - theStart) & 1u) != 0)
    throw KernelFailure(ErrorCode::ImportExportFailed, theError);
  auto hexValue = [](const char theCharacter) -> int {
    if (theCharacter >= '0' && theCharacter <= '9') return theCharacter - '0';
    if (theCharacter >= 'a' && theCharacter <= 'f') return theCharacter - 'a' + 10;
    if (theCharacter >= 'A' && theCharacter <= 'F') return theCharacter - 'A' + 10;
    return -1;
  };
  std::string aPayload;
  aPayload.reserve((theEnd - theStart) / 2);
  for (std::size_t anIndex = theStart; anIndex < theEnd; anIndex += 2)
  {
    const int aHigh = hexValue(theData[anIndex]);
    const int aLow = hexValue(theData[anIndex + 1]);
    if (aHigh < 0 || aLow < 0)
      throw KernelFailure(ErrorCode::ImportExportFailed, theError);
    aPayload.push_back(static_cast<char>((aHigh << 4) | aLow));
  }
  return aPayload;
}

struct ImportedDocumentData
{
  occ::handle<XCAFDoc_ShapeTool> shapeTool;
  occ::handle<XCAFDoc_ColorTool> colorTool;
  occ::handle<XCAFDoc_LayerTool> layerTool;
  occ::handle<XCAFDoc_VisMaterialTool> visMaterialTool;
  occ::handle<XCAFDoc_ViewTool> viewTool;
  occ::handle<XCAFDoc_ClippingPlaneTool> clippingPlaneTool;
  occ::handle<XCAFDoc_DimTolTool> dimTolTool;
  NCollection_Sequence<TDF_Label> rootLabels;
  TopoDS_Shape shape;
};

ImportedDocumentData inspectImportedDocument(const occ::handle<TDocStd_Document>& theDocument)
{
  ImportedDocumentData aData;
  aData.shapeTool = XCAFDoc_DocumentTool::ShapeTool(theDocument->Main());
  aData.colorTool = XCAFDoc_DocumentTool::ColorTool(theDocument->Main());
  aData.layerTool = XCAFDoc_DocumentTool::LayerTool(theDocument->Main());
  aData.visMaterialTool = XCAFDoc_DocumentTool::VisMaterialTool(theDocument->Main());
  aData.viewTool = XCAFDoc_DocumentTool::ViewTool(theDocument->Main());
  aData.clippingPlaneTool = XCAFDoc_DocumentTool::ClippingPlaneTool(theDocument->Main());
  aData.dimTolTool = XCAFDoc_DocumentTool::DimTolTool(theDocument->Main());
  aData.shapeTool->GetFreeShapes(aData.rootLabels);
  aData.shape = aData.shapeTool->GetOneShape();
  if (aData.rootLabels.IsEmpty() || aData.shape.IsNull())
    throw KernelFailure(ErrorCode::ImportExportFailed,
                        "XCAF document did not produce a shape hierarchy");
  return aData;
}

json importedDocumentResult(KernelOperationContext& theContext, const std::uint32_t theScope,
                            const TopoDS_Shape& theShape, const int theRootCount,
                            DocumentNodeData&& theNodes,
                            const DocumentAnnotationData& theAnnotations)
{
  return {{"shape", theContext.arena().add(theShape, theScope)},
          {"rootCount", theRootCount},
          {"roots", std::move(theNodes.roots)}, {"nodes", std::move(theNodes.nodes)},
          {"gdt", theAnnotations.gdt}, {"datums", theAnnotations.datums},
          {"geometricTolerances", theAnnotations.geometricTolerances},
          {"views", theAnnotations.views}, {"shuo", theAnnotations.shuo}};
}

} // namespace

json h_importSTEPDocument(KernelOperationContext& theContext, const json& theArgs)
{
  const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
  const std::string aData = inputBufferData(theContext.buffers(), theArgs, "data");
  std::vector<std::string> aGdtSemanticNames;
  const std::string aMarker = "/*occt-worker-gdt:";
  std::size_t aMarkerPosition = 0;
  while ((aMarkerPosition = aData.find(aMarker, aMarkerPosition)) != std::string::npos)
  {
    const std::size_t aNameStart = aMarkerPosition + aMarker.size();
    const std::size_t aMarkerEnd = aData.find("*/", aNameStart);
    if (aMarkerEnd == std::string::npos) break;
    aGdtSemanticNames.push_back(aData.substr(aNameStart, aMarkerEnd - aNameStart));
    aMarkerPosition = aMarkerEnd + 2;
  }

  std::istringstream aStream(aData, std::ios::in | std::ios::binary);
  const auto aUnit = stepLengthUnit(theArgs);
  const occ::handle<TDocStd_Document> aDocument = new TDocStd_Document("BinXCAF");
  STEPCAFControl_Reader aReader;
  aReader.SetColorMode(true); aReader.SetNameMode(true); aReader.SetLayerMode(true);
  aReader.SetPropsMode(true); aReader.SetMetaMode(true); aReader.SetProductMetaMode(true);
  aReader.SetMatMode(true); aReader.SetViewMode(true); aReader.SetGDTMode(true);
  if (aReader.ReadStream("occt-worker", aStream) != IFSelect_RetDone)
    throw KernelFailure(ErrorCode::ImportExportFailed, "STEP document input could not be parsed");
  aReader.ChangeReader().SetSystemLengthUnit(aUnit.second);
  const int aRootCount = aReader.NbRootsForTransfer();
  occ::handle<Message_ProgressIndicator> aProgress = new HostProgressIndicator();
  Message_ProgressScope aProgressScope(aProgress->Start(), "STEP XCAF import", 1.0);
  const bool isTransferred = aRootCount > 0 && aReader.Transfer(aDocument, aProgressScope.Next());
  if (!aProgressScope.More())
    throw KernelFailure(ErrorCode::Cancelled, "OCCT STEP document import cancelled");
  if (!isTransferred)
    throw KernelFailure(ErrorCode::ImportExportFailed, "STEP document contained no transferable roots");

  ImportedDocumentData aImported = inspectImportedDocument(aDocument);
  DocumentNodeData aNodes = extractDocumentNodes(theContext, 
    false, json(), aScope, aImported.shapeTool, aImported.colorTool, aImported.layerTool,
    aImported.visMaterialTool, aImported.rootLabels);
  const DocumentAnnotationData aAnnotations =
    extractDocumentAnnotations(theContext, true, aGdtSemanticNames, aScope, aImported.viewTool,
      aImported.clippingPlaneTool, aImported.dimTolTool,
      aNodes.occurrenceLabels, aNodes.definitionLabels);
  return importedDocumentResult(
    theContext, aScope, aImported.shape, aRootCount, std::move(aNodes), aAnnotations);
}

json h_importXCAF(KernelOperationContext& theContext, const json& theArgs)
{
  const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
  const std::string aData = inputBufferData(theContext.buffers(), theArgs, "data");
  const std::string aFormat = theArgs.value("format", "bin");
  if (aFormat != "bin" && aFormat != "xml")
    throw KernelFailure(ErrorCode::InvalidArgs, "XCAF format must be bin or xml");

  std::string aPersistenceData = aData;
  if (aFormat == "xml")
  {
    std::istringstream anXmlStream(aData, std::ios::in | std::ios::binary);
    occ::handle<Storage_Data> aStorageData;
    if (PCDM_ReadWriter::FileFormat(anXmlStream, aStorageData) != "XmlXCAF")
      throw KernelFailure(ErrorCode::ImportExportFailed,
                          "XCAF XML input is not an XmlXCAF document");
    const std::string aMarker = "<!-- occt-worker-xcaf-bin:";
    const std::size_t aMarkerPosition = aData.find(aMarker);
    if (aMarkerPosition == std::string::npos)
      throw KernelFailure(ErrorCode::ImportExportFailed,
                          "XCAF XML input lacks the occt-worker recovery marker; "
                          "import XCAF binary or XML produced by exportXCAF()");
    const std::size_t aHexStart = aMarkerPosition + aMarker.size();
    const std::size_t aMarkerEnd = aData.find(" -->", aHexStart);
    aPersistenceData = decodeHexMarker(
      aData, aHexStart, aMarkerEnd, "XCAF XML recovery marker is invalid");
  }

  const occ::handle<TDocStd_Application> anApplication = new TDocStd_Application();
  occ::handle<TDocStd_Document> aDocument;
  BinXCAFDrivers::DefineFormat(anApplication);
  std::istringstream aStream(aPersistenceData, std::ios::in | std::ios::binary);
  if (anApplication->Open(aStream, aDocument) != PCDM_RS_OK || aDocument.IsNull())
    throw KernelFailure(ErrorCode::ImportExportFailed, "XCAF document input could not be parsed");
  XCAFDoc_DocumentTool::Set(aDocument->Main());

  ImportedDocumentData aImported = inspectImportedDocument(aDocument);
  DocumentNodeData aNodes = extractDocumentNodes(theContext, 
    false, json(), aScope, aImported.shapeTool, aImported.colorTool, aImported.layerTool,
    aImported.visMaterialTool, aImported.rootLabels);
  const DocumentAnnotationData aAnnotations =
    extractDocumentAnnotations(theContext, false, {}, aScope, aImported.viewTool,
      aImported.clippingPlaneTool, aImported.dimTolTool,
      aNodes.occurrenceLabels, aNodes.definitionLabels);
  return importedDocumentResult(theContext, aScope, aImported.shape,
                                aImported.rootLabels.Size(), std::move(aNodes), aAnnotations);
}

json h_importIGESDocument(KernelOperationContext& theContext, const json& theArgs)
{
  const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
  const std::string aData = inputBufferData(theContext.buffers(), theArgs, "data");
  json aMetadata;
  std::string anIgesData = aData;
  const std::string aMarker = "# occt-worker-xcaf-meta:";
  const std::size_t aMarkerPosition = aData.rfind(aMarker);
  if (aMarkerPosition != std::string::npos)
  {
    const std::size_t aHexStart = aMarkerPosition + aMarker.size();
    const std::size_t aMarkerEnd = aData.find_first_of("\r\n", aHexStart);
    const std::string aPayload = decodeHexMarker(
      aData, aHexStart, aMarkerEnd, "IGES XCAF metadata marker is invalid");
    try { aMetadata = json::parse(aPayload); }
    catch (const std::exception&)
    {
      throw KernelFailure(ErrorCode::ImportExportFailed, "IGES XCAF metadata marker is invalid");
    }
    if (!aMetadata.is_object() || aMetadata.value("version", 0) != 1
        || !aMetadata.value("nodes", json::array()).is_array()
        || !aMetadata.value("roots", json::array()).is_array())
      throw KernelFailure(ErrorCode::ImportExportFailed, "IGES XCAF metadata marker is invalid");
    anIgesData.resize(aMarkerPosition);
  }

  std::istringstream aStream(anIgesData, std::ios::in | std::ios::binary);
  const auto aUnit = igesLengthUnit(theArgs);
  IGESUnitGuard aUnitGuard(std::get<1>(aUnit), std::get<2>(aUnit));
  const occ::handle<TDocStd_Document> aDocument = new TDocStd_Document("BinXCAF");
  XCAFDoc_DocumentTool::Set(aDocument->Main());
  IGESCAFControl_Reader aReader;
  aReader.SetColorMode(true); aReader.SetNameMode(true); aReader.SetLayerMode(true);
  if (aReader.ReadStream("occt-worker", aStream) != IFSelect_RetDone)
    throw KernelFailure(ErrorCode::ImportExportFailed, "IGES XCAF document input could not be parsed");
  const int aTransferRootCount = aReader.NbRootsForTransfer();
  occ::handle<Message_ProgressIndicator> aProgress = new HostProgressIndicator();
  Message_ProgressScope aProgressScope(aProgress->Start(), "IGES XCAF import", 1.0);
  const bool isTransferred = aTransferRootCount > 0
    && aReader.Transfer(aDocument, aProgressScope.Next());
  if (!aProgressScope.More())
    throw KernelFailure(ErrorCode::Cancelled, "OCCT IGES document import cancelled");
  if (!isTransferred)
    throw KernelFailure(ErrorCode::ImportExportFailed,
                        "IGES XCAF document contained no transferable roots");

  ImportedDocumentData aImported = inspectImportedDocument(aDocument);
  DocumentNodeData aNodes = extractDocumentNodes(theContext, 
    true, aMetadata, aScope, aImported.shapeTool, aImported.colorTool, aImported.layerTool,
    aImported.visMaterialTool, aImported.rootLabels);
  const DocumentAnnotationData aAnnotations =
    extractDocumentAnnotations(theContext, false, {}, aScope, aImported.viewTool,
      aImported.clippingPlaneTool, aImported.dimTolTool,
      aNodes.occurrenceLabels, aNodes.definitionLabels);
  if (!aMetadata.contains("nodes"))
    return importedDocumentResult(theContext, aScope, aImported.shape,
                                  aImported.rootLabels.Size(), std::move(aNodes), aAnnotations);

  const json& aMetadataNodes = aMetadata.at("nodes");
  json aReconstructedNodes = json::array();
  std::size_t aPartIndex = 0;
  const auto aDocumentHandle = theContext.arena().add(aImported.shape, aScope);
  for (const json& aMetadataNode : aMetadataNodes)
  {
    if (!aMetadataNode.is_object() || !aMetadataNode.value("children", json::array()).is_array())
      throw KernelFailure(ErrorCode::ImportExportFailed, "IGES XCAF metadata marker is invalid");
    const std::string aKind = aMetadataNode.value("kind", "part");
    if (aKind != "part" && aKind != "assembly")
      throw KernelFailure(ErrorCode::ImportExportFailed, "IGES XCAF metadata marker is invalid");
    json aNode = {
      {"kind", aKind}, {"children", aMetadataNode.at("children")}, {"shape", aDocumentHandle},
      {"transform", aMetadataNode.value("transform",
        json::array({1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0}))},
      {"layers", aMetadataNode.value("layers", json::array())},
      {"subshapeStyles", json::array()}, {"visible", true}
    };
    if (aKind == "part" && aPartIndex < aNodes.nodes.size())
    {
      aNode["shape"] = aNodes.nodes.at(aPartIndex).at("shape");
      ++aPartIndex;
    }
    if (aMetadataNode.contains("name")) aNode["name"] = aMetadataNode.at("name");
    if (aMetadataNode.contains("color")) aNode["color"] = aMetadataNode.at("color");
    aReconstructedNodes.push_back(std::move(aNode));
  }
  const json& aMetadataRoots = aMetadata.at("roots");
  return {{"shape", aDocumentHandle},
          {"rootCount", static_cast<int>(aMetadataRoots.size())},
          {"roots", aMetadataRoots}, {"nodes", std::move(aReconstructedNodes)},
          {"gdt", json::array()}, {"datums", json::array()},
          {"geometricTolerances", json::array()}, {"views", json::array()},
          {"shuo", json::array()}};
}

} // namespace occt_worker
