#include "kernel_exchange_helpers.hpp"
#include "kernel_protocol_helpers.hpp"
#include "kernel_operation_context.hpp"

namespace occt_worker {

TriangleMesh parseIndexedTriangleMesh(BufferStore& theBuffers,
                                      const json& theArgs,
                                      const char* theFormat)
  {
    const std::string aPositionBytes = inputBufferData(theBuffers, theArgs, "positions");
    const std::string anIndexBytes = inputBufferData(theBuffers, theArgs, "indices");
    const std::string aPrefix = std::string(theFormat) + " mesh ";
    if (aPositionBytes.empty() || aPositionBytes.size() % (sizeof(float) * 3) != 0
        || anIndexBytes.empty() || anIndexBytes.size() % (sizeof(std::uint32_t) * 3) != 0)
      throw KernelFailure(ErrorCode::InvalidArgs, aPrefix + "buffers have invalid dimensions");

    TriangleMesh aMesh;
    const std::size_t aVertexCount = aPositionBytes.size() / (sizeof(float) * 3);
    aMesh.positions.reserve(aVertexCount);
    for (std::size_t aVertex = 0; aVertex < aVertexCount; ++aVertex)
    {
      std::array<float, 3> aPoint{};
      std::memcpy(aPoint.data(), aPositionBytes.data() + aVertex * sizeof(float) * 3,
                  sizeof(float) * 3);
      if (!std::isfinite(aPoint[0]) || !std::isfinite(aPoint[1]) || !std::isfinite(aPoint[2]))
        throw KernelFailure(ErrorCode::InvalidArgs, aPrefix + "positions must be finite");
      aMesh.positions.emplace_back(aPoint[0], aPoint[1], aPoint[2]);
    }

    aMesh.indices.resize(anIndexBytes.size() / sizeof(std::uint32_t));
    std::memcpy(aMesh.indices.data(), anIndexBytes.data(), anIndexBytes.size());
    for (std::size_t anOffset = 0; anOffset < aMesh.indices.size(); anOffset += 3)
    {
      const std::uint32_t anIndex0 = aMesh.indices[anOffset];
      const std::uint32_t anIndex1 = aMesh.indices[anOffset + 1];
      const std::uint32_t anIndex2 = aMesh.indices[anOffset + 2];
      if (anIndex0 >= aVertexCount || anIndex1 >= aVertexCount || anIndex2 >= aVertexCount)
        throw KernelFailure(ErrorCode::InvalidArgs, aPrefix + "index is out of range");
      if (anIndex0 == anIndex1 || anIndex1 == anIndex2 || anIndex0 == anIndex2)
        throw KernelFailure(ErrorCode::InvalidArgs, aPrefix + "triangle indices must be distinct");
      gp_Vec aFirstEdge(aMesh.positions[anIndex0], aMesh.positions[anIndex1]);
      gp_Vec aSecondEdge(aMesh.positions[anIndex0], aMesh.positions[anIndex2]);
      aFirstEdge.Cross(aSecondEdge);
      if (aFirstEdge.SquareMagnitude() <= Precision::SquareConfusion())
        throw KernelFailure(ErrorCode::InvalidArgs, aPrefix + "triangle must not be degenerate");
    }

    if (theArgs.contains("normals"))
    {
      const std::string aNormalBytes = inputBufferData(theBuffers, theArgs, "normals");
      if (aNormalBytes.size() != aVertexCount * sizeof(float) * 3)
        throw KernelFailure(ErrorCode::InvalidArgs, aPrefix + "normals must match positions");
      aMesh.normals.reserve(aVertexCount);
      for (std::size_t aVertex = 0; aVertex < aVertexCount; ++aVertex)
      {
        std::array<float, 3> aNormal{};
        std::memcpy(aNormal.data(), aNormalBytes.data() + aVertex * sizeof(float) * 3,
                    sizeof(float) * 3);
        const double aSquaredNorm = static_cast<double>(aNormal[0]) * aNormal[0]
          + static_cast<double>(aNormal[1]) * aNormal[1]
          + static_cast<double>(aNormal[2]) * aNormal[2];
        if (!std::isfinite(aNormal[0]) || !std::isfinite(aNormal[1])
            || !std::isfinite(aNormal[2]) || aSquaredNorm <= Precision::SquareConfusion())
          throw KernelFailure(ErrorCode::InvalidArgs, aPrefix + "normals must be finite and non-zero");
        aMesh.normals.emplace_back(aNormal[0], aNormal[1], aNormal[2]);
      }
    }
    if (theArgs.contains("uvs"))
    {
      const std::string aUVBytes = inputBufferData(theBuffers, theArgs, "uvs");
      if (aUVBytes.size() != aVertexCount * sizeof(float) * 2)
        throw KernelFailure(ErrorCode::InvalidArgs, aPrefix + "UVs must match positions");
      aMesh.uvs.reserve(aVertexCount);
      for (std::size_t aVertex = 0; aVertex < aVertexCount; ++aVertex)
      {
        std::array<float, 2> aUV{};
        std::memcpy(aUV.data(), aUVBytes.data() + aVertex * sizeof(float) * 2,
                    sizeof(float) * 2);
        if (!std::isfinite(aUV[0]) || !std::isfinite(aUV[1]))
          throw KernelFailure(ErrorCode::InvalidArgs, aPrefix + "UVs must be finite");
        aMesh.uvs.emplace_back(aUV[0], aUV[1]);
      }
    }
    if (theArgs.contains("colors"))
    {
      const std::string aColorBytes = inputBufferData(theBuffers, theArgs, "colors");
      if (aColorBytes.size() != aVertexCount * 4)
        throw KernelFailure(ErrorCode::InvalidArgs,
                            aPrefix + "colors must be u8 RGBA matching positions");
      aMesh.colors.resize(aVertexCount);
      std::memcpy(aMesh.colors.data(), aColorBytes.data(), aColorBytes.size());
      for (const std::array<std::uint8_t, 4>& aColor : aMesh.colors)
        if (aColor[3] != 255)
          throw KernelFailure(ErrorCode::InvalidArgs,
                              aPrefix + "colors must have opaque alpha");
    }
    return aMesh;
  }

TriangleMesh buildTriangleMesh(const TopoDS_Shape& theInput,
                                        const double theLinearDeflection,
                                        const double theAngularDeflection,
                                        const bool theRelative)
  {
    if (theLinearDeflection <= 0.0 || theAngularDeflection <= 0.0)
    {
      throw KernelFailure(ErrorCode::InvalidArgs, "Mesh deflections must be positive");
    }

    TopoDS_Shape aShape = theInput;
    BRepTools::Clean(aShape);
    BRepMesh_IncrementalMesh aMesher(
      aShape, theLinearDeflection, theRelative, theAngularDeflection, false);
    if (!aMesher.IsDone())
    {
      throw KernelFailure(ErrorCode::TessellationFailed, "OCCT mesh generation failed");
    }

    return triangleMesh(aShape);
  }
TriangleMesh triangleMesh(const TopoDS_Shape& theShape)
  {
    TriangleMesh aMesh;
    for (TopExp_Explorer anExplorer(theShape, TopAbs_FACE); anExplorer.More(); anExplorer.Next())
    {
      const TopoDS_Face aFace = TopoDS::Face(anExplorer.Current());
      TopLoc_Location aLocation;
      const occ::handle<Poly_Triangulation> aTriangulation =
        BRep_Tool::Triangulation(aFace, aLocation);
      if (aTriangulation.IsNull())
      {
        continue;
      }

      const gp_Trsf aTransform = aLocation.Transformation();
      for (int aTriangleIndex = 1; aTriangleIndex <= aTriangulation->NbTriangles(); ++aTriangleIndex)
      {
        Poly_Triangle aTriangle = aTriangulation->Triangle(aTriangleIndex);
        if (aFace.Orientation() == TopAbs_REVERSED)
        {
          std::swap(aTriangle(2), aTriangle(3));
        }

        const std::uint32_t aStart = static_cast<std::uint32_t>(aMesh.positions.size());
        for (int aNode = 1; aNode <= 3; ++aNode)
        {
          aMesh.positions.push_back(aTriangulation->Node(aTriangle(aNode)).Transformed(aTransform));
        }
        gp_Vec aNormal = gp_Vec(aMesh.positions[aStart], aMesh.positions[aStart + 1])
          .Crossed(gp_Vec(aMesh.positions[aStart], aMesh.positions[aStart + 2]));
        if (aNormal.SquareMagnitude() <= Precision::SquareConfusion())
          aNormal = gp_Vec(0.0, 0.0, 1.0);
        else
          aNormal.Normalize();
        for (int aNode = 1; aNode <= 3; ++aNode)
        {
          aMesh.normals.emplace_back(aNormal.X(), aNormal.Y(), aNormal.Z());
          if (aTriangulation->HasUVNodes())
          {
            if (aMesh.uvs.empty() && aMesh.positions.size() > 3)
              aMesh.uvs.resize(aMesh.positions.size() - 3, gp_Pnt2d(0.0, 0.0));
            aMesh.uvs.push_back(aTriangulation->UVNode(aTriangle(aNode)));
          }
          else if (!aMesh.uvs.empty())
            aMesh.uvs.emplace_back(0.0, 0.0);
        }
        aMesh.indices.insert(aMesh.indices.end(), {aStart, aStart + 1, aStart + 2});
      }
    }
    if (aMesh.indices.empty())
    {
      throw KernelFailure(ErrorCode::ImportExportFailed, "Shape has no triangulated faces");
    }
    return aMesh;
  }

TopoDS_Shape makeTriangleShape(const TriangleMesh& theMesh)
  {
    if (theMesh.indices.empty() || theMesh.indices.size() % 3 != 0)
    {
      throw KernelFailure(ErrorCode::ImportExportFailed, "Mesh does not contain triangles");
    }

    BRep_Builder aBuilder;
    TopoDS_Compound aCompound;
    aBuilder.MakeCompound(aCompound);
    for (std::size_t aTriangle = 0; aTriangle < theMesh.indices.size(); aTriangle += 3)
    {
      const std::uint32_t anIndex0 = theMesh.indices[aTriangle];
      const std::uint32_t anIndex1 = theMesh.indices[aTriangle + 1];
      const std::uint32_t anIndex2 = theMesh.indices[aTriangle + 2];
      if (anIndex0 >= theMesh.positions.size() || anIndex1 >= theMesh.positions.size()
          || anIndex2 >= theMesh.positions.size())
      {
        throw KernelFailure(ErrorCode::ImportExportFailed, "Mesh triangle index is out of range");
      }

      BRepBuilderAPI_MakePolygon aPolygon;
      aPolygon.Add(theMesh.positions[anIndex0]);
      aPolygon.Add(theMesh.positions[anIndex1]);
      aPolygon.Add(theMesh.positions[anIndex2]);
      aPolygon.Close();
      if (!aPolygon.IsDone())
      {
        throw KernelFailure(ErrorCode::ImportExportFailed, "Mesh triangle cannot form a wire");
      }
      BRepBuilderAPI_MakeFace aFaceBuilder(aPolygon.Wire());
      if (!aFaceBuilder.IsDone() || aFaceBuilder.Face().IsNull())
      {
        throw KernelFailure(ErrorCode::ImportExportFailed, "Mesh triangle cannot form a face");
      }
      const TopoDS_Face aFace = aFaceBuilder.Face();
      const bool hasNormals = theMesh.normals.size() == theMesh.positions.size();
      const bool hasUVs = theMesh.uvs.size() == theMesh.positions.size();
      if (hasNormals || hasUVs)
      {
        const occ::handle<Poly_Triangulation> aTriangulation =
          new Poly_Triangulation(3, 1, hasUVs, hasNormals);
        const std::array<std::uint32_t, 3> anIndices = {anIndex0, anIndex1, anIndex2};
        for (int aNode = 0; aNode < 3; ++aNode)
        {
          const std::uint32_t anInputIndex = anIndices[static_cast<std::size_t>(aNode)];
          aTriangulation->SetNode(aNode + 1, theMesh.positions[anInputIndex]);
          if (hasNormals)
          {
            aTriangulation->SetNormal(aNode + 1, theMesh.normals[anInputIndex]);
          }
          if (hasUVs)
          {
            aTriangulation->SetUVNode(aNode + 1, theMesh.uvs[anInputIndex]);
          }
        }
        aTriangulation->SetTriangle(1, Poly_Triangle(1, 2, 3));
        aBuilder.UpdateFace(aFace, aTriangulation);
      }
      aBuilder.Add(aCompound, aFace);
    }
    return aCompound;
  }

TriangleMesh vrmlSceneMesh(const VrmlData_Scene& theScene)
  {
    TriangleMesh aMesh;
    bool hasFaces = false;
    bool hasCompleteUVs = true;
    bool hasCompleteColors = true;
    gp_Trsf aToZUp;
    aToZUp.SetRotation(gp_Ax1(gp::Origin(), gp::DX()), M_PI_2);

    std::function<void(const occ::handle<VrmlData_Node>&, const gp_Trsf&)> appendNode;
    appendNode = [&](const occ::handle<VrmlData_Node>& theNode, const gp_Trsf& theLocation)
    {
      if (theNode.IsNull()) return;
      if (theNode->IsKind(STANDARD_TYPE(VrmlData_Group)))
      {
        const occ::handle<VrmlData_Group> aGroup = occ::down_cast<VrmlData_Group>(theNode);
        gp_Trsf aChildLocation = theLocation;
        aChildLocation.Multiply(aGroup->GetTransform());
        for (VrmlData_Group::Iterator anIterator = aGroup->NodeIterator(); anIterator.More(); anIterator.Next())
          appendNode(anIterator.Value(), aChildLocation);
        return;
      }

      occ::handle<VrmlData_IndexedFaceSet> aFaceSet;
      if (theNode->IsKind(STANDARD_TYPE(VrmlData_ShapeNode)))
      {
        const occ::handle<VrmlData_ShapeNode> aShape = occ::down_cast<VrmlData_ShapeNode>(theNode);
        aFaceSet = occ::down_cast<VrmlData_IndexedFaceSet>(aShape->Geometry());
      }
      else if (theNode->IsKind(STANDARD_TYPE(VrmlData_IndexedFaceSet)))
      {
        aFaceSet = occ::down_cast<VrmlData_IndexedFaceSet>(theNode);
      }
      if (aFaceSet.IsNull() || aFaceSet->Coordinates().IsNull()) return;

      const occ::handle<VrmlData_Coordinate>& aCoordinates = aFaceSet->Coordinates();
      const occ::handle<VrmlData_Normal>& aNormals = aFaceSet->Normals();
      const occ::handle<VrmlData_TextureCoordinate>& aTextureCoords = aFaceSet->TextureCoords();
      const occ::handle<VrmlData_Color>& aColors = aFaceSet->Colors();
      const int** aNormalIndices = nullptr;
      const std::size_t aNormalFaceCount = aFaceSet->ArrayNormalInd(aNormalIndices);
      const int** aTextureIndices = nullptr;
      const std::size_t aTextureFaceCount = aFaceSet->ArrayTextureCoordInd(aTextureIndices);
      const int** aColorIndices = nullptr;
      const std::size_t aColorFaceCount = aFaceSet->ArrayColorInd(aColorIndices);
      const int** aPolygons = nullptr;
      const std::size_t aPolygonCount = aFaceSet->Polygons(aPolygons);
      if (aTextureCoords.IsNull()) hasCompleteUVs = false;
      if (aColors.IsNull()) hasCompleteColors = false;

      for (std::size_t aFaceIndex = 0; aFaceIndex < aPolygonCount; ++aFaceIndex)
      {
        const int* aPolygon = nullptr;
        const int aCornerCount = aFaceSet->Polygon(static_cast<int>(aFaceIndex), aPolygon);
        if (aCornerCount < 3) continue;
        for (int aTriangleCorner = 1; aTriangleCorner + 1 < aCornerCount; ++aTriangleCorner)
        {
          std::array<int, 3> aCorners = {0, aTriangleCorner, aTriangleCorner + 1};
          if (!aFaceSet->IsCCW()) std::swap(aCorners[1], aCorners[2]);

          std::array<gp_Pnt, 3> aPoints;
          bool hasValidTriangleUVs = !aTextureCoords.IsNull();
          bool hasValidTriangleColors = !aColors.IsNull();
          std::array<gp_Pnt2d, 3> aTriangleUVs;
          std::array<Quantity_Color, 3> aTriangleColors;
          std::array<gp_XYZ, 3> aSourceNormals = {
            gp_XYZ(0.0, 0.0, 0.0), gp_XYZ(0.0, 0.0, 0.0), gp_XYZ(0.0, 0.0, 0.0)};
          for (int aTriangleNode = 0; aTriangleNode < 3; ++aTriangleNode)
          {
            const int aCorner = aCorners[static_cast<std::size_t>(aTriangleNode)];
            const int aCoordinateIndex = aPolygon[aCorner];
            if (aCoordinateIndex < 0
                || static_cast<std::size_t>(aCoordinateIndex) >= aCoordinates->Length())
              throw KernelFailure(ErrorCode::ImportExportFailed, "VRML coordinate index is out of range");
            aPoints[static_cast<std::size_t>(aTriangleNode)] =
              gp_Pnt(aCoordinates->Coordinate(aCoordinateIndex)).Transformed(theLocation).Transformed(aToZUp);
            if (!aNormals.IsNull())
            {
              int aNormalIndex = aCoordinateIndex;
              if (aNormalIndices != nullptr && aFaceIndex < aNormalFaceCount)
              {
                const int* aFaceNormalIndices = aNormalIndices[aFaceIndex];
                if (aFaceNormalIndices != nullptr && aCorner < aFaceNormalIndices[0])
                  aNormalIndex = aFaceNormalIndices[aCorner + 1];
                else
                  aNormalIndex = -1;
              }
              if (aNormalIndex >= 0
                  && static_cast<std::size_t>(aNormalIndex) < aNormals->Length())
                aSourceNormals[static_cast<std::size_t>(aTriangleNode)] =
                  aNormals->Normal(aNormalIndex);
            }

            if (hasValidTriangleUVs)
            {
              int aTextureIndex = aCoordinateIndex;
              if (aTextureIndices != nullptr && aFaceIndex < aTextureFaceCount)
              {
                const int* aFaceTextureIndices = aTextureIndices[aFaceIndex];
                if (aFaceTextureIndices == nullptr || aCorner >= aFaceTextureIndices[0])
                {
                  hasValidTriangleUVs = false;
                }
                else
                {
                  aTextureIndex = aFaceTextureIndices[aCorner + 1];
                }
              }
              if (aTextureIndex < 0
                  || static_cast<std::size_t>(aTextureIndex) >= aTextureCoords->Length())
              {
                hasValidTriangleUVs = false;
              }
              else
              {
                const gp_XY& aUV = aTextureCoords->Points()[aTextureIndex];
                aTriangleUVs[static_cast<std::size_t>(aTriangleNode)] = gp_Pnt2d(aUV.X(), aUV.Y());
              }
            }
            if (hasValidTriangleColors)
            {
              int aColorIndex = aCoordinateIndex;
              if (aColorIndices != nullptr && aFaceIndex < aColorFaceCount)
              {
                const int* aFaceColorIndices = aColorIndices[aFaceIndex];
                if (aFaceColorIndices == nullptr || aCorner >= aFaceColorIndices[0])
                  hasValidTriangleColors = false;
                else
                  aColorIndex = aFaceColorIndices[aCorner + 1];
              }
              else if (aColors->Length() == aPolygonCount)
              {
                aColorIndex = static_cast<int>(aFaceIndex);
              }
              if (aColorIndex < 0 || static_cast<std::size_t>(aColorIndex) >= aColors->Length())
                hasValidTriangleColors = false;
              else
                aTriangleColors[static_cast<std::size_t>(aTriangleNode)] = aColors->Color(aColorIndex);
            }
          }

          const gp_Vec aFirstEdge(aPoints[0], aPoints[1]);
          const gp_Vec aSecondEdge(aPoints[0], aPoints[2]);
          const gp_Vec aCross = aFirstEdge.Crossed(aSecondEdge);
          if (aCross.SquareMagnitude() <= Precision::SquareConfusion()) continue;
          const gp_Dir aFaceNormal(aCross);
          for (int aTriangleNode = 0; aTriangleNode < 3; ++aTriangleNode)
          {
            aMesh.positions.push_back(aPoints[static_cast<std::size_t>(aTriangleNode)]);
            gp_Vec aNormal(aSourceNormals[static_cast<std::size_t>(aTriangleNode)]);
            if (aNormal.SquareMagnitude() > Precision::SquareConfusion())
            {
              aNormal.Transform(theLocation);
              aNormal.Transform(aToZUp);
              aMesh.normals.emplace_back(aNormal);
            }
            else
            {
              aMesh.normals.push_back(aFaceNormal);
            }
            if (hasValidTriangleUVs)
              aMesh.uvs.push_back(aTriangleUVs[static_cast<std::size_t>(aTriangleNode)]);
            if (hasValidTriangleColors)
            {
              const Quantity_Color& aColor =
                aTriangleColors[static_cast<std::size_t>(aTriangleNode)];
              aMesh.colors.push_back({
                static_cast<std::uint8_t>(std::lround(aColor.Red() * 255.0)),
                static_cast<std::uint8_t>(std::lround(aColor.Green() * 255.0)),
                static_cast<std::uint8_t>(std::lround(aColor.Blue() * 255.0)),
                255});
            }
            aMesh.indices.push_back(static_cast<std::uint32_t>(aMesh.indices.size()));
          }
          if (!hasValidTriangleUVs) hasCompleteUVs = false;
          if (!hasValidTriangleColors) hasCompleteColors = false;
          hasFaces = true;
        }
      }
    };

    const gp_Trsf anIdentity;
    for (VrmlData_Scene::Iterator anIterator = theScene.GetIterator(); anIterator.More(); anIterator.Next())
      appendNode(anIterator.Value(), anIdentity);
    if (!hasFaces) return {};
    if (!hasCompleteUVs || aMesh.uvs.size() != aMesh.positions.size()) aMesh.uvs.clear();
    if (!hasCompleteColors || aMesh.colors.size() != aMesh.positions.size()) aMesh.colors.clear();
    return aMesh;
  }

std::string writeVRMLIndexedMesh(const TriangleMesh& theMesh,
                                          const gp_Trsf& theTransform)
  {
    if (theMesh.positions.empty() || theMesh.indices.empty()
        || theMesh.indices.size() % 3 != 0)
      throw KernelFailure(ErrorCode::ImportExportFailed,
                          "VRML mesh does not contain triangles");

    const bool hasNormals = theMesh.normals.size() == theMesh.positions.size();
    const bool hasUVs = theMesh.uvs.size() == theMesh.positions.size();
    const bool hasColors = theMesh.colors.size() == theMesh.positions.size();
    std::ostringstream aStream;
    aStream << "#VRML V2.0 utf8\n"
            << "Shape {\n"
            << " geometry IndexedFaceSet {\n"
            << "  coord Coordinate { point [\n"
            << std::setprecision(17);
    for (const gp_Pnt& aPoint : theMesh.positions)
    {
      const gp_Pnt aTransformed = aPoint.Transformed(theTransform);
      aStream << "   " << aTransformed.X() << ' ' << aTransformed.Y() << ' '
              << aTransformed.Z() << ",\n";
    }
    aStream << "  ] }\n  coordIndex [\n";
    for (std::size_t anOffset = 0; anOffset < theMesh.indices.size(); anOffset += 3)
    {
      aStream << "   " << theMesh.indices[anOffset] << ' '
              << theMesh.indices[anOffset + 1] << ' '
              << theMesh.indices[anOffset + 2] << " -1,\n";
    }
    aStream << "  ]\n";
    if (hasNormals)
    {
      aStream << "  normal Normal { vector [\n";
      for (const gp_Dir& aNormal : theMesh.normals)
      {
        const gp_Dir aTransformed = aNormal.Transformed(theTransform);
        aStream << "   " << aTransformed.X() << ' ' << aTransformed.Y() << ' '
                << aTransformed.Z() << ",\n";
      }
      aStream << "  ] }\n  normalIndex [\n";
      for (std::size_t anOffset = 0; anOffset < theMesh.indices.size(); anOffset += 3)
      {
        aStream << "   " << theMesh.indices[anOffset] << ' '
                << theMesh.indices[anOffset + 1] << ' '
                << theMesh.indices[anOffset + 2] << " -1,\n";
      }
      aStream << "  ]\n";
    }
    if (hasUVs)
    {
      aStream << "  texCoord TextureCoordinate { point [\n";
      for (const gp_Pnt2d& aUV : theMesh.uvs)
        aStream << "   " << aUV.X() << ' ' << aUV.Y() << ",\n";
      aStream << "  ] }\n  texCoordIndex [\n";
      for (std::size_t anOffset = 0; anOffset < theMesh.indices.size(); anOffset += 3)
      {
        aStream << "   " << theMesh.indices[anOffset] << ' '
                << theMesh.indices[anOffset + 1] << ' '
                << theMesh.indices[anOffset + 2] << " -1,\n";
      }
      aStream << "  ]\n";
    }
    if (hasColors)
    {
      aStream << "  color Color { color [\n";
      for (const std::array<std::uint8_t, 4>& aColor : theMesh.colors)
        aStream << "   " << static_cast<double>(aColor[0]) / 255.0 << ' '
                << static_cast<double>(aColor[1]) / 255.0 << ' '
                << static_cast<double>(aColor[2]) / 255.0 << ",\n";
      aStream << "  ] }\n  colorIndex [\n";
      for (std::size_t anOffset = 0; anOffset < theMesh.indices.size(); anOffset += 3)
      {
        aStream << "   " << theMesh.indices[anOffset] << ' '
                << theMesh.indices[anOffset + 1] << ' '
                << theMesh.indices[anOffset + 2] << " -1,\n";
      }
      aStream << "  ]\n  colorPerVertex TRUE\n";
    }
    aStream << "  solid FALSE\n"
            << " }\n"
            << "}\n";
    return aStream.str();
  }

} // namespace occt_worker
