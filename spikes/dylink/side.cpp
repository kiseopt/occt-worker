// occt-worker dylink spike - Side module (plugin).
//
// Built with -sSIDE_MODULE=2 (never STANDALONE_WASM). All undefined symbols
// are resolved from the Main module at load time: the shared allocator,
// host callbacks, and the C++ exception runtime (__cxa_* personality).

#include <stdint.h>
#include <string.h>

#include <stdexcept>
#include <string>

#include "plugin_abi.h"

#define EMSCRIPTEN_KEEPALIVE __attribute__((used))

// Host API resolved from Main at load time.
extern "C" {
int plugin_register_v1(const occt_spike_plugin_descriptor_v1* descriptor);
void* host_alloc(uint32_t size);
void host_free(void* pointer);
int host_add_one(int value);
uint32_t host_memory_pages(void);

EMSCRIPTEN_KEEPALIVE int occt_plugin_init_v1(void);
EMSCRIPTEN_KEEPALIVE int side_op_identity(int input);
EMSCRIPTEN_KEEPALIVE int side_op_eh_roundtrip(int input);
EMSCRIPTEN_KEEPALIVE int side_op_trap(int input);
EMSCRIPTEN_KEEPALIVE int side_op_host_pages(int input);
}

namespace {

const char* const kOpNames = "identity,eh_roundtrip,trap,host_pages";

int (*const kOps[])(int) = {
  &side_op_identity,
  &side_op_eh_roundtrip,
  &side_op_trap,
  &side_op_host_pages,
};

const occt_spike_plugin_descriptor_v1 kDescriptor = {
  OCCT_PLUGIN_ABI_VERSION_V1,
  "dylink-spike",
  "spike-dylink-v1",
  4,
  kOpNames,
  kOps,
};

}  // namespace

extern "C" {

// Versioned init entry. Idempotent: Main rejects duplicate plugin names.
EMSCRIPTEN_KEEPALIVE int occt_plugin_init_v1(void) {
  return plugin_register_v1(&kDescriptor);
}

EMSCRIPTEN_KEEPALIVE int side_op_identity(int input) {
  constexpr uint32_t kSize = 64;
  uint8_t* buffer = static_cast<uint8_t*>(host_alloc(kSize));
  if (buffer == nullptr) return -1000;
  memset(buffer, 0xAB, kSize);
  const bool intact = buffer[0] == 0xAB && buffer[kSize - 1] == 0xAB;
  host_free(buffer);
  if (!intact) return -1001;
  return host_add_one(input);
}

// Exceptions must not escape the C ABI boundary: throw inside, catch inside.
// The personality/__cxa functions come from Main's libc++abi across dylink.
EMSCRIPTEN_KEEPALIVE int side_op_eh_roundtrip(int input) {
  try {
    std::string message = "spike internal failure";
    if (message.size() > 1000) return -1;
    throw std::runtime_error(message);
    return input;
  } catch (const std::exception&) {
    return input + 100;
  }
}

// Deliberate trap that escapes to JS as a WebAssembly.RuntimeError.
// The address is far beyond MAXIMUM_MEMORY so the store is out of bounds.
EMSCRIPTEN_KEEPALIVE int side_op_trap(int input) {
  (void)input;
  *reinterpret_cast<volatile uint32_t*>(static_cast<uintptr_t>(0xFFFFFF00u)) = 42u;
  return 0;
}

EMSCRIPTEN_KEEPALIVE int side_op_host_pages(int input) {
  if (input != 0) return -1;
  return static_cast<int>(host_memory_pages());
}

// Auto-registration on load: Emscripten runs __wasm_call_ctors for side
// modules right after relocation, so the descriptor is registered exactly
// once per loaded module instance, before any JS can call into it.
__attribute__((constructor)) static void spike_auto_register(void) {
  occt_plugin_init_v1();
}

}  // extern "C"
