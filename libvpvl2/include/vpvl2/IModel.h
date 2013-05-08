/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
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

#pragma once
#ifndef VPVL2_IMODEL_H_
#define VPVL2_IMODEL_H_

#include "vpvl2/Common.h"
#include "vpvl2/IVertex.h"

class btDiscreteDynamicsWorld;

namespace vpvl2
{

class IBone;
class ILabel;
class IMaterial;
class IMorph;
class IString;
class Scene;

/**
 * モデルをあらわすインターフェースです。
 *
 */
class VPVL2_API IModel
{
public:
    struct Buffer {
        enum StrideType {
            kVertexStride,
            kNormalStride,
            kTextureCoordStride,
            kMorphDeltaStride,
            kEdgeSizeStride,
            kEdgeVertexStride,
            kVertexIndexStride,
            kBoneIndexStride,
            kBoneWeightStride,
            kUVA0Stride,
            kUVA1Stride,
            kUVA2Stride,
            kUVA3Stride,
            kUVA4Stride,
            kIndexStride,
            kMaxStrideType
        };
        virtual ~Buffer() {}
        virtual size_t size() const = 0;
        virtual size_t strideOffset(StrideType type) const = 0;
        virtual size_t strideSize() const = 0;
        virtual const void *ident() const = 0;
    };
    struct DynamicVertexBuffer : Buffer {
        virtual void update(void *address,
                            const Vector3 &cameraPosition,
                            Vector3 &aabbMin,
                            Vector3 &aabbMax) const = 0;
        virtual void setParallelUpdateEnable(bool value) = 0;
        virtual void setSkinningEnable(bool value) = 0;
    };
    struct StaticVertexBuffer : Buffer {
        virtual void update(void *address) const = 0;
    };
    struct IndexBuffer : Buffer {
        enum Type {
            kIndex8,
            kIndex16,
            kIndex32,
            kMaxIndexType
        };
        virtual const void *bytes() const = 0;
        virtual int indexAt(int value) const = 0;
        virtual Type type() const = 0;
    };
    struct MatrixBuffer {
        virtual ~MatrixBuffer() {}
        virtual void update(void *address) = 0;
        virtual const float *bytes(int materialIndex) const = 0;
        virtual size_t size(int materialIndex) const = 0;
    };

    /**
      * Type of parsing errors.
      */
    enum ErrorType
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
        kInvalidLabelsError,
        kInvalidRigidBodiesError,
        kInvalidJointsError,
        kMaxErrorType
    };
    enum ObjectType {
        kBone,
        kIK,
        kIndex,
        kJoint,
        kMaterial,
        kMorph,
        kRigidBody,
        kVertex,
        kMaxObjectType
    };
    enum Type {
        kUnknownModel,
        kAssetModel,
        kPMDModel,
        kPMXModel,
        kMaxModelType
    };

    virtual ~IModel() {}

    /**
     * モデルの型を返します.
     *
     * @return Type
     */
    virtual Type type() const = 0;

    /**
     * モデル名(日本語)を返します.
     *
     * @return IString
     */
    virtual const IString *name() const = 0;

    /**
     * モデル名(英語)を返します.
     *
     * @return IString
     */
    virtual const IString *englishName() const = 0;

    /**
     * モデルの説明(日本語)を返します.
     *
     * @return IString
     */
    virtual const IString *comment() const = 0;

    /**
     * モデルの説明(英語)を返します.
     *
     * @return IString
     */
    virtual const IString *englishComment() const = 0;

    /**
     * モデルが可視であるかどうかを返します.
     *
     * @return bool
     */
    virtual bool isVisible() const = 0;

    /**
     * エラーを返します.
     *
     * IModel::load(data, size) が false を返した場合、error は kNoError 以外の値を返します。
     * true を返した場合は常に error は kNoError を返します。
     *
     * @return Error
     */
    virtual ErrorType error() const = 0;

    /**
     * オンメモリ上にある data とその長さ size に基づいてモデルを構築します.
     *
     * モデルの読み込みに成功した場合は true を、失敗した場合は false を返し、
     * IModel::error() が kNoError 以外の値を返すようになります。
     *
     * load は複数回呼んでも IModel では正しく処理されますが、IRenderEngine が
     * 正常に動作しなくなるため、IRenderEngine を利用している場合は複数回呼んではいけません。
     *
     * @param data
     * @param size
     * @return bool
     */
    virtual bool load(const uint8_t *data, size_t size) = 0;

