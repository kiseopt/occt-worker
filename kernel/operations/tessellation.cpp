#include "kernel_tessellation_operations.hpp"
#include "kernel_history_helpers.hpp"
#include "kernel_operation_context.hpp"
#include "kernel_protocol_helpers.hpp"
#include "side_registration.hpp"

namespace occt_worker {

json h_triangulationData(KernelOperationContext& theContext, const json& theArgs)
{
  const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
  return tessellation(theContext.buffers(), aShape, theArgs.value("includeUV", false));
}

json h_tessellate(KernelOperationContext& theContext, const json& theArgs)
{
  TopoDS_Shape aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
  const double aLinear = theArgs.value("linearDeflection", 0.1);
  const double anAngular = theArgs.value("angularDeflection", 0.5);
  const bool isRelative = theArgs.value("relative", false);
  if (aLinear <= 0.0 || anAngular <= 0.0)
    throw KernelFailure(ErrorCode::InvalidArgs, "Tessellation deflections must be positive");
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
    throw KernelFailure(ErrorCode::Cancelled, "OCCT tessellation cancelled");
  if (!aMesher.IsDone()) throw KernelFailure(ErrorCode::TessellationFailed, "OCCT tessellation failed");
  return tessellation(theContext.buffers(), aShape, theArgs.value("includeUV", false));
}

json h_tessellateEdges(KernelOperationContext& theContext, const json& theArgs)
{
  const double aLinear = theArgs.value("linearDeflection", 0.1);
  const double anAngular = theArgs.value("angularDeflection", 0.5);
  if (aLinear <= 0.0 || anAngular <= 0.0)
    throw KernelFailure(ErrorCode::InvalidArgs, "Edge deflections must be positive");
  const TopoDS_Shape& aShape = theContext.arena().get(requiredU32(theArgs, "shape"));
  TopTools_IndexedMapOfShape anEdges;
  TopExp::MapShapes(aShape, TopAbs_EDGE, anEdges);
  occ::handle<Message_ProgressIndicator> aProgress = new HostProgressIndicator();
  Message_ProgressScope aProgressScope(aProgress->Start(), "tessellateEdges",
                                        static_cast<double>(anEdges.Extent()));
  std::vector<float> aPositions;
  std::vector<std::uint32_t> anEdgeGroups;
  for (int anEdgeIndex = 1; anEdgeIndex <= anEdges.Extent(); ++anEdgeIndex)
  {
    if (!aProgressScope.More()) throw KernelFailure(ErrorCode::Cancelled, "OCCT edge tessellation cancelled");
    BRepAdaptor_Curve anAdaptor(TopoDS::Edge(anEdges(anEdgeIndex)));
    GCPnts_TangentialDeflection aPoints(anAdaptor, anAngular, aLinear);
    const std::uint32_t aStart = static_cast<std::uint32_t>(aPositions.size() / 3);
    for (int aPointIndex = 1; aPointIndex <= aPoints.NbPoints(); ++aPointIndex)
    {
      const gp_Pnt aPoint = aPoints.Value(aPointIndex);
      aPositions.insert(aPositions.end(), {static_cast<float>(aPoint.X()),
                                           static_cast<float>(aPoint.Y()),
                                           static_cast<float>(aPoint.Z())});
    }
    anEdgeGroups.insert(anEdgeGroups.end(), {static_cast<std::uint32_t>(anEdgeIndex - 1), aStart,
                                              static_cast<std::uint32_t>(aPoints.NbPoints())});
    aProgressScope.Next();
  }
  const std::uint32_t aPositionsId = theContext.buffers().copy(aPositions);
  const std::uint32_t anGroupsId = theContext.buffers().copy(anEdgeGroups);
  return {{"positions", bufferDescriptor(aPositionsId, aPositions.size() * sizeof(float), "f32x3")},
          {"edgeGroups", bufferDescriptor(anGroupsId, anEdgeGroups.size() * sizeof(std::uint32_t), "u32x3")}};
}

void register_tessellation_operations()
{
  auto& aRegistry = OperationRegistry::instance();
#define OCCT_REGISTER_HANDLER(theOperation) aRegistry.add(#theOperation, &h_##theOperation);
  OCCT_TESSELLATION_OPERATION_TABLE(OCCT_REGISTER_HANDLER)
#undef OCCT_REGISTER_HANDLER
}

OCCT_DEFINE_SIDE_PLUGIN(tessellation, kTessellationOperationNames, register_tessellation_operations)

} // namespace occt_worker
