#include "kernel_exchange_helpers.hpp"

namespace occt_worker {

std::string writeMTL(const std::vector<json>& theMaterials)
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

std::string writeOBJ(const TriangleMesh& theMesh)
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

std::string writePLY(const TriangleMesh& theMesh, const std::string& theEncoding)
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

} // namespace occt_worker
