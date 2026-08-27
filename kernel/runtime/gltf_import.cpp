#include "kernel_exchange_helpers.hpp"

namespace occt_worker {

TriangleMesh parseGLTF(
    const std::string& theData,
    const std::unordered_map<std::string, std::string>& theResources,
    const bool theAllowNoTriangles,
    const std::vector<double>* theMorphWeights,
    const std::size_t* theAnimationIndex,
    const double* theAnimationTime)
  {
    std::string aJsonText;
    std::vector<std::uint8_t> aGlbBuffer;
    bool hasGlbBuffer = false;
    if (theData.size() >= 4 && readU32LE(theData, 0) == 0x46546c67)
    {
      if (theData.size() < 12 || readU32LE(theData, 4) != 2
          || readU32LE(theData, 8) != theData.size())
      {
        throw KernelFailure(ErrorCode::ImportExportFailed, "GLB header is invalid");
      }
      std::size_t anOffset = 12;
      while (anOffset < theData.size())
      {
        if (theData.size() - anOffset < 8)
        {
          throw KernelFailure(ErrorCode::ImportExportFailed, "GLB chunk header is truncated");
        }
        const std::size_t aLength = readU32LE(theData, anOffset);
        const std::uint32_t aType = readU32LE(theData, anOffset + 4);
        anOffset += 8;
        if (aLength > theData.size() - anOffset)
        {
          throw KernelFailure(ErrorCode::ImportExportFailed, "GLB chunk is truncated");
        }
        if (aType == 0x4e4f534a && aJsonText.empty())
        {
          aJsonText.assign(theData.data() + anOffset, aLength);
          while (!aJsonText.empty() && aJsonText.back() == '\0') aJsonText.pop_back();
        }
        else if (aType == 0x004e4942 && !hasGlbBuffer)
        {
          const auto* aStart = reinterpret_cast<const std::uint8_t*>(theData.data() + anOffset);
          aGlbBuffer.assign(aStart, aStart + aLength);
          hasGlbBuffer = true;
        }
        anOffset += aLength;
      }
    }
    else
    {
      aJsonText = theData;
      if (aJsonText.size() >= 3
          && static_cast<std::uint8_t>(aJsonText[0]) == 0xef
          && static_cast<std::uint8_t>(aJsonText[1]) == 0xbb
          && static_cast<std::uint8_t>(aJsonText[2]) == 0xbf)
      {
        aJsonText.erase(0, 3);
      }
    }
    if (aJsonText.empty())
    {
      throw KernelFailure(ErrorCode::ImportExportFailed, "glTF JSON document is missing");
    }

    const json aDocument = json::parse(aJsonText);
    if (!aDocument.contains("asset")
        || aDocument.at("asset").value("version", "") != "2.0")
    {
      throw KernelFailure(ErrorCode::ImportExportFailed, "Only glTF 2.0 is supported");
    }
    if (aDocument.contains("extensionsRequired"))
    {
      if (!aDocument.at("extensionsRequired").is_array())
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF extensionsRequired must be an array");
      for (const json& anExtensionValue : aDocument.at("extensionsRequired"))
      {
        if (!anExtensionValue.is_string())
          throw KernelFailure(ErrorCode::ImportExportFailed, "glTF required extension names must be strings");
        const std::string anExtension = anExtensionValue.get<std::string>();
        if (anExtension == "KHR_draco_mesh_compression"
            || anExtension == "EXT_meshopt_compression"
            || anExtension == "KHR_mesh_quantization"
            || anExtension == "EXT_mesh_gpu_instancing")
        {
          throw KernelFailure(ErrorCode::ImportExportFailed,
                              "glTF required geometry extension is unsupported: " + anExtension);
        }
      }
    }
    if (aDocument.contains("buffers") && !aDocument.at("buffers").is_array())
    {
      throw KernelFailure(ErrorCode::ImportExportFailed, "glTF buffers must be an array");
    }
    std::vector<std::vector<std::uint8_t>> aBuffers;
    bool hasUsedGlbBuffer = false;
    static const json anEmptyArray = json::array();
    const json& aBufferDefinitions = aDocument.contains("buffers")
      ? aDocument.at("buffers") : anEmptyArray;
    std::unordered_set<std::string> aBufferUris;
    for (std::size_t aBufferIndex = 0; aBufferIndex < aBufferDefinitions.size(); ++aBufferIndex)
    {
      const json& aBuffer = aBufferDefinitions.at(aBufferIndex);
      if (!aBuffer.is_object() || !aBuffer.contains("byteLength")
          || !aBuffer.at("byteLength").is_number_unsigned())
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF buffer definition is invalid");
      std::vector<std::uint8_t> aBytes;
      if (aBuffer.contains("uri"))
      {
        if (!aBuffer.at("uri").is_string())
          throw KernelFailure(ErrorCode::ImportExportFailed, "glTF buffer URI must be a string");
        const std::string aUri = aBuffer.at("uri").get<std::string>();
        if (aUri.rfind("data:", 0) == 0)
          aBytes = decodeGltfDataUri(aUri);
        else
        {
          const auto aResource = theResources.find(aUri);
          if (aResource == theResources.end())
            throw KernelFailure(ErrorCode::ImportExportFailed,
                                "glTF external resource is missing: " + aUri);
          aBytes.assign(aResource->second.begin(), aResource->second.end());
          aBufferUris.insert(aUri);
        }
      }
      else if (aBufferIndex == 0 && !hasUsedGlbBuffer && hasGlbBuffer)
      {
        aBytes = aGlbBuffer;
        hasUsedGlbBuffer = true;
      }
      else
      {
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF buffer has no embedded data");
      }
      const std::size_t aDeclaredLength = aBuffer.at("byteLength").get<std::size_t>();
      if (aDeclaredLength > aBytes.size())
      {
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF buffer is truncated");
      }
      aBytes.resize(aDeclaredLength);
      aBuffers.push_back(std::move(aBytes));
    }

    std::vector<GltfEvaluatedNode> anEvaluatedNodes = evaluateGLTFAnimation(
      aDocument, aBuffers, theAnimationIndex, theAnimationTime);
    TriangleMesh aMesh;
    bool hasAnyNormals = false;
    bool hasAllNormals = true;
    bool hasAnyUVs = false;
    bool hasAllUVs = true;
    const GltfMatrix anIdentity = {
      1.0, 0.0, 0.0, 0.0,
      0.0, 1.0, 0.0, 0.0,
      0.0, 0.0, 1.0, 0.0,
      0.0, 0.0, 0.0, 1.0
    };
    const std::size_t noNode = std::numeric_limits<std::size_t>::max();
    const auto appendMesh = [&](const std::size_t theMeshIndex,
                                const std::size_t theNodeIndex,
                                const GltfMatrix& theTransform,
                                const std::vector<GltfMatrix>* theSkinMatrices) {
      const json& aMeshes = aDocument.at("meshes");
      if (!aMeshes.is_array() || theMeshIndex >= aMeshes.size())
      {
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF mesh index is out of range");
      }
      const json& aMeshDefinition = aMeshes.at(theMeshIndex);
      const json& aPrimitives = aMeshDefinition.at("primitives");
      if (!aPrimitives.is_array())
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF mesh primitives must be an array");
      for (std::size_t aPrimitiveIndex = 0; aPrimitiveIndex < aPrimitives.size(); ++aPrimitiveIndex)
      {
        const json& aPrimitive = aPrimitives.at(aPrimitiveIndex);
        const int aMode = aPrimitive.value("mode", 4);
        if (aMode != 4 && aMode != 5 && aMode != 6) continue;
        const json& anAttributes = aPrimitive.at("attributes");
        if (!anAttributes.contains("POSITION")) continue;
        const GltfAccessor aPositions = gltfAccessor(
          aDocument, aBuffers, anAttributes.at("POSITION").get<std::size_t>(), "VEC3");
        if (aPositions.componentType != 5126)
        {
          throw KernelFailure(ErrorCode::ImportExportFailed, "glTF POSITION accessor must use float32");
        }
        const std::size_t aBase = aMesh.positions.size();
        if (aBase > std::numeric_limits<std::uint32_t>::max()
            || aPositions.count > std::numeric_limits<std::uint32_t>::max() - aBase)
          throw KernelFailure(ErrorCode::ImportExportFailed, "glTF expanded mesh has too many vertices");
        GltfAccessor aNormals;
        const bool hasNormals = anAttributes.contains("NORMAL");
        if (hasNormals)
        {
          aNormals = gltfAccessor(
            aDocument, aBuffers, anAttributes.at("NORMAL").get<std::size_t>(), "VEC3");
          if (aNormals.count != aPositions.count
              || (aNormals.componentType != 5126
                  && !((aNormals.componentType == 5120 || aNormals.componentType == 5122)
                       && aNormals.normalized)))
            throw KernelFailure(ErrorCode::ImportExportFailed, "glTF NORMAL accessor is unsupported");
          hasAnyNormals = true;
        }
        else hasAllNormals = false;
        GltfAccessor aUVs;
        const bool hasUVs = anAttributes.contains("TEXCOORD_0");
        if (hasUVs)
        {
          aUVs = gltfAccessor(
            aDocument, aBuffers, anAttributes.at("TEXCOORD_0").get<std::size_t>(), "VEC2");
          if (aUVs.count != aPositions.count
              || (aUVs.componentType != 5126
                  && !((aUVs.componentType == 5121 || aUVs.componentType == 5123)
                       && aUVs.normalized)))
            throw KernelFailure(ErrorCode::ImportExportFailed, "glTF TEXCOORD_0 accessor is unsupported");
          hasAnyUVs = true;
        }
        else hasAllUVs = false;
        GltfAccessor aJoints;
        GltfAccessor aWeights;
        if (theSkinMatrices != nullptr)
        {
          if (!anAttributes.contains("JOINTS_0") || !anAttributes.contains("WEIGHTS_0"))
            throw KernelFailure(ErrorCode::ImportExportFailed,
                                "glTF skinned primitive requires JOINTS_0 and WEIGHTS_0");
          aJoints = gltfAccessor(
            aDocument, aBuffers, anAttributes.at("JOINTS_0").get<std::size_t>(), "VEC4");
          aWeights = gltfAccessor(
            aDocument, aBuffers, anAttributes.at("WEIGHTS_0").get<std::size_t>(), "VEC4");
          if (aJoints.count != aPositions.count || aJoints.normalized
              || (aJoints.componentType != 5121 && aJoints.componentType != 5123))
            throw KernelFailure(ErrorCode::ImportExportFailed,
                                "glTF JOINTS_0 accessor is unsupported");
          if (aWeights.count != aPositions.count
              || (aWeights.componentType != 5126
                  && !((aWeights.componentType == 5121 || aWeights.componentType == 5123)
                       && aWeights.normalized)))
            throw KernelFailure(ErrorCode::ImportExportFailed,
                                "glTF WEIGHTS_0 accessor is unsupported");
        }
        static const json anEmptyTargets = json::array();
        const json& aTargets = aPrimitive.contains("targets")
          ? aPrimitive.at("targets") : anEmptyTargets;
        if (!aTargets.is_array())
          throw KernelFailure(ErrorCode::ImportExportFailed, "glTF morph targets must be an array");
        std::vector<double> aMorphWeights(aTargets.size(), 0.0);
        const json* aDocumentWeights = nullptr;
        const std::vector<double>* anAnimatedWeights = nullptr;
        if (theNodeIndex != noNode)
        {
          const json& aNode = aDocument.at("nodes").at(theNodeIndex);
          if (aNode.contains("weights")) aDocumentWeights = &aNode.at("weights");
          if (!anEvaluatedNodes.empty() && anEvaluatedNodes.at(theNodeIndex).hasWeights)
            anAnimatedWeights = &anEvaluatedNodes.at(theNodeIndex).weights;
        }
        if (aDocumentWeights == nullptr && aMeshDefinition.contains("weights"))
          aDocumentWeights = &aMeshDefinition.at("weights");
        if (theMorphWeights != nullptr)
        {
          if (theMorphWeights->size() != aTargets.size())
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "glTF morphWeights must match the primitive target count");
          aMorphWeights = *theMorphWeights;
        }
        else if (anAnimatedWeights != nullptr)
        {
          if (anAnimatedWeights->size() != aTargets.size())
            throw KernelFailure(ErrorCode::ImportExportFailed,
                                "glTF animated weights must match the primitive target count");
          aMorphWeights = *anAnimatedWeights;
        }
        else if (aDocumentWeights != nullptr)
        {
          if (!aDocumentWeights->is_array() || aDocumentWeights->size() != aTargets.size())
            throw KernelFailure(ErrorCode::ImportExportFailed,
                                "glTF morph weights must match the primitive target count");
          for (std::size_t aTargetIndex = 0; aTargetIndex < aTargets.size(); ++aTargetIndex)
          {
            if (!aDocumentWeights->at(aTargetIndex).is_number())
              throw KernelFailure(ErrorCode::ImportExportFailed, "glTF morph weight is invalid");
            aMorphWeights[aTargetIndex] = aDocumentWeights->at(aTargetIndex).get<double>();
            if (!std::isfinite(aMorphWeights[aTargetIndex]))
              throw KernelFailure(ErrorCode::ImportExportFailed, "glTF morph weight is not finite");
          }
        }
        std::vector<GltfAccessor> aPositionTargets(aTargets.size());
        std::vector<GltfAccessor> aNormalTargets(aTargets.size());
        std::vector<bool> hasPositionTarget(aTargets.size(), false);
        std::vector<bool> hasNormalTarget(aTargets.size(), false);
        for (std::size_t aTargetIndex = 0; aTargetIndex < aTargets.size(); ++aTargetIndex)
        {
          const json& aTarget = aTargets.at(aTargetIndex);
          if (!aTarget.is_object())
            throw KernelFailure(ErrorCode::ImportExportFailed, "glTF morph target is invalid");
          if (aTarget.contains("POSITION"))
          {
            aPositionTargets[aTargetIndex] = gltfAccessor(
              aDocument, aBuffers, aTarget.at("POSITION").get<std::size_t>(), "VEC3");
            if (aPositionTargets[aTargetIndex].count != aPositions.count
                || aPositionTargets[aTargetIndex].componentType != 5126)
              throw KernelFailure(ErrorCode::ImportExportFailed,
                                  "glTF POSITION morph target accessor is unsupported");
            hasPositionTarget[aTargetIndex] = true;
          }
          if (aTarget.contains("NORMAL"))
          {
            if (!hasNormals)
              throw KernelFailure(ErrorCode::ImportExportFailed,
                                  "glTF NORMAL morph target requires base normals");
            aNormalTargets[aTargetIndex] = gltfAccessor(
              aDocument, aBuffers, aTarget.at("NORMAL").get<std::size_t>(), "VEC3");
            if (aNormalTargets[aTargetIndex].count != aPositions.count
                || aNormalTargets[aTargetIndex].componentType != 5126)
              throw KernelFailure(ErrorCode::ImportExportFailed,
                                  "glTF NORMAL morph target accessor is unsupported");
            hasNormalTarget[aTargetIndex] = true;
          }
        }
        const std::size_t anIndexStart = aMesh.indices.size();
        for (std::size_t aPosition = 0; aPosition < aPositions.count; ++aPosition)
        {
          double x = gltfAccessorValue(aPositions, aPosition, 0);
          double y = gltfAccessorValue(aPositions, aPosition, 1);
          double z = gltfAccessorValue(aPositions, aPosition, 2);
          for (std::size_t aTargetIndex = 0; aTargetIndex < aTargets.size(); ++aTargetIndex)
          {
            if (!hasPositionTarget[aTargetIndex]) continue;
            const double aTargetX = gltfAccessorValue(aPositionTargets[aTargetIndex], aPosition, 0);
            const double aTargetY = gltfAccessorValue(aPositionTargets[aTargetIndex], aPosition, 1);
            const double aTargetZ = gltfAccessorValue(aPositionTargets[aTargetIndex], aPosition, 2);
            if (!std::isfinite(aTargetX) || !std::isfinite(aTargetY) || !std::isfinite(aTargetZ))
              throw KernelFailure(ErrorCode::ImportExportFailed,
                                  "glTF POSITION morph target contains a non-finite value");
            x += aMorphWeights[aTargetIndex] * aTargetX;
            y += aMorphWeights[aTargetIndex] * aTargetY;
            z += aMorphWeights[aTargetIndex] * aTargetZ;
          }
          if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z))
          {
            throw KernelFailure(ErrorCode::ImportExportFailed, "glTF POSITION contains a non-finite value");
          }
          GltfMatrix aVertexTransform = theTransform;
          if (theSkinMatrices != nullptr)
          {
            GltfMatrix aBlendedTransform{};
            double aWeightSum = 0.0;
            for (std::size_t aComponent = 0; aComponent < 4; ++aComponent)
            {
              const double aJointValue = gltfAccessorValue(aJoints, aPosition, aComponent);
              const double aWeight = gltfAccessorValue(aWeights, aPosition, aComponent);
              if (!std::isfinite(aJointValue) || !std::isfinite(aWeight) || aWeight < 0.0)
                throw KernelFailure(ErrorCode::ImportExportFailed,
                                    "glTF skin joint or weight is invalid");
              const std::size_t aJointIndex = static_cast<std::size_t>(aJointValue);
              if (aJointIndex >= theSkinMatrices->size())
                throw KernelFailure(ErrorCode::ImportExportFailed,
                                    "glTF skin joint index is out of range");
              aWeightSum += aWeight;
              for (std::size_t anEntry = 0; anEntry < 16; ++anEntry)
                aBlendedTransform[anEntry] += aWeight * theSkinMatrices->at(aJointIndex)[anEntry];
            }
            if (!std::isfinite(aWeightSum)
                || aWeightSum <= std::numeric_limits<double>::epsilon())
              throw KernelFailure(ErrorCode::ImportExportFailed,
                                  "glTF skin weights must have a positive sum");
            for (double& anEntry : aBlendedTransform) anEntry /= aWeightSum;
            aVertexTransform = aBlendedTransform;
          }
          aMesh.positions.push_back(transformGltfPoint(aVertexTransform, x, y, z));
          if (hasNormals)
          {
            double normalX = gltfAccessorValue(aNormals, aPosition, 0);
            double normalY = gltfAccessorValue(aNormals, aPosition, 1);
            double normalZ = gltfAccessorValue(aNormals, aPosition, 2);
            for (std::size_t aTargetIndex = 0; aTargetIndex < aTargets.size(); ++aTargetIndex)
            {
              if (!hasNormalTarget[aTargetIndex]) continue;
              const double aTargetX = gltfAccessorValue(aNormalTargets[aTargetIndex], aPosition, 0);
              const double aTargetY = gltfAccessorValue(aNormalTargets[aTargetIndex], aPosition, 1);
              const double aTargetZ = gltfAccessorValue(aNormalTargets[aTargetIndex], aPosition, 2);
              if (!std::isfinite(aTargetX) || !std::isfinite(aTargetY) || !std::isfinite(aTargetZ))
                throw KernelFailure(ErrorCode::ImportExportFailed,
                                    "glTF NORMAL morph target contains a non-finite value");
              normalX += aMorphWeights[aTargetIndex] * aTargetX;
              normalY += aMorphWeights[aTargetIndex] * aTargetY;
              normalZ += aMorphWeights[aTargetIndex] * aTargetZ;
            }
            aMesh.normals.push_back(transformGltfNormal(
              aVertexTransform, normalX, normalY, normalZ));
          }
          else aMesh.normals.emplace_back(0.0, 0.0, 1.0);
          if (hasUVs)
          {
            const double u = gltfAccessorValue(aUVs, aPosition, 0);
            const double v = gltfAccessorValue(aUVs, aPosition, 1);
            if (!std::isfinite(u) || !std::isfinite(v))
              throw KernelFailure(ErrorCode::ImportExportFailed, "glTF TEXCOORD_0 contains a non-finite value");
            aMesh.uvs.emplace_back(u, v);
          }
          else aMesh.uvs.emplace_back(0.0, 0.0);
        }

