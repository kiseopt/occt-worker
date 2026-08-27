#include "kernel_runtime_operations.hpp"
#include "kernel_operation_context.hpp"
#include "kernel_protocol_helpers.hpp"

namespace occt_worker {

json h_exportBREP(KernelOperationContext& theContext, const json& theArgs)
{
  std::ostringstream aStream(std::ios::out | std::ios::binary);
  BRepTools::Write(theContext.arena().get(requiredU32(theArgs, "shape")), aStream, false, false,
                   TopTools_FormatVersion_CURRENT);
  const std::string aData = aStream.str();
  const std::uint32_t aBuffer = theContext.buffers().create(aData.size());
  if (!aData.empty()) std::memcpy(theContext.buffers().get(aBuffer).data(), aData.data(), aData.size());
  return {{"data", bufferDescriptor(aBuffer, aData.size(), "u8")}};
}

json h_importBREP(KernelOperationContext& theContext, const json& theArgs)
{
  const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
  const std::string aData = inputBufferData(theContext.buffers(), theArgs, "data");
  std::istringstream aStream(aData, std::ios::in | std::ios::binary);
  TopoDS_Shape aShape;
  BRep_Builder aBuilder;
  BRepTools::Read(aShape, aStream, aBuilder);
  if (aShape.IsNull())
    throw KernelFailure(ErrorCode::ImportExportFailed, "BREP input did not contain a shape");
  return {{"shape", theContext.arena().add(aShape, aScope)}};
}

} // namespace occt_worker
