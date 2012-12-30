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

#include "vpvl2/cg/EffectEngine.h"

#include "vpvl2/Common.h"
#include "vpvl2/Scene.h"
#include "vpvl2/IBone.h"
#include "vpvl2/ICamera.h"
#include "vpvl2/IEffect.h"
#include "vpvl2/ILight.h"
#include "vpvl2/IModel.h"
#include "vpvl2/IMorph.h"
#include "vpvl2/IRenderContext.h"
#include "vpvl2/IRenderEngine.h"
#include "vpvl2/IString.h"
#include "vpvl2/internal/BaseRenderEngine.h"

#include <string>
#include <sstream>

#if defined(VPVL2_LINK_QT)
#include <QtOpenGL/QtOpenGL>
#endif /* VPVL_LINK_QT */

#define VPVL2_CG_GET_LENGTH_CONST(s) (sizeof(s) - 1)
#define VPVL2_CG_GET_SUFFIX(s, c) (s + VPVL2_CG_GET_LENGTH_CONST(c))
#define VPVL2_CG_STREQ_CONST(s, l, c) (l == VPVL2_CG_GET_LENGTH_CONST(c) && \
    0 == strncmp((s), (c), VPVL2_CG_GET_LENGTH_CONST(c)))
#define VPVL2_CG_STREQ_SUFFIX(s, l, c) (l >= VPVL2_CG_GET_LENGTH_CONST(c) && \
    0 == strncmp((s), (c), VPVL2_CG_GET_LENGTH_CONST(c)))

namespace
{

using namespace vpvl2;

static const Scalar kWidth = 1, kHeight = 1;
static const Vector4 kVertices[] = {
    btVector4(-kWidth,  kHeight,  0,  1),
    btVector4(-kWidth, -kHeight,  0,  0),
    btVector4( kWidth, -kHeight,  1,  0),
    btVector4( kWidth,  kHeight,  1,  1)
};
static const size_t kVertexStride = sizeof(kVertices[0]);
static const int kIndices[] = { 0, 1, 2, 3 };
static const uint8_t *kBaseAddress = reinterpret_cast<const uint8_t *>(&kVertices[0]);
static const size_t kTextureOffset = reinterpret_cast<const uint8_t *>(&kVertices[0].z()) - kBaseAddress;
static const size_t kIndicesSize = sizeof(kIndices) / sizeof(kIndices[0]);
static const int kBaseRenderColorTargetIndex = GL_COLOR_ATTACHMENT0;
static const bool kEnableRTAA = true;

}

