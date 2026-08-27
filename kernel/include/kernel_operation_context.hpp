// Shared operation state for protocol handlers.
#pragma once
#include "kernel_support.hpp"

namespace occt_worker {

class KernelOperationContext
{
public:
  KernelOperationContext()
  {
    BOPAlgo_Options::SetParallelMode(false);
  }

  virtual ~KernelOperationContext() = default;
  virtual json dispatch(const std::string& theOp, const json& theArgs) = 0;
  BufferStore& buffers() { return myBuffers; }
  ShapeArena& arena() { return myArena; }

protected:
  ShapeArena myArena;
  BufferStore myBuffers;
};

} // namespace occt_worker
