// Focused CAD exchange handlers.

#include "kernel_step_shape_exchange_operations.hpp"
#include "kernel_exchange_cad_helpers.hpp"
#include "kernel_operation_context.hpp"
#include "kernel_protocol_helpers.hpp"

namespace occt_worker {

json h_exportSTEP(KernelOperationContext& theContext, const json& theArgs)
{
      std::vector<TopoDS_Shape> aShapes;
      if (theArgs.contains("shape"))
        aShapes.push_back(theContext.arena().get(requiredU32(theArgs, "shape")));
      else if (theArgs.contains("shapes") && theArgs.at("shapes").is_array()
               && !theArgs.at("shapes").empty())
      {
        for (const json& aShape : theArgs.at("shapes"))
          aShapes.push_back(theContext.arena().get(aShape.get<std::uint32_t>()));
      }
      else
        throw KernelFailure(ErrorCode::InvalidArgs, "STEP export requires shape or shapes");
      const auto aUnit = stepLengthUnit(theArgs);
      const std::string aTimestamp = theArgs.value("timestamp", "2026-01-01T00:00:00");
      STEPControl_Writer aWriter;
      DESTEP_Parameters aParams;
      aParams.WriteUnit = aUnit.first;
      aParams.WriteSchema = stepWriteSchema(theArgs);
      aParams.WriteProductName = "occt-worker";
      // Model coordinates and the written STEP file share the requested unit, so no scaling occurs.
      aWriter.Model(true)->SetLocalLengthUnit(aUnit.second);
      occ::handle<Message_ProgressIndicator> aProgress = new HostProgressIndicator();
      Message_ProgressScope aProgressScope(
        aProgress->Start(), "STEP export", static_cast<Standard_Real>(aShapes.size()));
      for (const TopoDS_Shape& aShape : aShapes)
      {
        const IFSelect_ReturnStatus aStatus = aWriter.Transfer(
          aShape, STEPControl_AsIs, aParams, true, aProgressScope.Next());
        if (!aProgressScope.More())
          throw KernelFailure(ErrorCode::Cancelled, "OCCT STEP export cancelled");
        if (aStatus != IFSelect_RetDone)
          throw KernelFailure(ErrorCode::ImportExportFailed, "OCCT STEP transfer failed");
      }
      APIHeaderSection_MakeHeader aHeader(aWriter.Model());
      aHeader.SetName(new TCollection_HAsciiString("occt-worker"));
      aHeader.SetTimeStamp(new TCollection_HAsciiString(aTimestamp.c_str()));
      std::ostringstream aStream;
      if (aWriter.WriteStream(aStream) != IFSelect_RetDone)
      {
        throw KernelFailure(ErrorCode::ImportExportFailed, "OCCT STEP write failed");
      }
      std::string aData = aStream.str();
      std::size_t aProductPosition = 0;
      std::size_t aProductIndex = 1;
      while ((aProductPosition = aData.find("PRODUCT('", aProductPosition)) != std::string::npos)
      {
        const std::size_t aFirstStart = aProductPosition + std::strlen("PRODUCT('");
        const std::size_t aFirstEnd = aData.find('\'', aFirstStart);
        const std::size_t aSecondQuote =
          aFirstEnd == std::string::npos ? std::string::npos : aData.find('\'', aFirstEnd + 1);
        const std::size_t aSecondStart =
          aSecondQuote == std::string::npos ? std::string::npos : aSecondQuote + 1;
        const std::size_t aSecondEnd =
          aSecondStart == std::string::npos ? std::string::npos : aData.find('\'', aSecondStart);
        if (aFirstEnd == std::string::npos || aSecondEnd == std::string::npos)
        {
          break;
        }
        const std::string aStableProductName =
          "occt-worker-" + std::to_string(aProductIndex++);
        aData.replace(aSecondStart, aSecondEnd - aSecondStart, aStableProductName);
        aData.replace(aFirstStart, aFirstEnd - aFirstStart, aStableProductName);
        aProductPosition = aFirstStart + aStableProductName.size();
      }
      const std::uint32_t aBuffer = theContext.buffers().create(aData.size());
      if (!aData.empty())
      {
        std::memcpy(theContext.buffers().get(aBuffer).data(), aData.data(), aData.size());
      }
      return {{"data", bufferDescriptor(aBuffer, aData.size(), "step-utf8")}};
    }

json h_importSTEP(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const auto aUnit = stepLengthUnit(theArgs);
      const std::string aData = inputBufferData(theContext.buffers(), theArgs, "data");
      std::istringstream aStream(aData, std::ios::in | std::ios::binary);
      STEPControl_Reader aReader;
      DESTEP_Parameters aParams;
      if (aReader.ReadStream("occt-worker", aParams, aStream) != IFSelect_RetDone)
      {
        throw KernelFailure(ErrorCode::ImportExportFailed, "STEP input could not be parsed");
      }
      aReader.SetSystemLengthUnit(aUnit.second);
      occ::handle<Message_ProgressIndicator> aProgress = new HostProgressIndicator();
      const int aRootCount = aReader.TransferRoots(aProgress->Start());
      if (occt_worker_cancelled() != 0)
        throw KernelFailure(ErrorCode::Cancelled, "OCCT STEP import cancelled");
      if (aRootCount == 0)
      {
        throw KernelFailure(ErrorCode::ImportExportFailed, "STEP input did not contain a transferable shape");
      }
      const TopoDS_Shape aShape = aReader.OneShape();
      if (aShape.IsNull())
      {
        throw KernelFailure(ErrorCode::ImportExportFailed, "STEP input did not contain a shape");
      }
      json aShapes = json::array();
      for (int anIndex = 1; anIndex <= aReader.NbShapes(); ++anIndex)
      {
        const TopoDS_Shape aRootShape = aReader.Shape(anIndex);
        if (!aRootShape.IsNull()) aShapes.push_back(theContext.arena().add(aRootShape, aScope));
      }
      return {{"shape", theContext.arena().add(aShape, aScope)}, {"shapes", std::move(aShapes)},
              {"rootCount", aRootCount}};
    }

} // namespace occt_worker
