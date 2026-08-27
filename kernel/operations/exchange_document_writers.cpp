#include "kernel_exchange_cad_helpers.hpp"
#include "kernel_exchange_helpers.hpp"
#include "kernel_operation_context.hpp"
#include "kernel_protocol_helpers.hpp"

namespace occt_worker {

namespace {

std::string encodeHex(const std::string& thePayload)
{
  static constexpr char aHex[] = "0123456789abcdef";
  std::string aEncoded;
  aEncoded.reserve(thePayload.size() * 2);
  for (const unsigned char aByte : thePayload)
  {
    aEncoded.push_back(aHex[aByte >> 4]);
    aEncoded.push_back(aHex[aByte & 0x0f]);
  }
  return aEncoded;
}

} // namespace

json writeXCAFDocument(KernelOperationContext& theContext,
                       const json& theArgs,
                               const occ::handle<TDocStd_Document>& theDocument,
                               const std::string& theFormat)
{
  const occ::handle<TDocStd_Application> anApplication = new TDocStd_Application();
  if (theFormat == "bin")
    BinXCAFDrivers::DefineFormat(anApplication);
  else
    XmlXCAFDrivers::DefineFormat(anApplication);
  theDocument->Open(anApplication);
  std::stringstream aStream(std::ios::in | std::ios::out | std::ios::binary);
  if (anApplication->SaveAs(theDocument, aStream) != PCDM_SS_OK)
    throw KernelFailure(ErrorCode::ImportExportFailed, "OCCT XCAF document write failed");
  std::string aData = aStream.str();
  if (theFormat == "xml"
      && ((theArgs.contains("datums") && !theArgs.at("datums").empty())
          || (theArgs.contains("geometricTolerances")
              && !theArgs.at("geometricTolerances").empty())))
  {
    json aMetadata = {
      {"version", 1},
      {"datums", theArgs.value("datums", json::array())},
      {"geometricTolerances", theArgs.value("geometricTolerances", json::array())}
    };
    auto persistMarkerPresentation = [&](json& theItem) {
      if (!theItem.is_object() || !theItem.contains("presentation")) return;
      const json& aPresentation = theItem.at("presentation");
      std::ostringstream aPresentationStream(std::ios::out | std::ios::binary);
      BRepTools::Write(theContext.arena().get(requiredU32(aPresentation, "shape")),
                       aPresentationStream, false, false,
                       TopTools_FormatVersion_CURRENT);
      theItem["presentationBrep"] = aPresentationStream.str();
      if (aPresentation.contains("name"))
        theItem["presentationName"] = aPresentation.at("name");
      theItem.erase("presentation");
    };
    for (json& aDatum : aMetadata["datums"])
      persistMarkerPresentation(aDatum);
    for (json& aTolerance : aMetadata["geometricTolerances"])
      persistMarkerPresentation(aTolerance);

    const std::string aPayload = aMetadata.dump();
    const std::string aEncoded = encodeHex(aPayload);
    const std::string aMarker = "<!-- occt-worker-xcaf-meta:" + aEncoded + " -->";
    const std::size_t anEnd = aData.rfind("</document>");
    if (anEnd == std::string::npos)
      aData += "\n" + aMarker + "\n";
    else
      aData.insert(anEnd, aMarker + "\n");
  }
  const std::uint32_t aBuffer = theContext.buffers().create(aData.size());
  if (!aData.empty()) std::memcpy(theContext.buffers().get(aBuffer).data(), aData.data(), aData.size());
  return {{"data", bufferDescriptor(aBuffer, aData.size(),
                                      theFormat == "bin" ? "xcaf-bin" : "xcaf-xml")}};
}

json writeIGESDocument(KernelOperationContext& theContext,
                       const json& theArgs,
                               const occ::handle<TDocStd_Document>& theDocument)
{
  const auto aUnit = igesLengthUnit(theArgs);
  IGESUnitGuard aUnitGuard(std::get<1>(aUnit), std::get<2>(aUnit));
  const occ::handle<XSControl_WorkSession> aSession = new XSControl_WorkSession();
  IGESCAFControl_Writer aWriter(aSession, std::get<0>(aUnit).c_str());
  aWriter.SetColorMode(true);
  aWriter.SetNameMode(true);
  aWriter.SetLayerMode(true);
  occ::handle<Message_ProgressIndicator> aProgress = new HostProgressIndicator();
  Message_ProgressScope aProgressScope(aProgress->Start(), "IGES XCAF export", 2.0);
  if (!aWriter.Transfer(theDocument, aProgressScope.Next()))
    throw KernelFailure(ErrorCode::ImportExportFailed, "OCCT IGES XCAF document transfer failed");
  if (!aProgressScope.More())
    throw KernelFailure(ErrorCode::Cancelled, "OCCT IGES XCAF export cancelled");
  std::ostringstream aStream(std::ios::out | std::ios::binary);
  if (!aWriter.Write(aStream))
    throw KernelFailure(ErrorCode::ImportExportFailed, "OCCT IGES XCAF document write failed");
  std::string aData = aStream.str();

  json aMetadata = { {"version", 1}, {"nodes", json::array()} };
  for (const json& aNode : theArgs.at("nodes"))
  {
    json anItem = {
      {"kind", aNode.value("kind", "part")},
      {"children", aNode.value("children", json::array())},
      {"layers", aNode.value("layers", json::array())}
    };
    if (aNode.contains("name")) anItem["name"] = aNode.at("name");
    if (aNode.contains("color")) anItem["color"] = aNode.at("color");
    if (aNode.contains("transform")) anItem["transform"] = aNode.at("transform");
    aMetadata["nodes"].push_back(std::move(anItem));
  }
  aMetadata["roots"] = theArgs.value("roots", json::array());
  const std::string aPayload = aMetadata.dump();
  const std::string aEncoded = encodeHex(aPayload);
  aData += "\n# occt-worker-xcaf-meta:" + aEncoded + "\n";
  const std::uint32_t aBuffer = theContext.buffers().create(aData.size());
  if (!aData.empty()) std::memcpy(theContext.buffers().get(aBuffer).data(), aData.data(), aData.size());
  return {{"data", bufferDescriptor(aBuffer, aData.size(), "iges-xcaf-text")}};
}

json writeSTEPDocument(KernelOperationContext& theContext,
                       const json& theArgs,
                               const occ::handle<TDocStd_Document>& theDocument)
{
  const auto aUnit = stepLengthUnit(theArgs);
  const std::string aTimestamp = theArgs.value("timestamp", "2026-01-01T00:00:00");
  STEPCAFControl_Writer aWriter;
  aWriter.SetColorMode(true);
  aWriter.SetNameMode(true);
  aWriter.SetLayerMode(true);
  aWriter.SetPropsMode(true);
  aWriter.SetMaterialMode(true);
  aWriter.SetVisualMaterialMode(true);
  aWriter.SetDimTolMode(true);
  DESTEP_Parameters aParams;
  aParams.WriteUnit = aUnit.first;
  aParams.WriteSchema = stepWriteSchema(theArgs);
  aParams.WriteProductName = "occt-worker";
  aWriter.ChangeWriter().Model(true)->SetLocalLengthUnit(aUnit.second);
  occ::handle<Message_ProgressIndicator> aProgress = new HostProgressIndicator();
  Message_ProgressScope aProgressScope(aProgress->Start(), "STEP XCAF export", 2.0);
  const bool isTransferred = aWriter.Transfer(
    theDocument, aParams, STEPControl_AsIs, nullptr, aProgressScope.Next());
  if (!aProgressScope.More())
    throw KernelFailure(ErrorCode::Cancelled, "OCCT STEP document export cancelled");
  if (!isTransferred)
    throw KernelFailure(ErrorCode::ImportExportFailed, "OCCT STEP document transfer failed");
  APIHeaderSection_MakeHeader aHeader(aWriter.ChangeWriter().Model());
  aHeader.SetName(new TCollection_HAsciiString("occt-worker"));
  aHeader.SetTimeStamp(new TCollection_HAsciiString(aTimestamp.c_str()));
  std::ostringstream aStream;
  if (aWriter.WriteStream(aStream) != IFSelect_RetDone)
    throw KernelFailure(ErrorCode::ImportExportFailed, "OCCT STEP document write failed");
  aProgressScope.Next();
  aProgressScope.Show();
  std::string aData = aStream.str();
  if (theArgs.contains("gdt") && theArgs.at("gdt").is_array())
  {
    std::string aMarkers;
    for (const json& anItem : theArgs.at("gdt"))
    {
      aMarkers += "/*occt-worker-gdt:";
      if (anItem.contains("semanticName"))
      {
        const std::string aSemanticName = anItem.at("semanticName").get<std::string>();
        for (const char aCharacter : aSemanticName)
        {
          if (aCharacter == '*' || aCharacter == '/') aMarkers += '_';
          else aMarkers += aCharacter;
        }
      }
      aMarkers += "*/\n";
    }
    const std::size_t anEndSection = aData.rfind("ENDSEC;");
    if (anEndSection != std::string::npos) aData.insert(anEndSection, aMarkers);
  }
  const std::uint32_t aBuffer = theContext.buffers().create(aData.size());
  if (!aData.empty()) std::memcpy(theContext.buffers().get(aBuffer).data(), aData.data(), aData.size());
  return {{"data", bufferDescriptor(aBuffer, aData.size(), "step-xcaf-utf8")}};
}

} // namespace occt_worker
