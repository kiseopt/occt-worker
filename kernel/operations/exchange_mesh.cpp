// Kernel mesh-exchange operations: one out-of-line ExchangeMeshOperations::h_* handler per dispatch branch.
// Handler bodies are byte-identical fragments of the original dispatchOperation.
// As a shared Side module the constructor registers into the Main registry
// right after Emscripten runs __wasm_call_ctors.

#include "kernel_exchange_mesh_operations.hpp"
#include "kernel_exchange_helpers.hpp"
#include "kernel_history_helpers.hpp"
#include "kernel_operation_context.hpp"
#include "kernel_protocol_helpers.hpp"
#include "side_registration.hpp"

namespace occt_worker {

json h_exportSTL(KernelOperationContext& theContext, const json& theArgs)
{
      const std::string anEncoding = theArgs.value("encoding", "binary");
      if (anEncoding != "binary" && anEncoding != "ascii")
        throw KernelFailure(ErrorCode::InvalidArgs, "STL encoding must be binary or ascii");
      const std::string aSolidName = theArgs.value("solidName", std::string());
      std::string aBinaryHeader;
      if (theArgs.contains("solidName"))
      {
        if (anEncoding != "ascii")
          throw KernelFailure(ErrorCode::InvalidArgs, "STL solidName requires ASCII encoding");
        if (std::any_of(aSolidName.begin(), aSolidName.end(), [](const unsigned char theCharacter)
            { return theCharacter < 0x20 || theCharacter > 0x7e; }))
          throw KernelFailure(ErrorCode::InvalidArgs, "STL solidName must contain printable ASCII characters");
      }
      if (theArgs.contains("binaryHeader"))
      {
        if (anEncoding != "binary")
          throw KernelFailure(ErrorCode::InvalidArgs, "STL binaryHeader requires binary encoding");
        aBinaryHeader = inputBufferData(theContext.buffers(), theArgs, "binaryHeader");
        if (aBinaryHeader.size() != 80)
          throw KernelFailure(ErrorCode::InvalidArgs, "STL binaryHeader must contain exactly 80 bytes");
      }
      occ::handle<Message_ProgressIndicator> aProgress = new HostProgressIndicator();
      Message_ProgressScope aProgressScope(aProgress->Start(), "STL export", 3.0);
      aProgressScope.Show();
      TriangleMesh aMesh;
      if (theArgs.contains("positions"))
      {
        const std::string aPositionBytes = inputBufferData(theContext.buffers(), theArgs, "positions");
        const std::string anIndexBytes = inputBufferData(theContext.buffers(), theArgs, "indices");
        if (aPositionBytes.empty() || aPositionBytes.size() % (sizeof(float) * 3) != 0
            || anIndexBytes.empty() || anIndexBytes.size() % (sizeof(std::uint32_t) * 3) != 0)
          throw KernelFailure(ErrorCode::InvalidArgs, "STL mesh buffers have invalid dimensions");
        const std::size_t aVertexCount = aPositionBytes.size() / (sizeof(float) * 3);
        if (aVertexCount > static_cast<std::size_t>(std::numeric_limits<int>::max()))
          throw KernelFailure(ErrorCode::InvalidArgs, "STL mesh has too many vertices");
        aMesh.positions.reserve(aVertexCount);
        for (std::size_t aVertex = 0; aVertex < aVertexCount; ++aVertex)
        {
          std::array<float, 3> aPoint{};
          std::memcpy(aPoint.data(), aPositionBytes.data() + aVertex * sizeof(float) * 3,
                      sizeof(float) * 3);
          if (!std::isfinite(aPoint[0]) || !std::isfinite(aPoint[1]) || !std::isfinite(aPoint[2]))
            throw KernelFailure(ErrorCode::InvalidArgs, "STL mesh positions must be finite");
          aMesh.positions.emplace_back(aPoint[0], aPoint[1], aPoint[2]);
        }
        aMesh.indices.resize(anIndexBytes.size() / sizeof(std::uint32_t));
        if (aMesh.indices.size() / 3 > static_cast<std::size_t>(std::numeric_limits<int>::max()))
          throw KernelFailure(ErrorCode::InvalidArgs, "STL mesh has too many triangles");
        std::memcpy(aMesh.indices.data(), anIndexBytes.data(), anIndexBytes.size());
        for (std::size_t anOffset = 0; anOffset < aMesh.indices.size(); anOffset += 3)
        {
          const std::uint32_t anIndex0 = aMesh.indices[anOffset];
          const std::uint32_t anIndex1 = aMesh.indices[anOffset + 1];
          const std::uint32_t anIndex2 = aMesh.indices[anOffset + 2];
          if (anIndex0 >= aVertexCount || anIndex1 >= aVertexCount || anIndex2 >= aVertexCount)
            throw KernelFailure(ErrorCode::InvalidArgs, "STL mesh index is out of range");
          if (anIndex0 == anIndex1 || anIndex1 == anIndex2 || anIndex0 == anIndex2)
            throw KernelFailure(ErrorCode::InvalidArgs, "STL mesh triangle indices must be distinct");
          gp_Vec aFirstEdge(aMesh.positions[anIndex0], aMesh.positions[anIndex1]);
          aFirstEdge.Cross(gp_Vec(aMesh.positions[anIndex0], aMesh.positions[anIndex2]));
          if (aFirstEdge.SquareMagnitude() <= Precision::SquareConfusion())
            throw KernelFailure(ErrorCode::InvalidArgs, "STL mesh triangle must not be degenerate");
        }
        if (theArgs.contains("normals"))
        {
          const std::string aNormalBytes = inputBufferData(theContext.buffers(), theArgs, "normals");
          if (aNormalBytes.size() != aVertexCount * sizeof(float) * 3)
            throw KernelFailure(ErrorCode::InvalidArgs, "STL mesh normals must match positions");
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
              throw KernelFailure(ErrorCode::InvalidArgs, "STL mesh normals must be finite and non-zero");
            aMesh.normals.emplace_back(aNormal[0], aNormal[1], aNormal[2]);
          }
        }
      }
      else
      {
        aMesh = buildTriangleMesh(theContext.arena().get(requiredU32(theArgs, "shape")),
                                  theArgs.value("linearDeflection", 0.1),
                                  theArgs.value("angularDeflection", 0.5),
                                  theArgs.value("relative", false));
      }
      aProgressScope.Next().Close();
      if (!aProgressScope.More())
        throw KernelFailure(ErrorCode::Cancelled, "STL export cancelled");
      aProgressScope.Show();

