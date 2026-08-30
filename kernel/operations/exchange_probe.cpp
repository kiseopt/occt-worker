// Focused CAD exchange handlers.

#include "kernel_step_shape_exchange_operations.hpp"
#include "kernel_operation_context.hpp"
#include "kernel_protocol_helpers.hpp"

namespace occt_worker {

json h_probeFormat(KernelOperationContext& theContext, const json& theArgs)
{
      const std::string aData = inputBufferData(theContext.buffers(), theArgs, "data");
      if (aData.size() >= 7 && std::memcmp(aData.data(), "BINFILE", 7) == 0)
        return {{"format", "xcaf"}, {"encoding", "binary"}, {"confidence", "exact"}};
      if (aData.size() >= 4 && std::memcmp(aData.data(), "glTF", 4) == 0)
        return {{"format", "gltf"}, {"encoding", "glb"}, {"confidence", "exact"}};
      if (aData.size() >= 84)
      {
        const auto* aBytes = reinterpret_cast<const unsigned char*>(aData.data());
        const std::uint32_t aTriangleCount = static_cast<std::uint32_t>(aBytes[80])
          | (static_cast<std::uint32_t>(aBytes[81]) << 8)
          | (static_cast<std::uint32_t>(aBytes[82]) << 16)
          | (static_cast<std::uint32_t>(aBytes[83]) << 24);
        if (aTriangleCount <= (std::numeric_limits<std::size_t>::max() - 84) / 50
            && 84 + static_cast<std::size_t>(aTriangleCount) * 50 == aData.size())
          return {{"format", "stl"}, {"encoding", "binary"}, {"confidence", "exact"}};
      }

      std::size_t aStart = 0;
      if (aData.size() >= 3 && static_cast<unsigned char>(aData[0]) == 0xEF
          && static_cast<unsigned char>(aData[1]) == 0xBB
          && static_cast<unsigned char>(aData[2]) == 0xBF)
        aStart = 3;
      while (aStart < aData.size()
             && (aData[aStart] == ' ' || aData[aStart] == '\t'
                 || aData[aStart] == '\r' || aData[aStart] == '\n'))
        ++aStart;
      const std::string_view aView(aData.data() + aStart, aData.size() - aStart);
      if (aView.rfind("<?xml", 0) == 0 && aView.find("format=\"XmlXCAF\"") != std::string_view::npos)
        return {{"format", "xcaf"}, {"encoding", "text"}, {"confidence", "exact"}};
      if (aView.rfind("ply\n", 0) == 0 || aView.rfind("ply\r\n", 0) == 0)
      {
        const std::size_t anEndHeader = aView.find("end_header");
        const std::string_view aHeader = aView.substr(0, anEndHeader);
        const bool isBinary = aHeader.find("format binary_") != std::string_view::npos;
        return {{"format", "ply"}, {"encoding", isBinary ? "binary" : "text"},
                {"confidence", "exact"}};
      }
      if (aView.rfind("#VRML V1.0", 0) == 0 || aView.rfind("#VRML V2.0", 0) == 0)
        return {{"format", "vrml"}, {"encoding", "text"}, {"confidence", "exact"}};
      if (aView.rfind("ISO-10303-21;", 0) == 0 && aView.find("HEADER;") != std::string_view::npos)
      {
        const bool hasDocumentMetadata =
          aView.find("NEXT_ASSEMBLY_USAGE_OCCURRENCE") != std::string_view::npos
          || aView.find("COLOUR_RGB") != std::string_view::npos
          || aView.find("DRAUGHTING_PRE_DEFINED_COLOUR") != std::string_view::npos
          || aView.find("PRESENTATION_LAYER_ASSIGNMENT") != std::string_view::npos
          || aView.find("INVISIBILITY") != std::string_view::npos;
        return {{"format", "step"}, {"encoding", "text"}, {"confidence", "exact"},
                {"documentMetadata", hasDocumentMetadata}};
      }
      if (aView.rfind("DBRep_DrawableShape", 0) == 0
          || aView.rfind("CASCADE Topology V", 0) == 0)
        return {{"format", "brep"}, {"encoding", "text"}, {"confidence", "exact"}};
      if (aData.size() >= 80 && aData[72] == 'S')
      {
        bool hasIgesSequence = true;
        for (std::size_t anIndex = 73; anIndex < 80; ++anIndex)
          hasIgesSequence = hasIgesSequence
            && (aData[anIndex] == ' ' || (aData[anIndex] >= '0' && aData[anIndex] <= '9'));
        if (hasIgesSequence)
          return {{"format", "iges"}, {"encoding", "text"}, {"confidence", "likely"}};
      }
      if (aView.rfind("solid", 0) == 0 && aView.find("facet normal") != std::string_view::npos
          && aView.find("endsolid") != std::string_view::npos)
        return {{"format", "stl"}, {"encoding", "text"}, {"confidence", "likely"}};

      const json aJson = !aView.empty() && aView.front() == '{'
        ? json::parse(aView.begin(), aView.end(), nullptr, false)
        : json(json::value_t::discarded);
      if (!aJson.is_discarded() && aJson.is_object() && aJson.contains("asset")
          && aJson.at("asset").is_object() && aJson.at("asset").contains("version")
          && aJson.at("asset").at("version").is_string()
          && aJson.at("asset").at("version").get<std::string>().rfind("2.", 0) == 0)
        return {{"format", "gltf"}, {"encoding", "json"}, {"confidence", "exact"}};

      bool hasObjVertex = false;
      bool hasObjFace = false;
      std::istringstream aStream{std::string(aView)};
      std::string aLine;
      while (std::getline(aStream, aLine) && !(hasObjVertex && hasObjFace))
      {
        std::istringstream aLineStream(aLine);
        std::string aCommand;
        aLineStream >> aCommand;
        hasObjVertex = hasObjVertex || aCommand == "v";
        hasObjFace = hasObjFace || aCommand == "f";
      }
      if (hasObjVertex && hasObjFace)
        return {{"format", "obj"}, {"encoding", "text"}, {"confidence", "likely"}};
      return {{"format", "unknown"}, {"confidence", "none"}};
    }

} // namespace occt_worker
