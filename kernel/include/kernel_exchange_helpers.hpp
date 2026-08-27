#pragma once
#include "kernel_support.hpp"

namespace occt_worker {

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

      bool hasSameState(const ObjPrimitive& theOther) const;
      json toJson() const;
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

      json toJson() const;
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

  struct DocumentNodeData
  {
    json roots = json::array();
    json nodes = json::array();
    std::vector<TDF_Label> occurrenceLabels;
    std::vector<TDF_Label> definitionLabels;
  };

  DocumentNodeData extractDocumentNodes(
    KernelOperationContext& theContext,
    bool theRestoreIgesHierarchy,
    const json& theIgesMetadata,
    std::uint32_t theScope,
    const occ::handle<XCAFDoc_ShapeTool>& theShapeTool,
    const occ::handle<XCAFDoc_ColorTool>& theColorTool,
    const occ::handle<XCAFDoc_LayerTool>& theLayerTool,
    const occ::handle<XCAFDoc_VisMaterialTool>& theVisMaterialTool,
    const NCollection_Sequence<TDF_Label>& theRootLabels);

  void validateDocumentTree(KernelOperationContext& theContext,
                            const json& theNodes,
                            const json& theRoots);

  struct DocumentAnnotationData
  {
    json gdt = json::array();
    json datums = json::array();
    json geometricTolerances = json::array();
    json views = json::array();
    json shuo = json::array();
  };

  DocumentAnnotationData extractDocumentAnnotations(
    KernelOperationContext& theContext,
    bool theIsStepDocument,
    const std::vector<std::string>& theStepGdtSemanticNames,
    std::uint32_t theScope,
    const occ::handle<XCAFDoc_ViewTool>& theViewTool,
    const occ::handle<XCAFDoc_ClippingPlaneTool>& theClippingPlaneTool,
    const occ::handle<XCAFDoc_DimTolTool>& theDimTolTool,
    const std::vector<TDF_Label>& theOccurrenceLabels,
    const std::vector<TDF_Label>& theDefinitionLabels);

  void applyDocumentAnnotations(
    KernelOperationContext& theContext,
    bool theIsStepDocument,
    const json& theArgs,
    const json& theNodes,
    const std::vector<TDF_Label>& theLabels,
    std::size_t theNodeCount,
    const occ::handle<XCAFDoc_DimTolTool>& theDimTolTool);

  std::array<float, 4> parseNormalizedColor(const json& theValue,
                                                   const char* theError);

  bool documentDimensionTypeFromName(
    const std::string& theName, XCAFDimTolObjects_DimensionType& theResult);
  const char* documentDimensionTypeName(XCAFDimTolObjects_DimensionType theType);
  bool documentGeomToleranceTypeFromName(
    const std::string& theName, XCAFDimTolObjects_GeomToleranceType& theResult);
  const char* documentGeomToleranceTypeName(
    XCAFDimTolObjects_GeomToleranceType theType);
  bool documentGeomToleranceValueTypeFromName(
    const std::string& theName, XCAFDimTolObjects_GeomToleranceTypeValue& theResult);
  const char* documentGeomToleranceValueTypeName(
    XCAFDimTolObjects_GeomToleranceTypeValue theType);
  bool documentGeomToleranceMaterialRequirementFromName(
    const std::string& theName,
    XCAFDimTolObjects_GeomToleranceMatReqModif& theResult);
  const char* documentGeomToleranceMaterialRequirementName(
    XCAFDimTolObjects_GeomToleranceMatReqModif theType);
  bool documentGeomToleranceZoneModifierFromName(
    const std::string& theName,
    XCAFDimTolObjects_GeomToleranceZoneModif& theResult);
  const char* documentGeomToleranceZoneModifierName(
    XCAFDimTolObjects_GeomToleranceZoneModif theType);
  bool documentGeomToleranceModifierFromName(
    const std::string& theName,
    XCAFDimTolObjects_GeomToleranceModif& theResult);
  const char* documentGeomToleranceModifierName(
    XCAFDimTolObjects_GeomToleranceModif theType);

  TriangleMesh parseIndexedTriangleMesh(BufferStore& theBuffers,
                                        const json& theArgs,
                                        const char* theFormat);

  TriangleMesh buildTriangleMesh(const TopoDS_Shape& theInput,
                                        const double theLinearDeflection,
                                        const double theAngularDeflection,
                                        const bool theRelative);

  TriangleMesh triangleMesh(const TopoDS_Shape& theShape);

  TopoDS_Shape makeTriangleShape(const TriangleMesh& theMesh);

  TriangleMesh vrmlSceneMesh(const VrmlData_Scene& theScene);

