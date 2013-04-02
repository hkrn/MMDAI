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
#include "vpvl2/cg/EffectContext.h"
#include "vpvl2/cg/EffectEngine.h"

namespace vpvl2
{
namespace cg
{

using namespace extensions::cg;

static IEffect::IParameter::Type toEffectType(CGtype type)
{
    switch (type) {
    case CG_BOOL:
        return IEffect::IParameter::kBoolean;
    case CG_FLOAT:
        return IEffect::IParameter::kFloat;
    case CG_FLOAT3:
        return IEffect::IParameter::kFloat3;
    case CG_FLOAT4:
        return IEffect::IParameter::kFloat4;
    case CG_FLOAT4x4:
        return IEffect::IParameter::kFloat4x4;
    case CG_INT:
        return IEffect::IParameter::kInteger;
    case CG_TEXTURE:
        return IEffect::IParameter::kTexture;
    case CG_SAMPLER2D:
        return IEffect::IParameter::kSampler2D;
    case CG_SAMPLER3D:
        return IEffect::IParameter::kSampler3D;
    case CG_SAMPLERCUBE:
        return IEffect::IParameter::kSamplerCube;
    default:
        return IEffect::IParameter::kUnknown;
    }
}

struct Effect::Annotation : IEffect::IAnnotation {
    static const char *kEmpty;

    Annotation(const Effect *e, CGannotation a)
        : effect(e),
          annotation(a)
    {
    }
    ~Annotation() {
        effect = 0;
        annotation = 0;
    }

    bool booleanValue() const {
        int nvalues;
        const CGbool *values = cgGetBoolAnnotationValues(annotation, &nvalues);
        return nvalues == 1 ? values[0] == CG_TRUE : false;
    }
    int integerValue() const {
        int nvalues;
        const int *values = integerValues(&nvalues);
        return nvalues == 1 ? values[0] : 0;
    }
    const int *integerValues(int *size) const {
        return cgGetIntAnnotationValues(annotation, size);
    }
    float floatValue() const {
        int nvalues;
        const float *values = floatValues(&nvalues);
        return nvalues == 1 ? values[0] : 0;
    }
    const float *floatValues(int *size) const {
        return cgGetFloatAnnotationValues(annotation, size);
    }
    const char *stringValue() const {
        return cgIsAnnotation(annotation) ? cgGetStringAnnotationValue(annotation) : kEmpty;
    }

    const Effect *effect;
    CGannotation annotation;
};

const char *Effect::Annotation::kEmpty = "";

struct Effect::Pass : IEffect::IPass {
    Pass(const Effect *e, const IEffect::ITechnique *t, CGpass a)
        : effect(e),
          technique(t),
          pass(a)
    {
    }
    ~Pass() {
        effect = 0;
        technique = 0;
        pass = 0;
    }

    IEffect::ITechnique *parentTechniqueRef() const {
        return const_cast<IEffect::ITechnique *>(technique);
    }
    const IEffect::IAnnotation *annotationRef(const char *name) const {
        CGannotation annotation = cgGetNamedPassAnnotation(pass, name);
        return effect->cacheAnnotationRef(annotation);
    }
    void setState() {
        cgSetPassState(pass);
    }
    void resetState() {
        cgResetPassState(pass);
    }

    const Effect *effect;
    const IEffect::ITechnique *technique;
    CGpass pass;
};

struct Effect::SamplerState : IEffect::ISamplerState {
    SamplerState(const Effect *e, CGstateassignment sa)
        : effect(e),
          state(cgGetSamplerStateAssignmentState(sa)),
          assignment(sa)
    {
    }
    ~SamplerState() {
        effect = 0;
        state = 0;
        assignment = 0;
    }

    const char *name() const {
        return cgIsState(state) ? cgGetStateName(state) : Effect::Annotation::kEmpty;
    }
    IParameter::Type type() const {
        return toEffectType(cgGetStateType(state));
    }
    IParameter *parameterRef() const {
        CGparameter parameter = cgGetTextureStateAssignmentValue(assignment);
        return effect->cacheParameterRef(parameter);
    }
    void getValue(int &value) const {
        int nvalues;
        const int *values = cgGetIntStateAssignmentValues(assignment, &nvalues);
        value = nvalues == 1 ? values[0] : 0;
    }

