// Kernel algorithms operations: one out-of-line AlgorithmsOperations::h_* handler per dispatch branch.
// Handler bodies are byte-identical fragments of the original dispatchOperation.
// As a shared Side module the constructor registers into the Main registry
// right after Emscripten runs __wasm_call_ctors.

#include "kernel_algorithms_operations.hpp"
#include "kernel_geometry_helpers.hpp"
#include "kernel_history_helpers.hpp"
#include "kernel_operation_context.hpp"
#include "kernel_protocol_helpers.hpp"
#include "side_registration.hpp"

namespace occt_worker {

json h_middlePath(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
      const TopoDS_Shape& aStart = theContext.arena().get(requiredU32(theArgs, "start"));
      const TopoDS_Shape& anEnd = theContext.arena().get(requiredU32(theArgs, "end"));
      for (const auto& aSection : std::vector<std::pair<const TopoDS_Shape*, const char*>>{
             {&aStart, "start"}, {&anEnd, "end"}})
      {
        const TopAbs_ShapeEnum aType = aSection.first->ShapeType();
        if (aType != TopAbs_FACE && aType != TopAbs_WIRE)
          throw KernelFailure(ErrorCode::InvalidArgs,
                              std::string("middlePath ") + aSection.second + " must be a face or wire");
        TopTools_IndexedMapOfShape aSections;
        TopExp::MapShapes(aShape, aType, aSections);
        if (aSections.FindIndex(*aSection.first) == 0)
          throw KernelFailure(ErrorCode::InvalidArgs,
                              std::string("middlePath ") + aSection.second
                                + " must be a subshape of shape");
      }
      BRepOffsetAPI_MiddlePath aBuilder(aShape, aStart, anEnd);
      aBuilder.Build();
      if (!aBuilder.IsDone() || aBuilder.Shape().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT middle path construction failed");
      return {{"shape", theContext.arena().add(aBuilder.Shape(), aScope)}};
    }

json h_sweepPipeShell(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape aSpine = theContext.arena().get(requiredU32(theArgs, "spine"));
      BRepOffsetAPI_MakePipeShell aBuilder(TopoDS::Wire(aSpine));
      std::vector<TopoDS_Shape> aHistoryInputs{aSpine};
      const std::string aMode = theArgs.value("mode", "frenet");
      if (aMode == "frenet") aBuilder.SetMode(true);
      else if (aMode == "correctedFrenet") aBuilder.SetMode(false);
      else if (aMode == "fixedAxis")
      {
        if (!theArgs.contains("axis") || !theArgs.at("axis").is_object())
          throw KernelFailure(ErrorCode::InvalidArgs, "fixedAxis mode requires axis");
        const json& anAxisValue = theArgs.at("axis");
        const auto anOrigin = anAxisValue.contains("origin")
          ? requiredVec3(anAxisValue, "origin") : std::array<double, 3>{0.0, 0.0, 0.0};
        const auto aDirection = requiredDirection(anAxisValue, "direction");
        if (anAxisValue.contains("xDirection"))
        {
          const auto anXDirection = requiredDirection(anAxisValue, "xDirection");
          if (gp_Dir(aDirection[0], aDirection[1], aDirection[2]).IsParallel(
                gp_Dir(anXDirection[0], anXDirection[1], anXDirection[2]), 1.0e-12))
            throw KernelFailure(ErrorCode::InvalidArgs, "Pipe-shell direction and xDirection must not be parallel");
          aBuilder.SetMode(gp_Ax2(gp_Pnt(anOrigin[0], anOrigin[1], anOrigin[2]),
                                  gp_Dir(aDirection[0], aDirection[1], aDirection[2]),
                                  gp_Dir(anXDirection[0], anXDirection[1], anXDirection[2])));
        }
        else
        {
          aBuilder.SetMode(gp_Ax2(gp_Pnt(anOrigin[0], anOrigin[1], anOrigin[2]),
                                  gp_Dir(aDirection[0], aDirection[1], aDirection[2])));
        }
      }
      else if (aMode == "auxiliarySpine")
      {
        const TopoDS_Shape anAuxiliarySpineShape = theContext.arena().get(requiredU32(theArgs, "auxiliarySpine"));
        const TopoDS_Wire anAuxiliarySpine = TopoDS::Wire(anAuxiliarySpineShape);
        aBuilder.SetMode(anAuxiliarySpine, theArgs.value("curvilinearEquivalence", false));
        aHistoryInputs.push_back(anAuxiliarySpineShape);
      }
      else throw KernelFailure(ErrorCode::InvalidArgs, "Unknown pipe-shell mode");
      if (!theArgs.contains("profiles") || !theArgs.at("profiles").is_array() || theArgs.at("profiles").empty())
        throw KernelFailure(ErrorCode::InvalidArgs, "sweepPipeShell requires profiles");
      std::vector<TopoDS_Shape> aProfiles;
      aProfiles.reserve(theArgs.at("profiles").size());
      for (const json& aProfile : theArgs.at("profiles"))
      {
        aProfiles.push_back(theContext.arena().get(aProfile.get<std::uint32_t>()));
        aBuilder.Add(aProfiles.back());
      }
      aHistoryInputs.insert(aHistoryInputs.begin() + 1, aProfiles.begin(), aProfiles.end());
      aBuilder.Build();
      if (!aBuilder.IsDone() || aBuilder.Shape().IsNull()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT pipe-shell sweep failed");
      json aHistory;
      if (theArgs.value("includeHistory", false))
        aHistory = buildHistory(aBuilder, aHistoryInputs, aBuilder.Shape());
      json aResult{{"shape", theContext.arena().add(aBuilder.Shape(), aScope)}};
      if (theArgs.value("includeHistory", false))
        aResult["history"] = std::move(aHistory);
      return aResult;
    }

json h_offsetShape(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const double anOffset = requiredNumber(theArgs, "offset");
      if (anOffset == 0.0) throw KernelFailure(ErrorCode::InvalidArgs, "Offset cannot be zero");
      BRepOffsetAPI_MakeOffsetShape aBuilder;
      aBuilder.PerformByJoin(theContext.arena().get(requiredU32(theArgs, "shape")), anOffset, theArgs.value("tolerance", 1.0e-6));
      if (!aBuilder.IsDone() || aBuilder.Shape().IsNull()) throw KernelFailure(ErrorCode::HealingFailed, "OCCT offset failed");
      return {{"shape", theContext.arena().add(aBuilder.Shape(), aScope)}};
    }

json h_offsetWire2D(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const double anOffset = requiredNumber(theArgs, "offset");
      if (anOffset == 0.0) throw KernelFailure(ErrorCode::InvalidArgs, "Wire offset cannot be zero");
      BRepOffsetAPI_MakeOffset aBuilder(TopoDS::Wire(theContext.arena().get(requiredU32(theArgs, "wire"))));
      aBuilder.Perform(anOffset);
      if (!aBuilder.IsDone() || aBuilder.Shape().IsNull()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT wire offset failed");
      return {{"shape", theContext.arena().add(aBuilder.Shape(), aScope)}};
    }

namespace {
enum class BooleanMergeMode { Fuse, Common };

json runBooleanMerge(KernelOperationContext& theContext, const json& theArgs, BooleanMergeMode theMode)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aBase = theContext.arena().get(requiredU32(theArgs, "base"));
      if (!theArgs.contains("tools") || !theArgs.at("tools").is_array() || theArgs.at("tools").empty())
        throw KernelFailure(ErrorCode::InvalidArgs, "Boolean operation requires at least one tool");
      NCollection_List<TopoDS_Shape> anArguments;
      NCollection_List<TopoDS_Shape> aTools;
      anArguments.Append(aBase);
      std::vector<TopoDS_Shape> anInputs{aBase};
      for (const json& aValue : theArgs.at("tools"))
      {
        const TopoDS_Shape& aTool = theContext.arena().get(aValue.get<std::uint32_t>());
        aTools.Append(aTool);
        anInputs.push_back(aTool);
      }
      if (theMode == BooleanMergeMode::Fuse)
      {
        BRepAlgoAPI_Fuse aBuilder;
        aBuilder.SetArguments(anArguments); aBuilder.SetTools(aTools); aBuilder.SetNonDestructive(true);
        aBuilder.SetRunParallel(false); aBuilder.SetToFillHistory(theArgs.value("includeHistory", false)); aBuilder.Build();
        if (!aBuilder.IsDone() || aBuilder.HasErrors()) throw KernelFailure(ErrorCode::BooleanFailed, "OCCT boolean fuse failed");
        json aResult{{"shape", theContext.arena().add(aBuilder.Shape(), aScope)}};
        if (theArgs.value("includeHistory", false)) aResult["history"] = buildHistory(aBuilder, anInputs, aBuilder.Shape());
        return aResult;
      }
      BRepAlgoAPI_Common aBuilder;
      aBuilder.SetArguments(anArguments); aBuilder.SetTools(aTools); aBuilder.SetNonDestructive(true);
      aBuilder.SetRunParallel(false); aBuilder.SetToFillHistory(theArgs.value("includeHistory", false)); aBuilder.Build();
      if (!aBuilder.IsDone() || aBuilder.HasErrors()) throw KernelFailure(ErrorCode::BooleanFailed, "OCCT boolean common failed");
      json aResult{{"shape", theContext.arena().add(aBuilder.Shape(), aScope)}};
      if (theArgs.value("includeHistory", false)) aResult["history"] = buildHistory(aBuilder, anInputs, aBuilder.Shape());
      return aResult;
    }

} // namespace

json h_booleanFuse(KernelOperationContext& theContext, const json& theArgs)
{
  return runBooleanMerge(theContext, theArgs, BooleanMergeMode::Fuse);
}

json h_booleanCommon(KernelOperationContext& theContext, const json& theArgs)
{
  return runBooleanMerge(theContext, theArgs, BooleanMergeMode::Common);
}

json h_booleanCut(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aBase = theContext.arena().get(requiredU32(theArgs, "base"));
      if (!theArgs.contains("tools") || !theArgs.at("tools").is_array()
          || theArgs.at("tools").empty())
      {
        throw KernelFailure(ErrorCode::InvalidArgs, "booleanCut requires at least one tool");
      }
      NCollection_List<TopoDS_Shape> anArguments;
      NCollection_List<TopoDS_Shape> aTools;
      anArguments.Append(aBase);
      std::vector<TopoDS_Shape> anInputs{aBase};
      for (const json& aValue : theArgs.at("tools"))
      {
        if (!aValue.is_number_unsigned() && !aValue.is_number_integer())
        {
          throw KernelFailure(ErrorCode::InvalidArgs, "Tool handles must be integers");
        }
        const TopoDS_Shape& aTool = theContext.arena().get(aValue.get<std::uint32_t>());
        aTools.Append(aTool);
        anInputs.push_back(aTool);
      }
      BRepAlgoAPI_Cut aBuilder;
      aBuilder.SetArguments(anArguments);
      aBuilder.SetTools(aTools);
      aBuilder.SetNonDestructive(true);
      aBuilder.SetRunParallel(false);
      aBuilder.SetToFillHistory(theArgs.value("includeHistory", false));
      aBuilder.Build();
      if (!aBuilder.IsDone() || aBuilder.HasErrors())
      {
        throw KernelFailure(ErrorCode::BooleanFailed, "OCCT boolean cut failed");
      }
      json aResult{{"shape", theContext.arena().add(aBuilder.Shape(), aScope)}};
      if (theArgs.value("includeHistory", false))
      {
        aResult["history"] = buildHistory(aBuilder, anInputs, aBuilder.Shape());
      }
      return aResult;
    }

json h_section(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      BRepAlgoAPI_Section aBuilder(theContext.arena().get(requiredU32(theArgs, "first")),
                                   theContext.arena().get(requiredU32(theArgs, "second")), false);
      aBuilder.SetNonDestructive(true);
      aBuilder.SetRunParallel(false);
      aBuilder.Build();
      if (!aBuilder.IsDone() || aBuilder.Shape().IsNull()) throw KernelFailure(ErrorCode::BooleanFailed, "OCCT section failed");
      return {{"shape", theContext.arena().add(aBuilder.Shape(), aScope)}};
    }

json h_projectHLR(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
      const auto aDirectionValue = requiredDirection(theArgs, "direction");
      const auto anUpValue = requiredDirection(theArgs, "up");
      for (const double aValue : aDirectionValue)
        if (!std::isfinite(aValue)) throw KernelFailure(ErrorCode::InvalidArgs, "HLR direction must be finite");
      for (const double aValue : anUpValue)
        if (!std::isfinite(aValue)) throw KernelFailure(ErrorCode::InvalidArgs, "HLR up direction must be finite");

      const gp_Vec aDirection(aDirectionValue[0], aDirectionValue[1], aDirectionValue[2]);
      const gp_Vec anUp(anUpValue[0], anUpValue[1], anUpValue[2]);
      const gp_Vec aRight = aDirection.Crossed(anUp);
      if (aRight.SquareMagnitude() <= 1.0e-24)
        throw KernelFailure(ErrorCode::InvalidArgs, "HLR direction and up direction must not be parallel");

      const std::string aProjection = theArgs.value("projection", "parallel");
      if (aProjection != "parallel" && aProjection != "perspective")
        throw KernelFailure(ErrorCode::InvalidArgs, "HLR projection must be parallel or perspective");
      double aFocus = 0.0;
      if (aProjection == "perspective")
      {
        aFocus = requiredNumber(theArgs, "focus");
        if (!std::isfinite(aFocus) || aFocus <= 0.0)
          throw KernelFailure(ErrorCode::InvalidArgs, "HLR perspective focus must be positive and finite");
      }

      const gp_Ax2 aViewSystem(
        gp_Pnt(0.0, 0.0, 0.0),
        gp_Dir(-aDirection.X(), -aDirection.Y(), -aDirection.Z()),
        gp_Dir(aRight));
      const HLRAlgo_Projector aProjector = aProjection == "perspective"
        ? HLRAlgo_Projector(aViewSystem, aFocus)
        : HLRAlgo_Projector(aViewSystem);
      occ::handle<HLRBRep_Algo> anAlgorithm = new HLRBRep_Algo();
      anAlgorithm->Add(aShape);
      anAlgorithm->Projector(aProjector);
      anAlgorithm->Update();
      anAlgorithm->Hide();

      HLRBRep_HLRToShape anExtractor(anAlgorithm);
      BRep_Builder aBuilder;
      auto combine = [&](const std::initializer_list<TopoDS_Shape>& theParts) {
        TopoDS_Compound aCompound;
        aBuilder.MakeCompound(aCompound);
        for (const TopoDS_Shape& aPart : theParts)
          if (!aPart.IsNull()) aBuilder.Add(aCompound, aPart);
        return aCompound;
      };
      const TopoDS_Compound aVisible = combine({
        anExtractor.VCompound(),
        anExtractor.Rg1LineVCompound(),
        anExtractor.RgNLineVCompound(),
        anExtractor.OutLineVCompound()});
      const TopoDS_Compound aHidden = combine({
        anExtractor.HCompound(),
        anExtractor.Rg1LineHCompound(),
        anExtractor.RgNLineHCompound(),
        anExtractor.OutLineHCompound()});
      return {{"visible", theContext.arena().add(aVisible, aScope)},
              {"hidden", theContext.arena().add(aHidden, aScope)}};
    }

json h_sectionAnalysis(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aFirst = theContext.arena().get(requiredU32(theArgs, "first"));
      const TopoDS_Shape& aSecond = theContext.arena().get(requiredU32(theArgs, "second"));
      BRepAlgoAPI_Section aBuilder(aFirst, aSecond, false);
      aBuilder.SetNonDestructive(true);
      aBuilder.SetRunParallel(false);
      aBuilder.Approximation(theArgs.value("approximation", false));
      aBuilder.ComputePCurveOn1(theArgs.value("computePCurveOnFirst", false));
      aBuilder.ComputePCurveOn2(theArgs.value("computePCurveOnSecond", false));
      aBuilder.Build();
      if (!aBuilder.IsDone() || aBuilder.HasErrors() || aBuilder.Shape().IsNull())
        throw KernelFailure(ErrorCode::BooleanFailed, "OCCT section analysis failed");

      TopTools_IndexedMapOfShape aFirstFaces;
      TopTools_IndexedMapOfShape aSecondFaces;
      TopTools_IndexedMapOfShape anEdges;
      TopTools_IndexedMapOfShape aVertices;
      TopTools_IndexedMapOfShape anEdgeVertices;
      TopExp::MapShapes(aFirst, TopAbs_FACE, aFirstFaces);
      TopExp::MapShapes(aSecond, TopAbs_FACE, aSecondFaces);
      TopExp::MapShapes(aBuilder.Shape(), TopAbs_EDGE, anEdges);
      TopExp::MapShapes(aBuilder.Shape(), TopAbs_VERTEX, aVertices);
      for (int anIndex = 1; anIndex <= anEdges.Extent(); ++anIndex)
        TopExp::MapShapes(anEdges(anIndex), TopAbs_VERTEX, anEdgeVertices);

      json anEdgeResults = json::array();
      for (int anIndex = 1; anIndex <= anEdges.Extent(); ++anIndex)
      {
        const TopoDS_Shape& anEdge = anEdges(anIndex);
        GProp_GProps aProperties;
        BRepGProp::LinearProperties(anEdge, aProperties);
        json anEdgeResult{{"index", anIndex - 1}, {"length", aProperties.Mass()}};
        TopoDS_Shape anAncestor;
        if (aBuilder.HasAncestorFaceOn1(anEdge, anAncestor))
        {
          const int aFaceIndex = aFirstFaces.FindIndex(anAncestor);
          if (aFaceIndex > 0) anEdgeResult["firstFaceIndex"] = aFaceIndex - 1;
        }
        if (aBuilder.HasAncestorFaceOn2(anEdge, anAncestor))
        {
          const int aFaceIndex = aSecondFaces.FindIndex(anAncestor);
          if (aFaceIndex > 0) anEdgeResult["secondFaceIndex"] = aFaceIndex - 1;
        }
        anEdgeResults.push_back(std::move(anEdgeResult));
      }
      json aStandaloneVertices = json::array();
      for (int anIndex = 1; anIndex <= aVertices.Extent(); ++anIndex)
      {
        if (!anEdgeVertices.Contains(aVertices(anIndex)))
          aStandaloneVertices.push_back(anIndex - 1);
      }
      return {{"shape", theContext.arena().add(aBuilder.Shape(), aScope)},
              {"edges", std::move(anEdgeResults)},
              {"standaloneVertices", std::move(aStandaloneVertices)}};
    }

json h_split(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      if (!theArgs.contains("objects") || !theArgs.at("objects").is_array()
          || theArgs.at("objects").empty())
        throw KernelFailure(ErrorCode::InvalidArgs, "split requires at least one object");
      if (theArgs.contains("tools") && !theArgs.at("tools").is_array())
        throw KernelFailure(ErrorCode::InvalidArgs, "split tools must be an array");
      const double aFuzzyValue = theArgs.value("fuzzyValue", 0.0);
      if (aFuzzyValue < 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs, "split fuzzyValue must be non-negative");
      BOPAlgo_Splitter aBuilder;
      aBuilder.SetNonDestructive(true);
      aBuilder.SetRunParallel(false);
      aBuilder.SetUseOBB(theArgs.value("useOBB", true));
      aBuilder.SetFuzzyValue(aFuzzyValue);
      for (const json& aValue : theArgs.at("objects"))
      {
        if (!aValue.is_number_integer())
          throw KernelFailure(ErrorCode::InvalidArgs, "split object handles must be integers");
        const std::int64_t aHandle = aValue.get<std::int64_t>();
        if (aHandle <= 0 || aHandle > UINT32_MAX)
          throw KernelFailure(ErrorCode::InvalidArgs, "split object handle is outside u32 range");
        aBuilder.AddArgument(theContext.arena().get(static_cast<std::uint32_t>(aHandle)));
      }
      if (theArgs.contains("tools"))
      {
        for (const json& aValue : theArgs.at("tools"))
        {
          if (!aValue.is_number_integer())
            throw KernelFailure(ErrorCode::InvalidArgs, "split tool handles must be integers");
          const std::int64_t aHandle = aValue.get<std::int64_t>();
          if (aHandle <= 0 || aHandle > UINT32_MAX)
            throw KernelFailure(ErrorCode::InvalidArgs, "split tool handle is outside u32 range");
          aBuilder.AddTool(theContext.arena().get(static_cast<std::uint32_t>(aHandle)));
        }
      }
      aBuilder.Perform();
      if (aBuilder.HasErrors() || aBuilder.Shape().IsNull())
        throw KernelFailure(ErrorCode::BooleanFailed, "OCCT splitter failed");
      return {{"shape", theContext.arena().add(aBuilder.Shape(), aScope)}};
    }

json h_glue(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aNewShape = theContext.arena().get(requiredU32(theArgs, "newShape"));
      const TopoDS_Shape& aBaseShape = theContext.arena().get(requiredU32(theArgs, "baseShape"));
      if (!theArgs.contains("faceBindings") || !theArgs.at("faceBindings").is_array()
          || theArgs.at("faceBindings").empty())
        throw KernelFailure(ErrorCode::InvalidArgs, "glue requires at least one face binding");
      if (theArgs.contains("edgeBindings") && !theArgs.at("edgeBindings").is_array())
        throw KernelFailure(ErrorCode::InvalidArgs, "glue edgeBindings must be an array");

      TopTools_IndexedMapOfShape aNewFaces;
      TopTools_IndexedMapOfShape aBaseFaces;
      TopExp::MapShapes(aNewShape, TopAbs_FACE, aNewFaces);
      TopExp::MapShapes(aBaseShape, TopAbs_FACE, aBaseFaces);
      BRepFeat_Gluer aBuilder(aNewShape, aBaseShape);
      for (const json& aBinding : theArgs.at("faceBindings"))
      {
        if (!aBinding.is_object() || !aBinding.contains("newIndex")
            || !aBinding.contains("baseIndex") || !aBinding.at("newIndex").is_number_integer()
            || !aBinding.at("baseIndex").is_number_integer())
          throw KernelFailure(ErrorCode::InvalidArgs, "glue face binding must contain integer newIndex and baseIndex");
        const std::int64_t aNewIndex = aBinding.at("newIndex").get<std::int64_t>();
        const std::int64_t aBaseIndex = aBinding.at("baseIndex").get<std::int64_t>();
        if (aNewIndex < 0 || aNewIndex >= aNewFaces.Extent()
            || aBaseIndex < 0 || aBaseIndex >= aBaseFaces.Extent())
          throw KernelFailure(ErrorCode::InvalidArgs, "glue face binding index is out of range");
        const TopoDS_Face aNewFace = TopoDS::Face(aNewFaces(static_cast<int>(aNewIndex) + 1));
        const TopoDS_Face aBaseFace = TopoDS::Face(aBaseFaces(static_cast<int>(aBaseIndex) + 1));
        aBuilder.Bind(aNewFace, aBaseFace);
        LocOpe_FindEdges aFinder;
        aFinder.Set(aNewFace, aBaseFace);
        for (aFinder.InitIterator(); aFinder.More(); aFinder.Next())
          aBuilder.Bind(aFinder.EdgeFrom(), aFinder.EdgeTo());
      }

      if (theArgs.contains("edgeBindings"))
      {
        TopTools_IndexedMapOfShape aNewEdges;
        TopTools_IndexedMapOfShape aBaseEdges;
        TopExp::MapShapes(aNewShape, TopAbs_EDGE, aNewEdges);
        TopExp::MapShapes(aBaseShape, TopAbs_EDGE, aBaseEdges);
        for (const json& aBinding : theArgs.at("edgeBindings"))
        {
          if (!aBinding.is_object() || !aBinding.contains("newIndex")
              || !aBinding.contains("baseIndex") || !aBinding.at("newIndex").is_number_integer()
              || !aBinding.at("baseIndex").is_number_integer())
            throw KernelFailure(ErrorCode::InvalidArgs, "glue edge binding must contain integer newIndex and baseIndex");
          const std::int64_t aNewIndex = aBinding.at("newIndex").get<std::int64_t>();
          const std::int64_t aBaseIndex = aBinding.at("baseIndex").get<std::int64_t>();
          if (aNewIndex < 0 || aNewIndex >= aNewEdges.Extent()
              || aBaseIndex < 0 || aBaseIndex >= aBaseEdges.Extent())
            throw KernelFailure(ErrorCode::InvalidArgs, "glue edge binding index is out of range");
          aBuilder.Bind(TopoDS::Edge(aNewEdges(static_cast<int>(aNewIndex) + 1)),
                        TopoDS::Edge(aBaseEdges(static_cast<int>(aBaseIndex) + 1)));
        }
      }

      aBuilder.Build();
      if (!aBuilder.IsDone() || aBuilder.Shape().IsNull()
          || !BRepCheck_Analyzer(aBuilder.Shape(), true).IsValid())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT glue operation failed");
      return {{"shape", theContext.arena().add(aBuilder.Shape(), aScope)}};
    }

json h_generalFuse(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      if (!theArgs.contains("shapes") || !theArgs.at("shapes").is_array()
          || theArgs.at("shapes").size() < 2)
        throw KernelFailure(ErrorCode::InvalidArgs, "generalFuse requires at least two shapes");
      const double aFuzzyValue = theArgs.value("fuzzyValue", 0.0);
      if (aFuzzyValue < 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs, "generalFuse fuzzyValue must be non-negative");
      NCollection_List<TopoDS_Shape> aArguments;
      std::vector<TopoDS_Shape> anInputShapes;
      for (const json& aValue : theArgs.at("shapes"))
      {
        if (!aValue.is_number_integer())
          throw KernelFailure(ErrorCode::InvalidArgs, "generalFuse shape handles must be integers");
        const std::int64_t aHandle = aValue.get<std::int64_t>();
        if (aHandle <= 0 || aHandle > UINT32_MAX)
          throw KernelFailure(ErrorCode::InvalidArgs, "generalFuse shape handle is outside u32 range");
        const TopoDS_Shape& anInput = theContext.arena().get(static_cast<std::uint32_t>(aHandle));
        aArguments.Append(anInput);
        anInputShapes.push_back(anInput);
      }

      CellsBuilder aBuilder;
      aBuilder.SetArguments(aArguments);
      aBuilder.SetNonDestructive(true);
      aBuilder.SetRunParallel(false);
      aBuilder.SetUseOBB(theArgs.value("useOBB", true));
      aBuilder.SetFuzzyValue(aFuzzyValue);
      const std::string aGlue = theArgs.value("glue", "off");
      if (aGlue == "off") aBuilder.SetGlue(BOPAlgo_GlueOff);
      else if (aGlue == "shift") aBuilder.SetGlue(BOPAlgo_GlueShift);
      else if (aGlue == "full") aBuilder.SetGlue(BOPAlgo_GlueFull);
      else throw KernelFailure(ErrorCode::InvalidArgs, "generalFuse glue must be off, shift, or full");
      aBuilder.Perform();
      if (aBuilder.HasErrors() || aBuilder.GetAllParts().IsNull())
        throw KernelFailure(ErrorCode::BooleanFailed, "OCCT general fuse failed");

      std::vector<TopoDS_Shape> aCells;
      std::function<void(const TopoDS_Shape&)> collectCells = [&](const TopoDS_Shape& theShape) {
        if (theShape.ShapeType() == TopAbs_COMPOUND)
        {
          for (TopoDS_Iterator anIterator(theShape); anIterator.More(); anIterator.Next())
            collectCells(anIterator.Value());
          return;
        }
        aCells.push_back(theShape);
      };
      collectCells(aBuilder.GetAllParts());
      if (aCells.empty())
        throw KernelFailure(ErrorCode::BooleanFailed, "OCCT general fuse produced no cells");

      json aShapes = json::array();
      json aSources = json::array();
      for (std::size_t aCellIndex = 0; aCellIndex < aCells.size(); ++aCellIndex)
      {
        aShapes.push_back(theContext.arena().add(aCells[aCellIndex], aScope));
        json aCellSources = json::array();
        const NCollection_List<TopoDS_Shape>* anOrigins = aBuilder.origins(aCells[aCellIndex]);
        if (anOrigins != nullptr)
        {
          for (std::size_t anInputIndex = 0; anInputIndex < anInputShapes.size(); ++anInputIndex)
          {
            for (NCollection_List<TopoDS_Shape>::Iterator anIterator(*anOrigins);
                 anIterator.More(); anIterator.Next())
            {
              if (anInputShapes[anInputIndex].IsSame(anIterator.Value()))
              {
                aCellSources.push_back(anInputIndex);
                break;
              }
            }
          }
        }
        aSources.push_back(std::move(aCellSources));
      }
      return {{"shapes", aShapes}, {"sourceIndices", aSources}};
    }

json h_selectGeneralFuseCells(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      if (!theArgs.contains("shapes") || !theArgs.at("shapes").is_array()
          || theArgs.at("shapes").size() < 2)
        throw KernelFailure(ErrorCode::InvalidArgs, "selectGeneralFuseCells requires at least two shapes");
      if (!theArgs.contains("rules") || !theArgs.at("rules").is_array()
          || theArgs.at("rules").empty())
        throw KernelFailure(ErrorCode::InvalidArgs, "selectGeneralFuseCells requires at least one rule");
      const double aFuzzyValue = theArgs.value("fuzzyValue", 0.0);
      if (aFuzzyValue < 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs, "selectGeneralFuseCells fuzzyValue must be non-negative");

      NCollection_List<TopoDS_Shape> aArguments;
      std::vector<TopoDS_Shape> anInputShapes;
      for (const json& aValue : theArgs.at("shapes"))
      {
        if (!aValue.is_number_integer())
          throw KernelFailure(ErrorCode::InvalidArgs, "selectGeneralFuseCells shape handles must be integers");
        const std::int64_t aHandle = aValue.get<std::int64_t>();
        if (aHandle <= 0 || aHandle > UINT32_MAX)
          throw KernelFailure(ErrorCode::InvalidArgs, "selectGeneralFuseCells shape handle is outside u32 range");
        const TopoDS_Shape& anInput = theContext.arena().get(static_cast<std::uint32_t>(aHandle));
        aArguments.Append(anInput);
        anInputShapes.push_back(anInput);
      }

      BOPAlgo_CellsBuilder aBuilder;
      aBuilder.SetArguments(aArguments);
      aBuilder.SetNonDestructive(true);
      aBuilder.SetRunParallel(false);
      aBuilder.SetUseOBB(theArgs.value("useOBB", true));
      aBuilder.SetFuzzyValue(aFuzzyValue);
      const std::string aGlue = theArgs.value("glue", "off");
      if (aGlue == "off") aBuilder.SetGlue(BOPAlgo_GlueOff);
      else if (aGlue == "shift") aBuilder.SetGlue(BOPAlgo_GlueShift);
      else if (aGlue == "full") aBuilder.SetGlue(BOPAlgo_GlueFull);
      else throw KernelFailure(ErrorCode::InvalidArgs, "selectGeneralFuseCells glue must be off, shift, or full");
      aBuilder.Perform();
      if (aBuilder.HasErrors() || aBuilder.GetAllParts().IsNull())
        throw KernelFailure(ErrorCode::BooleanFailed, "OCCT general fuse cell selection failed");

      for (const json& aRule : theArgs.at("rules"))
      {
        if (!aRule.is_object() || !aRule.contains("take") || !aRule.at("take").is_array()
            || aRule.at("take").empty())
          throw KernelFailure(ErrorCode::InvalidArgs, "Each cell selection rule requires a non-empty take array");
        if (aRule.contains("avoid") && !aRule.at("avoid").is_array())
          throw KernelFailure(ErrorCode::InvalidArgs, "Cell selection rule avoid must be an array");

        NCollection_List<TopoDS_Shape> aTake;
        NCollection_List<TopoDS_Shape> anAvoid;
        std::vector<bool> aSeen(anInputShapes.size(), false);
        const auto appendInputs = [&](const json& theIndices,
                                      NCollection_List<TopoDS_Shape>& theTarget) {
          for (const json& anIndexValue : theIndices)
          {
            if (!anIndexValue.is_number_integer())
              throw KernelFailure(ErrorCode::InvalidArgs, "Cell selection indices must be integers");
            const std::int64_t anIndex = anIndexValue.get<std::int64_t>();
            if (anIndex < 0 || anIndex >= static_cast<std::int64_t>(anInputShapes.size()))
              throw KernelFailure(ErrorCode::InvalidArgs, "Cell selection input index is out of range");
            if (aSeen[static_cast<std::size_t>(anIndex)])
              throw KernelFailure(ErrorCode::InvalidArgs, "Cell selection indices must be unique and disjoint");
            aSeen[static_cast<std::size_t>(anIndex)] = true;
            theTarget.Append(anInputShapes[static_cast<std::size_t>(anIndex)]);
          }
        };
        appendInputs(aRule.at("take"), aTake);
        if (aRule.contains("avoid")) appendInputs(aRule.at("avoid"), anAvoid);

        int aMaterial = 0;
        if (aRule.contains("material"))
        {
          if (!aRule.at("material").is_number_integer())
            throw KernelFailure(ErrorCode::InvalidArgs, "Cell selection material must be an integer");
          const std::int64_t aMaterialValue = aRule.at("material").get<std::int64_t>();
          if (aMaterialValue < std::numeric_limits<int>::min()
              || aMaterialValue > std::numeric_limits<int>::max())
            throw KernelFailure(ErrorCode::InvalidArgs, "Cell selection material is outside int range");
          aMaterial = static_cast<int>(aMaterialValue);
        }
        aBuilder.AddToResult(aTake, anAvoid, aMaterial, false);
      }

      if (theArgs.value("removeInternalBoundaries", false))
        aBuilder.RemoveInternalBoundaries();
      if (theArgs.value("makeContainers", false))
        aBuilder.MakeContainers();
      if (aBuilder.HasErrors() || aBuilder.Shape().IsNull())
        throw KernelFailure(ErrorCode::BooleanFailed, "OCCT general fuse cell result construction failed");
      return {{"shape", theContext.arena().add(aBuilder.Shape(), aScope)}};
    }

json h_sew(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      if (!theArgs.contains("shapes") || !theArgs.at("shapes").is_array() || theArgs.at("shapes").empty())
        throw KernelFailure(ErrorCode::InvalidArgs, "sew requires at least one shape");
      const double aTolerance = theArgs.value("tolerance", 1.0e-6);
      if (aTolerance <= 0.0) throw KernelFailure(ErrorCode::InvalidArgs, "Sewing tolerance must be positive");
      BRepBuilderAPI_Sewing aBuilder(aTolerance);
      std::vector<TopoDS_Shape> anInputs;
      for (const json& aShape : theArgs.at("shapes"))
      {
        anInputs.push_back(theContext.arena().get(aShape.get<std::uint32_t>()));
        aBuilder.Add(anInputs.back());
      }
      aBuilder.Perform();
      if (aBuilder.SewedShape().IsNull()) throw KernelFailure(ErrorCode::HealingFailed, "OCCT sewing produced no shape");
      json aResult{{"shape", theContext.arena().add(aBuilder.SewedShape(), aScope)},
                   {"freeEdges", aBuilder.NbFreeEdges()}, {"multipleEdges", aBuilder.NbMultipleEdges()}};
      if (theArgs.value("includeHistory", false))
        aResult["history"] = buildRecordedHistory(aBuilder.GetContext()->History(), anInputs, aBuilder.SewedShape());
      return aResult;
    }

json h_fixShape(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape anInput = theContext.arena().get(requiredU32(theArgs, "shape"));
      BRepBuilderAPI_Copy aCopy(anInput, true, false);
      if (!aCopy.IsDone() || aCopy.Shape().IsNull())
        throw KernelFailure(ErrorCode::HealingFailed, "OCCT failed to copy shape before fixing");
      ShapeFix_Shape aFixer(aCopy.Shape());
      if (theArgs.contains("precision"))
      {
        const double aPrecision = requiredNumber(theArgs, "precision");
        if (aPrecision <= 0.0) throw KernelFailure(ErrorCode::InvalidArgs, "Fix precision must be positive");
        aFixer.SetPrecision(aPrecision);
      }
      aFixer.Perform();
      const TopoDS_Shape aResult = aFixer.Shape();
      if (aResult.IsNull()) throw KernelFailure(ErrorCode::HealingFailed, "OCCT shape fixing produced no shape");
      json aResponse{{"shape", theContext.arena().add(aResult, aScope)}};
      if (theArgs.value("includeHistory", false))
      {
        NCollection_List<TopoDS_Shape> anArguments;
        anArguments.Append(anInput);
        occ::handle<BRepTools_History> aHistory = new BRepTools_History(anArguments, aCopy);
        aHistory->Merge(aFixer.Context()->History());
        aResponse["history"] = buildRecordedHistory(aHistory, {anInput}, aResult);
      }
      return aResponse;
    }

json h_unifySameDomain(KernelOperationContext& theContext, const json& theArgs)
{
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape anInput = theContext.arena().get(requiredU32(theArgs, "shape"));
      ShapeUpgrade_UnifySameDomain aBuilder(anInput,
                                            theArgs.value("unifyEdges", true),
                                            theArgs.value("unifyFaces", true),
                                            theArgs.value("concatBSplines", false));
      aBuilder.SetSafeInputMode(true);
      aBuilder.Build();
      if (aBuilder.Shape().IsNull()) throw KernelFailure(ErrorCode::HealingFailed, "OCCT same-domain unification produced no shape");
      json aResult{{"shape", theContext.arena().add(aBuilder.Shape(), aScope)}};
      if (theArgs.value("includeHistory", false))
        aResult["history"] = buildRecordedHistory(aBuilder.History(), {anInput}, aBuilder.Shape());
      return aResult;
    }













void register_algorithms_operations()
{
  auto& aRegistry = OperationRegistry::instance();
#define OCCT_REGISTER_HANDLER(theOperation) aRegistry.add(#theOperation, &h_##theOperation);
  OCCT_ALGORITHMS_OPERATION_TABLE(OCCT_REGISTER_HANDLER)
#undef OCCT_REGISTER_HANDLER
}

OCCT_DEFINE_SIDE_PLUGIN(algorithms, kAlgorithmsOperationNames, register_algorithms_operations)

} // namespace occt_worker
