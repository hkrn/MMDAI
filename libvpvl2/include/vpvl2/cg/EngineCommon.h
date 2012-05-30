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

struct BaseParameter {
    CGparameter parameter;
    BaseParameter() : parameter(0) {}
    virtual ~BaseParameter() {}
    void getParameter(CGeffect effect, const char *name) {
        parameter = cgGetEffectParameterBySemantic(effect, name);
    }
};
struct MatrixParameter : public BaseParameter {
    CGannotation object;
    MatrixParameter()
        : BaseParameter(),
          object(0)
    {
    }
    void getParameter(CGeffect effect, const char *name) {
        BaseParameter::getParameter(effect, name);
        object = cgGetNamedParameterAnnotation(parameter, "Object");
    }
};
struct MaterialParameter : public BaseParameter {
    CGannotation object;
    MaterialParameter()
        : BaseParameter(),
          object(0)
    {
    }
    void getParameter(CGeffect effect, const char *name) {
        BaseParameter::getParameter(effect, name);
        object = cgGetNamedParameterAnnotation(parameter, "Object");
    }
};
struct GeometryParameter : public BaseParameter {
    CGannotation object;
    GeometryParameter()
        : BaseParameter(),
          object(0)
    {
    }
    void getParameter(CGeffect effect, const char *name) {
        BaseParameter::getParameter(effect, name);
        object = cgGetNamedParameterAnnotation(parameter, "Object");
    }
};
struct TimeParameter : public BaseParameter {
    CGannotation syncInEditMode;
    TimeParameter()
        : BaseParameter(),
          syncInEditMode(0)
    {
    }
    void getParameter(CGeffect effect, const char *name) {
        BaseParameter::getParameter(effect, name);
        syncInEditMode = cgGetNamedParameterAnnotation(parameter, "SyncInEditMode");
    }
};
struct ControlObjectParameter : public BaseParameter {
    CGannotation name;
    CGannotation item;
    ControlObjectParameter()
        : name(0),
          item(0)
    {
    }
    void getParameter(CGeffect effect, const char *n) {
        BaseParameter::getParameter(effect, n);
        name = cgGetNamedParameterAnnotation(parameter, "name");
        item = cgGetNamedParameterAnnotation(parameter, "item");
    }
};
struct TextureParameter : public BaseParameter {
    CGannotation width;
    CGannotation height;
    CGannotation depth;
    CGannotation dimensions;
    CGannotation viewportRatio;
    CGannotation format;
    TextureParameter()
        : width(0),
          height(0),
          depth(0),
          dimensions(0),
          viewportRatio(0),
          format(0)
    {
    }
    void getParameter(CGeffect effect, const char *name) {
        BaseParameter::getParameter(effect, name);
        width = cgGetNamedParameterAnnotation(parameter, "Width");
        height = cgGetNamedParameterAnnotation(parameter, "Height");
        depth = cgGetNamedParameterAnnotation(parameter, "Depth");
        dimensions = cgGetNamedParameterAnnotation(parameter, "Dimensions");
        viewportRatio = cgGetNamedParameterAnnotation(parameter, "ViewportRatio");
        format = cgGetNamedParameterAnnotation(parameter, "Format");
    }
};
struct RenderConrolTargetParameter : public TextureParameter {
    CGannotation mipLevels;
    CGannotation levels;
    RenderConrolTargetParameter()
        : TextureParameter(),
          mipLevels(0),
          levels(0)
    {
    }
    void getParameter(CGeffect effect, const char *name) {
        TextureParameter::getParameter(effect, name);
        mipLevels = cgGetNamedParameterAnnotation(parameter, "MipLevels");
        levels = cgGetNamedParameterAnnotation(parameter, "Levels");
    }
};
struct OffscreenRenderTargetParameter : public RenderConrolTargetParameter {
    CGannotation clearColor;
    CGannotation clearDepth;
    CGannotation antiAlias;
    CGannotation description;
    CGannotation defaultEffect;
    OffscreenRenderTargetParameter()
        : RenderConrolTargetParameter(),
          clearColor(0),
          clearDepth(0),
          antiAlias(0),
          description(0),
          defaultEffect(0)
    {
    }
    void getParameter(CGeffect effect, const char *name) {
        RenderConrolTargetParameter::getParameter(effect, name);
        clearColor = cgGetNamedParameterAnnotation(parameter, "ClearColor");
        clearDepth = cgGetNamedParameterAnnotation(parameter, "ClearDepth");
        antiAlias = cgGetNamedParameterAnnotation(parameter, "AntiAlias");
        description = cgGetNamedParameterAnnotation(parameter, "Description");
        defaultEffect = cgGetNamedParameterAnnotation(parameter, "DefaultEffect");
    }
};
struct AnimatedTextureParameter : public BaseParameter {
    CGannotation resourceName;
    CGannotation offset;
    CGannotation speed;
    CGannotation seekVariable;
    AnimatedTextureParameter()
        : resourceName(0),
          offset(0),
          speed(0),
          seekVariable(0)
    {
    }
    void getParameter(CGeffect effect, const char *name) {
        BaseParameter::getParameter(effect, name);
        resourceName = cgGetNamedParameterAnnotation(parameter, "ResourceName");
        offset = cgGetNamedParameterAnnotation(parameter, "Offset");
        speed = cgGetNamedParameterAnnotation(parameter, "Speed");
        seekVariable = cgGetNamedParameterAnnotation(parameter, "SeekVariable");
    }
};
struct TextureValueParameter : public BaseParameter {
    CGannotation textureName;
    TextureValueParameter()
        : textureName(0)
    {
    }
    void getParameter(CGeffect effect, const char *name) {
        BaseParameter::getParameter(effect, name);
        textureName = cgGetNamedParameterAnnotation(parameter, "TextureName");
    }
};
struct StandardsGlobalParameter : public BaseParameter {
    CGannotation scriptOutput;
    CGannotation scriptClass;
    CGannotation scriptOrder;
    CGannotation script;
    StandardsGlobalParameter()
        : scriptOutput(0),
          scriptClass(0),
          scriptOrder(0),
          script(0)
    {
    }
    void getParameter(CGeffect effect, const char *name) {
        BaseParameter::getParameter(effect, name);
        scriptOutput = cgGetNamedParameterAnnotation(parameter, "ScriptOutput");
        scriptClass = cgGetNamedParameterAnnotation(parameter, "ScriptClass");
        scriptOrder = cgGetNamedParameterAnnotation(parameter, "ScriptOrder");
        script = cgGetNamedParameterAnnotation(parameter, "Script");
    }
};
struct EffectParameters {
    EffectParameters()
        : materialTexture(0),
          materialSphereMap(0),
          viewportPixelSize(0),
          mousePosition(0),
          leftMouseDown(0),
          middleMouseDown(0),
          rightMouseDown(0),
          pathf(0),
          spadd(0),
          transp(0),
          useTexture(0),
          useSpheremap(0),
          useToon(0),
          opadd(0),
          vertexCount(0),
          subsetCount(0),
          index(0)
    {
    }
    void getEffectParameters(CGeffect effect) {
        world.getParameter(effect, "WORLD");
        view.getParameter(effect, "VIEW");
        projection.getParameter(effect, "PROJECTION");
        worldView.getParameter(effect, "WORLDVIEW");
        viewProjection.getParameter(effect, "VIEWPROJECTION");
        worldViewProjection.getParameter(effect, "WORLDVIEWPROJECTION");
        diffuse.getParameter(effect, "DIFFUSE");
        ambient.getParameter(effect, "AMBIENT");
        emissive.getParameter(effect, "EMISSIVE");
        specular.getParameter(effect, "SPECULAR");
        specularPower.getParameter(effect, "SPECULARPOWER");
        toonColor.getParameter(effect, "TOONCOLOR");
        edgeColor.getParameter(effect, "EDGECOLOR");
        position.getParameter(effect, "POSITION");
        direction.getParameter(effect, "DIRECTION");
        materialTexture = cgGetEffectParameterBySemantic(effect, "MATERIALTEXTURE");
        materialSphereMap = cgGetEffectParameterBySemantic(effect, "MATERIALSPHEREMAP");
        viewportPixelSize = cgGetEffectParameterBySemantic(effect, "VIEWPORTPIXELSIZE");
        time.getParameter(effect, "TIME");
        elapsedTime.getParameter(effect, "ELAPSEDTIME");
        mousePosition = cgGetEffectParameterBySemantic(effect, "MOUSEPOSITION");
        leftMouseDown = cgGetEffectParameterBySemantic(effect, "LEFTMOUSEDOWN");
        middleMouseDown = cgGetEffectParameterBySemantic(effect, "MIDDLEMOUSEDOWN");
        rightMouseDown = cgGetEffectParameterBySemantic(effect, "RIGHTMOUSEDOWN");
        controlObject.getParameter(effect, "CONTROLOBJECT");
        renderControlTarget.getParameter(effect, "RENDERCONTROLTARGET");
        renderDepthStencilTarget.getParameter(effect, "RENDERDEPTHSTENCILTARGET");
        animatedTexture.getParameter(effect, "ANIMATEDTEXTURE");
        offscreenRenderTarget.getParameter(effect, "OFFSCREENRENDERTARGET");
        textureValue.getParameter(effect, "TEXTUREVALUE");
        standardsGlobal.getParameter(effect, "STANDARDSGLOBAL");
        pathf = cgGetNamedEffectParameter(effect, "pathf");
        spadd = cgGetNamedEffectParameter(effect, "spadd");
        transp = cgGetNamedEffectParameter(effect, "transp");
        useTexture = cgGetNamedEffectParameter(effect, "use_texture");
        useSpheremap = cgGetNamedEffectParameter(effect, "use_spheremap");
        useToon = cgGetNamedEffectParameter(effect, "use_toon");
        opadd = cgGetNamedEffectParameter(effect, "opadd");
        vertexCount = cgGetNamedEffectParameter(effect, "VertexCount");
        subsetCount = cgGetNamedEffectParameter(effect, "SubsetCount");
        index = cgGetNamedEffectParameter(effect, "_INDEX");
    }