      const occ::handle<Poly_Triangulation> aTriangulation = new Poly_Triangulation(
        static_cast<int>(aMesh.positions.size()), static_cast<int>(aMesh.indices.size() / 3),
        false, aMesh.normals.size() == aMesh.positions.size());
      for (std::size_t aVertex = 0; aVertex < aMesh.positions.size(); ++aVertex)
      {
        aTriangulation->SetNode(static_cast<int>(aVertex + 1), aMesh.positions[aVertex]);
        if (aTriangulation->HasNormals())
          aTriangulation->SetNormal(static_cast<int>(aVertex + 1), aMesh.normals[aVertex]);
      }
      for (std::size_t anOffset = 0; anOffset < aMesh.indices.size(); anOffset += 3)
      {
        aTriangulation->SetTriangle(static_cast<int>(anOffset / 3 + 1), Poly_Triangle(
          static_cast<int>(aMesh.indices[anOffset] + 1),
          static_cast<int>(aMesh.indices[anOffset + 1] + 1),
          static_cast<int>(aMesh.indices[anOffset + 2] + 1)));
      }
      aProgressScope.Next().Close();
      if (!aProgressScope.More())
        throw KernelFailure(ErrorCode::Cancelled, "STL export cancelled");
      aProgressScope.Show();
      std::ostringstream aStream(std::ios::out | std::ios::binary);
      const bool isWritten = anEncoding == "ascii"
        ? RWStl::WriteAscii(aTriangulation, aStream)
        : RWStl::WriteBinary(aTriangulation, aStream);
      if (!isWritten)
        throw KernelFailure(ErrorCode::ImportExportFailed, "OCCT STL write failed");
      std::string aData = aStream.str();
      if (anEncoding == "ascii" && theArgs.contains("solidName"))
      {
        const std::string aPrefix = "solid \n";
        const std::string aSuffix = "endsolid\n";
        if (aData.rfind(aPrefix, 0) != 0 || aData.size() < aSuffix.size()
            || aData.compare(aData.size() - aSuffix.size(), aSuffix.size(), aSuffix) != 0)
          throw KernelFailure(ErrorCode::ImportExportFailed, "OCCT STL ASCII output has an invalid envelope");
        aData.replace(0, aPrefix.size(), "solid " + aSolidName + "\n");
        aData.replace(aData.size() - aSuffix.size(), aSuffix.size(),
                      "endsolid " + aSolidName + "\n");
      }
      else if (!aBinaryHeader.empty())
      {
        if (aData.size() < 80)
          throw KernelFailure(ErrorCode::ImportExportFailed, "OCCT STL binary output has no header");
        std::memcpy(aData.data(), aBinaryHeader.data(), aBinaryHeader.size());
      }
      aProgressScope.Next().Close();
      if (!aProgressScope.More())
        throw KernelFailure(ErrorCode::Cancelled, "STL export cancelled");
      aProgressScope.Show();
      const std::uint32_t aBufferId = theContext.buffers().create(aData.size());
      if (!aData.empty()) std::memcpy(theContext.buffers().get(aBufferId).data(), aData.data(), aData.size());
      return {{"data", bufferDescriptor(aBufferId, aData.size(),
                                          anEncoding == "ascii" ? "stl-ascii" : "stl-binary")}};
    }

