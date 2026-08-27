// Kernel core runtime: construction, dispatch frame, history guard, and the
// runtime semantic-group handlers (capabilities, scopes, buffers, stats).
//
// Only runtime operations register here. Semantic-group operations live in
// kernel/operations/*.cpp objects: as standalone targets they register via
// ensureOperationsRegistered(); as shared Side modules their constructors
// register into the Main module's registry at load time.

#include "kernel_context.hpp"
#include "kernel_runtime_operations.hpp"
#include "kernel_protocol_helpers.hpp"
#include "build_identity.generated.h"

namespace occt_worker {

json h_capabilities(KernelOperationContext& theContext, const json& theArgs)
{
      json anOperations = json::array();
      for (const char* anOperation : kOperationNames)
        if (OperationRegistry::instance().find(anOperation) != nullptr)
          anOperations.push_back(anOperation);
      json aHistorySupport = json::object();
      for (const HistorySupportEntry& anEntry : kHistorySupport)
        if (OperationRegistry::instance().find(anEntry.operation) != nullptr)
          aHistorySupport[anEntry.operation] = anEntry.support;
      return {{"protocolVersion", kProtocolVersion},
              {"kernelVersion", kProtocolVersion},
              {"occtVersion", OCCT_SHARED_OCCT_VERSION},
              {"ops", anOperations},
              {"historySupport", aHistorySupport},
              {"buildFlags", {{"threads", false}, {"simd", false}, {"wasmExceptions", true}}}};
    }

json h_beginScope(KernelOperationContext& theContext, const json& theArgs)
{
      return {{"scopeId", theContext.arena().beginScope()}};
    }

json h_endScope(KernelOperationContext& theContext, const json& theArgs)
{
      theContext.arena().endScope(requiredU32(theArgs, "scopeId"));
      return json::object();
    }

json h_release(KernelOperationContext& theContext, const json& theArgs)
{
      theContext.arena().release(requiredU32(theArgs, "shape"));
      return json::object();
    }

json h_releaseAll(KernelOperationContext& theContext, const json& theArgs)
{
      theContext.arena().clear();
      theContext.buffers().clear();
      return json::object();
    }

json h_stats(KernelOperationContext& theContext, const json& theArgs)
{
      return {{"liveShapeHandles", theContext.arena().liveCount()},
              {"liveBufferBytes", theContext.buffers().liveBytes()},
              {"wasmMemorySize", static_cast<std::uint64_t>(__builtin_wasm_memory_size(0)) * 65536u}};
    }

json h_createBuffer(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aLength = requiredU32(theArgs, "byteLength");
      return bufferDescriptor(theContext.buffers().create(aLength), aLength, "u8");
    }

json h_freeBuffer(KernelOperationContext& theContext, const json& theArgs)
{
      theContext.buffers().release(requiredU32(theArgs, "bufferId"));
      return json::object();
    }

json h_batch(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      if (!theArgs.contains("ops") || !theArgs.at("ops").is_array())
      {
        throw KernelFailure(ErrorCode::InvalidArgs, "batch requires an ops array");
      }
      json aResults = json::array();
      for (const json& anOperation : theArgs.at("ops"))
      {
        std::string aBatchOperation;
        try
        {
          if (!anOperation.is_object() || !anOperation.contains("op")
              || !anOperation.at("op").is_string())
          {
            throw KernelFailure(ErrorCode::InvalidArgs, "Invalid batch operation");
          }
          aBatchOperation = anOperation.at("op").get<std::string>();
          json anOperationArgs = anOperation.value("args", json::object());
          anOperationArgs["scopeId"] = aScope;
          auto resolveReferences = [&](auto&& self, json& theValue) -> void {
            if (theValue.is_object() && theValue.size() == 1 && theValue.contains("$ref"))
            {
              const std::size_t anIndex = theValue.at("$ref").get<std::size_t>();
              if (anIndex >= aResults.size() || !aResults.at(anIndex).contains("shape"))
              {
                throw KernelFailure(ErrorCode::InvalidArgs, "Invalid batch shape reference");
              }
              theValue = aResults.at(anIndex).at("shape");
            }
            else if (theValue.is_array())
            {
              for (json& anItem : theValue) self(self, anItem);
            }
            else if (theValue.is_object())
            {
              for (auto& anItem : theValue.items()) self(self, anItem.value());
            }
          };
          resolveReferences(resolveReferences, anOperationArgs);
          aResults.push_back(theContext.dispatch(anOperation.at("op").get<std::string>(), anOperationArgs));
        }
        catch (const KernelFailure& aFailure)
        {
          json aDetails = {{"operation", "batch"}};
          if (!aBatchOperation.empty())
          {
            aDetails["nestedOperation"] = aBatchOperation;
            if (const char* aFormat = exchangeFormatForOperation(aBatchOperation))
              aDetails["format"] = aFormat;
          }
          return {{"results", aResults},
                  {"error", {{"code", errorName(aFailure.code)},
                              {"message", aFailure.what()}, {"details", std::move(aDetails)}}}};
        }
      }
      return {{"results", aResults}};
    }

void Kernel::ensureOperationsRegistered()
{
  static const bool aRegistered = []() {
    register_runtime_operations();
    return true;
  }();
  (void)aRegistered;
}

void register_runtime_operations()
{
  auto& aRegistry = OperationRegistry::instance();
#define OCCT_REGISTER_HANDLER(theOperation) aRegistry.add(#theOperation, &h_##theOperation);
  OCCT_RUNTIME_OPERATION_TABLE(OCCT_REGISTER_HANDLER)
#undef OCCT_REGISTER_HANDLER
}

} // namespace occt_worker
