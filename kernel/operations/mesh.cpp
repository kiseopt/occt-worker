// Kernel mesh operations: one out-of-line MeshOperations::h_* handler per dispatch branch.
// Handler bodies are byte-identical fragments of the original dispatchOperation.
// As a shared Side module the constructor registers into the Main registry
// right after Emscripten runs __wasm_call_ctors.

#include "kernel_mesh_operations.hpp"
#include "kernel_history_helpers.hpp"
#include "kernel_operation_context.hpp"
#include "kernel_protocol_helpers.hpp"
#include "side_registration.hpp"

namespace occt_worker {

json h_validateTriangulation(KernelOperationContext& theContext, const json& theArgs)
{
      const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
      const double aTolerance = theArgs.value("tolerance", 1.0e-7);
      if (aTolerance <= 0.0 || !std::isfinite(aTolerance))
        throw KernelFailure(ErrorCode::InvalidArgs, "Triangulation validation tolerance must be positive and finite");
      const bool checkNormals = theArgs.value("checkNormals", true);
      const bool checkUV = theArgs.value("checkUV", false);
      TopTools_IndexedMapOfShape aFaces;
      TopExp::MapShapes(aShape, TopAbs_FACE, aFaces);
      json anIssues = json::array();
      std::uint32_t aTriangulatedFaces = 0;
      std::uint32_t aTriangleCount = 0;
      auto addIssue = [&](const int theFaceIndex, const char* theCode,
                          const int theNodeIndex = 0, const int theTriangleIndex = 0) {
        json anIssue{{"faceIndex", theFaceIndex - 1}, {"code", theCode}};
        if (theNodeIndex > 0) anIssue["nodeIndex"] = theNodeIndex - 1;
        if (theTriangleIndex > 0) anIssue["triangleIndex"] = theTriangleIndex - 1;
        anIssues.push_back(std::move(anIssue));
      };
      for (int aFaceIndex = 1; aFaceIndex <= aFaces.Extent(); ++aFaceIndex)
      {
        const TopoDS_Face aFace = TopoDS::Face(aFaces(aFaceIndex));
        TopLoc_Location aLocation;
        const occ::handle<Poly_Triangulation>& aTriangulation =
          BRep_Tool::Triangulation(aFace, aLocation);
        if (aTriangulation.IsNull())
        {
          addIssue(aFaceIndex, "missingTriangulation");
          continue;
        }
        if (aTriangulation->NbNodes() <= 0 || aTriangulation->NbTriangles() <= 0)
        {
          addIssue(aFaceIndex, "emptyTriangulation");
          continue;
        }
        ++aTriangulatedFaces;
        aTriangleCount += static_cast<std::uint32_t>(aTriangulation->NbTriangles());
        if (checkNormals && !aTriangulation->HasNormals()) addIssue(aFaceIndex, "missingNormals");
        if (checkUV && !aTriangulation->HasUVNodes()) addIssue(aFaceIndex, "missingUV");
        for (int aNodeIndex = 1; aNodeIndex <= aTriangulation->NbNodes(); ++aNodeIndex)
        {
          const gp_Pnt aPoint = aTriangulation->Node(aNodeIndex);
          if (!std::isfinite(aPoint.X()) || !std::isfinite(aPoint.Y()) || !std::isfinite(aPoint.Z()))
            addIssue(aFaceIndex, "nonFinitePosition", aNodeIndex);
          if (checkNormals && aTriangulation->HasNormals())
          {
            const gp_Dir aNormal = aTriangulation->Normal(aNodeIndex);
            if (!std::isfinite(aNormal.X()) || !std::isfinite(aNormal.Y()) || !std::isfinite(aNormal.Z()))
              addIssue(aFaceIndex, "nonFiniteNormal", aNodeIndex);
          }
          if (checkUV && aTriangulation->HasUVNodes())
          {
            const gp_Pnt2d aUV = aTriangulation->UVNode(aNodeIndex);
            if (!std::isfinite(aUV.X()) || !std::isfinite(aUV.Y()))
              addIssue(aFaceIndex, "nonFiniteUV", aNodeIndex);
          }
        }
        for (int aTriangleIndex = 1; aTriangleIndex <= aTriangulation->NbTriangles(); ++aTriangleIndex)
        {
          int anIndex0 = 0;
          int anIndex1 = 0;
          int anIndex2 = 0;
          aTriangulation->Triangle(aTriangleIndex).Get(anIndex0, anIndex1, anIndex2);
          if (anIndex0 < 1 || anIndex1 < 1 || anIndex2 < 1
              || anIndex0 > aTriangulation->NbNodes()
              || anIndex1 > aTriangulation->NbNodes()
              || anIndex2 > aTriangulation->NbNodes())
          {
            addIssue(aFaceIndex, "invalidTriangle", 0, aTriangleIndex);
            continue;
          }
          const gp_Pnt aPoint0 = aTriangulation->Node(anIndex0);
          const gp_Pnt aPoint1 = aTriangulation->Node(anIndex1);
          const gp_Pnt aPoint2 = aTriangulation->Node(anIndex2);
          const gp_Vec aCross(gp_Vec(aPoint0, aPoint1).Crossed(gp_Vec(aPoint0, aPoint2)));
          if (aCross.SquareMagnitude() <= aTolerance * aTolerance)
            addIssue(aFaceIndex, "degenerateTriangle", 0, aTriangleIndex);
        }
      }
      return {{"valid", anIssues.empty()},
              {"faces", aFaces.Extent()},
              {"triangulatedFaces", aTriangulatedFaces},
              {"triangles", aTriangleCount},
              {"issues", std::move(anIssues)}};
    }

json h_repairTriangulation(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
      const double aLinear = theArgs.value("linearDeflection", 0.1);
      const double anAngular = theArgs.value("angularDeflection", 0.5);
      if (aLinear <= 0.0 || !std::isfinite(aLinear) || anAngular <= 0.0 || !std::isfinite(anAngular))
        throw KernelFailure(ErrorCode::InvalidArgs, "Triangulation repair deflections must be positive and finite");
      TopTools_IndexedMapOfShape aFaces;
      TopExp::MapShapes(aShape, TopAbs_FACE, aFaces);
      if (aFaces.IsEmpty())
        throw KernelFailure(ErrorCode::InvalidArgs, "Triangulation repair requires at least one face");
      BRepBuilderAPI_Copy aCopy(aShape, true, false);
      if (!aCopy.IsDone() || aCopy.Shape().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to copy the shape for triangulation repair");
      TopoDS_Shape aRepaired = aCopy.Shape();
      BRepTools::Clean(aRepaired);
      BRepMesh_IncrementalMesh aMesher(
        aRepaired, aLinear, theArgs.value("relative", false), anAngular, false);
      if (!aMesher.IsDone() || aMesher.GetStatusFlags() & (IMeshData_OpenWire
          | IMeshData_SelfIntersectingWire | IMeshData_Failure
          | IMeshData_TooFewPoints | IMeshData_UserBreak))
        throw KernelFailure(ErrorCode::TessellationFailed, "OCCT triangulation repair failed");
      TopTools_IndexedMapOfShape aRepairedFaces;
      TopExp::MapShapes(aRepaired, TopAbs_FACE, aRepairedFaces);
      std::uint32_t aTriangulatedFaces = 0;
      std::uint32_t aTriangleCount = 0;
      for (int aFaceIndex = 1; aFaceIndex <= aRepairedFaces.Extent(); ++aFaceIndex)
      {
        TopLoc_Location aLocation;
        const TopoDS_Face aFace = TopoDS::Face(aRepairedFaces(aFaceIndex));
        const occ::handle<Poly_Triangulation>& aTriangulation = BRep_Tool::Triangulation(
          aFace, aLocation);
        if (!aTriangulation.IsNull())
        {
          if (!aTriangulation->HasNormals())
            BRepLib_ToolTriangulatedShape::ComputeNormals(aFace, aTriangulation);
          ++aTriangulatedFaces;
          aTriangleCount += static_cast<std::uint32_t>(aTriangulation->NbTriangles());
        }
      }
      if (aTriangulatedFaces != static_cast<std::uint32_t>(aRepairedFaces.Extent())
          || aTriangleCount == 0)
        throw KernelFailure(ErrorCode::TessellationFailed, "OCCT triangulation repair produced an incomplete mesh");
      return {{"shape", theContext.arena().add(aRepaired, aScope)},
              {"faces", aFaces.Extent()},
              {"triangulatedFaces", aTriangulatedFaces},
              {"triangles", aTriangleCount}};
    }

json h_replaceTriangulation(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
      if (aShape.ShapeType() != TopAbs_FACE)
        throw KernelFailure(ErrorCode::InvalidArgs, "Triangulation replacement requires a face shape");
      const json& aPositions = theArgs.at("positions");
      const json& anIndices = theArgs.at("indices");
      if (!aPositions.is_array() || aPositions.size() < 3 || !anIndices.is_array()
          || anIndices.empty() || anIndices.size() % 3 != 0)
        throw KernelFailure(ErrorCode::InvalidArgs, "Triangulation positions and indices must contain triangles");
      const bool hasNormals = theArgs.contains("normals");
      const bool hasUVs = theArgs.contains("uvs");
      if (hasNormals && (!theArgs.at("normals").is_array()
                         || theArgs.at("normals").size() != aPositions.size()))
        throw KernelFailure(ErrorCode::InvalidArgs, "Triangulation normals must match positions");
      if (hasUVs && (!theArgs.at("uvs").is_array()
                     || theArgs.at("uvs").size() != aPositions.size() * 2))
        throw KernelFailure(ErrorCode::InvalidArgs, "Triangulation UVs must contain two numbers per position");

      BRepBuilderAPI_Copy aCopy(TopoDS::Face(aShape), true, false);
      if (!aCopy.IsDone() || aCopy.Shape().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to copy the face for triangulation replacement");
      TopoDS_Face aFace = TopoDS::Face(aCopy.Shape());
      const occ::handle<Poly_Triangulation> aTriangulation = new Poly_Triangulation(
        static_cast<int>(aPositions.size()), static_cast<int>(anIndices.size() / 3), hasUVs, hasNormals);
      for (int aNodeIndex = 1; aNodeIndex <= aTriangulation->NbNodes(); ++aNodeIndex)
      {
        const json& aPoint = aPositions.at(static_cast<std::size_t>(aNodeIndex - 1));
        if (!aPoint.is_array() || aPoint.size() != 3 || !aPoint.at(0).is_number()
            || !aPoint.at(1).is_number() || !aPoint.at(2).is_number())
          throw KernelFailure(ErrorCode::InvalidArgs, "Triangulation positions must be vec3 values");
        aTriangulation->SetNode(aNodeIndex, gp_Pnt(aPoint.at(0).get<double>(),
                                                   aPoint.at(1).get<double>(),
                                                   aPoint.at(2).get<double>()));
        if (hasNormals)
        {
          const json& aNormal = theArgs.at("normals").at(static_cast<std::size_t>(aNodeIndex - 1));
          if (!aNormal.is_array() || aNormal.size() != 3 || !aNormal.at(0).is_number()
              || !aNormal.at(1).is_number() || !aNormal.at(2).is_number())
            throw KernelFailure(ErrorCode::InvalidArgs, "Triangulation normals must be vec3 values");
          aTriangulation->SetNormal(aNodeIndex, gp_Dir(aNormal.at(0).get<double>(),
                                                       aNormal.at(1).get<double>(),
                                                       aNormal.at(2).get<double>()));
        }
        if (hasUVs)
        {
          const json& anUVs = theArgs.at("uvs");
          aTriangulation->SetUVNode(aNodeIndex, gp_Pnt2d(
            anUVs.at(static_cast<std::size_t>((aNodeIndex - 1) * 2)).get<double>(),
            anUVs.at(static_cast<std::size_t>((aNodeIndex - 1) * 2 + 1)).get<double>()));
        }
      }
      for (int aTriangleIndex = 1; aTriangleIndex <= aTriangulation->NbTriangles(); ++aTriangleIndex)
      {
        const std::size_t anOffset = static_cast<std::size_t>((aTriangleIndex - 1) * 3);
        const std::uint32_t anIndex0 = anIndices.at(anOffset).get<std::uint32_t>();
        const std::uint32_t anIndex1 = anIndices.at(anOffset + 1).get<std::uint32_t>();
        const std::uint32_t anIndex2 = anIndices.at(anOffset + 2).get<std::uint32_t>();
        if (anIndex0 >= aPositions.size() || anIndex1 >= aPositions.size() || anIndex2 >= aPositions.size()
            || anIndex0 == anIndex1 || anIndex1 == anIndex2 || anIndex0 == anIndex2)
          throw KernelFailure(ErrorCode::InvalidArgs, "Triangulation triangle index is invalid");
        aTriangulation->SetTriangle(aTriangleIndex, Poly_Triangle(
          static_cast<int>(anIndex0 + 1), static_cast<int>(anIndex1 + 1), static_cast<int>(anIndex2 + 1)));
      }
      BRep_Builder aShapeBuilder;
      aShapeBuilder.UpdateFace(aFace, aTriangulation);
      if (!BRepCheck_Analyzer(aFace, true).IsValid())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT rejected the replacement triangulation");
      return {{"shape", theContext.arena().add(aFace, aScope)}};
    }

void register_mesh_operations()
{
  auto& aRegistry = OperationRegistry::instance();
#define OCCT_REGISTER_HANDLER(theOperation) aRegistry.add(#theOperation, &h_##theOperation);
  OCCT_MESH_OPERATION_TABLE(OCCT_REGISTER_HANDLER)
#undef OCCT_REGISTER_HANDLER
}

OCCT_DEFINE_SIDE_PLUGIN(mesh, kMeshOperationNames, register_mesh_operations)

} // namespace occt_worker