    const Effect *effect;
    CGstate state;
    CGstateassignment assignment;
};

struct Effect::Parameter : IEffect::IParameter {
    Parameter(const Effect *e, CGparameter p)
        : effect(e),
          parameter(p)
    {
    }
    ~Parameter() {
        m_states.releaseAll();
        effect = 0;
        parameter = 0;
    }

    IEffect *parentEffectRef() const {
        return const_cast<Effect *>(effect);
    }
    const IEffect::IAnnotation *annotationRef(const char *name) const {
        CGannotation annotation = cgGetNamedParameterAnnotation(parameter, name);
        return effect->cacheAnnotationRef(annotation);
    }
    const char *name() const {
        return cgIsParameter(parameter) ? cgGetParameterName(parameter) : Annotation::kEmpty;
    }
    const char *semantic() const {
        return cgIsParameter(parameter) ? cgGetParameterSemantic(parameter) : Annotation::kEmpty;
    }
    Type type() const {
        return toEffectType(cgGetParameterType(parameter));
    }
    void connect(IParameter *destinationParameter) {
        if (Parameter *p = static_cast<Parameter *>(destinationParameter)) {
            cgConnectParameter(parameter, p->parameter);
        }
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
    void getSamplerStateRefs(Array<IEffect::ISamplerState *> &value) const {
        const int nstates = m_states.count();
        if (nstates == 0) {
            CGstateassignment assignment = cgGetFirstSamplerStateAssignment(parameter);
            while (assignment) {
                value.append(m_states.append(new Effect::SamplerState(effect, assignment)));
                assignment = cgGetNextStateAssignment(assignment);
            }
        }
        else {
            value.clear();
            for (int i = 0; i < nstates; i++) {
                Effect::SamplerState *state = m_states[i];
                value.append(state);
            }
        }
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
        cgGLSetParameter4fv(parameter, value);
    }
    void setValue(const Vector4 &value) {
        cgGLSetParameter4fv(parameter, value);
    }
    void setValue(const Vector4 *value) {
        cgGLSetParameter4fv(parameter, reinterpret_cast<const float *>(&value[0]));
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
    void setTexture(intptr_t value) {
        cgGLSetTextureParameter(parameter, static_cast<GLuint>(value));
    }

    mutable PointerArray<Effect::SamplerState> m_states;
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
        m_passes.releaseAll();
        effect = 0;
        technique = 0;
    }

    IEffect *parentEffectRef() const {
        return const_cast<Effect *>(effect);
    }
    IEffect::IPass *findPass(const char *name) const {
        CGpass pass = cgGetNamedPass(technique, name);
        return cachePassRef(pass);
    }
    const IEffect::IAnnotation *annotationRef(const char *name) const {
        CGannotation annotation = cgGetNamedTechniqueAnnotation(technique, name);
        return effect->cacheAnnotationRef(annotation);
    }
    void getPasses(Array<IEffect::IPass *> &passes) const {
        CGpass pass = cgGetFirstPass(technique);
        while (pass) {
            passes.append(cachePassRef(pass));
            pass = cgGetNextPass(pass);
        }
    }

    Effect::Pass *cachePassRef(CGpass pass) const {
        if (cgIsPass(pass)) {
            const char *name = cgGetPassName(pass);
            if (Effect::Pass *const *passPtr = m_passRefsHash.find(name)) {
                return *passPtr;
            }
            else {
                Effect::Pass *newPassPtr = m_passes.append(new Effect::Pass(effect, this, pass));
                m_passRefsHash.insert(name, newPassPtr);
                return newPassPtr;
            }
        }
        return 0;
    }

    mutable PointerArray<Effect::Pass> m_passes;
    mutable Hash<HashString, Effect::Pass *> m_passRefsHash;
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
    m_annotations.releaseAll();
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

void Effect::getRenderColorTargetIndices(Array<int> &value) const
{
    value.clear();
    const int nRenderColorTargetIndices = m_renderColorTargetIndices.count();
    for (int i = 0; i < nRenderColorTargetIndices; i++) {
        value.append(m_renderColorTargetIndices[i]);
    }
}

void Effect::addRenderColorTargetIndex(int targetIndex)
{
    m_renderColorTargetIndices.append(targetIndex);
    Util::setRenderColorTargets(&m_renderColorTargetIndices[0], m_renderColorTargetIndices.count());
}

void Effect::removeRenderColorTargetIndex(int targetIndex)
{
    m_renderColorTargetIndices.remove(targetIndex);
    Util::setRenderColorTargets(&m_renderColorTargetIndices[0], m_renderColorTargetIndices.count());
}

void Effect::clearRenderColorTargetIndices()
{
    m_renderColorTargetIndices.clear();
}

void Effect::inheritRenderColorTargetIndices(const IEffect *sourceEffect)
{
    Array<int> renderColorTargetIndices;
    sourceEffect->getRenderColorTargetIndices(renderColorTargetIndices);
    m_renderColorTargetIndices.clear();
    const int nRenderColorTargetIndices = renderColorTargetIndices.count();
    for (int i = 0; i < nRenderColorTargetIndices; i++) {
        m_renderColorTargetIndices.append(renderColorTargetIndices[i]);
    }
}

bool Effect::hasRenderColorTargetIndex(int targetIndex) const
{
    const int nRenderColorTargetIndices = m_renderColorTargetIndices.count();
    for (int i = 0; i < nRenderColorTargetIndices; i++) {
        int renderColorTarget = m_renderColorTargetIndices[i];
        if (renderColorTarget == targetIndex) {
            return true;
        }
    }
    return false;
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
    return cacheParameterRef(parameter);
}

IEffect::ITechnique *Effect::findTechnique(const char *name) const
{
    CGtechnique technique = cgGetNamedTechnique(m_effect, name);
    return cacheTechniqueRef(technique);
}

void Effect::getParameterRefs(Array<IParameter *> &parameters) const
{
    CGparameter parameter = cgGetFirstEffectParameter(m_effect);
    while (parameter) {
        parameters.append(cacheParameterRef(parameter));
        parameter = cgGetNextParameter(parameter);
    }
}

void Effect::getTechniqueRefs(Array<ITechnique *> &techniques) const
{
    CGtechnique technique = cgGetFirstTechnique(m_effect);
    while (technique) {
        techniques.append(cacheTechniqueRef(technique));
        technique = cgGetNextTechnique(technique);
    }
}

IEffect::IAnnotation *Effect::cacheAnnotationRef(CGannotation annotation) const
{
    if (cgIsAnnotation(annotation)) {
        if (Annotation *const *annotationPtr = m_annotationRefsHash.find(annotation)) {
            return *annotationPtr;
        }
        else {
            Effect::Annotation *newAnnotationPtr = m_annotations.append(new Effect::Annotation(this, annotation));
            m_annotationRefsHash.insert(annotation, newAnnotationPtr);
            return newAnnotationPtr;
        }
    }
    return 0;
}

IEffect::IParameter *Effect::cacheParameterRef(CGparameter parameter) const
{
    if (cgIsParameter(parameter)) {
        if (Parameter *const *parameterPtr = m_parameterRefsHash.find(parameter)) {
            return *parameterPtr;
        }
        else {
            Effect::Parameter *newParameterPtr = m_parameters.append(new Effect::Parameter(this, parameter));
            m_parameterRefsHash.insert(parameter, newParameterPtr);
            return newParameterPtr;
        }
    }
    return 0;
}

IEffect::ITechnique *Effect::cacheTechniqueRef(CGtechnique technique) const
{
    if (cgIsTechnique(technique)) {
        if (Technique *const *techniquePtr = m_techniqueRefsHash.find(technique)) {
            return *techniquePtr;
        }
        else {
            Effect::Technique *newTechniquePtr = m_techniques.append(new Effect::Technique(this, technique));
            m_techniqueRefsHash.insert(technique, newTechniquePtr);
            return newTechniquePtr;
        }
    }
    return 0;
}

} /* namespace cg */
} /* namespace vpvl2 */
