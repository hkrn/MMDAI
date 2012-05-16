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
class IMorph;
class IString;

/**
 * モデルをあらわすインターフェースです。
 *
 */
class VPVL2_API IModel
{
public:
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
        kInvalidLabelsError,
        kInvalidRigidBodiesError,
        kInvalidJointsError,
        kMaxErrors
    };
    enum Object {
        kBone,
        kIK,
        kIndex,
        kJoint,
        kMaterial,
        kMorph,
        kRigidBody,
        kVertex
    };
    enum Type {
        kAsset,
        kPMD,
        kPMX
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
    virtual Error error() const = 0;

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

    /**
     * モデルの変形を実行します。
     *
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

    virtual int count(Object value) const = 0;
    virtual void getBones(Array<IBone *> &value) const = 0;
    virtual void getMorphs(Array<IMorph *> &value) const = 0;
    virtual void getLabels(Array<ILabel *> &value) const = 0;

    /**
     * モデルのバウンディングボックスの大きさを取得します。
     *
     * 返す値に position() の付加はないため、取得する側で補正する必要があります。
     *
     * @param Vector3
     * @param Vector3
     * @sa getBoundingSphere
     */
    virtual void getBoundingBox(Vector3 &min, Vector3 &max) const = 0;

    /**
     * モデルのバウンディングスフィアの大きさを取得します。
     *
     * type() が返す値が kPMD または kPMX の場合は「センター」ボーンを中心としたバウンディングスフィアの
     * 大きさを算出します。kAsset かあるいは「センター」ボーンがない場合は getBoundingBox() を用いて
     * バウンディングボックスが収まるバウンディングスフィアの大きさを算出します。
     *
     * 返す値に position() の付加はないため、取得する側で補正する必要があります。
     *
     * @param Vector3
     * @param Scalar
     * @sa getBoundingBox
     */
    virtual void getBoundingSphere(Vector3 &center, Scalar &radius) const = 0;

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
};

} /* namespace vpvl2 */

#endif
