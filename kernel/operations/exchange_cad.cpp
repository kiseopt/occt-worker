#include "kernel_exchange_cad_operations.hpp"
#include "kernel_operation_context.hpp"
#include "side_registration.hpp"

namespace occt_worker {

void register_exchange_cad_operations()
{
  auto& aRegistry = OperationRegistry::instance();
#define OCCT_REGISTER_HANDLER(theOperation) aRegistry.add(#theOperation, &h_##theOperation);
  OCCT_EXCHANGE_CAD_OPERATION_TABLE(OCCT_REGISTER_HANDLER)
#undef OCCT_REGISTER_HANDLER
}

OCCT_DEFINE_SIDE_PLUGIN(exchange_cad, kExchangeCadOperationNames, register_exchange_cad_operations)

} // namespace occt_worker
