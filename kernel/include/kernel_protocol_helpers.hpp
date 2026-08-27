#pragma once
#include "kernel_support.hpp"

namespace occt_worker {

std::uint32_t requiredU32(const json& theObject, const char* theKey);
double requiredNumber(const json& theObject, const char* theKey);
std::array<double, 3> requiredVec3(const json& theObject, const char* theKey);
std::array<double, 3> optionalVec3(const json& theObject,
                                   const char* theKey,
                                   const std::array<double, 3>& theDefault);
std::array<double, 3> requiredDirection(const json& theObject, const char* theKey);
std::array<double, 3> optionalDirection(const json& theObject,
                                        const char* theKey,
                                        const std::array<double, 3>& theDefault);
json bufferDescriptor(std::uint32_t theId, std::size_t theLength, const char* theLayout);
std::string inputBufferData(BufferStore& theBuffers, const json& theArgs, const char* theKey);

} // namespace occt_worker
