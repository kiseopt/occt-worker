#include "kernel_exchange_helpers.hpp"

namespace occt_worker {

bool TriangleMesh::ObjPrimitive::hasSameState(const ObjPrimitive& theOther) const
{
  return object == theOther.object
    && groups == theOther.groups
    && material == theOther.material
    && smoothingGroup == theOther.smoothingGroup
    && hasObject == theOther.hasObject
    && hasMaterial == theOther.hasMaterial
    && hasSmoothingGroup == theOther.hasSmoothingGroup;
}

json TriangleMesh::ObjPrimitive::toJson() const
{
  json aResult{{"indexStart", indexStart}, {"indexCount", indexCount}, {"groups", groups}};
  if (hasObject) aResult["object"] = object;
  if (hasMaterial) aResult["material"] = material;
  if (hasSmoothingGroup) aResult["smoothingGroup"] = smoothingGroup;
  return aResult;
}

json TriangleMesh::GltfPrimitiveInstance::toJson() const
{
  json aResult{
    {"meshIndex", meshIndex}, {"primitiveIndex", primitiveIndex}, {"mode", mode},
    {"vertexStart", vertexStart}, {"vertexCount", vertexCount},
    {"indexStart", indexStart}, {"indexCount", indexCount},
    {"hasNormals", hasNormals}, {"hasUVs", hasUVs}
  };
  if (hasNode) aResult["nodeIndex"] = nodeIndex;
  if (hasMaterial) aResult["material"] = material;
  return aResult;
}

std::size_t parseOBJComponent(const std::string& theText,
                              const std::size_t theCount,
                              const char* theLabel)
{
  if (theText.empty())
  {
    throw KernelFailure(ErrorCode::ImportExportFailed,
                        std::string("OBJ face has no ") + theLabel + " index");
  }

  long long anIndex = 0;
  try
  {
    std::size_t anEnd = 0;
    anIndex = std::stoll(theText, &anEnd);
    if (anEnd != theText.size())
      throw std::invalid_argument("trailing characters");
  }
  catch (const std::exception&)
  {
    throw KernelFailure(ErrorCode::ImportExportFailed,
                        std::string("OBJ face has an invalid ") + theLabel + " index");
  }

  if (anIndex > 0)
    --anIndex;
  else if (anIndex < 0)
    anIndex = static_cast<long long>(theCount) + anIndex;
  else
    throw KernelFailure(ErrorCode::ImportExportFailed,
                        std::string("OBJ ") + theLabel + " index cannot be zero");
  if (anIndex < 0 || static_cast<std::size_t>(anIndex) >= theCount)
    throw KernelFailure(ErrorCode::ImportExportFailed,
                        std::string("OBJ ") + theLabel + " index is out of range");
  return static_cast<std::size_t>(anIndex);
}

ObjCorner parseOBJCorner(const std::string& theToken,
                         const std::size_t thePositionCount,
                         const std::size_t theUVCount,
                         const std::size_t theNormalCount)
{
  const std::size_t aFirstSlash = theToken.find('/');
  const std::string aPositionText = theToken.substr(0, aFirstSlash);
  ObjCorner aCorner;
  aCorner.position = parseOBJComponent(aPositionText, thePositionCount, "vertex");
  if (aFirstSlash == std::string::npos)
    return aCorner;

  const std::size_t aSecondSlash = theToken.find('/', aFirstSlash + 1);
  const std::string anUVText = theToken.substr(
    aFirstSlash + 1,
    aSecondSlash == std::string::npos ? std::string::npos : aSecondSlash - aFirstSlash - 1);
  if (!anUVText.empty())
    aCorner.uv = parseOBJComponent(anUVText, theUVCount, "texture coordinate");
  if (aSecondSlash == std::string::npos)
    return aCorner;

  const std::string aNormalText = theToken.substr(aSecondSlash + 1);
  aCorner.normal = parseOBJComponent(aNormalText, theNormalCount, "normal");
  return aCorner;
}

std::string objLineValue(std::istringstream& theStream)
{
  std::string aValue;
  std::getline(theStream, aValue);
  const std::size_t aComment = aValue.find('#');
  if (aComment != std::string::npos)
    aValue.erase(aComment);
  const std::size_t aFirst = aValue.find_first_not_of(" \t");
  if (aFirst == std::string::npos)
    return {};
  const std::size_t aLast = aValue.find_last_not_of(" \t");
  return aValue.substr(aFirst, aLast - aFirst + 1);
}

std::string objMapReference(std::istringstream& theStream)
{
  std::vector<std::string> aTokens;
  std::string aToken;
  while (theStream >> aToken)
  {
    if (!aToken.empty() && aToken[0] == '#') break;
    aTokens.push_back(aToken);
  }
  std::size_t anIndex = 0;
  const auto requireArguments = [&](const std::size_t theCount) {
    if (anIndex + theCount >= aTokens.size())
      throw KernelFailure(ErrorCode::ImportExportFailed,
                          "MTL map option is missing an argument or path");
    anIndex += theCount;
  };
  const auto isNumber = [](const std::string& theValue) {
    try
    {
      std::size_t anEnd = 0;
      const double aValue = std::stod(theValue, &anEnd);
      return anEnd == theValue.size() && std::isfinite(aValue);
    }
    catch (const std::exception&)
    {
      return false;
    }
  };
  while (anIndex < aTokens.size() && !aTokens[anIndex].empty()
         && aTokens[anIndex][0] == '-')
  {
    const std::string anOption = aTokens[anIndex++];
    if (anOption == "-mm")
    {
      requireArguments(2);
    }
    else if (anOption == "-o" || anOption == "-s" || anOption == "-t")
    {
      const std::size_t aStart = anIndex;
      while (anIndex < aTokens.size() && anIndex - aStart < 3
             && isNumber(aTokens[anIndex]))
        ++anIndex;
      if (anIndex == aStart)
        throw KernelFailure(ErrorCode::ImportExportFailed,
                            "MTL map vector option is invalid");
    }
    else if (anOption == "-blendu" || anOption == "-blendv" || anOption == "-boost"
             || anOption == "-texres" || anOption == "-clamp" || anOption == "-bm"
             || anOption == "-imfchan" || anOption == "-type" || anOption == "-cc"
             || anOption == "-colorspace")
    {
      requireArguments(1);
    }
    else if (anIndex + 1 < aTokens.size() && aTokens[anIndex][0] != '-')
    {
      ++anIndex;
    }
  }
  if (anIndex >= aTokens.size())
    throw KernelFailure(ErrorCode::ImportExportFailed, "MTL map has no resource path");
  std::string aPath = aTokens[anIndex++];
  while (anIndex < aTokens.size())
    aPath += " " + aTokens[anIndex++];
  return aPath;
}

TriangleMesh parseOBJ(
    const std::string& theData,
    const std::unordered_map<std::string, std::string>& theResources)
  {
    TriangleMesh aMesh;
    std::vector<gp_Pnt> aRawPositions;
    std::vector<gp_Pnt2d> aRawUVs;
    std::vector<gp_Dir> aRawNormals;
    std::unordered_map<std::string, std::size_t> aCornerMap;
    bool hasAnyUVs = false;
    bool hasAllUVs = true;
    bool hasAnyNormals = false;
    bool hasAllNormals = true;
    TriangleMesh::ObjPrimitive aState;
    std::istringstream aStream(theData);
    std::string aLine;
    while (std::getline(aStream, aLine))
    {
      if (!aLine.empty() && aLine.back() == '\r')
      {
        aLine.pop_back();
      }
      std::istringstream aLineStream(aLine);
      std::string aCommand;
      aLineStream >> aCommand;
      if (aCommand.empty() || aCommand[0] == '#')
      {
        continue;
      }
      if (aCommand == "v")
      {
        double x = 0.0, y = 0.0, z = 0.0;
        if (!(aLineStream >> x >> y >> z) || !std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z))
        {
          throw KernelFailure(ErrorCode::ImportExportFailed, "OBJ vertex is invalid");
        }
        aRawPositions.emplace_back(x, y, z);
      }
      else if (aCommand == "vt")
      {
        double u = 0.0, v = 0.0;
        if (!(aLineStream >> u >> v) || !std::isfinite(u) || !std::isfinite(v))
        {
          throw KernelFailure(ErrorCode::ImportExportFailed, "OBJ texture coordinate is invalid");
        }
        aRawUVs.emplace_back(u, v);
      }
      else if (aCommand == "vn")
      {
        double x = 0.0, y = 0.0, z = 0.0;
        if (!(aLineStream >> x >> y >> z) || !std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z)
            || x * x + y * y + z * z <= Precision::SquareConfusion())
        {
          throw KernelFailure(ErrorCode::ImportExportFailed, "OBJ normal is invalid");
        }
        aRawNormals.emplace_back(x, y, z);
      }
      else if (aCommand == "o")
      {
        aState.object = objLineValue(aLineStream);
        aState.hasObject = !aState.object.empty();
      }
      else if (aCommand == "g")
      {
        aState.groups.clear();
        std::string aGroup;
        while (aLineStream >> aGroup)
        {
          if (!aGroup.empty() && aGroup[0] == '#') break;
          aState.groups.push_back(aGroup);
        }
      }
      else if (aCommand == "usemtl")
      {
        aState.material = objLineValue(aLineStream);
        aState.hasMaterial = !aState.material.empty();
      }
      else if (aCommand == "s")
      {
        aState.smoothingGroup = objLineValue(aLineStream);
        aState.hasSmoothingGroup = !aState.smoothingGroup.empty();
      }
      else if (aCommand == "mtllib")
      {
        std::string aLibrary;
        while (aLineStream >> aLibrary)
        {
          if (!aLibrary.empty() && aLibrary[0] == '#') break;
          aMesh.objMaterialLibraries.push_back(aLibrary);
        }
      }
      else if (aCommand == "f")
      {
        std::vector<ObjCorner> aFace;
        std::string aToken;
        while (aLineStream >> aToken)
        {
          if (!aToken.empty() && aToken[0] == '#') break;
          aFace.push_back(parseOBJCorner(aToken, aRawPositions.size(), aRawUVs.size(), aRawNormals.size()));
        }
        if (aFace.size() < 3)
        {
          throw KernelFailure(ErrorCode::ImportExportFailed, "OBJ face must have at least three vertices");
        }
        std::vector<std::uint32_t> aOutputFace;
        aOutputFace.reserve(aFace.size());
        for (const ObjCorner& aCorner : aFace)
        {
          const std::string aKey = std::to_string(aCorner.position) + "/"
            + std::to_string(aCorner.uv) + "/" + std::to_string(aCorner.normal);
          const auto anExisting = aCornerMap.find(aKey);
          if (anExisting != aCornerMap.end())
          {
            aOutputFace.push_back(static_cast<std::uint32_t>(anExisting->second));
            continue;
          }
          const std::size_t anOutputIndex = aMesh.positions.size();
          aCornerMap.emplace(aKey, anOutputIndex);
          aMesh.positions.push_back(aRawPositions[aCorner.position]);
          if (aCorner.uv == std::numeric_limits<std::size_t>::max())
          {
            hasAllUVs = false;
            aMesh.uvs.emplace_back(0.0, 0.0);
          }
          else
          {
            hasAnyUVs = true;
            aMesh.uvs.push_back(aRawUVs[aCorner.uv]);
          }
          if (aCorner.normal == std::numeric_limits<std::size_t>::max())
          {
            hasAllNormals = false;
            aMesh.normals.emplace_back(0.0, 0.0, 1.0);
          }
          else
          {
            hasAnyNormals = true;
            aMesh.normals.push_back(aRawNormals[aCorner.normal]);
          }
          aOutputFace.push_back(static_cast<std::uint32_t>(anOutputIndex));
        }
        for (std::size_t anIndex = 1; anIndex + 1 < aFace.size(); ++anIndex)
        {
          aMesh.indices.insert(aMesh.indices.end(), {
            aOutputFace[0], aOutputFace[anIndex], aOutputFace[anIndex + 1]});
        }
        TriangleMesh::ObjPrimitive aPrimitive = aState;
        aPrimitive.indexStart = static_cast<std::uint32_t>(
          aMesh.indices.size() - (aFace.size() - 2) * 3);
        aPrimitive.indexCount = static_cast<std::uint32_t>((aFace.size() - 2) * 3);
        if (!aMesh.objPrimitives.empty()
            && aMesh.objPrimitives.back().indexStart + aMesh.objPrimitives.back().indexCount
              == aPrimitive.indexStart
            && aMesh.objPrimitives.back().hasSameState(aPrimitive))
          aMesh.objPrimitives.back().indexCount += aPrimitive.indexCount;
        else
          aMesh.objPrimitives.push_back(std::move(aPrimitive));
      }
    }
    if (aMesh.positions.empty() || aMesh.indices.empty())
    {
      throw KernelFailure(ErrorCode::ImportExportFailed, "OBJ input has no triangulated faces");
    }
    if (!hasAnyUVs || !hasAllUVs) aMesh.uvs.clear();
    if (!hasAnyNormals || !hasAllNormals) aMesh.normals.clear();
    for (const std::string& aLibrary : aMesh.objMaterialLibraries)
    {
      const auto aResource = theResources.find(aLibrary);
      if (aResource == theResources.end())
        continue;
      const std::vector<json> aMaterials = parseMTL(aResource->second);
      aMesh.objMaterials.insert(aMesh.objMaterials.end(), aMaterials.begin(), aMaterials.end());
    }
    return aMesh;
  }

