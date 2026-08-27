#include "kernel_exchange_helpers.hpp"
#include "kernel_protocol_helpers.hpp"
#include "kernel_operation_context.hpp"

namespace occt_worker {

json dispatchExportOBJDocument(KernelOperationContext& theContext, const json& theArgs)
  {
    occ::handle<Message_ProgressIndicator> aProgress = new HostProgressIndicator();
    Message_ProgressScope aProgressScope(aProgress->Start(), "OBJ document export", 3.0);
    aProgressScope.Show();
    const std::string aPositionBytes = inputBufferData(theContext.buffers(), theArgs, "positions");
    const std::string anIndexBytes = inputBufferData(theContext.buffers(), theArgs, "indices");
    if (aPositionBytes.empty() || aPositionBytes.size() % (sizeof(float) * 3) != 0
        || anIndexBytes.empty() || anIndexBytes.size() % (sizeof(std::uint32_t) * 3) != 0)
      throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document mesh buffers have invalid dimensions");
    TriangleMesh aMesh;
    const std::size_t aVertexCount = aPositionBytes.size() / (sizeof(float) * 3);
    if (aVertexCount > std::numeric_limits<std::uint32_t>::max())
      throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document mesh has too many vertices");
    aMesh.positions.reserve(aVertexCount);
    for (std::size_t aVertex = 0; aVertex < aVertexCount; ++aVertex)
    {
      std::array<float, 3> aPoint{};
      std::memcpy(aPoint.data(), aPositionBytes.data() + aVertex * sizeof(float) * 3,
                  sizeof(float) * 3);
      if (!std::isfinite(aPoint[0]) || !std::isfinite(aPoint[1]) || !std::isfinite(aPoint[2]))
        throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document positions must be finite");
      aMesh.positions.emplace_back(aPoint[0], aPoint[1], aPoint[2]);
    }
    aMesh.indices.resize(anIndexBytes.size() / sizeof(std::uint32_t));
    if (aMesh.indices.size() > std::numeric_limits<std::uint32_t>::max())
      throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document mesh has too many indices");
    std::memcpy(aMesh.indices.data(), anIndexBytes.data(), anIndexBytes.size());
    for (const std::uint32_t anIndex : aMesh.indices)
      if (anIndex >= aVertexCount)
        throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document index is out of range");
    if (theArgs.contains("normals"))
    {
      const std::string aNormalBytes = inputBufferData(theContext.buffers(), theArgs, "normals");
      if (aNormalBytes.size() != aVertexCount * sizeof(float) * 3)
        throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document normals must match positions");
      aMesh.normals.reserve(aVertexCount);
      for (std::size_t aVertex = 0; aVertex < aVertexCount; ++aVertex)
      {
        std::array<float, 3> aNormal{};
        std::memcpy(aNormal.data(), aNormalBytes.data() + aVertex * sizeof(float) * 3,
                    sizeof(float) * 3);
        if (!std::isfinite(aNormal[0]) || !std::isfinite(aNormal[1])
            || !std::isfinite(aNormal[2])
            || aNormal[0] * aNormal[0] + aNormal[1] * aNormal[1]
                 + aNormal[2] * aNormal[2] <= Precision::SquareConfusion())
          throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document normals must be finite and non-zero");
        aMesh.normals.emplace_back(aNormal[0], aNormal[1], aNormal[2]);
      }
    }
    if (theArgs.contains("uvs"))
    {
      const std::string aUVBytes = inputBufferData(theContext.buffers(), theArgs, "uvs");
      if (aUVBytes.size() != aVertexCount * sizeof(float) * 2)
        throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document UVs must match positions");
      aMesh.uvs.reserve(aVertexCount);
      for (std::size_t aVertex = 0; aVertex < aVertexCount; ++aVertex)
      {
        std::array<float, 2> aUV{};
        std::memcpy(aUV.data(), aUVBytes.data() + aVertex * sizeof(float) * 2,
                    sizeof(float) * 2);
        if (!std::isfinite(aUV[0]) || !std::isfinite(aUV[1]))
          throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document UVs must be finite");
        aMesh.uvs.emplace_back(aUV[0], aUV[1]);
      }
    }
    const json aDocument = theArgs.value("document", json{
      {"primitives", json::array()}, {"materials", json::array()},
      {"materialLibraries", json::array()}
    });
    if (!aDocument.is_object())
      throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document metadata is invalid");
    const auto checkedOBJText = [](const json& theValue, const char* theName) {
      if (!theValue.is_string())
        throw KernelFailure(ErrorCode::InvalidArgs,
                            std::string("OBJ document ") + theName + " is invalid");
      const std::string aText = theValue.get<std::string>();
      if (aText.empty() || aText.find_first_of("\r\n") != std::string::npos)
        throw KernelFailure(ErrorCode::InvalidArgs,
                            std::string("OBJ document ") + theName + " is invalid");
      return aText;
    };
    const json aPrimitives = aDocument.value("primitives", json::array());
    if (!aPrimitives.is_array())
      throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document primitives must be an array");
    std::size_t anExpectedIndex = 0;
    for (const json& aValue : aPrimitives)
    {
      if (!aValue.is_object() || !aValue.contains("indexStart") || !aValue.contains("indexCount")
          || !aValue.at("indexStart").is_number_unsigned()
          || !aValue.at("indexCount").is_number_unsigned())
        throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document primitive range is invalid");
      const std::size_t anIndexStart = aValue.at("indexStart").get<std::size_t>();
      const std::size_t anIndexCount = aValue.at("indexCount").get<std::size_t>();
      if (anIndexStart != anExpectedIndex || anIndexCount == 0 || anIndexCount % 3 != 0
          || anIndexStart > aMesh.indices.size()
          || anIndexCount > aMesh.indices.size() - anIndexStart)
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "OBJ document primitive ranges must partition indices");
      TriangleMesh::ObjPrimitive aPrimitive;
      aPrimitive.indexStart = static_cast<std::uint32_t>(anIndexStart);
      aPrimitive.indexCount = static_cast<std::uint32_t>(anIndexCount);
      if (aValue.contains("object"))
      {
        aPrimitive.object = checkedOBJText(aValue.at("object"), "object name");
        aPrimitive.hasObject = true;
      }
      if (aValue.contains("groups"))
      {
        if (!aValue.at("groups").is_array())
          throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document groups are invalid");
        for (const json& aGroup : aValue.at("groups"))
          aPrimitive.groups.push_back(checkedOBJText(aGroup, "group name"));
      }
      if (aValue.contains("material"))
      {
        aPrimitive.material = checkedOBJText(aValue.at("material"), "material name");
        aPrimitive.hasMaterial = true;
      }
      if (aValue.contains("smoothingGroup"))
      {
        aPrimitive.smoothingGroup = checkedOBJText(aValue.at("smoothingGroup"), "smoothing group");
        aPrimitive.hasSmoothingGroup = true;
      }
      aMesh.objPrimitives.push_back(std::move(aPrimitive));
      anExpectedIndex += anIndexCount;
    }
    if (!aMesh.objPrimitives.empty() && anExpectedIndex != aMesh.indices.size())
      throw KernelFailure(ErrorCode::InvalidArgs,
                          "OBJ document primitive ranges do not cover indices");
    const json aMaterials = aDocument.value("materials", json::array());
    const json aLibraries = aDocument.value("materialLibraries", json::array());
    if (!aMaterials.is_array() || !aLibraries.is_array())
      throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document materials are invalid");
    for (const json& aMaterial : aMaterials)
    {
      if (!aMaterial.is_object() || !aMaterial.contains("name")
          || !aMaterial.at("name").is_string()
          || aMaterial.at("name").get<std::string>().empty())
        throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document material is invalid");
      const auto hasNewline = [](const std::string& theValue) {
        return theValue.find('\n') != std::string::npos || theValue.find('\r') != std::string::npos;
      };
      if (hasNewline(aMaterial.at("name").get<std::string>()))
        throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document material name contains a newline");
      for (const char* aProperty : {"ambient", "diffuse", "specular", "emissive"})
      {
        if (!aMaterial.contains(aProperty)) continue;
        const json& aVector = aMaterial.at(aProperty);
        if (!aVector.is_array() || aVector.size() != 3)
          throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document material color is invalid");
        for (const json& aComponent : aVector)
          if (!aComponent.is_number() || !std::isfinite(aComponent.get<double>()))
            throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document material color is invalid");
      }
      for (const char* aProperty : {"opacity", "shininess", "opticalDensity"})
        if (aMaterial.contains(aProperty)
            && (!aMaterial.at(aProperty).is_number()
                || !std::isfinite(aMaterial.at(aProperty).get<double>())))
          throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document material number is invalid");
      if (aMaterial.contains("illuminationModel")
          && !aMaterial.at("illuminationModel").is_number_unsigned())
        throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document illumination model is invalid");
      for (const char* aProperty : {"diffuseMap", "specularMap", "opacityMap", "bumpMap"})
        if (aMaterial.contains(aProperty)
            && (!aMaterial.at(aProperty).is_string()
                || hasNewline(aMaterial.at(aProperty).get<std::string>())))
          throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document material map is invalid");
      aMesh.objMaterials.push_back(aMaterial);
    }
    for (const json& aLibrary : aLibraries)
      aMesh.objMaterialLibraries.push_back(checkedOBJText(aLibrary, "material library"));
    std::string aMaterialLibrary = theArgs.value("materialLibrary", std::string());
    if (aMaterialLibrary.empty() && !aMesh.objMaterials.empty())
      aMaterialLibrary = aMesh.objMaterialLibraries.empty()
        ? "materials.mtl" : aMesh.objMaterialLibraries.front();
    if (!aMaterialLibrary.empty())
    {
      if (aMaterialLibrary.find('\n') != std::string::npos
          || aMaterialLibrary.find('\r') != std::string::npos)
        throw KernelFailure(ErrorCode::InvalidArgs, "OBJ material library contains a newline");
      aMesh.objMaterialLibraries = {aMaterialLibrary};
    }
    aProgressScope.Next().Close();
    if (!aProgressScope.More())
      throw KernelFailure(ErrorCode::Cancelled, "OBJ document export cancelled");
    aProgressScope.Show();
    const std::string aData = writeOBJ(aMesh);
    aProgressScope.Next().Close();
    if (!aProgressScope.More())
      throw KernelFailure(ErrorCode::Cancelled, "OBJ document export cancelled");
    aProgressScope.Show();
    const std::uint32_t aDataBuffer = theContext.buffers().create(aData.size());
    if (!aData.empty()) std::memcpy(theContext.buffers().get(aDataBuffer).data(), aData.data(), aData.size());
    json aResources = json::array();
    if (!aMesh.objMaterials.empty())
    {
      const std::string aMTL = writeMTL(aMesh.objMaterials);
      const std::uint32_t aMTLBuffer = theContext.buffers().create(aMTL.size());
      if (!aMTL.empty()) std::memcpy(theContext.buffers().get(aMTLBuffer).data(), aMTL.data(), aMTL.size());
      aResources.push_back({{"uri", aMesh.objMaterialLibraries.front()},
                            {"data", bufferDescriptor(aMTLBuffer, aMTL.size(), "mtl-text")}});
    }
    aProgressScope.Next().Close();
    if (!aProgressScope.More())
      throw KernelFailure(ErrorCode::Cancelled, "OBJ document export cancelled");
    aProgressScope.Show();
    return {{"data", bufferDescriptor(aDataBuffer, aData.size(), "obj-text")},
            {"resources", std::move(aResources)}};
  }

