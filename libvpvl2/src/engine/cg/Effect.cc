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

#include "vpvl2/vpvl2.h"
#include "vpvl2/cg/Effect.h"
#include "vpvl2/cg/EffectEngine.h"

namespace vpvl2
{
namespace cg
{
using namespace extensions::cg;

struct Effect::Parameter : IEffect::IParameter {
    Parameter(const Effect *e, CGparameter p)
        : effect(e),
          parameter(p)
    {
    }
    ~Parameter() {
    }

    IEffect *parentEffectRef() const {
        return const_cast<Effect *>(effect);
    }
    const IEffect::IAnnotation *annotationRef(const char *name) const {
        return 0;
    }
    const char *name() const {
        return cgGetParameterName(parameter);
    }
    const char *semantic() const {
        return cgGetParameterSemantic(parameter);
    }
    Type type() const {
        switch (cgGetParameterType(parameter)) {
        }
        return kUnknown;
    }
    void connect(IParameter *destinationParameter) {
        Parameter *p = static_cast<Parameter *>(destinationParameter);
        cgConnectParameter(parameter, p->parameter);
    }
    void reset() {
        cgDisconnectParameter(parameter);
        parameter = 0;
    }
    void getValue(float &value) const {
        float v = 0;
        cgGLGetParameter1f(parameter, &v);
        value = v;
    }
    void getArrayDimension(int &value) const {
        value = cgGetArrayDimension(parameter);
    }
    void getArrayTotalSize(int &value) const {
        value = cgGetArrayTotalSize(parameter);
    }
    void getTextureRef(intptr_t &value) const {
        value = cgGLGetTextureParameter(parameter);
    }
    void getStateRefs(Array<IEffect::IState *> &value) const {
    }
    void setValue(bool value) {
        cgGLSetParameter1f(parameter, value ? 1 : 0);
    }
    void setValue(int value) {
        cgGLSetParameter1f(parameter, float(value));
    }
    void setValue(float value) {
        cgGLSetParameter1f(parameter, value);
    }
    void setValue(const Vector3 &value) {
        cgGLSetParameter3fv(parameter, value);
    }
    void setValue(const Vector4 &value) {
        cgGLSetParameter4fv(parameter, value);
    }
    void setValue(const Vector4 *value) {
    }
    void setMatrix(const float *value) {
        cgSetMatrixParameterfr(parameter, value);
    }
    void setSampler(const ITexture *value) {
        cgGLSetupSampler(parameter, value ? static_cast<GLuint>(value->data()) : 0);
    }
    void setTexture(const ITexture *value) {
        cgGLSetTextureParameter(parameter, value ? static_cast<GLuint>(value->data()) : 0);
    }

    const Effect *effect;
    CGparameter parameter;
};

struct Effect::Technique : IEffect::ITechnique {
    Technique(const Effect *e, CGtechnique p)
        : effect(e),
          technique(p)
    {
    }
    ~Technique() {
    }

    IEffect *parentEffectRef() const {
        return const_cast<Effect *>(effect);
    }
    IEffect::IPass *findPass(const char *name) const {
        return 0;
    }
    const IEffect::IAnnotation *annotationRef(const char *name) const {
        return 0;
    }
    void getPasses(Array<IEffect::IPass *> &passes) const {
    }

