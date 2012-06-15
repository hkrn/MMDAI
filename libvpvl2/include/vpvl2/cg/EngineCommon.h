/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2012  hkrn                                    */
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

#ifndef VPVL2_CG_ENGINECOMMON_H_
#define VPVL2_CG_ENGINECOMMON_H_

#include "vpvl2/Common.h"
#include "vpvl2/Scene.h"
#include "vpvl2/IBone.h"
#include "vpvl2/IModel.h"
#include "vpvl2/IMorph.h"
#include "vpvl2/IRenderDelegate.h"
#include "vpvl2/IRenderEngine.h"
#include "vpvl2/IString.h"

#include <string>
#include <sstream>

#ifdef VPVL2_LINK_QT
#include <QtOpenGL/QtOpenGL>
#include <QtOpenGL/QGLFunctions>
#endif /* VPVL_LINK_QT */

#include <cg/cg.h>
#include <cg/cgGL.h>

namespace vpvl2
{
namespace cg
{

class Util
{
public:
    static bool toBool(CGannotation annotation) {
        int nvalues = 0;
        const CGbool *values = cgGetBoolAnnotationValues(annotation, &nvalues);
        return nvalues > 0 ? values[0] == CG_TRUE : 0;
    }
    static int toInt(CGannotation annotation) {
        int nvalues = 0;
        const int *values = cgGetIntAnnotationValues(annotation, &nvalues);
        return nvalues > 0 ? values[0] : 0;
    }
    static bool isPassEquals(CGannotation annotation, const char *target) {
        if (!cgIsAnnotation(annotation))
            return true;
        const char *s = cgGetStringAnnotationValue(annotation);
        return strcmp(s, target) == 0;
    }
    static bool isIntegerParameter(CGparameter parameter) {
        return cgGetParameterType(parameter) == CG_BOOL ||
                cgGetParameterType(parameter) == CG_INT ||
                cgGetParameterType(parameter) == CG_FLOAT;
    }

private:
    Util() {}
    ~Util() {}
};

class BaseParameter
{
public:
    BaseParameter()
        : m_baseParameter(0)
    {
    }
    virtual ~BaseParameter() {
        m_baseParameter = 0;
    }

    void addParameter(CGparameter parameter) {
        connectParameter(parameter, m_baseParameter);
    }

protected:
    static void connectParameter(CGparameter sourceParameter, CGparameter &destinationParameter) {
        if (destinationParameter) {
            cgConnectParameter(sourceParameter, destinationParameter);
        }
        else {
            destinationParameter = sourceParameter;
        }
    }

    CGparameter m_baseParameter;
};

class BooleanParameter : public BaseParameter
{
public:
    BooleanParameter()
        : BaseParameter()
    {
    }
    ~BooleanParameter() {}

    void setValue(bool value) {
        if (cgIsParameter(m_baseParameter))
            cgSetParameter1i(m_baseParameter, value ? 1 : 0);
    }
};

class IntegerParameter : public BaseParameter
{
public:
    IntegerParameter()
        : BaseParameter()
    {
    }
    ~IntegerParameter() {}

    void setValue(int value) {
        if (cgIsParameter(m_baseParameter))
            cgSetParameter1i(m_baseParameter, value);
    }
};

class Float2Parameter : public BaseParameter
{
public:
    Float2Parameter()
        : BaseParameter()
    {
    }
    ~Float2Parameter() {}

    void setValue(const Vector3 &value) {
        if (cgIsParameter(m_baseParameter))
            cgSetParameter2fv(m_baseParameter, value);
    }
};

class Float4Parameter : public BaseParameter
{
public:
    Float4Parameter()
        : BaseParameter()
    {
    }
    ~Float4Parameter() {}

    void setValue(const Vector4 &value) {
        if (cgIsParameter(m_baseParameter))
            cgSetParameter4fv(m_baseParameter, value);
    }
};
class MatrixSemantic : public BaseParameter
{
public:
    MatrixSemantic(const IRenderDelegate *delegate, int flags)
        : BaseParameter(),
          m_delegate(delegate),
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
    ~MatrixSemantic() {
        m_delegate = 0;
        m_camera = 0;
        m_cameraInversed = 0;
        m_cameraTransposed = 0;
        m_cameraInverseTransposed = 0;
        m_light = 0;
        m_lightInversed = 0;
        m_lightTransposed = 0;
        m_lightInverseTransposed = 0;
    }

    void addParameter(CGparameter parameter, const char *suffix) {
        CGannotation annotation = cgGetNamedParameterAnnotation(parameter, "Object");
        if (!cgIsAnnotation(annotation)) {
            setParameter(parameter, suffix, m_cameraInversed, m_cameraTransposed, m_cameraInverseTransposed, m_camera);
        }
        else {
            const char *name = cgGetStringAnnotationValue(annotation);
            if (strcmp(name, "Camera") == 0) {
                setParameter(parameter, suffix, m_cameraInversed, m_cameraTransposed, m_cameraInverseTransposed, m_camera);
            }
            else if (strcmp(name, "Light") == 0) {
                setParameter(parameter, suffix, m_lightInversed, m_lightTransposed, m_lightInverseTransposed, m_light);
            }
        }
    }
    void setMatrices(const IModel *model, int extraCameraFlags, int extraLightFlags) {
        setMatrix(model, m_camera,
                  extraCameraFlags | IRenderDelegate::kCameraMatrix);
        setMatrix(model, m_cameraInversed,
                  extraCameraFlags | IRenderDelegate::kCameraMatrix | IRenderDelegate::kInverseMatrix);
        setMatrix(model, m_cameraTransposed,
                  extraCameraFlags | IRenderDelegate::kCameraMatrix | IRenderDelegate::kTransposeMatrix);
        setMatrix(model, m_cameraInverseTransposed,
                  extraCameraFlags | IRenderDelegate::kCameraMatrix | IRenderDelegate::kInverseMatrix | IRenderDelegate::kTransposeMatrix);
        setMatrix(model, m_light,
                  extraLightFlags | IRenderDelegate::kLightMatrix);
        setMatrix(model, m_lightInversed,
                  extraLightFlags | IRenderDelegate::kLightMatrix | IRenderDelegate::kInverseMatrix);
        setMatrix(model, m_lightTransposed,
                  extraLightFlags | IRenderDelegate::kLightMatrix | IRenderDelegate::kTransposeMatrix);
        setMatrix(model, m_lightInverseTransposed,
                  extraLightFlags | IRenderDelegate::kLightMatrix | IRenderDelegate::kInverseMatrix | IRenderDelegate::kTransposeMatrix);
    }

private:
    void setParameter(CGparameter sourceParameter,
                      const char *suffix,
                      CGparameter &inverse,
                      CGparameter &transposed,
                      CGparameter &inversetransposed,
                      CGparameter &baseParameter)
    {
        static const char kInverseTranspose[] = "INVERSETRANSPOSE";
        static const char kTranspose[] = "TRANSPOSE";
        static const char kInverse[] = "INVERSE";
        if (strncmp(suffix, kInverseTranspose, sizeof(kInverseTranspose)) == 0) {
            BaseParameter::connectParameter(sourceParameter, inversetransposed);
        }
        else if (strncmp(suffix, kTranspose, sizeof(kTranspose)) == 0) {
            BaseParameter::connectParameter(sourceParameter, transposed);
        }
        else if (strncmp(suffix, kInverse, sizeof(kInverse)) == 0) {
            BaseParameter::connectParameter(sourceParameter, inverse);
        }
        else {
            BaseParameter::connectParameter(sourceParameter, baseParameter);
        }
    }
    void setMatrix(const IModel *model, CGparameter parameter, int flags) {
        if (cgIsParameter(parameter)) {
            float matrix[16];
            m_delegate->getMatrix(matrix, model, m_flags | flags);
            cgSetMatrixParameterfr(parameter, matrix);
        }
    }

