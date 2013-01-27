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
#ifndef VPVL2_CG_EFFECT_H_
#define VPVL2_CG_EFFECT_H_

#include "vpvl2/Common.h"
#include "vpvl2/IEffect.h"
#include "vpvl2/IRenderContext.h"
#include "vpvl2/extensions/cg/Util.h"
#include "vpvl2/extensions/gl/FrameBufferObject.h"

#include <Cg/cg.h>
#include <Cg/cgGL.h>

namespace vpvl2
{
namespace cg
{
using namespace extensions::cg;

class Effect : public IEffect {
public:
    static bool isInteractiveParameter(CGparameter value) {
        CGannotation name = cgGetNamedParameterAnnotation(value, "UIName");
        CGannotation widget = cgGetNamedParameterAnnotation(value, "UIWidget");
        return cgIsAnnotation(name) && cgIsAnnotation(widget);
    }

    Effect(IRenderContext *renderContext, CGcontext context, CGeffect effect)
        : m_renderContextRef(renderContext),
          m_contextRef(context),
          m_effect(effect),
          m_parentEffectRef(0),
          m_parentFrameBufferObject(0),
          m_scriptOrderType(kStandard)
    {
    }
    ~Effect() {
        delete m_parentFrameBufferObject;
        m_parentFrameBufferObject = 0;
        cgDestroyEffect(m_effect);
        m_effect = 0;
        m_renderContextRef = 0;
        m_contextRef = 0;
        m_parentEffectRef = 0;
        m_scriptOrderType = kStandard;
    }

    void createFrameBufferObject() {
        m_parentFrameBufferObject = m_renderContextRef->createFrameBufferObject();
    }
    void addOffscreenRenderTarget(FrameBufferObject::AbstractTexture *textureRef,
                                  CGparameter textureParameter,
                                  CGparameter samplerParameter)
    {
        OffscreenRenderTarget target;
        target.textureRef = textureRef;
        target.textureParameter = textureParameter;
        target.samplerParameter = samplerParameter;
        m_offscreenRenderTargets.append(target);
    }
    void addInteractiveParameter(CGparameter value) {
        m_interactiveParameters.append(value);
    }

    const btAlignedObjectArray<GLuint> renderColorTargetIndices() const {
        return m_renderColorTargets;
    }
    void addRenderColorTargetIndex(const GLenum targetIndex) {
        m_renderColorTargets.push_back(targetIndex);
        Util::setRenderColorTargets(&m_renderColorTargets[0], m_renderColorTargets.size());
    }
    void removeRenderColorTargetIndex(const GLenum targetIndex) {
        m_renderColorTargets.remove(targetIndex);
        Util::setRenderColorTargets(&m_renderColorTargets[0], m_renderColorTargets.size());
    }
    void clearRenderColorTargetIndices() {
        m_renderColorTargets.clear();
    }
    void inheritRenderColorTargetIndices(const Effect *sourceEffect) {
        m_renderColorTargets.copyFromArray(sourceEffect->m_renderColorTargets);
    }
    bool hasRenderColorTargetIndex(const GLenum targetIndex) {
        return m_renderColorTargets.findLinearSearch(targetIndex) != m_renderColorTargets.size();
    }

    void *internalContext() const { return m_contextRef; }
    void *internalPointer() const { return m_effect; }
    void getOffscreenRenderTargets(Array<OffscreenRenderTarget> &value) const {
        value.copy(m_offscreenRenderTargets);
    }
    void getInteractiveParameters(Array<void *> &value) const {
        value.copy(m_interactiveParameters);
    }
    IEffect *parentEffectRef() const { return m_parentEffectRef; }
    void setParentEffectRef(IEffect *value) { m_parentEffectRef = value; }
    FrameBufferObject *parentFrameBufferObject() const { return m_parentFrameBufferObject; }
    ScriptOrderType scriptOrderType() const { return m_scriptOrderType; }
    void setScriptOrderType(ScriptOrderType value) { m_scriptOrderType = value; }

private:
    IRenderContext *m_renderContextRef;
    CGcontext m_contextRef;
    CGeffect m_effect;
    btAlignedObjectArray<GLuint> m_renderColorTargets;
    Array<OffscreenRenderTarget> m_offscreenRenderTargets;
    Array<void *> m_interactiveParameters;
    IEffect *m_parentEffectRef;
    FrameBufferObject *m_parentFrameBufferObject;
    ScriptOrderType m_scriptOrderType;

    VPVL2_DISABLE_COPY_AND_ASSIGN(Effect)
};

} /* namespace cg */
} /* namespace vpvl2 */

#endif
