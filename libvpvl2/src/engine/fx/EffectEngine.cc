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
#include "vpvl2/fx/EffectEngine.h"
#include "vpvl2/extensions/fx/Util.h"
#include "vpvl2/internal/util.h"

#include "vpvl2/extensions/gl/FrameBufferObject.h"
#include "vpvl2/extensions/gl/Texture2D.h"
#include "vpvl2/extensions/gl/Texture3D.h"
#include "vpvl2/extensions/gl/TexturePtrRef.h"
#include "vpvl2/extensions/gl/VertexBundle.h"

#include <string.h> /* for Linux */

#include <string>
#include <sstream>

namespace
{

using namespace vpvl2;
using namespace vpvl2::fx;
using namespace vpvl2::extensions::fx;
using namespace vpvl2::extensions::gl;

static const Scalar kWidth = 1, kHeight = 1;
static const Vector4 kVertices[] = {
    btVector4(-kWidth,  kHeight,  0,  1),
    btVector4(-kWidth, -kHeight,  0,  0),
    btVector4( kWidth, -kHeight,  1,  0),
    btVector4( kWidth,  kHeight,  1,  1)
};
static const vsize kVertexStride = sizeof(kVertices[0]);
static const int kIndices[] = { 0, 1, 2, 3 };
static const uint8 *kBaseAddress = reinterpret_cast<const uint8 *>(&kVertices[0]);
static const vsize kTextureOffset = reinterpret_cast<const uint8 *>(&kVertices[0].z()) - kBaseAddress;
static const vsize kIndicesSize = sizeof(kIndices) / sizeof(kIndices[0]);
static const EffectEngine::DrawPrimitiveCommand kQuadDrawCommand = EffectEngine::DrawPrimitiveCommand(kGL_QUADS, kIndicesSize, kGL_UNSIGNED_INT, 0, 0, sizeof(int));
static const char kWorldSemantic[] = "WORLD";
static const char kViewSemantic[] = "VIEW";
static const char kProjectionSemantic[] = "PROJECTION";
static const char kWorldViewSemantic[] = "WORLDVIEW";
static const char kViewProjectionSemantic[] = "VIEWPROJECTION";
static const char kWorldViewProjectionSemantic[] = "WORLDVIEWPROJECTION";
static const char kInverseTransposeSemanticsSuffix[] = "INVERSETRANSPOSE";
static const char kTransposeSemanticsSuffix[] = "TRANSPOSE";
static const char kInverseSemanticsSuffix[] = "INVERSE";
static const char kMultipleTechniquesPrefix[] = "Technique=Technique?";
static const char kSingleTechniquePrefix[] = "Technique=";

}

