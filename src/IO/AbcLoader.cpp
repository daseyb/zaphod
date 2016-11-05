#include "AbcLoader.h"

#include "SceneLoader.h"

#include "../Objects/Mesh.h"
#include "../Objects/Timeline.h"

#include <Alembic/AbcGeom/All.h>
#include <Alembic/AbcCoreAbstract/All.h>
#include <Alembic/AbcCoreFactory/All.h>
#include <Alembic/Util/All.h>
#include <Alembic/Abc/TypedPropertyTraits.h>

using namespace ::Alembic::AbcGeom;

static const std::string g_sep(";");

//-*****************************************************************************
// FORWARD
void visitProperties(ICompoundProperty, std::string &);

//-*****************************************************************************
template <class PROP>
void visitSimpleArrayProperty(PROP iProp, const std::string &iIndent) {
  std::string ptype = "ArrayProperty ";
  size_t asize = 0;

  AbcA::ArraySamplePtr samp;
  index_t maxSamples = iProp.getNumSamples();
  for (index_t i = 0; i < maxSamples; ++i) {
    iProp.get(samp, ISampleSelector(i));
    asize = samp->size();
  };

  std::string mdstring = "interpretation=";
  mdstring += iProp.getMetaData().get("interpretation");

  std::stringstream dtype;
  dtype << "datatype=";
  dtype << iProp.getDataType();

  std::stringstream asizestr;
  asizestr << ";arraysize=";
  asizestr << asize;

  mdstring += g_sep;

  mdstring += dtype.str();

  mdstring += asizestr.str();

  std::cout << iIndent << "  " << ptype << "name=" << iProp.getName() << g_sep
            << mdstring << g_sep << "numsamps=" << iProp.getNumSamples()
            << std::endl;
}

//-*****************************************************************************
template <class PROP>
void visitSimpleScalarProperty(PROP iProp, const std::string &iIndent) {
  std::string ptype = "ScalarProperty ";
  size_t asize = 0;

  const AbcA::DataType &dt = iProp.getDataType();
  const Alembic::Util::uint8_t extent = dt.getExtent();
  Alembic::Util::Dimensions dims(extent);
  AbcA::ArraySamplePtr samp = AbcA::AllocateArraySample(dt, dims);
  index_t maxSamples = iProp.getNumSamples();
  for (index_t i = 0; i < maxSamples; ++i) {
    iProp.get(const_cast<void *>(samp->getData()), ISampleSelector(i));
    asize = samp->size();
  };

  std::string mdstring = "interpretation=";
  mdstring += iProp.getMetaData().get("interpretation");

  std::stringstream dtype;
  dtype << "datatype=";
  dtype << dt;

  std::stringstream asizestr;
  asizestr << ";arraysize=";
  asizestr << asize;

  mdstring += g_sep;

  mdstring += dtype.str();

  mdstring += asizestr.str();

  std::cout << iIndent << "  " << ptype << "name=" << iProp.getName() << g_sep
            << mdstring << g_sep << "numsamps=" << iProp.getNumSamples()
            << std::endl;
}

//-*****************************************************************************
void visitCompoundProperty(ICompoundProperty iProp, std::string &ioIndent) {
  std::string oldIndent = ioIndent;
  ioIndent += "  ";

  std::string interp = "schema=";
  interp += iProp.getMetaData().get("schema");

  std::cout << ioIndent << "CompoundProperty "
            << "name=" << iProp.getName() << g_sep << interp << std::endl;

  if (false && iProp.getMetaData().get("schema") == "AbcGeom_PolyMesh_v1") {
    IPolyMeshSchema meshSchema = IPolyMeshSchema(iProp);
  }

  visitProperties(iProp, ioIndent);

  ioIndent = oldIndent;
}

