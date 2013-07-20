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

#include "vpvl2/vpvl2.h"
#include "vpvl2/IApplicationContext.h"
#include "vpvl2/cg/Effect.h"
#include "vpvl2/cg/EffectContext.h"
#include "vpvl2/fx/EffectEngine.h"

namespace vpvl2
{
namespace cg
{

using namespace extensions::fx;
using namespace extensions::gl;

static IEffect::Parameter::Type toEffectType(CGtype type)
{
    switch (type) {
    case CG_BOOL:
        return IEffect::Parameter::kBoolean;
    case CG_FLOAT:
        return IEffect::Parameter::kFloat;
    case CG_FLOAT3:
        return IEffect::Parameter::kFloat3;
    case CG_FLOAT4:
        return IEffect::Parameter::kFloat4;
    case CG_FLOAT4x4:
        return IEffect::Parameter::kFloat4x4;
    case CG_INT:
        return IEffect::Parameter::kInteger;
    case CG_TEXTURE:
        return IEffect::Parameter::kTexture;
    case CG_SAMPLER2D:
        return IEffect::Parameter::kSampler2D;
    case CG_SAMPLER3D:
        return IEffect::Parameter::kSampler3D;
    case CG_SAMPLERCUBE:
        return IEffect::Parameter::kSamplerCube;
    default:
        return IEffect::Parameter::kUnknownParameterType;
    }
}

struct Effect::CgFXAnnotation : IEffect::Annotation {
    static const char *kEmpty;

    CgFXAnnotation(const Effect *e, CGannotation a)
        : effect(e),
          annotation(a)
    {
    }
    ~CgFXAnnotation() {
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
        const char *value = cgGetStringAnnotationValue(annotation);
        return value ? value : kEmpty;
    }

    const Effect *effect;
    CGannotation annotation;
};

const char *Effect::CgFXAnnotation::kEmpty = "";

struct Effect::CgFXPass : IEffect::Pass {
    CgFXPass(const Effect *e, const IEffect::Technique *t, CGpass a)
        : effect(e),
          technique(t),
          pass(a)
    {
    }
    ~CgFXPass() {
        effect = 0;
        technique = 0;
        pass = 0;
    }

    IEffect::Technique *parentTechniqueRef() const {
        return const_cast<IEffect::Technique *>(technique);
    }
    const IEffect::Annotation *annotationRef(const char *name) const {
        CGannotation annotation = cgGetNamedPassAnnotation(pass, name);
        return effect->cacheAnnotationRef(annotation);
    }
    const char *name() const {
        const char *value = cgGetPassName(pass);
        return value ? value : CgFXAnnotation::kEmpty;
    }
    void setState() {
        cgSetPassState(pass);
    }
    void resetState() {
        cgResetPassState(pass);
    }

    const Effect *effect;
    const IEffect::Technique *technique;
    CGpass pass;
};

struct Effect::CgFXSamplerState : IEffect::SamplerState {
    CgFXSamplerState(const Effect *e, CGstateassignment sa)
        : effect(e),
          state(cgGetSamplerStateAssignmentState(sa)),
          assignment(sa)
    {
    }
    ~CgFXSamplerState() {
        effect = 0;
        state = 0;
        assignment = 0;
    }

