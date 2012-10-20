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

#ifndef VPVL2_IMODEL_H_
#define VPVL2_IMODEL_H_

#include "vpvl2/Common.h"

class btDiscreteDynamicsWorld;

namespace vpvl2
{

class IBone;
class ILabel;
class IMaterial;
class IMorph;
class IString;
class IVertex;

/**
 * モデルをあらわすインターフェースです。
 *
 */
class VPVL2_API IModel
{
public:
    struct IBuffer {
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
        virtual ~IBuffer() {}
        virtual size_t size() const = 0;
        virtual size_t strideOffset(StrideType type) const = 0;
        virtual size_t strideSize() const = 0;
        virtual const void *ident() const = 0;
    };
    struct IDynamicVertexBuffer : IBuffer {
        virtual void update(void *address,
                            const Vector3 &cameraPosition,
                            Vector3 &aabbMin,
                            Vector3 &aabbMax) const = 0;
        virtual void setSkinningEnable(bool value) = 0;
    };
    struct IStaticVertexBuffer : IBuffer {
        virtual void update(void *address) const = 0;
    };
    struct IIndexBuffer : IBuffer {
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
    struct IMatrixBuffer {
        virtual ~IMatrixBuffer() {}
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
        kUnknown,
        kAsset,
        kPMD,
        kPMX,
        kMaxModelType
    };

    virtual ~IModel() {}

    /**
     * モデルの型を返します。
     *
     * @return Type
     */
    virtual Type type() const = 0;

    /**
     * モデル名(日本語)を返します。
     *
     * @return IString
     */
    virtual const IString *name() const = 0;

    /**
     * モデル名(英語)を返します。
     *
     * @return IString
     */
    virtual const IString *englishName() const = 0;

    /**
     * モデルの説明(日本語)を返します。
     *
     * @return IString
     */
    virtual const IString *comment() const = 0;

    /**
     * モデルの説明(英語)を返します。
     *
     * @return IString
     */
    virtual const IString *englishComment() const = 0;

    /**
     * モデルが可視であるかどうかを返します。
     *
     * @return bool
     */
    virtual bool isVisible() const = 0;

    /**
     * エラーを返します。
     *
     * IModel::load(data, size) が false を返した場合、error は kNoError 以外の値を返します。
     * true を返した場合は常に error は kNoError を返します。
     *
     * @return Error
     */
    virtual ErrorType error() const = 0;

    /**
     * オンメモリ上にある data とその長さ size に基づいてモデルを構築します。
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
     * オンメモリ上にある data に IModel のインスタンスに基づいてデータを書き込みます。
     *
     * data の長さは IModel::estimateSize() が返す値を利用してください。
     * type が kPMD または kPMX の場合は完全なモデルデータを構築します。kAsset の場合は何もしません。
     *
     * @param data
     */
    virtual void save(uint8_t *data) const = 0;

    /**
     * IModel::save(data) に必要なデータの長さを返します。
     *
     * これは save を呼ぶ前に save に渡すデータをメモリ上に確保する量を求めるときに使います。
     * save と併せて使用する必要があります。
     *
     * @return size_t
     */
    virtual size_t estimateSize() const = 0;

    /**
     * モデルの全ての頂点の位置を初期状態にリセットします。
     *
     */
    virtual void resetVertices() = 0;

    virtual void resetMotionState() = 0;

    /**
     * モデルの変形を実行します。
     *
     * @param Vector3
     */
    virtual void performUpdate() = 0;

    /**
     * モデルの物理演算を有効にします。
     *
     * すでに joinWorld が呼ばれていて leaveWorld していない場合は何もしません。
     *
     * @param btDiscreteDynamicsWorld
     */
    virtual void joinWorld(btDiscreteDynamicsWorld *world) = 0;

    /**
     * モデルの物理演算を無効にします。
     *
     * モデルインスタンスの破壊前に leaveWorld を呼ばなかった場合は自動的に leaveWorld が呼ばれます。
     * そのため、モデルインスタンスを破壊する前に btDiscreteDynamicsWorld のインスタンスを解放し、
     * かつ leaveWorld していない場合モデルインスタンス破壊時にクラッシュします。
     *
     * すでに leaveWorld が呼ばれていて joinWorld していない場合は何もしません。
     *
     * @param btDiscreteDynamicsWorld
     */
    virtual void leaveWorld(btDiscreteDynamicsWorld *world) = 0;

    /**
     * ボーン名から IBone のインスタンスを返します。
     *
     * 該当するボーン名の IBone インスタンスが見つかった場合はそれを返します。
     * 見つからなかった場合は null を返します。
     *
     * @param IString
     * @return IBone
     */
    virtual IBone *findBone(const IString *value) const = 0;

    /**
     * ボーン名から IMorph のインスタンスを返します。
     *
     * 該当するボーン名の IMorph インスタンスが見つかった場合はそれを返します。
     * 見つからなかった場合は null を返します。
     *
     * @param IString
     * @return IMorph
     */
    virtual IMorph *findMorph(const IString *value) const = 0;

    virtual int count(ObjectType value) const = 0;
    virtual void getBoneRefs(Array<IBone *> &value) const = 0;
    virtual void getLabelRefs(Array<ILabel *> &value) const = 0;
    virtual void getMaterialRefs(Array<IMaterial *> &value) const = 0;
    virtual void getMorphRefs(Array<IMorph *> &value) const = 0;
    virtual void getVertexRefs(Array<IVertex *> &value) const = 0;
    virtual float edgeScaleFactor(const Vector3 &position) const = 0;
    virtual const Vector3 &position() const = 0;
    virtual const Quaternion &rotation() const = 0;
    virtual const Scalar &opacity() const = 0;
    virtual const Scalar &scaleFactor() const = 0;
    virtual const Vector3 &edgeColor() const = 0;
    virtual const Scalar &edgeWidth() const = 0;
    virtual IModel *parentModel() const = 0;
    virtual IBone *parentBone() const = 0;
    virtual void setName(const IString *value) = 0;
    virtual void setEnglishName(const IString *value) = 0;
    virtual void setComment(const IString *value) = 0;
    virtual void setEnglishComment(const IString *value) = 0;
    virtual void setPosition(const Vector3 &value) = 0;
    virtual void setRotation(const Quaternion &value) = 0;
    virtual void setOpacity(const Scalar &value) = 0;
    virtual void setScaleFactor(const Scalar &value) = 0;
    virtual void setEdgeColor(const Vector3 &value) = 0;
    virtual void setEdgeWidth(const Scalar &value) = 0;
    virtual void setParentModel(IModel *value) = 0;
    virtual void setParentBone(IBone *value) = 0;
    virtual void setVisible(bool value) = 0;

    virtual void getIndexBuffer(IIndexBuffer *&indexBuffer) const = 0;
    virtual void getStaticVertexBuffer(IStaticVertexBuffer *&staticBuffer) const = 0;
    virtual void getDynamicVertexBuffer(IDynamicVertexBuffer *&dynamicBuffer,
                                        const IIndexBuffer *indexBuffer) const = 0;
    virtual void getMatrixBuffer(IMatrixBuffer *&matrixBuffer,
                                 IDynamicVertexBuffer *dynamicBuffer,
                                 const IIndexBuffer *indexBuffer) const = 0;
};

} /* namespace vpvl2 */

#endif
