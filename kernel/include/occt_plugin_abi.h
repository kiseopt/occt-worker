// occt-worker plugin ABI: versioned contract between a shared Main module
// and dynamically loaded Side modules.
//
// The Main module owns memory, table, allocator, and the C++ exception
// runtime. Side modules are relocatable PIC objects that resolve every
// undefined symbol from Main at load time and register an operation table
// through occt_plugin_init_v1.

#ifndef OCCT_WORKER_PLUGIN_ABI_H
#define OCCT_WORKER_PLUGIN_ABI_H

#include <stdint.h>
#include "build_identity.generated.h"

// Build family identity: Side modules may only load into a Main module built
// from the same OCCT/emsdk/flag configuration. Mismatched families must be
// rejected by the loader before registration.
typedef struct occt_plugin_build_identity_v1 {
  const char* buildFamily;
  const char* occtVersion;
  const char* emsdkVersion;
  uint32_t abiVersion;       // OCCT_PLUGIN_ABI_VERSION_V1
} occt_plugin_build_identity_v1;

// Operation descriptor registered by a Side module at init time.
// opNames points to opCount '\0'-terminated names in registry order.
typedef struct occt_plugin_descriptor_v1 {
  uint32_t abiVersion;              // must equal OCCT_PLUGIN_ABI_VERSION_V1
  const char* pluginName;           // unique per side module
  const char* semanticModule;       // see protocol/modules.json
  occt_plugin_build_identity_v1 identity;
  uint32_t opCount;
  const char* const* opNames;       // opCount entries
  void (*registerOperations)(void);
} occt_plugin_descriptor_v1;

#ifdef __cplusplus
extern "C" {
#endif
int occt_host_register_plugin_v1(const occt_plugin_descriptor_v1* descriptor);
#ifdef __cplusplus
}
#endif

#endif