    /**
     * オンメモリ上にある data に IModel のインスタンスに基づいてデータを書き込みます.
     *
     * data の長さは IModel::estimateSize() が返す値を利用してください。
     * type が kPMD または kPMX の場合は完全なモデルデータを構築します。kAsset の場合は何もしません。
     *
     * @param data
     */
    virtual void save(uint8_t *data, size_t &written) const = 0;

    /**
     * IModel::save(data) に必要なデータの長さを返します.
     *
     * これは save を呼ぶ前に save に渡すデータをメモリ上に確保する量を求めるときに使います。
     * save と併せて使用する必要があります。
     *
     * @return size_t
     */
    virtual size_t estimateSize() const = 0;

    /**
     * モデルの全ての剛体と拘束条件を物理世界に追加します.
     *
     * isPhysicsEnabled が false で呼び出された場合は何も処理されないため、
     * joinWorld を呼ぶ前に setPhysicsEnable を true にしてから呼び出しください。
     *
     * @brief joinWorld
     * @param worldRef
     */
    virtual void joinWorld(btDiscreteDynamicsWorld *worldRef) = 0;

    /**
     * モデルの全ての剛体と拘束条件を物理世界から削除します.
     *
     * joinWorld の呼び出し時に isPhysicsEnabled が true で後で false に変更された場合を考慮して
     * leaveWorld は joinWorld と異なり isPhysicsEnabled の状態関わらず常に処理されます。
     *
     * @brief leaveWorld
     * @param worldRef
     */
    virtual void leaveWorld(btDiscreteDynamicsWorld *worldRef) = 0;

    /**
     * 物理世界を初期化します.
     *
     * @brief resetMotionState
     * @param worldRef
     */
    virtual void resetMotionState(btDiscreteDynamicsWorld *worldRef) = 0;

    /**
     * モデルの変形を実行します.
     *
     * @param Vector3
     */
    virtual void performUpdate() = 0;

    /**
     * ボーン名から IBone のインスタンスを返します.
     *
     * 該当するボーン名の IBone インスタンスが見つかった場合はそれを返します。
     * 見つからなかった場合または value が null の場合は null を返します。
     *
     * @param IString
     * @return IBone
     */
    virtual IBone *findBone(const IString *value) const = 0;

    /**
     * ボーン名から IMorph のインスタンスを返します.
     *
     * 該当するボーン名の IMorph インスタンスが見つかった場合はそれを返します。
     * 見つからなかった場合または value が null の場合は null を返します。
     *
     * @param IString
     * @return IMorph
     */
    virtual IMorph *findMorph(const IString *value) const = 0;

    /**
     * 型からインスタンスの数を取得します.
     *
     * @brief count
     * @param value
     * @return
     */
    virtual int count(ObjectType value) const = 0;

    /**
     * ボーンのインスタンスの配列を取得します.
     *
     * 引数にモデルに存在する全ての IBone インスタンスのポインタ参照が入ります。
     * ポインタ参照を返すため、delete で解放してはいけません。
     *
     * @brief getBoneRefs
     * @param value
     */
    virtual void getBoneRefs(Array<IBone *> &value) const = 0;

    /**
     * ラベルのインスタンスの配列を取得します.
     *
     * 引数にモデルに存在する全ての ILabel インスタンスのポインタ参照が入ります。
     * ポインタ参照を返すため、delete で解放してはいけません。
     *
     * @brief getLabelRefs
     * @param value
     */
    virtual void getLabelRefs(Array<ILabel *> &value) const = 0;

    /**
     * 材質のインスタンスの配列を取得します.
     *
     * 引数にモデルに存在する全ての IMaterial インスタンスのポインタ参照が入ります。
     * ポインタ参照を返すため、delete で解放してはいけません。
     *
     * @brief getMaterialRefs
     * @param value
     */
    virtual void getMaterialRefs(Array<IMaterial *> &value) const = 0;

    /**
     * モーフのインスタンスの配列を取得します.
     *
     * 引数にモデルに存在する全ての IMorph インスタンスのポインタ参照が入ります。
     * ポインタ参照を返すため、delete で解放してはいけません。
     *
     * @brief getMorphRefs
     * @param value
     */
    virtual void getMorphRefs(Array<IMorph *> &value) const = 0;

