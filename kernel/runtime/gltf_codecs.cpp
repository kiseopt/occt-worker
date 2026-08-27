#include "kernel_exchange_helpers.hpp"

namespace occt_worker {

int base64Value(const char theCharacter)
  {
    if (theCharacter >= 'A' && theCharacter <= 'Z') return theCharacter - 'A';
    if (theCharacter >= 'a' && theCharacter <= 'z') return theCharacter - 'a' + 26;
    if (theCharacter >= '0' && theCharacter <= '9') return theCharacter - '0' + 52;
    if (theCharacter == '+') return 62;
    if (theCharacter == '/') return 63;
    return -1;
  }

std::string encodeBase64(const std::vector<std::uint8_t>& theData)
  {
    static constexpr char aAlphabet[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string aResult;
    aResult.reserve(((theData.size() + 2) / 3) * 4);
    for (std::size_t anOffset = 0; anOffset < theData.size(); anOffset += 3)
    {
      const std::uint32_t aValue = static_cast<std::uint32_t>(theData[anOffset]) << 16
        | (anOffset + 1 < theData.size()
             ? static_cast<std::uint32_t>(theData[anOffset + 1]) << 8
             : 0)
        | (anOffset + 2 < theData.size()
             ? static_cast<std::uint32_t>(theData[anOffset + 2])
             : 0);
      aResult.push_back(aAlphabet[(aValue >> 18) & 63]);
      aResult.push_back(aAlphabet[(aValue >> 12) & 63]);
      aResult.push_back(anOffset + 1 < theData.size() ? aAlphabet[(aValue >> 6) & 63] : '=');
      aResult.push_back(anOffset + 2 < theData.size() ? aAlphabet[aValue & 63] : '=');
    }
    return aResult;
  }

std::vector<std::uint8_t> decodeBase64(const std::string& theText)
  {
    std::string aText;
    aText.reserve(theText.size());
    for (const char aCharacter : theText)
    {
      if (aCharacter != ' ' && aCharacter != '\t' && aCharacter != '\r' && aCharacter != '\n')
      {
        aText.push_back(aCharacter);
      }
    }
    if (aText.size() % 4 != 0)
    {
      throw KernelFailure(ErrorCode::ImportExportFailed, "glTF buffer has invalid base64 data");
    }

    std::vector<std::uint8_t> aResult;
    aResult.reserve(aText.size() / 4 * 3);
    for (std::size_t anOffset = 0; anOffset < aText.size(); anOffset += 4)
    {
      const bool hasThird = aText[anOffset + 2] != '=';
      const bool hasFourth = aText[anOffset + 3] != '=';
      const int a = base64Value(aText[anOffset]);
      const int b = base64Value(aText[anOffset + 1]);
      const int c = hasThird ? base64Value(aText[anOffset + 2]) : 0;
      const int d = hasFourth ? base64Value(aText[anOffset + 3]) : 0;
      const bool isLast = anOffset + 4 == aText.size();
      if (a < 0 || b < 0 || c < 0 || d < 0 || (!hasThird && hasFourth)
          || ((!hasThird || !hasFourth) && !isLast))
      {
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF buffer has invalid base64 data");
      }
      const std::uint32_t aValue = static_cast<std::uint32_t>(a) << 18
        | static_cast<std::uint32_t>(b) << 12
        | static_cast<std::uint32_t>(c) << 6
        | static_cast<std::uint32_t>(d);
      aResult.push_back(static_cast<std::uint8_t>(aValue >> 16));
      if (hasThird) aResult.push_back(static_cast<std::uint8_t>(aValue >> 8));
      if (hasFourth) aResult.push_back(static_cast<std::uint8_t>(aValue));
    }
    return aResult;
  }

std::vector<std::uint8_t> decodeGltfDataUri(const std::string& theUri)
  {
    const std::size_t aComma = theUri.find(',');
    if (theUri.rfind("data:", 0) != 0 || aComma == std::string::npos)
      throw KernelFailure(ErrorCode::ImportExportFailed, "glTF data URI is invalid");
    const std::string aHeader = theUri.substr(5, aComma - 5);
    const std::string aPayload = theUri.substr(aComma + 1);
    if (aHeader.size() >= 7 && aHeader.compare(aHeader.size() - 7, 7, ";base64") == 0)
      return decodeBase64(aPayload);

    const auto hexValue = [](const char theCharacter) {
      if (theCharacter >= '0' && theCharacter <= '9') return theCharacter - '0';
      if (theCharacter >= 'a' && theCharacter <= 'f') return theCharacter - 'a' + 10;
      if (theCharacter >= 'A' && theCharacter <= 'F') return theCharacter - 'A' + 10;
      return -1;
    };
    std::vector<std::uint8_t> aResult;
    aResult.reserve(aPayload.size());
    for (std::size_t anIndex = 0; anIndex < aPayload.size(); ++anIndex)
    {
      if (aPayload[anIndex] != '%')
      {
        aResult.push_back(static_cast<std::uint8_t>(aPayload[anIndex]));
        continue;
      }
      if (anIndex + 2 >= aPayload.size())
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF data URI has invalid percent encoding");
      const int aHigh = hexValue(aPayload[anIndex + 1]);
      const int aLow = hexValue(aPayload[anIndex + 2]);
      if (aHigh < 0 || aLow < 0)
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF data URI has invalid percent encoding");
      aResult.push_back(static_cast<std::uint8_t>((aHigh << 4) | aLow));
      anIndex += 2;
    }
    return aResult;
  }

void appendU32LE(std::vector<std::uint8_t>& theData, const std::uint32_t theValue)
  {
    theData.push_back(static_cast<std::uint8_t>(theValue));
    theData.push_back(static_cast<std::uint8_t>(theValue >> 8));
    theData.push_back(static_cast<std::uint8_t>(theValue >> 16));
    theData.push_back(static_cast<std::uint8_t>(theValue >> 24));
  }

std::uint32_t readU32LE(const std::string& theData, const std::size_t theOffset)
  {
    if (theOffset > theData.size() || theData.size() - theOffset < 4)
    {
      throw KernelFailure(ErrorCode::ImportExportFailed, "GLB data is truncated");
    }
    const auto* aData = reinterpret_cast<const std::uint8_t*>(theData.data() + theOffset);
    return static_cast<std::uint32_t>(aData[0])
      | static_cast<std::uint32_t>(aData[1]) << 8
      | static_cast<std::uint32_t>(aData[2]) << 16
      | static_cast<std::uint32_t>(aData[3]) << 24;
  }

void appendFloat32LE(std::vector<std::uint8_t>& theData, const float theValue)
  {
    std::uint32_t aBits = 0;
    std::memcpy(&aBits, &theValue, sizeof(float));
    appendU32LE(theData, aBits);
  }

std::vector<std::uint8_t> writeGLTF(const TriangleMesh& theMesh, const bool theBinary)
  {
    if (theMesh.positions.size() > std::numeric_limits<std::uint32_t>::max()
        || theMesh.indices.size() > std::numeric_limits<std::uint32_t>::max())
    {
      throw KernelFailure(ErrorCode::ImportExportFailed, "glTF mesh is too large");
    }

    std::vector<std::uint8_t> aBinary;
    aBinary.reserve(theMesh.positions.size() * (12 + 12 + 8) + theMesh.indices.size() * 4 + 12);
    std::array<double, 3> aMinimum = {
      std::numeric_limits<double>::infinity(),
      std::numeric_limits<double>::infinity(),
      std::numeric_limits<double>::infinity()};
    std::array<double, 3> aMaximum = {
      -std::numeric_limits<double>::infinity(),
      -std::numeric_limits<double>::infinity(),
      -std::numeric_limits<double>::infinity()};
    for (const gp_Pnt& aPoint : theMesh.positions)
    {
      const std::array<double, 3> aGltf = {aPoint.X(), aPoint.Z(), -aPoint.Y()};
      for (std::size_t anAxis = 0; anAxis < 3; ++anAxis)
      {
        aMinimum[anAxis] = std::min(aMinimum[anAxis], aGltf[anAxis]);
        aMaximum[anAxis] = std::max(aMaximum[anAxis], aGltf[anAxis]);
        appendFloat32LE(aBinary, static_cast<float>(aGltf[anAxis]));
      }
    }
    while (aBinary.size() % 4 != 0) aBinary.push_back(0);
    const std::size_t aNormalOffset = aBinary.size();
    for (const gp_Dir& aNormal : theMesh.normals)
    {
      appendFloat32LE(aBinary, static_cast<float>(aNormal.X()));
      appendFloat32LE(aBinary, static_cast<float>(aNormal.Z()));
      appendFloat32LE(aBinary, static_cast<float>(-aNormal.Y()));
    }
    while (aBinary.size() % 4 != 0) aBinary.push_back(0);
    const std::size_t anUvOffset = aBinary.size();
    for (const gp_Pnt2d& aUV : theMesh.uvs)
    {
      appendFloat32LE(aBinary, static_cast<float>(aUV.X()));
      appendFloat32LE(aBinary, static_cast<float>(aUV.Y()));
    }
    while (aBinary.size() % 4 != 0) aBinary.push_back(0);
    const std::size_t anIndexOffset = aBinary.size();
    for (const std::uint32_t anIndex : theMesh.indices)
    {
      appendU32LE(aBinary, anIndex);
    }

    json aDocument = {
      {"asset", {{"generator", "occt-worker"}, {"version", "2.0"}}},
      {"scene", 0},
      {"scenes", json::array({{{"nodes", json::array({0})}}})},
      {"nodes", json::array({{{"mesh", 0}}})},
      {"meshes", json::array({{{"primitives", json::array({
        {{"attributes", {{"POSITION", 0}, {"NORMAL", 1}, {"TEXCOORD_0", 2}}}, {"indices", 3}, {"mode", 4}}
      })}}})},
      {"bufferViews", json::array({
        {{"buffer", 0}, {"byteLength", theMesh.positions.size() * 12}, {"byteOffset", 0}, {"target", 34962}},
        {{"buffer", 0}, {"byteLength", theMesh.normals.size() * 12},
         {"byteOffset", aNormalOffset}, {"target", 34962}},
        {{"buffer", 0}, {"byteLength", theMesh.uvs.size() * 8},
         {"byteOffset", anUvOffset}, {"target", 34962}},
        {{"buffer", 0}, {"byteLength", theMesh.indices.size() * 4},
         {"byteOffset", anIndexOffset}, {"target", 34963}}
      })},
      {"accessors", json::array({
        {{"bufferView", 0}, {"componentType", 5126},
         {"count", theMesh.positions.size()}, {"type", "VEC3"},
         {"min", aMinimum}, {"max", aMaximum}},
        {{"bufferView", 1}, {"componentType", 5126},
         {"count", theMesh.normals.size()}, {"type", "VEC3"}},
        {{"bufferView", 2}, {"componentType", 5126},
         {"count", theMesh.uvs.size()}, {"type", "VEC2"}},
        {{"bufferView", 3}, {"componentType", 5125},
         {"count", theMesh.indices.size()}, {"type", "SCALAR"}}
      })}
    };
    if (theBinary)
    {
      aDocument["buffers"] = json::array({{{"byteLength", aBinary.size()}}});
      std::string aJson = aDocument.dump();
      while (aJson.size() % 4 != 0) aJson.push_back(' ');
      while (aBinary.size() % 4 != 0) aBinary.push_back(0);
      const std::uint64_t aTotal = 12ull + 8ull + aJson.size() + 8ull + aBinary.size();
      if (aTotal > std::numeric_limits<std::uint32_t>::max())
      {
        throw KernelFailure(ErrorCode::ImportExportFailed, "GLB output is too large");
      }
      std::vector<std::uint8_t> aGlb;
      aGlb.reserve(static_cast<std::size_t>(aTotal));
      appendU32LE(aGlb, 0x46546c67);
      appendU32LE(aGlb, 2);
      appendU32LE(aGlb, static_cast<std::uint32_t>(aTotal));
      appendU32LE(aGlb, static_cast<std::uint32_t>(aJson.size()));
      appendU32LE(aGlb, 0x4e4f534a);
      aGlb.insert(aGlb.end(), aJson.begin(), aJson.end());
      appendU32LE(aGlb, static_cast<std::uint32_t>(aBinary.size()));
      appendU32LE(aGlb, 0x004e4942);
      aGlb.insert(aGlb.end(), aBinary.begin(), aBinary.end());
      return aGlb;
    }

    aDocument["buffers"] = json::array({{
      {"byteLength", aBinary.size()},
      {"uri", "data:application/octet-stream;base64," + encodeBase64(aBinary)}
    }});
    const std::string aJson = aDocument.dump();
    return std::vector<std::uint8_t>(aJson.begin(), aJson.end());
  }

GltfDocumentExport writeGLTFDocument(
    const json& theSourceDocument,
    const std::vector<std::pair<std::string, std::vector<std::uint8_t>>>& theBuffers,
    const std::vector<std::pair<std::string, std::vector<std::uint8_t>>>& theResources,
    const bool theBinary)
  {
    if (!theSourceDocument.is_object()
        || !theSourceDocument.contains("asset")
        || !theSourceDocument.at("asset").is_object()
        || theSourceDocument.at("asset").value("version", "") != "2.0")
      throw KernelFailure(ErrorCode::InvalidArgs, "glTF document must be a glTF 2.0 object");
    json aDocument = theSourceDocument;
    static const json anEmptyArray = json::array();
    const json& aBufferDefinitions = aDocument.contains("buffers")
      ? aDocument.at("buffers") : anEmptyArray;
    if (!aBufferDefinitions.is_array() || aBufferDefinitions.size() != theBuffers.size())
      throw KernelFailure(ErrorCode::InvalidArgs,
                          "glTF document buffers must match supplied buffers");
    for (std::size_t aBufferIndex = 0; aBufferIndex < theBuffers.size(); ++aBufferIndex)
    {
      const json& aDefinition = aBufferDefinitions.at(aBufferIndex);
      if (!aDefinition.is_object() || !aDefinition.contains("byteLength")
          || !aDefinition.at("byteLength").is_number_unsigned()
          || aDefinition.at("byteLength").get<std::size_t>() != theBuffers[aBufferIndex].second.size())
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "glTF document buffer byteLength must match supplied data");
    }

    GltfDocumentExport aResult;
    std::unordered_set<std::string> aResourceUris;
    const auto appendResource = [&](const std::string& theUri,
                                    const std::vector<std::uint8_t>& theData) {
      if (theUri.empty() || theUri.rfind("data:", 0) == 0) return;
      if (!aResourceUris.insert(theUri).second)
      {
        const auto anExisting = std::find_if(
          aResult.resources.begin(), aResult.resources.end(),
          [&](const auto& theResource) { return theResource.first == theUri; });
        if (anExisting != aResult.resources.end() && anExisting->second == theData) return;
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "glTF export resource URI has conflicting data");
      }
      aResult.resources.emplace_back(theUri, theData);
    };