    MatrixParameter world;
    MatrixParameter view;
    MatrixParameter projection;
    MatrixParameter worldView;
    MatrixParameter viewProjection;
    MatrixParameter worldViewProjection;
    MaterialParameter diffuse;
    MaterialParameter ambient;
    MaterialParameter emissive;
    MaterialParameter specular;
    MaterialParameter specularPower;
    MaterialParameter toonColor;
    MaterialParameter edgeColor;
    GeometryParameter position;
    GeometryParameter direction;
    CGparameter materialTexture;
    CGparameter materialSphereMap;
    CGparameter viewportPixelSize;
    TimeParameter time;
    TimeParameter elapsedTime;
    CGparameter mousePosition;
    CGparameter leftMouseDown;
    CGparameter middleMouseDown;
    CGparameter rightMouseDown;
    ControlObjectParameter controlObject;
    RenderConrolTargetParameter renderControlTarget;
    TextureParameter renderDepthStencilTarget;
    AnimatedTextureParameter animatedTexture;
    OffscreenRenderTargetParameter offscreenRenderTarget;
    TextureValueParameter textureValue;
    StandardsGlobalParameter standardsGlobal;
    /* special parameters */
    CGparameter pathf;
    CGparameter spadd;
    CGparameter transp;
    CGparameter useTexture;
    CGparameter useSpheremap;
    CGparameter useToon;
    CGparameter opadd;
    CGparameter vertexCount;
    CGparameter subsetCount;
    CGparameter index;
};

} /* namespace gl2 */
} /* namespace vpvl2 */

#endif
