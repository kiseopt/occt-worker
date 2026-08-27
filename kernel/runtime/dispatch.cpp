#include "kernel_context.hpp"
#include "kernel_runtime_operations.hpp"

namespace occt_worker {

json Kernel::dispatchOperation(const std::string& theOp, const json& theArgs)
  {
    ensureOperationsRegistered();
    if (theArgs.value("includeHistory", false))
    {
      const auto aSupport = std::find_if(kHistorySupport.begin(), kHistorySupport.end(),
        [&](const HistorySupportEntry& theEntry) { return theOp == theEntry.operation; });
      if (aSupport == kHistorySupport.end()
          || std::string_view(aSupport->support) == "unsupported")
      {
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "History is unsupported for operation: " + theOp);
      }
    }

    const OperationHandler anHandler = OperationRegistry::instance().find(theOp);
    if (anHandler == nullptr)
    {
      throw KernelFailure(ErrorCode::ProtocolError, "Unknown operation: " + theOp);
    }
    return anHandler(*this, theArgs);
  }

} // namespace occt_worker