    std::vector<std::uint8_t> aBinary;
    if (theBinary)
    {
      std::vector<std::size_t> aBufferOffsets(theBuffers.size(), 0);
      for (std::size_t aBufferIndex = 0; aBufferIndex < theBuffers.size(); ++aBufferIndex)
      {
        while (aBinary.size() % 4 != 0) aBinary.push_back(0);
        aBufferOffsets[aBufferIndex] = aBinary.size();
        if (theBuffers[aBufferIndex].second.size()
            > std::numeric_limits<std::size_t>::max() - aBinary.size())
          throw KernelFailure(ErrorCode::InvalidArgs, "glTF document buffers are too large");
        aBinary.insert(aBinary.end(), theBuffers[aBufferIndex].second.begin(),
                       theBuffers[aBufferIndex].second.end());
      }
      if (aDocument.contains("bufferViews"))
      {
        if (!aDocument.at("bufferViews").is_array())
          throw KernelFailure(ErrorCode::InvalidArgs, "glTF document bufferViews must be an array");
        for (json& aView : aDocument["bufferViews"])
        {
          if (!aView.is_object() || !aView.contains("buffer")
              || !aView.at("buffer").is_number_unsigned())
            throw KernelFailure(ErrorCode::InvalidArgs, "glTF document bufferView is invalid");
          const std::size_t aBufferIndex = aView.at("buffer").get<std::size_t>();
          if (aBufferIndex >= aBufferOffsets.size())
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "glTF document bufferView buffer is out of range");
          if (!aView.contains("byteLength") || !aView.at("byteLength").is_number_unsigned())
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "glTF document bufferView byteLength is invalid");
          const std::size_t anOldOffset = aView.value("byteOffset", std::size_t(0));
          const std::size_t aViewLength = aView.at("byteLength").get<std::size_t>();
          if (anOldOffset > theBuffers[aBufferIndex].second.size()
              || aViewLength > theBuffers[aBufferIndex].second.size() - anOldOffset)
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "glTF document bufferView exceeds supplied buffer data");
          if (anOldOffset > std::numeric_limits<std::size_t>::max() - aBufferOffsets[aBufferIndex])
            throw KernelFailure(ErrorCode::InvalidArgs, "glTF document bufferView offset is too large");
          aView["buffer"] = 0;
          const std::size_t aNewOffset = aBufferOffsets[aBufferIndex] + anOldOffset;
          if (aNewOffset == 0) aView.erase("byteOffset");
          else aView["byteOffset"] = aNewOffset;
        }
      }
      if (aBinary.empty()) aDocument.erase("buffers");
      else
      {
        json aBufferDefinition = theBuffers.size() == 1
          ? aBufferDefinitions.at(0) : json::object();
        aBufferDefinition.erase("uri");
        aBufferDefinition["byteLength"] = aBinary.size();
        aDocument["buffers"] = json::array({std::move(aBufferDefinition)});
      }
    }
    else
    {
      json aDefinitions = json::array();
      for (std::size_t aBufferIndex = 0; aBufferIndex < theBuffers.size(); ++aBufferIndex)
      {
        json aDefinition = aBufferDefinitions.at(aBufferIndex);
        std::string aUri = theBuffers[aBufferIndex].first;
        if (aUri.empty() && aDefinition.contains("uri") && aDefinition.at("uri").is_string())
          aUri = aDefinition.at("uri").get<std::string>();
        if (aUri.rfind("data:", 0) == 0)
        {
          aDefinition["uri"] = "data:application/octet-stream;base64,"
            + encodeBase64(theBuffers[aBufferIndex].second);
        }
        else
        {
          if (aUri.empty()) aUri = "buffer" + std::to_string(aBufferIndex) + ".bin";
          aDefinition["uri"] = aUri;
          appendResource(aUri, theBuffers[aBufferIndex].second);
        }
        aDefinitions.push_back(std::move(aDefinition));
      }
      if (aDefinitions.empty()) aDocument.erase("buffers");
      else aDocument["buffers"] = std::move(aDefinitions);
    }
    for (const auto& aResource : theResources)
      appendResource(aResource.first, aResource.second);
    if (theBinary && aDocument.contains("images") && aDocument.at("images").is_array())
    {
      for (const json& anImage : aDocument.at("images"))
      {
        if (!anImage.is_object() || !anImage.contains("uri")
            || !anImage.at("uri").is_string()) continue;
        const std::string aUri = anImage.at("uri").get<std::string>();
        if (aUri.rfind("data:", 0) == 0 || aResourceUris.find(aUri) != aResourceUris.end()) continue;
        const auto aBuffer = std::find_if(
          theBuffers.begin(), theBuffers.end(),
          [&](const auto& theBuffer) { return theBuffer.first == aUri; });
        if (aBuffer != theBuffers.end()) appendResource(aUri, aBuffer->second);
      }
    }

    std::string aJson = aDocument.dump();
    if (!theBinary)
    {
      aResult.data.assign(aJson.begin(), aJson.end());
      return aResult;
    }
    while (aJson.size() % 4 != 0) aJson.push_back(' ');
    while (aBinary.size() % 4 != 0) aBinary.push_back(0);
    const std::uint64_t aTotal = 12ull + 8ull + aJson.size()
      + (aBinary.empty() ? 0ull : 8ull + aBinary.size());
    if (aTotal > std::numeric_limits<std::uint32_t>::max())
      throw KernelFailure(ErrorCode::InvalidArgs, "GLB document output is too large");
    aResult.data.reserve(static_cast<std::size_t>(aTotal));
    appendU32LE(aResult.data, 0x46546c67);
    appendU32LE(aResult.data, 2);
    appendU32LE(aResult.data, static_cast<std::uint32_t>(aTotal));
    appendU32LE(aResult.data, static_cast<std::uint32_t>(aJson.size()));
    appendU32LE(aResult.data, 0x4e4f534a);
    aResult.data.insert(aResult.data.end(), aJson.begin(), aJson.end());
    if (!aBinary.empty())
    {
      appendU32LE(aResult.data, static_cast<std::uint32_t>(aBinary.size()));
      appendU32LE(aResult.data, 0x004e4942);
      aResult.data.insert(aResult.data.end(), aBinary.begin(), aBinary.end());
    }
    return aResult;
  }
