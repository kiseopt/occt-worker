// Kernel geometry-topology operations: one out-of-line Kernel::h_* handler per dispatch branch.
// Handler bodies are byte-identical fragments of the original dispatchOperation.
// As a shared Side module the constructor registers into the Main registry
// right after Emscripten runs __wasm_call_ctors.

#include "kernel_geometry_topology_operations.hpp"
#include "kernel_operation_context.hpp"
#include "side_registration.hpp"

namespace occt_worker {














































void register_geometry_topology_operations()
{
  auto& aRegistry = OperationRegistry::instance();
#define OCCT_REGISTER_HANDLER(theOperation) aRegistry.add(#theOperation, &h_##theOperation);
  OCCT_GEOMETRY_TOPOLOGY_OPERATION_TABLE(OCCT_REGISTER_HANDLER)
#undef OCCT_REGISTER_HANDLER
}

OCCT_DEFINE_SIDE_PLUGIN(geometry_topology, kGeometryTopologyOperationNames, register_geometry_topology_operations)

} // namespace occt_worker
