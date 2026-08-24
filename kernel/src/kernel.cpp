#include "generated.hpp"

#include <APIHeaderSection_MakeHeader.hxx>
#include <BOPAlgo_Options.hxx>
#include <BOPAlgo_CellsBuilder.hxx>
#include <BOPAlgo_Splitter.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include <BRepAlgoAPI_Defeaturing.hxx>
#include <BRepAlgoAPI_Section.hxx>
#include <BRepBndLib.hxx>
#include <BRepBuilderAPI_GTransform.hxx>
#include <BRepBuilderAPI_Copy.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepBuilderAPI_MakeSolid.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_Sewing.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <BRepClass_FaceClassifier.hxx>
#include <BRepCheck_Analyzer.hxx>
#include <BRepCheck_Status.hxx>
#include <BRepExtrema_DistShapeShape.hxx>
#include <BRepFill.hxx>
#include <BRepFilletAPI_MakeChamfer.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <BRepFeat_MakePrism.hxx>
#include <BRepFeat_MakeRevol.hxx>
#include <BRepFeat_MakeLinearForm.hxx>
#include <BRepFeat_MakeRevolutionForm.hxx>
#include <BRepFeat_Gluer.hxx>
#include <BRepGProp.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <IMeshData_Status.hxx>
#include <Message_ProgressIndicator.hxx>
#include <BRepOffsetAPI_MakeOffsetShape.hxx>
#include <BRepOffsetAPI_MakeOffset.hxx>
#include <BRepOffsetAPI_DraftAngle.hxx>
#include <BRepOffsetAPI_MakeFilling.hxx>
#include <BRepOffsetAPI_MakePipe.hxx>
#include <BRepOffsetAPI_MakePipeShell.hxx>
#include <BRepOffsetAPI_MiddlePath.hxx>
#include <BRepOffsetAPI_MakeThickSolid.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeHalfSpace.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepPrimAPI_MakeTorus.hxx>
#include <BRepPrimAPI_MakeWedge.hxx>
#include <BRepTools.hxx>
#include <BRepTools_History.hxx>
#include <BRepTools_WireExplorer.hxx>
#include <LocOpe_FindEdges.hxx>
#include <BRepLib.hxx>
#include <BRepLib_ToolTriangulatedShape.hxx>
#include <BRepAdaptor_Curve.hxx>
#include <BRepAdaptor_Surface.hxx>
#include <BRepLProp_CLProps.hxx>
#include <BRepLProp_SLProps.hxx>
#include <BRep_Builder.hxx>
#include <BRep_Tool.hxx>
#include <Bnd_Box.hxx>
#include <Bnd_OBB.hxx>
#include <BinXCAFDrivers.hxx>
#include <BSplCLib.hxx>
#include <DESTEP_Parameters.hxx>
#include <Convert_ParameterisationType.hxx>
#include <GC_MakeArcOfCircle.hxx>
#include <GeomConvert.hxx>
#include <GeomConvert_ApproxCurve.hxx>
#include <GeomConvert_ApproxSurface.hxx>
#include <GeomAPI_Interpolate.hxx>
#include <GeomAPI_PointsToBSpline.hxx>
#include <GeomAPI_PointsToBSplineSurface.hxx>
#include <GeomLib.hxx>
#include <GeomAPI_ExtremaCurveCurve.hxx>
#include <GeomAPI_ExtremaCurveSurface.hxx>
#include <GeomAPI_ExtremaSurfaceSurface.hxx>
#include <GeomAPI_IntCS.hxx>
#include <GeomAPI_IntSS.hxx>
#include <GeomAPI_ProjectPointOnCurve.hxx>
#include <GeomAPI_ProjectPointOnSurf.hxx>
#include <Geom_BezierCurve.hxx>
#include <Geom_BezierSurface.hxx>
#include <Geom_BoundedCurve.hxx>
#include <Geom_BoundedSurface.hxx>
#include <Geom_BSplineCurve.hxx>
#include <Geom_BSplineSurface.hxx>
#include <Geom_ConicalSurface.hxx>
#include <Geom_CylindricalSurface.hxx>
#include <Geom_OffsetCurve.hxx>
#include <Geom_OffsetSurface.hxx>
#include <Geom_Plane.hxx>
#include <Geom_RectangularTrimmedSurface.hxx>
#include <Geom_SphericalSurface.hxx>
#include <Geom_SurfaceOfLinearExtrusion.hxx>
#include <Geom_SurfaceOfRevolution.hxx>
#include <Geom_ToroidalSurface.hxx>
#include <Geom_TrimmedCurve.hxx>
#include <Geom2d_Curve.hxx>
#include <Geom2d_Line.hxx>
#include <GProp_GProps.hxx>
#include <GCPnts_TangentialDeflection.hxx>
#include <Poly_Triangulation.hxx>
#include <Poly_Triangle.hxx>
#include <PCDM_ReaderStatus.hxx>
#include <PCDM_ReadWriter.hxx>
#include <PCDM_StoreStatus.hxx>
#include <RWStl.hxx>
#include <IGESControl_Reader.hxx>
#include <IGESControl_Writer.hxx>
#include <IGESCAFControl_Reader.hxx>
#include <IGESCAFControl_Writer.hxx>
#include <Interface_Static.hxx>
#include <IntCurvesFace_ShapeIntersector.hxx>
#include <HLRAlgo_Projector.hxx>
#include <HLRBRep_Algo.hxx>
#include <HLRBRep_HLRToShape.hxx>
#include <STEPControl_Reader.hxx>
#include <STEPControl_Writer.hxx>
#include <STEPCAFControl_Reader.hxx>
#include <STEPCAFControl_Writer.hxx>
#include <Standard_Failure.hxx>
#include <StlAPI_Reader.hxx>
#include <ShapeFix_Shape.hxx>
#include <ShapeFix_Edge.hxx>
#include <ShapeFix_ShapeTolerance.hxx>
#include <ShapeBuild_ReShape.hxx>
#include <ShapeUpgrade_UnifySameDomain.hxx>
#include <ShapeUpgrade_RemoveInternalWires.hxx>
#include <ShapeUpgrade_RemoveLocations.hxx>
#include <ShapeUpgrade_ShapeConvertToBezier.hxx>
#include <ShapeUpgrade_ShapeDivideAngle.hxx>
#include <ShapeUpgrade_ShapeDivideArea.hxx>
#include <ShapeUpgrade_ShapeDivideClosed.hxx>
#include <ShapeUpgrade_ShapeDivideClosedEdges.hxx>
#include <ShapeUpgrade_ShapeDivideContinuity.hxx>
#include <StepData_StepModel.hxx>
#include <TCollection_HAsciiString.hxx>
#include <TCollection_AsciiString.hxx>
#include <TCollection_ExtendedString.hxx>
#include <TDataStd_Name.hxx>
#include <TDataStd_TreeNode.hxx>
#include <TDF_ChildIterator.hxx>
#include <TDocStd_Application.hxx>
#include <TDocStd_Document.hxx>
#include <TDocStd_Owner.hxx>
#include <TNaming_NamedShape.hxx>
#include <XCAFDoc_Area.hxx>
#include <XCAFDoc_Centroid.hxx>
#include <XCAFDoc_ClippingPlaneTool.hxx>
#include <XCAFDoc_Dimension.hxx>
#include <XCAFDoc_Datum.hxx>
#include <XCAFDoc_DimTolTool.hxx>
#include <XCAFDoc_GeomTolerance.hxx>
#include <XCAFDoc_ColorTool.hxx>
#include <XCAFDoc_DocumentTool.hxx>
#include <XCAFDoc_LayerTool.hxx>
#include <XCAFDoc_MaterialTool.hxx>
#include <XCAFDoc_ShapeTool.hxx>
#include <XCAFDoc_VisMaterial.hxx>
#include <XCAFDoc_VisMaterialTool.hxx>
#include <XCAFDoc_View.hxx>
#include <XCAFDoc_ViewTool.hxx>
#include <XCAFView_Object.hxx>
#include <XCAFView_ProjectionType.hxx>
#include <XCAFDoc_GraphNode.hxx>
#include <XCAFDimTolObjects_DimensionObject.hxx>
#include <XCAFDimTolObjects_DimensionType.hxx>
#include <XCAFDimTolObjects_DatumObject.hxx>
#include <XCAFDimTolObjects_GeomToleranceObject.hxx>
#include <XCAFDimTolObjects_GeomToleranceType.hxx>
#include <XCAFDimTolObjects_GeomToleranceTypeValue.hxx>
#include <XCAFDoc_Volume.hxx>
#include <XCAFDoc.hxx>
#include <XmlXCAFDrivers.hxx>
#include <XmlXCAFDrivers_DocumentRetrievalDriver.hxx>
#include <XSControl_WorkSession.hxx>
#include <XmlMDF_ADriver.hxx>
#include <XmlMDF_ADriverTable.hxx>
#include <XmlMDF.hxx>
#include <Quantity_ColorRGBA.hxx>
#include <Storage_HeaderData.hxx>
#include <TDocStd_FormatVersion.hxx>
#include <TopAbs_Orientation.hxx>
#include <TopExp.hxx>
#include <TopLoc_Location.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopTools_ListOfShape.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Iterator.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Shape.hxx>
#include <TopoDS_Shell.hxx>
#include <UnitsMethods_LengthUnit.hxx>
#include <UnitsMethods.hxx>
#include <VrmlAPI_CafReader.hxx>
#include <VrmlAPI_Writer.hxx>
#include <VrmlData_Group.hxx>
#include <VrmlData_IndexedFaceSet.hxx>
#include <VrmlData_Scene.hxx>
#include <VrmlData_ShapeNode.hxx>
#include <RWMesh_CoordinateSystem.hxx>
#include <emscripten/emscripten.h>
#include <gp_Ax2.hxx>
#include <gp_Ax3.hxx>
#include <gp_Ax1.hxx>
#include <gp_Circ.hxx>
#include <gp_Dir.hxx>
#include <gp_Elips.hxx>
#include <gp_Hypr.hxx>
#include <gp_Parab.hxx>
#include <gp_GTrsf.hxx>
#include <gp_Lin.hxx>
#include <gp_Mat.hxx>
#include <gp_Pnt.hxx>
#include <gp_Pnt2d.hxx>
#include <gp_Pln.hxx>
#include <gp_Cylinder.hxx>
#include <gp_Cone.hxx>
#include <gp_Sphere.hxx>
#include <gp_Torus.hxx>
#include <gp_Dir2d.hxx>
#include <gp_Trsf.hxx>
#include <gp_Vec.hxx>
#include <gp_XYZ.hxx>
#include <TColgp_Array1OfPnt.hxx>
#include <TColgp_Array2OfPnt.hxx>
#include <TColgp_HArray1OfPnt.hxx>
#include <TColStd_Array1OfInteger.hxx>
#include <TColStd_Array1OfReal.hxx>
#include <TColStd_Array2OfReal.hxx>
#include <TopExp_Explorer.hxx>
#include <TopAbs.hxx>
#include <nlohmann/json.hpp>
#include <NCollection_List.hxx>
#include <NCollection_Array1.hxx>
#include <NCollection_Array2.hxx>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iomanip>
#include <initializer_list>
#include <limits>
#include <new>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

extern "C"
{
__attribute__((import_module("env"), import_name("occt_worker_cancelled")))
int occt_worker_cancelled();

__attribute__((import_module("env"), import_name("occt_worker_progress")))
void occt_worker_progress(double theFraction);
}

namespace occt_worker {
using json = nlohmann::json;

class HostProgressIndicator final : public Message_ProgressIndicator
{
protected:
  bool UserBreak() override
  {
    return occt_worker_cancelled() != 0;
  }

  void Show(const Message_ProgressScope&, const bool) override
  {
    occt_worker_progress(GetPosition());
  }
};

class KernelFailure : public std::runtime_error
{
public:
  KernelFailure(ErrorCode theCode, std::string theMessage)
      : std::runtime_error(std::move(theMessage)), code(theCode)
  {
  }

  ErrorCode code;
};

class CellsBuilder : public BOPAlgo_CellsBuilder
{
public:
  const NCollection_List<TopoDS_Shape>* origins(const TopoDS_Shape& theCell) const
  {
    return myIndex.Seek(theCell);
  }
};

const char* errorName(const ErrorCode theCode)
{
  switch (theCode)
  {
    case ErrorCode::ConstructionFailed: return "ConstructionFailed";
    case ErrorCode::BooleanFailed: return "BooleanFailed";
    case ErrorCode::FilletFailed: return "FilletFailed";
    case ErrorCode::TessellationFailed: return "TessellationFailed";
    case ErrorCode::ImportExportFailed: return "ImportExportFailed";
    case ErrorCode::HealingFailed: return "HealingFailed";
    case ErrorCode::Cancelled: return "Cancelled";
    case ErrorCode::InvalidHandle: return "InvalidHandle";
    case ErrorCode::InvalidArgs: return "InvalidArgs";
    case ErrorCode::OutOfMemory: return "OutOfMemory";
    case ErrorCode::KernelError: return "KernelError";
    case ErrorCode::ProtocolError: return "ProtocolError";
  }
  return "KernelError";
}

const char* exchangeFormatForOperation(const std::string_view theOperation)
{
  if (theOperation.find("XCAF") != std::string_view::npos) return "xcaf";
  if (theOperation.find("STEP") != std::string_view::npos) return "step";
  if (theOperation.find("IGES") != std::string_view::npos) return "iges";
  if (theOperation.find("STL") != std::string_view::npos) return "stl";
  if (theOperation.find("OBJ") != std::string_view::npos) return "obj";
  if (theOperation.find("PLY") != std::string_view::npos) return "ply";
  if (theOperation.find("GLTF") != std::string_view::npos) return "gltf";
  if (theOperation.find("VRML") != std::string_view::npos) return "vrml";
  if (theOperation.find("BREP") != std::string_view::npos) return "brep";
  return nullptr;
}

class ShapeArena
{
public:
  std::uint32_t beginScope()
  {
    const std::uint32_t anId = myNextScope++;
    myScopes.emplace(anId, std::unordered_set<std::uint32_t>{});
    return anId;
  }

  std::uint32_t add(const TopoDS_Shape& theShape, const std::uint32_t theScope)
  {
    const auto aScope = myScopes.find(theScope);
    if (aScope == myScopes.end())
    {
      throw KernelFailure(ErrorCode::InvalidArgs, "Unknown scopeId");
    }

    std::uint32_t anIndex;
    if (myFree.empty())
    {
      if (mySlots.size() >= kIndexMask)
      {
        throw KernelFailure(ErrorCode::OutOfMemory, "Shape handle table is full");
      }
      anIndex = static_cast<std::uint32_t>(mySlots.size());
      mySlots.emplace_back();
    }
    else
    {
      anIndex = myFree.back();
      myFree.pop_back();
    }

    Slot& aSlot = mySlots[anIndex];
    aSlot.shape = theShape;
    aSlot.scope = theScope;
    aSlot.live = true;
    aSlot.creationSequence = ++myCreationSequence;
    const std::uint32_t aHandle = (aSlot.generation << kIndexBits) | (anIndex + 1);
    aScope->second.insert(aHandle);
    ++myLiveCount;
    return aHandle;
  }

  const TopoDS_Shape& get(const std::uint32_t theHandle) const
  {
    const std::uint32_t anEncodedIndex = theHandle & kIndexMask;
    const std::uint32_t aGeneration = theHandle >> kIndexBits;
    if (anEncodedIndex == 0 || anEncodedIndex > mySlots.size())
    {
      throw KernelFailure(ErrorCode::InvalidHandle, "Shape handle does not exist");
    }
    const Slot& aSlot = mySlots[anEncodedIndex - 1];
    if (!aSlot.live || aSlot.generation != aGeneration)
    {
      throw KernelFailure(ErrorCode::InvalidHandle, "Shape handle is stale");
    }
    return aSlot.shape;
  }

  void release(const std::uint32_t theHandle)
  {
    get(theHandle);
    const std::uint32_t anIndex = (theHandle & kIndexMask) - 1;
    Slot& aSlot = mySlots[anIndex];
    const auto aScope = myScopes.find(aSlot.scope);
    if (aScope != myScopes.end())
    {
      aScope->second.erase(theHandle);
    }
    aSlot.shape.Nullify();
    aSlot.live = false;
    if (aSlot.generation == kGenerationMask)
    {
      // Retire exhausted slots instead of wrapping the generation and making
      // an old u32 handle valid again. A retired slot is never put on myFree.
      aSlot.retired = true;
    }
    else
    {
      ++aSlot.generation;
      myFree.push_back(anIndex);
    }
    --myLiveCount;
  }

  void endScope(const std::uint32_t theScope)
  {
    const auto aScope = myScopes.find(theScope);
    if (aScope == myScopes.end())
    {
      throw KernelFailure(ErrorCode::InvalidArgs, "Unknown scopeId");
    }
    const std::vector<std::uint32_t> aHandles(aScope->second.begin(), aScope->second.end());
    for (const std::uint32_t aHandle : aHandles)
    {
      release(aHandle);
    }
    myScopes.erase(theScope);
  }

  void clear()
  {
    for (Slot& aSlot : mySlots)
    {
      if (aSlot.live)
      {
        aSlot.shape.Nullify();
        aSlot.live = false;
        if (aSlot.generation == kGenerationMask)
        {
          aSlot.retired = true;
        }
        else
        {
          ++aSlot.generation;
        }
      }
    }
    myFree.clear();
    for (std::uint32_t anIndex = 0; anIndex < mySlots.size(); ++anIndex)
    {
      if (!mySlots[anIndex].retired)
      {
        myFree.push_back(anIndex);
      }
    }
    myScopes.clear();
    myLiveCount = 0;
  }

  std::size_t liveCount() const { return myLiveCount; }

  std::uint64_t checkpoint() const { return myCreationSequence; }

  void rollback(const std::uint64_t theCheckpoint)
  {
    for (std::uint32_t anIndex = 0; anIndex < mySlots.size(); ++anIndex)
    {
      const Slot& aSlot = mySlots[anIndex];
      if (aSlot.live && aSlot.creationSequence > theCheckpoint)
      {
        release((aSlot.generation << kIndexBits) | (anIndex + 1));
      }
    }
  }

private:
  static constexpr std::uint32_t kIndexBits = 20;
  static constexpr std::uint32_t kIndexMask = (1u << kIndexBits) - 1;
  static constexpr std::uint32_t kGenerationMask = (1u << (32 - kIndexBits)) - 1;

  struct Slot
  {
    TopoDS_Shape shape;
    std::uint32_t generation = 1;
    std::uint32_t scope = 0;
    bool live = false;
    bool retired = false;
    std::uint64_t creationSequence = 0;
  };

  std::vector<Slot> mySlots;
  std::vector<std::uint32_t> myFree;
  std::unordered_map<std::uint32_t, std::unordered_set<std::uint32_t>> myScopes;
  std::uint32_t myNextScope = 1;
  std::size_t myLiveCount = 0;
  std::uint64_t myCreationSequence = 0;
};

class BufferStore
{
public:
  std::uint32_t create(const std::size_t theSize)
  {
    const std::uint32_t anId = myNextId++;
    myBuffers.emplace(anId, Entry{std::vector<std::uint8_t>(theSize), ++myCreationSequence});
    myLiveBytes += theSize;
    return anId;
  }

  template <typename T>
  std::uint32_t copy(const std::vector<T>& theValues)
  {
    const std::size_t aSize = theValues.size() * sizeof(T);
    const std::uint32_t anId = create(aSize);
    if (aSize != 0)
    {
      std::memcpy(myBuffers.at(anId).bytes.data(), theValues.data(), aSize);
    }
    return anId;
  }

  std::vector<std::uint8_t>& get(const std::uint32_t theId)
  {
    const auto aBuffer = myBuffers.find(theId);
    if (aBuffer == myBuffers.end())
    {
      throw KernelFailure(ErrorCode::InvalidArgs, "Unknown bufferId");
    }
    return aBuffer->second.bytes;
  }

  void release(const std::uint32_t theId)
  {
    const auto aBuffer = myBuffers.find(theId);
    if (aBuffer == myBuffers.end())
    {
      throw KernelFailure(ErrorCode::InvalidArgs, "Unknown bufferId");
    }
    myLiveBytes -= aBuffer->second.bytes.size();
    myBuffers.erase(aBuffer);
  }

  void clear()
  {
    myBuffers.clear();
    myLiveBytes = 0;
  }

  std::size_t liveBytes() const { return myLiveBytes; }

  std::uint64_t checkpoint() const { return myCreationSequence; }

  void rollback(const std::uint64_t theCheckpoint)
  {
    for (auto anEntry = myBuffers.begin(); anEntry != myBuffers.end();)
    {
      if (anEntry->second.creationSequence <= theCheckpoint)
      {
        ++anEntry;
        continue;
      }
      myLiveBytes -= anEntry->second.bytes.size();
      anEntry = myBuffers.erase(anEntry);
    }
  }

private:
  struct Entry
  {
    std::vector<std::uint8_t> bytes;
    std::uint64_t creationSequence;
  };

  std::unordered_map<std::uint32_t, Entry> myBuffers;
  std::uint32_t myNextId = 1;
  std::size_t myLiveBytes = 0;
  std::uint64_t myCreationSequence = 0;
};

class IGESUnitGuard
{
public:
  IGESUnitGuard(const double theUnit, const UnitsMethods_LengthUnit theUnitEnum)
      : myPrevious(UnitsMethods::GetCasCadeLengthUnit()),
        myPreviousStatic(Interface_Static::IVal("xstep.cascade.unit"))
  {
    Interface_Static::SetIVal("xstep.cascade.unit", static_cast<int>(theUnitEnum));
    UnitsMethods::SetCasCadeLengthUnit(theUnit);
  }

  ~IGESUnitGuard()
  {
    Interface_Static::SetIVal("xstep.cascade.unit", myPreviousStatic);
    UnitsMethods::SetCasCadeLengthUnit(myPrevious);
  }

  IGESUnitGuard(const IGESUnitGuard&) = delete;
  IGESUnitGuard& operator=(const IGESUnitGuard&) = delete;

private:
  double myPrevious;
  int myPreviousStatic;
};

class XmlXCAFStreamReader : public XmlXCAFDrivers_DocumentRetrievalDriver
{
public:
  void read(Standard_IStream&                        theStream,
            const occ::handle<TDocStd_Document>&    theDocument,
            const occ::handle<TDocStd_Application>& theApplication)
  {
    try
    {
      myFileName = "memory.xml";
      Read(theStream,
           occ::handle<Storage_Data>(),
           theDocument,
           theApplication,
           occ::handle<PCDM_ReaderFilter>(),
           Message_ProgressRange());
    }
    catch (const Standard_Failure& theFailure)
    {
      throw Standard_Failure((std::string("XCAF XML stream parse failed: ")
                              + theFailure.what()).c_str());
    }
  }
};

class Kernel
{
public:
  Kernel()
  {
    BOPAlgo_Options::SetParallelMode(false);
  }

  json dispatch(const std::string& theOp, const json& theArgs)
  {
    const std::uint64_t aShapeCheckpoint = myArena.checkpoint();
    const std::uint64_t aBufferCheckpoint = myBuffers.checkpoint();
    const auto rollback = [&]() {
      myArena.rollback(aShapeCheckpoint);
      myBuffers.rollback(aBufferCheckpoint);
    };
    try
    {
      return dispatchOperation(theOp, theArgs);
    }
    catch (const KernelFailure&)
    {
      rollback();
      throw;
    }
    catch (const json::exception& aFailure)
    {
      rollback();
      throw KernelFailure(ErrorCode::InvalidArgs, aFailure.what());
    }
    catch (const Standard_Failure& aFailure)
    {
      rollback();
      throw KernelFailure(operationFailureCode(theOp), aFailure.what());
    }
    catch (const std::bad_alloc&)
    {
      rollback();
      throw;
    }
    catch (const std::exception& aFailure)
    {
      rollback();
      throw KernelFailure(operationFailureCode(theOp), aFailure.what());
    }
    catch (...)
    {
      rollback();
      throw;
    }
  }

  json dispatchOperation(const std::string& theOp, const json& theArgs)
  {
    if (theOp == "capabilities")
    {
      json anOperations = json::array();
      for (const char* anOperation : kOperationNames) anOperations.push_back(anOperation);
      json aHistorySupport = json::object();
      for (const HistorySupportEntry& anEntry : kHistorySupport)
        aHistorySupport[anEntry.operation] = anEntry.support;
      return {{"protocolVersion", kProtocolVersion},
              {"kernelVersion", kProtocolVersion},
              {"occtVersion", "8.0.1"},
              {"ops", anOperations},
              {"historySupport", aHistorySupport},
              {"buildFlags", {{"threads", false}, {"simd", false}, {"wasmExceptions", true}}}};
    }
    if (theArgs.value("includeHistory", false))
    {
      const auto aSupport = std::find_if(kHistorySupport.begin(), kHistorySupport.end(),
        [&](const HistorySupportEntry& theEntry) { return theOp == theEntry.operation; });
      if (aSupport == kHistorySupport.end() || std::string_view(aSupport->support) == "unsupported")
        throw KernelFailure(ErrorCode::InvalidArgs, "History is unsupported for operation: " + theOp);
    }
    if (theOp == "beginScope")
    {
      return {{"scopeId", myArena.beginScope()}};
    }
    if (theOp == "endScope")
    {
      myArena.endScope(requiredU32(theArgs, "scopeId"));
      return json::object();
    }
    if (theOp == "release")
    {
      myArena.release(requiredU32(theArgs, "shape"));
      return json::object();
    }
    if (theOp == "releaseAll")
    {
      myArena.clear();
      myBuffers.clear();
      return json::object();
    }
    if (theOp == "stats")
    {
      return {{"liveShapeHandles", myArena.liveCount()},
              {"liveBufferBytes", myBuffers.liveBytes()},
              {"wasmMemorySize", static_cast<std::uint64_t>(__builtin_wasm_memory_size(0)) * 65536u}};
    }
    if (theOp == "createBuffer")
    {
      const std::uint32_t aLength = requiredU32(theArgs, "byteLength");
      return bufferDescriptor(myBuffers.create(aLength), aLength, "u8");
    }
    if (theOp == "freeBuffer")
    {
      myBuffers.release(requiredU32(theArgs, "bufferId"));
      return json::object();
    }
    if (theOp == "makeBox")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const auto aSize = requiredVec3(theArgs, "size");
      const auto anOrigin = optionalVec3(theArgs, "origin", {0.0, 0.0, 0.0});
      if (aSize[0] <= 0.0 || aSize[1] <= 0.0 || aSize[2] <= 0.0)
      {
        throw KernelFailure(ErrorCode::InvalidArgs, "Box dimensions must be positive");
      }
      BRepPrimAPI_MakeBox aBuilder(gp_Pnt(anOrigin[0], anOrigin[1], anOrigin[2]),
                                  aSize[0], aSize[1], aSize[2]);
      aBuilder.Build();
      if (!aBuilder.IsDone())
      {
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct box");
      }
      return {{"shape", myArena.add(aBuilder.Shape(), aScope)}};
    }
    if (theOp == "makeCompound" || theOp == "makeShell" || theOp == "makeCompSolid")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const char* aKey = theOp == "makeCompound" ? "shapes" : (theOp == "makeShell" ? "faces" : "solids");
      if (!theArgs.contains(aKey) || !theArgs.at(aKey).is_array() || theArgs.at(aKey).empty())
        throw KernelFailure(ErrorCode::InvalidArgs, std::string(aKey) + " must contain at least one shape");
      BRep_Builder aBuilder;
      TopoDS_Shape aResult;
      if (theOp == "makeCompound")
      {
        TopoDS_Compound aCompound;
        aBuilder.MakeCompound(aCompound);
        aResult = aCompound;
      }
      else if (theOp == "makeShell")
      {
        TopoDS_Shell aShell;
        aBuilder.MakeShell(aShell);
        aResult = aShell;
      }
      else
      {
        TopoDS_CompSolid aCompSolid;
        aBuilder.MakeCompSolid(aCompSolid);
        aResult = aCompSolid;
      }
      for (const json& aHandle : theArgs.at(aKey))
      {
        if ((!aHandle.is_number_integer() && !aHandle.is_number_unsigned())
            || aHandle.get<std::int64_t>() < 0 || aHandle.get<std::uint64_t>() > UINT32_MAX)
          throw KernelFailure(ErrorCode::InvalidArgs, std::string(aKey) + " must contain shape handles");
        const TopoDS_Shape& aShape = myArena.get(aHandle.get<std::uint32_t>());
        if (theOp == "makeShell" && aShape.ShapeType() != TopAbs_FACE)
          throw KernelFailure(ErrorCode::InvalidArgs, "makeShell accepts faces only");
        if (theOp == "makeCompSolid" && aShape.ShapeType() != TopAbs_SOLID)
          throw KernelFailure(ErrorCode::InvalidArgs, "makeCompSolid accepts solids only");
        aBuilder.Add(aResult, aShape);
      }
      return {{"shape", myArena.add(aResult, aScope)}};
    }
    if (theOp == "makeCylinder")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const double aRadius = requiredNumber(theArgs, "radius");
      const double aHeight = requiredNumber(theArgs, "height");
      const auto anOrigin = optionalVec3(theArgs, "origin", {0.0, 0.0, 0.0});
      const auto aDirection = optionalDirection(theArgs, "direction", {0.0, 0.0, 1.0});
      if (aRadius <= 0.0 || aHeight <= 0.0)
      {
        throw KernelFailure(ErrorCode::InvalidArgs, "Cylinder radius and height must be positive");
      }
      BRepPrimAPI_MakeCylinder aBuilder(
        gp_Ax2(gp_Pnt(anOrigin[0], anOrigin[1], anOrigin[2]),
               gp_Dir(aDirection[0], aDirection[1], aDirection[2])),
        aRadius,
        aHeight);
      aBuilder.Build();
      if (!aBuilder.IsDone())
      {
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct cylinder");
      }
      return {{"shape", myArena.add(aBuilder.Shape(), aScope)}};
    }
    if (theOp == "makeSphere" || theOp == "makeCone" || theOp == "makeTorus")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const auto anOrigin = optionalVec3(theArgs, "origin", {0.0, 0.0, 0.0});
      const auto aDirection = optionalDirection(theArgs, "direction", {0.0, 0.0, 1.0});
      const gp_Ax2 anAxis(gp_Pnt(anOrigin[0], anOrigin[1], anOrigin[2]),
                          gp_Dir(aDirection[0], aDirection[1], aDirection[2]));
      if (theOp == "makeSphere")
      {
        const double aRadius = requiredNumber(theArgs, "radius");
        if (aRadius <= 0.0) throw KernelFailure(ErrorCode::InvalidArgs, "Sphere radius must be positive");
        BRepPrimAPI_MakeSphere aBuilder(anAxis, aRadius);
        aBuilder.Build();
        if (!aBuilder.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct sphere");
        return {{"shape", myArena.add(aBuilder.Shape(), aScope)}};
      }
      if (theOp == "makeCone")
      {
        const double aRadius1 = requiredNumber(theArgs, "radius1");
        const double aRadius2 = requiredNumber(theArgs, "radius2");
        const double aHeight = requiredNumber(theArgs, "height");
        if (aRadius1 < 0.0 || aRadius2 < 0.0 || aHeight <= 0.0 || (aRadius1 == 0.0 && aRadius2 == 0.0)
            || aRadius1 == aRadius2)
          throw KernelFailure(ErrorCode::InvalidArgs, "Cone radii must be non-negative, distinct, and height positive");
        BRepPrimAPI_MakeCone aBuilder(anAxis, aRadius1, aRadius2, aHeight);
        aBuilder.Build();
        if (!aBuilder.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct cone");
        return {{"shape", myArena.add(aBuilder.Shape(), aScope)}};
      }
      const double aMajor = requiredNumber(theArgs, "majorRadius");
      const double aMinor = requiredNumber(theArgs, "minorRadius");
      if (aMajor <= 0.0 || aMinor <= 0.0 || aMajor <= aMinor)
        throw KernelFailure(ErrorCode::InvalidArgs, "Torus radii must be positive with majorRadius > minorRadius");
      BRepPrimAPI_MakeTorus aBuilder(anAxis, aMajor, aMinor);
      aBuilder.Build();
      if (!aBuilder.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct torus");
      return {{"shape", myArena.add(aBuilder.Shape(), aScope)}};
    }
    if (theOp == "makeWedge")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const auto aSize = requiredVec3(theArgs, "size");
      const double aLtx = requiredNumber(theArgs, "ltx");
      const auto anOrigin = optionalVec3(theArgs, "origin", {0.0, 0.0, 0.0});
      const auto aDirection = optionalDirection(theArgs, "direction", {0.0, 0.0, 1.0});
      if (aSize[0] <= 0.0 || aSize[1] <= 0.0 || aSize[2] <= 0.0 || aLtx < 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs, "Wedge dimensions must be positive and ltx non-negative");
      BRepPrimAPI_MakeWedge aBuilder(
        gp_Ax2(gp_Pnt(anOrigin[0], anOrigin[1], anOrigin[2]),
               gp_Dir(aDirection[0], aDirection[1], aDirection[2])),
        aSize[0], aSize[1], aSize[2], aLtx);
      aBuilder.Build();
      if (!aBuilder.IsDone())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct wedge");
      return {{"shape", myArena.add(aBuilder.Shape(), aScope)}};
    }
    if (theOp == "makeHalfSpace")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aBoundary = myArena.get(requiredU32(theArgs, "face"));
      if (aBoundary.ShapeType() != TopAbs_FACE)
        throw KernelFailure(ErrorCode::InvalidArgs, "Half-space boundary must be a face");
      const auto aReference = requiredVec3(theArgs, "referencePoint");
      BRepPrimAPI_MakeHalfSpace aBuilder(
        TopoDS::Face(aBoundary), gp_Pnt(aReference[0], aReference[1], aReference[2]));
      if (!aBuilder.IsDone() || aBuilder.Shape().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct half-space");
      return {{"shape", myArena.add(aBuilder.Shape(), aScope)}};
    }
    if (theOp == "makeVertex")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const auto aPoint = requiredVec3(theArgs, "point");
      BRepBuilderAPI_MakeVertex aBuilder(gp_Pnt(aPoint[0], aPoint[1], aPoint[2]));
      if (!aBuilder.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct vertex");
      return {{"shape", myArena.add(aBuilder.Shape(), aScope)}};
    }
    if (theOp == "makeEdgeLine" || theOp == "makeEdgeArc")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      if (theOp == "makeEdgeArc" && theArgs.contains("center"))
      {
        const auto aCenter = requiredVec3(theArgs, "center");
        const auto aNormal = requiredDirection(theArgs, "normal");
        const double aRadius = requiredNumber(theArgs, "radius");
        const double aStartParameter = requiredNumber(theArgs, "startAngle");
        const double anEndParameter = requiredNumber(theArgs, "endAngle");
        if (aRadius <= 0.0 || aStartParameter == anEndParameter)
          throw KernelFailure(ErrorCode::InvalidArgs, "Arc radius must be positive and angles must be distinct");
        gp_Ax2 anAxis(gp_Pnt(aCenter[0], aCenter[1], aCenter[2]),
                      gp_Dir(aNormal[0], aNormal[1], aNormal[2]));
        if (theArgs.contains("xDirection"))
        {
          const auto anXDirection = requiredDirection(theArgs, "xDirection");
          if (gp_Dir(aNormal[0], aNormal[1], aNormal[2]).IsParallel(
                gp_Dir(anXDirection[0], anXDirection[1], anXDirection[2]), 1.0e-12))
            throw KernelFailure(ErrorCode::InvalidArgs, "Arc normal and xDirection must not be parallel");
          anAxis = gp_Ax2(gp_Pnt(aCenter[0], aCenter[1], aCenter[2]),
                          gp_Dir(aNormal[0], aNormal[1], aNormal[2]),
                          gp_Dir(anXDirection[0], anXDirection[1], anXDirection[2]));
        }
        BRepBuilderAPI_MakeEdge aBuilder(gp_Circ(anAxis, aRadius), aStartParameter, anEndParameter);
        if (!aBuilder.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct parameterized arc edge");
        return {{"shape", myArena.add(aBuilder.Edge(), aScope)}};
      }
      const auto aStart = requiredVec3(theArgs, "start");
      const auto anEnd = requiredVec3(theArgs, "end");
      const gp_Pnt aStartPoint(aStart[0], aStart[1], aStart[2]);
      const gp_Pnt anEndPoint(anEnd[0], anEnd[1], anEnd[2]);
      if (aStartPoint.Distance(anEndPoint) <= 1.0e-12)
        throw KernelFailure(ErrorCode::InvalidArgs, "Edge endpoints must be distinct");
      if (theOp == "makeEdgeLine")
      {
        BRepBuilderAPI_MakeEdge aBuilder(aStartPoint, anEndPoint);
        if (!aBuilder.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct line edge");
        return {{"shape", myArena.add(aBuilder.Edge(), aScope)}};
      }
      const auto aPoint = requiredVec3(theArgs, "point");
      GC_MakeArcOfCircle anArc(aStartPoint, gp_Pnt(aPoint[0], aPoint[1], aPoint[2]), anEndPoint);
      if (!anArc.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct arc");
      BRepBuilderAPI_MakeEdge aBuilder(anArc.Value());
      if (!aBuilder.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct arc edge");
      return {{"shape", myArena.add(aBuilder.Edge(), aScope)}};
    }
    if (theOp == "makeEdgeCircle" || theOp == "makeEdgeEllipse")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const auto aCenter = optionalVec3(theArgs, "center", {0.0, 0.0, 0.0});
      const auto aNormal = optionalDirection(theArgs, "normal", {0.0, 0.0, 1.0});
      const gp_Ax2 anAxis(gp_Pnt(aCenter[0], aCenter[1], aCenter[2]), gp_Dir(aNormal[0], aNormal[1], aNormal[2]));
      if (theOp == "makeEdgeCircle")
      {
        const double aRadius = requiredNumber(theArgs, "radius");
        if (aRadius <= 0.0) throw KernelFailure(ErrorCode::InvalidArgs, "Circle radius must be positive");
        BRepBuilderAPI_MakeEdge aBuilder(gp_Circ(anAxis, aRadius));
        if (!aBuilder.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct circle edge");
        return {{"shape", myArena.add(aBuilder.Edge(), aScope)}};
      }
      const double aMajor = requiredNumber(theArgs, "majorRadius");
      const double aMinor = requiredNumber(theArgs, "minorRadius");
      if (aMajor <= 0.0 || aMinor <= 0.0 || aMajor < aMinor)
        throw KernelFailure(ErrorCode::InvalidArgs, "Ellipse radii must be positive with majorRadius >= minorRadius");
      BRepBuilderAPI_MakeEdge aBuilder(gp_Elips(anAxis, aMajor, aMinor));
      if (!aBuilder.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct ellipse edge");
      return {{"shape", myArena.add(aBuilder.Edge(), aScope)}};
    }
    if (theOp == "makeEdgeHyperbola" || theOp == "makeEdgeParabola")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const auto aCenter = optionalVec3(theArgs, "center", {0.0, 0.0, 0.0});
      const auto aNormal = optionalDirection(theArgs, "normal", {0.0, 0.0, 1.0});
      gp_Ax2 anAxis(gp_Pnt(aCenter[0], aCenter[1], aCenter[2]),
                    gp_Dir(aNormal[0], aNormal[1], aNormal[2]));
      if (theArgs.contains("xDirection"))
      {
        const auto anXDirection = requiredDirection(theArgs, "xDirection");
        if (gp_Dir(aNormal[0], aNormal[1], aNormal[2]).IsParallel(
              gp_Dir(anXDirection[0], anXDirection[1], anXDirection[2]), 1.0e-12))
          throw KernelFailure(ErrorCode::InvalidArgs,
                              "Conic normal and xDirection must not be parallel");
        anAxis = gp_Ax2(gp_Pnt(aCenter[0], aCenter[1], aCenter[2]),
                        gp_Dir(aNormal[0], aNormal[1], aNormal[2]),
                        gp_Dir(anXDirection[0], anXDirection[1], anXDirection[2]));
      }
      const double aFirst = requiredNumber(theArgs, "firstParameter");
      const double aLast = requiredNumber(theArgs, "lastParameter");
      if (aFirst >= aLast)
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "Conic parameters must be finite and increasing");
      if (theOp == "makeEdgeHyperbola")
      {
        const double aMajorRadius = requiredNumber(theArgs, "majorRadius");
        const double aMinorRadius = requiredNumber(theArgs, "minorRadius");
        if (aMajorRadius <= 0.0 || aMinorRadius <= 0.0)
          throw KernelFailure(ErrorCode::InvalidArgs,
                              "Hyperbola radii must be positive");
        BRepBuilderAPI_MakeEdge aBuilder(
          gp_Hypr(anAxis, aMajorRadius, aMinorRadius), aFirst, aLast);
        if (!aBuilder.IsDone())
          throw KernelFailure(ErrorCode::ConstructionFailed,
                              "OCCT failed to construct hyperbola edge");
        return {{"shape", myArena.add(aBuilder.Edge(), aScope)}};
      }
      const double aFocal = requiredNumber(theArgs, "focal");
      if (aFocal <= 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs, "Parabola focal length must be positive");
      BRepBuilderAPI_MakeEdge aBuilder(gp_Parab(anAxis, aFocal), aFirst, aLast);
      if (!aBuilder.IsDone())
        throw KernelFailure(ErrorCode::ConstructionFailed,
                            "OCCT failed to construct parabola edge");
      return {{"shape", myArena.add(aBuilder.Edge(), aScope)}};
    }
    if (theOp == "makeEdgeOffset")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
      double aFirst = 0.0;
      double aLast = 0.0;
      const occ::handle<Geom_Curve> aCurve = curveForEdge(aShape, aFirst, aLast);
      const double anOffset = requiredNumber(theArgs, "offset");
      const auto aDirection = requiredDirection(theArgs, "direction");
      const occ::handle<Geom_OffsetCurve> anOffsetCurve = new Geom_OffsetCurve(
        aCurve, anOffset, gp_Dir(aDirection[0], aDirection[1], aDirection[2]));
      BRepBuilderAPI_MakeEdge aBuilder(anOffsetCurve, aFirst, aLast);
      if (!aBuilder.IsDone())
        throw KernelFailure(ErrorCode::ConstructionFailed,
                            "OCCT failed to construct offset curve edge");
      return {{"shape", myArena.add(aBuilder.Edge(), aScope)}};
    }
    if (theOp == "approximateCurveBSpline" || theOp == "approximateSurfaceBSpline")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const int aDegreeMin = theArgs.value("degreeMin", 3);
      const int aDegreeMax = theArgs.value("degreeMax", 8);
      const double aTolerance = theArgs.value("tolerance", 1.0e-3);
      const int aMaximumDegree = theOp == "approximateCurveBSpline"
        ? Geom_BSplineCurve::MaxDegree() : Geom_BSplineSurface::MaxDegree();
      if (aDegreeMin < 1 || aDegreeMax < aDegreeMin || aDegreeMax > aMaximumDegree
          || aTolerance <= 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs, "BSpline approximation degree range or tolerance is invalid");

      Approx_ParametrizationType aParameterization = Approx_ChordLength;
      const std::string aParameterizationName = theArgs.value("parameterization", "chordLength");
      if (aParameterizationName == "centripetal") aParameterization = Approx_Centripetal;
      else if (aParameterizationName == "uniform") aParameterization = Approx_IsoParametric;
      else if (aParameterizationName != "chordLength")
        throw KernelFailure(ErrorCode::InvalidArgs, "Unknown BSpline approximation parameterization");

      GeomAbs_Shape aContinuity = GeomAbs_C2;
      const std::string aContinuityName = theArgs.value("continuity", "c2");
      if (aContinuityName == "c0") aContinuity = GeomAbs_C0;
      else if (aContinuityName == "c1") aContinuity = GeomAbs_C1;
      else if (aContinuityName == "c3") aContinuity = GeomAbs_C3;
      else if (aContinuityName != "c2")
        throw KernelFailure(ErrorCode::InvalidArgs, "Unknown BSpline approximation continuity");

      std::array<double, 3> aSmoothingWeights{};
      const bool hasVariationalSmoothing = theArgs.contains("variationalSmoothing");
      if (hasVariationalSmoothing)
      {
        if (theArgs.contains("parameterization") || theArgs.contains("degreeMin"))
          throw KernelFailure(ErrorCode::InvalidArgs,
                              "Variational smoothing does not use parameterization or degreeMin");
        const json& aSmoothing = theArgs.at("variationalSmoothing");
        if (!aSmoothing.is_object())
          throw KernelFailure(ErrorCode::InvalidArgs, "Variational smoothing weights are invalid");
        for (std::size_t anIndex = 0; anIndex < aSmoothingWeights.size(); ++anIndex)
        {
          const char* aName = anIndex == 0 ? "length" : (anIndex == 1 ? "curvature" : "torsion");
          if (!aSmoothing.contains(aName) || !aSmoothing.at(aName).is_number())
            throw KernelFailure(ErrorCode::InvalidArgs, "Variational smoothing weights are invalid");
          aSmoothingWeights[anIndex] = aSmoothing.at(aName).get<double>();
          if (!std::isfinite(aSmoothingWeights[anIndex]) || aSmoothingWeights[anIndex] < 0.0)
            throw KernelFailure(ErrorCode::InvalidArgs, "Variational smoothing weights must be non-negative");
        }
        if (aSmoothingWeights[0] + aSmoothingWeights[1] + aSmoothingWeights[2] <= 0.0)
          throw KernelFailure(ErrorCode::InvalidArgs, "At least one variational smoothing weight must be positive");
      }

      if (!theArgs.contains("points"))
        throw KernelFailure(ErrorCode::InvalidArgs, "BSpline approximation points are required");
      if (theOp == "approximateCurveBSpline")
      {
        std::vector<gp_Pnt> aPointValues;
        if (theArgs.at("points").is_array())
        {
          for (const json& aPointValue : theArgs.at("points"))
          {
            if (!aPointValue.is_array() || aPointValue.size() != 3 || !aPointValue[0].is_number()
                || !aPointValue[1].is_number() || !aPointValue[2].is_number())
              throw KernelFailure(ErrorCode::InvalidArgs, "Curve approximation points must be vec3");
            aPointValues.emplace_back(aPointValue[0].get<double>(), aPointValue[1].get<double>(),
                                      aPointValue[2].get<double>());
          }
        }
        else if (theArgs.at("points").is_object())
        {
          const std::string aPointBytes = inputBufferData(theArgs, "points");
          if (aPointBytes.size() % (3 * sizeof(double)) != 0)
            throw KernelFailure(ErrorCode::InvalidArgs, "Curve approximation point buffer must use the f64x3 layout");
          for (std::size_t anOffset = 0; anOffset < aPointBytes.size(); anOffset += 3 * sizeof(double))
          {
            std::array<double, 3> aPoint{};
            std::memcpy(aPoint.data(), aPointBytes.data() + anOffset, 3 * sizeof(double));
            aPointValues.emplace_back(aPoint[0], aPoint[1], aPoint[2]);
          }
        }
        else
        {
          throw KernelFailure(ErrorCode::InvalidArgs, "Curve approximation points must be an array or f64x3 buffer reference");
        }
        if (aPointValues.size() < 2)
          throw KernelFailure(ErrorCode::InvalidArgs, "Curve approximation requires at least two points");
        NCollection_Array1<gp_Pnt> aPoints(1, static_cast<int>(aPointValues.size()));
        for (int anIndex = 1; anIndex <= aPoints.Upper(); ++anIndex)
          aPoints.SetValue(anIndex, aPointValues[static_cast<std::size_t>(anIndex - 1)]);
        GeomAPI_PointsToBSpline anApproximation;
        if (hasVariationalSmoothing)
          anApproximation.Init(aPoints, aSmoothingWeights[0], aSmoothingWeights[1],
                               aSmoothingWeights[2], aDegreeMax, aContinuity, aTolerance);
        else
          anApproximation.Init(
            aPoints, aParameterization, aDegreeMin, aDegreeMax, aContinuity, aTolerance);
        if (!anApproximation.IsDone() || anApproximation.Curve().IsNull())
          throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to approximate the BSpline curve");
        BRepBuilderAPI_MakeEdge aBuilder(anApproximation.Curve());
        if (!aBuilder.IsDone() || aBuilder.Edge().IsNull())
          throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct the approximated curve edge");
        return {{"shape", myArena.add(aBuilder.Edge(), aScope)}};
      }

      if (!theArgs.at("points").is_array() || theArgs.at("points").size() < 2)
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface approximation requires a point grid");
      const std::size_t aUCount = theArgs.at("points").size();
      const std::size_t aVCount = theArgs.at("points").at(0).is_array()
        ? theArgs.at("points").at(0).size() : 0;
      if (aVCount < 2)
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface approximation requires at least two points in each direction");
      NCollection_Array2<gp_Pnt> aPoints(1, static_cast<int>(aUCount), 1, static_cast<int>(aVCount));
      for (std::size_t aU = 0; aU < aUCount; ++aU)
      {
        const json& aRow = theArgs.at("points").at(aU);
        if (!aRow.is_array() || aRow.size() != aVCount)
          throw KernelFailure(ErrorCode::InvalidArgs, "Surface approximation point grid must be rectangular");
        for (std::size_t aV = 0; aV < aVCount; ++aV)
        {
          const json& aPoint = aRow.at(aV);
          if (!aPoint.is_array() || aPoint.size() != 3 || !aPoint[0].is_number()
              || !aPoint[1].is_number() || !aPoint[2].is_number())
            throw KernelFailure(ErrorCode::InvalidArgs, "Surface approximation points must be vec3");
          aPoints.SetValue(static_cast<int>(aU + 1), static_cast<int>(aV + 1),
                           gp_Pnt(aPoint[0].get<double>(), aPoint[1].get<double>(),
                                  aPoint[2].get<double>()));
        }
      }
      GeomAPI_PointsToBSplineSurface anApproximation;
      if (hasVariationalSmoothing)
        anApproximation.Init(aPoints, aSmoothingWeights[0], aSmoothingWeights[1],
                             aSmoothingWeights[2], aDegreeMax, aContinuity, aTolerance);
      else
        anApproximation.Init(
          aPoints, aParameterization, aDegreeMin, aDegreeMax, aContinuity, aTolerance);
      if (!anApproximation.IsDone() || anApproximation.Surface().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to approximate the BSpline surface");
      double aUFirst = 0.0;
      double aULast = 0.0;
      double aVFirst = 0.0;
      double aVLast = 0.0;
      anApproximation.Surface()->Bounds(aUFirst, aULast, aVFirst, aVLast);
      const TopoDS_Face aFace = makeRectangularFace(
        anApproximation.Surface(), aUFirst, aULast, aVFirst, aVLast,
        1.0e-7, TopAbs_FORWARD);
      return {{"shape", myArena.add(aFace, aScope)}};
    }
    if (theOp == "reduceCurveDegree")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
      if (aShape.ShapeType() != TopAbs_EDGE)
        throw KernelFailure(ErrorCode::InvalidArgs, "Curve degree reduction requires an edge shape");
      const TopoDS_Edge aEdge = TopoDS::Edge(aShape);
      BRepAdaptor_Curve anAdaptor(aEdge);
      if (anAdaptor.GetType() != GeomAbs_BezierCurve
          && anAdaptor.GetType() != GeomAbs_BSplineCurve)
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "Curve degree reduction requires a Bezier or BSpline edge");
      const int aTargetDegree = static_cast<int>(requiredU32(theArgs, "degree"));
      const int aMaxSegments = static_cast<int>(theArgs.value("maxSegments", 100u));
      const double aTolerance = theArgs.value("tolerance", 1.0e-3);
      if (aTargetDegree < 1 || aTargetDegree >= anAdaptor.Degree()
          || aMaxSegments < 1 || !std::isfinite(aTolerance) || aTolerance <= 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "Curve target degree must be lower than the input degree, with positive tolerance and maxSegments");
      GeomAbs_Shape aContinuity = GeomAbs_C1;
      const std::string aContinuityName = theArgs.value("continuity", "c1");
      if (aContinuityName == "c0") aContinuity = GeomAbs_C0;
      else if (aContinuityName == "c2") aContinuity = GeomAbs_C2;
      else if (aContinuityName == "c3") aContinuity = GeomAbs_C3;
      else if (aContinuityName != "c1")
        throw KernelFailure(ErrorCode::InvalidArgs, "Unknown curve degree reduction continuity");
      const int aContinuityOrder = aContinuity == GeomAbs_C0 ? 0
        : aContinuity == GeomAbs_C1 ? 1
        : aContinuity == GeomAbs_C2 ? 2 : 3;
      if (aContinuityOrder > aTargetDegree - 1)
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "Curve target degree is too low for the requested continuity");

      double aFirst = 0.0;
      double aLast = 0.0;
      const occ::handle<Geom_Curve> aCurve = curveForEdge(aShape, aFirst, aLast);
      const occ::handle<Geom_Curve> aTrimmedCurve =
        new Geom_TrimmedCurve(aCurve, aFirst, aLast);
      GeomConvert_ApproxCurve anApproximation(
        aTrimmedCurve, aTolerance, aContinuity, aMaxSegments, aTargetDegree);
      if (!anApproximation.IsDone() || !anApproximation.HasResult()
          || anApproximation.Curve().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed,
                            "OCCT could not reduce the curve degree within tolerance");
      BRepBuilderAPI_MakeEdge aBuilder(anApproximation.Curve(), aFirst, aLast);
      if (!aBuilder.IsDone() || aBuilder.Edge().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed,
                            "OCCT failed to construct the reduced-degree curve edge");
      TopoDS_Edge aResult = aBuilder.Edge();
      aResult.Orientation(aEdge.Orientation());
      return {{"shape", myArena.add(aResult, aScope)},
              {"maxError", anApproximation.MaxError()}};
    }
    if (theOp == "reduceSurfaceDegree")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
      if (aShape.ShapeType() != TopAbs_FACE)
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface degree reduction requires a face shape");
      const TopoDS_Face aFace = TopoDS::Face(aShape);
      BRepAdaptor_Surface anAdaptor(aFace, true);
      if (anAdaptor.GetType() != GeomAbs_BezierSurface
          && anAdaptor.GetType() != GeomAbs_BSplineSurface)
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "Surface degree reduction requires a Bezier or BSpline face");
      const int aUTargetDegree = static_cast<int>(requiredU32(theArgs, "uDegree"));
      const int aVTargetDegree = static_cast<int>(requiredU32(theArgs, "vDegree"));
      const int aMaxSegments = static_cast<int>(theArgs.value("maxSegments", 100u));
      const double aTolerance = theArgs.value("tolerance", 1.0e-3);
      if (aUTargetDegree < 1 || aVTargetDegree < 1
          || aUTargetDegree > anAdaptor.UDegree() || aVTargetDegree > anAdaptor.VDegree()
          || (aUTargetDegree == anAdaptor.UDegree() && aVTargetDegree == anAdaptor.VDegree())
          || aMaxSegments < 1 || !std::isfinite(aTolerance) || aTolerance <= 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "Surface target degrees must not exceed the input degrees and at least one must be lower, with positive tolerance and maxSegments");
      auto readContinuity = [&](const char* theKey) {
        const std::string aName = theArgs.value(theKey, "c1");
        if (aName == "c0") return GeomAbs_C0;
        if (aName == "c1") return GeomAbs_C1;
        if (aName == "c2") return GeomAbs_C2;
        if (aName == "c3") return GeomAbs_C3;
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "Unknown surface degree reduction continuity");
      };
      const GeomAbs_Shape aUContinuity = readContinuity("uContinuity");
      const GeomAbs_Shape aVContinuity = readContinuity("vContinuity");
      auto continuityOrder = [](const GeomAbs_Shape theContinuity) {
        return theContinuity == GeomAbs_C0 ? 0
          : theContinuity == GeomAbs_C1 ? 1
          : theContinuity == GeomAbs_C2 ? 2 : 3;
      };
      if (continuityOrder(aUContinuity) > aUTargetDegree - 1
          || continuityOrder(aVContinuity) > aVTargetDegree - 1)
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "Surface target degree is too low for the requested continuity");

      double aUFirst = 0.0;
      double aULast = 0.0;
      double aVFirst = 0.0;
      double aVLast = 0.0;
      const occ::handle<Geom_Surface> aSurface = surfaceForFace(
        aShape, aUFirst, aULast, aVFirst, aVLast);
      const occ::handle<Geom_Surface> aTrimmedSurface =
        new Geom_RectangularTrimmedSurface(
          aSurface, aUFirst, aULast, aVFirst, aVLast);
      GeomConvert_ApproxSurface anApproximation(
        aTrimmedSurface, aTolerance, aUContinuity, aVContinuity,
        aUTargetDegree, aVTargetDegree, aMaxSegments, 1);
      if (!anApproximation.IsDone() || !anApproximation.HasResult()
          || anApproximation.Surface().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed,
                            "OCCT could not reduce the surface degree within tolerance");
      const TopoDS_Face aResult = makeSurfaceFacePreservingWires(
        aFace, anApproximation.Surface(), aTolerance, aFace.Orientation());
      return {{"shape", myArena.add(aResult, aScope)},
              {"maxError", anApproximation.MaxError()}};
    }
    if (theOp == "extendCurve")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
      if (aShape.ShapeType() != TopAbs_EDGE)
        throw KernelFailure(ErrorCode::InvalidArgs, "Curve extension requires an edge shape");
      const TopoDS_Edge aEdge = TopoDS::Edge(aShape);
      BRepAdaptor_Curve anAdaptor(aEdge);
      occ::handle<Geom_BoundedCurve> aCurve;
      if (anAdaptor.GetType() == GeomAbs_BezierCurve)
      {
        aCurve = occ::down_cast<Geom_BoundedCurve>(anAdaptor.Bezier()->Copy());
      }
      else if (anAdaptor.GetType() == GeomAbs_BSplineCurve)
      {
        aCurve = occ::down_cast<Geom_BoundedCurve>(anAdaptor.BSpline()->Copy());
      }
      else
      {
        throw KernelFailure(ErrorCode::InvalidArgs, "Curve extension is available only for Bezier and BSpline edges");
      }
      const auto aPoint = requiredVec3(theArgs, "point");
      int aContinuity = 1;
      const std::string aContinuityName = theArgs.value("continuity", "c1");
      if (aContinuityName == "c2") aContinuity = 2;
      else if (aContinuityName == "c3") aContinuity = 3;
      else if (aContinuityName != "c1")
        throw KernelFailure(ErrorCode::InvalidArgs, "Unknown curve extension continuity");
      GeomLib::ExtendCurveToPoint(
        aCurve, gp_Pnt(aPoint[0], aPoint[1], aPoint[2]), aContinuity,
        theArgs.value("after", true));
      const double aFirst = aCurve->FirstParameter();
      const double aLast = aCurve->LastParameter();
      BRepBuilderAPI_MakeEdge aBuilder(aCurve, aFirst, aLast);
      if (!aBuilder.IsDone() || aBuilder.Edge().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct the extended curve edge");
      TopoDS_Edge anExtendedEdge = aBuilder.Edge();
      anExtendedEdge.Orientation(aEdge.Orientation());
      return {{"shape", myArena.add(anExtendedEdge, aScope)}};
    }
    if (theOp == "extendSurface")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
      if (aShape.ShapeType() != TopAbs_FACE)
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface extension requires a face shape");
      const TopoDS_Face aFace = TopoDS::Face(aShape);
      const occ::handle<Geom_Surface> aSurface = BRep_Tool::Surface(aFace);
      if (aSurface.IsNull())
        throw KernelFailure(ErrorCode::InvalidArgs, "Face has no geometric surface");
      double aUFirst = 0.0;
      double aULast = 0.0;
      double aVFirst = 0.0;
      double aVLast = 0.0;
      BRepTools::UVBounds(aFace, aUFirst, aULast, aVFirst, aVLast);
      if (!(aUFirst < aULast && aVFirst < aVLast))
        throw KernelFailure(ErrorCode::InvalidArgs, "Face has an invalid finite UV domain");
      const double aLength = requiredNumber(theArgs, "length");
      if (aLength <= 0.0) throw KernelFailure(ErrorCode::InvalidArgs, "Surface extension length must be positive");
      const std::string aDirection = theArgs.at("direction").get<std::string>();
      const std::string aSide = theArgs.at("side").get<std::string>();
      if ((aDirection != "u" && aDirection != "v") || (aSide != "before" && aSide != "after"))
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface extension direction or side is invalid");
      int aContinuity = 1;
      const std::string aContinuityName = theArgs.value("continuity", "c1");
      if (aContinuityName == "c2") aContinuity = 2;
      else if (aContinuityName == "c3") aContinuity = 3;
      else if (aContinuityName != "c1")
        throw KernelFailure(ErrorCode::InvalidArgs, "Unknown surface extension continuity");
      occ::handle<Geom_BoundedSurface> anExtendedSurface;
      if (aSurface->IsKind(STANDARD_TYPE(Geom_BoundedSurface)))
      {
        anExtendedSurface = occ::down_cast<Geom_BoundedSurface>(aSurface->Copy());
      }
      else
      {
        anExtendedSurface = new Geom_RectangularTrimmedSurface(
          aSurface, aUFirst, aULast, aVFirst, aVLast);
      }
      GeomLib::ExtendSurfByLength(
        anExtendedSurface, aLength, aContinuity, aDirection == "u", aSide == "after");
      anExtendedSurface->Bounds(aUFirst, aULast, aVFirst, aVLast);
      const std::array<double, 4> anExtendedBounds{aUFirst, aULast, aVFirst, aVLast};
      const TopoDS_Face anExtendedFace = makeSurfaceFacePreservingWires(
        aFace, anExtendedSurface, 1.0e-7, aFace.Orientation(), &anExtendedBounds);
      return {{"shape", myArena.add(anExtendedFace, aScope)}};
    }
    if (theOp == "makeEdgeBezier" || theOp == "makeEdgeBSpline" || theOp == "makeEdgeHelix")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      if (theOp == "makeEdgeHelix")
      {
        const double aRadius = requiredNumber(theArgs, "radius");
        const double aPitch = requiredNumber(theArgs, "pitch");
        const double aTurns = requiredNumber(theArgs, "turns");
        if (aRadius <= 0.0 || aPitch == 0.0 || aTurns <= 0.0) throw KernelFailure(ErrorCode::InvalidArgs, "Helix radius and turns must be positive and pitch non-zero");
        const auto anOrigin = optionalVec3(theArgs, "origin", {0.0, 0.0, 0.0});
        const auto aDirection = optionalDirection(theArgs, "direction", {0.0, 0.0, 1.0});
        const gp_Ax2 anAxis(gp_Pnt(anOrigin[0], anOrigin[1], anOrigin[2]), gp_Dir(aDirection[0], aDirection[1], aDirection[2]));
        const double aPi = 3.14159265358979323846;
        const occ::handle<Geom_Surface> aSurface = new Geom_CylindricalSurface(gp_Ax3(anAxis), aRadius);
        const std::string aHandedness = theArgs.value("handedness", "right");
        if (aHandedness != "right" && aHandedness != "left")
          throw KernelFailure(ErrorCode::InvalidArgs, "Helix handedness must be right or left");
        const double aSign = aHandedness == "right" ? 1.0 : -1.0;
        const double aSlope = aPitch / (2.0 * aPi);
        const gp_Dir2d aLineDirection(aSign, aSlope);
        const occ::handle<Geom2d_Line> aCurve = new Geom2d_Line(gp_Pnt2d(0.0, 0.0), aLineDirection);
        const double aEndU = aSign * 2.0 * aPi * aTurns;
        const double aEndParameter = aEndU / aLineDirection.X();
        BRepBuilderAPI_MakeEdge anEdge(aCurve, aSurface, 0.0, aEndParameter);
        if (!anEdge.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct helix edge");
        if (!BRepLib::BuildCurves3d(anEdge.Edge()))
          throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to build the helix 3D curve");
        return {{"shape", myArena.add(anEdge.Edge(), aScope)}};
      }
      if (!theArgs.contains("poles"))
        throw KernelFailure(ErrorCode::InvalidArgs, "Bezier/BSpline poles are required");
      std::vector<gp_Pnt> aPointValues;
      if (theArgs.at("poles").is_array())
      {
        for (const json& aPointValue : theArgs.at("poles"))
        {
          if (!aPointValue.is_array() || aPointValue.size() != 3 || !aPointValue[0].is_number()
              || !aPointValue[1].is_number() || !aPointValue[2].is_number())
            throw KernelFailure(ErrorCode::InvalidArgs, "Curve poles must be vec3");
          aPointValues.emplace_back(aPointValue[0].get<double>(), aPointValue[1].get<double>(), aPointValue[2].get<double>());
        }
      }
      else if (theArgs.at("poles").is_object())
      {
        const std::string aPoleBytes = inputBufferData(theArgs, "poles");
        if (aPoleBytes.size() % (3 * sizeof(double)) != 0)
          throw KernelFailure(ErrorCode::InvalidArgs, "Curve pole buffer must use the f64x3 layout");
        for (std::size_t anOffset = 0; anOffset < aPoleBytes.size(); anOffset += 3 * sizeof(double))
        {
          std::array<double, 3> aPoint{};
          std::memcpy(aPoint.data(), aPoleBytes.data() + anOffset, 3 * sizeof(double));
          aPointValues.emplace_back(aPoint[0], aPoint[1], aPoint[2]);
        }
      }
      else
      {
        throw KernelFailure(ErrorCode::InvalidArgs, "Curve poles must be an array or f64x3 buffer reference");
      }
      if (aPointValues.size() < 2)
        throw KernelFailure(ErrorCode::InvalidArgs, "Bezier/BSpline poles require at least two points");
      TColgp_Array1OfPnt aPoles(1, static_cast<int>(aPointValues.size()));
      for (int anIndex = 1; anIndex <= aPoles.Upper(); ++anIndex)
      {
        aPoles.SetValue(anIndex, aPointValues[static_cast<std::size_t>(anIndex - 1)]);
      }
      occ::handle<Geom_Curve> aCurve;
      if (theOp == "makeEdgeBezier")
      {
        if (theArgs.contains("weights"))
        {
          if (!theArgs.at("weights").is_array() || theArgs.at("weights").size() != aPointValues.size())
            throw KernelFailure(ErrorCode::InvalidArgs, "Bezier weights must match the pole count");
          TColStd_Array1OfReal aWeights(1, aPoles.Length());
          for (int aWeightIndex = 1; aWeightIndex <= aWeights.Upper(); ++aWeightIndex)
          {
            const json& aWeightValue = theArgs.at("weights").at(aWeightIndex - 1);
            if (!aWeightValue.is_number())
              throw KernelFailure(ErrorCode::InvalidArgs, "Bezier weights must be positive and finite");
            const double aWeight = aWeightValue.get<double>();
            if (aWeight <= 0.0 || !std::isfinite(aWeight))
              throw KernelFailure(ErrorCode::InvalidArgs, "Bezier weights must be positive and finite");
            aWeights.SetValue(aWeightIndex, aWeight);
          }
          aCurve = new Geom_BezierCurve(aPoles, aWeights);
        }
        else
        {
          aCurve = new Geom_BezierCurve(aPoles);
        }
      }
      else if (const std::string aMode = theArgs.value("mode", "interpolate");
               aMode != "interpolate" && aMode != "controlPoints")
      {
        throw KernelFailure(ErrorCode::InvalidArgs, "BSpline mode must be interpolate or controlPoints");
      }
      else if (aMode == "controlPoints")
      {
        const int aDegree = theArgs.value("degree", std::min(3, aPoles.Length() - 1));
        if (aDegree < 1 || aDegree >= aPoles.Length())
          throw KernelFailure(ErrorCode::InvalidArgs, "BSpline degree must be between 1 and poleCount - 1");
        const bool isPeriodic = theArgs.value("periodic", false);
        if (isPeriodic && (!theArgs.contains("knots") || !theArgs.contains("multiplicities")))
          throw KernelFailure(ErrorCode::InvalidArgs, "Periodic control-point BSplines require explicit knots and multiplicities");
        const int aKnotCount = theArgs.contains("knots")
          ? static_cast<int>(theArgs.at("knots").size()) : aPoles.Length() - aDegree + 1;
        if (aKnotCount < 2 || (theArgs.contains("multiplicities")
            && static_cast<int>(theArgs.at("multiplicities").size()) != aKnotCount))
          throw KernelFailure(ErrorCode::InvalidArgs, "BSpline knots and multiplicities must have equal length of at least two");
        TColStd_Array1OfReal aKnots(1, aKnotCount);
        TColStd_Array1OfInteger aMultiplicities(1, aKnotCount);
        for (int aKnotIndex = 1; aKnotIndex <= aKnotCount; ++aKnotIndex)
        {
          aKnots.SetValue(aKnotIndex, theArgs.contains("knots")
            ? theArgs.at("knots").at(aKnotIndex - 1).get<double>() : static_cast<double>(aKnotIndex - 1));
          aMultiplicities.SetValue(aKnotIndex, theArgs.contains("multiplicities")
            ? theArgs.at("multiplicities").at(aKnotIndex - 1).get<int>()
            : (aKnotIndex == 1 || aKnotIndex == aKnotCount ? aDegree + 1 : 1));
        }
        if (theArgs.contains("weights"))
        {
          if (!theArgs.at("weights").is_array() || theArgs.at("weights").size() != aPointValues.size())
            throw KernelFailure(ErrorCode::InvalidArgs, "BSpline weights must match the pole count");
          TColStd_Array1OfReal aWeights(1, aPoles.Length());
          for (int aWeightIndex = 1; aWeightIndex <= aWeights.Upper(); ++aWeightIndex)
            aWeights.SetValue(aWeightIndex, theArgs.at("weights").at(aWeightIndex - 1).get<double>());
          aCurve = new Geom_BSplineCurve(aPoles, aWeights, aKnots, aMultiplicities, aDegree, isPeriodic);
        }
        else
        {
          aCurve = new Geom_BSplineCurve(aPoles, aKnots, aMultiplicities, aDegree, isPeriodic);
        }
      }
      else
      {
        const occ::handle<TColgp_HArray1OfPnt> aInterpolationPoints = new TColgp_HArray1OfPnt(1, aPoles.Length());
        for (int aPointIndex = 1; aPointIndex <= aPoles.Length(); ++aPointIndex)
          aInterpolationPoints->SetValue(aPointIndex, aPoles.Value(aPointIndex));
        GeomAPI_Interpolate anInterpolation(aInterpolationPoints,
                                            theArgs.value("periodic", false),
                                            theArgs.value("tolerance", 1.0e-7));
        anInterpolation.Perform();
        if (!anInterpolation.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to interpolate BSpline");
        aCurve = anInterpolation.Curve();
      }
      BRepBuilderAPI_MakeEdge anEdge(aCurve);
      if (!anEdge.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct curve edge");
      return {{"shape", myArena.add(anEdge.Edge(), aScope)}};
    }
    if (theOp == "curveControlData")
    {
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
      if (aShape.ShapeType() != TopAbs_EDGE)
        throw KernelFailure(ErrorCode::InvalidArgs, "Curve control data requires an edge shape");
      BRepAdaptor_Curve anAdaptor(TopoDS::Edge(aShape));
      json aResult{{"poles", json::array()}, {"weights", json::array()},
                   {"knots", json::array()}, {"multiplicities", json::array()},
                   {"periodic", false}};
      if (anAdaptor.GetType() == GeomAbs_BezierCurve)
      {
        const occ::handle<Geom_BezierCurve> aBezier = anAdaptor.Bezier();
        aResult["type"] = "bezier";
        aResult["degree"] = aBezier->Degree();
        for (int anIndex = 1; anIndex <= aBezier->NbPoles(); ++anIndex)
        {
          const gp_Pnt aPole = aBezier->Pole(anIndex);
          aResult["poles"].push_back({aPole.X(), aPole.Y(), aPole.Z()});
          aResult["weights"].push_back(aBezier->Weight(anIndex));
        }
        return aResult;
      }
      if (anAdaptor.GetType() == GeomAbs_BSplineCurve)
      {
        const occ::handle<Geom_BSplineCurve> aBSpline = anAdaptor.BSpline();
        aResult["type"] = "bspline";
        aResult["degree"] = aBSpline->Degree();
        aResult["periodic"] = aBSpline->IsPeriodic();
        for (int anIndex = 1; anIndex <= aBSpline->NbPoles(); ++anIndex)
        {
          const gp_Pnt aPole = aBSpline->Pole(anIndex);
          aResult["poles"].push_back({aPole.X(), aPole.Y(), aPole.Z()});
          aResult["weights"].push_back(aBSpline->Weight(anIndex));
        }
        for (int anIndex = 1; anIndex <= aBSpline->NbKnots(); ++anIndex)
        {
          aResult["knots"].push_back(aBSpline->Knot(anIndex));
          aResult["multiplicities"].push_back(aBSpline->Multiplicity(anIndex));
        }
        return aResult;
      }
      throw KernelFailure(ErrorCode::InvalidArgs, "Curve control data is available only for Bezier and BSpline edges");
    }
    if (theOp == "updateCurvePole")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
      if (aShape.ShapeType() != TopAbs_EDGE)
        throw KernelFailure(ErrorCode::InvalidArgs, "Curve pole editing requires an edge shape");
      const TopoDS_Edge aEdge = TopoDS::Edge(aShape);
      BRepAdaptor_Curve anAdaptor(aEdge);
      const std::uint32_t anIndex = requiredU32(theArgs, "index");
      const auto aPoint = requiredVec3(theArgs, "point");
      const gp_Pnt aPole(aPoint[0], aPoint[1], aPoint[2]);
      occ::handle<Geom_Curve> anEditedCurve;
      if (anAdaptor.GetType() == GeomAbs_BezierCurve)
      {
        const occ::handle<Geom_BezierCurve> aBezier =
          occ::down_cast<Geom_BezierCurve>(anAdaptor.Bezier()->Copy());
        if (anIndex >= static_cast<std::uint32_t>(aBezier->NbPoles()))
          throw KernelFailure(ErrorCode::InvalidArgs, "Curve pole index is out of range");
        if (theArgs.contains("weight"))
        {
          const double aWeight = requiredNumber(theArgs, "weight");
          if (aWeight <= 0.0) throw KernelFailure(ErrorCode::InvalidArgs, "Curve pole weight must be positive");
          aBezier->SetPole(static_cast<int>(anIndex + 1), aPole, aWeight);
        }
        else
        {
          aBezier->SetPole(static_cast<int>(anIndex + 1), aPole);
        }
        anEditedCurve = aBezier;
      }
      else if (anAdaptor.GetType() == GeomAbs_BSplineCurve)
      {
        const occ::handle<Geom_BSplineCurve> aBSpline =
          occ::down_cast<Geom_BSplineCurve>(anAdaptor.BSpline()->Copy());
        if (anIndex >= static_cast<std::uint32_t>(aBSpline->NbPoles()))
          throw KernelFailure(ErrorCode::InvalidArgs, "Curve pole index is out of range");
        if (theArgs.contains("weight"))
        {
          const double aWeight = requiredNumber(theArgs, "weight");
          if (aWeight <= 0.0) throw KernelFailure(ErrorCode::InvalidArgs, "Curve pole weight must be positive");
          aBSpline->SetPole(static_cast<int>(anIndex + 1), aPole, aWeight);
        }
        else
        {
          aBSpline->SetPole(static_cast<int>(anIndex + 1), aPole);
        }
        anEditedCurve = aBSpline;
      }
      else
      {
        throw KernelFailure(ErrorCode::InvalidArgs, "Curve pole editing is available only for Bezier and BSpline edges");
      }
      BRepBuilderAPI_MakeEdge aBuilder(
        anEditedCurve, anAdaptor.FirstParameter(), anAdaptor.LastParameter());
      if (!aBuilder.IsDone() || aBuilder.Edge().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct the edited curve edge");
      TopoDS_Edge anEditedEdge = aBuilder.Edge();
      anEditedEdge.Orientation(aEdge.Orientation());
      return {{"shape", myArena.add(anEditedEdge, aScope)}};
    }
    if (theOp == "editCurveBSpline")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
      if (aShape.ShapeType() != TopAbs_EDGE)
        throw KernelFailure(ErrorCode::InvalidArgs, "BSpline curve editing requires an edge shape");
      const TopoDS_Edge aEdge = TopoDS::Edge(aShape);
      BRepAdaptor_Curve anAdaptor(aEdge);
      if (anAdaptor.GetType() != GeomAbs_BSplineCurve)
        throw KernelFailure(ErrorCode::InvalidArgs, "BSpline curve editing requires a BSpline edge");
      const occ::handle<Geom_BSplineCurve> aBSpline =
        occ::down_cast<Geom_BSplineCurve>(anAdaptor.BSpline()->Copy());
      const std::string anAction = theArgs.value("action", "");
      const double aTolerance = theArgs.value("tolerance", 1.0e-7);
      if (aTolerance <= 0.0 || !std::isfinite(aTolerance))
        throw KernelFailure(ErrorCode::InvalidArgs, "BSpline curve editing tolerance must be positive and finite");

      auto readArray = [&](const char* theKey) {
        if (!theArgs.contains(theKey) || !theArgs.at(theKey).is_array() || theArgs.at(theKey).empty())
          throw KernelFailure(ErrorCode::InvalidArgs, std::string("BSpline curve ") + theKey + " must be a non-empty array");
        return theArgs.at(theKey);
      };
      if (anAction == "insertKnot")
      {
        const double aKnot = requiredNumber(theArgs, "knot");
        const int aMultiplicity = theArgs.value("multiplicity", 1);
        if (!std::isfinite(aKnot) || aMultiplicity <= 0)
          throw KernelFailure(ErrorCode::InvalidArgs, "BSpline knot and multiplicity must be valid");
        aBSpline->InsertKnot(aKnot, aMultiplicity, aTolerance, theArgs.value("add", true));
      }
      else if (anAction == "insertKnots")
      {
        const json& aKnots = readArray("knots");
        const json& aMultiplicities = readArray("multiplicities");
        if (aKnots.size() != aMultiplicities.size())
          throw KernelFailure(ErrorCode::InvalidArgs, "BSpline knots and multiplicities must have equal length");
        TColStd_Array1OfReal aKnotArray(1, static_cast<int>(aKnots.size()));
        TColStd_Array1OfInteger aMultiplicityArray(1, static_cast<int>(aMultiplicities.size()));
        for (int anIndex = 1; anIndex <= aKnotArray.Upper(); ++anIndex)
        {
          const double aKnot = aKnots.at(static_cast<std::size_t>(anIndex - 1)).get<double>();
          const int aMultiplicity = aMultiplicities.at(static_cast<std::size_t>(anIndex - 1)).get<int>();
          if (!std::isfinite(aKnot) || aMultiplicity <= 0)
            throw KernelFailure(ErrorCode::InvalidArgs, "BSpline knot and multiplicity must be valid");
          aKnotArray.SetValue(anIndex, aKnot);
          aMultiplicityArray.SetValue(anIndex, aMultiplicity);
        }
        aBSpline->InsertKnots(aKnotArray, aMultiplicityArray, aTolerance, theArgs.value("add", false));
      }
      else if (anAction == "removeKnot")
      {
        const std::uint32_t aKnotIndex = requiredU32(theArgs, "knotIndex");
        const int aMultiplicity = static_cast<int>(requiredU32(theArgs, "multiplicity"));
        if (aKnotIndex >= static_cast<std::uint32_t>(aBSpline->NbKnots()) || aMultiplicity < 0)
          throw KernelFailure(ErrorCode::InvalidArgs, "BSpline knot index or multiplicity is out of range");
        if (!aBSpline->RemoveKnot(static_cast<int>(aKnotIndex + 1), aMultiplicity, aTolerance))
          throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT could not remove the BSpline knot within tolerance");
      }
      else if (anAction == "increaseDegree")
      {
        const std::uint32_t aDegree = requiredU32(theArgs, "degree");
        if (aDegree > static_cast<std::uint32_t>(Geom_BSplineCurve::MaxDegree()))
          throw KernelFailure(ErrorCode::InvalidArgs, "BSpline curve degree exceeds the OCCT maximum");
        aBSpline->IncreaseDegree(static_cast<int>(aDegree));
      }
      else if (anAction == "setPeriodic")
      {
        aBSpline->SetPeriodic();
      }
      else if (anAction == "setNotPeriodic")
      {
        aBSpline->SetNotPeriodic();
      }
      else if (anAction == "setControlNet")
      {
        const json& aPoles = readArray("poles");
        if (aPoles.size() != static_cast<std::size_t>(aBSpline->NbPoles()))
          throw KernelFailure(ErrorCode::InvalidArgs, "BSpline curve poles must match the control net size");
        const bool hasWeights = theArgs.contains("weights");
        if (hasWeights && (!theArgs.at("weights").is_array()
                           || theArgs.at("weights").size() != aPoles.size()))
          throw KernelFailure(ErrorCode::InvalidArgs, "BSpline curve weights must match the pole count");
        for (int anIndex = 1; anIndex <= aBSpline->NbPoles(); ++anIndex)
        {
          const json& aPoint = aPoles.at(static_cast<std::size_t>(anIndex - 1));
          if (!aPoint.is_array() || aPoint.size() != 3 || !aPoint.at(0).is_number()
              || !aPoint.at(1).is_number() || !aPoint.at(2).is_number())
            throw KernelFailure(ErrorCode::InvalidArgs, "BSpline curve poles must be vec3 values");
          const gp_Pnt aPole(aPoint.at(0).get<double>(), aPoint.at(1).get<double>(), aPoint.at(2).get<double>());
          if (hasWeights)
          {
            const double aWeight = theArgs.at("weights").at(static_cast<std::size_t>(anIndex - 1)).get<double>();
            if (aWeight <= 0.0 || !std::isfinite(aWeight))
              throw KernelFailure(ErrorCode::InvalidArgs, "BSpline curve weights must be positive and finite");
            aBSpline->SetPole(anIndex, aPole, aWeight);
          }
          else
          {
            aBSpline->SetPole(anIndex, aPole);
          }
        }
      }
      else
      {
        throw KernelFailure(ErrorCode::InvalidArgs, "Unknown BSpline curve edit action");
      }

      BRepBuilderAPI_MakeEdge aBuilder(aBSpline, aBSpline->FirstParameter(), aBSpline->LastParameter());
      if (!aBuilder.IsDone() || aBuilder.Edge().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct the edited BSpline curve edge");
      TopoDS_Edge anEditedEdge = aBuilder.Edge();
      anEditedEdge.Orientation(aEdge.Orientation());
      return {{"shape", myArena.add(anEditedEdge, aScope)}};
    }
    if (theOp == "reparameterizeCurve")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
      if (aShape.ShapeType() != TopAbs_EDGE)
        throw KernelFailure(ErrorCode::InvalidArgs, "Curve reparameterization requires an edge shape");
      const TopoDS_Edge aEdge = TopoDS::Edge(aShape);
      BRepAdaptor_Curve anAdaptor(aEdge);
      if (anAdaptor.GetType() != GeomAbs_BSplineCurve)
        throw KernelFailure(ErrorCode::InvalidArgs, "Curve reparameterization requires a BSpline edge");
      const double aFirst = requiredNumber(theArgs, "first");
      const double aLast = requiredNumber(theArgs, "last");
      if (!std::isfinite(aFirst) || !std::isfinite(aLast) || aFirst >= aLast)
        throw KernelFailure(ErrorCode::InvalidArgs, "Curve parameter domain must be finite and increasing");
      const occ::handle<Geom_BSplineCurve> aBSpline =
        occ::down_cast<Geom_BSplineCurve>(anAdaptor.BSpline()->Copy());
      TColStd_Array1OfReal aKnots(1, aBSpline->NbKnots());
      for (int anIndex = 1; anIndex <= aBSpline->NbKnots(); ++anIndex)
        aKnots.SetValue(anIndex, aBSpline->Knot(anIndex));
      BSplCLib::Reparametrize(aFirst, aLast, aKnots);
      aBSpline->SetKnots(aKnots);
      BRepBuilderAPI_MakeEdge aBuilder(aBSpline, aFirst, aLast);
      if (!aBuilder.IsDone() || aBuilder.Edge().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed,
                            "OCCT failed to construct the reparameterized curve edge");
      TopoDS_Edge aResult = aBuilder.Edge();
      aResult.Orientation(aEdge.Orientation());
      return {{"shape", myArena.add(aResult, aScope)}};
    }
    if (theOp == "trimCurve" || theOp == "convertCurveToBSpline")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      double aFirst = 0.0;
      double aLast = 0.0;
      const occ::handle<Geom_Curve> aCurve = curveForEdge(
        myArena.get(requiredU32(theArgs, "shape")), aFirst, aLast);
      if (theOp == "trimCurve")
      {
        const double aTrimFirst = requiredNumber(theArgs, "first");
        const double aTrimLast = requiredNumber(theArgs, "last");
        if (aTrimFirst < aFirst || aTrimLast > aLast || aTrimFirst >= aTrimLast)
          throw KernelFailure(ErrorCode::InvalidArgs, "Trim parameters must define an increasing subset of the edge domain");
        BRepBuilderAPI_MakeEdge aBuilder(aCurve, aTrimFirst, aTrimLast);
        if (!aBuilder.IsDone() || aBuilder.Edge().IsNull())
          throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to trim the curve");
        return {{"shape", myArena.add(aBuilder.Edge(), aScope)}};
      }

      Convert_ParameterisationType aParameterization = Convert_TgtThetaOver2;
      const std::string aParameterizationName = theArgs.value("parameterization", "tgtThetaOver2");
      if (aParameterizationName == "quasiAngular") aParameterization = Convert_QuasiAngular;
      else if (aParameterizationName == "rationalC1") aParameterization = Convert_RationalC1;
      else if (aParameterizationName == "polynomial") aParameterization = Convert_Polynomial;
      else if (aParameterizationName != "tgtThetaOver2")
        throw KernelFailure(ErrorCode::InvalidArgs, "Unknown curve conversion parameterization");
      const occ::handle<Geom_TrimmedCurve> aTrimmed = new Geom_TrimmedCurve(aCurve, aFirst, aLast);
      const occ::handle<Geom_BSplineCurve> aBSpline =
        GeomConvert::CurveToBSplineCurve(aTrimmed, aParameterization);
      if (aBSpline.IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to convert the curve to BSpline");
      BRepBuilderAPI_MakeEdge aBuilder(aBSpline);
      if (!aBuilder.IsDone() || aBuilder.Edge().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct the converted BSpline edge");
      return {{"shape", myArena.add(aBuilder.Edge(), aScope)}};
    }
    if (theOp == "makePolygon")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      if (!theArgs.contains("points") || !theArgs.at("points").is_array() || theArgs.at("points").size() < 2)
        throw KernelFailure(ErrorCode::InvalidArgs, "makePolygon requires at least two points");
      BRepBuilderAPI_MakePolygon aBuilder;
      for (const json& aPoint : theArgs.at("points"))
      {
        if (!aPoint.is_array() || aPoint.size() != 3) throw KernelFailure(ErrorCode::InvalidArgs, "Polygon points must be vec3");
        aBuilder.Add(gp_Pnt(aPoint[0].get<double>(), aPoint[1].get<double>(), aPoint[2].get<double>()));
      }
      if (theArgs.value("close", true)) aBuilder.Close();
      if (!aBuilder.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct polygon");
      return {{"shape", myArena.add(aBuilder.Wire(), aScope)}};
    }
    if (theOp == "makeWire")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      if (!theArgs.contains("edges") || !theArgs.at("edges").is_array() || theArgs.at("edges").empty())
        throw KernelFailure(ErrorCode::InvalidArgs, "makeWire requires edges");
      BRepBuilderAPI_MakeWire aBuilder;
      for (const json& anEdge : theArgs.at("edges")) aBuilder.Add(TopoDS::Edge(myArena.get(anEdge.get<std::uint32_t>())));
      if (!aBuilder.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct wire");
      return {{"shape", myArena.add(aBuilder.Wire(), aScope)}};
    }
    if (theOp == "makeFace")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Wire anOuter = TopoDS::Wire(myArena.get(requiredU32(theArgs, "outer")));
      std::vector<TopoDS_Wire> aHoles;
      if (theArgs.contains("holes"))
      {
        if (!theArgs.at("holes").is_array()) throw KernelFailure(ErrorCode::InvalidArgs, "Face holes must be an array");
        for (const json& aHoleValue : theArgs.at("holes"))
        {
          TopoDS_Wire aHole = TopoDS::Wire(myArena.get(aHoleValue.get<std::uint32_t>()));
          auto isValidWithHole = [&](const TopoDS_Wire& theCandidate) {
            BRepBuilderAPI_MakeFace aCandidate(anOuter, true);
            for (const TopoDS_Wire& anAcceptedHole : aHoles) aCandidate.Add(anAcceptedHole);
            aCandidate.Add(theCandidate);
            return aCandidate.IsDone() && !aCandidate.Face().IsNull()
              && BRepCheck_Analyzer(aCandidate.Face(), true).IsValid();
          };
          if (!isValidWithHole(aHole))
          {
            aHole.Reverse();
            if (!isValidWithHole(aHole))
              throw KernelFailure(ErrorCode::ConstructionFailed, "Face hole does not define a valid planar inner wire");
          }
          aHoles.push_back(aHole);
        }
      }
      BRepBuilderAPI_MakeFace aBuilder(anOuter, true);
      for (const TopoDS_Wire& aHole : aHoles) aBuilder.Add(aHole);
      if (!aBuilder.IsDone() || aBuilder.Face().IsNull()
          || !BRepCheck_Analyzer(aBuilder.Face(), true).IsValid())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct a valid planar face");
      return {{"shape", myArena.add(aBuilder.Face(), aScope)}};
    }
    if (theOp == "makeSurfaceFilling")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aBoundaryShape = myArena.get(requiredU32(theArgs, "boundary"));
      if (aBoundaryShape.ShapeType() != TopAbs_WIRE)
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface filling boundary must be a wire");
      const TopoDS_Wire aBoundary = TopoDS::Wire(aBoundaryShape);
      if (!BRep_Tool::IsClosed(aBoundary))
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface filling boundary wire must be closed");

      const int aDegree = theArgs.value("degree", 3);
      const int aPointsOnCurve = theArgs.value("pointsOnCurve", 15);
      const int anIterations = theArgs.value("iterations", 2);
      const bool isAnisotropic = theArgs.value("anisotropic", false);
      const double aTolerance2d = theArgs.value("tolerance2d", 1.0e-5);
      const double aTolerance3d = theArgs.value("tolerance3d", 1.0e-4);
      const double anAngularTolerance = theArgs.value("angularTolerance", 1.0e-2);
      const double aCurvatureTolerance = theArgs.value("curvatureTolerance", 1.0e-1);
      const int aMaxDegree = theArgs.value("maxDegree", 8);
      const int aMaxSegments = theArgs.value("maxSegments", 9);
      if (aDegree < 2 || aPointsOnCurve < 2 || anIterations < 1 || aMaxDegree < 1
          || aMaxSegments < 1 || aTolerance2d <= 0.0 || aTolerance3d <= 0.0
          || anAngularTolerance <= 0.0 || aCurvatureTolerance <= 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface filling settings must be positive and use degree >= 2");

      BRepOffsetAPI_MakeFilling aBuilder(
        aDegree, aPointsOnCurve, anIterations, isAnisotropic, aTolerance2d,
        aTolerance3d, anAngularTolerance, aCurvatureTolerance, aMaxDegree,
        aMaxSegments);
      std::vector<TopoDS_Edge> aBoundaryEdges;
      int anEdgeCount = 0;
      for (BRepTools_WireExplorer anExplorer(aBoundary); anExplorer.More(); anExplorer.Next())
      {
        aBoundaryEdges.push_back(TopoDS::Edge(anExplorer.Current()));
        ++anEdgeCount;
      }
      if (anEdgeCount < 2)
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface filling boundary must contain at least two edges");
      std::vector<GeomAbs_Shape> aConstraintOrders(aBoundaryEdges.size(), GeomAbs_C0);
      std::vector<TopoDS_Face> aConstraintSupports(aBoundaryEdges.size());
      if (theArgs.contains("constraints"))
      {
        const json& aConstraints = theArgs.at("constraints");
        if (!aConstraints.is_array())
          throw KernelFailure(ErrorCode::InvalidArgs, "Surface filling constraints must be an array");
        std::unordered_set<std::uint32_t> aConstrainedEdges;
        for (const json& aConstraint : aConstraints)
        {
          if (!aConstraint.is_object() || !aConstraint.contains("edgeIndex")
              || !aConstraint.contains("support") || !aConstraint.contains("continuity")
              || !aConstraint.at("edgeIndex").is_number_unsigned()
              || !aConstraint.at("support").is_number_unsigned()
              || !aConstraint.at("continuity").is_string())
            throw KernelFailure(ErrorCode::InvalidArgs, "Surface filling constraints require edgeIndex, support, and continuity");
          const std::uint32_t anEdgeIndex = aConstraint.at("edgeIndex").get<std::uint32_t>();
          if (anEdgeIndex >= aBoundaryEdges.size() || !aConstrainedEdges.insert(anEdgeIndex).second)
            throw KernelFailure(ErrorCode::InvalidArgs, "Surface filling constraint edgeIndex must be unique and in wire range");
          const TopoDS_Shape& aSupportShape = myArena.get(aConstraint.at("support").get<std::uint32_t>());
          if (aSupportShape.ShapeType() != TopAbs_FACE)
            throw KernelFailure(ErrorCode::InvalidArgs, "Surface filling constraint support must be a face");
          const std::string aContinuity = aConstraint.at("continuity").get<std::string>();
          // BRepFill interprets this enum value as a raw G-order (0..2), while
          // GeomAbs_G2 is 3 because GeomAbs_C1 occupies value 2.
          const GeomAbs_Shape anOrder = aContinuity == "g1" ? GeomAbs_G1
            : aContinuity == "g2" ? static_cast<GeomAbs_Shape>(2) : GeomAbs_C0;
          if (anOrder == GeomAbs_C0)
            throw KernelFailure(ErrorCode::InvalidArgs, "Surface filling constraint continuity must be g1 or g2");
          double aFirst = 0.0;
          double aLast = 0.0;
          if (BRep_Tool::CurveOnSurface(aBoundaryEdges[anEdgeIndex], TopoDS::Face(aSupportShape), aFirst, aLast).IsNull())
            throw KernelFailure(ErrorCode::InvalidArgs, "Surface filling support face has no pcurve for the constrained edge");
          aConstraintOrders[anEdgeIndex] = anOrder;
          aConstraintSupports[anEdgeIndex] = TopoDS::Face(aSupportShape);
        }
      }
      for (std::size_t anIndex = 0; anIndex < aBoundaryEdges.size(); ++anIndex)
      {
        if (aConstraintOrders[anIndex] == GeomAbs_C0)
          aBuilder.Add(aBoundaryEdges[anIndex], GeomAbs_C0, true);
        else
          aBuilder.Add(aBoundaryEdges[anIndex], aConstraintSupports[anIndex], aConstraintOrders[anIndex], true);
      }
      if (theArgs.contains("points"))
      {
        const json& aPoints = theArgs.at("points");
        if (!aPoints.is_array())
          throw KernelFailure(ErrorCode::InvalidArgs, "Surface filling points must be an array");
        for (const json& aPointValue : aPoints)
        {
          if (!aPointValue.is_array() || aPointValue.size() != 3
              || !aPointValue.at(0).is_number() || !aPointValue.at(1).is_number()
              || !aPointValue.at(2).is_number())
            throw KernelFailure(ErrorCode::InvalidArgs, "Surface filling points must be vec3 values");
          aBuilder.Add(gp_Pnt(aPointValue.at(0).get<double>(),
                              aPointValue.at(1).get<double>(),
                              aPointValue.at(2).get<double>()));
        }
      }
      aBuilder.Build();
      if (!aBuilder.IsDone() || aBuilder.Shape().IsNull()
          || aBuilder.Shape().ShapeType() != TopAbs_FACE
          || !BRepCheck_Analyzer(aBuilder.Shape(), true).IsValid())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct a valid filling surface");
      return {{"shape", myArena.add(aBuilder.Shape(), aScope)}};
    }
    if (theOp == "makeFaceOnSurface")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aSupportShape = myArena.get(requiredU32(theArgs, "surface"));
      if (aSupportShape.ShapeType() != TopAbs_FACE)
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface support must be a face");
      const TopoDS_Face aSupportFace = TopoDS::Face(aSupportShape);
      const occ::handle<Geom_Surface> aSurface = BRep_Tool::Surface(aSupportFace);
      if (aSurface.IsNull())
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface support face has no geometry");
      const double aTolerance = theArgs.value("tolerance", 1.0e-7);
      if (aTolerance <= 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface face tolerance must be positive");

      auto projectWire = [&](const std::uint32_t theHandle) {
        const TopoDS_Shape& aWireShape = myArena.get(theHandle);
        if (aWireShape.ShapeType() != TopAbs_WIRE)
          throw KernelFailure(ErrorCode::InvalidArgs, "Surface face boundaries must be wires");
        BRepBuilderAPI_Copy aCopy(aWireShape, true, false);
        if (!aCopy.IsDone() || aCopy.Shape().IsNull())
          throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to copy a surface boundary wire");
        TopoDS_Wire aWire = TopoDS::Wire(aCopy.Shape());
        ShapeFix_Edge aFixer;
        for (TopExp_Explorer anExplorer(aWire, TopAbs_EDGE); anExplorer.More(); anExplorer.Next())
        {
          const TopoDS_Edge anEdge = TopoDS::Edge(anExplorer.Current());
          double aFirst = 0.0;
          double aLast = 0.0;
          occ::handle<Geom2d_Curve> aPCurve = BRep_Tool::CurveOnSurface(
            anEdge, aSurface, TopLoc_Location(), aFirst, aLast);
          if (aPCurve.IsNull())
          {
            aFixer.FixAddPCurve(anEdge, aSurface, TopLoc_Location(), false, aTolerance);
            aPCurve = BRep_Tool::CurveOnSurface(
              anEdge, aSurface, TopLoc_Location(), aFirst, aLast);
          }
          if (aPCurve.IsNull())
            throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to project a boundary edge onto the surface");
          aFixer.FixSameParameter(anEdge, aTolerance);
        }
        return aWire;
      };

      const TopoDS_Wire anOuter = projectWire(requiredU32(theArgs, "outer"));
      std::vector<TopoDS_Wire> aHoles;
      if (theArgs.contains("holes"))
      {
        if (!theArgs.at("holes").is_array())
          throw KernelFailure(ErrorCode::InvalidArgs, "Surface face holes must be an array");
        for (const json& aHoleValue : theArgs.at("holes"))
        {
          if (!aHoleValue.is_number_unsigned() && !aHoleValue.is_number_integer())
            throw KernelFailure(ErrorCode::InvalidArgs, "Surface face hole handles must be integers");
          aHoles.push_back(projectWire(aHoleValue.get<std::uint32_t>()));
        }
      }

      auto buildFace = [&](const std::vector<TopoDS_Wire>& theHoles, TopoDS_Face& theFace) {
        BRepBuilderAPI_MakeFace aBuilder(aSurface, anOuter, true);
        for (const TopoDS_Wire& aHole : theHoles) aBuilder.Add(aHole);
        if (!aBuilder.IsDone() || aBuilder.Face().IsNull()
            || !BRepCheck_Analyzer(aBuilder.Face(), true).IsValid())
          return false;
        theFace = aBuilder.Face();
        return true;
      };
      TopoDS_Face aFace;
      if (!buildFace(aHoles, aFace))
      {
        for (TopoDS_Wire& aHole : aHoles) aHole.Reverse();
        if (!buildFace(aHoles, aFace))
          throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct a valid wire-trimmed surface face");
      }
      aFace.Orientation(aSupportFace.Orientation());
      return {{"shape", myArena.add(aFace, aScope)}};
    }
    if (theOp == "makeSurfaceBezier" || theOp == "makeSurfaceBSpline")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      if (!theArgs.contains("poles") || !theArgs.at("poles").is_array()
          || theArgs.at("poles").size() < 2)
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface poles must be a rectangular U-major grid with at least two rows");
      const json& aPoleGrid = theArgs.at("poles");
      if (!aPoleGrid.at(0).is_array() || aPoleGrid.at(0).size() < 2)
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface poles require at least two V poles");
      const int aUCount = static_cast<int>(aPoleGrid.size());
      const int aVCount = static_cast<int>(aPoleGrid.at(0).size());
      TColgp_Array2OfPnt aPoles(1, aUCount, 1, aVCount);
      for (int aU = 0; aU < aUCount; ++aU)
      {
        const json& aRow = aPoleGrid.at(aU);
        if (!aRow.is_array() || static_cast<int>(aRow.size()) != aVCount)
          throw KernelFailure(ErrorCode::InvalidArgs, "Surface pole grid must be rectangular");
        for (int aV = 0; aV < aVCount; ++aV)
        {
          const json& aPoint = aRow.at(aV);
          if (!aPoint.is_array() || aPoint.size() != 3 || !aPoint.at(0).is_number()
              || !aPoint.at(1).is_number() || !aPoint.at(2).is_number())
            throw KernelFailure(ErrorCode::InvalidArgs, "Surface poles must be vec3 values");
          aPoles.SetValue(aU + 1, aV + 1,
                          gp_Pnt(aPoint.at(0).get<double>(), aPoint.at(1).get<double>(),
                                 aPoint.at(2).get<double>()));
        }
      }

      const bool hasWeights = theArgs.contains("weights");
      TColStd_Array2OfReal aWeights(1, aUCount, 1, aVCount);
      if (hasWeights)
      {
        const json& aWeightGrid = theArgs.at("weights");
        if (!aWeightGrid.is_array() || static_cast<int>(aWeightGrid.size()) != aUCount)
          throw KernelFailure(ErrorCode::InvalidArgs, "Surface weights must match the pole grid");
        for (int aU = 0; aU < aUCount; ++aU)
        {
          const json& aRow = aWeightGrid.at(aU);
          if (!aRow.is_array() || static_cast<int>(aRow.size()) != aVCount)
            throw KernelFailure(ErrorCode::InvalidArgs, "Surface weights must match the pole grid");
          for (int aV = 0; aV < aVCount; ++aV)
          {
            if (!aRow.at(aV).is_number() || aRow.at(aV).get<double>() <= 0.0)
              throw KernelFailure(ErrorCode::InvalidArgs, "Surface weights must be positive numbers");
            aWeights.SetValue(aU + 1, aV + 1, aRow.at(aV).get<double>());
          }
        }
      }

      occ::handle<Geom_Surface> aSurface;
      if (theOp == "makeSurfaceBezier")
      {
        if (aUCount - 1 > Geom_BezierSurface::MaxDegree()
            || aVCount - 1 > Geom_BezierSurface::MaxDegree())
          throw KernelFailure(ErrorCode::InvalidArgs, "Bezier surface degree exceeds the OCCT maximum");
        aSurface = hasWeights
          ? occ::handle<Geom_Surface>(new Geom_BezierSurface(aPoles, aWeights))
          : occ::handle<Geom_Surface>(new Geom_BezierSurface(aPoles));
      }
      else
      {
        const int aUDegree = theArgs.value("uDegree", std::min(3, aUCount - 1));
        const int aVDegree = theArgs.value("vDegree", std::min(3, aVCount - 1));
        if (aUDegree < 1 || aUDegree >= aUCount || aUDegree > Geom_BSplineSurface::MaxDegree()
            || aVDegree < 1 || aVDegree >= aVCount || aVDegree > Geom_BSplineSurface::MaxDegree())
          throw KernelFailure(ErrorCode::InvalidArgs, "BSpline surface degrees must be between 1 and poleCount - 1");
        const bool isUPeriodic = theArgs.value("uPeriodic", false);
        const bool isVPeriodic = theArgs.value("vPeriodic", false);
        std::vector<double> aUKnots;
        std::vector<double> aVKnots;
        std::vector<int> aUMultiplicities;
        std::vector<int> aVMultiplicities;
        auto readDirection = [&](const char* theKnotKey,
                                 const char* theMultiplicityKey,
                                 const int thePoleCount,
                                 const int theDegree,
                                 const bool isPeriodic,
                                 std::vector<double>& theKnots,
                                 std::vector<int>& theMultiplicities) {
          const bool hasKnots = theArgs.contains(theKnotKey);
          const bool hasMultiplicities = theArgs.contains(theMultiplicityKey);
          if (hasKnots != hasMultiplicities)
            throw KernelFailure(ErrorCode::InvalidArgs, "BSpline knots and multiplicities must be provided together");
          if (isPeriodic && !hasKnots)
            throw KernelFailure(ErrorCode::InvalidArgs, "Periodic BSpline surfaces require explicit knots and multiplicities");
          if (!hasKnots)
          {
            const int aKnotCount = thePoleCount - theDegree + 1;
            for (int anIndex = 0; anIndex < aKnotCount; ++anIndex)
            {
              theKnots.push_back(static_cast<double>(anIndex));
              theMultiplicities.push_back(
                anIndex == 0 || anIndex == aKnotCount - 1 ? theDegree + 1 : 1);
            }
          }
          else
          {
            const json& aKnots = theArgs.at(theKnotKey);
            const json& aMultiplicities = theArgs.at(theMultiplicityKey);
            if (!aKnots.is_array() || !aMultiplicities.is_array() || aKnots.size() < 2
                || aKnots.size() != aMultiplicities.size())
              throw KernelFailure(ErrorCode::InvalidArgs, "BSpline knots and multiplicities must have equal length of at least two");
            for (std::size_t anIndex = 0; anIndex < aKnots.size(); ++anIndex)
            {
              if (!aKnots.at(anIndex).is_number()
                  || (!aMultiplicities.at(anIndex).is_number_integer()
                      && !aMultiplicities.at(anIndex).is_number_unsigned()))
                throw KernelFailure(ErrorCode::InvalidArgs, "BSpline knot data must be numeric with integer multiplicities");
              const double aKnot = aKnots.at(anIndex).get<double>();
              const int aMultiplicity = aMultiplicities.at(anIndex).get<int>();
              if (!std::isfinite(aKnot) || aMultiplicity <= 0
                  || (!theKnots.empty() && aKnot <= theKnots.back()))
                throw KernelFailure(ErrorCode::InvalidArgs, "BSpline knots must increase and multiplicities must be positive");
              theKnots.push_back(aKnot);
              theMultiplicities.push_back(aMultiplicity);
            }
          }
          TColStd_Array1OfInteger aMultiplicityArray(1, static_cast<int>(theMultiplicities.size()));
          for (int anIndex = 1; anIndex <= aMultiplicityArray.Upper(); ++anIndex)
            aMultiplicityArray.SetValue(anIndex, theMultiplicities[static_cast<std::size_t>(anIndex - 1)]);
          if (BSplCLib::NbPoles(theDegree, isPeriodic, aMultiplicityArray) != thePoleCount)
            throw KernelFailure(ErrorCode::InvalidArgs, "BSpline multiplicities do not match the degree and pole count");
        };
        readDirection("uKnots", "uMultiplicities", aUCount, aUDegree, isUPeriodic,
                      aUKnots, aUMultiplicities);
        readDirection("vKnots", "vMultiplicities", aVCount, aVDegree, isVPeriodic,
                      aVKnots, aVMultiplicities);

        TColStd_Array1OfReal aUKnotArray(1, static_cast<int>(aUKnots.size()));
        TColStd_Array1OfReal aVKnotArray(1, static_cast<int>(aVKnots.size()));
        TColStd_Array1OfInteger aUMultiplicityArray(1, static_cast<int>(aUMultiplicities.size()));
        TColStd_Array1OfInteger aVMultiplicityArray(1, static_cast<int>(aVMultiplicities.size()));
        for (int anIndex = 1; anIndex <= aUKnotArray.Upper(); ++anIndex)
        {
          aUKnotArray.SetValue(anIndex, aUKnots[static_cast<std::size_t>(anIndex - 1)]);
          aUMultiplicityArray.SetValue(anIndex, aUMultiplicities[static_cast<std::size_t>(anIndex - 1)]);
        }
        for (int anIndex = 1; anIndex <= aVKnotArray.Upper(); ++anIndex)
        {
          aVKnotArray.SetValue(anIndex, aVKnots[static_cast<std::size_t>(anIndex - 1)]);
          aVMultiplicityArray.SetValue(anIndex, aVMultiplicities[static_cast<std::size_t>(anIndex - 1)]);
        }
        aSurface = hasWeights
          ? occ::handle<Geom_Surface>(new Geom_BSplineSurface(
              aPoles, aWeights, aUKnotArray, aVKnotArray, aUMultiplicityArray,
              aVMultiplicityArray, aUDegree, aVDegree, isUPeriodic, isVPeriodic))
          : occ::handle<Geom_Surface>(new Geom_BSplineSurface(
              aPoles, aUKnotArray, aVKnotArray, aUMultiplicityArray,
              aVMultiplicityArray, aUDegree, aVDegree, isUPeriodic, isVPeriodic));
      }

      const double aTolerance = theArgs.value("tolerance", 1.0e-7);
      double aUFirst = 0.0;
      double aULast = 0.0;
      double aVFirst = 0.0;
      double aVLast = 0.0;
      aSurface->Bounds(aUFirst, aULast, aVFirst, aVLast);
      const TopoDS_Face aFace = makeRectangularFace(
        aSurface, aUFirst, aULast, aVFirst, aVLast, aTolerance, TopAbs_FORWARD);
      return {{"shape", myArena.add(aFace, aScope)}};
    }
    if (theOp == "makeSurfaceExtrusion" || theOp == "makeSurfaceRevolution")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aProfile = myArena.get(requiredU32(theArgs, "profile"));
      double aCurveFirst = 0.0;
      double aCurveLast = 0.0;
      const occ::handle<Geom_Curve> aCurve = curveForEdge(
        aProfile, aCurveFirst, aCurveLast);
      const occ::handle<Geom_TrimmedCurve> aProfileCurve = new Geom_TrimmedCurve(
        aCurve, aCurveFirst, aCurveLast);
      const double aTolerance = theArgs.value("tolerance", 1.0e-7);
      if (theOp == "makeSurfaceExtrusion")
      {
        const auto aDirection = requiredDirection(theArgs, "direction");
        const double aVFirst = requiredNumber(theArgs, "vFirst");
        const double aVLast = requiredNumber(theArgs, "vLast");
        const occ::handle<Geom_SurfaceOfLinearExtrusion> aSurface =
          new Geom_SurfaceOfLinearExtrusion(
            aProfileCurve, gp_Dir(aDirection[0], aDirection[1], aDirection[2]));
        const TopoDS_Face aFace = makeRectangularFace(
          aSurface, aCurveFirst, aCurveLast, aVFirst, aVLast, aTolerance,
          TopAbs_FORWARD);
        return {{"shape", myArena.add(aFace, aScope)}};
      }
      const auto anOrigin = optionalVec3(theArgs, "origin", {0.0, 0.0, 0.0});
      const auto aDirection = optionalDirection(theArgs, "direction", {0.0, 0.0, 1.0});
      const double anAngle = theArgs.value("angle", 6.28318530717958647693);
      if (anAngle <= 0.0 || anAngle > 6.28318530717958647693)
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "Revolution surface angle must be in (0, 2*pi]");
      const occ::handle<Geom_SurfaceOfRevolution> aSurface =
        new Geom_SurfaceOfRevolution(
          aProfileCurve,
          gp_Ax1(gp_Pnt(anOrigin[0], anOrigin[1], anOrigin[2]),
                 gp_Dir(aDirection[0], aDirection[1], aDirection[2])));
      const TopoDS_Face aFace = makeRectangularFace(
        aSurface, 0.0, anAngle, aCurveFirst, aCurveLast, aTolerance,
        TopAbs_FORWARD);
      return {{"shape", myArena.add(aFace, aScope)}};
    }
    if (theOp == "makeSurfaceRuled")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aFirstShape = myArena.get(requiredU32(theArgs, "first"));
      const TopoDS_Shape& aSecondShape = myArena.get(requiredU32(theArgs, "second"));
      double aFirstStart = 0.0;
      double aFirstEnd = 0.0;
      double aSecondStart = 0.0;
      double aSecondEnd = 0.0;
      curveForEdge(aFirstShape, aFirstStart, aFirstEnd);
      curveForEdge(aSecondShape, aSecondStart, aSecondEnd);
      const TopoDS_Face aFace = BRepFill::Face(
        TopoDS::Edge(aFirstShape), TopoDS::Edge(aSecondShape));
      if (aFace.IsNull() || !BRepCheck_Analyzer(aFace, true).IsValid())
        throw KernelFailure(ErrorCode::ConstructionFailed,
                            "OCCT failed to construct ruled surface");
      return {{"shape", myArena.add(aFace, aScope)}};
    }
    if (theOp == "makeSurfaceOffset")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
      if (aShape.ShapeType() != TopAbs_FACE)
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "Offset surface construction requires a face shape");
      const TopoDS_Face aSourceFace = TopoDS::Face(aShape);
      const occ::handle<Geom_Surface> aSourceSurface = BRep_Tool::Surface(aSourceFace);
      if (aSourceSurface.IsNull())
        throw KernelFailure(ErrorCode::InvalidArgs, "Face has no geometric surface");
      const occ::handle<Geom_OffsetSurface> aSurface = new Geom_OffsetSurface(
        aSourceSurface, requiredNumber(theArgs, "offset"));
      const TopoDS_Face aFace = makeSurfaceFacePreservingWires(
        aSourceFace, aSurface, theArgs.value("tolerance", 1.0e-7),
        aSourceFace.Orientation());
      return {{"shape", myArena.add(aFace, aScope)}};
    }
    if (theOp == "makeSurfaceFace")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      if (!theArgs.contains("surface") || !theArgs.at("surface").is_object())
        throw KernelFailure(ErrorCode::InvalidArgs, "makeSurfaceFace requires a surface definition");
      const json& aDefinition = theArgs.at("surface");
      const std::string aType = aDefinition.value("type", "");
      const auto anOrigin = optionalVec3(aDefinition, "origin", {0.0, 0.0, 0.0});
      const auto aDirection = optionalDirection(aDefinition, "direction", {0.0, 0.0, 1.0});
      gp_Ax3 anAxis(gp_Pnt(anOrigin[0], anOrigin[1], anOrigin[2]),
                    gp_Dir(aDirection[0], aDirection[1], aDirection[2]));
      if (aDefinition.contains("xDirection"))
      {
        const auto anXDirection = requiredDirection(aDefinition, "xDirection");
        if (gp_Dir(aDirection[0], aDirection[1], aDirection[2]).IsParallel(
              gp_Dir(anXDirection[0], anXDirection[1], anXDirection[2]), 1.0e-12))
          throw KernelFailure(ErrorCode::InvalidArgs, "Surface direction and xDirection must not be parallel");
        anAxis = gp_Ax3(gp_Pnt(anOrigin[0], anOrigin[1], anOrigin[2]),
                        gp_Dir(aDirection[0], aDirection[1], aDirection[2]),
                        gp_Dir(anXDirection[0], anXDirection[1], anXDirection[2]));
      }

      occ::handle<Geom_Surface> aSurface;
      if (aType == "plane")
      {
        aSurface = new Geom_Plane(anAxis);
      }
      else if (aType == "cylinder" || aType == "sphere")
      {
        const double aRadius = requiredNumber(aDefinition, "radius");
        if (aRadius <= 0.0)
          throw KernelFailure(ErrorCode::InvalidArgs, "Surface radius must be positive");
        aSurface = aType == "cylinder"
          ? occ::handle<Geom_Surface>(new Geom_CylindricalSurface(anAxis, aRadius))
          : occ::handle<Geom_Surface>(new Geom_SphericalSurface(anAxis, aRadius));
      }
      else if (aType == "cone")
      {
        const double aReferenceRadius = requiredNumber(aDefinition, "referenceRadius");
        const double aSemiAngle = requiredNumber(aDefinition, "semiAngle");
        if (aReferenceRadius < 0.0 || std::abs(aSemiAngle) <= 1.0e-12
            || std::abs(aSemiAngle) >= 1.57079632679489661923)
          throw KernelFailure(ErrorCode::InvalidArgs, "Cone referenceRadius must be non-negative and semiAngle must be between -pi/2 and pi/2 excluding zero");
        aSurface = new Geom_ConicalSurface(anAxis, aSemiAngle, aReferenceRadius);
      }
      else if (aType == "torus")
      {
        const double aMajorRadius = requiredNumber(aDefinition, "majorRadius");
        const double aMinorRadius = requiredNumber(aDefinition, "minorRadius");
        if (aMajorRadius <= 0.0 || aMinorRadius <= 0.0 || aMajorRadius <= aMinorRadius)
          throw KernelFailure(ErrorCode::InvalidArgs, "Torus radii must be positive with majorRadius > minorRadius");
        aSurface = new Geom_ToroidalSurface(anAxis, aMajorRadius, aMinorRadius);
      }
      else
      {
        throw KernelFailure(ErrorCode::InvalidArgs, "Unknown analytic surface type");
      }

      const double aUFirst = requiredNumber(theArgs, "uFirst");
      const double aULast = requiredNumber(theArgs, "uLast");
      const double aVFirst = requiredNumber(theArgs, "vFirst");
      const double aVLast = requiredNumber(theArgs, "vLast");
      const double aTolerance = theArgs.value("tolerance", 1.0e-7);
      if (aUFirst >= aULast || aVFirst >= aVLast || aTolerance <= 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface bounds must be increasing and tolerance positive");
      BRepBuilderAPI_MakeFace aBuilder(aSurface, aUFirst, aULast, aVFirst, aVLast, aTolerance);
      if (!aBuilder.IsDone() || aBuilder.Face().IsNull()
          || !BRepCheck_Analyzer(aBuilder.Face(), true).IsValid())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct a valid bounded surface face");
      return {{"shape", myArena.add(aBuilder.Face(), aScope)}};
    }
    if (theOp == "surfaceControlData")
    {
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
      if (aShape.ShapeType() != TopAbs_FACE)
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface control data requires a face shape");
      const occ::handle<Geom_Surface> aSurface = BRep_Tool::Surface(TopoDS::Face(aShape));
      if (aSurface.IsNull())
        throw KernelFailure(ErrorCode::InvalidArgs, "Face has no geometric surface");
      json aResult{{"poles", json::array()}, {"weights", json::array()},
                   {"uKnots", json::array()}, {"vKnots", json::array()},
                   {"uMultiplicities", json::array()}, {"vMultiplicities", json::array()},
                   {"uPeriodic", false}, {"vPeriodic", false}};
      if (aSurface->IsKind(STANDARD_TYPE(Geom_BezierSurface)))
      {
        const occ::handle<Geom_BezierSurface> aBezier = occ::down_cast<Geom_BezierSurface>(aSurface);
        aResult["type"] = "bezier";
        aResult["uDegree"] = aBezier->UDegree();
        aResult["vDegree"] = aBezier->VDegree();
        for (int aU = 1; aU <= aBezier->NbUPoles(); ++aU)
        {
          json aPoleRow = json::array();
          json aWeightRow = json::array();
          for (int aV = 1; aV <= aBezier->NbVPoles(); ++aV)
          {
            const gp_Pnt aPole = aBezier->Pole(aU, aV);
            aPoleRow.push_back({aPole.X(), aPole.Y(), aPole.Z()});
            aWeightRow.push_back(aBezier->Weight(aU, aV));
          }
          aResult["poles"].push_back(aPoleRow);
          aResult["weights"].push_back(aWeightRow);
        }
        return aResult;
      }
      if (aSurface->IsKind(STANDARD_TYPE(Geom_BSplineSurface)))
      {
        const occ::handle<Geom_BSplineSurface> aBSpline = occ::down_cast<Geom_BSplineSurface>(aSurface);
        aResult["type"] = "bspline";
        aResult["uDegree"] = aBSpline->UDegree();
        aResult["vDegree"] = aBSpline->VDegree();
        aResult["uPeriodic"] = aBSpline->IsUPeriodic();
        aResult["vPeriodic"] = aBSpline->IsVPeriodic();
        for (int aU = 1; aU <= aBSpline->NbUPoles(); ++aU)
        {
          json aPoleRow = json::array();
          json aWeightRow = json::array();
          for (int aV = 1; aV <= aBSpline->NbVPoles(); ++aV)
          {
            const gp_Pnt aPole = aBSpline->Pole(aU, aV);
            aPoleRow.push_back({aPole.X(), aPole.Y(), aPole.Z()});
            aWeightRow.push_back(aBSpline->Weight(aU, aV));
          }
          aResult["poles"].push_back(aPoleRow);
          aResult["weights"].push_back(aWeightRow);
        }
        for (int anIndex = 1; anIndex <= aBSpline->NbUKnots(); ++anIndex)
        {
          aResult["uKnots"].push_back(aBSpline->UKnot(anIndex));
          aResult["uMultiplicities"].push_back(aBSpline->UMultiplicity(anIndex));
        }
        for (int anIndex = 1; anIndex <= aBSpline->NbVKnots(); ++anIndex)
        {
          aResult["vKnots"].push_back(aBSpline->VKnot(anIndex));
          aResult["vMultiplicities"].push_back(aBSpline->VMultiplicity(anIndex));
        }
        return aResult;
      }
      throw KernelFailure(ErrorCode::InvalidArgs, "Surface control data is available only for Bezier and BSpline faces");
    }
    if (theOp == "editSurfaceBSpline")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
      if (aShape.ShapeType() != TopAbs_FACE)
        throw KernelFailure(ErrorCode::InvalidArgs, "BSpline surface editing requires a face shape");
      const TopoDS_Face aFace = TopoDS::Face(aShape);
      const occ::handle<Geom_Surface> aSurface = BRep_Tool::Surface(aFace);
      if (aSurface.IsNull() || !aSurface->IsKind(STANDARD_TYPE(Geom_BSplineSurface)))
        throw KernelFailure(ErrorCode::InvalidArgs, "BSpline surface editing requires a BSpline face");
      const occ::handle<Geom_BSplineSurface> aBSpline =
        occ::down_cast<Geom_BSplineSurface>(aSurface->Copy());
      const std::string anAction = theArgs.value("action", "");
      const double aTolerance = theArgs.value("tolerance", 1.0e-7);
      if (aTolerance <= 0.0 || !std::isfinite(aTolerance))
        throw KernelFailure(ErrorCode::InvalidArgs, "BSpline surface editing tolerance must be positive and finite");

      auto readArray = [&](const char* theKey) -> const json& {
        if (!theArgs.contains(theKey) || !theArgs.at(theKey).is_array() || theArgs.at(theKey).empty())
          throw KernelFailure(ErrorCode::InvalidArgs, std::string("BSpline surface ") + theKey + " must be a non-empty array");
        return theArgs.at(theKey);
      };
      auto insertKnots = [&](const char* theKnotsKey, const char* theMultiplicitiesKey,
                             const bool isU) {
        const json& aKnots = readArray(theKnotsKey);
        const json& aMultiplicities = readArray(theMultiplicitiesKey);
        if (aKnots.size() != aMultiplicities.size())
          throw KernelFailure(ErrorCode::InvalidArgs, "BSpline surface knots and multiplicities must have equal length");
        TColStd_Array1OfReal aKnotArray(1, static_cast<int>(aKnots.size()));
        TColStd_Array1OfInteger aMultiplicityArray(1, static_cast<int>(aMultiplicities.size()));
        for (int anIndex = 1; anIndex <= aKnotArray.Upper(); ++anIndex)
        {
          const double aKnot = aKnots.at(static_cast<std::size_t>(anIndex - 1)).get<double>();
          const int aMultiplicity = aMultiplicities.at(static_cast<std::size_t>(anIndex - 1)).get<int>();
          if (!std::isfinite(aKnot) || aMultiplicity <= 0)
            throw KernelFailure(ErrorCode::InvalidArgs, "BSpline surface knot and multiplicity must be valid");
          aKnotArray.SetValue(anIndex, aKnot);
          aMultiplicityArray.SetValue(anIndex, aMultiplicity);
        }
        if (isU)
          aBSpline->InsertUKnots(aKnotArray, aMultiplicityArray, aTolerance, theArgs.value("add", true));
        else
          aBSpline->InsertVKnots(aKnotArray, aMultiplicityArray, aTolerance, theArgs.value("add", true));
      };

      if (anAction == "insertUKnot")
      {
        const double aKnot = requiredNumber(theArgs, "uKnot");
        const int aMultiplicity = static_cast<int>(requiredU32(theArgs, "uMultiplicity"));
        if (!std::isfinite(aKnot) || aMultiplicity <= 0)
          throw KernelFailure(ErrorCode::InvalidArgs, "BSpline U knot and multiplicity must be valid");
        aBSpline->InsertUKnot(aKnot, aMultiplicity, aTolerance, theArgs.value("add", true));
      }
      else if (anAction == "insertVKnot")
      {
        const double aKnot = requiredNumber(theArgs, "vKnot");
        const int aMultiplicity = static_cast<int>(requiredU32(theArgs, "vMultiplicity"));
        if (!std::isfinite(aKnot) || aMultiplicity <= 0)
          throw KernelFailure(ErrorCode::InvalidArgs, "BSpline V knot and multiplicity must be valid");
        aBSpline->InsertVKnot(aKnot, aMultiplicity, aTolerance, theArgs.value("add", true));
      }
      else if (anAction == "insertUKnots")
      {
        insertKnots("uKnots", "uMultiplicities", true);
      }
      else if (anAction == "insertVKnots")
      {
        insertKnots("vKnots", "vMultiplicities", false);
      }
      else if (anAction == "removeUKnot" || anAction == "removeVKnot")
      {
        const char* anIndexKey = anAction == "removeUKnot" ? "uKnotIndex" : "vKnotIndex";
        const char* aMultiplicityKey = anAction == "removeUKnot" ? "uMultiplicity" : "vMultiplicity";
        const std::uint32_t aKnotIndex = requiredU32(theArgs, anIndexKey);
        const int aMultiplicity = static_cast<int>(requiredU32(theArgs, aMultiplicityKey));
        const int aKnotCount = anAction == "removeUKnot" ? aBSpline->NbUKnots() : aBSpline->NbVKnots();
        if (aKnotIndex >= static_cast<std::uint32_t>(aKnotCount))
          throw KernelFailure(ErrorCode::InvalidArgs, "BSpline surface knot index is out of range");
        const bool isRemoved = anAction == "removeUKnot"
          ? aBSpline->RemoveUKnot(static_cast<int>(aKnotIndex + 1), aMultiplicity, aTolerance)
          : aBSpline->RemoveVKnot(static_cast<int>(aKnotIndex + 1), aMultiplicity, aTolerance);
        if (!isRemoved)
          throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT could not remove the BSpline surface knot within tolerance");
      }
      else if (anAction == "increaseDegree")
      {
        const std::uint32_t aUDegree = requiredU32(theArgs, "uDegree");
        const std::uint32_t aVDegree = requiredU32(theArgs, "vDegree");
        if (aUDegree > static_cast<std::uint32_t>(Geom_BSplineSurface::MaxDegree())
            || aVDegree > static_cast<std::uint32_t>(Geom_BSplineSurface::MaxDegree()))
          throw KernelFailure(ErrorCode::InvalidArgs, "BSpline surface degree exceeds the OCCT maximum");
        aBSpline->IncreaseDegree(static_cast<int>(aUDegree), static_cast<int>(aVDegree));
      }
      else if (anAction == "setUPeriodic")
      {
        aBSpline->SetUPeriodic();
      }
      else if (anAction == "setVPeriodic")
      {
        aBSpline->SetVPeriodic();
      }
      else if (anAction == "setUNotPeriodic")
      {
        aBSpline->SetUNotPeriodic();
      }
      else if (anAction == "setVNotPeriodic")
      {
        aBSpline->SetVNotPeriodic();
      }
      else if (anAction == "exchangeUV")
      {
        aBSpline->ExchangeUV();
      }
      else if (anAction == "reverseU")
      {
        aBSpline->UReverse();
      }
      else if (anAction == "reverseV")
      {
        aBSpline->VReverse();
      }
      else if (anAction == "setControlNet")
      {
        const json& aPoles = readArray("poles");
        if (aPoles.size() != static_cast<std::size_t>(aBSpline->NbUPoles()))
          throw KernelFailure(ErrorCode::InvalidArgs, "BSpline surface pole grid U size does not match");
        const bool hasWeights = theArgs.contains("weights");
        if (hasWeights && (!theArgs.at("weights").is_array()
                           || theArgs.at("weights").size() != aPoles.size()))
          throw KernelFailure(ErrorCode::InvalidArgs, "BSpline surface weight grid U size does not match");
        for (int anUIndex = 1; anUIndex <= aBSpline->NbUPoles(); ++anUIndex)
        {
          const json& aPoleRow = aPoles.at(static_cast<std::size_t>(anUIndex - 1));
          if (!aPoleRow.is_array() || aPoleRow.size() != static_cast<std::size_t>(aBSpline->NbVPoles()))
            throw KernelFailure(ErrorCode::InvalidArgs, "BSpline surface pole grid V size does not match");
          const json* aWeightRow = nullptr;
          if (hasWeights)
          {
            aWeightRow = &theArgs.at("weights").at(static_cast<std::size_t>(anUIndex - 1));
            if (!aWeightRow->is_array() || aWeightRow->size() != aPoleRow.size())
              throw KernelFailure(ErrorCode::InvalidArgs, "BSpline surface weight grid V size does not match");
          }
          for (int anVIndex = 1; anVIndex <= aBSpline->NbVPoles(); ++anVIndex)
          {
            const json& aPoint = aPoleRow.at(static_cast<std::size_t>(anVIndex - 1));
            if (!aPoint.is_array() || aPoint.size() != 3 || !aPoint.at(0).is_number()
                || !aPoint.at(1).is_number() || !aPoint.at(2).is_number())
              throw KernelFailure(ErrorCode::InvalidArgs, "BSpline surface poles must be vec3 values");
            const gp_Pnt aPole(aPoint.at(0).get<double>(), aPoint.at(1).get<double>(), aPoint.at(2).get<double>());
            if (aWeightRow != nullptr)
            {
              const double aWeight = aWeightRow->at(static_cast<std::size_t>(anVIndex - 1)).get<double>();
              if (aWeight <= 0.0 || !std::isfinite(aWeight))
                throw KernelFailure(ErrorCode::InvalidArgs, "BSpline surface weights must be positive and finite");
              aBSpline->SetPole(anUIndex, anVIndex, aPole, aWeight);
            }
            else
            {
              aBSpline->SetPole(anUIndex, anVIndex, aPole);
            }
          }
        }
      }
      else
      {
        throw KernelFailure(ErrorCode::InvalidArgs, "Unknown BSpline surface edit action");
      }

      const TopoDS_Face anEditedFace = makeSurfaceFacePreservingWires(
        aFace, aBSpline, aTolerance, aFace.Orientation());
      return {{"shape", myArena.add(anEditedFace, aScope)}};
    }
    if (theOp == "reparameterizeSurface")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
      if (aShape.ShapeType() != TopAbs_FACE)
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "Surface reparameterization requires a face shape");
      const TopoDS_Face aFace = TopoDS::Face(aShape);
      const occ::handle<Geom_Surface> aSurface = BRep_Tool::Surface(aFace);
      if (aSurface.IsNull() || !aSurface->IsKind(STANDARD_TYPE(Geom_BSplineSurface)))
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "Surface reparameterization requires a BSpline face");
      const double aUFirst = requiredNumber(theArgs, "uFirst");
      const double aULast = requiredNumber(theArgs, "uLast");
      const double aVFirst = requiredNumber(theArgs, "vFirst");
      const double aVLast = requiredNumber(theArgs, "vLast");
      const double aTolerance = theArgs.value("tolerance", 1.0e-7);
      if (!std::isfinite(aUFirst) || !std::isfinite(aULast)
          || !std::isfinite(aVFirst) || !std::isfinite(aVLast)
          || aUFirst >= aULast || aVFirst >= aVLast
          || !std::isfinite(aTolerance) || aTolerance <= 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "Surface parameter domains must be finite and increasing");
      const occ::handle<Geom_BSplineSurface> aBSpline =
        occ::down_cast<Geom_BSplineSurface>(aSurface->Copy());
      TColStd_Array1OfReal aUKnots(1, aBSpline->NbUKnots());
      TColStd_Array1OfReal aVKnots(1, aBSpline->NbVKnots());
      for (int anIndex = 1; anIndex <= aBSpline->NbUKnots(); ++anIndex)
        aUKnots.SetValue(anIndex, aBSpline->UKnot(anIndex));
      for (int anIndex = 1; anIndex <= aBSpline->NbVKnots(); ++anIndex)
        aVKnots.SetValue(anIndex, aBSpline->VKnot(anIndex));
      BSplCLib::Reparametrize(aUFirst, aULast, aUKnots);
      BSplCLib::Reparametrize(aVFirst, aVLast, aVKnots);
      aBSpline->SetUKnots(aUKnots);
      aBSpline->SetVKnots(aVKnots);

      BRepBuilderAPI_Copy aCopy(aFace, true, false);
      if (!aCopy.IsDone() || aCopy.Shape().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed,
                            "OCCT failed to copy the surface boundary topology");
      const TopoDS_Face aCopiedFace = TopoDS::Face(aCopy.Shape());
      ShapeFix_Edge aFixer;
      for (TopExp_Explorer anExplorer(aCopiedFace, TopAbs_EDGE);
           anExplorer.More(); anExplorer.Next())
      {
        const TopoDS_Edge anEdge = TopoDS::Edge(anExplorer.Current());
        aFixer.FixAddPCurve(anEdge, aBSpline, TopLoc_Location(),
                            BRep_Tool::IsClosed(anEdge, aCopiedFace), aTolerance);
        double aFirst = 0.0;
        double aLast = 0.0;
        if (BRep_Tool::CurveOnSurface(
              anEdge, aBSpline, TopLoc_Location(), aFirst, aLast).IsNull())
          throw KernelFailure(ErrorCode::ConstructionFailed,
                              "OCCT failed to reparameterize a surface boundary pcurve");
        aFixer.FixSameParameter(anEdge, aTolerance);
      }
      const TopoDS_Wire anOuter = BRepTools::OuterWire(aCopiedFace);
      if (anOuter.IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed,
                            "Surface reparameterization requires an outer wire");
      BRepBuilderAPI_MakeFace aBuilder(aBSpline, anOuter, true);
      for (TopExp_Explorer anExplorer(aCopiedFace, TopAbs_WIRE);
           anExplorer.More(); anExplorer.Next())
      {
        const TopoDS_Wire aWire = TopoDS::Wire(anExplorer.Current());
        if (!aWire.IsSame(anOuter)) aBuilder.Add(aWire);
      }
      if (!aBuilder.IsDone() || aBuilder.Face().IsNull()
          || !BRepCheck_Analyzer(aBuilder.Face(), true).IsValid())
        throw KernelFailure(ErrorCode::ConstructionFailed,
                            "OCCT failed to construct the reparameterized surface face");
      TopoDS_Face aResult = aBuilder.Face();
      aResult.Orientation(aFace.Orientation());
      return {{"shape", myArena.add(aResult, aScope)}};
    }
    if (theOp == "updateSurfacePole" || theOp == "trimSurface" || theOp == "convertSurfaceToBSpline")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
      if (aShape.ShapeType() != TopAbs_FACE)
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface editing requires a face shape");
      const TopoDS_Face aFace = TopoDS::Face(aShape);
      const occ::handle<Geom_Surface> aSurface = BRep_Tool::Surface(aFace);
      if (aSurface.IsNull())
        throw KernelFailure(ErrorCode::InvalidArgs, "Face has no geometric surface");
      BRepAdaptor_Surface anAdaptor(aFace, true);
      const double aDomainUFirst = anAdaptor.FirstUParameter();
      const double aDomainULast = anAdaptor.LastUParameter();
      const double aDomainVFirst = anAdaptor.FirstVParameter();
      const double aDomainVLast = anAdaptor.LastVParameter();
      const double aTolerance = theArgs.value("tolerance", 1.0e-7);
      if (aTolerance <= 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface editing tolerance must be positive");

      if (theOp == "trimSurface")
      {
        const double aUFirst = requiredNumber(theArgs, "uFirst");
        const double aULast = requiredNumber(theArgs, "uLast");
        const double aVFirst = requiredNumber(theArgs, "vFirst");
        const double aVLast = requiredNumber(theArgs, "vLast");
        if (!std::isfinite(aUFirst) || !std::isfinite(aULast) || !std::isfinite(aVFirst)
            || !std::isfinite(aVLast) || aUFirst >= aULast || aVFirst >= aVLast
            || aUFirst < aDomainUFirst || aULast > aDomainULast
            || aVFirst < aDomainVFirst || aVLast > aDomainVLast)
          throw KernelFailure(ErrorCode::InvalidArgs, "Surface trim must be an increasing finite subset of the face UV domain");
        const TopoDS_Wire anOuter = BRepTools::OuterWire(aFace);
        int aWireCount = 0;
        int anOuterEdgeCount = 0;
        for (TopExp_Explorer anExplorer(aFace, TopAbs_WIRE);
             anExplorer.More(); anExplorer.Next())
        {
          ++aWireCount;
          if (TopoDS::Wire(anExplorer.Current()).IsSame(anOuter))
          {
            for (TopExp_Explorer anEdgeExplorer(anExplorer.Current(), TopAbs_EDGE);
                 anEdgeExplorer.More(); anEdgeExplorer.Next())
              ++anOuterEdgeCount;
          }
        }
        if (aWireCount != 1 || anOuter.IsNull() || anOuterEdgeCount != 4)
          throw KernelFailure(ErrorCode::InvalidArgs,
                              "Surface trim requires a single four-edge face boundary");
        const TopoDS_Face aTrimmedFace = makeRectangularFace(
          aSurface, aUFirst, aULast, aVFirst, aVLast, aTolerance, aFace.Orientation());
        return {{"shape", myArena.add(aTrimmedFace, aScope)}};
      }

      if (theOp == "updateSurfacePole")
      {
        const std::uint32_t anUIndex = requiredU32(theArgs, "uIndex");
        const std::uint32_t anVIndex = requiredU32(theArgs, "vIndex");
        const auto aPoint = requiredVec3(theArgs, "point");
        occ::handle<Geom_Surface> aCopySurface;
        if (aSurface->IsKind(STANDARD_TYPE(Geom_BezierSurface)))
        {
          const occ::handle<Geom_BezierSurface> aBezier =
            occ::down_cast<Geom_BezierSurface>(aSurface->Copy());
          if (anUIndex >= static_cast<std::uint32_t>(aBezier->NbUPoles())
              || anVIndex >= static_cast<std::uint32_t>(aBezier->NbVPoles()))
            throw KernelFailure(ErrorCode::InvalidArgs, "Surface pole index is out of range");
          if (theArgs.contains("weight"))
          {
            const double aWeight = requiredNumber(theArgs, "weight");
            if (aWeight <= 0.0) throw KernelFailure(ErrorCode::InvalidArgs, "Surface pole weight must be positive");
            aBezier->SetPole(static_cast<int>(anUIndex + 1), static_cast<int>(anVIndex + 1),
                             gp_Pnt(aPoint[0], aPoint[1], aPoint[2]), aWeight);
          }
          else
          {
            aBezier->SetPole(static_cast<int>(anUIndex + 1), static_cast<int>(anVIndex + 1),
                             gp_Pnt(aPoint[0], aPoint[1], aPoint[2]));
          }
          aCopySurface = aBezier;
        }
        else if (aSurface->IsKind(STANDARD_TYPE(Geom_BSplineSurface)))
        {
          const occ::handle<Geom_BSplineSurface> aBSpline =
            occ::down_cast<Geom_BSplineSurface>(aSurface->Copy());
          if (anUIndex >= static_cast<std::uint32_t>(aBSpline->NbUPoles())
              || anVIndex >= static_cast<std::uint32_t>(aBSpline->NbVPoles()))
            throw KernelFailure(ErrorCode::InvalidArgs, "Surface pole index is out of range");
          if (theArgs.contains("weight"))
          {
            const double aWeight = requiredNumber(theArgs, "weight");
            if (aWeight <= 0.0) throw KernelFailure(ErrorCode::InvalidArgs, "Surface pole weight must be positive");
            aBSpline->SetPole(static_cast<int>(anUIndex + 1), static_cast<int>(anVIndex + 1),
                              gp_Pnt(aPoint[0], aPoint[1], aPoint[2]), aWeight);
          }
          else
          {
            aBSpline->SetPole(static_cast<int>(anUIndex + 1), static_cast<int>(anVIndex + 1),
                              gp_Pnt(aPoint[0], aPoint[1], aPoint[2]));
          }
          aCopySurface = aBSpline;
        }
        else
        {
          throw KernelFailure(ErrorCode::InvalidArgs, "Surface pole editing is available only for Bezier and BSpline faces");
        }
        const TopoDS_Face aEditedFace = makeSurfaceFacePreservingWires(
          aFace, aCopySurface, aTolerance, aFace.Orientation());
        return {{"shape", myArena.add(aEditedFace, aScope)}};
      }

      occ::handle<Geom_BSplineSurface> aBSpline;
      if (aSurface->IsKind(STANDARD_TYPE(Geom_BSplineSurface)))
      {
        aBSpline = GeomConvert::SplitBSplineSurface(
          occ::down_cast<Geom_BSplineSurface>(aSurface), aDomainUFirst, aDomainULast,
          aDomainVFirst, aDomainVLast, aTolerance, true, true);
      }
      else
      {
        const occ::handle<Geom_Surface> aTrimmedSurface = new Geom_RectangularTrimmedSurface(
          aSurface, aDomainUFirst, aDomainULast, aDomainVFirst, aDomainVLast);
        aBSpline = GeomConvert::SurfaceToBSplineSurface(aTrimmedSurface);
      }
      if (aBSpline.IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to convert the surface to BSpline");
      const TopoDS_Face aConvertedFace = makeSurfaceFacePreservingWires(
        aFace, aBSpline, aTolerance, aFace.Orientation());
      return {{"shape", myArena.add(aConvertedFace, aScope)}};
    }
    if (theOp == "makeSolidFromShell")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShellShape = myArena.get(requiredU32(theArgs, "shell"));
      if (aShellShape.ShapeType() != TopAbs_SHELL)
        throw KernelFailure(ErrorCode::InvalidArgs, "makeSolidFromShell requires a shell");
      const TopoDS_Shell aShell = TopoDS::Shell(aShellShape);
      if (!BRep_Tool::IsClosed(aShell) || !BRepCheck_Analyzer(aShell, true).IsValid())
        throw KernelFailure(ErrorCode::InvalidArgs, "makeSolidFromShell requires a closed, valid shell");
      BRepBuilderAPI_MakeSolid aBuilder(aShell);
      if (!aBuilder.IsDone() || aBuilder.Solid().IsNull()
          || !BRepCheck_Analyzer(aBuilder.Solid(), true).IsValid())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct a valid solid from shell");
      return {{"shape", myArena.add(aBuilder.Solid(), aScope)}};
    }
    if (theOp == "extrude")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape aShape = myArena.get(requiredU32(theArgs, "shape"));
      const auto aVector = requiredVec3(theArgs, "vector");
      const double aSquaredLength = aVector[0] * aVector[0] + aVector[1] * aVector[1] + aVector[2] * aVector[2];
      if (aSquaredLength <= 1.0e-30)
        throw KernelFailure(ErrorCode::InvalidArgs, "Extrusion vector must be non-zero");
      BRepPrimAPI_MakePrism aBuilder(aShape, gp_Vec(aVector[0], aVector[1], aVector[2]), true);
      aBuilder.Build();
      if (!aBuilder.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to extrude shape");
      json aHistory;
      if (theArgs.value("includeHistory", false))
        aHistory = buildHistory(aBuilder, {aShape}, aBuilder.Shape());
      json aResult{{"shape", myArena.add(aBuilder.Shape(), aScope)}};
      if (theArgs.value("includeHistory", false))
        aResult["history"] = std::move(aHistory);
      return aResult;
    }
    if (theOp == "revolve")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape aShape = myArena.get(requiredU32(theArgs, "shape"));
      const auto anOrigin = requiredVec3(theArgs, "origin");
      const auto aDirection = requiredDirection(theArgs, "direction");
      const double anAngle = theArgs.value("angle", 2.0 * 3.14159265358979323846);
      if (anAngle <= 0.0 || anAngle > 2.0 * 3.14159265358979323846)
        throw KernelFailure(ErrorCode::InvalidArgs, "Revolve angle must be in (0, 2pi]");
      BRepPrimAPI_MakeRevol aBuilder(aShape,
                                     gp_Ax1(gp_Pnt(anOrigin[0], anOrigin[1], anOrigin[2]),
                                            gp_Dir(aDirection[0], aDirection[1], aDirection[2])), anAngle, true);
      aBuilder.Build();
      if (!aBuilder.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to revolve shape");
      json aHistory;
      if (theArgs.value("includeHistory", false))
        aHistory = buildHistory(aBuilder, {aShape}, aBuilder.Shape());
      json aResult{{"shape", myArena.add(aBuilder.Shape(), aScope)}};
      if (theArgs.value("includeHistory", false))
        aResult["history"] = std::move(aHistory);
      return aResult;
    }
    if (theOp == "localPrism")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aBase = myArena.get(requiredU32(theArgs, "base"));
      if (!theArgs.contains("faceIndices") || !theArgs.at("faceIndices").is_array()
          || theArgs.at("faceIndices").empty())
        throw KernelFailure(ErrorCode::InvalidArgs, "localPrism requires faceIndices");
      const auto aDirection = requiredDirection(theArgs, "direction");
      const std::string aMode = theArgs.value("mode", "length");
      if (aMode != "length" && aMode != "until" && aMode != "fromUntil"
          && aMode != "untilEnd" && aMode != "fromEnd" && aMode != "thruAll")
        throw KernelFailure(ErrorCode::InvalidArgs, "Unknown localPrism limit mode");
      const std::string anOperation = theArgs.value("operation", "add");
      if (anOperation != "add" && anOperation != "cut")
        throw KernelFailure(ErrorCode::InvalidArgs, "localPrism operation must be add or cut");

      TopTools_IndexedMapOfShape aFaces;
      TopExp::MapShapes(aBase, TopAbs_FACE, aFaces);
      std::vector<TopoDS_Face> aSelectedFaces;
      std::vector<bool> aSeen(static_cast<std::size_t>(aFaces.Extent()), false);
      for (const json& anIndexValue : theArgs.at("faceIndices"))
      {
        if (!anIndexValue.is_number_integer())
          throw KernelFailure(ErrorCode::InvalidArgs, "localPrism face indices must be integers");
        const std::int64_t anIndex = anIndexValue.get<std::int64_t>();
        if (anIndex < 0 || anIndex >= aFaces.Extent())
          throw KernelFailure(ErrorCode::InvalidArgs, "localPrism face index is out of range");
        if (aSeen[static_cast<std::size_t>(anIndex)])
          throw KernelFailure(ErrorCode::InvalidArgs, "localPrism face indices must be unique");
        aSeen[static_cast<std::size_t>(anIndex)] = true;
        aSelectedFaces.push_back(TopoDS::Face(aFaces(static_cast<int>(anIndex) + 1)));
      }

      TopoDS_Shape aProfile;
      TopoDS_Face aSketchFace;
      bool toModify = false;
      if (aSelectedFaces.size() == 1)
      {
        aProfile = aSelectedFaces.front();
        aSketchFace = aSelectedFaces.front();
        toModify = true;
      }
      else
      {
        BRep_Builder aShapeBuilder;
        TopoDS_Shell aShell;
        aShapeBuilder.MakeShell(aShell);
        for (const TopoDS_Face& aFace : aSelectedFaces) aShapeBuilder.Add(aShell, aFace);
        aShell.Closed(BRep_Tool::IsClosed(aShell));
        aProfile = aShell;
      }

      BRepFeat_MakePrism aBuilder;
      aBuilder.Init(aBase,
                    aProfile,
                    aSketchFace,
                    gp_Dir(aDirection[0], aDirection[1], aDirection[2]),
                    anOperation == "add" ? 1 : 0,
                    toModify);
      if (aMode == "length")
      {
        const double aLength = requiredNumber(theArgs, "length");
        if (aLength <= 0.0)
          throw KernelFailure(ErrorCode::InvalidArgs, "localPrism length must be positive");
        aBuilder.Perform(aLength);
      }
      else if (aMode == "until")
      {
        aBuilder.Perform(myArena.get(requiredU32(theArgs, "until")));
      }
      else if (aMode == "fromUntil")
      {
        aBuilder.Perform(myArena.get(requiredU32(theArgs, "from")),
                         myArena.get(requiredU32(theArgs, "until")));
      }
      else if (aMode == "untilEnd")
      {
        aBuilder.PerformUntilEnd();
      }
      else if (aMode == "fromEnd")
      {
        aBuilder.PerformFromEnd(myArena.get(requiredU32(theArgs, "from")));
      }
      else
      {
        aBuilder.PerformThruAll();
      }
      if (!aBuilder.IsDone() || aBuilder.Shape().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT local prism feature failed");
      return {{"shape", myArena.add(aBuilder.Shape(), aScope)}};
    }
    if (theOp == "localRevolution")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aBase = myArena.get(requiredU32(theArgs, "base"));
      if (!theArgs.contains("faceIndices") || !theArgs.at("faceIndices").is_array()
          || theArgs.at("faceIndices").empty())
        throw KernelFailure(ErrorCode::InvalidArgs, "localRevolution requires faceIndices");
      const auto anOrigin = requiredVec3(theArgs, "origin");
      const auto aDirection = requiredDirection(theArgs, "direction");
      const std::string aMode = theArgs.value("mode", "angle");
      if (aMode != "angle" && aMode != "until" && aMode != "fromUntil" && aMode != "thruAll")
        throw KernelFailure(ErrorCode::InvalidArgs, "Unknown localRevolution limit mode");
      const std::string anOperation = theArgs.value("operation", "add");
      if (anOperation != "add" && anOperation != "cut")
        throw KernelFailure(ErrorCode::InvalidArgs, "localRevolution operation must be add or cut");

      TopTools_IndexedMapOfShape aFaces;
      TopExp::MapShapes(aBase, TopAbs_FACE, aFaces);
      std::vector<TopoDS_Face> aSelectedFaces;
      std::vector<bool> aSeen(static_cast<std::size_t>(aFaces.Extent()), false);
      for (const json& anIndexValue : theArgs.at("faceIndices"))
      {
        if (!anIndexValue.is_number_integer())
          throw KernelFailure(ErrorCode::InvalidArgs, "localRevolution face indices must be integers");
        const std::int64_t anIndex = anIndexValue.get<std::int64_t>();
        if (anIndex < 0 || anIndex >= aFaces.Extent())
          throw KernelFailure(ErrorCode::InvalidArgs, "localRevolution face index is out of range");
        if (aSeen[static_cast<std::size_t>(anIndex)])
          throw KernelFailure(ErrorCode::InvalidArgs, "localRevolution face indices must be unique");
        aSeen[static_cast<std::size_t>(anIndex)] = true;
        aSelectedFaces.push_back(TopoDS::Face(aFaces(static_cast<int>(anIndex) + 1)));
      }

      TopoDS_Shape aProfile;
      TopoDS_Face aSketchFace;
      bool toModify = false;
      if (aSelectedFaces.size() == 1)
      {
        aProfile = aSelectedFaces.front();
        aSketchFace = aSelectedFaces.front();
        toModify = true;
      }
      else
      {
        BRep_Builder aShapeBuilder;
        TopoDS_Shell aShell;
        aShapeBuilder.MakeShell(aShell);
        for (const TopoDS_Face& aFace : aSelectedFaces) aShapeBuilder.Add(aShell, aFace);
        aShell.Closed(BRep_Tool::IsClosed(aShell));
        aProfile = aShell;
      }

      BRepFeat_MakeRevol aBuilder;
      aBuilder.Init(aBase, aProfile, aSketchFace,
                    gp_Ax1(gp_Pnt(anOrigin[0], anOrigin[1], anOrigin[2]),
                           gp_Dir(aDirection[0], aDirection[1], aDirection[2])),
                    anOperation == "add" ? 1 : 0, toModify);
      if (aMode == "angle")
      {
        const double anAngle = requiredNumber(theArgs, "angle");
        if (!std::isfinite(anAngle) || anAngle == 0.0 || std::abs(anAngle) > 2.0 * 3.14159265358979323846)
          throw KernelFailure(ErrorCode::InvalidArgs, "localRevolution angle must be non-zero and within +/-2pi");
        aBuilder.Perform(anAngle);
      }
      else if (aMode == "until")
      {
        aBuilder.Perform(myArena.get(requiredU32(theArgs, "until")));
      }
      else if (aMode == "fromUntil")
      {
        aBuilder.Perform(myArena.get(requiredU32(theArgs, "from")),
                         myArena.get(requiredU32(theArgs, "until")));
      }
      else
      {
        aBuilder.PerformThruAll();
      }
      if (!aBuilder.IsDone() || aBuilder.Shape().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT local revolution feature failed");
      return {{"shape", myArena.add(aBuilder.Shape(), aScope)}};
    }
    if (theOp == "linearForm")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aBase = myArena.get(requiredU32(theArgs, "base"));
      const TopoDS_Shape& aProfileShape = myArena.get(requiredU32(theArgs, "profile"));
      if (aProfileShape.ShapeType() != TopAbs_WIRE)
        throw KernelFailure(ErrorCode::InvalidArgs, "linearForm profile must be a wire");

      const auto aPlaneOrigin = requiredVec3(theArgs, "planeOrigin");
      const auto aPlaneNormal = requiredDirection(theArgs, "planeNormal");
      const auto aDirection = requiredVec3(theArgs, "direction");
      const auto aDirection1 = optionalVec3(theArgs, "direction1", {0.0, 0.0, 0.0});
      const gp_Vec aFirstDirection(aDirection[0], aDirection[1], aDirection[2]);
      if (aFirstDirection.Magnitude() <= Precision::Confusion())
        throw KernelFailure(ErrorCode::InvalidArgs, "linearForm direction must be non-zero");
      const gp_Vec aSecondDirection(aDirection1[0], aDirection1[1], aDirection1[2]);
      if (aSecondDirection.Magnitude() > Precision::Confusion())
      {
        const double aFirstMagnitude = aFirstDirection.Magnitude();
        const double aSecondMagnitude = aSecondDirection.Magnitude();
        const double aCosine = aFirstDirection.Dot(aSecondDirection)
                             / (aFirstMagnitude * aSecondMagnitude);
        const double aCrossMagnitude = aFirstDirection.Crossed(aSecondDirection).Magnitude()
                                     / (aFirstMagnitude * aSecondMagnitude);
        if (aCosine >= -1.0 + 1e-9 || aCrossMagnitude > 1e-9)
          throw KernelFailure(ErrorCode::InvalidArgs,
                              "linearForm direction1 must be zero or opposite to direction");
      }

      const std::string anOperation = theArgs.value("operation", "add");
      if (anOperation != "add" && anOperation != "cut")
        throw KernelFailure(ErrorCode::InvalidArgs, "linearForm operation must be add or cut");
      const occ::handle<Geom_Plane> aPlane = new Geom_Plane(
        gp_Pln(gp_Pnt(aPlaneOrigin[0], aPlaneOrigin[1], aPlaneOrigin[2]),
               gp_Dir(aPlaneNormal[0], aPlaneNormal[1], aPlaneNormal[2])));

      BRepFeat_MakeLinearForm aBuilder;
      aBuilder.Init(aBase, TopoDS::Wire(aProfileShape), aPlane, aFirstDirection,
                    aSecondDirection,
                    anOperation == "add" ? 1 : 0, theArgs.value("modify", true));
      if (!aBuilder.IsDone())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to initialize linearForm");

      aBuilder.Perform();
      if (!aBuilder.IsDone() || aBuilder.Shape().IsNull()
          || !BRepCheck_Analyzer(aBuilder.Shape(), true).IsValid())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT linearForm failed");
      return {{"shape", myArena.add(aBuilder.Shape(), aScope)}};
    }
    if (theOp == "revolutionForm")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aBase = myArena.get(requiredU32(theArgs, "base"));
      const TopoDS_Shape& aProfileShape = myArena.get(requiredU32(theArgs, "profile"));
      if (aProfileShape.ShapeType() != TopAbs_WIRE)
        throw KernelFailure(ErrorCode::InvalidArgs, "revolutionForm profile must be a wire");

      const auto aPlaneOrigin = requiredVec3(theArgs, "planeOrigin");
      const auto aPlaneNormal = requiredDirection(theArgs, "planeNormal");
      const auto anOrigin = requiredVec3(theArgs, "origin");
      const auto aDirection = requiredDirection(theArgs, "direction");
      const double aHeight1 = requiredNumber(theArgs, "height1");
      const double aHeight2 = requiredNumber(theArgs, "height2");
      if (!std::isfinite(aHeight1) || !std::isfinite(aHeight2)
          || aHeight1 < 0.0 || aHeight2 < 0.0 || (aHeight1 == 0.0 && aHeight2 == 0.0))
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "revolutionForm heights must be non-negative and not both zero");
      const std::string anOperation = theArgs.value("operation", "add");
      if (anOperation != "add" && anOperation != "cut")
        throw KernelFailure(ErrorCode::InvalidArgs, "revolutionForm operation must be add or cut");

      const occ::handle<Geom_Plane> aPlane = new Geom_Plane(
        gp_Pln(gp_Pnt(aPlaneOrigin[0], aPlaneOrigin[1], aPlaneOrigin[2]),
               gp_Dir(aPlaneNormal[0], aPlaneNormal[1], aPlaneNormal[2])));
      bool toModify = theArgs.value("modify", true);
      BRepFeat_MakeRevolutionForm aBuilder;
      aBuilder.Init(aBase,
                    TopoDS::Wire(aProfileShape),
                    aPlane,
                    gp_Ax1(gp_Pnt(anOrigin[0], anOrigin[1], anOrigin[2]),
                           gp_Dir(aDirection[0], aDirection[1], aDirection[2])),
                    aHeight1,
                    aHeight2,
                    anOperation == "add" ? 1 : 0,
                    toModify);
      if (!aBuilder.IsDone())
        throw KernelFailure(ErrorCode::ConstructionFailed,
                            "OCCT failed to initialize revolutionForm");

      aBuilder.Perform();
      if (!aBuilder.IsDone() || aBuilder.Shape().IsNull()
          || !BRepCheck_Analyzer(aBuilder.Shape(), true).IsValid())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT revolutionForm failed");
      return {{"shape", myArena.add(aBuilder.Shape(), aScope)}};
    }
    if (theOp == "cylindricalHole")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
      const auto anOrigin = requiredVec3(theArgs, "origin");
      const auto aDirection = requiredDirection(theArgs, "direction");
      const double aRadius = requiredNumber(theArgs, "radius");
      if (aRadius <= 0.0 || !std::isfinite(aRadius))
        throw KernelFailure(ErrorCode::InvalidArgs, "Cylindrical hole radius must be positive");
      const gp_Pnt anAxisOrigin(anOrigin[0], anOrigin[1], anOrigin[2]);
      const gp_Dir anAxisDirection(aDirection[0], aDirection[1], aDirection[2]);
      const gp_Lin anAxis(anAxisOrigin, anAxisDirection);

      Bnd_Box aBounds;
      BRepBndLib::Add(aShape, aBounds);
      if (aBounds.IsVoid())
        throw KernelFailure(ErrorCode::InvalidArgs, "Cylindrical hole requires a bounded shape");
      double aXMin = 0.0;
      double aYMin = 0.0;
      double aZMin = 0.0;
      double aXMax = 0.0;
      double aYMax = 0.0;
      double aZMax = 0.0;
      aBounds.Get(aXMin, aYMin, aZMin, aXMax, aYMax, aZMax);
      double aShapeFirst = std::numeric_limits<double>::max();
      double aShapeLast = std::numeric_limits<double>::lowest();
      for (const double anX : {aXMin, aXMax})
        for (const double aY : {aYMin, aYMax})
          for (const double aZ : {aZMin, aZMax})
          {
            const double aParameter = gp_Vec(anAxisOrigin, gp_Pnt(anX, aY, aZ))
                                        .Dot(gp_Vec(anAxisDirection));
            aShapeFirst = std::min(aShapeFirst, aParameter);
            aShapeLast = std::max(aShapeLast, aParameter);
          }
      const double aShapeSpan = std::max({aShapeLast - aShapeFirst, aRadius * 2.0, 1.0});
      const double aPad = std::max(aShapeSpan * 1.0e-7, 1.0e-6);
      double aToolFirst = 0.0;
      double aToolLast = 0.0;
      const std::string aMode = theArgs.value("mode", "throughAll");
      const bool withControl = theArgs.value("withControl", true);
      if (aMode == "throughAll")
      {
        aToolFirst = aShapeFirst - aShapeSpan;
        aToolLast = aShapeLast + aShapeSpan;
      }
      else if (aMode == "throughNext" || aMode == "untilEnd")
      {
        IntCurvesFace_ShapeIntersector anIntersector;
        anIntersector.Load(aShape, 1.0e-7);
        anIntersector.Perform(anAxis, aShapeFirst - aShapeSpan, aShapeLast + aShapeSpan);
        if (!anIntersector.IsDone())
          throw KernelFailure(ErrorCode::ConstructionFailed,
                              "OCCT failed to intersect the cylindrical hole axis");
        anIntersector.SortResult();
        int aFirstForward = 0;
        for (int anIndex = 1; anIndex <= anIntersector.NbPnt(); ++anIndex)
          if (anIntersector.Transition(anIndex) != IntCurveSurface_Tangent
              && anIntersector.WParameter(anIndex) >= -aPad)
          {
            aFirstForward = anIndex;
            break;
          }
        if (aFirstForward == 0)
          throw KernelFailure(ErrorCode::ConstructionFailed,
                              "Cylindrical hole axis does not enter the shape after its origin");
        int anEntry = 0;
        int anExit = 0;
        if (anIntersector.Transition(aFirstForward) == IntCurveSurface_In)
        {
          anEntry = aFirstForward;
        }
        else
        {
          for (int anIndex = aFirstForward - 1; anIndex >= 1; --anIndex)
            if (anIntersector.Transition(anIndex) == IntCurveSurface_In)
            {
              anEntry = anIndex;
              break;
            }
        }
        if (anEntry == 0)
          throw KernelFailure(ErrorCode::ConstructionFailed,
                              "Cylindrical hole axis has no entry intersection");
        for (int anIndex = anEntry + 1; anIndex <= anIntersector.NbPnt(); ++anIndex)
          if (anIntersector.Transition(anIndex) == IntCurveSurface_Out)
          {
            anExit = anIndex;
            if (aMode == "throughNext") break;
          }
        if (anExit == 0)
          throw KernelFailure(ErrorCode::ConstructionFailed,
                              "Cylindrical hole axis has no exit intersection");
        aToolFirst = anIntersector.WParameter(anEntry) - aPad;
        aToolLast = anIntersector.WParameter(anExit) + aPad;
      }
      else if (aMode == "blind")
      {
        const double aLength = requiredNumber(theArgs, "length");
        if (aLength <= 0.0 || !std::isfinite(aLength))
          throw KernelFailure(ErrorCode::InvalidArgs, "Blind hole length must be positive");
        aToolFirst = -aPad;
        aToolLast = aLength;
      }
      else if (aMode == "between")
      {
        const double aFrom = requiredNumber(theArgs, "from");
        const double aTo = requiredNumber(theArgs, "to");
        if (aFrom == aTo || !std::isfinite(aFrom) || !std::isfinite(aTo))
          throw KernelFailure(ErrorCode::InvalidArgs, "Cylindrical hole interval must have non-zero length");
        aToolFirst = std::min(aFrom, aTo);
        aToolLast = std::max(aFrom, aTo);
      }
      else
      {
        throw KernelFailure(ErrorCode::InvalidArgs, "Unknown cylindrical hole mode");
      }
      const gp_Pnt aToolOrigin = anAxisOrigin.Translated(gp_Vec(anAxisDirection) * aToolFirst);
      BRepPrimAPI_MakeCylinder aToolBuilder(
        gp_Ax2(aToolOrigin, anAxisDirection), aRadius, aToolLast - aToolFirst);
      aToolBuilder.Build();
      if (!aToolBuilder.IsDone() || aToolBuilder.Shape().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct the cylindrical hole tool");
      NCollection_List<TopoDS_Shape> anArguments;
      NCollection_List<TopoDS_Shape> aTools;
      anArguments.Append(aShape);
      aTools.Append(aToolBuilder.Shape());
      BRepAlgoAPI_Cut aBuilder;
      aBuilder.SetArguments(anArguments);
      aBuilder.SetTools(aTools);
      aBuilder.SetNonDestructive(true);
      aBuilder.SetRunParallel(false);
      aBuilder.SetToFillHistory(theArgs.value("includeHistory", false));
      aBuilder.Build();
      if (!aBuilder.IsDone() || aBuilder.HasErrors() || aBuilder.Shape().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT cylindrical hole feature failed");
      const TopoDS_Shape anOutput = aBuilder.Shape();
      json aHistory;
      if (theArgs.value("includeHistory", false))
        aHistory = buildHistory(aBuilder, {aShape}, anOutput);
      if (withControl && !BRepCheck_Analyzer(anOutput, true).IsValid())
        throw KernelFailure(ErrorCode::ConstructionFailed,
                            "OCCT cylindrical hole feature failed validation");
      json aResult{{"shape", myArena.add(anOutput, aScope)}};
      if (theArgs.value("includeHistory", false))
        aResult["history"] = std::move(aHistory);
      return aResult;
    }
    if (theOp == "defeature")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
      if (aShape.ShapeType() != TopAbs_SOLID && aShape.ShapeType() != TopAbs_COMPSOLID
          && aShape.ShapeType() != TopAbs_COMPOUND)
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "Defeaturing requires a solid, compsolid, or compound of solids");
      if (!theArgs.contains("faceIndices") || !theArgs.at("faceIndices").is_array()
          || theArgs.at("faceIndices").empty())
        throw KernelFailure(ErrorCode::InvalidArgs, "Defeaturing requires faceIndices");
      TopTools_IndexedMapOfShape aFaces;
      TopExp::MapShapes(aShape, TopAbs_FACE, aFaces);
      std::vector<bool> aSeen(static_cast<std::size_t>(aFaces.Extent()), false);
      BRepAlgoAPI_Defeaturing aBuilder;
      aBuilder.SetShape(aShape);
      aBuilder.SetRunParallel(false);
      aBuilder.SetToFillHistory(theArgs.value("includeHistory", false));
      for (const json& anIndexValue : theArgs.at("faceIndices"))
      {
        if (!anIndexValue.is_number_integer())
          throw KernelFailure(ErrorCode::InvalidArgs, "Defeaturing face indices must be integers");
        const std::int64_t anIndex = anIndexValue.get<std::int64_t>();
        if (anIndex < 0 || anIndex >= aFaces.Extent())
          throw KernelFailure(ErrorCode::InvalidArgs, "Defeaturing face index is out of range");
        if (aSeen[static_cast<std::size_t>(anIndex)])
          throw KernelFailure(ErrorCode::InvalidArgs, "Defeaturing face indices must be unique");
        aSeen[static_cast<std::size_t>(anIndex)] = true;
        aBuilder.AddFaceToRemove(aFaces(static_cast<int>(anIndex) + 1));
      }
      aBuilder.Build();
      if (!aBuilder.IsDone() || aBuilder.HasErrors() || aBuilder.Shape().IsNull())
        throw KernelFailure(ErrorCode::BooleanFailed, "OCCT defeaturing failed");
      json aHistory;
      if (theArgs.value("includeHistory", false))
        aHistory = buildHistory(aBuilder, {aShape}, aBuilder.Shape());
      json aResult{{"shape", myArena.add(aBuilder.Shape(), aScope)}};
      if (theArgs.value("includeHistory", false))
        aResult["history"] = std::move(aHistory);
      return aResult;
    }
    if (theOp == "loft")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      if (!theArgs.contains("sections") || !theArgs.at("sections").is_array() || theArgs.at("sections").size() < 2)
        throw KernelFailure(ErrorCode::InvalidArgs, "loft requires at least two wire sections");
      BRepOffsetAPI_ThruSections aBuilder(theArgs.value("solid", false), theArgs.value("ruled", false), theArgs.value("precision", 1.0e-6));
      aBuilder.CheckCompatibility(true);
      std::vector<TopoDS_Shape> aSections;
      for (const json& aSection : theArgs.at("sections"))
      {
        aSections.push_back(myArena.get(aSection.get<std::uint32_t>()));
        aBuilder.AddWire(TopoDS::Wire(aSections.back()));
      }
      aBuilder.Build();
      if (!aBuilder.IsDone() || aBuilder.Shape().IsNull()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT loft failed");
      json aHistory;
      if (theArgs.value("includeHistory", false))
        aHistory = buildHistory(aBuilder, aSections, aBuilder.Shape());
      json aResult{{"shape", myArena.add(aBuilder.Shape(), aScope)}};
      if (theArgs.value("includeHistory", false))
        aResult["history"] = std::move(aHistory);
      return aResult;
    }
    if (theOp == "sweepPipe")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape aSpine = myArena.get(requiredU32(theArgs, "spine"));
      const TopoDS_Shape aProfile = myArena.get(requiredU32(theArgs, "profile"));
      BRepOffsetAPI_MakePipe aBuilder(TopoDS::Wire(aSpine), aProfile);
      aBuilder.Build();
      if (!aBuilder.IsDone() || aBuilder.Shape().IsNull()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT pipe sweep failed");
      json aHistory;
      if (theArgs.value("includeHistory", false))
        aHistory = buildHistory(aBuilder, {aSpine, aProfile}, aBuilder.Shape());
      json aResult{{"shape", myArena.add(aBuilder.Shape(), aScope)}};
      if (theArgs.value("includeHistory", false))
        aResult["history"] = std::move(aHistory);
      return aResult;
    }
    if (theOp == "middlePath")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
      const TopoDS_Shape& aStart = myArena.get(requiredU32(theArgs, "start"));
      const TopoDS_Shape& anEnd = myArena.get(requiredU32(theArgs, "end"));
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
      return {{"shape", myArena.add(aBuilder.Shape(), aScope)}};
    }
    if (theOp == "sweepPipeShell")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape aSpine = myArena.get(requiredU32(theArgs, "spine"));
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
        const TopoDS_Shape anAuxiliarySpineShape = myArena.get(requiredU32(theArgs, "auxiliarySpine"));
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
        aProfiles.push_back(myArena.get(aProfile.get<std::uint32_t>()));
        aBuilder.Add(aProfiles.back());
      }
      aHistoryInputs.insert(aHistoryInputs.begin() + 1, aProfiles.begin(), aProfiles.end());
      aBuilder.Build();
      if (!aBuilder.IsDone() || aBuilder.Shape().IsNull()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT pipe-shell sweep failed");
      json aHistory;
      if (theArgs.value("includeHistory", false))
        aHistory = buildHistory(aBuilder, aHistoryInputs, aBuilder.Shape());
      json aResult{{"shape", myArena.add(aBuilder.Shape(), aScope)}};
      if (theArgs.value("includeHistory", false))
        aResult["history"] = std::move(aHistory);
      return aResult;
    }
    if (theOp == "fillet" || theOp == "chamfer")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape aShape = myArena.get(requiredU32(theArgs, "shape"));
      if (!theArgs.contains("edgeIndices") || !theArgs.at("edgeIndices").is_array() || theArgs.at("edgeIndices").empty())
        throw KernelFailure(ErrorCode::InvalidArgs, theOp + " requires edgeIndices");
      TopTools_IndexedMapOfShape anEdges;
      TopExp::MapShapes(aShape, TopAbs_EDGE, anEdges);
      const double aDistance = requiredNumber(theArgs, theOp == "fillet" ? "radius" : "distance");
      if (aDistance <= 0.0) throw KernelFailure(ErrorCode::InvalidArgs, "Fillet/chamfer size must be positive");
      if (theOp == "fillet")
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
        json aResult{{"shape", myArena.add(aBuilder.Shape(), aScope)}};
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
      json aResult{{"shape", myArena.add(aBuilder.Shape(), aScope)}};
      if (theArgs.value("includeHistory", false))
        aResult["history"] = buildFilletHistory(aBuilder, aShape, aBuilder.Shape());
      return aResult;
    }
    if (theOp == "draftAngle")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
      if (!theArgs.contains("faceIndices") || !theArgs.at("faceIndices").is_array()
          || theArgs.at("faceIndices").empty())
        throw KernelFailure(ErrorCode::InvalidArgs, "draftAngle requires faceIndices");
      if (!theArgs.contains("neutralPlane") || !theArgs.at("neutralPlane").is_object())
        throw KernelFailure(ErrorCode::InvalidArgs, "draftAngle requires neutralPlane");

      const auto aDirection = requiredDirection(theArgs, "direction");
      const double anAngle = requiredNumber(theArgs, "angle");
      if (std::abs(anAngle) <= 1.0e-4)
        throw KernelFailure(ErrorCode::InvalidArgs, "draftAngle angle magnitude must exceed 1e-4 radians");
      const json& aPlaneValue = theArgs.at("neutralPlane");
      const auto aPlaneOrigin = requiredVec3(aPlaneValue, "origin");
      const auto aPlaneNormal = requiredDirection(aPlaneValue, "normal");

      TopTools_IndexedMapOfShape aFaces;
      TopExp::MapShapes(aShape, TopAbs_FACE, aFaces);
      std::vector<bool> aSeen(static_cast<std::size_t>(aFaces.Extent()), false);
      BRepOffsetAPI_DraftAngle aBuilder(aShape);
      for (const json& anIndexValue : theArgs.at("faceIndices"))
      {
        if (!anIndexValue.is_number_integer())
          throw KernelFailure(ErrorCode::InvalidArgs, "draftAngle face indices must be integers");
        const std::int64_t anIndex = anIndexValue.get<std::int64_t>();
        if (anIndex < 0 || anIndex >= aFaces.Extent())
          throw KernelFailure(ErrorCode::InvalidArgs, "draftAngle face index is out of range");
        if (aSeen[static_cast<std::size_t>(anIndex)])
          throw KernelFailure(ErrorCode::InvalidArgs, "draftAngle face indices must be unique");
        aSeen[static_cast<std::size_t>(anIndex)] = true;
        aBuilder.Add(TopoDS::Face(aFaces(static_cast<int>(anIndex) + 1)),
                     gp_Dir(aDirection[0], aDirection[1], aDirection[2]),
                     anAngle,
                     gp_Pln(gp_Pnt(aPlaneOrigin[0], aPlaneOrigin[1], aPlaneOrigin[2]),
                            gp_Dir(aPlaneNormal[0], aPlaneNormal[1], aPlaneNormal[2])),
                     theArgs.value("flag", true));
        if (!aBuilder.AddDone())
          throw KernelFailure(ErrorCode::ConstructionFailed,
                              "OCCT draft failed while adding face index " + std::to_string(anIndex));
      }
      aBuilder.Build();
      if (!aBuilder.IsDone() || aBuilder.Shape().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT draft build failed");
      return {{"shape", myArena.add(aBuilder.Shape(), aScope)}};
    }
    if (theOp == "hollow")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const double anOffset = requiredNumber(theArgs, "thickness");
      if (anOffset == 0.0) throw KernelFailure(ErrorCode::InvalidArgs, "Hollow thickness cannot be zero");
      const double aTolerance = theArgs.value("tolerance", 1.0e-6);
      if (aTolerance <= 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs, "Hollow tolerance must be positive");
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
      BRepOffsetAPI_MakeThickSolid aBuilder;
      NCollection_List<TopoDS_Shape> aClosingFaces;
      if (theArgs.contains("closingFaceIndices"))
      {
        TopTools_IndexedMapOfShape aFaces;
        TopExp::MapShapes(aShape, TopAbs_FACE, aFaces);
        for (const json& anIndex : theArgs.at("closingFaceIndices"))
        {
          const std::int64_t aFaceIndex = anIndex.get<std::int64_t>();
          if (aFaceIndex < 0 || aFaceIndex >= aFaces.Extent()) throw KernelFailure(ErrorCode::InvalidArgs, "Hollow face index out of range");
          aClosingFaces.Append(aFaces(static_cast<int>(aFaceIndex) + 1));
        }
      }
      aBuilder.MakeThickSolidByJoin(aShape, aClosingFaces, anOffset, aTolerance);
      if (!aBuilder.IsDone() || aBuilder.Shape().IsNull()) throw KernelFailure(ErrorCode::HealingFailed, "OCCT hollow failed");
      return {{"shape", myArena.add(aBuilder.Shape(), aScope)}};
    }
    if (theOp == "offsetShape")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const double anOffset = requiredNumber(theArgs, "offset");
      if (anOffset == 0.0) throw KernelFailure(ErrorCode::InvalidArgs, "Offset cannot be zero");
      BRepOffsetAPI_MakeOffsetShape aBuilder;
      aBuilder.PerformByJoin(myArena.get(requiredU32(theArgs, "shape")), anOffset, theArgs.value("tolerance", 1.0e-6));
      if (!aBuilder.IsDone() || aBuilder.Shape().IsNull()) throw KernelFailure(ErrorCode::HealingFailed, "OCCT offset failed");
      return {{"shape", myArena.add(aBuilder.Shape(), aScope)}};
    }
    if (theOp == "offsetWire2D")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const double anOffset = requiredNumber(theArgs, "offset");
      if (anOffset == 0.0) throw KernelFailure(ErrorCode::InvalidArgs, "Wire offset cannot be zero");
      BRepOffsetAPI_MakeOffset aBuilder(TopoDS::Wire(myArena.get(requiredU32(theArgs, "wire"))));
      aBuilder.Perform(anOffset);
      if (!aBuilder.IsDone() || aBuilder.Shape().IsNull()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT wire offset failed");
      return {{"shape", myArena.add(aBuilder.Shape(), aScope)}};
    }
    if (theOp == "booleanFuse" || theOp == "booleanCommon")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aBase = myArena.get(requiredU32(theArgs, "base"));
      if (!theArgs.contains("tools") || !theArgs.at("tools").is_array() || theArgs.at("tools").empty())
        throw KernelFailure(ErrorCode::InvalidArgs, "Boolean operation requires at least one tool");
      NCollection_List<TopoDS_Shape> anArguments;
      NCollection_List<TopoDS_Shape> aTools;
      anArguments.Append(aBase);
      std::vector<TopoDS_Shape> anInputs{aBase};
      for (const json& aValue : theArgs.at("tools"))
      {
        const TopoDS_Shape& aTool = myArena.get(aValue.get<std::uint32_t>());
        aTools.Append(aTool);
        anInputs.push_back(aTool);
      }
      if (theOp == "booleanFuse")
      {
        BRepAlgoAPI_Fuse aBuilder;
        aBuilder.SetArguments(anArguments); aBuilder.SetTools(aTools); aBuilder.SetNonDestructive(true);
        aBuilder.SetRunParallel(false); aBuilder.SetToFillHistory(theArgs.value("includeHistory", false)); aBuilder.Build();
        if (!aBuilder.IsDone() || aBuilder.HasErrors()) throw KernelFailure(ErrorCode::BooleanFailed, "OCCT boolean fuse failed");
        json aResult{{"shape", myArena.add(aBuilder.Shape(), aScope)}};
        if (theArgs.value("includeHistory", false)) aResult["history"] = buildHistory(aBuilder, anInputs, aBuilder.Shape());
        return aResult;
      }
      BRepAlgoAPI_Common aBuilder;
      aBuilder.SetArguments(anArguments); aBuilder.SetTools(aTools); aBuilder.SetNonDestructive(true);
      aBuilder.SetRunParallel(false); aBuilder.SetToFillHistory(theArgs.value("includeHistory", false)); aBuilder.Build();
      if (!aBuilder.IsDone() || aBuilder.HasErrors()) throw KernelFailure(ErrorCode::BooleanFailed, "OCCT boolean common failed");
      json aResult{{"shape", myArena.add(aBuilder.Shape(), aScope)}};
      if (theArgs.value("includeHistory", false)) aResult["history"] = buildHistory(aBuilder, anInputs, aBuilder.Shape());
      return aResult;
    }
    if (theOp == "booleanCut")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aBase = myArena.get(requiredU32(theArgs, "base"));
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
        const TopoDS_Shape& aTool = myArena.get(aValue.get<std::uint32_t>());
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
      json aResult{{"shape", myArena.add(aBuilder.Shape(), aScope)}};
      if (theArgs.value("includeHistory", false))
      {
        aResult["history"] = buildHistory(aBuilder, anInputs, aBuilder.Shape());
      }
      return aResult;
    }
    if (theOp == "section")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      BRepAlgoAPI_Section aBuilder(myArena.get(requiredU32(theArgs, "first")),
                                   myArena.get(requiredU32(theArgs, "second")), false);
      aBuilder.SetNonDestructive(true);
      aBuilder.SetRunParallel(false);
      aBuilder.Build();
      if (!aBuilder.IsDone() || aBuilder.Shape().IsNull()) throw KernelFailure(ErrorCode::BooleanFailed, "OCCT section failed");
      return {{"shape", myArena.add(aBuilder.Shape(), aScope)}};
    }
    if (theOp == "projectHLR")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
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
      return {{"visible", myArena.add(aVisible, aScope)},
              {"hidden", myArena.add(aHidden, aScope)}};
    }
    if (theOp == "sectionAnalysis")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aFirst = myArena.get(requiredU32(theArgs, "first"));
      const TopoDS_Shape& aSecond = myArena.get(requiredU32(theArgs, "second"));
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
      return {{"shape", myArena.add(aBuilder.Shape(), aScope)},
              {"edges", std::move(anEdgeResults)},
              {"standaloneVertices", std::move(aStandaloneVertices)}};
    }
    if (theOp == "split")
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
        aBuilder.AddArgument(myArena.get(static_cast<std::uint32_t>(aHandle)));
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
          aBuilder.AddTool(myArena.get(static_cast<std::uint32_t>(aHandle)));
        }
      }
      aBuilder.Perform();
      if (aBuilder.HasErrors() || aBuilder.Shape().IsNull())
        throw KernelFailure(ErrorCode::BooleanFailed, "OCCT splitter failed");
      return {{"shape", myArena.add(aBuilder.Shape(), aScope)}};
    }
    if (theOp == "glue")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aNewShape = myArena.get(requiredU32(theArgs, "newShape"));
      const TopoDS_Shape& aBaseShape = myArena.get(requiredU32(theArgs, "baseShape"));
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
      return {{"shape", myArena.add(aBuilder.Shape(), aScope)}};
    }
    if (theOp == "generalFuse")
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
        const TopoDS_Shape& anInput = myArena.get(static_cast<std::uint32_t>(aHandle));
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
        aShapes.push_back(myArena.add(aCells[aCellIndex], aScope));
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
    if (theOp == "selectGeneralFuseCells")
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
        const TopoDS_Shape& anInput = myArena.get(static_cast<std::uint32_t>(aHandle));
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
      return {{"shape", myArena.add(aBuilder.Shape(), aScope)}};
    }
    if (theOp == "translate" || theOp == "rotate" || theOp == "scale" || theOp == "mirror")
    {
      json aTransformArgs = theArgs;
      if (theOp == "translate") aTransformArgs["translation"] = requiredVec3(theArgs, "translation");
      else if (theOp == "rotate") aTransformArgs["rotation"] = theArgs.at("rotation");
      else if (theOp == "scale")
      {
        aTransformArgs["scale"] = requiredNumber(theArgs, "factor");
        if (theArgs.contains("origin")) aTransformArgs["scaleOrigin"] = theArgs.at("origin");
      }
      else aTransformArgs["mirror"] = requiredVec3(theArgs, "normal");
      return dispatch("transform", aTransformArgs);
    }
    if (theOp == "transform")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape aShape = myArena.get(requiredU32(theArgs, "shape"));
      gp_Trsf aTrsf;
      if (theArgs.contains("matrix"))
      {
        const json& aMatrix = theArgs.at("matrix");
        if (!aMatrix.is_array() || aMatrix.size() != 12) throw KernelFailure(ErrorCode::InvalidArgs, "transform matrix must contain 12 numbers");
        aTrsf.SetValues(aMatrix[0].get<double>(), aMatrix[1].get<double>(), aMatrix[2].get<double>(), aMatrix[3].get<double>(),
                        aMatrix[4].get<double>(), aMatrix[5].get<double>(), aMatrix[6].get<double>(), aMatrix[7].get<double>(),
                        aMatrix[8].get<double>(), aMatrix[9].get<double>(), aMatrix[10].get<double>(), aMatrix[11].get<double>());
      }
      else
      {
        if (theArgs.contains("translation"))
        {
          const auto aTranslation = requiredVec3(theArgs, "translation");
          gp_Trsf aPart;
          aPart.SetTranslationPart(gp_Vec(aTranslation[0], aTranslation[1], aTranslation[2]));
          aTrsf.Multiply(aPart);
        }
        if (theArgs.contains("rotation"))
        {
          const json& aRotation = theArgs.at("rotation");
          if (!aRotation.is_object()) throw KernelFailure(ErrorCode::InvalidArgs, "rotation must be an object");
          const auto anOrigin = aRotation.contains("origin") ? requiredVec3(aRotation, "origin") : std::array<double, 3>{0, 0, 0};
          const auto aDirection = aRotation.contains("direction") ? requiredDirection(aRotation, "direction") : std::array<double, 3>{0, 0, 1};
          const double anAngle = aRotation.at("angle").get<double>();
          gp_Trsf aPart;
          aPart.SetRotation(gp_Ax1(gp_Pnt(anOrigin[0], anOrigin[1], anOrigin[2]), gp_Dir(aDirection[0], aDirection[1], aDirection[2])), anAngle);
          aTrsf.Multiply(aPart);
        }
        if (theArgs.contains("scale"))
        {
          const double aScale = theArgs.at("scale").get<double>();
          if (aScale == 0.0) throw KernelFailure(ErrorCode::InvalidArgs, "Scale cannot be zero");
          const auto anOrigin = theArgs.contains("scaleOrigin") ? requiredVec3(theArgs, "scaleOrigin") : std::array<double, 3>{0, 0, 0};
          gp_Trsf aPart;
          aPart.SetScale(gp_Pnt(anOrigin[0], anOrigin[1], anOrigin[2]), aScale);
          aTrsf.Multiply(aPart);
        }
        if (theArgs.contains("mirror"))
        {
          const auto aMirror = requiredDirection(theArgs, "mirror");
          gp_Trsf aPart;
          aPart.SetMirror(gp_Ax2(gp_Pnt(0, 0, 0), gp_Dir(aMirror[0], aMirror[1], aMirror[2])));
          aTrsf.Multiply(aPart);
        }
      }
      const bool isCopy = theArgs.value("copy", true);
      BRepBuilderAPI_Transform aBuilder(aShape, aTrsf, isCopy, false);
      if (!aBuilder.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT transform failed");
      json aHistory;
      if (theArgs.value("includeHistory", false))
        aHistory = buildHistory(aBuilder, {aShape}, aBuilder.Shape());
      json aResult{{"shape", myArena.add(aBuilder.Shape(), aScope)}};
      if (theArgs.value("includeHistory", false))
        aResult["history"] = std::move(aHistory);
      return aResult;
    }
    if (theOp == "batchTransformCopy")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
      const std::uint32_t aCount = requiredU32(theArgs, "count");
      if (aCount == 0) throw KernelFailure(ErrorCode::InvalidArgs, "batchTransformCopy count must be positive");
      const std::string aMode = theArgs.value("mode", "linear");
      std::array<double, 3> aStep{};
      std::array<double, 3> anOrigin{};
      std::array<double, 3> aDirection{};
      double anAngle = 0.0;
      if (aMode == "linear")
      {
        aStep = requiredVec3(theArgs, "translation");
      }
      else if (aMode == "circular")
      {
        anOrigin = theArgs.contains("origin")
          ? requiredVec3(theArgs, "origin") : std::array<double, 3>{0.0, 0.0, 0.0};
        aDirection = requiredDirection(theArgs, "direction");
        anAngle = requiredNumber(theArgs, "angle");
        if (anAngle == 0.0) throw KernelFailure(ErrorCode::InvalidArgs, "Circular array angle cannot be zero");
      }
      else throw KernelFailure(ErrorCode::InvalidArgs, "batchTransformCopy mode must be linear or circular");
      json aShapes = json::array();
      for (std::uint32_t anIndex = 1; anIndex <= aCount; ++anIndex)
      {
        gp_Trsf aTrsf;
        if (aMode == "linear")
        {
          gp_Vec aTranslation(aStep[0] * anIndex, aStep[1] * anIndex, aStep[2] * anIndex);
          aTrsf.SetTranslation(aTranslation);
        }
        else
        {
          aTrsf.SetRotation(gp_Ax1(gp_Pnt(anOrigin[0], anOrigin[1], anOrigin[2]),
                                    gp_Dir(aDirection[0], aDirection[1], aDirection[2])),
                            anAngle * static_cast<double>(anIndex) / static_cast<double>(aCount));
        }
        BRepBuilderAPI_Transform aBuilder(aShape, aTrsf, true, false);
        if (!aBuilder.IsDone() || aBuilder.Shape().IsNull())
          throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT batch transform failed");
        aShapes.push_back(myArena.add(aBuilder.Shape(), aScope));
      }
      return {{"shapes", aShapes}};
    }
    if (theOp == "generalTransform")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape aShape = myArena.get(requiredU32(theArgs, "shape"));
      if (!theArgs.contains("matrix") || !theArgs.at("matrix").is_array() || theArgs.at("matrix").size() != 12)
        throw KernelFailure(ErrorCode::InvalidArgs, "generalTransform matrix must contain 12 numbers");
      const json& aMatrix = theArgs.at("matrix");
      gp_GTrsf aTrsf;
      for (int aRow = 1; aRow <= 3; ++aRow)
        for (int aColumn = 1; aColumn <= 4; ++aColumn)
          aTrsf.SetValue(aRow, aColumn, aMatrix[(aRow - 1) * 4 + aColumn - 1].get<double>());
      if (aTrsf.IsSingular()) throw KernelFailure(ErrorCode::InvalidArgs, "generalTransform matrix must be non-singular");
      aTrsf.SetForm();
      BRepBuilderAPI_GTransform aBuilder(aShape, aTrsf, true);
      if (!aBuilder.IsDone()) throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT general transform failed");
      json aResult{{"shape", myArena.add(aBuilder.Shape(), aScope)}};
      if (theArgs.value("includeHistory", false))
        aResult["history"] = buildHistory(aBuilder, {aShape}, aBuilder.Shape());
      return aResult;
    }
    if (theOp == "sew")
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
        anInputs.push_back(myArena.get(aShape.get<std::uint32_t>()));
        aBuilder.Add(anInputs.back());
      }
      aBuilder.Perform();
      if (aBuilder.SewedShape().IsNull()) throw KernelFailure(ErrorCode::HealingFailed, "OCCT sewing produced no shape");
      json aResult{{"shape", myArena.add(aBuilder.SewedShape(), aScope)},
                   {"freeEdges", aBuilder.NbFreeEdges()}, {"multipleEdges", aBuilder.NbMultipleEdges()}};
      if (theArgs.value("includeHistory", false))
        aResult["history"] = buildRecordedHistory(aBuilder.GetContext()->History(), anInputs, aBuilder.SewedShape());
      return aResult;
    }
    if (theOp == "fixShape")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape anInput = myArena.get(requiredU32(theArgs, "shape"));
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
      json aResponse{{"shape", myArena.add(aResult, aScope)}};
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
    if (theOp == "unifySameDomain")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape anInput = myArena.get(requiredU32(theArgs, "shape"));
      ShapeUpgrade_UnifySameDomain aBuilder(anInput,
                                            theArgs.value("unifyEdges", true),
                                            theArgs.value("unifyFaces", true),
                                            theArgs.value("concatBSplines", false));
      aBuilder.SetSafeInputMode(true);
      aBuilder.Build();
      if (aBuilder.Shape().IsNull()) throw KernelFailure(ErrorCode::HealingFailed, "OCCT same-domain unification produced no shape");
      json aResult{{"shape", myArena.add(aBuilder.Shape(), aScope)}};
      if (theArgs.value("includeHistory", false))
        aResult["history"] = buildRecordedHistory(aBuilder.History(), {anInput}, aBuilder.Shape());
      return aResult;
    }
    if (theOp == "shapeUpgrade")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
      const std::string aMode = theArgs.value("mode", "");

      const auto readCriterion = [&](const char* theName, const GeomAbs_Shape theDefault) {
        if (!theArgs.contains(theName)) return theDefault;
        const std::string aValue = theArgs.at(theName).get<std::string>();
        if (aValue == "c0") return GeomAbs_C0;
        if (aValue == "c1") return GeomAbs_C1;
        if (aValue == "c2") return GeomAbs_C2;
        if (aValue == "c3") return GeomAbs_C3;
        if (aValue == "cn") return GeomAbs_CN;
        throw KernelFailure(ErrorCode::InvalidArgs,
                            std::string("shapeUpgrade ") + theName
                            + " must be c0, c1, c2, c3, or cn");
      };
      const auto readPositive = [&](const char* theName, const double theDefault) {
        const double aValue = theArgs.value(theName, theDefault);
        if (!std::isfinite(aValue) || aValue <= 0.0)
          throw KernelFailure(ErrorCode::InvalidArgs,
                              std::string("shapeUpgrade ") + theName
                              + " must be positive and finite");
        return aValue;
      };
      const auto configureDivide = [&](ShapeUpgrade_ShapeDivide& theBuilder) {
        const double aPrecision = readPositive("precision", Precision::Confusion());
        const double aMinTolerance = readPositive("minTolerance", Precision::Confusion());
        const double aMaxTolerance = readPositive("maxTolerance", 1.0e-2);
        if (aMinTolerance > aMaxTolerance)
          throw KernelFailure(ErrorCode::InvalidArgs,
                              "shapeUpgrade minTolerance must not exceed maxTolerance");
        const int anEdgeMode = theArgs.value("edgeMode", 0);
        if (anEdgeMode < 0 || anEdgeMode > 2)
          throw KernelFailure(ErrorCode::InvalidArgs, "shapeUpgrade edgeMode must be 0, 1, or 2");
        theBuilder.SetPrecision(aPrecision);
        theBuilder.SetMinTolerance(aMinTolerance);
        theBuilder.SetMaxTolerance(aMaxTolerance);
        theBuilder.SetSurfaceSegmentMode(theArgs.value("surfaceSegmentMode", false));
        theBuilder.SetEdgeMode(anEdgeMode);
      };
      const auto performDivide = [&](ShapeUpgrade_ShapeDivide& theBuilder,
                                     const char* theDescription) {
        configureDivide(theBuilder);
        theBuilder.Perform(true);
        const TopoDS_Shape aResult = theBuilder.Result();
        if (theBuilder.Status(ShapeExtend_FAIL) || aResult.IsNull())
          throw KernelFailure(ErrorCode::HealingFailed,
                              std::string("OCCT ") + theDescription + " produced no result");
        return aResult;
      };
      const auto readPositiveInt = [&](const char* theName) {
        const std::uint32_t aValue = requiredU32(theArgs, theName);
        if (aValue == 0 || aValue > static_cast<std::uint32_t>(std::numeric_limits<int>::max()))
          throw KernelFailure(ErrorCode::InvalidArgs,
                              std::string("shapeUpgrade ") + theName + " must be a positive integer");
        return static_cast<int>(aValue);
      };

      if (aMode == "continuity")
      {
        ShapeUpgrade_ShapeDivideContinuity aBuilder(aShape);
        aBuilder.SetTolerance(readPositive("tolerance", Precision::Confusion()));
        aBuilder.SetTolerance2d(readPositive("tolerance2d", Precision::PConfusion()));
        aBuilder.SetBoundaryCriterion(readCriterion("boundaryCriterion", GeomAbs_C1));
        aBuilder.SetPCurveCriterion(readCriterion("pcurveCriterion", GeomAbs_C1));
        aBuilder.SetSurfaceCriterion(readCriterion("surfaceCriterion", GeomAbs_C1));
        return {{"shape", myArena.add(
          performDivide(aBuilder, "continuity shape upgrade"), aScope)}};
      }
      if (aMode == "angle")
      {
        const double aMaxAngle = readPositive("maxAngle", 0.0);
        if (aMaxAngle > 2.0 * std::acos(-1.0))
          throw KernelFailure(ErrorCode::InvalidArgs,
                              "shapeUpgrade maxAngle must not exceed 2*pi radians");
        ShapeUpgrade_ShapeDivideAngle aBuilder(aMaxAngle, aShape);
        return {{"shape", myArena.add(
          performDivide(aBuilder, "angle shape upgrade"), aScope)}};
      }
      if (aMode == "area")
      {
        ShapeUpgrade_ShapeDivideArea aBuilder(aShape);
        const std::string anAreaMode = theArgs.value("areaMode", "");
        if (anAreaMode == "maxArea")
        {
          aBuilder.MaxArea() = readPositive("maxArea", 0.0);
        }
        else if (anAreaMode == "parts")
        {
          const int aPartCount = readPositiveInt("nbParts");
          if (aPartCount < 2)
            throw KernelFailure(ErrorCode::InvalidArgs, "shapeUpgrade nbParts must be at least 2");
          aBuilder.SetSplittingByNumber(true);
          aBuilder.NbParts() = aPartCount;
        }
        else if (anAreaMode == "uv")
        {
          aBuilder.SetNumbersUVSplits(readPositiveInt("uSplits"), readPositiveInt("vSplits"));
        }
        else
        {
          throw KernelFailure(ErrorCode::InvalidArgs,
                              "shapeUpgrade areaMode must be maxArea, parts, or uv");
        }
        return {{"shape", myArena.add(
          performDivide(aBuilder, "area shape upgrade"), aScope)}};
      }
      if (aMode == "closedFaces")
      {
        ShapeUpgrade_ShapeDivideClosed aBuilder(aShape);
        if (theArgs.contains("splitPoints"))
          aBuilder.SetNbSplitPoints(readPositiveInt("splitPoints"));
        return {{"shape", myArena.add(
          performDivide(aBuilder, "closed-face shape upgrade"), aScope)}};
      }
      if (aMode == "closedEdges")
      {
        ShapeUpgrade_ShapeDivideClosedEdges aBuilder(aShape);
        return {{"shape", myArena.add(
          performDivide(aBuilder, "closed-edge shape upgrade"), aScope)}};
      }
      if (aMode == "convertToBezier")
      {
        const bool aConvert2d = theArgs.value("convert2d", true);
        const bool aConvert3d = theArgs.value("convert3d", true);
        const bool aConvertSurfaces = theArgs.value("convertSurfaces", true);
        if (!aConvert2d && !aConvert3d && !aConvertSurfaces)
          throw KernelFailure(ErrorCode::InvalidArgs,
                              "shapeUpgrade convertToBezier requires at least one conversion mode");
        ShapeUpgrade_ShapeConvertToBezier aBuilder(aShape);
        aBuilder.Set2dConversion(aConvert2d);
        aBuilder.Set3dConversion(aConvert3d);
        aBuilder.SetSurfaceConversion(aConvertSurfaces);
        aBuilder.Set3dLineConversion(theArgs.value("convertLines", true));
        aBuilder.Set3dCircleConversion(theArgs.value("convertCircles", true));
        aBuilder.Set3dConicConversion(theArgs.value("convertConics", true));
        aBuilder.SetPlaneMode(theArgs.value("convertPlanes", true));
        aBuilder.SetRevolutionMode(theArgs.value("convertRevolutions", true));
        aBuilder.SetExtrusionMode(theArgs.value("convertExtrusions", true));
        aBuilder.SetBSplineMode(theArgs.value("convertBSplines", true));
        return {{"shape", myArena.add(
          performDivide(aBuilder, "Bezier conversion"), aScope)}};
      }
      if (aMode == "removeInternalWires")
      {
        ShapeUpgrade_RemoveInternalWires aBuilder(aShape);
        aBuilder.MinArea() = readPositive("minArea", 0.0);
        aBuilder.RemoveFaceMode() = theArgs.value("removeFaces", true);
        aBuilder.Perform();
        const TopoDS_Shape aResult = aBuilder.GetResult();
        if (aBuilder.Status(ShapeExtend_FAIL) || aResult.IsNull())
          throw KernelFailure(ErrorCode::HealingFailed,
                              "OCCT internal-wire removal produced no result");
        return {{"shape", myArena.add(aResult, aScope)}};
      }
      if (aMode == "removeLocations")
      {
        const std::string aLevel = theArgs.value("removeLevel", "shape");
        TopAbs_ShapeEnum aRemoveLevel = TopAbs_SHAPE;
        if (aLevel == "compound") aRemoveLevel = TopAbs_COMPOUND;
        else if (aLevel == "solid") aRemoveLevel = TopAbs_SOLID;
        else if (aLevel == "shell") aRemoveLevel = TopAbs_SHELL;
        else if (aLevel == "face") aRemoveLevel = TopAbs_FACE;
        else if (aLevel != "shape")
          throw KernelFailure(ErrorCode::InvalidArgs,
                              "shapeUpgrade removeLevel must be shape, compound, solid, shell, or face");
        ShapeUpgrade_RemoveLocations aBuilder;
        aBuilder.SetRemoveLevel(aRemoveLevel);
        aBuilder.Remove(aShape);
        const TopoDS_Shape aResult = aBuilder.GetResult();
        if (aResult.IsNull())
          throw KernelFailure(ErrorCode::HealingFailed,
                              "OCCT location removal produced no result");
        return {{"shape", myArena.add(aResult, aScope)}};
      }
      throw KernelFailure(ErrorCode::InvalidArgs,
                          "shapeUpgrade mode is not supported");
    }
    if (theOp == "massProps")
    {
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
      GProp_GProps aProps;
      const std::string aKind = theArgs.value("kind", "volume");
      if (aKind == "volume") BRepGProp::VolumeProperties(aShape, aProps);
      else if (aKind == "surface") BRepGProp::SurfaceProperties(aShape, aProps);
      else if (aKind == "linear") BRepGProp::LinearProperties(aShape, aProps);
      else throw KernelFailure(ErrorCode::InvalidArgs, "massProps kind must be volume, surface, or linear");
      const gp_Pnt aCenter = aProps.CentreOfMass();
      const gp_Mat anInertia = aProps.MatrixOfInertia();
      return {{"mass", aProps.Mass()}, {"center", {aCenter.X(), aCenter.Y(), aCenter.Z()}},
              {"inertia", {anInertia.Value(1, 1), anInertia.Value(1, 2), anInertia.Value(1, 3),
                           anInertia.Value(2, 1), anInertia.Value(2, 2), anInertia.Value(2, 3),
                           anInertia.Value(3, 1), anInertia.Value(3, 2), anInertia.Value(3, 3)}}};
    }
    if (theOp == "distance")
    {
      const TopoDS_Shape& aFirstShape = myArena.get(requiredU32(theArgs, "first"));
      const TopoDS_Shape& aSecondShape = myArena.get(requiredU32(theArgs, "second"));
      BRepExtrema_DistShapeShape aDistance(aFirstShape, aSecondShape);
      if (!aDistance.IsDone() || aDistance.NbSolution() == 0)
        throw KernelFailure(ErrorCode::KernelError, "OCCT distance computation failed");

      TopTools_IndexedMapOfShape aFirstVertices;
      TopTools_IndexedMapOfShape aFirstEdges;
      TopTools_IndexedMapOfShape aFirstFaces;
      TopTools_IndexedMapOfShape aSecondVertices;
      TopTools_IndexedMapOfShape aSecondEdges;
      TopTools_IndexedMapOfShape aSecondFaces;
      TopExp::MapShapes(aFirstShape, TopAbs_VERTEX, aFirstVertices);
      TopExp::MapShapes(aFirstShape, TopAbs_EDGE, aFirstEdges);
      TopExp::MapShapes(aFirstShape, TopAbs_FACE, aFirstFaces);
      TopExp::MapShapes(aSecondShape, TopAbs_VERTEX, aSecondVertices);
      TopExp::MapShapes(aSecondShape, TopAbs_EDGE, aSecondEdges);
      TopExp::MapShapes(aSecondShape, TopAbs_FACE, aSecondFaces);

      json aSolutions = json::array();
      for (int anIndex = 1; anIndex <= aDistance.NbSolution(); ++anIndex)
      {
        const auto makeSupport = [&](const bool theFirst) {
          const BRepExtrema_SupportType aType = theFirst
            ? aDistance.SupportTypeShape1(anIndex)
            : aDistance.SupportTypeShape2(anIndex);
          const TopoDS_Shape aSupportShape = theFirst
            ? aDistance.SupportOnShape1(anIndex)
            : aDistance.SupportOnShape2(anIndex);
          json aSupport;
          if (aType == BRepExtrema_IsVertex)
          {
            aSupport["type"] = "vertex";
            const int aShapeIndex = (theFirst ? aFirstVertices : aSecondVertices).FindIndex(aSupportShape);
            if (aShapeIndex > 0) aSupport["index"] = aShapeIndex - 1;
          }
          else if (aType == BRepExtrema_IsOnEdge)
          {
            aSupport["type"] = "edge";
            const int aShapeIndex = (theFirst ? aFirstEdges : aSecondEdges).FindIndex(aSupportShape);
            if (aShapeIndex > 0) aSupport["index"] = aShapeIndex - 1;
            double aParameter = 0.0;
            if (theFirst) aDistance.ParOnEdgeS1(anIndex, aParameter);
            else aDistance.ParOnEdgeS2(anIndex, aParameter);
            aSupport["parameter"] = aParameter;
          }
          else
          {
            aSupport["type"] = "face";
            const int aShapeIndex = (theFirst ? aFirstFaces : aSecondFaces).FindIndex(aSupportShape);
            if (aShapeIndex > 0) aSupport["index"] = aShapeIndex - 1;
            double aU = 0.0;
            double aV = 0.0;
            if (theFirst) aDistance.ParOnFaceS1(anIndex, aU, aV);
            else aDistance.ParOnFaceS2(anIndex, aU, aV);
            aSupport["u"] = aU;
            aSupport["v"] = aV;
          }
          return aSupport;
        };
        const gp_Pnt& aFirst = aDistance.PointOnShape1(anIndex);
        const gp_Pnt& aSecond = aDistance.PointOnShape2(anIndex);
        aSolutions.push_back({
          {"distance", aFirst.Distance(aSecond)},
          {"pointOnFirst", {aFirst.X(), aFirst.Y(), aFirst.Z()}},
          {"pointOnSecond", {aSecond.X(), aSecond.Y(), aSecond.Z()}},
          {"supportOnFirst", makeSupport(true)},
          {"supportOnSecond", makeSupport(false)}});
      }
      const gp_Pnt& aFirst = aDistance.PointOnShape1(1);
      const gp_Pnt& aSecond = aDistance.PointOnShape2(1);
      return {{"distance", aDistance.Value()}, {"pointOnFirst", {aFirst.X(), aFirst.Y(), aFirst.Z()}},
              {"pointOnSecond", {aSecond.X(), aSecond.Y(), aSecond.Z()}},
              {"innerSolution", aDistance.InnerSolution()}, {"solutions", std::move(aSolutions)}};
    }
    if (theOp == "classifyPoint")
    {
      const auto aPoint = requiredVec3(theArgs, "point");
      const double aTolerance = theArgs.value("tolerance", 1.0e-7);
      if (aTolerance <= 0.0) throw KernelFailure(ErrorCode::InvalidArgs, "Classification tolerance must be positive");
      BRepClass3d_SolidClassifier aClassifier(myArena.get(requiredU32(theArgs, "shape")),
                                              gp_Pnt(aPoint[0], aPoint[1], aPoint[2]), aTolerance);
      const TopAbs_State aState = aClassifier.State();
      return {{"state", aState == TopAbs_IN ? "inside" : aState == TopAbs_OUT ? "outside" :
                         aState == TopAbs_ON ? "on" : "unknown"}};
    }
    if (theOp == "topologyCounts")
    {
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
      json aCounts = json::object();
      for (const auto& aType : std::vector<std::pair<TopAbs_ShapeEnum, const char*>>{
             {TopAbs_COMPOUND, "compound"}, {TopAbs_COMPSOLID, "compsolid"}, {TopAbs_SOLID, "solid"},
             {TopAbs_SHELL, "shell"}, {TopAbs_FACE, "face"}, {TopAbs_WIRE, "wire"}, {TopAbs_EDGE, "edge"}, {TopAbs_VERTEX, "vertex"}})
      {
        TopTools_IndexedMapOfShape aMap;
        TopExp::MapShapes(aShape, aType.first, aMap);
        aCounts[aType.second] = aMap.Extent();
      }
      return aCounts;
    }
    if (theOp == "getSubShapes")
    {
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
      const std::string aTypeName = theArgs.value("type", "face");
      const auto aType = shapeTypeFromName(aTypeName);
      TopTools_IndexedMapOfShape aMap;
      TopExp::MapShapes(aShape, aType, aMap);
      json aShapes = json::array();
      for (int anIndex = 1; anIndex <= aMap.Extent(); ++anIndex)
        aShapes.push_back({{"type", aTypeName}, {"index", anIndex - 1}});
      return {{"shapes", aShapes}};
    }
    if (theOp == "getSubShape")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
      const TopAbs_ShapeEnum aType = shapeTypeFromName(theArgs.at("type").get<std::string>());
      const std::uint32_t anIndex = requiredU32(theArgs, "index");
      TopTools_IndexedMapOfShape aMap;
      TopExp::MapShapes(aShape, aType, aMap);
      if (anIndex >= static_cast<std::uint32_t>(aMap.Extent()))
        throw KernelFailure(ErrorCode::InvalidArgs, "Subshape index is out of range");
      return {{"shape", myArena.add(aMap(static_cast<int>(anIndex + 1)), aScope)}};
    }
    if (theOp == "getAdjacency")
    {
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
      const std::string aFromName = theArgs.value("from", "edge");
      const std::string aToName = theArgs.value("to", aFromName == "face" || aFromName == "vertex" ? "edge" : "face");
      const TopAbs_ShapeEnum aFrom = shapeTypeFromName(aFromName);
      const TopAbs_ShapeEnum aTo = shapeTypeFromName(aToName);
      const bool isAncestorDirection = (aFrom == TopAbs_EDGE && aTo == TopAbs_FACE)
        || (aFrom == TopAbs_VERTEX && aTo == TopAbs_EDGE);
      const bool isChildDirection = (aFrom == TopAbs_FACE && aTo == TopAbs_EDGE)
        || (aFrom == TopAbs_EDGE && aTo == TopAbs_VERTEX);
      if (!isAncestorDirection && !isChildDirection)
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "getAdjacency supports edge-to-face, face-to-edge, vertex-to-edge, and edge-to-vertex");
      TopTools_IndexedMapOfShape aFromShapes;
      TopTools_IndexedMapOfShape aToShapes;
      TopExp::MapShapes(aShape, aFrom, aFromShapes);
      TopExp::MapShapes(aShape, aTo, aToShapes);
      NCollection_IndexedDataMap<TopoDS_Shape, TopTools_ListOfShape, TopTools_ShapeMapHasher> aMap;
      if (isAncestorDirection) TopExp::MapShapesAndUniqueAncestors(aShape, aFrom, aTo, aMap);
      json anItems = json::array();
      for (int anIndex = 1; anIndex <= aFromShapes.Extent(); ++anIndex)
      {
        std::vector<int> aTargetIndices;
        if (isAncestorDirection)
        {
          TopTools_ListOfShape aList;
          if (aMap.FindFromKey(aFromShapes(anIndex), aList))
            for (const TopoDS_Shape& aTarget : aList)
            {
              const int aTargetIndex = aToShapes.FindIndex(aTarget);
              if (aTargetIndex > 0) aTargetIndices.push_back(aTargetIndex - 1);
            }
        }
        else
        {
          TopTools_IndexedMapOfShape aChildren;
          TopExp::MapShapes(aFromShapes(anIndex), aTo, aChildren);
          for (int aChildIndex = 1; aChildIndex <= aChildren.Extent(); ++aChildIndex)
          {
            const int aTargetIndex = aToShapes.FindIndex(aChildren(aChildIndex));
            if (aTargetIndex > 0) aTargetIndices.push_back(aTargetIndex - 1);
          }
        }
        std::sort(aTargetIndices.begin(), aTargetIndices.end());
        aTargetIndices.erase(std::unique(aTargetIndices.begin(), aTargetIndices.end()), aTargetIndices.end());
        json aTargets = json::array();
        for (const int aTargetIndex : aTargetIndices) aTargets.push_back(aTargetIndex);
        anItems.push_back({{"index", anIndex - 1}, {"adjacent", aTargets}});
      }
      return {{"from", aFromName}, {"to", aToName}, {"items", anItems}};
    }
    if (theOp == "shapeType")
    {
      return {{"type", shapeTypeName(myArena.get(requiredU32(theArgs, "shape")).ShapeType())}};
    }
    if (theOp == "isSameShape")
    {
      return {{"same", myArena.get(requiredU32(theArgs, "first")).IsSame(myArena.get(requiredU32(theArgs, "second")))}};
    }
    if (theOp == "isValid")
    {
      BRepCheck_Analyzer anAnalyzer(myArena.get(requiredU32(theArgs, "shape")), true);
      return {{"valid", anAnalyzer.IsValid()}};
    }
    if (theOp == "diagnoseShape")
    {
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
      BRepCheck_Analyzer anAnalyzer(
        aShape, theArgs.value("geomControls", true), false, theArgs.value("exact", false));
      json anIssues = json::array();
      for (const auto& aType : std::vector<std::pair<TopAbs_ShapeEnum, const char*>>{
             {TopAbs_COMPOUND, "compound"}, {TopAbs_COMPSOLID, "compsolid"},
             {TopAbs_SOLID, "solid"}, {TopAbs_SHELL, "shell"}, {TopAbs_FACE, "face"},
             {TopAbs_WIRE, "wire"}, {TopAbs_EDGE, "edge"}, {TopAbs_VERTEX, "vertex"}})
      {
        TopTools_IndexedMapOfShape aShapes;
        TopExp::MapShapes(aShape, aType.first, aShapes);
        for (int anIndex = 1; anIndex <= aShapes.Extent(); ++anIndex)
        {
          const occ::handle<BRepCheck_Result>& aResult = anAnalyzer.Result(aShapes(anIndex));
          if (aResult.IsNull()) continue;
          for (const BRepCheck_Status aStatus : aResult->Status())
          {
            if (aStatus == BRepCheck_NoError) continue;
            anIssues.push_back({
              {"type", aType.second}, {"index", anIndex - 1},
              {"status", checkStatusName(aStatus)}});
          }
        }
      }
      return {{"valid", anAnalyzer.IsValid()}, {"issues", anIssues}};
    }
    if (theOp == "inspectTolerances")
    {
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
      json aResult;
      for (const auto& aType : std::vector<std::pair<TopAbs_ShapeEnum, const char*>>{
             {TopAbs_FACE, "faces"}, {TopAbs_EDGE, "edges"}, {TopAbs_VERTEX, "vertices"}})
      {
        TopTools_IndexedMapOfShape aShapes;
        TopExp::MapShapes(aShape, aType.first, aShapes);
        json aValues = json::array();
        for (int anIndex = 1; anIndex <= aShapes.Extent(); ++anIndex)
        {
          double aTolerance = 0.0;
          if (aType.first == TopAbs_FACE)
            aTolerance = BRep_Tool::Tolerance(TopoDS::Face(aShapes(anIndex)));
          else if (aType.first == TopAbs_EDGE)
            aTolerance = BRep_Tool::Tolerance(TopoDS::Edge(aShapes(anIndex)));
          else
            aTolerance = BRep_Tool::Tolerance(TopoDS::Vertex(aShapes(anIndex)));
          aValues.push_back({{"index", anIndex - 1}, {"tolerance", aTolerance}});
        }
        aResult[aType.second] = std::move(aValues);
      }
      return aResult;
    }
    if (theOp == "setTolerance")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
      const double aTolerance = requiredNumber(theArgs, "tolerance");
      if (aTolerance <= 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs, "Shape tolerance must be positive");
      const std::string aTypeName = theArgs.value("type", "all");
      TopAbs_ShapeEnum aType = TopAbs_SHAPE;
      if (aTypeName == "face") aType = TopAbs_FACE;
      else if (aTypeName == "edge") aType = TopAbs_EDGE;
      else if (aTypeName == "vertex") aType = TopAbs_VERTEX;
      else if (aTypeName != "all")
        throw KernelFailure(ErrorCode::InvalidArgs, "Shape tolerance type is invalid");
      BRepBuilderAPI_Copy aCopy(aShape, true, false);
      if (!aCopy.IsDone() || aCopy.Shape().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to copy shape for tolerance editing");
      TopoDS_Shape aResult = aCopy.Shape();
      ShapeFix_ShapeTolerance().SetTolerance(aResult, aTolerance, aType);
      return {{"shape", myArena.add(aResult, aScope)}};
    }
    if (theOp == "bbox")
    {
      Bnd_Box aBox;
      BRepBndLib::Add(myArena.get(requiredU32(theArgs, "shape")), aBox, true);
      if (aBox.IsVoid())
      {
        throw KernelFailure(ErrorCode::KernelError, "Shape has an empty bounding box");
      }
      double xMin, yMin, zMin, xMax, yMax, zMax;
      aBox.Get(xMin, yMin, zMin, xMax, yMax, zMax);
      return {{"min", {xMin, yMin, zMin}}, {"max", {xMax, yMax, zMax}}};
    }
    if (theOp == "obb")
    {
      Bnd_OBB aBox;
      BRepBndLib::AddOBB(
        myArena.get(requiredU32(theArgs, "shape")), aBox,
        theArgs.value("useTriangulation", true),
        theArgs.value("optimal", false),
        theArgs.value("useShapeTolerance", true));
      if (aBox.IsVoid())
        throw KernelFailure(ErrorCode::KernelError, "Shape has an empty oriented bounding box");
      const gp_XYZ& aCenter = aBox.Center();
      const gp_XYZ& anX = aBox.XDirection();
      const gp_XYZ& aY = aBox.YDirection();
      const gp_XYZ& aZ = aBox.ZDirection();
      return {
        {"center", {aCenter.X(), aCenter.Y(), aCenter.Z()}},
        {"axes", {{anX.X(), anX.Y(), anX.Z()},
                  {aY.X(), aY.Y(), aY.Z()},
                  {aZ.X(), aZ.Y(), aZ.Z()}}},
        {"halfSizes", {aBox.XHSize(), aBox.YHSize(), aBox.ZHSize()}},
        {"axisAligned", aBox.IsAABox()}};
    }
    if (theOp == "curveContinuity" || theOp == "curveDomain"
        || theOp == "curveGeometry" || theOp == "evaluateCurve")
    {
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
      if (aShape.ShapeType() != TopAbs_EDGE)
        throw KernelFailure(ErrorCode::InvalidArgs, "Curve evaluation requires an edge shape");
      BRepAdaptor_Curve anAdaptor(TopoDS::Edge(aShape));
      const bool isPeriodic = anAdaptor.IsPeriodic();
      if (theOp == "curveContinuity")
        return {{"continuity", continuityName(anAdaptor.Continuity())}};
      if (theOp == "curveGeometry")
      {
        json aResult;
        auto setAxis = [&](const gp_Ax2& theAxis) {
          const gp_Pnt& aCenter = theAxis.Location();
          const gp_Dir& aNormal = theAxis.Direction();
          const gp_Dir& anXDirection = theAxis.XDirection();
          aResult["center"] = {aCenter.X(), aCenter.Y(), aCenter.Z()};
          aResult["normal"] = {aNormal.X(), aNormal.Y(), aNormal.Z()};
          aResult["xDirection"] = {anXDirection.X(), anXDirection.Y(), anXDirection.Z()};
        };
        switch (anAdaptor.GetType())
        {
          case GeomAbs_Line:
          {
            const gp_Lin aLine = anAdaptor.Line();
            const gp_Pnt& anOrigin = aLine.Location();
            const gp_Dir& aDirection = aLine.Direction();
            aResult = {{"type", "line"},
                       {"origin", {anOrigin.X(), anOrigin.Y(), anOrigin.Z()}},
                       {"direction", {aDirection.X(), aDirection.Y(), aDirection.Z()}}};
            break;
          }
          case GeomAbs_Circle:
          {
            const gp_Circ aCircle = anAdaptor.Circle();
            aResult["type"] = "circle";
            setAxis(aCircle.Position());
            aResult["radius"] = aCircle.Radius();
            break;
          }
          case GeomAbs_Ellipse:
          {
            const gp_Elips anEllipse = anAdaptor.Ellipse();
            aResult["type"] = "ellipse";
            setAxis(anEllipse.Position());
            aResult["majorRadius"] = anEllipse.MajorRadius();
            aResult["minorRadius"] = anEllipse.MinorRadius();
            break;
          }
          case GeomAbs_Hyperbola:
          {
            const gp_Hypr aHyperbola = anAdaptor.Hyperbola();
            aResult["type"] = "hyperbola";
            setAxis(aHyperbola.Position());
            aResult["majorRadius"] = aHyperbola.MajorRadius();
            aResult["minorRadius"] = aHyperbola.MinorRadius();
            break;
          }
          case GeomAbs_Parabola:
          {
            const gp_Parab aParabola = anAdaptor.Parabola();
            aResult["type"] = "parabola";
            setAxis(aParabola.Position());
            aResult["focal"] = aParabola.Focal();
            break;
          }
          case GeomAbs_BezierCurve:
            aResult = {{"type", "bezier"}, {"degree", anAdaptor.Degree()},
                       {"poleCount", anAdaptor.NbPoles()}, {"rational", anAdaptor.IsRational()}};
            break;
          case GeomAbs_BSplineCurve:
            aResult = {{"type", "bspline"}, {"degree", anAdaptor.Degree()},
                       {"poleCount", anAdaptor.NbPoles()}, {"knotCount", anAdaptor.NbKnots()},
                       {"rational", anAdaptor.IsRational()}};
            break;
          case GeomAbs_OffsetCurve:
          {
            const occ::handle<Geom_OffsetCurve> anOffset = anAdaptor.OffsetCurve();
            const gp_Dir& aDirection = anOffset->Direction();
            aResult = {{"type", "offset"}, {"offset", anOffset->Offset()},
                       {"direction", {aDirection.X(), aDirection.Y(), aDirection.Z()}}};
            break;
          }
          case GeomAbs_OtherCurve:
          default:
            aResult = {{"type", "other"}};
            break;
        }
        return aResult;
      }
      if (theOp == "curveDomain")
      {
        return {{"first", anAdaptor.FirstParameter()}, {"last", anAdaptor.LastParameter()},
                {"periodic", isPeriodic}, {"period", isPeriodic ? anAdaptor.Period() : 0.0}};
      }
      const double aParameter = requiredNumber(theArgs, "parameter");
      const double aResolution = theArgs.value("resolution", 1.0e-12);
      if (aResolution <= 0.0) throw KernelFailure(ErrorCode::InvalidArgs, "Curve resolution must be positive");
      if (aParameter < anAdaptor.FirstParameter() || aParameter > anAdaptor.LastParameter())
        throw KernelFailure(ErrorCode::InvalidArgs, "Curve parameter is outside the edge domain");
      BRepLProp_CLProps aProperties(anAdaptor, aParameter, 2, aResolution);
      const gp_Pnt& aPoint = aProperties.Value();
      const gp_Vec& aD1 = aProperties.D1();
      const gp_Vec& aD2 = aProperties.D2();
      const auto isCurveIntervalBoundary = [&](const GeomAbs_Shape theContinuity) {
        const int anIntervalCount = anAdaptor.NbIntervals(theContinuity);
        if (anIntervalCount <= 1) return false;
        TColStd_Array1OfReal anIntervals(1, anIntervalCount + 1);
        anAdaptor.Intervals(anIntervals, theContinuity);
        const double aTolerance = std::max(aResolution, Precision::PConfusion());
        for (int anIndex = 2; anIndex <= anIntervalCount; ++anIndex)
          if (std::abs(aParameter - anIntervals(anIndex)) <= aTolerance) return true;
        return false;
      };
      const bool hasTangent = aProperties.IsTangentDefined()
        && !isCurveIntervalBoundary(GeomAbs_C1);
      const bool hasCurvature = hasTangent;
      json aResult{{"parameter", aParameter},
                   {"point", {aPoint.X(), aPoint.Y(), aPoint.Z()}},
                   {"d1", {aD1.X(), aD1.Y(), aD1.Z()}},
                   {"d2", {aD2.X(), aD2.Y(), aD2.Z()}},
                   {"tangentDefined", hasTangent},
                   {"curvatureDefined", hasCurvature}};
      if (hasTangent)
      {
        gp_Dir aTangent;
        aProperties.Tangent(aTangent);
        aResult["tangent"] = {aTangent.X(), aTangent.Y(), aTangent.Z()};
      }
      if (hasCurvature)
      {
        aResult["curvature"] = aProperties.Curvature();
      }
      return aResult;
    }
    if (theOp == "surfaceContinuity" || theOp == "surfaceDomain"
        || theOp == "surfaceGeometry" || theOp == "evaluateSurface")
    {
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
      if (aShape.ShapeType() != TopAbs_FACE)
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface evaluation requires a face shape");
      const TopoDS_Face aFace = TopoDS::Face(aShape);
      BRepAdaptor_Surface anAdaptor(aFace, true);
      const bool isUPeriodic = anAdaptor.IsUPeriodic();
      const bool isVPeriodic = anAdaptor.IsVPeriodic();
      if (theOp == "surfaceContinuity")
        return {{"uContinuity", continuityName(anAdaptor.UContinuity())},
                {"vContinuity", continuityName(anAdaptor.VContinuity())}};
      if (theOp == "surfaceGeometry")
      {
        json aResult;
        auto setAxis = [&](const gp_Ax3& theAxis) {
          const gp_Pnt& anOrigin = theAxis.Location();
          const gp_Dir& aDirection = theAxis.Direction();
          const gp_Dir& anXDirection = theAxis.XDirection();
          aResult["origin"] = {anOrigin.X(), anOrigin.Y(), anOrigin.Z()};
          aResult["direction"] = {aDirection.X(), aDirection.Y(), aDirection.Z()};
          aResult["xDirection"] = {anXDirection.X(), anXDirection.Y(), anXDirection.Z()};
        };
        switch (anAdaptor.GetType())
        {
          case GeomAbs_Plane:
            aResult["type"] = "plane";
            setAxis(anAdaptor.Plane().Position());
            break;
          case GeomAbs_Cylinder:
          {
            const gp_Cylinder aCylinder = anAdaptor.Cylinder();
            aResult["type"] = "cylinder";
            setAxis(aCylinder.Position());
            aResult["radius"] = aCylinder.Radius();
            break;
          }
          case GeomAbs_Cone:
          {
            const gp_Cone aCone = anAdaptor.Cone();
            aResult["type"] = "cone";
            setAxis(aCone.Position());
            aResult["referenceRadius"] = aCone.RefRadius();
            aResult["semiAngle"] = aCone.SemiAngle();
            break;
          }
          case GeomAbs_Sphere:
          {
            const gp_Sphere aSphere = anAdaptor.Sphere();
            aResult["type"] = "sphere";
            setAxis(aSphere.Position());
            aResult["radius"] = aSphere.Radius();
            break;
          }
          case GeomAbs_Torus:
          {
            const gp_Torus aTorus = anAdaptor.Torus();
            aResult["type"] = "torus";
            setAxis(aTorus.Position());
            aResult["majorRadius"] = aTorus.MajorRadius();
            aResult["minorRadius"] = aTorus.MinorRadius();
            break;
          }
          case GeomAbs_BezierSurface:
            aResult = {{"type", "bezier"}, {"uDegree", anAdaptor.UDegree()},
                       {"vDegree", anAdaptor.VDegree()}, {"uPoleCount", anAdaptor.NbUPoles()},
                       {"vPoleCount", anAdaptor.NbVPoles()}};
            break;
          case GeomAbs_BSplineSurface:
            aResult = {{"type", "bspline"}, {"uDegree", anAdaptor.UDegree()},
                       {"vDegree", anAdaptor.VDegree()}, {"uPoleCount", anAdaptor.NbUPoles()},
                       {"vPoleCount", anAdaptor.NbVPoles()}, {"uKnotCount", anAdaptor.NbUKnots()},
                       {"vKnotCount", anAdaptor.NbVKnots()}};
            break;
          case GeomAbs_SurfaceOfRevolution:
          {
            const gp_Ax1 anAxis = anAdaptor.AxeOfRevolution();
            const gp_Pnt& anOrigin = anAxis.Location();
            const gp_Dir& aDirection = anAxis.Direction();
            aResult = {{"type", "revolution"},
                       {"origin", {anOrigin.X(), anOrigin.Y(), anOrigin.Z()}},
                       {"direction", {aDirection.X(), aDirection.Y(), aDirection.Z()}}};
            break;
          }
          case GeomAbs_SurfaceOfExtrusion:
          {
            const gp_Dir aDirection = anAdaptor.Direction();
            aResult = {{"type", "extrusion"},
                       {"direction", {aDirection.X(), aDirection.Y(), aDirection.Z()}}};
            break;
          }
          case GeomAbs_OffsetSurface:
            aResult = {{"type", "offset"}, {"offset", anAdaptor.OffsetValue()}};
            break;
          case GeomAbs_OtherSurface:
          default:
            aResult = {{"type", "other"}};
            break;
        }
        return aResult;
      }
      if (theOp == "surfaceDomain")
      {
        return {{"uFirst", anAdaptor.FirstUParameter()}, {"uLast", anAdaptor.LastUParameter()},
                {"vFirst", anAdaptor.FirstVParameter()}, {"vLast", anAdaptor.LastVParameter()},
                {"uPeriodic", isUPeriodic}, {"uPeriod", isUPeriodic ? anAdaptor.UPeriod() : 0.0},
                {"vPeriodic", isVPeriodic}, {"vPeriod", isVPeriodic ? anAdaptor.VPeriod() : 0.0}};
      }
      const double aU = requiredNumber(theArgs, "u");
      const double aV = requiredNumber(theArgs, "v");
      const double aResolution = theArgs.value("resolution", 1.0e-12);
      if (aResolution <= 0.0) throw KernelFailure(ErrorCode::InvalidArgs, "Surface resolution must be positive");
      if (aU < anAdaptor.FirstUParameter() || aU > anAdaptor.LastUParameter()
          || aV < anAdaptor.FirstVParameter() || aV > anAdaptor.LastVParameter())
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface parameters are outside the face domain");
      BRepLProp_SLProps aProperties(anAdaptor, aU, aV, 2, aResolution);
      const gp_Pnt& aPoint = aProperties.Value();
      const gp_Vec& aD1U = aProperties.D1U();
      const gp_Vec& aD1V = aProperties.D1V();
      const gp_Vec& aD2U = aProperties.D2U();
      const gp_Vec& aD2V = aProperties.D2V();
      const gp_Vec& aDUV = aProperties.DUV();
      const auto isSurfaceIntervalBoundary = [&](const bool isU,
                                                  const GeomAbs_Shape theContinuity) {
        const int anIntervalCount = isU
          ? anAdaptor.NbUIntervals(theContinuity)
          : anAdaptor.NbVIntervals(theContinuity);
        if (anIntervalCount <= 1) return false;
        TColStd_Array1OfReal anIntervals(1, anIntervalCount + 1);
        if (isU) anAdaptor.UIntervals(anIntervals, theContinuity);
        else anAdaptor.VIntervals(anIntervals, theContinuity);
        const double aParameter = isU ? aU : aV;
        const double aTolerance = std::max(aResolution, Precision::PConfusion());
        for (int anIndex = 2; anIndex <= anIntervalCount; ++anIndex)
          if (std::abs(aParameter - anIntervals(anIndex)) <= aTolerance) return true;
        return false;
      };
      const bool hasNormal = aProperties.IsNormalDefined()
        && !isSurfaceIntervalBoundary(true, GeomAbs_C1)
        && !isSurfaceIntervalBoundary(false, GeomAbs_C1);
      const bool hasCurvature = hasNormal && aProperties.IsCurvatureDefined();
      json aResult{{"u", aU}, {"v", aV},
                   {"point", {aPoint.X(), aPoint.Y(), aPoint.Z()}},
                   {"d1u", {aD1U.X(), aD1U.Y(), aD1U.Z()}},
                   {"d1v", {aD1V.X(), aD1V.Y(), aD1V.Z()}},
                   {"d2u", {aD2U.X(), aD2U.Y(), aD2U.Z()}},
                   {"d2v", {aD2V.X(), aD2V.Y(), aD2V.Z()}},
                   {"duv", {aDUV.X(), aDUV.Y(), aDUV.Z()}},
                   {"normalDefined", hasNormal}, {"curvatureDefined", hasCurvature}};
      const bool isReversed = aFace.Orientation() == TopAbs_REVERSED;
      if (hasNormal)
      {
        gp_Dir aNormal = aProperties.Normal();
        if (isReversed) aNormal.Reverse();
        aResult["normal"] = {aNormal.X(), aNormal.Y(), aNormal.Z()};
      }
      if (hasCurvature)
      {
        const double aRawMinimum = aProperties.MinCurvature();
        const double aRawMaximum = aProperties.MaxCurvature();
        aResult["minimumCurvature"] = isReversed ? -aRawMaximum : aRawMinimum;
        aResult["maximumCurvature"] = isReversed ? -aRawMinimum : aRawMaximum;
        aResult["meanCurvature"] = isReversed ? -aProperties.MeanCurvature() : aProperties.MeanCurvature();
        aResult["gaussianCurvature"] = aProperties.GaussianCurvature();
        if (!aProperties.IsUmbilic())
        {
          gp_Dir aMaximumDirection;
          gp_Dir aMinimumDirection;
          aProperties.CurvatureDirections(aMaximumDirection, aMinimumDirection);
          const gp_Dir& aResultMinimumDirection = isReversed ? aMaximumDirection : aMinimumDirection;
          const gp_Dir& aResultMaximumDirection = isReversed ? aMinimumDirection : aMaximumDirection;
          aResult["minimumDirection"] = {aResultMinimumDirection.X(), aResultMinimumDirection.Y(), aResultMinimumDirection.Z()};
          aResult["maximumDirection"] = {aResultMaximumDirection.X(), aResultMaximumDirection.Y(), aResultMaximumDirection.Z()};
        }
      }
      return aResult;
    }
    if (theOp == "surfaceIsoCurve")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      double aUFirst = 0.0;
      double aULast = 0.0;
      double aVFirst = 0.0;
      double aVLast = 0.0;
      const TopoDS_Shape& aSurfaceShape = myArena.get(requiredU32(theArgs, "surface"));
      const occ::handle<Geom_Surface> aSurface = surfaceForFace(
        aSurfaceShape, aUFirst, aULast, aVFirst, aVLast);
      const std::string aDirection = theArgs.value("direction", "");
      const double aParameter = requiredNumber(theArgs, "parameter");
      const double aTolerance = theArgs.value("tolerance", 1.0e-7);
      if (aTolerance <= 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs, "Iso-curve tolerance must be positive");

      occ::handle<Geom_Curve> anIsoCurve;
      double aFirst = 0.0;
      double aLast = 0.0;
      if (aDirection == "u")
      {
        if (aParameter < aUFirst || aParameter > aULast)
          throw KernelFailure(ErrorCode::InvalidArgs, "U iso-curve parameter is outside the face domain");
        anIsoCurve = aSurface->UIso(aParameter);
        aFirst = aVFirst;
        aLast = aVLast;
      }
      else if (aDirection == "v")
      {
        if (aParameter < aVFirst || aParameter > aVLast)
          throw KernelFailure(ErrorCode::InvalidArgs, "V iso-curve parameter is outside the face domain");
        anIsoCurve = aSurface->VIso(aParameter);
        aFirst = aUFirst;
        aLast = aULast;
      }
      else
      {
        throw KernelFailure(ErrorCode::InvalidArgs, "Iso-curve direction must be u or v");
      }
      BRepBuilderAPI_MakeEdge anIsoEdge(anIsoCurve, aFirst, aLast);
      if (!anIsoEdge.IsDone())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct the surface iso-curve");

      BRepAlgoAPI_Section aClip(anIsoEdge.Edge(), aSurfaceShape, false);
      aClip.SetNonDestructive(true);
      aClip.SetRunParallel(false);
      aClip.SetFuzzyValue(aTolerance);
      aClip.Approximation(true);
      aClip.ComputePCurveOn2(true);
      aClip.Build();
      if (aClip.HasErrors())
        throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to clip the surface iso-curve");

      json aShapes = json::array();
      TopTools_IndexedMapOfShape anEdges;
      TopExp::MapShapes(aClip.Shape(), TopAbs_EDGE, anEdges);
      for (int anIndex = 1; anIndex <= anEdges.Extent(); ++anIndex)
      {
        const TopoDS_Shape& anEdge = anEdges(anIndex);
        aShapes.push_back(myArena.add(anEdge, aScope));
      }
      return {{"shapes", aShapes}};
    }
    if (theOp == "projectPointCurve")
    {
      double aFirst = 0.0;
      double aLast = 0.0;
      const occ::handle<Geom_Curve> aCurve = curveForEdge(
        myArena.get(requiredU32(theArgs, "shape")), aFirst, aLast);
      const auto aPoint = requiredVec3(theArgs, "point");
      GeomAPI_ProjectPointOnCurve aProjection(
        gp_Pnt(aPoint[0], aPoint[1], aPoint[2]), aCurve, aFirst, aLast);
      json aSolutions = json::array();
      for (int anIndex = 1; anIndex <= aProjection.NbPoints(); ++anIndex)
      {
        const gp_Pnt aProjected = aProjection.Point(anIndex);
        aSolutions.push_back({
          {"parameter", aProjection.Parameter(anIndex)},
          {"point", {aProjected.X(), aProjected.Y(), aProjected.Z()}},
          {"distance", aProjection.Distance(anIndex)}});
      }
      return {{"solutions", aSolutions}};
    }
    if (theOp == "projectPointSurface")
    {
      double aUFirst = 0.0;
      double aULast = 0.0;
      double aVFirst = 0.0;
      double aVLast = 0.0;
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
      const occ::handle<Geom_Surface> aSurface = surfaceForFace(
        aShape, aUFirst, aULast, aVFirst, aVLast);
      const TopoDS_Face aFace = TopoDS::Face(aShape);
      const auto aPoint = requiredVec3(theArgs, "point");
      const double aTolerance = theArgs.value("tolerance", 1.0e-7);
      if (aTolerance <= 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs, "Projection tolerance must be positive");
      GeomAPI_ProjectPointOnSurf aProjection(
        gp_Pnt(aPoint[0], aPoint[1], aPoint[2]), aSurface,
        aUFirst, aULast, aVFirst, aVLast, aTolerance);
      json aSolutions = json::array();
      for (int anIndex = 1; anIndex <= aProjection.NbPoints(); ++anIndex)
      {
        const gp_Pnt aProjected = aProjection.Point(anIndex);
        double aU = 0.0;
        double aV = 0.0;
        aProjection.Parameters(anIndex, aU, aV);
        BRepClass_FaceClassifier aClassifier(aFace, gp_Pnt2d(aU, aV), aTolerance);
        if (aClassifier.State() != TopAbs_IN && aClassifier.State() != TopAbs_ON) continue;
        aSolutions.push_back({
          {"u", aU}, {"v", aV},
          {"point", {aProjected.X(), aProjected.Y(), aProjected.Z()}},
          {"distance", aProjection.Distance(anIndex)}});
      }
      return {{"solutions", aSolutions}};
    }
    if (theOp == "extremaCurveCurve")
    {
      double aFirst1 = 0.0;
      double aLast1 = 0.0;
      double aFirst2 = 0.0;
      double aLast2 = 0.0;
      const TopoDS_Shape& aShape1 = myArena.get(requiredU32(theArgs, "first"));
      const TopoDS_Shape& aShape2 = myArena.get(requiredU32(theArgs, "second"));
      const occ::handle<Geom_Curve> aCurve1 = curveForEdge(
        aShape1, aFirst1, aLast1);
      const occ::handle<Geom_Curve> aCurve2 = curveForEdge(
        aShape2, aFirst2, aLast2);
      GeomAPI_ExtremaCurveCurve anExtrema(
        aCurve1, aCurve2, aFirst1, aLast1, aFirst2, aLast2);
      json aSolutions = json::array();
      if (anExtrema.IsParallel())
      {
        BRepExtrema_DistShapeShape aDistance(aShape1, aShape2);
        if (!aDistance.IsDone() || aDistance.NbSolution() == 0)
          throw KernelFailure(ErrorCode::KernelError, "OCCT curve-curve extrema failed");
        for (int anIndex = 1; anIndex <= aDistance.NbSolution(); ++anIndex)
        {
          const gp_Pnt aPoint1 = aDistance.PointOnShape1(anIndex);
          const gp_Pnt aPoint2 = aDistance.PointOnShape2(anIndex);
          GeomAPI_ProjectPointOnCurve aProjection1(aPoint1, aCurve1, aFirst1, aLast1);
          GeomAPI_ProjectPointOnCurve aProjection2(aPoint2, aCurve2, aFirst2, aLast2);
          aSolutions.push_back({
            {"parameter1", aProjection1.LowerDistanceParameter()},
            {"parameter2", aProjection2.LowerDistanceParameter()},
            {"point1", {aPoint1.X(), aPoint1.Y(), aPoint1.Z()}},
            {"point2", {aPoint2.X(), aPoint2.Y(), aPoint2.Z()}},
            {"distance", aPoint1.Distance(aPoint2)}});
        }
      }
      else
      {
        for (int anIndex = 1; anIndex <= anExtrema.NbExtrema(); ++anIndex)
        {
          gp_Pnt aPoint1;
          gp_Pnt aPoint2;
          double aParameter1 = 0.0;
          double aParameter2 = 0.0;
          anExtrema.Points(anIndex, aPoint1, aPoint2);
          anExtrema.Parameters(anIndex, aParameter1, aParameter2);
          aSolutions.push_back({
            {"parameter1", aParameter1}, {"parameter2", aParameter2},
            {"point1", {aPoint1.X(), aPoint1.Y(), aPoint1.Z()}},
            {"point2", {aPoint2.X(), aPoint2.Y(), aPoint2.Z()}},
            {"distance", anExtrema.Distance(anIndex)}});
        }
      }
      return {{"solutions", aSolutions}, {"parallel", anExtrema.IsParallel()}};
    }
    if (theOp == "extremaCurveSurface")
    {
      double aFirst = 0.0;
      double aLast = 0.0;
      double aUFirst = 0.0;
      double aULast = 0.0;
      double aVFirst = 0.0;
      double aVLast = 0.0;
      const TopoDS_Shape& aCurveShape = myArena.get(requiredU32(theArgs, "curve"));
      const TopoDS_Shape& aSurfaceShape = myArena.get(requiredU32(theArgs, "surface"));
      const occ::handle<Geom_Curve> aCurve = curveForEdge(
        aCurveShape, aFirst, aLast);
      const occ::handle<Geom_Surface> aSurface = surfaceForFace(
        aSurfaceShape, aUFirst, aULast, aVFirst, aVLast);
      GeomAPI_ExtremaCurveSurface anExtrema(
        aCurve, aSurface, aFirst, aLast, aUFirst, aULast, aVFirst, aVLast);
      json aSolutions = json::array();
      if (anExtrema.IsParallel())
      {
        BRepExtrema_DistShapeShape aDistance(aCurveShape, aSurfaceShape);
        if (!aDistance.IsDone() || aDistance.NbSolution() == 0)
          throw KernelFailure(ErrorCode::KernelError, "OCCT curve-surface extrema failed");
        for (int anIndex = 1; anIndex <= aDistance.NbSolution(); ++anIndex)
        {
          const gp_Pnt aPointOnCurve = aDistance.PointOnShape1(anIndex);
          const gp_Pnt aPointOnSurface = aDistance.PointOnShape2(anIndex);
          GeomAPI_ProjectPointOnCurve aCurveProjection(aPointOnCurve, aCurve, aFirst, aLast);
          GeomAPI_ProjectPointOnSurf aSurfaceProjection(
            aPointOnSurface, aSurface, aUFirst, aULast, aVFirst, aVLast, 1.0e-7);
          double aU = 0.0;
          double aV = 0.0;
          aSurfaceProjection.LowerDistanceParameters(aU, aV);
          aSolutions.push_back({
            {"curveParameter", aCurveProjection.LowerDistanceParameter()}, {"u", aU}, {"v", aV},
            {"pointOnCurve", {aPointOnCurve.X(), aPointOnCurve.Y(), aPointOnCurve.Z()}},
            {"pointOnSurface", {aPointOnSurface.X(), aPointOnSurface.Y(), aPointOnSurface.Z()}},
            {"distance", aPointOnCurve.Distance(aPointOnSurface)}});
        }
      }
      else
      {
        const TopoDS_Face aFace = TopoDS::Face(aSurfaceShape);
        for (int anIndex = 1; anIndex <= anExtrema.NbExtrema(); ++anIndex)
        {
          gp_Pnt aPointOnCurve;
          gp_Pnt aPointOnSurface;
          double aCurveParameter = 0.0;
          double aU = 0.0;
          double aV = 0.0;
          anExtrema.Points(anIndex, aPointOnCurve, aPointOnSurface);
          anExtrema.Parameters(anIndex, aCurveParameter, aU, aV);
          BRepClass_FaceClassifier aClassifier(aFace, gp_Pnt2d(aU, aV), 1.0e-7);
          if (aClassifier.State() != TopAbs_IN && aClassifier.State() != TopAbs_ON) continue;
          aSolutions.push_back({
            {"curveParameter", aCurveParameter}, {"u", aU}, {"v", aV},
            {"pointOnCurve", {aPointOnCurve.X(), aPointOnCurve.Y(), aPointOnCurve.Z()}},
            {"pointOnSurface", {aPointOnSurface.X(), aPointOnSurface.Y(), aPointOnSurface.Z()}},
            {"distance", anExtrema.Distance(anIndex)}});
        }
      }
      return {{"solutions", aSolutions}, {"parallel", anExtrema.IsParallel()}};
    }
    if (theOp == "extremaSurfaceSurface")
    {
      double aUFirst1 = 0.0;
      double aULast1 = 0.0;
      double aVFirst1 = 0.0;
      double aVLast1 = 0.0;
      double aUFirst2 = 0.0;
      double aULast2 = 0.0;
      double aVFirst2 = 0.0;
      double aVLast2 = 0.0;
      const TopoDS_Shape& aShape1 = myArena.get(requiredU32(theArgs, "first"));
      const TopoDS_Shape& aShape2 = myArena.get(requiredU32(theArgs, "second"));
      const occ::handle<Geom_Surface> aSurface1 = surfaceForFace(
        aShape1, aUFirst1, aULast1, aVFirst1, aVLast1);
      const occ::handle<Geom_Surface> aSurface2 = surfaceForFace(
        aShape2, aUFirst2, aULast2, aVFirst2, aVLast2);
      GeomAPI_ExtremaSurfaceSurface anExtrema(
        aSurface1, aSurface2,
        aUFirst1, aULast1, aVFirst1, aVLast1,
        aUFirst2, aULast2, aVFirst2, aVLast2);
      json aSolutions = json::array();
      if (anExtrema.IsParallel())
      {
        BRepExtrema_DistShapeShape aDistance(aShape1, aShape2);
        if (!aDistance.IsDone() || aDistance.NbSolution() == 0)
          throw KernelFailure(ErrorCode::KernelError, "OCCT surface-surface extrema failed");
        for (int anIndex = 1; anIndex <= aDistance.NbSolution(); ++anIndex)
        {
          const gp_Pnt aPoint1 = aDistance.PointOnShape1(anIndex);
          const gp_Pnt aPoint2 = aDistance.PointOnShape2(anIndex);
          GeomAPI_ProjectPointOnSurf aProjection1(
            aPoint1, aSurface1, aUFirst1, aULast1, aVFirst1, aVLast1, 1.0e-7);
          GeomAPI_ProjectPointOnSurf aProjection2(
            aPoint2, aSurface2, aUFirst2, aULast2, aVFirst2, aVLast2, 1.0e-7);
          double aU1 = 0.0;
          double aV1 = 0.0;
          double aU2 = 0.0;
          double aV2 = 0.0;
          aProjection1.LowerDistanceParameters(aU1, aV1);
          aProjection2.LowerDistanceParameters(aU2, aV2);
          aSolutions.push_back({
            {"u1", aU1}, {"v1", aV1}, {"u2", aU2}, {"v2", aV2},
            {"point1", {aPoint1.X(), aPoint1.Y(), aPoint1.Z()}},
            {"point2", {aPoint2.X(), aPoint2.Y(), aPoint2.Z()}},
            {"distance", aPoint1.Distance(aPoint2)}});
        }
      }
      else
      {
        const TopoDS_Face aFace1 = TopoDS::Face(aShape1);
        const TopoDS_Face aFace2 = TopoDS::Face(aShape2);
        for (int anIndex = 1; anIndex <= anExtrema.NbExtrema(); ++anIndex)
        {
          gp_Pnt aPoint1;
          gp_Pnt aPoint2;
          double aU1 = 0.0;
          double aV1 = 0.0;
          double aU2 = 0.0;
          double aV2 = 0.0;
          anExtrema.Points(anIndex, aPoint1, aPoint2);
          anExtrema.Parameters(anIndex, aU1, aV1, aU2, aV2);
          BRepClass_FaceClassifier aClassifier1(aFace1, gp_Pnt2d(aU1, aV1), 1.0e-7);
          BRepClass_FaceClassifier aClassifier2(aFace2, gp_Pnt2d(aU2, aV2), 1.0e-7);
          if ((aClassifier1.State() != TopAbs_IN && aClassifier1.State() != TopAbs_ON)
              || (aClassifier2.State() != TopAbs_IN && aClassifier2.State() != TopAbs_ON)) continue;
          aSolutions.push_back({
            {"u1", aU1}, {"v1", aV1}, {"u2", aU2}, {"v2", aV2},
            {"point1", {aPoint1.X(), aPoint1.Y(), aPoint1.Z()}},
            {"point2", {aPoint2.X(), aPoint2.Y(), aPoint2.Z()}},
            {"distance", anExtrema.Distance(anIndex)}});
        }
      }
      return {{"solutions", aSolutions}, {"parallel", anExtrema.IsParallel()}};
    }
    if (theOp == "intersectCurveCurve")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aFirstShape = myArena.get(requiredU32(theArgs, "first"));
      const TopoDS_Shape& aSecondShape = myArena.get(requiredU32(theArgs, "second"));
      if (aFirstShape.ShapeType() != TopAbs_EDGE || aSecondShape.ShapeType() != TopAbs_EDGE)
        throw KernelFailure(ErrorCode::InvalidArgs, "Curve intersection requires edge shapes");
      const double aTolerance = theArgs.value("tolerance", 1.0e-7);
      if (aTolerance <= 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs, "Curve intersection tolerance must be positive");
      BRepAlgoAPI_Section anIntersection(aFirstShape, aSecondShape, false);
      anIntersection.SetNonDestructive(true);
      anIntersection.SetRunParallel(false);
      anIntersection.SetFuzzyValue(aTolerance);
      anIntersection.Approximation(true);
      anIntersection.Build();
      if (anIntersection.HasErrors())
        throw KernelFailure(ErrorCode::KernelError, "OCCT curve-curve intersection failed");
      json aShapes = json::array();
      TopTools_IndexedMapOfShape anEdges;
      TopExp::MapShapes(anIntersection.Shape(), TopAbs_EDGE, anEdges);
      TopTools_IndexedMapOfShape anEdgeVertices;
      for (int anIndex = 1; anIndex <= anEdges.Extent(); ++anIndex)
      {
        const TopoDS_Shape& anEdge = anEdges(anIndex);
        TopExp::MapShapes(anEdge, TopAbs_VERTEX, anEdgeVertices);
        aShapes.push_back(myArena.add(anEdge, aScope));
      }
      TopTools_IndexedMapOfShape allVertices;
      TopExp::MapShapes(anIntersection.Shape(), TopAbs_VERTEX, allVertices);
      for (int anIndex = 1; anIndex <= allVertices.Extent(); ++anIndex)
        if (!anEdgeVertices.Contains(allVertices(anIndex)))
          aShapes.push_back(myArena.add(allVertices(anIndex), aScope));
      return {{"shapes", aShapes}};
    }
    if (theOp == "intersectCurveSurface")
    {
      double aFirst = 0.0;
      double aLast = 0.0;
      double aUFirst = 0.0;
      double aULast = 0.0;
      double aVFirst = 0.0;
      double aVLast = 0.0;
      const TopoDS_Shape& aCurveShape = myArena.get(requiredU32(theArgs, "curve"));
      const TopoDS_Shape& aSurfaceShape = myArena.get(requiredU32(theArgs, "surface"));
      const occ::handle<Geom_Curve> aCurve = curveForEdge(aCurveShape, aFirst, aLast);
      const occ::handle<Geom_Surface> aSurface = surfaceForFace(
        aSurfaceShape, aUFirst, aULast, aVFirst, aVLast);
      const double aTolerance = theArgs.value("tolerance", 1.0e-7);
      if (aTolerance <= 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs, "Curve-surface intersection tolerance must be positive");
      BRepAlgoAPI_Section anIntersection(aCurveShape, aSurfaceShape, false);
      anIntersection.SetNonDestructive(true);
      anIntersection.SetRunParallel(false);
      anIntersection.SetFuzzyValue(aTolerance);
      anIntersection.Approximation(true);
      anIntersection.ComputePCurveOn2(true);
      anIntersection.Build();
      if (anIntersection.HasErrors())
        throw KernelFailure(ErrorCode::KernelError, "OCCT curve-surface intersection failed");

      TopTools_IndexedMapOfShape anEdges;
      TopExp::MapShapes(anIntersection.Shape(), TopAbs_EDGE, anEdges);
      TopTools_IndexedMapOfShape anEdgeVertices;
      for (int anIndex = 1; anIndex <= anEdges.Extent(); ++anIndex)
        TopExp::MapShapes(anEdges(anIndex), TopAbs_VERTEX, anEdgeVertices);

      json aPoints = json::array();
      TopTools_IndexedMapOfShape allVertices;
      TopExp::MapShapes(anIntersection.Shape(), TopAbs_VERTEX, allVertices);
      for (int anIndex = 1; anIndex <= allVertices.Extent(); ++anIndex)
      {
        const TopoDS_Shape& aVertex = allVertices(anIndex);
        if (anEdgeVertices.Contains(aVertex)) continue;
        const gp_Pnt aPoint = BRep_Tool::Pnt(TopoDS::Vertex(aVertex));
        GeomAPI_ProjectPointOnCurve aCurveProjection(aPoint, aCurve, aFirst, aLast);
        GeomAPI_ProjectPointOnSurf aSurfaceProjection(
          aPoint, aSurface, aUFirst, aULast, aVFirst, aVLast, aTolerance);
        if (aCurveProjection.NbPoints() == 0 || aSurfaceProjection.NbPoints() == 0)
          throw KernelFailure(ErrorCode::KernelError, "OCCT failed to parameterize an intersection point");
        double aU = 0.0;
        double aV = 0.0;
        aSurfaceProjection.LowerDistanceParameters(aU, aV);
        aPoints.push_back({
          {"point", {aPoint.X(), aPoint.Y(), aPoint.Z()}},
          {"curveParameter", aCurveProjection.LowerDistanceParameter()}, {"u", aU}, {"v", aV}});
      }
      json aSegments = json::array();
      for (int anIndex = 1; anIndex <= anEdges.Extent(); ++anIndex)
      {
        double aU1 = 0.0;
        double aV1 = 0.0;
        double aU2 = 0.0;
        double aV2 = 0.0;
        const TopoDS_Edge aSectionEdge = TopoDS::Edge(anEdges(anIndex));
        double aPCurveFirst = 0.0;
        double aPCurveLast = 0.0;
        const occ::handle<Geom2d_Curve> aPCurve = BRep_Tool::CurveOnSurface(
          aSectionEdge, TopoDS::Face(aSurfaceShape), aPCurveFirst, aPCurveLast);
        if (!aPCurve.IsNull())
        {
          const gp_Pnt2d aStartUV = aPCurve->Value(aPCurveFirst);
          const gp_Pnt2d anEndUV = aPCurve->Value(aPCurveLast);
          aU1 = aStartUV.X();
          aV1 = aStartUV.Y();
          aU2 = anEndUV.X();
          aV2 = anEndUV.Y();
        }
        else
        {
          double aSegmentFirst = 0.0;
          double aSegmentLast = 0.0;
          const occ::handle<Geom_Curve> aSegment = BRep_Tool::Curve(
            aSectionEdge, aSegmentFirst, aSegmentLast);
          if (aSegment.IsNull()) continue;
          GeomAPI_ProjectPointOnSurf aStartProjection(
            aSegment->Value(aSegmentFirst), aSurface,
            aUFirst, aULast, aVFirst, aVLast, aTolerance);
          GeomAPI_ProjectPointOnSurf anEndProjection(
            aSegment->Value(aSegmentLast), aSurface,
            aUFirst, aULast, aVFirst, aVLast, aTolerance);
          if (aStartProjection.NbPoints() == 0 || anEndProjection.NbPoints() == 0)
            throw KernelFailure(ErrorCode::KernelError, "OCCT failed to parameterize an intersection segment");
          aStartProjection.LowerDistanceParameters(aU1, aV1);
          anEndProjection.LowerDistanceParameters(aU2, aV2);
        }
        aSegments.push_back({{"u1", aU1}, {"v1", aV1}, {"u2", aU2}, {"v2", aV2}});
      }
      return {{"points", aPoints}, {"segments", aSegments}};
    }
    if (theOp == "intersectSurfaceSurface")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aFirstShape = myArena.get(requiredU32(theArgs, "first"));
      const TopoDS_Shape& aSecondShape = myArena.get(requiredU32(theArgs, "second"));
      if (aFirstShape.ShapeType() != TopAbs_FACE || aSecondShape.ShapeType() != TopAbs_FACE)
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface intersection requires face shapes");
      const double aTolerance = theArgs.value("tolerance", 1.0e-7);
      if (aTolerance <= 0.0)
        throw KernelFailure(ErrorCode::InvalidArgs, "Surface intersection tolerance must be positive");
      BRepAlgoAPI_Section anIntersection(aFirstShape, aSecondShape, false);
      anIntersection.SetNonDestructive(true);
      anIntersection.SetRunParallel(false);
      anIntersection.SetFuzzyValue(aTolerance);
      anIntersection.Approximation(true);
      anIntersection.ComputePCurveOn1(true);
      anIntersection.ComputePCurveOn2(true);
      anIntersection.Build();
      if (anIntersection.HasErrors())
        throw KernelFailure(ErrorCode::KernelError, "OCCT surface-surface intersection failed");
      json aShapes = json::array();
      TopTools_IndexedMapOfShape anEdges;
      TopExp::MapShapes(anIntersection.Shape(), TopAbs_EDGE, anEdges);
      TopTools_IndexedMapOfShape anEdgeVertices;
      for (int anIndex = 1; anIndex <= anEdges.Extent(); ++anIndex)
      {
        const TopoDS_Shape& anEdge = anEdges(anIndex);
        TopExp::MapShapes(anEdge, TopAbs_VERTEX, anEdgeVertices);
        aShapes.push_back(myArena.add(anEdge, aScope));
      }
      TopTools_IndexedMapOfShape allVertices;
      TopExp::MapShapes(anIntersection.Shape(), TopAbs_VERTEX, allVertices);
      for (int anIndex = 1; anIndex <= allVertices.Extent(); ++anIndex)
        if (!anEdgeVertices.Contains(allVertices(anIndex)))
          aShapes.push_back(myArena.add(allVertices(anIndex), aScope));
      return {{"shapes", aShapes}};
    }
    if (theOp == "triangulationData")
    {
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
      return tessellation(aShape, theArgs.value("includeUV", false));
    }
    if (theOp == "validateTriangulation")
    {
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
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
    if (theOp == "repairTriangulation")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
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
      return {{"shape", myArena.add(aRepaired, aScope)},
              {"faces", aFaces.Extent()},
              {"triangulatedFaces", aTriangulatedFaces},
              {"triangles", aTriangleCount}};
    }
    if (theOp == "replaceTriangulation")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
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
      return {{"shape", myArena.add(aFace, aScope)}};
    }
    if (theOp == "tessellate")
    {
      TopoDS_Shape aShape = myArena.get(requiredU32(theArgs, "shape"));
      const double aLinear = theArgs.value("linearDeflection", 0.1);
      const double anAngular = theArgs.value("angularDeflection", 0.5);
      const bool isRelative = theArgs.value("relative", false);
      if (aLinear <= 0.0 || anAngular <= 0.0)
      {
        throw KernelFailure(ErrorCode::InvalidArgs, "Tessellation deflections must be positive");
      }
      BRepTools::Clean(aShape);
      IMeshTools_Parameters aParameters;
      aParameters.Deflection = aLinear;
      aParameters.Angle = anAngular;
      aParameters.Relative = isRelative;
      aParameters.InParallel = false;
      occ::handle<Message_ProgressIndicator> aProgress = new HostProgressIndicator();
      Message_ProgressScope aProgressScope(aProgress->Start(), "tessellate", 1.0);
      BRepMesh_IncrementalMesh aMesher(aShape, aParameters, aProgressScope.Next());
      if (occt_worker_cancelled() != 0 || !aProgressScope.More())
      {
        throw KernelFailure(ErrorCode::Cancelled, "OCCT tessellation cancelled");
      }
      if (!aMesher.IsDone())
      {
        throw KernelFailure(ErrorCode::TessellationFailed, "OCCT tessellation failed");
      }
      return tessellation(aShape, theArgs.value("includeUV", false));
    }
    if (theOp == "tessellateEdges")
    {
      const double aLinear = theArgs.value("linearDeflection", 0.1);
      const double anAngular = theArgs.value("angularDeflection", 0.5);
      if (aLinear <= 0.0 || anAngular <= 0.0) throw KernelFailure(ErrorCode::InvalidArgs, "Edge deflections must be positive");
      const TopoDS_Shape& aShape = myArena.get(requiredU32(theArgs, "shape"));
      TopTools_IndexedMapOfShape anEdges;
      TopExp::MapShapes(aShape, TopAbs_EDGE, anEdges);
      occ::handle<Message_ProgressIndicator> aProgress = new HostProgressIndicator();
      Message_ProgressScope aProgressScope(aProgress->Start(), "tessellateEdges",
                                            static_cast<double>(anEdges.Extent()));
      std::vector<float> aPositions;
      std::vector<std::uint32_t> anEdgeGroups;
      for (int anEdgeIndex = 1; anEdgeIndex <= anEdges.Extent(); ++anEdgeIndex)
      {
        if (!aProgressScope.More())
          throw KernelFailure(ErrorCode::Cancelled, "OCCT edge tessellation cancelled");
        BRepAdaptor_Curve anAdaptor(TopoDS::Edge(anEdges(anEdgeIndex)));
        GCPnts_TangentialDeflection aPoints(anAdaptor, anAngular, aLinear);
        const std::uint32_t aStart = static_cast<std::uint32_t>(aPositions.size() / 3);
        for (int aPointIndex = 1; aPointIndex <= aPoints.NbPoints(); ++aPointIndex)
        {
          const gp_Pnt aPoint = aPoints.Value(aPointIndex);
          aPositions.insert(aPositions.end(), {static_cast<float>(aPoint.X()), static_cast<float>(aPoint.Y()), static_cast<float>(aPoint.Z())});
        }
        anEdgeGroups.insert(anEdgeGroups.end(), {static_cast<std::uint32_t>(anEdgeIndex - 1), aStart,
                                                  static_cast<std::uint32_t>(aPoints.NbPoints())});
        aProgressScope.Next();
      }
      const std::uint32_t aPositionsId = myBuffers.copy(aPositions);
      const std::uint32_t anGroupsId = myBuffers.copy(anEdgeGroups);
      return {{"positions", bufferDescriptor(aPositionsId, aPositions.size() * sizeof(float), "f32x3")},
              {"edgeGroups", bufferDescriptor(anGroupsId, anEdgeGroups.size() * sizeof(std::uint32_t), "u32x3")}};
    }
    if (theOp == "exportSTL")
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
        aBinaryHeader = inputBufferData(theArgs, "binaryHeader");
        if (aBinaryHeader.size() != 80)
          throw KernelFailure(ErrorCode::InvalidArgs, "STL binaryHeader must contain exactly 80 bytes");
      }
      occ::handle<Message_ProgressIndicator> aProgress = new HostProgressIndicator();
      Message_ProgressScope aProgressScope(aProgress->Start(), "STL export", 3.0);
      aProgressScope.Show();
      TriangleMesh aMesh;
      if (theArgs.contains("positions"))
      {
        const std::string aPositionBytes = inputBufferData(theArgs, "positions");
        const std::string anIndexBytes = inputBufferData(theArgs, "indices");
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
          const std::string aNormalBytes = inputBufferData(theArgs, "normals");
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
        aMesh = buildTriangleMesh(myArena.get(requiredU32(theArgs, "shape")),
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
      const std::uint32_t aBufferId = myBuffers.create(aData.size());
      if (!aData.empty()) std::memcpy(myBuffers.get(aBufferId).data(), aData.data(), aData.size());
      return {{"data", bufferDescriptor(aBufferId, aData.size(),
                                          anEncoding == "ascii" ? "stl-ascii" : "stl-binary")}};
    }
    if (theOp == "importSTL")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const std::string aData = inputBufferData(theArgs, "data");
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
          myBuffers.copy(aPositions), aPositions.size() * sizeof(float), "f32x3");
        aResult["indices"] = bufferDescriptor(
          myBuffers.copy(anIndices), anIndices.size() * sizeof(std::uint32_t), "u32");
        aResult["normals"] = bufferDescriptor(
          myBuffers.copy(aNormals), aNormals.size() * sizeof(float), "f32x3");
        if (isBinary)
        {
          const std::uint32_t aHeader = myBuffers.create(80);
          std::memcpy(myBuffers.get(aHeader).data(), aData.data(), 80);
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
      aResult["shape"] = myArena.add(aShape, aScope);
      aProgressScope.Next().Close();
      if (!aProgressScope.More())
        throw KernelFailure(ErrorCode::Cancelled, "STL import cancelled");
      aProgressScope.Show();
      return aResult;
    }
    if (theOp == "exportVRML")
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
        anInputMesh = parseIndexedTriangleMesh(theArgs, "VRML");
        aSourceShape = makeTriangleShape(anInputMesh);
      }
      else
      {
        aSourceShape = myArena.get(requiredU32(theArgs, "shape"));
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
      const std::uint32_t aBuffer = myBuffers.create(aData.size());
      if (!aData.empty())
        std::memcpy(myBuffers.get(aBuffer).data(), aData.data(), aData.size());
      return {{"data", bufferDescriptor(aBuffer, aData.size(), "vrml-utf8")}};
    }
    if (theOp == "importVRML")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const std::string aData = inputBufferData(theArgs, "data");
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
      json aResult = {{"shape", myArena.add(makeTriangleShape(aMesh), aScope)}};
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
          myBuffers.copy(aPositions), aPositions.size() * sizeof(float), "f32x3");
        aResult["indices"] = bufferDescriptor(
          myBuffers.copy(aMesh.indices), aMesh.indices.size() * sizeof(std::uint32_t), "u32");
        aResult["normals"] = bufferDescriptor(
          myBuffers.copy(aNormals), aNormals.size() * sizeof(float), "f32x3");
        if (!aUVs.empty())
          aResult["uvs"] = bufferDescriptor(
            myBuffers.copy(aUVs), aUVs.size() * sizeof(float), "f32x2");
        if (!aMesh.colors.empty())
          aResult["colors"] = bufferDescriptor(
            myBuffers.copy(aMesh.colors), aMesh.colors.size() * 4, "u8x4");
      }
      return aResult;
    }
    if (theOp == "exportOBJ" && theArgs.contains("positions"))
    {
      occ::handle<Message_ProgressIndicator> aProgress = new HostProgressIndicator();
      Message_ProgressScope aProgressScope(aProgress->Start(), "OBJ document export", 3.0);
      aProgressScope.Show();
      const std::string aPositionBytes = inputBufferData(theArgs, "positions");
      const std::string anIndexBytes = inputBufferData(theArgs, "indices");
      if (aPositionBytes.empty() || aPositionBytes.size() % (sizeof(float) * 3) != 0
          || anIndexBytes.empty() || anIndexBytes.size() % (sizeof(std::uint32_t) * 3) != 0)
        throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document mesh buffers have invalid dimensions");
      TriangleMesh aMesh;
      const std::size_t aVertexCount = aPositionBytes.size() / (sizeof(float) * 3);
      if (aVertexCount > std::numeric_limits<std::uint32_t>::max())
        throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document mesh has too many vertices");
      aMesh.positions.reserve(aVertexCount);
      for (std::size_t aVertex = 0; aVertex < aVertexCount; ++aVertex)
      {
        std::array<float, 3> aPoint{};
        std::memcpy(aPoint.data(), aPositionBytes.data() + aVertex * sizeof(float) * 3,
                    sizeof(float) * 3);
        if (!std::isfinite(aPoint[0]) || !std::isfinite(aPoint[1]) || !std::isfinite(aPoint[2]))
          throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document positions must be finite");
        aMesh.positions.emplace_back(aPoint[0], aPoint[1], aPoint[2]);
      }
      aMesh.indices.resize(anIndexBytes.size() / sizeof(std::uint32_t));
      if (aMesh.indices.size() > std::numeric_limits<std::uint32_t>::max())
        throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document mesh has too many indices");
      std::memcpy(aMesh.indices.data(), anIndexBytes.data(), anIndexBytes.size());
      for (const std::uint32_t anIndex : aMesh.indices)
        if (anIndex >= aVertexCount)
          throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document index is out of range");
      if (theArgs.contains("normals"))
      {
        const std::string aNormalBytes = inputBufferData(theArgs, "normals");
        if (aNormalBytes.size() != aVertexCount * sizeof(float) * 3)
          throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document normals must match positions");
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
            throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document normals must be finite and non-zero");
          aMesh.normals.emplace_back(aNormal[0], aNormal[1], aNormal[2]);
        }
      }
      if (theArgs.contains("uvs"))
      {
        const std::string aUVBytes = inputBufferData(theArgs, "uvs");
        if (aUVBytes.size() != aVertexCount * sizeof(float) * 2)
          throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document UVs must match positions");
        aMesh.uvs.reserve(aVertexCount);
        for (std::size_t aVertex = 0; aVertex < aVertexCount; ++aVertex)
        {
          std::array<float, 2> aUV{};
          std::memcpy(aUV.data(), aUVBytes.data() + aVertex * sizeof(float) * 2,
                      sizeof(float) * 2);
          if (!std::isfinite(aUV[0]) || !std::isfinite(aUV[1]))
            throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document UVs must be finite");
          aMesh.uvs.emplace_back(aUV[0], aUV[1]);
        }
      }
      const json aDocument = theArgs.value("document", json{
        {"primitives", json::array()},
        {"materials", json::array()},
        {"materialLibraries", json::array()}
      });
      if (!aDocument.is_object())
        throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document metadata is invalid");
      const auto checkedOBJText = [](const json& theValue, const char* theName) {
        if (!theValue.is_string())
          throw KernelFailure(ErrorCode::InvalidArgs, std::string("OBJ document ") + theName + " is invalid");
        const std::string aText = theValue.get<std::string>();
        if (aText.empty() || aText.find_first_of("\r\n") != std::string::npos)
          throw KernelFailure(ErrorCode::InvalidArgs, std::string("OBJ document ") + theName + " is invalid");
        return aText;
      };
      const json aPrimitives = aDocument.value("primitives", json::array());
      if (!aPrimitives.is_array())
        throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document primitives must be an array");
      std::size_t anExpectedIndex = 0;
      for (const json& aValue : aPrimitives)
      {
        if (!aValue.is_object() || !aValue.contains("indexStart") || !aValue.contains("indexCount")
            || !aValue.at("indexStart").is_number_unsigned()
            || !aValue.at("indexCount").is_number_unsigned())
          throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document primitive range is invalid");
        const std::size_t anIndexStart = aValue.at("indexStart").get<std::size_t>();
        const std::size_t anIndexCount = aValue.at("indexCount").get<std::size_t>();
        if (anIndexStart != anExpectedIndex || anIndexCount == 0 || anIndexCount % 3 != 0
            || anIndexStart > aMesh.indices.size()
            || anIndexCount > aMesh.indices.size() - anIndexStart)
          throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document primitive ranges must partition indices");
        TriangleMesh::ObjPrimitive aPrimitive;
        aPrimitive.indexStart = static_cast<std::uint32_t>(anIndexStart);
        aPrimitive.indexCount = static_cast<std::uint32_t>(anIndexCount);
        if (aValue.contains("object"))
        {
          aPrimitive.object = checkedOBJText(aValue.at("object"), "object name");
          aPrimitive.hasObject = true;
        }
        if (aValue.contains("groups"))
        {
          if (!aValue.at("groups").is_array())
            throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document groups are invalid");
          for (const json& aGroup : aValue.at("groups"))
          {
            aPrimitive.groups.push_back(checkedOBJText(aGroup, "group name"));
          }
        }
        if (aValue.contains("material"))
        {
          aPrimitive.material = checkedOBJText(aValue.at("material"), "material name");
          aPrimitive.hasMaterial = true;
        }
        if (aValue.contains("smoothingGroup"))
        {
          aPrimitive.smoothingGroup = checkedOBJText(aValue.at("smoothingGroup"), "smoothing group");
          aPrimitive.hasSmoothingGroup = true;
        }
        aMesh.objPrimitives.push_back(std::move(aPrimitive));
        anExpectedIndex += anIndexCount;
      }
      if (!aMesh.objPrimitives.empty() && anExpectedIndex != aMesh.indices.size())
        throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document primitive ranges do not cover indices");
      const json aMaterials = aDocument.value("materials", json::array());
      const json aLibraries = aDocument.value("materialLibraries", json::array());
      if (!aMaterials.is_array() || !aLibraries.is_array())
        throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document materials are invalid");
      for (const json& aMaterial : aMaterials)
      {
        if (!aMaterial.is_object() || !aMaterial.contains("name") || !aMaterial.at("name").is_string()
            || aMaterial.at("name").get<std::string>().empty())
          throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document material is invalid");
        const auto hasNewline = [](const std::string& theValue) {
          return theValue.find('\n') != std::string::npos || theValue.find('\r') != std::string::npos;
        };
        if (hasNewline(aMaterial.at("name").get<std::string>()))
          throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document material name contains a newline");
        for (const char* aProperty : {"ambient", "diffuse", "specular", "emissive"})
        {
          if (!aMaterial.contains(aProperty)) continue;
          const json& aVector = aMaterial.at(aProperty);
          if (!aVector.is_array() || aVector.size() != 3)
            throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document material color is invalid");
          for (const json& aComponent : aVector)
            if (!aComponent.is_number() || !std::isfinite(aComponent.get<double>()))
              throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document material color is invalid");
        }
        for (const char* aProperty : {"opacity", "shininess", "opticalDensity"})
          if (aMaterial.contains(aProperty)
              && (!aMaterial.at(aProperty).is_number()
                  || !std::isfinite(aMaterial.at(aProperty).get<double>())))
            throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document material number is invalid");
        if (aMaterial.contains("illuminationModel")
            && !aMaterial.at("illuminationModel").is_number_unsigned())
          throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document illumination model is invalid");
        for (const char* aProperty : {"diffuseMap", "specularMap", "opacityMap", "bumpMap"})
          if (aMaterial.contains(aProperty)
              && (!aMaterial.at(aProperty).is_string()
                  || hasNewline(aMaterial.at(aProperty).get<std::string>())))
            throw KernelFailure(ErrorCode::InvalidArgs, "OBJ document material map is invalid");
        aMesh.objMaterials.push_back(aMaterial);
      }
      for (const json& aLibrary : aLibraries)
      {
        aMesh.objMaterialLibraries.push_back(checkedOBJText(aLibrary, "material library"));
      }
      std::string aMaterialLibrary = theArgs.value("materialLibrary", std::string());
      if (aMaterialLibrary.empty() && !aMesh.objMaterials.empty())
        aMaterialLibrary = aMesh.objMaterialLibraries.empty() ? "materials.mtl" : aMesh.objMaterialLibraries.front();
      if (!aMaterialLibrary.empty())
      {
        if (aMaterialLibrary.find('\n') != std::string::npos || aMaterialLibrary.find('\r') != std::string::npos)
          throw KernelFailure(ErrorCode::InvalidArgs, "OBJ material library contains a newline");
        aMesh.objMaterialLibraries = {aMaterialLibrary};
      }
      aProgressScope.Next().Close();
      if (!aProgressScope.More())
        throw KernelFailure(ErrorCode::Cancelled, "OBJ document export cancelled");
      aProgressScope.Show();
      const std::string aData = writeOBJ(aMesh);
      aProgressScope.Next().Close();
      if (!aProgressScope.More())
        throw KernelFailure(ErrorCode::Cancelled, "OBJ document export cancelled");
      aProgressScope.Show();
      const std::uint32_t aDataBuffer = myBuffers.create(aData.size());
      if (!aData.empty()) std::memcpy(myBuffers.get(aDataBuffer).data(), aData.data(), aData.size());
      json aResources = json::array();
      if (!aMesh.objMaterials.empty())
      {
        const std::string aMTL = writeMTL(aMesh.objMaterials);
        const std::uint32_t aMTLBuffer = myBuffers.create(aMTL.size());
        if (!aMTL.empty()) std::memcpy(myBuffers.get(aMTLBuffer).data(), aMTL.data(), aMTL.size());
        aResources.push_back({{"uri", aMesh.objMaterialLibraries.front()},
                              {"data", bufferDescriptor(aMTLBuffer, aMTL.size(), "mtl-text")} });
      }
      aProgressScope.Next().Close();
      if (!aProgressScope.More())
        throw KernelFailure(ErrorCode::Cancelled, "OBJ document export cancelled");
      aProgressScope.Show();
      return {{"data", bufferDescriptor(aDataBuffer, aData.size(), "obj-text")},
              {"resources", std::move(aResources)}};
    }
    if (theOp == "exportGLTF" && theArgs.contains("document"))
    {
      occ::handle<Message_ProgressIndicator> aProgress = new HostProgressIndicator();
      Message_ProgressScope aProgressScope(aProgress->Start(), "glTF document export", 3.0);
      aProgressScope.Show();
      if (!theArgs.at("document").is_object())
        throw KernelFailure(ErrorCode::InvalidArgs, "glTF document metadata is invalid");
      if (!theArgs.contains("buffers") || !theArgs.at("buffers").is_array())
        throw KernelFailure(ErrorCode::InvalidArgs, "glTF document buffers must be an array");
      std::vector<std::pair<std::string, std::vector<std::uint8_t>>> aBuffers;
      for (const json& aBuffer : theArgs.at("buffers"))
      {
        if (!aBuffer.is_object() || !aBuffer.contains("data") || !aBuffer.at("data").is_object()
            || (aBuffer.contains("uri") && !aBuffer.at("uri").is_string()))
          throw KernelFailure(ErrorCode::InvalidArgs, "glTF document buffer input is invalid");
        const std::string aData = inputBufferData(aBuffer, "data");
        aBuffers.emplace_back(aBuffer.value("uri", std::string()),
                              std::vector<std::uint8_t>(aData.begin(), aData.end()));
      }
      std::vector<std::pair<std::string, std::vector<std::uint8_t>>> aResources;
      if (theArgs.contains("resources"))
      {
        if (!theArgs.at("resources").is_array())
          throw KernelFailure(ErrorCode::InvalidArgs, "glTF document resources must be an array");
        for (const json& aResource : theArgs.at("resources"))
        {
          if (!aResource.is_object() || !aResource.contains("uri")
              || !aResource.at("uri").is_string()
              || aResource.at("uri").get<std::string>().empty()
              || !aResource.contains("data") || !aResource.at("data").is_object())
            throw KernelFailure(ErrorCode::InvalidArgs, "glTF document resource input is invalid");
          const std::string aData = inputBufferData(aResource, "data");
          aResources.emplace_back(aResource.at("uri").get<std::string>(),
                                  std::vector<std::uint8_t>(aData.begin(), aData.end()));
        }
      }
      const std::string aFormat = theArgs.value("format", "glb");
      if (aFormat != "glb" && aFormat != "gltf")
        throw KernelFailure(ErrorCode::InvalidArgs, "glTF format must be glb or gltf");
      aProgressScope.Next().Close();
      if (!aProgressScope.More())
        throw KernelFailure(ErrorCode::Cancelled, "glTF document export cancelled");
      aProgressScope.Show();
      GltfDocumentExport anExport = writeGLTFDocument(
        theArgs.at("document"), aBuffers, aResources, aFormat == "glb");
      aProgressScope.Next().Close();
      if (!aProgressScope.More())
        throw KernelFailure(ErrorCode::Cancelled, "glTF document export cancelled");
      aProgressScope.Show();
      const std::uint32_t aDataBuffer = myBuffers.copy(anExport.data);
      json anExportedResources = json::array();
      for (const auto& aResource : anExport.resources)
      {
        anExportedResources.push_back({
          {"uri", aResource.first},
          {"data", bufferDescriptor(myBuffers.copy(aResource.second),
                                     aResource.second.size(), "u8")}
        });
      }
      aProgressScope.Next().Close();
      if (!aProgressScope.More())
        throw KernelFailure(ErrorCode::Cancelled, "glTF document export cancelled");
      aProgressScope.Show();
      return {{"data", bufferDescriptor(aDataBuffer, anExport.data.size(),
                                          aFormat == "glb" ? "glb" : "gltf-json")},
              {"resources", std::move(anExportedResources)}};
    }
    if (theOp == "exportGLTF" || theOp == "exportOBJ" || theOp == "exportPLY")
    {
      const double aLinear = theArgs.value("linearDeflection", 0.1);
      const double anAngular = theArgs.value("angularDeflection", 0.5);
      const bool isRelative = theArgs.value("relative", false);
      occ::handle<Message_ProgressIndicator> aProgress = new HostProgressIndicator();
      Message_ProgressScope aProgressScope(aProgress->Start(), "mesh export", 3.0);
      aProgressScope.Show();
      TriangleMesh aMesh;
      if (theOp == "exportPLY" && theArgs.contains("positions"))
      {
        const std::string aPositionBytes = inputBufferData(theArgs, "positions");
        const std::string anIndexBytes = inputBufferData(theArgs, "indices");
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
          const std::string aNormalBytes = inputBufferData(theArgs, "normals");
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
          const std::string aUVBytes = inputBufferData(theArgs, "uvs");
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
          const std::string aColorBytes = inputBufferData(theArgs, "colors");
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
          myArena.get(requiredU32(theArgs, "shape")), aLinear, anAngular, isRelative);
      }
      aProgressScope.Next().Close();
      if (!aProgressScope.More())
        throw KernelFailure(ErrorCode::Cancelled, "Mesh export cancelled");
      aProgressScope.Show();
      if (theOp == "exportGLTF")
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
        const std::uint32_t aBuffer = myBuffers.create(aData.size());
        if (!aData.empty())
        {
          std::memcpy(myBuffers.get(aBuffer).data(), aData.data(), aData.size());
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
      if (theOp == "exportPLY" && aPLYEncoding != "ascii"
          && aPLYEncoding != "binary_little_endian"
          && aPLYEncoding != "binary_big_endian")
      {
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "PLY encoding must be ascii, binary_little_endian, or binary_big_endian");
      }
      const std::string aData = theOp == "exportOBJ"
        ? writeOBJ(aMesh)
        : writePLY(aMesh, aPLYEncoding);
      aProgressScope.Next().Close();
      if (!aProgressScope.More())
        throw KernelFailure(ErrorCode::Cancelled, "Mesh export cancelled");
      aProgressScope.Show();
      const std::uint32_t aBuffer = myBuffers.create(aData.size());
      if (!aData.empty())
      {
        std::memcpy(myBuffers.get(aBuffer).data(), aData.data(), aData.size());
      }
      aProgressScope.Next().Close();
      if (!aProgressScope.More())
        throw KernelFailure(ErrorCode::Cancelled, "Mesh export cancelled");
      aProgressScope.Show();
      return {{"data", bufferDescriptor(aBuffer,
                                         aData.size(),
                                         theOp == "exportOBJ"
                                           ? "obj-text"
                                           : (aPLYEncoding == "ascii"
                                                ? "ply-ascii"
                                                : (aPLYEncoding == "binary_big_endian"
                                                     ? "ply-binary_big_endian"
                                                     : "ply-binary_little_endian")))}};
    }
    if (theOp == "importGLTF" || theOp == "importOBJ" || theOp == "importPLY")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const std::string aData = inputBufferData(theArgs, "data");
      occ::handle<Message_ProgressIndicator> aProgress = new HostProgressIndicator();
      Message_ProgressScope aProgressScope(aProgress->Start(), "mesh import", 3.0);
      aProgressScope.Show();
      TriangleMesh aMesh;
      if (theOp == "importGLTF")
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
            if (!aResources.emplace(aUri, inputBufferData(aResource, "data")).second)
              throw KernelFailure(ErrorCode::InvalidArgs, "glTF resource URIs must be unique");
          }
        }
        aMesh = parseGLTF(aData, aResources, theArgs.value("includeDocument", false),
                          aMorphWeightsPointer, anAnimationIndexPointer,
                          anAnimationTimePointer);
      }
      else if (theOp == "importOBJ")
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
            if (!aResources.emplace(aUri, inputBufferData(aResource, "data")).second)
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
        aResult["shape"] = myArena.add(makeTriangleShape(aMesh), aScope);
      const bool includeMesh = (theOp == "importPLY" && theArgs.value("includeMesh", false))
        || ((theOp == "importOBJ" || theOp == "importGLTF")
            && theArgs.value("includeDocument", false));
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
          myBuffers.copy(aPositions), aPositions.size() * sizeof(float), "f32x3");
        aResult["indices"] = bufferDescriptor(
          myBuffers.copy(aMesh.indices), aMesh.indices.size() * sizeof(std::uint32_t), "u32");
        if (!aNormals.empty())
          aResult["normals"] = bufferDescriptor(
            myBuffers.copy(aNormals), aNormals.size() * sizeof(float), "f32x3");
        if (!aUVs.empty())
          aResult["uvs"] = bufferDescriptor(
            myBuffers.copy(aUVs), aUVs.size() * sizeof(float), "f32x2");
        if (!aMesh.colors.empty())
          aResult["colors"] = bufferDescriptor(
            myBuffers.copy(aMesh.colors), aMesh.colors.size() * 4, "u8x4");
        if (theOp == "importPLY")
        {
          aResult["document"] = {
            {"comments", aMesh.plyComments},
            {"objectInfo", aMesh.plyObjectInfo}
          };
        }
        else if (theOp == "importOBJ")
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
        else if (theOp == "importGLTF")
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
              myBuffers.copy(aBytes), aBytes.size(), "u8")}};
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
                myBuffers.copy(aResource.second), aResource.second.size(), "u8")}
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
    if (theOp == "exportBREP")
    {
      std::ostringstream aStream(std::ios::out | std::ios::binary);
      BRepTools::Write(myArena.get(requiredU32(theArgs, "shape")), aStream, false, false,
                       TopTools_FormatVersion_CURRENT);
      const std::string aData = aStream.str();
      const std::uint32_t aBuffer = myBuffers.create(aData.size());
      if (!aData.empty())
      {
        std::memcpy(myBuffers.get(aBuffer).data(), aData.data(), aData.size());
      }
      return {{"data", bufferDescriptor(aBuffer, aData.size(), "u8")}};
    }
    if (theOp == "importBREP")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const std::string aData = inputBufferData(theArgs, "data");
      std::istringstream aStream(aData, std::ios::in | std::ios::binary);
      TopoDS_Shape aShape;
      BRep_Builder aBuilder;
      BRepTools::Read(aShape, aStream, aBuilder);
      if (aShape.IsNull())
      {
        throw KernelFailure(ErrorCode::ImportExportFailed, "BREP input did not contain a shape");
      }
      return {{"shape", myArena.add(aShape, aScope)}};
    }
    if (theOp == "probeFormat")
    {
      const std::string aData = inputBufferData(theArgs, "data");
      if (aData.size() >= 7 && std::memcmp(aData.data(), "BINFILE", 7) == 0)
        return {{"format", "xcaf"}, {"encoding", "binary"}, {"confidence", "exact"}};
      if (aData.size() >= 4 && std::memcmp(aData.data(), "glTF", 4) == 0)
        return {{"format", "gltf"}, {"encoding", "glb"}, {"confidence", "exact"}};
      if (aData.size() >= 84)
      {
        const auto* aBytes = reinterpret_cast<const unsigned char*>(aData.data());
        const std::uint32_t aTriangleCount = static_cast<std::uint32_t>(aBytes[80])
          | (static_cast<std::uint32_t>(aBytes[81]) << 8)
          | (static_cast<std::uint32_t>(aBytes[82]) << 16)
          | (static_cast<std::uint32_t>(aBytes[83]) << 24);
        if (aTriangleCount <= (std::numeric_limits<std::size_t>::max() - 84) / 50
            && 84 + static_cast<std::size_t>(aTriangleCount) * 50 == aData.size())
          return {{"format", "stl"}, {"encoding", "binary"}, {"confidence", "exact"}};
      }

      std::size_t aStart = 0;
      if (aData.size() >= 3 && static_cast<unsigned char>(aData[0]) == 0xEF
          && static_cast<unsigned char>(aData[1]) == 0xBB
          && static_cast<unsigned char>(aData[2]) == 0xBF)
        aStart = 3;
      while (aStart < aData.size()
             && (aData[aStart] == ' ' || aData[aStart] == '\t'
                 || aData[aStart] == '\r' || aData[aStart] == '\n'))
        ++aStart;
      const std::string_view aView(aData.data() + aStart, aData.size() - aStart);
      if (aView.rfind("<?xml", 0) == 0 && aView.find("format=\"XmlXCAF\"") != std::string_view::npos)
        return {{"format", "xcaf"}, {"encoding", "text"}, {"confidence", "exact"}};
      if (aView.rfind("ply\n", 0) == 0 || aView.rfind("ply\r\n", 0) == 0)
      {
        const std::size_t anEndHeader = aView.find("end_header");
        const std::string_view aHeader = aView.substr(0, anEndHeader);
        const bool isBinary = aHeader.find("format binary_") != std::string_view::npos;
        return {{"format", "ply"}, {"encoding", isBinary ? "binary" : "text"},
                {"confidence", "exact"}};
      }
      if (aView.rfind("#VRML V1.0", 0) == 0 || aView.rfind("#VRML V2.0", 0) == 0)
        return {{"format", "vrml"}, {"encoding", "text"}, {"confidence", "exact"}};
      if (aView.rfind("ISO-10303-21;", 0) == 0 && aView.find("HEADER;") != std::string_view::npos)
      {
        const bool hasDocumentMetadata =
          aView.find("NEXT_ASSEMBLY_USAGE_OCCURRENCE") != std::string_view::npos
          || aView.find("COLOUR_RGB") != std::string_view::npos
          || aView.find("DRAUGHTING_PRE_DEFINED_COLOUR") != std::string_view::npos
          || aView.find("PRESENTATION_LAYER_ASSIGNMENT") != std::string_view::npos
          || aView.find("INVISIBILITY") != std::string_view::npos;
        return {{"format", "step"}, {"encoding", "text"}, {"confidence", "exact"},
                {"documentMetadata", hasDocumentMetadata}};
      }
      if (aView.rfind("DBRep_DrawableShape", 0) == 0
          || aView.rfind("CASCADE Topology V", 0) == 0)
        return {{"format", "brep"}, {"encoding", "text"}, {"confidence", "exact"}};
      if (aData.size() >= 80 && aData[72] == 'S')
      {
        bool hasIgesSequence = true;
        for (std::size_t anIndex = 73; anIndex < 80; ++anIndex)
          hasIgesSequence = hasIgesSequence
            && (aData[anIndex] == ' ' || (aData[anIndex] >= '0' && aData[anIndex] <= '9'));
        if (hasIgesSequence)
          return {{"format", "iges"}, {"encoding", "text"}, {"confidence", "likely"}};
      }
      if (aView.rfind("solid", 0) == 0 && aView.find("facet normal") != std::string_view::npos
          && aView.find("endsolid") != std::string_view::npos)
        return {{"format", "stl"}, {"encoding", "text"}, {"confidence", "likely"}};

      const json aJson = !aView.empty() && aView.front() == '{'
        ? json::parse(aView.begin(), aView.end(), nullptr, false)
        : json(json::value_t::discarded);
      if (!aJson.is_discarded() && aJson.is_object() && aJson.contains("asset")
          && aJson.at("asset").is_object() && aJson.at("asset").contains("version")
          && aJson.at("asset").at("version").is_string()
          && aJson.at("asset").at("version").get<std::string>().rfind("2.", 0) == 0)
        return {{"format", "gltf"}, {"encoding", "json"}, {"confidence", "exact"}};

      bool hasObjVertex = false;
      bool hasObjFace = false;
      std::istringstream aStream{std::string(aView)};
      std::string aLine;
      while (std::getline(aStream, aLine) && !(hasObjVertex && hasObjFace))
      {
        std::istringstream aLineStream(aLine);
        std::string aCommand;
        aLineStream >> aCommand;
        hasObjVertex = hasObjVertex || aCommand == "v";
        hasObjFace = hasObjFace || aCommand == "f";
      }
      if (hasObjVertex && hasObjFace)
        return {{"format", "obj"}, {"encoding", "text"}, {"confidence", "likely"}};
      return {{"format", "unknown"}, {"confidence", "none"}};
    }
    if (theOp == "exportIGES")
    {
      std::vector<TopoDS_Shape> aShapes;
      if (theArgs.contains("shape"))
        aShapes.push_back(myArena.get(requiredU32(theArgs, "shape")));
      else if (theArgs.contains("shapes") && theArgs.at("shapes").is_array()
               && !theArgs.at("shapes").empty())
      {
        for (const json& aShape : theArgs.at("shapes"))
          aShapes.push_back(myArena.get(aShape.get<std::uint32_t>()));
      }
      else
        throw KernelFailure(ErrorCode::InvalidArgs, "IGES export requires shape or shapes");
      const auto aUnit = igesLengthUnit(theArgs);
      const std::string aModeName = theArgs.value("mode", "faces");
      int aMode = 0;
      if (aModeName == "brep")
      {
        aMode = 1;
      }
      else if (aModeName != "faces")
      {
        throw KernelFailure(ErrorCode::InvalidArgs, "Unsupported IGES write mode: " + aModeName);
      }
      IGESUnitGuard aUnitGuard(std::get<1>(aUnit), std::get<2>(aUnit));
      IGESControl_Writer aWriter(std::get<0>(aUnit).c_str(), aMode);
      occ::handle<Message_ProgressIndicator> aProgress = new HostProgressIndicator();
      Message_ProgressScope aProgressScope(
        aProgress->Start(), "IGES export", static_cast<double>(aShapes.size() + 1));
      aProgressScope.Next().Close();
      if (!aProgressScope.More())
        throw KernelFailure(ErrorCode::Cancelled, "OCCT IGES export cancelled");
      for (const TopoDS_Shape& aShape : aShapes)
      {
        const bool isTransferred = aWriter.AddShape(aShape, aProgressScope.Next());
        if (!aProgressScope.More())
          throw KernelFailure(ErrorCode::Cancelled, "OCCT IGES export cancelled");
        if (!isTransferred)
          throw KernelFailure(ErrorCode::ImportExportFailed, "OCCT IGES transfer failed");
      }
      std::ostringstream aStream(std::ios::out | std::ios::binary);
      if (!aWriter.Write(aStream))
      {
        throw KernelFailure(ErrorCode::ImportExportFailed, "OCCT IGES write failed");
      }
      const std::string aData = aStream.str();
      const std::uint32_t aBuffer = myBuffers.create(aData.size());
      if (!aData.empty())
      {
        std::memcpy(myBuffers.get(aBuffer).data(), aData.data(), aData.size());
      }
      return {{"data", bufferDescriptor(aBuffer, aData.size(), "iges-text")}};
    }
    if (theOp == "importIGES")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const auto aUnit = igesLengthUnit(theArgs);
      const std::string aData = inputBufferData(theArgs, "data");
      IGESUnitGuard aUnitGuard(std::get<1>(aUnit), std::get<2>(aUnit));
      std::istringstream aStream(aData, std::ios::in | std::ios::binary);
      IGESControl_Reader aReader;
      if (aReader.ReadStream("occt-worker", aStream) != IFSelect_RetDone)
      {
        throw KernelFailure(ErrorCode::ImportExportFailed, "IGES input could not be parsed");
      }
      occ::handle<Message_ProgressIndicator> aProgress = new HostProgressIndicator();
      Message_ProgressScope aProgressScope(aProgress->Start(), "IGES import", 2.0);
      aProgressScope.Next().Close();
      if (!aProgressScope.More())
        throw KernelFailure(ErrorCode::Cancelled, "OCCT IGES import cancelled");
      const int aRootCount = aReader.TransferRoots(aProgressScope.Next());
      if (!aProgressScope.More())
        throw KernelFailure(ErrorCode::Cancelled, "OCCT IGES import cancelled");
      if (aRootCount <= 0)
      {
        throw KernelFailure(ErrorCode::ImportExportFailed, "IGES input did not contain a transferable shape");
      }
      const TopoDS_Shape aShape = aReader.OneShape();
      if (aShape.IsNull())
      {
        throw KernelFailure(ErrorCode::ImportExportFailed, "IGES input did not contain a shape");
      }
      json aShapes = json::array();
      for (int anIndex = 1; anIndex <= aReader.NbShapes(); ++anIndex)
      {
        const TopoDS_Shape aRootShape = aReader.Shape(anIndex);
        if (!aRootShape.IsNull()) aShapes.push_back(myArena.add(aRootShape, aScope));
      }
      return {{"shape", myArena.add(aShape, aScope)}, {"shapes", std::move(aShapes)},
              {"rootCount", aRootCount}};
    }
    if (theOp == "exportSTEP")
    {
      std::vector<TopoDS_Shape> aShapes;
      if (theArgs.contains("shape"))
        aShapes.push_back(myArena.get(requiredU32(theArgs, "shape")));
      else if (theArgs.contains("shapes") && theArgs.at("shapes").is_array()
               && !theArgs.at("shapes").empty())
      {
        for (const json& aShape : theArgs.at("shapes"))
          aShapes.push_back(myArena.get(aShape.get<std::uint32_t>()));
      }
      else
        throw KernelFailure(ErrorCode::InvalidArgs, "STEP export requires shape or shapes");
      const auto aUnit = stepLengthUnit(theArgs);
      const std::string aTimestamp = theArgs.value("timestamp", "2026-01-01T00:00:00");
      STEPControl_Writer aWriter;
      DESTEP_Parameters aParams;
      aParams.WriteUnit = aUnit.first;
      aParams.WriteSchema = stepWriteSchema(theArgs);
      aParams.WriteProductName = "occt-worker";
      // Model coordinates and the written STEP file share the requested unit, so no scaling occurs.
      aWriter.Model(true)->SetLocalLengthUnit(aUnit.second);
      occ::handle<Message_ProgressIndicator> aProgress = new HostProgressIndicator();
      Message_ProgressScope aProgressScope(
        aProgress->Start(), "STEP export", static_cast<Standard_Real>(aShapes.size()));
      for (const TopoDS_Shape& aShape : aShapes)
      {
        const IFSelect_ReturnStatus aStatus = aWriter.Transfer(
          aShape, STEPControl_AsIs, aParams, true, aProgressScope.Next());
        if (!aProgressScope.More())
          throw KernelFailure(ErrorCode::Cancelled, "OCCT STEP export cancelled");
        if (aStatus != IFSelect_RetDone)
          throw KernelFailure(ErrorCode::ImportExportFailed, "OCCT STEP transfer failed");
      }
      APIHeaderSection_MakeHeader aHeader(aWriter.Model());
      aHeader.SetName(new TCollection_HAsciiString("occt-worker"));
      aHeader.SetTimeStamp(new TCollection_HAsciiString(aTimestamp.c_str()));
      std::ostringstream aStream;
      if (aWriter.WriteStream(aStream) != IFSelect_RetDone)
      {
        throw KernelFailure(ErrorCode::ImportExportFailed, "OCCT STEP write failed");
      }
      std::string aData = aStream.str();
      std::size_t aProductPosition = 0;
      std::size_t aProductIndex = 1;
      while ((aProductPosition = aData.find("PRODUCT('", aProductPosition)) != std::string::npos)
      {
        const std::size_t aFirstStart = aProductPosition + std::strlen("PRODUCT('");
        const std::size_t aFirstEnd = aData.find('\'', aFirstStart);
        const std::size_t aSecondQuote =
          aFirstEnd == std::string::npos ? std::string::npos : aData.find('\'', aFirstEnd + 1);
        const std::size_t aSecondStart =
          aSecondQuote == std::string::npos ? std::string::npos : aSecondQuote + 1;
        const std::size_t aSecondEnd =
          aSecondStart == std::string::npos ? std::string::npos : aData.find('\'', aSecondStart);
        if (aFirstEnd == std::string::npos || aSecondEnd == std::string::npos)
        {
          break;
        }
        const std::string aStableProductName =
          "occt-worker-" + std::to_string(aProductIndex++);
        aData.replace(aSecondStart, aSecondEnd - aSecondStart, aStableProductName);
        aData.replace(aFirstStart, aFirstEnd - aFirstStart, aStableProductName);
        aProductPosition = aFirstStart + aStableProductName.size();
      }
      const std::uint32_t aBuffer = myBuffers.create(aData.size());
      if (!aData.empty())
      {
        std::memcpy(myBuffers.get(aBuffer).data(), aData.data(), aData.size());
      }
      return {{"data", bufferDescriptor(aBuffer, aData.size(), "step-utf8")}};
    }
    if (theOp == "exportSTEPDocument" || theOp == "exportXCAF" || theOp == "exportIGESDocument")
    {
      if (!theArgs.contains("nodes") || !theArgs.at("nodes").is_array()
          || theArgs.at("nodes").empty())
        throw KernelFailure(ErrorCode::InvalidArgs, "STEP document requires at least one node");
      if (!theArgs.contains("roots") || !theArgs.at("roots").is_array()
          || theArgs.at("roots").empty())
        throw KernelFailure(ErrorCode::InvalidArgs, "STEP document requires at least one root");

      const json& aNodes = theArgs.at("nodes");
      const std::size_t aNodeCount = aNodes.size();
      if (theOp == "exportIGESDocument")
      {
        auto hasEntries = [&](const char* theKey) {
          return theArgs.contains(theKey) && theArgs.at(theKey).is_array()
                 && !theArgs.at(theKey).empty();
        };
        if (hasEntries("gdt") || hasEntries("datums") || hasEntries("geometricTolerances")
            || hasEntries("views") || hasEntries("shuo"))
          throw KernelFailure(ErrorCode::ImportExportFailed,
                              "IGES transfer cannot preserve XCAF semantic annotations, views, or SHUO");
      }
      std::vector<std::size_t> aParentCounts(aNodeCount, 0);
      std::vector<bool> isRoot(aNodeCount, false);
      for (const json& aRootValue : theArgs.at("roots"))
      {
        if (!aRootValue.is_number_unsigned() && !aRootValue.is_number_integer())
          throw KernelFailure(ErrorCode::InvalidArgs, "STEP document root indices must be integers");
        const std::int64_t aRoot = aRootValue.get<std::int64_t>();
        if (aRoot < 0 || static_cast<std::size_t>(aRoot) >= aNodeCount || isRoot[aRoot])
          throw KernelFailure(ErrorCode::InvalidArgs, "STEP document root index is invalid or duplicated");
        isRoot[aRoot] = true;
      }
      for (std::size_t aNodeIndex = 0; aNodeIndex < aNodeCount; ++aNodeIndex)
      {
        const json& aNode = aNodes[aNodeIndex];
        if (!aNode.is_object() || !aNode.contains("kind") || !aNode.at("kind").is_string())
          throw KernelFailure(ErrorCode::InvalidArgs, "STEP document node kind is required");
        const std::string aKind = aNode.at("kind").get<std::string>();
        const json aChildren = aNode.value("children", json::array());
        if (!aChildren.is_array())
          throw KernelFailure(ErrorCode::InvalidArgs, "STEP document node children must be an array");
        if (aKind == "part")
        {
          if (!aNode.contains("shape") || !aChildren.empty())
            throw KernelFailure(ErrorCode::InvalidArgs, "STEP part nodes require shape and cannot have children");
          myArena.get(requiredU32(aNode, "shape"));
        }
        else if (aKind == "assembly")
        {
          if (aChildren.empty())
            throw KernelFailure(ErrorCode::InvalidArgs, "STEP assembly nodes require children");
        }
        else
        {
          throw KernelFailure(ErrorCode::InvalidArgs, "STEP document node kind must be part or assembly");
        }
        for (const json& aChildValue : aChildren)
        {
          if (!aChildValue.is_number_unsigned() && !aChildValue.is_number_integer())
            throw KernelFailure(ErrorCode::InvalidArgs, "STEP document child indices must be integers");
          const std::int64_t aChild = aChildValue.get<std::int64_t>();
          if (aChild < 0 || static_cast<std::size_t>(aChild) >= aNodeCount
              || static_cast<std::size_t>(aChild) == aNodeIndex)
            throw KernelFailure(ErrorCode::InvalidArgs, "STEP document child index is invalid");
          ++aParentCounts[aChild];
        }
      }
      for (std::size_t aNodeIndex = 0; aNodeIndex < aNodeCount; ++aNodeIndex)
      {
        const bool shouldBeRoot = aParentCounts[aNodeIndex] == 0;
        if (isRoot[aNodeIndex] != shouldBeRoot || aParentCounts[aNodeIndex] > 1)
          throw KernelFailure(ErrorCode::InvalidArgs, "STEP document nodes must form rooted trees");
      }

      std::vector<int> aVisitState(aNodeCount, 0);
      std::function<void(std::size_t)> visitNode = [&](const std::size_t theNodeIndex) {
        if (aVisitState[theNodeIndex] == 1)
          throw KernelFailure(ErrorCode::InvalidArgs, "STEP document contains a cycle");
        if (aVisitState[theNodeIndex] == 2) return;
        aVisitState[theNodeIndex] = 1;
        for (const json& aChild : aNodes[theNodeIndex].value("children", json::array()))
          visitNode(aChild.get<std::size_t>());
        aVisitState[theNodeIndex] = 2;
      };
      for (std::size_t aNodeIndex = 0; aNodeIndex < aNodeCount; ++aNodeIndex)
        if (isRoot[aNodeIndex]) visitNode(aNodeIndex);
      for (const int aState : aVisitState)
        if (aState != 2)
          throw KernelFailure(ErrorCode::InvalidArgs, "STEP document contains nodes outside its rooted trees");

      auto nodeTransform = [](const json& theNode) {
        gp_Trsf aTransform;
        if (!theNode.contains("transform")) return aTransform;
        const json& aMatrix = theNode.at("transform");
        if (!aMatrix.is_array() || aMatrix.size() != 12)
          throw KernelFailure(ErrorCode::InvalidArgs, "STEP document transform must contain 12 numbers");
        for (const json& aValue : aMatrix)
          if (!aValue.is_number() || !std::isfinite(aValue.get<double>()))
            throw KernelFailure(ErrorCode::InvalidArgs, "STEP document transform must be finite");
        aTransform.SetValues(
          aMatrix[0].get<double>(), aMatrix[1].get<double>(), aMatrix[2].get<double>(), aMatrix[3].get<double>(),
          aMatrix[4].get<double>(), aMatrix[5].get<double>(), aMatrix[6].get<double>(), aMatrix[7].get<double>(),
          aMatrix[8].get<double>(), aMatrix[9].get<double>(), aMatrix[10].get<double>(), aMatrix[11].get<double>());
        return aTransform;
      };

      std::string aDocumentFormat = "BinXCAF";
      std::string anXCAFFormat;
      if (theOp == "exportXCAF")
      {
        anXCAFFormat = theArgs.value("format", "bin");
        if (anXCAFFormat == "xml")
          aDocumentFormat = "XmlXCAF";
        else if (anXCAFFormat != "bin")
          throw KernelFailure(ErrorCode::InvalidArgs, "XCAF format must be bin or xml");
      }
      const occ::handle<TDocStd_Document> aDocument =
        new TDocStd_Document(aDocumentFormat.c_str());
      XCAFDoc_DocumentTool::Set(aDocument->Main());
      const occ::handle<XCAFDoc_ShapeTool> aShapeTool =
        XCAFDoc_DocumentTool::ShapeTool(aDocument->Main());
      const occ::handle<XCAFDoc_ColorTool> aColorTool =
        XCAFDoc_DocumentTool::ColorTool(aDocument->Main());
      const occ::handle<XCAFDoc_LayerTool> aLayerTool =
        XCAFDoc_DocumentTool::LayerTool(aDocument->Main());
      const occ::handle<XCAFDoc_MaterialTool> aMaterialTool =
        XCAFDoc_DocumentTool::MaterialTool(aDocument->Main());
      const occ::handle<XCAFDoc_VisMaterialTool> aVisMaterialTool =
        XCAFDoc_DocumentTool::VisMaterialTool(aDocument->Main());
      const occ::handle<XCAFDoc_ClippingPlaneTool> aClippingPlaneTool =
        XCAFDoc_DocumentTool::ClippingPlaneTool(aDocument->Main());
      occ::handle<XCAFDoc_ViewTool> aViewTool;
      if (theArgs.contains("views"))
        aViewTool = XCAFDoc_DocumentTool::ViewTool(aDocument->Main());
      const occ::handle<XCAFDoc_DimTolTool> aDimTolTool =
        XCAFDoc_DocumentTool::DimTolTool(aDocument->Main());
      std::vector<TDF_Label> aLabels(aNodeCount);
      std::vector<TDF_Label> aComponentLabels(aNodeCount);

      auto applyMetadata = [&](const TDF_Label& theLabel, const json& theNode) {
        if (theNode.contains("visible") && !theNode.at("visible").is_boolean())
          throw KernelFailure(ErrorCode::InvalidArgs, "STEP document visibility must be boolean");
        if (theNode.contains("name"))
        {
          if (!theNode.at("name").is_string())
            throw KernelFailure(ErrorCode::InvalidArgs, "STEP document node name must be a string");
          const std::string aName = theNode.at("name").get<std::string>();
          TDataStd_Name::Set(theLabel, TCollection_ExtendedString(aName.c_str(), true));
        }
        if (theNode.contains("color"))
        {
          const json& aColor = theNode.at("color");
          if (!aColor.is_array() || aColor.size() != 4)
            throw KernelFailure(ErrorCode::InvalidArgs, "STEP document color must contain RGBA values");
          std::array<float, 4> aValues;
          for (std::size_t anIndex = 0; anIndex < aValues.size(); ++anIndex)
          {
            if (!aColor[anIndex].is_number())
              throw KernelFailure(ErrorCode::InvalidArgs, "STEP document color values must be numbers");
            const double aValue = aColor[anIndex].get<double>();
            if (!std::isfinite(aValue) || aValue < 0.0 || aValue > 1.0)
              throw KernelFailure(ErrorCode::InvalidArgs, "STEP document color values must be between zero and one");
            aValues[anIndex] = static_cast<float>(aValue);
          }
          aColorTool->SetColor(theLabel,
            Quantity_ColorRGBA(aValues[0], aValues[1], aValues[2], aValues[3]), XCAFDoc_ColorGen);
          if (theOp == "exportIGESDocument")
          {
            const Quantity_ColorRGBA aPresentationColor(aValues[0], aValues[1], aValues[2], aValues[3]);
            aColorTool->SetColor(theLabel, aPresentationColor, XCAFDoc_ColorSurf);
            aColorTool->SetColor(theLabel, aPresentationColor, XCAFDoc_ColorCurv);
            std::function<void(const TDF_Label&)> applySubshapeColor =
              [&](const TDF_Label& theCurrent) {
                NCollection_Sequence<TDF_Label> aSubshapeLabels;
                if (!XCAFDoc_ShapeTool::GetSubShapes(theCurrent, aSubshapeLabels)) return;
                for (int anIndex = 1; anIndex <= aSubshapeLabels.Size(); ++anIndex)
                {
                  const TDF_Label& aSubshapeLabel = aSubshapeLabels.Value(anIndex);
                  aColorTool->SetColor(aSubshapeLabel, aPresentationColor, XCAFDoc_ColorSurf);
                  aColorTool->SetColor(aSubshapeLabel, aPresentationColor, XCAFDoc_ColorCurv);
                  applySubshapeColor(aSubshapeLabel);
                }
              };
            applySubshapeColor(theLabel);
          }
        }
        if (theNode.contains("layers"))
        {
          if (!theNode.at("layers").is_array())
            throw KernelFailure(ErrorCode::InvalidArgs, "STEP document layers must be an array");
          for (const json& aLayer : theNode.at("layers"))
          {
            if (!aLayer.is_string())
              throw KernelFailure(ErrorCode::InvalidArgs, "STEP document layer names must be strings");
            const std::string aName = aLayer.get<std::string>();
            aLayerTool->SetLayer(theLabel, TCollection_ExtendedString(aName.c_str(), true));
            if (theOp == "exportIGESDocument")
            {
              const TopoDS_Shape aNodeShape = XCAFDoc_ShapeTool::GetShape(theLabel);
              if (!aNodeShape.IsNull())
                aLayerTool->SetLayer(aNodeShape, TCollection_ExtendedString(aName.c_str(), true));
            }
          }
        }
        if (theNode.contains("material"))
        {
          const json& aMaterial = theNode.at("material");
          if (!aMaterial.is_object() || !aMaterial.contains("name")
              || !aMaterial.at("name").is_string() || !aMaterial.contains("density")
              || !aMaterial.at("density").is_number())
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "STEP document material requires name and density");
          const double aDensity = aMaterial.at("density").get<double>();
          if (!std::isfinite(aDensity) || aDensity < 0.0)
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "STEP document material density must be finite and non-negative");
          auto materialString = [&](const char* theKey, const char* theDefault) {
            if (!aMaterial.contains(theKey))
              return occ::handle<TCollection_HAsciiString>(new TCollection_HAsciiString(theDefault));
            if (!aMaterial.at(theKey).is_string())
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  std::string("STEP document material ") + theKey + " must be a string");
            return occ::handle<TCollection_HAsciiString>(
              new TCollection_HAsciiString(aMaterial.at(theKey).get<std::string>().c_str()));
          };
          const auto aName = materialString("name", "");
          const auto aDescription = materialString("description", "");
          const auto aDensityName = materialString("densityName", "kg/m^3");
          const auto aDensityValueType = materialString("densityValueType", "mass density");
          aMaterialTool->SetMaterial(theLabel, aName, aDescription, aDensity,
                                     aDensityName, aDensityValueType);
        }
        if (theNode.contains("visualMaterial"))
        {
          const json& aMaterial = theNode.at("visualMaterial");
          if (!aMaterial.is_object() || !aMaterial.contains("name")
              || !aMaterial.at("name").is_string())
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "STEP document visual material requires a name");

          XCAFDoc_VisMaterialPBR aPbr;
          if (aMaterial.contains("baseColor"))
          {
            const json& aColor = aMaterial.at("baseColor");
            if (!aColor.is_array() || aColor.size() != 4)
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "STEP document visual material baseColor must contain RGBA values");
            std::array<float, 4> aValues;
            for (std::size_t anIndex = 0; anIndex < aValues.size(); ++anIndex)
            {
              if (!aColor[anIndex].is_number())
                throw KernelFailure(ErrorCode::InvalidArgs,
                                    "STEP document visual material baseColor values must be numbers");
              const double aValue = aColor[anIndex].get<double>();
              if (!std::isfinite(aValue) || aValue < 0.0 || aValue > 1.0)
                throw KernelFailure(ErrorCode::InvalidArgs,
                                    "STEP document visual material baseColor values must be between zero and one");
              aValues[anIndex] = static_cast<float>(aValue);
            }
            aPbr.BaseColor = Quantity_ColorRGBA(aValues[0], aValues[1], aValues[2], aValues[3]);
          }
          auto unitValue = [&](const char* theKey, const float theDefault) {
            if (!aMaterial.contains(theKey)) return theDefault;
            if (!aMaterial.at(theKey).is_number())
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  std::string("STEP document visual material ") + theKey
                                  + " must be a number");
            const double aValue = aMaterial.at(theKey).get<double>();
            if (!std::isfinite(aValue) || aValue < 0.0 || aValue > 1.0)
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  std::string("STEP document visual material ") + theKey
                                  + " must be between zero and one");
            return static_cast<float>(aValue);
          };
          aPbr.Metallic = unitValue("metallic", aPbr.Metallic);
          aPbr.Roughness = unitValue("roughness", aPbr.Roughness);
          if (aMaterial.contains("emissive"))
          {
            const json& anEmissive = aMaterial.at("emissive");
            if (!anEmissive.is_array() || anEmissive.size() != 3)
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "STEP document visual material emissive must contain RGB values");
            std::array<float, 3> aValues;
            for (std::size_t anIndex = 0; anIndex < aValues.size(); ++anIndex)
            {
              if (!anEmissive[anIndex].is_number())
                throw KernelFailure(ErrorCode::InvalidArgs,
                                    "STEP document visual material emissive values must be numbers");
              const double aValue = anEmissive[anIndex].get<double>();
              if (!std::isfinite(aValue) || aValue < 0.0 || aValue > 1.0)
                throw KernelFailure(ErrorCode::InvalidArgs,
                                    "STEP document visual material emissive values must be between zero and one");
              aValues[anIndex] = static_cast<float>(aValue);
            }
            aPbr.EmissiveFactor = NCollection_Vec3<float>(aValues[0], aValues[1], aValues[2]);
          }
          if (aMaterial.contains("refractionIndex"))
          {
            if (!aMaterial.at("refractionIndex").is_number())
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "STEP document visual material refractionIndex must be a number");
            const double aValue = aMaterial.at("refractionIndex").get<double>();
            if (!std::isfinite(aValue) || aValue < 1.0 || aValue > 3.0)
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "STEP document visual material refractionIndex must be between one and three");
            aPbr.RefractionIndex = static_cast<float>(aValue);
          }
          const occ::handle<XCAFDoc_VisMaterial> aVisMaterial = new XCAFDoc_VisMaterial();
          aVisMaterial->SetPbrMaterial(aPbr);
          const TDF_Label aMaterialLabel = aVisMaterialTool->AddMaterial(
            aVisMaterial, TCollection_AsciiString(aMaterial.at("name").get<std::string>().c_str()));
          aVisMaterialTool->SetShapeMaterial(theLabel, aMaterialLabel);
        }
        if (theNode.contains("validationProperties"))
        {
          const json& aProperties = theNode.at("validationProperties");
          if (!aProperties.is_object())
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "STEP document validationProperties must be an object");
          auto nonNegativeProperty = [&](const char* theKey) {
            if (!aProperties.at(theKey).is_number())
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  std::string("STEP document ") + theKey + " must be a number");
            const double aValue = aProperties.at(theKey).get<double>();
            if (!std::isfinite(aValue) || aValue < 0.0)
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  std::string("STEP document ") + theKey
                                  + " must be finite and non-negative");
            return aValue;
          };
          if (aProperties.contains("area"))
            XCAFDoc_Area::Set(theLabel, nonNegativeProperty("area"));
          if (aProperties.contains("volume"))
            XCAFDoc_Volume::Set(theLabel, nonNegativeProperty("volume"));
          if (aProperties.contains("centroid"))
          {
            const json& aCentroid = aProperties.at("centroid");
            if (!aCentroid.is_array() || aCentroid.size() != 3)
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "STEP document centroid must contain three numbers");
            std::array<double, 3> aValues;
            for (std::size_t anIndex = 0; anIndex < aValues.size(); ++anIndex)
            {
              if (!aCentroid[anIndex].is_number())
                throw KernelFailure(ErrorCode::InvalidArgs,
                                    "STEP document centroid values must be numbers");
              aValues[anIndex] = aCentroid[anIndex].get<double>();
              if (!std::isfinite(aValues[anIndex]))
                throw KernelFailure(ErrorCode::InvalidArgs,
                                    "STEP document centroid values must be finite");
            }
            XCAFDoc_Centroid::Set(theLabel, gp_Pnt(aValues[0], aValues[1], aValues[2]));
          }
        }
        aColorTool->SetVisibility(theLabel, theNode.value("visible", true));
      };

      auto applySubshapeStyles = [&](const TDF_Label& theLabel, const json& theNode) {
        if (!theNode.contains("subshapeStyles")) return;
        const json& aStyles = theNode.at("subshapeStyles");
        if (!aStyles.is_array())
          throw KernelFailure(ErrorCode::InvalidArgs,
                              "STEP document subshapeStyles must be an array");
        if (theNode.at("kind") != "part" && !aStyles.empty())
          throw KernelFailure(ErrorCode::InvalidArgs,
                              "STEP document subshapeStyles are only valid on part nodes");

        const TopoDS_Shape aShape = XCAFDoc_ShapeTool::GetShape(theLabel);
        std::unordered_set<std::string> aStyledSubshapes;
        for (const json& aStyle : aStyles)
        {
          if (!aStyle.is_object() || !aStyle.contains("topology")
              || !aStyle.at("topology").is_string() || !aStyle.contains("index")
              || (!aStyle.at("index").is_number_unsigned()
                  && !aStyle.at("index").is_number_integer())
              || !aStyle.contains("color"))
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "STEP document subshape style requires topology, index, and color");
          const std::string aTopology = aStyle.at("topology").get<std::string>();
          if (aTopology != "face" && aTopology != "edge")
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "STEP document subshape style topology must be face or edge");
          const std::int64_t anIndex = aStyle.at("index").get<std::int64_t>();
          if (anIndex < 0)
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "STEP document subshape style index must be non-negative");
          const std::string aKey = aTopology + ":" + std::to_string(anIndex);
          if (!aStyledSubshapes.insert(aKey).second)
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "STEP document subshape styles must not contain duplicates");

          const json& aColor = aStyle.at("color");
          if (!aColor.is_array() || aColor.size() != 4)
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "STEP document subshape style color must contain RGBA values");
          std::array<float, 4> aValues;
          for (std::size_t aColorIndex = 0; aColorIndex < aValues.size(); ++aColorIndex)
          {
            if (!aColor[aColorIndex].is_number())
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "STEP document subshape style color values must be numbers");
            const double aValue = aColor[aColorIndex].get<double>();
            if (!std::isfinite(aValue) || aValue < 0.0 || aValue > 1.0)
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "STEP document subshape style color values must be between zero and one");
            aValues[aColorIndex] = static_cast<float>(aValue);
          }

          TopTools_IndexedMapOfShape aSubshapes;
          const TopAbs_ShapeEnum aShapeType = aTopology == "face" ? TopAbs_FACE : TopAbs_EDGE;
          TopExp::MapShapes(aShape, aShapeType, aSubshapes);
          if (anIndex >= aSubshapes.Extent())
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "STEP document subshape style index is out of range");
          const TDF_Label aSubshapeLabel =
            aShapeTool->AddSubShape(theLabel, aSubshapes(static_cast<int>(anIndex + 1)));
          if (aSubshapeLabel.IsNull())
            throw KernelFailure(ErrorCode::ImportExportFailed,
                                "OCCT could not create an XCAF subshape label");
          aColorTool->SetColor(
            aSubshapeLabel,
            Quantity_ColorRGBA(aValues[0], aValues[1], aValues[2], aValues[3]),
            aTopology == "face" ? XCAFDoc_ColorSurf : XCAFDoc_ColorCurv);
        }
      };

      for (std::size_t aNodeIndex = 0; aNodeIndex < aNodeCount; ++aNodeIndex)
      {
        const json& aNode = aNodes[aNodeIndex];
        aLabels[aNodeIndex] = aShapeTool->NewShape();
        if (aNode.at("kind") == "part")
        {
          aShapeTool->SetShape(aLabels[aNodeIndex], myArena.get(requiredU32(aNode, "shape")));
        }
        else
        {
          BRep_Builder aBuilder;
          TopoDS_Compound anAssembly;
          aBuilder.MakeCompound(anAssembly);
          aShapeTool->SetShape(aLabels[aNodeIndex], anAssembly);
        }
        applyMetadata(aLabels[aNodeIndex], aNode);
        applySubshapeStyles(aLabels[aNodeIndex], aNode);
      }
      for (std::size_t aNodeIndex = 0; aNodeIndex < aNodeCount; ++aNodeIndex)
      {
        if (aNodes[aNodeIndex].at("kind") != "assembly") continue;
        for (const json& aChildValue : aNodes[aNodeIndex].value("children", json::array()))
        {
          const std::size_t aChild = aChildValue.get<std::size_t>();
          const TDF_Label aComponent = aShapeTool->AddComponent(
            aLabels[aNodeIndex], aLabels[aChild], TopLoc_Location(nodeTransform(aNodes[aChild])));
          if (aComponent.IsNull())
            throw KernelFailure(ErrorCode::ImportExportFailed, "OCCT could not create STEP assembly component");
          applyMetadata(aComponent, aNodes[aChild]);
          aComponentLabels[aChild] = aComponent;
        }
      }

      auto finiteXCAFVec3 = [&](const json& theValue, const char* theName) {
        if (!theValue.is_array() || theValue.size() != 3)
          throw KernelFailure(ErrorCode::InvalidArgs,
                              std::string("XCAF ") + theName + " must contain three numbers");
        std::array<double, 3> aResult;
        for (std::size_t anIndex = 0; anIndex < 3; ++anIndex)
        {
          if (!theValue[anIndex].is_number()
              || !std::isfinite(theValue[anIndex].get<double>()))
            throw KernelFailure(ErrorCode::InvalidArgs,
                                std::string("XCAF ") + theName + " must be finite");
          aResult[anIndex] = theValue[anIndex].get<double>();
        }
        return aResult;
      };
      if (theArgs.contains("views"))
      {
        const json& aViews = theArgs.at("views");
        if (!aViews.is_array())
          throw KernelFailure(ErrorCode::InvalidArgs, "XCAF views must be an array");
        for (const json& aView : aViews)
        {
          if (!aView.is_object() || !aView.contains("name") || !aView.at("name").is_string()
              || !aView.contains("viewDirection") || !aView.contains("upDirection")
              || !aView.contains("nodeIndices") || !aView.at("nodeIndices").is_array()
              || aView.at("nodeIndices").empty())
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "XCAF view requires name, directions, and nodeIndices");
          const auto aDirection = finiteXCAFVec3(aView.at("viewDirection"), "viewDirection");
          const auto anUp = finiteXCAFVec3(aView.at("upDirection"), "upDirection");
          if (aDirection[0] * aDirection[0] + aDirection[1] * aDirection[1]
                + aDirection[2] * aDirection[2] < 1e-24
              || anUp[0] * anUp[0] + anUp[1] * anUp[1] + anUp[2] * anUp[2] < 1e-24)
            throw KernelFailure(ErrorCode::InvalidArgs, "XCAF view directions must be non-zero");
          const std::string aProjection = aView.value("projection", "none");
          XCAFView_ProjectionType aProjectionType = XCAFView_ProjectionType_NoCamera;
          if (aProjection == "parallel") aProjectionType = XCAFView_ProjectionType_Parallel;
          else if (aProjection == "central") aProjectionType = XCAFView_ProjectionType_Central;
          else if (aProjection != "none")
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "XCAF view projection must be none, parallel, or central");
          auto positiveViewValue = [&](const char* theKey, double theDefault) {
            if (!aView.contains(theKey)) return theDefault;
            if (!aView.at(theKey).is_number())
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  std::string("XCAF view ") + theKey + " must be a number");
            const double aValue = aView.at(theKey).get<double>();
            if (!std::isfinite(aValue) || aValue <= 0.0)
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  std::string("XCAF view ") + theKey + " must be positive");
            return aValue;
          };
          const occ::handle<XCAFView_Object> aViewObject = new XCAFView_Object();
          aViewObject->SetName(new TCollection_HAsciiString(aView.at("name").get<std::string>().c_str()));
          aViewObject->SetType(aProjectionType);
          if (aView.contains("projectionPoint"))
          {
            const auto aPoint = finiteXCAFVec3(aView.at("projectionPoint"), "projectionPoint");
            aViewObject->SetProjectionPoint(gp_Pnt(aPoint[0], aPoint[1], aPoint[2]));
          }
          aViewObject->SetViewDirection(gp_Dir(aDirection[0], aDirection[1], aDirection[2]));
          aViewObject->SetUpDirection(gp_Dir(anUp[0], anUp[1], anUp[2]));
          aViewObject->SetZoomFactor(positiveViewValue("zoomFactor", 1.0));
          aViewObject->SetWindowHorizontalSize(positiveViewValue("windowHorizontalSize", 1.0));
          aViewObject->SetWindowVerticalSize(positiveViewValue("windowVerticalSize", 1.0));
          if (aView.contains("frontPlaneDistance"))
            aViewObject->SetFrontPlaneDistance(positiveViewValue("frontPlaneDistance", 1.0));
          if (aView.contains("backPlaneDistance"))
            aViewObject->SetBackPlaneDistance(positiveViewValue("backPlaneDistance", 1.0));
          if (aView.contains("viewVolumeSidesClipping")
              && !aView.at("viewVolumeSidesClipping").is_boolean())
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "XCAF view viewVolumeSidesClipping must be boolean");
          aViewObject->SetViewVolumeSidesClipping(aView.value("viewVolumeSidesClipping", false));
          NCollection_Sequence<TDF_Label> aViewShapes;
          for (const json& aNodeValue : aView.at("nodeIndices"))
          {
            if ((!aNodeValue.is_number_unsigned() && !aNodeValue.is_number_integer())
                || aNodeValue.get<std::int64_t>() < 0
                || static_cast<std::size_t>(aNodeValue.get<std::uint64_t>()) >= aNodeCount)
              throw KernelFailure(ErrorCode::InvalidArgs, "XCAF view node index is invalid");
            aViewShapes.Append(aLabels[aNodeValue.get<std::size_t>()]);
          }
          NCollection_Sequence<TDF_Label> aViewClippingPlanes;
          if (aView.contains("clippingPlanes"))
          {
            const json& aClippingPlanes = aView.at("clippingPlanes");
            if (!aClippingPlanes.is_array())
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "XCAF view clippingPlanes must be an array");
            for (const json& aClippingPlane : aClippingPlanes)
            {
              if (!aClippingPlane.is_object() || !aClippingPlane.contains("name")
                  || !aClippingPlane.at("name").is_string()
                  || !aClippingPlane.contains("origin")
                  || !aClippingPlane.contains("normal"))
                throw KernelFailure(ErrorCode::InvalidArgs,
                                    "XCAF clipping plane requires name, origin, and normal");
              const auto anOrigin = finiteXCAFVec3(aClippingPlane.at("origin"), "clippingPlane origin");
              const auto aNormal = finiteXCAFVec3(aClippingPlane.at("normal"), "clippingPlane normal");
              if (aNormal[0] * aNormal[0] + aNormal[1] * aNormal[1]
                    + aNormal[2] * aNormal[2] < 1e-24)
                throw KernelFailure(ErrorCode::InvalidArgs,
                                    "XCAF clipping plane normal must be non-zero");
              if (aClippingPlane.contains("capping")
                  && !aClippingPlane.at("capping").is_boolean())
                throw KernelFailure(ErrorCode::InvalidArgs,
                                    "XCAF clipping plane capping must be boolean");
              const bool aCapping = aClippingPlane.value("capping", false);
              const TCollection_ExtendedString aName(
                aClippingPlane.at("name").get<std::string>().c_str(), true);
              aViewClippingPlanes.Append(aClippingPlaneTool->AddClippingPlane(
                gp_Pln(gp_Pnt(anOrigin[0], anOrigin[1], anOrigin[2]),
                       gp_Dir(aNormal[0], aNormal[1], aNormal[2])),
                aName, aCapping));
            }
          }
          const TDF_Label aViewLabel = aViewTool->AddView();
          XCAFDoc_View::Set(aViewLabel)->SetObject(aViewObject);
          aViewTool->SetView(aViewShapes, NCollection_Sequence<TDF_Label>(),
                             aViewClippingPlanes, aViewLabel);
        }
      }
      if (theArgs.contains("shuo"))
      {
        const json& aShuo = theArgs.at("shuo");
        if (!aShuo.is_array())
          throw KernelFailure(ErrorCode::InvalidArgs, "XCAF shuo must be an array");
        std::unordered_set<std::string> aPaths;
        for (const json& aDefinition : aShuo)
        {
          if (!aDefinition.is_object() || !aDefinition.contains("nodeIndices")
              || !aDefinition.at("nodeIndices").is_array()
              || aDefinition.at("nodeIndices").size() < 2)
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "XCAF shuo requires at least two nodeIndices");
          NCollection_Sequence<TDF_Label> aPath;
          std::string aPathKey;
          std::size_t aPrevious = 0;
          bool isFirst = true;
          for (const json& aNodeValue : aDefinition.at("nodeIndices"))
          {
            if ((!aNodeValue.is_number_unsigned() && !aNodeValue.is_number_integer())
                || aNodeValue.get<std::int64_t>() < 0
                || static_cast<std::size_t>(aNodeValue.get<std::uint64_t>()) >= aNodeCount)
              throw KernelFailure(ErrorCode::InvalidArgs, "XCAF shuo node index is invalid");
            const std::size_t aNode = aNodeValue.get<std::size_t>();
            if (!isFirst)
            {
              bool isChild = false;
              for (const json& aChild : aNodes[aPrevious].value("children", json::array()))
                if (aChild.get<std::size_t>() == aNode) isChild = true;
              if (!isChild)
                throw KernelFailure(ErrorCode::InvalidArgs,
                                    "XCAF shuo nodeIndices must follow assembly components");
            }
            isFirst = false;
            if (aComponentLabels[aNode].IsNull())
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "XCAF shuo nodeIndices must identify component occurrences");
            if (!aPathKey.empty()) aPathKey += "/";
            aPathKey += std::to_string(aNode);
            aPath.Append(aComponentLabels[aNode]);
            aPrevious = aNode;
          }
          if (!aPaths.insert(aPathKey).second)
            throw KernelFailure(ErrorCode::InvalidArgs, "XCAF shuo paths must be unique");
          occ::handle<XCAFDoc_GraphNode> aMainShuo;
          if (!aShapeTool->SetSHUO(aPath, aMainShuo) || aMainShuo.IsNull())
            throw KernelFailure(ErrorCode::ImportExportFailed, "OCCT could not create XCAF SHUO");
          if (aDefinition.contains("color"))
          {
            const json& aColor = aDefinition.at("color");
            if (!aColor.is_array() || aColor.size() != 4)
              throw KernelFailure(ErrorCode::InvalidArgs, "XCAF SHUO color must contain RGBA values");
            std::array<float, 4> aValues;
            for (std::size_t anIndex = 0; anIndex < aValues.size(); ++anIndex)
            {
              if (!aColor[anIndex].is_number())
                throw KernelFailure(ErrorCode::InvalidArgs, "XCAF SHUO color values must be numbers");
              const double aValue = aColor[anIndex].get<double>();
              if (!std::isfinite(aValue) || aValue < 0.0 || aValue > 1.0)
                throw KernelFailure(ErrorCode::InvalidArgs, "XCAF SHUO color values must be between zero and one");
              aValues[anIndex] = static_cast<float>(aValue);
            }
            aColorTool->SetColor(aMainShuo->Label(),
                                 Quantity_ColorRGBA(aValues[0], aValues[1], aValues[2], aValues[3]),
                                 XCAFDoc_ColorGen);
          }
        }
      }
      aShapeTool->UpdateAssemblies();
      for (std::size_t aNodeIndex = 0; aNodeIndex < aNodeCount; ++aNodeIndex)
      {
        if (!isRoot[aNodeIndex] || !aNodes[aNodeIndex].contains("transform")) continue;
        const gp_Trsf aTransform = nodeTransform(aNodes[aNodeIndex]);
        if (aTransform.Form() == gp_Identity) continue;
        TDF_Label aLocatedRoot;
        if (!aShapeTool->SetLocation(aLabels[aNodeIndex], TopLoc_Location(aTransform), aLocatedRoot))
          throw KernelFailure(ErrorCode::ImportExportFailed, "OCCT could not locate STEP document root");
        aLabels[aNodeIndex] = aLocatedRoot;
        applyMetadata(aLocatedRoot, aNodes[aNodeIndex]);
      }

      auto gdtTypeFromName = [](const std::string& theType,
                                XCAFDimTolObjects_DimensionType& theResult) {
        if (theType == "curveLength") theResult = XCAFDimTolObjects_DimensionType_Size_CurveLength;
        else if (theType == "diameter") theResult = XCAFDimTolObjects_DimensionType_Size_Diameter;
        else if (theType == "sphericalDiameter") theResult = XCAFDimTolObjects_DimensionType_Size_SphericalDiameter;
        else if (theType == "radius") theResult = XCAFDimTolObjects_DimensionType_Size_Radius;
        else if (theType == "sphericalRadius") theResult = XCAFDimTolObjects_DimensionType_Size_SphericalRadius;
        else if (theType == "toroidalMinorDiameter") theResult = XCAFDimTolObjects_DimensionType_Size_ToroidalMinorDiameter;
        else if (theType == "toroidalMajorDiameter") theResult = XCAFDimTolObjects_DimensionType_Size_ToroidalMajorDiameter;
        else if (theType == "toroidalMinorRadius") theResult = XCAFDimTolObjects_DimensionType_Size_ToroidalMinorRadius;
        else if (theType == "toroidalMajorRadius") theResult = XCAFDimTolObjects_DimensionType_Size_ToroidalMajorRadius;
        else if (theType == "toroidalHighMajorDiameter") theResult = XCAFDimTolObjects_DimensionType_Size_ToroidalHighMajorDiameter;
        else if (theType == "toroidalLowMajorDiameter") theResult = XCAFDimTolObjects_DimensionType_Size_ToroidalLowMajorDiameter;
        else if (theType == "toroidalHighMajorRadius") theResult = XCAFDimTolObjects_DimensionType_Size_ToroidalHighMajorRadius;
        else if (theType == "toroidalLowMajorRadius") theResult = XCAFDimTolObjects_DimensionType_Size_ToroidalLowMajorRadius;
        else if (theType == "thickness") theResult = XCAFDimTolObjects_DimensionType_Size_Thickness;
        else if (theType == "angular") theResult = XCAFDimTolObjects_DimensionType_Size_Angular;
        else return false;
        return true;
      };
      auto gdtTypeName = [](const XCAFDimTolObjects_DimensionType theType) -> const char* {
        switch (theType)
        {
          case XCAFDimTolObjects_DimensionType_Size_CurveLength: return "curveLength";
          case XCAFDimTolObjects_DimensionType_Size_Diameter: return "diameter";
          case XCAFDimTolObjects_DimensionType_Size_SphericalDiameter: return "sphericalDiameter";
          case XCAFDimTolObjects_DimensionType_Size_Radius: return "radius";
          case XCAFDimTolObjects_DimensionType_Size_SphericalRadius: return "sphericalRadius";
          case XCAFDimTolObjects_DimensionType_Size_ToroidalMinorDiameter: return "toroidalMinorDiameter";
          case XCAFDimTolObjects_DimensionType_Size_ToroidalMajorDiameter: return "toroidalMajorDiameter";
          case XCAFDimTolObjects_DimensionType_Size_ToroidalMinorRadius: return "toroidalMinorRadius";
          case XCAFDimTolObjects_DimensionType_Size_ToroidalMajorRadius: return "toroidalMajorRadius";
          case XCAFDimTolObjects_DimensionType_Size_ToroidalHighMajorDiameter: return "toroidalHighMajorDiameter";
          case XCAFDimTolObjects_DimensionType_Size_ToroidalLowMajorDiameter: return "toroidalLowMajorDiameter";
          case XCAFDimTolObjects_DimensionType_Size_ToroidalHighMajorRadius: return "toroidalHighMajorRadius";
          case XCAFDimTolObjects_DimensionType_Size_ToroidalLowMajorRadius: return "toroidalLowMajorRadius";
          case XCAFDimTolObjects_DimensionType_Size_Thickness: return "thickness";
          case XCAFDimTolObjects_DimensionType_Size_Angular: return "angular";
          default: return nullptr;
        }
      };
      auto geomToleranceTypeFromName = [](const std::string& theType,
                                          XCAFDimTolObjects_GeomToleranceType& theResult) {
        if (theType == "angularity") theResult = XCAFDimTolObjects_GeomToleranceType_Angularity;
        else if (theType == "circularRunout") theResult = XCAFDimTolObjects_GeomToleranceType_CircularRunout;
        else if (theType == "circularity") theResult = XCAFDimTolObjects_GeomToleranceType_CircularityOrRoundness;
        else if (theType == "coaxiality") theResult = XCAFDimTolObjects_GeomToleranceType_Coaxiality;
        else if (theType == "concentricity") theResult = XCAFDimTolObjects_GeomToleranceType_Concentricity;
        else if (theType == "cylindricity") theResult = XCAFDimTolObjects_GeomToleranceType_Cylindricity;
        else if (theType == "flatness") theResult = XCAFDimTolObjects_GeomToleranceType_Flatness;
        else if (theType == "parallelism") theResult = XCAFDimTolObjects_GeomToleranceType_Parallelism;
        else if (theType == "perpendicularity") theResult = XCAFDimTolObjects_GeomToleranceType_Perpendicularity;
        else if (theType == "position") theResult = XCAFDimTolObjects_GeomToleranceType_Position;
        else if (theType == "profileOfLine") theResult = XCAFDimTolObjects_GeomToleranceType_ProfileOfLine;
        else if (theType == "profileOfSurface") theResult = XCAFDimTolObjects_GeomToleranceType_ProfileOfSurface;
        else if (theType == "straightness") theResult = XCAFDimTolObjects_GeomToleranceType_Straightness;
        else if (theType == "symmetry") theResult = XCAFDimTolObjects_GeomToleranceType_Symmetry;
        else if (theType == "totalRunout") theResult = XCAFDimTolObjects_GeomToleranceType_TotalRunout;
        else return false;
        return true;
      };
      auto geomToleranceValueTypeFromName = [](const std::string& theType,
                                               XCAFDimTolObjects_GeomToleranceTypeValue& theResult) {
        if (theType == "none") theResult = XCAFDimTolObjects_GeomToleranceTypeValue_None;
        else if (theType == "diameter") theResult = XCAFDimTolObjects_GeomToleranceTypeValue_Diameter;
        else if (theType == "sphericalDiameter") theResult = XCAFDimTolObjects_GeomToleranceTypeValue_SphericalDiameter;
        else return false;
        return true;
      };
      auto applyAnnotationPresentation = [&](const json& theItem, const auto& theObject,
                                             const char* theKind) {
        if (!theItem.contains("presentation")) return;
        const json& aPresentation = theItem.at("presentation");
        if (!aPresentation.is_object() || !aPresentation.contains("shape")
            || (!aPresentation.at("shape").is_number_unsigned()
                && !aPresentation.at("shape").is_number_integer())
            || (aPresentation.contains("name") && !aPresentation.at("name").is_string()))
          throw KernelFailure(ErrorCode::InvalidArgs,
                              std::string("XCAF ") + theKind
                              + " presentation requires a shape and optional name");
        occ::handle<TCollection_HAsciiString> aName;
        if (aPresentation.contains("name"))
          aName = new TCollection_HAsciiString(
            aPresentation.at("name").get<std::string>().c_str());
        theObject->SetPresentation(
          myArena.get(requiredU32(aPresentation, "shape")), aName);
      };
      if (theArgs.contains("gdt"))
      {
        const json& aGdt = theArgs.at("gdt");
        if (!aGdt.is_array())
          throw KernelFailure(ErrorCode::InvalidArgs, "STEP document gdt must be an array");
        for (const json& anItem : aGdt)
        {
          if (!anItem.is_object() || !anItem.contains("node") || !anItem.at("node").is_number_integer()
              || !anItem.contains("type") || !anItem.at("type").is_string()
              || !anItem.contains("value") || !anItem.at("value").is_number())
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "STEP document gdt requires a supported type, node, and value");
          XCAFDimTolObjects_DimensionType aType;
          if (!gdtTypeFromName(anItem.at("type").get<std::string>(), aType))
            throw KernelFailure(ErrorCode::InvalidArgs, "STEP document gdt type is unsupported");
          const std::int64_t aNode = anItem.at("node").get<std::int64_t>();
          const double aValue = anItem.at("value").get<double>();
          if (aNode < 0 || static_cast<std::size_t>(aNode) >= aNodeCount
              || aNodes[static_cast<std::size_t>(aNode)].at("kind") != "part"
              || !std::isfinite(aValue) || aValue <= 0.0)
            throw KernelFailure(ErrorCode::InvalidArgs, "STEP document gdt entry is invalid");
          const auto aDimension = new XCAFDimTolObjects_DimensionObject();
          aDimension->SetType(aType);
          aDimension->SetValue(aValue);
          if (anItem.contains("semanticName"))
          {
            if (!anItem.at("semanticName").is_string())
              throw KernelFailure(ErrorCode::InvalidArgs, "STEP document gdt semanticName must be a string");
            aDimension->SetSemanticName(new TCollection_HAsciiString(
              anItem.at("semanticName").get<std::string>().c_str()));
          }
          applyAnnotationPresentation(anItem, aDimension, "dimension");
          const TDF_Label aDimensionLabel = aDimTolTool->AddDimension();
          XCAFDoc_Dimension::Set(aDimensionLabel)->SetObject(aDimension);
          aDimTolTool->SetDimension(aLabels[static_cast<std::size_t>(aNode)], aDimensionLabel);
        }
      }

      std::vector<TDF_Label> aInputDatumLabels;
      if (theArgs.contains("datums"))
      {
        const json& aDatums = theArgs.at("datums");
        if (!aDatums.is_array())
          throw KernelFailure(ErrorCode::InvalidArgs, "STEP document datums must be an array");
          if (theOp == "exportSTEPDocument" && !aDatums.empty())
          throw KernelFailure(ErrorCode::ImportExportFailed,
                              "STEP transfer cannot preserve XCAF datums; use exportXCAF");
        for (const json& anItem : aDatums)
        {
          if (!anItem.is_object() || !anItem.contains("node")
              || !anItem.at("node").is_number_integer()
              || !anItem.contains("name") || !anItem.at("name").is_string()
              || !anItem.contains("identification") || !anItem.at("identification").is_string())
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "STEP document datum requires node, name, and identification");
          const std::int64_t aNode = anItem.at("node").get<std::int64_t>();
          if (aNode < 0 || static_cast<std::size_t>(aNode) >= aNodeCount
              || aNodes[static_cast<std::size_t>(aNode)].at("kind") != "part")
            throw KernelFailure(ErrorCode::InvalidArgs, "STEP document datum node is invalid");
          auto datumString = [&](const char* theKey, const char* theDefault) {
            if (!anItem.contains(theKey))
              return occ::handle<TCollection_HAsciiString>(new TCollection_HAsciiString(theDefault));
            if (!anItem.at(theKey).is_string())
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  std::string("STEP document datum ") + theKey + " must be a string");
            return occ::handle<TCollection_HAsciiString>(
              new TCollection_HAsciiString(anItem.at(theKey).get<std::string>().c_str()));
          };
          const auto aName = datumString("name", "");
          const auto aDescription = datumString("description", "");
          const auto anIdentification = datumString("identification", "");
          const TDF_Label aDatumLabel = aDimTolTool->AddDatum(aName, aDescription, anIdentification);
          occ::handle<XCAFDoc_Datum> aDatumAttribute;
          if (!aDatumLabel.FindAttribute(XCAFDoc_Datum::GetID(), aDatumAttribute)
              || aDatumAttribute.IsNull())
            throw KernelFailure(ErrorCode::ImportExportFailed, "OCCT could not create XCAF datum");
          const auto aDatumObject = aDatumAttribute->GetObject();
          if (anItem.contains("semanticName"))
          {
            if (!anItem.at("semanticName").is_string())
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "STEP document datum semanticName must be a string");
            aDatumObject->SetSemanticName(new TCollection_HAsciiString(
              anItem.at("semanticName").get<std::string>().c_str()));
          }
          applyAnnotationPresentation(anItem, aDatumObject, "datum");
          aDatumAttribute->SetObject(aDatumObject);
          NCollection_Sequence<TDF_Label> aDatumShapeLabels;
          aDatumShapeLabels.Append(aLabels[static_cast<std::size_t>(aNode)]);
          aDimTolTool->SetDatum(aDatumShapeLabels, aDatumLabel);
          aInputDatumLabels.push_back(aDatumLabel);
        }
      }
      if (theArgs.contains("geometricTolerances"))
      {
        const json& aTolerances = theArgs.at("geometricTolerances");
        if (!aTolerances.is_array())
          throw KernelFailure(ErrorCode::InvalidArgs, "XCAF geometricTolerances must be an array");
        if (theOp == "exportSTEPDocument" && !aTolerances.empty())
          throw KernelFailure(ErrorCode::ImportExportFailed,
                              "STEP transfer cannot preserve XCAF geometric tolerance links; use exportXCAF");
        for (const json& anItem : aTolerances)
        {
          if (!anItem.is_object() || !anItem.contains("node")
              || !anItem.at("node").is_number_integer()
              || !anItem.contains("type") || !anItem.at("type").is_string()
              || !anItem.contains("value") || !anItem.at("value").is_number())
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "XCAF geometric tolerance requires node, type, and value");
          const std::int64_t aNode = anItem.at("node").get<std::int64_t>();
          const double aValue = anItem.at("value").get<double>();
          if (aNode < 0 || static_cast<std::size_t>(aNode) >= aNodeCount
              || aNodes[static_cast<std::size_t>(aNode)].at("kind") != "part"
              || !std::isfinite(aValue) || aValue <= 0.0)
            throw KernelFailure(ErrorCode::InvalidArgs, "XCAF geometric tolerance entry is invalid");
          XCAFDimTolObjects_GeomToleranceType aType;
          if (!geomToleranceTypeFromName(anItem.at("type").get<std::string>(), aType))
            throw KernelFailure(ErrorCode::InvalidArgs, "XCAF geometric tolerance type is unsupported");
          XCAFDimTolObjects_GeomToleranceTypeValue aValueType = XCAFDimTolObjects_GeomToleranceTypeValue_None;
          if (anItem.contains("valueType"))
          {
            if (!anItem.at("valueType").is_string()
                || !geomToleranceValueTypeFromName(anItem.at("valueType").get<std::string>(), aValueType))
              throw KernelFailure(ErrorCode::InvalidArgs, "XCAF geometric tolerance valueType is unsupported");
          }
          const auto aTolerance = new XCAFDimTolObjects_GeomToleranceObject();
          aTolerance->SetType(aType);
          aTolerance->SetValue(aValue);
          aTolerance->SetTypeOfValue(aValueType);
          if (anItem.contains("materialRequirement"))
          {
            if (!anItem.at("materialRequirement").is_string())
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "XCAF geometric tolerance materialRequirement is unsupported");
            const std::string aModifier = anItem.at("materialRequirement").get<std::string>();
            if (aModifier == "none")
              aTolerance->SetMaterialRequirementModifier(XCAFDimTolObjects_GeomToleranceMatReqModif_None);
            else if (aModifier == "maximum")
              aTolerance->SetMaterialRequirementModifier(XCAFDimTolObjects_GeomToleranceMatReqModif_M);
            else if (aModifier == "least")
              aTolerance->SetMaterialRequirementModifier(XCAFDimTolObjects_GeomToleranceMatReqModif_L);
            else
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "XCAF geometric tolerance materialRequirement is unsupported");
          }
          if (anItem.contains("zoneModifier"))
          {
            if (!anItem.at("zoneModifier").is_string())
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "XCAF geometric tolerance zoneModifier is unsupported");
            const std::string aModifier = anItem.at("zoneModifier").get<std::string>();
            if (aModifier == "none")
              aTolerance->SetZoneModifier(XCAFDimTolObjects_GeomToleranceZoneModif_None);
            else if (aModifier == "projected")
              aTolerance->SetZoneModifier(XCAFDimTolObjects_GeomToleranceZoneModif_Projected);
            else if (aModifier == "runout")
              aTolerance->SetZoneModifier(XCAFDimTolObjects_GeomToleranceZoneModif_Runout);
            else if (aModifier == "nonUniform")
              aTolerance->SetZoneModifier(XCAFDimTolObjects_GeomToleranceZoneModif_NonUniform);
            else
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "XCAF geometric tolerance zoneModifier is unsupported");
          }
          if (anItem.contains("zoneModifierValue"))
          {
            if (!anItem.at("zoneModifierValue").is_number())
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "XCAF geometric tolerance zoneModifierValue must be positive");
            const double aModifierValue = anItem.at("zoneModifierValue").get<double>();
            if (!std::isfinite(aModifierValue) || aModifierValue <= 0.0)
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "XCAF geometric tolerance zoneModifierValue must be positive");
            aTolerance->SetValueOfZoneModifier(aModifierValue);
          }
          if (anItem.contains("modifiers"))
          {
            if (!anItem.at("modifiers").is_array())
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "XCAF geometric tolerance modifiers must be an array");
            std::unordered_set<std::string> aSeenModifiers;
            for (const json& aModifierValue : anItem.at("modifiers"))
            {
              if (!aModifierValue.is_string())
                throw KernelFailure(ErrorCode::InvalidArgs,
                                    "XCAF geometric tolerance modifier is unsupported");
              const std::string aModifier = aModifierValue.get<std::string>();
              if (!aSeenModifiers.insert(aModifier).second)
                throw KernelFailure(ErrorCode::InvalidArgs,
                                    "XCAF geometric tolerance modifiers must be unique");
              XCAFDimTolObjects_GeomToleranceModif anOcctModifier;
              if (aModifier == "anyCrossSection") anOcctModifier = XCAFDimTolObjects_GeomToleranceModif_Any_Cross_Section;
              else if (aModifier == "commonZone") anOcctModifier = XCAFDimTolObjects_GeomToleranceModif_Common_Zone;
              else if (aModifier == "eachRadialElement") anOcctModifier = XCAFDimTolObjects_GeomToleranceModif_Each_Radial_Element;
              else if (aModifier == "freeState") anOcctModifier = XCAFDimTolObjects_GeomToleranceModif_Free_State;
              else if (aModifier == "leastMaterialRequirement") anOcctModifier = XCAFDimTolObjects_GeomToleranceModif_Least_Material_Requirement;
              else if (aModifier == "lineElement") anOcctModifier = XCAFDimTolObjects_GeomToleranceModif_Line_Element;
              else if (aModifier == "majorDiameter") anOcctModifier = XCAFDimTolObjects_GeomToleranceModif_Major_Diameter;
              else if (aModifier == "maximumMaterialRequirement") anOcctModifier = XCAFDimTolObjects_GeomToleranceModif_Maximum_Material_Requirement;
              else if (aModifier == "minorDiameter") anOcctModifier = XCAFDimTolObjects_GeomToleranceModif_Minor_Diameter;
              else if (aModifier == "notConvex") anOcctModifier = XCAFDimTolObjects_GeomToleranceModif_Not_Convex;
              else if (aModifier == "pitchDiameter") anOcctModifier = XCAFDimTolObjects_GeomToleranceModif_Pitch_Diameter;
              else if (aModifier == "reciprocityRequirement") anOcctModifier = XCAFDimTolObjects_GeomToleranceModif_Reciprocity_Requirement;
              else if (aModifier == "separateRequirement") anOcctModifier = XCAFDimTolObjects_GeomToleranceModif_Separate_Requirement;
              else if (aModifier == "statisticalTolerance") anOcctModifier = XCAFDimTolObjects_GeomToleranceModif_Statistical_Tolerance;
              else if (aModifier == "tangentPlane") anOcctModifier = XCAFDimTolObjects_GeomToleranceModif_Tangent_Plane;
              else if (aModifier == "allAround") anOcctModifier = XCAFDimTolObjects_GeomToleranceModif_All_Around;
              else if (aModifier == "allOver") anOcctModifier = XCAFDimTolObjects_GeomToleranceModif_All_Over;
              else
                throw KernelFailure(ErrorCode::InvalidArgs,
                                    "XCAF geometric tolerance modifier is unsupported");
              aTolerance->AddModifier(anOcctModifier);
            }
          }
          if (anItem.contains("maxValueModifier"))
          {
            if (!anItem.at("maxValueModifier").is_number())
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "XCAF geometric tolerance maxValueModifier must be positive");
            const double aMaxValue = anItem.at("maxValueModifier").get<double>();
            if (!std::isfinite(aMaxValue) || aMaxValue <= 0.0)
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "XCAF geometric tolerance maxValueModifier must be positive");
            aTolerance->SetMaxValueModifier(aMaxValue);
          }
          if (anItem.contains("semanticName"))
          {
            if (!anItem.at("semanticName").is_string())
              throw KernelFailure(ErrorCode::InvalidArgs,
                                  "XCAF geometric tolerance semanticName must be a string");
            aTolerance->SetSemanticName(new TCollection_HAsciiString(
              anItem.at("semanticName").get<std::string>().c_str()));
          }
          applyAnnotationPresentation(anItem, aTolerance, "geometric tolerance");
          const TDF_Label aToleranceLabel = aDimTolTool->AddGeomTolerance();
          XCAFDoc_GeomTolerance::Set(aToleranceLabel)->SetObject(aTolerance);
          aDimTolTool->SetGeomTolerance(aLabels[static_cast<std::size_t>(aNode)], aToleranceLabel);
          if (anItem.contains("datumIndices"))
          {
            if (!anItem.at("datumIndices").is_array())
              throw KernelFailure(ErrorCode::InvalidArgs, "XCAF geometric tolerance datumIndices must be an array");
            for (const json& aDatumIndexValue : anItem.at("datumIndices"))
            {
              if (!aDatumIndexValue.is_number_integer() || aDatumIndexValue.get<std::int64_t>() < 0
                  || static_cast<std::size_t>(aDatumIndexValue.get<std::uint64_t>()) >= aInputDatumLabels.size())
                throw KernelFailure(ErrorCode::InvalidArgs,
                                    "XCAF geometric tolerance datum index is invalid");
              aDimTolTool->SetDatumToGeomTol(
                aInputDatumLabels[aDatumIndexValue.get<std::size_t>()], aToleranceLabel);
            }
          }
        }
      }

      if (theOp == "exportXCAF")
      {
        const occ::handle<TDocStd_Application> anApplication = new TDocStd_Application();
        if (anXCAFFormat == "bin")
          BinXCAFDrivers::DefineFormat(anApplication);
        else
          XmlXCAFDrivers::DefineFormat(anApplication);
        aDocument->Open(anApplication);
        std::stringstream aStream(std::ios::in | std::ios::out | std::ios::binary);
        if (anApplication->SaveAs(aDocument, aStream) != PCDM_SS_OK)
          throw KernelFailure(ErrorCode::ImportExportFailed, "OCCT XCAF document write failed");
        std::string aData = aStream.str();
        if (anXCAFFormat == "xml"
            && ((theArgs.contains("datums") && !theArgs.at("datums").empty())
                || (theArgs.contains("geometricTolerances")
                    && !theArgs.at("geometricTolerances").empty())))
        {
          // XmlXCAF does not persist all DimTol links. Keep the protocol metadata
          // in a comment using hex so arbitrary JSON cannot form an invalid XML comment.
          json aMetadata = {
            {"version", 1},
            {"datums", theArgs.value("datums", json::array())},
            {"geometricTolerances", theArgs.value("geometricTolerances", json::array())}
          };
          // XML may omit datum/tolerance presentation labels. Store BREP topology
          // in the marker because protocol shape handles are instance-local.
          auto persistMarkerPresentation = [&](json& theItem) {
            if (!theItem.is_object() || !theItem.contains("presentation")) return;
            const json& aPresentation = theItem.at("presentation");
            std::ostringstream aPresentationStream(std::ios::out | std::ios::binary);
            BRepTools::Write(myArena.get(requiredU32(aPresentation, "shape")),
                             aPresentationStream, false, false,
                             TopTools_FormatVersion_CURRENT);
            theItem["presentationBrep"] = aPresentationStream.str();
            if (aPresentation.contains("name"))
              theItem["presentationName"] = aPresentation.at("name");
            theItem.erase("presentation");
          };
          for (json& aDatum : aMetadata["datums"])
            persistMarkerPresentation(aDatum);
          for (json& aTolerance : aMetadata["geometricTolerances"])
            persistMarkerPresentation(aTolerance);
          const std::string aPayload = aMetadata.dump();
          static constexpr char aHex[] = "0123456789abcdef";
          std::string aEncoded;
          aEncoded.reserve(aPayload.size() * 2);
          for (const unsigned char aByte : aPayload)
          {
            aEncoded.push_back(aHex[aByte >> 4]);
            aEncoded.push_back(aHex[aByte & 0x0f]);
          }
          const std::string aMarker = "<!-- occt-worker-xcaf-meta:" + aEncoded + " -->";
          const std::size_t anEnd = aData.rfind("</document>");
          if (anEnd == std::string::npos)
            aData += "\n" + aMarker + "\n";
          else
            aData.insert(anEnd, aMarker + "\n");
        }
        const std::uint32_t aBuffer = myBuffers.create(aData.size());
        if (!aData.empty()) std::memcpy(myBuffers.get(aBuffer).data(), aData.data(), aData.size());
        return {{"data", bufferDescriptor(aBuffer, aData.size(),
                                            anXCAFFormat == "bin" ? "xcaf-bin" : "xcaf-xml")}};
      }

      if (theOp == "exportIGESDocument")
      {
        const auto aUnit = igesLengthUnit(theArgs);
        IGESUnitGuard aUnitGuard(std::get<1>(aUnit), std::get<2>(aUnit));
        const occ::handle<XSControl_WorkSession> aSession = new XSControl_WorkSession();
        IGESCAFControl_Writer aWriter(aSession, std::get<0>(aUnit).c_str());
        aWriter.SetColorMode(true);
        aWriter.SetNameMode(true);
        aWriter.SetLayerMode(true);
        occ::handle<Message_ProgressIndicator> aProgress = new HostProgressIndicator();
        Message_ProgressScope aProgressScope(aProgress->Start(), "IGES XCAF export", 2.0);
        if (!aWriter.Transfer(aDocument, aProgressScope.Next()))
          throw KernelFailure(ErrorCode::ImportExportFailed, "OCCT IGES XCAF document transfer failed");
        if (!aProgressScope.More())
          throw KernelFailure(ErrorCode::Cancelled, "OCCT IGES XCAF export cancelled");
        std::ostringstream aStream(std::ios::out | std::ios::binary);
        if (!aWriter.Write(aStream))
          throw KernelFailure(ErrorCode::ImportExportFailed, "OCCT IGES XCAF document write failed");
        std::string aData = aStream.str();
        // IGESCAF in the pinned OCCT release does not persist XCAF layer labels.
        // Keep the public document metadata in a validated trailing marker; the
        // importer strips it before handing the stream back to OCCT.
        json aMetadata = { {"version", 1}, {"nodes", json::array()} };
        for (const json& aNode : aNodes)
        {
          json anItem = {
            {"kind", aNode.value("kind", "part")},
            {"children", aNode.value("children", json::array())},
            {"layers", aNode.value("layers", json::array())}
          };
          if (aNode.contains("name")) anItem["name"] = aNode.at("name");
          if (aNode.contains("color")) anItem["color"] = aNode.at("color");
          if (aNode.contains("transform")) anItem["transform"] = aNode.at("transform");
          aMetadata["nodes"].push_back(std::move(anItem));
        }
        aMetadata["roots"] = theArgs.value("roots", json::array());
        const std::string aPayload = aMetadata.dump();
        static constexpr char aHex[] = "0123456789abcdef";
        std::string aEncoded;
        aEncoded.reserve(aPayload.size() * 2);
        for (const unsigned char aByte : aPayload)
        {
          aEncoded.push_back(aHex[aByte >> 4]);
          aEncoded.push_back(aHex[aByte & 0x0f]);
        }
        aData += "\n# occt-worker-xcaf-meta:" + aEncoded + "\n";
        const std::uint32_t aBuffer = myBuffers.create(aData.size());
        if (!aData.empty()) std::memcpy(myBuffers.get(aBuffer).data(), aData.data(), aData.size());
        return {{"data", bufferDescriptor(aBuffer, aData.size(), "iges-xcaf-text")}};
      }

      const auto aUnit = stepLengthUnit(theArgs);
      const std::string aTimestamp = theArgs.value("timestamp", "2026-01-01T00:00:00");
      STEPCAFControl_Writer aWriter;
      aWriter.SetColorMode(true);
      aWriter.SetNameMode(true);
      aWriter.SetLayerMode(true);
      aWriter.SetPropsMode(true);
      aWriter.SetMaterialMode(true);
      aWriter.SetVisualMaterialMode(true);
      aWriter.SetDimTolMode(true);
      DESTEP_Parameters aParams;
      aParams.WriteUnit = aUnit.first;
      aParams.WriteSchema = stepWriteSchema(theArgs);
      aParams.WriteProductName = "occt-worker";
      aWriter.ChangeWriter().Model(true)->SetLocalLengthUnit(aUnit.second);
      occ::handle<Message_ProgressIndicator> aStepProgress = new HostProgressIndicator();
      Message_ProgressScope aStepProgressScope(aStepProgress->Start(), "STEP XCAF export", 2.0);
      const bool isTransferred = aWriter.Transfer(
        aDocument, aParams, STEPControl_AsIs, nullptr, aStepProgressScope.Next());
      if (!aStepProgressScope.More())
        throw KernelFailure(ErrorCode::Cancelled, "OCCT STEP document export cancelled");
      if (!isTransferred)
        throw KernelFailure(ErrorCode::ImportExportFailed, "OCCT STEP document transfer failed");
      APIHeaderSection_MakeHeader aHeader(aWriter.ChangeWriter().Model());
      aHeader.SetName(new TCollection_HAsciiString("occt-worker"));
      aHeader.SetTimeStamp(new TCollection_HAsciiString(aTimestamp.c_str()));
      std::ostringstream aStream;
      if (aWriter.WriteStream(aStream) != IFSelect_RetDone)
        throw KernelFailure(ErrorCode::ImportExportFailed, "OCCT STEP document write failed");
      aStepProgressScope.Next();
      aStepProgressScope.Show();
      std::string aData = aStream.str();
      if (theArgs.contains("gdt") && theArgs.at("gdt").is_array())
      {
        // Preserve semantic names in comments because STEP transfer can normalize
        // the names emitted for individual dimension types.
        std::string aMarkers;
        for (const json& anItem : theArgs.at("gdt"))
        {
          aMarkers += "/*occt-worker-gdt:";
          if (anItem.contains("semanticName"))
          {
            const std::string aSemanticName = anItem.at("semanticName").get<std::string>();
            for (const char aCharacter : aSemanticName)
            {
              if (aCharacter == '*' || aCharacter == '/') aMarkers += '_';
              else aMarkers += aCharacter;
            }
          }
          aMarkers += "*/\n";
        }
        const std::size_t anEndSection = aData.rfind("ENDSEC;");
        if (anEndSection != std::string::npos) aData.insert(anEndSection, aMarkers);
      }
      const std::uint32_t aBuffer = myBuffers.create(aData.size());
      if (!aData.empty()) std::memcpy(myBuffers.get(aBuffer).data(), aData.data(), aData.size());
      return {{"data", bufferDescriptor(aBuffer, aData.size(), "step-xcaf-utf8")}};
    }
    if (theOp == "importSTEP")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const auto aUnit = stepLengthUnit(theArgs);
      const std::string aData = inputBufferData(theArgs, "data");
      std::istringstream aStream(aData, std::ios::in | std::ios::binary);
      STEPControl_Reader aReader;
      DESTEP_Parameters aParams;
      if (aReader.ReadStream("occt-worker", aParams, aStream) != IFSelect_RetDone)
      {
        throw KernelFailure(ErrorCode::ImportExportFailed, "STEP input could not be parsed");
      }
      aReader.SetSystemLengthUnit(aUnit.second);
      occ::handle<Message_ProgressIndicator> aProgress = new HostProgressIndicator();
      const int aRootCount = aReader.TransferRoots(aProgress->Start());
      if (occt_worker_cancelled() != 0)
        throw KernelFailure(ErrorCode::Cancelled, "OCCT STEP import cancelled");
      if (aRootCount == 0)
      {
        throw KernelFailure(ErrorCode::ImportExportFailed, "STEP input did not contain a transferable shape");
      }
      const TopoDS_Shape aShape = aReader.OneShape();
      if (aShape.IsNull())
      {
        throw KernelFailure(ErrorCode::ImportExportFailed, "STEP input did not contain a shape");
      }
      json aShapes = json::array();
      for (int anIndex = 1; anIndex <= aReader.NbShapes(); ++anIndex)
      {
        const TopoDS_Shape aRootShape = aReader.Shape(anIndex);
        if (!aRootShape.IsNull()) aShapes.push_back(myArena.add(aRootShape, aScope));
      }
      return {{"shape", myArena.add(aShape, aScope)}, {"shapes", std::move(aShapes)},
              {"rootCount", aRootCount}};
    }
    if (theOp == "importSTEPDocument" || theOp == "importXCAF" || theOp == "importIGESDocument")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      const std::string aData = inputBufferData(theArgs, "data");
      json aXmlMetadata;
      json aIgesMetadata;
      std::string aIgesData = aData;
      if (theOp == "importIGESDocument")
      {
        const std::string aMarker = "# occt-worker-xcaf-meta:";
        const std::size_t aMarkerPosition = aData.rfind(aMarker);
        if (aMarkerPosition != std::string::npos)
        {
          const std::size_t aHexStart = aMarkerPosition + aMarker.size();
          const std::size_t aMarkerEnd = aData.find_first_of("\r\n", aHexStart);
          if (aMarkerEnd == std::string::npos || ((aMarkerEnd - aHexStart) & 1u) != 0)
            throw KernelFailure(ErrorCode::ImportExportFailed, "IGES XCAF metadata marker is invalid");
          auto hexValue = [](const char theCharacter) -> int {
            if (theCharacter >= '0' && theCharacter <= '9') return theCharacter - '0';
            if (theCharacter >= 'a' && theCharacter <= 'f') return theCharacter - 'a' + 10;
            if (theCharacter >= 'A' && theCharacter <= 'F') return theCharacter - 'A' + 10;
            return -1;
          };
          std::string aPayload;
          aPayload.reserve((aMarkerEnd - aHexStart) / 2);
          for (std::size_t anIndex = aHexStart; anIndex < aMarkerEnd; anIndex += 2)
          {
            const int aHigh = hexValue(aData[anIndex]);
            const int aLow = hexValue(aData[anIndex + 1]);
            if (aHigh < 0 || aLow < 0)
              throw KernelFailure(ErrorCode::ImportExportFailed, "IGES XCAF metadata marker is invalid");
            aPayload.push_back(static_cast<char>((aHigh << 4) | aLow));
          }
          try { aIgesMetadata = json::parse(aPayload); }
          catch (const std::exception&) {
            throw KernelFailure(ErrorCode::ImportExportFailed, "IGES XCAF metadata marker is invalid");
          }
          if (!aIgesMetadata.is_object() || aIgesMetadata.value("version", 0) != 1
              || !aIgesMetadata.value("nodes", json::array()).is_array()
              || !aIgesMetadata.value("roots", json::array()).is_array())
            throw KernelFailure(ErrorCode::ImportExportFailed, "IGES XCAF metadata marker is invalid");
          aIgesData.resize(aMarkerPosition);
        }
      }
      if (theOp == "importXCAF" && theArgs.value("format", "bin") == "xml")
      {
        const std::string aMarker = "<!-- occt-worker-xcaf-meta:";
        const std::size_t aMarkerPosition = aData.find(aMarker);
        if (aMarkerPosition != std::string::npos)
        {
          const std::size_t aHexStart = aMarkerPosition + aMarker.size();
          const std::size_t aMarkerEnd = aData.find(" -->", aHexStart);
          if (aMarkerEnd == std::string::npos || ((aMarkerEnd - aHexStart) & 1u) != 0)
            throw KernelFailure(ErrorCode::ImportExportFailed, "XCAF XML metadata marker is invalid");
          auto hexValue = [](const char theCharacter) -> int {
            if (theCharacter >= '0' && theCharacter <= '9') return theCharacter - '0';
            if (theCharacter >= 'a' && theCharacter <= 'f') return theCharacter - 'a' + 10;
            if (theCharacter >= 'A' && theCharacter <= 'F') return theCharacter - 'A' + 10;
            return -1;
          };
          std::string aPayload;
          aPayload.reserve((aMarkerEnd - aHexStart) / 2);
          for (std::size_t anIndex = aHexStart; anIndex < aMarkerEnd; anIndex += 2)
          {
            const int aHigh = hexValue(aData[anIndex]);
            const int aLow = hexValue(aData[anIndex + 1]);
            if (aHigh < 0 || aLow < 0)
              throw KernelFailure(ErrorCode::ImportExportFailed, "XCAF XML metadata marker is invalid");
            aPayload.push_back(static_cast<char>((aHigh << 4) | aLow));
          }
          try
          {
            aXmlMetadata = json::parse(aPayload);
          }
          catch (const std::exception&)
          {
            throw KernelFailure(ErrorCode::ImportExportFailed, "XCAF XML metadata marker is invalid");
          }
          if (!aXmlMetadata.is_object() || aXmlMetadata.value("version", 0) != 1
              || !aXmlMetadata.value("datums", json::array()).is_array()
              || !aXmlMetadata.value("geometricTolerances", json::array()).is_array())
            throw KernelFailure(ErrorCode::ImportExportFailed, "XCAF XML metadata marker is invalid");
        }
      }
      std::vector<std::string> aStepGdtSemanticNames;
      if (theOp == "importSTEPDocument")
      {
        const std::string aMarker = "/*occt-worker-gdt:";
        std::size_t aMarkerPosition = 0;
        while ((aMarkerPosition = aData.find(aMarker, aMarkerPosition)) != std::string::npos)
        {
          const std::size_t aNameStart = aMarkerPosition + aMarker.size();
          const std::size_t aMarkerEnd = aData.find("*/", aNameStart);
          if (aMarkerEnd == std::string::npos) break;
          aStepGdtSemanticNames.push_back(aData.substr(aNameStart, aMarkerEnd - aNameStart));
          aMarkerPosition = aMarkerEnd + 2;
        }
      }
      std::istringstream aStream(theOp == "importIGESDocument" ? aIgesData : aData,
                                 std::ios::in | std::ios::binary);
      occ::handle<TDocStd_Document> aDocument;
      int aRootCount = 0;
      if (theOp == "importSTEPDocument")
      {
        const auto aUnit = stepLengthUnit(theArgs);
        aDocument = new TDocStd_Document("BinXCAF");
        STEPCAFControl_Reader aReader;
        aReader.SetColorMode(true);
        aReader.SetNameMode(true);
        aReader.SetLayerMode(true);
        aReader.SetPropsMode(true);
        aReader.SetMetaMode(true);
        aReader.SetProductMetaMode(true);
        aReader.SetMatMode(true);
        aReader.SetViewMode(true);
        aReader.SetGDTMode(true);
        if (aReader.ReadStream("occt-worker", aStream) != IFSelect_RetDone)
          throw KernelFailure(ErrorCode::ImportExportFailed, "STEP document input could not be parsed");
        aReader.ChangeReader().SetSystemLengthUnit(aUnit.second);
        aRootCount = aReader.NbRootsForTransfer();
        occ::handle<Message_ProgressIndicator> aStepProgress = new HostProgressIndicator();
        Message_ProgressScope aStepProgressScope(aStepProgress->Start(), "STEP XCAF import", 1.0);
        const bool isTransferred = aRootCount > 0
          && aReader.Transfer(aDocument, aStepProgressScope.Next());
        if (!aStepProgressScope.More())
          throw KernelFailure(ErrorCode::Cancelled, "OCCT STEP document import cancelled");
        if (!isTransferred)
          throw KernelFailure(ErrorCode::ImportExportFailed, "STEP document contained no transferable roots");
      }
      else if (theOp == "importXCAF")
      {
        const std::string anXCAFFormat = theArgs.value("format", "bin");
        if (anXCAFFormat != "bin" && anXCAFFormat != "xml")
          throw KernelFailure(ErrorCode::InvalidArgs, "XCAF format must be bin or xml");
        const occ::handle<TDocStd_Application> anApplication = new TDocStd_Application();
        if (anXCAFFormat == "bin")
          BinXCAFDrivers::DefineFormat(anApplication);
        else
          XmlXCAFDrivers::DefineFormat(anApplication);
        if (anXCAFFormat == "bin")
        {
          if (anApplication->Open(aStream, aDocument) != PCDM_RS_OK || aDocument.IsNull())
            throw KernelFailure(ErrorCode::ImportExportFailed, "XCAF document input could not be parsed");
        }
        else
        {
          auto readXml = [&]() {
            aDocument = new TDocStd_Document("XmlXCAF");
            aDocument->Open(anApplication);
            std::istringstream anXmlStream(aData, std::ios::in | std::ios::binary);
            XmlXCAFStreamReader aReader;
            aReader.read(anXmlStream, aDocument, anApplication);
            if (aReader.GetStatus() != PCDM_RS_OK)
              throw KernelFailure(ErrorCode::ImportExportFailed,
                                  "XCAF document input could not be parsed");
          };
          readXml();
        }
        XCAFDoc_DocumentTool::Set(aDocument->Main());
      }
      else
      {
        const auto aUnit = igesLengthUnit(theArgs);
        IGESUnitGuard aUnitGuard(std::get<1>(aUnit), std::get<2>(aUnit));
        aDocument = new TDocStd_Document("BinXCAF");
        XCAFDoc_DocumentTool::Set(aDocument->Main());
        IGESCAFControl_Reader aReader;
        aReader.SetColorMode(true);
        aReader.SetNameMode(true);
        aReader.SetLayerMode(true);
        if (aReader.ReadStream("occt-worker", aStream) != IFSelect_RetDone)
          throw KernelFailure(ErrorCode::ImportExportFailed, "IGES XCAF document input could not be parsed");
        aRootCount = aReader.NbRootsForTransfer();
        occ::handle<Message_ProgressIndicator> anIgesProgress = new HostProgressIndicator();
        Message_ProgressScope anIgesProgressScope(
          anIgesProgress->Start(), "IGES XCAF import", 1.0);
        const bool isTransferred = aRootCount > 0
          && aReader.Transfer(aDocument, anIgesProgressScope.Next());
        if (!anIgesProgressScope.More())
          throw KernelFailure(ErrorCode::Cancelled, "OCCT IGES document import cancelled");
        if (!isTransferred)
          throw KernelFailure(ErrorCode::ImportExportFailed,
                              "IGES XCAF document contained no transferable roots");
      }

      const occ::handle<XCAFDoc_ShapeTool> aShapeTool =
        XCAFDoc_DocumentTool::ShapeTool(aDocument->Main());
      const occ::handle<XCAFDoc_ColorTool> aColorTool =
        XCAFDoc_DocumentTool::ColorTool(aDocument->Main());
      const occ::handle<XCAFDoc_LayerTool> aLayerTool =
        XCAFDoc_DocumentTool::LayerTool(aDocument->Main());
      const occ::handle<XCAFDoc_VisMaterialTool> aVisMaterialTool =
        XCAFDoc_DocumentTool::VisMaterialTool(aDocument->Main());
      const occ::handle<XCAFDoc_ViewTool> aViewTool =
        XCAFDoc_DocumentTool::ViewTool(aDocument->Main());
      const occ::handle<XCAFDoc_ClippingPlaneTool> aClippingPlaneTool =
        XCAFDoc_DocumentTool::ClippingPlaneTool(aDocument->Main());
      const occ::handle<XCAFDoc_DimTolTool> aDimTolTool =
        XCAFDoc_DocumentTool::DimTolTool(aDocument->Main());
      NCollection_Sequence<TDF_Label> aRootLabels;
      aShapeTool->GetFreeShapes(aRootLabels);
      if (theOp == "importXCAF" || theOp == "importIGESDocument") aRootCount = aRootLabels.Size();
      const TopoDS_Shape aDocumentShape = aShapeTool->GetOneShape();
      if (aRootLabels.IsEmpty() || aDocumentShape.IsNull())
        throw KernelFailure(ErrorCode::ImportExportFailed,
                            "XCAF document did not produce a shape hierarchy");

      auto nameForLabel = [](const TDF_Label& theLabel) {
        occ::handle<TDataStd_Name> aName;
        if (!theLabel.FindAttribute(TDataStd_Name::GetID(), aName) || aName.IsNull())
          return std::string();
        return std::string(TCollection_AsciiString(aName->Get()).ToCString());
      };
      auto colorForLabel = [](const TDF_Label& theLabel, Quantity_ColorRGBA& theColor) {
        return XCAFDoc_ColorTool::GetColor(theLabel, XCAFDoc_ColorGen, theColor)
               || XCAFDoc_ColorTool::GetColor(theLabel, XCAFDoc_ColorSurf, theColor)
               || XCAFDoc_ColorTool::GetColor(theLabel, XCAFDoc_ColorCurv, theColor);
      };
      auto layersForLabel = [&](const TDF_Label& theLabel) {
        json aLayers = json::array();
        std::unordered_set<std::string> aSeen;
        std::function<void(const TDF_Label&)> appendLayers = [&](const TDF_Label& theCurrent) {
          NCollection_Sequence<TDF_Label> aLayerLabels;
          if (aLayerTool->GetLayers(theCurrent, aLayerLabels))
            for (int anIndex = 1; anIndex <= aLayerLabels.Size(); ++anIndex)
            {
              TCollection_ExtendedString aLayerName;
              if (!aLayerTool->GetLayer(aLayerLabels.Value(anIndex), aLayerName)) continue;
              const std::string aName(TCollection_AsciiString(aLayerName).ToCString());
              if (aSeen.insert(aName).second) aLayers.push_back(aName);
            }
          NCollection_Sequence<TDF_Label> aSubshapeLabels;
          if (!XCAFDoc_ShapeTool::GetSubShapes(theCurrent, aSubshapeLabels)) return;
          for (int anIndex = 1; anIndex <= aSubshapeLabels.Size(); ++anIndex)
            appendLayers(aSubshapeLabels.Value(anIndex));
        };
        appendLayers(theLabel);
        return aLayers;
      };
      auto materialForLabel = [](const TDF_Label& theLabel, json& theMaterial) {
        occ::handle<TDataStd_TreeNode> aReference;
        if (!theLabel.FindAttribute(XCAFDoc::MaterialRefGUID(), aReference)
            || aReference.IsNull() || !aReference->HasFather())
          return false;
        occ::handle<TCollection_HAsciiString> aName;
        occ::handle<TCollection_HAsciiString> aDescription;
        occ::handle<TCollection_HAsciiString> aDensityName;
        occ::handle<TCollection_HAsciiString> aDensityValueType;
        double aDensity = 0.0;
        if (!XCAFDoc_MaterialTool::GetMaterial(aReference->Father()->Label(), aName,
                                               aDescription, aDensity, aDensityName,
                                               aDensityValueType))
          return false;
        theMaterial = {{"name", aName.IsNull() ? "" : aName->ToCString()},
                       {"density", aDensity}};
        if (!aDescription.IsNull() && aDescription->Length() > 0)
          theMaterial["description"] = aDescription->ToCString();
        if (!aDensityName.IsNull() && aDensityName->Length() > 0)
          theMaterial["densityName"] = aDensityName->ToCString();
        if (!aDensityValueType.IsNull() && aDensityValueType->Length() > 0)
          theMaterial["densityValueType"] = aDensityValueType->ToCString();
        return true;
      };
      auto visualMaterialForLabel = [&](const TDF_Label& theLabel, json& theMaterial) {
        TDF_Label aMaterialLabel;
        if (!aVisMaterialTool->GetShapeMaterial(theLabel, aMaterialLabel)) return false;
        const occ::handle<XCAFDoc_VisMaterial> aMaterial =
          XCAFDoc_VisMaterialTool::GetMaterial(aMaterialLabel);
        if (aMaterial.IsNull() || !aMaterial->HasPbrMaterial()) return false;
        const XCAFDoc_VisMaterialPBR& aPbr = aMaterial->PbrMaterial();
        const Quantity_Color& aBaseColor = aPbr.BaseColor.GetRGB();
        theMaterial = {
          {"name", nameForLabel(aMaterialLabel)},
          {"baseColor", {aBaseColor.Red(), aBaseColor.Green(), aBaseColor.Blue(),
                         aPbr.BaseColor.Alpha()}},
          {"metallic", aPbr.Metallic},
          {"roughness", aPbr.Roughness},
          {"emissive", {aPbr.EmissiveFactor.x(), aPbr.EmissiveFactor.y(),
                        aPbr.EmissiveFactor.z()}},
          {"refractionIndex", aPbr.RefractionIndex}
        };
        return true;
      };
      auto validationPropertiesForLabel = [](const TDF_Label& theLabel, json& theProperties) {
        double anArea = 0.0;
        double aVolume = 0.0;
        gp_Pnt aCentroid;
        if (!theProperties.contains("area") && XCAFDoc_Area::Get(theLabel, anArea))
          theProperties["area"] = anArea;
        if (!theProperties.contains("volume") && XCAFDoc_Volume::Get(theLabel, aVolume))
          theProperties["volume"] = aVolume;
        if (!theProperties.contains("centroid") && XCAFDoc_Centroid::Get(theLabel, aCentroid))
          theProperties["centroid"] = {aCentroid.X(), aCentroid.Y(), aCentroid.Z()};
      };
      auto subshapeStylesForLabels = [&](const TDF_Label& theOccurrence,
                                         const TDF_Label& theDefinition,
                                         const TopoDS_Shape& theShape) {
        TopTools_IndexedMapOfShape aFaces;
        TopTools_IndexedMapOfShape anEdges;
        TopExp::MapShapes(theShape, TopAbs_FACE, aFaces);
        TopExp::MapShapes(theShape, TopAbs_EDGE, anEdges);
        std::unordered_map<std::string, json> aStyles;
        auto appendLabelStyles = [&](const TDF_Label& theLabel) {
          NCollection_Sequence<TDF_Label> aSubshapeLabels;
          if (!XCAFDoc_ShapeTool::GetSubShapes(theLabel, aSubshapeLabels)) return;
          for (int aLabelIndex = 1; aLabelIndex <= aSubshapeLabels.Size(); ++aLabelIndex)
          {
            const TDF_Label& aSubshapeLabel = aSubshapeLabels.Value(aLabelIndex);
            const TopoDS_Shape aSubshape = XCAFDoc_ShapeTool::GetShape(aSubshapeLabel);
            const TopTools_IndexedMapOfShape* aMap = nullptr;
            XCAFDoc_ColorType aColorType = XCAFDoc_ColorSurf;
            const char* aTopology = nullptr;
            if (aSubshape.ShapeType() == TopAbs_FACE)
            {
              aMap = &aFaces;
              aTopology = "face";
            }
            else if (aSubshape.ShapeType() == TopAbs_EDGE)
            {
              aMap = &anEdges;
              aColorType = XCAFDoc_ColorCurv;
              aTopology = "edge";
            }
            else
            {
              continue;
            }
            const int aSubshapeIndex = aMap->FindIndex(aSubshape);
            if (aSubshapeIndex == 0) continue;
            const std::string aKey = std::string(aTopology) + ":"
                                     + std::to_string(aSubshapeIndex - 1);
            if (aStyles.find(aKey) != aStyles.end()) continue;
            Quantity_ColorRGBA aColor;
            if (!XCAFDoc_ColorTool::GetColor(aSubshapeLabel, aColorType, aColor)) continue;
            const Quantity_Color& anRgb = aColor.GetRGB();
            aStyles.emplace(aKey, json{{"topology", aTopology},
                                      {"index", aSubshapeIndex - 1},
                                      {"color", {anRgb.Red(), anRgb.Green(), anRgb.Blue(),
                                                 aColor.Alpha()}}});
          }
        };
        appendLabelStyles(theOccurrence);
        if (theDefinition != theOccurrence) appendLabelStyles(theDefinition);

        json aResult = json::array();
        for (const char* aTopology : {"face", "edge"})
        {
          const int aCount = std::strcmp(aTopology, "face") == 0
                               ? aFaces.Extent() : anEdges.Extent();
          for (int anIndex = 0; anIndex < aCount; ++anIndex)
          {
            const auto anItem = aStyles.find(std::string(aTopology) + ":"
                                             + std::to_string(anIndex));
            if (anItem != aStyles.end()) aResult.push_back(anItem->second);
          }
        }
        return aResult;
      };

      json aNodes = json::array();
      std::vector<TDF_Label> aNodeOccurrenceLabels;
      std::vector<TDF_Label> aNodeDefinitionLabels;
      std::function<std::size_t(const TDF_Label&)> appendNode = [&](const TDF_Label& theOccurrence) {
        TDF_Label aDefinition = theOccurrence;
        XCAFDoc_ShapeTool::GetReferredShape(theOccurrence, aDefinition);
        const TopoDS_Shape aShape = XCAFDoc_ShapeTool::GetShape(aDefinition);
        if (aShape.IsNull())
          throw KernelFailure(ErrorCode::ImportExportFailed, "STEP document node has no shape");
        gp_Trsf aTransform = XCAFDoc_ShapeTool::GetLocation(theOccurrence).Transformation();
        const std::size_t aNodeIndex = aNodes.size();
        if (theOp == "importIGESDocument" && aIgesMetadata.contains("nodes")
            && aNodeIndex < aIgesMetadata.at("nodes").size())
        {
          const json& aMetadataNode = aIgesMetadata.at("nodes").at(aNodeIndex);
          if (!aMetadataNode.is_object() || !aMetadataNode.value("layers", json::array()).is_array())
            throw KernelFailure(ErrorCode::ImportExportFailed, "IGES XCAF metadata marker is invalid");
          for (const json& aLayer : aMetadataNode.value("layers", json::array()))
          {
            if (!aLayer.is_string())
              throw KernelFailure(ErrorCode::ImportExportFailed, "IGES XCAF metadata marker is invalid");
            const TCollection_ExtendedString aLayerName(aLayer.get<std::string>().c_str(), true);
            aLayerTool->SetLayer(theOccurrence, aLayerName);
            aLayerTool->SetLayer(aShape, aLayerName);
          }
          if (aMetadataNode.contains("color") && aMetadataNode.at("color").is_array()
              && aMetadataNode.at("color").size() == 4)
          {
            const auto& aColor = aMetadataNode.at("color");
            const Quantity_ColorRGBA aValue(aColor[0].get<float>(), aColor[1].get<float>(),
                                             aColor[2].get<float>(), aColor[3].get<float>());
            aColorTool->SetColor(theOccurrence, aValue, XCAFDoc_ColorGen);
            aColorTool->SetColor(theOccurrence, aValue, XCAFDoc_ColorSurf);
            aColorTool->SetColor(theOccurrence, aValue, XCAFDoc_ColorCurv);
          }
          if (aMetadataNode.contains("transform") && aMetadataNode.at("transform").is_array()
              && aMetadataNode.at("transform").size() == 12)
          {
            const auto& aMatrix = aMetadataNode.at("transform");
            gp_Trsf aTransform;
            aTransform.SetValues(aMatrix[0].get<double>(), aMatrix[1].get<double>(), aMatrix[2].get<double>(), aMatrix[3].get<double>(),
                                 aMatrix[4].get<double>(), aMatrix[5].get<double>(), aMatrix[6].get<double>(), aMatrix[7].get<double>(),
                                 aMatrix[8].get<double>(), aMatrix[9].get<double>(), aMatrix[10].get<double>(), aMatrix[11].get<double>());
            TDF_Label aLocatedRoot;
            aShapeTool->SetLocation(theOccurrence, TopLoc_Location(aTransform), aLocatedRoot);
            aTransform = XCAFDoc_ShapeTool::GetLocation(theOccurrence).Transformation();
          }
        }
        json aNode = {
          {"kind", XCAFDoc_ShapeTool::IsAssembly(aDefinition) ? "assembly" : "part"},
          {"shape", myArena.add(aShape, aScope)},
          {"children", json::array()},
          {"transform", {
            aTransform.Value(1, 1), aTransform.Value(1, 2), aTransform.Value(1, 3), aTransform.Value(1, 4),
            aTransform.Value(2, 1), aTransform.Value(2, 2), aTransform.Value(2, 3), aTransform.Value(2, 4),
            aTransform.Value(3, 1), aTransform.Value(3, 2), aTransform.Value(3, 3), aTransform.Value(3, 4)}},
          {"layers", layersForLabel(theOccurrence)},
          {"subshapeStyles", subshapeStylesForLabels(theOccurrence, aDefinition, aShape)},
          {"visible", XCAFDoc_ColorTool::IsVisible(theOccurrence)
                      && XCAFDoc_ColorTool::IsVisible(aDefinition)}};
        std::string aName = nameForLabel(theOccurrence);
        if (aName.empty()) aName = nameForLabel(aDefinition);
        if (!aName.empty()) aNode["name"] = aName;
        Quantity_ColorRGBA aColor;
        if (colorForLabel(theOccurrence, aColor) || colorForLabel(aDefinition, aColor))
        {
          const Quantity_Color& anRgb = aColor.GetRGB();
          aNode["color"] = {anRgb.Red(), anRgb.Green(), anRgb.Blue(), aColor.Alpha()};
        }
        if (aNode.at("layers").empty()) aNode["layers"] = layersForLabel(aDefinition);
        json aMaterial;
        if (materialForLabel(theOccurrence, aMaterial)
            || materialForLabel(aDefinition, aMaterial))
          aNode["material"] = std::move(aMaterial);
        json aVisualMaterial;
        if (visualMaterialForLabel(theOccurrence, aVisualMaterial)
            || visualMaterialForLabel(aDefinition, aVisualMaterial))
          aNode["visualMaterial"] = std::move(aVisualMaterial);
        json aValidationProperties;
        validationPropertiesForLabel(theOccurrence, aValidationProperties);
        validationPropertiesForLabel(aDefinition, aValidationProperties);
        if (!aValidationProperties.empty())
          aNode["validationProperties"] = std::move(aValidationProperties);

        aNodes.push_back(std::move(aNode));
        aNodeOccurrenceLabels.push_back(theOccurrence);
        aNodeDefinitionLabels.push_back(aDefinition);
        NCollection_Sequence<TDF_Label> aComponents;
        if (XCAFDoc_ShapeTool::GetComponents(aDefinition, aComponents, false))
        {
          for (int aComponentIndex = 1; aComponentIndex <= aComponents.Size(); ++aComponentIndex)
            aNodes[aNodeIndex]["children"].push_back(appendNode(aComponents.Value(aComponentIndex)));
        }
        return aNodeIndex;
      };

      json aRoots = json::array();
      for (int aRootIndex = 1; aRootIndex <= aRootLabels.Size(); ++aRootIndex)
        aRoots.push_back(appendNode(aRootLabels.Value(aRootIndex)));

      // XmlXCAF may omit Datum labels and Datum-to-tolerance links. Restore the
      // protocol metadata marker after node labels are available for references.
      std::vector<TDF_Label> aXmlDatumLabels;
      if (!aXmlMetadata.empty())
      {
        auto geomToleranceTypeFromName = [](const std::string& theType,
                                            XCAFDimTolObjects_GeomToleranceType& theResult) {
          if (theType == "angularity") theResult = XCAFDimTolObjects_GeomToleranceType_Angularity;
          else if (theType == "circularRunout") theResult = XCAFDimTolObjects_GeomToleranceType_CircularRunout;
          else if (theType == "circularity") theResult = XCAFDimTolObjects_GeomToleranceType_CircularityOrRoundness;
          else if (theType == "coaxiality") theResult = XCAFDimTolObjects_GeomToleranceType_Coaxiality;
          else if (theType == "concentricity") theResult = XCAFDimTolObjects_GeomToleranceType_Concentricity;
          else if (theType == "cylindricity") theResult = XCAFDimTolObjects_GeomToleranceType_Cylindricity;
          else if (theType == "flatness") theResult = XCAFDimTolObjects_GeomToleranceType_Flatness;
          else if (theType == "parallelism") theResult = XCAFDimTolObjects_GeomToleranceType_Parallelism;
          else if (theType == "perpendicularity") theResult = XCAFDimTolObjects_GeomToleranceType_Perpendicularity;
          else if (theType == "position") theResult = XCAFDimTolObjects_GeomToleranceType_Position;
          else if (theType == "profileOfLine") theResult = XCAFDimTolObjects_GeomToleranceType_ProfileOfLine;
          else if (theType == "profileOfSurface") theResult = XCAFDimTolObjects_GeomToleranceType_ProfileOfSurface;
          else if (theType == "straightness") theResult = XCAFDimTolObjects_GeomToleranceType_Straightness;
          else if (theType == "symmetry") theResult = XCAFDimTolObjects_GeomToleranceType_Symmetry;
          else if (theType == "totalRunout") theResult = XCAFDimTolObjects_GeomToleranceType_TotalRunout;
          else return false;
          return true;
        };
        auto geomToleranceValueTypeFromName = [](const std::string& theType,
                                                 XCAFDimTolObjects_GeomToleranceTypeValue& theResult) {
          if (theType == "none") theResult = XCAFDimTolObjects_GeomToleranceTypeValue_None;
          else if (theType == "diameter") theResult = XCAFDimTolObjects_GeomToleranceTypeValue_Diameter;
          else if (theType == "sphericalDiameter") theResult = XCAFDimTolObjects_GeomToleranceTypeValue_SphericalDiameter;
          else return false;
          return true;
        };
        auto markerPresentation = [](const json& theItem) {
          TopoDS_Shape aShape;
          occ::handle<TCollection_HAsciiString> aName;
          if (!theItem.contains("presentationBrep"))
            return std::make_pair(aShape, aName);
          if (!theItem.at("presentationBrep").is_string()
              || (theItem.contains("presentationName")
                  && !theItem.at("presentationName").is_string()))
            throw KernelFailure(ErrorCode::ImportExportFailed,
                                "XCAF XML annotation presentation metadata is invalid");
          std::istringstream aStream(theItem.at("presentationBrep").get<std::string>(),
                                     std::ios::in | std::ios::binary);
          BRep_Builder aBuilder;
          BRepTools::Read(aShape, aStream, aBuilder);
          if (aShape.IsNull())
            throw KernelFailure(ErrorCode::ImportExportFailed,
                                "XCAF XML annotation presentation metadata is invalid");
          if (theItem.contains("presentationName"))
            aName = new TCollection_HAsciiString(
              theItem.at("presentationName").get<std::string>().c_str());
          return std::make_pair(aShape, aName);
        };

        const json& aDatums = aXmlMetadata.at("datums");
        NCollection_Sequence<TDF_Label> aExistingDatumLabels;
        aDimTolTool->GetDatumLabels(aExistingDatumLabels);
        for (const json& anItem : aDatums)
        {
          if (!anItem.is_object() || !anItem.contains("node") || !anItem.at("node").is_number_integer()
              || !anItem.contains("name") || !anItem.at("name").is_string()
              || !anItem.contains("identification") || !anItem.at("identification").is_string())
            throw KernelFailure(ErrorCode::ImportExportFailed, "XCAF XML datum metadata is invalid");
          const std::int64_t aNode = anItem.at("node").get<std::int64_t>();
          if (aNode < 0 || static_cast<std::size_t>(aNode) >= aNodeOccurrenceLabels.size())
            throw KernelFailure(ErrorCode::ImportExportFailed, "XCAF XML datum metadata is invalid");
          const std::string aName = anItem.at("name").get<std::string>();
          const std::string anIdentification = anItem.at("identification").get<std::string>();
          TDF_Label aDatumLabel;
          for (int anIndex = 1; anIndex <= aExistingDatumLabels.Size(); ++anIndex)
          {
            occ::handle<TCollection_HAsciiString> anExistingName;
            occ::handle<TCollection_HAsciiString> anExistingDescription;
            occ::handle<TCollection_HAsciiString> anExistingIdentification;
            if (aDimTolTool->GetDatum(aExistingDatumLabels.Value(anIndex), anExistingName,
                                       anExistingDescription, anExistingIdentification)
                && !anExistingName.IsNull() && !anExistingIdentification.IsNull()
                && aName == anExistingName->ToCString()
                && anIdentification == anExistingIdentification->ToCString())
            {
              aDatumLabel = aExistingDatumLabels.Value(anIndex);
              break;
            }
          }
          if (aDatumLabel.IsNull())
          {
            const std::string aDescription = anItem.value("description", "");
            aDatumLabel = aDimTolTool->AddDatum(
              new TCollection_HAsciiString(aName.c_str()),
              new TCollection_HAsciiString(aDescription.c_str()),
              new TCollection_HAsciiString(anIdentification.c_str()));
          }
          occ::handle<XCAFDoc_Datum> aDatumAttribute;
          if (!aDatumLabel.FindAttribute(XCAFDoc_Datum::GetID(), aDatumAttribute)
              || aDatumAttribute.IsNull())
            throw KernelFailure(ErrorCode::ImportExportFailed,
                                "XCAF XML datum metadata is invalid");
          const auto aDatumObject = aDatumAttribute->GetObject();
          if (anItem.contains("semanticName"))
          {
            if (!anItem.at("semanticName").is_string())
              throw KernelFailure(ErrorCode::ImportExportFailed, "XCAF XML datum metadata is invalid");
            aDatumObject->SetSemanticName(new TCollection_HAsciiString(
              anItem.at("semanticName").get<std::string>().c_str()));
          }
          const auto aPresentation = markerPresentation(anItem);
          if (!aPresentation.first.IsNull())
            aDatumObject->SetPresentation(aPresentation.first, aPresentation.second);
          aDatumAttribute->SetObject(aDatumObject);
          NCollection_Sequence<TDF_Label> aShapeLabels;
          aShapeLabels.Append(aNodeOccurrenceLabels[static_cast<std::size_t>(aNode)]);
          aDimTolTool->SetDatum(aShapeLabels, aDatumLabel);
          aXmlDatumLabels.push_back(aDatumLabel);
        }

        const json& aTolerances = aXmlMetadata.at("geometricTolerances");
        NCollection_Sequence<TDF_Label> aExistingToleranceLabels;
        aDimTolTool->GetGeomToleranceLabels(aExistingToleranceLabels);
        for (const json& anItem : aTolerances)
        {
          if (!anItem.is_object() || !anItem.contains("node") || !anItem.at("node").is_number_integer()
              || !anItem.contains("type") || !anItem.at("type").is_string()
              || !anItem.contains("value") || !anItem.at("value").is_number())
            throw KernelFailure(ErrorCode::ImportExportFailed, "XCAF XML geometric tolerance metadata is invalid");
          const std::int64_t aNode = anItem.at("node").get<std::int64_t>();
          const double aValue = anItem.at("value").get<double>();
          if (aNode < 0 || static_cast<std::size_t>(aNode) >= aNodeOccurrenceLabels.size()
              || !std::isfinite(aValue) || aValue <= 0.0)
            throw KernelFailure(ErrorCode::ImportExportFailed, "XCAF XML geometric tolerance metadata is invalid");
          XCAFDimTolObjects_GeomToleranceType aType;
          if (!geomToleranceTypeFromName(anItem.at("type").get<std::string>(), aType))
            throw KernelFailure(ErrorCode::ImportExportFailed, "XCAF XML geometric tolerance metadata is invalid");
          XCAFDimTolObjects_GeomToleranceTypeValue aValueType = XCAFDimTolObjects_GeomToleranceTypeValue_None;
          if (anItem.contains("valueType")
              && (!anItem.at("valueType").is_string()
                  || !geomToleranceValueTypeFromName(anItem.at("valueType").get<std::string>(), aValueType)))
            throw KernelFailure(ErrorCode::ImportExportFailed, "XCAF XML geometric tolerance metadata is invalid");
          const TDF_Label aShapeLabel = aNodeOccurrenceLabels[static_cast<std::size_t>(aNode)];
          TDF_Label aToleranceLabel;
          TDF_Label anUnlinkedCandidate;
          bool hasAmbiguousUnlinkedCandidates = false;
          for (int anIndex = 1; anIndex <= aExistingToleranceLabels.Size(); ++anIndex)
          {
            occ::handle<XCAFDoc_GeomTolerance> anAttribute;
            if (!aExistingToleranceLabels.Value(anIndex).FindAttribute(
                  XCAFDoc_GeomTolerance::GetID(), anAttribute) || anAttribute.IsNull()) continue;
            const auto anObject = anAttribute->GetObject();
            if (anObject.IsNull() || anObject->GetType() != aType
                || std::abs(anObject->GetValue() - aValue) > 1e-12) continue;
            NCollection_Sequence<TDF_Label> aRefs;
            NCollection_Sequence<TDF_Label> anUnused;
            if (!XCAFDoc_DimTolTool::GetRefShapeLabel(
                  aExistingToleranceLabels.Value(anIndex), aRefs, anUnused)
                || aRefs.IsEmpty())
            {
              if (anUnlinkedCandidate.IsNull())
                anUnlinkedCandidate = aExistingToleranceLabels.Value(anIndex);
              else
                hasAmbiguousUnlinkedCandidates = true;
              continue;
            }
            for (int aRefIndex = 1; aRefIndex <= aRefs.Size(); ++aRefIndex)
              if (aRefs.Value(aRefIndex).IsEqual(aShapeLabel)) { aToleranceLabel = aExistingToleranceLabels.Value(anIndex); break; }
            if (!aToleranceLabel.IsNull()) break;
          }
          if (aToleranceLabel.IsNull() && !hasAmbiguousUnlinkedCandidates)
            aToleranceLabel = anUnlinkedCandidate;
          if (aToleranceLabel.IsNull()) aToleranceLabel = aDimTolTool->AddGeomTolerance();
          TopoDS_Shape aPresentation;
          occ::handle<TCollection_HAsciiString> aPresentationName;
          occ::handle<XCAFDoc_GeomTolerance> anExistingAttribute;
          if (aToleranceLabel.FindAttribute(XCAFDoc_GeomTolerance::GetID(), anExistingAttribute)
              && !anExistingAttribute.IsNull())
          {
            const auto anExistingObject = anExistingAttribute->GetObject();
            if (!anExistingObject.IsNull())
            {
              aPresentation = anExistingObject->GetPresentation();
              aPresentationName = anExistingObject->GetPresentationName();
            }
          }
          const auto aToleranceObject = new XCAFDimTolObjects_GeomToleranceObject();
          aToleranceObject->SetType(aType);
          aToleranceObject->SetValue(aValue);
          aToleranceObject->SetTypeOfValue(aValueType);
          if (anItem.contains("materialRequirement"))
          {
            if (!anItem.at("materialRequirement").is_string())
              throw KernelFailure(ErrorCode::ImportExportFailed, "XCAF XML geometric tolerance metadata is invalid");
            const std::string aModifier = anItem.at("materialRequirement").get<std::string>();
            if (aModifier == "none") aToleranceObject->SetMaterialRequirementModifier(XCAFDimTolObjects_GeomToleranceMatReqModif_None);
            else if (aModifier == "maximum") aToleranceObject->SetMaterialRequirementModifier(XCAFDimTolObjects_GeomToleranceMatReqModif_M);
            else if (aModifier == "least") aToleranceObject->SetMaterialRequirementModifier(XCAFDimTolObjects_GeomToleranceMatReqModif_L);
            else throw KernelFailure(ErrorCode::ImportExportFailed, "XCAF XML geometric tolerance metadata is invalid");
          }
          if (anItem.contains("zoneModifier"))
          {
            if (!anItem.at("zoneModifier").is_string())
              throw KernelFailure(ErrorCode::ImportExportFailed, "XCAF XML geometric tolerance metadata is invalid");
            const std::string aModifier = anItem.at("zoneModifier").get<std::string>();
            if (aModifier == "none") aToleranceObject->SetZoneModifier(XCAFDimTolObjects_GeomToleranceZoneModif_None);
            else if (aModifier == "projected") aToleranceObject->SetZoneModifier(XCAFDimTolObjects_GeomToleranceZoneModif_Projected);
            else if (aModifier == "runout") aToleranceObject->SetZoneModifier(XCAFDimTolObjects_GeomToleranceZoneModif_Runout);
            else if (aModifier == "nonUniform") aToleranceObject->SetZoneModifier(XCAFDimTolObjects_GeomToleranceZoneModif_NonUniform);
            else throw KernelFailure(ErrorCode::ImportExportFailed, "XCAF XML geometric tolerance metadata is invalid");
          }
          if (anItem.contains("zoneModifierValue"))
          {
            if (!anItem.at("zoneModifierValue").is_number())
              throw KernelFailure(ErrorCode::ImportExportFailed, "XCAF XML geometric tolerance metadata is invalid");
            const double aModifierValue = anItem.at("zoneModifierValue").get<double>();
            if (!std::isfinite(aModifierValue) || aModifierValue <= 0.0)
              throw KernelFailure(ErrorCode::ImportExportFailed, "XCAF XML geometric tolerance metadata is invalid");
            aToleranceObject->SetValueOfZoneModifier(aModifierValue);
          }
          if (anItem.contains("modifiers"))
          {
            if (!anItem.at("modifiers").is_array())
              throw KernelFailure(ErrorCode::ImportExportFailed, "XCAF XML geometric tolerance metadata is invalid");
            std::unordered_set<std::string> aSeenModifiers;
            for (const json& aModifierValue : anItem.at("modifiers"))
            {
              if (!aModifierValue.is_string())
                throw KernelFailure(ErrorCode::ImportExportFailed, "XCAF XML geometric tolerance metadata is invalid");
              const std::string aModifier = aModifierValue.get<std::string>();
              if (!aSeenModifiers.insert(aModifier).second)
                throw KernelFailure(ErrorCode::ImportExportFailed, "XCAF XML geometric tolerance metadata is invalid");
              XCAFDimTolObjects_GeomToleranceModif anOcctModifier;
              if (aModifier == "anyCrossSection") anOcctModifier = XCAFDimTolObjects_GeomToleranceModif_Any_Cross_Section;
              else if (aModifier == "commonZone") anOcctModifier = XCAFDimTolObjects_GeomToleranceModif_Common_Zone;
              else if (aModifier == "eachRadialElement") anOcctModifier = XCAFDimTolObjects_GeomToleranceModif_Each_Radial_Element;
              else if (aModifier == "freeState") anOcctModifier = XCAFDimTolObjects_GeomToleranceModif_Free_State;
              else if (aModifier == "leastMaterialRequirement") anOcctModifier = XCAFDimTolObjects_GeomToleranceModif_Least_Material_Requirement;
              else if (aModifier == "lineElement") anOcctModifier = XCAFDimTolObjects_GeomToleranceModif_Line_Element;
              else if (aModifier == "majorDiameter") anOcctModifier = XCAFDimTolObjects_GeomToleranceModif_Major_Diameter;
              else if (aModifier == "maximumMaterialRequirement") anOcctModifier = XCAFDimTolObjects_GeomToleranceModif_Maximum_Material_Requirement;
              else if (aModifier == "minorDiameter") anOcctModifier = XCAFDimTolObjects_GeomToleranceModif_Minor_Diameter;
              else if (aModifier == "notConvex") anOcctModifier = XCAFDimTolObjects_GeomToleranceModif_Not_Convex;
              else if (aModifier == "pitchDiameter") anOcctModifier = XCAFDimTolObjects_GeomToleranceModif_Pitch_Diameter;
              else if (aModifier == "reciprocityRequirement") anOcctModifier = XCAFDimTolObjects_GeomToleranceModif_Reciprocity_Requirement;
              else if (aModifier == "separateRequirement") anOcctModifier = XCAFDimTolObjects_GeomToleranceModif_Separate_Requirement;
              else if (aModifier == "statisticalTolerance") anOcctModifier = XCAFDimTolObjects_GeomToleranceModif_Statistical_Tolerance;
              else if (aModifier == "tangentPlane") anOcctModifier = XCAFDimTolObjects_GeomToleranceModif_Tangent_Plane;
              else if (aModifier == "allAround") anOcctModifier = XCAFDimTolObjects_GeomToleranceModif_All_Around;
              else if (aModifier == "allOver") anOcctModifier = XCAFDimTolObjects_GeomToleranceModif_All_Over;
              else throw KernelFailure(ErrorCode::ImportExportFailed, "XCAF XML geometric tolerance metadata is invalid");
              aToleranceObject->AddModifier(anOcctModifier);
            }
          }
          if (anItem.contains("maxValueModifier"))
          {
            if (!anItem.at("maxValueModifier").is_number())
              throw KernelFailure(ErrorCode::ImportExportFailed, "XCAF XML geometric tolerance metadata is invalid");
            const double aMaxValue = anItem.at("maxValueModifier").get<double>();
            if (!std::isfinite(aMaxValue) || aMaxValue <= 0.0)
              throw KernelFailure(ErrorCode::ImportExportFailed, "XCAF XML geometric tolerance metadata is invalid");
            aToleranceObject->SetMaxValueModifier(aMaxValue);
          }
          if (anItem.contains("semanticName"))
          {
            if (!anItem.at("semanticName").is_string())
              throw KernelFailure(ErrorCode::ImportExportFailed, "XCAF XML geometric tolerance metadata is invalid");
            aToleranceObject->SetSemanticName(new TCollection_HAsciiString(
              anItem.at("semanticName").get<std::string>().c_str()));
          }
          const auto aMarkerPresentation = markerPresentation(anItem);
          if (!aMarkerPresentation.first.IsNull())
            aToleranceObject->SetPresentation(aMarkerPresentation.first,
                                              aMarkerPresentation.second);
          else if (!aPresentation.IsNull())
            aToleranceObject->SetPresentation(aPresentation, aPresentationName);
          XCAFDoc_GeomTolerance::Set(aToleranceLabel)->SetObject(aToleranceObject);
          aDimTolTool->SetGeomTolerance(aShapeLabel, aToleranceLabel);
          if (anItem.contains("datumIndices"))
          {
            if (!anItem.at("datumIndices").is_array())
              throw KernelFailure(ErrorCode::ImportExportFailed, "XCAF XML geometric tolerance metadata is invalid");
            for (const json& aDatumIndex : anItem.at("datumIndices"))
            {
              if (!aDatumIndex.is_number_integer() || aDatumIndex.get<std::int64_t>() < 0
                  || static_cast<std::size_t>(aDatumIndex.get<std::uint64_t>()) >= aXmlDatumLabels.size())
                throw KernelFailure(ErrorCode::ImportExportFailed, "XCAF XML geometric tolerance metadata is invalid");
              aDimTolTool->SetDatumToGeomTol(
                aXmlDatumLabels[aDatumIndex.get<std::size_t>()], aToleranceLabel);
            }
          }
        }
      }
      json aGdt = json::array();
      auto gdtTypeName = [](const XCAFDimTolObjects_DimensionType theType) -> const char* {
        switch (theType)
        {
          case XCAFDimTolObjects_DimensionType_Size_CurveLength: return "curveLength";
          case XCAFDimTolObjects_DimensionType_Size_Diameter: return "diameter";
          case XCAFDimTolObjects_DimensionType_Size_SphericalDiameter: return "sphericalDiameter";
          case XCAFDimTolObjects_DimensionType_Size_Radius: return "radius";
          case XCAFDimTolObjects_DimensionType_Size_SphericalRadius: return "sphericalRadius";
          case XCAFDimTolObjects_DimensionType_Size_ToroidalMinorDiameter: return "toroidalMinorDiameter";
          case XCAFDimTolObjects_DimensionType_Size_ToroidalMajorDiameter: return "toroidalMajorDiameter";
          case XCAFDimTolObjects_DimensionType_Size_ToroidalMinorRadius: return "toroidalMinorRadius";
          case XCAFDimTolObjects_DimensionType_Size_ToroidalMajorRadius: return "toroidalMajorRadius";
          case XCAFDimTolObjects_DimensionType_Size_ToroidalHighMajorDiameter: return "toroidalHighMajorDiameter";
          case XCAFDimTolObjects_DimensionType_Size_ToroidalLowMajorDiameter: return "toroidalLowMajorDiameter";
          case XCAFDimTolObjects_DimensionType_Size_ToroidalHighMajorRadius: return "toroidalHighMajorRadius";
          case XCAFDimTolObjects_DimensionType_Size_ToroidalLowMajorRadius: return "toroidalLowMajorRadius";
          case XCAFDimTolObjects_DimensionType_Size_Thickness: return "thickness";
          case XCAFDimTolObjects_DimensionType_Size_Angular: return "angular";
          default: return nullptr;
        }
      };
      auto geomToleranceTypeName = [](const XCAFDimTolObjects_GeomToleranceType theType) -> const char* {
        switch (theType)
        {
          case XCAFDimTolObjects_GeomToleranceType_Angularity: return "angularity";
          case XCAFDimTolObjects_GeomToleranceType_CircularRunout: return "circularRunout";
          case XCAFDimTolObjects_GeomToleranceType_CircularityOrRoundness: return "circularity";
          case XCAFDimTolObjects_GeomToleranceType_Coaxiality: return "coaxiality";
          case XCAFDimTolObjects_GeomToleranceType_Concentricity: return "concentricity";
          case XCAFDimTolObjects_GeomToleranceType_Cylindricity: return "cylindricity";
          case XCAFDimTolObjects_GeomToleranceType_Flatness: return "flatness";
          case XCAFDimTolObjects_GeomToleranceType_Parallelism: return "parallelism";
          case XCAFDimTolObjects_GeomToleranceType_Perpendicularity: return "perpendicularity";
          case XCAFDimTolObjects_GeomToleranceType_Position: return "position";
          case XCAFDimTolObjects_GeomToleranceType_ProfileOfLine: return "profileOfLine";
          case XCAFDimTolObjects_GeomToleranceType_ProfileOfSurface: return "profileOfSurface";
          case XCAFDimTolObjects_GeomToleranceType_Straightness: return "straightness";
          case XCAFDimTolObjects_GeomToleranceType_Symmetry: return "symmetry";
          case XCAFDimTolObjects_GeomToleranceType_TotalRunout: return "totalRunout";
          default: return nullptr;
        }
      };
      auto geomToleranceValueTypeName = [](const XCAFDimTolObjects_GeomToleranceTypeValue theType) -> const char* {
        switch (theType)
        {
          case XCAFDimTolObjects_GeomToleranceTypeValue_None: return "none";
          case XCAFDimTolObjects_GeomToleranceTypeValue_Diameter: return "diameter";
          case XCAFDimTolObjects_GeomToleranceTypeValue_SphericalDiameter: return "sphericalDiameter";
          default: return nullptr;
        }
      };
      auto appendAnnotationPresentation = [&](json& theItem, const TopoDS_Shape& theShape,
                                              const occ::handle<TCollection_HAsciiString>& theName) {
        if (theShape.IsNull()) return;
        json aPresentation = {{"shape", myArena.add(theShape, aScope)}};
        if (!theName.IsNull() && theName->Length() > 0)
          aPresentation["name"] = theName->ToCString();
        theItem["presentation"] = std::move(aPresentation);
      };
      std::size_t aGdtIndex = 0;
      NCollection_Sequence<TDF_Label> aDimensionLabels;
      aDimTolTool->GetDimensionLabels(aDimensionLabels);
      for (int aDimensionIndex = 1; aDimensionIndex <= aDimensionLabels.Size(); ++aDimensionIndex)
      {
        const TDF_Label& aDimensionLabel = aDimensionLabels.Value(aDimensionIndex);
        occ::handle<XCAFDoc_Dimension> aDimensionAttribute;
        if (!aDimensionLabel.FindAttribute(XCAFDoc_Dimension::GetID(), aDimensionAttribute)
            || aDimensionAttribute.IsNull())
          continue;
        const occ::handle<XCAFDimTolObjects_DimensionObject> aDimension =
          aDimensionAttribute->GetObject();
        const char* aTypeName = aDimension.IsNull() ? nullptr : gdtTypeName(aDimension->GetType());
        if (aTypeName == nullptr)
          continue;
        NCollection_Sequence<TDF_Label> aFirstLabels;
        NCollection_Sequence<TDF_Label> aSecondLabels;
        if (!XCAFDoc_DimTolTool::GetRefShapeLabel(aDimensionLabel, aFirstLabels, aSecondLabels)
            || aFirstLabels.IsEmpty())
          continue;
        std::size_t aTargetNode = aNodeOccurrenceLabels.size();
        for (int aRefIndex = 1; aRefIndex <= aFirstLabels.Size() && aTargetNode == aNodeOccurrenceLabels.size(); ++aRefIndex)
        {
          for (std::size_t aNodeIndex = 0; aNodeIndex < aNodeOccurrenceLabels.size(); ++aNodeIndex)
          {
            if (aFirstLabels.Value(aRefIndex).IsEqual(aNodeOccurrenceLabels[aNodeIndex])
                || aFirstLabels.Value(aRefIndex).IsEqual(aNodeDefinitionLabels[aNodeIndex]))
            {
              aTargetNode = aNodeIndex;
              break;
            }
          }
        }
        if (aTargetNode == aNodeOccurrenceLabels.size()) continue;
        double aDimensionValue = aDimension->GetValue();
        if (theOp == "importSTEPDocument"
            && aDimension->GetType() == XCAFDimTolObjects_DimensionType_Size_Angular)
          aDimensionValue *= 3.14159265358979323846 / 180.0;
        json anItem = {{"node", aTargetNode}, {"type", aTypeName}, {"value", aDimensionValue}};
        const occ::handle<TCollection_HAsciiString> aName = aDimension->GetSemanticName();
        if (aGdtIndex < aStepGdtSemanticNames.size() && !aStepGdtSemanticNames[aGdtIndex].empty())
          anItem["semanticName"] = aStepGdtSemanticNames[aGdtIndex];
        else if (!aName.IsNull() && aName->Length() > 0)
          anItem["semanticName"] = aName->ToCString();
        appendAnnotationPresentation(anItem, aDimension->GetPresentation(),
                                     aDimension->GetPresentationName());
        ++aGdtIndex;
        aGdt.push_back(std::move(anItem));
      }
      json aDatums = json::array();
      std::vector<TDF_Label> aOutputDatumLabels;
      NCollection_Sequence<TDF_Label> aDatumLabels;
      aDimTolTool->GetDatumLabels(aDatumLabels);
      for (int aDatumIndex = 1; aDatumIndex <= aDatumLabels.Size(); ++aDatumIndex)
      {
        const TDF_Label& aDatumLabel = aDatumLabels.Value(aDatumIndex);
        NCollection_Sequence<TDF_Label> aShapeLabels;
        NCollection_Sequence<TDF_Label> anUnusedSecond;
        if (!XCAFDoc_DimTolTool::GetRefShapeLabel(aDatumLabel, aShapeLabels, anUnusedSecond)
            || aShapeLabels.IsEmpty())
          continue;
        std::size_t aTargetNode = aNodeOccurrenceLabels.size();
        for (int aRefIndex = 1; aRefIndex <= aShapeLabels.Size(); ++aRefIndex)
        {
          for (std::size_t aNodeIndex = 0; aNodeIndex < aNodeOccurrenceLabels.size(); ++aNodeIndex)
          {
            if (aShapeLabels.Value(aRefIndex).IsEqual(aNodeOccurrenceLabels[aNodeIndex])
                || aShapeLabels.Value(aRefIndex).IsEqual(aNodeDefinitionLabels[aNodeIndex]))
            {
              aTargetNode = aNodeIndex;
              break;
            }
          }
          if (aTargetNode < aNodeOccurrenceLabels.size()) break;
        }
        if (aTargetNode == aNodeOccurrenceLabels.size()) continue;
        occ::handle<TCollection_HAsciiString> aName;
        occ::handle<TCollection_HAsciiString> aDescription;
        occ::handle<TCollection_HAsciiString> anIdentification;
        if (!aDimTolTool->GetDatum(aDatumLabel, aName, aDescription, anIdentification)
            || aName.IsNull() || anIdentification.IsNull())
          continue;
        json anItem = {
          {"node", aTargetNode},
          {"name", aName->ToCString()},
          {"identification", anIdentification->ToCString()}
        };
        if (!aDescription.IsNull() && aDescription->Length() > 0)
          anItem["description"] = aDescription->ToCString();
        occ::handle<XCAFDoc_Datum> aDatumAttribute;
        if (aDatumLabel.FindAttribute(XCAFDoc_Datum::GetID(), aDatumAttribute)
            && !aDatumAttribute.IsNull())
        {
          const auto aDatumObject = aDatumAttribute->GetObject();
          if (!aDatumObject.IsNull() && !aDatumObject->GetSemanticName().IsNull()
              && aDatumObject->GetSemanticName()->Length() > 0)
            anItem["semanticName"] = aDatumObject->GetSemanticName()->ToCString();
          if (!aDatumObject.IsNull())
            appendAnnotationPresentation(anItem, aDatumObject->GetPresentation(),
                                         aDatumObject->GetPresentationName());
        }
        aDatums.push_back(std::move(anItem));
        aOutputDatumLabels.push_back(aDatumLabel);
      }
      auto nodeIndexForLabel = [&](const TDF_Label& theLabel) {
        for (std::size_t anIndex = 0; anIndex < aNodeOccurrenceLabels.size(); ++anIndex)
          if (theLabel.IsEqual(aNodeOccurrenceLabels[anIndex])
              || theLabel.IsEqual(aNodeDefinitionLabels[anIndex]))
            return anIndex;
        return aNodeOccurrenceLabels.size();
      };
      json aGeometricTolerances = json::array();
      NCollection_Sequence<TDF_Label> aGeomToleranceLabels;
      aDimTolTool->GetGeomToleranceLabels(aGeomToleranceLabels);
      for (int aToleranceIndex = 1; aToleranceIndex <= aGeomToleranceLabels.Size(); ++aToleranceIndex)
      {
        const TDF_Label& aToleranceLabel = aGeomToleranceLabels.Value(aToleranceIndex);
        occ::handle<XCAFDoc_GeomTolerance> aToleranceAttribute;
        if (!aToleranceLabel.FindAttribute(XCAFDoc_GeomTolerance::GetID(), aToleranceAttribute)
            || aToleranceAttribute.IsNull())
          continue;
        const occ::handle<XCAFDimTolObjects_GeomToleranceObject> aTolerance =
          aToleranceAttribute->GetObject();
        const char* aTypeName = aTolerance.IsNull() ? nullptr : geomToleranceTypeName(aTolerance->GetType());
        const char* aValueTypeName = aTolerance.IsNull() ? nullptr
          : geomToleranceValueTypeName(aTolerance->GetTypeOfValue());
        if (aTypeName == nullptr || aValueTypeName == nullptr || aTolerance->GetValue() <= 0.0)
          continue;
        NCollection_Sequence<TDF_Label> aShapeLabels;
        NCollection_Sequence<TDF_Label> anUnusedSecond;
        if (!XCAFDoc_DimTolTool::GetRefShapeLabel(aToleranceLabel, aShapeLabels, anUnusedSecond)
            || aShapeLabels.IsEmpty())
          continue;
        std::size_t aTargetNode = aNodeOccurrenceLabels.size();
        for (int aRefIndex = 1; aRefIndex <= aShapeLabels.Size() && aTargetNode == aNodeOccurrenceLabels.size(); ++aRefIndex)
          aTargetNode = nodeIndexForLabel(aShapeLabels.Value(aRefIndex));
        if (aTargetNode >= aNodeOccurrenceLabels.size()) continue;
        json anItem = {{"node", aTargetNode}, {"type", aTypeName},
                       {"value", aTolerance->GetValue()}};
        if (std::string(aValueTypeName) != "none") anItem["valueType"] = aValueTypeName;
        const occ::handle<TCollection_HAsciiString> aSemanticName = aTolerance->GetSemanticName();
        if (!aSemanticName.IsNull() && aSemanticName->Length() > 0)
          anItem["semanticName"] = aSemanticName->ToCString();
        appendAnnotationPresentation(anItem, aTolerance->GetPresentation(),
                                     aTolerance->GetPresentationName());
        switch (aTolerance->GetMaterialRequirementModifier())
        {
          case XCAFDimTolObjects_GeomToleranceMatReqModif_M:
            anItem["materialRequirement"] = "maximum";
            break;
          case XCAFDimTolObjects_GeomToleranceMatReqModif_L:
            anItem["materialRequirement"] = "least";
            break;
          default:
            break;
        }
        switch (aTolerance->GetZoneModifier())
        {
          case XCAFDimTolObjects_GeomToleranceZoneModif_Projected:
            anItem["zoneModifier"] = "projected";
            break;
          case XCAFDimTolObjects_GeomToleranceZoneModif_Runout:
            anItem["zoneModifier"] = "runout";
            break;
          case XCAFDimTolObjects_GeomToleranceZoneModif_NonUniform:
            anItem["zoneModifier"] = "nonUniform";
            break;
          default:
            break;
        }
        if (anItem.contains("zoneModifier") && aTolerance->GetValueOfZoneModifier() > 0.0)
          anItem["zoneModifierValue"] = aTolerance->GetValueOfZoneModifier();
        const NCollection_Sequence<XCAFDimTolObjects_GeomToleranceModif> aModifiers =
          aTolerance->GetModifiers();
        if (!aModifiers.IsEmpty())
        {
          anItem["modifiers"] = json::array();
          for (int aModifierIndex = 1; aModifierIndex <= aModifiers.Size(); ++aModifierIndex)
          {
            const char* aModifierName = nullptr;
            switch (aModifiers.Value(aModifierIndex))
            {
              case XCAFDimTolObjects_GeomToleranceModif_Any_Cross_Section: aModifierName = "anyCrossSection"; break;
              case XCAFDimTolObjects_GeomToleranceModif_Common_Zone: aModifierName = "commonZone"; break;
              case XCAFDimTolObjects_GeomToleranceModif_Each_Radial_Element: aModifierName = "eachRadialElement"; break;
              case XCAFDimTolObjects_GeomToleranceModif_Free_State: aModifierName = "freeState"; break;
              case XCAFDimTolObjects_GeomToleranceModif_Least_Material_Requirement: aModifierName = "leastMaterialRequirement"; break;
              case XCAFDimTolObjects_GeomToleranceModif_Line_Element: aModifierName = "lineElement"; break;
              case XCAFDimTolObjects_GeomToleranceModif_Major_Diameter: aModifierName = "majorDiameter"; break;
              case XCAFDimTolObjects_GeomToleranceModif_Maximum_Material_Requirement: aModifierName = "maximumMaterialRequirement"; break;
              case XCAFDimTolObjects_GeomToleranceModif_Minor_Diameter: aModifierName = "minorDiameter"; break;
              case XCAFDimTolObjects_GeomToleranceModif_Not_Convex: aModifierName = "notConvex"; break;
              case XCAFDimTolObjects_GeomToleranceModif_Pitch_Diameter: aModifierName = "pitchDiameter"; break;
              case XCAFDimTolObjects_GeomToleranceModif_Reciprocity_Requirement: aModifierName = "reciprocityRequirement"; break;
              case XCAFDimTolObjects_GeomToleranceModif_Separate_Requirement: aModifierName = "separateRequirement"; break;
              case XCAFDimTolObjects_GeomToleranceModif_Statistical_Tolerance: aModifierName = "statisticalTolerance"; break;
              case XCAFDimTolObjects_GeomToleranceModif_Tangent_Plane: aModifierName = "tangentPlane"; break;
              case XCAFDimTolObjects_GeomToleranceModif_All_Around: aModifierName = "allAround"; break;
              case XCAFDimTolObjects_GeomToleranceModif_All_Over: aModifierName = "allOver"; break;
            }
            if (aModifierName != nullptr) anItem["modifiers"].push_back(aModifierName);
          }
          if (anItem["modifiers"].empty()) anItem.erase("modifiers");
        }
        if (aTolerance->GetMaxValueModifier() > 0.0)
          anItem["maxValueModifier"] = aTolerance->GetMaxValueModifier();
        anItem["datumIndices"] = json::array();
        NCollection_Sequence<TDF_Label> aDatumOfTolerance;
        if (XCAFDoc_DimTolTool::GetDatumOfTolerLabels(aToleranceLabel, aDatumOfTolerance))
        {
          for (int aDatumRefIndex = 1; aDatumRefIndex <= aDatumOfTolerance.Size(); ++aDatumRefIndex)
          {
            for (std::size_t anOutputIndex = 0; anOutputIndex < aOutputDatumLabels.size(); ++anOutputIndex)
            {
              if (aDatumOfTolerance.Value(aDatumRefIndex).IsEqual(aOutputDatumLabels[anOutputIndex]))
              {
                anItem["datumIndices"].push_back(anOutputIndex);
                break;
              }
            }
          }
        }
        if (anItem["datumIndices"].empty()) anItem.erase("datumIndices");
        aGeometricTolerances.push_back(std::move(anItem));
      }
      json aViews = json::array();
      NCollection_Sequence<TDF_Label> aViewLabels;
      aViewTool->GetViewLabels(aViewLabels);
      for (int aViewIndex = 1; aViewIndex <= aViewLabels.Size(); ++aViewIndex)
      {
        occ::handle<XCAFDoc_View> aViewAttribute;
        if (!aViewLabels.Value(aViewIndex).FindAttribute(XCAFDoc_View::GetID(), aViewAttribute)
            || aViewAttribute.IsNull()) continue;
        const occ::handle<XCAFView_Object> aViewObject = aViewAttribute->GetObject();
        if (aViewObject.IsNull()) continue;
        json aView = {
          {"name", aViewObject->Name().IsNull() ? "" : aViewObject->Name()->ToCString()},
          {"projection", aViewObject->Type() == XCAFView_ProjectionType_Parallel ? "parallel"
                        : aViewObject->Type() == XCAFView_ProjectionType_Central ? "central" : "none"},
          {"projectionPoint", {aViewObject->ProjectionPoint().X(), aViewObject->ProjectionPoint().Y(), aViewObject->ProjectionPoint().Z()}},
          {"viewDirection", {aViewObject->ViewDirection().X(), aViewObject->ViewDirection().Y(), aViewObject->ViewDirection().Z()}},
          {"upDirection", {aViewObject->UpDirection().X(), aViewObject->UpDirection().Y(), aViewObject->UpDirection().Z()}},
          {"zoomFactor", aViewObject->ZoomFactor()},
          {"windowHorizontalSize", aViewObject->WindowHorizontalSize()},
          {"windowVerticalSize", aViewObject->WindowVerticalSize()},
          {"viewVolumeSidesClipping", aViewObject->HasViewVolumeSidesClipping()},
          {"nodeIndices", json::array()}
        };
        if (aViewObject->HasFrontPlaneClipping()) aView["frontPlaneDistance"] = aViewObject->FrontPlaneDistance();
        if (aViewObject->HasBackPlaneClipping()) aView["backPlaneDistance"] = aViewObject->BackPlaneDistance();
        aView["clippingPlanes"] = json::array();
        NCollection_Sequence<TDF_Label> aClippingPlaneLabels;
        if (aViewTool->GetRefClippingPlaneLabel(aViewLabels.Value(aViewIndex),
                                                aClippingPlaneLabels))
        {
          for (int aPlaneIndex = 1; aPlaneIndex <= aClippingPlaneLabels.Size(); ++aPlaneIndex)
          {
            gp_Pln aPlane;
            TCollection_ExtendedString aPlaneName;
            bool aCapping = false;
            if (!aClippingPlaneTool->GetClippingPlane(
                  aClippingPlaneLabels.Value(aPlaneIndex), aPlane, aPlaneName, aCapping))
              continue;
            const gp_Pnt& anOrigin = aPlane.Location();
            const gp_Dir& aNormal = aPlane.Axis().Direction();
            aView["clippingPlanes"].push_back({
              {"name", TCollection_AsciiString(aPlaneName).ToCString()},
              {"origin", {anOrigin.X(), anOrigin.Y(), anOrigin.Z()}},
              {"normal", {aNormal.X(), aNormal.Y(), aNormal.Z()}},
              {"capping", aCapping}});
          }
        }
        NCollection_Sequence<TDF_Label> aShapeLabels;
        if (aViewTool->GetRefShapeLabel(aViewLabels.Value(aViewIndex), aShapeLabels))
          for (int aShapeIndex = 1; aShapeIndex <= aShapeLabels.Size(); ++aShapeIndex)
          {
            const std::size_t aNodeIndex = nodeIndexForLabel(aShapeLabels.Value(aShapeIndex));
            if (aNodeIndex < aNodeOccurrenceLabels.size()) aView["nodeIndices"].push_back(aNodeIndex);
          }
        if (!aView["nodeIndices"].empty()) aViews.push_back(std::move(aView));
      }
      json aShuo = json::array();
      std::unordered_set<std::string> aShuoPaths;
      for (const TDF_Label& aNodeLabel : aNodeOccurrenceLabels)
      {
        NCollection_Sequence<occ::handle<TDF_Attribute>> aAttributes;
        if (!XCAFDoc_ShapeTool::GetAllComponentSHUO(aNodeLabel, aAttributes)) continue;
        for (int anAttributeIndex = 1; anAttributeIndex <= aAttributes.Size(); ++anAttributeIndex)
        {
          const occ::handle<XCAFDoc_GraphNode> aStart = occ::down_cast<XCAFDoc_GraphNode>(aAttributes.Value(anAttributeIndex));
          if (aStart.IsNull() || aStart->NbFathers() != 0) continue;
          json aPath = json::array();
          std::string aPathKey;
          occ::handle<XCAFDoc_GraphNode> aCurrent = aStart;
          while (!aCurrent.IsNull())
          {
            const std::size_t aNodeIndex = nodeIndexForLabel(aCurrent->Label().Father());
            if (aNodeIndex >= aNodeOccurrenceLabels.size()) break;
            if (!aPathKey.empty()) aPathKey += "/";
            aPathKey += std::to_string(aNodeIndex);
            aPath.push_back(aNodeIndex);
            if (aCurrent->NbChildren() != 1) break;
            aCurrent = aCurrent->GetChild(1);
          }
          if (aPath.size() >= 2 && aShuoPaths.insert(aPathKey).second)
          {
            json aDefinition = {{"nodeIndices", std::move(aPath)}};
            Quantity_ColorRGBA aColor;
            if (XCAFDoc_ColorTool::GetColor(aStart->Label(), XCAFDoc_ColorGen, aColor))
            {
              const Quantity_Color& anRgb = aColor.GetRGB();
              aDefinition["color"] = {anRgb.Red(), anRgb.Green(), anRgb.Blue(), aColor.Alpha()};
            }
            aShuo.push_back(std::move(aDefinition));
          }
        }
      }
      if (theOp == "importIGESDocument" && aIgesMetadata.contains("nodes"))
      {
        const json& aMetadataNodes = aIgesMetadata.at("nodes");
        json aReconstructedNodes = json::array();
        std::size_t aPartIndex = 0;
        const auto aDocumentHandle = myArena.add(aDocumentShape, aScope);
        for (const json& aMetadataNode : aMetadataNodes)
        {
          if (!aMetadataNode.is_object() || !aMetadataNode.value("children", json::array()).is_array())
            throw KernelFailure(ErrorCode::ImportExportFailed, "IGES XCAF metadata marker is invalid");
          const std::string aKind = aMetadataNode.value("kind", "part");
          if (aKind != "part" && aKind != "assembly")
            throw KernelFailure(ErrorCode::ImportExportFailed, "IGES XCAF metadata marker is invalid");
          json aNode = {
            {"kind", aKind},
            {"children", aMetadataNode.at("children")},
            {"shape", aDocumentHandle},
            {"transform", aMetadataNode.value("transform", json::array({1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0}))},
            {"layers", aMetadataNode.value("layers", json::array())},
            {"subshapeStyles", json::array()},
            {"visible", true}
          };
          if (aKind == "part" && aPartIndex < aNodes.size())
          {
            aNode["shape"] = aNodes.at(aPartIndex).at("shape");
            ++aPartIndex;
          }
          if (aMetadataNode.contains("name")) aNode["name"] = aMetadataNode.at("name");
          if (aMetadataNode.contains("color")) aNode["color"] = aMetadataNode.at("color");
          aReconstructedNodes.push_back(std::move(aNode));
        }
        const json& aMetadataRoots = aIgesMetadata.at("roots");
        return {{"shape", aDocumentHandle},
                {"rootCount", static_cast<int>(aMetadataRoots.size())},
                {"roots", aMetadataRoots},
                {"nodes", std::move(aReconstructedNodes)},
                {"gdt", json::array()}, {"datums", json::array()},
                {"geometricTolerances", json::array()}, {"views", json::array()},
                {"shuo", json::array()}};
      }
      return {{"shape", myArena.add(aDocumentShape, aScope)},
              {"rootCount", aRootCount},
              {"roots", aRoots},
              {"nodes", aNodes},
              {"gdt", aGdt},
              {"datums", aDatums},
              {"geometricTolerances", aGeometricTolerances},
              {"views", aViews},
              {"shuo", aShuo}};
    }
    if (theOp == "batch")
    {
      const std::uint32_t aScope = requiredU32(theArgs, "scopeId");
      if (!theArgs.contains("ops") || !theArgs.at("ops").is_array())
      {
        throw KernelFailure(ErrorCode::InvalidArgs, "batch requires an ops array");
      }
      json aResults = json::array();
      for (const json& anOperation : theArgs.at("ops"))
      {
        std::string aBatchOperation;
        try
        {
          if (!anOperation.is_object() || !anOperation.contains("op")
              || !anOperation.at("op").is_string())
          {
            throw KernelFailure(ErrorCode::InvalidArgs, "Invalid batch operation");
          }
          aBatchOperation = anOperation.at("op").get<std::string>();
          json anOperationArgs = anOperation.value("args", json::object());
          anOperationArgs["scopeId"] = aScope;
          auto resolveReferences = [&](auto&& self, json& theValue) -> void {
            if (theValue.is_object() && theValue.size() == 1 && theValue.contains("$ref"))
            {
              const std::size_t anIndex = theValue.at("$ref").get<std::size_t>();
              if (anIndex >= aResults.size() || !aResults.at(anIndex).contains("shape"))
              {
                throw KernelFailure(ErrorCode::InvalidArgs, "Invalid batch shape reference");
              }
              theValue = aResults.at(anIndex).at("shape");
            }
            else if (theValue.is_array())
            {
              for (json& anItem : theValue) self(self, anItem);
            }
            else if (theValue.is_object())
            {
              for (auto& anItem : theValue.items()) self(self, anItem.value());
            }
          };
          resolveReferences(resolveReferences, anOperationArgs);
          aResults.push_back(dispatch(anOperation.at("op").get<std::string>(), anOperationArgs));
        }
        catch (const KernelFailure& aFailure)
        {
          json aDetails = {{"operation", "batch"}};
          if (!aBatchOperation.empty())
          {
            aDetails["nestedOperation"] = aBatchOperation;
            if (const char* aFormat = exchangeFormatForOperation(aBatchOperation))
              aDetails["format"] = aFormat;
          }
          return {{"results", aResults},
                  {"error", {{"code", errorName(aFailure.code)},
                              {"message", aFailure.what()}, {"details", std::move(aDetails)}}}};
        }
      }
      return {{"results", aResults}};
    }

    throw KernelFailure(ErrorCode::ProtocolError, "Unknown operation: " + theOp);
  }

  BufferStore& buffers() { return myBuffers; }

private:
  static occ::handle<Geom_Curve> curveForEdge(const TopoDS_Shape& theShape,
                                              double&              theFirst,
                                              double&              theLast)
  {
    if (theShape.ShapeType() != TopAbs_EDGE)
      throw KernelFailure(ErrorCode::InvalidArgs, "Curve analysis requires an edge shape");
    occ::handle<Geom_Curve> aCurve = BRep_Tool::Curve(TopoDS::Edge(theShape), theFirst, theLast);
    if (aCurve.IsNull())
      throw KernelFailure(ErrorCode::InvalidArgs, "Edge has no 3D curve");
    return aCurve;
  }

  static occ::handle<Geom_Surface> surfaceForFace(const TopoDS_Shape& theShape,
                                                  double&              theUFirst,
                                                  double&              theULast,
                                                  double&              theVFirst,
                                                  double&              theVLast)
  {
    if (theShape.ShapeType() != TopAbs_FACE)
      throw KernelFailure(ErrorCode::InvalidArgs, "Surface analysis requires a face shape");
    const TopoDS_Face aFace = TopoDS::Face(theShape);
    occ::handle<Geom_Surface> aSurface = BRep_Tool::Surface(aFace);
    if (aSurface.IsNull())
      throw KernelFailure(ErrorCode::InvalidArgs, "Face has no geometric surface");
    BRepTools::UVBounds(aFace, theUFirst, theULast, theVFirst, theVLast);
    if (!(theUFirst <= theULast && theVFirst <= theVLast))
      throw KernelFailure(ErrorCode::InvalidArgs, "Face has an invalid UV domain");
    return aSurface;
  }

  struct TriangleMesh
  {
    struct ObjPrimitive
    {
      std::uint32_t indexStart = 0;
      std::uint32_t indexCount = 0;
      std::string object;
      std::vector<std::string> groups;
      std::string material;
      std::string smoothingGroup;
      bool hasObject = false;
      bool hasMaterial = false;
      bool hasSmoothingGroup = false;

      bool hasSameState(const ObjPrimitive& theOther) const
      {
        return object == theOther.object
          && groups == theOther.groups
          && material == theOther.material
          && smoothingGroup == theOther.smoothingGroup
          && hasObject == theOther.hasObject
          && hasMaterial == theOther.hasMaterial
          && hasSmoothingGroup == theOther.hasSmoothingGroup;
      }

      json toJson() const
      {
        json aResult{{"indexStart", indexStart}, {"indexCount", indexCount}, {"groups", groups}};
        if (hasObject) aResult["object"] = object;
        if (hasMaterial) aResult["material"] = material;
        if (hasSmoothingGroup) aResult["smoothingGroup"] = smoothingGroup;
        return aResult;
      }
    };

    struct GltfPrimitiveInstance
    {
      std::uint32_t meshIndex = 0;
      std::uint32_t primitiveIndex = 0;
      std::uint32_t mode = 4;
      std::uint32_t vertexStart = 0;
      std::uint32_t vertexCount = 0;
      std::uint32_t indexStart = 0;
      std::uint32_t indexCount = 0;
      std::uint32_t nodeIndex = 0;
      std::uint32_t material = 0;
      bool hasNode = false;
      bool hasMaterial = false;
      bool hasNormals = false;
      bool hasUVs = false;

      json toJson() const
      {
        json aResult{
          {"meshIndex", meshIndex}, {"primitiveIndex", primitiveIndex}, {"mode", mode},
          {"vertexStart", vertexStart}, {"vertexCount", vertexCount},
          {"indexStart", indexStart}, {"indexCount", indexCount},
          {"hasNormals", hasNormals}, {"hasUVs", hasUVs}
        };
        if (hasNode) aResult["nodeIndex"] = nodeIndex;
        if (hasMaterial) aResult["material"] = material;
        return aResult;
      }
    };

    std::vector<gp_Pnt> positions;
    std::vector<gp_Dir> normals;
    std::vector<gp_Pnt2d> uvs;
    std::vector<std::array<std::uint8_t, 4>> colors;
    std::vector<std::uint32_t> indices;
    std::vector<std::string> plyComments;
    std::vector<std::string> plyObjectInfo;
    bool hasPlyDocument = false;
    std::vector<ObjPrimitive> objPrimitives;
    std::vector<json> objMaterials;
    std::vector<std::string> objMaterialLibraries;
    json gltfDocument = json::object();
    std::vector<GltfPrimitiveInstance> gltfPrimitives;
    std::vector<std::vector<std::uint8_t>> gltfBuffers;
    std::vector<std::pair<std::string, std::vector<std::uint8_t>>> gltfResources;
    std::vector<std::uint32_t> gltfSceneRoots;
    std::uint32_t gltfActiveScene = 0;
    bool hasGltfActiveScene = false;
  };

  TriangleMesh parseIndexedTriangleMesh(const json& theArgs, const char* theFormat)
  {
    const std::string aPositionBytes = inputBufferData(theArgs, "positions");
    const std::string anIndexBytes = inputBufferData(theArgs, "indices");
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
      const std::string aNormalBytes = inputBufferData(theArgs, "normals");
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
      const std::string aUVBytes = inputBufferData(theArgs, "uvs");
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
      const std::string aColorBytes = inputBufferData(theArgs, "colors");
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

  static TriangleMesh buildTriangleMesh(const TopoDS_Shape& theInput,
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

  static TriangleMesh triangleMesh(const TopoDS_Shape& theShape)
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

  static TopoDS_Shape makeTriangleShape(const TriangleMesh& theMesh)
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

  static TriangleMesh vrmlSceneMesh(const VrmlData_Scene& theScene)
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

  static std::string writeVRMLIndexedMesh(const TriangleMesh& theMesh,
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

  static std::size_t parseOBJComponent(const std::string& theText,
                                       const std::size_t theCount,
                                       const char* theLabel)
  {
    if (theText.empty())
    {
      throw KernelFailure(ErrorCode::ImportExportFailed, std::string("OBJ face has no ") + theLabel + " index");
    }

    long long anIndex = 0;
    try
    {
      std::size_t anEnd = 0;
      anIndex = std::stoll(theText, &anEnd);
      if (anEnd != theText.size())
      {
        throw std::invalid_argument("trailing characters");
      }
    }
    catch (const std::exception&)
    {
      throw KernelFailure(ErrorCode::ImportExportFailed, std::string("OBJ face has an invalid ") + theLabel + " index");
    }

    if (anIndex > 0)
    {
      --anIndex;
    }
    else if (anIndex < 0)
    {
      anIndex = static_cast<long long>(theCount) + anIndex;
    }
    else
    {
      throw KernelFailure(ErrorCode::ImportExportFailed, std::string("OBJ ") + theLabel + " index cannot be zero");
    }
    if (anIndex < 0 || static_cast<std::size_t>(anIndex) >= theCount)
    {
      throw KernelFailure(ErrorCode::ImportExportFailed, std::string("OBJ ") + theLabel + " index is out of range");
    }
    return static_cast<std::size_t>(anIndex);
  }

  struct ObjCorner
  {
    std::size_t position = 0;
    std::size_t uv = std::numeric_limits<std::size_t>::max();
    std::size_t normal = std::numeric_limits<std::size_t>::max();
  };

  static ObjCorner parseOBJCorner(const std::string& theToken,
                                  const std::size_t thePositionCount,
                                  const std::size_t theUVCount,
                                  const std::size_t theNormalCount)
  {
    const std::size_t aFirstSlash = theToken.find('/');
    const std::string aPositionText = theToken.substr(0, aFirstSlash);
    ObjCorner aCorner;
    aCorner.position = parseOBJComponent(aPositionText, thePositionCount, "vertex");
    if (aFirstSlash == std::string::npos)
      return aCorner;

    const std::size_t aSecondSlash = theToken.find('/', aFirstSlash + 1);
    const std::string anUVText = theToken.substr(
      aFirstSlash + 1,
      aSecondSlash == std::string::npos ? std::string::npos : aSecondSlash - aFirstSlash - 1);
    if (!anUVText.empty())
      aCorner.uv = parseOBJComponent(anUVText, theUVCount, "texture coordinate");
    if (aSecondSlash == std::string::npos)
      return aCorner;

    const std::string aNormalText = theToken.substr(aSecondSlash + 1);
    aCorner.normal = parseOBJComponent(aNormalText, theNormalCount, "normal");
    return aCorner;
  }

  static std::string objLineValue(std::istringstream& theStream)
  {
    std::string aValue;
    std::getline(theStream, aValue);
    const std::size_t aComment = aValue.find('#');
    if (aComment != std::string::npos)
      aValue.erase(aComment);
    const std::size_t aFirst = aValue.find_first_not_of(" \t");
    if (aFirst == std::string::npos)
      return {};
    const std::size_t aLast = aValue.find_last_not_of(" \t");
    return aValue.substr(aFirst, aLast - aFirst + 1);
  }

  static std::string objMapReference(std::istringstream& theStream)
  {
    std::vector<std::string> aTokens;
    std::string aToken;
    while (theStream >> aToken)
    {
      if (!aToken.empty() && aToken[0] == '#') break;
      aTokens.push_back(aToken);
    }
    std::size_t anIndex = 0;
    const auto requireArguments = [&](const std::size_t theCount) {
      if (anIndex + theCount >= aTokens.size())
        throw KernelFailure(ErrorCode::ImportExportFailed, "MTL map option is missing an argument or path");
      anIndex += theCount;
    };
    const auto isNumber = [](const std::string& theValue) {
      try
      {
        std::size_t anEnd = 0;
        const double aValue = std::stod(theValue, &anEnd);
        return anEnd == theValue.size() && std::isfinite(aValue);
      }
      catch (const std::exception&)
      {
        return false;
      }
    };
    while (anIndex < aTokens.size() && !aTokens[anIndex].empty() && aTokens[anIndex][0] == '-')
    {
      const std::string anOption = aTokens[anIndex++];
      if (anOption == "-mm")
      {
        requireArguments(2);
      }
      else if (anOption == "-o" || anOption == "-s" || anOption == "-t")
      {
        const std::size_t aStart = anIndex;
        while (anIndex < aTokens.size() && anIndex - aStart < 3 && isNumber(aTokens[anIndex]))
          ++anIndex;
        if (anIndex == aStart)
          throw KernelFailure(ErrorCode::ImportExportFailed, "MTL map vector option is invalid");
      }
      else if (anOption == "-blendu" || anOption == "-blendv" || anOption == "-boost"
               || anOption == "-texres" || anOption == "-clamp" || anOption == "-bm"
               || anOption == "-imfchan" || anOption == "-type" || anOption == "-cc"
               || anOption == "-colorspace")
      {
        requireArguments(1);
      }
      else if (anIndex + 1 < aTokens.size() && aTokens[anIndex][0] != '-')
      {
        ++anIndex;
      }
    }
    if (anIndex >= aTokens.size())
      throw KernelFailure(ErrorCode::ImportExportFailed, "MTL map has no resource path");
    std::string aPath = aTokens[anIndex++];
    while (anIndex < aTokens.size())
      aPath += " " + aTokens[anIndex++];
    return aPath;
  }

  static std::vector<json> parseMTL(const std::string& theData)
  {
    std::vector<json> aMaterials;
    std::size_t aCurrent = std::numeric_limits<std::size_t>::max();
    std::istringstream aStream(theData);
    std::string aLine;
    while (std::getline(aStream, aLine))
    {
      if (!aLine.empty() && aLine.back() == '\r')
        aLine.pop_back();
      std::istringstream aLineStream(aLine);
      std::string aCommand;
      aLineStream >> aCommand;
      if (aCommand.empty() || aCommand[0] == '#')
        continue;
      if (aCommand == "newmtl")
      {
        const std::string aName = objLineValue(aLineStream);
        if (aName.empty())
          throw KernelFailure(ErrorCode::ImportExportFailed, "MTL material has no name");
        aMaterials.push_back({{"name", aName}});
        aCurrent = aMaterials.size() - 1;
        continue;
      }
      if (aCurrent == std::numeric_limits<std::size_t>::max())
        continue;

      json& aMaterial = aMaterials[aCurrent];
      const auto readVector = [&](const char* theProperty) {
        double x = 0.0, y = 0.0, z = 0.0;
        if (!(aLineStream >> x >> y >> z)
            || !std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z))
        {
          throw KernelFailure(ErrorCode::ImportExportFailed,
                              std::string("MTL ") + aCommand + " value is invalid");
        }
        aMaterial[theProperty] = {x, y, z};
      };
      const auto readNumber = [&]() {
        double aValue = 0.0;
        if (!(aLineStream >> aValue) || !std::isfinite(aValue))
          throw KernelFailure(ErrorCode::ImportExportFailed,
                              std::string("MTL ") + aCommand + " value is invalid");
        return aValue;
      };

      if (aCommand == "Ka") readVector("ambient");
      else if (aCommand == "Kd") readVector("diffuse");
      else if (aCommand == "Ks") readVector("specular");
      else if (aCommand == "Ke") readVector("emissive");
      else if (aCommand == "d") aMaterial["opacity"] = readNumber();
      else if (aCommand == "Tr") aMaterial["opacity"] = 1.0 - readNumber();
      else if (aCommand == "Ns") aMaterial["shininess"] = readNumber();
      else if (aCommand == "Ni") aMaterial["opticalDensity"] = readNumber();
      else if (aCommand == "illum")
      {
        long long aValue = -1;
        if (!(aLineStream >> aValue) || aValue < 0
            || static_cast<unsigned long long>(aValue) > std::numeric_limits<std::uint32_t>::max())
          throw KernelFailure(ErrorCode::ImportExportFailed, "MTL illum value is invalid");
        aMaterial["illuminationModel"] = static_cast<std::uint32_t>(aValue);
      }
      else if (aCommand == "map_Kd") aMaterial["diffuseMap"] = objMapReference(aLineStream);
      else if (aCommand == "map_Ks") aMaterial["specularMap"] = objMapReference(aLineStream);
      else if (aCommand == "map_d") aMaterial["opacityMap"] = objMapReference(aLineStream);
      else if (aCommand == "bump" || aCommand == "map_Bump" || aCommand == "map_bump")
        aMaterial["bumpMap"] = objMapReference(aLineStream);
    }
    return aMaterials;
  }

  static TriangleMesh parseOBJ(
    const std::string& theData,
    const std::unordered_map<std::string, std::string>& theResources = {})
  {
    TriangleMesh aMesh;
    std::vector<gp_Pnt> aRawPositions;
    std::vector<gp_Pnt2d> aRawUVs;
    std::vector<gp_Dir> aRawNormals;
    std::unordered_map<std::string, std::size_t> aCornerMap;
    bool hasAnyUVs = false;
    bool hasAllUVs = true;
    bool hasAnyNormals = false;
    bool hasAllNormals = true;
    TriangleMesh::ObjPrimitive aState;
    std::istringstream aStream(theData);
    std::string aLine;
    while (std::getline(aStream, aLine))
    {
      if (!aLine.empty() && aLine.back() == '\r')
      {
        aLine.pop_back();
      }
      std::istringstream aLineStream(aLine);
      std::string aCommand;
      aLineStream >> aCommand;
      if (aCommand.empty() || aCommand[0] == '#')
      {
        continue;
      }
      if (aCommand == "v")
      {
        double x = 0.0, y = 0.0, z = 0.0;
        if (!(aLineStream >> x >> y >> z) || !std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z))
        {
          throw KernelFailure(ErrorCode::ImportExportFailed, "OBJ vertex is invalid");
        }
        aRawPositions.emplace_back(x, y, z);
      }
      else if (aCommand == "vt")
      {
        double u = 0.0, v = 0.0;
        if (!(aLineStream >> u >> v) || !std::isfinite(u) || !std::isfinite(v))
        {
          throw KernelFailure(ErrorCode::ImportExportFailed, "OBJ texture coordinate is invalid");
        }
        aRawUVs.emplace_back(u, v);
      }
      else if (aCommand == "vn")
      {
        double x = 0.0, y = 0.0, z = 0.0;
        if (!(aLineStream >> x >> y >> z) || !std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z)
            || x * x + y * y + z * z <= Precision::SquareConfusion())
        {
          throw KernelFailure(ErrorCode::ImportExportFailed, "OBJ normal is invalid");
        }
        aRawNormals.emplace_back(x, y, z);
      }
      else if (aCommand == "o")
      {
        aState.object = objLineValue(aLineStream);
        aState.hasObject = !aState.object.empty();
      }
      else if (aCommand == "g")
      {
        aState.groups.clear();
        std::string aGroup;
        while (aLineStream >> aGroup)
        {
          if (!aGroup.empty() && aGroup[0] == '#') break;
          aState.groups.push_back(aGroup);
        }
      }
      else if (aCommand == "usemtl")
      {
        aState.material = objLineValue(aLineStream);
        aState.hasMaterial = !aState.material.empty();
      }
      else if (aCommand == "s")
      {
        aState.smoothingGroup = objLineValue(aLineStream);
        aState.hasSmoothingGroup = !aState.smoothingGroup.empty();
      }
      else if (aCommand == "mtllib")
      {
        std::string aLibrary;
        while (aLineStream >> aLibrary)
        {
          if (!aLibrary.empty() && aLibrary[0] == '#') break;
          aMesh.objMaterialLibraries.push_back(aLibrary);
        }
      }
      else if (aCommand == "f")
      {
        std::vector<ObjCorner> aFace;
        std::string aToken;
        while (aLineStream >> aToken)
        {
          if (!aToken.empty() && aToken[0] == '#') break;
          aFace.push_back(parseOBJCorner(aToken, aRawPositions.size(), aRawUVs.size(), aRawNormals.size()));
        }
        if (aFace.size() < 3)
        {
          throw KernelFailure(ErrorCode::ImportExportFailed, "OBJ face must have at least three vertices");
        }
        std::vector<std::uint32_t> aOutputFace;
        aOutputFace.reserve(aFace.size());
        for (const ObjCorner& aCorner : aFace)
        {
          const std::string aKey = std::to_string(aCorner.position) + "/"
            + std::to_string(aCorner.uv) + "/" + std::to_string(aCorner.normal);
          const auto anExisting = aCornerMap.find(aKey);
          if (anExisting != aCornerMap.end())
          {
            aOutputFace.push_back(static_cast<std::uint32_t>(anExisting->second));
            continue;
          }
          const std::size_t anOutputIndex = aMesh.positions.size();
          aCornerMap.emplace(aKey, anOutputIndex);
          aMesh.positions.push_back(aRawPositions[aCorner.position]);
          if (aCorner.uv == std::numeric_limits<std::size_t>::max())
          {
            hasAllUVs = false;
            aMesh.uvs.emplace_back(0.0, 0.0);
          }
          else
          {
            hasAnyUVs = true;
            aMesh.uvs.push_back(aRawUVs[aCorner.uv]);
          }
          if (aCorner.normal == std::numeric_limits<std::size_t>::max())
          {
            hasAllNormals = false;
            aMesh.normals.emplace_back(0.0, 0.0, 1.0);
          }
          else
          {
            hasAnyNormals = true;
            aMesh.normals.push_back(aRawNormals[aCorner.normal]);
          }
          aOutputFace.push_back(static_cast<std::uint32_t>(anOutputIndex));
        }
        for (std::size_t anIndex = 1; anIndex + 1 < aFace.size(); ++anIndex)
        {
          aMesh.indices.insert(aMesh.indices.end(), {
            aOutputFace[0], aOutputFace[anIndex], aOutputFace[anIndex + 1]});
        }
        TriangleMesh::ObjPrimitive aPrimitive = aState;
        aPrimitive.indexStart = static_cast<std::uint32_t>(
          aMesh.indices.size() - (aFace.size() - 2) * 3);
        aPrimitive.indexCount = static_cast<std::uint32_t>((aFace.size() - 2) * 3);
        if (!aMesh.objPrimitives.empty()
            && aMesh.objPrimitives.back().indexStart + aMesh.objPrimitives.back().indexCount
              == aPrimitive.indexStart
            && aMesh.objPrimitives.back().hasSameState(aPrimitive))
          aMesh.objPrimitives.back().indexCount += aPrimitive.indexCount;
        else
          aMesh.objPrimitives.push_back(std::move(aPrimitive));
      }
    }
    if (aMesh.positions.empty() || aMesh.indices.empty())
    {
      throw KernelFailure(ErrorCode::ImportExportFailed, "OBJ input has no triangulated faces");
    }
    if (!hasAnyUVs || !hasAllUVs) aMesh.uvs.clear();
    if (!hasAnyNormals || !hasAllNormals) aMesh.normals.clear();
    for (const std::string& aLibrary : aMesh.objMaterialLibraries)
    {
      const auto aResource = theResources.find(aLibrary);
      if (aResource == theResources.end())
        continue;
      const std::vector<json> aMaterials = parseMTL(aResource->second);
      aMesh.objMaterials.insert(aMesh.objMaterials.end(), aMaterials.begin(), aMaterials.end());
    }
    return aMesh;
  }

  static TriangleMesh parsePLY(const std::string& theData)
  {
    struct Property { bool isList = false; std::string type; std::string name; std::string listType; };
    struct Element { std::string name; std::size_t count = 0; std::vector<Property> properties; };
    if (theData.size() < 4 || theData.compare(0, 4, "ply\n") != 0 && theData.compare(0, 5, "ply\r\n") != 0)
      throw KernelFailure(ErrorCode::ImportExportFailed, "PLY header is missing");
    const std::size_t aHeaderEnd = theData.find("end_header");
    if (aHeaderEnd == std::string::npos)
      throw KernelFailure(ErrorCode::ImportExportFailed, "PLY header is missing end_header");
    const std::size_t aBodyStart = theData.find('\n', aHeaderEnd);
    if (aBodyStart == std::string::npos)
      throw KernelFailure(ErrorCode::ImportExportFailed, "PLY header is truncated");
    std::istringstream aHeader(theData.substr(0, aBodyStart + 1));
    std::string aLine, aFormat;
    double aVersion = 0.0;
    std::vector<Element> anElements;
    std::vector<std::string> aComments;
    std::vector<std::string> anObjectInfo;
    Element* aCurrent = nullptr;
    while (std::getline(aHeader, aLine))
    {
      if (!aLine.empty() && aLine.back() == '\r') aLine.pop_back();
      std::istringstream aTokens(aLine);
      std::string aCommand;
      aTokens >> aCommand;
      if (aCommand == "format") aTokens >> aFormat >> aVersion;
      else if (aCommand == "comment" || aCommand == "obj_info")
      {
        std::string aValue;
        std::getline(aTokens, aValue);
        const std::size_t aFirst = aValue.find_first_not_of(" \t");
        if (aFirst == std::string::npos) aValue.clear();
        else aValue.erase(0, aFirst);
        (aCommand == "comment" ? aComments : anObjectInfo).push_back(std::move(aValue));
      }
      else if (aCommand == "element")
      {
        Element anElement;
        aTokens >> anElement.name >> anElement.count;
        anElements.push_back(std::move(anElement));
        aCurrent = &anElements.back();
      }
      else if (aCommand == "property" && aCurrent != nullptr)
      {
        Property aProperty;
        std::string aKind;
        aTokens >> aKind;
        if (aKind == "list")
        {
          aProperty.isList = true;
          aTokens >> aProperty.listType >> aProperty.type >> aProperty.name;
        }
        else
        {
          aProperty.type = aKind;
          aTokens >> aProperty.name;
        }
        if (!aProperty.name.empty()) aCurrent->properties.push_back(std::move(aProperty));
      }
    }
    const bool isAscii = aFormat == "ascii" && aVersion == 1.0;
    const bool isLittle = aFormat == "binary_little_endian" && aVersion == 1.0;
    const bool isBig = aFormat == "binary_big_endian" && aVersion == 1.0;
    if ((!isAscii && !isLittle && !isBig) || anElements.empty())
      throw KernelFailure(ErrorCode::ImportExportFailed, "Unsupported PLY format");
    auto aVertexIt = std::find_if(anElements.begin(), anElements.end(), [](const Element& e) { return e.name == "vertex"; });
    auto aFaceIt = std::find_if(anElements.begin(), anElements.end(), [](const Element& e) { return e.name == "face"; });
    if (aVertexIt == anElements.end() || aFaceIt == anElements.end() || aVertexIt->count == 0 || aFaceIt->count == 0)
      throw KernelFailure(ErrorCode::ImportExportFailed, "PLY input has no vertices or faces");
    auto propertyIndex = [&](const Element& e, const char* name, std::size_t fallback) {
      std::size_t aScalarIndex = 0;
      for (const Property& aProperty : e.properties)
      {
        if (aProperty.isList) continue;
        if (aProperty.name == name) return aScalarIndex;
        ++aScalarIndex;
      }
      if (fallback >= aScalarIndex)
        throw KernelFailure(ErrorCode::ImportExportFailed, "PLY vertex position properties are missing");
      return fallback;
    };
    auto optionalPropertyIndex = [&](const Element& e,
                                     const std::initializer_list<const char*> theNames) {
      std::size_t aScalarIndex = 0;
      for (const Property& aProperty : e.properties)
      {
        if (aProperty.isList) continue;
        for (const char* aName : theNames)
          if (aProperty.name == aName) return aScalarIndex;
        ++aScalarIndex;
      }
      return std::numeric_limits<std::size_t>::max();
    };
    const std::size_t anX = propertyIndex(*aVertexIt, "x", 0);
    const std::size_t aY = propertyIndex(*aVertexIt, "y", 1);
    const std::size_t aZ = propertyIndex(*aVertexIt, "z", 2);
    const std::size_t anNX = optionalPropertyIndex(*aVertexIt, {"nx"});
    const std::size_t anNY = optionalPropertyIndex(*aVertexIt, {"ny"});
    const std::size_t anNZ = optionalPropertyIndex(*aVertexIt, {"nz"});
    const std::size_t anU = optionalPropertyIndex(*aVertexIt, {"s", "u", "texture_u"});
    const std::size_t aV = optionalPropertyIndex(*aVertexIt, {"t", "v", "texture_v"});
    const std::size_t aRed = optionalPropertyIndex(*aVertexIt, {"red", "r"});
    const std::size_t aGreen = optionalPropertyIndex(*aVertexIt, {"green", "g"});
    const std::size_t aBlue = optionalPropertyIndex(*aVertexIt, {"blue", "b"});
    const std::size_t anAlpha = optionalPropertyIndex(*aVertexIt, {"alpha", "a"});
    const bool hasNormals = anNX != std::numeric_limits<std::size_t>::max()
      && anNY != std::numeric_limits<std::size_t>::max()
      && anNZ != std::numeric_limits<std::size_t>::max();
    const bool hasUVs = anU != std::numeric_limits<std::size_t>::max()
      && aV != std::numeric_limits<std::size_t>::max();
    const bool hasAnyColor = aRed != std::numeric_limits<std::size_t>::max()
      || aGreen != std::numeric_limits<std::size_t>::max()
      || aBlue != std::numeric_limits<std::size_t>::max()
      || anAlpha != std::numeric_limits<std::size_t>::max();
    const bool hasColors = aRed != std::numeric_limits<std::size_t>::max()
      && aGreen != std::numeric_limits<std::size_t>::max()
      && aBlue != std::numeric_limits<std::size_t>::max();
    if (hasAnyColor && !hasColors)
      throw KernelFailure(ErrorCode::ImportExportFailed, "PLY vertex colors require red, green, and blue");
    std::vector<std::string> aVertexScalarTypes;
    for (const Property& aProperty : aVertexIt->properties)
      if (!aProperty.isList) aVertexScalarTypes.push_back(aProperty.type);
    auto colorByte = [&](const double theValue, const std::size_t thePropertyIndex) {
      if (!std::isfinite(theValue) || thePropertyIndex >= aVertexScalarTypes.size())
        throw KernelFailure(ErrorCode::ImportExportFailed, "PLY vertex color is invalid");
      const std::string& aType = aVertexScalarTypes[thePropertyIndex];
      double aMaximum = 1.0;
      if (aType == "uchar" || aType == "uint8") aMaximum = 255.0;
      else if (aType == "ushort" || aType == "uint16") aMaximum = 65535.0;
      else if (aType == "uint" || aType == "uint32") aMaximum = 4294967295.0;
      else if (aType == "uint64") aMaximum = 18446744073709551615.0;
      else if (aType != "float" && aType != "float32" && aType != "double" && aType != "float64")
        throw KernelFailure(ErrorCode::ImportExportFailed, "PLY vertex color type is unsupported");
      if (theValue < 0.0 || theValue > aMaximum)
        throw KernelFailure(ErrorCode::ImportExportFailed, "PLY vertex color is out of range");
      return static_cast<std::uint8_t>(std::lround(theValue / aMaximum * 255.0));
    };
    std::size_t aFaceList = 0;
    bool hasFaceList = false;
    for (std::size_t i = 0; i < aFaceIt->properties.size(); ++i)
      if (aFaceIt->properties[i].isList && (aFaceIt->properties[i].name == "vertex_indices" || !hasFaceList)) { aFaceList = i; hasFaceList = true; }
    if (!hasFaceList) throw KernelFailure(ErrorCode::ImportExportFailed, "PLY face indices are missing");
    TriangleMesh aMesh;
    aMesh.plyComments = std::move(aComments);
    aMesh.plyObjectInfo = std::move(anObjectInfo);
    aMesh.hasPlyDocument = true;
    aMesh.positions.reserve(aVertexIt->count);
    const char* aBody = theData.data() + aBodyStart + 1;
    const std::size_t aBodySize = theData.size() - (aBodyStart + 1);
    if (isAscii)
    {
      std::istringstream aStream(std::string(aBody, aBodySize));
      for (const Element& e : anElements)
      {
        for (std::size_t r = 0; r < e.count; ++r)
        {
          if (!std::getline(aStream, aLine)) throw KernelFailure(ErrorCode::ImportExportFailed, "PLY data is truncated");
          std::istringstream values(aLine);
          if (e.name == "vertex")
          {
            std::vector<double> v;
            for (const Property& p : e.properties) { if (p.isList) { std::size_t n; if (!(values >> n)) throw KernelFailure(ErrorCode::ImportExportFailed, "PLY vertex data is invalid"); double x; for (std::size_t i=0;i<n;++i) if (!(values>>x)) throw KernelFailure(ErrorCode::ImportExportFailed, "PLY vertex data is invalid"); } else { double x; if (!(values>>x)) throw KernelFailure(ErrorCode::ImportExportFailed, "PLY vertex data is invalid"); v.push_back(x); } }
            if (v.size() <= std::max({anX,aY,aZ}) || !std::isfinite(v[anX]) || !std::isfinite(v[aY]) || !std::isfinite(v[aZ])) throw KernelFailure(ErrorCode::ImportExportFailed, "PLY vertex data is invalid");
            aMesh.positions.emplace_back(v[anX], v[aY], v[aZ]);
            if (hasNormals)
            {
              if (v.size() <= std::max({anNX, anNY, anNZ})
                  || !std::isfinite(v[anNX]) || !std::isfinite(v[anNY]) || !std::isfinite(v[anNZ])
                  || v[anNX] * v[anNX] + v[anNY] * v[anNY] + v[anNZ] * v[anNZ]
                       <= Precision::SquareConfusion())
                throw KernelFailure(ErrorCode::ImportExportFailed, "PLY vertex normal is invalid");
              aMesh.normals.emplace_back(v[anNX], v[anNY], v[anNZ]);
            }
            if (hasUVs)
            {
              if (v.size() <= std::max(anU, aV) || !std::isfinite(v[anU]) || !std::isfinite(v[aV]))
                throw KernelFailure(ErrorCode::ImportExportFailed, "PLY vertex UV is invalid");
              aMesh.uvs.emplace_back(v[anU], v[aV]);
            }
            if (hasColors)
            {
              if (v.size() <= std::max({aRed, aGreen, aBlue}))
                throw KernelFailure(ErrorCode::ImportExportFailed, "PLY vertex color is invalid");
              const std::uint8_t anAlphaValue = anAlpha == std::numeric_limits<std::size_t>::max()
                ? 255 : colorByte(v.at(anAlpha), anAlpha);
              aMesh.colors.push_back({colorByte(v[aRed], aRed), colorByte(v[aGreen], aGreen),
                                      colorByte(v[aBlue], aBlue), anAlphaValue});
            }
          }
          else if (e.name == "face")
          {
            std::vector<std::size_t> indices;
            for (std::size_t p=0;p<e.properties.size();++p) { if (e.properties[p].isList) { std::size_t n; if (!(values>>n)) throw KernelFailure(ErrorCode::ImportExportFailed, "PLY face data is invalid"); if (p==aFaceList) { indices.resize(n); for (std::size_t i=0;i<n;++i) if (!(values>>indices[i]) || indices[i]>=aVertexIt->count) throw KernelFailure(ErrorCode::ImportExportFailed, "PLY face index is out of range"); } else { std::size_t x; for (std::size_t i=0;i<n;++i) if (!(values>>x)) throw KernelFailure(ErrorCode::ImportExportFailed, "PLY face data is invalid"); } } else { double x; if (!(values>>x)) throw KernelFailure(ErrorCode::ImportExportFailed, "PLY face data is invalid"); } }
            if (indices.size()<3) throw KernelFailure(ErrorCode::ImportExportFailed, "PLY face must have at least three vertices");
            for (std::size_t i=1;i+1<indices.size();++i) aMesh.indices.insert(aMesh.indices.end(), {static_cast<std::uint32_t>(indices[0]),static_cast<std::uint32_t>(indices[i]),static_cast<std::uint32_t>(indices[i+1])});
          }
        }
      }
    }
    else
    {
      std::size_t pos = 0;
      auto typeSize = [](const std::string& t) -> std::size_t { if (t=="char"||t=="int8") return 1; if (t=="uchar"||t=="uint8") return 1; if (t=="short"||t=="int16"||t=="ushort"||t=="uint16") return 2; if (t=="int"||t=="int32"||t=="uint"||t=="uint32"||t=="float"||t=="float32") return 4; if (t=="double"||t=="float64"||t=="int64"||t=="uint64") return 8; return 0; };
      auto readScalar = [&](const std::string& type) -> double {
        const std::size_t n = typeSize(type); if (n == 0 || pos + n > aBodySize) throw KernelFailure(ErrorCode::ImportExportFailed, "PLY binary data is truncated");
        const bool signedType = type=="char"||type=="int8"||type=="short"||type=="int16"||type=="int"||type=="int32"||type=="int64";
        std::uint64_t u = 0; if (isLittle) { for (std::size_t i=0;i<n;++i) u |= static_cast<std::uint64_t>(static_cast<unsigned char>(aBody[pos+i])) << (8*i); } else { for (std::size_t i=0;i<n;++i) u = (u<<8) | static_cast<unsigned char>(aBody[pos+i]); } pos += n;
        if (type=="float" || type=="float32" || type=="double" || type=="float64") { double d=0; if (n==4) { std::uint32_t bits=static_cast<std::uint32_t>(u); float f; std::memcpy(&f,&bits,4); d=f; } else { std::uint64_t bits=u; std::memcpy(&d,&bits,8); } return d; }
        if (signedType) { const std::uint64_t sign=std::uint64_t(1)<<(n*8-1); const std::int64_t signedValue = (u&sign) ? (n == 8 ? static_cast<std::int64_t>(u) : static_cast<std::int64_t>(u | (~std::uint64_t(0) << (n*8)))) : static_cast<std::int64_t>(u); return static_cast<double>(signedValue); }
        return static_cast<double>(u);
      };
      for (const Element& e : anElements) for (std::size_t r=0;r<e.count;++r)
      {
        std::vector<double> values; std::vector<std::size_t> indices;
        for (std::size_t p=0;p<e.properties.size();++p) { const Property& prop=e.properties[p]; if (prop.isList) { const std::size_t n=static_cast<std::size_t>(readScalar(prop.listType)); if (n>100000000) throw KernelFailure(ErrorCode::ImportExportFailed,"PLY list is too large"); if (e.name=="face" && p==aFaceList) { indices.resize(n); for (std::size_t i=0;i<n;++i) { const double x=readScalar(prop.type); if (!std::isfinite(x)||x<0||x>=static_cast<double>(aVertexIt->count)) throw KernelFailure(ErrorCode::ImportExportFailed,"PLY face index is out of range"); indices[i]=static_cast<std::size_t>(x); } } else for (std::size_t i=0;i<n;++i) (void)readScalar(prop.type); } else values.push_back(readScalar(prop.type)); }
        if (e.name=="vertex")
        {
          if (values.size()<=std::max({anX,aY,aZ}) || !std::isfinite(values[anX]) || !std::isfinite(values[aY]) || !std::isfinite(values[aZ]))
            throw KernelFailure(ErrorCode::ImportExportFailed,"PLY vertex data is invalid");
          aMesh.positions.emplace_back(values[anX],values[aY],values[aZ]);
          if (hasNormals)
          {
            if (values.size() <= std::max({anNX, anNY, anNZ})
                || !std::isfinite(values[anNX]) || !std::isfinite(values[anNY]) || !std::isfinite(values[anNZ])
                || values[anNX] * values[anNX] + values[anNY] * values[anNY]
                     + values[anNZ] * values[anNZ] <= Precision::SquareConfusion())
              throw KernelFailure(ErrorCode::ImportExportFailed, "PLY vertex normal is invalid");
            aMesh.normals.emplace_back(values[anNX], values[anNY], values[anNZ]);
          }
          if (hasUVs)
          {
            if (values.size() <= std::max(anU, aV)
                || !std::isfinite(values[anU]) || !std::isfinite(values[aV]))
              throw KernelFailure(ErrorCode::ImportExportFailed, "PLY vertex UV is invalid");
            aMesh.uvs.emplace_back(values[anU], values[aV]);
          }
          if (hasColors)
          {
            if (values.size() <= std::max({aRed, aGreen, aBlue}))
              throw KernelFailure(ErrorCode::ImportExportFailed, "PLY vertex color is invalid");
            const std::uint8_t anAlphaValue = anAlpha == std::numeric_limits<std::size_t>::max()
              ? 255 : colorByte(values.at(anAlpha), anAlpha);
            aMesh.colors.push_back({colorByte(values[aRed], aRed), colorByte(values[aGreen], aGreen),
                                    colorByte(values[aBlue], aBlue), anAlphaValue});
          }
        }
        else if (e.name=="face") { if (indices.size()<3) throw KernelFailure(ErrorCode::ImportExportFailed,"PLY face must have at least three vertices"); for (std::size_t i=1;i+1<indices.size();++i) aMesh.indices.insert(aMesh.indices.end(), {static_cast<std::uint32_t>(indices[0]),static_cast<std::uint32_t>(indices[i]),static_cast<std::uint32_t>(indices[i+1])}); }
      }
    }
    if (aMesh.indices.empty()) throw KernelFailure(ErrorCode::ImportExportFailed, "PLY input has no triangulated faces");
    return aMesh;
  }

  static std::string writeMTL(const std::vector<json>& theMaterials)
  {
    std::ostringstream aStream;
    aStream << "# occt-worker MTL\n" << std::setprecision(17);
    const auto writeVector = [&](const json& theMaterial, const char* theProperty,
                                 const char* theCommand) {
      if (!theMaterial.contains(theProperty)) return;
      const json& aValue = theMaterial.at(theProperty);
      aStream << theCommand << " " << aValue.at(0).get<double>() << " "
              << aValue.at(1).get<double>() << " " << aValue.at(2).get<double>() << "\n";
    };
    for (const json& aMaterial : theMaterials)
    {
      aStream << "newmtl " << aMaterial.at("name").get<std::string>() << "\n";
      writeVector(aMaterial, "ambient", "Ka");
      writeVector(aMaterial, "diffuse", "Kd");
      writeVector(aMaterial, "specular", "Ks");
      writeVector(aMaterial, "emissive", "Ke");
      if (aMaterial.contains("opacity")) aStream << "d " << aMaterial.at("opacity").get<double>() << "\n";
      if (aMaterial.contains("shininess")) aStream << "Ns " << aMaterial.at("shininess").get<double>() << "\n";
      if (aMaterial.contains("opticalDensity")) aStream << "Ni " << aMaterial.at("opticalDensity").get<double>() << "\n";
      if (aMaterial.contains("illuminationModel"))
        aStream << "illum " << aMaterial.at("illuminationModel").get<std::uint32_t>() << "\n";
      if (aMaterial.contains("diffuseMap")) aStream << "map_Kd " << aMaterial.at("diffuseMap").get<std::string>() << "\n";
      if (aMaterial.contains("specularMap")) aStream << "map_Ks " << aMaterial.at("specularMap").get<std::string>() << "\n";
      if (aMaterial.contains("opacityMap")) aStream << "map_d " << aMaterial.at("opacityMap").get<std::string>() << "\n";
      if (aMaterial.contains("bumpMap")) aStream << "bump " << aMaterial.at("bumpMap").get<std::string>() << "\n";
    }
    return aStream.str();
  }

  static std::string writeOBJ(const TriangleMesh& theMesh)
  {
    std::ostringstream aStream;
    aStream << "# occt-worker OBJ\n" << std::setprecision(17);
    for (const std::string& aLibrary : theMesh.objMaterialLibraries)
      aStream << "mtllib " << aLibrary << "\n";
    for (const gp_Pnt& aPoint : theMesh.positions)
      aStream << "v " << aPoint.X() << " " << aPoint.Y() << " " << aPoint.Z() << "\n";
    const bool hasUVs = theMesh.uvs.size() == theMesh.positions.size();
    const bool hasNormals = theMesh.normals.size() == theMesh.positions.size();
    if (hasUVs)
      for (const gp_Pnt2d& aUV : theMesh.uvs)
        aStream << "vt " << aUV.X() << " " << aUV.Y() << "\n";
    if (hasNormals)
      for (const gp_Dir& aNormal : theMesh.normals)
        aStream << "vn " << aNormal.X() << " " << aNormal.Y() << " " << aNormal.Z() << "\n";
    std::size_t aPrimitiveIndex = 0;
    for (std::size_t aTriangle = 0; aTriangle < theMesh.indices.size(); aTriangle += 3)
    {
      while (aPrimitiveIndex < theMesh.objPrimitives.size()
             && aTriangle >= static_cast<std::size_t>(theMesh.objPrimitives[aPrimitiveIndex].indexStart)
                            + theMesh.objPrimitives[aPrimitiveIndex].indexCount)
        ++aPrimitiveIndex;
      if (aPrimitiveIndex < theMesh.objPrimitives.size()
          && aTriangle == theMesh.objPrimitives[aPrimitiveIndex].indexStart)
      {
        const TriangleMesh::ObjPrimitive& aPrimitive = theMesh.objPrimitives[aPrimitiveIndex];
        aStream << "o" << (aPrimitive.hasObject ? " " + aPrimitive.object : "") << "\n";
        aStream << "g";
        for (const std::string& aGroup : aPrimitive.groups) aStream << " " << aGroup;
        aStream << "\n";
        aStream << "usemtl" << (aPrimitive.hasMaterial ? " " + aPrimitive.material : "") << "\n";
        aStream << "s" << (aPrimitive.hasSmoothingGroup ? " " + aPrimitive.smoothingGroup : "") << "\n";
      }
      aStream << "f ";
      for (std::size_t anIndex = 0; anIndex < 3; ++anIndex)
      {
        const std::uint32_t aVertex = theMesh.indices[aTriangle + anIndex] + 1;
        aStream << aVertex;
        if (hasUVs || hasNormals) aStream << "/";
        if (hasUVs) aStream << aVertex;
        if (hasNormals) aStream << "/" << aVertex;
        aStream << (anIndex == 2 ? '\n' : ' ');
      }
    }
    return aStream.str();
  }

  static std::string writePLY(const TriangleMesh& theMesh, const std::string& theEncoding)
  {
    const bool hasNormals = theMesh.normals.size() == theMesh.positions.size();
    const bool hasUVs = theMesh.uvs.size() == theMesh.positions.size();
    const bool hasColors = theMesh.colors.size() == theMesh.positions.size();
    std::string aMetadata;
    const auto appendMetadata = [&](const char* theCommand,
                                    const std::vector<std::string>& theValues) {
      for (const std::string& aValue : theValues)
      {
        aMetadata += theCommand;
        if (!aValue.empty()) aMetadata += " " + aValue;
        aMetadata += "\n";
      }
    };
    if (theMesh.hasPlyDocument)
    {
      appendMetadata("comment", theMesh.plyComments);
      appendMetadata("obj_info", theMesh.plyObjectInfo);
    }
    else
    {
      aMetadata = "comment Exported by occt-worker\n";
    }
    if (theEncoding == "binary_little_endian" || theEncoding == "binary_big_endian")
    {
      const bool isLittleEndian = theEncoding == "binary_little_endian";
      std::string aData = "ply\nformat "
        + std::string(isLittleEndian ? "binary_little_endian" : "binary_big_endian")
        + " 1.0\n" + aMetadata;
      aData += "element vertex " + std::to_string(theMesh.positions.size())
        + "\nproperty double x\nproperty double y\nproperty double z\n";
      if (hasNormals) aData += "property double nx\nproperty double ny\nproperty double nz\n";
      if (hasUVs) aData += "property double s\nproperty double t\n";
      if (hasColors) aData += "property uchar red\nproperty uchar green\nproperty uchar blue\nproperty uchar alpha\n";
      aData += "element face " + std::to_string(theMesh.indices.size() / 3) + "\nproperty list uchar uint vertex_indices\nend_header\n";
      auto appendU = [&](std::uint64_t v, std::size_t size) {
        if (isLittleEndian)
        {
          for (std::size_t i = 0; i < size; ++i)
            aData.push_back(static_cast<char>((v >> (8 * i)) & 0xff));
        }
        else
        {
          for (std::size_t i = size; i > 0; --i)
            aData.push_back(static_cast<char>((v >> (8 * (i - 1))) & 0xff));
        }
      };
      for (std::size_t aVertex = 0; aVertex < theMesh.positions.size(); ++aVertex)
      {
        const gp_Pnt& p = theMesh.positions[aVertex];
        const std::array<double, 3> aPosition = {p.X(), p.Y(), p.Z()};
        for (const double v : aPosition) { std::uint64_t bits; std::memcpy(&bits, &v, 8); appendU(bits, 8); }
        if (hasNormals)
        {
          const gp_Dir& n = theMesh.normals[aVertex];
          const std::array<double, 3> aNormal = {n.X(), n.Y(), n.Z()};
          for (const double v : aNormal) { std::uint64_t bits; std::memcpy(&bits, &v, 8); appendU(bits, 8); }
        }
        if (hasUVs)
        {
          const gp_Pnt2d& uv = theMesh.uvs[aVertex];
          const std::array<double, 2> aUV = {uv.X(), uv.Y()};
          for (const double v : aUV) { std::uint64_t bits; std::memcpy(&bits, &v, 8); appendU(bits, 8); }
        }
        if (hasColors)
          for (const std::uint8_t aChannel : theMesh.colors[aVertex]) appendU(aChannel, 1);
      }
      for (std::size_t i=0;i<theMesh.indices.size();i+=3) { appendU(3,1); appendU(theMesh.indices[i],4); appendU(theMesh.indices[i+1],4); appendU(theMesh.indices[i+2],4); }
      return aData;
    }
    std::ostringstream aStream;
    aStream << "ply\nformat ascii 1.0\n" << aMetadata
             << "element vertex " << theMesh.positions.size() << "\n"
             << "property double x\nproperty double y\nproperty double z\n";
    if (hasNormals) aStream << "property double nx\nproperty double ny\nproperty double nz\n";
    if (hasUVs) aStream << "property double s\nproperty double t\n";
    if (hasColors) aStream << "property uchar red\nproperty uchar green\nproperty uchar blue\nproperty uchar alpha\n";
    aStream << "element face " << theMesh.indices.size() / 3 << "\n"
            << "property list uchar uint vertex_indices\nend_header\n" << std::setprecision(17);
    for (std::size_t aVertex = 0; aVertex < theMesh.positions.size(); ++aVertex)
    {
      const gp_Pnt& aPoint = theMesh.positions[aVertex];
      aStream << aPoint.X() << " " << aPoint.Y() << " " << aPoint.Z();
      if (hasNormals)
      {
        const gp_Dir& aNormal = theMesh.normals[aVertex];
        aStream << " " << aNormal.X() << " " << aNormal.Y() << " " << aNormal.Z();
      }
      if (hasUVs)
      {
        const gp_Pnt2d& aUV = theMesh.uvs[aVertex];
        aStream << " " << aUV.X() << " " << aUV.Y();
      }
      if (hasColors)
      {
        const std::array<std::uint8_t, 4>& aColor = theMesh.colors[aVertex];
        aStream << " " << static_cast<unsigned int>(aColor[0])
                << " " << static_cast<unsigned int>(aColor[1])
                << " " << static_cast<unsigned int>(aColor[2])
                << " " << static_cast<unsigned int>(aColor[3]);
      }
      aStream << "\n";
    }
    for (std::size_t aTriangle = 0; aTriangle < theMesh.indices.size(); aTriangle += 3)
    {
      aStream << "3 " << theMesh.indices[aTriangle] << " "
              << theMesh.indices[aTriangle + 1] << " "
              << theMesh.indices[aTriangle + 2] << "\n";
    }
    return aStream.str();
  }

  static int base64Value(const char theCharacter)
  {
    if (theCharacter >= 'A' && theCharacter <= 'Z') return theCharacter - 'A';
    if (theCharacter >= 'a' && theCharacter <= 'z') return theCharacter - 'a' + 26;
    if (theCharacter >= '0' && theCharacter <= '9') return theCharacter - '0' + 52;
    if (theCharacter == '+') return 62;
    if (theCharacter == '/') return 63;
    return -1;
  }

  static std::string encodeBase64(const std::vector<std::uint8_t>& theData)
  {
    static constexpr char aAlphabet[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string aResult;
    aResult.reserve(((theData.size() + 2) / 3) * 4);
    for (std::size_t anOffset = 0; anOffset < theData.size(); anOffset += 3)
    {
      const std::uint32_t aValue = static_cast<std::uint32_t>(theData[anOffset]) << 16
        | (anOffset + 1 < theData.size()
             ? static_cast<std::uint32_t>(theData[anOffset + 1]) << 8
             : 0)
        | (anOffset + 2 < theData.size()
             ? static_cast<std::uint32_t>(theData[anOffset + 2])
             : 0);
      aResult.push_back(aAlphabet[(aValue >> 18) & 63]);
      aResult.push_back(aAlphabet[(aValue >> 12) & 63]);
      aResult.push_back(anOffset + 1 < theData.size() ? aAlphabet[(aValue >> 6) & 63] : '=');
      aResult.push_back(anOffset + 2 < theData.size() ? aAlphabet[aValue & 63] : '=');
    }
    return aResult;
  }

  static std::vector<std::uint8_t> decodeBase64(const std::string& theText)
  {
    std::string aText;
    aText.reserve(theText.size());
    for (const char aCharacter : theText)
    {
      if (aCharacter != ' ' && aCharacter != '\t' && aCharacter != '\r' && aCharacter != '\n')
      {
        aText.push_back(aCharacter);
      }
    }
    if (aText.size() % 4 != 0)
    {
      throw KernelFailure(ErrorCode::ImportExportFailed, "glTF buffer has invalid base64 data");
    }

    std::vector<std::uint8_t> aResult;
    aResult.reserve(aText.size() / 4 * 3);
    for (std::size_t anOffset = 0; anOffset < aText.size(); anOffset += 4)
    {
      const bool hasThird = aText[anOffset + 2] != '=';
      const bool hasFourth = aText[anOffset + 3] != '=';
      const int a = base64Value(aText[anOffset]);
      const int b = base64Value(aText[anOffset + 1]);
      const int c = hasThird ? base64Value(aText[anOffset + 2]) : 0;
      const int d = hasFourth ? base64Value(aText[anOffset + 3]) : 0;
      const bool isLast = anOffset + 4 == aText.size();
      if (a < 0 || b < 0 || c < 0 || d < 0 || (!hasThird && hasFourth)
          || ((!hasThird || !hasFourth) && !isLast))
      {
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF buffer has invalid base64 data");
      }
      const std::uint32_t aValue = static_cast<std::uint32_t>(a) << 18
        | static_cast<std::uint32_t>(b) << 12
        | static_cast<std::uint32_t>(c) << 6
        | static_cast<std::uint32_t>(d);
      aResult.push_back(static_cast<std::uint8_t>(aValue >> 16));
      if (hasThird) aResult.push_back(static_cast<std::uint8_t>(aValue >> 8));
      if (hasFourth) aResult.push_back(static_cast<std::uint8_t>(aValue));
    }
    return aResult;
  }

  static std::vector<std::uint8_t> decodeGltfDataUri(const std::string& theUri)
  {
    const std::size_t aComma = theUri.find(',');
    if (theUri.rfind("data:", 0) != 0 || aComma == std::string::npos)
      throw KernelFailure(ErrorCode::ImportExportFailed, "glTF data URI is invalid");
    const std::string aHeader = theUri.substr(5, aComma - 5);
    const std::string aPayload = theUri.substr(aComma + 1);
    if (aHeader.size() >= 7 && aHeader.compare(aHeader.size() - 7, 7, ";base64") == 0)
      return decodeBase64(aPayload);

    const auto hexValue = [](const char theCharacter) {
      if (theCharacter >= '0' && theCharacter <= '9') return theCharacter - '0';
      if (theCharacter >= 'a' && theCharacter <= 'f') return theCharacter - 'a' + 10;
      if (theCharacter >= 'A' && theCharacter <= 'F') return theCharacter - 'A' + 10;
      return -1;
    };
    std::vector<std::uint8_t> aResult;
    aResult.reserve(aPayload.size());
    for (std::size_t anIndex = 0; anIndex < aPayload.size(); ++anIndex)
    {
      if (aPayload[anIndex] != '%')
      {
        aResult.push_back(static_cast<std::uint8_t>(aPayload[anIndex]));
        continue;
      }
      if (anIndex + 2 >= aPayload.size())
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF data URI has invalid percent encoding");
      const int aHigh = hexValue(aPayload[anIndex + 1]);
      const int aLow = hexValue(aPayload[anIndex + 2]);
      if (aHigh < 0 || aLow < 0)
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF data URI has invalid percent encoding");
      aResult.push_back(static_cast<std::uint8_t>((aHigh << 4) | aLow));
      anIndex += 2;
    }
    return aResult;
  }

  static void appendU32LE(std::vector<std::uint8_t>& theData, const std::uint32_t theValue)
  {
    theData.push_back(static_cast<std::uint8_t>(theValue));
    theData.push_back(static_cast<std::uint8_t>(theValue >> 8));
    theData.push_back(static_cast<std::uint8_t>(theValue >> 16));
    theData.push_back(static_cast<std::uint8_t>(theValue >> 24));
  }

  static std::uint32_t readU32LE(const std::string& theData, const std::size_t theOffset)
  {
    if (theOffset > theData.size() || theData.size() - theOffset < 4)
    {
      throw KernelFailure(ErrorCode::ImportExportFailed, "GLB data is truncated");
    }
    const auto* aData = reinterpret_cast<const std::uint8_t*>(theData.data() + theOffset);
    return static_cast<std::uint32_t>(aData[0])
      | static_cast<std::uint32_t>(aData[1]) << 8
      | static_cast<std::uint32_t>(aData[2]) << 16
      | static_cast<std::uint32_t>(aData[3]) << 24;
  }

  static void appendFloat32LE(std::vector<std::uint8_t>& theData, const float theValue)
  {
    std::uint32_t aBits = 0;
    std::memcpy(&aBits, &theValue, sizeof(float));
    appendU32LE(theData, aBits);
  }

  static std::vector<std::uint8_t> writeGLTF(const TriangleMesh& theMesh, const bool theBinary)
  {
    if (theMesh.positions.size() > std::numeric_limits<std::uint32_t>::max()
        || theMesh.indices.size() > std::numeric_limits<std::uint32_t>::max())
    {
      throw KernelFailure(ErrorCode::ImportExportFailed, "glTF mesh is too large");
    }

    std::vector<std::uint8_t> aBinary;
    aBinary.reserve(theMesh.positions.size() * (12 + 12 + 8) + theMesh.indices.size() * 4 + 12);
    std::array<double, 3> aMinimum = {
      std::numeric_limits<double>::infinity(),
      std::numeric_limits<double>::infinity(),
      std::numeric_limits<double>::infinity()};
    std::array<double, 3> aMaximum = {
      -std::numeric_limits<double>::infinity(),
      -std::numeric_limits<double>::infinity(),
      -std::numeric_limits<double>::infinity()};
    for (const gp_Pnt& aPoint : theMesh.positions)
    {
      const std::array<double, 3> aGltf = {aPoint.X(), aPoint.Z(), -aPoint.Y()};
      for (std::size_t anAxis = 0; anAxis < 3; ++anAxis)
      {
        aMinimum[anAxis] = std::min(aMinimum[anAxis], aGltf[anAxis]);
        aMaximum[anAxis] = std::max(aMaximum[anAxis], aGltf[anAxis]);
        appendFloat32LE(aBinary, static_cast<float>(aGltf[anAxis]));
      }
    }
    while (aBinary.size() % 4 != 0) aBinary.push_back(0);
    const std::size_t aNormalOffset = aBinary.size();
    for (const gp_Dir& aNormal : theMesh.normals)
    {
      appendFloat32LE(aBinary, static_cast<float>(aNormal.X()));
      appendFloat32LE(aBinary, static_cast<float>(aNormal.Z()));
      appendFloat32LE(aBinary, static_cast<float>(-aNormal.Y()));
    }
    while (aBinary.size() % 4 != 0) aBinary.push_back(0);
    const std::size_t anUvOffset = aBinary.size();
    for (const gp_Pnt2d& aUV : theMesh.uvs)
    {
      appendFloat32LE(aBinary, static_cast<float>(aUV.X()));
      appendFloat32LE(aBinary, static_cast<float>(aUV.Y()));
    }
    while (aBinary.size() % 4 != 0) aBinary.push_back(0);
    const std::size_t anIndexOffset = aBinary.size();
    for (const std::uint32_t anIndex : theMesh.indices)
    {
      appendU32LE(aBinary, anIndex);
    }

    json aDocument = {
      {"asset", {{"generator", "occt-worker"}, {"version", "2.0"}}},
      {"scene", 0},
      {"scenes", json::array({{{"nodes", json::array({0})}}})},
      {"nodes", json::array({{{"mesh", 0}}})},
      {"meshes", json::array({{{"primitives", json::array({
        {{"attributes", {{"POSITION", 0}, {"NORMAL", 1}, {"TEXCOORD_0", 2}}}, {"indices", 3}, {"mode", 4}}
      })}}})},
      {"bufferViews", json::array({
        {{"buffer", 0}, {"byteLength", theMesh.positions.size() * 12}, {"byteOffset", 0}, {"target", 34962}},
        {{"buffer", 0}, {"byteLength", theMesh.normals.size() * 12},
         {"byteOffset", aNormalOffset}, {"target", 34962}},
        {{"buffer", 0}, {"byteLength", theMesh.uvs.size() * 8},
         {"byteOffset", anUvOffset}, {"target", 34962}},
        {{"buffer", 0}, {"byteLength", theMesh.indices.size() * 4},
         {"byteOffset", anIndexOffset}, {"target", 34963}}
      })},
      {"accessors", json::array({
        {{"bufferView", 0}, {"componentType", 5126},
         {"count", theMesh.positions.size()}, {"type", "VEC3"},
         {"min", aMinimum}, {"max", aMaximum}},
        {{"bufferView", 1}, {"componentType", 5126},
         {"count", theMesh.normals.size()}, {"type", "VEC3"}},
        {{"bufferView", 2}, {"componentType", 5126},
         {"count", theMesh.uvs.size()}, {"type", "VEC2"}},
        {{"bufferView", 3}, {"componentType", 5125},
         {"count", theMesh.indices.size()}, {"type", "SCALAR"}}
      })}
    };
    if (theBinary)
    {
      aDocument["buffers"] = json::array({{{"byteLength", aBinary.size()}}});
      std::string aJson = aDocument.dump();
      while (aJson.size() % 4 != 0) aJson.push_back(' ');
      while (aBinary.size() % 4 != 0) aBinary.push_back(0);
      const std::uint64_t aTotal = 12ull + 8ull + aJson.size() + 8ull + aBinary.size();
      if (aTotal > std::numeric_limits<std::uint32_t>::max())
      {
        throw KernelFailure(ErrorCode::ImportExportFailed, "GLB output is too large");
      }
      std::vector<std::uint8_t> aGlb;
      aGlb.reserve(static_cast<std::size_t>(aTotal));
      appendU32LE(aGlb, 0x46546c67);
      appendU32LE(aGlb, 2);
      appendU32LE(aGlb, static_cast<std::uint32_t>(aTotal));
      appendU32LE(aGlb, static_cast<std::uint32_t>(aJson.size()));
      appendU32LE(aGlb, 0x4e4f534a);
      aGlb.insert(aGlb.end(), aJson.begin(), aJson.end());
      appendU32LE(aGlb, static_cast<std::uint32_t>(aBinary.size()));
      appendU32LE(aGlb, 0x004e4942);
      aGlb.insert(aGlb.end(), aBinary.begin(), aBinary.end());
      return aGlb;
    }

    aDocument["buffers"] = json::array({{
      {"byteLength", aBinary.size()},
      {"uri", "data:application/octet-stream;base64," + encodeBase64(aBinary)}
    }});
    const std::string aJson = aDocument.dump();
    return std::vector<std::uint8_t>(aJson.begin(), aJson.end());
  }

  struct GltfDocumentExport
  {
    std::vector<std::uint8_t> data;
    std::vector<std::pair<std::string, std::vector<std::uint8_t>>> resources;
  };

  static GltfDocumentExport writeGLTFDocument(
    const json& theSourceDocument,
    const std::vector<std::pair<std::string, std::vector<std::uint8_t>>>& theBuffers,
    const std::vector<std::pair<std::string, std::vector<std::uint8_t>>>& theResources,
    const bool theBinary)
  {
    if (!theSourceDocument.is_object()
        || !theSourceDocument.contains("asset")
        || !theSourceDocument.at("asset").is_object()
        || theSourceDocument.at("asset").value("version", "") != "2.0")
      throw KernelFailure(ErrorCode::InvalidArgs, "glTF document must be a glTF 2.0 object");
    json aDocument = theSourceDocument;
    static const json anEmptyArray = json::array();
    const json& aBufferDefinitions = aDocument.contains("buffers")
      ? aDocument.at("buffers") : anEmptyArray;
    if (!aBufferDefinitions.is_array() || aBufferDefinitions.size() != theBuffers.size())
      throw KernelFailure(ErrorCode::InvalidArgs,
                          "glTF document buffers must match supplied buffers");
    for (std::size_t aBufferIndex = 0; aBufferIndex < theBuffers.size(); ++aBufferIndex)
    {
      const json& aDefinition = aBufferDefinitions.at(aBufferIndex);
      if (!aDefinition.is_object() || !aDefinition.contains("byteLength")
          || !aDefinition.at("byteLength").is_number_unsigned()
          || aDefinition.at("byteLength").get<std::size_t>() != theBuffers[aBufferIndex].second.size())
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "glTF document buffer byteLength must match supplied data");
    }

    GltfDocumentExport aResult;
    std::unordered_set<std::string> aResourceUris;
    const auto appendResource = [&](const std::string& theUri,
                                    const std::vector<std::uint8_t>& theData) {
      if (theUri.empty() || theUri.rfind("data:", 0) == 0) return;
      if (!aResourceUris.insert(theUri).second)
      {
        const auto anExisting = std::find_if(
          aResult.resources.begin(), aResult.resources.end(),
          [&](const auto& theResource) { return theResource.first == theUri; });
        if (anExisting != aResult.resources.end() && anExisting->second == theData) return;
        throw KernelFailure(ErrorCode::InvalidArgs,
                            "glTF export resource URI has conflicting data");
      }
      aResult.resources.emplace_back(theUri, theData);
    };

    std::vector<std::uint8_t> aBinary;
    if (theBinary)
    {
      std::vector<std::size_t> aBufferOffsets(theBuffers.size(), 0);
      for (std::size_t aBufferIndex = 0; aBufferIndex < theBuffers.size(); ++aBufferIndex)
      {
        while (aBinary.size() % 4 != 0) aBinary.push_back(0);
        aBufferOffsets[aBufferIndex] = aBinary.size();
        if (theBuffers[aBufferIndex].second.size()
            > std::numeric_limits<std::size_t>::max() - aBinary.size())
          throw KernelFailure(ErrorCode::InvalidArgs, "glTF document buffers are too large");
        aBinary.insert(aBinary.end(), theBuffers[aBufferIndex].second.begin(),
                       theBuffers[aBufferIndex].second.end());
      }
      if (aDocument.contains("bufferViews"))
      {
        if (!aDocument.at("bufferViews").is_array())
          throw KernelFailure(ErrorCode::InvalidArgs, "glTF document bufferViews must be an array");
        for (json& aView : aDocument["bufferViews"])
        {
          if (!aView.is_object() || !aView.contains("buffer")
              || !aView.at("buffer").is_number_unsigned())
            throw KernelFailure(ErrorCode::InvalidArgs, "glTF document bufferView is invalid");
          const std::size_t aBufferIndex = aView.at("buffer").get<std::size_t>();
          if (aBufferIndex >= aBufferOffsets.size())
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "glTF document bufferView buffer is out of range");
          if (!aView.contains("byteLength") || !aView.at("byteLength").is_number_unsigned())
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "glTF document bufferView byteLength is invalid");
          const std::size_t anOldOffset = aView.value("byteOffset", std::size_t(0));
          const std::size_t aViewLength = aView.at("byteLength").get<std::size_t>();
          if (anOldOffset > theBuffers[aBufferIndex].second.size()
              || aViewLength > theBuffers[aBufferIndex].second.size() - anOldOffset)
            throw KernelFailure(ErrorCode::InvalidArgs,
                                "glTF document bufferView exceeds supplied buffer data");
          if (anOldOffset > std::numeric_limits<std::size_t>::max() - aBufferOffsets[aBufferIndex])
            throw KernelFailure(ErrorCode::InvalidArgs, "glTF document bufferView offset is too large");
          aView["buffer"] = 0;
          const std::size_t aNewOffset = aBufferOffsets[aBufferIndex] + anOldOffset;
          if (aNewOffset == 0) aView.erase("byteOffset");
          else aView["byteOffset"] = aNewOffset;
        }
      }
      if (aBinary.empty()) aDocument.erase("buffers");
      else
      {
        json aBufferDefinition = theBuffers.size() == 1
          ? aBufferDefinitions.at(0) : json::object();
        aBufferDefinition.erase("uri");
        aBufferDefinition["byteLength"] = aBinary.size();
        aDocument["buffers"] = json::array({std::move(aBufferDefinition)});
      }
    }
    else
    {
      json aDefinitions = json::array();
      for (std::size_t aBufferIndex = 0; aBufferIndex < theBuffers.size(); ++aBufferIndex)
      {
        json aDefinition = aBufferDefinitions.at(aBufferIndex);
        std::string aUri = theBuffers[aBufferIndex].first;
        if (aUri.empty() && aDefinition.contains("uri") && aDefinition.at("uri").is_string())
          aUri = aDefinition.at("uri").get<std::string>();
        if (aUri.rfind("data:", 0) == 0)
        {
          aDefinition["uri"] = "data:application/octet-stream;base64,"
            + encodeBase64(theBuffers[aBufferIndex].second);
        }
        else
        {
          if (aUri.empty()) aUri = "buffer" + std::to_string(aBufferIndex) + ".bin";
          aDefinition["uri"] = aUri;
          appendResource(aUri, theBuffers[aBufferIndex].second);
        }
        aDefinitions.push_back(std::move(aDefinition));
      }
      if (aDefinitions.empty()) aDocument.erase("buffers");
      else aDocument["buffers"] = std::move(aDefinitions);
    }
    for (const auto& aResource : theResources)
      appendResource(aResource.first, aResource.second);
    if (theBinary && aDocument.contains("images") && aDocument.at("images").is_array())
    {
      for (const json& anImage : aDocument.at("images"))
      {
        if (!anImage.is_object() || !anImage.contains("uri")
            || !anImage.at("uri").is_string()) continue;
        const std::string aUri = anImage.at("uri").get<std::string>();
        if (aUri.rfind("data:", 0) == 0 || aResourceUris.find(aUri) != aResourceUris.end()) continue;
        const auto aBuffer = std::find_if(
          theBuffers.begin(), theBuffers.end(),
          [&](const auto& theBuffer) { return theBuffer.first == aUri; });
        if (aBuffer != theBuffers.end()) appendResource(aUri, aBuffer->second);
      }
    }

    std::string aJson = aDocument.dump();
    if (!theBinary)
    {
      aResult.data.assign(aJson.begin(), aJson.end());
      return aResult;
    }
    while (aJson.size() % 4 != 0) aJson.push_back(' ');
    while (aBinary.size() % 4 != 0) aBinary.push_back(0);
    const std::uint64_t aTotal = 12ull + 8ull + aJson.size()
      + (aBinary.empty() ? 0ull : 8ull + aBinary.size());
    if (aTotal > std::numeric_limits<std::uint32_t>::max())
      throw KernelFailure(ErrorCode::InvalidArgs, "GLB document output is too large");
    aResult.data.reserve(static_cast<std::size_t>(aTotal));
    appendU32LE(aResult.data, 0x46546c67);
    appendU32LE(aResult.data, 2);
    appendU32LE(aResult.data, static_cast<std::uint32_t>(aTotal));
    appendU32LE(aResult.data, static_cast<std::uint32_t>(aJson.size()));
    appendU32LE(aResult.data, 0x4e4f534a);
    aResult.data.insert(aResult.data.end(), aJson.begin(), aJson.end());
    if (!aBinary.empty())
    {
      appendU32LE(aResult.data, static_cast<std::uint32_t>(aBinary.size()));
      appendU32LE(aResult.data, 0x004e4942);
      aResult.data.insert(aResult.data.end(), aBinary.begin(), aBinary.end());
    }
    return aResult;
  }

  struct GltfAccessor
  {
    std::vector<std::uint8_t> data;
    std::size_t count = 0;
    std::size_t stride = 0;
    std::size_t elementSize = 0;
    std::size_t components = 0;
    int componentType = 0;
    bool normalized = false;
  };

  static std::size_t gltfComponentSize(const int theComponentType)
  {
    switch (theComponentType)
    {
      case 5120:
      case 5121: return 1;
      case 5122:
      case 5123: return 2;
      case 5125:
      case 5126: return 4;
      default:
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF accessor component type is unsupported");
    }
  }

  static GltfAccessor gltfAccessor(const json& theDocument,
                                   const std::vector<std::vector<std::uint8_t>>& theBuffers,
                                   const std::size_t theAccessorIndex,
                                   const std::string& theExpectedType)
  {
    const json& anAccessors = theDocument.at("accessors");
    if (!anAccessors.is_array() || theAccessorIndex >= anAccessors.size())
    {
      throw KernelFailure(ErrorCode::ImportExportFailed, "glTF accessor index is out of range");
    }
    const json& anAccessor = anAccessors.at(theAccessorIndex);
    if (!anAccessor.is_object() || !anAccessor.contains("componentType")
        || !anAccessor.at("componentType").is_number_integer()
        || !anAccessor.contains("count") || !anAccessor.at("count").is_number_unsigned()
        || (anAccessor.contains("normalized") && !anAccessor.at("normalized").is_boolean())
        || anAccessor.value("type", "") != theExpectedType
        || (!anAccessor.contains("bufferView") && !anAccessor.contains("sparse")))
    {
      throw KernelFailure(ErrorCode::ImportExportFailed, "glTF accessor layout is unsupported");
    }

    const std::int64_t aComponentTypeValue = anAccessor.at("componentType").get<std::int64_t>();
    if (aComponentTypeValue < std::numeric_limits<int>::min()
        || aComponentTypeValue > std::numeric_limits<int>::max())
      throw KernelFailure(ErrorCode::ImportExportFailed,
                          "glTF accessor component type is unsupported");
    const int aComponentType = static_cast<int>(aComponentTypeValue);
    const std::size_t aComponents = theExpectedType == "MAT4" ? 16
      : (theExpectedType == "VEC4" ? 4
      : (theExpectedType == "VEC3" ? 3 : (theExpectedType == "VEC2" ? 2 : 1)));
    const std::size_t aComponentSize = gltfComponentSize(aComponentType);
    const std::size_t anElementSize = aComponentSize * aComponents;
    const std::uint64_t aCountValue = anAccessor.at("count").get<std::uint64_t>();
    if (aCountValue > std::numeric_limits<std::size_t>::max())
      throw KernelFailure(ErrorCode::ImportExportFailed, "glTF accessor is too large");
    const std::size_t aCount = static_cast<std::size_t>(aCountValue);
    const bool isNormalized = anAccessor.value("normalized", false);
    if (isNormalized && (aComponentType == 5125 || aComponentType == 5126))
      throw KernelFailure(ErrorCode::ImportExportFailed, "glTF accessor normalization is invalid");
    if (aCount > std::numeric_limits<std::size_t>::max() / anElementSize)
      throw KernelFailure(ErrorCode::ImportExportFailed, "glTF accessor is too large");

    GltfAccessor aResult;
    aResult.data.resize(aCount * anElementSize, 0);
    aResult.count = aCount;
    aResult.stride = anElementSize;
    aResult.elementSize = anElementSize;
    aResult.components = aComponents;
    aResult.componentType = aComponentType;
    aResult.normalized = isNormalized;
    const json& aViews = theDocument.at("bufferViews");

    if (anAccessor.contains("bufferView"))
    {
      const std::size_t aViewIndex = anAccessor.at("bufferView").get<std::size_t>();
      if (!aViews.is_array() || aViewIndex >= aViews.size())
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF buffer view index is out of range");
      const json& aView = aViews.at(aViewIndex);
      const std::size_t aBufferIndex = aView.at("buffer").get<std::size_t>();
      if (aBufferIndex >= theBuffers.size())
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF buffer index is out of range");
      const std::size_t aStride = aView.value("byteStride", anElementSize);
      if (aStride < anElementSize || aStride % aComponentSize != 0
          || (aView.contains("byteStride")
              && (aStride < 4 || aStride > 252 || aStride % 4 != 0)))
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF accessor stride is invalid");
      const std::size_t aViewOffset = aView.value("byteOffset", std::size_t(0));
      const std::size_t aViewLength = aView.at("byteLength").get<std::size_t>();
      const std::size_t anAccessorOffset = anAccessor.value("byteOffset", std::size_t(0));
      const std::vector<std::uint8_t>& aBuffer = theBuffers[aBufferIndex];
      if (aViewOffset % aComponentSize != 0 || anAccessorOffset % aComponentSize != 0)
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF accessor offset is misaligned");
      if (aViewOffset > aBuffer.size() || aViewLength > aBuffer.size() - aViewOffset
          || anAccessorOffset > aViewLength
          || (aCount > 0
              && (anElementSize > aViewLength - anAccessorOffset
                  || aCount - 1 > (aViewLength - anAccessorOffset - anElementSize) / aStride)))
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF accessor data is truncated");
      const std::size_t anOffset = aViewOffset + anAccessorOffset;
      for (std::size_t anElement = 0; anElement < aCount; ++anElement)
      {
        std::memcpy(aResult.data.data() + anElement * anElementSize,
                    aBuffer.data() + anOffset + anElement * aStride,
                    anElementSize);
      }
    }
    else if (anAccessor.contains("byteOffset"))
    {
      throw KernelFailure(ErrorCode::ImportExportFailed,
                          "glTF accessor offset requires a buffer view");
    }

    if (anAccessor.contains("sparse"))
    {
      const json& aSparse = anAccessor.at("sparse");
      const std::size_t aSparseCount = aSparse.at("count").get<std::size_t>();
      if (aSparseCount == 0 || aSparseCount > aCount)
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF sparse accessor count is invalid");
      const json& anIndices = aSparse.at("indices");
      const int anIndexType = anIndices.at("componentType").get<int>();
      if (anIndexType != 5121 && anIndexType != 5123 && anIndexType != 5125)
        throw KernelFailure(ErrorCode::ImportExportFailed,
                            "glTF sparse index component type is unsupported");
      const std::size_t anIndexSize = gltfComponentSize(anIndexType);
      const std::size_t anIndexViewIndex = anIndices.at("bufferView").get<std::size_t>();
      const json& aValues = aSparse.at("values");
      const std::size_t aValueViewIndex = aValues.at("bufferView").get<std::size_t>();
      if (!aViews.is_array() || anIndexViewIndex >= aViews.size()
          || aValueViewIndex >= aViews.size())
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF buffer view index is out of range");
      const json& anIndexView = aViews.at(anIndexViewIndex);
      const json& aValueView = aViews.at(aValueViewIndex);
      const std::size_t anIndexBufferIndex = anIndexView.at("buffer").get<std::size_t>();
      const std::size_t aValueBufferIndex = aValueView.at("buffer").get<std::size_t>();
      if (anIndexBufferIndex >= theBuffers.size() || aValueBufferIndex >= theBuffers.size())
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF buffer index is out of range");
      const auto& anIndexBuffer = theBuffers[anIndexBufferIndex];
      const auto& aValueBuffer = theBuffers[aValueBufferIndex];
      const std::size_t anIndexViewOffset = anIndexView.value("byteOffset", std::size_t(0));
      const std::size_t anIndexViewLength = anIndexView.at("byteLength").get<std::size_t>();
      const std::size_t anIndexOffset = anIndices.value("byteOffset", std::size_t(0));
      const std::size_t aValueViewOffset = aValueView.value("byteOffset", std::size_t(0));
      const std::size_t aValueViewLength = aValueView.at("byteLength").get<std::size_t>();
      const std::size_t aValueOffset = aValues.value("byteOffset", std::size_t(0));
      if (anIndexViewOffset % anIndexSize != 0 || anIndexOffset % anIndexSize != 0
          || aValueViewOffset % aComponentSize != 0 || aValueOffset % aComponentSize != 0)
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF sparse accessor offset is misaligned");
      if (anIndexViewOffset > anIndexBuffer.size()
          || anIndexViewLength > anIndexBuffer.size() - anIndexViewOffset
          || anIndexOffset > anIndexViewLength
          || aSparseCount > (anIndexViewLength - anIndexOffset) / anIndexSize
          || aValueViewOffset > aValueBuffer.size()
          || aValueViewLength > aValueBuffer.size() - aValueViewOffset
          || aValueOffset > aValueViewLength
          || aSparseCount > (aValueViewLength - aValueOffset) / anElementSize)
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF sparse accessor data is truncated");

      const std::uint8_t* anIndexData = anIndexBuffer.data()
        + anIndexViewOffset + anIndexOffset;
      const std::uint8_t* aValueData = aValueBuffer.data()
        + aValueViewOffset + aValueOffset;
      std::uint32_t aPreviousIndex = 0;
      for (std::size_t aSparseIndex = 0; aSparseIndex < aSparseCount; ++aSparseIndex)
      {
        std::uint32_t anElement = 0;
        const std::uint8_t* anIndexValue = anIndexData + aSparseIndex * anIndexSize;
        if (anIndexType == 5121) anElement = *anIndexValue;
        else if (anIndexType == 5123)
        {
          std::uint16_t aShort = 0;
          std::memcpy(&aShort, anIndexValue, sizeof(aShort));
          anElement = aShort;
        }
        else std::memcpy(&anElement, anIndexValue, sizeof(anElement));
        if (anElement >= aCount || (aSparseIndex > 0 && anElement <= aPreviousIndex))
          throw KernelFailure(ErrorCode::ImportExportFailed,
                              "glTF sparse accessor indices are invalid");
        std::memcpy(aResult.data.data() + static_cast<std::size_t>(anElement) * anElementSize,
                    aValueData + aSparseIndex * anElementSize,
                    anElementSize);
        aPreviousIndex = anElement;
      }
    }
    return aResult;
  }

  static double gltfAccessorValue(const GltfAccessor& theAccessor,
                                  const std::size_t theElement,
                                  const std::size_t theComponent)
  {
    if (theElement >= theAccessor.count || theComponent >= theAccessor.components)
      throw KernelFailure(ErrorCode::ImportExportFailed, "glTF accessor component is out of range");
    const std::uint8_t* aValue = theAccessor.data.data()
      + theElement * theAccessor.stride
      + theComponent * gltfComponentSize(theAccessor.componentType);
    switch (theAccessor.componentType)
    {
      case 5120:
      {
        std::int8_t aScalar = 0;
        std::memcpy(&aScalar, aValue, sizeof(aScalar));
        return theAccessor.normalized ? std::max(-1.0, static_cast<double>(aScalar) / 127.0)
                                      : static_cast<double>(aScalar);
      }
      case 5121:
      {
        const std::uint8_t aScalar = *aValue;
        return theAccessor.normalized ? static_cast<double>(aScalar) / 255.0
                                      : static_cast<double>(aScalar);
      }
      case 5122:
      {
        std::int16_t aScalar = 0;
        std::memcpy(&aScalar, aValue, sizeof(aScalar));
        return theAccessor.normalized ? std::max(-1.0, static_cast<double>(aScalar) / 32767.0)
                                      : static_cast<double>(aScalar);
      }
      case 5123:
      {
        std::uint16_t aScalar = 0;
        std::memcpy(&aScalar, aValue, sizeof(aScalar));
        return theAccessor.normalized ? static_cast<double>(aScalar) / 65535.0
                                      : static_cast<double>(aScalar);
      }
      case 5125:
      {
        std::uint32_t aScalar = 0;
        std::memcpy(&aScalar, aValue, sizeof(aScalar));
        return static_cast<double>(aScalar);
      }
      case 5126:
      {
        float aScalar = 0.0f;
        std::memcpy(&aScalar, aValue, sizeof(aScalar));
        return static_cast<double>(aScalar);
      }
      default:
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF accessor component type is unsupported");
    }
  }

  using GltfMatrix = std::array<double, 16>;

  struct GltfEvaluatedNode
  {
    bool hasTranslation = false;
    bool hasRotation = false;
    bool hasScale = false;
    bool hasWeights = false;
    std::array<double, 4> translation{};
    std::array<double, 4> rotation{};
    std::array<double, 4> scale{};
    std::vector<double> weights;
  };

  static double gltfTransformDeterminant(const GltfMatrix& theMatrix)
  {
    return theMatrix[0] * (theMatrix[5] * theMatrix[10] - theMatrix[9] * theMatrix[6])
      - theMatrix[4] * (theMatrix[1] * theMatrix[10] - theMatrix[9] * theMatrix[2])
      + theMatrix[8] * (theMatrix[1] * theMatrix[6] - theMatrix[5] * theMatrix[2]);
  }

  static GltfMatrix multiplyGltfMatrices(const GltfMatrix& theLeft, const GltfMatrix& theRight)
  {
    GltfMatrix aResult{};
    for (std::size_t aColumn = 0; aColumn < 4; ++aColumn)
    {
      for (std::size_t aRow = 0; aRow < 4; ++aRow)
      {
        for (std::size_t anInner = 0; anInner < 4; ++anInner)
        {
          aResult[aColumn * 4 + aRow] +=
            theLeft[anInner * 4 + aRow] * theRight[aColumn * 4 + anInner];
        }
      }
    }
    return aResult;
  }

  static GltfMatrix gltfNodeMatrix(const json& theNode,
                                   const GltfEvaluatedNode* theEvaluated = nullptr)
  {
    const bool hasEvaluatedTransform = theEvaluated != nullptr
      && (theEvaluated->hasTranslation || theEvaluated->hasRotation
          || theEvaluated->hasScale);
    if (theNode.contains("matrix") && !hasEvaluatedTransform)
    {
      const json& aMatrix = theNode.at("matrix");
      if (!aMatrix.is_array() || aMatrix.size() != 16)
      {
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF node matrix is invalid");
      }
      GltfMatrix aResult{};
      for (std::size_t anIndex = 0; anIndex < 16; ++anIndex)
      {
        aResult[anIndex] = aMatrix.at(anIndex).get<double>();
        if (!std::isfinite(aResult[anIndex]))
        {
          throw KernelFailure(ErrorCode::ImportExportFailed, "glTF node matrix is not finite");
        }
      }
      return aResult;
    }

    const auto readVector = [&](const char* theName,
                                const std::array<double, 4>& theDefault,
                                const std::size_t theSize) {
      std::array<double, 4> aResult = theDefault;
      if (!theNode.contains(theName)) return aResult;
      const json& aValue = theNode.at(theName);
      if (!aValue.is_array() || aValue.size() != theSize)
      {
        throw KernelFailure(ErrorCode::ImportExportFailed, "glTF node transform is invalid");
      }
      for (std::size_t anIndex = 0; anIndex < theSize; ++anIndex)
      {
        aResult[anIndex] = aValue.at(anIndex).get<double>();
        if (!std::isfinite(aResult[anIndex]))
        {
          throw KernelFailure(ErrorCode::ImportExportFailed, "glTF node transform is not finite");
        }
      }
      return aResult;
    };
    auto aTranslation = readVector("translation", {0.0, 0.0, 0.0, 0.0}, 3);
    auto aScale = readVector("scale", {1.0, 1.0, 1.0, 0.0}, 3);
    auto aRotation = readVector("rotation", {0.0, 0.0, 0.0, 1.0}, 4);
    if (theEvaluated != nullptr)
    {
      if (theEvaluated->hasTranslation) aTranslation = theEvaluated->translation;
      if (theEvaluated->hasRotation) aRotation = theEvaluated->rotation;
      if (theEvaluated->hasScale) aScale = theEvaluated->scale;
    }
    const double aLength = std::sqrt(aRotation[0] * aRotation[0]
                                    + aRotation[1] * aRotation[1]
                                    + aRotation[2] * aRotation[2]
                                    + aRotation[3] * aRotation[3]);
    if (aLength <= std::numeric_limits<double>::epsilon())
    {
      throw KernelFailure(ErrorCode::ImportExportFailed, "glTF node quaternion is invalid");
    }
    for (double& aValue : aRotation) aValue /= aLength;
    const double x = aRotation[0], y = aRotation[1], z = aRotation[2], w = aRotation[3];
    GltfMatrix aResult = {
      (1.0 - 2.0 * (y * y + z * z)) * aScale[0],
      (2.0 * (x * y + z * w)) * aScale[0],
      (2.0 * (x * z - y * w)) * aScale[0],
      0.0,
      (2.0 * (x * y - z * w)) * aScale[1],
      (1.0 - 2.0 * (x * x + z * z)) * aScale[1],
      (2.0 * (y * z + x * w)) * aScale[1],
      0.0,
      (2.0 * (x * z + y * w)) * aScale[2],
      (2.0 * (y * z - x * w)) * aScale[2],
      (1.0 - 2.0 * (x * x + y * y)) * aScale[2],
      0.0,
      aTranslation[0], aTranslation[1], aTranslation[2], 1.0
    };
    return aResult;
  }

  static gp_Pnt transformGltfPoint(const GltfMatrix& theMatrix,
                                   const double theX,
                                   const double theY,
                                   const double theZ)
  {
    const double x = theMatrix[0] * theX + theMatrix[4] * theY
                   + theMatrix[8] * theZ + theMatrix[12];
    const double y = theMatrix[1] * theX + theMatrix[5] * theY
                   + theMatrix[9] * theZ + theMatrix[13];
    const double z = theMatrix[2] * theX + theMatrix[6] * theY
                   + theMatrix[10] * theZ + theMatrix[14];
    return gp_Pnt(x, -z, y);
  }

  static gp_Dir transformGltfNormal(const GltfMatrix& theMatrix,
                                    const double theX,
                                    const double theY,
                                    const double theZ)
  {
    const double aDeterminant = gltfTransformDeterminant(theMatrix);
    if (std::abs(aDeterminant) <= Precision::Confusion())
      throw KernelFailure(ErrorCode::ImportExportFailed, "glTF normal transform is singular");
    const double x = ((theMatrix[5] * theMatrix[10] - theMatrix[9] * theMatrix[6]) * theX
                      + (theMatrix[9] * theMatrix[2] - theMatrix[1] * theMatrix[10]) * theY
                      + (theMatrix[1] * theMatrix[6] - theMatrix[5] * theMatrix[2]) * theZ)
                     / aDeterminant;
    const double y = ((theMatrix[8] * theMatrix[6] - theMatrix[4] * theMatrix[10]) * theX
                      + (theMatrix[0] * theMatrix[10] - theMatrix[8] * theMatrix[2]) * theY
                      + (theMatrix[4] * theMatrix[2] - theMatrix[0] * theMatrix[6]) * theZ)
                     / aDeterminant;
    const double z = ((theMatrix[4] * theMatrix[9] - theMatrix[8] * theMatrix[5]) * theX
                      + (theMatrix[8] * theMatrix[1] - theMatrix[0] * theMatrix[9]) * theY
                      + (theMatrix[0] * theMatrix[5] - theMatrix[4] * theMatrix[1]) * theZ)
                     / aDeterminant;
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z)
        || x * x + y * y + z * z <= Precision::SquareConfusion())
      throw KernelFailure(ErrorCode::ImportExportFailed, "glTF transformed normal is invalid");
    return gp_Dir(x, -z, y);
  }

  static TriangleMesh parseGLTF(
    const std::string& theData,
    const std::unordered_map<std::string, std::string>& theResources = {},
    const bool theAllowNoTriangles = false,
    const std::vector<double>* theMorphWeights = nullptr,
    const std::size_t* theAnimationIndex = nullptr,
    const double* theAnimationTime = nullptr)
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

  static ErrorCode operationFailureCode(const std::string& theOp)
  {
    if (theOp == "booleanCut" || theOp == "booleanFuse" || theOp == "booleanCommon"
        || theOp == "generalFuse" || theOp == "selectGeneralFuseCells"
        || theOp == "section" || theOp == "split" || theOp == "defeature")
      return ErrorCode::BooleanFailed;
    if (theOp == "fillet" || theOp == "chamfer") return ErrorCode::FilletFailed;
    if (theOp == "tessellate" || theOp == "tessellateEdges" || theOp == "triangulationData"
        || theOp == "validateTriangulation") return ErrorCode::TessellationFailed;
    if (theOp == "repairTriangulation") return ErrorCode::HealingFailed;
    if (theOp == "replaceTriangulation") return ErrorCode::ConstructionFailed;
    if (theOp == "exportIGES" || theOp == "importIGES" || theOp == "exportSTEP" || theOp == "importSTEP"
        || theOp == "exportSTEPDocument" || theOp == "importSTEPDocument"
        || theOp == "exportIGESDocument" || theOp == "importIGESDocument"
        || theOp == "exportXCAF" || theOp == "importXCAF" || theOp == "exportBREP"
        || theOp == "importBREP" || theOp == "exportSTL" || theOp == "importSTL"
        || theOp == "exportGLTF" || theOp == "importGLTF"
        || theOp == "exportVRML" || theOp == "importVRML"
        || theOp == "exportOBJ" || theOp == "importOBJ" || theOp == "exportPLY"
        || theOp == "importPLY")
      return ErrorCode::ImportExportFailed;
    if (theOp == "hollow" || theOp == "offsetShape" || theOp == "sew"
        || theOp == "fixShape" || theOp == "unifySameDomain" || theOp == "shapeUpgrade")
      return ErrorCode::HealingFailed;
    if (theOp == "approximateCurveBSpline" || theOp == "approximateSurfaceBSpline"
        || theOp == "reduceCurveDegree" || theOp == "reduceSurfaceDegree"
        || theOp == "extendCurve" || theOp == "extendSurface"
        || theOp == "makeBox" || theOp == "makeCylinder" || theOp == "makeSphere"
        || theOp == "makeCone" || theOp == "makeTorus" || theOp == "makeWedge"
        || theOp == "makeHalfSpace" || theOp == "makeVertex"
        || theOp == "makePolygon" || theOp == "makeWire" || theOp == "makeFace"
        || theOp == "makeCompound" || theOp == "makeShell" || theOp == "makeCompSolid"
        || theOp == "makeSurfaceFace" || theOp == "makeSurfaceBezier"
        || theOp == "makeSurfaceBSpline" || theOp == "makeSurfaceExtrusion"
        || theOp == "makeSurfaceOffset" || theOp == "makeSurfaceRevolution"
        || theOp == "makeSurfaceRuled" || theOp == "surfaceIsoCurve"
        || theOp == "makeSolidFromShell" || theOp.rfind("makeEdge", 0) == 0
        || theOp == "trimCurve" || theOp == "trimSurface"
        || theOp == "convertCurveToBSpline" || theOp == "convertSurfaceToBSpline"
        || theOp == "updateCurvePole" || theOp == "updateSurfacePole"
        || theOp == "extrude" || theOp == "revolve" || theOp == "loft" || theOp == "draftAngle"
        || theOp == "localPrism" || theOp == "localRevolution" || theOp == "linearForm"
        || theOp == "cylindricalHole"
        || theOp == "sweepPipe" || theOp == "sweepPipeShell" || theOp == "offsetWire2D"
        || theOp == "transform" || theOp == "generalTransform" || theOp == "batchTransformCopy"
        || theOp == "translate" || theOp == "rotate" || theOp == "scale" || theOp == "mirror")
      return ErrorCode::ConstructionFailed;
    return ErrorCode::KernelError;
  }

  static const char* continuityName(const GeomAbs_Shape theContinuity)
  {
    switch (theContinuity)
    {
      case GeomAbs_C0: return "c0";
      case GeomAbs_G1: return "g1";
      case GeomAbs_C1: return "c1";
      case GeomAbs_G2: return "g2";
      case GeomAbs_C2: return "c2";
      case GeomAbs_C3: return "c3";
      case GeomAbs_CN: return "cn";
    }
    return "c0";
  }

  static TopAbs_ShapeEnum shapeTypeFromName(const std::string& theName)
  {
    if (theName == "compound") return TopAbs_COMPOUND;
    if (theName == "compsolid") return TopAbs_COMPSOLID;
    if (theName == "solid") return TopAbs_SOLID;
    if (theName == "shell") return TopAbs_SHELL;
    if (theName == "face") return TopAbs_FACE;
    if (theName == "wire") return TopAbs_WIRE;
    if (theName == "edge") return TopAbs_EDGE;
    if (theName == "vertex") return TopAbs_VERTEX;
    throw KernelFailure(ErrorCode::InvalidArgs, "Unknown topology type");
  }

  static const char* shapeTypeName(const TopAbs_ShapeEnum theType)
  {
    switch (theType)
    {
      case TopAbs_COMPOUND: return "compound";
      case TopAbs_COMPSOLID: return "compsolid";
      case TopAbs_SOLID: return "solid";
      case TopAbs_SHELL: return "shell";
      case TopAbs_FACE: return "face";
      case TopAbs_WIRE: return "wire";
      case TopAbs_EDGE: return "edge";
      case TopAbs_VERTEX: return "vertex";
      case TopAbs_SHAPE: return "shape";
      default: return "unknown";
    }
  }

  static const char* checkStatusName(const BRepCheck_Status theStatus)
  {
    switch (theStatus)
    {
      case BRepCheck_NoError: return "noError";
      case BRepCheck_InvalidPointOnCurve: return "invalidPointOnCurve";
      case BRepCheck_InvalidPointOnCurveOnSurface: return "invalidPointOnCurveOnSurface";
      case BRepCheck_InvalidPointOnSurface: return "invalidPointOnSurface";
      case BRepCheck_No3DCurve: return "no3DCurve";
      case BRepCheck_Multiple3DCurve: return "multiple3DCurve";
      case BRepCheck_Invalid3DCurve: return "invalid3DCurve";
      case BRepCheck_NoCurveOnSurface: return "noCurveOnSurface";
      case BRepCheck_InvalidCurveOnSurface: return "invalidCurveOnSurface";
      case BRepCheck_InvalidCurveOnClosedSurface: return "invalidCurveOnClosedSurface";
      case BRepCheck_InvalidSameRangeFlag: return "invalidSameRangeFlag";
      case BRepCheck_InvalidSameParameterFlag: return "invalidSameParameterFlag";
      case BRepCheck_InvalidDegeneratedFlag: return "invalidDegeneratedFlag";
      case BRepCheck_FreeEdge: return "freeEdge";
      case BRepCheck_InvalidMultiConnexity: return "invalidMultiConnexity";
      case BRepCheck_InvalidRange: return "invalidRange";
      case BRepCheck_EmptyWire: return "emptyWire";
      case BRepCheck_RedundantEdge: return "redundantEdge";
      case BRepCheck_SelfIntersectingWire: return "selfIntersectingWire";
      case BRepCheck_NoSurface: return "noSurface";
      case BRepCheck_InvalidWire: return "invalidWire";
      case BRepCheck_RedundantWire: return "redundantWire";
      case BRepCheck_IntersectingWires: return "intersectingWires";
      case BRepCheck_InvalidImbricationOfWires: return "invalidImbricationOfWires";
      case BRepCheck_EmptyShell: return "emptyShell";
      case BRepCheck_RedundantFace: return "redundantFace";
      case BRepCheck_InvalidImbricationOfShells: return "invalidImbricationOfShells";
      case BRepCheck_UnorientableShape: return "unorientableShape";
      case BRepCheck_NotClosed: return "notClosed";
      case BRepCheck_NotConnected: return "notConnected";
      case BRepCheck_SubshapeNotInShape: return "subshapeNotInShape";
      case BRepCheck_BadOrientation: return "badOrientation";
      case BRepCheck_BadOrientationOfSubshape: return "badOrientationOfSubshape";
      case BRepCheck_InvalidPolygonOnTriangulation: return "invalidPolygonOnTriangulation";
      case BRepCheck_InvalidToleranceValue: return "invalidToleranceValue";
      case BRepCheck_EnclosedRegion: return "enclosedRegion";
      case BRepCheck_CheckFail: return "checkFail";
    }
    return "unknown";
  }

  static std::uint32_t requiredU32(const json& theObject, const char* theKey)
  {
    if (!theObject.is_object() || !theObject.contains(theKey)
        || (!theObject.at(theKey).is_number_unsigned() && !theObject.at(theKey).is_number_integer()))
    {
      throw KernelFailure(ErrorCode::InvalidArgs, std::string("Missing integer argument: ") + theKey);
    }
    const std::int64_t aValue = theObject.at(theKey).get<std::int64_t>();
    if (aValue < 0 || aValue > UINT32_MAX)
    {
      throw KernelFailure(ErrorCode::InvalidArgs, std::string("Argument is outside u32 range: ") + theKey);
    }
    return static_cast<std::uint32_t>(aValue);
  }

  static double requiredNumber(const json& theObject, const char* theKey)
  {
    if (!theObject.contains(theKey) || !theObject.at(theKey).is_number())
    {
      throw KernelFailure(ErrorCode::InvalidArgs, std::string("Missing numeric argument: ") + theKey);
    }
    return theObject.at(theKey).get<double>();
  }

  static std::array<double, 3> requiredVec3(const json& theObject, const char* theKey)
  {
    if (!theObject.contains(theKey) || !theObject.at(theKey).is_array()
        || theObject.at(theKey).size() != 3)
    {
      throw KernelFailure(ErrorCode::InvalidArgs, std::string("Expected a three-number array: ") + theKey);
    }
    std::array<double, 3> aValue{};
    for (std::size_t anIndex = 0; anIndex < 3; ++anIndex)
    {
      if (!theObject.at(theKey).at(anIndex).is_number())
      {
        throw KernelFailure(ErrorCode::InvalidArgs, std::string("Expected a three-number array: ") + theKey);
      }
      aValue[anIndex] = theObject.at(theKey).at(anIndex).get<double>();
    }
    return aValue;
  }

  static std::array<double, 3> optionalVec3(const json& theObject,
                                             const char* theKey,
                                             const std::array<double, 3>& theDefault)
  {
    return theObject.contains(theKey) ? requiredVec3(theObject, theKey) : theDefault;
  }

  static std::array<double, 3> requiredDirection(const json& theObject, const char* theKey)
  {
    const std::array<double, 3> aValue = requiredVec3(theObject, theKey);
    const double aSquaredNorm = aValue[0] * aValue[0] + aValue[1] * aValue[1] + aValue[2] * aValue[2];
    if (aSquaredNorm <= 1.0e-30)
      throw KernelFailure(ErrorCode::InvalidArgs, std::string("Direction must be non-zero: ") + theKey);
    return aValue;
  }

  static std::array<double, 3> optionalDirection(const json& theObject,
                                                  const char* theKey,
                                                  const std::array<double, 3>& theDefault)
  {
    return theObject.contains(theKey) ? requiredDirection(theObject, theKey) : theDefault;
  }

  static TopoDS_Face makeRectangularFace(const occ::handle<Geom_Surface>& theSurface,
                                         const double                     theUFirst,
                                         const double                     theULast,
                                         const double                     theVFirst,
                                         const double                     theVLast,
                                         const double                     theTolerance,
                                         const TopAbs_Orientation         theOrientation)
  {
    if (theSurface.IsNull() || !std::isfinite(theUFirst) || !std::isfinite(theULast)
        || !std::isfinite(theVFirst) || !std::isfinite(theVLast)
        || theUFirst >= theULast || theVFirst >= theVLast || theTolerance <= 0.0)
      throw KernelFailure(ErrorCode::InvalidArgs, "Surface bounds must be increasing, finite, and tolerance positive");
    BRepBuilderAPI_MakeFace aBuilder(
      theSurface, theUFirst, theULast, theVFirst, theVLast, theTolerance);
    if (!aBuilder.IsDone() || aBuilder.Face().IsNull()
        || !BRepCheck_Analyzer(aBuilder.Face(), true).IsValid())
      throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to construct a valid bounded surface face");
    TopoDS_Face aFace = aBuilder.Face();
    aFace.Orientation(theOrientation);
    return aFace;
  }

  static TopoDS_Face makeSurfaceFacePreservingWires(
    const TopoDS_Face&                  theSourceFace,
    const occ::handle<Geom_Surface>&   theSurface,
    const double                       theTolerance,
    const TopAbs_Orientation           theOrientation,
    const std::array<double, 4>*        theOuterBounds = nullptr)
  {
    if (theSurface.IsNull() || theTolerance <= 0.0)
      throw KernelFailure(ErrorCode::InvalidArgs, "Surface bounds must be valid and tolerance positive");

    BRepBuilderAPI_Copy aCopy(theSourceFace, true, false);
    if (!aCopy.IsDone() || aCopy.Shape().IsNull())
      throw KernelFailure(ErrorCode::ConstructionFailed, "OCCT failed to copy the surface face topology");
    const TopoDS_Face aCopiedFace = TopoDS::Face(aCopy.Shape());

    BRep_Builder aShapeBuilder;
    for (TopExp_Explorer anExplorer(aCopiedFace, TopAbs_EDGE);
         anExplorer.More(); anExplorer.Next())
    {
      const TopoDS_Edge anEdge = TopoDS::Edge(anExplorer.Current());
      double aFirst = 0.0;
      double aLast = 0.0;
      if (BRep_Tool::IsClosed(anEdge, aCopiedFace))
      {
        occ::handle<Geom2d_Curve> aFirstPCurve;
        occ::handle<Geom2d_Curve> aSecondPCurve;
        occ::handle<Geom_Surface> anOldSurface;
        TopLoc_Location anOldLocation;
        BRep_Tool::CurveOnSurface(anEdge, aFirstPCurve, anOldSurface,
                                  anOldLocation, aFirst, aLast, 1);
        BRep_Tool::CurveOnSurface(anEdge, aSecondPCurve, anOldSurface,
                                  anOldLocation, aFirst, aLast, 2);
        if (aFirstPCurve.IsNull() || aSecondPCurve.IsNull())
          throw KernelFailure(ErrorCode::ConstructionFailed,
                              "OCCT could not preserve a closed-surface boundary pcurve");
        aShapeBuilder.UpdateEdge(anEdge, aFirstPCurve, aSecondPCurve, theSurface,
                                 TopLoc_Location(), theTolerance);
      }
      else
      {
        const occ::handle<Geom2d_Curve> aPCurve =
          BRep_Tool::CurveOnSurface(anEdge, aCopiedFace, aFirst, aLast);
        if (aPCurve.IsNull())
          throw KernelFailure(ErrorCode::ConstructionFailed,
                              "OCCT could not preserve a surface boundary pcurve");
        aShapeBuilder.UpdateEdge(anEdge, aPCurve, theSurface,
                                 TopLoc_Location(), theTolerance);
      }
      aShapeBuilder.UpdateEdge(anEdge, occ::handle<Geom_Curve>(), theTolerance);
      if (!BRepLib::BuildCurve3d(anEdge, theTolerance))
        throw KernelFailure(ErrorCode::ConstructionFailed,
                            "OCCT could not rebuild a surface boundary curve");
      BRepLib::SameParameter(anEdge, theTolerance);
    }

    const TopoDS_Wire anOuter = BRepTools::OuterWire(aCopiedFace);
    if (anOuter.IsNull())
      throw KernelFailure(ErrorCode::ConstructionFailed, "Surface face has no outer wire");
    TopoDS_Face aResult;
    if (theOuterBounds != nullptr)
    {
      BRepBuilderAPI_MakeFace aBuilder(theSurface, (*theOuterBounds)[0], (*theOuterBounds)[1],
                                       (*theOuterBounds)[2], (*theOuterBounds)[3], theTolerance);
      if (!aBuilder.IsDone() || aBuilder.Face().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed,
                            "OCCT failed to rebuild the extended surface boundary");
      aResult = aBuilder.Face();
    }
    else
    {
      BRepBuilderAPI_MakeFace aBuilder(theSurface, anOuter, true);
      if (!aBuilder.IsDone() || aBuilder.Face().IsNull())
        throw KernelFailure(ErrorCode::ConstructionFailed,
                            "OCCT failed to rebuild the surface face boundary");
      aResult = aBuilder.Face();
    }
    BRep_Builder aResultBuilder;
    for (TopExp_Explorer anExplorer(aCopiedFace, TopAbs_WIRE);
         anExplorer.More(); anExplorer.Next())
    {
      const TopoDS_Wire aWire = TopoDS::Wire(anExplorer.Current());
      if (!aWire.IsSame(anOuter))
        aResultBuilder.Add(aResult, aWire);
    }
    ShapeFix_Edge aEdgeFixer;
    for (TopExp_Explorer anExplorer(aResult, TopAbs_EDGE);
         anExplorer.More(); anExplorer.Next())
    {
      const TopoDS_Edge anEdge = TopoDS::Edge(anExplorer.Current());
      aEdgeFixer.FixVertexTolerance(anEdge, aResult);
      aEdgeFixer.FixSameParameter(anEdge, theTolerance);
    }
    if (!BRepCheck_Analyzer(aResult, true).IsValid())
      throw KernelFailure(ErrorCode::ConstructionFailed,
                          "OCCT failed to validate the rebuilt surface face");
    aResult.Orientation(theOrientation);
    return aResult;
  }

  static json bufferDescriptor(const std::uint32_t theId,
                               const std::size_t theLength,
                               const char* theLayout)
  {
    return {{"bufferId", theId}, {"byteLength", theLength}, {"layout", theLayout}};
  }

  std::string inputBufferData(const json& theArgs, const char* theKey)
  {
    if (!theArgs.contains(theKey) || !theArgs.at(theKey).is_object())
    {
      throw KernelFailure(ErrorCode::InvalidArgs,
                          std::string("Missing buffer reference argument: ") + theKey);
    }
    const json& aRef = theArgs.at(theKey);
    std::vector<std::uint8_t>& aBuffer = myBuffers.get(requiredU32(aRef, "bufferId"));
    const std::size_t anOffset = aRef.value("byteOffset", 0u);
    const std::size_t aLength =
      aRef.value("byteLength", aBuffer.size() - std::min(anOffset, aBuffer.size()));
    if (anOffset > aBuffer.size() || aLength > aBuffer.size() - anOffset)
    {
      throw KernelFailure(ErrorCode::InvalidArgs, "Buffer range is out of bounds");
    }
    return std::string(reinterpret_cast<const char*>(aBuffer.data() + anOffset), aLength);
  }

  static std::pair<UnitsMethods_LengthUnit, double> stepLengthUnit(const json& theArgs)
  {
    const std::string aName = theArgs.value("unit", "mm");
    if (aName == "mm") return {UnitsMethods_LengthUnit_Millimeter, 1.0};
    if (aName == "cm") return {UnitsMethods_LengthUnit_Centimeter, 10.0};
    if (aName == "m") return {UnitsMethods_LengthUnit_Meter, 1000.0};
    if (aName == "inch") return {UnitsMethods_LengthUnit_Inch, 25.4};
    if (aName == "foot") return {UnitsMethods_LengthUnit_Foot, 304.8};
    throw KernelFailure(ErrorCode::InvalidArgs, "Unsupported STEP length unit: " + aName);
  }

  static DESTEP_Parameters::WriteMode_StepSchema stepWriteSchema(const json& theArgs)
  {
    const std::string aName = theArgs.value("schema", "AP242");
    if (aName == "AP203") return DESTEP_Parameters::WriteMode_StepSchema_AP203;
    if (aName == "AP214") return DESTEP_Parameters::WriteMode_StepSchema_AP214IS;
    if (aName == "AP242") return DESTEP_Parameters::WriteMode_StepSchema_AP242DIS;
    throw KernelFailure(ErrorCode::InvalidArgs, "Unsupported STEP schema: " + aName);
  }

  static std::tuple<std::string, double, UnitsMethods_LengthUnit> igesLengthUnit(const json& theArgs)
  {
    const std::string aName = theArgs.value("unit", "mm");
    if (aName == "mm") return {"MM", 1.0, UnitsMethods_LengthUnit_Millimeter};
    if (aName == "cm") return {"CM", 10.0, UnitsMethods_LengthUnit_Centimeter};
    if (aName == "m") return {"M", 1000.0, UnitsMethods_LengthUnit_Meter};
    if (aName == "inch") return {"INCH", 25.4, UnitsMethods_LengthUnit_Inch};
    if (aName == "foot") return {"FT", 304.8, UnitsMethods_LengthUnit_Foot};
    throw KernelFailure(ErrorCode::InvalidArgs, "Unsupported IGES length unit: " + aName);
  }

  json tessellation(const TopoDS_Shape& theShape, const bool theIncludeUV = false)
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
      {
        continue;
      }
      if (!aTriangulation->HasNormals())
      {
        BRepLib_ToolTriangulatedShape::ComputeNormals(aFace, aTriangulation);
      }
      if (theIncludeUV && !aTriangulation->HasUVNodes())
      {
        throw KernelFailure(ErrorCode::TessellationFailed, "OCCT tessellation did not provide UV nodes");
      }
      const std::uint32_t aVertexBase = static_cast<std::uint32_t>(aPositions.size() / 3);
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
      for (int aTriangleIndex = 1; aTriangleIndex <= aTriangulation->NbTriangles(); ++aTriangleIndex)
      {
        int a, b, c;
        aTriangulation->Triangle(aTriangleIndex).Get(a, b, c);
        if (isReversed) std::swap(b, c);
        anIndices.insert(anIndices.end(), {aVertexBase + static_cast<std::uint32_t>(a - 1),
                                           aVertexBase + static_cast<std::uint32_t>(b - 1),
                                           aVertexBase + static_cast<std::uint32_t>(c - 1)});
      }
      aFaceGroups.insert(aFaceGroups.end(), {static_cast<std::uint32_t>(aFaceIndex - 1),
                                              aStart,
                                              static_cast<std::uint32_t>(anIndices.size()) - aStart});
    }

    const std::uint32_t aPositionsId = myBuffers.copy(aPositions);
    const std::uint32_t aNormalsId = myBuffers.copy(aNormals);
    const std::uint32_t anIndicesId = myBuffers.copy(anIndices);
    const std::uint32_t aGroupsId = myBuffers.copy(aFaceGroups);
    json aResult{{"positions", bufferDescriptor(aPositionsId, aPositions.size() * sizeof(float), "f32x3")},
                 {"normals", bufferDescriptor(aNormalsId, aNormals.size() * sizeof(float), "f32x3")},
                 {"indices", bufferDescriptor(anIndicesId, anIndices.size() * sizeof(std::uint32_t), "u32")},
                 {"faceGroups", bufferDescriptor(aGroupsId, aFaceGroups.size() * sizeof(std::uint32_t), "u32x3")}};
    if (theIncludeUV)
    {
      const std::uint32_t aUVsId = myBuffers.copy(aUVs);
      aResult["uvs"] = bufferDescriptor(aUVsId, aUVs.size() * sizeof(float), "f32x2");
    }
    return aResult;
  }

  template <typename TBuilder>
  static json buildFilletHistory(TBuilder& theBuilder,
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

        // OCCT fillet/chamfer history defines Generated() for input edges and
        // Modified()/IsDeleted() for input faces.
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

        // Deletion is independent from derivation: a selected edge can generate
        // a blend face while the original edge itself disappears.
        const bool isDeleted = aType.first == TopAbs_FACE
          ? theBuilder.IsDeleted(aSource) : aRetainedIndex == 0;
        if (isDeleted)
          aHistory["deleted"].push_back(aFrom);
      }
    }
    return aHistory;
  }

  template <typename TBuilder>
  static json buildHistory(TBuilder& theBuilder,
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
          const json aFrom{{"input", anInputIndex}, {"type", aType.second}, {"index", aShapeIndex - 1}};
          const int aRetainedIndex = anOutputMap.FindIndex(aSource);
          if (aRetainedIndex > 0)
          {
            aHistory["retained"].push_back({{"from", aFrom},
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
              for (const auto& aTargetType : std::vector<std::tuple<TopAbs_ShapeEnum,
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
                  const int aTargetIndex = std::get<2>(aTargetType)->FindIndex(aMappedTargets(aMappedIndex));
                  const std::uint64_t aTargetKey =
                    (static_cast<std::uint64_t>(std::get<0>(aTargetType)) << 32)
                    | static_cast<std::uint32_t>(aTargetIndex);
                  if (aTargetIndex > 0 && aSeenTargets.insert(aTargetKey).second)
                    aTargets.push_back({{"type", std::get<1>(aTargetType)}, {"index", aTargetIndex - 1}});
                }
              }
            }
            if (!aTargets.empty())
            {
              aHistory[theCategory].push_back({{"from", aFrom}, {"to", aTargets}});
            }
          };

          // Some OCCT builders return Generated() and Modified() through the
          // same mutable scratch list. Copy each result before the next query.
          const NCollection_List<TopoDS_Shape> aGenerated = theBuilder.Generated(aSource);
          appendCategory("generated", aGenerated);
          const NCollection_List<TopoDS_Shape> aModified = theBuilder.Modified(aSource);
          appendCategory("modified", aModified);
          if (theBuilder.IsDeleted(aSource))
          {
            aHistory["deleted"].push_back(aFrom);
          }
        }
      }
    }
    return aHistory;
  }

  static json buildRecordedHistory(const occ::handle<BRepTools_History>& theRecordedHistory,
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
          const json aFrom{{"input", anInputIndex}, {"type", aType.second}, {"index", aShapeIndex - 1}};
          const int aRetainedIndex = anOutputMap.FindIndex(aSource);
          if (aRetainedIndex > 0)
          {
            aHistory["retained"].push_back({{"from", aFrom},
                                             {"to", {{"type", aType.second}, {"index", aRetainedIndex - 1}}}});
          }

          for (const auto& aCategory : std::vector<std::pair<const char*, const NCollection_List<TopoDS_Shape>*>>{
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
                aTargetValues.push_back({{"type", aType.second}, {"index", anOutputIndex - 1}});
            }
            if (!aTargetValues.empty())
              aHistory[aCategory.first].push_back({{"from", aFrom}, {"to", aTargetValues}});
          }
          if (theRecordedHistory->IsRemoved(aSource))
            aHistory["deleted"].push_back(aFrom);
        }
      }
    }
    return aHistory;
  }

  static json buildRetainedHistory(const TopoDS_Shape& theInput, const TopoDS_Shape& theOutput)
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

  ShapeArena myArena;
  BufferStore myBuffers;
};

Kernel THE_KERNEL;
std::string THE_RESPONSE;
} // namespace occt_worker

extern "C" {
#ifdef __EMSCRIPTEN__
std::uint32_t _emscripten_lookup_name(const char*)
{
  return 0;
}
#endif

EMSCRIPTEN_KEEPALIVE void* k_alloc(const std::uint32_t theLength)
{
  return std::malloc(theLength);
}

EMSCRIPTEN_KEEPALIVE void k_free(void* thePointer)
{
  std::free(thePointer);
}

EMSCRIPTEN_KEEPALIVE std::uint32_t k_handle(const std::uint8_t* theRequest,
                                            const std::uint32_t theLength)
{
  using namespace occt_worker;
  std::int64_t anId = -1;
  std::string anOperation;
  try
  {
    const json aRequest = json::parse(theRequest, theRequest + theLength);
    if (!aRequest.is_object() || !aRequest.contains("id") || !aRequest.at("id").is_number_integer()
        || !aRequest.contains("op") || !aRequest.at("op").is_string()
        || !aRequest.contains("args") || !aRequest.at("args").is_object())
    {
      throw KernelFailure(ErrorCode::ProtocolError, "Request must contain integer id, string op, and object args");
    }
    anId = aRequest.at("id").get<std::int64_t>();
    anOperation = aRequest.at("op").get<std::string>();
    THE_RESPONSE = json{{"id", anId}, {"ok", true},
                        {"result", THE_KERNEL.dispatch(anOperation,
                                                       aRequest.at("args"))}}.dump();
  }
  catch (const KernelFailure& aFailure)
  {
    json anError{{"code", errorName(aFailure.code)}, {"message", aFailure.what()}};
    if (!anOperation.empty())
    {
      anError["details"] = {{"operation", anOperation}};
      if (const char* aFormat = exchangeFormatForOperation(anOperation))
        anError["details"]["format"] = aFormat;
    }
    THE_RESPONSE = json{{"id", anId}, {"ok", false},
                        {"error", anError}}.dump();
  }
  catch (const std::bad_alloc&)
  {
    try
    {
      json anError{{"code", "OutOfMemory"}, {"message", "Linear memory allocation failed"}};
      if (!anOperation.empty())
      {
        anError["details"] = {{"operation", anOperation}};
        if (const char* aFormat = exchangeFormatForOperation(anOperation))
          anError["details"]["format"] = aFormat;
      }
      THE_RESPONSE = json{{"id", anId}, {"ok", false},
                          {"error", anError}}.dump();
    }
    catch (...)
    {
      return 0;
    }
  }
  catch (const Standard_Failure& aFailure)
  {
    json anError{{"code", "KernelError"}, {"message", aFailure.GetMessageString()}};
    if (!anOperation.empty())
    {
      anError["details"] = {{"operation", anOperation}};
      if (const char* aFormat = exchangeFormatForOperation(anOperation))
        anError["details"]["format"] = aFormat;
    }
    THE_RESPONSE = json{{"id", anId}, {"ok", false},
                        {"error", anError}}.dump();
  }
  catch (const std::exception& aFailure)
  {
    json anError{{"code", "ProtocolError"}, {"message", aFailure.what()}};
    if (!anOperation.empty())
    {
      anError["details"] = {{"operation", anOperation}};
      if (const char* aFormat = exchangeFormatForOperation(anOperation))
        anError["details"]["format"] = aFormat;
    }
    THE_RESPONSE = json{{"id", anId}, {"ok", false},
                        {"error", anError}}.dump();
  }
  catch (...)
  {
    THE_RESPONSE = R"({"id":-1,"ok":false,"error":{"code":"KernelError","message":"Unknown kernel failure"}})";
  }
  return static_cast<std::uint32_t>(THE_RESPONSE.size());
}

EMSCRIPTEN_KEEPALIVE const std::uint8_t* k_response_ptr()
{
  return reinterpret_cast<const std::uint8_t*>(occt_worker::THE_RESPONSE.data());
}

EMSCRIPTEN_KEEPALIVE std::uint8_t* k_buffer_ptr(const std::uint32_t theBufferId)
{
  try
  {
    return occt_worker::THE_KERNEL.buffers().get(theBufferId).data();
  }
  catch (...)
  {
    return nullptr;
  }
}

EMSCRIPTEN_KEEPALIVE std::uint32_t k_buffer_len(const std::uint32_t theBufferId)
{
  try
  {
    return static_cast<std::uint32_t>(occt_worker::THE_KERNEL.buffers().get(theBufferId).size());
  }
  catch (...)
  {
    return 0;
  }
}
}