std::size_t gltfComponentSize(const int theComponentType)
  {
    switch (theComponentType)
    {
      case 5120:
      case 5121: return 1;
      case 5122:
      case 5123: return 2;
      case 5125:
      case 5126: return 4;
      default:
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF accessor component type is unsupported");
    }
  }

GltfAccessor gltfAccessor(const json& theDocument,
                                   const std::vector<std::vector<std::uint8_t>>& theBuffers,
                                   const std::size_t theAccessorIndex,
                                   const std::string& theExpectedType)
  {
    const json& anAccessors = theDocument.at("accessors");
    if (!anAccessors.is_array() || theAccessorIndex >= anAccessors.size())
    {
      throw KernelFailure(ErrorCode::ImportExportFailed, "glTF accessor index is out of range");
    }
    const json& anAccessor = anAccessors.at(theAccessorIndex);
    if (!anAccessor.is_object() || !anAccessor.contains("componentType")
        || !anAccessor.at("componentType").is_number_integer()
        || !anAccessor.contains("count") || !anAccessor.at("count").is_number_unsigned()
        || (anAccessor.contains("normalized") && !anAccessor.at("normalized").is_boolean())
        || anAccessor.value("type", "") != theExpectedType
        || (!anAccessor.contains("bufferView") && !anAccessor.contains("sparse")))
    {
      throw KernelFailure(ErrorCode::ImportExportFailed, "glTF accessor layout is unsupported");
    }

    const std::int64_t aComponentTypeValue = anAccessor.at("componentType").get<std::int64_t>();
    if (aComponentTypeValue < std::numeric_limits<int>::min()
        || aComponentTypeValue > std::numeric_limits<int>::max())
      throw KernelFailure(ErrorCode::ImportExportFailed,
                          "glTF accessor component type is unsupported");
    const int aComponentType = static_cast<int>(aComponentTypeValue);
    const std::size_t aComponents = theExpectedType == "MAT4" ? 16
      : (theExpectedType == "VEC4" ? 4
      : (theExpectedType == "VEC3" ? 3 : (theExpectedType == "VEC2" ? 2 : 1)));
    const std::size_t aComponentSize = gltfComponentSize(aComponentType);
    const std::size_t anElementSize = aComponentSize * aComponents;
    const std::uint64_t aCountValue = anAccessor.at("count").get<std::uint64_t>();
    if (aCountValue > std::numeric_limits<std::size_t>::max())
      throw KernelFailure(ErrorCode::ImportExportFailed, "glTF accessor is too large");
    const std::size_t aCount = static_cast<std::size_t>(aCountValue);
    const bool isNormalized = anAccessor.value("normalized", false);
    if (isNormalized && (aComponentType == 5125 || aComponentType == 5126))
      throw KernelFailure(ErrorCode::ImportExportFailed, "glTF accessor normalization is invalid");
    if (aCount > std::numeric_limits<std::size_t>::max() / anElementSize)
      throw KernelFailure(ErrorCode::ImportExportFailed, "glTF accessor is too large");

    GltfAccessor aResult;
    aResult.data.resize(aCount * anElementSize, 0);
    aResult.count = aCount;
    aResult.stride = anElementSize;
    aResult.elementSize = anElementSize;
    aResult.components = aComponents;
    aResult.componentType = aComponentType;
    aResult.normalized = isNormalized;
    const json& aViews = theDocument.at("bufferViews");

    if (anAccessor.contains("bufferView"))
    {
      const std::size_t aViewIndex = anAccessor.at("bufferView").get<std::size_t>();
      if (!aViews.is_array() || aViewIndex >= aViews.size())
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF buffer view index is out of range");
      const json& aView = aViews.at(aViewIndex);
      const std::size_t aBufferIndex = aView.at("buffer").get<std::size_t>();
      if (aBufferIndex >= theBuffers.size())
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF buffer index is out of range");
      const std::size_t aStride = aView.value("byteStride", anElementSize);
      if (aStride < anElementSize || aStride % aComponentSize != 0
          || (aView.contains("byteStride")
              && (aStride < 4 || aStride > 252 || aStride % 4 != 0)))
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF accessor stride is invalid");
      const std::size_t aViewOffset = aView.value("byteOffset", std::size_t(0));
      const std::size_t aViewLength = aView.at("byteLength").get<std::size_t>();
      const std::size_t anAccessorOffset = anAccessor.value("byteOffset", std::size_t(0));
      const std::vector<std::uint8_t>& aBuffer = theBuffers[aBufferIndex];
      if (aViewOffset % aComponentSize != 0 || anAccessorOffset % aComponentSize != 0)
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF accessor offset is misaligned");
      if (aViewOffset > aBuffer.size() || aViewLength > aBuffer.size() - aViewOffset
          || anAccessorOffset > aViewLength
          || (aCount > 0
              && (anElementSize > aViewLength - anAccessorOffset
                  || aCount - 1 > (aViewLength - anAccessorOffset - anElementSize) / aStride)))
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF accessor data is truncated");
      const std::size_t anOffset = aViewOffset + anAccessorOffset;
      for (std::size_t anElement = 0; anElement < aCount; ++anElement)
      {
        std::memcpy(aResult.data.data() + anElement * anElementSize,
                    aBuffer.data() + anOffset + anElement * aStride,
                    anElementSize);
      }
    }
    else if (anAccessor.contains("byteOffset"))
    {
      throw KernelFailure(ErrorCode::ImportExportFailed,
                          "glTF accessor offset requires a buffer view");
    }

    if (anAccessor.contains("sparse"))
    {
      const json& aSparse = anAccessor.at("sparse");
      const std::size_t aSparseCount = aSparse.at("count").get<std::size_t>();
      if (aSparseCount == 0 || aSparseCount > aCount)
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF sparse accessor count is invalid");
      const json& anIndices = aSparse.at("indices");
      const int anIndexType = anIndices.at("componentType").get<int>();
      if (anIndexType != 5121 && anIndexType != 5123 && anIndexType != 5125)
        throw KernelFailure(ErrorCode::ImportExportFailed,
                            "glTF sparse index component type is unsupported");
      const std::size_t anIndexSize = gltfComponentSize(anIndexType);
      const std::size_t anIndexViewIndex = anIndices.at("bufferView").get<std::size_t>();
      const json& aValues = aSparse.at("values");
      const std::size_t aValueViewIndex = aValues.at("bufferView").get<std::size_t>();
      if (!aViews.is_array() || anIndexViewIndex >= aViews.size()
          || aValueViewIndex >= aViews.size())
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF buffer view index is out of range");
      const json& anIndexView = aViews.at(anIndexViewIndex);
      const json& aValueView = aViews.at(aValueViewIndex);
      const std::size_t anIndexBufferIndex = anIndexView.at("buffer").get<std::size_t>();
      const std::size_t aValueBufferIndex = aValueView.at("buffer").get<std::size_t>();
      if (anIndexBufferIndex >= theBuffers.size() || aValueBufferIndex >= theBuffers.size())
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF buffer index is out of range");
      const auto& anIndexBuffer = theBuffers[anIndexBufferIndex];
      const auto& aValueBuffer = theBuffers[aValueBufferIndex];
      const std::size_t anIndexViewOffset = anIndexView.value("byteOffset", std::size_t(0));
      const std::size_t anIndexViewLength = anIndexView.at("byteLength").get<std::size_t>();
      const std::size_t anIndexOffset = anIndices.value("byteOffset", std::size_t(0));
      const std::size_t aValueViewOffset = aValueView.value("byteOffset", std::size_t(0));
      const std::size_t aValueViewLength = aValueView.at("byteLength").get<std::size_t>();
      const std::size_t aValueOffset = aValues.value("byteOffset", std::size_t(0));
      if (anIndexViewOffset % anIndexSize != 0 || anIndexOffset % anIndexSize != 0
          || aValueViewOffset % aComponentSize != 0 || aValueOffset % aComponentSize != 0)
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF sparse accessor offset is misaligned");
      if (anIndexViewOffset > anIndexBuffer.size()
          || anIndexViewLength > anIndexBuffer.size() - anIndexViewOffset
          || anIndexOffset > anIndexViewLength
          || aSparseCount > (anIndexViewLength - anIndexOffset) / anIndexSize
          || aValueViewOffset > aValueBuffer.size()
          || aValueViewLength > aValueBuffer.size() - aValueViewOffset
          || aValueOffset > aValueViewLength
          || aSparseCount > (aValueViewLength - aValueOffset) / anElementSize)
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF sparse accessor data is truncated");

      const std::uint8_t* anIndexData = anIndexBuffer.data()
        + anIndexViewOffset + anIndexOffset;
      const std::uint8_t* aValueData = aValueBuffer.data()
        + aValueViewOffset + aValueOffset;
      std::uint32_t aPreviousIndex = 0;
      for (std::size_t aSparseIndex = 0; aSparseIndex < aSparseCount; ++aSparseIndex)
      {
        std::uint32_t anElement = 0;
        const std::uint8_t* anIndexValue = anIndexData + aSparseIndex * anIndexSize;
        if (anIndexType == 5121) anElement = *anIndexValue;
        else if (anIndexType == 5123)
        {
          std::uint16_t aShort = 0;
          std::memcpy(&aShort, anIndexValue, sizeof(aShort));
          anElement = aShort;
        }
        else std::memcpy(&anElement, anIndexValue, sizeof(anElement));
        if (anElement >= aCount || (aSparseIndex > 0 && anElement <= aPreviousIndex))
          throw KernelFailure(ErrorCode::ImportExportFailed,
                              "glTF sparse accessor indices are invalid");
        std::memcpy(aResult.data.data() + static_cast<std::size_t>(anElement) * anElementSize,
                    aValueData + aSparseIndex * anElementSize,
                    anElementSize);
        aPreviousIndex = anElement;
      }
    }
    return aResult;
  }

