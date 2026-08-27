// Kernel byte-buffer store: explicit protocol buffers + checkpoints.
// Bodies are byte-identical fragments of the original kernel/src/kernel.cpp.

#include "kernel_support.hpp"

namespace occt_worker {

std::uint32_t BufferStore::create(const std::size_t theSize)
{
    const std::uint32_t anId = myNextId++;
    myBuffers.emplace(anId, Entry{std::vector<std::uint8_t>(theSize), ++myCreationSequence});
    myLiveBytes += theSize;
    return anId;
  }

std::vector<std::uint8_t>& BufferStore::get(const std::uint32_t theId)
{
    const auto aBuffer = myBuffers.find(theId);
    if (aBuffer == myBuffers.end())
    {
      throw KernelFailure(ErrorCode::InvalidArgs, "Unknown bufferId");
    }
    return aBuffer->second.bytes;
  }

void BufferStore::release(const std::uint32_t theId)
{
    const auto aBuffer = myBuffers.find(theId);
    if (aBuffer == myBuffers.end())
    {
      throw KernelFailure(ErrorCode::InvalidArgs, "Unknown bufferId");
    }
    myLiveBytes -= aBuffer->second.bytes.size();
    myBuffers.erase(aBuffer);
  }

void BufferStore::clear()
{
    myBuffers.clear();
    myLiveBytes = 0;
  }

std::size_t BufferStore::liveBytes() const
{ return myLiveBytes; }

std::uint64_t BufferStore::checkpoint() const
{ return myCreationSequence; }

void BufferStore::rollback(const std::uint64_t theCheckpoint)
{
    for (auto anEntry = myBuffers.begin(); anEntry != myBuffers.end();)
    {
      if (anEntry->second.creationSequence <= theCheckpoint)
      {
        ++anEntry;
        continue;
      }
      myLiveBytes -= anEntry->second.bytes.size();
      anEntry = myBuffers.erase(anEntry);
    }
  }

// Explicit instantiations for element types referenced from shared Side
// modules (the standalone TU instantiates them at its own use sites).
template std::uint32_t BufferStore::copy<float>(const std::vector<float>&);
template std::uint32_t BufferStore::copy<std::uint8_t>(const std::vector<std::uint8_t>&);
template std::uint32_t BufferStore::copy<std::uint32_t>(const std::vector<std::uint32_t>&);
template std::uint32_t BufferStore::copy<std::array<unsigned char, 4>>(const std::vector<std::array<unsigned char, 4>>&);

} // namespace occt_worker
