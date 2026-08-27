#include "kernel_context.hpp"

namespace occt_worker {

std::string dispatchProtocolRequest(const char* theRequest, const std::size_t theLength)
{
  std::int64_t anId = -1;
  std::string anOperation;
  try
  {
    const json aRequest = json::parse(theRequest, theRequest + theLength);
    if (!aRequest.is_object() || !aRequest.contains("id") || !aRequest.at("id").is_number_integer()
        || !aRequest.contains("op") || !aRequest.at("op").is_string()
        || !aRequest.contains("args") || !aRequest.at("args").is_object())
      throw KernelFailure(ErrorCode::ProtocolError, "Request must contain integer id, string op, and object args");
    anId = aRequest.at("id").get<std::int64_t>();
    anOperation = aRequest.at("op").get<std::string>();
    return json{{"id", anId}, {"ok", true},
                {"result", THE_KERNEL.dispatch(anOperation, aRequest.at("args"))}}.dump();
  }
  catch (const KernelFailure& theFailure)
  {
    json anError{{"code", errorName(theFailure.code)}, {"message", theFailure.what()}};
    if (!anOperation.empty())
    {
      anError["details"] = {{"operation", anOperation}};
      if (const char* aFormat = exchangeFormatForOperation(anOperation))
        anError["details"]["format"] = aFormat;
    }
    return json{{"id", anId}, {"ok", false}, {"error", anError}}.dump();
  }
  catch (const std::bad_alloc&)
  {
    return json{{"id", anId}, {"ok", false},
                {"error", {{"code", "OutOfMemory"}, {"message", "Linear memory allocation failed"}}}}.dump();
  }
  catch (const Standard_Failure& theFailure)
  {
    return json{{"id", anId}, {"ok", false},
                {"error", {{"code", "KernelError"}, {"message", theFailure.what()}}}}.dump();
  }
  catch (const std::exception& theFailure)
  {
    return json{{"id", anId}, {"ok", false},
                {"error", {{"code", "ProtocolError"}, {"message", theFailure.what()}}}}.dump();
  }
  catch (...)
  {
    return R"({"id":-1,"ok":false,"error":{"code":"KernelError","message":"Unknown kernel failure"}})";
  }
}

const char* errorName(const ErrorCode theCode)
{
  switch (theCode)
  {
    case ErrorCode::ConstructionFailed: return "ConstructionFailed";
    case ErrorCode::BooleanFailed: return "BooleanFailed";
    case ErrorCode::FilletFailed: return "FilletFailed";
    case ErrorCode::TessellationFailed: return "TessellationFailed";
    case ErrorCode::ImportExportFailed: return "ImportExportFailed";
    case ErrorCode::HealingFailed: return "HealingFailed";
    case ErrorCode::Cancelled: return "Cancelled";
    case ErrorCode::InvalidHandle: return "InvalidHandle";
    case ErrorCode::InvalidArgs: return "InvalidArgs";
    case ErrorCode::OutOfMemory: return "OutOfMemory";
    case ErrorCode::KernelError: return "KernelError";
    case ErrorCode::ProtocolError: return "ProtocolError";
  }
  return "KernelError";
}

const char* exchangeFormatForOperation(const std::string_view theOperation)
{
  if (theOperation.find("XCAF") != std::string_view::npos) return "xcaf";
  if (theOperation.find("STEP") != std::string_view::npos) return "step";
  if (theOperation.find("IGES") != std::string_view::npos) return "iges";
  if (theOperation.find("STL") != std::string_view::npos) return "stl";
  if (theOperation.find("OBJ") != std::string_view::npos) return "obj";
  if (theOperation.find("PLY") != std::string_view::npos) return "ply";
  if (theOperation.find("GLTF") != std::string_view::npos) return "gltf";
  if (theOperation.find("VRML") != std::string_view::npos) return "vrml";
  if (theOperation.find("BREP") != std::string_view::npos) return "brep";
  return nullptr;
}

} // namespace occt_worker
