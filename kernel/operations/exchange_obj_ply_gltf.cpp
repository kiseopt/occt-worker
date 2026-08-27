#include "kernel_exchange_mesh_operations.hpp"
#include "kernel_exchange_helpers.hpp"
#include "kernel_operation_context.hpp"
#include "kernel_protocol_helpers.hpp"

namespace occt_worker {

enum class TriangleMeshFormat
{
  Gltf,
  Obj,
  Ply
};

static json exportTriangleMesh(KernelOperationContext& theContext,
                               const TriangleMeshFormat theFormat,
                               const json& theArgs)
{
      const double aLinear = theArgs.value("linearDeflection", 0.1);
      const double anAngular = theArgs.value("angularDeflection", 0.5);
      const bool isRelative = theArgs.value("relative", false);
      occ::handle<Message_ProgressIndicator> aProgress = new HostProgressIndicator();
      Message_ProgressScope aProgressScope(aProgress->Start(), "mesh export", 3.0);
      aProgressScope.Show();
      TriangleMesh aMesh;
      if (theFormat == TriangleMeshFormat::Ply && theArgs.contains("positions"))
      {
        const std::string aPositionBytes = inputBufferData(theContext.buffers(), theArgs, "positions");
        const std::string anIndexBytes = inputBufferData(theContext.buffers(), theArgs, "indices");
        if (aPositionBytes.empty() || aPositionBytes.size() % (sizeof(float) * 3) != 0
            || anIndexBytes.empty() || anIndexBytes.size() % (sizeof(std::uint32_t) * 3) != 0)
          throw KernelFailure(ErrorCode::InvalidArgs, "PLY mesh buffers have invalid dimensions");
        const std::size_t aVertexCount = aPositionBytes.size() / (sizeof(float) * 3);
        aMesh.positions.reserve(aVertexCount);
        for (std::size_t aVertex = 0; aVertex < aVertexCount; ++aVertex)
        {
          std::array<float, 3> aPoint;
          std::memcpy(aPoint.data(), aPositionBytes.data() + aVertex * sizeof(float) * 3,
                      sizeof(float) * 3);
          const float x = aPoint[0];
          const float y = aPoint[1];
          const float z = aPoint[2];
          if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z))
            throw KernelFailure(ErrorCode::InvalidArgs, "PLY mesh positions must be finite");
          aMesh.positions.emplace_back(x, y, z);
        }
        aMesh.indices.resize(anIndexBytes.size() / sizeof(std::uint32_t));
        std::memcpy(aMesh.indices.data(), anIndexBytes.data(), anIndexBytes.size());
        for (const std::uint32_t anIndex : aMesh.indices)
          if (anIndex >= aVertexCount)
            throw KernelFailure(ErrorCode::InvalidArgs, "PLY mesh index is out of range");
        for (std::size_t aTriangle = 0; aTriangle < aMesh.indices.size(); aTriangle += 3)
        {
          const std::uint32_t aFirstIndex = aMesh.indices[aTriangle];
          const std::uint32_t aSecondIndex = aMesh.indices[aTriangle + 1];
          const std::uint32_t aThirdIndex = aMesh.indices[aTriangle + 2];
          if (aFirstIndex == aSecondIndex || aFirstIndex == aThirdIndex
              || aSecondIndex == aThirdIndex)
            throw KernelFailure(ErrorCode::InvalidArgs, "PLY mesh contains a repeated-index triangle");
          const gp_Pnt& aFirstPoint = aMesh.positions[aFirstIndex];
          const gp_Pnt& aSecondPoint = aMesh.positions[aSecondIndex];
          const gp_Pnt& aThirdPoint = aMesh.positions[aThirdIndex];
          gp_Vec aFirstEdge(aFirstPoint, aSecondPoint);
          gp_Vec aSecondEdge(aFirstPoint, aThirdPoint);
          aFirstEdge.Cross(aSecondEdge);
          if (aFirstEdge.SquareMagnitude() <= Precision::SquareConfusion())
            throw KernelFailure(ErrorCode::InvalidArgs, "PLY mesh contains a degenerate triangle");
        }
        if (theArgs.contains("normals"))
        {
          const std::string aNormalBytes = inputBufferData(theContext.buffers(), theArgs, "normals");
          if (aNormalBytes.size() != aVertexCount * sizeof(float) * 3)
            throw KernelFailure(ErrorCode::InvalidArgs, "PLY mesh normals must match positions");
          aMesh.normals.reserve(aVertexCount);
          for (std::size_t aVertex = 0; aVertex < aVertexCount; ++aVertex)
          {
            std::array<float, 3> aNormal;
            std::memcpy(aNormal.data(), aNormalBytes.data() + aVertex * sizeof(float) * 3,
                        sizeof(float) * 3);
            const float x = aNormal[0];
            const float y = aNormal[1];
            const float z = aNormal[2];
            if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z)
                || x * x + y * y + z * z <= Precision::SquareConfusion())
              throw KernelFailure(ErrorCode::InvalidArgs, "PLY mesh normals must be finite and non-zero");
            aMesh.normals.emplace_back(x, y, z);
          }
        }
        if (theArgs.contains("uvs"))
        {
          const std::string aUVBytes = inputBufferData(theContext.buffers(), theArgs, "uvs");
          if (aUVBytes.size() != aVertexCount * sizeof(float) * 2)
            throw KernelFailure(ErrorCode::InvalidArgs, "PLY mesh UVs must match positions");
          aMesh.uvs.reserve(aVertexCount);
          for (std::size_t aVertex = 0; aVertex < aVertexCount; ++aVertex)
          {
            std::array<float, 2> aUV;
            std::memcpy(aUV.data(), aUVBytes.data() + aVertex * sizeof(float) * 2,
                        sizeof(float) * 2);
            const float u = aUV[0];
            const float v = aUV[1];
            if (!std::isfinite(u) || !std::isfinite(v))
              throw KernelFailure(ErrorCode::InvalidArgs, "PLY mesh UVs must be finite");
            aMesh.uvs.emplace_back(u, v);
          }
        }
        if (theArgs.contains("colors"))
        {
          const std::string aColorBytes = inputBufferData(theContext.buffers(), theArgs, "colors");
          if (aColorBytes.size() != aVertexCount * 4)
            throw KernelFailure(ErrorCode::InvalidArgs, "PLY mesh colors must be u8 RGBA values matching positions");
          aMesh.colors.resize(aVertexCount);
          std::memcpy(aMesh.colors.data(), aColorBytes.data(), aColorBytes.size());
        }
        if (theArgs.contains("document"))
        {
          const json& aDocument = theArgs.at("document");
          if (!aDocument.is_object() || !aDocument.contains("comments")
              || !aDocument.at("comments").is_array() || !aDocument.contains("objectInfo")
              || !aDocument.at("objectInfo").is_array())
            throw KernelFailure(ErrorCode::InvalidArgs, "PLY document metadata is invalid");
          const auto appendLines = [](const json& theLines,
                                      std::vector<std::string>& theTarget) {
            for (const json& aLine : theLines)
            {
              if (!aLine.is_string())
                throw KernelFailure(ErrorCode::InvalidArgs, "PLY document metadata is invalid");
              const std::string aValue = aLine.get<std::string>();
              if (aValue.find('\n') != std::string::npos || aValue.find('\r') != std::string::npos)
                throw KernelFailure(ErrorCode::InvalidArgs,
                                    "PLY document metadata must contain single-line values");
              theTarget.push_back(aValue);
            }
          };
          appendLines(aDocument.at("comments"), aMesh.plyComments);
          appendLines(aDocument.at("objectInfo"), aMesh.plyObjectInfo);
          aMesh.hasPlyDocument = true;
        }
      }
      else
      {
        aMesh = buildTriangleMesh(
          theContext.arena().get(requiredU32(theArgs, "shape")), aLinear, anAngular, isRelative);
      }
      aProgressScope.Next().Close();
      if (!aProgressScope.More())
        throw KernelFailure(ErrorCode::Cancelled, "Mesh export cancelled");
      aProgressScope.Show();
      if (theFormat == TriangleMeshFormat::Gltf)
      {
        const std::string aFormat = theArgs.value("format", "glb");
        if (aFormat != "glb" && aFormat != "gltf")
        {
          throw KernelFailure(ErrorCode::InvalidArgs, "glTF format must be glb or gltf");
        }
        const std::vector<std::uint8_t> aData = writeGLTF(aMesh, aFormat == "glb");
        aProgressScope.Next().Close();
        if (!aProgressScope.More())
          throw KernelFailure(ErrorCode::Cancelled, "glTF export cancelled");
        aProgressScope.Show();
        const std::uint32_t aBuffer = theContext.buffers().create(aData.size());
        if (!aData.empty())
        {
          std::memcpy(theContext.buffers().get(aBuffer).data(), aData.data(), aData.size());
        }
        aProgressScope.Next().Close();
        if (!aProgressScope.More())
          throw KernelFailure(ErrorCode::Cancelled, "glTF export cancelled");
        aProgressScope.Show();
        return {{"data", bufferDescriptor(aBuffer,
                                           aData.size(),
                                           aFormat == "glb" ? "glb" : "gltf-json")}};
      }
      const std::string aPLYEncoding = theArgs.value("encoding", "ascii");
      if (theFormat == TriangleMeshFormat::Ply && aPLYEncoding != "ascii"
          && aPLYEncoding != "binary_little_endian"
          && aPLYEncoding != "binary_big_endian")
      {
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "PLY encoding must be ascii, binary_little_endian, or binary_big_endian");
      }
      const std::string aData = theFormat == TriangleMeshFormat::Obj
        ? writeOBJ(aMesh)
        : writePLY(aMesh, aPLYEncoding);
      aProgressScope.Next().Close();
      if (!aProgressScope.More())
        throw KernelFailure(ErrorCode::Cancelled, "Mesh export cancelled");
      aProgressScope.Show();
      const std::uint32_t aBuffer = theContext.buffers().create(aData.size());
      if (!aData.empty())
      {
        std::memcpy(theContext.buffers().get(aBuffer).data(), aData.data(), aData.size());
      }
      aProgressScope.Next().Close();
      if (!aProgressScope.More())
        throw KernelFailure(ErrorCode::Cancelled, "Mesh export cancelled");
      aProgressScope.Show();
      return {{"data", bufferDescriptor(aBuffer,
                                         aData.size(),
                                         theFormat == TriangleMeshFormat::Obj
                                           ? "obj-text"
                                           : (aPLYEncoding == "ascii"
                                                ? "ply-ascii"
                                                : (aPLYEncoding == "binary_big_endian"
                                                     ? "ply-binary_big_endian"
                                                     : "ply-binary_little_endian")))}};
    }