    /**
     * 頂点のインスタンスの配列を取得します.
     *
     * 引数にモデルに存在する全ての IVertex インスタンスのポインタ参照が入ります。
     * ポインタ参照を返すため、delete で解放してはいけません。
     *
     * @brief getVertexRefs
     * @param value
     */
    virtual void getVertexRefs(Array<IVertex *> &value) const = 0;

    virtual void getIndices(Array<int> &value) const = 0;

    /**
     * カメラの位置からモデルに適用するエッジ幅を取得します.
     *
     * @brief edgeScaleFactor
     * @param cameraPosition
     * @return Vector3
     */
    virtual IVertex::EdgeSizePrecision edgeScaleFactor(const Vector3 &cameraPosition) const = 0;

    /**
     * ワールド座標系におけるモデルの補正位置を返します.
     *
     * @brief worldPosition
     * @return
     */
    virtual Vector3 worldPosition() const = 0;

    /**
     * ワールド座標系におけるモデルの補正回転量を返します.
     *
     * @brief worldRotation
     * @return
     */
    virtual Quaternion worldRotation() const = 0;

    /**
     * モデルの不透明度を返します.
     *
     * @brief opacity
     * @return
     */
    virtual Scalar opacity() const = 0;

    /**
     * モデルの拡大率を返します.
     *
     * 通常 PMD/PMX の場合は 1.0 を、アクセサリの場合は 10.0 を返します。
     *
     * @brief scaleFactor
     * @return
     */
    virtual Scalar scaleFactor() const = 0;

    /**
     * モデルのエッジ色を返します.
     *
     * @brief edgeColor
     * @return
     */
    virtual Vector3 edgeColor() const = 0;

    /**
     * モデルのエッジ幅を返します.
     *
     * @brief edgeWidth
     * @return
     */
    virtual Scalar edgeWidth() const = 0;

    /**
     * 親の Scene インスタンス参照先を返します.
     *
     * @brief parentSceneRef
     * @return
     */
    virtual Scene *parentSceneRef() const = 0;

    /**
     * 親のモデルのインスタンス参照を返します.
     *
     * @brief parentModelRef
     * @return
     */
    virtual IModel *parentModelRef() const = 0;

    /**
     * 親のボーンのインスタンス参照を返します.
     *
     * @brief parentBoneRef
     * @return
     */
    virtual IBone *parentBoneRef() const = 0;

    /**
     * モデル名の日本語名設定します.
     *
     * @brief setName
     * @param value
     */
    virtual void setName(const IString *value) = 0;

    /**
     * モデル名の英語名を設定します.
     *
     * @brief setEnglishName
     * @param value
     */
    virtual void setEnglishName(const IString *value) = 0;

    /**
     * モデルの日本語でのコメントを設定します.
     *
     * @brief setComment
     * @param value
     */
    virtual void setComment(const IString *value) = 0;

    /**
     * モデルの英語でのコメントを設定します.
     *
     * @brief setEnglishComment
     * @param value
     */
    virtual void setEnglishComment(const IString *value) = 0;

    /**
     * ワールド座標系におけるモデルの補正位置を設定します.
     *
     * @brief setWorldPosition
     * @param value
     */
    virtual void setWorldPosition(const Vector3 &value) = 0;

    /**
     * ワールド座標系におけるモデルの補正回転量を設定します.
     *
     * @brief setWorldRotation
     * @param value
     */
    virtual void setWorldRotation(const Quaternion &value) = 0;

    /**
     * モデルの不透明度を設定します.
     *
     * @brief setOpacity
     * @param value
     */
    virtual void setOpacity(const Scalar &value) = 0;

    /**
     * モデルの拡大率を設定します.
     *
     * @brief setScaleFactor
     * @param value
     */
    virtual void setScaleFactor(const Scalar &value) = 0;

    /**
     * モデルのエッジ色を設定します.
     *
     * @brief setEdgeColor
     * @param value
     */
    virtual void setEdgeColor(const Vector3 &value) = 0;

    /**
     * モデルのエッジ幅を設定します.
     *
     * @brief setEdgeWidth
     * @param value
     */
    virtual void setEdgeWidth(const Scalar &value) = 0;

    /**
     * モデルの親のモデルインスタンス参照を設定します.
     *
     * @brief setParentModelRef
     * @param value
     */
    virtual void setParentModelRef(IModel *value) = 0;

