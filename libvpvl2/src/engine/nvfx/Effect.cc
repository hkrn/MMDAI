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
#include "vpvl2/nvfx/Effect.h"
#include "vpvl2/nvfx/EffectContext.h"
#include "vpvl2/fx/EffectEngine.h"

#include <FxParser.h>

namespace vpvl2
{
namespace nvfx
{

using namespace extensions::fx;
using namespace extensions::gl;

static IEffect::Parameter::Type toEffectType(nvFX::IUniform::Type type)
{
    switch (type) {
    case nvFX::IUniform::TBool:
        return IEffect::Parameter::kBoolean;
    case nvFX::IUniform::TFloat:
        return IEffect::Parameter::kFloat;
    case nvFX::IUniform::TVec3:
        return IEffect::Parameter::kFloat3;
    case nvFX::IUniform::TVec4:
        return IEffect::Parameter::kFloat4;
    case nvFX::IUniform::TMat4:
        return IEffect::Parameter::kFloat4x4;
    case nvFX::IUniform::TInt:
        return IEffect::Parameter::kInteger;
    case nvFX::IUniform::TTexture1D:
        return IEffect::Parameter::kSampler1D;
    case nvFX::IUniform::TTexture2D:
        return IEffect::Parameter::kSampler2D;
    case nvFX::IUniform::TTexture3D:
        return IEffect::Parameter::kSampler3D;
    case nvFX::IUniform::TTextureCube:
        return IEffect::Parameter::kSamplerCube;
    default:
        return IEffect::Parameter::kUnknownParameterType;
    }
}

struct Effect::NvFXAnnotation : IEffect::Annotation {
    static const char *kEmpty;

    NvFXAnnotation(const Effect *e, nvFX::IAnnotation *a, const char *n)
        : effect(e),
          name(n),
          annotation(a),
          fval(0),
          ival(0)
    {
    }
    ~NvFXAnnotation() {
        effect = 0;
        annotation = 0;
    }

    bool booleanValue() const {
        return annotation->getAnnotationInt(name.c_str()) == 1;
    }
    int integerValue() const {
        return annotation->getAnnotationInt(name.c_str());
    }
    const int *integerValues(int *size) const {
        *size = 1;
        ival = annotation->getAnnotationInt(name.c_str());
        return &ival;
    }
    float floatValue() const {
        return annotation->getAnnotationFloat(name.c_str());
    }
    const float *floatValues(int *size) const {
        *size = 1;
        fval = annotation->getAnnotationFloat(name.c_str());
        return &fval;
    }
    const char *stringValue() const {
        return annotation->getAnnotationString(name.c_str());
    }

    const Effect *effect;
    const std::string name;
    nvFX::IAnnotation *annotation;
    mutable float fval;
    mutable int ival;
};

const char *Effect::NvFXAnnotation::kEmpty = "";

struct Effect::NvFXPass : IEffect::Pass {
    NvFXPass(const Effect *e, const IEffect::Technique *t, nvFX::IPass *a)
        : effect(e),
          technique(t),
          pass(a)
    {
    }
    ~NvFXPass() {
        effect = 0;
        technique = 0;
        pass = 0;
    }

    IEffect::Technique *parentTechniqueRef() const {
        return const_cast<IEffect::Technique *>(technique);
    }
    const IEffect::Annotation *annotationRef(const char *name) const {
        return effect->cacheAnnotationRef(pass->annotations(), name);
    }
    const char *name() const {
        return pass->getName();
    }
    void setState() {
        pass->execute();
    }
    void resetState() {
    }

    const Effect *effect;
    const IEffect::Technique *technique;
    nvFX::IPass *pass;
};

struct Effect::NvFXSamplerState : IEffect::SamplerState {
    NvFXSamplerState(const Effect *e, nvFX::ISamplerState *sa)
        : effect(e),
          assignment(sa)
    {
    }
    ~NvFXSamplerState() {
        effect = 0;
        assignment = 0;
    }

    const char *name() const {
        return assignment->getName();
    }
    Parameter::Type type() const {
        return Parameter::kUnknownParameterType;// toEffectType(cgGetStateType(state));
    }
    Parameter *parameterRef() const {
        return 0;
        /*
        Parameter *parameter = 0;
        return effect->cacheParameterRef(parameter);
        */
    }
    void getValue(int &value) const {
        /*
        int nvalues;
        const int *values = cgGetIntStateAssignmentValues(assignment, &nvalues);
        value = nvalues == 1 ? values[0] : 0;
        */
        value = 0;
    }

    const Effect *effect;
    nvFX::ISamplerState *assignment;
};

struct Effect::NvFXParameter : IEffect::Parameter {
    NvFXParameter(const Effect *e, nvFX::IUniform *p)
        : effect(e),
          parameter(p)
    {
    }
    ~NvFXParameter() {
        m_states.releaseAll();
        effect = 0;
        parameter = 0;
    }