json h_importSTL(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const std::string aData = inputBufferData(theContext.buffers(), theArgs, "data");
      bool isBinary = false;
      if (aData.size() >= 84)
      {
        const auto* aBytes = reinterpret_cast<const unsigned char*>(aData.data());
        const std::uint32_t aTriangleCount = static_cast<std::uint32_t>(aBytes[80])
          | (static_cast<std::uint32_t>(aBytes[81]) << 8)
          | (static_cast<std::uint32_t>(aBytes[82]) << 16)
          | (static_cast<std::uint32_t>(aBytes[83]) << 24);
        isBinary = aTriangleCount <= (std::numeric_limits<std::size_t>::max() - 84) / 50
          && aData.size() == 84 + static_cast<std::size_t>(aTriangleCount) * 50;
      }
      occ::handle<Message_ProgressIndicator> aProgress = new HostProgressIndicator();
      Message_ProgressScope aProgressScope(aProgress->Start(), "STL import", 3.0);
      aProgressScope.Show();
      std::istringstream aStream(aData, std::ios::in | std::ios::binary);
      TopoDS_Shape aShape;
      StlAPI_Reader aReader;
      if (!aReader.Read(aShape, aStream) || aShape.IsNull())
        throw KernelFailure(ErrorCode::ImportExportFailed, "OCCT failed to import STL stream");
      aProgressScope.Next().Close();
      if (!aProgressScope.More())
        throw KernelFailure(ErrorCode::Cancelled, "STL import cancelled");
      aProgressScope.Show();
      json aResult = json::object();
      if (theArgs.value("includeMesh", false))
      {
        std::istringstream aMeshStream(aData, std::ios::in | std::ios::binary);
        const occ::handle<Poly_Triangulation> aTriangulation = RWStl::ReadStream(aMeshStream, 0.0);
        if (aTriangulation.IsNull() || aTriangulation->NbTriangles() <= 0)
          throw KernelFailure(ErrorCode::ImportExportFailed,
                              "OCCT failed to read STL triangulation stream");
        std::vector<float> aPositions;
        std::vector<float> aNormals;
        std::vector<std::uint32_t> anIndices;
        aPositions.reserve(static_cast<std::size_t>(aTriangulation->NbTriangles()) * 9);
        aNormals.reserve(aPositions.capacity());
        anIndices.reserve(static_cast<std::size_t>(aTriangulation->NbTriangles()) * 3);
        for (int aTriangleIndex = 1;
             aTriangleIndex <= aTriangulation->NbTriangles(); ++aTriangleIndex)
        {
          int aNodeIndices[3] = {};
          aTriangulation->Triangle(aTriangleIndex).Get(
            aNodeIndices[0], aNodeIndices[1], aNodeIndices[2]);
          const gp_Pnt aPoints[3] = {
            aTriangulation->Node(aNodeIndices[0]),
            aTriangulation->Node(aNodeIndices[1]),
            aTriangulation->Node(aNodeIndices[2])
          };
          gp_Vec aNormal(aPoints[0], aPoints[1]);
          aNormal.Cross(gp_Vec(aPoints[0], aPoints[2]));
          if (aNormal.SquareMagnitude() <= gp::Resolution())
            throw KernelFailure(ErrorCode::ImportExportFailed,
                                "STL contains a degenerate triangle");
          aNormal.Normalize();
          for (const gp_Pnt& aPoint : aPoints)
          {
            anIndices.push_back(static_cast<std::uint32_t>(anIndices.size()));
            aPositions.insert(aPositions.end(), {static_cast<float>(aPoint.X()),
                                                 static_cast<float>(aPoint.Y()),
                                                 static_cast<float>(aPoint.Z())});
            aNormals.insert(aNormals.end(), {static_cast<float>(aNormal.X()),
                                             static_cast<float>(aNormal.Y()),
                                             static_cast<float>(aNormal.Z())});
          }
        }
        aResult["positions"] = bufferDescriptor(
          theContext.buffers().copy(aPositions), aPositions.size() * sizeof(float), "f32x3");
        aResult["indices"] = bufferDescriptor(
          theContext.buffers().copy(anIndices), anIndices.size() * sizeof(std::uint32_t), "u32");
        aResult["normals"] = bufferDescriptor(
          theContext.buffers().copy(aNormals), aNormals.size() * sizeof(float), "f32x3");
        if (isBinary)
        {
          const std::uint32_t aHeader = theContext.buffers().create(80);
          std::memcpy(theContext.buffers().get(aHeader).data(), aData.data(), 80);
          aResult["binaryHeader"] = bufferDescriptor(aHeader, 80, "u8");
        }
        else
        {
          const std::size_t aLineEnd = aData.find('\n');
          std::string aFirstLine = aData.substr(0, aLineEnd);
          if (!aFirstLine.empty() && aFirstLine.back() == '\r') aFirstLine.pop_back();
          if (aFirstLine.rfind("solid", 0) == 0)
          {
            std::size_t aNameStart = 5;
            while (aNameStart < aFirstLine.size()
                   && (aFirstLine[aNameStart] == ' ' || aFirstLine[aNameStart] == '\t'))
              ++aNameStart;
            aResult["solidName"] = aFirstLine.substr(aNameStart);
          }
        }
      }
      aProgressScope.Next().Close();
      if (!aProgressScope.More())
        throw KernelFailure(ErrorCode::Cancelled, "STL import cancelled");
      aProgressScope.Show();
      aResult["shape"] = theContext.arena().add(aShape, aScope);
      aProgressScope.Next().Close();
      if (!aProgressScope.More())
        throw KernelFailure(ErrorCode::Cancelled, "STL import cancelled");
      aProgressScope.Show();
      return aResult;
    }

