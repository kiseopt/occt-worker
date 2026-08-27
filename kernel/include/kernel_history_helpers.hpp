#pragma once
#include "kernel_support.hpp"

namespace occt_worker {

json tessellation(BufferStore& theBuffers,
                  const TopoDS_Shape& theShape,
                  bool theIncludeUV = false);

template <typename TBuilder>
json buildFilletHistory(TBuilder& theBuilder,
                        const TopoDS_Shape& theInput,
                        const TopoDS_Shape& theOutput)
{
  TopTools_IndexedMapOfShape anOutputFaces;
  TopTools_IndexedMapOfShape anOutputEdges;
  TopExp::MapShapes(theOutput, TopAbs_FACE, anOutputFaces);
  TopExp::MapShapes(theOutput, TopAbs_EDGE, anOutputEdges);
  json aHistory{{"retained", json::array()}, {"generated", json::array()},
                {"modified", json::array()}, {"deleted", json::array()}};

  for (const auto& aType : std::vector<std::pair<TopAbs_ShapeEnum, const char*>>{
         {TopAbs_FACE, "face"}, {TopAbs_EDGE, "edge"}})
  {
    TopTools_IndexedMapOfShape anInputShapes;
    TopExp::MapShapes(theInput, aType.first, anInputShapes);
    const TopTools_IndexedMapOfShape& anOutputMap =
      aType.first == TopAbs_FACE ? anOutputFaces : anOutputEdges;
    for (int aShapeIndex = 1; aShapeIndex <= anInputShapes.Extent(); ++aShapeIndex)
    {
      const TopoDS_Shape& aSource = anInputShapes(aShapeIndex);
      const json aFrom{{"input", 0}, {"type", aType.second}, {"index", aShapeIndex - 1}};
      const int aRetainedIndex = anOutputMap.FindIndex(aSource);
      if (aRetainedIndex > 0)
      {
        aHistory["retained"].push_back({
          {"from", aFrom}, {"to", {{"type", aType.second}, {"index", aRetainedIndex - 1}}}});
      }

      const NCollection_List<TopoDS_Shape>& aTargets = aType.first == TopAbs_FACE
        ? theBuilder.Modified(aSource) : theBuilder.Generated(aSource);
      json aTargetValues = json::array();
      for (const TopoDS_Shape& aTarget : aTargets)
      {
        const TopTools_IndexedMapOfShape* aTargetMap = nullptr;
        const char* aTargetType = nullptr;
        if (aTarget.ShapeType() == TopAbs_FACE)
        {
          aTargetMap = &anOutputFaces;
          aTargetType = "face";
        }
        else if (aTarget.ShapeType() == TopAbs_EDGE)
        {
          aTargetMap = &anOutputEdges;
          aTargetType = "edge";
        }
        if (aTargetMap == nullptr) continue;
        const int aTargetIndex = aTargetMap->FindIndex(aTarget);
        if (aTargetIndex > 0)
          aTargetValues.push_back({{"type", aTargetType}, {"index", aTargetIndex - 1}});
      }
      if (!aTargetValues.empty())
      {
        aHistory[aType.first == TopAbs_FACE ? "modified" : "generated"].push_back(
          {{"from", aFrom}, {"to", aTargetValues}});
      }

      const bool isDeleted = aType.first == TopAbs_FACE
        ? theBuilder.IsDeleted(aSource) : aRetainedIndex == 0;
      if (isDeleted)
        aHistory["deleted"].push_back(aFrom);
    }
  }
  return aHistory;
}

template <typename TBuilder>
json buildHistory(TBuilder& theBuilder,
                  const std::vector<TopoDS_Shape>& theInputs,
                  const TopoDS_Shape& theOutput)
{
  TopTools_IndexedMapOfShape anOutputFaces;
  TopTools_IndexedMapOfShape anOutputEdges;
  TopExp::MapShapes(theOutput, TopAbs_FACE, anOutputFaces);
  TopExp::MapShapes(theOutput, TopAbs_EDGE, anOutputEdges);
  json aHistory{{"retained", json::array()}, {"generated", json::array()},
                {"modified", json::array()}, {"deleted", json::array()}};

  for (std::size_t anInputIndex = 0; anInputIndex < theInputs.size(); ++anInputIndex)
  {
    for (const auto& aType : std::vector<std::pair<TopAbs_ShapeEnum, const char*>>{
           {TopAbs_FACE, "face"}, {TopAbs_EDGE, "edge"}})
    {
      TopTools_IndexedMapOfShape anInputShapes;
      TopExp::MapShapes(theInputs[anInputIndex], aType.first, anInputShapes);
      for (int aShapeIndex = 1; aShapeIndex <= anInputShapes.Extent(); ++aShapeIndex)
      {
        const TopoDS_Shape& aSource = anInputShapes(aShapeIndex);
        const TopTools_IndexedMapOfShape& anOutputMap =
          aType.first == TopAbs_FACE ? anOutputFaces : anOutputEdges;
        const json aFrom{
          {"input", anInputIndex}, {"type", aType.second}, {"index", aShapeIndex - 1}};
        const int aRetainedIndex = anOutputMap.FindIndex(aSource);
        if (aRetainedIndex > 0)
        {
          aHistory["retained"].push_back({
            {"from", aFrom},
            {"to", {{"type", aType.second}, {"index", aRetainedIndex - 1}}}});
        }

        const auto appendCategory = [&](const char* theCategory,
                                        const NCollection_List<TopoDS_Shape>& theRecordedTargets)
        {
          json aTargets = json::array();
          std::unordered_set<std::uint64_t> aSeenTargets;
          for (const TopoDS_Shape& aTarget : theRecordedTargets)
          {
            const TopAbs_ShapeEnum aDirectType = aTarget.ShapeType();
            for (const auto& aTargetType :
                 std::vector<std::tuple<TopAbs_ShapeEnum,
                                        const char*,
                                        const TopTools_IndexedMapOfShape*>>{
                   {TopAbs_FACE, "face", &anOutputFaces},
                   {TopAbs_EDGE, "edge", &anOutputEdges}})
            {
              if ((aDirectType == TopAbs_FACE || aDirectType == TopAbs_EDGE)
                  && aDirectType != std::get<0>(aTargetType))
                continue;
              TopTools_IndexedMapOfShape aMappedTargets;
              if (aDirectType == std::get<0>(aTargetType))
                aMappedTargets.Add(aTarget);
              else
                TopExp::MapShapes(aTarget, std::get<0>(aTargetType), aMappedTargets);
              for (int aMappedIndex = 1; aMappedIndex <= aMappedTargets.Extent(); ++aMappedIndex)
              {
                const int aTargetIndex =
                  std::get<2>(aTargetType)->FindIndex(aMappedTargets(aMappedIndex));
                const std::uint64_t aTargetKey =
                  (static_cast<std::uint64_t>(std::get<0>(aTargetType)) << 32)
                  | static_cast<std::uint32_t>(aTargetIndex);
                if (aTargetIndex > 0 && aSeenTargets.insert(aTargetKey).second)
                  aTargets.push_back({
                    {"type", std::get<1>(aTargetType)}, {"index", aTargetIndex - 1}});
              }
            }
          }
          if (!aTargets.empty())
            aHistory[theCategory].push_back({{"from", aFrom}, {"to", aTargets}});
        };

        const NCollection_List<TopoDS_Shape> aGenerated = theBuilder.Generated(aSource);
        appendCategory("generated", aGenerated);
        const NCollection_List<TopoDS_Shape> aModified = theBuilder.Modified(aSource);
        appendCategory("modified", aModified);
        if (theBuilder.IsDeleted(aSource))
          aHistory["deleted"].push_back(aFrom);
      }
    }
  }
  return aHistory;
}

json buildRecordedHistory(const occ::handle<BRepTools_History>& theRecordedHistory,
                          const std::vector<TopoDS_Shape>& theInputs,
                          const TopoDS_Shape& theOutput);
json buildRetainedHistory(const TopoDS_Shape& theInput, const TopoDS_Shape& theOutput);

} // namespace occt_worker
