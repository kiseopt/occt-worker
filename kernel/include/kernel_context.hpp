// Kernel dispatch boundary. Operation handlers live in focused module classes.
#pragma once

#include "kernel_operation_context.hpp"

namespace occt_worker {

class Kernel final : public KernelOperationContext
{
public:
  json dispatch(const std::string& theOp, const json& theArgs) override
  {
    const std::uint64_t aShapeCheckpoint = myArena.checkpoint();
    const std::uint64_t aBufferCheckpoint = myBuffers.checkpoint();
    const auto rollback = [&]() {
      myArena.rollback(aShapeCheckpoint);
      myBuffers.rollback(aBufferCheckpoint);
    };
    try
    {
      return dispatchOperation(theOp, theArgs);
    }
    catch (const KernelFailure&)
    {
      rollback();
      throw;
    }
    catch (const json::exception& aFailure)
    {
      rollback();
      throw KernelFailure(ErrorCode::InvalidArgs, aFailure.what());
    }
    catch (const Standard_Failure& aFailure)
    {
      rollback();
      throw KernelFailure(operationFailureCode(theOp), aFailure.what());
    }
    catch (const std::bad_alloc&)
    {
      rollback();
      throw;
    }
    catch (const std::exception& aFailure)
    {
      rollback();
      throw KernelFailure(operationFailureCode(theOp), aFailure.what());
    }
    catch (...)
    {
      rollback();
      throw;
    }
  }

  json dispatchOperation(const std::string& theOp, const json& theArgs);
  static void ensureOperationsRegistered();
};

extern Kernel THE_KERNEL;
extern std::string THE_RESPONSE;

} // namespace occt_worker
