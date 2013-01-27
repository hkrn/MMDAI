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
#include "vpvl2/cg/EffectEngine.h"

#include "vpvl2/extensions/cg/Util.h"
#include "vpvl2/extensions/gl/FrameBufferObject.h"
#include "vpvl2/extensions/gl/VertexBundleLayout.h"

#include <string.h> /* for Linux */

#include <string>
#include <sstream>

namespace
{

using namespace vpvl2;
using namespace vpvl2::cg;
using namespace vpvl2::extensions::cg;

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
static const EffectEngine::DrawPrimitiveCommand kQuadDrawCommand = EffectEngine::DrawPrimitiveCommand(GL_QUADS, kIndicesSize, GL_UNSIGNED_INT, 0, 0, sizeof(int));
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
namespace cg
{

/* BasicParameter */

BaseParameter::BaseParameter()
    : m_baseParameter(0),
      m_effectRef(0)
{
}

BaseParameter::~BaseParameter()
{
    m_baseParameter = 0;
    m_effectRef = 0;
}

void BaseParameter::addParameter(CGparameter parameter, IEffect *effectRef)
{
    connectParameter(parameter, m_baseParameter);
    m_effectRef = effectRef;
}

void BaseParameter::connectParameter(const CGparameter sourceParameter, CGparameter &destinationParameter)
{
    /* prevent infinite reference loop */
    if (sourceParameter != destinationParameter) {
        if (destinationParameter) {
            cgConnectParameter(sourceParameter, destinationParameter);
        }
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

void MatrixSemantic::addParameter(CGparameter parameter, IEffect *effectRef, const char *suffix)
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
    const size_t len = strlen(suffix);
    if (VPVL2_CG_STREQ_CONST(suffix, len, kInverseTransposeSemanticsSuffix)) {
        BaseParameter::connectParameter(sourceParameter, inversetransposed);
    }
    else if (VPVL2_CG_STREQ_CONST(suffix, len, kTransposeSemanticsSuffix)) {
        BaseParameter::connectParameter(sourceParameter, transposed);
    }
    else if (VPVL2_CG_STREQ_CONST(suffix, len, kInverseSemanticsSuffix)) {
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

void MaterialSemantic::addParameter(CGparameter parameter, IEffect *effectRef)
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
    m_effectRef = effectRef;
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
    : BaseParameter(),
      m_mipmap(false)
{
}

MaterialTextureSemantic::~MaterialTextureSemantic()
{
    m_mipmap = false;
}

bool MaterialTextureSemantic::hasMipmap(const CGparameter textureParameter, const CGparameter samplerParameter)
{
    const CGannotation mipmapAnnotation = cgGetNamedParameterAnnotation(textureParameter, "MipLevels");
    bool hasMipmap = false;
    if (cgIsAnnotation(mipmapAnnotation)) {
        hasMipmap = Util::toInt(mipmapAnnotation) != 1;
    }
    const CGannotation levelAnnotation = cgGetNamedParameterAnnotation(textureParameter, "Level");
    if (cgIsAnnotation(levelAnnotation)) {
        hasMipmap = Util::toInt(levelAnnotation) != 1;
    }
    CGstateassignment sa = cgGetFirstSamplerStateAssignment(samplerParameter);
    while (sa) {
        const CGstate s = cgGetSamplerStateAssignmentState(sa);
        if (cgGetStateType(s) == CG_INT) {
            const char *name = cgGetStateName(s);
            const size_t len = strlen(name);
            if (VPVL2_CG_STREQ_CASE_CONST(name, len, "MINFILTER")) {
                int nvalue = 0;
                const int *v = cgGetIntStateAssignmentValues(sa, &nvalue);
                if (nvalue > 0) {
                    int value = v[0];
                    switch (value) {
                    case GL_NEAREST_MIPMAP_NEAREST:
                    case GL_NEAREST_MIPMAP_LINEAR:
                    case GL_LINEAR_MIPMAP_NEAREST:
                    case GL_LINEAR_MIPMAP_LINEAR:
                        hasMipmap = true;
                        break;
                    default:
                        break;
                    }
                    if (hasMipmap)
                        break;
                }
            }
        }
        sa = cgGetNextStateAssignment(sa);
    }
    return hasMipmap;
}

void MaterialTextureSemantic::addParameter(const CGparameter textureParameter,
                                           CGparameter samplerParameter,
                                           IEffect *effectRef)
{
    if (hasMipmap(textureParameter, samplerParameter))
        m_mipmap = true;
    BaseParameter::addParameter(samplerParameter, effectRef);
}

void MaterialTextureSemantic::setTexture(const HashPtr &key, GLuint value)
{
    m_textures.insert(key, value);
    cgGLSetupSampler(m_baseParameter, value);
}

void MaterialTextureSemantic::updateParameter(const HashPtr &key)
{
    if (const GLuint *value = m_textures.find(key)) {
        cgGLSetTextureParameter(m_baseParameter, *value);
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
    cgGLSetTextureParameter(m_baseParameter, value);
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

void GeometrySemantic::addParameter(CGparameter parameter, IEffect *effectRef)
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
    m_effectRef = effectRef;
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

void TimeSemantic::addParameter(CGparameter parameter, IEffect *effectRef)
{
    CGannotation annotation = cgGetNamedParameterAnnotation(parameter, "SyncInEditMode");
    if (cgIsAnnotation(annotation)) {
        if (Util::toBool(annotation)) {
            BaseParameter::connectParameter(parameter, m_syncEnabled);
            return;
        }
    }
    BaseParameter::connectParameter(parameter, m_syncDisabled);
    m_effectRef = effectRef;
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

ControlObjectSemantic::ControlObjectSemantic(const Scene *scene, const IRenderContext *renderContextRef)
    : BaseParameter(),
      m_sceneRef(scene),
      m_renderContextRef(renderContextRef)
{
}

ControlObjectSemantic::~ControlObjectSemantic()
{
    m_sceneRef = 0;
    m_renderContextRef = 0;
}

void ControlObjectSemantic::addParameter(CGparameter parameter, IEffect *effectRef)
{
    if (cgIsAnnotation(cgGetNamedParameterAnnotation(parameter, "name"))) {
        m_parameters.append(parameter);
        m_parameter2EffectRefs.insert(parameter, effectRef);
    }
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
            IEffect *parent = m_effectRef->parentEffectRef();
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
            if (type == IModel::kPMDModel || type == IModel::kPMXModel) {
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
                    cgSetParameter1f(parameter, float(morph->weight()));
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
}

RenderColorTargetSemantic::~RenderColorTargetSemantic()
{
    m_textures.releaseAll();
    m_renderContextRef = 0;
}

bool RenderColorTargetSemantic::tryGetTextureFlags(const CGparameter textureParameter,
                                                   const CGparameter samplerParameter,
                                                   bool enableAllTextureTypes,
                                                   int &flags)
{
    const CGannotation resourceType = cgGetNamedParameterAnnotation(textureParameter, "ResourceType");
    if (enableAllTextureTypes && cgIsAnnotation(resourceType)) {
        const char *typeName = cgGetStringAnnotationValue(resourceType);
        const size_t len = strlen(typeName);
        const CGtype samplerType = cgGetParameterType(samplerParameter);
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
            return false;
        }
    }
    else {
        flags = IRenderContext::kTexture2D;
    }
    if (MaterialTextureSemantic::hasMipmap(textureParameter, samplerParameter))
        flags |= IRenderContext::kGenerateTextureMipmap;
    return true;
}

void RenderColorTargetSemantic::addParameter(CGparameter textureParameter,
                                             CGparameter samplerParameter,
                                             IEffect *effectRef,
                                             FrameBufferObject *frameBufferObjectRef,
                                             const IString *dir,
                                             bool enableResourceName,
                                             bool enableAllTextureTypes)
{
    int flags;
    if (!tryGetTextureFlags(textureParameter, samplerParameter, enableAllTextureTypes, flags))
        return;
    const CGannotation resourceName = cgGetNamedParameterAnnotation(textureParameter, "ResourceName");
    const char *name = cgGetParameterName(textureParameter);
    IRenderContext::SharedTextureParameter sharedTextureParameter(cgGetParameterContext(textureParameter));
    GLuint textureID = 0;
    if (enableResourceName && cgIsAnnotation(resourceName)) {
        const char *name = cgGetStringAnnotationValue(resourceName);
        IString *s = m_renderContextRef->toUnicode(reinterpret_cast<const uint8_t*>(name));
        IRenderContext::Texture texture(flags);
        texture.async = false;
        if (m_renderContextRef->uploadTexture(s, dir, texture, 0)) {
            textureID = texture.opaque;
            cgGLSetupSampler(samplerParameter, textureID);
            m_path2parameters.insert(name, textureParameter);
            m_textures.append(new FrameBufferObject::ExternalTexture(Vector3(texture.width, texture.height, 0), 0, 0, 0, GL_TEXTURE_2D, textureID));
            FrameBufferObject::AbstractTexture *texture = m_textures[m_textures.count() - 1];
            m_name2textures.insert(cgGetParameterName(textureParameter), Texture(frameBufferObjectRef, texture, textureParameter, samplerParameter));
        }
        delete s;
    }
    else if (m_renderContextRef->tryGetSharedTextureParameter(name, sharedTextureParameter)) {
        CGparameter parameter = static_cast<CGparameter>(sharedTextureParameter.parameter);
        if (strcmp(cgGetParameterSemantic(parameter), cgGetParameterSemantic(textureParameter)) == 0) {
            textureParameter = parameter;
            textureID = static_cast<GLuint>(sharedTextureParameter.opaque);
        }
    }
    else if ((flags & IRenderContext::kTexture3D) != 0) {
        generateTexture3D0(textureParameter, samplerParameter, frameBufferObjectRef);
        textureID = m_textures[m_textures.count() - 1]->name();
    }
    else if ((flags & IRenderContext::kTexture2D) != 0) {
        generateTexture2D0(textureParameter, samplerParameter, frameBufferObjectRef);
        textureID = m_textures[m_textures.count() - 1]->name();
    }
    m_parameters.append(textureParameter);
    m_parameter2EffectRefs.insert(textureParameter, effectRef);
    if (cgIsParameter(samplerParameter) && textureID > 0) {
        cgGLSetupSampler(samplerParameter, textureID);
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

int RenderColorTargetSemantic::countParameters() const
{
    return m_parameters.count();
}

void RenderColorTargetSemantic::generateTexture2D(const CGparameter parameter,
                                                  const CGparameter sampler,
                                                  const Vector3 &size,
                                                  FrameBufferObject *frameBufferObjectRef,
                                                  GLenum &format)
{
    GLenum textureInternal, textureFormat, byteAlignType;
    Util::getTextureFormat(parameter, textureInternal, textureFormat, byteAlignType);
    m_textures.append(new FrameBufferObject::Texture2D(size, textureFormat, textureInternal, byteAlignType));
    FrameBufferObject::AbstractTexture *tex = lastTextureRef();
    tex->create();
    m_name2textures.insert(cgGetParameterName(parameter), Texture(frameBufferObjectRef, tex, parameter, sampler));
    glBindTexture(GL_TEXTURE_2D, tex->name());
    if (MaterialTextureSemantic::hasMipmap(parameter, sampler))
        glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    format = textureInternal;
}

void RenderColorTargetSemantic::generateTexture3D(const CGparameter parameter,
                                                  const CGparameter sampler,
                                                  const Vector3 &size,
                                                  FrameBufferObject *frameBufferObjectRef)
{
    GLenum textureInternal, textureFormat, byteAlignType;
    Util::getTextureFormat(parameter, textureInternal, textureFormat, byteAlignType);
    m_textures.append(new FrameBufferObject::Texture3D(size, textureFormat, textureInternal, byteAlignType));
    FrameBufferObject::AbstractTexture *tex = lastTextureRef();
    tex->create();
    m_name2textures.insert(cgGetParameterName(parameter), Texture(frameBufferObjectRef, tex, parameter, sampler));
    glBindTexture(GL_TEXTURE_3D, tex->name());
    if (MaterialTextureSemantic::hasMipmap(parameter, sampler))
        glGenerateMipmap(GL_TEXTURE_3D);
    glBindTexture(GL_TEXTURE_3D, 0);
}

void RenderColorTargetSemantic::generateTexture2D0(const CGparameter parameter,
                                                   const CGparameter sampler,
                                                   FrameBufferObject *frameBufferObjectRef)
{
    size_t width, height;
    GLenum format;
    getSize2(parameter, width, height);
    generateTexture2D(parameter, sampler, Vector3(width, height, 0), frameBufferObjectRef, format);
}

void RenderColorTargetSemantic::generateTexture3D0(const CGparameter parameter,
                                                   const CGparameter sampler,
                                                   FrameBufferObject *frameBufferObjectRef)
{
    size_t width, height, depth;
    getSize3(parameter, width, height, depth);
    generateTexture3D(parameter, sampler, Vector3(width, height, depth), frameBufferObjectRef);
}

void RenderColorTargetSemantic::getSize2(const CGparameter parameter, size_t &width, size_t &height) const
{
    Vector3 size;
    if (Util::getSize2(parameter, size)) {
        width = size.x();
        height = size.y();
    }
    else {
        Vector3 viewport;
        m_renderContextRef->getViewport(viewport);
        width = btMax(size_t(1), size_t(viewport.x() * size.x()));
        height = btMax(size_t(1), size_t(viewport.y() * size.y()));
    }
}

void RenderColorTargetSemantic::getSize3(const CGparameter parameter, size_t &width, size_t &height, size_t &depth) const
{
    Vector3 size;
    if (Util::getSize3(parameter, size)) {
        width = size.x();
        height = size.y();
        depth = size.z();
    }
    else {
        Vector3 viewport;
        m_renderContextRef->getViewport(viewport);
        width = btMax(size_t(1), size_t(viewport.x()));
        height = btMax(size_t(1), size_t(viewport.y()));
        depth = 24;
    }
}

FrameBufferObject::AbstractTexture *RenderColorTargetSemantic::lastTextureRef() const
{
    return m_textures[m_textures.count() - 1];
}

/* RenderDepthStencilSemantic */

RenderDepthStencilTargetSemantic::RenderDepthStencilTargetSemantic(IRenderContext *renderContextRef)
    : RenderColorTargetSemantic(renderContextRef)
{
}

RenderDepthStencilTargetSemantic::~RenderDepthStencilTargetSemantic()
{
}

void RenderDepthStencilTargetSemantic::addParameter(CGparameter parameter,
                                                    IEffect *effectRef,
                                                    FrameBufferObject *frameBufferObjectRef)
{
    if (cgIsParameter(parameter)) {
        size_t width, height;
        getSize2(parameter, width, height);
        m_parameters.append(parameter);
        m_effectRef = effectRef;
        m_renderBuffers.append(new FrameBufferObject::StandardRenderBuffer(Vector3(width, height, 0), GL_DEPTH24_STENCIL8));
        FrameBufferObject::AbstractRenderBuffer *renderBuffer = m_renderBuffers[m_renderBuffers.count() - 1];
        renderBuffer->create();
        m_buffers.insert(cgGetParameterName(parameter), Buffer(frameBufferObjectRef, renderBuffer, parameter));
    }
}

const RenderDepthStencilTargetSemantic::Buffer *RenderDepthStencilTargetSemantic::findDepthStencilBuffer(const char *name) const
{
    return m_buffers.find(name);
}

/* OffscreenRenderTargetSemantic */

OffscreenRenderTargetSemantic::OffscreenRenderTargetSemantic(IRenderContext *renderContextRef)
    : RenderColorTargetSemantic(renderContextRef),
      m_effectRef(0)
{
}

OffscreenRenderTargetSemantic::~OffscreenRenderTargetSemantic()
{
    m_effectRef = 0;
}

void OffscreenRenderTargetSemantic::generateTexture2D(const CGparameter parameter,
                                                      const CGparameter sampler,
                                                      const Vector3 &size,
                                                      FrameBufferObject *frameBufferObjectRef,
                                                      GLenum &format)
{
    RenderColorTargetSemantic::generateTexture2D(parameter, sampler, size, frameBufferObjectRef, format);
    FrameBufferObject::AbstractTexture *tex = lastTextureRef();
    m_effectRef->addOffscreenRenderTarget(tex, parameter, sampler);
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

void AnimatedTextureSemantic::addParameter(CGparameter parameter, IEffect *effectRef)
{
    CGannotation annotation = cgGetNamedParameterAnnotation(parameter, "ResourceName");
    if (cgIsAnnotation(annotation) && cgGetParameterType(parameter) == CG_TEXTURE) {
        m_parameters.append(parameter);
        m_parameter2EffectRefs.insert(parameter, effectRef);
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
            GLuint textureID = t->textureRef->name();
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

void TextureValueSemantic::addParameter(CGparameter parameter, IEffect *effectRef)
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
            m_parameters.append(parameter);
            m_parameter2EffectRefs.insert(parameter, effectRef);
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

void SelfShadowSemantic::addParameter(CGparameter parameter, IEffect *effectRef)
{
    CGannotation nameAnnotation = cgGetNamedParameterAnnotation(parameter, "name");
    if (cgIsAnnotation(nameAnnotation)) {
        const char *name = cgGetStringAnnotationValue(nameAnnotation);
        size_t len = strlen(name);
        if (VPVL2_CG_STREQ_CASE_CONST(name, len, "rate")) {
            m_rate = parameter;
        }
        if (VPVL2_CG_STREQ_CASE_CONST(name, len, "size")) {
            m_size = parameter;
        }
        else if (VPVL2_CG_STREQ_CASE_CONST(name, len, "center")) {
            m_center = parameter;
        }
        else if (VPVL2_CG_STREQ_CASE_CONST(name, len, "distance")) {
            m_distance = parameter;
        }
        m_effectRef = effectRef;
    }
}

void SelfShadowSemantic::updateParameter(const IShadowMap *shadowMapRef)
{
    if (cgIsParameter(m_center)) {
        cgSetParameter3fv(m_center, shadowMapRef->position());
    }
    if (cgIsParameter(m_distance)) {
        cgSetParameter1f(m_distance, shadowMapRef->distance());
    }
    if (cgIsParameter(m_rate)) {
        cgSetParameter1f(m_rate, 1); //shadowMapRef->rate());
    }
    if (cgIsParameter(m_size)) {
        cgSetParameter2fv(m_size, shadowMapRef->size());
    }
}

/* Effect::RectRenderEngine */
class EffectEngine::RectangleRenderEngine
{
public:
    RectangleRenderEngine()
        : m_vertexBundle(0),
          m_verticesBuffer(0),
          m_indicesBuffer(0)
    {
    }
    ~RectangleRenderEngine() {
        m_bundle.releaseVertexArrayObjects(&m_vertexBundle, 1);
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
        m_bundle.allocateVertexArrayObjects(&m_vertexBundle, 1);
        m_bundle.bindVertexArrayObject(m_vertexBundle);
        bindVertexBundle(false);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        m_bundle.unbindVertexArrayObject();
        unbindVertexBundle(false);
    }
    void bindVertexBundle(bool bundle) {
        if (!bundle || !m_bundle.bindVertexArrayObject(m_vertexBundle)) {
            glBindBuffer(GL_ARRAY_BUFFER, m_verticesBuffer);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indicesBuffer);
            glVertexPointer(2, GL_FLOAT, kVertexStride, reinterpret_cast<const GLvoid *>(0));
            glTexCoordPointer(2, GL_FLOAT, kVertexStride, reinterpret_cast<const GLvoid *>(kTextureOffset));
        }
        glDisable(GL_DEPTH_TEST);
    }
    void unbindVertexBundle(bool bundle) {
        if (!bundle || !m_bundle.unbindVertexArrayObject()) {
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        }
        glEnable(GL_DEPTH_TEST);
    }

private:
    VertexBundleLayout m_bundle;
    GLuint m_vertexBundle;
    GLuint m_verticesBuffer;
    GLuint m_indicesBuffer;
};

/* EffectEngine */
EffectEngine::EffectEngine(Scene *sceneRef, IRenderContext *renderContextRef)
    : world(renderContextRef, IRenderContext::kWorldMatrix),
      view(renderContextRef, IRenderContext::kViewMatrix),
      projection(renderContextRef, IRenderContext::kProjectionMatrix),
      worldView(renderContextRef, IRenderContext::kWorldMatrix | IRenderContext::kViewMatrix),
      viewProjection(renderContextRef, IRenderContext::kViewMatrix | IRenderContext::kProjectionMatrix),
      worldViewProjection(renderContextRef, IRenderContext::kWorldMatrix | IRenderContext::kViewMatrix | IRenderContext::kProjectionMatrix),
      time(renderContextRef),
      elapsedTime(renderContextRef),
      controlObject(sceneRef, renderContextRef),
      renderColorTarget(renderContextRef),
      renderDepthStencilTarget(renderContextRef),
      animatedTexture(renderContextRef),
      offscreenRenderTarget(renderContextRef),
      index(0),
      m_effectRef(0),
      m_defaultStandardEffect(0),
      m_renderContextRef(renderContextRef),
      m_rectangleRenderEngine(0),
      m_frameBufferObjectRef(0),
      m_scriptOutput(kColor),
      m_scriptClass(kObject)
{
    /* prepare pre/post effect that uses rectangle (quad) rendering */
    m_rectangleRenderEngine = new RectangleRenderEngine();
    m_rectangleRenderEngine->initializeVertexBundle();
}

EffectEngine::~EffectEngine()
{
    delete m_rectangleRenderEngine;
    m_rectangleRenderEngine = 0;
    m_effectRef = 0;
    m_renderContextRef = 0;
}

bool EffectEngine::setEffect(IEffect *effectRef, const IString *dir, bool isDefaultStandardEffect)
{
    if (!effectRef)
        return false;
    CGeffect value = static_cast<CGeffect>(effectRef->internalPointer());
    if (!cgIsEffect(value))
        return false;
    m_effectRef = static_cast<Effect *>(effectRef);
    CGparameter parameter = cgGetFirstEffectParameter(value), standardsGlobal = 0;
    FrameBufferObject *frameBufferObjectRef = m_frameBufferObjectRef = m_effectRef->parentFrameBufferObject();
    while (parameter) {
        const char *semantic = cgGetParameterSemantic(parameter);
        const size_t slen = strlen(semantic);
        if (VPVL2_CG_STREQ_CONST(semantic, slen, "VIEWPORTPIXELSIZE")) {
            viewportPixelSize.addParameter(parameter, effectRef);
        }
        else if (VPVL2_CG_STREQ_SUFFIX(semantic, slen, kWorldViewProjectionSemantic)) {
            worldViewProjection.addParameter(parameter, effectRef, VPVL2_CG_GET_SUFFIX(semantic, kWorldViewProjectionSemantic));
        }
        else if (VPVL2_CG_STREQ_SUFFIX(semantic, slen, kWorldViewSemantic)) {
            worldView.addParameter(parameter, effectRef, VPVL2_CG_GET_SUFFIX(semantic, kWorldViewSemantic));
        }
        else if (VPVL2_CG_STREQ_SUFFIX(semantic, slen, kViewProjectionSemantic)) {
            viewProjection.addParameter(parameter, effectRef, VPVL2_CG_GET_SUFFIX(semantic, kViewProjectionSemantic));
        }
        else if (VPVL2_CG_STREQ_SUFFIX(semantic, slen, kWorldSemantic)) {
            world.addParameter(parameter, effectRef, VPVL2_CG_GET_SUFFIX(semantic, kWorldSemantic));
        }
        else if (VPVL2_CG_STREQ_SUFFIX(semantic, slen, kViewSemantic)) {
            view.addParameter(parameter, effectRef, VPVL2_CG_GET_SUFFIX(semantic, kViewSemantic));
        }
        else if (VPVL2_CG_STREQ_SUFFIX(semantic, slen, kProjectionSemantic)) {
            projection.addParameter(parameter, effectRef, VPVL2_CG_GET_SUFFIX(semantic, kProjectionSemantic));
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "DIFFUSE")) {
            diffuse.addParameter(parameter, effectRef);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "AMBIENT")) {
            ambient.addParameter(parameter, effectRef);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "EMISSIVE")) {
            emissive.addParameter(parameter, effectRef);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "SPECULARPOWER")) {
            specularPower.addParameter(parameter, effectRef);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "SPECULAR")) {
            specular.addParameter(parameter, effectRef);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "TOONCOLOR")) {
            toonColor.addParameter(parameter, effectRef);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "EDGECOLOR")) {
            edgeColor.addParameter(parameter, effectRef);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "_POSITION")) {
            position.addParameter(parameter, effectRef);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "_DIRECTION")) {
            direction.addParameter(parameter, effectRef);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "TIME")) {
            time.addParameter(parameter, effectRef);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "ELAPSEDTIME")) {
            elapsedTime.addParameter(parameter, effectRef);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "MOUSEPOSITION")) {
            mousePosition.addParameter(parameter, effectRef);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "LEFTMOUSEDOWN")) {
            leftMouseDown.addParameter(parameter, effectRef);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "MIDDLEMOUSEDOWN")) {
            middleMouseDown.addParameter(parameter, effectRef);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "RIGHTMOUSEDOWN")) {
            rightMouseDown.addParameter(parameter, effectRef);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "CONTROLOBJECT")) {
            controlObject.addParameter(parameter, effectRef);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "ANIMATEDTEXTURE")) {
            animatedTexture.addParameter(parameter, effectRef);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "TEXTUREVALUE")) {
            textureValue.addParameter(parameter, effectRef);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "RENDERDEPTHSTENCILTARGET")) {
            renderDepthStencilTarget.addParameter(parameter, effectRef, frameBufferObjectRef);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "SELFSHADOWVPVM")) {
            selfShadow.addParameter(parameter, effectRef);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "SHAREDRENDERCOLORTARGETVPVM")) {
            addSharedTextureParameter(parameter, effectRef, frameBufferObjectRef, renderColorTarget);
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "SHAREDOFFSCREENRENDERTARGETVPVM")) {
            addSharedTextureParameter(parameter, effectRef, frameBufferObjectRef, offscreenRenderTarget);
        }
        else if (!standardsGlobal && VPVL2_CG_STREQ_CONST(semantic, slen, "STANDARDSGLOBAL")) {
            standardsGlobal = parameter;
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "_INDEX")) {
        }
        else if (VPVL2_CG_STREQ_CONST(semantic, slen, "TEXUNIT0")) {
            depthTexture.addParameter(parameter, effectRef);
        }
        else {
            const char *name = cgGetParameterName(parameter);
            const size_t nlen = strlen(name);
            if (VPVL2_CG_STREQ_CONST(name, nlen, "parthf")) {
                parthf.addParameter(parameter, effectRef);
            }
            else if (VPVL2_CG_STREQ_CONST(name, nlen, "spadd")) {
                spadd.addParameter(parameter, effectRef);
            }
            else if (VPVL2_CG_STREQ_CONST(name, nlen, "transp")) {
                transp.addParameter(parameter, effectRef);
            }
            else if (VPVL2_CG_STREQ_CONST(name, nlen, "use_texture")) {
                useTexture.addParameter(parameter, effectRef);
            }
            else if (VPVL2_CG_STREQ_CONST(name, nlen, "use_spheremap")) {
                useSpheremap.addParameter(parameter, effectRef);
            }
            else if (VPVL2_CG_STREQ_CONST(name, nlen, "use_toon")) {
                useToon.addParameter(parameter, effectRef);
            }
            else if (VPVL2_CG_STREQ_CONST(name, nlen, "opadd")) {
                opadd.addParameter(parameter, effectRef);
            }
            else if (VPVL2_CG_STREQ_CONST(name, nlen, "VertexCount")) {
                vertexCount.addParameter(parameter, effectRef);
            }
            else if (VPVL2_CG_STREQ_CONST(name, nlen, "SubsetCount")) {
                subsetCount.addParameter(parameter, effectRef);
            }
            else {
                const CGtype parameterType = cgGetParameterType(parameter);
                if (parameterType == CG_SAMPLER2D ||
                        parameterType == CG_SAMPLER3D ||
                        parameterType == CG_SAMPLERCUBE) {
                    parseSamplerStateParameter(parameter, effectRef, frameBufferObjectRef, dir);
                }
            }
        }
        if (Effect::isInteractiveParameter(parameter))
            m_effectRef->addInteractiveParameter(parameter);
        parameter = cgGetNextParameter(parameter);
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
        CGtechnique technique = cgGetFirstTechnique(value);
        while (technique) {
            addTechniquePasses(technique);
            technique = cgGetNextTechnique(technique);
        }
    }
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
    if (!technique) {
        const int ntechniques2 = m_defaultTechniques.size();
        for (int i = 0; i < ntechniques2; i++) {
            CGtechnique t = m_defaultTechniques[i];
            if (testTechnique(t, pass, offset, nmaterials, hasTexture, hasSphereMap, useToon)) {
                technique = t;
                break;
            }
        }
    }
    return technique;
}

