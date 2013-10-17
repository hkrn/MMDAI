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
#ifndef VPVL2_IRENDERENGINE_H_
#define VPVL2_IRENDERENGINE_H_

#include "vpvl2/IEffect.h"

namespace vpvl2
{

class IEffect;
class IModel;
class IString;

class VPVL2_API IRenderEngine
{
public:
    enum UpdateOptionFlags {
        kNone = 0,
        kParallelUpdate = 1
    };

    virtual ~IRenderEngine() {}

    /**
     * レンダリングエンジンに紐付けられている IModel のインスタンスを返します.
     *
     * @brief parentModelRef
     * @return
     */
    virtual IModel *parentModelRef() const = 0;

    /**
     * 使用しているグラフィック API に対して必要なリソースを確保してアップロードします.
     *
     * インスタンスに一度だけ呼び出しを行う必要があります。
     * データを変更して再度アップロードする場合は一旦インスタンスを破棄して
     * 再度インスタンスを作成してからアップロードする必要があります。
     *
     * @brief upload
     * @param userData
     * @return
     */
    virtual bool upload(void *userData) = 0;

    /**
     * モデルを描画します.
     *
     * @brief renderModel
     */
    virtual void renderModel() = 0;

    /**
     * モデルのエッジを描画します.
     *
     * @brief renderEdge
     */
    virtual void renderEdge() = 0;

    /**
     * モデルの投影影を描写します.
     *
     * @brief renderShadow
     */
    virtual void renderShadow() = 0;

    /**
     * セルフシャドウ用の深度テクスチャ向けの描画をします.
     *
     * @brief renderZPlot
     */
    virtual void renderZPlot() = 0;

    /**
     * モデルのバッファをアップデートします.
     *
     * 現在のモデルからグラフィック API における頂点バッファの更新を行うのみのため、モデルの変形が必要な場合は
     * 紐付けられているモデルに対して IModel#performUpdate を呼び出す必要があります。
     *
     * @brief update
     */
    virtual void update() = 0;

    /**
     * IRenderEngine#update におけるオプションを設定します.
     *
     * @brief setUpdateOptions
     * @param options
     */
    virtual void setUpdateOptions(int options) = 0;

    /**
     * エフェクト向けのプリプロセスが存在するかどうかを返します.
     *
     * @brief hasPreProcess
     * @return
     */
    virtual bool hasPreProcess() const = 0;

    /**
     * エフェクト向けのポストプロセスが存在するかどうかを返します.
     *
     * @brief hasPostProcess
     * @return
     */
    virtual bool hasPostProcess() const = 0;

    /**
     * ポストエフェクト向けの前準備処理を行います.
     *
     * このメソッドは performPreProcess を呼び出す前に実行する必要があります。
     *
     * @brief preparePostProcess
     */
    virtual void preparePostProcess() = 0;

    /**
     * プリエフェクトの処理を行います.
     *
     * @brief performPreProcess
     */
    virtual void performPreProcess() = 0;

    /**
     * ポストエフェクトの処理を行います.
     *
     * nextPostEffect には次に行う必要があるポストエフェクトを渡します。
     * 終端の場合は NULL を設定する必要があります。
     * このメソッドを実行する際は予め preparePostProcess を呼び出されなければなりません。
     *
     * @brief performPostProcess
     * @param nextPostEffect
     */
    virtual void performPostProcess(IEffect *nextPostEffect) = 0;

    /**
     * ScriptOrderType に紐付けられている IEffect のインスタンスを返します.
     *
     * @brief effect
     * @param type
     * @return
     */
    virtual IEffect *effectRef(IEffect::ScriptOrderType type) const = 0;

    /**
     * ScriptOrderType に対してエフェクトを設定します.
     *
     * @brief setEffect
     * @param effect
     * @param type
     * @param userData
     */
    virtual void setEffect(IEffect *effectRef, IEffect::ScriptOrderType type, void *userData) = 0;

    virtual void setOverridePass(IEffect::Pass *pass) = 0;

    /**
     * モデルが可視かどうかを判定します.
     *
     * このメソッドは GL_ARB_occulusion_query2 か GL_ARB_occulusion_query が必要です。
     * いずれも存在しない場合は常に true を返します（常時可視と判定）。
     * エンジンによっては実装していないことがあり、その場合は常に可視を返すようになっています。
     *
     * 内部的に renderEdge を呼び出しているため高コストな処理ですが、編集モード時にフレーム移動の際に
     * 可視かどうかを判定するために使用し、モーションに可視状態を保存させることを想定して実装しています。
     *
     * @brief testVisible
     * @return
     */
    virtual bool testVisible() = 0;
};

} /* namespace vpvl2 */

#endif
