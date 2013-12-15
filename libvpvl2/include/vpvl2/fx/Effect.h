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
#ifndef VPVL2_FX_EFFECT_H_
#define VPVL2_FX_EFFECT_H_

#include <vpvl2/IEffect.h>

/* This class no longer will be used by nvFX */

namespace vpvl2
{
namespace fx
{

class VPVL2_API Effect VPVL2_DECL_FINAL : IEffect {
public:
    Effect();
    ~Effect();

    void load(const char *path);

    void *internalContext() const;
    void *internalPointer() const;
    void getOffscreenRenderTargets(Array<OffscreenRenderTarget> &value) const;
    void getInteractiveParameters(Array<IEffect::Parameter *> &value) const;
    IEffect *parentEffectRef() const;
    void setParentEffectRef(IEffect *value);
    gl::FrameBufferObject *parentFrameBufferObject() const;
    void createFrameBufferObject();
    ScriptOrderType scriptOrderType() const;
    void addOffscreenRenderTarget(ITexture *textureRef, Parameter *textureParameterRef, Parameter *samplerParameterRef);
    void addInteractiveParameter(IEffect::Parameter *value);
    void addRenderColorTargetIndex(int targetIndex);
    void removeRenderColorTargetIndex(int targetIndex);
    void clearRenderColorTargetIndices();
    void setScriptOrderType(ScriptOrderType value);
    bool hasRenderColorTargetIndex(int targetIndex) const;
    IEffect::Parameter *findUniformParameter(const char *name) const;
    IEffect::Technique *findTechnique(const char *name) const;
    void getParameterRefs(Array<Parameter *> &parameters) const;
    void getTechniqueRefs(Array<Technique *> &techniques) const;
    void setVertexAttributePointer(VertexAttributeType vtype, Parameter::Type ptype, vsize stride, const void *ptr);
    void activateVertexAttribute(VertexAttributeType vtype);

private:
    struct PrivateContext;
    PrivateContext *m_context;
};

} /* namespace fx */
} /* namespace vpvl2 */

#endif
