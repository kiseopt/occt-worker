// Kernel shape arena: generation-counted u32 handles, scopes, checkpoints.
// Bodies are byte-identical fragments of the original kernel/src/kernel.cpp.

#include "kernel_support.hpp"

namespace occt_worker {

std::uint32_t ShapeArena::beginScope()
{
    const std::uint32_t anId = myNextScope++;
    myScopes.emplace(anId, std::unordered_set<std::uint32_t>{});
    return anId;
  }

std::uint32_t ShapeArena::add(const TopoDS_Shape& theShape, const std::uint32_t theScope)
{
    const auto aScope = myScopes.find(theScope);
    if (aScope == myScopes.end())
    {
      throw KernelFailure(ErrorCode::InvalidArgs, "Unknown scopeId");
    }

    std::uint32_t anIndex;
    bool isNewSlot = false;
    if (myFree.empty())
    {
      if (mySlots.size() >= kIndexMask)
      {
        throw KernelFailure(ErrorCode::OutOfMemory, "Shape handle table is full");
      }
      anIndex = static_cast<std::uint32_t>(mySlots.size());
      mySlots.emplace_back();
      isNewSlot = true;
      try
      {
        // Keep release() and rollback() allocation-free for every live slot.
        myFree.reserve(mySlots.size());
      }
      catch (...)
      {
        mySlots.pop_back();
        throw;
      }
    }
    else
    {
      anIndex = myFree.back();
    }

    Slot& aSlot = mySlots[anIndex];
    const std::uint32_t aHandle = (aSlot.generation << kIndexBits) | (anIndex + 1);
    try
    {
      aScope->second.insert(aHandle);
      aSlot.shape = theShape;
    }
    catch (...)
    {
      aScope->second.erase(aHandle);
      if (isNewSlot) mySlots.pop_back();
      throw;
    }
    if (!isNewSlot) myFree.pop_back();
    aSlot.scope = theScope;
    aSlot.live = true;
    aSlot.creationSequence = ++myCreationSequence;
    ++myLiveCount;
    return aHandle;
  }

const TopoDS_Shape& ShapeArena::get(const std::uint32_t theHandle) const
{
    const std::uint32_t anEncodedIndex = theHandle & kIndexMask;
    const std::uint32_t aGeneration = theHandle >> kIndexBits;
    if (anEncodedIndex == 0 || anEncodedIndex > mySlots.size())
    {
      throw KernelFailure(ErrorCode::InvalidHandle, "Shape handle does not exist");
    }
    const Slot& aSlot = mySlots[anEncodedIndex - 1];
    if (!aSlot.live || aSlot.generation != aGeneration)
    {
      throw KernelFailure(ErrorCode::InvalidHandle, "Shape handle is stale");
    }
    return aSlot.shape;
  }

void ShapeArena::release(const std::uint32_t theHandle)
{
    get(theHandle);
    const std::uint32_t anIndex = (theHandle & kIndexMask) - 1;
    Slot& aSlot = mySlots[anIndex];
    const bool isRetired = aSlot.generation == kGenerationMask;
    if (!isRetired)
    {
      // Capacity is reserved when a slot is created, so this cannot allocate.
      myFree.push_back(anIndex);
    }
    const auto aScope = myScopes.find(aSlot.scope);
    if (aScope != myScopes.end())
    {
      aScope->second.erase(theHandle);
    }
    aSlot.shape.Nullify();
    aSlot.live = false;
    if (isRetired)
    {
      // Retire exhausted slots instead of wrapping the generation and making
      // an old u32 handle valid again. A retired slot is never put on myFree.
      aSlot.retired = true;
    }
    else
    {
      ++aSlot.generation;
    }
    --myLiveCount;
  }

void ShapeArena::endScope(const std::uint32_t theScope)
{
    const auto aScope = myScopes.find(theScope);
    if (aScope == myScopes.end())
    {
      throw KernelFailure(ErrorCode::InvalidArgs, "Unknown scopeId");
    }
    const std::vector<std::uint32_t> aHandles(aScope->second.begin(), aScope->second.end());
    for (const std::uint32_t aHandle : aHandles)
    {
      release(aHandle);
    }
    myScopes.erase(theScope);
  }

void ShapeArena::clear()
{
    for (Slot& aSlot : mySlots)
    {
      if (aSlot.live)
      {
        aSlot.shape.Nullify();
        aSlot.live = false;
        if (aSlot.generation == kGenerationMask)
        {
          aSlot.retired = true;
        }
        else
        {
          ++aSlot.generation;
        }
      }
    }
    myFree.clear();
    for (std::uint32_t anIndex = 0; anIndex < mySlots.size(); ++anIndex)
    {
      if (!mySlots[anIndex].retired)
      {
        myFree.push_back(anIndex);
      }
    }
    myScopes.clear();
    myLiveCount = 0;
  }

std::size_t ShapeArena::liveCount() const
{ return myLiveCount; }

std::uint64_t ShapeArena::checkpoint() const
{ return myCreationSequence; }

void ShapeArena::rollback(const std::uint64_t theCheckpoint)
{
    for (std::uint32_t anIndex = 0; anIndex < mySlots.size(); ++anIndex)
    {
      const Slot& aSlot = mySlots[anIndex];
      if (aSlot.live && aSlot.creationSequence > theCheckpoint)
      {
        release((aSlot.generation << kIndexBits) | (anIndex + 1));
      }
    }
  }

} // namespace occt_worker