    IEffect *parentEffectRef() const {
        return const_cast<Effect *>(effect);
    }
    const IEffect::Annotation *annotationRef(const char *name) const {
        nvFX::IAnnotation *annotation = parameter->annotations();
        return effect->cacheAnnotationRef(annotation, name);
    }
    const char *name() const {
        return parameter->getName();
    }
    const char *semantic() const {
        return parameter->getSemantic();
    }
    Type type() const {
        return toEffectType(parameter->getType());
    }
    VariableType variableType() const {
        return kUniform;
    }
    void connect(Parameter * /* destinationParameter */) {
    }
    void reset() {
        parameter = 0;
    }
    void getValue(int &value) const {
        int v = 0;
        parameter->getValueiv(&v, 1);
        value = v;
    }
    void getValue(float &value) const {
        float v = 0;
        parameter->getValuefv(&v, 1);
        value = v;
    }
    void getValue(Vector3 &value) const {
        parameter->getValuefv(static_cast<float *>(&value[0]), 1);
    }
    void getValue(Vector4 &value) const {
        parameter->getValuefv(static_cast<float *>(&value[0]), 1);
    }
    void getMatrix(float *value) const {
        parameter->getValuefv(value, 2, 4);
    }
    void getArrayDimension(int &value) const {
        value = parameter->getArraySz();
    }
    void getArrayTotalSize(int &value) const {
        value = parameter->getArraySz();;
    }
    void getTextureRef(intptr_t &value) const {
        value = 0; //cgGLGetTextureParameter(parameter);
    }
    void getSamplerStateRefs(Array<IEffect::SamplerState *> &value) const {
        value.clear();
        /*
        const int nstates = m_states.count();
        if (nstates == 0) {
            CGstateassignment assignment = cgGetFirstSamplerStateAssignment(parameter);
            while (assignment) {
                value.append(m_states.append(new Effect::NvFXSamplerState(effect, assignment)));
                assignment = cgGetNextStateAssignment(assignment);
            }
        }
        else {
            value.clear();
            for (int i = 0; i < nstates; i++) {
                Effect::NvFXSamplerState *state = m_states[i];
                value.append(state);
            }
        }
        */
    }
    void setValue(bool value) {
        parameter->setValue1b(value);
    }
    void setValue(int value) {
        parameter->setValueiv(&value, 1);
    }
    void setValue(float value) {
        parameter->setValuefv(&value, 1);
    }
    void setValue(const Vector3 &value) {
        const float *v = value;
        parameter->setValuefv(const_cast<float *>(v), 1, 4);
    }
    void setValue(const Vector4 &value) {
        const float *v = value;
        parameter->setValuefv(const_cast<float *>(v), 1, 4);
    }
    void setValue(const Vector4 *value) {
        const float *v = *value;
        parameter->setValuefv(const_cast<float *>(v), 1, 4);
    }
    void setMatrix(const float *value) {
        parameter->setMatrix4f(const_cast<float *>(value));
    }
    void setSampler(const ITexture *value) {
        const Vector3 &size = value->size();
        nvFX::ResourceType type(nvFX::RESOURCE_UNKNOWN);
        if (size.z() > 1) {
            type = nvFX::RESTEX_3D;
        }
        else if (size.y() > 1) {
            type = nvFX::RESTEX_2D;
        }
        else if (size.x() > 0) {
            type = nvFX::RESTEX_1D;
        }
        GLuint textureID = value ? static_cast<GLuint>(value->data()) : 0;
        parameter->setSamplerResource(type, textureID);
    }
    void setTexture(const ITexture *value) {
        setTexture(value->data());
    }
    void setTexture(intptr_t value) {
        GLint textureID = static_cast<GLint>(value);
        parameter->setImageResource(nvFX::RESTEX_2D, textureID);
    }
    void setPointer(const void *ptr, vsize size, vsize stride, Type type) {
        switch (type) {
        case IEffect::Parameter::kInteger:
            // cgGLSetParameterPointer(parameter, size, GL_INT, stride, ptr);
            break;
        case IEffect::Parameter::kFloat:
            // cgGLSetParameterPointer(parameter, size, GL_FLOAT, stride, ptr);
            break;
        default:
            break;
        }
    }

    mutable PointerArray<Effect::NvFXSamplerState> m_states;
    const Effect *effect;
    nvFX::IUniform *parameter;
};

struct Effect::NvFXTechnique : IEffect::Technique {
    NvFXTechnique(const Effect *e, nvFX::ITechnique *p)
        : effect(e),
          technique(p)
    {
    }
    ~NvFXTechnique() {
        m_passes.releaseAll();
        effect = 0;
        technique = 0;
    }