    /**
     * モデルの親のボーンインスタンス参照を設定します.
     *
     * @brief setParentBoneRef
     * @param value
     */
    virtual void setParentBoneRef(IBone *value) = 0;

    /**
     * モデルが可視かどうかを設定します.
     *
     * @brief setVisible
     * @param value
     */
    virtual void setVisible(bool value) = 0;

    virtual bool isPhysicsEnabled() const = 0;

    virtual void setPhysicsEnable(bool value) = 0;

    /**
     * インデックスバッファを取得します.
     *
     * 引数は delete で一度解放してから IIndexBuffer のインスタンスが入ります。
     *
     * @brief getIndexBuffer
     * @param indexBuffer
     */
    virtual void getIndexBuffer(IndexBuffer *&indexBuffer) const = 0;

    /**
     * 静的な頂点バッファを取得します.
     *
     * 引数は delete で一度解放してから IStaticVertexBuffer のインスタンスが入ります。
     *
     * @brief getStaticVertexBuffer
     * @param staticBuffer
     */
    virtual void getStaticVertexBuffer(StaticVertexBuffer *&staticBuffer) const = 0;

    /**
     * 動的な頂点バッファを取得します.
     *
     * 引数は delete で一度解放してから IDynamicVertexBuffer のインスタンスが入ります。
     * IIndexBuffer は同じ型で取得したインスタンスを渡す必要があります。
     * 条件を満たさない場合は dynamicBuffer に 0 が入ります。
     *
     * @brief getDynamicVertexBuffer
     * @param dynamicBuffer
     * @param indexBuffer
     */
    virtual void getDynamicVertexBuffer(DynamicVertexBuffer *&dynamicBuffer,
                                        const IndexBuffer *indexBuffer) const = 0;

    /**
     * ボーン行列のバッファを取得します.
     *
     * 引数は delete で一度解放してから IMatrixBuffer のインスタンスが入ります。
     * IDynamicVertexBuffer と IIndexBuffer は同じ型で取得したインスタンスを渡す必要があります。
     * 条件を満たさない場合は matrixBuffer に 0 が入ります。
     *
     * @brief getMatrixBuffer
     * @param matrixBuffer
     * @param dynamicBuffer
     * @param indexBuffer
     */
    virtual void getMatrixBuffer(MatrixBuffer *&matrixBuffer,
                                 DynamicVertexBuffer *dynamicBuffer,
                                 const IndexBuffer *indexBuffer) const = 0;

    /**
     * AABB (Axis Aligned Bounding Box) の最小値と最大値を設定します.
     *
     * @brief setAabb
     * @param min
     * @param max
     */
    virtual void setAabb(const Vector3 &min, const Vector3 &max) = 0;

    /**
     * AABB (Axis Aligned Bounding Box) の最小値と最大値を取得します.
     *
     * 引数に AABB の最小値と最大値が更新されます。setAabb が呼ばれていない場合はそれぞれ kZeroV3 が設定されます。
     *
     * @brief getAabb
     * @param min
     * @param max
     */
    virtual void getAabb(Vector3 &min, Vector3 &max) const = 0;

    virtual float32_t version() const = 0;
    virtual void setVersion(float32_t value) = 0;
    virtual IBone *createBone() = 0;
    virtual ILabel *createLabel() = 0;
    virtual IMaterial *createMaterial() = 0;
    virtual IMorph *createMorph() = 0;
    virtual IVertex *createVertex() = 0;
    virtual IBone *findBoneAt(int value) const = 0;
    virtual ILabel *findLabelAt(int value) const = 0;
    virtual IMaterial *findMaterialAt(int value) const = 0;
    virtual IMorph *findMorphAt(int value) const = 0;
    virtual IVertex *findVertexAt(int value) const = 0;
    virtual void setIndices(const Array<int> &value) = 0;
    virtual void addBone(IBone *value) = 0;
    virtual void addLabel(ILabel *value) = 0;
    virtual void addMaterial(IMaterial *value) = 0;
    virtual void addMorph(IMorph *value) = 0;
    virtual void addVertex(IVertex *value) = 0;
    virtual void removeBone(IBone *value) = 0;
    virtual void removeLabel(ILabel *value) = 0;
    virtual void removeMaterial(IMaterial *value) = 0;
    virtual void removeMorph(IMorph *value) = 0;
    virtual void removeVertex(IVertex *value) = 0;
};

} /* namespace vpvl2 */

#endif
