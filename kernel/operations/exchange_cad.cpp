#include "kernel_step_shape_exchange_operations.hpp"
#include "kernel_operation_context.hpp"
#include "side_registration.hpp"

namespace occt_worker {

void register_step_shape_exchange_operations()
{
  auto& aRegistry = OperationRegistry::instance();
#define OCCT_REGISTER_HANDLER(theOperation) aRegistry.add(#theOperation, &h_##theOperation);
  OCCT_STEP_SHAPE_EXCHANGE_OPERATION_TABLE(OCCT_REGISTER_HANDLER)
#undef OCCT_REGISTER_HANDLER
}

OCCT_DEFINE_SIDE_PLUGIN(step_shape_exchange, kStepShapeExchangeOperationNames, register_step_shape_exchange_operations)

} // namespace occt_worker