namespace vpvl2
{
namespace fx
{
using namespace extensions::gl;

/* BasicParameter */

BaseParameter::BaseParameter()
    : m_parameterRef(0)
{
}

BaseParameter::~BaseParameter()
{
    invalidate();
    m_parameterRef = 0;
}

void BaseParameter::addParameter(IEffect::Parameter *parameter)
{
    connectParameter(parameter, m_parameterRef);
}

void BaseParameter::invalidate()
{
    if (m_parameterRef) {
        m_parameterRef->reset();
    }
}

IEffect::Parameter *BaseParameter::parameterRef() const
{
    return m_parameterRef;
}


void BaseParameter::connectParameter(IEffect::Parameter *sourceParameter, IEffect::Parameter *&destinationParameter)
{
    /* prevent infinite reference loop */
    if (sourceParameter && sourceParameter != destinationParameter) {
        sourceParameter->connect(destinationParameter);
        destinationParameter = sourceParameter;
    }
}

/* BoolParameter */

BooleanParameter::BooleanParameter()
    : BaseParameter()
{
}

BooleanParameter::~BooleanParameter()
{
}

void BooleanParameter::setValue(bool value)
{
    if (m_parameterRef) {
        m_parameterRef->setValue(value);
    }
}

/* IntegerParameter */

IntegerParameter::IntegerParameter()
    : BaseParameter()
{
}

IntegerParameter::~IntegerParameter()
{
}

void IntegerParameter::setValue(int value)
{
    if (m_parameterRef) {
        m_parameterRef->setValue(value);
    }
}

/* FloatParameter */

FloatParameter::FloatParameter()
{
}

FloatParameter::~FloatParameter()
{
}

void FloatParameter::setValue(float value)
{
    if (m_parameterRef) {
        m_parameterRef->setValue(value);
    }
}

/* Float2Parameter */

Float2Parameter::Float2Parameter()
    : BaseParameter()
{
}

Float2Parameter::~Float2Parameter()
{
}

void Float2Parameter::setValue(const Vector3 &value)
{
    if (m_parameterRef) {
        m_parameterRef->setValue(value);
    }
}

/* Float4Parameter */

Float4Parameter::Float4Parameter()
    : BaseParameter()
{
}

Float4Parameter::~Float4Parameter()
{
}

void Float4Parameter::setValue(const Vector4 &value)
{
    if (m_parameterRef) {
        m_parameterRef->setValue(value);
    }
}

/* MatrixSemantic */

MatrixSemantic::MatrixSemantic(const IApplicationContext *applicationContextRef, int flags)
    : BaseParameter(),
      m_applicationContextRef(applicationContextRef),
      m_camera(0),
      m_cameraInversed(0),
      m_cameraTransposed(0),
      m_cameraInverseTransposed(0),
      m_light(0),
      m_lightInversed(0),
      m_lightTransposed(0),
      m_lightInverseTransposed(0),
      m_flags(flags)
{
}

MatrixSemantic::~MatrixSemantic()
{
    invalidate();
    m_camera = 0;
    m_cameraInversed = 0;
    m_cameraTransposed = 0;
    m_cameraInverseTransposed = 0;
    m_light = 0;
    m_lightInversed = 0;
    m_lightTransposed = 0;
    m_lightInverseTransposed = 0;
    m_flags = 0;
    m_applicationContextRef = 0;
}

void MatrixSemantic::setParameter(IEffect::Parameter *parameterRef, const char *suffix)
{
    if (const IEffect::Annotation *annotationRef = parameterRef->annotationRef("Object")) {
        const char *name = annotationRef->stringValue();
        const vsize len = std::strlen(name);
        if (VPVL2_FX_STREQ_CONST(name, len, "Camera")) {
            setMatrixParameters(suffix, parameterRef, m_cameraInversed, m_cameraTransposed, m_cameraInverseTransposed, m_camera);
        }
        else if (VPVL2_FX_STREQ_CONST(name, len, "Light")) {
            setMatrixParameters(suffix, parameterRef, m_lightInversed, m_lightTransposed, m_lightInverseTransposed, m_light);
        }
    }
    else {
        setMatrixParameters(suffix, parameterRef, m_cameraInversed, m_cameraTransposed, m_cameraInverseTransposed, m_camera);
    }
}

void MatrixSemantic::invalidate()
{
    BaseParameter::invalidate();
    if (m_camera) {
        m_camera->reset();
    }
    if (m_cameraInversed) {
        m_cameraInversed->reset();
    }
    if (m_cameraTransposed) {
        m_cameraTransposed->reset();
    }
    if (m_cameraInverseTransposed) {
        m_cameraInverseTransposed->reset();
    }
    if (m_light) {
        m_light->reset();
    }
    if (m_lightInversed) {
        m_lightInversed->reset();
    }
    if (m_lightTransposed) {
        m_lightTransposed->reset();
    }
    if (m_lightInverseTransposed) {
        m_lightInverseTransposed->reset();
    }
}

void MatrixSemantic::setMatrices(const IModel *model, int extraCameraFlags, int extraLightFlags)
{
    setMatrix(model, m_camera,                  extraCameraFlags | IApplicationContext::kCameraMatrix);
    setMatrix(model, m_cameraInversed,          extraCameraFlags | IApplicationContext::kCameraMatrix | IApplicationContext::kInverseMatrix);
    setMatrix(model, m_cameraTransposed,        extraCameraFlags | IApplicationContext::kCameraMatrix | IApplicationContext::kTransposeMatrix);
    setMatrix(model, m_cameraInverseTransposed, extraCameraFlags | IApplicationContext::kCameraMatrix | IApplicationContext::kInverseMatrix | IApplicationContext::kTransposeMatrix);
    setMatrix(model, m_light,                   extraLightFlags  | IApplicationContext::kLightMatrix);
    setMatrix(model, m_lightInversed,           extraLightFlags  | IApplicationContext::kLightMatrix  | IApplicationContext::kInverseMatrix);
    setMatrix(model, m_lightTransposed,         extraLightFlags  | IApplicationContext::kLightMatrix  | IApplicationContext::kTransposeMatrix);
    setMatrix(model, m_lightInverseTransposed,  extraLightFlags  | IApplicationContext::kLightMatrix  | IApplicationContext::kInverseMatrix | IApplicationContext::kTransposeMatrix);
}

void MatrixSemantic::setMatrixParameters(const char *suffix,
                                         IEffect::Parameter *sourceParameterRef,
                                         IEffect::Parameter *&inverse,
                                         IEffect::Parameter *&transposedRef,
                                         IEffect::Parameter *&inversetransposedRef,
                                         IEffect::Parameter *&baseParameterRef)
{
    const vsize len = std::strlen(suffix);
    if (VPVL2_FX_STREQ_CONST(suffix, len, kInverseTransposeSemanticsSuffix)) {
        BaseParameter::connectParameter(sourceParameterRef, inversetransposedRef);
    }
    else if (VPVL2_FX_STREQ_CONST(suffix, len, kTransposeSemanticsSuffix)) {
        BaseParameter::connectParameter(sourceParameterRef, transposedRef);
    }
    else if (VPVL2_FX_STREQ_CONST(suffix, len, kInverseSemanticsSuffix)) {
        BaseParameter::connectParameter(sourceParameterRef, inverse);
    }
    else {
        BaseParameter::connectParameter(sourceParameterRef, baseParameterRef);
    }
}

void MatrixSemantic::setMatrix(const IModel *model, IEffect::Parameter *parameterRef, int flags)
{
    if (parameterRef) {
        float matrix[16];
        m_applicationContextRef->getMatrix(matrix, model, m_flags | flags);
        parameterRef->setMatrix(matrix);
    }
}

/* Materialemantic */

MaterialSemantic::MaterialSemantic()
    : BaseParameter(),
      m_geometry(0),
      m_light(0)
{
}

MaterialSemantic::~MaterialSemantic()
{
    invalidate();
    m_geometry = 0;
    m_light = 0;
}

void MaterialSemantic::addParameter(IEffect::Parameter *parameterRef)
{
    const char *semantic = parameterRef->semantic();
    const vsize nlen = std::strlen(semantic);
    if (VPVL2_FX_STREQ_CONST(semantic, nlen, "SPECULARPOWER")
            || VPVL2_FX_STREQ_CONST(semantic, nlen, "EDGECOLOR")
            || VPVL2_FX_STREQ_CONST(semantic, nlen, "EMISSIVE")
            || VPVL2_FX_STREQ_CONST(semantic, nlen, "TOONCOLOR")) {
        BaseParameter::connectParameter(parameterRef, m_geometry);
    }
    else if (const IEffect::Annotation *annotationRef = parameterRef->annotationRef("Object")) {
        const char *aname = annotationRef->stringValue();
        const vsize alen = std::strlen(aname);
        if (VPVL2_FX_STREQ_CONST(aname, alen,  "Geometry")) {
            BaseParameter::connectParameter(parameterRef, m_geometry);
        }
        else if (VPVL2_FX_STREQ_CONST(aname, alen, "Light")) {
            BaseParameter::connectParameter(parameterRef, m_light);
        }
    }
    else {
        const char *name = parameterRef->name();
        const vsize nlen2 = std::strlen(name);
        if (VPVL2_FX_STREQ_CONST(name, nlen2,  "EgColor")
                || VPVL2_FX_STREQ_CONST(name, nlen2,  "SpcColor")) {
            BaseParameter::connectParameter(parameterRef, m_geometry);
        }
    }
}

void MaterialSemantic::invalidate()
{
    BaseParameter::invalidate();
    if (m_geometry) {
        m_geometry->reset();
    }
    if (m_light) {
        m_light->reset();
    }
}

void MaterialSemantic::setGeometryColor(const Color &value)
{
    if (m_geometry) {
        m_geometry->setValue(value);
    }
}

void MaterialSemantic::setGeometryValue(const Scalar &value)
{
    if (m_geometry) {
        m_geometry->setValue(value);
    }
}

void MaterialSemantic::setLightColor(const Color &value)
{
    if (m_light) {
        m_light->setValue(value);
    }
}

void MaterialSemantic::setLightValue(const Scalar &value)
{
    if (m_light) {
        m_light->setValue(value);
    }
}

/* MaterialTextureSemantic */

MaterialTextureSemantic::MaterialTextureSemantic()
    : BaseParameter(),
      m_mipmap(false)
{
}

MaterialTextureSemantic::~MaterialTextureSemantic()
{
    invalidate();
}

bool MaterialTextureSemantic::hasMipmap(const IEffect::Parameter *textureParameterRef, const IEffect::Parameter *samplerParameterRef)
{
    bool hasMipmap = false;
    if (const IEffect::Annotation *annotationRef = textureParameterRef->annotationRef("MipLevels")) {
        hasMipmap = annotationRef->integerValue() != 1;
    }
    if (const IEffect::Annotation *annotaitonRef = textureParameterRef->annotationRef("Level")) {
        hasMipmap = annotaitonRef->integerValue() != 1;
    }
    Array<IEffect::SamplerState *> states;
    samplerParameterRef->getSamplerStateRefs(states);
    const int nstates = states.count();
    for (int i = 0; i < nstates; i++) {
        const IEffect::SamplerState *state = states[i];
        if (state->type() == IEffect::Parameter::kInteger) {
            const char *name = state->name();
            const vsize len = std::strlen(name);
            if (VPVL2_FX_STREQ_CASE_CONST(name, len, "MINFILTER")) {
                int value = 0;
                state->getValue(value);
                switch (value) {
                case BaseTexture::kGL_NEAREST_MIPMAP_NEAREST:
                case BaseTexture::kGL_NEAREST_MIPMAP_LINEAR:
                case BaseTexture::kGL_LINEAR_MIPMAP_NEAREST:
                case BaseTexture::kGL_LINEAR_MIPMAP_LINEAR:
                    hasMipmap = true;
                    break;
                default:
                    break;
                }
                if (hasMipmap) {
                    break;
                }
            }
        }
    }
    return hasMipmap;
}

void MaterialTextureSemantic::addTextureParameter(const IEffect::Parameter *textureParameterRef, IEffect::Parameter *samplerParameterRef)
{
    m_mipmap = hasMipmap(textureParameterRef, samplerParameterRef);
    BaseParameter::addParameter(samplerParameterRef);
}

void MaterialTextureSemantic::invalidate()
{
    BaseParameter::invalidate();
    m_textures.clear();
    m_mipmap = false;
}

void MaterialTextureSemantic::setTexture(const HashPtr &key, const ITexture *value)
{
    if (m_parameterRef && value) {
        m_textures.insert(key, value);
        m_parameterRef->setSampler(value);
    }
}

void MaterialTextureSemantic::updateParameter(const HashPtr &key)
{
    if (const ITexture *const *value = m_textures.find(key)) {
        const ITexture *textureRef = *value;
        if (m_parameterRef) {
            m_parameterRef->setTexture(textureRef);
        }
    }
}

/* TextureUnit */

TextureUnit::TextureUnit()
    : BaseParameter()
{
}

TextureUnit::~TextureUnit()
{
}

void TextureUnit::setTexture(GLuint value)
{
    if (m_parameterRef) {
        m_parameterRef->setTexture(static_cast<intptr_t>(value));
    }
}

/* GeometrySemantic */

GeometrySemantic::GeometrySemantic()
    : BaseParameter(),
      m_camera(0),
      m_light(0)
{
}

GeometrySemantic::~GeometrySemantic()
{
    invalidate();
    m_camera = 0;
    m_light = 0;
}

void GeometrySemantic::addParameter(IEffect::Parameter *parameterRef)
{
    if (const IEffect::Annotation *annotationRef = parameterRef->annotationRef("Object")) {
        const char *name = annotationRef->stringValue();
        const vsize len = std::strlen(name);
        if (VPVL2_FX_STREQ_CONST(name, len, "Camera")) {
            BaseParameter::connectParameter(parameterRef, m_camera);
        }
        else if (VPVL2_FX_STREQ_CONST(name, len, "Light")) {
            BaseParameter::connectParameter(parameterRef, m_light);
        }
    }
}

void GeometrySemantic::invalidate()
{
    BaseParameter::invalidate();
    if (m_camera) {
        m_camera->reset();
    }
    if (m_light) {
        m_light->reset();
    }
}

void GeometrySemantic::setCameraValue(const Vector3 &value)
{
    if (m_camera) {
        m_camera->setValue(value);
    }
}

void GeometrySemantic::setLightValue(const Vector3 &value)
{
    if (m_light) {
        m_light->setValue(value);
    }
}

/* TimeSemantic */

TimeSemantic::TimeSemantic(const IApplicationContext *applicationContextRef)
    : BaseParameter(),
      m_applicationContextRef(applicationContextRef),
      m_syncEnabled(0),
      m_syncDisabled(0)
{
}

TimeSemantic::~TimeSemantic()
{
    invalidate();
    m_syncEnabled = 0;
    m_syncDisabled = 0;
}

void TimeSemantic::addParameter(IEffect::Parameter *parameterRef)
{
    if (const IEffect::Annotation *annotationRef = parameterRef->annotationRef("SyncInEditMode")) {
        if (annotationRef->booleanValue()) {
            BaseParameter::connectParameter(parameterRef, m_syncEnabled);
            return;
        }
    }
    BaseParameter::connectParameter(parameterRef, m_syncDisabled);
}

void TimeSemantic::invalidate()
{
    BaseParameter::invalidate();
    if (m_syncEnabled) {
        m_syncEnabled->reset();
    }
    if (m_syncDisabled) {
        m_syncDisabled->reset();
    }
}

void TimeSemantic::update()
{
    float value = 0;
    if (m_syncEnabled) {
        m_applicationContextRef->getTime(value, true);
        m_syncEnabled->setValue(value);
    }
    if (m_syncDisabled) {
        m_applicationContextRef->getTime(value, false);
        m_syncDisabled->setValue(value);
    }
}

/* ControlObjectSemantic */

ControlObjectSemantic::ControlObjectSemantic(const Scene *sceneRef, const IApplicationContext *applicationContextRef)
    : BaseParameter(),
      m_sceneRef(sceneRef),
      m_applicationContextRef(applicationContextRef)
{
}

ControlObjectSemantic::~ControlObjectSemantic()
{
    m_sceneRef = 0;
    m_applicationContextRef = 0;
}

void ControlObjectSemantic::addParameter(IEffect::Parameter *parameterRef)
{
    if (parameterRef->annotationRef("name")) {
        m_parameterRefs.append(parameterRef);
    }
}

void ControlObjectSemantic::invalidate()
{
    BaseParameter::invalidate();
    m_parameterRefs.clear();
}

void ControlObjectSemantic::update(const IModel *self)
{
    const int nparameters = m_parameterRefs.count();
    for (int i = 0; i < nparameters; i++) {
        IEffect::Parameter *parameterRef = m_parameterRefs[i];
        if (const IEffect::Annotation *annotationRef = parameterRef->annotationRef("name")) {
            const char *name = annotationRef->stringValue();
            const vsize len = std::strlen(name);
            if (VPVL2_FX_STREQ_CONST(name, len, "(self)")) {
                setParameter(self, parameterRef);
            }
            else if (VPVL2_FX_STREQ_CONST(name, len, "(OffscreenOwner)")) {
                if (IEffect *parent = parameterRef->parentEffectRef()->parentEffectRef()) {
                    const IModel *model = m_applicationContextRef->effectOwner(parent);
                    setParameter(model, parameterRef);
                }
            }
            else {
                IString *s = m_applicationContextRef->toUnicode(reinterpret_cast<const uint8 *>(name));
                const IModel *model = m_applicationContextRef->findModel(s);
                internal::deleteObject(s);
                setParameter(model, parameterRef);
            }
        }
    }
}

void ControlObjectSemantic::setParameter(const IModel *model, IEffect::Parameter *parameterRef)
{
    if (model) {
        if (const IEffect::Annotation *annotationRef = parameterRef->annotationRef("item")) {
            switch (model->type()) {
            case IModel::kPMDModel:
            case IModel::kPMXModel:
                setModelBoneMorphParameter(model, annotationRef, parameterRef);
                break;
            default:
                setAssetParameter(model, annotationRef, parameterRef);
                break;
            }
        }
        else {
            setModelParameter(model, parameterRef);
        }
    }
    else {
        setNullParameter(parameterRef);
    }
}

void ControlObjectSemantic::setModelBoneMorphParameter(const IModel *model, const IEffect::Annotation *annotationRef, IEffect::Parameter *parameterRef)
{
    const char *item = annotationRef->stringValue();
    const IString *s = m_applicationContextRef->toUnicode(reinterpret_cast<const uint8 *>(item));
    IBone *bone = model->findBoneRef(s);
    IMorph *morph = model->findMorphRef(s);
    internal::deleteObject(s);
    if (bone) {
        float matrix4x4[16] = { 0 };
        switch (parameterRef->type()) {
        case IEffect::Parameter::kFloat3:
        case IEffect::Parameter::kFloat4:
            parameterRef->setValue(bone->worldTransform().getOrigin());
            break;
        case IEffect::Parameter::kFloat4x4:
            bone->worldTransform().getOpenGLMatrix(matrix4x4);
            parameterRef->setMatrix(matrix4x4);
            break;
        default:
            break;
        }
    }
    else if (morph) {
        parameterRef->setValue(float(morph->weight()));
    }
}

void ControlObjectSemantic::setAssetParameter(const IModel *model, const IEffect::Annotation *annotationRef, IEffect::Parameter *parameterRef)
{
    const Vector3 &position = model->worldTranslation();
    const Quaternion &rotation = model->worldOrientation();
    const char *item = annotationRef->stringValue();
    const vsize len = std::strlen(item);
    if (VPVL2_FX_STREQ_CONST(item, len, "X")) {
        parameterRef->setValue(position.x());
    }
    else if (VPVL2_FX_STREQ_CONST(item, len, "Y")) {
        parameterRef->setValue(position.y());
    }
    else if (VPVL2_FX_STREQ_CONST(item, len, "Z")) {
        parameterRef->setValue(position.z());
    }
    else if (VPVL2_FX_STREQ_CONST(item, len, "XYZ")) {
        parameterRef->setValue(position);
    }
    else if (VPVL2_FX_STREQ_CONST(item, len, "Rx")) {
        parameterRef->setValue(btDegrees(rotation.x()));
    }
    else if (VPVL2_FX_STREQ_CONST(item, len, "Ry")) {
        parameterRef->setValue(btDegrees(rotation.y()));
    }
    else if (VPVL2_FX_STREQ_CONST(item, len, "Rz")) {
        parameterRef->setValue(btDegrees(rotation.z()));
    }
    else if (VPVL2_FX_STREQ_CONST(item, len, "Rxyz")) {
        const Vector3 rotationDegree(btDegrees(rotation.x()), btDegrees(rotation.y()), btDegrees(rotation.z()));
        parameterRef->setValue(rotationDegree);
    }
    else if (VPVL2_FX_STREQ_CONST(item, len, "Si")) {
        parameterRef->setValue(model->scaleFactor());
    }
    else if (VPVL2_FX_STREQ_CONST(item, len, "Tr")) {
        parameterRef->setValue(model->opacity());
    }
}

void ControlObjectSemantic::setModelParameter(const IModel *model, IEffect::Parameter *parameterRef)
{
    float matrix4x4[16] = { 0 };
    switch (parameterRef->type()) {
    case IEffect::Parameter::kBoolean:
        parameterRef->setValue(model->isVisible());
        break;
    case IEffect::Parameter::kFloat:
        parameterRef->setValue(model->scaleFactor());
        break;
    case IEffect::Parameter::kFloat3:
    case IEffect::Parameter::kFloat4:
        parameterRef->setValue(model->worldTranslation());
        break;
    case IEffect::Parameter::kFloat4x4:
        m_applicationContextRef->getMatrix(matrix4x4, model, IApplicationContext::kWorldMatrix | IApplicationContext::kCameraMatrix);
        parameterRef->setMatrix(matrix4x4);
        break;
    default:
        break;
    }
}

void ControlObjectSemantic::setNullParameter(IEffect::Parameter *parameterRef)
{
    float matrix4x4[16] = { 0 };
    switch (parameterRef->type()) {
    case IEffect::Parameter::kBoolean:
        parameterRef->setValue(false);
        break;
    case IEffect::Parameter::kFloat:
        parameterRef->setValue(0.0f);
        break;
    case IEffect::Parameter::kFloat3:
    case IEffect::Parameter::kFloat4:
        parameterRef->setValue(kZeroV4);
        break;
    case IEffect::Parameter::kFloat4x4:
        Transform::getIdentity().getOpenGLMatrix(matrix4x4);
        parameterRef->setMatrix(matrix4x4);
        break;
    default:
        break;
    }
}

/* RenderColorTargetSemantic */

RenderColorTargetSemantic::RenderColorTargetSemantic(IApplicationContext *applicationContextRef)
    : BaseParameter(),
      m_applicationContextRef(applicationContextRef)
{
}

RenderColorTargetSemantic::~RenderColorTargetSemantic()
{
    m_textures.releaseAll();
    m_applicationContextRef = 0;
}

bool RenderColorTargetSemantic::tryGetTextureFlags(const IEffect::Parameter *textureParameterRef,
                                                   const IEffect::Parameter *samplerParameterRef,
                                                   bool enableAllTextureTypes,
                                                   int &flags)
{
    const IEffect::Annotation *annotationRef = textureParameterRef->annotationRef("ResourceType");
    if (enableAllTextureTypes && annotationRef) {
        const char *typeName = annotationRef->stringValue();
        const vsize len = std::strlen(typeName);
        const IEffect::Parameter::Type samplerType = samplerParameterRef->type();
        if (VPVL2_FX_STREQ_CONST(typeName, len, "CUBE") && samplerType == IEffect::Parameter::kSamplerCube) {
            flags = IApplicationContext::kTextureCube;
        }
        else if (VPVL2_FX_STREQ_CONST(typeName, len, "3D") && samplerType == IEffect::Parameter::kSampler3D) {
            flags = IApplicationContext::kTexture3D;
        }
        else if (VPVL2_FX_STREQ_CONST(typeName, len, "2D") && samplerType == IEffect::Parameter::kSampler2D) {
            flags = IApplicationContext::kTexture2D;
        }
        else {
            return false;
        }
    }
    else {
        flags = IApplicationContext::kTexture2D;
    }
    if (MaterialTextureSemantic::hasMipmap(textureParameterRef, samplerParameterRef)) {
        flags |= IApplicationContext::kGenerateTextureMipmap;
    }
    return true;
}

void RenderColorTargetSemantic::addFrameBufferObjectParameter(IEffect::Parameter *textureParameterRef,
                                                              IEffect::Parameter *samplerParameterRef,
                                                              FrameBufferObject *frameBufferObjectRef,
                                                              void *userData,
                                                              bool enableResourceName,
                                                              bool enableAllTextureTypes)
{
    int flags;
    if (!tryGetTextureFlags(textureParameterRef, samplerParameterRef, enableAllTextureTypes, flags)) {
        return;
    }
    const IEffect::Annotation *annotationRef = textureParameterRef->annotationRef("ResourceName");
    const char *textureParameterName = textureParameterRef->name();
    IApplicationContext::SharedTextureParameter sharedTextureParameter(textureParameterRef);
    const ITexture *textureRef = 0;
    if (enableResourceName && annotationRef) {
        const char *name = annotationRef->stringValue();
        IString *s = m_applicationContextRef->toUnicode(reinterpret_cast<const uint8*>(name));
        IApplicationContext::TextureDataBridge texture(flags & ~IApplicationContext::kAsyncLoadingTexture);
        if (m_applicationContextRef->uploadTexture(s, texture, userData)) {
            textureRef = texture.dataRef;
            if (textureRef) {
                samplerParameterRef->setSampler(textureRef);
                m_path2parameterRefs.insert(name, textureParameterRef);
                ITexture *tex = m_textures.append(new TexturePtrRef(textureRef));
                m_name2textures.insert(textureParameterName, TextureReference(frameBufferObjectRef, tex, textureParameterRef, samplerParameterRef));
            }
        }
        internal::deleteObject(s);
    }
    else if (m_applicationContextRef->tryGetSharedTextureParameter(textureParameterName, sharedTextureParameter)) {
        IEffect::Parameter *parameterRef = sharedTextureParameter.parameterRef;
        if (std::strcmp(parameterRef->semantic(), textureParameterRef->semantic()) == 0) {
            textureParameterRef = parameterRef;
            textureRef = sharedTextureParameter.textureRef;
        }
    }
    else if ((flags & IApplicationContext::kTexture3D) != 0) {
        generateTexture3D0(textureParameterRef, samplerParameterRef, frameBufferObjectRef);
        textureRef = m_textures[m_textures.count() - 1];
    }
    else if ((flags & IApplicationContext::kTexture2D) != 0) {
        generateTexture2D0(textureParameterRef, samplerParameterRef, frameBufferObjectRef);
        textureRef = m_textures[m_textures.count() - 1];
    }
    m_parameters.append(textureParameterRef);
    if (samplerParameterRef) {
        samplerParameterRef->setSampler(textureRef);
    }
}

void RenderColorTargetSemantic::invalidate()
{
    BaseParameter::invalidate();
    m_name2textures.clear();
    m_parameters.clear();
}

const RenderColorTargetSemantic::TextureReference *RenderColorTargetSemantic::findTexture(const char *name) const
{
    return m_name2textures.find(name);
}

IEffect::Parameter *RenderColorTargetSemantic::findParameter(const char *name) const
{
    IEffect::Parameter *const *ref = m_path2parameterRefs.find(name);
    return ref ? *ref : 0;
}

int RenderColorTargetSemantic::countParameters() const
{
    return m_parameters.count();
}

void RenderColorTargetSemantic::generateTexture2D(IEffect::Parameter *textureParameterRef,
                                                  IEffect::Parameter *samplerParameterRef,
                                                  const Vector3 &size,
                                                  FrameBufferObject *frameBufferObjectRef,
                                                  BaseSurface::Format &format)
{
    const IApplicationContext::FunctionResolver *resolver = m_applicationContextRef->sharedFunctionResolverInstance();
    Util::getTextureFormat(textureParameterRef, resolver, format);
    ITexture *texture = m_textures.append(new Texture2D(resolver, format, size, 0));
    texture->create();
    m_name2textures.insert(textureParameterRef->name(), TextureReference(frameBufferObjectRef, texture, textureParameterRef, samplerParameterRef));
    texture->bind();
    if (MaterialTextureSemantic::hasMipmap(textureParameterRef, samplerParameterRef)) {
        generateMipmap(Texture2D::kGL_TEXTURE_2D);
    }
    texture->unbind();
}

void RenderColorTargetSemantic::generateTexture3D(IEffect::Parameter *textureParamaterRef,
                                                  IEffect::Parameter *samplerParameterRef,
                                                  const Vector3 &size,
                                                  FrameBufferObject *frameBufferObjectRef)
{
    BaseSurface::Format format;
    const IApplicationContext::FunctionResolver *resolver = m_applicationContextRef->sharedFunctionResolverInstance();
    Util::getTextureFormat(textureParamaterRef, resolver, format);
    ITexture *texture = m_textures.append(new Texture3D(resolver, format, size, 0));
    texture->create();
    m_name2textures.insert(textureParamaterRef->name(), TextureReference(frameBufferObjectRef, texture, textureParamaterRef, samplerParameterRef));
    texture->bind();
    if (MaterialTextureSemantic::hasMipmap(textureParamaterRef, samplerParameterRef)) {
        generateMipmap(Texture3D::kGL_TEXTURE_3D);
    }
    texture->unbind();
}

void RenderColorTargetSemantic::generateTexture2D0(IEffect::Parameter *textureRef,
                                                   IEffect::Parameter *samplerRef,
                                                   FrameBufferObject *frameBufferObjectRef)
{
    vsize width, height;
    BaseSurface::Format format;
    getSize2(textureRef, width, height);
    generateTexture2D(textureRef, samplerRef, Vector3(Scalar(width), Scalar(height), 0), frameBufferObjectRef, format);
}

void RenderColorTargetSemantic::generateTexture3D0(IEffect::Parameter *textureRef,
                                                   IEffect::Parameter *samplerRef,
                                                   FrameBufferObject *frameBufferObjectRef)
{
    vsize width, height, depth;
    getSize3(textureRef, width, height, depth);
    generateTexture3D(textureRef, samplerRef, Vector3(Scalar(width), Scalar(height), Scalar(depth)), frameBufferObjectRef);
}

void RenderColorTargetSemantic::getSize2(const IEffect::Parameter *parameterRef, vsize &width, vsize &height) const
{
    Vector3 size;
    if (Util::getSize2(parameterRef, size)) {
        width = vsize(size.x());
        height = vsize(size.y());
    }
    else {
        Vector3 viewport;
        m_applicationContextRef->getViewport(viewport);
        width = btMax(vsize(1), vsize(viewport.x() * size.x()));
        height = btMax(vsize(1), vsize(viewport.y() * size.y()));
    }
}

void RenderColorTargetSemantic::getSize3(const IEffect::Parameter *parameterRef, vsize &width, vsize &height, vsize &depth) const
{
    Vector3 size;
    if (Util::getSize3(parameterRef, size)) {
        width = vsize(size.x());
        height = vsize(size.y());
        depth = vsize(size.z());
    }
    else {
        Vector3 viewport;
        m_applicationContextRef->getViewport(viewport);
        width = btMax(vsize(1), vsize(viewport.x()));
        height = btMax(vsize(1), vsize(viewport.y()));
        depth = 24;
    }
}

ITexture *RenderColorTargetSemantic::lastTextureRef() const
{
    return m_textures[m_textures.count() - 1];
}

/* RenderDepthStencilSemantic */

RenderDepthStencilTargetSemantic::RenderDepthStencilTargetSemantic(IApplicationContext *applicationContextRef)
    : RenderColorTargetSemantic(applicationContextRef)
{
}

RenderDepthStencilTargetSemantic::~RenderDepthStencilTargetSemantic()
{
    invalidate();
}

void RenderDepthStencilTargetSemantic::addFrameBufferObjectParameter(IEffect::Parameter *parameterRef, FrameBufferObject *frameBufferObjectRef)
{
    vsize width, height;
    getSize2(parameterRef, width, height);
    m_parameters.append(parameterRef);
    GLenum internalFormat = FrameBufferObject::kGL_DEPTH24_STENCIL8;
    if (const IEffect::Annotation *annotationRef = parameterRef->annotationRef("Format")) {
        Hash<HashString, GLenum> formats;
        GLenum d24 = FrameBufferObject::kGL_DEPTH24_STENCIL8, d32 = FrameBufferObject::kGL_DEPTH32F_STENCIL8;
        formats.insert("D24S8", d24);
        formats.insert("D32FS8", d32);
        const char *value = annotationRef->stringValue();
        if (const GLenum *internalFormatPtr = formats.find(value)) {
            internalFormat = *internalFormatPtr;
        }
    }
    BaseSurface::Format format(FrameBufferObject::kGL_DEPTH_COMPONENT, internalFormat, kGL_UNSIGNED_BYTE, Texture2D::kGL_TEXTURE_2D);
    m_renderBuffers.append(new FrameBufferObject::StandardRenderBuffer(m_applicationContextRef->sharedFunctionResolverInstance(), format, Vector3(Scalar(width), Scalar(height), 0)));
    FrameBufferObject::BaseRenderBuffer *renderBuffer = m_renderBuffers[m_renderBuffers.count() - 1];
    renderBuffer->create();
    m_buffers.insert(parameterRef->name(), Buffer(frameBufferObjectRef, renderBuffer, parameterRef));
}

void RenderDepthStencilTargetSemantic::invalidate()
{
    BaseParameter::invalidate();
    m_renderBuffers.clear();
    m_parameters.clear();
    m_buffers.clear();
}

const RenderDepthStencilTargetSemantic::Buffer *RenderDepthStencilTargetSemantic::findDepthStencilBuffer(const char *name) const
{
    return m_buffers.find(name);
}

/* OffscreenRenderTargetSemantic */

OffscreenRenderTargetSemantic::OffscreenRenderTargetSemantic(IApplicationContext *applicationContextRef)
    : RenderColorTargetSemantic(applicationContextRef)
{
}

OffscreenRenderTargetSemantic::~OffscreenRenderTargetSemantic()
{
}

void OffscreenRenderTargetSemantic::generateTexture2D(IEffect::Parameter *textureParameterRef,
                                                      IEffect::Parameter *samplerParameterRef,
                                                      const Vector3 &size,
                                                      FrameBufferObject *frameBufferObjectRef,
                                                      BaseSurface::Format &format)
{
    RenderColorTargetSemantic::generateTexture2D(textureParameterRef, samplerParameterRef, size, frameBufferObjectRef, format);
    ITexture *texture = lastTextureRef();
    if (IEffect *effectRef = textureParameterRef->parentEffectRef()) {
        effectRef->addOffscreenRenderTarget(texture, textureParameterRef, samplerParameterRef);
    }
}

/* AnimatedTextureSemantic */

AnimatedTextureSemantic::AnimatedTextureSemantic(IApplicationContext *applicationContextRef)
    : m_applicationContextRef(applicationContextRef)
{
}

AnimatedTextureSemantic::~AnimatedTextureSemantic()
{
    m_applicationContextRef = 0;
}

void AnimatedTextureSemantic::addParameter(IEffect::Parameter *parameterRef)
{
    const IEffect::Annotation *annotationRef = parameterRef->annotationRef("ResourceName");
    if (annotationRef && parameterRef->type() == IEffect::Parameter::kTexture) {
        m_parameterRefs.append(parameterRef);
    }
}

void AnimatedTextureSemantic::invalidate()
{
    BaseParameter::invalidate();
    m_parameterRefs.clear();
}

void AnimatedTextureSemantic::update(const RenderColorTargetSemantic &renderColorTarget)
{
    const int nparameters = m_parameterRefs.count();
    for (int i = 0; i < nparameters; i++) {
        IEffect::Parameter *parameter = m_parameterRefs[i];
        float offset = 0, speed = 1, seek = 0;
        if (const IEffect::Annotation *annotationRef = parameter->annotationRef("Offset")) {
            offset = annotationRef->floatValue();
        }
        if (const IEffect::Annotation *annotationRef = parameter->annotationRef("Speed")) {
            speed = annotationRef->floatValue();
        }
        if (const IEffect::Annotation *annotationRef = parameter->annotationRef("SeekVariable")) {
            const IEffect *effect = parameter->parentEffectRef();
            IEffect::Parameter *seekParameter = effect->findUniformParameter(annotationRef->stringValue());
            seekParameter->getValue(seek);
        }
        else {
            m_applicationContextRef->getTime(seek, true);
        }
        if (const IEffect::Annotation *annotationRef = parameter->annotationRef("ResourceName")) {
            const char *resourceName = annotationRef->stringValue();
            const IEffect::Parameter *textureParameterRef = renderColorTarget.findParameter(resourceName);
            if (const RenderColorTargetSemantic::TextureReference *t = renderColorTarget.findTexture(textureParameterRef->name())) {
                GLuint textureID = static_cast<GLuint>(t->textureRef->data());
                m_applicationContextRef->uploadAnimatedTexture(offset, speed, seek, &textureID);
            }
        }
    }
}

/* TextureValueSemantic */

TextureValueSemantic::TextureValueSemantic(const IApplicationContext *context)
    : bindTexture(reinterpret_cast<PFNGLBINDTEXTUREPROC>(context->sharedFunctionResolverInstance()->resolveSymbol("glBindTexture"))),
      getTexImage(reinterpret_cast<PFNGLGETTEXIMAGEPROC>(context->sharedFunctionResolverInstance()->resolveSymbol("glGetTexImage")))
{
}

TextureValueSemantic::~TextureValueSemantic()
{
}

void TextureValueSemantic::addParameter(IEffect::Parameter *parameterRef)
{
    if (const IEffect::Annotation *annotationRef = parameterRef->annotationRef("TextureName")) {
        int ndimensions = 0;
        parameterRef->getArrayDimension(ndimensions);
        bool isFloat4 = parameterRef->type() == IEffect::Parameter::kFloat4;
        bool isValidDimension = ndimensions == 1 || ndimensions == 2;
        if (isFloat4 && isValidDimension) {
            const char *name = annotationRef->stringValue();
            const IEffect *effect = parameterRef->parentEffectRef();
            IEffect::Parameter *textureParameterRef = effect->findUniformParameter(name);
            if (textureParameterRef->type() == IEffect::Parameter::kTexture) {
                m_parameterRefs.append(textureParameterRef);
            }
        }
    }
}

void TextureValueSemantic::invalidate()
{
    BaseParameter::invalidate();
    m_parameterRefs.clear();
}

void TextureValueSemantic::update()
{
    const int nparameters = m_parameterRefs.count();
    for (int i = 0; i < nparameters; i++) {
        int size = 0;
        intptr_t texture;
        IEffect::Parameter *parameterRef = m_parameterRefs[i];
        parameterRef->getTextureRef(texture);
        parameterRef->getArrayTotalSize(size);
        if (Vector4 *pixels = new Vector4[size]) {
            bindTexture(Texture2D::kGL_TEXTURE_2D, static_cast<GLuint>(texture));
            getTexImage(Texture2D::kGL_TEXTURE_2D, 0, kGL_RGBA, kGL_UNSIGNED_BYTE, pixels);
            parameterRef->setValue(pixels);
            internal::deleteObjectArray(pixels);
        }
    }
    bindTexture(Texture2D::kGL_TEXTURE_2D, 0);
}

/* SelfShadowObjectSemantic */

SelfShadowSemantic::SelfShadowSemantic()
    : BaseParameter(),
      m_center(0),
      m_size(0),
      m_distance(0),
      m_rate(0)
{
}

SelfShadowSemantic::~SelfShadowSemantic()
{
}

void SelfShadowSemantic::addParameter(IEffect::Parameter *parameterRef)
{
    if (const IEffect::Annotation *annotationRef = parameterRef->annotationRef("name")) {
        const char *name = annotationRef->stringValue();
        vsize len = std::strlen(name);
        if (VPVL2_FX_STREQ_CASE_CONST(name, len, "rate")) {
            m_rate = parameterRef;
        }
        if (VPVL2_FX_STREQ_CASE_CONST(name, len, "size")) {
            m_size = parameterRef;
        }
        else if (VPVL2_FX_STREQ_CASE_CONST(name, len, "center")) {
            m_center = parameterRef;
        }
        else if (VPVL2_FX_STREQ_CASE_CONST(name, len, "distance")) {
            m_distance = parameterRef;
        }
    }
}

void SelfShadowSemantic::invalidate()
{
    BaseParameter::invalidate();
    m_rate = 0;
    m_size = 0;
    m_center = 0;
    m_distance = 0;
}

void SelfShadowSemantic::updateParameter(const IShadowMap *shadowMapRef)
{
    if (m_center) {
        m_center->setValue(shadowMapRef->position());
    }
    if (m_distance) {
        m_distance->setValue(shadowMapRef->distance());
    }
    if (m_rate) {
        m_rate->setValue(1); //shadowMapRef->rate());
    }
    if (m_size) {
        m_size->setValue(shadowMapRef->size());
    }
}

MatricesParameter::MatricesParameter()
    : BaseParameter()
{
}

MatricesParameter::~MatricesParameter()
{
}

void MatricesParameter::setValues(const float32 *value, size_t size)
{
    if (m_parameterRef) {
        m_parameterRef->setMatrices(value, size);
    }
}

#ifdef VPVL2_ENABLE_NVIDIA_CG
/* Effect::RectRenderEngine */
class EffectEngine::RectangleRenderEngine
{
public:
    RectangleRenderEngine(const IApplicationContext::FunctionResolver *resolver)
        : bindBuffer(reinterpret_cast<PFNGLBINDBUFFERPROC>(resolver->resolveSymbol("glBindBuffer"))),
          bufferData(reinterpret_cast<PFNGLBUFFERDATAPROC>(resolver->resolveSymbol("glBufferData"))),
          deleteBuffers(reinterpret_cast<PFNGLDELETEBUFFERSPROC>(resolver->resolveSymbol("glDeleteBuffers"))),
          disable(reinterpret_cast<PFNGLDISABLEPROC>(resolver->resolveSymbol("glDisable"))),
          m_vertexBundle(0),
          m_verticesBuffer(0),
          m_indicesBuffer(0)
    {
    }
    ~RectangleRenderEngine() {
        deleteBuffers(1, &m_verticesBuffer);
        deleteBuffers(1, &m_indicesBuffer);
    }

