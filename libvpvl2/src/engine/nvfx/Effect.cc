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
#include "vpvl2/extensions/fx/Util.h"
#include "vpvl2/internal/util.h"

/* prevent compile error */
#ifndef GLhandleARB
#define GLhandleARB void *
#endif
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
    case nvFX::IUniform::TTexture:
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
        const char *value = annotation->getAnnotationString(name.c_str());
        return value ? value : kEmpty;
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
        : effectRef(e),
          techniqueRef(t),
          valueRef(a)
    {
    }
    ~NvFXPass() {
        effectRef = 0;
        techniqueRef = 0;
        valueRef = 0;
    }

    static void castPasses(const Array<Pass *> &srcPasses, Array<nvFX::IPass *> &destPasses) {
        destPasses.clear();
        const int npasses = srcPasses.count();
        for (int i = 0; i < npasses; i++) {
            NvFXPass *pass = static_cast<NvFXPass *>(srcPasses[i]);
            destPasses.append(pass->valueRef);
        }
    }
    void internalReleaseOverrides(const Array<nvFX::IPass *> &passes) {
        if (passes.count() > 0) {
            nvFX::IPass *pass = passes[0];
            pass->releaseOverrides(&pass, passes.count());
        }
    }

    IEffect::Technique *parentTechniqueRef() const {
        return const_cast<IEffect::Technique *>(techniqueRef);
    }
    const IEffect::Annotation *annotationRef(const char *name) const {
        nvFX::IAnnotation *annotations = valueRef->annotations();
        return effectRef->cacheAnnotationRef(annotations, name);
    }
    const char *name() const {
        return valueRef->getName();
    }
    void setState() {
        pushAnnotationGroup(std::string("NvFXPass#setState name=").append(name()).c_str(), effectRef->applicationContextRef()->sharedFunctionResolverInstance());
        valueRef->execute(&info);
        popAnnotationGroup(effectRef->applicationContextRef()->sharedFunctionResolverInstance());
    }
    void resetState() {
        valueRef->unbindProgram();
    }
    void setupOverrides(const Array<Pass *> &passes) {
        pushAnnotationGroup(std::string("NvFXPass#setupOverrides name=").append(name()).c_str(), effectRef->applicationContextRef()->sharedFunctionResolverInstance());
        internalReleaseOverrides(overridePasses);
        if (passes.count() > 0) {
            castPasses(passes, overridePasses);
            const int numOverridePasses = overridePasses.count();
            valueRef->setupOverrides(&overridePasses[0], numOverridePasses);
            for (int i = 0; i < numOverridePasses; i++) {
                nvFX::IPass *passRef = overridePasses[i];
                passRef->validate();
            }
        }
        popAnnotationGroup(effectRef->applicationContextRef()->sharedFunctionResolverInstance());
    }
    void releaseOverrides(const Array<Pass *> &passes) {
        pushAnnotationGroup(std::string("NvFXPass#releaseOverrides name=").append(name()).c_str(), effectRef->applicationContextRef()->sharedFunctionResolverInstance());
        if (passes.count() > 0) {
            Array<nvFX::IPass *> castedPasses;
            castPasses(passes, castedPasses);
            valueRef->releaseOverrides(&castedPasses[0], castedPasses.count());
        }
        popAnnotationGroup(effectRef->applicationContextRef()->sharedFunctionResolverInstance());
    }

    const Effect *effectRef;
    const IEffect::Technique *techniqueRef;
    Array<nvFX::IPass *> overridePasses;
    nvFX::IPass *valueRef;
    nvFX::PassInfo info;
};

struct Effect::NvFXSamplerState : IEffect::SamplerState {
    NvFXSamplerState(const IEffect::Parameter *p)
        : effectRef(p->parentEffectRef()),
          valueRef(const_cast<IEffect::Parameter *>(p))
    {
    }
    ~NvFXSamplerState() {
        effectRef = 0;
        valueRef = 0;
    }

    const char *name() const {
        return valueRef->name();
    }
    Parameter::Type type() const {
        return Effect::Parameter::kTexture;
    }
    Parameter *parameterRef() const {
        return valueRef;
    }
    void getValue(int &value) const {
        valueRef->getValue(value);
    }

    const IEffect *effectRef;
    IEffect::Parameter *valueRef;
};

struct Effect::NvFXParameter : IEffect::Parameter {

    static nvFX::ResourceType detectResourceType(const ITexture *texture) {
        const Vector3 &size = texture->size();
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
        return type;
    }

    NvFXParameter(const Effect *e, nvFX::IUniform *p)
        : effectRef(e),
          valueRef(p)
    {
    }
    ~NvFXParameter() {
        m_states.releaseAll();
        effectRef = 0;
        valueRef = 0;
    }