        std::vector<std::uint32_t> anIndices;
        if (aPrimitive.contains("indices"))
        {
          const GltfAccessor anAccessor = gltfAccessor(
            aDocument, aBuffers, aPrimitive.at("indices").get<std::size_t>(), "SCALAR");
          if (anAccessor.componentType != 5121 && anAccessor.componentType != 5123
              && anAccessor.componentType != 5125)
          {
            throw KernelFailure(ErrorCode::ImportExportFailed, "glTF index component type is unsupported");
          }
          if (anAccessor.normalized)
            throw KernelFailure(ErrorCode::ImportExportFailed, "glTF index accessor must not be normalized");
          anIndices.reserve(anAccessor.count);
          for (std::size_t anIndex = 0; anIndex < anAccessor.count; ++anIndex)
          {
            const std::uint32_t anIndexValue = static_cast<std::uint32_t>(
              gltfAccessorValue(anAccessor, anIndex, 0));
            if (anIndexValue >= aPositions.count)
            {
              throw KernelFailure(ErrorCode::ImportExportFailed, "glTF triangle index is out of range");
            }
            anIndices.push_back(anIndexValue);
          }
        }
        else
        {
          if (aPositions.count > std::numeric_limits<std::uint32_t>::max())
            throw KernelFailure(ErrorCode::ImportExportFailed, "glTF primitive has too many vertices");
          anIndices.resize(aPositions.count);
          for (std::size_t anIndex = 0; anIndex < aPositions.count; ++anIndex)
          {
            anIndices[anIndex] = static_cast<std::uint32_t>(anIndex);
          }
        }