double gltfAccessorValue(const GltfAccessor& theAccessor,
                                  const std::size_t theElement,
                                  const std::size_t theComponent)
  {
    if (theElement >= theAccessor.count || theComponent >= theAccessor.components)
      throw KernelFailure(ErrorCode::ImportExportFailed, "glTF accessor component is out of range");
    const std::uint8_t* aValue = theAccessor.data.data()
      + theElement * theAccessor.stride
      + theComponent * gltfComponentSize(theAccessor.componentType);
    switch (theAccessor.componentType)
    {
      case 5120:
      {
        std::int8_t aScalar = 0;
        std::memcpy(&aScalar, aValue, sizeof(aScalar));
        return theAccessor.normalized ? std::max(-1.0, static_cast<double>(aScalar) / 127.0)
                                      : static_cast<double>(aScalar);
      }
      case 5121:
      {
        const std::uint8_t aScalar = *aValue;
        return theAccessor.normalized ? static_cast<double>(aScalar) / 255.0
                                      : static_cast<double>(aScalar);
      }
      case 5122:
      {
        std::int16_t aScalar = 0;
        std::memcpy(&aScalar, aValue, sizeof(aScalar));
        return theAccessor.normalized ? std::max(-1.0, static_cast<double>(aScalar) / 32767.0)
                                      : static_cast<double>(aScalar);
      }
      case 5123:
      {
        std::uint16_t aScalar = 0;
        std::memcpy(&aScalar, aValue, sizeof(aScalar));
        return theAccessor.normalized ? static_cast<double>(aScalar) / 65535.0
                                      : static_cast<double>(aScalar);
      }
      case 5125:
      {
        std::uint32_t aScalar = 0;
        std::memcpy(&aScalar, aValue, sizeof(aScalar));
        return static_cast<double>(aScalar);
      }
      case 5126:
      {
        float aScalar = 0.0f;
        std::memcpy(&aScalar, aValue, sizeof(aScalar));
        return static_cast<double>(aScalar);
      }
      default:
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF accessor component type is unsupported");
    }
  }