TriangleMesh parsePLY(const std::string& theData)
  {
    struct Property { bool isList = false; std::string type; std::string name; std::string listType; };
    struct Element { std::string name; std::size_t count = 0; std::vector<Property> properties; };
    if (theData.size() < 4 || theData.compare(0, 4, "ply\n") != 0 && theData.compare(0, 5, "ply\r\n") != 0)
      throw KernelFailure(ErrorCode::ImportExportFailed, "PLY header is missing");
    const std::size_t aHeaderEnd = theData.find("end_header");
    if (aHeaderEnd == std::string::npos)
      throw KernelFailure(ErrorCode::ImportExportFailed, "PLY header is missing end_header");
    const std::size_t aBodyStart = theData.find('\n', aHeaderEnd);
    if (aBodyStart == std::string::npos)
      throw KernelFailure(ErrorCode::ImportExportFailed, "PLY header is truncated");
    std::istringstream aHeader(theData.substr(0, aBodyStart + 1));
    std::string aLine, aFormat;
    double aVersion = 0.0;
    std::vector<Element> anElements;
    std::vector<std::string> aComments;
    std::vector<std::string> anObjectInfo;
    Element* aCurrent = nullptr;
    while (std::getline(aHeader, aLine))
    {
      if (!aLine.empty() && aLine.back() == '\r') aLine.pop_back();
      std::istringstream aTokens(aLine);
      std::string aCommand;
      aTokens >> aCommand;
      if (aCommand == "format") aTokens >> aFormat >> aVersion;
      else if (aCommand == "comment" || aCommand == "obj_info")
      {
        std::string aValue;
        std::getline(aTokens, aValue);
        const std::size_t aFirst = aValue.find_first_not_of(" \t");
        if (aFirst == std::string::npos) aValue.clear();
        else aValue.erase(0, aFirst);
        (aCommand == "comment" ? aComments : anObjectInfo).push_back(std::move(aValue));
      }
      else if (aCommand == "element")
      {
        Element anElement;
        aTokens >> anElement.name >> anElement.count;
        anElements.push_back(std::move(anElement));
        aCurrent = &anElements.back();
      }
      else if (aCommand == "property" && aCurrent != nullptr)
      {
        Property aProperty;
        std::string aKind;
        aTokens >> aKind;
        if (aKind == "list")
        {
          aProperty.isList = true;
          aTokens >> aProperty.listType >> aProperty.type >> aProperty.name;
        }
        else
        {
          aProperty.type = aKind;
          aTokens >> aProperty.name;
        }
        if (!aProperty.name.empty()) aCurrent->properties.push_back(std::move(aProperty));
      }
    }
    const bool isAscii = aFormat == "ascii" && aVersion == 1.0;
    const bool isLittle = aFormat == "binary_little_endian" && aVersion == 1.0;
    const bool isBig = aFormat == "binary_big_endian" && aVersion == 1.0;
    if ((!isAscii && !isLittle && !isBig) || anElements.empty())
      throw KernelFailure(ErrorCode::ImportExportFailed, "Unsupported PLY format");
    auto aVertexIt = std::find_if(anElements.begin(), anElements.end(), [](const Element& e) { return e.name == "vertex"; });
    auto aFaceIt = std::find_if(anElements.begin(), anElements.end(), [](const Element& e) { return e.name == "face"; });
    if (aVertexIt == anElements.end() || aFaceIt == anElements.end() || aVertexIt->count == 0 || aFaceIt->count == 0)
      throw KernelFailure(ErrorCode::ImportExportFailed, "PLY input has no vertices or faces");
    auto propertyIndex = [&](const Element& e, const char* name, std::size_t fallback) {
      std::size_t aScalarIndex = 0;
      for (const Property& aProperty : e.properties)
      {
        if (aProperty.isList) continue;
        if (aProperty.name == name) return aScalarIndex;
        ++aScalarIndex;
      }
      if (fallback >= aScalarIndex)
        throw KernelFailure(ErrorCode::ImportExportFailed, "PLY vertex position properties are missing");
      return fallback;
    };
    auto optionalPropertyIndex = [&](const Element& e,
                                     const std::initializer_list<const char*> theNames) {
      std::size_t aScalarIndex = 0;
      for (const Property& aProperty : e.properties)
      {
        if (aProperty.isList) continue;
        for (const char* aName : theNames)
          if (aProperty.name == aName) return aScalarIndex;
        ++aScalarIndex;
      }
      return std::numeric_limits<std::size_t>::max();
    };
    const std::size_t anX = propertyIndex(*aVertexIt, "x", 0);
    const std::size_t aY = propertyIndex(*aVertexIt, "y", 1);
    const std::size_t aZ = propertyIndex(*aVertexIt, "z", 2);
    const std::size_t anNX = optionalPropertyIndex(*aVertexIt, {"nx"});
    const std::size_t anNY = optionalPropertyIndex(*aVertexIt, {"ny"});
    const std::size_t anNZ = optionalPropertyIndex(*aVertexIt, {"nz"});
    const std::size_t anU = optionalPropertyIndex(*aVertexIt, {"s", "u", "texture_u"});
    const std::size_t aV = optionalPropertyIndex(*aVertexIt, {"t", "v", "texture_v"});
    const std::size_t aRed = optionalPropertyIndex(*aVertexIt, {"red", "r"});
    const std::size_t aGreen = optionalPropertyIndex(*aVertexIt, {"green", "g"});
    const std::size_t aBlue = optionalPropertyIndex(*aVertexIt, {"blue", "b"});
    const std::size_t anAlpha = optionalPropertyIndex(*aVertexIt, {"alpha", "a"});
    const bool hasNormals = anNX != std::numeric_limits<std::size_t>::max()
      && anNY != std::numeric_limits<std::size_t>::max()
      && anNZ != std::numeric_limits<std::size_t>::max();
    const bool hasUVs = anU != std::numeric_limits<std::size_t>::max()
      && aV != std::numeric_limits<std::size_t>::max();
    const bool hasAnyColor = aRed != std::numeric_limits<std::size_t>::max()
      || aGreen != std::numeric_limits<std::size_t>::max()
      || aBlue != std::numeric_limits<std::size_t>::max()
      || anAlpha != std::numeric_limits<std::size_t>::max();
    const bool hasColors = aRed != std::numeric_limits<std::size_t>::max()
      && aGreen != std::numeric_limits<std::size_t>::max()
      && aBlue != std::numeric_limits<std::size_t>::max();
    if (hasAnyColor && !hasColors)
      throw KernelFailure(ErrorCode::ImportExportFailed, "PLY vertex colors require red, green, and blue");
    std::vector<std::string> aVertexScalarTypes;
    for (const Property& aProperty : aVertexIt->properties)
      if (!aProperty.isList) aVertexScalarTypes.push_back(aProperty.type);
    auto colorByte = [&](const double theValue, const std::size_t thePropertyIndex) {
      if (!std::isfinite(theValue) || thePropertyIndex >= aVertexScalarTypes.size())
        throw KernelFailure(ErrorCode::ImportExportFailed, "PLY vertex color is invalid");
      const std::string& aType = aVertexScalarTypes[thePropertyIndex];
      double aMaximum = 1.0;
      if (aType == "uchar" || aType == "uint8") aMaximum = 255.0;
      else if (aType == "ushort" || aType == "uint16") aMaximum = 65535.0;
      else if (aType == "uint" || aType == "uint32") aMaximum = 4294967295.0;
      else if (aType == "uint64") aMaximum = 18446744073709551615.0;
      else if (aType != "float" && aType != "float32" && aType != "double" && aType != "float64")
        throw KernelFailure(ErrorCode::ImportExportFailed, "PLY vertex color type is unsupported");
      if (theValue < 0.0 || theValue > aMaximum)
        throw KernelFailure(ErrorCode::ImportExportFailed, "PLY vertex color is out of range");
      return static_cast<std::uint8_t>(std::lround(theValue / aMaximum * 255.0));
    };
    std::size_t aFaceList = 0;
    bool hasFaceList = false;
    for (std::size_t i = 0; i < aFaceIt->properties.size(); ++i)
    {
      if (aFaceIt->properties[i].isList
          && (aFaceIt->properties[i].name == "vertex_indices" || !hasFaceList))
      {
        aFaceList = i;
        hasFaceList = true;
      }
    }
    if (!hasFaceList)
      throw KernelFailure(ErrorCode::ImportExportFailed, "PLY face indices are missing");
    TriangleMesh aMesh;
    aMesh.plyComments = std::move(aComments);
    aMesh.plyObjectInfo = std::move(anObjectInfo);
    aMesh.hasPlyDocument = true;
    aMesh.positions.reserve(aVertexIt->count);
    const char* aBody = theData.data() + aBodyStart + 1;
    const std::size_t aBodySize = theData.size() - (aBodyStart + 1);
    if (isAscii)
    {
      std::istringstream aStream(std::string(aBody, aBodySize));
      for (const Element& e : anElements)
      {
        for (std::size_t r = 0; r < e.count; ++r)
        {
          if (!std::getline(aStream, aLine))
            throw KernelFailure(ErrorCode::ImportExportFailed, "PLY data is truncated");
          std::istringstream values(aLine);
          if (e.name == "vertex")
          {
            std::vector<double> v;
            for (const Property& p : e.properties)
            {
              if (p.isList)
              {
                std::size_t n;
                if (!(values >> n))
                  throw KernelFailure(ErrorCode::ImportExportFailed, "PLY vertex data is invalid");
                double x;
                for (std::size_t i = 0; i < n; ++i)
                {
                  if (!(values >> x))
                    throw KernelFailure(ErrorCode::ImportExportFailed, "PLY vertex data is invalid");
                }
              }
              else
              {
                double x;
                if (!(values >> x))
                  throw KernelFailure(ErrorCode::ImportExportFailed, "PLY vertex data is invalid");
                v.push_back(x);
              }
            }
            if (v.size() <= std::max({anX, aY, aZ})
                || !std::isfinite(v[anX]) || !std::isfinite(v[aY]) || !std::isfinite(v[aZ]))
              throw KernelFailure(ErrorCode::ImportExportFailed, "PLY vertex data is invalid");
            aMesh.positions.emplace_back(v[anX], v[aY], v[aZ]);
            if (hasNormals)
            {
              if (v.size() <= std::max({anNX, anNY, anNZ})
                  || !std::isfinite(v[anNX]) || !std::isfinite(v[anNY]) || !std::isfinite(v[anNZ])
                  || v[anNX] * v[anNX] + v[anNY] * v[anNY] + v[anNZ] * v[anNZ]
                       <= Precision::SquareConfusion())
                throw KernelFailure(ErrorCode::ImportExportFailed, "PLY vertex normal is invalid");
              aMesh.normals.emplace_back(v[anNX], v[anNY], v[anNZ]);
            }
            if (hasUVs)
            {
              if (v.size() <= std::max(anU, aV) || !std::isfinite(v[anU]) || !std::isfinite(v[aV]))
                throw KernelFailure(ErrorCode::ImportExportFailed, "PLY vertex UV is invalid");
              aMesh.uvs.emplace_back(v[anU], v[aV]);
            }
            if (hasColors)
            {
              if (v.size() <= std::max({aRed, aGreen, aBlue}))
                throw KernelFailure(ErrorCode::ImportExportFailed, "PLY vertex color is invalid");
              const std::uint8_t anAlphaValue = anAlpha == std::numeric_limits<std::size_t>::max()
                ? 255 : colorByte(v.at(anAlpha), anAlpha);
              aMesh.colors.push_back({colorByte(v[aRed], aRed), colorByte(v[aGreen], aGreen),
                                      colorByte(v[aBlue], aBlue), anAlphaValue});
            }
          }
          else if (e.name == "face")
          {
            std::vector<std::size_t> indices;
            for (std::size_t p = 0; p < e.properties.size(); ++p)
            {
              if (e.properties[p].isList)
              {
                std::size_t n;
                if (!(values >> n))
                  throw KernelFailure(ErrorCode::ImportExportFailed, "PLY face data is invalid");
                if (p == aFaceList)
                {
                  indices.resize(n);
                  for (std::size_t i = 0; i < n; ++i)
                  {
                    if (!(values >> indices[i]) || indices[i] >= aVertexIt->count)
                      throw KernelFailure(ErrorCode::ImportExportFailed,
                                          "PLY face index is out of range");
                  }
                }
                else
                {
                  std::size_t x;
                  for (std::size_t i = 0; i < n; ++i)
                  {
                    if (!(values >> x))
                      throw KernelFailure(ErrorCode::ImportExportFailed, "PLY face data is invalid");
                  }
                }
              }
              else
              {
                double x;
                if (!(values >> x))
                  throw KernelFailure(ErrorCode::ImportExportFailed, "PLY face data is invalid");
              }
            }
            if (indices.size() < 3)
              throw KernelFailure(ErrorCode::ImportExportFailed,
                                  "PLY face must have at least three vertices");
            for (std::size_t i = 1; i + 1 < indices.size(); ++i)
            {
              aMesh.indices.insert(aMesh.indices.end(),
                                   {static_cast<std::uint32_t>(indices[0]),
                                    static_cast<std::uint32_t>(indices[i]),
                                    static_cast<std::uint32_t>(indices[i + 1])});
            }
          }
        }
      }
    }
    else
    {
      std::size_t pos = 0;
      auto typeSize = [](const std::string& t) -> std::size_t {
        if (t == "char" || t == "int8" || t == "uchar" || t == "uint8") return 1;
        if (t == "short" || t == "int16" || t == "ushort" || t == "uint16") return 2;
        if (t == "int" || t == "int32" || t == "uint" || t == "uint32"
            || t == "float" || t == "float32")
          return 4;
        if (t == "double" || t == "float64" || t == "int64" || t == "uint64") return 8;
        return 0;
      };
      auto readScalar = [&](const std::string& type) -> double {
        const std::size_t n = typeSize(type);
        if (n == 0 || pos + n > aBodySize)
          throw KernelFailure(ErrorCode::ImportExportFailed, "PLY binary data is truncated");
        const bool signedType = type == "char" || type == "int8" || type == "short"
          || type == "int16" || type == "int" || type == "int32" || type == "int64";
        std::uint64_t u = 0;
        if (isLittle)
        {
          for (std::size_t i = 0; i < n; ++i)
          {
            u |= static_cast<std::uint64_t>(static_cast<unsigned char>(aBody[pos + i]))
              << (8 * i);
          }
        }
        else
        {
          for (std::size_t i = 0; i < n; ++i)
            u = (u << 8) | static_cast<unsigned char>(aBody[pos + i]);
        }
        pos += n;
        if (type == "float" || type == "float32" || type == "double" || type == "float64")
        {
          double d = 0;
          if (n == 4)
          {
            const std::uint32_t bits = static_cast<std::uint32_t>(u);
            float f;
            std::memcpy(&f, &bits, 4);
            d = f;
          }
          else
          {
            const std::uint64_t bits = u;
            std::memcpy(&d, &bits, 8);
          }
          return d;
        }
        if (signedType)
        {
          const std::uint64_t sign = std::uint64_t(1) << (n * 8 - 1);
          const std::int64_t signedValue = (u & sign)
            ? (n == 8 ? static_cast<std::int64_t>(u)
                      : static_cast<std::int64_t>(u | (~std::uint64_t(0) << (n * 8))))
            : static_cast<std::int64_t>(u);
          return static_cast<double>(signedValue);
        }
        return static_cast<double>(u);
      };
      for (const Element& e : anElements)
      {
        for (std::size_t r = 0; r < e.count; ++r)
        {
          std::vector<double> values;
          std::vector<std::size_t> indices;
          for (std::size_t p = 0; p < e.properties.size(); ++p)
          {
            const Property& prop = e.properties[p];
            if (!prop.isList)
            {
              values.push_back(readScalar(prop.type));
              continue;
            }
            const std::size_t n = static_cast<std::size_t>(readScalar(prop.listType));
            if (n > 100000000)
              throw KernelFailure(ErrorCode::ImportExportFailed, "PLY list is too large");
            if (e.name == "face" && p == aFaceList)
            {
              indices.resize(n);
              for (std::size_t i = 0; i < n; ++i)
              {
                const double x = readScalar(prop.type);
                if (!std::isfinite(x) || x < 0
                    || x >= static_cast<double>(aVertexIt->count))
                  throw KernelFailure(ErrorCode::ImportExportFailed,
                                      "PLY face index is out of range");
                indices[i] = static_cast<std::size_t>(x);
              }
            }
            else
            {
              for (std::size_t i = 0; i < n; ++i)
                (void)readScalar(prop.type);
            }
          }
          if (e.name == "vertex")
          {
            if (values.size() <= std::max({anX, aY, aZ})
                || !std::isfinite(values[anX]) || !std::isfinite(values[aY])
                || !std::isfinite(values[aZ]))
              throw KernelFailure(ErrorCode::ImportExportFailed, "PLY vertex data is invalid");
            aMesh.positions.emplace_back(values[anX], values[aY], values[aZ]);
            if (hasNormals)
            {
              if (values.size() <= std::max({anNX, anNY, anNZ})
                  || !std::isfinite(values[anNX]) || !std::isfinite(values[anNY])
                  || !std::isfinite(values[anNZ])
                  || values[anNX] * values[anNX] + values[anNY] * values[anNY]
                       + values[anNZ] * values[anNZ] <= Precision::SquareConfusion())
                throw KernelFailure(ErrorCode::ImportExportFailed, "PLY vertex normal is invalid");
              aMesh.normals.emplace_back(values[anNX], values[anNY], values[anNZ]);
            }
            if (hasUVs)
            {
              if (values.size() <= std::max(anU, aV)
                  || !std::isfinite(values[anU]) || !std::isfinite(values[aV]))
                throw KernelFailure(ErrorCode::ImportExportFailed, "PLY vertex UV is invalid");
              aMesh.uvs.emplace_back(values[anU], values[aV]);
            }
            if (hasColors)
            {
              if (values.size() <= std::max({aRed, aGreen, aBlue}))
                throw KernelFailure(ErrorCode::ImportExportFailed, "PLY vertex color is invalid");
              const std::uint8_t anAlphaValue = anAlpha == std::numeric_limits<std::size_t>::max()
                ? 255 : colorByte(values.at(anAlpha), anAlpha);
              aMesh.colors.push_back({colorByte(values[aRed], aRed), colorByte(values[aGreen], aGreen),
                                      colorByte(values[aBlue], aBlue), anAlphaValue});
            }
          }
          else if (e.name == "face")
          {
            if (indices.size() < 3)
              throw KernelFailure(ErrorCode::ImportExportFailed,
                                  "PLY face must have at least three vertices");
            for (std::size_t i = 1; i + 1 < indices.size(); ++i)
            {
              aMesh.indices.insert(aMesh.indices.end(),
                                   {static_cast<std::uint32_t>(indices[0]),
                                    static_cast<std::uint32_t>(indices[i]),
                                    static_cast<std::uint32_t>(indices[i + 1])});
            }
          }
        }
      }
    }
    if (aMesh.indices.empty()) throw KernelFailure(ErrorCode::ImportExportFailed, "PLY input has no triangulated faces");
    return aMesh;
  }

