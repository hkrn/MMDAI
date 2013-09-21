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
#ifndef VPVL2_NVFX_EFFECT_H_
#define VPVL2_NVFX_EFFECT_H_

#include "vpvl2/IEffect.h"

namespace nvFX {
class IAnnotation;
class IContainer;
class IUniform;
class ITechnique;
}

namespace vpvl2
{

class IApplicationContext;

namespace nvfx
{

class EffectContext;

class Effect : public IEffect {
public:
    static bool isInteractiveParameter(const Parameter *value);

    Effect(EffectContext *contextRef, IApplicationContext *applicationContextRef, nvFX::IContainer *container);
    ~Effect();

    void createFrameBufferObject();
    void addOffscreenRenderTarget(ITexture *textureRef, IEffect::Parameter *textureParameterRef, IEffect::Parameter *samplerParameterRef);
    void addInteractiveParameter(IEffect::Parameter *value);

    void addRenderColorTargetIndex(int targetIndex);
    void removeRenderColorTargetIndex(int targetIndex);
    void clearRenderColorTargetIndices();
    bool hasRenderColorTargetIndex(int targetIndex) const;

    void *internalContext() const;
    void *internalPointer() const;
    void getOffscreenRenderTargets(Array<OffscreenRenderTarget> &value) const;
    void getInteractiveParameters(Array<Parameter *> &value) const;
    IEffect *parentEffectRef() const;
    void setParentEffectRef(IEffect *value);
    extensions::gl::FrameBufferObject *parentFrameBufferObject() const;
    ScriptOrderType scriptOrderType() const;
    void setScriptOrderType(ScriptOrderType value);
    IEffect::Parameter *findVaryingParameter(const char *name) const;
    IEffect::Parameter *findUniformParameter(const char *name) const;
    IEffect::Technique *findTechnique(const char *name) const;
    void getParameterRefs(Array<Parameter *> &parameters) const;
    void getTechniqueRefs(Array<Technique *> &techniques) const;
    void setVertexAttributePointer(VertexAttributeType vtype, Parameter::Type ptype, vsize stride, const void *ptr);
    void activateVertexAttribute(VertexAttributeType vtype);

private:
    struct NvFXParameter;
    struct NvFXTechnique;
    struct NvFXPass;
    struct NvFXSamplerState;
    struct NvFXAnnotation;
    Annotation *cacheAnnotationRef(nvFX::IAnnotation *annotation, const char *name) const;
    Parameter *cacheParameterRef(nvFX::IUniform *parameter) const;
    Technique *cacheTechniqueRef(nvFX::ITechnique *technique) const;

    typedef PointerHash<HashString, NvFXAnnotation> NvFXAnnotationHash;
    mutable PointerArray<NvFXParameter> m_parameters;
    mutable PointerArray<NvFXTechnique> m_techniques;
    mutable PointerHash<HashPtr, NvFXAnnotationHash> m_annotationRefsHash;
    mutable Hash<HashPtr, NvFXParameter *> m_parameterRefsHash;
    mutable Hash<HashPtr, NvFXTechnique *> m_techniqueRefsHash;
    IApplicationContext *m_applicationContextRef;
    EffectContext *m_effectContextRef;
    nvFX::IContainer *m_container;
    Array<uint32> m_renderColorTargetIndices;
    Array<OffscreenRenderTarget> m_offscreenRenderTargets;
    Array<IEffect::Parameter *> m_interactiveParameters;
    IEffect *m_parentEffectRef;
    extensions::gl::FrameBufferObject *m_parentFrameBufferObject;
    ScriptOrderType m_scriptOrderType;

    VPVL2_DISABLE_COPY_AND_ASSIGN(Effect)
};

} /* namespace nvfx */
} /* namespace vpvl2 */

#endif