double gltfTransformDeterminant(const GltfMatrix& theMatrix)
  {
    return theMatrix[0] * (theMatrix[5] * theMatrix[10] - theMatrix[9] * theMatrix[6])
      - theMatrix[4] * (theMatrix[1] * theMatrix[10] - theMatrix[9] * theMatrix[2])
      + theMatrix[8] * (theMatrix[1] * theMatrix[6] - theMatrix[5] * theMatrix[2]);
  }

GltfMatrix multiplyGltfMatrices(const GltfMatrix& theLeft,
                                                const GltfMatrix& theRight)
  {
    GltfMatrix aResult{};
    for (std::size_t aColumn = 0; aColumn < 4; ++aColumn)
    {
      for (std::size_t aRow = 0; aRow < 4; ++aRow)
      {
        for (std::size_t anInner = 0; anInner < 4; ++anInner)
        {
          aResult[aColumn * 4 + aRow] +=
            theLeft[anInner * 4 + aRow] * theRight[aColumn * 4 + anInner];
        }
      }
    }
    return aResult;
  }

GltfMatrix gltfNodeMatrix(const json& theNode,
                                          const GltfEvaluatedNode* theEvaluated)
  {
    const bool hasEvaluatedTransform = theEvaluated != nullptr
      && (theEvaluated->hasTranslation || theEvaluated->hasRotation
          || theEvaluated->hasScale);
    if (theNode.contains("matrix") && !hasEvaluatedTransform)
    {
      const json& aMatrix = theNode.at("matrix");
      if (!aMatrix.is_array() || aMatrix.size() != 16)
      {
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF node matrix is invalid");
      }
      GltfMatrix aResult{};
      for (std::size_t anIndex = 0; anIndex < 16; ++anIndex)
      {
        aResult[anIndex] = aMatrix.at(anIndex).get<double>();
        if (!std::isfinite(aResult[anIndex]))
        {
          throw KernelFailure(ErrorCode::ImportExportFailed, "glTF node matrix is not finite");
        }
      }
      return aResult;
    }

    const auto readVector = [&](const char* theName,
                                const std::array<double, 4>& theDefault,
                                const std::size_t theSize) {
      std::array<double, 4> aResult = theDefault;
      if (!theNode.contains(theName)) return aResult;
      const json& aValue = theNode.at(theName);
      if (!aValue.is_array() || aValue.size() != theSize)
      {
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF node transform is invalid");
      }
      for (std::size_t anIndex = 0; anIndex < theSize; ++anIndex)
      {
        aResult[anIndex] = aValue.at(anIndex).get<double>();
        if (!std::isfinite(aResult[anIndex]))
        {
          throw KernelFailure(ErrorCode::ImportExportFailed, "glTF node transform is not finite");
        }
      }
      return aResult;
    };
    auto aTranslation = readVector("translation", {0.0, 0.0, 0.0, 0.0}, 3);
    auto aScale = readVector("scale", {1.0, 1.0, 1.0, 0.0}, 3);
    auto aRotation = readVector("rotation", {0.0, 0.0, 0.0, 1.0}, 4);
    if (theEvaluated != nullptr)
    {
      if (theEvaluated->hasTranslation) aTranslation = theEvaluated->translation;
      if (theEvaluated->hasRotation) aRotation = theEvaluated->rotation;
      if (theEvaluated->hasScale) aScale = theEvaluated->scale;
    }
    const double aLength = std::sqrt(aRotation[0] * aRotation[0]
                                    + aRotation[1] * aRotation[1]
                                    + aRotation[2] * aRotation[2]
                                    + aRotation[3] * aRotation[3]);
    if (aLength <= std::numeric_limits<double>::epsilon())
    {
      throw KernelFailure(ErrorCode::ImportExportFailed, "glTF node quaternion is invalid");
    }
    for (double& aValue : aRotation) aValue /= aLength;
    const double x = aRotation[0], y = aRotation[1], z = aRotation[2], w = aRotation[3];
    GltfMatrix aResult = {
      (1.0 - 2.0 * (y * y + z * z)) * aScale[0],
      (2.0 * (x * y + z * w)) * aScale[0],
      (2.0 * (x * z - y * w)) * aScale[0],
      0.0,
      (2.0 * (x * y - z * w)) * aScale[1],
      (1.0 - 2.0 * (x * x + z * z)) * aScale[1],
      (2.0 * (y * z + x * w)) * aScale[1],
      0.0,
      (2.0 * (x * z + y * w)) * aScale[2],
      (2.0 * (y * z - x * w)) * aScale[2],
      (1.0 - 2.0 * (x * x + y * y)) * aScale[2],
      0.0,
      aTranslation[0], aTranslation[1], aTranslation[2], 1.0
    };
    return aResult;
  }

