/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2012  hkrn                                    */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAI project team nor the names of     */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#ifndef VPVL2_PMX_MODEL_H_
#define VPVL2_PMX_MODEL_H_

#include "vpvl2/Common.h"

namespace vpvl2
{
namespace pmx
{

/**
 * @file
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * Model class represents a morph of a Polygon Model Extended object.
 */

class Bone;
class Joint;
class Material;
class Morph;
class RigidBody;
class Vertex;

class VPVL2_API Model
{
public:
    class UserData {
    public:
        UserData() {}
        virtual ~UserData() {}
    };

    /**
      * Type of parsing errors.
      */
    enum Error
    {
        kNoError,
        kInvalidHeaderError,
        kInvalidSignatureError,
        kInvalidVersionError,
        kInvalidFlagSizeError,
        kInvalidNameSizeError,
        kInvalidEnglishNameSizeError,
        kInvalidCommentSizeError,
        kInvalidEnglishCommentSizeError,
        kInvalidVerticesError,
        kInvalidIndicesError,
        kInvalidTextureSizeError,
        kInvalidTextureError,
        kInvalidMaterialsError,
        kInvalidBonesError,
        kInvalidMorphsError,
        kInvalidDisplayNameSizeError,
        kInvalidDisplayNameError,
        kInvalidDisplayEnglishNameError,
        kInvalidRigidBodiesError,
        kInvalidJointsError,
        kMaxErrors
    };
    enum Encoding {
        kUTF8,
        kUTF16
    };

    struct DataInfo
    {
        uint8_t *basePtr;
        uint8_t *namePtr;
        bool isUTF8;
        size_t additionalUVSize;
        size_t vertexIndexSize;
        size_t textureIndexSize;
        size_t materialIndexSize;
        size_t boneIndexSize;
        size_t morphIndexSize;
        size_t rigidBodyIndexSize;
        size_t nameSize;
        uint8_t *englishNamePtr;
        size_t englishNameSize;
        uint8_t *commentPtr;
        size_t commentSize;
        uint8_t *englishCommentPtr;
        size_t englishCommentSize;
        uint8_t *verticesPtr;
        size_t verticesCount;
        uint8_t *indicesPtr;
        size_t indicesCount;
        uint8_t *texturesPtr;
        size_t texturesCount;
        uint8_t *materialsPtr;
        size_t materialsCount;
        uint8_t *bonesPtr;
        size_t bonesCount;
        uint8_t *morphsPtr;
        size_t morphsCount;
        uint8_t *displayNamesPtr;
        size_t displayNamesCount;
        uint8_t *rigidBodiesPtr;
        size_t rigidBodiesCount;
        uint8_t *jointsPtr;
        size_t jointsCount;
        uint8_t *endPtr;
    };

    /**
     * Constructor
     */
    Model();
    ~Model();

    bool preparse(const uint8_t *data, size_t size, DataInfo &info);

    /**
     * Read and parse the buffer with id and sets it's result to the class.
     *
     * @param data The buffer to read and parse
     */
    bool load(const uint8_t *data, size_t size);

    void save(uint8_t *data) const;

    const StaticString *name() const { return m_name; }
    const StaticString *englishName() const { return m_englishName; }
    const StaticString *comment() const { return m_comment; }
    const StaticString *englishComment() const { return m_englishComment; }
    const StaticString *texture(int index) const;
    void getTextures(Array<StaticString *> &value) const { value.copy(m_textures); }
    Encoding encoding() const { return m_encoding; }
    Error error() const { return m_error; }

private:
    void release();
    void parseNamesAndComments(const DataInfo &info);
    void parseVertices(const DataInfo &info);
    void parseIndices(const DataInfo &info);
    void parseTextures(const DataInfo &info);
    void parseMaterials(const DataInfo &info);
    void parseBones(const DataInfo &info);
    void parseMorphs(const DataInfo &info);
    void parseDisplayNames(const DataInfo &info);
    void parseRigidBodies(const DataInfo &info);
    void parseJoints(const DataInfo &info);

    Array<Vertex *> m_vertices;
    Array<int> m_indices;
    Array<StaticString *> m_textures;
    Array<Material *> m_materials;
    Array<Bone *> m_bones;
    Array<Morph *> m_morphs;
    Array<RigidBody *> m_rigidBodies;
    Array<Joint *> m_joints;
    StaticString *m_name;
    StaticString *m_englishName;
    StaticString *m_comment;
    StaticString *m_englishComment;
    Error m_error;
    Encoding m_encoding;

    VPVL2_DISABLE_COPY_AND_ASSIGN(Model)
};

} /* namespace pmx */
} /* namespace vpvl2 */

#endif