json h_exportVRML(KernelOperationContext& theContext, const json& theArgs)
{
      const double aLinear = theArgs.value("linearDeflection", 0.1);
      const double anAngular = theArgs.value("angularDeflection", 0.5);
      const bool isRelative = theArgs.value("relative", false);
      const int aVersion = theArgs.value("version", 2);
      if (aLinear <= 0.0 || anAngular <= 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs, "VRML deflections must be positive");
      if (aVersion != 1 && aVersion != 2)
        throw KernelFailure(ErrorCode::InvalidArgs, "VRML version must be 1 or 2");

      gp_Trsf aToYUp;
      aToYUp.SetRotation(gp_Ax1(gp::Origin(), gp::DX()), -M_PI_2);
      const bool isMeshInput = theArgs.contains("positions");
      TriangleMesh anInputMesh;
      TopoDS_Shape aSourceShape;
      if (isMeshInput)
      {
        anInputMesh = parseIndexedTriangleMesh(theContext.buffers(), theArgs, "VRML");
        aSourceShape = makeTriangleShape(anInputMesh);
      }
      else
      {
        aSourceShape = theContext.arena().get(requiredU32(theArgs, "shape"));
      }
      TopoDS_Shape aShape = BRepBuilderAPI_Transform(aSourceShape, aToYUp, true).Shape();
      IMeshTools_Parameters aParameters;
      aParameters.Deflection = aLinear;
      aParameters.Angle = anAngular;
      aParameters.Relative = isRelative;
      aParameters.InParallel = false;
      occ::handle<Message_ProgressIndicator> aProgress = new HostProgressIndicator();
      Message_ProgressScope aProgressScope(aProgress->Start(), "VRML export", 2.0);
      BRepTools::Clean(aShape);
      BRepMesh_IncrementalMesh aMesher(aShape, aParameters, aProgressScope.Next());
      if (!aProgressScope.More())
        throw KernelFailure(ErrorCode::Cancelled, "OCCT VRML export cancelled");
      if (!aMesher.IsDone())
        throw KernelFailure(ErrorCode::TessellationFailed, "OCCT VRML tessellation failed");

      std::string aData;
      if (isMeshInput && aVersion == 2)
      {
        aData = writeVRMLIndexedMesh(anInputMesh, aToYUp);
      }
      else
      {
        VrmlAPI_Writer aWriter;
        aWriter.SetRepresentation(VrmlAPI_ShadedRepresentation);
        aWriter.SetDeflection(aLinear);
        std::ostringstream aStream;
        if (!aWriter.Write(aShape, aStream, aVersion))
          throw KernelFailure(ErrorCode::ImportExportFailed, "OCCT VRML stream write failed");
        aData = aStream.str();
      }
      aProgressScope.Next();
      if (!aProgressScope.More())
        throw KernelFailure(ErrorCode::Cancelled, "OCCT VRML export cancelled");
      aProgressScope.Show();
      const std::uint32_t aBuffer = theContext.buffers().create(aData.size());
      if (!aData.empty())
        std::memcpy(theContext.buffers().get(aBuffer).data(), aData.data(), aData.size());
      return {{"data", bufferDescriptor(aBuffer, aData.size(), "vrml-utf8")}};
    }

