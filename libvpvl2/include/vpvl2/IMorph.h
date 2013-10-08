/**

 Copyright (c) 2010-2013  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#pragma once
#ifndef VPVL2_IMORPH_H_
#define VPVL2_IMORPH_H_

#include "vpvl2/IEncoding.h"

namespace vpvl2
{

class IBone;
class IMaterial;
class IModel;
class IRigidBody;
class IString;
class IVertex;

/**
 * モデルのモーフをあらわすインターフェースです。
 *
 */
class VPVL2_API IMorph
{
public:
#ifdef VPVL2_ENABLE_GLES2
    typedef float32 WeightPrecision;
#else
    typedef float64 WeightPrecision;
#endif
    enum Type {
        kUnknownMorph = -1,
        kGroupMorph,
        kVertexMorph,
        kBoneMorph,
        kTexCoordMorph,
        kUVA1Morph,
        kUVA2Morph,
        kUVA3Morph,
        kUVA4Morph,
        kMaterialMorph,
        kFlipMorph,
        kImpulseMorph,
        kMaxMorphType
    };
    enum Category {
        kBase,
        kEyeblow,
        kEye,
        kLip,
        kOther,
        kMaxCategoryType
    };

    class PropertyEventListener {
    public:
        virtual ~PropertyEventListener() {}
        virtual void nameWillChange(const IString *value, IEncoding::LanguageType type, IMorph *morph) = 0;
        virtual void weightWillChange(const WeightPrecision &value, IMorph *morph) = 0;
    };
    struct Bone {
        Bone()
            : bone(0),
              index(-1)
        {
        }
        IBone *bone;
        Vector3 position;
        Quaternion rotation;
        int index;
    };
    struct Group {
        Group()
            : morph(0),
              fixedWeight(0),
              index(-1)
        {
        }
        IMorph *morph;
        WeightPrecision fixedWeight;
        int index;
    };
    struct Material {
        Material()
            : materials(0),
              shininess(0),
              edgeSize(0),
              index(-1),
              operation(0)
        {
        }
        ~Material() {
            delete materials;
            materials = 0;
        }
        Array<IMaterial *> *materials;
        Vector3 ambient;
        Vector4 diffuse;
        Vector3 specular;
        Vector4 edgeColor;
        Vector4 textureWeight;
        Vector4 sphereTextureWeight;
        Vector4 toonTextureWeight;
        float32 shininess;
        IVertex::EdgeSizePrecision edgeSize;
        int index;
        uint8 operation;
    };
    struct UV {
        UV()
            : vertex(0),
              index(-1),
              offset(0)
        {
        }
        IVertex *vertex;
        Vector4 position;
        uint32 index;
        int offset;
    };
    struct Vertex {
        Vertex()
            : vertex(0),
              index(-1),
              base(-1) /* for pmd::Morph */
        {
        }
        IVertex *vertex;
        Vector3 position;
        uint32 index;
        uint32 base;
    };
    struct Flip {
        Flip()
            : morph(0),
              fixedWeight(0),
              index(-1)
        {
        }
        IMorph *morph;
        WeightPrecision fixedWeight;
        int index;
    };
    struct Impulse {
        Impulse()
            : rigidBody(0),
              velocity(kZeroV3),
              torque(kZeroV3),
              index(-1),
              isLocal(false)
        {
        }
        IRigidBody *rigidBody;
        Vector3 velocity;
        Vector3 torque;
        int index;
        bool isLocal;
    };

    virtual ~IMorph() {}

    virtual void addEventListenerRef(PropertyEventListener *value) = 0;
    virtual void removeEventListenerRef(PropertyEventListener *value) = 0;
    virtual void getEventListenerRefs(Array<PropertyEventListener *> &value) = 0;

    /**
     * モーフの名前を返します.
     *
     * @return IString
     */
    virtual const IString *name(IEncoding::LanguageType type) const = 0;

    /**
     * モーフの名前を設定します.
     *
     * @brief setName
     * @param value
     * @param type
     */
    virtual void setName(const IString *value, IEncoding::LanguageType type) = 0;

    /**
     * モーフの ID を返します.
     *
     * 常にユニークな値で返します。
     *
     * @return int
     */
    virtual int index() const = 0;

    /**
     * 親のモデルのインスタンスを返します.
     *
     * @brief parentModelRef
     * @return IModel
     */
    virtual IModel *parentModelRef() const = 0;

    /**
     * モーフのカテゴリを返します.
     *
     * @return Category
     */
    virtual Category category() const = 0;

    /**
     * モーフの種類を返します.
     *
     * @return Type
     */
    virtual Type type() const = 0;

    /**
     * グループモーフに所属しているかを返します.
     *
     * @return bool
     */
    virtual bool hasParent() const = 0;

    /**
     * モーフの変形度係数を返します.
     *
     * @return float
     * @sa setWeight
     */
    virtual WeightPrecision weight() const = 0;

    /**
     * 係数 value に基づいて変形を行います.
     *
     * value は 0.0 以上 1.0 以下でなければなりません。
     *
     * @param float
     * @sa weight
     */
    virtual void setWeight(const WeightPrecision &value) = 0;

