#include "kernel_history_helpers.hpp"
#include "kernel_protocol_helpers.hpp"

namespace occt_worker {

json tessellation(BufferStore& theBuffers,
                  const TopoDS_Shape& theShape,
                  const bool theIncludeUV)
{
  std::vector<float> aPositions;
  std::vector<float> aNormals;
  std::vector<float> aUVs;
  std::vector<std::uint32_t> anIndices;
  std::vector<std::uint32_t> aFaceGroups;
  TopTools_IndexedMapOfShape aFaces;
  TopExp::MapShapes(theShape, TopAbs_FACE, aFaces);

  for (int aFaceIndex = 1; aFaceIndex <= aFaces.Extent(); ++aFaceIndex)
  {
    const TopoDS_Face aFace = TopoDS::Face(aFaces(aFaceIndex));
    TopLoc_Location aLocation;
    const occ::handle<Poly_Triangulation>& aTriangulation =
      BRep_Tool::Triangulation(aFace, aLocation);
    if (aTriangulation.IsNull())
      continue;
    if (!aTriangulation->HasNormals())
      BRepLib_ToolTriangulatedShape::ComputeNormals(aFace, aTriangulation);
    if (theIncludeUV && !aTriangulation->HasUVNodes())
      throw KernelFailure(ErrorCode::TessellationFailed,
                          "OCCT tessellation did not provide UV nodes");

    const std::uint32_t aVertexBase =
      static_cast<std::uint32_t>(aPositions.size() / 3);
    const gp_Trsf aTransform = aLocation.Transformation();
    const bool isReversed = aFace.Orientation() == TopAbs_REVERSED;
    for (int aNodeIndex = 1; aNodeIndex <= aTriangulation->NbNodes(); ++aNodeIndex)
    {
      gp_Pnt aPoint = aTriangulation->Node(aNodeIndex);
      aPoint.Transform(aTransform);
      gp_Dir aNormal = aTriangulation->Normal(aNodeIndex);
      aNormal.Transform(aTransform);
      if (isReversed) aNormal.Reverse();
      aPositions.insert(aPositions.end(), {static_cast<float>(aPoint.X()),
                                           static_cast<float>(aPoint.Y()),
                                           static_cast<float>(aPoint.Z())});
      aNormals.insert(aNormals.end(), {static_cast<float>(aNormal.X()),
                                       static_cast<float>(aNormal.Y()),
                                       static_cast<float>(aNormal.Z())});
      if (theIncludeUV)
      {
        const gp_Pnt2d aUV = aTriangulation->UVNode(aNodeIndex);
        aUVs.insert(aUVs.end(), {static_cast<float>(aUV.X()), static_cast<float>(aUV.Y())});
      }
    }
    const std::uint32_t aStart = static_cast<std::uint32_t>(anIndices.size());
    for (int aTriangleIndex = 1; aTriangleIndex <= aTriangulation->NbTriangles();
         ++aTriangleIndex)
    {
      int a;
      int b;
      int c;
      aTriangulation->Triangle(aTriangleIndex).Get(a, b, c);
      if (isReversed) std::swap(b, c);
      anIndices.insert(anIndices.end(), {
        aVertexBase + static_cast<std::uint32_t>(a - 1),
        aVertexBase + static_cast<std::uint32_t>(b - 1),
        aVertexBase + static_cast<std::uint32_t>(c - 1)});
    }
    aFaceGroups.insert(aFaceGroups.end(), {
      static_cast<std::uint32_t>(aFaceIndex - 1),
      aStart,
      static_cast<std::uint32_t>(anIndices.size()) - aStart});
  }

  const std::uint32_t aPositionsId = theBuffers.copy(aPositions);
  const std::uint32_t aNormalsId = theBuffers.copy(aNormals);
  const std::uint32_t anIndicesId = theBuffers.copy(anIndices);
  const std::uint32_t aGroupsId = theBuffers.copy(aFaceGroups);
  json aResult{
    {"positions", bufferDescriptor(aPositionsId, aPositions.size() * sizeof(float), "f32x3")},
    {"normals", bufferDescriptor(aNormalsId, aNormals.size() * sizeof(float), "f32x3")},
    {"indices",
     bufferDescriptor(anIndicesId, anIndices.size() * sizeof(std::uint32_t), "u32")},
    {"faceGroups",
     bufferDescriptor(aGroupsId, aFaceGroups.size() * sizeof(std::uint32_t), "u32x3")}};
  if (theIncludeUV)
  {
    const std::uint32_t aUVsId = theBuffers.copy(aUVs);
    aResult["uvs"] = bufferDescriptor(aUVsId, aUVs.size() * sizeof(float), "f32x2");
  }
  return aResult;
}

json buildRecordedHistory(const occ::handle<BRepTools_History>& theRecordedHistory,
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
      const TopTools_IndexedMapOfShape& anOutputMap =
        aType.first == TopAbs_FACE ? anOutputFaces : anOutputEdges;
      for (int aShapeIndex = 1; aShapeIndex <= anInputShapes.Extent(); ++aShapeIndex)
      {
        const TopoDS_Shape& aSource = anInputShapes(aShapeIndex);
        const json aFrom{
          {"input", anInputIndex}, {"type", aType.second}, {"index", aShapeIndex - 1}};
        const int aRetainedIndex = anOutputMap.FindIndex(aSource);
        if (aRetainedIndex > 0)
        {
          aHistory["retained"].push_back({
            {"from", aFrom},
            {"to", {{"type", aType.second}, {"index", aRetainedIndex - 1}}}});
        }

        for (const auto& aCategory :
             std::vector<std::pair<const char*, const NCollection_List<TopoDS_Shape>*>>{
               {"generated", &theRecordedHistory->Generated(aSource)},
               {"modified", &theRecordedHistory->Modified(aSource)}})
        {
          TopTools_IndexedMapOfShape aTargets;
          for (const TopoDS_Shape& aRecordedTarget : *aCategory.second)
          {
            if (aRecordedTarget.ShapeType() == aType.first)
              aTargets.Add(aRecordedTarget);
            else
              TopExp::MapShapes(aRecordedTarget, aType.first, aTargets);
          }
          json aTargetValues = json::array();
          for (int aTargetIndex = 1; aTargetIndex <= aTargets.Extent(); ++aTargetIndex)
          {
            const int anOutputIndex = anOutputMap.FindIndex(aTargets(aTargetIndex));
            if (anOutputIndex > 0)
              aTargetValues.push_back({
                {"type", aType.second}, {"index", anOutputIndex - 1}});
          }
          if (!aTargetValues.empty())
            aHistory[aCategory.first].push_back({
              {"from", aFrom}, {"to", aTargetValues}});
        }
        if (theRecordedHistory->IsRemoved(aSource))
          aHistory["deleted"].push_back(aFrom);
      }
    }
  }
  return aHistory;
}

json buildRetainedHistory(const TopoDS_Shape& theInput, const TopoDS_Shape& theOutput)
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
    for (int anIndex = 1; anIndex <= anInputShapes.Extent(); ++anIndex)
    {
      const int anOutputIndex = anOutputMap.FindIndex(anInputShapes(anIndex));
      if (anOutputIndex > 0)
      {
        aHistory["retained"].push_back({
          {"from", {{"input", 0}, {"type", aType.second}, {"index", anIndex - 1}}},
          {"to", {{"type", aType.second}, {"index", anOutputIndex - 1}}}});
      }
    }
  }
  return aHistory;
}

} // namespace occt_worker
