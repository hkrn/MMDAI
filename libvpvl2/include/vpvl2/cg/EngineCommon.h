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
    static bool containsSubset(CGannotation annotation, int subset, int nmaterials) {
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
                int to = strtol(segment.substr(offset).c_str(), 0, 10);
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
    MatrixSemantic(int flags)
        : BaseParameter(),
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
    void setMatrices(const IRenderDelegate *delegate, const IModel *model, int extraCameraFlags, int extraLightFlags) {
        setMatrix(delegate, model, m_camera,
                  extraCameraFlags | IRenderDelegate::kCameraMatrix);
        setMatrix(delegate, model, m_cameraInversed,
                  extraCameraFlags | IRenderDelegate::kCameraMatrix | IRenderDelegate::kInverseMatrix);
        setMatrix(delegate, model, m_cameraTransposed,
                  extraCameraFlags | IRenderDelegate::kCameraMatrix | IRenderDelegate::kTransposeMatrix);
        setMatrix(delegate, model, m_cameraInverseTransposed,
                  extraCameraFlags | IRenderDelegate::kCameraMatrix | IRenderDelegate::kInverseMatrix | IRenderDelegate::kTransposeMatrix);
        setMatrix(delegate, model, m_light,
                  extraLightFlags | IRenderDelegate::kLightMatrix);
        setMatrix(delegate, model, m_lightInversed,
                  extraLightFlags | IRenderDelegate::kLightMatrix | IRenderDelegate::kInverseMatrix);
        setMatrix(delegate, model, m_lightTransposed,
                  extraLightFlags | IRenderDelegate::kLightMatrix | IRenderDelegate::kTransposeMatrix);
        setMatrix(delegate, model, m_lightInverseTransposed,
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
    void setMatrix(const IRenderDelegate *delegate, const IModel *model, CGparameter parameter, int flags) {
        if (cgIsParameter(parameter)) {
            float matrix[16];
            delegate->getMatrix(matrix, model, m_flags | flags);
            cgSetMatrixParameterfr(parameter, matrix);
        }
    }

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
    TimeSemantic()
        : BaseParameter(),
          m_syncEnabled(0),
          m_syncDisabled(0)
    {
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
    void setValue(const IRenderDelegate *delegate) {
        float value = 0;
        if (cgIsParameter(m_syncEnabled)) {
            delegate->getTime(value, true);
            cgSetParameter1f(m_syncEnabled, value);
        }
        if (cgIsParameter(m_syncDisabled)) {
            delegate->getTime(value, false);
            cgSetParameter1f(m_syncDisabled, value);
        }
    }

private:
    CGparameter m_syncEnabled;
    CGparameter m_syncDisabled;
};

class ControlObjectSemantic : public BaseParameter
{
public:
    ControlObjectSemantic()
    {
    }
    void addParameter(CGparameter parameter) {
        if (cgIsAnnotation(cgGetNamedParameterAnnotation(parameter, "name")))
            m_parameters.add(parameter);
    }
    void update(const IRenderDelegate *delegate, const IModel *self) {
        const int nparameters = m_parameters.count();
        for (int i = 0; i < nparameters; i++) {
            CGparameter parameter = m_parameters[i];
            CGannotation nameAnnotation = cgGetNamedParameterAnnotation(parameter, "name");
            const char *name = cgGetStringAnnotationValue(nameAnnotation);
            if (strcmp(name, "(self)") == 0) {
                setParameter(delegate, self, parameter);
            }
            else if (strcmp(name, "(OffscreenOwenr)") == 0) {
                // TODO
            }
            else {
                IModel *model = delegate->findModel(name);
                setParameter(delegate, model, parameter);
            }
        }
    }

private:
    void setParameter(const IRenderDelegate *delegate, const IModel *model, CGparameter parameter) {
        float matrix4x4[16];
        Transform::getIdentity().getOpenGLMatrix(matrix4x4);
        CGtype type = cgGetParameterType(parameter);
        if (model) {
            CGannotation itemAnnotation = cgGetNamedParameterAnnotation(parameter, "item");
            if (cgIsAnnotation(itemAnnotation)) {
                const char *item = cgGetStringAnnotationValue(itemAnnotation);
                IModel::Type type = model->type();
                if (type == IModel::kPMD || type == IModel::kPMX) {
                    const IString *s = delegate->toUnicode(reinterpret_cast<const uint8_t *>(item));
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

    Array<CGparameter> m_parameters;
};

class TextureSemantic : public BaseParameter
{
public:
    CGannotation width;
    CGannotation height;
    CGannotation depth;
    CGannotation dimensions;
    CGannotation viewportRatio;
    CGannotation format;
    TextureSemantic()
        : width(0),
          height(0),
          depth(0),
          dimensions(0),
          viewportRatio(0),
          format(0)
    {
    }
    void addParameter(CGparameter parameter) {
        BaseParameter::addParameter(parameter);
        width = cgGetNamedParameterAnnotation(parameter, "Width");
        height = cgGetNamedParameterAnnotation(parameter, "Height");
        depth = cgGetNamedParameterAnnotation(parameter, "Depth");
        dimensions = cgGetNamedParameterAnnotation(parameter, "Dimensions");
        viewportRatio = cgGetNamedParameterAnnotation(parameter, "ViewportRatio");
        format = cgGetNamedParameterAnnotation(parameter, "Format");
    }
};

class RenderConrolTargetSemantic : public TextureSemantic
{
public:
    CGannotation mipLevels;
    CGannotation levels;
    RenderConrolTargetSemantic()
        : TextureSemantic(),
          mipLevels(0),
          levels(0)
    {
    }
    void addParameter(CGparameter parameter) {
        TextureSemantic::addParameter(parameter);
        mipLevels = cgGetNamedParameterAnnotation(parameter, "MipLevels");
        levels = cgGetNamedParameterAnnotation(parameter, "Levels");
    }
};

class OffscreenRenderTargetSemantic : public RenderConrolTargetSemantic
{
public:
    CGannotation clearColor;
    CGannotation clearDepth;
    CGannotation antiAlias;
    CGannotation description;
    CGannotation defaultEffect;
    OffscreenRenderTargetSemantic()
        : RenderConrolTargetSemantic(),
          clearColor(0),
          clearDepth(0),
          antiAlias(0),
          description(0),
          defaultEffect(0)
    {
    }

    void addParameter(CGparameter parameter) {
        RenderConrolTargetSemantic::addParameter(parameter);
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
    void addParameter(CGparameter parameter) {
        BaseParameter::addParameter(parameter);
        textureName = cgGetNamedParameterAnnotation(parameter, "TextureName");
    }
};

class StandardsGlobalSemantic : public BaseParameter
{
public:
    CGannotation scriptOutput;
    CGannotation scriptClass;
    CGannotation scriptOrder;
    CGannotation script;
    StandardsGlobalSemantic()
        : scriptOutput(0),
          scriptClass(0),
          scriptOrder(0),
          script(0)
    {
    }
    void addParameter(CGparameter parameter) {
        BaseParameter::addParameter(parameter);
        scriptOutput = cgGetNamedParameterAnnotation(parameter, "ScriptOutput");
        scriptClass = cgGetNamedParameterAnnotation(parameter, "ScriptClass");
        scriptOrder = cgGetNamedParameterAnnotation(parameter, "ScriptOrder");
        script = cgGetNamedParameterAnnotation(parameter, "Script");
    }
};

class Effect {
public:
    Effect()
        : effect(0),
          world(IRenderDelegate::kWorldMatrix),
          view(IRenderDelegate::kViewMatrix),
          projection(IRenderDelegate::kProjectionMatrix),
          worldView(IRenderDelegate::kWorldMatrix | IRenderDelegate::kViewMatrix),
          viewProjection(IRenderDelegate::kViewMatrix | IRenderDelegate::kProjectionMatrix),
          worldViewProjection(IRenderDelegate::kWorldMatrix | IRenderDelegate::kViewMatrix | IRenderDelegate::kProjectionMatrix),
          index(0)
    {
    }
    ~Effect()
    {
        if (cgIsEffect(effect))
            cgDestroyEffect(effect);
        effect = 0;
    }

    bool attachEffect(CGeffect value) {
        CGparameter parameter = cgGetFirstEffectParameter(value);
        static const char kWorldSemantic[] = "WORLD";
        static const char kViewSemantic[] = "VIEW";
        static const char kProjectionSemantic[] = "PROJECTION";
        static const char kWorldViewSemantic[] = "WORLDVIEW";
        static const char kViewProjectionSemantic[] = "VIEWPROJECTION";
        static const char kWorldViewProjectionSemantic[] = "WORLDVIEWPROJECTION";
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
            else if (strcmp(semantic, "RENDERCONTROLTARGET") == 0) {
                renderControlTarget.addParameter(parameter);
            }
            else if (strcmp(semantic, "RENDERDEPTHSTENCILTARGET") == 0) {
                renderDepthStencilTarget.addParameter(parameter);
            }
            else if (strcmp(semantic, "ANIMATEDTEXTURE") == 0) {
                animatedTexture.addParameter(parameter);
            }
            else if (strcmp(semantic, "OFFSCREENRENDERTARGET") == 0) {
                offscreenRenderTarget.addParameter(parameter);
            }
            else if (strcmp(semantic, "TEXTUREVALUE") == 0) {
                textureValue.addParameter(parameter);
            }
            else if (strcmp(semantic, "STANDARDSGLOBAL") == 0) {
                standardsGlobal.addParameter(parameter);
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
                    CGtype type = cgGetParameterType(parameter);
                    if (type == CG_SAMPLER2D) {
                        CGstateassignment sa = cgGetFirstSamplerStateAssignment(parameter);
                        while (sa) {
                            CGstate s = cgGetSamplerStateAssignmentState(sa);
                            if (cgIsState(s) && cgGetStateType(s) == CG_TEXTURE) {
                                CGparameter textureParameter = cgGetTextureStateAssignmentValue(sa);
                                const char *semantic = cgGetParameterSemantic(textureParameter);
                                if (strcmp(semantic, "MATERIALTEXTURE") == 0) {
                                    materialTexture.addParameter(parameter);
                                }
                                else if (strcmp(semantic, "MATERIALSPHEREMAP") == 0) {
                                    materialSphereMap.addParameter(parameter);
                                }
                            }
                            sa = cgGetNextStateAssignment(sa);
                        }
                    }
                }
            }
            parameter = cgGetNextParameter(parameter);
        }
        effect = value;
        return true;
    }
    CGtechnique findTechnique(const char *pass, int offset, int nmaterials, bool hasTexture, bool hasSphereMap, bool useToon) {
        CGtechnique technique = cgGetFirstTechnique(effect);
        while (technique) {
            if (cgValidateTechnique(technique) == CG_FALSE) {
                technique = cgGetNextTechnique(technique);
                continue;
            }
            int ok = 1;
            CGannotation passAnnotation = cgGetNamedTechniqueAnnotation(technique, "MMDPass");
            ok &= Util::isPassEquals(passAnnotation, pass);
            CGannotation subsetAnnotation = cgGetNamedTechniqueAnnotation(technique, "Subset");
            ok &= Util::containsSubset(subsetAnnotation, offset, nmaterials);
            CGannotation useTextureAnnotation = cgGetNamedTechniqueAnnotation(technique, "UseTexture");
            ok &= (!cgIsAnnotation(useTextureAnnotation) || Util::toBool(useTextureAnnotation) == hasTexture);
            CGannotation useSphereMapAnnotation = cgGetNamedTechniqueAnnotation(technique, "UseSphereMap");
            ok &= (!cgIsAnnotation(useSphereMapAnnotation) || Util::toBool(useSphereMapAnnotation) == hasSphereMap);
            CGannotation useToonAnnotation = cgGetNamedTechniqueAnnotation(technique, "UseToon");
            ok &= (!cgIsAnnotation(useToonAnnotation) || Util::toBool(useToonAnnotation) == useToon);
            if (ok == 1)
                break;
            technique = cgGetNextTechnique(technique);
        }
        return technique;
    }
    void setModelMatrixParameters(const IRenderDelegate *delegate,
                                  const IModel *model,
                                  int extraCameraFlags = 0,
                                  int extraLightFlags = 0)
    {
        world.setMatrices(delegate, model, extraCameraFlags, extraLightFlags);
        view.setMatrices(delegate, model, extraCameraFlags, extraLightFlags);
        projection.setMatrices(delegate, model, extraCameraFlags, extraLightFlags);
        worldView.setMatrices(delegate, model, extraCameraFlags, extraLightFlags);
        viewProjection.setMatrices(delegate, model, extraCameraFlags, extraLightFlags);
        worldViewProjection.setMatrices(delegate, model, extraCameraFlags, extraLightFlags);
    }
    void setZeroGeometryParameters(const IModel *model) {
        edgeColor.setGeometryColor(model->edgeColor());
        toonColor.setGeometryColor(kZeroV3);
        ambient.setGeometryColor(kZeroV3);
        diffuse.setGeometryColor(kZeroV3);
        specular.setGeometryColor(kZeroV3);
        specularPower.setGeometryValue(0);
        materialTexture.setTexture(0);
        materialSphereMap.setTexture(0);
        spadd.setValue(false);
        useTexture.setValue(false);
    }
    void updateModelGeometryParameters(const IRenderDelegate *delegate, const Scene *scene, const IModel *model) {
        const Scene::ILight *light = scene->light();
        const Vector3 kOne(1, 1, 1), &color = light->color();
        ambient.setLightColor(Vector3(0.7, 0.7, 0.7) - (kOne - color));
        diffuse.setLightColor(kOne);
        emissive.setLightColor(kZeroV3);
        emissive.setGeometryColor(kZeroV3);
        specular.setLightColor(light->color());
        edgeColor.setLightColor(kZeroV3);
        const Vector3 &lightDirection = light->direction();
        position.setLightValue(-lightDirection);
        direction.setLightValue(lightDirection.normalized());
        const Scene::ICamera *camera = scene->camera();
        const Vector3 &cameraPosition = camera->modelViewTransform().getOrigin();
        position.setCameraValue(cameraPosition);
        direction.setCameraValue((cameraPosition - camera->position()).normalized());
        controlObject.update(delegate, model);
    }
    void updateViewportParameters(const IRenderDelegate *delegate) {
        Vector3 viewport;
        delegate->getViewport(viewport);
        viewportPixelSize.setValue(viewport);
        Vector4 position;
        delegate->getMousePosition(position, IRenderDelegate::kMouseCursorPosition);
        mousePosition.setValue(position);
        delegate->getMousePosition(position, IRenderDelegate::kMouseLeftPressPosition);
        leftMouseDown.setValue(position);
        delegate->getMousePosition(position, IRenderDelegate::kMouseMiddlePressPosition);
        middleMouseDown.setValue(position);
        delegate->getMousePosition(position, IRenderDelegate::kMouseRightPressPosition);
        rightMouseDown.setValue(position);
        time.setValue(delegate);
        elapsedTime.setValue(delegate);
    }
    bool isAttached() const {
        return cgIsEffect(effect) == CG_TRUE;
    }

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
    RenderConrolTargetSemantic renderControlTarget;
    TextureSemantic renderDepthStencilTarget;
    AnimatedTextureSemantic animatedTexture;
    OffscreenRenderTargetSemantic offscreenRenderTarget;
    TextureValueSemantic textureValue;
    StandardsGlobalSemantic standardsGlobal;
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
};

} /* namespace gl2 */
} /* namespace vpvl2 */

#endif