    IEffect *parentEffectRef() const {
        return const_cast<Effect *>(effectRef);
    }
    const IEffect::Annotation *annotationRef(const char *name) const {
        nvFX::IAnnotation *annotation = valueRef->annotations();
        return effectRef->cacheAnnotationRef(annotation, name);
    }
    const char *name() const {
        return valueRef->getName();
    }
    const char *semantic() const {
        return valueRef->getSemantic();
    }
    Type type() const {
        return toEffectType(valueRef->getType());
    }
    VariableType variableType() const {
        return kUniform;
    }
    void connect(Parameter * /* destinationParameter */) {
    }
    void reset() {
        valueRef = 0;
    }
    void getValue(int &value) const {
        int v = 0;
        valueRef->getValueiv(&v, 1);
        value = v;
    }
    void getValue(float32 &value) const {
        float32 v = 0;
        valueRef->getValuefv(&v, 1);
        value = v;
    }
    void getValue(Vector3 &value) const {
        valueRef->getValuefv(static_cast<float32 *>(&value[0]), 1);
    }
    void getValue(Vector4 &value) const {
        valueRef->getValuefv(static_cast<float32 *>(&value[0]), 1);
    }
    void getMatrix(float32 *value) const {
        valueRef->getValuefv(value, 2, 4);
    }
    void getArrayDimension(int &value) const {
        value = valueRef->getArraySz();
    }
    void getArrayTotalSize(int &value) const {
        value = valueRef->getArraySz();;
    }
    void getTextureRef(intptr_t &value) const {
        value = 0; //cgGLGetTextureParameter(parameter);
    }
    void getSamplerStateRefs(Array<IEffect::SamplerState *> &value) const {
        value.clear();
        const int nstates = m_states.count();
        if (nstates == 0) {
            value.append(m_states.append(new Effect::NvFXSamplerState(this)));
        }
        else {
            value.append(m_states[0]);
        }
    }
    void setValue(bool value) {
        valueRef->setValue1b(value);
    }
    void setValue(int value) {
        valueRef->setValue1i(value);
    }
    void setValue(float32 value) {
        valueRef->setValue1f(value);
    }
    void setValue(const Vector3 &value) {
        const float32 *v = value;
        setFloatVector(const_cast<float32 *>(v));
    }
    void setValue(const Vector4 &value) {
        const float32 *v = value;
        setFloatVector(const_cast<float32 *>(v));
    }
    void setValue(const Vector4 *value) {
        const float32 *v = *value;
        valueRef->setValuefv(const_cast<float32 *>(v), 4);
    }
    void setMatrix(const float32 *value) {
        valueRef->setMatrix4f(const_cast<float32 *>(value));
    }
    void setMatrices(const float32 *value, size_t size) {
        valueRef->setValuefv(const_cast<float32 *>(value), 1, int(size * 16));
    }
    void setSampler(const ITexture *value) {
        GLuint textureID = value ? static_cast<GLuint>(value->data()) : 0;
        nvFX::ResourceType type = detectResourceType(value);
        valueRef->setSamplerResource(type, textureID);
    }
    void setTexture(const ITexture *value) {
        GLuint textureID = value ? static_cast<GLuint>(value->data()) : 0;
        nvFX::ResourceType type = detectResourceType(value);
        valueRef->setImageResource(type, textureID);
    }
    void setTexture(intptr_t value) {
        GLint textureID = static_cast<GLint>(value);
        valueRef->setImageResource(nvFX::RESTEX_2D, textureID);
    }

    void setFloatVector(float *v) {
        switch (valueRef->getType()) {
        case nvFX::TVec2:
            valueRef->setValue2fv(v);
            break;
        case nvFX::TVec3:
            valueRef->setValue3fv(v);
            break;
        case nvFX::TVec4:
            valueRef->setValue4fv(v);
            break;
        default:
            break;
        }
    }

    mutable PointerArray<Effect::NvFXSamplerState> m_states;
    const Effect *effectRef;
    nvFX::IUniform *valueRef;
};

struct Effect::NvFXTechnique : IEffect::Technique {
    NvFXTechnique(const Effect *e, nvFX::ITechnique *p)
        : effectRef(e),
          valueRef(p)
    {
    }
    ~NvFXTechnique() {
        m_passes.releaseAll();
        effectRef = 0;
        valueRef = 0;
    }

