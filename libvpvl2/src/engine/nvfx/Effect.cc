/**

 Copyright (c) 2010-2014  hkrn

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
#include <FxLib.h>
#include <FxParser.h>

namespace vpvl2
{
namespace VPVL2_VERSION_NS
{
namespace nvfx
{

using namespace gl;
using namespace extensions::fx;

static const char *kOverrideGroupNames[] = {
    "vertexGroupObject",
    "vertexGroupEdge",
    "vertexGroupShadow",
    "vertexGroupZplot",
    "fragmentGroupObject",
    "fragmentGroupEdge",
    "fragmentGroupShadow",
    "fragmentGroupZplot"
};
static const int kNumOverrideGroupNames = sizeof(kOverrideGroupNames) / sizeof(kOverrideGroupNames[0]);

template<typename TAttributeBindable>
static void internalBindAttributes(TAttributeBindable *value)
{
    value->bindAttribute("vpvl2_inPosition",    IEffect::kPositionVertexAttribute);
    value->bindAttribute("vpvl2_inNormal",      IEffect::kNormalVertexAttribute);
    value->bindAttribute("vpvl2_inTexCoord",    IEffect::kTextureCoordVertexAttribute);
    value->bindAttribute("vpvl2_inBoneIndices", IEffect::kBoneIndexVertexAttribute);
    value->bindAttribute("vpvl2_inBoneWeights", IEffect::kBoneWeightVertexAttribute);
    value->bindAttribute("vpvl2_inUVA1",        IEffect::kUVA1VertexAttribute);
    value->bindAttribute("vpvl2_inUVA2",        IEffect::kUVA2VertexAttribute);
    value->bindAttribute("vpvl2_inUVA3",        IEffect::kUVA3VertexAttribute);
    value->bindAttribute("vpvl2_inUVA4",        IEffect::kUVA4VertexAttribute);
    value->bindAttributes();
}

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

static void appendShaderHeader(nvFX::IContainer *container, const IApplicationContext::FunctionResolver *resolver)
{
    static const char kAppendingShaderHeader[] =
            "#if defined(GL_ES) || __VERSION__ >= 150\n"
            "precision highp float;\n"
            "#else\n"
            "#define highp\n"
            "#define mediump\n"
            "#define lowp\n"
            "#endif\n"
            "#if __VERSION__ >= 130\n"
            "#define vpvl2FXGetTexturePixel2D(samp, uv) texture(samp, (uv))\n"
            "#else\n"
            "#define vpvl2FXGetTexturePixel2D(samp, uv) texture2D(samp, (uv))\n"
            "#define layout(expr)\n"
            "#endif\n"
            "#if __VERSION__ >= 400\n"
            "#define vpvl2FXFMA(v, m, a) fma((v), (m), (a))\n"
            "#else\n"
            "#define vpvl2FXFMA(v, m, a) ((v) * (m) + (a))\n"
            "#endif\n"
            "#define vpvl2FXSaturate(v) clamp((v), 0, 1)\n"
            ;
    char appendingHeader[1024];
    int version = resolver->query(IApplicationContext::FunctionResolver::kQueryShaderVersion);
    if (resolver->query(IApplicationContext::FunctionResolver::kQueryCoreProfile) != 0) {
        static const char kFormat[] = "#version %d core\n%s";
        internal::snprintf(appendingHeader, sizeof(appendingHeader), kFormat, version, kAppendingShaderHeader);
    }
    else {
        static const char kFormat[] = "#version %d\n%s";
        internal::snprintf(appendingHeader, sizeof(appendingHeader), kFormat, version, kAppendingShaderHeader);
    }
    int i = 0;
    while (nvFX::IShader *shader = container->findShader(i++)) {
        nvFX::TargetType type = shader->getType();
        const char *name = shader->getName();
        if (*name == '\0' && type == nvFX::TGLSL) {
            shader->getExInterface()->addHeaderCode(appendingHeader);
        }
    }
}

struct Effect::NvFXAnnotation : IEffect::Annotation {
    static const char *kEmpty;

    NvFXAnnotation(const Effect *e, nvFX::IAnnotation *a, const char *n)
        : m_effect(e),
          m_name(n),
          m_annotation(a),
          m_fval(0),
          m_ival(0)
    {
    }
    ~NvFXAnnotation() {
        m_effect = 0;
        m_annotation = 0;
    }

    bool booleanValue() const {
        return m_annotation->getAnnotationInt(m_name.c_str()) == 1;
    }
    int integerValue() const {
        return m_annotation->getAnnotationInt(m_name.c_str());
    }
    const int *integerValues(int *size) const {
        *size = 1;
        m_ival = m_annotation->getAnnotationInt(m_name.c_str());
        return &m_ival;
    }
    float floatValue() const {
        return m_annotation->getAnnotationFloat(m_name.c_str());
    }
    const float *floatValues(int *size) const {
        *size = 1;
        m_fval = m_annotation->getAnnotationFloat(m_name.c_str());
        return &m_fval;
    }
    const char *stringValue() const {
        const char *value = m_annotation->getAnnotationString(m_name.c_str());
        return value ? value : kEmpty;
    }

    const Effect *m_effect;
    const std::string m_name;
    nvFX::IAnnotation *m_annotation;
    mutable float m_fval;
    mutable int m_ival;
};

const char *Effect::NvFXAnnotation::kEmpty = "";

struct Effect::NvFXPass : IEffect::Pass {
    NvFXPass(const Effect *e, const IEffect::Technique *t, nvFX::IPass *a)
        : m_effectRef(e),
          m_techniqueRef(t),
          m_valueRef(a),
          m_hasOverride(false)
    {
        for (int i = 0; i < kNumOverrideGroupNames; i++) {
            m_hasOverride = m_hasOverride || a->findStateOverride(kOverrideGroupNames[i]);
        }
        internal::zerofill(&m_info, sizeof(m_info));
    }
    ~NvFXPass() {
        m_effectRef = 0;
        m_techniqueRef = 0;
        m_valueRef = 0;
    }

    static void castPasses(const Array<Pass *> &srcPasses, Array<nvFX::IPass *> &destPasses) {
        const int npasses = srcPasses.count();
        destPasses.clear();
        destPasses.reserve(npasses);
        for (int i = 0; i < npasses; i++) {
            NvFXPass *pass = static_cast<NvFXPass *>(srcPasses[i]);
            destPasses.append(pass->m_valueRef);
        }
    }

    IEffect::Technique *parentTechniqueRef() const {
        return const_cast<IEffect::Technique *>(m_techniqueRef);
    }
    const IEffect::Annotation *annotationRef(const char *name) const {
        nvFX::IAnnotation *annotations = m_valueRef->annotations();
        return m_effectRef->cacheAnnotationRef(annotations, name);
    }
    const char *name() const {
        return m_valueRef->getName();
    }
    void setState() {
        pushAnnotationGroup(std::string("NvFXPass#setState name=").append(name()).c_str(), m_effectRef->applicationContextRef());
        m_valueRef->execute(&m_info);
        popAnnotationGroup(m_effectRef->applicationContextRef());
    }
    void resetState() {
        m_valueRef->unbindProgram();
    }
    bool isRenderable() const {
        return m_info.renderingMode == nvFX::RENDER_SCENEGRAPH_SHADED;
    }
    void setupOverrides(const IEffect *effectRef) {
        Array<IEffect::Technique *> techniques;
        Array<IEffect::Pass *> passes;
        effectRef->getTechniqueRefs(techniques);
        const int ntechniques = techniques.count();
        for (int i = 0; i < ntechniques; i++) {
            IEffect::Technique *technique = techniques[i];
            technique->getPasses(passes);
            setupOverrides(passes);
        }
    }
    void setupOverrides(const Array<Pass *> &passes) {
        if (m_hasOverride && passes.count() > 0) {
            pushAnnotationGroup(std::string("NvFXPass#setupOverrides name=").append(name()).c_str(), m_effectRef->applicationContextRef());
            Array<nvFX::IPass *> overridePasses;
            castPasses(passes, overridePasses);
            const int numOverridePasses = overridePasses.count();
             m_valueRef->setupOverrides(&overridePasses[0], numOverridePasses);
            for (int i = 0; i < numOverridePasses; i++) {
                nvFX::IPass *passRef = overridePasses[i];
                passRef->validate();
                internalBindAttributes(passRef);
                VPVL2_VLOG(2, "Setup pass override: target=" << passRef->getName() << " source=" << name());
            }
            popAnnotationGroup(m_effectRef->applicationContextRef());
        }
    }
    void releaseOverrides(const Array<Pass *> &passes) {
        pushAnnotationGroup(std::string("NvFXPass#releaseOverrides name=").append(name()).c_str(), m_effectRef->applicationContextRef());
        if (passes.count() > 0) {
            Array<nvFX::IPass *> castedPasses;
            castPasses(passes, castedPasses);
            m_valueRef->releaseOverrides(&castedPasses[0], castedPasses.count());
        }
        popAnnotationGroup(m_effectRef->applicationContextRef());
    }

    const Effect *m_effectRef;
    const IEffect::Technique *m_techniqueRef;
    nvFX::IPass *m_valueRef;
    nvFX::PassInfo m_info;
    bool m_hasOverride;
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
        : m_effectRef(e),
          m_valueRef(p)
    {
    }
    ~NvFXParameter() {
        m_states.releaseAll();
        m_effectRef = 0;
        m_valueRef = 0;
    }

    IEffect *parentEffectRef() const {
        return const_cast<Effect *>(m_effectRef);
    }
    const IEffect::Annotation *annotationRef(const char *name) const {
        nvFX::IAnnotation *annotation = m_valueRef->annotations();
        return m_effectRef->cacheAnnotationRef(annotation, name);
    }
    const char *name() const {
        return m_valueRef->getName();
    }
    const char *semantic() const {
        return m_valueRef->getSemantic();
    }
    Type type() const {
        return toEffectType(m_valueRef->getType());
    }
    VariableType variableType() const {
        return kUniform;
    }
    void connect(Parameter * /* destinationParameter */) {
    }
    void reset() {
        m_valueRef = 0;
    }
    void getValue(int &value) const {
        int v = 0;
        m_valueRef->getValueiv(&v, 1);
        value = v;
    }
    void getValue(float32 &value) const {
        float32 v = 0;
        m_valueRef->getValuefv(&v, 1);
        value = v;
    }
    void getValue(Vector3 &value) const {
        m_valueRef->getValuefv(static_cast<float32 *>(&value[0]), 1);
    }
    void getValue(Vector4 &value) const {
        m_valueRef->getValuefv(static_cast<float32 *>(&value[0]), 1);
    }
    void getMatrix(float32 *value) const {
        m_valueRef->getValuefv(value, 2, 4);
    }
    void getArrayDimension(int &value) const {
        value = m_valueRef->getArraySz();
    }
    void getArrayTotalSize(int &value) const {
        value = m_valueRef->getArraySz();;
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
        m_valueRef->setValue1b(value);
    }
    void setValue(int value) {
        m_valueRef->setValue1i(value);
    }
    void setValue(float32 value) {
        m_valueRef->setValue1f(value);
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
        m_valueRef->setValuefv(const_cast<float32 *>(v), 4);
    }
    void setMatrix(const float32 *value) {
        m_valueRef->setMatrix4f(const_cast<float32 *>(value));
    }
    void setMatrices(const float32 *value, size_t size) {
        m_valueRef->setValuefv(const_cast<float32 *>(value), 1, int(size * 16));
    }
    void setSampler(const ITexture *value) {
        GLuint textureID = value ? static_cast<GLuint>(value->data()) : 0;
        nvFX::ResourceType type = detectResourceType(value);
        m_valueRef->setSamplerResource(type, textureID);
    }
    void setTexture(const ITexture *value) {
        GLuint textureID = value ? static_cast<GLuint>(value->data()) : 0;
        nvFX::ResourceType type = detectResourceType(value);
        m_valueRef->setImageResource(type, textureID);
    }
    void setTexture(intptr_t value) {
        GLint textureID = static_cast<GLint>(value);
        m_valueRef->setImageResource(nvFX::RESTEX_2D, textureID);
    }

    void setFloatVector(float *v) {
        switch (m_valueRef->getType()) {
        case nvFX::IUniform::TVec2:
            m_valueRef->setValue2fv(v);
            break;
        case nvFX::IUniform::TVec3:
            m_valueRef->setValue3fv(v);
            break;
        case nvFX::IUniform::TVec4:
            m_valueRef->setValue4fv(v);
            break;
        default:
            break;
        }
    }

    mutable PointerArray<Effect::NvFXSamplerState> m_states;
    const Effect *m_effectRef;
    nvFX::IUniform *m_valueRef;
};

