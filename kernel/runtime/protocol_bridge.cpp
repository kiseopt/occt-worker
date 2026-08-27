// Protocol-facing helpers shared by the dispatch layer and operations.
// Bodies are byte-identical fragments of the original kernel/src/kernel.cpp.
//
// Also hosts the C ABI host bridge consumed by shared Side modules
// (see kernel/include/occt_plugin_abi.h): sides resolve these exports from
// Main at load time and never touch JS directly.

#include "kernel_support.hpp"

#include "occt_plugin_abi.h"

namespace occt_worker {

namespace {

std::string& sideResponseScratch()
{
  static std::string aScratch;
  return aScratch;
}

} // namespace

IGESUnitGuard::~IGESUnitGuard()
{
  Interface_Static::SetIVal("xstep.cascade.unit", myPreviousStatic);
  UnitsMethods::SetCasCadeLengthUnit(myPrevious);
}

void XmlXCAFStreamReader::read(Standard_IStream& theStream, const occ::handle<TDocStd_Document>& theDocument, const occ::handle<TDocStd_Application>& theApplication)
{
    try
    {
      myFileName = "memory.xml";
      Read(theStream,
           occ::handle<Storage_Data>(),
           theDocument,
           theApplication,
           occ::handle<PCDM_ReaderFilter>(),
           Message_ProgressRange());
    }
    catch (const Standard_Failure& theFailure)
    {
      throw Standard_Failure((std::string("XCAF XML stream parse failed: ")
                              + theFailure.what()).c_str());
    }
  }

} // namespace occt_worker

// The C host bridge exists only in shared Main builds; the standalone full
// artifact keeps its frozen export surface untouched.
#ifdef OCCT_WORKER_SHARED_MAIN
extern "C" {

// Dispatch one protocol operation against the resident Kernel instance.
// Returns response JSON length; response bytes live at occt_host_response_ptr().
EMSCRIPTEN_KEEPALIVE int occt_host_dispatch_v1(const char* theRequest, int theLength)
{
  occt_worker::sideResponseScratch() = occt_worker::dispatchProtocolRequest(
    theRequest, static_cast<std::size_t>(theLength));
  return static_cast<int>(occt_worker::sideResponseScratch().size());
}

EMSCRIPTEN_KEEPALIVE const char* occt_host_response_ptr()
{
  return occt_worker::sideResponseScratch().c_str();
}

} // extern "C"
#endif // OCCT_WORKER_SHARED_MAIN