void readMeshData(IPolyMeshSchema meshSchema, std::vector<Triangle> &tris,
                  std::vector<Vector3> &verts, std::vector<Vector3> &normals,
                  std::vector<Vector2> &uvs) {

  auto meshSample = meshSchema.getValue();

  {
    auto positions = meshSample.getPositions();

    verts.reserve(positions->size());
    for (size_t i = 0; i < positions->size(); i++) {
      auto vert = positions->get()[i];
      verts.push_back({vert.x, vert.y, vert.z});
    }
  }

  {
    auto faceCounts = meshSample.getFaceCounts();
    auto indices = meshSample.getFaceIndices();

    tris.reserve(indices->size() / 3);
    for (size_t i = 0; i < indices->size(); i += 3) {
      Triangle tri;
      tri.m_Indices[0] = indices->get()[i + 0];
      tri.m_Indices[1] = indices->get()[i + 1];
      tri.m_Indices[2] = indices->get()[i + 2];
      tris.push_back(tri);
    }
  }

  {
    auto normalsParam = meshSchema.getNormalsParam();

    auto normalsSample = normalsParam.getIndexedValue();
    auto normalsPtr = normalsSample.getVals();

    auto dim = normalsPtr->getDimensions();
    normals.reserve(normalsPtr->size());
    for (size_t i = 0; i < normalsPtr->size(); i++) {
      auto norm = normalsPtr->get()[i];
      normals.push_back({norm.x, norm.y, norm.z});
    }
  }

  {
    auto uvsParam = meshSchema.getUVsParam();

    if (uvsParam.isIndexed()) {
      auto uvsSample = uvsParam.getIndexedValue();
      auto uvsPtr = uvsSample.getVals();

      uvs.reserve(uvsPtr->size());
      for (size_t i = 0; i < uvsPtr->size(); i++) {
        auto uv = uvsPtr->get()[i];
        uvs.push_back({uv.x, uv.y});
      }
    }
  }
}

void readTransformData(XformSample transformSample, Vector3 &pos,
                       Quaternion &rotation, Vector3 &scale) {

  auto xPos = transformSample.getTranslation();
  pos.x = xPos.x;
  pos.y = xPos.y;
  pos.z = xPos.z;

  std::cout << transformSample.getXRotation() << ","
            << transformSample.getYRotation() << ","
            << transformSample.getZRotation() << std::endl;

  rotation =
      Quaternion::CreateFromAxisAngle(
          Vector3(1, 0, 0), DegreesToRadians(transformSample.getXRotation())) *
      Quaternion::CreateFromAxisAngle(
          Vector3(0, 1, 0), DegreesToRadians(transformSample.getYRotation())) *
      Quaternion::CreateFromAxisAngle(
          Vector3(0, 0, 1), DegreesToRadians(transformSample.getZRotation()));

  auto xScale = transformSample.getScale();
  scale.x = xScale.x;
  scale.y = xScale.y;
  scale.z = xScale.z;
}

//-*****************************************************************************
void visitProperties(ICompoundProperty iParent, std::string &ioIndent) {
  std::string oldIndent = ioIndent;
  for (size_t i = 0; i < iParent.getNumProperties(); i++) {
    PropertyHeader header = iParent.getPropertyHeader(i);

    if (header.isCompound()) {
      visitCompoundProperty(ICompoundProperty(iParent, header.getName()),
                            ioIndent);
    } else if (header.isScalar()) {
      visitSimpleScalarProperty(IScalarProperty(iParent, header.getName()),
                                ioIndent);
    } else {
      assert(header.isArray());
      visitSimpleArrayProperty(IArrayProperty(iParent, header.getName()),
                               ioIndent);
    }
  }

  ioIndent = oldIndent;
}