json h_importVRML(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const std::string aData = inputBufferData(theContext.buffers(), theArgs, "data");
      const bool includeMesh = theArgs.value("includeMesh", false);
      TriangleMesh aSceneMesh;
      if (includeMesh && aData.rfind("#VRML V2.0", 0) == 0)
      {
        std::istringstream aSceneStream(aData);
        VrmlData_Scene aScene;
        aScene << aSceneStream;
        if (aScene.Status() == VrmlData_StatusOK)
          aSceneMesh = vrmlSceneMesh(aScene);
      }
      std::istringstream aStream(aData);
      VrmlAPI_CafReader aReader;
      aReader.SetFileLengthUnit(1.0);
      aReader.SetSystemLengthUnit(1.0);
      aReader.SetFileCoordinateSystem(RWMesh_CoordinateSystem_Yup);
      aReader.SetSystemCoordinateSystem(RWMesh_CoordinateSystem_Zup);
      aReader.SetFillIncompleteDocument(false);
      occ::handle<Message_ProgressIndicator> aProgress = new HostProgressIndicator();
      Message_ProgressScope aProgressScope(aProgress->Start(), "VRML import", 1.0);
      const bool isRead = aReader.Perform(aStream, aProgressScope.Next());
      if (!aProgressScope.More())
        throw KernelFailure(ErrorCode::Cancelled, "OCCT VRML import cancelled");
      if (!isRead || aReader.SingleShape().IsNull())
        throw KernelFailure(ErrorCode::ImportExportFailed, "OCCT failed to import VRML stream");
      gp_Trsf aToZUp;
      aToZUp.SetRotation(gp_Ax1(gp::Origin(), gp::DX()), M_PI_2);
      const TopoDS_Shape aShape = BRepBuilderAPI_Transform(
        aReader.SingleShape(), aToZUp, true, true).Shape();
      const TriangleMesh aMesh = aSceneMesh.indices.empty() ? triangleMesh(aShape) : aSceneMesh;
      json aResult = {{"shape", theContext.arena().add(makeTriangleShape(aMesh), aScope)}};
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
        aResult["normals"] = bufferDescriptor(
          theContext.buffers().copy(aNormals), aNormals.size() * sizeof(float), "f32x3");
        if (!aUVs.empty())
          aResult["uvs"] = bufferDescriptor(
            theContext.buffers().copy(aUVs), aUVs.size() * sizeof(float), "f32x2");
        if (!aMesh.colors.empty())
          aResult["colors"] = bufferDescriptor(
            theContext.buffers().copy(aMesh.colors), aMesh.colors.size() * 4, "u8x4");
      }
      return aResult;
    }

void register_exchange_mesh_operations()
{
  auto& aRegistry = OperationRegistry::instance();
#define OCCT_REGISTER_HANDLER(theOperation) aRegistry.add(#theOperation, &h_##theOperation);
  OCCT_EXCHANGE_MESH_OPERATION_TABLE(OCCT_REGISTER_HANDLER)
#undef OCCT_REGISTER_HANDLER
}

OCCT_DEFINE_SIDE_PLUGIN(exchange_mesh, kExchangeMeshOperationNames, register_exchange_mesh_operations)

} // namespace occt_worker