namespace vpvl2
{
namespace cg
{

/* Util */
bool Util::toBool(const CGannotation annotation)
{
    int nvalues = 0;
    const CGbool *values = cgGetBoolAnnotationValues(annotation, &nvalues);
    return nvalues > 0 ? values[0] == CG_TRUE : false;
}

int Util::toInt(const CGannotation annotation)
{
    int nvalues = 0;
    const int *values = cgGetIntAnnotationValues(annotation, &nvalues);
    return nvalues > 0 ? values[0] : 0;
}

float Util::toFloat(const CGannotation annotation)
{
    int nvalues = 0;
    const float *values = cgGetFloatAnnotationValues(annotation, &nvalues);
    return nvalues > 0 ? values[0] : 0;
}

bool Util::isPassEquals(const CGannotation annotation, const char *target)
{
    if (!cgIsAnnotation(annotation))
        return true;
    const char *s = cgGetStringAnnotationValue(annotation);
    return s ? strcmp(s, target) == 0 : false;
}

bool Util::isIntegerParameter(const CGparameter parameter)
{
    return cgGetParameterType(parameter) == CG_BOOL ||
            cgGetParameterType(parameter) == CG_INT ||
            cgGetParameterType(parameter) == CG_FLOAT;
}

const std::string Util::trim(const std::string &value)
{
    std::string::const_iterator stringFrom = value.begin(), stringTo = value.end() - 1;
    while (isspace(*stringFrom) && (stringFrom != value.end()))
        ++stringFrom;
    while (isspace(*stringTo) && (stringTo != value.begin()))
        --stringTo;
    return (stringTo - stringFrom >= 0) ? std::string(stringFrom, ++stringTo) : "";
}

/* BasicParameter */

BaseParameter::BaseParameter()
    : m_baseParameter(0)
{
}

BaseParameter::~BaseParameter()
{
    m_baseParameter = 0;
}

void BaseParameter::addParameter(CGparameter parameter)
{
    connectParameter(parameter, m_baseParameter);
}

void BaseParameter::connectParameter(const CGparameter sourceParameter, CGparameter &destinationParameter)
{
    /* prevent infinite reference loop */
    if (sourceParameter != destinationParameter) {
        if (destinationParameter) {
            cgConnectParameter(sourceParameter, destinationParameter);
        }
        else {
            destinationParameter = sourceParameter;
        }
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
    if (cgIsParameter(m_baseParameter))
        cgSetParameter1i(m_baseParameter, value ? 1 : 0);
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
    if (cgIsParameter(m_baseParameter))
        cgSetParameter1i(m_baseParameter, value);
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
    if (cgIsParameter(m_baseParameter))
        cgSetParameter2fv(m_baseParameter, value);
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
    if (cgIsParameter(m_baseParameter))
        cgSetParameter4fv(m_baseParameter, value);
}

/* MatrixSemantic */

MatrixSemantic::MatrixSemantic(const IRenderContext *renderContextRef, int flags)
    : BaseParameter(),
      m_renderContextRef(renderContextRef),
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
    m_renderContextRef = 0;
    m_camera = 0;
    m_cameraInversed = 0;
    m_cameraTransposed = 0;
    m_cameraInverseTransposed = 0;
    m_light = 0;
    m_lightInversed = 0;
    m_lightTransposed = 0;
    m_lightInverseTransposed = 0;
}

void MatrixSemantic::addParameter(CGparameter parameter, const char *suffix)
{
    CGannotation annotation = cgGetNamedParameterAnnotation(parameter, "Object");
    if (!cgIsAnnotation(annotation)) {
        setParameter(parameter, suffix, m_cameraInversed, m_cameraTransposed, m_cameraInverseTransposed, m_camera);
    }
    else {
        const char *name = cgGetStringAnnotationValue(annotation);
        const size_t len = strlen(name);
        if (VPVL2_CG_STREQ_CONST(name, len, "Camera")) {
            setParameter(parameter, suffix, m_cameraInversed, m_cameraTransposed, m_cameraInverseTransposed, m_camera);
        }
        else if (VPVL2_CG_STREQ_CONST(name, len, "Light")) {
            setParameter(parameter, suffix, m_lightInversed, m_lightTransposed, m_lightInverseTransposed, m_light);
        }
    }
}

void MatrixSemantic::setMatrices(const IModel *model, int extraCameraFlags, int extraLightFlags)
{
    setMatrix(model, m_camera,
              extraCameraFlags | IRenderContext::kCameraMatrix);
    setMatrix(model, m_cameraInversed,
              extraCameraFlags | IRenderContext::kCameraMatrix | IRenderContext::kInverseMatrix);
    setMatrix(model, m_cameraTransposed,
              extraCameraFlags | IRenderContext::kCameraMatrix | IRenderContext::kTransposeMatrix);
    setMatrix(model, m_cameraInverseTransposed,
              extraCameraFlags | IRenderContext::kCameraMatrix | IRenderContext::kInverseMatrix | IRenderContext::kTransposeMatrix);
    setMatrix(model, m_light,
              extraLightFlags | IRenderContext::kLightMatrix);
    setMatrix(model, m_lightInversed,
              extraLightFlags | IRenderContext::kLightMatrix | IRenderContext::kInverseMatrix);
    setMatrix(model, m_lightTransposed,
              extraLightFlags | IRenderContext::kLightMatrix | IRenderContext::kTransposeMatrix);
    setMatrix(model, m_lightInverseTransposed,
              extraLightFlags | IRenderContext::kLightMatrix | IRenderContext::kInverseMatrix | IRenderContext::kTransposeMatrix);
}

void MatrixSemantic::setParameter(const CGparameter sourceParameter,
                                  const char *suffix,
                                  CGparameter &inverse,
                                  CGparameter &transposed,
                                  CGparameter &inversetransposed,
                                  CGparameter &baseParameter)
{
    static const char kInverseTranspose[] = "INVERSETRANSPOSE";
    static const char kTranspose[] = "TRANSPOSE";
    static const char kInverse[] = "INVERSE";
    const size_t len = strlen(suffix);
    if (VPVL2_CG_STREQ_CONST(suffix, len, kInverseTranspose)) {
        BaseParameter::connectParameter(sourceParameter, inversetransposed);
    }
    else if (VPVL2_CG_STREQ_CONST(suffix, len, kTranspose)) {
        BaseParameter::connectParameter(sourceParameter, transposed);
    }
    else if (VPVL2_CG_STREQ_CONST(suffix, len, kInverse)) {
        BaseParameter::connectParameter(sourceParameter, inverse);
    }
    else {
        BaseParameter::connectParameter(sourceParameter, baseParameter);
    }
}

void MatrixSemantic::setMatrix(const IModel *model, CGparameter parameter, int flags)
{
    if (cgIsParameter(parameter)) {
        float matrix[16];
        m_renderContextRef->getMatrix(matrix, model, m_flags | flags);
        cgSetMatrixParameterfr(parameter, matrix);
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
    m_geometry = 0;
    m_light = 0;
}

void MaterialSemantic::addParameter(CGparameter parameter)
{
    const char *name = cgGetParameterSemantic(parameter);
    const size_t nlen = strlen(name);
    if (VPVL2_CG_STREQ_CONST(name, nlen, "SPECULARPOWER")
            || VPVL2_CG_STREQ_CONST(name, nlen, "EMISSIVE")
            || VPVL2_CG_STREQ_CONST(name, nlen, "TOONCOLOR")) {
        BaseParameter::connectParameter(parameter, m_geometry);
    }
    else {
        CGannotation annotation = cgGetNamedParameterAnnotation(parameter, "Object");
        if (cgIsAnnotation(annotation)) {
            const char *aname = cgGetStringAnnotationValue(annotation);
            const size_t alen = strlen(aname);
            if (VPVL2_CG_STREQ_CONST(aname, alen,  "Geometry")) {
                BaseParameter::connectParameter(parameter, m_geometry);
            }
            else if (VPVL2_CG_STREQ_CONST(aname, alen, "Light")) {
                BaseParameter::connectParameter(parameter, m_light);
            }
        }
    }
}

void MaterialSemantic::setGeometryColor(const Vector3 &value)
{
    if (cgIsParameter(m_geometry))
        cgSetParameter4fv(m_geometry, value);
}

void MaterialSemantic::setGeometryValue(const Scalar &value)
{
    if (cgIsParameter(m_geometry))
        cgSetParameter1f(m_geometry, value);
}

void MaterialSemantic::setLightColor(const Vector3 &value)
{
    if (cgIsParameter(m_light))
        cgSetParameter4fv(m_light, value);
}

void MaterialSemantic::setLightValue(const Scalar &value)
{
    if (cgIsParameter(m_light))
        cgSetParameter1f(m_light, value);
}

/* MaterialTextureSemantic */

MaterialTextureSemantic::MaterialTextureSemantic()
    : BaseParameter()
{
}

MaterialTextureSemantic::~MaterialTextureSemantic()
{
}

void MaterialTextureSemantic::setTexture(GLuint value)
{
    if (cgIsParameter(m_baseParameter)) {
        if (value) {
            cgGLSetupSampler(m_baseParameter, value);
        }
        else {
            cgGLSetTextureParameter(m_baseParameter, 0);
        }
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
    m_camera = 0;
    m_light = 0;
}

void GeometrySemantic::addParameter(CGparameter parameter)
{
    CGannotation annotation = cgGetNamedParameterAnnotation(parameter, "Object");
    if (cgIsAnnotation(annotation)) {
        const char *name = cgGetStringAnnotationValue(annotation);
        const size_t len = strlen(name);
        if (VPVL2_CG_STREQ_CONST(name, len, "Camera")) {
            BaseParameter::connectParameter(parameter, m_camera);
        }
        else if (VPVL2_CG_STREQ_CONST(name, len, "Light")) {
            BaseParameter::connectParameter(parameter, m_light);
        }
    }
}

void GeometrySemantic::setCameraValue(const Vector3 &value)
{
    if (cgIsParameter(m_camera))
        cgSetParameter4fv(m_camera, value);
}

void GeometrySemantic::setLightValue(const Vector3 &value)
{
    if (cgIsParameter(m_light))
        cgSetParameter4fv(m_light, value);
}

/* TimeSemantic */

TimeSemantic::TimeSemantic(const IRenderContext *renderContextRef)
    : BaseParameter(),
      m_renderContextRef(renderContextRef),
      m_syncEnabled(0),
      m_syncDisabled(0)
{
}

TimeSemantic::~TimeSemantic()
{
    m_renderContextRef = 0;
    m_syncEnabled = 0;
    m_syncDisabled = 0;
}

void TimeSemantic::addParameter(CGparameter parameter)
{
    CGannotation annotation = cgGetNamedParameterAnnotation(parameter, "SyncInEditMode");
    if (cgIsAnnotation(annotation)) {
        if (Util::toBool(annotation)) {
            BaseParameter::connectParameter(parameter, m_syncEnabled);
            return;
        }
    }
    BaseParameter::connectParameter(parameter, m_syncDisabled);
}

void TimeSemantic::update()
{
    float value = 0;
    if (cgIsParameter(m_syncEnabled)) {
        m_renderContextRef->getTime(value, true);
        cgSetParameter1f(m_syncEnabled, value);
    }
    if (cgIsParameter(m_syncDisabled)) {
        m_renderContextRef->getTime(value, false);
        cgSetParameter1f(m_syncDisabled, value);
    }
}

/* ControlObjectSemantic */

ControlObjectSemantic::ControlObjectSemantic(const IEffect *effect,
                                             const Scene *scene,
                                             const IRenderContext *renderContextRef)
    : BaseParameter(),
      m_sceneRef(scene),
      m_renderContextRef(renderContextRef),
      m_effectRef(effect)
{
}

ControlObjectSemantic::~ControlObjectSemantic()
{
    m_sceneRef = 0;
    m_renderContextRef = 0;
}

void ControlObjectSemantic::addParameter(CGparameter parameter)
{
    if (cgIsAnnotation(cgGetNamedParameterAnnotation(parameter, "name")))
        m_parameters.add(parameter);
}

void ControlObjectSemantic::update(const IModel *self)
{
    const int nparameters = m_parameters.count();
    for (int i = 0; i < nparameters; i++) {
        CGparameter parameter = m_parameters[i];
        CGannotation nameAnnotation = cgGetNamedParameterAnnotation(parameter, "name");
        const char *name = cgGetStringAnnotationValue(nameAnnotation);
        const size_t len = strlen(name);
        if (VPVL2_CG_STREQ_CONST(name, len, "(self)")) {
            setParameter(self, parameter);
        }
        else if (VPVL2_CG_STREQ_CONST(name, len, "(OffscreenOwner)")) {
            IEffect *parent = m_effectRef->parentEffect();
            if (parent) {
                const IModel *model = m_renderContextRef->effectOwner(parent);
                setParameter(model, parameter);
            }
        }
        else {
            IString *s = m_renderContextRef->toUnicode(reinterpret_cast<const uint8_t *>(name));
            const IModel *model = m_renderContextRef->findModel(s);
            delete s;
            setParameter(model, parameter);
        }
    }
}

void ControlObjectSemantic::setParameter(const IModel *model, const CGparameter parameter)
{
    float matrix4x4[16];
    const CGtype parameterType = cgGetParameterType(parameter);
    if (model) {
        const CGannotation itemAnnotation = cgGetNamedParameterAnnotation(parameter, "item");
        if (cgIsAnnotation(itemAnnotation)) {
            const char *item = cgGetStringAnnotationValue(itemAnnotation);
            const size_t len = strlen(item);
            const IModel::Type type = model->type();
            if (type == IModel::kPMD || type == IModel::kPMX) {
                const IString *s = m_renderContextRef->toUnicode(reinterpret_cast<const uint8_t *>(item));
                IBone *bone = model->findBone(s);
                IMorph *morph = model->findMorph(s);
                delete s;
                if (bone) {
                    switch (parameterType) {
                    case CG_FLOAT3:
                    case CG_FLOAT4:
                        cgSetParameter4fv(parameter, bone->worldTransform().getOrigin());
                        break;
                    case CG_FLOAT4x4:
                        bone->worldTransform().getOpenGLMatrix(matrix4x4);
                        cgSetMatrixParameterfr(parameter, matrix4x4);
                        break;
                    default:
                        break;
                    }
                }
                else if (morph && parameterType == CG_FLOAT) {
                    cgSetParameter1f(parameter, morph->weight());
                }
            }
            else {
                const Vector3 &position = model->worldPosition();
                const Quaternion &rotation = model->worldRotation();
                const CGtype parameterType = cgGetParameterType(parameter);
                if (VPVL2_CG_STREQ_CONST(item, len, "X") && parameterType == CG_FLOAT) {
                    cgSetParameter1f(parameter, position.x());
                }
                else if (VPVL2_CG_STREQ_CONST(item, len, "Y") && parameterType == CG_FLOAT) {
                    cgSetParameter1f(parameter, position.y());
                }
                else if (VPVL2_CG_STREQ_CONST(item, len, "Z") && parameterType == CG_FLOAT) {
                    cgSetParameter1f(parameter, position.z());
                }
                else if (VPVL2_CG_STREQ_CONST(item, len, "XYZ") && parameterType == CG_FLOAT3) {
                    cgSetParameter3fv(parameter, position);
                }
                else if (VPVL2_CG_STREQ_CONST(item, len, "Rx") && parameterType == CG_FLOAT) {
                    cgSetParameter1f(parameter, btDegrees(rotation.x()));
                }
                else if (VPVL2_CG_STREQ_CONST(item, len, "Ry") && parameterType == CG_FLOAT) {
                    cgSetParameter1f(parameter, btDegrees(rotation.y()));
                }
                else if (VPVL2_CG_STREQ_CONST(item, len, "Rz") && parameterType == CG_FLOAT) {
                    cgSetParameter1f(parameter, btDegrees(rotation.z()));
                }
                else if (VPVL2_CG_STREQ_CONST(item, len, "Rxyz") && parameterType == CG_FLOAT3) {
                    const Vector3 rotationDegree(btDegrees(rotation.x()), btDegrees(rotation.y()), btDegrees(rotation.z()));
                    cgSetParameter3fv(parameter, rotationDegree);
                }
                else if (VPVL2_CG_STREQ_CONST(item, len, "Si") && parameterType == CG_FLOAT) {
                    cgSetParameter1f(parameter, model->scaleFactor());
                }
                else if (VPVL2_CG_STREQ_CONST(item, len, "Tr") && parameterType == CG_FLOAT) {
                    cgSetParameter1f(parameter, model->opacity());
                }
            }
        }
        else {
            switch (parameterType) {
            case CG_BOOL:
                cgSetParameter1i(parameter, model->isVisible());
                break;
            case CG_FLOAT:
                cgSetParameter1f(parameter, model->scaleFactor());
                break;
            case CG_FLOAT3:
            case CG_FLOAT4:
                cgSetParameter4fv(parameter, model->worldPosition());
                break;
            case CG_FLOAT4x4:
                m_renderContextRef->getMatrix(matrix4x4, model, IRenderContext::kWorldMatrix | IRenderContext::kCameraMatrix);
                cgSetMatrixParameterfr(parameter, matrix4x4);
                break;
            default:
                break;
            }
        }
    }
    else {
        const CGtype type = cgGetParameterType(parameter);
        switch (type) {
        case CG_BOOL:
            cgSetParameter1i(parameter, 0);
            break;
        case CG_FLOAT:
            cgSetParameter1f(parameter, 0);
            break;
        case CG_FLOAT3:
        case CG_FLOAT4:
            cgSetParameter4f(parameter, 0, 0, 0, 1);
            break;
        case CG_FLOAT4x4:
            Transform::getIdentity().getOpenGLMatrix(matrix4x4);
            cgSetMatrixParameterfr(parameter, matrix4x4);
            break;
        default:
            break;
        }
    }
}

/* RenderColorTargetSemantic */

RenderColorTargetSemantic::RenderColorTargetSemantic(IRenderContext *renderContextRef)
    : BaseParameter(),
      m_renderContextRef(renderContextRef)
{
#ifdef VPVL2_LINK_QT
    initializeGLFunctions();
#endif
}

RenderColorTargetSemantic::~RenderColorTargetSemantic()
{
    const int ntextures = m_textures.count();
    if (ntextures > 0)
        glDeleteTextures(ntextures, &m_textures[0]);
    m_renderContextRef = 0;
}

void RenderColorTargetSemantic::addParameter(CGparameter parameter,
                                             CGparameter sampler,
                                             const IString *dir,
                                             bool enableResourceName,
                                             bool enableAllTextureTypes)
{
    const CGannotation resourceType = cgGetNamedParameterAnnotation(parameter, "ResourceType");
    int flags;
    if (enableAllTextureTypes && cgIsAnnotation(resourceType)) {
        const char *typeName = cgGetStringAnnotationValue(resourceType);
        const size_t len = strlen(typeName);
        const CGtype samplerType = cgGetParameterType(sampler);
        if (VPVL2_CG_STREQ_CONST(typeName, len, "CUBE") && samplerType == CG_SAMPLERCUBE) {
            flags = IRenderContext::kTextureCube;
        }
        else if (VPVL2_CG_STREQ_CONST(typeName, len, "3D") && samplerType == CG_SAMPLER3D) {
            flags = IRenderContext::kTexture3D;
        }
        else if (VPVL2_CG_STREQ_CONST(typeName, len, "2D") && samplerType == CG_SAMPLER2D) {
            flags = IRenderContext::kTexture2D;
        }
        else {
            return;
        }
    }
    else {
        flags = IRenderContext::kTexture2D;
    }
    const CGannotation resourceName = cgGetNamedParameterAnnotation(parameter, "ResourceName");
    GLuint textureID = 0;
    if (enableResourceName && cgIsAnnotation(resourceName)) {
        const char *name = cgGetStringAnnotationValue(resourceName);
        IString *s = m_renderContextRef->toUnicode(reinterpret_cast<const uint8_t*>(name));
        if (isMimapEnabled(parameter))
            flags |= IRenderContext::kGenerateTextureMipmap;
        IRenderContext::Texture texture;
        texture.async = false;
        texture.object = &textureID;
        if (m_renderContextRef->uploadTexture(s, dir, flags, texture, 0)) {
            textureID = *static_cast<const GLuint *>(texture.object);
            cgGLSetupSampler(sampler, textureID);
            Texture t(texture.width, texture.height, 0, parameter, sampler, textureID);
            m_name2textures.insert(cgGetParameterName(parameter), t);
            m_path2parameters.insert(name, parameter);
            m_textures.add(textureID);
        }
        delete s;
    }
    else {
        switch (flags) {
        case IRenderContext::kTextureCube:
            break;
        case IRenderContext::kTexture3D:
            textureID = generateTexture3D0(parameter, sampler);
            break;
        case IRenderContext::kTexture2D:
            textureID = generateTexture2D0(parameter, sampler);
            break;
        case IRenderContext::kToonTexture:
        case IRenderContext::kGenerateTextureMipmap:
        case IRenderContext::kMaxTextureTypeFlags:
        default:
            break;
        }
    }
    if (cgIsParameter(sampler) && textureID > 0) {
        m_parameters.add(parameter);
        cgGLSetupSampler(sampler, textureID);
    }
}

const RenderColorTargetSemantic::Texture *RenderColorTargetSemantic::findTexture(const char *name) const
{
    return m_name2textures.find(name);
}

CGparameter RenderColorTargetSemantic::findParameter(const char *name) const
{
    const CGparameter *ref = m_path2parameters.find(name);
    return ref ? *ref : 0;
}

bool RenderColorTargetSemantic::isMimapEnabled(const CGparameter parameter) const
{
    const CGannotation mipmapAnnotation = cgGetNamedParameterAnnotation(parameter, "Miplevels");
    bool enableGeneratingMipmap = false;
    if (cgIsAnnotation(mipmapAnnotation)) {
        enableGeneratingMipmap = Util::toInt(mipmapAnnotation) != 1;
    }
    const CGannotation levelAnnotation = cgGetNamedParameterAnnotation(parameter, "Level");
    if (cgIsAnnotation(levelAnnotation)) {
        enableGeneratingMipmap = Util::toInt(levelAnnotation) != 1;
    }
    return enableGeneratingMipmap;
}

void RenderColorTargetSemantic::getTextureFormat(const CGparameter parameter,
                                                 GLenum &internal,
                                                 GLenum &format,
                                                 GLenum &type) const
{
    CGannotation formatAnnotation = cgGetNamedParameterAnnotation(parameter, "Format");
    internal = GL_RGBA8;
    format = GL_RGBA;
    type = GL_UNSIGNED_BYTE;
    const char *formatString = cgGetStringAnnotationValue(formatAnnotation);
    if (!formatString)
        return;
    static const char kPrefix[] = "D3DFMT_";
    const char *ptr = VPVL2_CG_STREQ_SUFFIX(formatString, VPVL2_CG_GET_LENGTH_CONST(kPrefix), kPrefix)
            ? VPVL2_CG_GET_SUFFIX(formatString, kPrefix) : formatString;
    const size_t len = strlen(ptr);
    if (VPVL2_CG_STREQ_CONST(ptr, len, "A32B32G32R32F")) {
        internal = GL_RGBA32F_ARB;
        type = GL_FLOAT;
    }
    else if (VPVL2_CG_STREQ_CONST(ptr, len, "A16B16G16R16F")) {
        internal = GL_RGBA16F_ARB;
        type = GL_HALF_FLOAT;
    }
    else if (VPVL2_CG_STREQ_CONST(ptr, len, "R32F")) {
        internal = GL_R32F;
        format = GL_RED;
        type = GL_FLOAT;
    }
    else if (VPVL2_CG_STREQ_CONST(ptr, len, "R16F")) {
        internal = GL_R16F;
        format = GL_RG;
        type = GL_HALF_FLOAT;
    }
    else if (VPVL2_CG_STREQ_CONST(ptr, len, "G32R32F")) {
        internal = GL_RG32F;
        format = GL_RG;
        type = GL_FLOAT;
    }
    else if (VPVL2_CG_STREQ_CONST(ptr, len, "G16R16F")) {
        internal = GL_RG16F;
        format = GL_RG;
        type = GL_HALF_FLOAT;
    }
    else if (VPVL2_CG_STREQ_CONST(formatString, len, "G16R16")) {
        internal = GL_RG16;
        format = GL_RG;
        type = GL_UNSIGNED_SHORT;
    }
}

void RenderColorTargetSemantic::generateTexture2D(const CGparameter parameter,
                                                  const CGparameter sampler,
                                                  GLuint texture,
                                                  size_t width,
                                                  size_t height)
{
    GLenum internal, format, type;
    getTextureFormat(parameter, internal, format, type);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, internal, width, height, 0, format, type, 0);
    if (isMimapEnabled(parameter))
        glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    Texture t(width, height, 0, parameter, sampler, texture);
    m_name2textures.insert(cgGetParameterName(parameter), t);
}

void RenderColorTargetSemantic::generateTexture3D(const CGparameter parameter,
                                                  const CGparameter sampler,
                                                  GLuint texture,
                                                  size_t width,
                                                  size_t height,
                                                  size_t depth)
{
    GLenum internal, format, type;
    getTextureFormat(parameter, internal, format, type);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage3D(GL_TEXTURE_2D, 0, internal, width, height, depth, 0, format, type, 0);
    if (isMimapEnabled(parameter))
        glGenerateMipmap(GL_TEXTURE_3D);
    glBindTexture(GL_TEXTURE_3D, 0);
    Texture t(width, height, depth, parameter, sampler, texture);
    m_name2textures.insert(cgGetParameterName(parameter), t);
}

GLuint RenderColorTargetSemantic::generateTexture2D0(const CGparameter parameter, const CGparameter sampler)
{
    size_t width, height;
    getSize2(parameter, width, height);
    GLuint texture;
    glGenTextures(1, &texture);
    m_textures.add(texture);
    generateTexture2D(parameter, sampler, texture, width, height);
    return texture;
}

GLuint RenderColorTargetSemantic::generateTexture3D0(const CGparameter parameter, const CGparameter sampler)
{
    size_t width, height, depth;
    getSize3(parameter, width, height, depth);
    GLuint texture;
    glGenTextures(1, &texture);
    m_textures.add(texture);
    generateTexture3D(parameter, sampler, texture, width, height, depth);
    return texture;
}

void RenderColorTargetSemantic::getSize2(const CGparameter parameter, size_t &width, size_t &height)
{
    const CGannotation viewportRatioAnnotation = cgGetNamedParameterAnnotation(parameter, "ViewPortRatio");
    int nvalues = 0;
    if (cgIsAnnotation(viewportRatioAnnotation)) {
        const float *values = cgGetFloatAnnotationValues(viewportRatioAnnotation, &nvalues);
        if (nvalues == 2) {
            Vector3 viewport;
            m_renderContextRef->getViewport(viewport);
            float widthRatio = values[0];
            float heightRatio = values[1];
            width = btMax(1, int(viewport.x() * widthRatio));
            height = btMax(1, int(viewport.y() * heightRatio));
            return;
        }
    }
    const CGannotation dimensionsAnnotation = cgGetNamedParameterAnnotation(parameter, "Dimensions");
    if (cgIsAnnotation(dimensionsAnnotation)) {
        const int *values = cgGetIntAnnotationValues(viewportRatioAnnotation, &nvalues);
        if (nvalues == 2) {
            width = btMax(1,values[0]);
            height = btMax(1,values[1]);
            return;
        }
    }
    const CGannotation widthAnnotation = cgGetNamedParameterAnnotation(parameter, "Width");
    const CGannotation heightAnnotation = cgGetNamedParameterAnnotation(parameter, "Height");
    if (cgIsAnnotation(widthAnnotation) && cgIsAnnotation(heightAnnotation)) {
        width = btMax(1, Util::toInt(widthAnnotation));
        height = btMax(1, Util::toInt(heightAnnotation));
        return;
    }
    Vector3 viewport;
    m_renderContextRef->getViewport(viewport);
    width = btMax(size_t(1), size_t(viewport.x()));
    height = btMax(size_t(1), size_t(viewport.y()));
}

void RenderColorTargetSemantic::getSize3(const CGparameter parameter, size_t &width, size_t &height, size_t &depth)
{
    int nvalues = 0;
    const CGannotation dimensionsAnnotation = cgGetNamedParameterAnnotation(parameter, "Dimensions");
    if (cgIsAnnotation(dimensionsAnnotation)) {
        const int *values = cgGetIntAnnotationValues(dimensionsAnnotation, &nvalues);
        if (nvalues == 3) {
            width = btMax(1,values[0]);
            height = btMax(1,values[1]);
            depth = btMax(1,values[2]);
            return;
        }
    }
    const CGannotation widthAnnotation = cgGetNamedParameterAnnotation(parameter, "Width");
    const CGannotation heightAnnotation = cgGetNamedParameterAnnotation(parameter, "Height");
    const CGannotation depthAnnotation = cgGetNamedParameterAnnotation(parameter, "Depth");
    if (cgIsAnnotation(widthAnnotation) && cgIsAnnotation(heightAnnotation) && cgIsAnnotation(depthAnnotation)) {
        width = btMax(size_t(1), size_t(Util::toInt(widthAnnotation)));
        height = btMax(size_t(1), size_t(Util::toInt(heightAnnotation)));
        depth = btMax(size_t(1), size_t(Util::toInt(depthAnnotation)));
        return;
    }
    Vector3 viewport;
    m_renderContextRef->getViewport(viewport);
    width = btMax(size_t(1), size_t(viewport.x()));
    height = btMax(size_t(1), size_t(viewport.y()));
    depth = 24;
}

/* RenderDepthStencilSemantic */

RenderDepthStencilTargetSemantic::RenderDepthStencilTargetSemantic(IRenderContext *renderContextRef)
    : RenderColorTargetSemantic(renderContextRef)
{
}

RenderDepthStencilTargetSemantic::~RenderDepthStencilTargetSemantic()
{
    const int nRenderBuffers = m_renderBuffers.count();
    if (nRenderBuffers > 0)
        glDeleteRenderbuffers(nRenderBuffers, &m_renderBuffers[0]);
}

const RenderDepthStencilTargetSemantic::Buffer *RenderDepthStencilTargetSemantic::findRenderBuffer(const char *name) const
{
    return m_name2buffer.find(name);
}

void RenderDepthStencilTargetSemantic::generateTexture2D(const CGparameter parameter,
                                                         const CGparameter /* sampler */,
                                                         GLuint /* texture */,
                                                         size_t width,
                                                         size_t height)
{
    GLuint renderBuffer;
    glGenRenderbuffers(1, &renderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    m_renderBuffers.add(renderBuffer);
    Buffer buffer(width, height, parameter, renderBuffer);
    m_name2buffer.insert(cgGetParameterName(parameter), buffer);
}

/* OffscreenRenderTargetSemantic */

OffscreenRenderTargetSemantic::OffscreenRenderTargetSemantic(Effect *effect, IRenderContext *renderContextRef)
    : RenderColorTargetSemantic(renderContextRef),
      m_effectRef(effect)
{
}

OffscreenRenderTargetSemantic::~OffscreenRenderTargetSemantic()
{
    m_effectRef = 0;
}

void OffscreenRenderTargetSemantic::addParameter(CGparameter parameter, CGparameter sampler, const IString *dir)
{
    RenderColorTargetSemantic::addParameter(parameter, sampler, dir, false, false);
}

void OffscreenRenderTargetSemantic::generateTexture2D(const CGparameter parameter,
                                                      const CGparameter sampler,
                                                      GLuint texture,
                                                      size_t width,
                                                      size_t height)
{
    RenderColorTargetSemantic::generateTexture2D(parameter, sampler, texture, width, height);
    m_effectRef->addOffscreenRenderTarget(parameter, sampler, width, height);
}

/* AnimatedTextureSemantic */

AnimatedTextureSemantic::AnimatedTextureSemantic(IRenderContext *renderContextRef)
    : m_renderContextRef(renderContextRef)
{
}

AnimatedTextureSemantic::~AnimatedTextureSemantic()
{
    m_renderContextRef = 0;
}

void AnimatedTextureSemantic::addParameter(CGparameter parameter)
{
    CGannotation annotation = cgGetNamedParameterAnnotation(parameter, "ResourceName");
    if (cgIsAnnotation(annotation) && cgGetParameterType(parameter) == CG_TEXTURE) {
        m_parameters.add(parameter);
    }
}

void AnimatedTextureSemantic::update(const RenderColorTargetSemantic &renderColorTarget)
{
    const int nparameters = m_parameters.count();
    for (int i = 0; i < nparameters; i++) {
        CGparameter parameter = m_parameters[i];
        const CGannotation resourceNameAnnotation = cgGetNamedParameterAnnotation(parameter, "ResourceName");
        const CGannotation offsetAnnotation = cgGetNamedParameterAnnotation(parameter, "Offset");
        const CGannotation speedAnnotation = cgGetNamedParameterAnnotation(parameter, "Speed");
        const CGannotation seekVariableAnnotation = cgGetNamedParameterAnnotation(parameter, "SeekVariable");
        const char *resourceName = cgGetStringAnnotationValue(resourceNameAnnotation);
        float offset = Util::toFloat(offsetAnnotation), speed = 1, seek;
        const char *seekVariable = cgGetStringAnnotationValue(seekVariableAnnotation);
        if (cgIsAnnotation(speedAnnotation)) {
            speed = Util::toFloat(speedAnnotation);
        }
        if (seekVariable) {
            CGeffect effect = cgGetParameterEffect(parameter);
            CGparameter seekParameter = cgGetNamedEffectParameter(effect, seekVariable);
            cgGLGetParameter1f(seekParameter, &seek);
        }
        else {
            m_renderContextRef->getTime(seek, true);
        }
        const CGparameter texParam = renderColorTarget.findParameter(resourceName);
        const RenderColorTargetSemantic::Texture *t = renderColorTarget.findTexture(cgGetParameterName(texParam));
        if (t) {
            GLuint textureID = t->id;
            m_renderContextRef->uploadAnimatedTexture(offset, speed, seek, &textureID);
        }
    }
}

/* TextureValueSemantic */

TextureValueSemantic::TextureValueSemantic()
{
}

TextureValueSemantic::~TextureValueSemantic()
{
}

void TextureValueSemantic::addParameter(CGparameter parameter)
{
    const CGannotation annotation = cgGetNamedParameterAnnotation(parameter, "TextureName");
    int ndimensions = cgGetArrayDimension(parameter);
    bool isFloat4 = cgGetParameterType(parameter) == CG_FLOAT4;
    bool isValidDimension = ndimensions == 1 || ndimensions == 2;
    if (cgIsAnnotation(annotation) && isFloat4 && isValidDimension) {
        const char *name = cgGetStringAnnotationValue(annotation);
        const CGeffect effect = cgGetParameterEffect(parameter);
        const CGparameter textureParameter = cgGetNamedEffectParameter(effect, name);
        if (cgGetParameterType(textureParameter) == CG_TEXTURE) {
            m_parameters.add(parameter);
        }
    }
}

void TextureValueSemantic::update()
{
    const int nparameters = m_parameters.count();
    for (int i = 0; i < nparameters; i++) {
        CGparameter parameter = m_parameters[i];
        GLuint texture = cgGLGetTextureParameter(parameter);
        int size = cgGetArrayTotalSize(parameter);
        float *pixels = new float[size * 4];
        glBindTexture(GL_TEXTURE_2D, texture);
        glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
        cgGLSetParameter4fv(parameter, pixels);
        delete[] pixels;
    }
    glBindTexture(GL_TEXTURE_2D, 0);
}

/* Effect::RectRenderEngine */
class EffectEngine::RectRenderEngine : public internal::BaseRenderEngine
        #ifdef VPVL2_LINK_QT
        , protected QGLFunctions
        #endif
{
public:
    RectRenderEngine(const Scene *sceneRef, IRenderContext *renderContext)
        : BaseRenderEngine(sceneRef, renderContext)
    {
#ifdef VPVL2_LINK_QT
        initializeGLFunctions();
#endif
        initializeExtensions();
    }
    ~RectRenderEngine() {
        glDeleteBuffers(1, &m_verticesBuffer);
        glDeleteBuffers(1, &m_indicesBuffer);
    }

    void initializeVertexBundle() {
        glGenBuffers(1, &m_verticesBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, m_verticesBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(kVertices), kVertices, GL_STATIC_DRAW);
        glGenBuffers(1, &m_indicesBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(kIndices), kIndices, GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        allocateVertexArrayObjects(&m_vertexBundle, 1);
        bindVertexArrayObject(m_vertexBundle);
        bindVertexBundle(false);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        unbindVertexArrayObject();
        unbindVertexBundle(false);
    }
    void bindVertexBundle(bool bundle) {
        if (!bundle || !bindVertexArrayObject(m_vertexBundle)) {
            glBindBuffer(GL_ARRAY_BUFFER, m_verticesBuffer);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesBuffer);
            glVertexPointer(2, GL_FLOAT, kVertexStride, reinterpret_cast<const GLvoid *>(0));
            glTexCoordPointer(2, GL_FLOAT, kVertexStride, reinterpret_cast<const GLvoid *>(kTextureOffset));
        }
    }
    void unbindVertexBundle(bool bundle) {
        if (!bundle || !unbindVertexArrayObject()) {
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }
    }

private:
    GLuint m_vertexBundle;
    GLuint m_verticesBuffer;
    GLuint m_indicesBuffer;
};

/* EffectEngine */
EffectEngine::EffectEngine(const Scene *scene,
                           const IString *dir,
                           Effect *effect,
                           IRenderContext *renderContextRef)
    : world(renderContextRef, IRenderContext::kWorldMatrix),
      view(renderContextRef, IRenderContext::kViewMatrix),
      projection(renderContextRef, IRenderContext::kProjectionMatrix),
      worldView(renderContextRef, IRenderContext::kWorldMatrix | IRenderContext::kViewMatrix),
      viewProjection(renderContextRef, IRenderContext::kViewMatrix | IRenderContext::kProjectionMatrix),
      worldViewProjection(renderContextRef, IRenderContext::kWorldMatrix | IRenderContext::kViewMatrix | IRenderContext::kProjectionMatrix),
      time(renderContextRef),
      elapsedTime(renderContextRef),
      controlObject(effect, scene, renderContextRef),
      renderColorTarget(renderContextRef),
      renderDepthStencilTarget(renderContextRef),
      animatedTexture(renderContextRef),
      offscreenRenderTarget(effect, renderContextRef),
      index(0),
      m_effectRef(0),
      m_renderContextRef(renderContextRef),
      m_rectRenderEngine(0),
      m_scriptOutput(kColor),
      m_scriptClass(kObject),
      m_scriptOrder(IEffect::kStandard)
{
    m_rectRenderEngine = new RectRenderEngine(scene, renderContextRef);
#ifdef VPVL2_LINK_QT
    initializeGLFunctions();
#endif
    attachEffect(effect, dir);
}

EffectEngine::~EffectEngine()
{
    delete m_rectRenderEngine;
    m_rectRenderEngine = 0;
    m_effectRef = 0;
    m_renderContextRef = 0;
}

bool EffectEngine::attachEffect(IEffect *effect, const IString *dir)
{
    static const char kWorldSemantic[] = "WORLD";
    static const char kViewSemantic[] = "VIEW";
    static const char kProjectionSemantic[] = "PROJECTION";
    static const char kWorldViewSemantic[] = "WORLDVIEW";
    static const char kViewProjectionSemantic[] = "VIEWPROJECTION";
    static const char kWorldViewProjectionSemantic[] = "WORLDVIEWPROJECTION";
    m_effectRef = static_cast<Effect *>(effect);
    CGeffect value = static_cast<CGeffect>(effect->internalPointer());
    if (!cgIsEffect(value))
        return false;
    CGparameter parameter = cgGetFirstEffectParameter(value);
    bool ownTechniques = false, foundSAS = false;
    while (parameter) {
        const char *semantic = cgGetParameterSemantic(parameter);
        const size_t slen = strlen(semantic);
        if (VPVL2_CG_STREQ_CONST(semantic, slen, "VIEWPORTPIXELSIZE")) {
            viewportPixelSize.addParameter(parameter);
        }
        else if (VPVL2_CG_STREQ_SUFFIX(semantic, slen, kWorldViewProjectionSemantic)) {
            worldViewProjection.addParameter(parameter, VPVL2_CG_GET_SUFFIX(semantic, kWorldViewProjectionSemantic));
        }
        else if (VPVL2_CG_STREQ_SUFFIX(semantic, slen, kWorldViewSemantic)) {
            worldView.addParameter(parameter, VPVL2_CG_GET_SUFFIX(semantic, kWorldViewSemantic));
        }
        else if (VPVL2_CG_STREQ_SUFFIX(semantic, slen, kViewProjectionSemantic)) {
            viewProjection.addParameter(parameter, VPVL2_CG_GET_SUFFIX(semantic, kViewProjectionSemantic));
        }
        else if (VPVL2_CG_STREQ_SUFFIX(semantic, slen, kWorldSemantic)) {
            world.addParameter(parameter, VPVL2_CG_GET_SUFFIX(semantic, kWorldSemantic));
        }
        else if (VPVL2_CG_STREQ_SUFFIX(semantic, slen, kViewSemantic)) {
            view.addParameter(parameter, VPVL2_CG_GET_SUFFIX(semantic, kViewSemantic));
        }
        else if (VPVL2_CG_STREQ_SUFFIX(semantic, slen, kProjectionSemantic)) {
            projection.addParameter(parameter, VPVL2_CG_GET_SUFFIX(semantic, kProjectionSemantic));
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "DIFFUSE")) {
            diffuse.addParameter(parameter);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "AMBIENT")) {
            ambient.addParameter(parameter);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "EMISSIVE")) {
            emissive.addParameter(parameter);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "SPECULARPOWER")) {
            specularPower.addParameter(parameter);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "SPECULAR")) {
            specular.addParameter(parameter);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "TOONCOLOR")) {
            toonColor.addParameter(parameter);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "EDGECOLOR")) {
            edgeColor.addParameter(parameter);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "_POSITION")) {
            position.addParameter(parameter);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "_DIRECTION")) {
            direction.addParameter(parameter);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "TIME")) {
            time.addParameter(parameter);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "ELAPSEDTIME")) {
            elapsedTime.addParameter(parameter);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "MOUSEPOSITION")) {
            mousePosition.addParameter(parameter);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "LEFTMOUSEDOWN")) {
            leftMouseDown.addParameter(parameter);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "MIDDLEMOUSEDOWN")) {
            middleMouseDown.addParameter(parameter);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "RIGHTMOUSEDOWN")) {
            rightMouseDown.addParameter(parameter);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "CONTROLOBJECT")) {
            controlObject.addParameter(parameter);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "ANIMATEDTEXTURE")) {
            animatedTexture.addParameter(parameter);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "TEXTUREVALUE")) {
            textureValue.addParameter(parameter);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "RENDERDEPTHSTENCILTARGET")) {
            renderDepthStencilTarget.addParameter(parameter, 0, dir, false, false);
        }
        else if (!foundSAS && VPVL2_CG_STREQ_CONST(semantic, slen, "STANDARDSGLOBAL")) {
            setStandardsGlobal(parameter, ownTechniques);
            foundSAS = true;
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "_INDEX")) {
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "TEXUNIT0")) {
            depthTexture.addParameter(parameter);
        }
        else {
            const char *name = cgGetParameterName(parameter);
            const size_t nlen = strlen(name);
            if (VPVL2_CG_STREQ_CONST(name, nlen, "parthf")) {
                parthf.addParameter(parameter);
            }
            else if (VPVL2_CG_STREQ_CONST(name, nlen, "spadd")) {
                spadd.addParameter(parameter);
            }
            else if (VPVL2_CG_STREQ_CONST(name, nlen, "transp")) {
                transp.addParameter(parameter);
            }
            else if (VPVL2_CG_STREQ_CONST(name, nlen, "use_texture")) {
                useTexture.addParameter(parameter);
            }
            else if (VPVL2_CG_STREQ_CONST(name, nlen, "use_spheremap")) {
                useSpheremap.addParameter(parameter);
            }
            else if (VPVL2_CG_STREQ_CONST(name, nlen, "use_toon")) {
                useToon.addParameter(parameter);
            }
            else if (VPVL2_CG_STREQ_CONST(name, nlen, "opadd")) {
                opadd.addParameter(parameter);
            }
            else if (VPVL2_CG_STREQ_CONST(name, nlen, "VertexCount")) {
                vertexCount.addParameter(parameter);
            }
            else if (VPVL2_CG_STREQ_CONST(name, nlen, "SubsetCount")) {
                subsetCount.addParameter(parameter);
            }
            else {
                setTextureParameters(parameter, dir);
            }
        }
        if (Effect::isInteractiveParameter(parameter))
            m_effectRef->addInteractiveParameter(parameter);
        parameter = cgGetNextParameter(parameter);
    }
    if (!ownTechniques) {
        CGtechnique technique = cgGetFirstTechnique(value);
        while (technique) {
            addTechniquePasses(technique);
            technique = cgGetNextTechnique(technique);
        }
    }
    m_rectRenderEngine->initializeVertexBundle();
    return true;
}

