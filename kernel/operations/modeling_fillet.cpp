#include "kernel_modeling_operations.hpp"
#include "kernel_history_helpers.hpp"
#include "kernel_operation_context.hpp"
#include "kernel_protocol_helpers.hpp"

namespace occt_worker {

namespace {
enum class EdgeBlendMode { Fillet, Chamfer };

json applyEdgeBlend(KernelOperationContext& theContext, const json& theArgs, EdgeBlendMode theMode)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
      if (!theArgs.contains("edgeIndices") || !theArgs.at("edgeIndices").is_array() || theArgs.at("edgeIndices").empty())
        throw KernelFailure(ErrorCode::InvalidArgs,
                            std::string(theMode == EdgeBlendMode::Fillet ? "fillet" : "chamfer")
                              + " requires edgeIndices");
      TopTools_IndexedMapOfShape anEdges;
      TopExp::MapShapes(aShape, TopAbs_EDGE, anEdges);
      const double aDistance = requiredNumber(
        theArgs, theMode == EdgeBlendMode::Fillet ? "radius" : "distance");
      if (aDistance <= 0.0) throw KernelFailure(ErrorCode::InvalidArgs, "Fillet/chamfer size must be positive");
      if (theMode == EdgeBlendMode::Fillet)
      {
        BRepFilletAPI_MakeFillet aBuilder(aShape);
        const bool hasSecondRadius = theArgs.contains("radius2");
        const double aSecondRadius = hasSecondRadius ? requiredNumber(theArgs, "radius2") : aDistance;
        if (aSecondRadius <= 0.0) throw KernelFailure(ErrorCode::InvalidArgs, "Fillet radius2 must be positive");
        const bool hasPerEdgeRadii = theArgs.contains("radii");
        if (hasPerEdgeRadii && (!theArgs.at("radii").is_array()
                                || theArgs.at("radii").size() != theArgs.at("edgeIndices").size()))
          throw KernelFailure(ErrorCode::InvalidArgs, "Fillet radii must match edgeIndices");
        const bool hasRadiusLaw = theArgs.contains("radiusLaw");
        const bool hasPerEdgeRadiusLaws = theArgs.contains("radiusLaws");
        if (hasRadiusLaw && hasPerEdgeRadiusLaws)
          throw KernelFailure(ErrorCode::InvalidArgs, "Fillet accepts radiusLaw or radiusLaws, not both");
        if ((hasRadiusLaw || hasPerEdgeRadiusLaws) && (hasPerEdgeRadii || hasSecondRadius))
          throw KernelFailure(ErrorCode::InvalidArgs,
                              "Fillet radius laws cannot be combined with radii or radius2");
        if (hasPerEdgeRadiusLaws
            && (!theArgs.at("radiusLaws").is_array()
                || theArgs.at("radiusLaws").size() != theArgs.at("edgeIndices").size()))
          throw KernelFailure(ErrorCode::InvalidArgs, "Fillet radiusLaws must match edgeIndices");
        std::size_t anEdgeOrdinal = 0;
        for (const json& anIndex : theArgs.at("edgeIndices"))
        {
          const std::int64_t anEdgeIndex = anIndex.get<std::int64_t>();
          if (anEdgeIndex < 0 || anEdgeIndex >= anEdges.Extent()) throw KernelFailure(ErrorCode::InvalidArgs, "Fillet edge index out of range");
          const TopoDS_Edge anEdge = TopoDS::Edge(anEdges(static_cast<int>(anEdgeIndex) + 1));
          const double aRadius = hasPerEdgeRadii ? theArgs.at("radii").at(anEdgeOrdinal).get<double>() : aDistance;
          if (!(aRadius > 0.0) || !std::isfinite(aRadius))
            throw KernelFailure(ErrorCode::InvalidArgs, "Fillet radii must be positive and finite");
          if (hasRadiusLaw || hasPerEdgeRadiusLaws)
          {
            const json& aLaw = hasPerEdgeRadiusLaws
              ? theArgs.at("radiusLaws").at(anEdgeOrdinal) : theArgs.at("radiusLaw");
            if (!aLaw.is_array() || aLaw.size() < 3)
              throw KernelFailure(ErrorCode::InvalidArgs, "Fillet radius law requires at least three points");
            NCollection_Array1<gp_Pnt2d> aLawPoints(1, static_cast<int>(aLaw.size()));
            double aPreviousParameter = -1.0;
            for (std::size_t aPointIndex = 0; aPointIndex < aLaw.size(); ++aPointIndex)
            {
              const json& aPoint = aLaw.at(aPointIndex);
              if (!aPoint.is_object())
                throw KernelFailure(ErrorCode::InvalidArgs, "Fillet radius law points must be objects");
              const double aParameter = requiredNumber(aPoint, "parameter");
              const double aLawRadius = requiredNumber(aPoint, "radius");
              if (!std::isfinite(aParameter) || aParameter < 0.0 || aParameter > 1.0
                  || aParameter <= aPreviousParameter)
                throw KernelFailure(ErrorCode::InvalidArgs,
                                    "Fillet radius law parameters must increase strictly from 0 to 1");
              if (!(aLawRadius > 0.0) || !std::isfinite(aLawRadius))
                throw KernelFailure(ErrorCode::InvalidArgs,
                                    "Fillet radius law radii must be positive and finite");
              aLawPoints.SetValue(static_cast<int>(aPointIndex) + 1, gp_Pnt2d(aParameter, aLawRadius));
              aPreviousParameter = aParameter;
            }
            if (aLawPoints.First().X() != 0.0 || aLawPoints.Last().X() != 1.0)
              throw KernelFailure(ErrorCode::InvalidArgs, "Fillet radius law must start at 0 and end at 1");
            aBuilder.Add(aLawPoints, anEdge);
          }
          else if (hasSecondRadius) aBuilder.Add(aRadius, aSecondRadius, anEdge);
          else aBuilder.Add(aRadius, anEdge);
          ++anEdgeOrdinal;
        }
        aBuilder.Build();
        if (!aBuilder.IsDone() || aBuilder.Shape().IsNull()) throw KernelFailure(ErrorCode::FilletFailed, "OCCT fillet failed");
        json aResult{{"shape", theContext.arena().add(aBuilder.Shape(), aScope)}};
        if (theArgs.value("includeHistory", false))
          aResult["history"] = buildFilletHistory(aBuilder, aShape, aBuilder.Shape());
        return aResult;
      }
      BRepFilletAPI_MakeChamfer aBuilder(aShape);
      const bool hasPerEdgeDistances = theArgs.contains("distances");
      if (hasPerEdgeDistances && (!theArgs.at("distances").is_array()
                                  || theArgs.at("distances").size() != theArgs.at("edgeIndices").size()))
        throw KernelFailure(ErrorCode::InvalidArgs, "Chamfer distances must match edgeIndices");
      const bool hasSecondDistance = theArgs.contains("distance2");
      const bool hasPerEdgeSecondDistances = theArgs.contains("distances2");
      const bool hasReferenceFaces = theArgs.contains("referenceFaceIndices");
      const bool usesTwoDistances = hasSecondDistance || hasPerEdgeSecondDistances || hasReferenceFaces;
      if (usesTwoDistances && (!hasReferenceFaces || (!hasSecondDistance && !hasPerEdgeSecondDistances)))
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "Two-distance chamfer requires distance2 or distances2 and referenceFaceIndices");
      if (hasPerEdgeSecondDistances
          && (!theArgs.at("distances2").is_array()
              || theArgs.at("distances2").size() != theArgs.at("edgeIndices").size()))
        throw KernelFailure(ErrorCode::InvalidArgs, "Chamfer distances2 must match edgeIndices");
      if (hasReferenceFaces
          && (!theArgs.at("referenceFaceIndices").is_array()
              || theArgs.at("referenceFaceIndices").size() != theArgs.at("edgeIndices").size()))
        throw KernelFailure(ErrorCode::InvalidArgs, "Chamfer referenceFaceIndices must match edgeIndices");
      const double aSecondDistance = hasSecondDistance ? requiredNumber(theArgs, "distance2") : 0.0;
      if (hasSecondDistance && (!(aSecondDistance > 0.0) || !std::isfinite(aSecondDistance)))
        throw KernelFailure(ErrorCode::InvalidArgs, "Chamfer distance2 must be positive and finite");
      TopTools_IndexedMapOfShape aFaces;
      if (usesTwoDistances) TopExp::MapShapes(aShape, TopAbs_FACE, aFaces);
      std::size_t anEdgeOrdinal = 0;
      for (const json& anIndex : theArgs.at("edgeIndices"))
      {
        const std::int64_t anEdgeIndex = anIndex.get<std::int64_t>();
        if (anEdgeIndex < 0 || anEdgeIndex >= anEdges.Extent()) throw KernelFailure(ErrorCode::InvalidArgs, "Chamfer edge index out of range");
        const TopoDS_Edge anEdge = TopoDS::Edge(anEdges(static_cast<int>(anEdgeIndex) + 1));
        const double aChamferDistance = hasPerEdgeDistances
          ? theArgs.at("distances").at(anEdgeOrdinal).get<double>() : aDistance;
        if (!(aChamferDistance > 0.0) || !std::isfinite(aChamferDistance))
          throw KernelFailure(ErrorCode::InvalidArgs, "Chamfer distances must be positive and finite");
        if (usesTwoDistances)
        {
          const double anEdgeSecondDistance = hasPerEdgeSecondDistances
            ? theArgs.at("distances2").at(anEdgeOrdinal).get<double>() : aSecondDistance;
          if (!(anEdgeSecondDistance > 0.0) || !std::isfinite(anEdgeSecondDistance))
            throw KernelFailure(ErrorCode::InvalidArgs, "Chamfer distances2 must be positive and finite");
          const std::int64_t aFaceIndex = theArgs.at("referenceFaceIndices").at(anEdgeOrdinal).get<std::int64_t>();
          if (aFaceIndex < 0 || aFaceIndex >= aFaces.Extent())
            throw KernelFailure(ErrorCode::InvalidArgs, "Chamfer reference face index out of range");
          const TopoDS_Face aFace = TopoDS::Face(aFaces(static_cast<int>(aFaceIndex) + 1));
          TopTools_IndexedMapOfShape aFaceEdges;
          TopExp::MapShapes(aFace, TopAbs_EDGE, aFaceEdges);
          if (aFaceEdges.FindIndex(anEdge) == 0)
            throw KernelFailure(ErrorCode::InvalidArgs, "Chamfer reference face must be adjacent to its edge");
          aBuilder.Add(aChamferDistance, anEdgeSecondDistance, anEdge, aFace);
        }
        else aBuilder.Add(aChamferDistance, anEdge);
        ++anEdgeOrdinal;
      }
      aBuilder.Build();
      if (!aBuilder.IsDone() || aBuilder.Shape().IsNull()) throw KernelFailure(ErrorCode::FilletFailed, "OCCT chamfer failed");
      json aResult{{"shape", theContext.arena().add(aBuilder.Shape(), aScope)}};
      if (theArgs.value("includeHistory", false))
        aResult["history"] = buildFilletHistory(aBuilder, aShape, aBuilder.Shape());
      return aResult;
    }

} // namespace

json h_fillet(KernelOperationContext& theContext, const json& theArgs)
{
  return applyEdgeBlend(theContext, theArgs, EdgeBlendMode::Fillet);
}

json h_chamfer(KernelOperationContext& theContext, const json& theArgs)
{
  return applyEdgeBlend(theContext, theArgs, EdgeBlendMode::Chamfer);
}

} // namespace occt_worker
