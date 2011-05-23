#ifndef VPVL_PMDPARSER_H_
#define VPVL_PMDPARSER_H_

#include "vpvl/IModelParser.h"
#include "vpvl/internal/PMDModel.h"

namespace vpvl
{

struct PMDParserResult
{
    const char *basePtr;
    const char *namePtr;
    const char *commentPtr;
    const char *verticesPtr;
    size_t verticesCount;
    const char *indicesPtr;
    size_t indicesCount;
    const char *materialsPtr;
    size_t materialsCount;
    const char *bonesPtr;
    size_t bonesCount;
    const char *IKsPtr;
    size_t IKsCount;
    const char *facesPtr;
    size_t facesCount;
    const char *faceDisplayNamesPtr;
    size_t faceDisplayNamesCount;
    const char *boneFrameNamesPtr;
    size_t boneFrameNamesCount;
    const char *boneDisplayNamesPtr;
    size_t boneDisplayNamesCount;
    const char *englishNamePtr;
    const char *englishCommentPtr;
    const char *toonTextureNamesPtr;
    const char *rigidBodiesPtr;
    size_t rigidBodiesCount;
    const char *constraintsPtr;
    size_t constranitsCount;
};

class PMDParser
{
public:
    PMDParser(const char *data, size_t size);
    ~PMDParser();

    bool preparse();
    PMDModel *parse();

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

    const PMDParserResult &result() const {
        return m_result;
    }
    const char *data() const {
        return m_data;
    }
    size_t size() const {
        return m_size;
    }

private:
    PMDParserResult m_result;
    char *m_data;
    size_t m_size;
};

}

#endif