CGtechnique EffectEngine::findTechnique(const char *pass,
                                        int offset,
                                        int nmaterials,
                                        bool hasTexture,
                                        bool hasSphereMap,
                                        bool useToon) const
{
    CGtechnique technique = 0;
    const int ntechniques = m_techniques.size();
    for (int i = 0; i < ntechniques; i++) {
        CGtechnique t = m_techniques[i];
        if (testTechnique(t, pass, offset, nmaterials, hasTexture, hasSphereMap, useToon)) {
            technique = t;
            break;
        }
    }
    return technique;
}

void EffectEngine::executeScriptExternal()
{
    if (m_scriptOrder == IEffect::kPostProcess)
        executeScript(&m_externalScript, 0, 0, 0, 0);
}

bool EffectEngine::hasTechniques(IEffect::ScriptOrderType order) const
{
    return m_scriptOrder == order ? m_techniqueScripts.size() > 0 : false;
}

void EffectEngine::executeProcess(const IModel *model, IEffect::ScriptOrderType order)
{
    if (!model || !m_effectRef || m_scriptOrder != order)
        return;
    m_rectRenderEngine->bindVertexBundle(true);
    setZeroGeometryParameters(model);
    diffuse.setGeometryColor(Color(0, 0, 0, model->opacity())); /* for asset opacity */
    CGtechnique technique = findTechnique("object", 0, 0, false, false, false);
    executeTechniquePasses(technique, GL_QUADS, kIndicesSize, GL_UNSIGNED_INT, 0);
    m_rectRenderEngine->unbindVertexBundle(true);
}

