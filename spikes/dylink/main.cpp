// occt-worker dylink spike - Main module (host).
//
// Built with -sMAIN_MODULE=2. Provides the shared allocator, memory, table,
// exception runtime, and the plugin registry that Side modules register into.

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <stdexcept>

#include "plugin_abi.h"

#define EMSCRIPTEN_KEEPALIVE __attribute__((used))

namespace {

struct Registration {
  const occt_spike_plugin_descriptor_v1* descriptor;
};

constexpr uint32_t kMaxRegistrations = 8;

Registration g_registrations[kMaxRegistrations];
uint32_t g_registrationCount = 0;
char g_lastError[160] = {0};

void setLastError(const char* message) {
  strncpy(g_lastError, message, sizeof(g_lastError) - 1);
  g_lastError[sizeof(g_lastError) - 1] = '\0';
}

}  // namespace

extern "C" {

EMSCRIPTEN_KEEPALIVE int plugin_register_v1(const occt_spike_plugin_descriptor_v1* descriptor) {
  if (descriptor == nullptr) {
    setLastError("descriptor is null");
    return -1;
  }
  if (descriptor->abiVersion != OCCT_PLUGIN_ABI_VERSION_V1) {
    setLastError("unsupported descriptor abiVersion");
    return -2;
  }
  if (descriptor->pluginName == nullptr || descriptor->buildFamily == nullptr || descriptor->ops == nullptr) {
    setLastError("descriptor fields are null");
    return -3;
  }
  for (uint32_t i = 0; i < g_registrationCount; ++i) {
    if (strcmp(g_registrations[i].descriptor->pluginName, descriptor->pluginName) == 0) {
      setLastError("plugin already registered");
      return -4;
    }
  }
  if (g_registrationCount >= kMaxRegistrations) {
    setLastError("registration table full");
    return -5;
  }
  g_registrations[g_registrationCount].descriptor = descriptor;
  const int pluginId = static_cast<int>(g_registrationCount);
  ++g_registrationCount;
  return pluginId;
}

EMSCRIPTEN_KEEPALIVE int32_t plugin_registration_count(void) {
  return static_cast<int32_t>(g_registrationCount);
}

EMSCRIPTEN_KEEPALIVE const char* plugin_descriptor_name(int32_t pluginId) {
  if (pluginId < 0 || static_cast<uint32_t>(pluginId) >= g_registrationCount) return nullptr;
  return g_registrations[pluginId].descriptor->pluginName;
}

EMSCRIPTEN_KEEPALIVE uint32_t plugin_descriptor_abi_version(int32_t pluginId) {
  if (pluginId < 0 || static_cast<uint32_t>(pluginId) >= g_registrationCount) return 0;
  return g_registrations[pluginId].descriptor->abiVersion;
}

EMSCRIPTEN_KEEPALIVE const char* host_last_error(void) { return g_lastError; }

EMSCRIPTEN_KEEPALIVE int host_add_one(int value) { return value + 1; }

EMSCRIPTEN_KEEPALIVE void* host_alloc(uint32_t size) { return malloc(size); }

EMSCRIPTEN_KEEPALIVE void host_free(void* pointer) { free(pointer); }

EMSCRIPTEN_KEEPALIVE uint32_t host_memory_pages(void) {
  return static_cast<uint32_t>(__builtin_wasm_memory_size(0));
}

EMSCRIPTEN_KEEPALIVE uint32_t host_grow_pages(uint32_t delta) {
  return static_cast<uint32_t>(__builtin_wasm_memory_grow(0, delta));
}

EMSCRIPTEN_KEEPALIVE int host_dispatch(int32_t pluginId, int32_t opIndex, int input) {
  if (pluginId < 0 || static_cast<uint32_t>(pluginId) >= g_registrationCount) {
    setLastError("dispatch: bad pluginId");
    return -1900001;
  }
  const occt_spike_plugin_descriptor_v1* descriptor = g_registrations[pluginId].descriptor;
  if (opIndex < 0 || static_cast<uint32_t>(opIndex) >= descriptor->opCount) {
    setLastError("dispatch: bad opIndex");
    return -1900002;
  }
  // Calls through a function pointer stored in the side module's data section.
  // The pointer was relocated into the single shared table at load time.
  return descriptor->ops[opIndex](input);
}

EMSCRIPTEN_KEEPALIVE const char* host_build_family(void) { return "spike-dylink-v1"; }

EMSCRIPTEN_KEEPALIVE const char* occt_host_build_family(void) { return host_build_family(); }
EMSCRIPTEN_KEEPALIVE int32_t occt_host_plugin_count(void) { return plugin_registration_count(); }
EMSCRIPTEN_KEEPALIVE int32_t occt_host_plugin_error(void) { return 0; }

// Keeps the C++ exception symbols that side modules resolve from Main
// (runtime_error vtable/typeinfo/destructor, __cxa personality). Without a
// live reference the linker would drop them because Main itself never throws.
EMSCRIPTEN_KEEPALIVE int host_eh_symbol_anchor(void) {
  try {
    std::runtime_error error("anchor");
    throw error;
  } catch (const std::exception&) {
    return 0;
  }
}

}  // extern "C"