    IEffect *parentEffectRef() const {
        return const_cast<Effect *>(effect);
    }
    IEffect::Pass *findPass(const char *name) const {
        if (Effect::NvFXPass *const *passPtr = m_passRefsHash.find(name)) {
            return *passPtr;
        }
        return 0;
    }
    const IEffect::Annotation *annotationRef(const char *name) const {
        nvFX::IAnnotation *annotation = technique->annotations();
        return effect->cacheAnnotationRef(annotation, name);
    }
    const char *name() const {
        return technique->getName();
    }
    void getPasses(Array<IEffect::Pass *> &passes) const {
        const int npasses = technique->getNumPasses();
        for (int i = 0; i < npasses; i++) {
            nvFX::IPass *pass = technique->getPass(i);
            passes.append(cachePassRef(pass));
        }
    }

    Effect::NvFXPass *cachePassRef(nvFX::IPass *pass) const {
        const char *name = pass->getName();
        if (Effect::NvFXPass *const *passPtr = m_passRefsHash.find(name)) {
            return *passPtr;
        }
        else {
            Effect::NvFXPass *newPassPtr = m_passes.append(new Effect::NvFXPass(effect, this, pass));
            m_passRefsHash.insert(name, newPassPtr);
            return newPassPtr;
        }
        return 0;
    }

    mutable PointerArray<Effect::NvFXPass> m_passes;
    mutable Hash<HashString, Effect::NvFXPass *> m_passRefsHash;
    const Effect *effect;
    nvFX::ITechnique *technique;
};

bool Effect::isInteractiveParameter(const Parameter *value)
{
    const IEffect::Annotation *name = value->annotationRef("UIName");
    const IEffect::Annotation *widget = value->annotationRef("UIWidget");
    return name && widget;
}

Effect::Effect(EffectContext *contextRef, IApplicationContext *applicationContextRef, nvFX::IContainer *container)
    : m_applicationContextRef(applicationContextRef),
      m_effectContextRef(contextRef),
      m_container(container),
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
    nvFX::IContainer::destroy(m_container);
    m_container = 0;
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
    return m_container;
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
    return 0; //return cacheParameterRef(cgGetEffectParameterBySemantic(m_container, name));
}

IEffect::Parameter *Effect::findUniformParameter(const char *name) const
{
    nvFX::IUniform *parameter = m_container->findUniform(name);
    return cacheParameterRef(parameter);
}

IEffect::Technique *Effect::findTechnique(const char *name) const
{
    nvFX::ITechnique *technique = m_container->findTechnique(name);
    return cacheTechniqueRef(technique);
}

void Effect::getParameterRefs(Array<Parameter *> &parameters) const
{
    int i = 0;
    while (nvFX::IUniform *parameter = m_container->findUniform(i)) {
        parameters.append(cacheParameterRef(parameter));
        i++;
    }
}

void Effect::getTechniqueRefs(Array<Technique *> &techniques) const
{
    const int ntechniques = m_container->getNumTechniques();
    for (int i = 0; i < ntechniques; i++) {
        nvFX::ITechnique *technique = m_container->findTechnique(i);
        techniques.append(cacheTechniqueRef(technique));
    }
}

IEffect::Annotation *Effect::cacheAnnotationRef(nvFX::IAnnotation *annotation, const char *name) const
{
    if (NvFXAnnotation *const *annotationPtr = m_annotationRefsHash.find(annotation)) {
        return *annotationPtr;
    }
    else {
        NvFXAnnotation *newAnnotationPtr = m_annotations.append(new NvFXAnnotation(this, annotation, name));
        m_annotationRefsHash.insert(annotation, newAnnotationPtr);
        return newAnnotationPtr;
    }
}

IEffect::Parameter *Effect::cacheParameterRef(nvFX::IUniform *parameter) const
{
    if (NvFXParameter *const *parameterPtr = m_parameterRefsHash.find(parameter)) {
        return *parameterPtr;
    }
    else {
        NvFXParameter *newParameterPtr = m_parameters.append(new NvFXParameter(this, parameter));
        m_parameterRefsHash.insert(parameter, newParameterPtr);
        return newParameterPtr;
    }
}

IEffect::Technique *Effect::cacheTechniqueRef(nvFX::ITechnique *technique) const
{
    if (NvFXTechnique *const *techniquePtr = m_techniqueRefsHash.find(technique)) {
        return *techniquePtr;
    }
    else {
        NvFXTechnique *newTechniquePtr = m_techniques.append(new NvFXTechnique(this, technique));
        m_techniqueRefsHash.insert(technique, newTechniquePtr);
        return newTechniquePtr;
    }
}

} /* namespace nvfx */
} /* namespace vpvl2 */