  std::string writeVRMLIndexedMesh(const TriangleMesh& theMesh,
                                          const gp_Trsf& theTransform);

  std::size_t parseOBJComponent(const std::string& theText,
                                std::size_t theCount,
                                const char* theLabel);

  struct ObjCorner
  {
    std::size_t position = 0;
    std::size_t uv = std::numeric_limits<std::size_t>::max();
    std::size_t normal = std::numeric_limits<std::size_t>::max();
  };

  ObjCorner parseOBJCorner(const std::string& theToken,
                           std::size_t thePositionCount,
                           std::size_t theUVCount,
                           std::size_t theNormalCount);
  std::string objLineValue(std::istringstream& theStream);
  std::string objMapReference(std::istringstream& theStream);

  std::vector<json> parseMTL(const std::string& theData);

  TriangleMesh parseOBJ(
    const std::string& theData,
    const std::unordered_map<std::string, std::string>& theResources = {});

  TriangleMesh parsePLY(const std::string& theData);

  std::string writeMTL(const std::vector<json>& theMaterials);

  std::string writeOBJ(const TriangleMesh& theMesh);

  std::string writePLY(const TriangleMesh& theMesh, const std::string& theEncoding);

  int base64Value(const char theCharacter);

  std::string encodeBase64(const std::vector<std::uint8_t>& theData);

  std::vector<std::uint8_t> decodeBase64(const std::string& theText);

  std::vector<std::uint8_t> decodeGltfDataUri(const std::string& theUri);

  void appendU32LE(std::vector<std::uint8_t>& theData, const std::uint32_t theValue);

  std::uint32_t readU32LE(const std::string& theData, const std::size_t theOffset);

  void appendFloat32LE(std::vector<std::uint8_t>& theData, const float theValue);

  std::vector<std::uint8_t> writeGLTF(const TriangleMesh& theMesh, const bool theBinary);

  struct GltfDocumentExport
  {
    std::vector<std::uint8_t> data;
    std::vector<std::pair<std::string, std::vector<std::uint8_t>>> resources;
  };

  GltfDocumentExport writeGLTFDocument(
    const json& theSourceDocument,
    const std::vector<std::pair<std::string, std::vector<std::uint8_t>>>& theBuffers,
    const std::vector<std::pair<std::string, std::vector<std::uint8_t>>>& theResources,
    const bool theBinary);

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

  std::size_t gltfComponentSize(const int theComponentType);

  GltfAccessor gltfAccessor(const json& theDocument,
                                   const std::vector<std::vector<std::uint8_t>>& theBuffers,
                                   const std::size_t theAccessorIndex,
                                   const std::string& theExpectedType);

  double gltfAccessorValue(const GltfAccessor& theAccessor,
                                  const std::size_t theElement,
                                  const std::size_t theComponent);

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

  double gltfTransformDeterminant(const GltfMatrix& theMatrix);

  GltfMatrix multiplyGltfMatrices(const GltfMatrix& theLeft, const GltfMatrix& theRight);

  GltfMatrix gltfNodeMatrix(const json& theNode,
                                   const GltfEvaluatedNode* theEvaluated = nullptr);

  gp_Pnt transformGltfPoint(const GltfMatrix& theMatrix,
                                   const double theX,
                                   const double theY,
                                   const double theZ);

  gp_Dir transformGltfNormal(const GltfMatrix& theMatrix,
                                    const double theX,
                                    const double theY,
                                    const double theZ);

  std::vector<GltfEvaluatedNode> evaluateGLTFAnimation(
    const json& theDocument,
    const std::vector<std::vector<std::uint8_t>>& theBuffers,
    const std::size_t* theAnimationIndex,
    const double* theAnimationTime);

  TriangleMesh parseGLTF(
    const std::string& theData,
    const std::unordered_map<std::string, std::string>& theResources = {},
    const bool theAllowNoTriangles = false,
    const std::vector<double>* theMorphWeights = nullptr,
    const std::size_t* theAnimationIndex = nullptr,
    const double* theAnimationTime = nullptr);

  json dispatchExportOBJDocument(KernelOperationContext& theContext, const json& theArgs);
  json dispatchExportGLTFDocument(KernelOperationContext& theContext, const json& theArgs);
  json writeXCAFDocument(KernelOperationContext& theContext,
                         const json& theArgs,
                         const occ::handle<TDocStd_Document>& theDocument,
                         const std::string& theFormat);
  json writeIGESDocument(KernelOperationContext& theContext,
                         const json& theArgs,
                         const occ::handle<TDocStd_Document>& theDocument);
  json writeSTEPDocument(KernelOperationContext& theContext,
                         const json& theArgs,
                         const occ::handle<TDocStd_Document>& theDocument);

} // namespace occt_worker
