#pragma once
#include "generated.hpp"
#include "kernel_occt_includes.hpp"

extern "C"
{
__attribute__((import_module("env"), import_name("occt_worker_cancelled")))
int occt_worker_cancelled();

__attribute__((import_module("env"), import_name("occt_worker_progress")))
void occt_worker_progress(double theFraction);
}

namespace occt_worker {
using json = nlohmann::json;

std::string dispatchProtocolRequest(const char* theRequest, std::size_t theLength);

class HostProgressIndicator final : public Message_ProgressIndicator
{
protected:
  bool UserBreak() override
  {
    return occt_worker_cancelled() != 0;
  }

  void Show(const Message_ProgressScope&, const bool) override
  {
    occt_worker_progress(GetPosition());
  }
};

class KernelFailure : public std::runtime_error
{
public:
  KernelFailure(ErrorCode theCode, std::string theMessage)
      : std::runtime_error(std::move(theMessage)), code(theCode)
  {
  }

  ErrorCode code;
};

class CellsBuilder : public BOPAlgo_CellsBuilder
{
public:
  const NCollection_List<TopoDS_Shape>* origins(const TopoDS_Shape& theCell) const
  {
    return myIndex.Seek(theCell);
  }
};

const char* errorName(const ErrorCode theCode) ;

const char* exchangeFormatForOperation(const std::string_view theOperation) ;

class ShapeArena
{

public:

std::uint32_t beginScope() ;

std::uint32_t add(const TopoDS_Shape& theShape, const std::uint32_t theScope) ;

const TopoDS_Shape& get(const std::uint32_t theHandle) const ;

void release(const std::uint32_t theHandle) ;

void endScope(const std::uint32_t theScope) ;

void clear() ;

std::size_t liveCount() const ;

std::uint64_t checkpoint() const ;

void rollback(const std::uint64_t theCheckpoint) ;



private:

static constexpr std::uint32_t kIndexBits = 20;

  static constexpr std::uint32_t kIndexMask = (1u << kIndexBits) - 1;

  static constexpr std::uint32_t kGenerationMask = (1u << (32 - kIndexBits)) - 1;


  struct Slot
  {
    TopoDS_Shape shape;
    std::uint32_t generation = 1;
    std::uint32_t scope = 0;
    bool live = false;
    bool retired = false;
    std::uint64_t creationSequence = 0;
  }
;


  std::vector<Slot> mySlots;

  std::vector<std::uint32_t> myFree;

  std::unordered_map<std::uint32_t, std::unordered_set<std::uint32_t>> myScopes;

  std::uint32_t myNextScope = 1;

  std::size_t myLiveCount = 0;

  std::uint64_t myCreationSequence = 0;
};;

class BufferStore
{

public:

std::uint32_t create(const std::size_t theSize) ;



  template <typename T>
  std::uint32_t copy(const std::vector<T>& theValues)
  {
    const std::size_t aSize = theValues.size() * sizeof(T);
    const std::uint32_t anId = create(aSize);
    if (aSize != 0)
    {
      std::memcpy(myBuffers.at(anId).bytes.data(), theValues.data(), aSize);
    }
    return anId;
  }
std::vector<std::uint8_t>& get(const std::uint32_t theId) ;

void release(const std::uint32_t theId) ;

void clear() ;

std::size_t liveBytes() const ;

std::uint64_t checkpoint() const ;

void rollback(const std::uint64_t theCheckpoint) ;



private:

struct Entry
  {
    std::vector<std::uint8_t> bytes;
    std::uint64_t creationSequence;
  }
;


  std::unordered_map<std::uint32_t, Entry> myBuffers;

  std::uint32_t myNextId = 1;

  std::size_t myLiveBytes = 0;

  std::uint64_t myCreationSequence = 0;
};;

class IGESUnitGuard
{
public:
  IGESUnitGuard(const double theUnit, const UnitsMethods_LengthUnit theUnitEnum)
      : myPrevious(UnitsMethods::GetCasCadeLengthUnit()),
        myPreviousStatic(Interface_Static::IVal("xstep.cascade.unit"))
  {
    Interface_Static::SetIVal("xstep.cascade.unit", static_cast<int>(theUnitEnum));
    UnitsMethods::SetCasCadeLengthUnit(theUnit);
  }

  ~IGESUnitGuard() ;

  IGESUnitGuard(const IGESUnitGuard&) = delete;
  IGESUnitGuard& operator=(const IGESUnitGuard&) = delete;

private:
  double myPrevious;
  int myPreviousStatic;
};

class XmlXCAFStreamReader : public XmlXCAFDrivers_DocumentRetrievalDriver
{

public:

void read(Standard_IStream& theStream, const occ::handle<TDocStd_Document>& theDocument, const occ::handle<TDocStd_Application>& theApplication) ;

};;


// ---------------------------------------------------------------------------
// Operation registry: maps protocol operation names to context-based handlers.
// Registration runs once per process (see runtime/operation_registry.cpp).
// ---------------------------------------------------------------------------

class KernelOperationContext;
using OperationHandler = json (*)(KernelOperationContext&, const json&);

class OperationRegistry
{
public:
  static OperationRegistry& instance();
  void add(const char* theOperation, OperationHandler theHandler);
  OperationHandler find(const std::string& theOperation) const;
  std::vector<std::string> names() const;
  void beginRegistration();
  bool commitRegistration(const char* const* theOperations, std::size_t theCount);
  void rollbackRegistration();

private:
  OperationRegistry() = default;
  std::unordered_map<std::string, OperationHandler> myHandlers;
  std::vector<std::string> myPendingOperations;
  bool myRegistrationActive = false;
};

void register_runtime_operations();
void register_geometry_topology_operations();
void register_modeling_operations();
void register_algorithms_operations();
void register_mesh_operations();
void register_exchange_mesh_operations();
void register_exchange_cad_operations();

} // namespace occt_worker