    const IRenderDelegate *m_delegate;
    CGparameter m_camera;
    CGparameter m_cameraInversed;
    CGparameter m_cameraTransposed;
    CGparameter m_cameraInverseTransposed;
    CGparameter m_light;
    CGparameter m_lightInversed;
    CGparameter m_lightTransposed;
    CGparameter m_lightInverseTransposed;
    int m_flags;
};

class MaterialSemantic : public BaseParameter
{
public:
    MaterialSemantic()
        : BaseParameter(),
          m_geometry(0),
          m_light(0)
    {
    }
    ~MaterialSemantic() {
        m_geometry = 0;
        m_light = 0;
    }

    void addParameter(CGparameter parameter) {
        const char *name = cgGetParameterSemantic(parameter);
        if (strcmp(name, "SPECULARPOWER") == 0 || strcmp(name, "EMISSIVE") == 0 || strcmp(name, "TOONCOLOR") == 0) {
            BaseParameter::connectParameter(parameter, m_geometry);
        }
        else {
            CGannotation annotation = cgGetNamedParameterAnnotation(parameter, "Object");
            if (cgIsAnnotation(annotation)) {
                const char *name = cgGetStringAnnotationValue(annotation);
                if (strcmp(name, "Geometry") == 0) {
                    BaseParameter::connectParameter(parameter, m_geometry);
                }
                else if (strcmp(name, "Light") == 0) {
                    BaseParameter::connectParameter(parameter, m_light);
                }
            }
        }
    }
    void setGeometryColor(const Vector3 &value) {
        if (cgIsParameter(m_geometry))
            cgSetParameter4fv(m_geometry, value);
    }
    void setGeometryValue(const Scalar &value) {
        if (cgIsParameter(m_geometry))
            cgSetParameter1f(m_geometry, value);
    }
    void setLightColor(const Vector3 &value) {
        if (cgIsParameter(m_light))
            cgSetParameter4fv(m_light, value);
    }
    void setLightValue(const Scalar &value) {
        if (cgIsParameter(m_light))
            cgSetParameter1f(m_light, value);
    }

private:
    CGparameter m_geometry;
    CGparameter m_light;
};

class MaterialTextureSemantic : public BaseParameter
{
public:
    MaterialTextureSemantic()
        : BaseParameter()
    {
    }
    ~MaterialTextureSemantic() {
    }

    void addParameter(CGparameter parameter) {
        BaseParameter::addParameter(parameter);
    }

    void setTexture(GLuint value) {
        if (cgIsParameter(m_baseParameter)) {
            if (value) {
                cgGLSetTextureParameter(m_baseParameter, value);
                cgSetSamplerState(m_baseParameter);
            }
            else {
                cgGLSetTextureParameter(m_baseParameter, 0);
            }
        }
    }
};

class GeometrySemantic : public BaseParameter
{
public:
    GeometrySemantic()
        : BaseParameter(),
          m_camera(0),
          m_light(0)
    {
    }
    ~GeometrySemantic() {
        m_camera = 0;
        m_light = 0;
    }

    void addParameter(CGparameter parameter) {
        CGannotation annotation = cgGetNamedParameterAnnotation(parameter, "Object");
        if (cgIsAnnotation(annotation)) {
            const char *name = cgGetStringAnnotationValue(annotation);
            if (strcmp(name, "Camera") == 0) {
                BaseParameter::connectParameter(parameter, m_camera);
            }
            else if (strcmp(name, "Light") == 0) {
                BaseParameter::connectParameter(parameter, m_light);
            }
        }
    }
    void setCameraValue(const Vector3 &value) {
        if (cgIsParameter(m_camera))
            cgSetParameter4fv(m_camera, value);
    }
    void setLightValue(const Vector3 &value) {
        if (cgIsParameter(m_light))
            cgSetParameter4fv(m_light, value);
    }

private:
    CGparameter m_camera;
    CGparameter m_light;
};

class TimeSemantic : public BaseParameter
{
public:
    TimeSemantic(const IRenderDelegate *delegate)
        : BaseParameter(),
          m_delegate(delegate),
          m_syncEnabled(0),
          m_syncDisabled(0)
    {
    }
    ~TimeSemantic() {
        m_delegate = 0;
        m_syncEnabled = 0;
        m_syncDisabled = 0;
    }

    void addParameter(CGparameter parameter) {
        CGannotation annotation = cgGetNamedParameterAnnotation(parameter, "SyncInEditMode");
        if (cgIsAnnotation(annotation)) {
            if (Util::toBool(annotation)) {
                BaseParameter::connectParameter(parameter, m_syncEnabled);
                return;
            }
        }
        BaseParameter::connectParameter(parameter, m_syncDisabled);
    }
    void setValue() {
        float value = 0;
        if (cgIsParameter(m_syncEnabled)) {
            m_delegate->getTime(value, true);
            cgSetParameter1f(m_syncEnabled, value);
        }
        if (cgIsParameter(m_syncDisabled)) {
            m_delegate->getTime(value, false);
            cgSetParameter1f(m_syncDisabled, value);
        }
    }

private:
    const IRenderDelegate *m_delegate;
    CGparameter m_syncEnabled;
    CGparameter m_syncDisabled;
};

class ControlObjectSemantic : public BaseParameter
{
public:
    ControlObjectSemantic(const IRenderDelegate *delegate)
        : BaseParameter(),
          m_delegate(delegate)
    {
    }
    ~ControlObjectSemantic() {
        m_delegate = 0;
    }