    IEffect *parentEffectRef() const {
        return const_cast<Effect *>(effectRef);
    }
    IEffect::Pass *findPass(const char *name) const {
        if (Effect::NvFXPass *const *passPtr = m_passRefsHash.find(name)) {
            return *passPtr;
        }
        return 0;
    }
    const IEffect::Annotation *annotationRef(const char *name) const {
        nvFX::IAnnotation *annotation = valueRef->annotations();
        return effectRef->cacheAnnotationRef(annotation, name);
    }
    const char *name() const {
        return valueRef->getName();
    }
    void getPasses(Array<IEffect::Pass *> &passes) const {
        const int npasses = valueRef->getNumPasses();
        passes.clear();
        for (int i = 0; i < npasses; i++) {
            nvFX::IPass *pass = valueRef->getPass(i);
            if (Effect::NvFXPass *newPass = cachePassRef(pass)) {
                passes.append(newPass);
            }
        }
    }
    void setOverridePass(Pass *pass, bool &rendering) {
        rendering = true;
        int overrideID = 0;
        if (const NvFXPass *v = static_cast<const NvFXPass *>(pass)) {
            const nvFX::PassInfo &info = v->info;
            overrideID = info.overrideID;
            rendering = info.renderingMode == nvFX::RENDER_SCENEGRAPH_SHADED;
        }
        valueRef->setActiveProgramLayer(overrideID);
    }

    Effect::NvFXPass *cachePassRef(nvFX::IPass *pass) const {
        const char *name = pass->getName();
        if (Effect::NvFXPass *const *passPtr = m_passRefsHash.find(name)) {
            return *passPtr;
        }
        else {
            VPVL2_VLOG(2, "Validating a pass: " << pass->getName());
            if (pass->validate()) {
                Effect::NvFXPass *newPassPtr = m_passes.append(new Effect::NvFXPass(effectRef, this, pass));
                m_passRefsHash.insert(name, newPassPtr);
                return newPassPtr;
            }
            VPVL2_VLOG(2, "Validation is failed and cannot use this pass");
        }
        return 0;
    }

    mutable PointerArray<Effect::NvFXPass> m_passes;
    mutable Hash<HashString, Effect::NvFXPass *> m_passRefsHash;
    const Effect *effectRef;
    nvFX::ITechnique *valueRef;
};

bool Effect::isInteractiveParameter(const Parameter *value)
{
    const IEffect::Annotation *name = value->annotationRef("UIName");
    const IEffect::Annotation *widget = value->annotationRef("UIWidget");
    return name && widget;
}

Effect::Effect(EffectContext *contextRef, IApplicationContext *applicationContextRef, nvFX::IContainer *container)
    : enableVertexAttribArray(reinterpret_cast<PFNGLENABLEVERTEXATTRIBARRAYPROC>(applicationContextRef->sharedFunctionResolverInstance()->resolveSymbol("glEnableVertexAttribArray"))),
      disableVertexAttribArray(reinterpret_cast<PFNGLDISABLEVERTEXATTRIBARRAYPROC>(applicationContextRef->sharedFunctionResolverInstance()->resolveSymbol("glDisableVertexAttribArray"))),
      vertexAttribPointer(reinterpret_cast<PFNGLVERTEXATTRIBPOINTERPROC>(applicationContextRef->sharedFunctionResolverInstance()->resolveSymbol("glVertexAttribPointer"))),
      m_applicationContextRef(applicationContextRef),
      m_effectContextRef(contextRef),
      m_container(container),
      m_parentEffectRef(0),
      m_parentFrameBufferObject(0),
      m_scriptOrderType(kStandard)
{
}

Effect::~Effect()
{
    const int numAnnotationHash = m_annotationRefsHash.count();
    for (int i = 0; i < numAnnotationHash; i++) {
        NvFXAnnotationHash **hash = m_annotationRefsHash.value(i);
        (*hash)->releaseAll();
    }
    m_annotationRefsHash.releaseAll();
    m_techniques.releaseAll();
    m_parameters.releaseAll();
    internal::deleteObject(m_parentFrameBufferObject);
    nvFX::IContainer::destroy(m_container);
    m_container = 0;
    m_applicationContextRef = 0;
    m_effectContextRef = 0;
    m_parentEffectRef = 0;
    m_scriptOrderType = kStandard;
}

void Effect::createFrameBufferObject()
{
    internal::deleteObject(m_parentFrameBufferObject);
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
    Util::setRenderColorTargets(m_applicationContextRef->sharedFunctionResolverInstance(), &m_renderColorTargetIndices[0], m_renderColorTargetIndices.count());
}

void Effect::removeRenderColorTargetIndex(int targetIndex)
{
    m_renderColorTargetIndices.remove(targetIndex);
    Util::setRenderColorTargets(m_applicationContextRef->sharedFunctionResolverInstance(), &m_renderColorTargetIndices[0], m_renderColorTargetIndices.count());
}

void Effect::clearRenderColorTargetIndices()
{
    m_renderColorTargetIndices.clear();
    Util::setRenderColorTargets(m_applicationContextRef->sharedFunctionResolverInstance(), 0, 0);
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
    parameters.clear();
    while (nvFX::IUniform *parameter = m_container->findUniform(i)) {
        parameters.append(cacheParameterRef(parameter));
        i++;
    }
}