    /**
     * 強制的にモーフの更新を行うように指示します.
     *
     * @brief markDirty
     */
    virtual void markDirty() = 0;

    /**
     * ボーンモーフを追加します.
     *
     * IMorph::Bone は必ずヒープ上で確保してから追加して下さい。追加された後のメモリ管理が
     * IMorph のインスタンスに委譲されるため、追加後は delete で解放してはいけません。
     *
     * 親の IModel インスタンスが pmx の場合のみ有効です。それ以外の場合は何もしません。
     *
     * @brief addBone
     * @param value
     */
    virtual void addBoneMorph(Bone *value) = 0;

    virtual void removeBoneMorph(Bone *value) = 0;

    /**
     * グループモーフを追加します.
     *
     * IMorph::Group は必ずヒープ上で確保してから追加して下さい。追加された後のメモリ管理が
     * IMorph のインスタンスに委譲されるため、追加後は delete で解放してはいけません。
     *
     * 親の IModel インスタンスが pmx の場合のみ有効です。それ以外の場合は何もしません。
     *
     * @brief addGroup
     * @param value
     */
    virtual void addGroupMorph(Group *value) = 0;

    virtual void removeGroupMorph(Group *value) = 0;

    /**
     * 材質モーフを追加します.
     *
     * IMorph::Material は必ずヒープ上で確保してから追加して下さい。追加された後のメモリ管理が
     * IMorph のインスタンスに委譲されるため、追加後は delete で解放してはいけません。
     *
     * 親の IModel インスタンスが pmx の場合のみ有効です。それ以外の場合は何もしません。
     *
     * @brief addMaterial
     * @param value
     */
    virtual void addMaterialMorph(Material *value) = 0;

    virtual void removeMaterialMorph(Material *value) = 0;

    /**
     * UV モーフを追加します.
     *
     * IMorph::UV は必ずヒープ上で確保してから追加して下さい。追加された後のメモリ管理が
     * IMorph のインスタンスに委譲されるため、追加後は delete で解放してはいけません。
     *
     * 親の IModel インスタンスが pmx の場合のみ有効です。それ以外の場合は何もしません。
     *
     * @brief addUV
     * @param value
     */
    virtual void addUVMorph(UV *value) = 0;

    virtual void removeUVMorph(UV *value) = 0;

    /**
     * 頂点モーフを追加します.
     *
     * IMorph::Vertex は必ずヒープ上で確保してから追加して下さい。追加された後のメモリ管理が
     * IMorph のインスタンスに委譲されるため、追加後は delete で解放してはいけません。
     *
     * 親の IModel インスタンスが pmd または pmx の場合のみ有効です。それ以外の場合は何もしません。
     *
     * @brief addVertex
     * @param value
     */
    virtual void addVertexMorph(Vertex *value) = 0;

    virtual void removeVertexMorph(Vertex *value) = 0;

    /**
     * フリップモーフを追加します.
     *
     * IMorph::Flip は必ずヒープ上で確保してから追加して下さい。追加された後のメモリ管理が
     * IMorph のインスタンスに委譲されるため、追加後は delete で解放してはいけません。
     *
     * 親の IModel インスタンスが pmx の場合のみ有効です。それ以外の場合は何もしません。
     *
     * @brief addFlip
     * @param value
     */
    virtual void addFlipMorph(Flip *value) = 0;

    virtual void removeFlipMorph(Flip *value) = 0;

    /**
     * インパルスモーフを追加します.
     *
     * IMorph::Impulse は必ずヒープ上で確保してから追加して下さい。追加された後のメモリ管理が
     * IMorph のインスタンスに委譲されるため、追加後は delete で解放してはいけません。
     *
     * 親の IModel インスタンスが pmx の場合のみ有効です。それ以外の場合は何もしません。
     *
     * @brief addImpulse
     * @param value
     */
    virtual void addImpulseMorph(Impulse *value) = 0;

    virtual void removeImpulseMorph(Impulse *value) = 0;

    /**
     * モーフ種別を設定します.
     *
     * モーフ種別は保存時に使用され、モーフ種別によって書き出しが変更されます。
     * 例えば kBoneMorph に設定された場合 addBone によって追加されたボーンモーフのみが書き出されます。
     * それ以外に追加されたモーフは書き出されません。
     *
     * 親の IModel インスタンスが pmx の場合のみ有効です。それ以外の場合は何もしません。
     * pmd は仕様上頂点モーフしか種別が無いため、常に頂点モーフのみが書き出されます。
     *
     * @brief addBone
     * @param value
     */
    virtual void setType(Type value) = 0;

    virtual void getBoneMorphs(Array<Bone *> &morphs) const = 0;
    virtual void getGroupMorphs(Array<Group *> &morphs) const = 0;
    virtual void getMaterialMorphs(Array<Material *> &morphs) const = 0;
    virtual void getUVMorphs(Array<UV *> &morphs) const = 0;
    virtual void getVertexMorphs(Array<Vertex *> &morphs) const = 0;
    virtual void getFlipMorphs(Array<Flip *> &morphs) const = 0;
    virtual void getImpulseMorphs(Array<Impulse *> &morphs) const = 0;
};

} /* namespace vpvl2 */

#endif