    void addParameter(CGparameter parameter) {
        if (cgIsAnnotation(cgGetNamedParameterAnnotation(parameter, "name")))
            m_parameters.add(parameter);
    }
    void update(const IModel *self) {
        const int nparameters = m_parameters.count();
        for (int i = 0; i < nparameters; i++) {
            CGparameter parameter = m_parameters[i];
            CGannotation nameAnnotation = cgGetNamedParameterAnnotation(parameter, "name");
            const char *name = cgGetStringAnnotationValue(nameAnnotation);
            if (strcmp(name, "(self)") == 0) {
                setParameter(self, parameter);
            }
            else if (strcmp(name, "(OffscreenOwner)") == 0) {
                // TODO
            }
            else {
                IModel *model = m_delegate->findModel(name);
                setParameter(model, parameter);
            }
        }
    }

private:
    void setParameter(const IModel *model, CGparameter parameter) {
        float matrix4x4[16];
        Transform::getIdentity().getOpenGLMatrix(matrix4x4);
        CGtype type = cgGetParameterType(parameter);
        if (model) {
            CGannotation itemAnnotation = cgGetNamedParameterAnnotation(parameter, "item");
            if (cgIsAnnotation(itemAnnotation)) {
                const char *item = cgGetStringAnnotationValue(itemAnnotation);
                IModel::Type type = model->type();
                if (type == IModel::kPMD || type == IModel::kPMX) {
                    const IString *s = m_delegate->toUnicode(reinterpret_cast<const uint8_t *>(item));
                    IBone *bone = model->findBone(s);
                    IMorph *morph = model->findMorph(s);
                    delete s;
                    if (bone) {
                        switch (type) {
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
                    else if (morph && type == CG_FLOAT) {
                        cgSetParameter1f(parameter, morph->weight());
                    }
                }
                else {
                    const Vector3 &position = model->position();
                    const Quaternion &rotation = model->rotation();
                    if (strcmp(item, "X") == 0 && type == CG_FLOAT) {
                        cgSetParameter1f(parameter, position.x());
                    }
                    else if (strcmp(item, "Y") == 0 && type == CG_FLOAT) {
                        cgSetParameter1f(parameter, position.y());
                    }
                    else if (strcmp(item, "Z") == 0 && type == CG_FLOAT) {
                        cgSetParameter1f(parameter, position.z());
                    }
                    else if (strcmp(item, "XYZ") == 0 && type == CG_FLOAT3) {
                        cgSetParameter3fv(parameter, position);
                    }
                    else if (strcmp(item, "Rx") == 0 && type == CG_FLOAT) {
                        cgSetParameter1f(parameter, btDegrees(rotation.x()));
                    }
                    else if (strcmp(item, "Ry") == 0 && type == CG_FLOAT) {
                        cgSetParameter1f(parameter, btDegrees(rotation.y()));
                    }
                    else if (strcmp(item, "Rz") == 0 && type == CG_FLOAT) {
                        cgSetParameter1f(parameter, btDegrees(rotation.z()));
                    }
                    else if (strcmp(item, "Rxyz") == 0 && type == CG_FLOAT3) {
                        const Vector3 rotationDegree(btDegrees(rotation.x()), btDegrees(rotation.y()), btDegrees(rotation.z()));
                        cgSetParameter3fv(parameter, rotationDegree);
                    }
                    else if (strcmp(item, "Sr") == 0 && type == CG_FLOAT) {
                        cgSetParameter1f(parameter, model->scaleFactor());
                    }
                    else if (strcmp(item, "Tr") == 0 && type == CG_FLOAT) {
                        cgSetParameter1f(parameter, model->opacity());
                    }
                }
            }
            else {
                switch (type) {
                case CG_BOOL:
                    cgSetParameter1i(parameter, model->isVisible());
                    break;
                case CG_FLOAT:
                    cgSetParameter1i(parameter, model->scaleFactor());
                    break;
                case CG_FLOAT3:
                case CG_FLOAT4:
                    cgSetParameter4fv(parameter, model->position());
                    break;
                case CG_FLOAT4x4:
                    cgSetMatrixParameterfr(parameter, matrix4x4);
                    break;
                default:
                    break;
                }
            }
        }
        else {
            CGtype type = cgGetParameterType(parameter);
            switch (type) {
            case CG_BOOL:
                cgSetParameter1i(parameter, 0);
                break;
            case CG_FLOAT:
                cgSetParameter1i(parameter, 1);
                break;
            case CG_FLOAT3:
            case CG_FLOAT4:
                cgSetParameter4f(parameter, 0, 0, 0, 1);
                break;
            case CG_FLOAT4x4:
                cgSetMatrixParameterfr(parameter, matrix4x4);
                break;
            default:
                break;
            }
        }
    }

    const IRenderDelegate *m_delegate;
    Array<CGparameter> m_parameters;
};

class RenderColorTargetSemantic : public BaseParameter
{
public:
    RenderColorTargetSemantic(const IRenderDelegate *delegate)
        : BaseParameter(),
          m_delegate(delegate)
    {
    }
    ~RenderColorTargetSemantic() {
        const int ntextures = m_textures.count();
        glDeleteTextures(ntextures, &m_textures[0]);
    }

    void addParameter(CGparameter parameter) {
        CGannotation type = cgGetNamedParameterAnnotation(parameter, "ResourceType");
        GLuint texture = 0;
        if (cgIsAnnotation(type)) {
            const char *typeName = cgGetStringAnnotationValue(type);
            if (strcmp(typeName, "texture3D") == 0) {
                texture = generateTexture3D0(parameter);
            }
            else if (strcmp(typeName, "texture2D") == 0) {
                texture = generateTexture2D0(parameter);
            }
        }
        else {
            texture = generateTexture2D0(parameter);
        }
        m_parameters.add(parameter);
        m_name2texture.insert(cgGetParameterName(parameter), texture);
    }
    GLuint findTexture(const char *name) const {
        GLuint *texture = const_cast<GLuint *>(m_name2texture.find(name));
        return texture ? *texture : 0;
    }

protected:
    bool isMimapEnabled(const CGparameter parameter) const {
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
    virtual void generateTexture2D(const CGparameter parameter, GLuint texture, int width, int height) {
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        if (isMimapEnabled(parameter))
            glGenerateMipmap(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    virtual void generateTexture3D(const CGparameter parameter, GLuint texture, int width, int height, int depth) {
        glBindTexture(GL_TEXTURE_3D, texture);
        glTexImage3D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, depth, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        if (isMimapEnabled(parameter))
            glGenerateMipmap(GL_TEXTURE_3D);
        glBindTexture(GL_TEXTURE_3D, 0);
    }

private:
    GLuint generateTexture2D0(CGparameter parameter) {
        int width, height;
        getSize2(parameter, width, height);
        GLuint texture;
        glGenTextures(1, &texture);
        m_textures.add(texture);
        generateTexture2D(parameter, texture, width, height);
        return texture;
    }
    GLuint generateTexture3D0(CGparameter parameter) {
        int width, height, depth;
        getSize3(parameter, width, height, depth);
        GLuint texture;
        glGenTextures(1, &texture);
        m_textures.add(texture);
        generateTexture3D(parameter, texture, width, height, depth);
        return texture;
    }
    void getSize2(CGparameter parameter, int &width, int &height) {
        CGannotation viewportRatioAnnotation = cgGetNamedParameterAnnotation(parameter, "ViewportRatio");
        int nvalues = 0;
        if (cgIsAnnotation(viewportRatioAnnotation)) {
            const float *values = cgGetFloatAnnotationValues(viewportRatioAnnotation, &nvalues);
            if (nvalues == 2) {
                Vector3 viewport;
                m_delegate->getViewport(viewport);
                float widthRatio = values[0];
                float heightRatio = values[1];
                width = btMax(1, int(viewport.x() * widthRatio));
                height = btMax(1, int(viewport.y() * heightRatio));
                return;
            }
        }
        CGannotation dimensionsAnnotation = cgGetNamedParameterAnnotation(parameter, "Dimensions");
        if (cgIsAnnotation(dimensionsAnnotation)) {
            const int *values = cgGetIntAnnotationValues(viewportRatioAnnotation, &nvalues);
            if (nvalues == 2) {
                width = btMax(1,values[0]);
                height = btMax(1,values[1]);
                return;
            }
        }
        CGannotation widthAnnotation = cgGetNamedParameterAnnotation(parameter, "Width");
        CGannotation heightAnnotation = cgGetNamedParameterAnnotation(parameter, "Height");
        if (cgIsAnnotation(widthAnnotation) && cgIsAnnotation(heightAnnotation)) {
            width = btMax(1, Util::toInt(widthAnnotation));
            height = btMax(1, Util::toInt(heightAnnotation));
            return;
        }
        Vector3 viewport;
        m_delegate->getViewport(viewport);
        width = btMax(1, int(viewport.x()));
        height = btMax(1, int(viewport.y()));
    }
    void getSize3(CGparameter parameter, int &width, int &height, int &depth) {
        int nvalues = 0;
        CGannotation dimensionsAnnotation = cgGetNamedParameterAnnotation(parameter, "Dimensions");
        if (cgIsAnnotation(dimensionsAnnotation)) {
            const int *values = cgGetIntAnnotationValues(dimensionsAnnotation, &nvalues);
            if (nvalues == 3) {
                width = btMax(1,values[0]);
                height = btMax(1,values[1]);
                depth = btMax(1,values[2]);
                return;
            }
        }
        CGannotation widthAnnotation = cgGetNamedParameterAnnotation(parameter, "Width");
        CGannotation heightAnnotation = cgGetNamedParameterAnnotation(parameter, "Height");
        CGannotation depthAnnotation = cgGetNamedParameterAnnotation(parameter, "Depth");
        if (cgIsAnnotation(widthAnnotation) && cgIsAnnotation(heightAnnotation) && cgIsAnnotation(depthAnnotation)) {
            width = btMax(1, Util::toInt(widthAnnotation));
            height = btMax(1, Util::toInt(heightAnnotation));
            depth = btMax(1, Util::toInt(depthAnnotation));
            return;
        }
        Vector3 viewport;
        m_delegate->getViewport(viewport);
        width = btMax(1, int(viewport.x()));
        height = btMax(1, int(viewport.y()));
        depth = 24;
    }

    const IRenderDelegate *m_delegate;
    Array<CGparameter> m_parameters;
    Array<GLuint> m_textures;
    Hash<HashString, GLuint> m_name2texture;
};

class RenderDepthStencilTargetSemantic : public RenderColorTargetSemantic
{
public:
    RenderDepthStencilTargetSemantic(const IRenderDelegate *delegate)
        : RenderColorTargetSemantic(delegate)
    {
    }
    ~RenderDepthStencilTargetSemantic() {
        const int nRenderBuffers = m_renderBuffers.count();
        glDeleteRenderbuffers(nRenderBuffers, &m_renderBuffers[0]);
    }

    GLuint findRenderBuffer(const char *name) const {
        GLuint *renderBuffer = const_cast<GLuint *>(m_name2buffer.find(name));
        return renderBuffer ? *renderBuffer : 0;
    }

protected:
    void generateTexture2D(const CGparameter parameter, GLuint /* texture */, int width, int height) {
        GLuint renderBuffer;
        glGenRenderbuffers(1, &renderBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        m_renderBuffers.add(renderBuffer);
        m_name2buffer.insert(cgGetParameterName(parameter), renderBuffer);
    }

private:
    Array<GLuint> m_renderBuffers;
    Hash<HashString, GLuint> m_name2buffer;
};

class OffscreenRenderTargetSemantic : public RenderColorTargetSemantic
{
public:
    CGannotation clearColor;
    CGannotation clearDepth;
    CGannotation antiAlias;
    CGannotation description;
    CGannotation defaultEffect;
    OffscreenRenderTargetSemantic(const IRenderDelegate *delegate)
        : RenderColorTargetSemantic(delegate),
          clearColor(0),
          clearDepth(0),
          antiAlias(0),
          description(0),
          defaultEffect(0)
    {
    }
    ~OffscreenRenderTargetSemantic() {
    }

    void addParameter(CGparameter parameter) {
        RenderColorTargetSemantic::addParameter(parameter);
        clearColor = cgGetNamedParameterAnnotation(parameter, "ClearColor");
        clearDepth = cgGetNamedParameterAnnotation(parameter, "ClearDepth");
        antiAlias = cgGetNamedParameterAnnotation(parameter, "AntiAlias");
        description = cgGetNamedParameterAnnotation(parameter, "Description");
        defaultEffect = cgGetNamedParameterAnnotation(parameter, "DefaultEffect");
    }
};

class AnimatedTextureSemantic : public BaseParameter
{
public:
    CGannotation resourceName;
    CGannotation offset;
    CGannotation speed;
    CGannotation seekVariable;
    AnimatedTextureSemantic()
        : resourceName(0),
          offset(0),
          speed(0),
          seekVariable(0)
    {
    }
    ~AnimatedTextureSemantic() {
    }

    void addParameter(CGparameter parameter) {
        BaseParameter::addParameter(parameter);
        resourceName = cgGetNamedParameterAnnotation(parameter, "ResourceName");
        offset = cgGetNamedParameterAnnotation(parameter, "Offset");
        speed = cgGetNamedParameterAnnotation(parameter, "Speed");
        seekVariable = cgGetNamedParameterAnnotation(parameter, "SeekVariable");
    }
};

class TextureValueSemantic : public BaseParameter
{
public:
    CGannotation textureName;
    TextureValueSemantic()
        : textureName(0)
    {
    }
    ~TextureValueSemantic() {
    }

    void addParameter(CGparameter parameter) {
        BaseParameter::addParameter(parameter);
        textureName = cgGetNamedParameterAnnotation(parameter, "TextureName");
    }
};

class Effect {
public:
    typedef btAlignedObjectArray<CGtechnique> Techniques;
    typedef btAlignedObjectArray<CGpass> Passes;
    typedef Hash<HashPtr, Passes> TechniquePasses;
    enum ScriptOutputType {
        kColor
    };
    enum ScriptClassType {
        kObject,
        kScene,
        kSceneObject
    };
    enum ScriptOrderType {
        kPreProcess,
        kStandard,
        kPostProcess
    };

    Effect(const IRenderDelegate *delegate)
        : effect(0),
          world(delegate, IRenderDelegate::kWorldMatrix),
          view(delegate, IRenderDelegate::kViewMatrix),
          projection(delegate, IRenderDelegate::kProjectionMatrix),
          worldView(delegate, IRenderDelegate::kWorldMatrix | IRenderDelegate::kViewMatrix),
          viewProjection(delegate, IRenderDelegate::kViewMatrix | IRenderDelegate::kProjectionMatrix),
          worldViewProjection(delegate, IRenderDelegate::kWorldMatrix | IRenderDelegate::kViewMatrix | IRenderDelegate::kProjectionMatrix),
          time(delegate),
          elapsedTime(delegate),
          controlObject(delegate),
          renderColorTarget(delegate),
          renderDepthStencilTarget(delegate),
          offscreenRenderTarget(delegate),
          index(0),
          m_delegate(delegate),
          m_scriptOutput(kColor),
          m_scriptClass(kObject),
          m_scriptOrder(kStandard)
    {
    }
    ~Effect()
    {
        const int nTechniqueFrameBuffers = m_techniqueFrameBuffers.size();
        for (int i = 0; i < nTechniqueFrameBuffers; i++) {
            GLuint *frameBufferObject = m_techniqueFrameBuffers.getAtIndex(i);
            glDeleteFramebuffers(1, frameBufferObject);
        }
        if (cgIsEffect(effect))
            cgDestroyEffect(effect);
        effect = 0;
        m_delegate = 0;
    }

    bool attachEffect(CGeffect value) {
        static const char kWorldSemantic[] = "WORLD";
        static const char kViewSemantic[] = "VIEW";
        static const char kProjectionSemantic[] = "PROJECTION";
        static const char kWorldViewSemantic[] = "WORLDVIEW";
        static const char kViewProjectionSemantic[] = "VIEWPROJECTION";
        static const char kWorldViewProjectionSemantic[] = "WORLDVIEWPROJECTION";
        CGparameter parameter = cgGetFirstEffectParameter(value);
        while (parameter) {
            const char *semantic = cgGetParameterSemantic(parameter);
            if (strncmp(semantic, kWorldSemantic, sizeof(kWorldSemantic)) == 0) {
                world.addParameter(parameter, semantic + sizeof(kWorldSemantic));
            }
            else if (strncmp(semantic, kViewSemantic, sizeof(kViewSemantic)) == 0) {
                view.addParameter(parameter, semantic + sizeof(kViewSemantic));
            }
            else if (strncmp(semantic, kProjectionSemantic, sizeof(kProjectionSemantic)) == 0) {
                projection.addParameter(parameter, semantic + sizeof(kProjectionSemantic));
            }
            else if (strncmp(semantic, kWorldViewSemantic, sizeof(kWorldViewSemantic)) == 0) {
                worldView.addParameter(parameter, semantic + sizeof(kWorldViewSemantic));
            }
            else if (strncmp(semantic, kViewProjectionSemantic, sizeof(kViewProjectionSemantic)) == 0) {
                viewProjection.addParameter(parameter, semantic + sizeof(kViewProjectionSemantic));
            }
            else if (strncmp(semantic, kWorldViewProjectionSemantic, sizeof(kWorldViewProjectionSemantic)) == 0) {
                worldViewProjection.addParameter(parameter, semantic + sizeof(kWorldViewProjectionSemantic));
            }
            else if (strcmp(semantic, "DIFFUSE") == 0) {
                diffuse.addParameter(parameter);
            }
            else if (strcmp(semantic, "AMBIENT") == 0) {
                ambient.addParameter(parameter);
            }
            else if (strcmp(semantic, "EMISSIVE") == 0) {
                emissive.addParameter(parameter);
            }
            else if (strcmp(semantic, "SPECULAR") == 0) {
                specular.addParameter(parameter);
            }
            else if (strcmp(semantic, "SPECULARPOWER") == 0) {
                specularPower.addParameter(parameter);
            }
            else if (strcmp(semantic, "TOONCOLOR") == 0) {
                toonColor.addParameter(parameter);
            }
            else if (strcmp(semantic, "EDGECOLOR") == 0) {
                edgeColor.addParameter(parameter);
            }
            else if (strcmp(semantic, "_POSITION") == 0) {
                position.addParameter(parameter);
            }
            else if (strcmp(semantic, "_DIRECTION") == 0) {
                direction.addParameter(parameter);
            }
            else if (strcmp(semantic, "VIEWPORTPIXELSIZE") == 0) {
                viewportPixelSize.addParameter(parameter);
            }
            else if (strcmp(semantic, "MOUSEPOSITION") == 0) {
                mousePosition.addParameter(parameter);
            }
            else if (strcmp(semantic, "LEFTMOUSEDOWN") == 0) {
                leftMouseDown.addParameter(parameter);
            }
            else if (strcmp(semantic, "MIDDLEMOUSEDOWN") == 0) {
                middleMouseDown.addParameter(parameter);
            }
            else if (strcmp(semantic, "RIGHTMOUSEDOWN") == 0) {
                rightMouseDown.addParameter(parameter);
            }
            else if (strcmp(semantic, "CONTROLOBJECT") == 0) {
                controlObject.addParameter(parameter);
            }
            else if (strcmp(semantic, "ANIMATEDTEXTURE") == 0) {
                animatedTexture.addParameter(parameter);
            }
            else if (strcmp(semantic, "TEXTUREVALUE") == 0) {
                textureValue.addParameter(parameter);
            }
            else if (strcmp(semantic, "RENDERDEPTHSTENCILTARGET") == 0) {
                renderDepthStencilTarget.addParameter(parameter);
            }
            else if (strcmp(semantic, "STANDARDSGLOBAL") == 0) {
                setStandardsGlobal(parameter);
            }
            else if (strcmp(semantic, "_INDEX") == 0) {
            }
            else {
                const char *name = cgGetParameterName(parameter);
                if (strcmp(name, "parthf") == 0) {
                    parthf.addParameter(parameter);
                }
                else if (strcmp(name, "spadd") == 0) {
                    spadd.addParameter(parameter);
                }
                else if (strcmp(name, "transp") == 0) {
                    transp.addParameter(parameter);
                }
                else if (strcmp(name, "use_texture") == 0) {
                    useTexture.addParameter(parameter);
                }
                else if (strcmp(name,"use_spheremap") == 0) {
                    useSpheremap.addParameter(parameter);
                }
                else if (strcmp(name, "use_toon") == 0) {
                    useToon.addParameter(parameter);
                }
                else if (strcmp(name, "opadd") == 0) {
                    opadd.addParameter(parameter);
                }
                else if (strcmp(name, "VertexCount") == 0) {
                    vertexCount.addParameter(parameter);
                }
                else if (strcmp(name, "SubsetCount") == 0) {
                    subsetCount.addParameter(parameter);
                }
                else {
                    setTextureParameters(parameter);
                }
            }
            parameter = cgGetNextParameter(parameter);
        }
        effect = value;
        CGtechnique technique = cgGetFirstTechnique(value);
        while (technique) {
            addTechniquePasses(technique);
            technique = cgGetNextTechnique(technique);
        }
        return true;
    }
    CGtechnique findTechnique(const char *pass,
                              int offset,
                              int nmaterials,
                              bool hasTexture,
                              bool hasSphereMap,
                              bool useToon)
    {
        CGtechnique technique = 0;
        const int ntechniques = m_techniques.size();
        for (int i = 0; i < ntechniques; i++) {
            technique = m_techniques[i];
            if (testTechnique(technique, pass, offset, nmaterials, hasTexture, hasSphereMap, useToon))
                break;
        }
        return technique;
    }
    void executeTechniquePasses(const CGtechnique technique, GLsizei count, GLenum type, const GLvoid *ptr) {
        if (cgIsTechnique(technique)) {
            const ScriptStates *tss = m_techniqueScriptStates.find(technique);
            executeScriptStates(tss, count, type, ptr);
            const Passes *passes = m_techniquePasses.find(technique);
            if (passes) {
                const int npasses = passes->size();
                for (int i = 0; i < npasses; i++) {
                    CGpass pass = passes->at(i);
                    const ScriptStates *pss = m_passScriptStates.find(pass);
                    executeScriptStates(pss, count, type, ptr);
                    executePass(pass, count, type, ptr);
                }
            }
        }
    }
    void setModelMatrixParameters(const IModel *model,
                                  int extraCameraFlags = 0,
                                  int extraLightFlags = 0)
    {
        world.setMatrices(model, extraCameraFlags, extraLightFlags);
        view.setMatrices(model, extraCameraFlags, extraLightFlags);
        projection.setMatrices(model, extraCameraFlags, extraLightFlags);
        worldView.setMatrices(model, extraCameraFlags, extraLightFlags);
        viewProjection.setMatrices(model, extraCameraFlags, extraLightFlags);
        worldViewProjection.setMatrices(model, extraCameraFlags, extraLightFlags);
    }
    void setZeroGeometryParameters(const IModel *model) {
        edgeColor.setGeometryColor(model->edgeColor());
        toonColor.setGeometryColor(kZeroC);
        ambient.setGeometryColor(kZeroC);
        diffuse.setGeometryColor(kZeroC);
        specular.setGeometryColor(kZeroC);
        specularPower.setGeometryValue(0);
        materialTexture.setTexture(0);
        materialSphereMap.setTexture(0);
        spadd.setValue(false);
        useTexture.setValue(false);
    }
    void updateModelGeometryParameters(const Scene *scene, const IModel *model) {
        const Scene::ILight *light = scene->light();
        const Vector3 &lightColor = light->color();
        if (model->type() == IModel::kAsset) {
            const Vector3 &ac = Vector3(0.7, 0.7, 0.7) - lightColor;
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
        const Scene::ICamera *camera = scene->camera();
        const Vector3 &cameraPosition = camera->modelViewTransform().getOrigin();
        position.setCameraValue(cameraPosition);
        direction.setCameraValue((cameraPosition - camera->position()).normalized());
        controlObject.update(model);
    }
    void updateViewportParameters() {
        Vector3 viewport;
        m_delegate->getViewport(viewport);
        viewportPixelSize.setValue(viewport);
        Vector4 position;
        m_delegate->getMousePosition(position, IRenderDelegate::kMouseCursorPosition);
        mousePosition.setValue(position);
        m_delegate->getMousePosition(position, IRenderDelegate::kMouseLeftPressPosition);
        leftMouseDown.setValue(position);
        m_delegate->getMousePosition(position, IRenderDelegate::kMouseMiddlePressPosition);
        middleMouseDown.setValue(position);
        m_delegate->getMousePosition(position, IRenderDelegate::kMouseRightPressPosition);
        rightMouseDown.setValue(position);
        time.setValue();
        elapsedTime.setValue();
    }

    bool isAttached() const { return cgIsEffect(effect) == CG_TRUE; }
    ScriptOutputType scriptOutput() const { return m_scriptOutput; }
    ScriptClassType scriptClass() const { return m_scriptClass; }
    ScriptOrderType scriptOrder() const { return m_scriptOrder; }

    CGeffect effect;
    MatrixSemantic world;
    MatrixSemantic view;
    MatrixSemantic projection;
    MatrixSemantic worldView;
    MatrixSemantic viewProjection;
    MatrixSemantic worldViewProjection;
    MaterialSemantic diffuse;
    MaterialSemantic ambient;
    MaterialSemantic emissive;
    MaterialSemantic specular;
    MaterialSemantic specularPower;
    MaterialSemantic toonColor;
    MaterialSemantic edgeColor;
    GeometrySemantic position;
    GeometrySemantic direction;
    MaterialTextureSemantic materialTexture;
    MaterialTextureSemantic materialSphereMap;
    Float2Parameter viewportPixelSize;
    TimeSemantic time;
    TimeSemantic elapsedTime;
    Float2Parameter mousePosition;
    Float4Parameter leftMouseDown;
    Float4Parameter middleMouseDown;
    Float4Parameter rightMouseDown;
    ControlObjectSemantic controlObject;
    RenderColorTargetSemantic renderColorTarget;
    RenderDepthStencilTargetSemantic renderDepthStencilTarget;
    AnimatedTextureSemantic animatedTexture;
    OffscreenRenderTargetSemantic offscreenRenderTarget;
    TextureValueSemantic textureValue;
    /* special parameters */
    BooleanParameter parthf;
    BooleanParameter spadd;
    BooleanParameter transp;
    BooleanParameter useTexture;
    BooleanParameter useSpheremap;
    BooleanParameter useToon;
    BooleanParameter opadd;
    IntegerParameter vertexCount;
    IntegerParameter subsetCount;
    CGparameter index;

private:
    struct ScriptState {
        ScriptState()
            : type(kUnknown),
              parameter(0),
              pass(0),
              frameBufferObject(0),
              texture(0),
              depthBuffer(0),
              stencilBuffer(0),
              enterLoop(false)
        {
        }
        ~ScriptState() {
            reset();
        }

        void reset() {
            type = kUnknown;
            parameter = 0;
            pass = 0;
            texture = 0;
            frameBufferObject = 0;
            depthBuffer = 0;
            stencilBuffer = 0;
            enterLoop = false;
        }
        enum Type {
            kUnknown,
            kRenderColorTarget0,
            kRenderColorTarget1,
            kRenderColorTarget2,
            kRenderColorTarget3,
            kRenderDepthStencilTarget,
            kClearSetColor,
            kClearSetDepth,
            kClearColor,
            kClearDepth,
            kScriptExternal,
            kPass,
            kLoopByCount,
            kLoopEnd,
            kLoopGetIndex,
            kDrawGeometry,
            kDrawBuffer
        } type;
        CGparameter parameter;
        CGpass pass;
        GLuint frameBufferObject;
        GLuint texture;
        GLuint depthBuffer;
        GLuint stencilBuffer;
        bool enterLoop;
    };
    typedef btAlignedObjectArray<ScriptState> ScriptStates;

    static bool testTechnique(const CGtechnique technique,
                              const char *pass,
                              int offset,
                              int nmaterials,
                              bool hasTexture,
                              bool hasSphereMap,
                              bool useToon)
    {
        if (cgValidateTechnique(technique) == CG_FALSE)
            return false;
        int ok = 1;
        const CGannotation passAnnotation = cgGetNamedTechniqueAnnotation(technique, "MMDPass");
        ok &= Util::isPassEquals(passAnnotation, pass);
        const CGannotation subsetAnnotation = cgGetNamedTechniqueAnnotation(technique, "Subset");
        ok &= containsSubset(subsetAnnotation, offset, nmaterials);
        const CGannotation useTextureAnnotation = cgGetNamedTechniqueAnnotation(technique, "UseTexture");
        ok &= (!cgIsAnnotation(useTextureAnnotation) || Util::toBool(useTextureAnnotation) == hasTexture);
        const CGannotation useSphereMapAnnotation = cgGetNamedTechniqueAnnotation(technique, "UseSphereMap");
        ok &= (!cgIsAnnotation(useSphereMapAnnotation) || Util::toBool(useSphereMapAnnotation) == hasSphereMap);
        const CGannotation useToonAnnotation = cgGetNamedTechniqueAnnotation(technique, "UseToon");
        ok &= (!cgIsAnnotation(useToonAnnotation) || Util::toBool(useToonAnnotation) == useToon);
        return ok == 1;
    }
    static bool containsSubset(const CGannotation annotation, int subset, int nmaterials) {
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
    static void setStateFromRenderColorTargetSemantic(const RenderColorTargetSemantic &semantic,
                                                      const std::string &value,
                                                      GLuint frameBufferObject,
                                                      ScriptState::Type type,
                                                      ScriptState &state)
    {
        state.type = type;
        if (!value.empty()) {
            GLuint texture = semantic.findTexture(value.c_str());
            state.texture = texture;
            state.frameBufferObject = frameBufferObject;
        }
    }
    static void setStateFromRenderDepthStencilTargetSemantic(const RenderDepthStencilTargetSemantic &semantic,
                                                             const std::string &value,
                                                             GLuint frameBufferObject,
                                                             ScriptState::Type type,
                                                             ScriptState &state)
    {
        state.type = type;
        if (!value.empty()) {
            GLuint renderBuffer = semantic.findRenderBuffer(value.c_str());
            state.depthBuffer = renderBuffer;
            state.stencilBuffer = renderBuffer;
            state.frameBufferObject = frameBufferObject;
        }
    }
    static void setStateFromParameter(const CGeffect effect,
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
    static void executePass(CGpass pass, GLsizei count, GLenum type, const GLvoid *ptr) {
        if (cgIsPass(pass)) {
            cgSetPassState(pass);
            glDrawElements(GL_TRIANGLES, count, type, ptr);
            cgResetPassState(pass);
        }
    }
    static void setFrameBufferTexture(const GLenum attachment, const ScriptState &state) {
        GLuint texture = state.texture;
        glBindFramebuffer(GL_FRAMEBUFFER, state.frameBufferObject);
        if (texture > 0) {
            glBindTexture(GL_TEXTURE_2D, texture);
            glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        else {
            glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, 0, 0);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    void executeScriptStates(const ScriptStates *states, GLsizei count, GLenum type, const GLvoid *ptr) {
        if (states) {
            const int nstates = states->size();
            int stateIndex = 0, nloop = 0, backStateIndex = 0;
            GLuint frameBufferObject, depthBuffer, stencilBuffer, colorBuffers[] = {
                GL_COLOR_ATTACHMENT0,
                GL_COLOR_ATTACHMENT1,
                GL_COLOR_ATTACHMENT2,
                GL_COLOR_ATTACHMENT3,
            };
            static const size_t nbuffers = sizeof(colorBuffers) / sizeof(colorBuffers[0]);
            Vector4 v4;
            while (stateIndex < nstates) {
                const ScriptState &state = states->at(stateIndex);
                switch (state.type) {
                case ScriptState::kClearColor:
                    frameBufferObject = state.frameBufferObject;
                    if (frameBufferObject) {
                        glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);
                        glClear(GL_COLOR_BUFFER_BIT);
                        glBindFramebuffer(GL_FRAMEBUFFER, 0);
                    }
                    else {
                        glClear(GL_COLOR_BUFFER_BIT);
                    }
                    break;
                case ScriptState::kClearDepth:
                    frameBufferObject = state.frameBufferObject;
                    if (frameBufferObject) {
                        glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);
                        glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                        glBindFramebuffer(GL_FRAMEBUFFER, 0);
                    }
                    else {
                        glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
                    }
                    break;
                case ScriptState::kClearSetColor:
                    cgGLGetParameter4f(state.parameter, v4);
                    glClearColor(v4.x(), v4.y(), v4.z(), v4.w());
                    break;
                case ScriptState::kClearSetDepth:
                    cgGLGetParameter1f(state.parameter, v4);
                    glClearDepth(v4.x());
                    break;
                case ScriptState::kPass:
                    executePass(state.pass, count, type, ptr);
                    break;
                case ScriptState::kLoopByCount:
                    cgGLGetParameter1f(state.parameter, v4);
                    backStateIndex = stateIndex + 1;
                    nloop = int(v4.x());
                    break;
                case ScriptState::kLoopEnd:
                    if (--nloop >= 0) {
                        stateIndex = backStateIndex;
                        continue;
                    }
                    break;
                case ScriptState::kLoopGetIndex:
                    cgGLGetParameter1f(state.parameter, v4);
                    nloop = int(v4.x());
                    break;
                case ScriptState::kRenderColorTarget0:
                    setFrameBufferTexture(GL_COLOR_ATTACHMENT0, state);
                    break;
                case ScriptState::kRenderColorTarget1:
                    setFrameBufferTexture(GL_COLOR_ATTACHMENT1, state);
                    break;
                case ScriptState::kRenderColorTarget2:
                    setFrameBufferTexture(GL_COLOR_ATTACHMENT2, state);
                    break;
                case ScriptState::kRenderColorTarget3:
                    setFrameBufferTexture(GL_COLOR_ATTACHMENT3, state);
                    break;
                case ScriptState::kRenderDepthStencilTarget:
                    glBindFramebuffer(GL_FRAMEBUFFER, state.frameBufferObject);
                    depthBuffer = state.depthBuffer;
                    stencilBuffer = state.stencilBuffer;
                    if (depthBuffer > 0 && stencilBuffer > 0) {
                        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
                        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, stencilBuffer);
                    }
                    else {
                        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
                        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, 0);
                    }
                    glBindFramebuffer(GL_FRAMEBUFFER, 0);
                    break;
                case ScriptState::kDrawBuffer:
                    glBindFramebuffer(GL_FRAMEBUFFER, state.frameBufferObject);
                    glDrawBuffers(nbuffers, colorBuffers);
                    executePass(state.pass, count, type, ptr);
                    glBindFramebuffer(GL_FRAMEBUFFER, 0);
                    break;
                case ScriptState::kDrawGeometry:
                case ScriptState::kScriptExternal:
                case ScriptState::kUnknown:
                default:
                    break;
                }
                stateIndex++;
            }
        }
    }
    void addTechniquePasses(CGtechnique technique) {
        GLuint frameBufferObject;
        if (parseTechniqueScript(technique, frameBufferObject)) {
            Passes passes;
            CGpass pass = cgGetFirstPass(technique);
            while (pass) {
                if (parsePassScript(pass, frameBufferObject))
                    passes.push_back(pass);
                pass = cgGetNextPass(pass);
            }
            m_techniquePasses.insert(technique, passes);
            m_techniques.push_back(technique);
        }
    }
    void setStandardsGlobal(const CGparameter parameter) {
        float version;
        cgGLGetParameter1f(parameter, &version);
        if (!btFuzzyZero(version - 0.8f))
            return;
        const CGannotation scriptClassAnnotation = cgGetNamedParameterAnnotation(parameter, "ScriptClass");
        if (cgIsAnnotation(scriptClassAnnotation)) {
            const char *value = cgGetStringAnnotationValue(scriptClassAnnotation);
            if (strcmp(value, "object") == 0) {
                m_scriptClass = kObject;
            }
            else if (strcmp(value, "scene") == 0) {
                m_scriptClass = kScene;
            }
            else if (strcmp(value, "sceneobject") == 0) {
                m_scriptClass = kSceneObject;
            }
        }
        const CGannotation scriptOrderAnnotation = cgGetNamedParameterAnnotation(parameter, "ScriptOrder");
        if (cgIsAnnotation(scriptOrderAnnotation)) {
            const char *value = cgGetStringAnnotationValue(scriptOrderAnnotation);
            if (strcmp(value, "standard") == 0) {
                m_scriptOrder = kStandard;
            }
            else if (strcmp(value, "preprocess") == 0) {
                m_scriptOrder = kPreProcess;
            }
            else if (strcmp(value, "postprocess") == 0) {
                m_scriptOrder = kPostProcess;
            }
        }
        const CGannotation scriptAnnotation = cgGetNamedParameterAnnotation(parameter, "Script");
        if (cgIsAnnotation(scriptAnnotation)) {
            const char *value = cgGetStringAnnotationValue(scriptAnnotation);
            static const char kMultipleTechniques[] = "Technique=Technique?";
            static const char kSingleTechnique[] = "Technique=";
            m_techniques.clear();
            if (strncmp(value, kMultipleTechniques, sizeof(kMultipleTechniques)) == 0) {
                const std::string s(value + sizeof(kMultipleTechniques));
                std::istringstream stream(s);
                std::string segment;
                while (std::getline(stream, segment, ':')) {
                    CGtechnique technique = cgGetNamedTechnique(effect, segment.c_str());
                    addTechniquePasses(technique);
                }
            }
            else if (strncmp(value, kSingleTechnique, sizeof(kSingleTechnique)) == 0) {
                CGtechnique technique = cgGetNamedTechnique(effect, value + sizeof(kSingleTechnique));
                addTechniquePasses(technique);
            }
        }
    }
    void setTextureParameters(CGparameter parameter) {
        const CGtype type = cgGetParameterType(parameter);
        if (type == CG_SAMPLER2D) {
            CGstateassignment sa = cgGetFirstSamplerStateAssignment(parameter);
            while (sa) {
                const CGstate s = cgGetSamplerStateAssignmentState(sa);
                if (cgIsState(s) && cgGetStateType(s) == CG_TEXTURE) {
                    CGparameter textureParameter = cgGetTextureStateAssignmentValue(sa);
                    const char *semantic = cgGetParameterSemantic(textureParameter);
                    if (strcmp(semantic, "MATERIALTEXTURE") == 0) {
                        materialTexture.addParameter(parameter);
                        break;
                    }
                    else if (strcmp(semantic, "MATERIALSPHEREMAP") == 0) {
                        materialSphereMap.addParameter(parameter);
                        break;
                    }
                    else if (strcmp(semantic, "RENDERCOLORTARGET") == 0) {
                        renderColorTarget.addParameter(textureParameter);
                        break;
                    }
                    else if (strcmp(semantic, "OFFSCREENRENDERTARGET") == 0) {
                        offscreenRenderTarget.addParameter(textureParameter);
                        break;
                    }
                }
                sa = cgGetNextStateAssignment(sa);
            }
        }
    }
    bool parsePassScript(const CGpass pass, GLuint frameBufferObject) {
        if (m_passScriptStates[pass])
            return true;
        if (!cgIsPass(pass))
            return false;
        const CGannotation scriptAnnotation = cgGetNamedPassAnnotation(pass, "Script");
        ScriptStates passScriptStates;
        if (cgIsAnnotation(scriptAnnotation)) {
            const std::string s(cgGetStringAnnotationValue(scriptAnnotation));
            ScriptState lastState, newState;
            std::istringstream stream(s);
            std::string segment;
            bool renderColorTarget0DidSet = false,
                    renderDepthStencilTargetDidSet = false,
                    useRenderBuffer = false;
            while (std::getline(stream, segment, ';')) {
                std::string::size_type offset = segment.find("=");
                if (offset != std::string::npos) {
                    const std::string &command = segment.substr(0, offset);
                    const std::string &value = segment.substr(offset + 1);
                    newState.enterLoop = lastState.enterLoop;
                    if (command == "RenderColorTarget" || command == "RenderColorTarget0") {
                        setStateFromRenderColorTargetSemantic(renderColorTarget,
                                                              value,
                                                              frameBufferObject,
                                                              ScriptState::kRenderColorTarget0,
                                                              newState);
                        useRenderBuffer = newState.frameBufferObject > 0;
                        renderColorTarget0DidSet = true;
                    }
                    else if (renderColorTarget0DidSet && command == "RenderColorTarget1") {
                        setStateFromRenderColorTargetSemantic(renderColorTarget,
                                                              value,
                                                              frameBufferObject,
                                                              ScriptState::kRenderColorTarget1,
                                                              newState);
                        useRenderBuffer = newState.frameBufferObject > 0;
                    }
                    else if (renderColorTarget0DidSet && command == "RenderColorTarget2") {
                        setStateFromRenderColorTargetSemantic(renderColorTarget,
                                                              value,
                                                              frameBufferObject,
                                                              ScriptState::kRenderColorTarget2,
                                                              newState);
                        useRenderBuffer = newState.frameBufferObject > 0;
                    }
                    else if (renderColorTarget0DidSet && command == "RenderColorTarget3") {
                        setStateFromRenderColorTargetSemantic(renderColorTarget,
                                                              value,
                                                              frameBufferObject,
                                                              ScriptState::kRenderColorTarget3,
                                                              newState);
                        useRenderBuffer = newState.frameBufferObject > 0;
                    }
                    else if (command == "RenderDepthStencilTarget") {
                        setStateFromRenderDepthStencilTargetSemantic(renderDepthStencilTarget,
                                                                     value,
                                                                     frameBufferObject,
                                                                     ScriptState::kRenderDepthStencilTarget,
                                                                     newState);
                        useRenderBuffer = newState.frameBufferObject > 0;
                        renderDepthStencilTargetDidSet = true;
                    }
                    else if (command == "ClearSetColor") {
                        setStateFromParameter(effect, value, CG_FLOAT4, ScriptState::kClearSetColor, newState);
                    }
                    else if (command == "ClearSetDepth") {
                        setStateFromParameter(effect, value, CG_FLOAT, ScriptState::kClearSetDepth, newState);
                    }
                    else if (command == "Clear") {
                        if (value == "Color")
                            newState.type = ScriptState::kClearColor;
                        else if (value == "Depth")
                            newState.type = ScriptState::kClearDepth;
                        if (useRenderBuffer)
                            newState.frameBufferObject = frameBufferObject;
                    }
                    else if (command == "Draw") {
                        if (value == "Buffer") {
                            if (m_scriptClass == kObject)
                                return false;
                            newState.type = ScriptState::kDrawBuffer;
                            newState.frameBufferObject = frameBufferObject;
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
            state.type = ScriptState::kDrawGeometry;
            passScriptStates.push_back(state);
        }
        m_passScriptStates.insert(pass, passScriptStates);
        return true;
    }
    bool parseTechniqueScript(const CGtechnique technique, GLuint &frameBufferObject) {
        if (!cgIsTechnique(technique) || !cgValidateTechnique(technique))
            return false;
        const CGannotation scriptAnnotation = cgGetNamedTechniqueAnnotation(technique, "Script");
        ScriptStates techniqueScriptStates;
        if (cgIsAnnotation(scriptAnnotation)) {
            const std::string s(cgGetStringAnnotationValue(scriptAnnotation));
            std::istringstream stream(s);
            std::string segment;
            ScriptStates scriptExternalStates;
            ScriptState lastState, newState;
            bool useScriptExternal = m_scriptOrder == kPostProcess,
                    renderColorTarget0DidSet = false,
                    renderDepthStencilTargetDidSet = false,
                    useRenderBuffer = false;
            glGenFramebuffers(1, &frameBufferObject);
            while (std::getline(stream, segment, ';')) {
                std::string::size_type offset = segment.find("=");
                if (offset != std::string::npos) {
                    const std::string &command = segment.substr(0, offset);
                    const std::string &value = segment.substr(offset + 1);
                    newState.enterLoop = lastState.enterLoop;
                    if (command == "RenderColorTarget" || command == "RenderColorTarget0") {
                        setStateFromRenderColorTargetSemantic(renderColorTarget,
                                                              value,
                                                              frameBufferObject,
                                                              ScriptState::kRenderColorTarget0,
                                                              newState);
                        useRenderBuffer = newState.frameBufferObject > 0;
                        renderColorTarget0DidSet = true;
                    }
                    else if (renderColorTarget0DidSet && command == "RenderColorTarget1") {
                        setStateFromRenderColorTargetSemantic(renderColorTarget,
                                                              value,
                                                              frameBufferObject,
                                                              ScriptState::kRenderColorTarget1,
                                                              newState);
                        useRenderBuffer = newState.frameBufferObject > 0;
                    }
                    else if (renderColorTarget0DidSet && command == "RenderColorTarget2") {
                        setStateFromRenderColorTargetSemantic(renderColorTarget,
                                                              value,
                                                              frameBufferObject,
                                                              ScriptState::kRenderColorTarget2,
                                                              newState);
                        useRenderBuffer = newState.frameBufferObject > 0;
                    }
                    else if (renderColorTarget0DidSet && command == "RenderColorTarget3") {
                        setStateFromRenderColorTargetSemantic(renderColorTarget,
                                                              value,
                                                              frameBufferObject,
                                                              ScriptState::kRenderColorTarget3,
                                                              newState);
                        useRenderBuffer = newState.frameBufferObject > 0;
                    }
                    else if (command == "RenderDepthStencilTarget") {
                        setStateFromRenderDepthStencilTargetSemantic(renderDepthStencilTarget,
                                                                     value,
                                                                     frameBufferObject,
                                                                     ScriptState::kRenderDepthStencilTarget,
                                                                     newState);
                        useRenderBuffer = newState.frameBufferObject > 0;
                        renderDepthStencilTargetDidSet = true;
                    }
                    else if (command == "ClearSetColor") {
                        setStateFromParameter(effect, value, CG_FLOAT4, ScriptState::kClearSetColor, newState);
                    }
                    else if (command == "ClearSetDepth") {
                        setStateFromParameter(effect, value, CG_FLOAT, ScriptState::kClearSetDepth, newState);
                    }
                    else if (command == "Clear") {
                        if (value == "Color")
                            newState.type = ScriptState::kClearColor;
                        else if (value == "Depth")
                            newState.type = ScriptState::kClearDepth;
                        if (useRenderBuffer)
                            newState.frameBufferObject = frameBufferObject;
                    }
                    else if (command == "Pass") {
                        CGpass pass = cgGetNamedPass(technique, value.c_str());
                        if (parsePassScript(pass, frameBufferObject)) {
                            newState.type = ScriptState::kPass;
                            newState.pass = pass;
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
                            newState.type = ScriptState::kLoopByCount;
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
            if (renderColorTarget0DidSet || renderDepthStencilTargetDidSet) {
                m_techniqueFrameBuffers.insert(technique, frameBufferObject);
            }
            else {
                glDeleteFramebuffers(1, &frameBufferObject);
                frameBufferObject = 0;
            }
            m_techniqueScriptStates.insert(technique, techniqueScriptStates);
            m_scriptExternalStates.insert(technique, scriptExternalStates);
            return !lastState.enterLoop;
        }
        return true;
    }

    const IRenderDelegate *m_delegate;
    ScriptOutputType m_scriptOutput;
    ScriptClassType m_scriptClass;
    ScriptOrderType m_scriptOrder;
    Techniques m_techniques;
    TechniquePasses m_techniquePasses;
    Hash<HashPtr, ScriptStates> m_scriptExternalStates;
    Hash<HashPtr, ScriptStates> m_techniqueScriptStates;
    Hash<HashPtr, ScriptStates> m_passScriptStates;
    btHashMap<btHashPtr, GLuint> m_techniqueFrameBuffers;
};

} /* namespace gl2 */
} /* namespace vpvl2 */

#endif
