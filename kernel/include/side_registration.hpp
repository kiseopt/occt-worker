#pragma once

#include "occt_plugin_abi.h"

#if defined(OCCT_WORKER_SHARED_SIDE)
#define OCCT_DEFINE_SIDE_PLUGIN(theToken, theOperations, theRegisterFunction) \
  static const occt_plugin_descriptor_v1 theToken##_descriptor = { \
    OCCT_PLUGIN_ABI_VERSION_V1, OCCT_SIDE_NAME_##theToken, OCCT_SIDE_SEMANTIC_MODULE_##theToken, \
    {OCCT_SHARED_BUILD_FAMILY, OCCT_SHARED_OCCT_VERSION, OCCT_SHARED_EMSDK_VERSION, OCCT_PLUGIN_ABI_VERSION_V1}, \
    static_cast<uint32_t>(theOperations.size()), theOperations.data(), theRegisterFunction \
  }; \
  __attribute__((constructor)) static void theToken##_side_init() \
  { \
    (void)occt_host_register_plugin_v1(&theToken##_descriptor); \
  }
#else
#define OCCT_DEFINE_SIDE_PLUGIN(theToken, theOperations, theRegisterFunction) \
  __attribute__((constructor)) static void theToken##_standalone_init() \
  { \
    theRegisterFunction(); \
  }
#endif