    void initializeVertexBundle() {
        genBuffers(1, &m_verticesBuffer);
        bindBuffer(VertexBundle::kGL_ARRAY_BUFFER, m_verticesBuffer);
        bufferData(VertexBundle::kGL_ARRAY_BUFFER, sizeof(kVertices), kVertices, VertexBundle::kGL_STATIC_DRAW);
        genBuffers(1, &m_indicesBuffer);
        bindBuffer(VertexBundle::kGL_ELEMENT_ARRAY_BUFFER, m_indicesBuffer);
        bufferData(VertexBundle::kGL_ELEMENT_ARRAY_BUFFER, sizeof(kIndices), kIndices, VertexBundle::kGL_STATIC_DRAW);
        bindBuffer(VertexBundle::kGL_ARRAY_BUFFER, 0);
        bindBuffer(VertexBundle::kGL_ELEMENT_ARRAY_BUFFER, 0);
        m_bundle.allocateVertexArrayObjects(&m_vertexBundle, 1);
        m_bundle.bindVertexArrayObject(m_vertexBundle);
        bindVertexBundle(false);
        glEnableClientState(kGL_VERTEX_ARRAY);
        glEnableClientState(kGL_TEXTURE_COORD_ARRAY);
        m_bundle.unbindVertexArrayObject();
        unbindVertexBundle(false);
    }
    void bindVertexBundle(bool bundle) {
        if (!bundle || !m_bundle.bindVertexArrayObject(m_vertexBundle)) {
            bindBuffer(VertexBundle::kGL_ARRAY_BUFFER, m_verticesBuffer);
            bindBuffer(VertexBundle::kGL_ELEMENT_ARRAY_BUFFER, m_indicesBuffer);
            glVertexPointer(2, kGL_FLOAT, kVertexStride, reinterpret_cast<const GLvoid *>(0));
            glTexCoordPointer(2, kGL_FLOAT, kVertexStride, reinterpret_cast<const GLvoid *>(kTextureOffset));
        }
        disable(kGL_DEPTH_TEST);
    }
    void unbindVertexBundle(bool bundle) {
        if (!bundle || !m_bundle.unbindVertexArrayObject()) {
            bindBuffer(VertexBundle::GL_ARRAY_BUFFER, 0);
            bindBuffer(VertexBundle::GL_ELEMENT_ARRAY_BUFFER, 0);
        }
        glEnable(kGL_DEPTH_TEST);
    }

private:
    typedef void (GLAPIENTRY * PFNGLGENBUFFERSPROC) (GLsizei n, GLuint* buffers);
    typedef void (GLAPIENTRY * PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
    typedef void (GLAPIENTRY * PFNGLBUFFERDATAPROC) (GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
    typedef void (GLAPIENTRY * PFNGLDELETEBUFFERSPROC) (GLsizei n, const GLuint* buffers);    PFNGLGENBUFFERSPROC genBuffers;
    typedef void (GLAPIENTRY * PFNGLDISABLEPROC) (GLenum cap);
    PFNGLBINDBUFFERPROC bindBuffer;
    PFNGLBUFFERDATAPROC bufferData;
    PFNGLDELETEBUFFERSPROC deleteBuffers;
    PFNGLDISABLEPROC disable;

    GLuint m_vertexBundle;
    GLuint m_verticesBuffer;
    GLuint m_indicesBuffer;
};
#else
class EffectEngine::RectangleRenderEngine
{
public:
    RectangleRenderEngine(const IApplicationContext::FunctionResolver * /* resolver */) {}
    ~RectangleRenderEngine() {}
    void initializeVertexBundle() {}
    void bindVertexBundle(bool /* value */) {}
    void unbindVertexBundle(bool /* value */) {}
};
#endif

/* EffectEngine */
EffectEngine::EffectEngine(Scene *sceneRef, IApplicationContext *applicationContextRef)
    : world(applicationContextRef, IApplicationContext::kWorldMatrix),
      view(applicationContextRef, IApplicationContext::kViewMatrix),
      projection(applicationContextRef, IApplicationContext::kProjectionMatrix),
      worldView(applicationContextRef, IApplicationContext::kWorldMatrix | IApplicationContext::kViewMatrix),
      viewProjection(applicationContextRef, IApplicationContext::kViewMatrix | IApplicationContext::kProjectionMatrix),
      worldViewProjection(applicationContextRef, IApplicationContext::kWorldMatrix | IApplicationContext::kViewMatrix | IApplicationContext::kProjectionMatrix),
      time(applicationContextRef),
      elapsedTime(applicationContextRef),
      controlObject(sceneRef, applicationContextRef),
      renderColorTarget(applicationContextRef),
      renderDepthStencilTarget(applicationContextRef),
      animatedTexture(applicationContextRef),
      offscreenRenderTarget(applicationContextRef),
      textureValue(applicationContextRef),
      clear(reinterpret_cast<PFNGLCLEARPROC>(applicationContextRef->sharedFunctionResolverInstance()->resolveSymbol("glClear"))),
      clearColor(reinterpret_cast<PFNGLCLEARCOLORPROC>(applicationContextRef->sharedFunctionResolverInstance()->resolveSymbol("glClearColor"))),
      clearDepth(reinterpret_cast<PFNGLCLEARDEPTHPROC>(applicationContextRef->sharedFunctionResolverInstance()->resolveSymbol("glClearDepth"))),
      m_effectRef(0),
      m_defaultStandardEffectRef(0),
      m_applicationContextRef(applicationContextRef),
      m_rectangleRenderEngine(0),
      m_frameBufferObjectRef(0),
      m_scriptOutput(kColor),
      m_scriptClass(kObject)
{
    /* prepare pre/post effect that uses rectangle (quad) rendering */
    m_rectangleRenderEngine = new RectangleRenderEngine(m_applicationContextRef->sharedFunctionResolverInstance());
    m_rectangleRenderEngine->initializeVertexBundle();
}

EffectEngine::~EffectEngine()
{
    invalidate();
#ifdef VPVL2_ENABLE_NVIDIA_CG
    internal::deleteObject(m_rectangleRenderEngine);
#endif
    m_defaultTechniques.clear();
    m_defaultStandardEffectRef = 0;
    m_applicationContextRef = 0;
}

bool EffectEngine::setEffect(IEffect *effectRef, void *userData, bool isDefaultStandardEffect)
{
    VPVL2_CHECK(effectRef);
    Hash<HashString, BaseParameter *> semantic2BaseParameterRefs, name2BaseParameterRefs;
    semantic2BaseParameterRefs.insert("VIEWPORTPIXELSIZE", &viewportPixelSize);
    semantic2BaseParameterRefs.insert("DIFFUSE", &diffuse);
    semantic2BaseParameterRefs.insert("AMBIENT", &ambient);
    semantic2BaseParameterRefs.insert("EMISSIVE", &emissive);
    semantic2BaseParameterRefs.insert("SPECULARPOWER", &specularPower);
    semantic2BaseParameterRefs.insert("SPECULAR", &specular);
    semantic2BaseParameterRefs.insert("TOONCOLOR", &toonColor);
    semantic2BaseParameterRefs.insert("EDGECOLOR", &edgeColor);
    semantic2BaseParameterRefs.insert("EDGEWIDTH", &edgeWidth);
    semantic2BaseParameterRefs.insert("ADDINGTEXTURE", &addingTexture);
    semantic2BaseParameterRefs.insert("ADDINGSPHERE", &addingSphere);
    semantic2BaseParameterRefs.insert("MULTIPLYINGTEXTURE", &multiplyTexture);
    semantic2BaseParameterRefs.insert("MULTIPLYINGSPHERE", &multiplySphere);
    semantic2BaseParameterRefs.insert("_POSITION", &position);
    semantic2BaseParameterRefs.insert("DIRECTION", &direction);
    semantic2BaseParameterRefs.insert("_DIRECTION", &direction); /* for compatibility */
    semantic2BaseParameterRefs.insert("TIME", &time);
    semantic2BaseParameterRefs.insert("ELAPSEDTIME", &elapsedTime);
    semantic2BaseParameterRefs.insert("MOUSEPOSITION", &mousePosition);
    semantic2BaseParameterRefs.insert("LEFTMOUSEDOWN", &leftMouseDown);
    semantic2BaseParameterRefs.insert("MIDDLEMOUSEDOWN", &middleMouseDown);
    semantic2BaseParameterRefs.insert("RIGHTMOUSEDOWN", &rightMouseDown);
    semantic2BaseParameterRefs.insert("CONTROLOBJECT", &controlObject);
    semantic2BaseParameterRefs.insert("ANIMATEDTEXTURE", &animatedTexture);
    semantic2BaseParameterRefs.insert("TEXTUREVALUE", &textureValue);
    semantic2BaseParameterRefs.insert("SELFSHADOWVPVM", &selfShadow);
    semantic2BaseParameterRefs.insert("TEXUNIT0", &depthTexture);
    semantic2BaseParameterRefs.insert("BONEMATRICES", &boneMatrices);
    name2BaseParameterRefs.insert("parthf", &parthf);
    name2BaseParameterRefs.insert("spadd", &spadd);
    name2BaseParameterRefs.insert("spsub", &spsub);
    name2BaseParameterRefs.insert("transp", &transp);
    name2BaseParameterRefs.insert("use_texture", &useTexture);
    name2BaseParameterRefs.insert("use_spheremap", &useSpheremap);
    name2BaseParameterRefs.insert("use_toon", &useToon);
    name2BaseParameterRefs.insert("opadd", &opadd);
    name2BaseParameterRefs.insert("VertexCount", &vertexCount);
    name2BaseParameterRefs.insert("SubsetCount", &subsetCount);
    name2BaseParameterRefs.insert("EgColor", &ambient);
    name2BaseParameterRefs.insert("SpcColor", &specular);
    IEffect::Parameter *standardsGlobal = 0;
    FrameBufferObject *frameBufferObjectRef = m_frameBufferObjectRef = effectRef->parentFrameBufferObject();
    Array<IEffect::Parameter *> parameters;
    effectRef->getParameterRefs(parameters);
    m_effectRef = effectRef;
    const int nparameters = parameters.count();
    for (int i = 0; i < nparameters; i++) {
        IEffect::Parameter *parameterRef = parameters[i];
        const char *name = parameterRef->name();
        const char *semantic = parameterRef->semantic();
        VPVL2_VLOG(2, "parameter name=" << name << " semantic=" << semantic);
        if (parameterRef->variableType() != IEffect::Parameter::kUniform) {
            continue;
        }
        const vsize slen = std::strlen(semantic);
        if (BaseParameter *const *baseParameterRef = semantic2BaseParameterRefs.find(semantic)) {
            (*baseParameterRef)->addParameter(parameterRef);
        }
        else if (VPVL2_FX_STREQ_SUFFIX(semantic, slen, kWorldViewProjectionSemantic)) {
            worldViewProjection.setParameter(parameterRef, VPVL2_FX_GET_SUFFIX(semantic, kWorldViewProjectionSemantic));
        }
        else if (VPVL2_FX_STREQ_SUFFIX(semantic, slen, kWorldViewSemantic)) {
            worldView.setParameter(parameterRef, VPVL2_FX_GET_SUFFIX(semantic, kWorldViewSemantic));
        }
        else if (VPVL2_FX_STREQ_SUFFIX(semantic, slen, kViewProjectionSemantic)) {
            viewProjection.setParameter(parameterRef, VPVL2_FX_GET_SUFFIX(semantic, kViewProjectionSemantic));
        }
        else if (VPVL2_FX_STREQ_SUFFIX(semantic, slen, kWorldSemantic)) {
            world.setParameter(parameterRef, VPVL2_FX_GET_SUFFIX(semantic, kWorldSemantic));
        }
        else if (VPVL2_FX_STREQ_SUFFIX(semantic, slen, kViewSemantic)) {
            view.setParameter(parameterRef, VPVL2_FX_GET_SUFFIX(semantic, kViewSemantic));
        }
        else if (VPVL2_FX_STREQ_SUFFIX(semantic, slen, kProjectionSemantic)) {
            projection.setParameter(parameterRef, VPVL2_FX_GET_SUFFIX(semantic, kProjectionSemantic));
        }
        else if (VPVL2_FX_STREQ_CONST(semantic, slen, "RENDERDEPTHSTENCILTARGET")) {
            renderDepthStencilTarget.addFrameBufferObjectParameter(parameterRef, frameBufferObjectRef);
        }
        else if (VPVL2_FX_STREQ_CONST(semantic, slen, "SHAREDRENDERCOLORTARGETVPVM")) {
            addSharedTextureParameter(parameterRef, frameBufferObjectRef, renderColorTarget);
        }
        else if (VPVL2_FX_STREQ_CONST(semantic, slen, "SHAREDOFFSCREENRENDERTARGETVPVM")) {
            addSharedTextureParameter(parameterRef, frameBufferObjectRef, offscreenRenderTarget);
        }
        else if (!standardsGlobal && VPVL2_FX_STREQ_CONST(semantic, slen, "STANDARDSGLOBAL")) {
            standardsGlobal = parameterRef;
        }
        else if (VPVL2_FX_STREQ_CONST(semantic, slen, "_INDEX")) {
            /* FIXME: handling _INDEX (number of vertex index) semantic */
        }
        else if (BaseParameter *const *baseParameterRef = name2BaseParameterRefs.find(name)) {
            (*baseParameterRef)->addParameter(parameterRef);
        }
        else {
            const IEffect::Parameter::Type parameterType = parameterRef->type();
            if (parameterType == IEffect::Parameter::kSampler2D ||
                    parameterType == IEffect::Parameter::kSampler3D ||
                    parameterType == IEffect::Parameter::kSamplerCube) {
                parseSamplerStateParameter(parameterRef, frameBufferObjectRef, userData);
            }
        }
        m_effectRef->addInteractiveParameter(parameterRef);
    }
    /*
     * parse STANDARDSGLOBAL semantic parameter at last to resolve parameters in
     * script process dependencies correctly
     */
    bool ownTechniques = false;
    if (standardsGlobal) {
        setStandardsGlobal(standardsGlobal, ownTechniques);
    }
    if (isDefaultStandardEffect) {
        setDefaultStandardEffectRef(effectRef);
    }
    else if (!ownTechniques) {
        Array<IEffect::Technique *> techniques;
        effectRef->getTechniqueRefs(techniques);
        const int ntechniques = techniques.count();
        for (int i = 0; i < ntechniques; i++) {
            IEffect::Technique *technique = techniques[i];
            addTechniquePasses(technique);
        }
    }
    return true;
}

void EffectEngine::invalidate()
{
    viewportPixelSize.invalidate();
    worldViewProjection.invalidate();
    worldView.invalidate();
    viewProjection.invalidate();
    world.invalidate();
    view.invalidate();
    projection.invalidate();
    diffuse.invalidate();
    ambient.invalidate();
    emissive.invalidate();
    specularPower.invalidate();
    specular.invalidate();
    toonColor.invalidate();
    edgeColor.invalidate();
    edgeWidth.invalidate();
    addingTexture.invalidate();
    addingSphere.invalidate();
    multiplyTexture.invalidate();
    multiplySphere.invalidate();
    position.invalidate();
    direction.invalidate();
    time.invalidate();
    elapsedTime.invalidate();
    mousePosition.invalidate();
    leftMouseDown.invalidate();
    middleMouseDown.invalidate();
    rightMouseDown.invalidate();
    controlObject.invalidate();
    animatedTexture.invalidate();
    textureValue.invalidate();
    renderDepthStencilTarget.invalidate();
    selfShadow.invalidate();
    parthf.invalidate();
    spadd.invalidate();
    transp.invalidate();
    useTexture.invalidate();
    useSpheremap.invalidate();
    useToon.invalidate();
    opadd.invalidate();
    vertexCount.invalidate();
    subsetCount.invalidate();
    m_target2BufferRefs.clear();
    m_target2TextureRefs.clear();
    m_passScripts.clear();
    m_techniquePasses.clear();
    m_techniques.clear();
    m_techniqueScripts.clear();
    m_frameBufferObjectRef = 0;
    m_effectRef = 0;
}

IEffect::Technique *EffectEngine::findTechnique(const char *pass,
                                                int offset,
                                                int nmaterials,
                                                bool hasTexture,
                                                bool hasSphereMap,
                                                bool useToon) const
{
    if (IEffect::Technique *technique = findTechniqueIn(m_techniques,
                                                        pass,
                                                        offset,
                                                        nmaterials,
                                                        hasTexture,
                                                        hasSphereMap,
                                                        useToon)) {
        return technique;
    }
    else if (IEffect::Technique *technique = findTechniqueIn(m_defaultTechniques,
                                                             pass,
                                                             offset,
                                                             nmaterials,
                                                             hasTexture,
                                                             hasSphereMap,
                                                             useToon)) {
        return technique;
    }
    return 0;
}

void EffectEngine::executeScriptExternal()
{
    if (scriptOrder() == IEffect::kPostProcess) {
        bool isPassExecuted; /* unused and ignored */
        executeScript(&m_externalScript, kQuadDrawCommand, 0, isPassExecuted);
    }
}

bool EffectEngine::hasTechniques(IEffect::ScriptOrderType order) const
{
    return scriptOrder() == order ? m_techniqueScripts.size() > 0 : false;
}

void EffectEngine::executeProcess(const IModel *model,
                                  IEffect *nextPostEffectRef,
                                  IEffect::ScriptOrderType order)
{
    if (m_effectRef && scriptOrder() == order) {
        setZeroGeometryParameters(model);
        diffuse.setGeometryColor(Color(0, 0, 0, model ? model->opacity() : 0)); /* for asset opacity */
        const IEffect::Technique *technique = findTechnique("object", 0, 0, false, false, false);
        executeTechniquePasses(technique, kQuadDrawCommand, nextPostEffectRef);
    }
    if (m_frameBufferObjectRef && order == IEffect::kPostProcess) {
        if (nextPostEffectRef) {
            m_frameBufferObjectRef->transferTo(nextPostEffectRef->parentFrameBufferObject());
        }
        else {
            Vector3 viewport;
            m_applicationContextRef->getViewport(viewport);
            /* clearRenderColorTargetIndices must be called before transfering render buffer to the window */
            m_effectRef->clearRenderColorTargetIndices();
            m_frameBufferObjectRef->transferToWindow(viewport);
        }
    }
}

void EffectEngine::executeTechniquePasses(const IEffect::Technique *technique,
                                          const DrawPrimitiveCommand &command,
                                          IEffect *nextPostEffectRef)
{
    if (technique) {
        const Script *tss = m_techniqueScripts.find(technique);
        bool isPassExecuted;
        executeScript(tss, command, nextPostEffectRef, isPassExecuted);
        if (!isPassExecuted) {
            if (const Passes *passes = m_techniquePasses.find(technique)) {
                const int npasses = passes->size();
                for (int i = 0; i < npasses; i++) {
                    IEffect::Pass *pass = passes->at(i);
                    const Script *pss = m_passScripts.find(pass);
                    executeScript(pss, command, nextPostEffectRef, isPassExecuted);
                }
            }
        }
    }
}

void EffectEngine::setModelMatrixParameters(const IModel *model,
                                            int extraCameraFlags,
                                            int extraLightFlags)
{
    world.setMatrices(model, extraCameraFlags, extraLightFlags);
    view.setMatrices(model, extraCameraFlags, extraLightFlags);
    projection.setMatrices(model, extraCameraFlags, extraLightFlags);
    worldView.setMatrices(model, extraCameraFlags, extraLightFlags);
    viewProjection.setMatrices(model, extraCameraFlags, extraLightFlags);
    worldViewProjection.setMatrices(model, extraCameraFlags, extraLightFlags);
}

void EffectEngine::setDefaultStandardEffectRef(IEffect *effectRef)
{
    if (!m_defaultStandardEffectRef) {
        Array<IEffect::Technique *> techniques;
        effectRef->getTechniqueRefs(techniques);
        const int ntechniques = techniques.count();
        Passes passes;
        for (int i = 0; i < ntechniques; i++) {
            IEffect::Technique *technique = techniques[i];
            if (parseTechniqueScript(technique, passes)) {
                m_techniquePasses.insert(technique, passes);
                m_defaultTechniques.append(technique);
            }
            passes.clear();
        }
        m_defaultStandardEffectRef = effectRef;
    }
}

void EffectEngine::setZeroGeometryParameters(const IModel *model)
{
    edgeColor.setGeometryColor(model ? model->edgeColor() : kZeroC);
    toonColor.setGeometryColor(kZeroC);
    ambient.setGeometryColor(kZeroC);
    diffuse.setGeometryColor(kZeroC);
    emissive.setGeometryColor(kZeroC);
    specular.setGeometryColor(kZeroC);
    specularPower.setGeometryValue(0);
    spadd.setValue(false);
    useTexture.setValue(false);
    useSpheremap.setValue(false);
}

void EffectEngine::updateModelLightParameters(const Scene *scene, const IModel *model)
{
    const ILight *light = scene->lightRef();
    const Vector3 &lc = light->color();
    Color lightColor(lc.x(), lc.y(), lc.z(), 1);
    if (model && model->type() == IModel::kAssetModel) {
        const Vector3 &ac = Vector3(0.7f, 0.7f, 0.7f) - lightColor;
        ambient.setLightColor(Color(ac.x(), ac.y(), ac.z(), 1));
        diffuse.setLightColor(Color(1, 1, 1, 1));
        specular.setLightColor(lightColor);
    }
    else {
        ambient.setLightColor(lightColor);
        diffuse.setLightColor(kZeroC);
        specular.setLightColor(lightColor);
    }
    emissive.setLightColor(kZeroC);
    edgeColor.setLightColor(kZeroC);
    const Vector3 &lightDirection = light->direction();
    position.setLightValue(-lightDirection);
    direction.setLightValue(lightDirection.normalized());
    const ICamera *camera = scene->cameraRef();
    const Vector3 &cameraPosition = camera->modelViewTransform().getOrigin();
    position.setCameraValue(cameraPosition);
    direction.setCameraValue((cameraPosition - camera->lookAt()).normalized());
    controlObject.update(model);
}

void EffectEngine::updateSceneParameters()
{
    Vector3 viewport;
    m_applicationContextRef->getViewport(viewport);
    viewportPixelSize.setValue(viewport);
    Vector4 position;
    m_applicationContextRef->getMousePosition(position, IApplicationContext::kMouseCursorPosition);
    mousePosition.setValue(position);
    m_applicationContextRef->getMousePosition(position, IApplicationContext::kMouseLeftPressPosition);
    leftMouseDown.setValue(position);
    m_applicationContextRef->getMousePosition(position, IApplicationContext::kMouseMiddlePressPosition);
    middleMouseDown.setValue(position);
    m_applicationContextRef->getMousePosition(position, IApplicationContext::kMouseRightPressPosition);
    rightMouseDown.setValue(position);
    time.update();
    elapsedTime.update();
    textureValue.update();
    animatedTexture.update(renderColorTarget);
}

bool EffectEngine::isStandardEffect() const
{
    return scriptOrder() == IEffect::kStandard;
}

const EffectEngine::Script *EffectEngine::findTechniqueScript(const IEffect::Technique *technique) const
{
    return m_techniqueScripts.find(technique);
}

const EffectEngine::Script *EffectEngine::findPassScript(const IEffect::Pass *pass) const
{
    return m_passScripts.find(pass);
}

bool EffectEngine::containsSubset(const IEffect::Annotation *annotation, int subset, int nmaterials)
{
    if (annotation) {
        const std::string s(annotation->stringValue());
        if (s.empty()) {
            return true;
        }
        std::istringstream stream(s);
        std::string segment;
        while (std::getline(stream, segment, ',')) {
            if (strtol(segment.c_str(), 0, 10) == subset) {
                return true;
            }
            std::string::size_type offset = segment.find("-");
            if (offset != std::string::npos) {
                int from = strtol(segment.substr(0, offset).c_str(), 0, 10);
                int to = strtol(segment.substr(offset + 1).c_str(), 0, 10);
                if (to == 0) {
                    to = nmaterials;
                }
                if (from > to) {
                    std::swap(from, to);
                }
                if (from <= subset && subset <= to) {
                    return true;
                }
            }
        }
        return false;
    }
    return true;
}

bool EffectEngine::testTechnique(const IEffect::Technique *technique,
                                 const char *pass,
                                 int offset,
                                 int nmaterials,
                                 bool hasTexture,
                                 bool hasSphereMap,
                                 bool useToon)
{
    if (technique) {
        int ok = 1;
        const IEffect::Annotation *annonMMDPass = technique->annotationRef("MMDPass");
        ok &= Util::isPassEquals(annonMMDPass, pass) ? 1 : 0;
        const IEffect::Annotation *annonSubset = technique->annotationRef("Subset");
        ok &= containsSubset(annonSubset, offset, nmaterials) ? 1 : 0;
        if (const IEffect::Annotation *annotationRef = technique->annotationRef("UseTexture")) {
            ok &= annotationRef->booleanValue() == hasTexture ? 1 : 0;
        }
        if (const IEffect::Annotation *annotationRef = technique->annotationRef("UseSphereMap")) {
            ok &= annotationRef->booleanValue() == hasSphereMap ? 1 : 0;
        }
        if (const IEffect::Annotation *annotationRef = technique->annotationRef("UseToon")) {
            ok &= annotationRef->booleanValue() == useToon ? 1 : 0;
        }
        return ok == 1;
    }
    return false;
}

IEffect::Technique *EffectEngine::findTechniqueIn(const Techniques &techniques,
                                                  const char *pass,
                                                  int offset,
                                                  int nmaterials,
                                                  bool hasTexture,
                                                  bool hasSphereMap,
                                                  bool useToon)
{
    const int ntechniques = techniques.count();
    for (int i = 0; i < ntechniques; i++) {
        IEffect::Technique *technique = techniques[i];
        if (testTechnique(technique, pass, offset, nmaterials, hasTexture, hasSphereMap, useToon)) {
            return technique;
        }
    }
    return 0;
}

void EffectEngine::setScriptStateFromRenderColorTargetSemantic(const RenderColorTargetSemantic &semantic,
                                                               const std::string &value,
                                                               ScriptState::Type type,
                                                               ScriptState &state)
{
    bool bound = false;
    state.type = type;
    if (!value.empty()) {
        if (const RenderColorTargetSemantic::TextureReference *texture = semantic.findTexture(value.c_str())) {
            state.renderColorTargetTextureRef = texture;
            m_target2TextureRefs.insert(type, texture);
            bound = true;
        }
    }
    else if (const RenderColorTargetSemantic::TextureReference *const *texturePtr = m_target2TextureRefs.find(type)) {
        const RenderColorTargetSemantic::TextureReference *texture = *texturePtr;
        state.renderColorTargetTextureRef = texture;
        m_target2TextureRefs.remove(type);
    }
    state.isRenderTargetBound = bound;
}

void EffectEngine::setScriptStateFromRenderDepthStencilTargetSemantic(const RenderDepthStencilTargetSemantic &semantic,
                                                                      const std::string &value,
                                                                      ScriptState::Type type,
                                                                      ScriptState &state)
{
    bool bound = false;
    state.type = type;
    if (!value.empty()) {
        if (const RenderDepthStencilTargetSemantic::Buffer *buffer = semantic.findDepthStencilBuffer(value.c_str())) {
            state.renderDepthStencilBufferRef = buffer;
            m_target2BufferRefs.insert(type, buffer);
            bound = true;
        }
    }
    else if (const RenderDepthStencilTargetSemantic::Buffer *const *bufferPtr = m_target2BufferRefs.find(type)) {
        const RenderDepthStencilTargetSemantic::Buffer *buffer = *bufferPtr;
        state.renderDepthStencilBufferRef = buffer;
        m_target2BufferRefs.remove(type);
    }
    state.isRenderTargetBound = bound;
}

void EffectEngine::setScriptStateFromParameter(const IEffect *effectRef,
                                               const std::string &value,
                                               IEffect::Parameter::Type testType,
                                               ScriptState::Type type,
                                               ScriptState &state)
{
    IEffect::Parameter *parameter = effectRef->findUniformParameter(value.c_str());
    if (parameter && parameter->type() == testType) {
        state.type = type;
        state.parameter = parameter;
    }
}

void EffectEngine::executePass(IEffect::Pass *pass, const DrawPrimitiveCommand &command) const
{
    if (pass) {
        pass->setState();
        drawPrimitives(command);
        pass->resetState();
    }
}

void EffectEngine::setRenderColorTargetFromScriptState(const ScriptState &state)
{
    Vector3 viewport;
    m_applicationContextRef->getViewport(viewport);
    if (const RenderColorTargetSemantic::TextureReference *textureRef = state.renderColorTargetTextureRef) {
        const int index = state.type - ScriptState::kRenderColorTarget0, targetIndex = FrameBufferObject::kGL_COLOR_ATTACHMENT0 + index;
        if (FrameBufferObject *fbo = textureRef->frameBufferObjectRef) {
            if (state.isRenderTargetBound) {
                fbo->readMSAABuffer(index);
                fbo->bindTexture(textureRef->textureRef, index);
                fbo->resize(viewport, index);
                if (!m_effectRef->hasRenderColorTargetIndex(targetIndex)) {
                    m_effectRef->addRenderColorTargetIndex(targetIndex);
                }
            }
            else {
                fbo->readMSAABuffer(index);
                fbo->unbindTexture(index);
                fbo->unbind();
                m_effectRef->removeRenderColorTargetIndex(index);
            }
        }
    }
    else if (m_frameBufferObjectRef) {
        setDefaultRenderTarget(viewport);
    }
}

void EffectEngine::setRenderDepthStencilTargetFromScriptState(const ScriptState &state)
{
    if (const RenderDepthStencilTargetSemantic::Buffer *bufferRef = state.renderDepthStencilBufferRef) {
        if (FrameBufferObject *fbo = bufferRef->frameBufferObjectRef) {
            if (state.isRenderTargetBound) {
                fbo->bindDepthStencilBuffer(bufferRef->renderBufferRef);
            }
            else {
                fbo->unbindDepthStencilBuffer();
                fbo->unbind();
            }
        }
    }
    else if (m_frameBufferObjectRef) {
        Vector3 viewport;
        m_applicationContextRef->getViewport(viewport);
        setDefaultRenderTarget(viewport);
    }
}

void EffectEngine::setDefaultRenderTarget(const Vector3 &viewport)
{
    m_frameBufferObjectRef->create(viewport);
    m_frameBufferObjectRef->unbind();
    m_effectRef->clearRenderColorTargetIndices();
    m_effectRef->addRenderColorTargetIndex(FrameBufferObject::kGL_COLOR_ATTACHMENT0);
}

void EffectEngine::executeScript(const Script *script,
                                 const DrawPrimitiveCommand &command,
                                 IEffect *nextPostEffectRef,
                                 bool &isPassExecuted)
{
    isPassExecuted = scriptOrder() == IEffect::kPostProcess;
    if (script) {
        const int nstates = script->size();
        int stateIndex = 0, nloop = 0, currentIndex = 0, backStateIndex = 0;
        Vector4 v4;
        Vector3 viewport;
        m_applicationContextRef->getViewport(viewport);
        while (stateIndex < nstates) {
            const ScriptState &state = script->at(stateIndex);
            switch (state.type) {
            case ScriptState::kClearColor:
                clear(kGL_COLOR_BUFFER_BIT);
                break;
            case ScriptState::kClearDepth:
                clear(kGL_DEPTH_BUFFER_BIT | kGL_STENCIL_BUFFER_BIT);
                break;
            case ScriptState::kClearSetColor:
                if (const IEffect::Parameter *parameter = state.parameter) {
                    parameter->getValue(v4);
                    clearColor(v4.x(), v4.y(), v4.z(), v4.w());
                }
                break;
            case ScriptState::kClearSetDepth:
                if (const IEffect::Parameter *parameter = state.parameter) {
                    float depth;
                    parameter->getValue(depth);
                    clearDepth(depth);
                }
                break;
            case ScriptState::kLoopByCount:
                if (const IEffect::Parameter *parameter = state.parameter) {
                    parameter->getValue(nloop);
                    backStateIndex = stateIndex + 1;
                    currentIndex = 0;
                }
                break;
            case ScriptState::kLoopEnd:
                if (--nloop >= 0) {
                    stateIndex = backStateIndex;
                    ++currentIndex;
                    continue;
                }
                break;
            case ScriptState::kLoopGetIndex:
                if (IEffect::Parameter *parameter = state.parameter) {
                    parameter->setValue(currentIndex);
                }
                break;
            case ScriptState::kRenderColorTarget0:
            case ScriptState::kRenderColorTarget1:
            case ScriptState::kRenderColorTarget2:
            case ScriptState::kRenderColorTarget3:
                setRenderColorTargetFromScriptState(state);
                break;
            case ScriptState::kRenderDepthStencilTarget:
                setRenderDepthStencilTargetFromScriptState(state);
                break;
            case ScriptState::kDrawBuffer:
                if (m_scriptClass != kObject) {
                    m_rectangleRenderEngine->bindVertexBundle(true);
                    executePass(state.pass, kQuadDrawCommand);
                    m_rectangleRenderEngine->unbindVertexBundle(true);
                    rebindVertexBundle();
                }
                break;
            case ScriptState::kDrawGeometry:
                if (m_scriptClass != kScene) {
                    executePass(state.pass, command);
                }
                break;
            case ScriptState::kPass:
                executeScript(m_passScripts.find(state.pass), command, nextPostEffectRef, isPassExecuted);
                isPassExecuted = true;
                break;
            case ScriptState::kScriptExternal:
            case ScriptState::kUnknown:
            default:
                break;
            }
            stateIndex++;
        }
    }
}

void EffectEngine::addTechniquePasses(IEffect::Technique *technique)
{
    Passes passes;
    if (parseTechniqueScript(technique, passes)) {
        m_techniquePasses.insert(technique, passes);
        m_techniques.append(technique);
    }
}

void EffectEngine::setStandardsGlobal(const IEffect::Parameter *parameterRef, bool &ownTechniques)
{
    float version;
    parameterRef->getValue(version);
    if (!btFuzzyZero(version - 0.8f)) {
        return;
    }
    if (const IEffect::Annotation *annotationRef = parameterRef->annotationRef("ScriptClass")) {
        const char *value = annotationRef->stringValue();
        const vsize len = std::strlen(value);
        if (VPVL2_FX_STREQ_CONST(value, len, "object")) {
            m_scriptClass = kObject;
        }
        else if (VPVL2_FX_STREQ_CONST(value, len, "scene")) {
            m_scriptClass = kScene;
        }
        else if (VPVL2_FX_STREQ_CONST(value, len, "sceneorobject")) {
            m_scriptClass = kSceneOrObject;
        }
    }
    if (const IEffect::Annotation *annotationRef = parameterRef->annotationRef("ScriptOrder")) {
        const char *value = annotationRef->stringValue();
        const vsize len = std::strlen(value);
        if (VPVL2_FX_STREQ_CONST(value, len, "standard")) {
            m_effectRef->setScriptOrderType(IEffect::kStandard);
        }
        else if (VPVL2_FX_STREQ_CONST(value, len, "preprocess")) {
            m_effectRef->setScriptOrderType(IEffect::kPreProcess);
        }
        else if (VPVL2_FX_STREQ_CONST(value, len, "postprocess")) {
            m_effectRef->setScriptOrderType(IEffect::kPostProcess);
        }
    }
    if (const IEffect::Annotation *annotationRef = parameterRef->annotationRef("Script")) {
        const char *value = annotationRef->stringValue();
        const vsize len = std::strlen(value);
        m_techniques.clear();
        if (VPVL2_FX_STREQ_SUFFIX(value, len, kMultipleTechniquesPrefix)) {
            const std::string &s = Util::trimLastSemicolon(VPVL2_FX_GET_SUFFIX(value, kMultipleTechniquesPrefix));
            std::istringstream stream(s);
            std::string segment;
            while (std::getline(stream, segment, ':')) {
                IEffect::Technique *technique = m_effectRef->findTechnique(segment.c_str());
                addTechniquePasses(technique);
            }
            ownTechniques = true;
        }
        else if (VPVL2_FX_STREQ_SUFFIX(value, len, kSingleTechniquePrefix)) {
            const std::string &s = Util::trimLastSemicolon(VPVL2_FX_GET_SUFFIX(value, kSingleTechniquePrefix));
            IEffect::Technique *technique = parameterRef->parentEffectRef()->findTechnique(s.c_str());
            addTechniquePasses(technique);
            ownTechniques = true;
        }
    }
}

void EffectEngine::parseSamplerStateParameter(IEffect::Parameter *samplerParameterRef,
                                              FrameBufferObject *frameBufferObjectRef,
                                              void *userData)
{
    Array<IEffect::SamplerState *> states;
    samplerParameterRef->getSamplerStateRefs(states);
    const int nstates = states.count();
    for (int i = 0; i < nstates; i++) {
        const IEffect::SamplerState *samplerState = states[i];
        if (samplerState->type() == IEffect::Parameter::kTexture) {
            IEffect::Parameter *textureParameterRef = samplerState->parameterRef();
            const char *semantic = textureParameterRef->semantic();
            const vsize len = std::strlen(semantic);
            if (VPVL2_FX_STREQ_CONST(semantic, len, "MATERIALTEXTURE")) {
                materialTexture.addTextureParameter(textureParameterRef, samplerParameterRef);
            }
            else if (VPVL2_FX_STREQ_CONST(semantic, len, "MATERIALTOONTEXTURE")) {
                materialToonTexture.addTextureParameter(textureParameterRef, samplerParameterRef);
            }
            else if (VPVL2_FX_STREQ_CONST(semantic, len, "MATERIALSPHEREMAP")) {
                materialSphereMap.addTextureParameter(textureParameterRef, samplerParameterRef);
            }
            else if (VPVL2_FX_STREQ_CONST(semantic, len, "RENDERCOLORTARGET")) {
                renderColorTarget.addFrameBufferObjectParameter(textureParameterRef,
                                                                samplerParameterRef,
                                                                frameBufferObjectRef,
                                                                0, false, false);
            }
            else if (VPVL2_FX_STREQ_CONST(semantic, len, "OFFSCREENRENDERTARGET")) {
                offscreenRenderTarget.addFrameBufferObjectParameter(textureParameterRef,
                                                                    samplerParameterRef,
                                                                    frameBufferObjectRef,
                                                                    0, false, false);
            }
            else {
                renderColorTarget.addFrameBufferObjectParameter(textureParameterRef,
                                                                samplerParameterRef,
                                                                frameBufferObjectRef,
                                                                userData, true, true);
            }
            break;
        }
    }
}

void EffectEngine::addSharedTextureParameter(IEffect::Parameter *textureParameterRef,
                                             FrameBufferObject *frameBufferObjectRef,
                                             RenderColorTargetSemantic &semantic)
{
    if (textureParameterRef) {
        const char *name = textureParameterRef->name();
        IApplicationContext::SharedTextureParameter sharedTextureParameter(textureParameterRef);
        if (!m_applicationContextRef->tryGetSharedTextureParameter(name, sharedTextureParameter)) {
            sharedTextureParameter.parameterRef = textureParameterRef;
            semantic.addFrameBufferObjectParameter(textureParameterRef, 0, frameBufferObjectRef, 0, false, false);
            if (const RenderColorTargetSemantic::TextureReference *texture = semantic.findTexture(name)) {
                /* parse semantic first and add shared parameter not to fetch unparsed semantic parameter at RenderColorTarget#addParameter */
                sharedTextureParameter.textureRef = texture->textureRef;
                m_applicationContextRef->addSharedTextureParameter(name, sharedTextureParameter);
            }
        }
    }
}

bool EffectEngine::parsePassScript(IEffect::Pass *pass)
{
    if (m_passScripts[pass]) {
        return true;
    }
    if (pass) {
        Script passScriptStates;
        if (const IEffect::Annotation *annotationRef = pass->annotationRef("Script")) {
            const std::string s(annotationRef->stringValue()), passName(pass->name());
            ScriptState lastState, newState;
            std::istringstream stream(s);
            std::string segment;
            int stateIndex = 1;
            bool renderColorTarget0DidSet = false;
            while (std::getline(stream, segment, ';')) {
                std::string::size_type offset = segment.find("=");
                if (offset != std::string::npos) {
                    const std::string &command = Util::trim(segment.substr(0, offset));
                    const std::string &value = Util::trim(segment.substr(offset + 1));
                    newState.setFromState(lastState);
                    if (command == "RenderColorTarget" || command == "RenderColorTarget0") {
                        setScriptStateFromRenderColorTargetSemantic(renderColorTarget,
                                                                    value,
                                                                    ScriptState::kRenderColorTarget0,
                                                                    newState);
                        renderColorTarget0DidSet = true;
                        VPVL2_VLOG(2, "SAS.RenderColorTarget0: value=" << value << " pass=" << passName << " line=" << stateIndex);
                    }
                    else if (renderColorTarget0DidSet && command == "RenderColorTarget1") {
                        setScriptStateFromRenderColorTargetSemantic(renderColorTarget,
                                                                    value,
                                                                    ScriptState::kRenderColorTarget1,
                                                                    newState);
                        VPVL2_VLOG(2, "SAS.RenderColorTarget1: value=" << value << " pass=" << passName << " line=" << stateIndex);
                    }
                    else if (renderColorTarget0DidSet && command == "RenderColorTarget2") {
                        setScriptStateFromRenderColorTargetSemantic(renderColorTarget,
                                                                    value,
                                                                    ScriptState::kRenderColorTarget2,
                                                                    newState);
                        VPVL2_VLOG(2, "SAS.RenderColorTarget2: value=" << value << " pass=" << passName << " line=" << stateIndex);
                    }
                    else if (renderColorTarget0DidSet && command == "RenderColorTarget3") {
                        setScriptStateFromRenderColorTargetSemantic(renderColorTarget,
                                                                    value,
                                                                    ScriptState::kRenderColorTarget3,
                                                                    newState);
                        VPVL2_VLOG(2, "SAS.RenderColorTarget3: value=" << value << " pass=" << passName << " line=" << stateIndex);
                    }
                    else if (command == "RenderDepthStencilTarget") {
                        setScriptStateFromRenderDepthStencilTargetSemantic(renderDepthStencilTarget,
                                                                           value,
                                                                           ScriptState::kRenderDepthStencilTarget,
                                                                           newState);
                        VPVL2_VLOG(2, "SAS.RenderDepthStencilTarget: value=" << value << " pass=" << passName << " line=" << stateIndex);
                    }
                    else if (command == "ClearSetColor") {
                        setScriptStateFromParameter(m_effectRef, value, IEffect::Parameter::kFloat4, ScriptState::kClearSetColor, newState);
                        VPVL2_VLOG(2, "SAS.ClearSetColor: value=" << value << " pass=" << passName << " line=" << stateIndex);
                    }
                    else if (command == "ClearSetDepth") {
                        setScriptStateFromParameter(m_effectRef, value, IEffect::Parameter::kFloat, ScriptState::kClearSetDepth, newState);
                        VPVL2_VLOG(2, "SAS.ClearSetDepth: value=" << value << " pass=" << passName << " line=" << stateIndex);
                    }
                    else if (command == "Clear") {
                        if (value == "Color") {
                            newState.type = ScriptState::kClearColor;
                            VPVL2_VLOG(2, "SAS.ClearColor: pass=" << passName << " line=" << stateIndex);
                        }
                        else if (value == "Depth") {
                            newState.type = ScriptState::kClearDepth;
                            VPVL2_VLOG(2, "SAS.ClearDepth: pass=" << passName << " line=" << stateIndex);
                        }
                        else {
                            VPVL2_LOG(WARNING, "SAS.Clear is called with invalid value: value=" << value << " pass=" << passName << " line=" << stateIndex);
                        }
                    }
                    else if (command == "Draw") {
                        if (value == "Buffer") {
                            if (m_scriptClass == kObject) {
                                VPVL2_VLOG(2, "SAS.DrawBuffer is called while the script class of this class is object: pass=" << passName << " line=" << stateIndex);
                                return false;
                            }
                            newState.type = ScriptState::kDrawBuffer;
                            VPVL2_VLOG(2, "SAS.DrawBuffer: pass=" << passName << " line=" << stateIndex);
                        }
                        if (value == "Geometry") {
                            if (m_scriptClass == kScene) {
                                VPVL2_VLOG(2, "SAS.DrawGeometry is called while the script class of this class is scene: pass=" << passName << " line=" << stateIndex);
                                return false;
                            }
                            newState.type = ScriptState::kDrawGeometry;
                            VPVL2_VLOG(2, "SAS.DrawGeometry: pass=" << passName << " line=" << stateIndex);
                        }
                        newState.pass = pass;
                    }
                    if (newState.type != ScriptState::kUnknown) {
                        lastState = newState;
                        passScriptStates.push_back(newState);
                        newState.reset();
                    }
                    else if (command != "Clear") {
                        VPVL2_LOG(WARNING, "Unknown SAS command is found: command=" << command << " pass=" << passName << " line=" << stateIndex);
                    }
                    stateIndex++;
                }
            }
        }
        else {
            /* just draw geometry primitives */
            if (m_scriptClass == kScene) {
                return false;
            }
            ScriptState state;
            state.pass = pass;
            state.type = ScriptState::kDrawGeometry;
            passScriptStates.push_back(state);
        }
        m_passScripts.insert(pass, passScriptStates);
        return true;
    }
    return false;
}

bool EffectEngine::parseTechniqueScript(const IEffect::Technique *technique, Passes &passes)
{
    /* just check only it's technique object for technique without pass */
    if (technique) {
        Script techniqueScriptStates;
        if (const IEffect::Annotation *annotationRef = technique->annotationRef("Script")) {
            const std::string s(annotationRef->stringValue()), techniqueName(technique->name());
            std::istringstream stream(s);
            std::string segment;
            Script scriptExternalStates;
            ScriptState lastState, newState;
            int stateIndex = 1;
            bool useScriptExternal = scriptOrder() == IEffect::kPostProcess,
                    renderColorTarget0DidSet = false,
                    renderDepthStencilTargetDidSet = false;
            while (std::getline(stream, segment, ';')) {
                std::string::size_type offset = segment.find("=");
                if (offset != std::string::npos) {
                    const std::string &command = Util::trim(segment.substr(0, offset));
                    const std::string &value = Util::trim(segment.substr(offset + 1));
                    newState.setFromState(lastState);
                    if (command == "RenderColorTarget" || command == "RenderColorTarget0") {
                        setScriptStateFromRenderColorTargetSemantic(renderColorTarget,
                                                                    value,
                                                                    ScriptState::kRenderColorTarget0,
                                                                    newState);
                        renderColorTarget0DidSet = true;
                        VPVL2_VLOG(2, "SAS.RenderColorTarget0: value=" << value << " technique=" << techniqueName << " line=" << stateIndex);
                    }
                    else if (renderColorTarget0DidSet && command == "RenderColorTarget1") {
                        setScriptStateFromRenderColorTargetSemantic(renderColorTarget,
                                                                    value,
                                                                    ScriptState::kRenderColorTarget1,
                                                                    newState);
                        VPVL2_VLOG(2, "SAS.RenderColorTarget1: value=" << value << " technique=" << techniqueName << " line=" << stateIndex);
                    }
                    else if (renderColorTarget0DidSet && command == "RenderColorTarget2") {
                        setScriptStateFromRenderColorTargetSemantic(renderColorTarget,
                                                                    value,
                                                                    ScriptState::kRenderColorTarget2,
                                                                    newState);
                        VPVL2_VLOG(2, "SAS.RenderColorTarget2: value=" << value << " technique=" << techniqueName << " line=" << stateIndex);
                    }
                    else if (renderColorTarget0DidSet && command == "RenderColorTarget3") {
                        setScriptStateFromRenderColorTargetSemantic(renderColorTarget,
                                                                    value,
                                                                    ScriptState::kRenderColorTarget3,
                                                                    newState);
                        VPVL2_VLOG(2, "SAS.RenderColorTarget3: value=" << value << " technique=" << techniqueName << " line=" << stateIndex);
                    }
                    else if (command == "RenderDepthStencilTarget") {
                        setScriptStateFromRenderDepthStencilTargetSemantic(renderDepthStencilTarget,
                                                                           value,
                                                                           ScriptState::kRenderDepthStencilTarget,
                                                                           newState);
                        renderDepthStencilTargetDidSet = true;
                        VPVL2_VLOG(2, "SAS.RenderDepthStencilTarget: value=" << value << " technique=" << techniqueName << " line=" << stateIndex);
                    }
                    else if (command == "ClearSetColor") {
                        setScriptStateFromParameter(m_effectRef, value, IEffect::Parameter::kFloat4, ScriptState::kClearSetColor, newState);
                        VPVL2_VLOG(2, "SAS.ClearSetColor: technique=" << techniqueName << " line=" << stateIndex);
                    }
                    else if (command == "ClearSetDepth") {
                        setScriptStateFromParameter(m_effectRef, value, IEffect::Parameter::kFloat, ScriptState::kClearSetDepth, newState);
                        VPVL2_VLOG(2, "SAS.ClearSetDepth: technique=" << techniqueName << " line=" << stateIndex);
                    }
                    else if (command == "Clear") {
                        if (value == "Color") {
                            newState.type = ScriptState::kClearColor;
                            VPVL2_VLOG(2, "SAS.ClearColor: technique=" << techniqueName << " line=" << stateIndex);
                        }
                        else if (value == "Depth") {
                            newState.type = ScriptState::kClearDepth;
                            VPVL2_VLOG(2, "SAS.ClearDepth: technique=" << techniqueName << " line=" << stateIndex);
                        }
                        else {
                            VPVL2_LOG(WARNING, "SAS.Clear is called with invalid value: value=" << value << " technique=" << techniqueName << " line=" << stateIndex);
                        }
                    }
                    else if (command == "Pass") {
                        IEffect::Pass *pass = technique->findPass(value.c_str());
                        if (parsePassScript(pass)) {
                            newState.type = ScriptState::kPass;
                            newState.pass = pass;
                            passes.push_back(pass);
                            VPVL2_VLOG(2, "SAS.Pass: pass=" << value << " technique=" << techniqueName << " line=" << stateIndex);
                        }
                        else {
                            VPVL2_LOG(WARNING, "SAS.Pass: pass cannot be parsed: pass=" << value << " technique=" << techniqueName << " line=" << stateIndex);
                        }
                    }
                    else if (!lastState.enterLoop && command == "LoopByCount") {
                        IEffect::Parameter *parameter = m_effectRef->findUniformParameter(value.c_str());
                        if (Util::isIntegerParameter(parameter)) {
                            newState.type = ScriptState::kLoopByCount;
                            newState.enterLoop = true;
                            newState.parameter = parameter;
                            VPVL2_VLOG(2, "SAS.LoopByCount: parameter=" << value << " technique=" << techniqueName << " line=" << stateIndex);
                        }
                        else {
                            VPVL2_LOG(WARNING, "SAS.LoopByCount parameter is not found, or the parameter is not integer: parameter=" << value << " technique=" << techniqueName << " line=" << stateIndex);
                        }
                    }
                    else if (lastState.enterLoop && command == "LoopEnd") {
                        newState.type = ScriptState::kLoopEnd;
                        newState.enterLoop = false;
                        VPVL2_VLOG(2, "SAS.LoopEnd: technique=" << techniqueName << " line=" << stateIndex);
                    }
                    else if (lastState.enterLoop && command == "LoopGetIndex") {
                        IEffect::Parameter *parameter = m_effectRef->findUniformParameter(value.c_str());
                        if (Util::isIntegerParameter(parameter)) {
                            newState.type = ScriptState::kLoopGetIndex;
                            newState.enterLoop = true;
                            newState.parameter = parameter;
                            VPVL2_VLOG(2, "SAS.LoopGetIndex: parameter=" << value << " technique=" << techniqueName << " line=" << stateIndex);
                        }
                        else {
                            VPVL2_LOG(WARNING, "SAS.LoopGetIndex parameter is not found, or the parameter is not integer: parameter=" << value << " technique=" << techniqueName << " line=" << stateIndex);
                        }
                    }
                    else if (useScriptExternal && command == "ScriptExternal") {
                        newState.type = ScriptState::kScriptExternal;
                        useScriptExternal = false;
                        if (lastState.enterLoop) {
                            VPVL2_VLOG(2, "SAS.ScriptExternal is in the loop and aborted parsing: technique=" << techniqueName << " line=" << stateIndex);
                            return false;
                        }
                        else {
                            VPVL2_VLOG(2, "SAS.ScriptExternal: technique=" << techniqueName << " line=" << stateIndex);
                        }
                    }
                    if (newState.type != ScriptState::kUnknown) {
                        lastState = newState;
                        if (useScriptExternal) {
                            scriptExternalStates.push_back(newState);
                        }
                        else {
                            techniqueScriptStates.push_back(newState);
                        }
                        newState.reset();
                    }
                    else if (command != "Clear") {
                        VPVL2_LOG(WARNING, "Unknown SAS command is found: command=" << command << " technique=" << techniqueName << " line=" << stateIndex);
                    }
                }
                stateIndex++;
            }
            m_techniqueScripts.insert(technique, techniqueScriptStates);
            if (m_externalScript.size() == 0) {
                m_externalScript.copyFromArray(scriptExternalStates);
            }
            return !lastState.enterLoop;
        }
        else {
            ScriptState state;
            Array<IEffect::Pass *> passesToParse;
            technique->getPasses(passesToParse);
            const int npasses = passesToParse.count();
            for (int i = 0; i < npasses; i++) {
                IEffect::Pass *pass = passesToParse[i];
                if (parsePassScript(pass)) {
                    state.type = ScriptState::kPass;
                    state.pass = pass;
                    passes.push_back(pass);
                }
            }
        }
        return true;
    }
    return false;
}

/* EffectEngine::ScriptState */

EffectEngine::ScriptState::ScriptState()
    : type(kUnknown),
      renderColorTargetTextureRef(0),
      renderDepthStencilBufferRef(0),
      parameter(0),
      pass(0),
      enterLoop(false),
      isRenderTargetBound(false)
{
}

EffectEngine::ScriptState::~ScriptState()
{
    reset();
}

void EffectEngine::ScriptState::reset()
{
    type = kUnknown;
    renderColorTargetTextureRef = 0;
    renderDepthStencilBufferRef = 0;
    parameter = 0;
    pass = 0;
    enterLoop = false;
    isRenderTargetBound = false;
}

void EffectEngine::ScriptState::setFromState(const ScriptState &other)
{
    enterLoop = other.enterLoop;
    isRenderTargetBound = other.isRenderTargetBound;
}

} /* namespace fx */
} /* namespace vpvl2 */
