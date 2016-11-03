#include "AbcLoader.h"
#include "../Objects/AlembicCache.h"

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

  if (iProp.getMetaData().get("schema") == "AbcGeom_PolyMesh_v1") {
    IPolyMeshSchema meshSchema = IPolyMeshSchema(iProp);
    auto positions = meshSchema.getPositionsProperty();
	P3fArraySamplePtr arrayPtr;
	positions.get(arrayPtr);
	auto dim = arrayPtr->getDimensions();
	std::cout << "Vertex Count:" << dim.numPoints() << std::endl;
	for (int i = 0; i < dim.numPoints(); i++) {
		std::cout << arrayPtr->get()[i] << "\n";
	}

  } else {
    visitProperties(iProp, ioIndent);
  }

  ioIndent = oldIndent;
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
void visitObject(IObject iObj, std::string iIndent) {
  // Object has a name, a full name, some meta data,
  // and then it has a compound property full of properties.
  std::string path = iObj.getFullName();

  if (path != "/") {
    std::cout << "Object "
              << "name=" << path << std::endl;
  }

  // Get the properties.
  ICompoundProperty props = iObj.getProperties();
  visitProperties(props, iIndent);

  // now the child objects
  for (size_t i = 0; i < iObj.getNumChildren(); i++) {
    visitObject(IObject(iObj, iObj.getChildHeader(i).getName()), iIndent);
  }
}

bool LoadAbc(std::string abcFile, BaseObject **result) {
  *result = nullptr;

  Alembic::AbcCoreFactory::IFactory factory;
  factory.setPolicy(Alembic::AbcGeom::ErrorHandler::kQuietNoopPolicy);
  Alembic::AbcGeom::IArchive archive = factory.getArchive(abcFile);

  if (archive) {
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

    visitObject(archive.getTop(), "");
  }

  return false;
}