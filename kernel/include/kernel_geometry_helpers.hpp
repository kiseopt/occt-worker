#pragma once
#include "kernel_support.hpp"

namespace occt_worker {

const char* continuityName(GeomAbs_Shape theContinuity);
TopAbs_ShapeEnum shapeTypeFromName(const std::string& theName);
const char* shapeTypeName(TopAbs_ShapeEnum theType);
const char* checkStatusName(BRepCheck_Status theStatus);

occ::handle<Geom_Curve> curveForEdge(const TopoDS_Shape& theShape,
                                     double& theFirst,
                                     double& theLast);
occ::handle<Geom_Surface> surfaceForFace(const TopoDS_Shape& theShape,
                                         double& theUFirst,
                                         double& theULast,
                                         double& theVFirst,
                                         double& theVLast);
TopoDS_Face makeRectangularFace(const occ::handle<Geom_Surface>& theSurface,
                                double theUFirst,
                                double theULast,
                                double theVFirst,
                                double theVLast,
                                double theTolerance,
                                TopAbs_Orientation theOrientation);
TopoDS_Face makeSurfaceFacePreservingWires(
  const TopoDS_Face& theSourceFace,
  const occ::handle<Geom_Surface>& theSurface,
  double theTolerance,
  TopAbs_Orientation theOrientation,
  const std::array<double, 4>* theOuterBounds = nullptr);

} // namespace occt_worker