void EffectEngine::executeTechniquePasses(const CGtechnique technique,
                                          const GLenum mode,
                                          const GLsizei count,
                                          const GLenum type,
                                          const GLvoid *ptr)
{
    if (cgIsTechnique(technique)) {
        const Script *tss = m_techniqueScripts.find(technique);
        executeScript(tss, mode, count, type, ptr);
        const Passes *passes = m_techniquePasses.find(technique);
        if (passes) {
            const int npasses = passes->size();
            for (int i = 0; i < npasses; i++) {
                CGpass pass = passes->at(i);
                const Script *pss = m_passScripts.find(pass);
                executeScript(pss, mode, count, type, ptr);
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

void EffectEngine::setZeroGeometryParameters(const IModel *model)
{
    edgeColor.setGeometryColor(model->edgeColor());
    toonColor.setGeometryColor(kZeroC);
    ambient.setGeometryColor(kZeroC);
    diffuse.setGeometryColor(kZeroC);
    emissive.setGeometryColor(kZeroC);
    specular.setGeometryColor(kZeroC);
    specularPower.setGeometryValue(0);
    materialTexture.setTexture(0);
    materialSphereMap.setTexture(0);
    spadd.setValue(false);
    useTexture.setValue(false);
    useSpheremap.setValue(false);
}

void EffectEngine::updateModelGeometryParameters(const Scene *scene, const IModel *model)
{
    const ILight *light = scene->light();
    const Vector3 &lightColor = light->color();
    if (model->type() == IModel::kAsset) {
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
    const ICamera *camera = scene->camera();
    const Vector3 &cameraPosition = camera->modelViewTransform().getOrigin();
    position.setCameraValue(cameraPosition);
    direction.setCameraValue((cameraPosition - camera->lookAt()).normalized());
    controlObject.update(model);
}

void EffectEngine::updateSceneParameters()
{
    Vector3 viewport;
    m_renderContextRef->getViewport(viewport);
    viewportPixelSize.setValue(viewport);
    Vector4 position;
    m_renderContextRef->getMousePosition(position, IRenderContext::kMouseCursorPosition);
    mousePosition.setValue(position);
    m_renderContextRef->getMousePosition(position, IRenderContext::kMouseLeftPressPosition);
    leftMouseDown.setValue(position);
    m_renderContextRef->getMousePosition(position, IRenderContext::kMouseMiddlePressPosition);
    middleMouseDown.setValue(position);
    m_renderContextRef->getMousePosition(position, IRenderContext::kMouseRightPressPosition);
    rightMouseDown.setValue(position);
    time.update();
    elapsedTime.update();
    textureValue.update();
    animatedTexture.update(renderColorTarget);
}

bool EffectEngine::validateStandard() const
{
    return m_scriptOrder == IEffect::kStandard;
}

const EffectEngine::Script *EffectEngine::findTechniqueScript(const CGtechnique technique) const
{
    return m_techniqueScripts.find(technique);
}

const EffectEngine::Script *EffectEngine::findPassScript(const CGpass pass) const
{
    return m_passScripts.find(pass);
}

bool EffectEngine::testTechnique(const CGtechnique technique,
                                 const char *pass,
                                 int offset,
                                 int nmaterials,
                                 bool hasTexture,
                                 bool hasSphereMap,
                                 bool useToon)
{
    if (!cgIsTechniqueValidated(technique) && cgValidateTechnique(technique) == CG_FALSE)
        return false;
    int ok = 1;
    const CGannotation passAnnotation = cgGetNamedTechniqueAnnotation(technique, "MMDPass");
    ok &= Util::isPassEquals(passAnnotation, pass) ? 1 : 0;
    const CGannotation subsetAnnotation = cgGetNamedTechniqueAnnotation(technique, "Subset");
    ok &= containsSubset(subsetAnnotation, offset, nmaterials) ? 1 : 0;
    const CGannotation useTextureAnnotation = cgGetNamedTechniqueAnnotation(technique, "UseTexture");
    ok &= (!cgIsAnnotation(useTextureAnnotation) || Util::toBool(useTextureAnnotation) == hasTexture) ? 1 : 0;
    const CGannotation useSphereMapAnnotation = cgGetNamedTechniqueAnnotation(technique, "UseSphereMap");
    ok &= (!cgIsAnnotation(useSphereMapAnnotation) || Util::toBool(useSphereMapAnnotation) == hasSphereMap) ? 1 : 0;
    const CGannotation useToonAnnotation = cgGetNamedTechniqueAnnotation(technique, "UseToon");
    ok &= (!cgIsAnnotation(useToonAnnotation) || Util::toBool(useToonAnnotation) == useToon) ? 1 : 0;
    return ok == 1;
}

bool EffectEngine::containsSubset(const CGannotation annotation, int subset, int nmaterials)
{
    if (!cgIsAnnotation(annotation))
        return true;
    const std::string s(cgGetStringAnnotationValue(annotation));
    std::istringstream stream(s);
    std::string segment;
    while (std::getline(stream, segment, ',')) {
        if (strtol(segment.c_str(), 0, 10) == subset)
            return true;
        std::string::size_type offset = segment.find("-");
        if (offset != std::string::npos) {
            int from = strtol(segment.substr(0, offset).c_str(), 0, 10);
            int to = strtol(segment.substr(offset + 1).c_str(), 0, 10);
            if (to == 0)
                to = nmaterials;
            if (from > to)
                std::swap(from, to);
            if (from <= subset && subset <= to)
                return true;
        }
    }
    return false;
}

void EffectEngine::setStateFromRenderColorTargetSemantic(const RenderColorTargetSemantic &semantic,
                                                         const std::string &value,
                                                         ScriptState::Type type,
                                                         ScriptState &state)
{
    bool bound = false;
    state.type = type;
    if (!value.empty()) {
        const RenderColorTargetSemantic::Texture *texture = semantic.findTexture(value.c_str());
        if (texture) {
            state.setFromTexture(texture);
            m_target2textureRefs.insert(type, texture);
            bound = true;
        }
    }
    else {
        const RenderColorTargetSemantic::Texture **texturePtr = m_target2textureRefs.find(type);
        if (texturePtr) {
            const RenderColorTargetSemantic::Texture *texture = *texturePtr;
            state.setFromTexture(texture);
            m_target2textureRefs.remove(type);
        }
    }
    state.isRenderTargetBound = bound;
}

void EffectEngine::setStateFromRenderDepthStencilTargetSemantic(const RenderDepthStencilTargetSemantic &semantic,
                                                                const std::string &value,
                                                                ScriptState::Type type,
                                                                ScriptState &state)
{
    bool bound = false;
    state.type = type;
    if (!value.empty()) {
        const RenderDepthStencilTargetSemantic::Buffer *buffer = semantic.findRenderBuffer(value.c_str());
        if (buffer) {
            state.setFromBuffer(buffer);
            m_target2bufferRefs.insert(type, buffer);
            bound = true;
        }
    }
    else {
        const RenderDepthStencilTargetSemantic::Buffer **bufferPtr = m_target2bufferRefs.find(type);
        if (bufferPtr) {
            const RenderDepthStencilTargetSemantic::Buffer *buffer = *bufferPtr;
            state.setFromBuffer(buffer);
            m_target2bufferRefs.remove(type);
        }
    }
    state.isRenderTargetBound = bound;
}

void EffectEngine::setStateFromParameter(const CGeffect effect,
                                         const std::string &value,
                                         CGtype testType,
                                         ScriptState::Type type,
                                         ScriptState &state)
{
    CGparameter parameter = cgGetNamedEffectParameter(effect, value.c_str());
    if (cgIsParameter(parameter) && cgGetParameterType(parameter) == testType) {
        state.type = type;
        state.parameter = parameter;
    }
}

void EffectEngine::executePass(CGpass pass,
                               const GLenum mode,
                               const GLsizei count,
                               const GLenum type,
                               const GLvoid *ptr) const
{
    if (cgIsPass(pass)) {
        cgSetPassState(pass);
        drawPrimitives(mode, count, type, ptr);
        cgResetPassState(pass);
    }
}

void EffectEngine::setRenderColorTargetFromState(const ScriptState &state)
{
    GLuint texture = state.texture;
    const size_t width = state.width, height = state.height;
    const int index = state.type - ScriptState::kRenderColorTarget0;
    const int target = kBaseRenderColorTargetIndex + index;
    if (state.isRenderTargetBound) {
        if (m_renderColorTargets.findLinearSearch(target) == m_renderColorTargets.size()) {
            m_renderColorTargets.push_back(target);
            m_renderContextRef->setRenderColorTargets(&m_renderColorTargets[0], m_renderColorTargets.size());
        }
        m_renderContextRef->bindRenderColorTarget(&texture, width, height, index, kEnableRTAA);
        glViewport(0, 0, width, height);
    }
    else {
        Vector3 viewport;
        m_renderColorTargets.remove(target);
        const int nRenderColorTargets = m_renderColorTargets.size();
        if (nRenderColorTargets > 0 && target != kBaseRenderColorTargetIndex)
            m_renderContextRef->setRenderColorTargets(&m_renderColorTargets[0], nRenderColorTargets);
        m_renderContextRef->releaseRenderColorTarget(&texture, width, height, index, kEnableRTAA);
        m_renderContextRef->getViewport(viewport);
        glViewport(0, 0, GLsizei(viewport.x()), GLsizei(viewport.y()));
    }
}

void EffectEngine::setRenderDepthStencilTargetFromState(const ScriptState &state)
{
    GLuint texture = state.texture;
    GLuint depthBuffer = state.depthBuffer;
    GLuint stencilBuffer = state.stencilBuffer;
    if (state.isRenderTargetBound) {
        m_renderContextRef->bindRenderDepthStencilTarget(&texture,
                                                    &depthBuffer,
                                                    &stencilBuffer,
                                                    state.width,
                                                    state.height,
                                                    kEnableRTAA);
    }
    else {
        m_renderContextRef->releaseRenderDepthStencilTarget(&texture,
                                                       &depthBuffer,
                                                       &stencilBuffer,
                                                       state.width,
                                                       state.height,
                                                       kEnableRTAA);
    }
}

void EffectEngine::executeScript(const Script *script,
                                 const GLenum mode,
                                 const GLsizei count,
                                 const GLenum type,
                                 const GLvoid *ptr)
{
    if (script) {
        const int nstates = script->size();
        int stateIndex = 0, nloop = 0, currentIndex = 0, backStateIndex = 0;
        Vector4 v4;
        while (stateIndex < nstates) {
            const ScriptState &state = script->at(stateIndex);
            switch (state.type) {
            case ScriptState::kClearColor:
                glClear(GL_COLOR_BUFFER_BIT);
                break;
            case ScriptState::kClearDepth:
                glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                break;
            case ScriptState::kClearSetColor:
                cgGLGetParameter4f(state.parameter, v4);
                glClearColor(v4.x(), v4.y(), v4.z(), v4.w());
                break;
            case ScriptState::kClearSetDepth:
                cgGLGetParameter1f(state.parameter, v4);
                glClearDepth(v4.x());
                break;
            case ScriptState::kLoopByCount:
                cgGLGetParameter1f(state.parameter, v4);
                backStateIndex = stateIndex + 1;
                currentIndex = 0;
                nloop = int(v4.x());
                break;
            case ScriptState::kLoopEnd:
                if (--nloop >= 0) {
                    stateIndex = backStateIndex;
                    ++currentIndex;
                    continue;
                }
                break;
            case ScriptState::kLoopGetIndex:
                cgGLSetParameter1f(state.parameter, float(currentIndex));
                break;
            case ScriptState::kRenderColorTarget0:
            case ScriptState::kRenderColorTarget1:
            case ScriptState::kRenderColorTarget2:
            case ScriptState::kRenderColorTarget3:
                setRenderColorTargetFromState(state);
                break;
            case ScriptState::kRenderDepthStencilTarget:
                setRenderDepthStencilTargetFromState(state);
                break;
            case ScriptState::kDrawBuffer:
                if (m_scriptClass != kObject) {
                    executePass(state.pass, mode, count, type, ptr);
                }
                break;
            case ScriptState::kDrawGeometry:
                if (m_scriptClass != kScene) {
                    executePass(state.pass, mode, count, type, ptr);
                }
                break;
            case ScriptState::kPass:
                executeScript(m_passScripts.find(state.pass), mode, count, type, ptr);
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

void EffectEngine::addTechniquePasses(const CGtechnique technique)
{
    Passes passes;
    if (parseTechniqueScript(technique, passes)) {
        m_techniquePasses.insert(technique, passes);
        m_techniques.push_back(technique);
    }
}

void EffectEngine::setStandardsGlobal(const CGparameter parameter, bool &ownTechniques)
{
    float version;
    cgGLGetParameter1f(parameter, &version);
    if (!btFuzzyZero(version - 0.8f))
        return;
    const CGannotation scriptClassAnnotation = cgGetNamedParameterAnnotation(parameter, "ScriptClass");
    if (cgIsAnnotation(scriptClassAnnotation)) {
        const char *value = cgGetStringAnnotationValue(scriptClassAnnotation);
        const size_t len = strlen(value);
        if (VPVL2_CG_STREQ_CONST(value, len, "object")) {
            m_scriptClass = kObject;
        }
        else if (VPVL2_CG_STREQ_CONST(value, len, "scene")) {
            m_scriptClass = kScene;
        }
        else if (VPVL2_CG_STREQ_CONST(value, len, "sceneorobject")) {
            m_scriptClass = kSceneOrObject;
        }
    }
    const CGannotation scriptOrderAnnotation = cgGetNamedParameterAnnotation(parameter, "ScriptOrder");
    if (cgIsAnnotation(scriptOrderAnnotation)) {
        const char *value = cgGetStringAnnotationValue(scriptOrderAnnotation);
        const size_t len = strlen(value);
        if (VPVL2_CG_STREQ_CONST(value, len, "standard")) {
            m_scriptOrder = IEffect::kStandard;
        }
        else if (VPVL2_CG_STREQ_CONST(value, len, "preprocess")) {
            m_scriptOrder = IEffect::kPreProcess;
        }
        else if (VPVL2_CG_STREQ_CONST(value, len, "postprocess")) {
            m_scriptOrder = IEffect::kPostProcess;
        }
    }
    const CGannotation scriptAnnotation = cgGetNamedParameterAnnotation(parameter, "Script");
    if (cgIsAnnotation(scriptAnnotation)) {
        const char *value = cgGetStringAnnotationValue(scriptAnnotation);
        const size_t len = strlen(value);
        static const char kMultipleTechniques[] = "Technique=Technique?";
        static const char kSingleTechnique[] = "Technique=";
        m_techniques.clear();
        CGeffect effect = static_cast<CGeffect>(m_effectRef->internalPointer());
        if (VPVL2_CG_STREQ_SUFFIX(value, len, kMultipleTechniques)) {
            const std::string s(VPVL2_CG_GET_SUFFIX(value, kMultipleTechniques));
            std::istringstream stream(s);
            std::string segment;
            while (std::getline(stream, segment, ':')) {
                CGtechnique technique = cgGetNamedTechnique(effect, segment.c_str());
                addTechniquePasses(technique);
            }
            ownTechniques = true;
        }
        else if (VPVL2_CG_STREQ_SUFFIX(value, len, kSingleTechnique)) {
            CGtechnique technique = cgGetNamedTechnique(effect, VPVL2_CG_GET_SUFFIX(value, kSingleTechnique));
            addTechniquePasses(technique);
            ownTechniques = true;
        }
    }
}

void EffectEngine::setTextureParameters(CGparameter parameter, const IString *dir)
{
    const CGtype type = cgGetParameterType(parameter);
    if (type == CG_SAMPLER2D || type == CG_SAMPLER3D || type == CG_SAMPLERCUBE) {
        CGstateassignment sa = cgGetFirstSamplerStateAssignment(parameter);
        while (sa) {
            const CGstate s = cgGetSamplerStateAssignmentState(sa);
            if (cgIsState(s) && cgGetStateType(s) == CG_TEXTURE) {
                CGparameter textureParameter = cgGetTextureStateAssignmentValue(sa);
                const char *semantic = cgGetParameterSemantic(textureParameter);
                const size_t len = strlen(semantic);
                if (VPVL2_CG_STREQ_CONST(semantic, len, "MATERIALTEXTURE")) {
                    materialTexture.addParameter(parameter);
                }
                else if (VPVL2_CG_STREQ_CONST(semantic, len, "MATERIALSPHEREMAP")) {
                    materialSphereMap.addParameter(parameter);
                }
                else if (VPVL2_CG_STREQ_CONST(semantic, len, "RENDERCOLORTARGET")) {
                    renderColorTarget.addParameter(textureParameter, parameter, dir, false, false);
                }
                else if (VPVL2_CG_STREQ_CONST(semantic, len, "OFFSCREENRENDERTARGET")) {
                    offscreenRenderTarget.addParameter(textureParameter, parameter, dir);
                }
                else {
                    renderColorTarget.addParameter(textureParameter, parameter, dir, true, true);
                }
                break;
            }
            sa = cgGetNextStateAssignment(sa);
        }
    }
}

bool EffectEngine::parsePassScript(const CGpass pass)
{
    if (m_passScripts[pass])
        return true;
    if (!cgIsPass(pass))
        return false;
    const CGannotation scriptAnnotation = cgGetNamedPassAnnotation(pass, "Script");
    Script passScriptStates;
    if (cgIsAnnotation(scriptAnnotation)) {
        const std::string s(cgGetStringAnnotationValue(scriptAnnotation));
        ScriptState lastState, newState;
        std::istringstream stream(s);
        std::string segment;
        CGeffect effect = static_cast<CGeffect>(m_effectRef->internalPointer());
        bool renderColorTarget0DidSet = false,
                useRenderBuffer = false,
                useDepthStencilBuffer = false;
        while (std::getline(stream, segment, ';')) {
            std::string::size_type offset = segment.find("=");
            if (offset != std::string::npos) {
                const std::string &command = Util::trim(segment.substr(0, offset));
                const std::string &value = Util::trim(segment.substr(offset + 1));
                newState.setFromState(lastState);
                if (command == "RenderColorTarget" || command == "RenderColorTarget0") {
                    setStateFromRenderColorTargetSemantic(renderColorTarget,
                                                          value,
                                                          ScriptState::kRenderColorTarget0,
                                                          newState);
                    useRenderBuffer = newState.isRenderTargetBound;
                    renderColorTarget0DidSet = true;
                }
                else if (renderColorTarget0DidSet && command == "RenderColorTarget1") {
                    setStateFromRenderColorTargetSemantic(renderColorTarget,
                                                          value,
                                                          ScriptState::kRenderColorTarget1,
                                                          newState);
                    useRenderBuffer = newState.isRenderTargetBound;
                }
                else if (renderColorTarget0DidSet && command == "RenderColorTarget2") {
                    setStateFromRenderColorTargetSemantic(renderColorTarget,
                                                          value,
                                                          ScriptState::kRenderColorTarget2,
                                                          newState);
                    useRenderBuffer = newState.isRenderTargetBound;
                }
                else if (renderColorTarget0DidSet && command == "RenderColorTarget3") {
                    setStateFromRenderColorTargetSemantic(renderColorTarget,
                                                          value,
                                                          ScriptState::kRenderColorTarget3,
                                                          newState);
                    useRenderBuffer = newState.isRenderTargetBound;
                }
                else if (command == "RenderDepthStencilTarget") {
                    setStateFromRenderDepthStencilTargetSemantic(renderDepthStencilTarget,
                                                                 value,
                                                                 ScriptState::kRenderDepthStencilTarget,
                                                                 newState);
                    useDepthStencilBuffer = newState.isRenderTargetBound;
                }
                else if (command == "ClearSetColor") {
                    setStateFromParameter(effect, value, CG_FLOAT4, ScriptState::kClearSetColor, newState);
                }
                else if (command == "ClearSetDepth") {
                    setStateFromParameter(effect, value, CG_FLOAT, ScriptState::kClearSetDepth, newState);
                }
                else if (command == "Clear") {
                    if (value == "Color" && useRenderBuffer) {
                        newState.type = ScriptState::kClearColor;
                    }
                    else if (value == "Depth" && useDepthStencilBuffer) {
                        newState.type = ScriptState::kClearDepth;
                    }
                }
                else if (command == "Draw") {
                    if (value == "Buffer") {
                        if (m_scriptClass == kObject)
                            return false;
                        newState.type = ScriptState::kDrawBuffer;
                    }
                    if (value == "Geometry") {
                        if (m_scriptClass == kScene)
                            return false;
                        newState.type = ScriptState::kDrawGeometry;
                    }
                    newState.pass = pass;
                }
                if (newState.type != ScriptState::kUnknown) {
                    lastState = newState;
                    passScriptStates.push_back(newState);
                    newState.reset();
                }
            }
        }
    }
    else {
        if (m_scriptClass == kScene)
            return false;
        ScriptState state;
        state.pass = pass;
        state.type = ScriptState::kDrawGeometry;
        passScriptStates.push_back(state);
    }
    m_passScripts.insert(pass, passScriptStates);
    return true;
}

bool EffectEngine::parseTechniqueScript(const CGtechnique technique, Passes &passes)
{
    if (!cgIsTechnique(technique) || !cgValidateTechnique(technique)) {
        return false;
    }
    const CGannotation scriptAnnotation = cgGetNamedTechniqueAnnotation(technique, "Script");
    Script techniqueScriptStates;
    if (cgIsAnnotation(scriptAnnotation)) {
        const std::string s(cgGetStringAnnotationValue(scriptAnnotation));
        std::istringstream stream(s);
        std::string segment;
        Script scriptExternalStates;
        ScriptState lastState, newState;
        CGeffect effect = static_cast<CGeffect>(m_effectRef->internalPointer());
        bool useScriptExternal = m_scriptOrder == IEffect::kPostProcess,
                renderColorTarget0DidSet = false,
                renderDepthStencilTargetDidSet = false,
                useRenderBuffer = false,
                useDepthStencilBuffer = false;
        while (std::getline(stream, segment, ';')) {
            std::string::size_type offset = segment.find("=");
            if (offset != std::string::npos) {
                const std::string &command = Util::trim(segment.substr(0, offset));
                const std::string &value = Util::trim(segment.substr(offset + 1));
                newState.setFromState(lastState);
                if (command == "RenderColorTarget" || command == "RenderColorTarget0") {
                    setStateFromRenderColorTargetSemantic(renderColorTarget,
                                                          value,
                                                          ScriptState::kRenderColorTarget0,
                                                          newState);
                    useRenderBuffer = newState.isRenderTargetBound;
                    renderColorTarget0DidSet = true;
                }
                else if (renderColorTarget0DidSet && command == "RenderColorTarget1") {
                    setStateFromRenderColorTargetSemantic(renderColorTarget,
                                                          value,
                                                          ScriptState::kRenderColorTarget1,
                                                          newState);
                    useRenderBuffer = newState.isRenderTargetBound;
                }
                else if (renderColorTarget0DidSet && command == "RenderColorTarget2") {
                    setStateFromRenderColorTargetSemantic(renderColorTarget,
                                                          value,
                                                          ScriptState::kRenderColorTarget2,
                                                          newState);
                    useRenderBuffer = newState.isRenderTargetBound;
                }
                else if (renderColorTarget0DidSet && command == "RenderColorTarget3") {
                    setStateFromRenderColorTargetSemantic(renderColorTarget,
                                                          value,
                                                          ScriptState::kRenderColorTarget3,
                                                          newState);
                    useRenderBuffer = newState.isRenderTargetBound;
                }
                else if (command == "RenderDepthStencilTarget") {
                    setStateFromRenderDepthStencilTargetSemantic(renderDepthStencilTarget,
                                                                 value,
                                                                 ScriptState::kRenderDepthStencilTarget,
                                                                 newState);
                    useDepthStencilBuffer = newState.isRenderTargetBound;
                    renderDepthStencilTargetDidSet = true;
                }
                else if (command == "ClearSetColor") {
                    setStateFromParameter(effect, value, CG_FLOAT4, ScriptState::kClearSetColor, newState);
                }
                else if (command == "ClearSetDepth") {
                    setStateFromParameter(effect, value, CG_FLOAT, ScriptState::kClearSetDepth, newState);
                }
                else if (command == "Clear") {
                    if (value == "Color" && useRenderBuffer) {
                        newState.type = ScriptState::kClearColor;
                    }
                    else if (value == "Depth" && useDepthStencilBuffer) {
                        newState.type = ScriptState::kClearDepth;
                    }
                }
                else if (command == "Pass") {
                    CGpass pass = cgGetNamedPass(technique, value.c_str());
                    if (parsePassScript(pass)) {
                        newState.type = ScriptState::kPass;
                        newState.pass = pass;
                        passes.push_back(pass);
                    }
                }
                else if (!lastState.enterLoop && command == "LoopByCount") {
                    CGparameter parameter = cgGetNamedEffectParameter(effect, value.c_str());
                    if (Util::isIntegerParameter(parameter)) {
                        newState.type = ScriptState::kLoopByCount;
                        newState.enterLoop = true;
                        newState.parameter = parameter;
                    }
                }
                else if (lastState.enterLoop && command == "LoopEnd") {
                    newState.type = ScriptState::kLoopEnd;
                    newState.enterLoop = false;
                }
                else if (lastState.enterLoop && command == "LoopGetIndex") {
                    CGparameter parameter = cgGetNamedEffectParameter(effect, value.c_str());
                    if (Util::isIntegerParameter(parameter)) {
                        newState.type = ScriptState::kLoopGetIndex;
                        newState.enterLoop = true;
                        newState.parameter = parameter;
                    }
                }
                else if (useScriptExternal && command == "ScriptExternal") {
                    newState.type = ScriptState::kScriptExternal;
                    useScriptExternal = false;
                    if (lastState.enterLoop)
                        return false;
                }
                if (newState.type != ScriptState::kUnknown) {
                    lastState = newState;
                    if (useScriptExternal)
                        scriptExternalStates.push_back(newState);
                    else
                        techniqueScriptStates.push_back(newState);
                    newState.reset();
                }
            }
        }
        /*
        if (renderColorTarget0DidSet || renderDepthStencilTargetDidSet) {
            m_techniqueFrameBuffers.insert(technique, frameBufferObject);
        }
        else {
            glDeleteFramebuffers(1, &frameBufferObject);
            frameBufferObject = 0;
        }
        */
        m_techniqueScripts.insert(technique, techniqueScriptStates);
        if (m_externalScript.size() == 0)
            m_externalScript.copyFromArray(scriptExternalStates);
        return !lastState.enterLoop;
    }
    else {
        ScriptState state;
        CGpass pass = cgGetFirstPass(technique);
        while (pass) {
            if (parsePassScript(pass)) {
                state.type = ScriptState::kPass;
                state.pass = pass;
                passes.push_back(pass);
            }
            pass = cgGetNextPass(pass);
        }
    }
    return true;
}

/* EffectEngine::ScriptState */

EffectEngine::ScriptState::ScriptState()
    : type(kUnknown),
      parameter(0),
      pass(0),
      texture(0),
      depthBuffer(0),
      stencilBuffer(0),
      width(0),
      height(0),
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
    parameter = 0;
    pass = 0;
    texture = 0;
    depthBuffer = 0;
    stencilBuffer = 0;
    width = 0;
    height = 0;
    enterLoop = false;
    isRenderTargetBound = false;
}

void EffectEngine::ScriptState::setFromState(const ScriptState &other)
{
    texture = other.texture;
    depthBuffer = other.depthBuffer;
    stencilBuffer = other.stencilBuffer;
    width = other.width;
    height = other.height;
    enterLoop = other.enterLoop;
    isRenderTargetBound = other.isRenderTargetBound;
}

void EffectEngine::ScriptState::setFromTexture(const RenderColorTargetSemantic::Texture *t)
{
    texture = t->id;
    width = t->width;
    height = t->height;
    parameter = t->parameter;
}

void EffectEngine::ScriptState::setFromBuffer(const RenderDepthStencilTargetSemantic::Buffer *b)
{
    const GLuint id = b->id;
    width = b->width;
    height = b->height;
    parameter = b->parameter;
    depthBuffer = id;
    stencilBuffer = id;
}

}
}