void Effect::getTechniqueRefs(Array<Technique *> &techniques) const
{
    const int ntechniques = m_container->getNumTechniques();
    techniques.clear();
    for (int i = 0; i < ntechniques; i++) {
        nvFX::ITechnique *technique = m_container->findTechnique(i);
        if (Technique *newTechnique = cacheTechniqueRef(technique)) {
            techniques.append(newTechnique);
        }
    }
}

void Effect::setVertexAttributePointer(VertexAttributeType vtype, Parameter::Type /* ptype */, vsize stride, const void *ptr)
{
    switch (vtype) {
    case kPositionVertexAttribute:
    case kNormalVertexAttribute:
    case kBoneIndexVertexAttribute:
    case kBoneWeightVertexAttribute:
    case kUVA1VertexAttribute:
    case kUVA2VertexAttribute:
    case kUVA3VertexAttribute:
    case kUVA4VertexAttribute:
        vertexAttribPointer(vtype, 4, kGL_FLOAT, kGL_FALSE, int(stride), ptr);
        break;
    case kTextureCoordVertexAttribute:
        vertexAttribPointer(vtype, 2, kGL_FLOAT, kGL_FALSE, int(stride), ptr);
        break;
    default:
        /* do nothing */
        break;
    }
}

void Effect::activateVertexAttribute(VertexAttributeType vtype)
{
    enableVertexAttribArray(vtype);
}

void Effect::deactivateVertexAttribute(VertexAttributeType vtype)
{
    disableVertexAttribArray(vtype);
}

const char *Effect::errorString() const
{
    return 0;
}

IApplicationContext *Effect::applicationContextRef() const
{
    return m_applicationContextRef;
}

IEffect::Annotation *Effect::cacheAnnotationRef(nvFX::IAnnotation *annotation, const char *name) const
{
    if (NvFXAnnotationHash *const *annotationHashPtr = m_annotationRefsHash.find(annotation)) {
        if (NvFXAnnotation *const *annotationPtr = (*annotationHashPtr)->find(name)) {
            return *annotationPtr;
        }
    }
    if (annotation->getAnnotationFloat(name) != 0.0f ||
            annotation->getAnnotationInt(name) != 0 ||
            annotation->getAnnotationString(name)) {
        NvFXAnnotation *newAnnotationPtr = 0;
        NvFXAnnotationHash *annotationHashPtr = 0;
        if (NvFXAnnotationHash *const *annotationHash = m_annotationRefsHash.find(annotation)) {
            annotationHashPtr = *const_cast<NvFXAnnotationHash **>(annotationHash);
        }
        else {
            annotationHashPtr = m_annotationRefsHash.insert(annotation, new NvFXAnnotationHash());
        }
        newAnnotationPtr = annotationHashPtr->insert(name, new NvFXAnnotation(this, annotation, name));
        return newAnnotationPtr;
    }
    return 0;
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
    NvFXTechnique *techniqueRef = 0;
    if (NvFXTechnique *const *techniquePtr = m_techniqueRefsHash.find(technique)) {
        techniqueRef = *techniquePtr;
    }
    else {
        pushAnnotationGroup(std::string("Effect#cacheTechniqueRef name=").append(technique->getName()).c_str(), m_applicationContextRef->sharedFunctionResolverInstance());
        VPVL2_VLOG(2, "Validating a technique: " << technique->getName());
        if (technique->validate()) {
            technique->bindAttribute("vpvl2_inPosition",    IEffect::kPositionVertexAttribute);
            technique->bindAttribute("vpvl2_inNormal",      IEffect::kNormalVertexAttribute);
            technique->bindAttribute("vpvl2_inTexCoord",    IEffect::kTextureCoordVertexAttribute);
            technique->bindAttribute("vpvl2_inBoneIndices", IEffect::kBoneIndexVertexAttribute);
            technique->bindAttribute("vpvl2_inBoneWeights", IEffect::kBoneWeightVertexAttribute);
            technique->bindAttribute("vpvl2_inUVA1",        IEffect::kUVA1VertexAttribute);
            technique->bindAttribute("vpvl2_inUVA2",        IEffect::kUVA2VertexAttribute);
            technique->bindAttribute("vpvl2_inUVA3",        IEffect::kUVA3VertexAttribute);
            technique->bindAttribute("vpvl2_inUVA4",        IEffect::kUVA4VertexAttribute);
            technique->bindAttributes();
            NvFXTechnique *newTechniquePtr = m_techniques.append(new NvFXTechnique(this, technique));
            m_techniqueRefsHash.insert(technique, newTechniquePtr);
            techniqueRef = newTechniquePtr;
        }
        else {
            VPVL2_VLOG(2, "Validation is failed and cannot use this technique");
        }
        popAnnotationGroup(m_applicationContextRef->sharedFunctionResolverInstance());
    }
    return techniqueRef;
}

} /* namespace nvfx */
} /* namespace vpvl2 */