json dispatchExportGLTFDocument(KernelOperationContext& theContext, const json& theArgs)
  {
    occ::handle<Message_ProgressIndicator> aProgress = new HostProgressIndicator();
    Message_ProgressScope aProgressScope(aProgress->Start(), "glTF document export", 3.0);
    aProgressScope.Show();
    if (!theArgs.at("document").is_object())
      throw KernelFailure(ErrorCode::InvalidArgs, "glTF document metadata is invalid");
    if (!theArgs.contains("buffers") || !theArgs.at("buffers").is_array())
      throw KernelFailure(ErrorCode::InvalidArgs, "glTF document buffers must be an array");
    std::vector<std::pair<std::string, std::vector<std::uint8_t>>> aBuffers;
    for (const json& aBuffer : theArgs.at("buffers"))
    {
      if (!aBuffer.is_object() || !aBuffer.contains("data") || !aBuffer.at("data").is_object()
          || (aBuffer.contains("uri") && !aBuffer.at("uri").is_string()))
        throw KernelFailure(ErrorCode::InvalidArgs, "glTF document buffer input is invalid");
      const std::string aData = inputBufferData(theContext.buffers(), aBuffer, "data");
      aBuffers.emplace_back(aBuffer.value("uri", std::string()),
                            std::vector<std::uint8_t>(aData.begin(), aData.end()));
    }
    std::vector<std::pair<std::string, std::vector<std::uint8_t>>> aResources;
    if (theArgs.contains("resources"))
    {
      if (!theArgs.at("resources").is_array())
        throw KernelFailure(ErrorCode::InvalidArgs, "glTF document resources must be an array");
      for (const json& aResource : theArgs.at("resources"))
      {
        if (!aResource.is_object() || !aResource.contains("uri")
            || !aResource.at("uri").is_string()
            || aResource.at("uri").get<std::string>().empty()
            || !aResource.contains("data") || !aResource.at("data").is_object())
          throw KernelFailure(ErrorCode::InvalidArgs, "glTF document resource input is invalid");
        const std::string aData = inputBufferData(theContext.buffers(), aResource, "data");
        aResources.emplace_back(aResource.at("uri").get<std::string>(),
                                std::vector<std::uint8_t>(aData.begin(), aData.end()));
      }
    }
    const std::string aFormat = theArgs.value("format", "glb");
    if (aFormat != "glb" && aFormat != "gltf")
      throw KernelFailure(ErrorCode::InvalidArgs, "glTF format must be glb or gltf");
    aProgressScope.Next().Close();
    if (!aProgressScope.More())
      throw KernelFailure(ErrorCode::Cancelled, "glTF document export cancelled");
    aProgressScope.Show();
    GltfDocumentExport anExport = writeGLTFDocument(
      theArgs.at("document"), aBuffers, aResources, aFormat == "glb");
    aProgressScope.Next().Close();
    if (!aProgressScope.More())
      throw KernelFailure(ErrorCode::Cancelled, "glTF document export cancelled");
    aProgressScope.Show();
    const std::uint32_t aDataBuffer = theContext.buffers().copy(anExport.data);
    json anExportedResources = json::array();
    for (const auto& aResource : anExport.resources)
    {
      anExportedResources.push_back({
        {"uri", aResource.first},
        {"data", bufferDescriptor(theContext.buffers().copy(aResource.second),
                                   aResource.second.size(), "u8")}
      });
    }
    aProgressScope.Next().Close();
    if (!aProgressScope.More())
      throw KernelFailure(ErrorCode::Cancelled, "glTF document export cancelled");
    aProgressScope.Show();
    return {{"data", bufferDescriptor(aDataBuffer, anExport.data.size(),
                                        aFormat == "glb" ? "glb" : "gltf-json")},
            {"resources", std::move(anExportedResources)}};
  }

} // namespace occt_worker
