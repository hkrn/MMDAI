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
#include "vpvl2/IRenderDelegate.h"
#include "vpvl2/IRenderEngine.h"

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

class MatrixSemantic : public BaseParameter
{
public:
    MatrixSemantic()
        : BaseParameter(),
          m_camera(0),
          m_light(0)
    {
    }
    ~MatrixSemantic() {
        m_camera = 0;
        m_light = 0;
    }

    void addParameter(CGparameter parameter) {
        CGannotation annotation = cgGetNamedParameterAnnotation(parameter, "Object");
        if (!cgIsAnnotation(annotation)) {
            BaseParameter::connectParameter(parameter, m_camera);
        }
        else {
            const char *name = cgGetStringAnnotationValue(annotation);
            if (strcmp(name, "Camera") == 0) {
                BaseParameter::connectParameter(parameter, m_camera);
            }
            else if (strcmp(name, "Light") == 0) {
                BaseParameter::connectParameter(parameter, m_light);
            }
        }
    }
    void setCameraMatrix(float *value) {
        if (cgIsParameter(m_camera))
            cgSetMatrixParameterfr(m_camera, value);
    }
    void setLightMatrix(float *value) {
        if (cgIsParameter(m_light))
            cgSetMatrixParameterfr(m_light, value);
    }

private:
    CGparameter m_camera;
    CGparameter m_light;
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
    CGannotation syncInEditMode;
    TimeSemantic()
        : BaseParameter(),
          syncInEditMode(0)
    {
    }
    void addParameter(CGparameter parameter) {
        BaseParameter::addParameter(parameter);
        syncInEditMode = cgGetNamedParameterAnnotation(parameter, "SyncInEditMode");
    }
};

class ControlObjectSemantic : public BaseParameter
{
public:
    CGannotation name;
    CGannotation item;
    ControlObjectSemantic()
        : name(0),
          item(0)
    {
    }
    void addParameter(CGparameter parameter) {
        BaseParameter::addParameter(parameter);
        name = cgGetNamedParameterAnnotation(parameter, "name");
        item = cgGetNamedParameterAnnotation(parameter, "item");
    }
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

struct Effect {
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
        if (!annotation)
            return true;
        const char *s = cgGetStringAnnotationValue(annotation);
        return strcmp(s, target) == 0;
    }
    static bool containsSubset(CGannotation annotation, int subset, int nmaterials) {
        if (!annotation)
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

    Effect()
        : effect(0),
          viewportPixelSize(0),
          mousePosition(0),
          leftMouseDown(0),
          middleMouseDown(0),
          rightMouseDown(0),
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
        while (parameter) {
            const char *semantic = cgGetParameterSemantic(parameter);
            if (strcmp(semantic, "WORLD") == 0) {
                world.addParameter(parameter);
            }
            else if (strcmp(semantic, "VIEW") == 0) {
                view.addParameter(parameter);
            }
            else if (strcmp(semantic, "PROJECTION") == 0) {
                projection.addParameter(parameter);
            }
            else if (strcmp(semantic, "WORLDVIEW") == 0) {
                worldView.addParameter(parameter);
            }
            else if (strcmp(semantic, "VIEWPROJECTION") == 0) {
                viewProjection.addParameter(parameter);
            }
            else if (strcmp(semantic, "WORLDVIEWPROJECTION") == 0) {
                worldViewProjection.addParameter(parameter);
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
            }
            else if (strcmp(semantic, "MOUSEPOSITION") == 0) {
            }
            else if (strcmp(semantic, "LEFTMOUSEDOWN") == 0) {
            }
            else if (strcmp(semantic, "MIDDLEMOUSEDOWN") == 0) {
            }
            else if (strcmp(semantic, "RIGHTMOUSEDOWN") == 0) {
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
    CGtechnique findTechnique(const char *pass, int offset, int nmaterials, bool hasTexture, bool useToon) {
        CGtechnique technique = cgGetFirstTechnique(effect);
        while (technique) {
            if (cgValidateTechnique(technique) == CG_FALSE) {
                technique = cgGetNextTechnique(technique);
                continue;
            }
            int ok = 1;
            CGannotation passAnnotation = cgGetNamedTechniqueAnnotation(technique, "MMDPass");
            ok &= Effect::isPassEquals(passAnnotation, pass);
            CGannotation subsetAnnotation = cgGetNamedTechniqueAnnotation(technique, "Subset");
            ok &= Effect::containsSubset(subsetAnnotation, offset, nmaterials);
            CGannotation useTextureAnnotation = cgGetNamedTechniqueAnnotation(technique, "UseTexture");
            ok &= (!useTextureAnnotation || Effect::toBool(useTextureAnnotation) == hasTexture);
            CGannotation useSphereMapAnnotation = cgGetNamedTechniqueAnnotation(technique, "UseSphereMap");
            ok &= (!useSphereMapAnnotation || Effect::toBool(useSphereMapAnnotation) == hasTexture);
            CGannotation useToonAnnotation = cgGetNamedTechniqueAnnotation(technique, "UseToon");
            ok &= (!useToonAnnotation || Effect::toBool(useToonAnnotation) == useToon);
            if (ok == 1)
                break;
            technique = cgGetNextTechnique(technique);
        }
        return technique;
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
    CGparameter viewportPixelSize;
    TimeSemantic time;
    TimeSemantic elapsedTime;
    CGparameter mousePosition;
    CGparameter leftMouseDown;
    CGparameter middleMouseDown;
    CGparameter rightMouseDown;
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