//-*****************************************************************************
void visitObject(IObject iObj, std::string iIndent, BaseObject *parent,
                 std::vector<BaseObject *> &result) {
  // Object has a name, a full name, some meta data,
  // and then it has a compound property full of properties.
  std::string path = iObj.getFullName();

  std::string oldIndent = iIndent;
  iIndent += "  ";

  // Get the properties.
  ICompoundProperty props = iObj.getProperties();

  IXformSchema transform;
  IPolyMeshSchema mesh;
  bool hasTransform = false;
  bool hasGeom = false;
  for (size_t i = 0; i < props.getNumProperties(); i++) {
    PropertyHeader header = props.getPropertyHeader(i);
    if (header.getName() == ".xform") {
      hasTransform = true;
      transform = IXformSchema(ICompoundProperty(props, header.getName()));
    } else if (header.getName() == ".geom") {
      hasGeom = true;
      mesh = IPolyMeshSchema(ICompoundProperty(props, header.getName()));
    }
  }

  BaseObject *newParent = parent;

  if (hasGeom) {
    std::vector<Triangle> tris;
    std::vector<Vector3> verts;
    std::vector<Vector3> normals;
    std::vector<Vector2> uvs;
    readMeshData(mesh, tris, verts, normals, uvs);
    newParent =
        new Mesh(Vector3(0, 0, 0), tris, verts, normals, uvs, false, parent);
    std::cout << "Loading mesh" << std::endl;
  }

  if (hasTransform) {
    if (newParent == parent) {
      newParent = new BaseObject(parent);
    }

    Timeline<Vector3> posTimeline;
    Timeline<Quaternion> rotTimeline;
    Timeline<Vector3> scaleTimeline;

    Vector3 pos;
    Quaternion rot;
    Vector3 scale;

    for (int i = 0; i < transform.getNumSamples(); i++) {
      auto transformSample = transform.getValue(i);

      readTransformData(transformSample, pos, rot, scale);

      posTimeline.keyframes.push_back({i, pos});
      rotTimeline.keyframes.push_back({i, rot});
      scaleTimeline.keyframes.push_back({i, scale});
    }

    newParent->SetPositionTimeline(posTimeline);
    newParent->SetRotationTimeline(rotTimeline);
    newParent->SetScaleTimeline(scaleTimeline);
  }

  if (newParent != parent) {
    result.push_back(newParent);
  }

  std::cout << iIndent << "Object "
            << "name=\"" << path << "\";childCount=" << iObj.getNumChildren()
            << ";hasTransform=" << hasTransform << ";hasGeom=" << hasGeom
            << std::endl;

  // visitProperties(props, iIndent);

  // now the child objects
  for (size_t i = 0; i < iObj.getNumChildren(); i++) {
    visitObject(IObject(iObj, iObj.getChildHeader(i).getName()), iIndent,
                newParent, result);
  }
  iIndent = oldIndent;
}

bool LoadAbc(std::string abcFile, BaseObject **root,
             std::vector<ObjectData *> &resultData) {
  *root = nullptr;

  Alembic::AbcCoreFactory::IFactory factory;
  factory.setPolicy(Alembic::AbcGeom::ErrorHandler::kQuietNoopPolicy);
  Alembic::AbcGeom::IArchive archive = factory.getArchive(abcFile);

  if (!archive)
    return false;

  std::cout << "AbcEcho for " << Alembic::AbcCoreAbstract::GetLibraryVersion()
            << std::endl;
  ;

  std::string appName;
  std::string libraryVersionString;
  Alembic::Util::uint32_t libraryVersion;
  std::string whenWritten;
  std::string userDescription;
  GetArchiveInfo(archive, appName, libraryVersionString, libraryVersion,
                 whenWritten, userDescription);

  if (appName != "") {
    std::cout << "  file written by: " << appName << std::endl;
    std::cout << "  using Alembic : " << libraryVersionString << std::endl;
    std::cout << "  written on : " << whenWritten << std::endl;
    std::cout << "  user description : " << userDescription << std::endl;
    std::cout << std::endl;
  } else {
    std::cout << abcFile << std::endl;
    std::cout << "  (file doesn't have any ArchiveInfo)" << std::endl;
    std::cout << std::endl;
  }

  std::vector<BaseObject *> objs;

  visitObject(archive.getTop(), "", nullptr, objs);

  if (objs.size() == 0)
    return false;

  *root = objs[0];

  for (auto obj : objs) {
    resultData.push_back(new ObjectData{"", obj});
  }

  std::cout << "Loaded object count: " << objs.size() << std::endl;

  return true;
}