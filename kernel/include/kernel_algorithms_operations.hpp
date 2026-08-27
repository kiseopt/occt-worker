#pragma once
#include "generated.hpp"
#include "kernel_operation_handler.hpp"

namespace occt_worker {

#define OCCT_DECLARE_HANDLER(theOperation) \
  json h_##theOperation(KernelOperationContext& theContext, const json& theArgs);
OCCT_ALGORITHMS_OPERATION_TABLE(OCCT_DECLARE_HANDLER)
#undef OCCT_DECLARE_HANDLER

} // namespace occt_worker
