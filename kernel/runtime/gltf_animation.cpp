#include "kernel_exchange_helpers.hpp"

namespace occt_worker {

std::vector<GltfEvaluatedNode> evaluateGLTFAnimation(
  const json& aDocument,
  const std::vector<std::vector<std::uint8_t>>& aBuffers,
  const std::size_t* theAnimationIndex,
  const double* theAnimationTime)
{
  std::vector<GltfEvaluatedNode> anEvaluatedNodes;
  if (theAnimationIndex != nullptr)
  {
    if (theAnimationTime == nullptr || !aDocument.contains("animations")
        || !aDocument.at("animations").is_array()
        || *theAnimationIndex >= aDocument.at("animations").size())
      throw KernelFailure(ErrorCode::ImportExportFailed,
                          "glTF animation index is out of range");
    if (!aDocument.contains("nodes") || !aDocument.at("nodes").is_array())
      throw KernelFailure(ErrorCode::ImportExportFailed,
                          "glTF animation requires a node array");
    const json& aNodes = aDocument.at("nodes");
    const auto animationIndexValue = [](const json& theValue,
                                        const char* theMessage) {
      if (!theValue.is_number_unsigned())
        throw KernelFailure(ErrorCode::ImportExportFailed, theMessage);
      const std::uint64_t aValue = theValue.get<std::uint64_t>();
      if (aValue > std::numeric_limits<std::size_t>::max())
        throw KernelFailure(ErrorCode::ImportExportFailed, theMessage);
      return static_cast<std::size_t>(aValue);
    };
    const json& anAnimation = aDocument.at("animations").at(*theAnimationIndex);
    if (!anAnimation.is_object() || !anAnimation.contains("samplers")
        || !anAnimation.at("samplers").is_array()
        || anAnimation.at("samplers").empty()
        || !anAnimation.contains("channels")
        || !anAnimation.at("channels").is_array()
        || anAnimation.at("channels").empty())
      throw KernelFailure(ErrorCode::ImportExportFailed,
                          "glTF animation definition is invalid");
    const json& aSamplers = anAnimation.at("samplers");
    for (const json& aSampler : aSamplers)
    {
      if (!aSampler.is_object() || !aSampler.contains("input")
          || !aSampler.contains("output")
          || (aSampler.contains("interpolation")
              && !aSampler.at("interpolation").is_string()))
        throw KernelFailure(ErrorCode::ImportExportFailed,
                            "glTF animation sampler is invalid");
      animationIndexValue(aSampler.at("input"), "glTF animation sampler is invalid");
      animationIndexValue(aSampler.at("output"), "glTF animation sampler is invalid");
      const std::string anInterpolation = aSampler.value("interpolation", "LINEAR");
      if (anInterpolation != "STEP" && anInterpolation != "LINEAR"
          && anInterpolation != "CUBICSPLINE")
        throw KernelFailure(ErrorCode::ImportExportFailed,
                            "glTF animation interpolation is unsupported");
    }
    anEvaluatedNodes.resize(aNodes.size());
    std::unordered_set<std::string> aTargets;
    for (const json& aChannel : anAnimation.at("channels"))
    {
      if (!aChannel.is_object() || !aChannel.contains("sampler")
          || !aChannel.contains("target") || !aChannel.at("target").is_object())
        throw KernelFailure(ErrorCode::ImportExportFailed,
                            "glTF animation channel is invalid");
      const std::size_t aSamplerIndex = animationIndexValue(
        aChannel.at("sampler"), "glTF animation sampler index is invalid");
      if (aSamplerIndex >= aSamplers.size())
        throw KernelFailure(ErrorCode::ImportExportFailed,
                            "glTF animation sampler index is out of range");
      const json& aTarget = aChannel.at("target");
      if (!aTarget.contains("node") || !aTarget.contains("path")
          || !aTarget.at("path").is_string())
        throw KernelFailure(ErrorCode::ImportExportFailed,
                            "glTF animation channel target is invalid");
      const std::size_t aNodeIndex = animationIndexValue(
        aTarget.at("node"), "glTF animation target node is invalid");
      if (aNodeIndex >= aNodes.size() || !aNodes.at(aNodeIndex).is_object())
        throw KernelFailure(ErrorCode::ImportExportFailed,
                            "glTF animation target node is out of range");
      const std::string aPath = aTarget.at("path").get<std::string>();
      if (aPath != "translation" && aPath != "rotation"
          && aPath != "scale" && aPath != "weights")
        throw KernelFailure(ErrorCode::ImportExportFailed,
                            "glTF animation target path is unsupported");
      if (!aTargets.insert(std::to_string(aNodeIndex) + ":" + aPath).second)
        throw KernelFailure(ErrorCode::ImportExportFailed,
                            "glTF animation has duplicate target channels");
      if (aPath != "weights" && aNodes.at(aNodeIndex).contains("matrix"))
        throw KernelFailure(ErrorCode::ImportExportFailed,
                            "glTF animation cannot target a matrix node transform");
       std::size_t aValueCount = aPath == "rotation" ? 4 : 3;
      if (aPath == "weights")
      {
        const json& aNode = aNodes.at(aNodeIndex);
        if (!aNode.contains("mesh") || !aNode.at("mesh").is_number_unsigned()
            || !aDocument.contains("meshes") || !aDocument.at("meshes").is_array())
          throw KernelFailure(ErrorCode::ImportExportFailed,
                              "glTF weight animation target requires a mesh node");
        const std::size_t aMeshIndex = animationIndexValue(
          aNode.at("mesh"), "glTF animation target mesh is invalid");
        if (aMeshIndex >= aDocument.at("meshes").size())
          throw KernelFailure(ErrorCode::ImportExportFailed,
                              "glTF animation target mesh is out of range");
        const json& aMesh = aDocument.at("meshes").at(aMeshIndex);
        if (!aMesh.is_object() || !aMesh.contains("primitives")
            || !aMesh.at("primitives").is_array() || aMesh.at("primitives").empty())
          throw KernelFailure(ErrorCode::ImportExportFailed,
                              "glTF animation target mesh is invalid");
        aValueCount = std::numeric_limits<std::size_t>::max();
        for (const json& aPrimitive : aMesh.at("primitives"))
        {
          if (!aPrimitive.is_object() || !aPrimitive.contains("targets")
              || !aPrimitive.at("targets").is_array()
              || aPrimitive.at("targets").empty())
            throw KernelFailure(ErrorCode::ImportExportFailed,
                                "glTF weight animation requires morph targets");
          const std::size_t aTargetCount = aPrimitive.at("targets").size();
          if (aValueCount == std::numeric_limits<std::size_t>::max())
            aValueCount = aTargetCount;
          else if (aValueCount != aTargetCount)
            throw KernelFailure(ErrorCode::ImportExportFailed,
                                "glTF morph target counts must match within a mesh");
        }
      }
       const json& aSampler = aSamplers.at(aSamplerIndex);
      const std::string anInterpolation = aSampler.value("interpolation", "LINEAR");
      const GltfAccessor anInput = gltfAccessor(
        aDocument, aBuffers,
        animationIndexValue(aSampler.at("input"), "glTF animation input accessor is invalid"),
        "SCALAR");
      if (anInput.componentType != 5126 || anInput.normalized || anInput.count == 0)
        throw KernelFailure(ErrorCode::ImportExportFailed,
                            "glTF animation input accessor is unsupported");
      std::vector<double> aTimes(anInput.count);
      for (std::size_t aKey = 0; aKey < anInput.count; ++aKey)
      {
        aTimes[aKey] = gltfAccessorValue(anInput, aKey, 0);
        if (!std::isfinite(aTimes[aKey])
            || (aKey > 0 && aTimes[aKey] <= aTimes[aKey - 1]))
          throw KernelFailure(ErrorCode::ImportExportFailed,
                              "glTF animation key times must be finite and increasing");
      }
      const bool isWeights = aPath == "weights";
      const GltfAccessor anOutput = gltfAccessor(
        aDocument, aBuffers,
        animationIndexValue(aSampler.at("output"), "glTF animation output accessor is invalid"),
        isWeights ? "SCALAR" : (aPath == "rotation" ? "VEC4" : "VEC3"));
      if (anOutput.componentType != 5126 || anOutput.normalized)
        throw KernelFailure(ErrorCode::ImportExportFailed,
                            "glTF animation output accessor is unsupported");
      const std::size_t aFactor = anInterpolation == "CUBICSPLINE" ? 3 : 1;
      if (anInput.count > std::numeric_limits<std::size_t>::max() / aFactor
          || anInput.count * aFactor > std::numeric_limits<std::size_t>::max() / (isWeights ? aValueCount : 1)
          || anOutput.count != anInput.count * aFactor * (isWeights ? aValueCount : 1))
        throw KernelFailure(ErrorCode::ImportExportFailed,
                            "glTF animation output count is invalid");
      for (std::size_t anElement = 0; anElement < anOutput.count; ++anElement)
        for (std::size_t aComponent = 0; aComponent < anOutput.components; ++aComponent)
          if (!std::isfinite(gltfAccessorValue(anOutput, anElement, aComponent)))
            throw KernelFailure(ErrorCode::ImportExportFailed,
                                "glTF animation output contains a non-finite value");
       std::size_t aKey0 = 0;
      std::size_t aKey1 = 0;
      double aParameter = 0.0;
      if (*theAnimationTime >= aTimes.back())
        aKey0 = aKey1 = aTimes.size() - 1;
      else if (*theAnimationTime > aTimes.front())
      {
        const auto anUpper = std::upper_bound(aTimes.begin(), aTimes.end(), *theAnimationTime);
        aKey0 = static_cast<std::size_t>(anUpper - aTimes.begin() - 1);
        aKey1 = aKey0 + 1;
        aParameter = (*theAnimationTime - aTimes[aKey0]) / (aTimes[aKey1] - aTimes[aKey0]);
      }
      const auto outputValue = [&](const std::size_t theKey,
                                   const std::size_t theSlot,
                                   const std::size_t theComponent) {
        const std::size_t anElement = isWeights
          ? (theKey * aFactor + theSlot) * aValueCount + theComponent
          : theKey * aFactor + theSlot;
        return gltfAccessorValue(anOutput, anElement, isWeights ? 0 : theComponent);
      };
      std::vector<double> aSample(aValueCount);
      const std::size_t aValueSlot = anInterpolation == "CUBICSPLINE" ? 1 : 0;
      if (aKey0 == aKey1 || anInterpolation == "STEP")
      {
        for (std::size_t aComponent = 0; aComponent < aValueCount; ++aComponent)
          aSample[aComponent] = outputValue(aKey0, aValueSlot, aComponent);
      }
      else if (anInterpolation == "CUBICSPLINE")
      {
        const double aDuration = aTimes[aKey1] - aTimes[aKey0];
        const double u2 = aParameter * aParameter;
        const double u3 = u2 * aParameter;
        const double h00 = 2.0 * u3 - 3.0 * u2 + 1.0;
        const double h10 = u3 - 2.0 * u2 + aParameter;
        const double h01 = -2.0 * u3 + 3.0 * u2;
        const double h11 = u3 - u2;
        for (std::size_t aComponent = 0; aComponent < aValueCount; ++aComponent)
          aSample[aComponent] = h00 * outputValue(aKey0, 1, aComponent)
            + h10 * aDuration * outputValue(aKey0, 2, aComponent)
            + h01 * outputValue(aKey1, 1, aComponent)
            + h11 * aDuration * outputValue(aKey1, 0, aComponent);
      }
      else if (aPath == "rotation")
      {
        std::array<double, 4> q0{}, q1{};
        double aLength0 = 0.0, aLength1 = 0.0;
        for (std::size_t aComponent = 0; aComponent < 4; ++aComponent)
        {
          q0[aComponent] = outputValue(aKey0, 0, aComponent);
          q1[aComponent] = outputValue(aKey1, 0, aComponent);
          aLength0 += q0[aComponent] * q0[aComponent];
          aLength1 += q1[aComponent] * q1[aComponent];
        }
        aLength0 = std::sqrt(aLength0);
        aLength1 = std::sqrt(aLength1);
        if (aLength0 <= std::numeric_limits<double>::epsilon()
            || aLength1 <= std::numeric_limits<double>::epsilon())
          throw KernelFailure(ErrorCode::ImportExportFailed,
                              "glTF animation quaternion is invalid");
        double aDot = 0.0;
        for (std::size_t aComponent = 0; aComponent < 4; ++aComponent)
        {
          q0[aComponent] /= aLength0;
          q1[aComponent] /= aLength1;
          aDot += q0[aComponent] * q1[aComponent];
        }
        if (aDot < 0.0)
        {
          aDot = -aDot;
          for (double& aValue : q1) aValue = -aValue;
        }
        aDot = std::min(1.0, aDot);
        if (aDot > 0.9995)
        {
          for (std::size_t aComponent = 0; aComponent < 4; ++aComponent)
            aSample[aComponent] = q0[aComponent]
              + aParameter * (q1[aComponent] - q0[aComponent]);
        }
        else
        {
          const double anAngle = std::acos(aDot);
          const double aDenominator = std::sin(anAngle);
          const double aWeight0 = std::sin((1.0 - aParameter) * anAngle) / aDenominator;
          const double aWeight1 = std::sin(aParameter * anAngle) / aDenominator;
          for (std::size_t aComponent = 0; aComponent < 4; ++aComponent)
            aSample[aComponent] = aWeight0 * q0[aComponent] + aWeight1 * q1[aComponent];
        }
      }
      else
      {
        for (std::size_t aComponent = 0; aComponent < aValueCount; ++aComponent)
          aSample[aComponent] = outputValue(aKey0, 0, aComponent)
            + aParameter * (outputValue(aKey1, 0, aComponent)
                            - outputValue(aKey0, 0, aComponent));
      }
      if (aPath == "rotation")
      {
        double aLength = 0.0;
        for (const double aValue : aSample) aLength += aValue * aValue;
        aLength = std::sqrt(aLength);
        if (!std::isfinite(aLength) || aLength <= std::numeric_limits<double>::epsilon())
          throw KernelFailure(ErrorCode::ImportExportFailed,
                              "glTF animation quaternion is invalid");
        for (double& aValue : aSample) aValue /= aLength;
      }
      for (const double aValue : aSample)
        if (!std::isfinite(aValue))
          throw KernelFailure(ErrorCode::ImportExportFailed,
                              "glTF sampled animation value is not finite");
       GltfEvaluatedNode& anEvaluated = anEvaluatedNodes[aNodeIndex];
      if (aPath == "weights")
      {
        anEvaluated.hasWeights = true;
        anEvaluated.weights = std::move(aSample);
      }
      else
      {
        std::array<double, 4>* aDestination = aPath == "translation"
          ? &anEvaluated.translation : (aPath == "rotation"
            ? &anEvaluated.rotation : &anEvaluated.scale);
        std::copy(aSample.begin(), aSample.end(), aDestination->begin());
        if (aPath == "translation") anEvaluated.hasTranslation = true;
        else if (aPath == "rotation") anEvaluated.hasRotation = true;
        else anEvaluated.hasScale = true;
      }
    }
  }

  return anEvaluatedNodes;
}

} // namespace occt_worker
