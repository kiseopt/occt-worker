// Standalone full-kernel WASM entry: k_alloc/k_free/k_handle/k_response_ptr/
// k_buffer_ptr/k_buffer_len exports preserved unchanged.

#include "kernel_context.hpp"
#include "occt_plugin_abi.h"
#include "side_registration.hpp"

namespace occt_worker {

#if defined(OCCT_WORKER_SHARED_MAIN)
static std::uint32_t THE_PLUGIN_COUNT = 0;
static int THE_PLUGIN_ERROR = 0;
static std::unordered_set<std::string> THE_PLUGIN_NAMES;
static std::string THE_LAST_PLUGIN_NAME;

extern "C" EMSCRIPTEN_KEEPALIVE int occt_host_register_plugin_v1(
  const occt_plugin_descriptor_v1* theDescriptor)
{
  THE_PLUGIN_ERROR = 0;
  if (theDescriptor == nullptr || theDescriptor->abiVersion != OCCT_PLUGIN_ABI_VERSION_V1
      || theDescriptor->identity.abiVersion != OCCT_PLUGIN_ABI_VERSION_V1
      || theDescriptor->pluginName == nullptr || theDescriptor->pluginName[0] == '\0'
      || theDescriptor->semanticModule == nullptr || theDescriptor->semanticModule[0] == '\0'
      || theDescriptor->identity.buildFamily == nullptr
      || std::strcmp(theDescriptor->identity.buildFamily, OCCT_SHARED_BUILD_FAMILY) != 0
      || theDescriptor->identity.occtVersion == nullptr
      || std::strcmp(theDescriptor->identity.occtVersion, OCCT_SHARED_OCCT_VERSION) != 0
      || theDescriptor->identity.emsdkVersion == nullptr
      || std::strcmp(theDescriptor->identity.emsdkVersion, OCCT_SHARED_EMSDK_VERSION) != 0
      || theDescriptor->opCount == 0 || theDescriptor->opNames == nullptr
      || theDescriptor->registerOperations == nullptr)
  {
    THE_PLUGIN_ERROR = 1;
    return 0;
  }
  const std::string aPluginName(theDescriptor->pluginName);
  if (!THE_PLUGIN_NAMES.emplace(aPluginName).second)
  {
    THE_PLUGIN_ERROR = 1;
    return 0;
  }
  OperationRegistry& aRegistry = OperationRegistry::instance();
  bool isRegistrationActive = false;
  try
  {
    THE_LAST_PLUGIN_NAME = aPluginName;
    aRegistry.beginRegistration();
    isRegistrationActive = true;
    theDescriptor->registerOperations();
    if (!aRegistry.commitRegistration(theDescriptor->opNames, theDescriptor->opCount))
    {
      isRegistrationActive = false;
      THE_PLUGIN_NAMES.erase(aPluginName);
      THE_PLUGIN_ERROR = 2;
      return 0;
    }
    isRegistrationActive = false;
    ++THE_PLUGIN_COUNT;
    return 1;
  }
  catch (...)
  {
    if (isRegistrationActive) aRegistry.rollbackRegistration();
    THE_PLUGIN_NAMES.erase(aPluginName);
    THE_PLUGIN_ERROR = 2;
    return 0;
  }
}

extern "C" EMSCRIPTEN_KEEPALIVE std::uint32_t occt_host_plugin_count()
{
  return THE_PLUGIN_COUNT;
}

extern "C" EMSCRIPTEN_KEEPALIVE int occt_host_plugin_error()
{
  return THE_PLUGIN_ERROR;
}

extern "C" EMSCRIPTEN_KEEPALIVE const char* occt_host_plugin_name()
{
  return THE_LAST_PLUGIN_NAME.c_str();
}

extern "C" EMSCRIPTEN_KEEPALIVE const char* occt_host_build_family()
{
  return OCCT_SHARED_BUILD_FAMILY;
}
#endif

Kernel THE_KERNEL;
std::string THE_RESPONSE;

} // namespace occt_worker

extern "C" {
#ifdef __EMSCRIPTEN__
std::uint32_t _emscripten_lookup_name(const char*)
{
  return 0;
}
#endif

EMSCRIPTEN_KEEPALIVE void* k_alloc(const std::uint32_t theLength)
{
  return std::malloc(theLength);
}

EMSCRIPTEN_KEEPALIVE void k_free(void* thePointer)
{
  std::free(thePointer);
}

EMSCRIPTEN_KEEPALIVE std::uint32_t k_handle(const std::uint8_t* theRequest,
                                            const std::uint32_t theLength)
{
  using namespace occt_worker;
  THE_RESPONSE = dispatchProtocolRequest(reinterpret_cast<const char*>(theRequest), theLength);
  return static_cast<std::uint32_t>(THE_RESPONSE.size());
}

EMSCRIPTEN_KEEPALIVE const std::uint8_t* k_response_ptr()
{
  return reinterpret_cast<const std::uint8_t*>(occt_worker::THE_RESPONSE.data());
}

EMSCRIPTEN_KEEPALIVE std::uint8_t* k_buffer_ptr(const std::uint32_t theBufferId)
{
  try
  {
    return occt_worker::THE_KERNEL.buffers().get(theBufferId).data();
  }
  catch (...)
  {
    return nullptr;
  }
}

EMSCRIPTEN_KEEPALIVE std::uint32_t k_buffer_len(const std::uint32_t theBufferId)
{
  try
  {
    return static_cast<std::uint32_t>(occt_worker::THE_KERNEL.buffers().get(theBufferId).size());
  }
  catch (...)
  {
    return 0;
  }
}
}
