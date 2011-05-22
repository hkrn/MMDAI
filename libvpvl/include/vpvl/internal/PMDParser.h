#ifndef VPVL_PMDPARSER_H_
#define VPVL_PMDPARSER_H_

#include "vpvl/IModelParser.h"
#include "vpvl/internal/PMDModel.h"

namespace vpvl
{

struct ParserResult
{
    char *basePtr;
    char *namePtr;
    char *commentPtr;
    char *verticesPtr;
    size_t verticesCount;
    char *indicesPtr;
    size_t indicesCount;
    char *materialsPtr;
    size_t materialsCount;
    char *bonesPtr;
    size_t bonesCount;
    char *IKsPtr;
    size_t IKsCount;
    char *facesPtr;
    size_t facesCount;
    char *faceDisplayNamesPtr;
    size_t faceDisplayNamesCount;
    char *boneFrameNamesPtr;
    size_t boneFrameNamesCount;
    char *boneDisplayNamesPtr;
    size_t boneDisplayNamesCount;
    char *englishDisplayNamesPtr;
    char *toonTextureNamesPtr;
    char *rigidBodiesPtr;
    size_t rigidBodiesCount;
    char *constraintsPtr;
    size_t constranitsCount;
};

class PMDParser : public IModelParser
{
public:
    PMDParser(const char *data, size_t size);
    virtual ~PMDParser();

    virtual bool preparse();
    virtual IModel *parse();

    void parseHeader(PMDModel *model);
    void parseVertices(PMDModel *model);
    void parseIndices(PMDModel *model);
    void parseMatrials(PMDModel *model);
    void parseBones(PMDModel *model);
    void parseIKs(PMDModel *model);
    void parseFaces(PMDModel *model);
    void parseFaceDisplayNames(PMDModel *model);
    void parseBoneDisplayNames(PMDModel *model);
    void parseEnglishDisplayNames(PMDModel *model);
    void parseToonTextureNames(PMDModel *model);
    void parseRigidBodies(PMDModel *model);
    void parseConstraints(PMDModel *model);

private:
    ParserResult m_result;
    char *m_data;
    size_t m_size;
    size_t m_rest;
};

}

#endif
