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
#include "vpvl2/extensions/cg/Util.h"

#ifdef __APPLE__
#include <cg.h>
#include <cgGL.h>
#else /* __APPLE__ */
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#endif /* __APPLE__ */

namespace vpvl2
{

class IRenderContext;

namespace cg
{

class EffectContext;

class Effect : public IEffect {
public:
    static bool isInteractiveParameter(const IParameter *value);

    Effect(EffectContext *contextRef, IRenderContext *renderContext, CGeffect effect);
    ~Effect();

    void createFrameBufferObject();
    void addOffscreenRenderTarget(ITexture *textureRef, IEffect::IParameter *textureParameterRef, IEffect::IParameter *samplerParameterRef);
    void addInteractiveParameter(IEffect::IParameter *value);

    void addRenderColorTargetIndex(int targetIndex);
    void removeRenderColorTargetIndex(int targetIndex);
    void clearRenderColorTargetIndices();
    bool hasRenderColorTargetIndex(int targetIndex) const;

    void *internalContext() const;
    void *internalPointer() const;
    void getOffscreenRenderTargets(Array<OffscreenRenderTarget> &value) const;
    void getInteractiveParameters(Array<IParameter *> &value) const;
    IEffect *parentEffectRef() const;
    void setParentEffectRef(IEffect *value);
    extensions::gl::FrameBufferObject *parentFrameBufferObject() const;
    ScriptOrderType scriptOrderType() const;
    void setScriptOrderType(ScriptOrderType value);
    IEffect::IParameter *findVaryingParameter(const char *name) const;
    IEffect::IParameter *findUniformParameter(const char *name) const;
    IEffect::ITechnique *findTechnique(const char *name) const;
    void getParameterRefs(Array<IParameter *> &parameters) const;
    void getTechniqueRefs(Array<ITechnique *> &techniques) const;

private:
    struct Parameter;
    struct Technique;
    struct Pass;
    struct SamplerState;
    struct Annotation;
    IAnnotation *cacheAnnotationRef(CGannotation annotation) const;
    IParameter *cacheParameterRef(CGparameter parameter) const;
    ITechnique *cacheTechniqueRef(CGtechnique technique) const;

    mutable PointerArray<Annotation> m_annotations;
    mutable PointerArray<Parameter> m_parameters;
    mutable PointerArray<Technique> m_techniques;
    mutable Hash<HashPtr, Annotation *> m_annotationRefsHash;
    mutable Hash<HashPtr, Parameter *> m_parameterRefsHash;
    mutable Hash<HashPtr, Technique *> m_techniqueRefsHash;
    IRenderContext *m_renderContextRef;
    EffectContext *m_effectContextRef;
    CGeffect m_effect;
    Array<GLenum> m_renderColorTargetIndices;
    Array<OffscreenRenderTarget> m_offscreenRenderTargets;
    Array<IEffect::IParameter *> m_interactiveParameters;
    IEffect *m_parentEffectRef;
    extensions::gl::FrameBufferObject *m_parentFrameBufferObject;
    ScriptOrderType m_scriptOrderType;

    VPVL2_DISABLE_COPY_AND_ASSIGN(Effect)
};

} /* namespace cg */
} /* namespace vpvl2 */

#endif