struct Effect::NvFXTechnique : IEffect::Technique {
    NvFXTechnique(const Effect *e, nvFX::ITechnique *p)
        : m_effectRef(e),
          m_valueRef(p)
    {
    }
    ~NvFXTechnique() {
        m_passes.releaseAll();
        m_overrideUniforms.releaseAll();
        m_effectRef = 0;
        m_valueRef = 0;
    }

    IEffect *parentEffectRef() const {
        return const_cast<Effect *>(m_effectRef);
    }
    IEffect::Pass *findPass(const char *name) const {
        if (Effect::NvFXPass *const *passPtr = m_passRefsHash.find(name)) {
            return *passPtr;
        }
        return 0;
    }
    const IEffect::Annotation *annotationRef(const char *name) const {
        nvFX::IAnnotation *annotation = m_valueRef->annotations();
        return m_effectRef->cacheAnnotationRef(annotation, name);
    }
    const char *name() const {
        return m_valueRef->getName();
    }
    void getPasses(Array<IEffect::Pass *> &passes) const {
        const int npasses = m_valueRef->getNumPasses();
        passes.clear();
        passes.reserve(npasses);
        for (int i = 0; i < npasses; i++) {
            nvFX::IPass *pass = m_valueRef->getPass(i);
            if (Effect::NvFXPass *newPass = cachePassRef(pass)) {
                passes.append(newPass);
            }
        }
    }
    void getOverridePasses(Array<IEffect::Pass *> &passes) const {
        const int npasses = m_overridePasses.count();
        passes.clear();
        passes.reserve(npasses);
        for (int i = 0; i < npasses; i++) {
            IEffect::Pass *pass = m_overridePasses[i];
            passes.append(pass);
        }
    }
    void setOverridePass(IEffect::Pass *sourcePass) {
        pushAnnotationGroup(std::string("NvFXTechnique#setOverridePass name=").append(name()).c_str(), m_effectRef->applicationContextRef());
        if (Effect::NvFXPass *v = static_cast<Effect::NvFXPass *>(sourcePass)) {
            int overrideID = v->m_info.overrideID;
            m_valueRef->setActiveProgramLayer(overrideID);
            const int npasses = m_passes.count();
            nvFX::IContainer *container = static_cast<nvFX::IContainer *>(v->m_effectRef->internalPointer());
            EffectContext::disableMessageCallback();
            for (int i = 0; i < npasses; i++) {
                nvFX::IPassEx *passEx = m_passes[i]->m_valueRef->getExInterface();
                pushAnnotationGroup(std::string("NvFXTechnique#setProgramUniforms name=").append(passEx->getName()).c_str(), m_effectRef->applicationContextRef());
                if (nvFX::IProgramPipeline *pipeline = passEx->getProgramPipeline(overrideID)) {
                    int index = 0;
                    while (nvFX::IProgram *program = pipeline->getShaderProgram(index++)) {
                        setProgramUniforms(program, container, true);
                    }
                }
                else if (nvFX::IProgram *program = passEx->getProgram(overrideID)) {
                    setProgramUniforms(program, container, false);
                }
                popAnnotationGroup(m_effectRef->applicationContextRef());
            }
            EffectContext::enableMessageCallback();
        }
        else {
            m_valueRef->setActiveProgramLayer(0);
        }
        popAnnotationGroup(m_effectRef->applicationContextRef());
    }
    void setProgramUniforms(nvFX::IProgram *program, nvFX::IContainer *container, bool isPipeline) {
        int index = 0;
        const IApplicationContext::FunctionResolver *resolver = m_effectRef->applicationContextRef()->sharedFunctionResolverInstance();
        nvFX::IProgramEx *programEx = program->getExInterface();
        const int programID = programEx->getProgram();
        programEx->bind(container);
        while (nvFX::IUniform *uniform = container->findUniform(index++)) {
            GLint location = findOverrideUniformLocation(uniform, programID);
            if (location < 0) {
                continue;
            }
            nvFX::IUniform::Type type = uniform->getType();
            switch (type) {
            case nvFX::IUniform::TBool:
            case nvFX::IUniform::TBool2:
            case nvFX::IUniform::TBool3:
            case nvFX::IUniform::TBool4: {
                int iv[4] = { 0, 0, 0, 0 }, count = (type - nvFX::IUniform::TBool) + 1;
                bool bv[4] = { false, false, false, false };
                uniform->getValuebv(bv, count);
                for (int i = 0; i < 4; i++) {
                    iv[i] = static_cast<int>(bv[i]);
                }
                setUniformInt(iv, resolver, programID, location, count, isPipeline);
                break;
            }
            case nvFX::IUniform::TFloat:
            case nvFX::IUniform::TVec2:
            case nvFX::IUniform::TVec3:
            case nvFX::IUniform::TVec4: {
                float v[4] = { 0, 0, 0, 0 };
                int count = (type - nvFX::IUniform::TFloat) + 1;
                uniform->getValuefv(v, count);
                setUniformFloat(v, resolver, programID, location, count, isPipeline);
                break;
            }
            case nvFX::IUniform::TInt:
            case nvFX::IUniform::TInt2:
            case nvFX::IUniform::TInt3:
            case nvFX::IUniform::TInt4: {
                int v[4] = { 0, 0, 0, 0 }, count = (type - nvFX::IUniform::TInt) + 1;
                uniform->getValueiv(v, count);
                setUniformInt(v, resolver, programID, location, count, isPipeline);
                break;
            }
            case nvFX::IUniform::TMat2: {
                float v[4];
                uniform->getValueRaw(v, sizeof(v));
                if (isPipeline) {
                    typedef void (GLAPIENTRY * PFNGLPROGRAMUNIFORMMATRIX2FVPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
                    reinterpret_cast<PFNGLPROGRAMUNIFORMMATRIX2FVPROC>(resolver->resolveSymbol("glProgramUniform2iv"))(programID, location, 1, kGL_FALSE, v);
                }
                else {
                    typedef void (GLAPIENTRY * PFNGLUNIFORMMATRIX2FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
                    reinterpret_cast<PFNGLUNIFORMMATRIX2FVPROC>(resolver->resolveSymbol("glUniformMatrix2fv"))(location, 1, kGL_FALSE, v);
                }
                break;
            }
            case nvFX::IUniform::TMat3: {
                float v[9];
                uniform->getValueRaw(v, sizeof(v));
                if (isPipeline) {
                    typedef void (GLAPIENTRY * PFNGLPROGRAMUNIFORMMATRIX3FVPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
                    reinterpret_cast<PFNGLPROGRAMUNIFORMMATRIX3FVPROC>(resolver->resolveSymbol("glProgramUniform3iv"))(programID, location, 1, kGL_FALSE, v);
                }
                else {
                    typedef void (GLAPIENTRY * PFNGLUNIFORMMATRIX3FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
                    reinterpret_cast<PFNGLUNIFORMMATRIX3FVPROC>(resolver->resolveSymbol("glUniformMatrix3fv"))(location, 1, kGL_FALSE, v);
                }
                break;
            }
            case nvFX::IUniform::TMat4: {
                float v[16];
                uniform->getValueRaw(v, sizeof(v));
                if (isPipeline) {
                    typedef void (GLAPIENTRY * PFNGLPROGRAMUNIFORMMATRIX4FVPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
                    reinterpret_cast<PFNGLPROGRAMUNIFORMMATRIX4FVPROC>(resolver->resolveSymbol("glProgramUniform4iv"))(programID, location, 1, kGL_FALSE, v);
                }
                else {
                    typedef void (GLAPIENTRY * PFNGLUNIFORMMATRIX4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
                    reinterpret_cast<PFNGLUNIFORMMATRIX4FVPROC>(resolver->resolveSymbol("glUniformMatrix4fv"))(location, 1, kGL_FALSE, v);
                }
                break;
            }
            default:
                break;
            }
        }
        programEx->unbind(container);
    }
    static void setUniformFloat(const float *v, const IApplicationContext::FunctionResolver *resolver, GLint programID, GLint location, int count, bool isPipeline) {
        switch (count) {
        case 1:
            if (isPipeline) {
                typedef void (GLAPIENTRY * PFNGLPROGRAMUNIFORM1FVPROC) (GLuint program, GLint location, GLsizei count, const GLfloat* value);
                reinterpret_cast<PFNGLPROGRAMUNIFORM1FVPROC>(resolver->resolveSymbol("glProgramUniform1fv"))(programID, location, 1, v);
            }
            else {
                typedef void (GLAPIENTRY * PFNGLUNIFORM1FVPROC) (GLint location, GLsizei count, const GLfloat* value);
                reinterpret_cast<PFNGLUNIFORM1FVPROC>(resolver->resolveSymbol("glUniform1fv"))(location, 1, v);
            }
            break;
        case 2:
            if (isPipeline) {
                typedef void (GLAPIENTRY * PFNGLPROGRAMUNIFORM2FVPROC) (GLuint program, GLint location, GLsizei count, const GLfloat* value);
                reinterpret_cast<PFNGLPROGRAMUNIFORM2FVPROC>(resolver->resolveSymbol("glProgramUniform2fv"))(programID, location, 1, v);
            }
            else {
                typedef void (GLAPIENTRY * PFNGLUNIFORM2FVPROC) (GLint location, GLsizei count, const GLfloat* value);
                reinterpret_cast<PFNGLUNIFORM2FVPROC>(resolver->resolveSymbol("glUniform2fv"))(location, 1, v);
            }
            break;
        case 3:
            if (isPipeline) {
                typedef void (GLAPIENTRY * PFNGLPROGRAMUNIFORM3FVPROC) (GLuint program, GLint location, GLsizei count, const GLfloat* value);
                reinterpret_cast<PFNGLPROGRAMUNIFORM3FVPROC>(resolver->resolveSymbol("glProgramUniform3fv"))(programID, location, 1, v);
            }
            else {
                typedef void (GLAPIENTRY * PFNGLUNIFORM3FVPROC) (GLint location, GLsizei count, const GLfloat* value);
                reinterpret_cast<PFNGLUNIFORM3FVPROC>(resolver->resolveSymbol("glUniform3fv"))(location, 1, v);
            }
            break;
        case 4:
            if (isPipeline) {
                typedef void (GLAPIENTRY * PFNGLPROGRAMUNIFORM4FVPROC) (GLuint program, GLint location, GLsizei count, const GLfloat* value);
                reinterpret_cast<PFNGLPROGRAMUNIFORM4FVPROC>(resolver->resolveSymbol("glProgramUniform4fv"))(programID, location, 1, v);
            }
            else {
                typedef void (GLAPIENTRY * PFNGLUNIFORM4FVPROC) (GLint location, GLsizei count, const GLfloat* value);
                reinterpret_cast<PFNGLUNIFORM4FVPROC>(resolver->resolveSymbol("glUniform4fv"))(location, 1, v);
            }
            break;
        default:
            break;
        }
    }
    static void setUniformInt(const int *v, const IApplicationContext::FunctionResolver *resolver, GLint programID, GLint location, int count, bool isPipeline) {
        switch (count) {
        case 1:
            if (isPipeline) {
                typedef void (GLAPIENTRY * PFNGLPROGRAMUNIFORM1IVPROC) (GLuint program, GLint location, GLsizei count, const GLint* value);
                reinterpret_cast<PFNGLPROGRAMUNIFORM1IVPROC>(resolver->resolveSymbol("glProgramUniform1iv"))(programID, location, 1, v);
            }
            else {
                typedef void (GLAPIENTRY * PFNGLUNIFORM1IVPROC) (GLint location, GLsizei count, const GLint* value);
                reinterpret_cast<PFNGLUNIFORM1IVPROC>(resolver->resolveSymbol("glUniform1iv"))(location, 1, v);
            }
            break;
        case 2:
            if (isPipeline) {
                typedef void (GLAPIENTRY * PFNGLPROGRAMUNIFORM2IVPROC) (GLuint program, GLint location, GLsizei count, const GLint* value);
                reinterpret_cast<PFNGLPROGRAMUNIFORM2IVPROC>(resolver->resolveSymbol("glProgramUniform2iv"))(programID, location, 1, v);
            }
            else {
                typedef void (GLAPIENTRY * PFNGLUNIFORM2IVPROC) (GLint location, GLsizei count, const GLint* value);
                reinterpret_cast<PFNGLUNIFORM2IVPROC>(resolver->resolveSymbol("glUniform2iv"))(location, 1, v);
            }
            break;
        case 3:
            if (isPipeline) {
                typedef void (GLAPIENTRY * PFNGLPROGRAMUNIFORM3IVPROC) (GLuint program, GLint location, GLsizei count, const GLint* value);
                reinterpret_cast<PFNGLPROGRAMUNIFORM3IVPROC>(resolver->resolveSymbol("glProgramUniform3iv"))(programID, location, 1, v);
            }
            else {
                typedef void (GLAPIENTRY * PFNGLUNIFORM3IVPROC) (GLint location, GLsizei count, const GLint* value);
                reinterpret_cast<PFNGLUNIFORM3IVPROC>(resolver->resolveSymbol("glUniform3iv"))(location, 1, v);
            }
            break;
        case 4:
            if (isPipeline) {
                typedef void (GLAPIENTRY * PFNGLPROGRAMUNIFORM4IVPROC) (GLuint program, GLint location, GLsizei count, const GLint* value);
                reinterpret_cast<PFNGLPROGRAMUNIFORM4IVPROC>(resolver->resolveSymbol("glProgramUniform4iv"))(programID, location, 1, v);
            }
            else {
                typedef void (GLAPIENTRY * PFNGLUNIFORM4IVPROC) (GLint location, GLsizei count, const GLint* value);
                reinterpret_cast<PFNGLUNIFORM4IVPROC>(resolver->resolveSymbol("glUniform4iv"))(location, 1, v);
            }
            break;
        default:
            break;
        }
    }
    GLint findOverrideUniformLocation(nvFX::IUniform *uniform, int programID) {
        UniformLocation *uniformLocation = 0;
        if (UniformLocation *const *uniformLocationPtr = m_overrideUniforms.find(programID)) {
            const char *name = uniform->getName();
            uniformLocation = *uniformLocationPtr;
            if (const GLint *location = uniformLocation->find(name)) {
                return *location;
            }
        }
        else {
            uniformLocation = m_overrideUniforms.insert(programID, new UniformLocation());
        }
        const IApplicationContext::FunctionResolver *resolver = m_effectRef->applicationContextRef()->sharedFunctionResolverInstance();
        const char *name = uniform->getName();
        typedef GLint (GLAPIENTRY * PFNGLGETUNIFORMLOCATIONPROC) (GLuint program, const GLchar* name);
        GLint location = reinterpret_cast<PFNGLGETUNIFORMLOCATIONPROC>(resolver->resolveSymbol("glGetUniformLocation"))(programID, name);
        uniformLocation->insert(name, location);
        return location;
    }

    Effect::NvFXPass *cachePassRef(nvFX::IPass *pass) const {
        const char *name = pass->getName();
        if (Effect::NvFXPass *const *passPtr = m_passRefsHash.find(name)) {
            return *passPtr;
        }
        else {
            VPVL2_VLOG(2, "Validating a pass: " << pass->getName());
            if (pass->validate()) {
                Effect::NvFXPass *newPassPtr = m_passes.append(new Effect::NvFXPass(m_effectRef, this, pass));
                if (newPassPtr->m_hasOverride) {
                    m_overridePasses.append(newPassPtr);
                }
                m_passRefsHash.insert(name, newPassPtr);
                return newPassPtr;
            }
            VPVL2_VLOG(2, "Validation is failed and cannot use this pass");
        }
        return 0;
    }

    mutable PointerArray<Effect::NvFXPass> m_passes;
    mutable Array<Effect::NvFXPass *> m_overridePasses;
    mutable Hash<HashString, Effect::NvFXPass *> m_passRefsHash;
    Hash<HashString, GLuint> m_uniformLocationsCache;
    typedef Hash<HashString, int> UniformLocation;
    PointerHash<HashInt, UniformLocation> m_overrideUniforms;
    const Effect *m_effectRef;
    nvFX::ITechnique *m_valueRef;
};

bool Effect::isInteractiveParameter(const Parameter *value)
{
    VPVL2_CHECK(value);
    const IEffect::Annotation *name = value->annotationRef("UIName");
    const IEffect::Annotation *widget = value->annotationRef("UIWidget");
    return name && widget;
}

Effect::Effect(EffectContext *contextRef, IApplicationContext *applicationContextRef, nvFX::IContainer *container, const IString *pathRef)
    : enableVertexAttribArray(reinterpret_cast<PFNGLENABLEVERTEXATTRIBARRAYPROC>(applicationContextRef->sharedFunctionResolverInstance()->resolveSymbol("glEnableVertexAttribArray"))),
      disableVertexAttribArray(reinterpret_cast<PFNGLDISABLEVERTEXATTRIBARRAYPROC>(applicationContextRef->sharedFunctionResolverInstance()->resolveSymbol("glDisableVertexAttribArray"))),
      vertexAttribPointer(reinterpret_cast<PFNGLVERTEXATTRIBPOINTERPROC>(applicationContextRef->sharedFunctionResolverInstance()->resolveSymbol("glVertexAttribPointer"))),
      m_applicationContextRef(applicationContextRef),
      m_effectContextRef(contextRef),
      m_container(container),
      m_path(0),
      m_name(0),
      m_texturePathPtr(0),
      m_parentEffectRef(0),
      m_parentFrameBufferObject(0),
      m_scriptOrderType(kStandard),
      m_enabled(true),
      m_dirty(false)
{
    m_path = pathRef ? pathRef->clone() : 0;
    appendShaderHeader(container, applicationContextRef->sharedFunctionResolverInstance());
    uploadTextureResources();
}

Effect::~Effect()
{
    release();
    internal::deleteObject(m_path);
    internal::deleteObject(m_name);
    internal::deleteObject(m_texturePathPtr);
    m_textureResources.releaseAll();
    m_name = 0;
    m_applicationContextRef = 0;
    m_effectContextRef = 0;
    m_parentEffectRef = 0;
    m_scriptOrderType = kStandard;
    m_enabled = false;
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
    return m_effectContextRef;
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

void Effect::validate()
{
    Array<Technique *> techniqueRefs;
    Array<Pass *> passRefs;
    getTechniqueRefs(techniqueRefs);
    const int ntechniques = techniqueRefs.count();
    for (int i = 0; i < ntechniques; i++) {
        Technique *techinque = techniqueRefs[i];
        techinque->getPasses(passRefs);
    }
}

void Effect::setupOverride(const IEffect *effectRef)
{
    Array<IEffect::Technique *> techniques;
    Array<IEffect::Pass *> passes;
    effectRef->getTechniqueRefs(techniques);
    const int ntechniques = techniques.count();
    for (int i = 0; i < ntechniques; i++) {
        IEffect::Technique *technique = techniques[i];
        technique->getPasses(passes);
        const int npasses = passes.count();
        for (int j = 0; j < npasses; j++) {
            IEffect::Pass *pass = passes[j];
            pass->setupOverrides(this);
        }
    }
}

const IString *Effect::path() const
{
    return m_path;
}

const IString *Effect::name() const
{
    return m_name;
}

void Effect::setName(const IString *value)
{
    internal::setString(value, m_name);
    m_container->setName(reinterpret_cast<const char *>(value->toByteArray()));
}

bool Effect::isEnabled() const
{
    return m_enabled;
}

void Effect::setEnabled(bool value)
{
    m_enabled = value;
}

bool Effect::recompileFromFile(const char *filePath)
{
    pushAnnotationGroupWithName("Effect#recompileFromFile");
    nvFX::IContainer *container = nvFX::IContainer::create();
    bool ret = false;
    if (nvFX::loadEffectFromFile(container, filePath)) {
        VPVL2_VLOG(1, "Recompiled the effect succeeded: " << filePath);
        release();
        resetEffect(container);
        ret = true;
    }
    else {
        VPVL2_LOG(WARNING, "Recompiling the effect failed: " << filePath);
        nvFX::IContainer::destroy(container);
    }
    gl::popAnnotationGroup(m_applicationContextRef);
    return ret;
}

bool Effect::recompileFromSource(const char *source, int /* length */)
{
    pushAnnotationGroupWithName("Effect#recompileFromSource");
    nvFX::IContainer *container = nvFX::IContainer::create();
    bool ret = false;
    if (nvFX::loadEffect(container, source)) {
        release();
        resetEffect(container);
        ret = true;
    }
    else {
        nvFX::IContainer::destroy(container);
    }
    gl::popAnnotationGroup(m_applicationContextRef);
    return ret;
}

bool Effect::isDirty() const
{
    return m_dirty;
}

void Effect::setDirty(bool value)
{
    m_dirty = value;
}

const char *Effect::errorString() const
{
    return 0;
}

IApplicationContext *Effect::applicationContextRef() const
{
    return m_applicationContextRef;
}

void Effect::uploadTextureResources()
{
    m_textureResources.releaseAll();
    for (int i = 0; nvFX::IResource *resource = m_container->findResource(i); i++) {
        nvFX::IAnnotation *annotation = resource->annotations();
        if (const char *name = annotation->getAnnotationString("defaultFile")) {
            m_texturePathPtr = m_applicationContextRef->toUnicode(reinterpret_cast<const uint8 *>(name));
            if (ITexture *texturePtr = m_applicationContextRef->uploadEffectTexture(m_texturePathPtr, this)) {
                VPVL2_VLOG(1, "Created the effect texture resource: name=" << name << " ID=" << texturePtr->data());
                resource->setGLTexture(texturePtr->data());
                m_textureResources.append(texturePtr);
            }
            else {
                VPVL2_LOG(WARNING, "Cannot create the effect texture resource: name=" << name);
            }
            internal::deleteObject(m_texturePathPtr);
        }
    }
}

void Effect::release()
{
    if (m_container) {
        pushAnnotationGroupWithName("Effect#release");
        m_enabled = false;
        const int numAnnotationHash = m_annotationRefsHash.count();
        for (int i = 0; i < numAnnotationHash; i++) {
            NvFXAnnotationHash **hash = m_annotationRefsHash.value(i);
            (*hash)->releaseAll();
        }
        m_annotationRefsHash.releaseAll();
        m_techniques.releaseAll();
        m_parameters.releaseAll();
        m_renderColorTargetIndices.clear();
        m_offscreenRenderTargets.clear();
        m_interactiveParameters.clear();
        internal::deleteObject(m_parentFrameBufferObject);
        nvFX::IContainer::destroy(m_container);
        m_container = 0;
        gl::popAnnotationGroup(m_applicationContextRef);
    }
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
        pushAnnotationGroup(std::string("Effect#cacheTechniqueRef name=").append(technique->getName()).c_str(), m_applicationContextRef);
        VPVL2_VLOG(2, "Validating a technique: " << technique->getName());
        if (technique->validate()) {
            internalBindAttributes(technique);
            NvFXTechnique *newTechniquePtr = m_techniques.append(new NvFXTechnique(this, technique));
            m_techniqueRefsHash.insert(technique, newTechniquePtr);
            techniqueRef = newTechniquePtr;
        }
        else {
            VPVL2_VLOG(2, "Validation is failed and cannot use this technique");
        }
        popAnnotationGroup(m_applicationContextRef);
    }
    return techniqueRef;
}

void Effect::resetEffect(nvFX::IContainer *container)
{
    m_container = container;
    m_enabled = true;
    setDirty(true);
    if (m_name) {
        container->setName(reinterpret_cast<const char *>(m_name->toByteArray()));
    }
    appendShaderHeader(container, m_applicationContextRef->sharedFunctionResolverInstance());
    uploadTextureResources();
}

void Effect::pushAnnotationGroupWithName(const char *message)
{
    char buffer[1024];
    internal::snprintf(buffer, sizeof(buffer), "%s name=%s", message, internal::cstr(m_name, "(null)"));
    gl::pushAnnotationGroup(buffer, m_applicationContextRef);
}

} /* namespace nvfx */
} /* namespace VPVL2_VERSION_NS */
} /* namespace vpvl2 */
