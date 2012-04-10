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
#include "vpvl2/IBone.h"
#include "vpvl2/IEncoding.h"
#include "vpvl2/IModel.h"
#include "vpvl2/IMorph.h"
#include "vpvl2/IString.h"

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
class Label;
class Joint;
class Material;
class Morph;
class RigidBody;
class Vertex;

class VPVL2_API Model : public IModel
{
public:
    class UserData {
    public:
        virtual ~UserData() {}
    };
    struct SkinnedVertex;

    enum StrideType {
        kVertexStride,
        kNormalStride,
        kTexCoordStride,
        kUVA0Stride,
        kUVA1Stride,
        kUVA2Stride,
        kUVA3Stride,
        kUVA4Stride,
        kIndexStride
    };

    struct DataInfo
    {
        IEncoding *encoding;
        uint8_t *basePtr;
        uint8_t *namePtr;
        IString::Codec codec;
        Error error;
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
    Model(IEncoding *encoding);
    ~Model();

    static size_t strideOffset(StrideType type);
    static size_t strideSize(StrideType type);

    /**
     * Read and parse the buffer with id and sets it's result to the class.
     *
     * @param data The buffer to read and parse
     */
    bool load(const uint8_t *data, size_t size);
    void save(uint8_t *data) const;
    size_t estimateSize() const;

    void resetVertices();
    void joinWorld(btDiscreteDynamicsWorld *world);
    void leaveWorld(btDiscreteDynamicsWorld *world);
    IBone *findBone(const IString *value) const;
    IMorph *findMorph(const IString *value) const;

    bool preparse(const uint8_t *data, size_t size, DataInfo &info);
    void setUserData(UserData *value);
    void setVisible(bool value);
    void update();

    const void *vertexPtr() const;
    const void *indicesPtr() const;

    const Array<Vertex *> &vertices() const { return m_vertices; }
    const Array<int> &indices() const { return m_indices; }
    const Array<IString *> &textures() const { return m_textures; }
    const Array<Material *> &materials() const { return m_materials; }
    const Array<Bone *> &bones() const { return m_bones; }
    const Array<Morph *> &morphs() const { return m_morphs; }
    const Array<RigidBody *> &rigidBodies() const { return m_rigidBodies; }
    const Array<Joint *> &joints() const { return m_joints; }
    const IString *name() const { return m_name; }
    const IString *englishName() const { return m_englishName; }
    const IString *comment() const { return m_comment; }
    const IString *englishComment() const { return m_englishComment; }
    UserData *userData() const { return m_userData; }
    Error error() const { return m_info.error; }
    bool isVisible() const { return m_visible; }

    void setName(const IString *value);
    void setEnglishName(const IString *value);
    void setComment(const IString *value);
    void setEnglishComment(const IString *value);

private:
    void release();
    void parseNamesAndComments(const DataInfo &info);
    void parseVertices(const DataInfo &info);
    void parseIndices(const DataInfo &info);
    void parseTextures(const DataInfo &info);
    void parseMaterials(const DataInfo &info);
    void parseBones(const DataInfo &info);
    void parseMorphs(const DataInfo &info);
    void parseLabels(const DataInfo &info);
    void parseRigidBodies(const DataInfo &info);
    void parseJoints(const DataInfo &info);

    btDiscreteDynamicsWorld *m_world;
    IEncoding *m_encoding;
    Array<Vertex *> m_vertices;
    Array<int> m_indices;
    Array<IString *> m_textures;
    Array<Material *> m_materials;
    Array<Bone *> m_bones;
    Array<Bone *> m_BPSOrderedBones;
    Array<Bone *> m_APSOrderedBones;
    Array<Morph *> m_morphs;
    Array<Label *> m_labels;
    Array<RigidBody *> m_rigidBodies;
    Array<Joint *> m_joints;
    Hash<HashString, IBone *> m_name2bones;
    Hash<HashString, IMorph *> m_name2morphs;
    SkinnedVertex *m_skinnedVertices;
    int *m_skinnedIndices;
    IString *m_name;
    IString *m_englishName;
    IString *m_comment;
    IString *m_englishComment;
    UserData *m_userData;
    DataInfo m_info;
    bool m_visible;

    VPVL2_DISABLE_COPY_AND_ASSIGN(Model)
};

} /* namespace pmx */
} /* namespace vpvl2 */

#endif

