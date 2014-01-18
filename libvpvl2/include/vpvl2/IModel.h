/**

 Copyright (c) 2010-2014  hkrn

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
#ifndef VPVL2_IMODEL_H_
#define VPVL2_IMODEL_H_

#include "vpvl2/IEncoding.h"
#include "vpvl2/IProgressReporter.h"
#include "vpvl2/IVertex.h"

class btDiscreteDynamicsWorld;

namespace vpvl2
{

class IBone;
class IJoint;
class ILabel;
class IMaterial;
class IMorph;
class IRigidBody;
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
            kUVA1Stride,
            kUVA2Stride,
            kUVA3Stride,
            kUVA4Stride,
            kIndexStride,
            kMaxStrideType
        };
        virtual ~Buffer() {}
        virtual vsize size() const = 0;
        virtual vsize strideOffset(StrideType type) const = 0;
        virtual vsize strideSize() const = 0;
        virtual const void *ident() const = 0;
    };
    struct DynamicVertexBuffer : Buffer {
        virtual void setupBindPose(void *address) const = 0;
        virtual void update(void *address) const = 0;
        virtual void performTransform(void *address, const Vector3 &cameraPosition, Vector3 &aabbMin, Vector3 &aabbMax) const = 0;
        virtual void setParallelUpdateEnable(bool value) = 0;
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
        virtual const float32 *bytes(int materialIndex) const = 0;
        virtual vsize size(int materialIndex) const = 0;
    };
    class PropertyEventListener {
    public:
        virtual ~PropertyEventListener() {}
        virtual void nameWillChange(const IString *value, IEncoding::LanguageType type, IModel *model) = 0;
        virtual void commentWillChange(const IString *value, IEncoding::LanguageType type, IModel *model) = 0;
        virtual void worldTranslationWillChange(const Vector3 &value, IModel *model) = 0;
        virtual void worldOrientationWillChange(const Quaternion &value, IModel *model) = 0;
        virtual void opacityWillChange(const Scalar &value, IModel *model) = 0;
        virtual void scaleFactorWillChange(const Scalar &value, IModel *model) = 0;
        virtual void edgeColorWillChange(const Vector3 &value, IModel *model) = 0;
        virtual void edgeWidthWillChange(const IVertex::EdgeSizePrecision &value, IModel *model) = 0;
        virtual void parentModelRefWillChange(IModel *value, IModel *model) = 0;
        virtual void parentBoneRefWillChange(IBone *value, IModel *model) = 0;
        virtual void visibleWillChange(bool value, IModel *model) = 0;
        virtual void physicsEnableWillChange(bool value, IModel *model) = 0;
        virtual void aabbWillChange(const Vector3 &min, const Vector3 &max, IModel *model) = 0;
        virtual void versionWillChange(float32 value, IModel *model) = 0;
    };

    /**
      * Type of parsing errors.
      */
    enum ErrorType {
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
        kInvalidSoftBodiesError,
        kMaxErrorType
    };
    enum ObjectType {
        kUnknownObjectType = -1,
        kBone,
        kIK,
        kIndex,
        kJoint,
        kMaterial,
        kMorph,
        kRigidBody,
        kSoftBody,
        kTexture,
        kVertex,
        kMaxObjectType
    };
    enum Type {
        kUnknownModelType = -1,
        kAssetModel,
        kPMDModel,
        kPMXModel,
        kMaxModelType
    };

    virtual ~IModel() {}

    virtual void addEventListenerRef(PropertyEventListener *value) = 0;
    virtual void removeEventListenerRef(PropertyEventListener *value) = 0;
    virtual void getEventListenerRefs(Array<PropertyEventListener *> &value) = 0;

    /**
     * モデルの型を返します.
     *
     * @return Type
     */
    virtual Type type() const = 0;

    /**
     * LanguageType に基づいたモデル名を返します.
     *
     * @return IString
     * @param type
     */
    virtual const IString *name(IEncoding::LanguageType type) const = 0;

    /**
     * LanguageType に基づいたモデルの説明文を返します.
     *
     * @return IString
     * @param type
     */
    virtual const IString *comment(IEncoding::LanguageType type) const = 0;

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
    virtual bool load(const uint8 *data, vsize size) = 0;

    /**
     * オンメモリ上にある data に IModel のインスタンスに基づいてデータを書き込みます.
     *
     * data の長さは IModel::estimateSize() が返す値を利用してください。
     * type が kPMD または kPMX の場合は完全なモデルデータを構築します。kAsset の場合は何もしません。
     *
     * @param data
     */
    virtual void save(uint8 *data, vsize &written) const = 0;

    /**
     * IModel::save(data) に必要なデータの長さを返します.
     *
     * これは save を呼ぶ前に save に渡すデータをメモリ上に確保する量を求めるときに使います。
     * save と併せて使用する必要があります。
     *
     * @return size_t
     */
    virtual vsize estimateSize() const = 0;

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
    virtual IBone *findBoneRef(const IString *value) const = 0;

    /**
     * ボーン名から IMorph のインスタンスを返します.
     *
     * 該当するボーン名の IMorph インスタンスが見つかった場合はそれを返します。
     * 見つからなかった場合または value が null の場合は null を返します。
     *
     * @param IString
     * @return IMorph
     */
    virtual IMorph *findMorphRef(const IString *value) const = 0;

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
     * ジョイントのインスタンスの配列を取得します.
     *
     * 引数にモデルに存在する全ての IJoint インスタンスのポインタ参照が入ります。
     * ポインタ参照を返すため、delete で解放してはいけません。
     *
     * @brief getJointRefs
     * @param value
     */
    virtual void getJointRefs(Array<IJoint *> &value) const = 0;

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
     * 剛体のインスタンスの配列を取得します.
     *
     * 引数にモデルに存在する全ての IRigidBody インスタンスのポインタ参照が入ります。
     * ポインタ参照を返すため、delete で解放してはいけません。
     *
     * @brief getJointRefs
     * @param value
     */
    virtual void getRigidBodyRefs(Array<IRigidBody *> &value) const = 0;

    /**
     * テクスチャのパスのインスタンスの配列を取得します.
     *
     * 引数にモデルに存在する全てのテクスチャのパス (IString) のインスタンスのポインタ参照が入ります。
     * ポインタ参照を返すため、delete で解放してはいけません。
     *
     * @brief getTextureRefs
     * @param value
     */
    virtual void getTextureRefs(Array<const IString *> &value) const = 0;

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

    /**
     * 頂点のインデックスの配列を取得します.
     *
     * @brief getIndices
     * @param value
     */
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
     * @brief worldTranslation
     * @return
     */
    virtual Vector3 worldTranslation() const = 0;

    /**
     * ワールド座標系におけるモデルの補正回転量を返します.
     *
     * @brief worldOrientation
     * @return
     */
    virtual Quaternion worldOrientation() const = 0;

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
    virtual Color edgeColor() const = 0;

    /**
     * モデルのエッジ幅を返します.
     *
     * @brief edgeWidth
     * @return
     */
    virtual IVertex::EdgeSizePrecision edgeWidth() const = 0;

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
     * LanguageType に基づいてモデル名の日本語名設定します.
     *
     * @brief setName
     * @param value
     * @param type
     */
    virtual void setName(const IString *value, IEncoding::LanguageType type) = 0;

    /**
     * LanguageType に基づいてモデルのコメントを設定します.
     *
     * @brief setComment
     * @param value
     * @param type
     */
    virtual void setComment(const IString *value, IEncoding::LanguageType type) = 0;

    /**
     * ワールド座標系におけるモデルの補正位置を設定します.
     *
     * @brief setWorldTranslation
     * @param value
     */
    virtual void setWorldTranslation(const Vector3 &value) = 0;

    /**
     * ワールド座標系におけるモデルの補正回転量を設定します.
     *
     * @brief setWorldOrientation
     * @param value
     */
    virtual void setWorldOrientation(const Quaternion &value) = 0;

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
    virtual void setEdgeColor(const Color &value) = 0;

    /**
     * モデルのエッジ幅を設定します.
     *
     * @brief setEdgeWidth
     * @param value
     */
    virtual void setEdgeWidth(const IVertex::EdgeSizePrecision &value) = 0;

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

    /**
     * 物理演算が有効かどうかを返します.
     *
     * @brief isPhysicsEnabled
     * @return
     */
    virtual bool isPhysicsEnabled() const = 0;

    /**
     * 物理演算の有効無効状態を設定します.
     *
     * このメソッドは純粋に有効無効状態を切り替えるだけの処理です。
     * これを呼び出すだけでは物理演算を有効または無効にすることが出来ません。
     *
     * 実際に物理演算を有効にするにはこれを呼び出した上で IModel#joinWorld を呼び出してから
     * Scene#update に引数 Scene::kResetMotionState つけて呼び出す必要があります。
     * その後 Scene#update に Scene::kUpdateModel をつけて呼び出して初めて機能します。
     *
     * 逆に無効にする場合は IModel#leaveWorld を呼び出し、そのあと Scene#update に
     * Scene::kUpdateModel を呼び出すことで物理演算を無効にすることが出来ます。
     *
     * @brief setPhysicsEnable
     * @param value
     */
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

    /**
     * モデルのバージョンを返します.
     *
     * IModel#type() が kAsset または kPMD の場合は常に 1.0 を返します。
     * IModel#type() が kPMX の場合はモデルに設定されているバージョンによって 2.0 または 2.1を返します。
     *
     * @brief version
     * @return
     */
    virtual float32 version() const = 0;

    /**
     * モデルのバージョンを設定します.
     *
     * このメソッドは IModel#type() が kPMX の場合のみです。その場合値は 2.0 または 2.1 を設定できます。
     * それ以外の場合このメソッドは何も行いません。
     *
     * @brief setVersion
     * @param value
     */
    virtual void setVersion(float32 value) = 0;

    /**
     * 頂点に格納可能な最大UV数を取得します.
     *
     * このメソッドが返す値は PMX の場合 0 から 4 を、それ以外の場合は 0 を返します。
     *
     * @brief maxUVCount
     * @return
     */
    virtual int maxUVCount() const = 0;

    /**
     * 頂点に格納可能な最大UV数を設定します.
     *
     * このメソッドは PMX の場合 0 から 4 の範囲で設定可能です。それ以外の場合は何もしません。
     *
     * @brief setMaxUVCount
     * @param value
     */
    virtual void setMaxUVCount(int value) = 0;

    /**
     * モデルのボーンのインスタンスを作成します.
     *
     * 作成されたインスタンスは作成元の IModel のインスタンスに対してのみ追加を行うことが出来ます。
     *
     * @brief createBone
     * @return
     */
    virtual IBone *createBone() = 0;

    /**
     * モデルのジョイントのインスタンスを作成します.
     *
     * 作成されたインスタンスは作成元の IModel のインスタンスに対してのみ追加を行うことが出来ます。
     *
     * @brief createJoint
     * @return
     */
    virtual IJoint *createJoint() = 0;

    /**
     * モデルのラベルのインスタンスを作成します.
     *
     * 作成されたインスタンスは作成元の IModel のインスタンスに対してのみ追加を行うことが出来ます。
     *
     * @brief createLabel
     * @return
     */
    virtual ILabel *createLabel() = 0;

    /**
     * モデルの材質のインスタンスを作成します.
     *
     * 作成されたインスタンスは作成元の IModel のインスタンスに対してのみ追加を行うことが出来ます。
     *
     * @brief createMaterial
     * @return
     */
    virtual IMaterial *createMaterial() = 0;

    /**
     * モデルのモーフのインスタンスを作成します.
     *
     * 作成されたインスタンスは作成元の IModel のインスタンスに対してのみ追加を行うことが出来ます。
     *
     * @brief createMorph
     * @return
     */
    virtual IMorph *createMorph() = 0;

    /**
     * モデルの剛体のインスタンスを作成します.
     *
     * 作成されたインスタンスは作成元の IModel のインスタンスに対してのみ追加を行うことが出来ます。
     *
     * @brief createRigidBody
     * @return
     */
    virtual IRigidBody *createRigidBody() = 0;

    /**
     * モデルの頂点のインスタンスを作成します.
     *
     * 作成されたインスタンスは作成元の IModel のインスタンスに対してのみ追加を行うことが出来ます。
     *
     * @brief createVertex
     * @return
     */
    virtual IVertex *createVertex() = 0;

    /**
     * 指定されたインデックスに対するボーンのインスタンスを返します.
     *
     * 0 未満またはモデルに存在するボーン数より大きい値を指定された場合は NULL を返します。
     *
     * @brief findBoneAt
     * @param value
     * @return
     */
    virtual IBone *findBoneRefAt(int value) const = 0;

    /**
     * 指定されたインデックスに対するジョイントのインスタンスを返します.
     *
     * 0 未満またはモデルに存在するジョイント数より大きい値を指定された場合は NULL を返します。
     *
     * @brief findJointAt
     * @param value
     * @return
     */
    virtual IJoint *findJointRefAt(int value) const = 0;

    /**
     * 指定されたインデックスに対するラベルのインスタンスを返します.
     *
     * 0 未満またはモデルに存在するラベル数より大きい値を指定された場合は NULL を返します。
     *
     * @brief findLabelAt
     * @param value
     * @return
     */
    virtual ILabel *findLabelRefAt(int value) const = 0;

    /**
     * 指定されたインデックスに対する材質のインスタンスを返します.
     *
     * 0 未満またはモデルに存在する材質数より大きい値を指定された場合は NULL を返します。
     *
     * @brief findMaterialAt
     * @param value
     * @return
     */
    virtual IMaterial *findMaterialRefAt(int value) const = 0;

    /**
     * 指定されたインデックスに対するモーフのインスタンスを返します.
     *
     * 0 未満またはモデルに存在するモーフ数より大きい値を指定された場合は NULL を返します。
     *
     * @brief findMorphAt
     * @param value
     * @return
     */
    virtual IMorph *findMorphRefAt(int value) const = 0;

    /**
     * 指定されたインデックスに対する剛体のインスタンスを返します.
     *
     * 0 未満またはモデルに存在する剛体数より大きい値を指定された場合は NULL を返します。
     *
     * @brief findRigidBodyAt
     * @param value
     * @return
     */
    virtual IRigidBody *findRigidBodyRefAt(int value) const = 0;

    /**
     * 指定されたインデックスに対する頂点のインスタンスを返します.
     *
     * 0 未満またはモデルに存在する頂点数より大きい値を指定された場合は NULL を返します。
     *
     * @brief findVertexAt
     * @param value
     * @return
     */
    virtual IVertex *findVertexRefAt(int value) const = 0;

    /**
     * 頂点のインデックスの配列を設定します.
     *
     * 頂点のインデックスの配列は設定時に全てのインデックスのチェックを行い、
     * 0 未満または頂点数を超える場合は 0 に設定されます。
     *
     * @brief setIndices
     * @param value
     */
    virtual void setIndices(const Array<int> &value) = 0;

    /**
     * ボーンのインスタンスを追加します.
     *
     * addBone を呼んでいる IModel のインスタンスによる createBone で作成された
     * ボーンのインスタンスのみ追加可能です。異なる IModel のインスタンスの場合は何も行われません。
     * 呼び出し後は IBone#index の値が -1 からモデル内のユニークなボーンの ID に設定されます。
     *
     * @brief addBone
     * @param value
     */
    virtual void addBone(IBone *value) = 0;

    /**
     * ジョイントのインスタンスを追加します.
     *
     * addJoint を呼んでいる IModel のインスタンスによる createJoint で作成された
     * ジョイントのインスタンスのみ追加可能です。異なる IModel のインスタンスの場合は何も行われません。
     * 呼び出し後は IJoint#index の値が -1 からモデル内のユニークなジョイントの ID に設定されます。
     *
     * @brief addJoint
     * @param value
     */
    virtual void addJoint(IJoint *value) = 0;

    /**
     * ラベルのインスタンスを追加します.
     *
     * addLabel を呼んでいる IModel のインスタンスによる createLabel で作成された
     * ラベルのインスタンスのみ追加可能です。異なる IModel のインスタンスの場合は何も行われません。
     * 呼び出し後は ILabel#index の値が -1 からモデル内のユニークなラベルの ID に設定されます。
     *
     * @brief addLabel
     * @param value
     */
    virtual void addLabel(ILabel *value) = 0;

    /**
     * 材質のインスタンスを追加します.
     *
     * addMaterial を呼んでいる IModel のインスタンスによる createMaterial で作成された
     * 材質のインスタンスのみ追加可能です。異なる IModel のインスタンスの場合は何も行われません。
     * 呼び出し後は IMaterial#index の値が -1 からモデル内のユニークな材質の ID に設定されます。
     *
     * @brief addMaterial
     * @param value
     */
    virtual void addMaterial(IMaterial *value) = 0;

    /**
     * モーフのインスタンスを追加します.
     *
     * addMorph を呼んでいる IModel のインスタンスによる createMorph で作成された
     * モーフのインスタンスのみ追加可能です。異なる IModel のインスタンスの場合は何も行われません。
     * 呼び出し後は IMorph#index の値が -1 からモデル内のユニークなモーフの ID に設定されます。
     *
     * @brief addMorph
     * @param value
     */
    virtual void addMorph(IMorph *value) = 0;

    /**
     * 剛体のインスタンスを追加します.
     *
     * addRigidBody を呼んでいる IModel のインスタンスによる createRigidBody で作成された
     * 剛体のインスタンスのみ追加可能です。異なる IModel のインスタンスの場合は何も行われません。
     * 呼び出し後は IRigidBody#index の値が -1 からモデル内のユニークな剛体の ID に設定されます。
     *
     * @brief addJoint
     * @param value
     */
    virtual void addRigidBody(IRigidBody *value) = 0;

    /**
     * 頂点のインスタンスを追加します.
     *
     * addVertex を呼んでいる IModel のインスタンスによる createVertex で作成された
     * ラベルのインスタンスのみ追加可能です。異なる IModel のインスタンスの場合は何も行われません。
     * 呼び出し後は IVertex#index の値が -1 からモデル内のユニークな頂点の ID に設定されます。
     *
     * @brief addVertex
     * @param value
     */
    virtual void addVertex(IVertex *value) = 0;

    /**
     * ボーンの削除を行います.
     *
     * 呼び出し後は引数の IBone#index の値が -1 に設定されます。
     *
     * @brief removeBone
     * @param value
     */
    virtual void removeBone(IBone *value) = 0;

    /**
     * ジョイントの削除を行います.
     *
     * 呼び出し後は引数の IJoint#index の値が -1 に設定されます。
     *
     * @brief removeJoint
     * @param value
     */
    virtual void removeJoint(IJoint *value) = 0;

    /**
     * ラベルの削除を行います.
     *
     * 呼び出し後は引数の ILabel#index の値が -1 に設定されます。
     *
     * @brief removeLabel
     * @param value
     */
    virtual void removeLabel(ILabel *value) = 0;

    /**
     * 材質の削除を行います.
     *
     * 呼び出し後は引数の IMaterial#index の値が -1 に設定されます。
     *
     * @brief removeMaterial
     * @param value
     */
    virtual void removeMaterial(IMaterial *value) = 0;

    /**
     * モーフの削除を行います.
     *
     * 呼び出し後は引数の IMorph#index の値が -1 に設定されます。
     *
     * @brief removeMorph
     * @param value
     */
    virtual void removeMorph(IMorph *value) = 0;

    /**
     * 剛体の削除を行います.
     *
     * 呼び出し後は引数の IRigidBody#index の値が -1 に設定されます。
     *
     * @brief removeRigidBody
     * @param value
     */
    virtual void removeRigidBody(IRigidBody *value) = 0;

    /**
     * 頂点の削除を行います.
     *
     * 呼び出し後は引数の IVertex#index の値が -1 に設定されます。
     *
     * @brief removeVertex
     * @param value
     */
    virtual void removeVertex(IVertex *value) = 0;

    virtual IProgressReporter *progressReporterRef() const = 0;

    virtual void setProgressReporterRef(IProgressReporter *value) = 0;
};

} /* namespace vpvl2 */

#endif
