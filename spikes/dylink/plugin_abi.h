// occt-worker dylink spike - shared plugin ABI v1 definitions.
//
// This header defines the versioned contract between the Main module (host)
// and Side modules (plugins). It is the only file shared by both targets.

#ifndef OCCT_SPIKE_PLUGIN_ABI_H
#define OCCT_SPIKE_PLUGIN_ABI_H

#include <stdint.h>

#define OCCT_PLUGIN_ABI_VERSION_V1 1u

typedef struct occt_spike_plugin_descriptor_v1 {
  uint32_t abiVersion;
  const char* pluginName;
  const char* buildFamily;
  uint32_t opCount;
  const char* opNames;      // comma-separated names, opCount entries
  int (*const* ops)(int);   // opCount entries, each a C ABI int(int) operation
} occt_spike_plugin_descriptor_v1;

#endif