std::vector<json> parseMTL(const std::string& theData)
  {
    std::vector<json> aMaterials;
    std::size_t aCurrent = std::numeric_limits<std::size_t>::max();
    std::istringstream aStream(theData);
    std::string aLine;
    while (std::getline(aStream, aLine))
    {
      if (!aLine.empty() && aLine.back() == '\r')
        aLine.pop_back();
      std::istringstream aLineStream(aLine);
      std::string aCommand;
      aLineStream >> aCommand;
      if (aCommand.empty() || aCommand[0] == '#')
        continue;
      if (aCommand == "newmtl")
      {
        const std::string aName = objLineValue(aLineStream);
        if (aName.empty())
          throw KernelFailure(ErrorCode::ImportExportFailed, "MTL material has no name");
        aMaterials.push_back({{"name", aName}});
        aCurrent = aMaterials.size() - 1;
        continue;
      }
      if (aCurrent == std::numeric_limits<std::size_t>::max())
        continue;

      json& aMaterial = aMaterials[aCurrent];
      const auto readVector = [&](const char* theProperty) {
        double x = 0.0, y = 0.0, z = 0.0;
        if (!(aLineStream >> x >> y >> z)
            || !std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z))
        {
          throw KernelFailure(ErrorCode::ImportExportFailed,
                              std::string("MTL ") + aCommand + " value is invalid");
        }
        aMaterial[theProperty] = {x, y, z};
      };
      const auto readNumber = [&]() {
        double aValue = 0.0;
        if (!(aLineStream >> aValue) || !std::isfinite(aValue))
          throw KernelFailure(ErrorCode::ImportExportFailed,
                              std::string("MTL ") + aCommand + " value is invalid");
        return aValue;
      };

      if (aCommand == "Ka") readVector("ambient");
      else if (aCommand == "Kd") readVector("diffuse");
      else if (aCommand == "Ks") readVector("specular");
      else if (aCommand == "Ke") readVector("emissive");
      else if (aCommand == "d") aMaterial["opacity"] = readNumber();
      else if (aCommand == "Tr") aMaterial["opacity"] = 1.0 - readNumber();
      else if (aCommand == "Ns") aMaterial["shininess"] = readNumber();
      else if (aCommand == "Ni") aMaterial["opticalDensity"] = readNumber();
      else if (aCommand == "illum")
      {
        long long aValue = -1;
        if (!(aLineStream >> aValue) || aValue < 0
            || static_cast<unsigned long long>(aValue) > std::numeric_limits<std::uint32_t>::max())
          throw KernelFailure(ErrorCode::ImportExportFailed, "MTL illum value is invalid");
        aMaterial["illuminationModel"] = static_cast<std::uint32_t>(aValue);
      }
      else if (aCommand == "map_Kd") aMaterial["diffuseMap"] = objMapReference(aLineStream);
      else if (aCommand == "map_Ks") aMaterial["specularMap"] = objMapReference(aLineStream);
      else if (aCommand == "map_d") aMaterial["opacityMap"] = objMapReference(aLineStream);
      else if (aCommand == "bump" || aCommand == "map_Bump" || aCommand == "map_bump")
        aMaterial["bumpMap"] = objMapReference(aLineStream);
    }
    return aMaterials;
  }

} // namespace occt_worker
