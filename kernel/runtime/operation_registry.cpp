// Operation registry singleton backing Kernel::dispatchOperation.

#include "kernel_support.hpp"

namespace occt_worker {

OperationRegistry& OperationRegistry::instance()
{
  static OperationRegistry aRegistry;
  return aRegistry;
}

void OperationRegistry::add(const char* theOperation, OperationHandler theHandler)
{
  const auto [anEntry, isInserted] = myHandlers.emplace(theOperation, theHandler);
  if (!isInserted)
  {
    throw KernelFailure(ErrorCode::ProtocolError, std::string("duplicate operation registration: ") + theOperation);
  }
  if (myRegistrationActive)
  {
    try
    {
      myPendingOperations.emplace_back(theOperation);
    }
    catch (...)
    {
      myHandlers.erase(anEntry);
      throw;
    }
  }
}

OperationHandler OperationRegistry::find(const std::string& theOperation) const
{
  const auto anEntry = myHandlers.find(theOperation);
  return anEntry == myHandlers.end() ? nullptr : anEntry->second;
}

std::vector<std::string> OperationRegistry::names() const
{
  std::vector<std::string> aNames;
  aNames.reserve(myHandlers.size());
  for (const auto& anEntry : myHandlers) aNames.push_back(anEntry.first);
  std::sort(aNames.begin(), aNames.end());
  return aNames;
}

void OperationRegistry::beginRegistration()
{
  if (myRegistrationActive)
    throw KernelFailure(ErrorCode::ProtocolError, "nested operation registration");
  myRegistrationActive = true;
  myPendingOperations.clear();
}

bool OperationRegistry::commitRegistration(const char* const* theOperations,
                                            const std::size_t theCount)
{
  if (!myRegistrationActive)
    throw KernelFailure(ErrorCode::ProtocolError, "no operation registration is active");

  std::unordered_set<std::string> anExpected;
  bool isValid = theOperations != nullptr && theCount == myPendingOperations.size();
  for (std::size_t anIndex = 0; isValid && anIndex < theCount; ++anIndex)
  {
    if (theOperations[anIndex] == nullptr || !anExpected.emplace(theOperations[anIndex]).second)
      isValid = false;
  }
  if (isValid)
  {
    for (const std::string& anOperation : myPendingOperations)
      if (anExpected.find(anOperation) == anExpected.end()) isValid = false;
  }

  if (!isValid)
  {
    rollbackRegistration();
    return false;
  }
  myPendingOperations.clear();
  myRegistrationActive = false;
  return true;
}

void OperationRegistry::rollbackRegistration()
{
  if (!myRegistrationActive) return;
  for (const std::string& anOperation : myPendingOperations) myHandlers.erase(anOperation);
  myPendingOperations.clear();
  myRegistrationActive = false;
}

} // namespace occt_worker