gp_Pnt transformGltfPoint(const GltfMatrix& theMatrix,
                                   const double theX,
                                   const double theY,
                                   const double theZ)
  {
    const double x = theMatrix[0] * theX + theMatrix[4] * theY
                   + theMatrix[8] * theZ + theMatrix[12];
    const double y = theMatrix[1] * theX + theMatrix[5] * theY
                   + theMatrix[9] * theZ + theMatrix[13];
    const double z = theMatrix[2] * theX + theMatrix[6] * theY
                   + theMatrix[10] * theZ + theMatrix[14];
    return gp_Pnt(x, -z, y);
  }

gp_Dir transformGltfNormal(const GltfMatrix& theMatrix,
                                    const double theX,
                                    const double theY,
                                    const double theZ)
  {
    const double aDeterminant = gltfTransformDeterminant(theMatrix);
    if (std::abs(aDeterminant) <= Precision::Confusion())
      throw KernelFailure(ErrorCode::ImportExportFailed, "glTF normal transform is singular");
    const double x = ((theMatrix[5] * theMatrix[10] - theMatrix[9] * theMatrix[6]) * theX
                      + (theMatrix[9] * theMatrix[2] - theMatrix[1] * theMatrix[10]) * theY
                      + (theMatrix[1] * theMatrix[6] - theMatrix[5] * theMatrix[2]) * theZ)
                     / aDeterminant;
    const double y = ((theMatrix[8] * theMatrix[6] - theMatrix[4] * theMatrix[10]) * theX
                      + (theMatrix[0] * theMatrix[10] - theMatrix[8] * theMatrix[2]) * theY
                      + (theMatrix[4] * theMatrix[2] - theMatrix[0] * theMatrix[6]) * theZ)
                     / aDeterminant;
    const double z = ((theMatrix[4] * theMatrix[9] - theMatrix[8] * theMatrix[5]) * theX
                      + (theMatrix[8] * theMatrix[1] - theMatrix[0] * theMatrix[9]) * theY
                      + (theMatrix[0] * theMatrix[5] - theMatrix[4] * theMatrix[1]) * theZ)
                     / aDeterminant;
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z)
        || x * x + y * y + z * z <= Precision::SquareConfusion())
      throw KernelFailure(ErrorCode::ImportExportFailed, "glTF transformed normal is invalid");
    return gp_Dir(x, -z, y);
  }

} // namespace occt_worker