        const bool reverseWinding = theSkinMatrices == nullptr
          && gltfTransformDeterminant(theTransform) < 0.0;
        const auto appendTriangle = [&](std::uint32_t a, std::uint32_t b, std::uint32_t c) {
          if (a == b || b == c || a == c) return;
          if (reverseWinding) std::swap(b, c);
          const gp_Pnt& p0 = aMesh.positions[aBase + a];
          const gp_Pnt& p1 = aMesh.positions[aBase + b];
          const gp_Pnt& p2 = aMesh.positions[aBase + c];
          if (gp_Vec(p0, p1).Crossed(gp_Vec(p0, p2)).SquareMagnitude() <= 1e-30) return;
          if (aMesh.indices.size() > std::numeric_limits<std::uint32_t>::max() - 3)
            throw KernelFailure(ErrorCode::ImportExportFailed, "glTF expanded mesh has too many indices");
          aMesh.indices.insert(aMesh.indices.end(), {
            static_cast<std::uint32_t>(aBase + a),
            static_cast<std::uint32_t>(aBase + b),
            static_cast<std::uint32_t>(aBase + c)});
        };
        if (aMode == 4)
        {
          if (anIndices.size() % 3 != 0)
          {
            throw KernelFailure(ErrorCode::ImportExportFailed, "glTF triangle list index count is invalid");
          }
          for (std::size_t anIndex = 0; anIndex < anIndices.size(); anIndex += 3)
            appendTriangle(anIndices[anIndex], anIndices[anIndex + 1], anIndices[anIndex + 2]);
        }
        else if (aMode == 5)
        {
          for (std::size_t anIndex = 2; anIndex < anIndices.size(); ++anIndex)
          {
            if (anIndex % 2 == 0)
              appendTriangle(anIndices[anIndex - 2], anIndices[anIndex - 1], anIndices[anIndex]);
            else
              appendTriangle(anIndices[anIndex - 1], anIndices[anIndex - 2], anIndices[anIndex]);
          }
        }
        else
        {
          for (std::size_t anIndex = 2; anIndex < anIndices.size(); ++anIndex)
            appendTriangle(anIndices[0], anIndices[anIndex - 1], anIndices[anIndex]);
        }
        if (theMeshIndex > std::numeric_limits<std::uint32_t>::max()
            || aPrimitiveIndex > std::numeric_limits<std::uint32_t>::max()
            || anIndexStart > std::numeric_limits<std::uint32_t>::max()
            || aMesh.indices.size() - anIndexStart > std::numeric_limits<std::uint32_t>::max())
          throw KernelFailure(ErrorCode::ImportExportFailed, "glTF expanded mesh is too large");
        TriangleMesh::GltfPrimitiveInstance anInstance;
        anInstance.meshIndex = static_cast<std::uint32_t>(theMeshIndex);
        anInstance.primitiveIndex = static_cast<std::uint32_t>(aPrimitiveIndex);
        anInstance.mode = static_cast<std::uint32_t>(aMode);
        anInstance.vertexStart = static_cast<std::uint32_t>(aBase);
        anInstance.vertexCount = static_cast<std::uint32_t>(aPositions.count);
        anInstance.indexStart = static_cast<std::uint32_t>(anIndexStart);
        anInstance.indexCount = static_cast<std::uint32_t>(aMesh.indices.size() - anIndexStart);
        anInstance.hasNormals = hasNormals;
        anInstance.hasUVs = hasUVs;
        if (theNodeIndex != noNode)
        {
          if (theNodeIndex > std::numeric_limits<std::uint32_t>::max())
            throw KernelFailure(ErrorCode::ImportExportFailed, "glTF node index is too large");
          anInstance.nodeIndex = static_cast<std::uint32_t>(theNodeIndex);
          anInstance.hasNode = true;
        }
        if (aPrimitive.contains("material"))
        {
          if (!aPrimitive.at("material").is_number_unsigned()
              || aPrimitive.at("material").get<std::uint64_t>() > std::numeric_limits<std::uint32_t>::max())
            throw KernelFailure(ErrorCode::ImportExportFailed, "glTF material index is invalid");
          const std::size_t aMaterial = aPrimitive.at("material").get<std::size_t>();
          if (!aDocument.contains("materials") || !aDocument.at("materials").is_array()
              || aMaterial >= aDocument.at("materials").size())
            throw KernelFailure(ErrorCode::ImportExportFailed, "glTF material index is out of range");
          anInstance.material = aPrimitive.at("material").get<std::uint32_t>();
          anInstance.hasMaterial = true;
        }
        aMesh.gltfPrimitives.push_back(anInstance);
      }
    };

    if (aDocument.contains("nodes") && aDocument.at("nodes").is_array()
        && !aDocument.at("nodes").empty())
    {
      const json& aNodes = aDocument.at("nodes");
      std::vector<bool> isActive(aNodes.size(), false);
      std::vector<bool> hasWorld(aNodes.size(), false);
      std::vector<GltfMatrix> aNodeWorlds(aNodes.size(), anIdentity);
      std::vector<std::pair<std::size_t, GltfMatrix>> aNodeVisits;
      std::function<void(std::size_t, const GltfMatrix&)> visitNode;
      visitNode = [&](const std::size_t theNodeIndex, const GltfMatrix& theParent) {
        if (theNodeIndex >= aNodes.size())
        {
          throw KernelFailure(ErrorCode::ImportExportFailed, "glTF node index is out of range");
        }
        if (isActive[theNodeIndex])
        {
          throw KernelFailure(ErrorCode::ImportExportFailed, "glTF node hierarchy contains a cycle");
        }
        isActive[theNodeIndex] = true;
        const json& aNode = aNodes.at(theNodeIndex);
        const GltfEvaluatedNode* anEvaluated = anEvaluatedNodes.empty()
          ? nullptr : &anEvaluatedNodes.at(theNodeIndex);
        const GltfMatrix aWorld = multiplyGltfMatrices(
          theParent, gltfNodeMatrix(aNode, anEvaluated));
        if (!hasWorld[theNodeIndex])
        {
          aNodeWorlds[theNodeIndex] = aWorld;
          hasWorld[theNodeIndex] = true;
        }
        aNodeVisits.emplace_back(theNodeIndex, aWorld);
        if (aNode.contains("children"))
        {
          if (!aNode.at("children").is_array())
            throw KernelFailure(ErrorCode::ImportExportFailed, "glTF node children must be an array");
          for (const json& aChild : aNode.at("children"))
            visitNode(aChild.get<std::size_t>(), aWorld);
        }
        isActive[theNodeIndex] = false;
      };

      std::vector<std::size_t> aRoots;
      if (aDocument.contains("scenes") && aDocument.at("scenes").is_array()
          && !aDocument.at("scenes").empty())
      {
        const std::size_t aSceneIndex = aDocument.value("scene", std::size_t(0));
        if (aSceneIndex >= aDocument.at("scenes").size())
        {
          throw KernelFailure(ErrorCode::ImportExportFailed, "glTF scene index is out of range");
        }
        if (aSceneIndex > std::numeric_limits<std::uint32_t>::max())
          throw KernelFailure(ErrorCode::ImportExportFailed, "glTF scene index is too large");
        const json& aScene = aDocument.at("scenes").at(aSceneIndex);
        aMesh.gltfActiveScene = static_cast<std::uint32_t>(aSceneIndex);
        aMesh.hasGltfActiveScene = true;
        if (aScene.contains("nodes"))
          for (const json& aRoot : aScene.at("nodes")) aRoots.push_back(aRoot.get<std::size_t>());
      }
      else
      {
        std::vector<bool> isChild(aNodes.size(), false);
        for (const json& aNode : aNodes)
          if (aNode.contains("children"))
            for (const json& aChild : aNode.at("children"))
            {
              const std::size_t aChildIndex = aChild.get<std::size_t>();
              if (aChildIndex >= aNodes.size())
                throw KernelFailure(ErrorCode::ImportExportFailed, "glTF node index is out of range");
              isChild[aChildIndex] = true;
            }
        for (std::size_t aNode = 0; aNode < aNodes.size(); ++aNode)
          if (!isChild[aNode]) aRoots.push_back(aNode);
      }
      for (const std::size_t aRoot : aRoots)
      {
        if (aRoot > std::numeric_limits<std::uint32_t>::max())
          throw KernelFailure(ErrorCode::ImportExportFailed, "glTF scene root index is too large");
        aMesh.gltfSceneRoots.push_back(static_cast<std::uint32_t>(aRoot));
        visitNode(aRoot, anIdentity);
      }
      for (const auto& aVisit : aNodeVisits)
      {
        const std::size_t aNodeIndex = aVisit.first;
        const json& aNode = aNodes.at(aNodeIndex);
        if (!aNode.contains("mesh")) continue;
        std::vector<GltfMatrix> aSkinMatrices;
        const std::vector<GltfMatrix>* aSkinMatricesPtr = nullptr;
        if (aNode.contains("skin"))
        {
          if (!aNode.at("skin").is_number_unsigned()
              || !aDocument.contains("skins") || !aDocument.at("skins").is_array())
            throw KernelFailure(ErrorCode::ImportExportFailed, "glTF skin index is invalid");
          const std::size_t aSkinIndex = aNode.at("skin").get<std::size_t>();
          if (aSkinIndex >= aDocument.at("skins").size())
            throw KernelFailure(ErrorCode::ImportExportFailed, "glTF skin index is out of range");
          const json& aSkin = aDocument.at("skins").at(aSkinIndex);
          if (!aSkin.is_object() || !aSkin.contains("joints")
              || !aSkin.at("joints").is_array() || aSkin.at("joints").empty())
            throw KernelFailure(ErrorCode::ImportExportFailed, "glTF skin joints are invalid");
          GltfAccessor anInverseBindMatrices;
          const bool hasInverseBindMatrices = aSkin.contains("inverseBindMatrices");
          if (hasInverseBindMatrices)
          {
            if (!aSkin.at("inverseBindMatrices").is_number_unsigned())
              throw KernelFailure(ErrorCode::ImportExportFailed,
                                  "glTF inverse bind matrix accessor is invalid");
            anInverseBindMatrices = gltfAccessor(
              aDocument, aBuffers, aSkin.at("inverseBindMatrices").get<std::size_t>(), "MAT4");
            if (anInverseBindMatrices.componentType != 5126
                || anInverseBindMatrices.normalized
                || anInverseBindMatrices.count < aSkin.at("joints").size())
              throw KernelFailure(ErrorCode::ImportExportFailed,
                                  "glTF inverse bind matrix accessor is unsupported");
          }
          aSkinMatrices.reserve(aSkin.at("joints").size());
          for (std::size_t aJoint = 0; aJoint < aSkin.at("joints").size(); ++aJoint)
          {
            const json& aJointValue = aSkin.at("joints").at(aJoint);
            if (!aJointValue.is_number_unsigned())
              throw KernelFailure(ErrorCode::ImportExportFailed, "glTF skin joint is invalid");
            const std::size_t aJointNode = aJointValue.get<std::size_t>();
            if (aJointNode >= aNodes.size() || !hasWorld[aJointNode])
              throw KernelFailure(ErrorCode::ImportExportFailed,
                                  "glTF skin joint is not in the active scene");
            GltfMatrix anInverseBind = anIdentity;
            if (hasInverseBindMatrices)
            {
              for (std::size_t anEntry = 0; anEntry < 16; ++anEntry)
              {
                anInverseBind[anEntry] = gltfAccessorValue(
                  anInverseBindMatrices, aJoint, anEntry);
                if (!std::isfinite(anInverseBind[anEntry]))
                  throw KernelFailure(ErrorCode::ImportExportFailed,
                                      "glTF inverse bind matrix contains a non-finite value");
              }
            }
            aSkinMatrices.push_back(multiplyGltfMatrices(
              aNodeWorlds[aJointNode], anInverseBind));
          }
          aSkinMatricesPtr = &aSkinMatrices;
        }
        appendMesh(aNode.at("mesh").get<std::size_t>(), aNodeIndex,
                   aVisit.second, aSkinMatricesPtr);
      }
    }
    else if (aDocument.contains("meshes") && aDocument.at("meshes").is_array())
    {
      for (std::size_t aMeshIndex = 0; aMeshIndex < aDocument.at("meshes").size(); ++aMeshIndex)
        appendMesh(aMeshIndex, noNode, anIdentity, nullptr);
    }
    if (aMesh.indices.empty() && !theAllowNoTriangles)
    {
      throw KernelFailure(ErrorCode::ImportExportFailed, "glTF input has no triangle primitives");
    }
    if (!hasAnyNormals || !hasAllNormals) aMesh.normals.clear();
    if (!hasAnyUVs || !hasAllUVs) aMesh.uvs.clear();
    if (theAllowNoTriangles)
    {
      aMesh.gltfDocument = aDocument;
      aMesh.gltfBuffers = std::move(aBuffers);
      if (aDocument.contains("images"))
      {
        if (!aDocument.at("images").is_array())
          throw KernelFailure(ErrorCode::ImportExportFailed, "glTF images must be an array");
        std::unordered_set<std::string> aResolvedImageUris;
        for (const json& anImage : aDocument.at("images"))
        {
          if (!anImage.is_object() || anImage.contains("uri") == anImage.contains("bufferView"))
            throw KernelFailure(ErrorCode::ImportExportFailed, "glTF image definition is invalid");
          if (anImage.contains("uri"))
          {
            if (!anImage.at("uri").is_string())
              throw KernelFailure(ErrorCode::ImportExportFailed, "glTF image URI must be a string");
            const std::string aUri = anImage.at("uri").get<std::string>();
            std::vector<std::uint8_t> aBytes;
            if (aUri.rfind("data:", 0) == 0)
              aBytes = decodeGltfDataUri(aUri);
            else
            {
              const auto aResource = theResources.find(aUri);
              if (aResource == theResources.end())
                throw KernelFailure(ErrorCode::ImportExportFailed,
                                    "glTF external resource is missing: " + aUri);
              aBytes.assign(aResource->second.begin(), aResource->second.end());
            }
            if (aBufferUris.find(aUri) == aBufferUris.end()
                && aResolvedImageUris.insert(aUri).second)
              aMesh.gltfResources.push_back({aUri, std::move(aBytes)});
          }
          else
          {
            if (!anImage.contains("mimeType") || !anImage.at("mimeType").is_string()
                || !anImage.at("bufferView").is_number_unsigned())
              throw KernelFailure(ErrorCode::ImportExportFailed, "glTF buffer view image is invalid");
            const std::size_t aViewIndex = anImage.at("bufferView").get<std::size_t>();
            if (!aDocument.contains("bufferViews") || !aDocument.at("bufferViews").is_array()
                || aViewIndex >= aDocument.at("bufferViews").size())
              throw KernelFailure(ErrorCode::ImportExportFailed, "glTF image buffer view is out of range");
            const json& aView = aDocument.at("bufferViews").at(aViewIndex);
            const std::size_t aBufferIndex = aView.at("buffer").get<std::size_t>();
            const std::size_t aViewOffset = aView.value("byteOffset", std::size_t(0));
            const std::size_t aViewLength = aView.at("byteLength").get<std::size_t>();
            if (aBufferIndex >= aMesh.gltfBuffers.size()
                || aViewOffset > aMesh.gltfBuffers[aBufferIndex].size()
                || aViewLength > aMesh.gltfBuffers[aBufferIndex].size() - aViewOffset)
              throw KernelFailure(ErrorCode::ImportExportFailed, "glTF image buffer view is truncated");
          }
        }
      }
      std::sort(aMesh.gltfResources.begin(), aMesh.gltfResources.end(),
                [](const auto& theLeft, const auto& theRight) { return theLeft.first < theRight.first; });
    }
    return aMesh;
  }

} // namespace occt_worker
