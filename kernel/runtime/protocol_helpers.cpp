#include "kernel_protocol_helpers.hpp"

namespace occt_worker {

std::uint32_t requiredU32(const json& theObject, const char* theKey)
{
  if (!theObject.is_object() || !theObject.contains(theKey)
      || (!theObject.at(theKey).is_number_unsigned() && !theObject.at(theKey).is_number_integer()))
  {
    throw KernelFailure(ErrorCode::InvalidArgs, std::string("Missing integer argument: ") + theKey);
  }
  const std::int64_t aValue = theObject.at(theKey).get<std::int64_t>();
  if (aValue < 0 || aValue > UINT32_MAX)
  {
    throw KernelFailure(ErrorCode::InvalidArgs, std::string("Argument is outside u32 range: ") + theKey);
  }
  return static_cast<std::uint32_t>(aValue);
}

double requiredNumber(const json& theObject, const char* theKey)
{
  if (!theObject.contains(theKey) || !theObject.at(theKey).is_number())
  {
    throw KernelFailure(ErrorCode::InvalidArgs, std::string("Missing numeric argument: ") + theKey);
  }
  return theObject.at(theKey).get<double>();
}

std::array<double, 3> requiredVec3(const json& theObject, const char* theKey)
{
  if (!theObject.contains(theKey) || !theObject.at(theKey).is_array()
      || theObject.at(theKey).size() != 3)
  {
    throw KernelFailure(ErrorCode::InvalidArgs, std::string("Expected a three-number array: ") + theKey);
  }
  std::array<double, 3> aValue{};
  for (std::size_t anIndex = 0; anIndex < 3; ++anIndex)
  {
    if (!theObject.at(theKey).at(anIndex).is_number())
    {
      throw KernelFailure(ErrorCode::InvalidArgs, std::string("Expected a three-number array: ") + theKey);
    }
    aValue[anIndex] = theObject.at(theKey).at(anIndex).get<double>();
  }
  return aValue;
}

std::array<double, 3> optionalVec3(const json& theObject,
                                   const char* theKey,
                                   const std::array<double, 3>& theDefault)
{
  return theObject.contains(theKey) ? requiredVec3(theObject, theKey) : theDefault;
}

std::array<double, 3> requiredDirection(const json& theObject, const char* theKey)
{
  const std::array<double, 3> aValue = requiredVec3(theObject, theKey);
  const double aSquaredNorm = aValue[0] * aValue[0] + aValue[1] * aValue[1] + aValue[2] * aValue[2];
  if (aSquaredNorm <= 1.0e-30)
    throw KernelFailure(ErrorCode::InvalidArgs, std::string("Direction must be non-zero: ") + theKey);
  return aValue;
}

std::array<double, 3> optionalDirection(const json& theObject,
                                        const char* theKey,
                                        const std::array<double, 3>& theDefault)
{
  return theObject.contains(theKey) ? requiredDirection(theObject, theKey) : theDefault;
}

json bufferDescriptor(const std::uint32_t theId,
                      const std::size_t theLength,
                      const char* theLayout)
{
  return {{"bufferId", theId}, {"byteLength", theLength}, {"layout", theLayout}};
}

std::string inputBufferData(BufferStore& theBuffers, const json& theArgs, const char* theKey)
{
  if (!theArgs.contains(theKey) || !theArgs.at(theKey).is_object())
  {
    throw KernelFailure(ErrorCode::InvalidArgs,
                        std::string("Missing buffer reference argument: ") + theKey);
  }
  const json& aRef = theArgs.at(theKey);
  std::vector<std::uint8_t>& aBuffer = theBuffers.get(requiredU32(aRef, "bufferId"));
  const std::size_t anOffset = aRef.value("byteOffset", 0u);
  const std::size_t aLength =
    aRef.value("byteLength", aBuffer.size() - std::min(anOffset, aBuffer.size()));
  if (anOffset > aBuffer.size() || aLength > aBuffer.size() - anOffset)
  {
    throw KernelFailure(ErrorCode::InvalidArgs, "Buffer range is out of bounds");
  }
  return std::string(reinterpret_cast<const char*>(aBuffer.data() + anOffset), aLength);
}

} // namespace occt_worker