    const char *name() const {
        const char *value = cgGetStateName(state);
        return value ? value : CgFXAnnotation::kEmpty;
    }
    Parameter::Type type() const {
        return toEffectType(cgGetStateType(state));
    }
    Parameter *parameterRef() const {
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

struct Effect::CgFXParameter : IEffect::Parameter {
    CgFXParameter(const Effect *e, CGparameter p)
        : effect(e),
          parameter(p)
    {
    }
    ~CgFXParameter() {
        m_states.releaseAll();
        effect = 0;
        parameter = 0;
    }

    IEffect *parentEffectRef() const {
        return const_cast<Effect *>(effect);
    }
    const IEffect::Annotation *annotationRef(const char *name) const {
        CGannotation annotation = cgGetNamedParameterAnnotation(parameter, name);
        return effect->cacheAnnotationRef(annotation);
    }
    const char *name() const {
        const char *value = cgGetParameterName(parameter);
        return value ? value : CgFXAnnotation::kEmpty;
    }
    const char *semantic() const {
        const char *value = cgGetParameterSemantic(parameter);
        return value ? value : CgFXAnnotation::kEmpty;
    }
    Type type() const {
        return toEffectType(cgGetParameterType(parameter));
    }
    VariableType variableType() const {
        const CGenum type = cgGetParameterVariability(parameter);
        switch (type) {
        case CG_VARYING:
            return kVarying;
        case CG_UNIFORM:
            return kUniform;
        case CG_CONSTANT:
            return kConstant;
        default:
            return kUnknownVariableType;
        }
    }
    void connect(Parameter *destinationParameter) {
        if (CgFXParameter *p = static_cast<CgFXParameter *>(destinationParameter)) {
            cgConnectParameter(parameter, p->parameter);
        }
    }
    void reset() {
        cgDisconnectParameter(parameter);
        parameter = 0;
    }
    void getValue(int &value) const {
        float v = 0;
        getValue(v);
        value = int(v);
    }
    void getValue(float &value) const {
        float v = 0;
        cgGLGetParameter1f(parameter, &v);
        value = v;
    }
    void getValue(Vector3 &value) const {
        cgGLGetParameter4f(parameter, static_cast<float *>(&value[0]));
    }
    void getValue(Vector4 &value) const {
        cgGLGetParameter4f(parameter, static_cast<float *>(&value[0]));
    }
    void getMatrix(float *value) const {
        cgGLGetMatrixParameterfr(parameter, value);
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
    void getSamplerStateRefs(Array<IEffect::SamplerState *> &value) const {
        const int nstates = m_states.count();
        if (nstates == 0) {
            CGstateassignment assignment = cgGetFirstSamplerStateAssignment(parameter);
            while (assignment) {
                value.append(m_states.append(new Effect::CgFXSamplerState(effect, assignment)));
                assignment = cgGetNextStateAssignment(assignment);
            }
        }
        else {
            value.clear();
            for (int i = 0; i < nstates; i++) {
                Effect::CgFXSamplerState *state = m_states[i];
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
        GLuint textureID = value ? static_cast<GLuint>(value->data()) : 0;
        cgGLSetupSampler(parameter, textureID);
    }
    void setTexture(const ITexture *value) {
        GLuint textureID = value ? static_cast<GLuint>(value->data()) : 0;
        cgGLSetTextureParameter(parameter, textureID);
    }
    void setTexture(intptr_t value) {
        cgGLSetTextureParameter(parameter, static_cast<GLuint>(value));
    }

    mutable PointerArray<Effect::CgFXSamplerState> m_states;
    const Effect *effect;
    CGparameter parameter;
};

struct Effect::CgFXTechnique : IEffect::Technique {
    CgFXTechnique(const Effect *e, CGtechnique p)
        : effect(e),
          technique(p)
    {
    }
    ~CgFXTechnique() {
        m_passes.releaseAll();
        effect = 0;
        technique = 0;
    }

    IEffect *parentEffectRef() const {
        return const_cast<Effect *>(effect);
    }
    IEffect::Pass *findPass(const char *name) const {
        CGpass pass = cgGetNamedPass(technique, name);
        return cachePassRef(pass);
    }
    const IEffect::Annotation *annotationRef(const char *name) const {
        CGannotation annotation = cgGetNamedTechniqueAnnotation(technique, name);
        return effect->cacheAnnotationRef(annotation);
    }
    const char *name() const {
        const char *value = cgGetTechniqueName(technique);
        return value ? value : CgFXAnnotation::kEmpty;
    }
    void getPasses(Array<IEffect::Pass *> &passes) const {
        CGpass pass = cgGetFirstPass(technique);
        while (pass) {
            passes.append(cachePassRef(pass));
            pass = cgGetNextPass(pass);
        }
    }

    Effect::CgFXPass *cachePassRef(CGpass pass) const {
        if (cgIsPass(pass)) {
            const char *name = cgGetPassName(pass);
            if (Effect::CgFXPass *const *passPtr = m_passRefsHash.find(name)) {
                return *passPtr;
            }
            else {
                Effect::CgFXPass *newPassPtr = m_passes.append(new Effect::CgFXPass(effect, this, pass));
                m_passRefsHash.insert(name, newPassPtr);
                return newPassPtr;
            }
        }
        return 0;
    }

    mutable PointerArray<Effect::CgFXPass> m_passes;
    mutable Hash<HashString, Effect::CgFXPass *> m_passRefsHash;
    const Effect *effect;
    CGtechnique technique;
};

bool Effect::isInteractiveParameter(const Parameter *value)
{
    const IEffect::Annotation *name = value->annotationRef("UIName");
    const IEffect::Annotation *widget = value->annotationRef("UIWidget");
    return name && widget;
}

Effect::Effect(EffectContext *contextRef, IApplicationContext *applicationContextRef, CGeffect effect)
    : m_applicationContextRef(applicationContextRef),
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
    m_applicationContextRef = 0;
    m_effectContextRef = 0;
    m_parentEffectRef = 0;
    m_scriptOrderType = kStandard;
}

void Effect::createFrameBufferObject()
{
    delete m_parentFrameBufferObject;
    m_parentFrameBufferObject = m_applicationContextRef->createFrameBufferObject();
}

void Effect::addOffscreenRenderTarget(ITexture *textureRef, IEffect::Parameter *textureParameterRef, IEffect::Parameter *samplerParameterRef)
{
    OffscreenRenderTarget target;
    target.textureRef = textureRef;
    target.textureParameterRef = textureParameterRef;
    target.samplerParameterRef = samplerParameterRef;
    m_offscreenRenderTargets.append(target);
}

void Effect::addInteractiveParameter(Parameter *value)
{
    if (isInteractiveParameter(value)) {
        m_interactiveParameters.append(value);
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
    Util::setRenderColorTargets(0, 0);
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

void Effect::getInteractiveParameters(Array<IEffect::Parameter *> &value) const
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

IEffect::Parameter *Effect::findVaryingParameter(const char *name) const
{
    return cacheParameterRef(cgGetEffectParameterBySemantic(m_effect, name));
}

IEffect::Parameter *Effect::findUniformParameter(const char *name) const
{
    CGparameter parameter = cgGetNamedEffectParameter(m_effect, name);
    return cacheParameterRef(parameter);
}

IEffect::Technique *Effect::findTechnique(const char *name) const
{
    CGtechnique technique = cgGetNamedTechnique(m_effect, name);
    return cacheTechniqueRef(technique);
}

void Effect::getParameterRefs(Array<Parameter *> &parameters) const
{
    CGparameter parameter = cgGetFirstEffectParameter(m_effect);
    while (parameter) {
        parameters.append(cacheParameterRef(parameter));
        parameter = cgGetNextParameter(parameter);
    }
}

void Effect::getTechniqueRefs(Array<Technique *> &techniques) const
{
    CGtechnique technique = cgGetFirstTechnique(m_effect);
    while (technique) {
        techniques.append(cacheTechniqueRef(technique));
        technique = cgGetNextTechnique(technique);
    }
}

void Effect::setVertexAttributePointer(VertexAttributeType vtype, Parameter::Type /* ptype */, vsize stride, const void *ptr)
{
    switch (vtype) {
    case kPositionVertexAttribute:
        glVertexPointer(3, GL_FLOAT, stride, ptr);
        break;
    case kNormalVertexAttribute:
        glNormalPointer(GL_FLOAT, stride, ptr);
        break;
    case kTextureCoordVertexAttribute:
        glTexCoordPointer(2, GL_FLOAT, stride, ptr);
        break;
    default:
        /* do nothing */
        break;
    }
}

void Effect::activateVertexAttribute(VertexAttributeType vtype)
{
    switch (vtype) {
    case kPositionVertexAttribute:
        glEnableClientState(GL_VERTEX_ARRAY);
        break;
    case kNormalVertexAttribute:
        glEnableClientState(GL_NORMAL_ARRAY);
        break;
    case kTextureCoordVertexAttribute:
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        break;
    default:
        /* do nothing */
        break;
    }
}

IEffect::Annotation *Effect::cacheAnnotationRef(CGannotation annotation) const
{
    if (cgIsAnnotation(annotation)) {
        if (CgFXAnnotation *const *annotationPtr = m_annotationRefsHash.find(annotation)) {
            return *annotationPtr;
        }
        else {
            Effect::CgFXAnnotation *newAnnotationPtr = m_annotations.append(new Effect::CgFXAnnotation(this, annotation));
            m_annotationRefsHash.insert(annotation, newAnnotationPtr);
            return newAnnotationPtr;
        }
    }
    return 0;
}

IEffect::Parameter *Effect::cacheParameterRef(CGparameter parameter) const
{
    if (cgIsParameter(parameter)) {
        if (CgFXParameter *const *parameterPtr = m_parameterRefsHash.find(parameter)) {
            return *parameterPtr;
        }
        else {
            Effect::CgFXParameter *newParameterPtr = m_parameters.append(new Effect::CgFXParameter(this, parameter));
            m_parameterRefsHash.insert(parameter, newParameterPtr);
            return newParameterPtr;
        }
    }
    return 0;
}

IEffect::Technique *Effect::cacheTechniqueRef(CGtechnique technique) const
{
    if (cgIsTechnique(technique)) {
        if (CgFXTechnique *const *techniquePtr = m_techniqueRefsHash.find(technique)) {
            return *techniquePtr;
        }
        else {
            Effect::CgFXTechnique *newTechniquePtr = m_techniques.append(new Effect::CgFXTechnique(this, technique));
            m_techniqueRefsHash.insert(technique, newTechniquePtr);
            return newTechniquePtr;
        }
    }
    return 0;
}

} /* namespace cg */
} /* namespace vpvl2 */
