// Focused CAD exchange handlers.

#include "kernel_exchange_cad_operations.hpp"
#include "kernel_exchange_cad_helpers.hpp"
#include "kernel_operation_context.hpp"
#include "kernel_protocol_helpers.hpp"

namespace occt_worker {

json h_exportIGES(KernelOperationContext& theContext, const json& theArgs)
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
        throw KernelFailure(ErrorCode::InvalidArgs, "IGES export requires shape or shapes");
      const auto aUnit = igesLengthUnit(theArgs);
      const std::string aModeName = theArgs.value("mode", "faces");
      int aMode = 0;
      if (aModeName == "brep")
      {
        aMode = 1;
      }
      else if (aModeName != "faces")
      {
        throw KernelFailure(ErrorCode::InvalidArgs, "Unsupported IGES write mode: " + aModeName);
      }
      IGESUnitGuard aUnitGuard(std::get<1>(aUnit), std::get<2>(aUnit));
      IGESControl_Writer aWriter(std::get<0>(aUnit).c_str(), aMode);
      occ::handle<Message_ProgressIndicator> aProgress = new HostProgressIndicator();
      Message_ProgressScope aProgressScope(
        aProgress->Start(), "IGES export", static_cast<double>(aShapes.size() + 1));
      aProgressScope.Next().Close();
      if (!aProgressScope.More())
        throw KernelFailure(ErrorCode::Cancelled, "OCCT IGES export cancelled");
      for (const TopoDS_Shape& aShape : aShapes)
      {
        const bool isTransferred = aWriter.AddShape(aShape, aProgressScope.Next());
        if (!aProgressScope.More())
          throw KernelFailure(ErrorCode::Cancelled, "OCCT IGES export cancelled");
        if (!isTransferred)
          throw KernelFailure(ErrorCode::ImportExportFailed, "OCCT IGES transfer failed");
      }
      std::ostringstream aStream(std::ios::out | std::ios::binary);
      if (!aWriter.Write(aStream))
      {
        throw KernelFailure(ErrorCode::ImportExportFailed, "OCCT IGES write failed");
      }
      const std::string aData = aStream.str();
      const std::uint32_t aBuffer = theContext.buffers().create(aData.size());
      if (!aData.empty())
      {
        std::memcpy(theContext.buffers().get(aBuffer).data(), aData.data(), aData.size());
      }
      return {{"data", bufferDescriptor(aBuffer, aData.size(), "iges-text")}};
    }

json h_importIGES(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const auto aUnit = igesLengthUnit(theArgs);
      const std::string aData = inputBufferData(theContext.buffers(), theArgs, "data");
      IGESUnitGuard aUnitGuard(std::get<1>(aUnit), std::get<2>(aUnit));
      std::istringstream aStream(aData, std::ios::in | std::ios::binary);
      IGESControl_Reader aReader;
      if (aReader.ReadStream("occt-worker", aStream) != IFSelect_RetDone)
      {
        throw KernelFailure(ErrorCode::ImportExportFailed, "IGES input could not be parsed");
      }
      occ::handle<Message_ProgressIndicator> aProgress = new HostProgressIndicator();
      Message_ProgressScope aProgressScope(aProgress->Start(), "IGES import", 2.0);
      aProgressScope.Next().Close();
      if (!aProgressScope.More())
        throw KernelFailure(ErrorCode::Cancelled, "OCCT IGES import cancelled");
      const int aRootCount = aReader.TransferRoots(aProgressScope.Next());
      if (!aProgressScope.More())
        throw KernelFailure(ErrorCode::Cancelled, "OCCT IGES import cancelled");
      if (aRootCount <= 0)
      {
        throw KernelFailure(ErrorCode::ImportExportFailed, "IGES input did not contain a transferable shape");
      }
      const TopoDS_Shape aShape = aReader.OneShape();
      if (aShape.IsNull())
      {
        throw KernelFailure(ErrorCode::ImportExportFailed, "IGES input did not contain a shape");
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
