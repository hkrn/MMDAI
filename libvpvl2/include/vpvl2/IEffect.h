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

#pragma once
#ifndef VPVL2_IEFFECT_H_
#define VPVL2_IEFFECT_H_

#include "vpvl2/Common.h"

namespace vpvl2
{

namespace extensions
{
namespace gl
{
class FrameBufferObject;
}
}

using namespace extensions::gl;

class IEffect
{
public:
    enum ScriptOrderType {
        kPreProcess,
        kStandard,
        kStandardOffscreen,
        kPostProcess,
        kAutoDetection,
        kMaxScriptOrderType
    };
    struct OffscreenRenderTarget {
        void *textureParameter;
        void *samplerParameter;
        size_t width;
        size_t height;
        int format;
    };
    virtual ~IEffect() {}

    /**
     * CGcontext のインスタンスを返します.
     *
     * void* で返すため、static_cast で CGcontext にキャストする必要があります。
     *
     * @brief internalContext
     * @return
     */
    virtual void *internalContext() const = 0;

    /**
     * CGeffect のインスタンスを返します.
     *
     * void* で返すため、static_cast で CGeffect にキャストする必要があります。
     *
     * @brief internalPointer
     * @return
     */
    virtual void *internalPointer() const = 0;

    /**
     * オフスクリーンターゲットの配列を取得します.
     *
     * @brief getOffscreenRenderTargets
     * @param value
     */
    virtual void getOffscreenRenderTargets(Array<OffscreenRenderTarget> &value) const = 0;

    /**
     * UIWidget セマンティクスがあるパラメータの配列を返します.
     *
     * void* で返すため、個々に static_cast で CGparameter にキャストする必要があります。
     * 現在実装していないため機能していません。
     *
     * @brief getInteractiveParameters
     * @param value
     */
    virtual void getInteractiveParameters(Array<void *> &value) const = 0;

    /**
     * 親の IEffect インスタンスのポインタ参照を返します.
     *
     * @brief parentEffect
     * @return
     */
    virtual IEffect *parentEffect() const = 0;

    /**
     * 親の IEffect インスタンスのポインタ参照を設定します.
     *
     * @brief parentEffect
     * @param value
     */
    virtual void setParentEffect(IEffect *value) = 0;

    /**
     * 親のフレームバッファの参照を返します.
     *
     * @brief parentFrameBufferObject
     * @return
     */
    virtual FrameBufferObject *parentFrameBufferObject() const = 0;

    /**
     * スクリプトの描画タイミングを返します.
     *
     * @brief scriptOrderType
     * @return
     */
    virtual ScriptOrderType scriptOrderType() const = 0;
};

} /* namespace vpvl2 */

#endif