void EffectEngine::executeScriptExternal()
{
    if (scriptOrder() == IEffect::kPostProcess) {
        bool isPassExecuted; /* unused and ignored */
        DrawPrimitiveCommand command;
        executeScript(&m_externalScript, 0, command, isPassExecuted);
    }
}

bool EffectEngine::hasTechniques(IEffect::ScriptOrderType order) const
{
    return scriptOrder() == order ? m_techniqueScripts.size() > 0 : false;
}

void EffectEngine::executeProcess(const IModel *model,
                                  const IEffect *nextPostEffectRef,
                                  IEffect::ScriptOrderType order)
{
    if (!m_effectRef || scriptOrder() != order)
        return;
    setZeroGeometryParameters(model);
    diffuse.setGeometryColor(Color(0, 0, 0, model ? model->opacity() : 0)); /* for asset opacity */
    CGtechnique technique = findTechnique("object", 0, 0, false, false, false);
    executeTechniquePasses(technique, nextPostEffectRef, kQuadDrawCommand);
}

void EffectEngine::executeTechniquePasses(const CGtechnique technique,
                                          const IEffect *nextPostEffectRef,
                                          const DrawPrimitiveCommand &command)
{
    if (cgIsTechnique(technique)) {
        const Script *tss = m_techniqueScripts.find(technique);
        bool isPassExecuted;
        executeScript(tss, nextPostEffectRef, command, isPassExecuted);
        if (!isPassExecuted) {
            if (const Passes *passes = m_techniquePasses.find(technique)) {
                const int npasses = passes->size();
                for (int i = 0; i < npasses; i++) {
                    CGpass pass = passes->at(i);
                    const Script *pss = m_passScripts.find(pass);
                    executeScript(pss, nextPostEffectRef, command, isPassExecuted);
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
    if (!m_defaultStandardEffect) {
        CGeffect effect = static_cast<CGeffect>(effectRef->internalPointer());
        CGtechnique technique = cgGetFirstTechnique(effect);
        while (technique) {
            Passes passes;
            if (parseTechniqueScript(technique, passes)) {
                m_techniquePasses.insert(technique, passes);
                m_defaultTechniques.push_back(technique);
            }
            technique = cgGetNextTechnique(technique);
        }
        m_defaultStandardEffect = effectRef;
    }
}

void EffectEngine::setZeroGeometryParameters(const IModel *model)
{
    edgeColor.setGeometryColor(model ? model->edgeColor() : kZeroV3);
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

void EffectEngine::updateModelGeometryParameters(const Scene *scene, const IModel *model)
{
    const ILight *light = scene->light();
    const Vector3 &lightColor = light->color();
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

bool EffectEngine::isStandardEffect() const
{
    return scriptOrder() == IEffect::kStandard;
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
    if (!cgIsTechnique(technique))
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

void EffectEngine::setScriptStateFromRenderColorTargetSemantic(const RenderColorTargetSemantic &semantic,
                                                               const std::string &value,
                                                               ScriptState::Type type,
                                                               ScriptState &state)
{
    bool bound = false;
    state.type = type;
    if (!value.empty()) {
        if (const RenderColorTargetSemantic::Texture *texture = semantic.findTexture(value.c_str())) {
            state.renderColorTargetTextureRef = texture;
            m_target2textureRefs.insert(type, texture);
            bound = true;
        }
    }
    else if (const RenderColorTargetSemantic::Texture *const *texturePtr = m_target2textureRefs.find(type)) {
        const RenderColorTargetSemantic::Texture *texture = *texturePtr;
        state.renderColorTargetTextureRef = texture;
        m_target2textureRefs.remove(type);
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
            m_target2bufferRefs.insert(type, buffer);
            bound = true;
        }
    }
    else if (const RenderDepthStencilTargetSemantic::Buffer *const *bufferPtr = m_target2bufferRefs.find(type)) {
        const RenderDepthStencilTargetSemantic::Buffer *buffer = *bufferPtr;
        state.renderDepthStencilBufferRef = buffer;
        m_target2bufferRefs.remove(type);
    }
    state.isRenderTargetBound = bound;
}

void EffectEngine::setScriptStateFromParameter(const CGeffect effect,
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

void EffectEngine::executePass(CGpass pass, const DrawPrimitiveCommand &command) const
{
    if (cgIsPass(pass)) {
        cgSetPassState(pass);
        drawPrimitives(command);
        cgResetPassState(pass);
    }
}

void EffectEngine::setRenderColorTargetFromScriptState(const ScriptState &state, const IEffect *nextPostEffectRef)
{
    if (const RenderColorTargetSemantic::Texture *textureRef = state.renderColorTargetTextureRef) {
        const int index = state.type - ScriptState::kRenderColorTarget0, target = GL_COLOR_ATTACHMENT0 + index;
        if (FrameBufferObject *fbo = textureRef->frameBufferObjectRef) {
            const int nRenderColorTargets = m_renderColorTargets.size();
            if (state.isRenderTargetBound) {
                const FrameBufferObject::AbstractTexture *tref = textureRef->textureRef;
                if (m_renderColorTargets.findLinearSearch(target) == nRenderColorTargets) {
                    /* The render color target is not bound yet  */
                    m_renderColorTargets.push_back(target);
                    fbo->bindTexture(tref, index);
                    Util::setRenderColorTargets(&m_renderColorTargets[0], m_renderColorTargets.size());
                }
                else {
                    /* change current color attachment to the specified texture */
                    fbo->transferMSAABuffer(index);
                    fbo->bindTexture(tref, index);
                }
                const Vector3 &size = tref->size();
                glViewport(0, 0, size.x(), size.y());
            }
            else if (nextPostEffectRef && nRenderColorTargets > 0) {
                /* discards all color attachments */
                m_renderColorTargets.clear();
            }
            else if (!nextPostEffectRef && nRenderColorTargets > 0) {
                /* final color output */
                if (index > 0) {
                    fbo->transferMSAABuffer(index);
                    m_renderColorTargets.remove(target);
                    Util::setRenderColorTargets(&m_renderColorTargets[0], m_renderColorTargets.size());
                }
                else {
                    /* reset to the default window framebuffer */
                    for (int i = 0; i < nRenderColorTargets; i++) {
                        const int target2 = m_renderColorTargets[i], index2 = target2 - GL_COLOR_ATTACHMENT0;
                        fbo->transferMSAABuffer(index2);
                    }
                    for (int i = 0; i < nRenderColorTargets; i++) {
                        const int target2 = m_renderColorTargets[i], index2 = target2 - GL_COLOR_ATTACHMENT0;
                        fbo->unbindTexture(index2);
                        fbo->unbindDepthStencilBuffer();
                    }
                    fbo->unbind();
                    m_renderColorTargets.clear();
                    Util::setRenderColorTargets(0, 0);
                    Vector3 viewport;
                    m_renderContextRef->getViewport(viewport);
                    glViewport(0, 0, GLsizei(viewport.x()), GLsizei(viewport.y()));
                }
            }
        }
    }
}

void EffectEngine::setRenderDepthStencilTargetFromScriptState(const ScriptState &state, const IEffect *nextPostEffectRef)
{
    if (const RenderDepthStencilTargetSemantic::Buffer *bufferRef = state.renderDepthStencilBufferRef) {
        if (FrameBufferObject *fbo = bufferRef->frameBufferObjectRef) {
            if (state.isRenderTargetBound) {
                fbo->bindDepthStencilBuffer(bufferRef->renderBufferRef);
            }
            else if (!nextPostEffectRef && m_renderColorTargets.size() > 0) {
                fbo->unbindDepthStencilBuffer();
            }
        }
    }
}

void EffectEngine::executeScript(const Script *script,
                                 const IEffect *nextPostEffectRef,
                                 const DrawPrimitiveCommand &command,
                                 bool &isPassExecuted)
{
    isPassExecuted = scriptOrder() == IEffect::kPostProcess;
    if (script) {
        const int nstates = script->size();
        int stateIndex = 0, nloop = 0, currentIndex = 0, backStateIndex = 0;
        Vector4 v4;
        Vector3 viewport;
        m_renderContextRef->getViewport(viewport);
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
                setRenderColorTargetFromScriptState(state, nextPostEffectRef);
                break;
            case ScriptState::kRenderDepthStencilTarget:
                setRenderDepthStencilTargetFromScriptState(state, nextPostEffectRef);
                break;
            case ScriptState::kDrawBuffer:
                if (m_scriptClass != kObject) {
                    FrameBufferObject *nextPostEffectFrameBufferRef = 0;
                    const bool noRenderTarget = m_renderColorTargets.size() == 0;
                    /* redirect drawing window buffer to the frame buffer */
                    if (noRenderTarget) {
                        static const GLenum target[] = { GL_COLOR_ATTACHMENT0 };
                        nextPostEffectFrameBufferRef = nextPostEffectRef
                                ? nextPostEffectRef->parentFrameBufferObject() : 0;
                        m_frameBufferObjectRef->bindSwapBuffer(nextPostEffectFrameBufferRef, viewport);
                        Util::setRenderColorTargets(target, 1);
                    }
                    m_rectangleRenderEngine->bindVertexBundle(true);
                    executePass(state.pass, kQuadDrawCommand);
                    m_rectangleRenderEngine->unbindVertexBundle(true);
                    /* transfer current frame buffer to the next post effect */
                    if (nextPostEffectFrameBufferRef) {
                        m_frameBufferObjectRef->transferSwapBuffer(nextPostEffectFrameBufferRef, kZeroV3);
                    }
                    /* transfer current frame buffer to the window buffer  */
                    else if (noRenderTarget) {
                        m_frameBufferObjectRef->transferSwapBuffer(0, viewport);
                        Util::setRenderColorTargets(0, 0);
                    }
                    rebindVertexBundle();
                }
                break;
            case ScriptState::kDrawGeometry:
                if (m_scriptClass != kScene) {
                    executePass(state.pass, command);
                }
                break;
            case ScriptState::kPass:
                executeScript(m_passScripts.find(state.pass), nextPostEffectRef, command, isPassExecuted);
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
            m_effectRef->setScriptOrderType(IEffect::kStandard);
        }
        else if (VPVL2_CG_STREQ_CONST(value, len, "preprocess")) {
            m_effectRef->setScriptOrderType(IEffect::kPreProcess);
        }
        else if (VPVL2_CG_STREQ_CONST(value, len, "postprocess")) {
            m_effectRef->setScriptOrderType(IEffect::kPostProcess);
        }
    }
    const CGannotation scriptAnnotation = cgGetNamedParameterAnnotation(parameter, "Script");
    if (cgIsAnnotation(scriptAnnotation)) {
        const char *value = cgGetStringAnnotationValue(scriptAnnotation);
        const size_t len = strlen(value);
        m_techniques.clear();
        CGeffect effect = static_cast<CGeffect>(m_effectRef->internalPointer());
        if (VPVL2_CG_STREQ_SUFFIX(value, len, kMultipleTechniquesPrefix)) {
            const std::string &s = Util::trimLastSemicolon(VPVL2_CG_GET_SUFFIX(value, kMultipleTechniquesPrefix));
            std::istringstream stream(s);
            std::string segment;
            while (std::getline(stream, segment, ':')) {
                CGtechnique technique = cgGetNamedTechnique(effect, segment.c_str());
                addTechniquePasses(technique);
            }
            ownTechniques = true;
        }
        else if (VPVL2_CG_STREQ_SUFFIX(value, len, kSingleTechniquePrefix)) {
            const std::string &s = Util::trimLastSemicolon(VPVL2_CG_GET_SUFFIX(value, kSingleTechniquePrefix));
            CGtechnique technique = cgGetNamedTechnique(effect, s.c_str());
            addTechniquePasses(technique);
            ownTechniques = true;
        }
    }
}

void EffectEngine::parseSamplerStateParameter(CGparameter samplerParameter,
                                              IEffect *effectRef,
                                              FrameBufferObject *frameBufferObjectRef,
                                              const IString *dir)
{
    CGstateassignment sa = cgGetFirstSamplerStateAssignment(samplerParameter);
    while (sa) {
        const CGstate s = cgGetSamplerStateAssignmentState(sa);
        if (cgGetStateType(s) == CG_TEXTURE) {
            CGparameter textureParameter = cgGetTextureStateAssignmentValue(sa);
            const char *semantic = cgGetParameterSemantic(textureParameter);
            const size_t len = strlen(semantic);
            if (VPVL2_CG_STREQ_CONST(semantic, len, "MATERIALTEXTURE")) {
                materialTexture.addParameter(textureParameter, samplerParameter, effectRef);
            }
            else if (VPVL2_CG_STREQ_CONST(semantic, len, "MATERIALSPHEREMAP")) {
                materialSphereMap.addParameter(textureParameter, samplerParameter, effectRef);
            }
            else if (VPVL2_CG_STREQ_CONST(semantic, len, "RENDERCOLORTARGET")) {
                renderColorTarget.addParameter(textureParameter,
                                               samplerParameter,
                                               effectRef,
                                               frameBufferObjectRef,
                                               0, false, false);
            }
            else if (VPVL2_CG_STREQ_CONST(semantic, len, "OFFSCREENRENDERTARGET")) {
                offscreenRenderTarget.addParameter(textureParameter,
                                                   samplerParameter,
                                                   effectRef,
                                                   frameBufferObjectRef,
                                                   0, false, false);
            }
            else {
                renderColorTarget.addParameter(textureParameter,
                                               samplerParameter,
                                               effectRef,
                                               frameBufferObjectRef,
                                               dir, true, true);
            }
            break;
        }
        sa = cgGetNextStateAssignment(sa);
    }
}

void EffectEngine::addSharedTextureParameter(CGparameter textureParameter,
                                             IEffect *effectRef,
                                             FrameBufferObject *frameBufferObjectRef,
                                             RenderColorTargetSemantic &semantic)
{
    const char *name = cgGetParameterName(textureParameter);
    IRenderContext::SharedTextureParameter parameter(cgGetParameterContext(textureParameter));
    if (!m_renderContextRef->tryGetSharedTextureParameter(name, parameter)) {
        parameter.parameter = textureParameter;
        semantic.addParameter(textureParameter, 0, effectRef, frameBufferObjectRef, 0, false, false);
        if (const RenderColorTargetSemantic::Texture *texture = semantic.findTexture(cgGetParameterName(textureParameter))) {
            /* parse semantic first and add shared parameter not to fetch unparsed semantic parameter at RenderColorTarget#addParameter */
            parameter.opaque = texture->textureRef->name();
            m_renderContextRef->addSharedTextureParameter(name, parameter);
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
                    setScriptStateFromRenderColorTargetSemantic(renderColorTarget,
                                                                value,
                                                                ScriptState::kRenderColorTarget0,
                                                                newState);
                    useRenderBuffer = newState.isRenderTargetBound;
                    renderColorTarget0DidSet = true;
                }
                else if (renderColorTarget0DidSet && command == "RenderColorTarget1") {
                    setScriptStateFromRenderColorTargetSemantic(renderColorTarget,
                                                                value,
                                                                ScriptState::kRenderColorTarget1,
                                                                newState);
                    useRenderBuffer = newState.isRenderTargetBound;
                }
                else if (renderColorTarget0DidSet && command == "RenderColorTarget2") {
                    setScriptStateFromRenderColorTargetSemantic(renderColorTarget,
                                                                value,
                                                                ScriptState::kRenderColorTarget2,
                                                                newState);
                    useRenderBuffer = newState.isRenderTargetBound;
                }
                else if (renderColorTarget0DidSet && command == "RenderColorTarget3") {
                    setScriptStateFromRenderColorTargetSemantic(renderColorTarget,
                                                                value,
                                                                ScriptState::kRenderColorTarget3,
                                                                newState);
                    useRenderBuffer = newState.isRenderTargetBound;
                }
                else if (command == "RenderDepthStencilTarget") {
                    setScriptStateFromRenderDepthStencilTargetSemantic(renderDepthStencilTarget,
                                                                       value,
                                                                       ScriptState::kRenderDepthStencilTarget,
                                                                       newState);
                    useDepthStencilBuffer = newState.isRenderTargetBound;
                }
                else if (command == "ClearSetColor") {
                    setScriptStateFromParameter(effect, value, CG_FLOAT4, ScriptState::kClearSetColor, newState);
                }
                else if (command == "ClearSetDepth") {
                    setScriptStateFromParameter(effect, value, CG_FLOAT, ScriptState::kClearSetDepth, newState);
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
        /* just draw geometry primitives */
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
    /* just check only it's technique object for technique without pass */
    if (!cgIsTechnique(technique)) {
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
        bool useScriptExternal = scriptOrder() == IEffect::kPostProcess,
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
                    setScriptStateFromRenderColorTargetSemantic(renderColorTarget,
                                                                value,
                                                                ScriptState::kRenderColorTarget0,
                                                                newState);
                    useRenderBuffer = newState.isRenderTargetBound;
                    renderColorTarget0DidSet = true;
                }
                else if (renderColorTarget0DidSet && command == "RenderColorTarget1") {
                    setScriptStateFromRenderColorTargetSemantic(renderColorTarget,
                                                                value,
                                                                ScriptState::kRenderColorTarget1,
                                                                newState);
                    useRenderBuffer = newState.isRenderTargetBound;
                }
                else if (renderColorTarget0DidSet && command == "RenderColorTarget2") {
                    setScriptStateFromRenderColorTargetSemantic(renderColorTarget,
                                                                value,
                                                                ScriptState::kRenderColorTarget2,
                                                                newState);
                    useRenderBuffer = newState.isRenderTargetBound;
                }
                else if (renderColorTarget0DidSet && command == "RenderColorTarget3") {
                    setScriptStateFromRenderColorTargetSemantic(renderColorTarget,
                                                                value,
                                                                ScriptState::kRenderColorTarget3,
                                                                newState);
                    useRenderBuffer = newState.isRenderTargetBound;
                }
                else if (command == "RenderDepthStencilTarget") {
                    setScriptStateFromRenderDepthStencilTargetSemantic(renderDepthStencilTarget,
                                                                       value,
                                                                       ScriptState::kRenderDepthStencilTarget,
                                                                       newState);
                    useDepthStencilBuffer = newState.isRenderTargetBound;
                    renderDepthStencilTargetDidSet = true;
                }
                else if (command == "ClearSetColor") {
                    setScriptStateFromParameter(effect, value, CG_FLOAT4, ScriptState::kClearSetColor, newState);
                }
                else if (command == "ClearSetDepth") {
                    setScriptStateFromParameter(effect, value, CG_FLOAT, ScriptState::kClearSetDepth, newState);
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

}
}