    const Effect *effect;
    CGtechnique technique;
};

bool Effect::isInteractiveParameter(const IParameter *value)
{
    const IEffect::IAnnotation *name = value->annotationRef("UIName");
    const IEffect::IAnnotation *widget = value->annotationRef("UIWidget");
    return name && widget;
}

Effect::Effect(EffectContext *contextRef, IRenderContext *renderContext, CGeffect effect)
    : m_renderContextRef(renderContext),
      m_effectContextRef(contextRef),
      m_effect(effect),
      m_parentEffectRef(0),
      m_parentFrameBufferObject(0),
      m_scriptOrderType(kStandard)
{
}

Effect::~Effect()
{
    m_techniques.releaseAll();
    m_parameters.releaseAll();
    delete m_parentFrameBufferObject;
    m_parentFrameBufferObject = 0;
    cgDestroyEffect(m_effect);
    m_effect = 0;
    m_renderContextRef = 0;
    m_effectContextRef = 0;
    m_parentEffectRef = 0;
    m_scriptOrderType = kStandard;
}

void Effect::createFrameBufferObject()
{
    delete m_parentFrameBufferObject;
    m_parentFrameBufferObject = m_renderContextRef->createFrameBufferObject();
}

void Effect::addOffscreenRenderTarget(ITexture *textureRef, IEffect::IParameter *textureParameterRef, IEffect::IParameter *samplerParameterRef)
{
    OffscreenRenderTarget target;
    target.textureRef = textureRef;
    target.textureParameterRef = textureParameterRef;
    target.samplerParameterRef = samplerParameterRef;
    m_offscreenRenderTargets.append(target);
}

void Effect::addInteractiveParameter(IParameter *value)
{
    if (isInteractiveParameter(value)) {
        m_interactiveParameters.append(value);
    }
}

const btAlignedObjectArray<GLuint> Effect::renderColorTargetIndices() const
{
    return m_renderColorTargets;
}

void Effect::addRenderColorTargetIndex(const GLenum targetIndex)
{
    m_renderColorTargets.push_back(targetIndex);
    Util::setRenderColorTargets(&m_renderColorTargets[0], m_renderColorTargets.size());
}

void Effect::removeRenderColorTargetIndex(const GLenum targetIndex)
{
    m_renderColorTargets.remove(targetIndex);
    Util::setRenderColorTargets(&m_renderColorTargets[0], m_renderColorTargets.size());
}

void Effect::clearRenderColorTargetIndices()
{
    m_renderColorTargets.clear();
}

void Effect::inheritRenderColorTargetIndices(const Effect *sourceEffect)
{
    m_renderColorTargets.copyFromArray(sourceEffect->m_renderColorTargets);
}

bool Effect::hasRenderColorTargetIndex(const GLenum targetIndex)
{
    return m_renderColorTargets.findLinearSearch(targetIndex) != m_renderColorTargets.size();
}

void *Effect::internalContext() const
{
    return m_effectContextRef->internalContext();
}

void *Effect::internalPointer() const
{
    return m_effect;
}

void Effect::getOffscreenRenderTargets(Array<OffscreenRenderTarget> &value) const
{
    value.copy(m_offscreenRenderTargets);
}

void Effect::getInteractiveParameters(Array<IEffect::IParameter *> &value) const
{
    value.copy(m_interactiveParameters);
}

IEffect *Effect::parentEffectRef() const
{
    return m_parentEffectRef;
}

void Effect::setParentEffectRef(IEffect *value)
{
    m_parentEffectRef = value;
}

FrameBufferObject *Effect::parentFrameBufferObject() const
{
    return m_parentFrameBufferObject;
}

IEffect::ScriptOrderType Effect::scriptOrderType() const
{
    return m_scriptOrderType;
}

void Effect::setScriptOrderType(ScriptOrderType value)
{
    m_scriptOrderType = value;
}

IEffect::IParameter *Effect::findParameter(const char *name) const
{
    CGparameter parameter = cgGetNamedEffectParameter(m_effect, name);
    return addParameter(parameter);
}

IEffect::ITechnique *Effect::findTechnique(const char *name) const
{
    CGtechnique technique = cgGetNamedTechnique(m_effect, name);
    return addTechnique(technique);
}

void Effect::getParameterRefs(Array<IParameter *> &parameters) const
{
    CGparameter parameter = cgGetFirstEffectParameter(m_effect);
    while (parameter) {
        parameters.append(addParameter(parameter));
        parameter = cgGetNextParameter(parameter);
    }
}

void Effect::getTechniqueRefs(Array<ITechnique *> &techniques) const
{
    CGtechnique technique = cgGetFirstTechnique(m_effect);
    while (technique) {
        techniques.append(addTechnique(technique));
        technique = cgGetNextTechnique(technique);
    }
}

IEffect::IParameter *Effect::addParameter(CGparameter parameter) const
{
    const char *name = cgGetParameterName(parameter);
    if (Parameter *const *parameterPtr = m_parameterRefsHash.find(name)) {
        return *parameterPtr;
    }
    else {
        Effect::Parameter *newParameterPtr = m_parameters.append(new Effect::Parameter(this, parameter));
        m_parameterRefsHash.insert(cgGetParameterName(parameter), newParameterPtr);
        return newParameterPtr;
    }
}

IEffect::ITechnique *Effect::addTechnique(CGtechnique technique) const
{
    const char *name = cgGetTechniqueName(technique);
    if (Technique *const *techniquePtr = m_techniqueRefsHash.find(name)) {
        return *techniquePtr;
    }
    else {
        Effect::Technique *newTechniquePtr = m_techniques.append(new Effect::Technique(this, technique));
        m_techniqueRefsHash.insert(name, newTechniquePtr);
        return newTechniquePtr;
    }
}

} /* namespace cg */
} /* namespace vpvl2 */