json h_exportGLTF(KernelOperationContext& theContext, const json& theArgs)
{
  if (theArgs.contains("document"))
    return dispatchExportGLTFDocument(theContext, theArgs);
  return exportTriangleMesh(theContext, TriangleMeshFormat::Gltf, theArgs);
}

json h_exportOBJ(KernelOperationContext& theContext, const json& theArgs)
{
  if (theArgs.contains("positions"))
    return dispatchExportOBJDocument(theContext, theArgs);
  return exportTriangleMesh(theContext, TriangleMeshFormat::Obj, theArgs);
}

json h_exportPLY(KernelOperationContext& theContext, const json& theArgs)
{
  return exportTriangleMesh(theContext, TriangleMeshFormat::Ply, theArgs);
}

static json importTriangleMesh(KernelOperationContext& theContext,
                               const TriangleMeshFormat theFormat,
                               const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const std::string aData = inputBufferData(theContext.buffers(), theArgs, "data");
      occ::handle<Message_ProgressIndicator> aProgress = new HostProgressIndicator();
      Message_ProgressScope aProgressScope(aProgress->Start(), "mesh import", 3.0);
      aProgressScope.Show();
      TriangleMesh aMesh;
      if (theFormat == TriangleMeshFormat::Gltf)
      {
        std::unordered_map<std::string, std::string> aResources;
        std::vector<double> aMorphWeights;
        const std::vector<double>* aMorphWeightsPointer = nullptr;
        std::size_t anAnimationIndex = 0;
        double anAnimationTime = 0.0;
        const std::size_t* anAnimationIndexPointer = nullptr;
        const double* anAnimationTimePointer = nullptr;
        if (theArgs.contains("animationIndex") != theArgs.contains("animationTime"))
          throw KernelFailure(ErrorCode::InvalidArgs,
                              "glTF animationIndex and animationTime must be supplied together");
        if (theArgs.contains("animationIndex"))
        {
          if (!theArgs.at("animationIndex").is_number_unsigned())
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "glTF animationIndex must be a non-negative integer");
          const std::uint64_t anIndex = theArgs.at("animationIndex").get<std::uint64_t>();
          if (anIndex > std::numeric_limits<std::size_t>::max())
            throw KernelFailure(ErrorCode::InvalidArgs, "glTF animationIndex is too large");
          if (!theArgs.at("animationTime").is_number())
            throw KernelFailure(ErrorCode::InvalidArgs, "glTF animationTime must be a number");
          anAnimationIndex = static_cast<std::size_t>(anIndex);
          anAnimationTime = theArgs.at("animationTime").get<double>();
          if (!std::isfinite(anAnimationTime))
            throw KernelFailure(ErrorCode::InvalidArgs, "glTF animationTime must be finite");
          anAnimationIndexPointer = &anAnimationIndex;
          anAnimationTimePointer = &anAnimationTime;
        }
        if (theArgs.contains("morphWeights"))
        {
          if (!theArgs.at("morphWeights").is_array())
            throw KernelFailure(ErrorCode::InvalidArgs, "glTF morphWeights must be an array");
          for (const json& aWeight : theArgs.at("morphWeights"))
          {
            if (!aWeight.is_number())
              throw KernelFailure(ErrorCode::InvalidArgs, "glTF morphWeights must contain numbers");
            const double aValue = aWeight.get<double>();
            if (!std::isfinite(aValue))
              throw KernelFailure(ErrorCode::InvalidArgs, "glTF morphWeights must be finite");
            aMorphWeights.push_back(aValue);
          }
          aMorphWeightsPointer = &aMorphWeights;
        }
        if (theArgs.contains("resources"))
        {
          if (!theArgs.at("resources").is_array())
            throw KernelFailure(ErrorCode::InvalidArgs, "glTF resources must be an array");
          for (const json& aResource : theArgs.at("resources"))
          {
            if (!aResource.is_object() || !aResource.contains("uri")
                || !aResource.at("uri").is_string() || aResource.at("uri").get<std::string>().empty()
                || !aResource.contains("data") || !aResource.at("data").is_object())
              throw KernelFailure(ErrorCode::InvalidArgs, "glTF resources require a non-empty uri and buffer data");
            const std::string aUri = aResource.at("uri").get<std::string>();
            if (!aResources.emplace(aUri, inputBufferData(theContext.buffers(), aResource, "data")).second)
              throw KernelFailure(ErrorCode::InvalidArgs, "glTF resource URIs must be unique");
          }
        }
        aMesh = parseGLTF(aData, aResources, theArgs.value("includeDocument", false),
                          aMorphWeightsPointer, anAnimationIndexPointer,
                          anAnimationTimePointer);
      }
      else if (theFormat == TriangleMeshFormat::Obj)
      {
        std::unordered_map<std::string, std::string> aResources;
        if (theArgs.contains("resources"))
        {
          if (!theArgs.at("resources").is_array())
            throw KernelFailure(ErrorCode::InvalidArgs, "OBJ resources must be an array");
          for (const json& aResource : theArgs.at("resources"))
          {
            if (!aResource.is_object() || !aResource.contains("uri")
                || !aResource.at("uri").is_string() || aResource.at("uri").get<std::string>().empty()
                || !aResource.contains("data") || !aResource.at("data").is_object())
              throw KernelFailure(ErrorCode::InvalidArgs, "OBJ resources require a non-empty uri and buffer data");
            const std::string aUri = aResource.at("uri").get<std::string>();
            if (!aResources.emplace(aUri, inputBufferData(theContext.buffers(), aResource, "data")).second)
              throw KernelFailure(ErrorCode::InvalidArgs, "OBJ resource URIs must be unique");
          }
        }
        aMesh = parseOBJ(aData, aResources);
      }
      else
      {
        aMesh = parsePLY(aData);
      }
      aProgressScope.Next().Close();
      if (!aProgressScope.More())
        throw KernelFailure(ErrorCode::Cancelled, "Mesh import cancelled");
      aProgressScope.Show();
      json aResult = json::object();
      if (!aMesh.indices.empty())
        aResult["shape"] = theContext.arena().add(makeTriangleShape(aMesh), aScope);
      const bool includeMesh = theFormat == TriangleMeshFormat::Ply
        ? theArgs.value("includeMesh", false)
        : theArgs.value("includeDocument", false);
      if (includeMesh)
      {
        std::vector<float> aPositions;
        std::vector<float> aNormals;
        std::vector<float> aUVs;
        aPositions.reserve(aMesh.positions.size() * 3);
        for (const gp_Pnt& aPoint : aMesh.positions)
          aPositions.insert(aPositions.end(), {static_cast<float>(aPoint.X()),
                                               static_cast<float>(aPoint.Y()),
                                               static_cast<float>(aPoint.Z())});
        for (const gp_Dir& aNormal : aMesh.normals)
          aNormals.insert(aNormals.end(), {static_cast<float>(aNormal.X()),
                                           static_cast<float>(aNormal.Y()),
                                           static_cast<float>(aNormal.Z())});
        for (const gp_Pnt2d& aUV : aMesh.uvs)
          aUVs.insert(aUVs.end(), {static_cast<float>(aUV.X()), static_cast<float>(aUV.Y())});
        aResult["positions"] = bufferDescriptor(
          theContext.buffers().copy(aPositions), aPositions.size() * sizeof(float), "f32x3");
        aResult["indices"] = bufferDescriptor(
          theContext.buffers().copy(aMesh.indices), aMesh.indices.size() * sizeof(std::uint32_t), "u32");
        if (!aNormals.empty())
          aResult["normals"] = bufferDescriptor(
            theContext.buffers().copy(aNormals), aNormals.size() * sizeof(float), "f32x3");
        if (!aUVs.empty())
          aResult["uvs"] = bufferDescriptor(
            theContext.buffers().copy(aUVs), aUVs.size() * sizeof(float), "f32x2");
        if (!aMesh.colors.empty())
          aResult["colors"] = bufferDescriptor(
            theContext.buffers().copy(aMesh.colors), aMesh.colors.size() * 4, "u8x4");
        if (theFormat == TriangleMeshFormat::Ply)
        {
          aResult["document"] = {
            {"comments", aMesh.plyComments},
            {"objectInfo", aMesh.plyObjectInfo}
          };
        }
        else if (theFormat == TriangleMeshFormat::Obj)
        {
          json aPrimitives = json::array();
          for (const TriangleMesh::ObjPrimitive& aPrimitive : aMesh.objPrimitives)
            aPrimitives.push_back(aPrimitive.toJson());
          aResult["document"] = {
            {"primitives", std::move(aPrimitives)},
            {"materials", aMesh.objMaterials},
            {"materialLibraries", aMesh.objMaterialLibraries}
          };
        }
        else if (theFormat == TriangleMeshFormat::Gltf)
        {
          json aPrimitives = json::array();
          for (const TriangleMesh::GltfPrimitiveInstance& aPrimitive : aMesh.gltfPrimitives)
            aPrimitives.push_back(aPrimitive.toJson());
          json aBuffers = json::array();
          static const json anEmptyArray = json::array();
          const json& aDefinitions = aMesh.gltfDocument.contains("buffers")
            ? aMesh.gltfDocument.at("buffers") : anEmptyArray;
          for (std::size_t anIndex = 0; anIndex < aMesh.gltfBuffers.size(); ++anIndex)
          {
            const std::vector<std::uint8_t>& aBytes = aMesh.gltfBuffers[anIndex];
            json aResolved{{"data", bufferDescriptor(
              theContext.buffers().copy(aBytes), aBytes.size(), "u8")}};
            if (anIndex < aDefinitions.size() && aDefinitions.at(anIndex).contains("uri"))
              aResolved["uri"] = aDefinitions.at(anIndex).at("uri");
            aBuffers.push_back(std::move(aResolved));
          }
          json aResources = json::array();
          for (const auto& aResource : aMesh.gltfResources)
          {
            aResources.push_back({
              {"uri", aResource.first},
              {"data", bufferDescriptor(
                theContext.buffers().copy(aResource.second), aResource.second.size(), "u8")}
            });
          }
          aResult["document"] = aMesh.gltfDocument;
          if (aMesh.hasGltfActiveScene) aResult["activeScene"] = aMesh.gltfActiveScene;
          aResult["sceneRoots"] = aMesh.gltfSceneRoots;
          aResult["primitives"] = std::move(aPrimitives);
          aResult["buffers"] = std::move(aBuffers);
            aResult["resources"] = std::move(aResources);
        }
      }
      aProgressScope.Next().Close();
      if (!aProgressScope.More())
        throw KernelFailure(ErrorCode::Cancelled, "Mesh import cancelled");
      aProgressScope.Show();
      aProgressScope.Next().Close();
      if (!aProgressScope.More())
        throw KernelFailure(ErrorCode::Cancelled, "Mesh import cancelled");
      aProgressScope.Show();
      return aResult;
    }

json h_importGLTF(KernelOperationContext& theContext, const json& theArgs)
{
  return importTriangleMesh(theContext, TriangleMeshFormat::Gltf, theArgs);
}

json h_importOBJ(KernelOperationContext& theContext, const json& theArgs)
{
  return importTriangleMesh(theContext, TriangleMeshFormat::Obj, theArgs);
}

json h_importPLY(KernelOperationContext& theContext, const json& theArgs)
{
  return importTriangleMesh(theContext, TriangleMeshFormat::Ply, theArgs);
}

} // namespace occt_worker
