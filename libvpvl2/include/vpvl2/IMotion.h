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

#ifndef VPVL2_IMOTION_H_
#define VPVL2_IMOTION_H_

#include "vpvl2/Common.h"

namespace vpvl2
{

class IKeyframe;
class IModel;
class IString;

/**
 * モーションをあらわすインターフェースです。
 *
 */
class VPVL2_API IMotion
{
public:
    virtual ~IMotion() {}

    /**
     * オンメモリ上にある data とその長さ size に基づいてモーションを構築します。
     *
     * モデルの読み込みに成功した場合は true を、失敗した場合は false を返し、
     * IMotion::error() が kNoError 以外の値を返すようになります。
     *
     * load は複数回呼んでも IMotion では正しく処理されますが、IMotion に依存する
     * クラスが正しく処理されなくなるので、あまり推奨しません。
     *
     * @param data
     * @param size
     * @return bool
     */
    virtual bool load(const uint8_t *data, size_t size) = 0;

    /**
     * オンメモリ上にある data に IMotion のインスタンスに基づいてデータを書き込みます。
     *
     * data の長さは IMotion::estimateSize() が返す値を利用してください。
     *
     * @param data
     */
    virtual void save(uint8_t *data) const = 0;

    /**
     * IMotion::save(data) に必要なデータの長さを返します。
     *
     * これは save を呼ぶ前に save に渡すデータをメモリ上に確保する量を求めるときに使います。
     * save と併せて使用する必要があります。
     *
     * @return size_t
     */
    virtual size_t estimateSize() const = 0;

    /**
     * モーションが依存するモデルのポインタを返します。
     *
     * ボーンとモーフのモーションは原則非 null を返します。
     * カメラと照明のモーションはモデルに依存しないため、null を返します。
     *
     * @return IModel
     */
    virtual IModel *parentModel() const = 0;

    /**
     * モーションが依存するモデルを設定します。
     *
     * null の設定も可能です。
     *
     * @param IModel
     */
    virtual void setParentModel(IModel *model) = 0;

    /**
     * モーションを指定されたフレームの位置に移動します。
     *
     * 内部的にはフレームの位置が保存されています。
     *
     * @param float
     */
    virtual void seek(float frameIndex) = 0;

    /**
     * モーションを指定されたフレームの位置分進めます。
     *
     * 前回 advance または seek を呼んだ位置から引数の値を加算してフレームを進めます。
     * 内部的には IMotion::seek() を呼び出しています。
     *
     * @param float
     */
    virtual void advance(float delta) = 0;

    /**
     * モーションを最初の位置にリセットします。
     *
     */
    virtual void reset() = 0;

    /**
     * モーションの一番後ろのフレームの位置を返します。
     *
     * @return float
     */
    virtual float maxFrameIndex() const = 0;

    /**
     * モーションが指定されたフレームの位置まで進んでいるかを返します。
     *
     * @return bool
     */
    virtual bool isReachedTo(float frameIndex) const = 0;

    /**
     * キーフレームを追加します。
     *
     * @param IKeyframe
     */
    virtual void addKeyframe(IKeyframe *value) = 0;

    /**
     * キーフレームを削除します。
     *
     * キーフレームを物理的に削除するため、呼び出し後引数の値は無効になります。
     *
     * @param IKeyframe
     */
    virtual void deleteKeyframe(IKeyframe *value) = 0;
};

}

#endif

