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
#include "vpvl2/ICamera.h"
#include "vpvl2/IEffect.h"
#include "vpvl2/ILight.h"
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

#include <Cg/cg.h>
#include <Cg/cgGL.h>

namespace vpvl2
{
namespace cg
{

class Util
{
public:
    static bool toBool(const CGannotation annotation);
    static int toInt(const CGannotation annotation);
    static bool isPassEquals(const CGannotation annotation, const char *target);
    static bool isIntegerParameter(const CGparameter parameter);

private:
    Util() {}
    ~Util() {}
};

class BaseParameter
{
public:
    BaseParameter();
    virtual ~BaseParameter();

    void addParameter(CGparameter parameter);

protected:
    static void connectParameter(const CGparameter sourceParameter, CGparameter &destinationParameter);

    CGparameter m_baseParameter;

private:
    VPVL2_DISABLE_COPY_AND_ASSIGN(BaseParameter)
};

class BooleanParameter : public BaseParameter
{
public:
    BooleanParameter();
    ~BooleanParameter();

    void setValue(bool value);

private:
    VPVL2_DISABLE_COPY_AND_ASSIGN(BooleanParameter)
};

class IntegerParameter : public BaseParameter
{
public:
    IntegerParameter();
    ~IntegerParameter();

    void setValue(int value);

private:
    VPVL2_DISABLE_COPY_AND_ASSIGN(IntegerParameter)
};

class Float2Parameter : public BaseParameter
{
public:
    Float2Parameter();
    ~Float2Parameter();

    void setValue(const Vector3 &value);

private:
    VPVL2_DISABLE_COPY_AND_ASSIGN(Float2Parameter)
};

class Float4Parameter : public BaseParameter
{
public:
    Float4Parameter();
    ~Float4Parameter();

    void setValue(const Vector4 &value);

private:
    VPVL2_DISABLE_COPY_AND_ASSIGN(Float4Parameter)
};

class MatrixSemantic : public BaseParameter
{
public:
    MatrixSemantic(const IRenderDelegate *delegate, int flags);
    ~MatrixSemantic();

    void addParameter(CGparameter parameter, const char *suffix);
    void setMatrices(const IModel *model, int extraCameraFlags, int extraLightFlags);

private:
    void setParameter(const CGparameter sourceParameter,
                      const char *suffix,
                      CGparameter &inverse,
                      CGparameter &transposed,
                      CGparameter &inversetransposed,
                      CGparameter &baseParameter);
    void setMatrix(const IModel *model, CGparameter parameter, int flags);

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

    VPVL2_DISABLE_COPY_AND_ASSIGN(MatrixSemantic)
};

class MaterialSemantic : public BaseParameter
{
public:
    MaterialSemantic();
    ~MaterialSemantic();

    void addParameter(CGparameter parameter);
    void setGeometryColor(const Vector3 &value);
    void setGeometryValue(const Scalar &value);
    void setLightColor(const Vector3 &value);
    void setLightValue(const Scalar &value);

private:
    CGparameter m_geometry;
    CGparameter m_light;

    VPVL2_DISABLE_COPY_AND_ASSIGN(MaterialSemantic)
};

class MaterialTextureSemantic : public BaseParameter
{
public:
    MaterialTextureSemantic();
    ~MaterialTextureSemantic();

    void setTexture(GLuint value);

private:
    VPVL2_DISABLE_COPY_AND_ASSIGN(MaterialTextureSemantic)
};

class GeometrySemantic : public BaseParameter
{
public:
    GeometrySemantic();
    ~GeometrySemantic();

    void addParameter(CGparameter parameter);
    void setCameraValue(const Vector3 &value);
    void setLightValue(const Vector3 &value);

private:
    CGparameter m_camera;
    CGparameter m_light;

    VPVL2_DISABLE_COPY_AND_ASSIGN(GeometrySemantic)
};

class TimeSemantic : public BaseParameter
{
public:
    TimeSemantic(const IRenderDelegate *delegate);
    ~TimeSemantic();

    void addParameter(CGparameter parameter);
    void setValue();

private:
    const IRenderDelegate *m_delegate;
    CGparameter m_syncEnabled;
    CGparameter m_syncDisabled;

    VPVL2_DISABLE_COPY_AND_ASSIGN(TimeSemantic)
};

class ControlObjectSemantic : public BaseParameter
{
public:
    ControlObjectSemantic(const IRenderDelegate *delegate);
    ~ControlObjectSemantic();

    void addParameter(CGparameter parameter);
    void update(const IModel *self);

private:
    void setParameter(const IModel *model, const CGparameter parameter);

    const IRenderDelegate *m_delegate;
    Array<CGparameter> m_parameters;

    VPVL2_DISABLE_COPY_AND_ASSIGN(ControlObjectSemantic)
};

class RenderColorTargetSemantic : public BaseParameter
        #ifdef VPVL2_LINK_QT
        , protected QGLFunctions
        #endif
{
public:
    struct Texture {
        Texture(int w, int h, int d, CGparameter p, CGparameter s, GLuint i)
            : width(w),
              height(h),
              depth(d),
              parameter(p),
              sampler(s),
              id(i)
        {
        }
        int width;
        int height;
        int depth;
        CGparameter parameter;
        CGparameter sampler;
        GLuint id;
    };

    RenderColorTargetSemantic(IRenderDelegate *delegate);
    ~RenderColorTargetSemantic();

    void addParameter(CGparameter parameter, CGparameter sampler, const IString *dir);
    const Texture *findTexture(const char *name) const;

protected:
    bool isMimapEnabled(const CGparameter parameter) const;
    void getTextureFormat(const CGparameter parameter, GLenum &internal, GLenum &format) const;
    virtual void generateTexture2D(const CGparameter parameter, const CGparameter sampler, GLuint texture, int width, int height);
    virtual void generateTexture3D(const CGparameter parameter, const CGparameter sampler, GLuint texture, int width, int height, int depth);

private:
    GLuint generateTexture2D0(const CGparameter parameter, const CGparameter sampler);
    GLuint generateTexture3D0(const CGparameter parameter, const CGparameter sampler);
    void getSize2(const CGparameter parameter, int &width, int &height);
    void getSize3(const CGparameter parameter, int &width, int &height, int &depth);

    IRenderDelegate *m_delegate;
    Array<CGparameter> m_parameters;
    Array<GLuint> m_textures;
    Hash<HashString, Texture> m_name2texture;

    VPVL2_DISABLE_COPY_AND_ASSIGN(RenderColorTargetSemantic)
};

class RenderDepthStencilTargetSemantic : public RenderColorTargetSemantic
{
public:
    RenderDepthStencilTargetSemantic(IRenderDelegate *delegate);
    ~RenderDepthStencilTargetSemantic();

    GLuint findRenderBuffer(const char *name) const;

protected:
    void generateTexture2D(const CGparameter parameter, const CGparameter /* sampler */, GLuint /* texture */, int width, int height);

private:
    Array<GLuint> m_renderBuffers;
    Hash<HashString, GLuint> m_name2buffer;

    VPVL2_DISABLE_COPY_AND_ASSIGN(RenderDepthStencilTargetSemantic)
};

class OffscreenRenderTargetSemantic : public RenderColorTargetSemantic
{
public:
    CGannotation clearColor;
    CGannotation clearDepth;
    CGannotation antiAlias;
    CGannotation description;
    CGannotation defaultEffect;

    OffscreenRenderTargetSemantic(IRenderDelegate *delegate);
    ~OffscreenRenderTargetSemantic();

    void addParameter(CGparameter parameter, CGparameter sampler, const IString *dir);

private:
    VPVL2_DISABLE_COPY_AND_ASSIGN(OffscreenRenderTargetSemantic)
};

class AnimatedTextureSemantic : public BaseParameter
{
public:
    CGannotation resourceName;
    CGannotation offset;
    CGannotation speed;
    CGannotation seekVariable;

    AnimatedTextureSemantic();
    ~AnimatedTextureSemantic();

    void addParameter(CGparameter parameter);

private:
    VPVL2_DISABLE_COPY_AND_ASSIGN(AnimatedTextureSemantic)
};

class TextureValueSemantic : public BaseParameter
{
public:
    CGannotation textureName;

    TextureValueSemantic();
    ~TextureValueSemantic();

    void addParameter(CGparameter parameter);

private:
    VPVL2_DISABLE_COPY_AND_ASSIGN(TextureValueSemantic)
};

class Effect
        #ifdef VPVL2_LINK_QT
        : protected QGLFunctions
        #endif
{
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
        kSceneOrObject
    };
    enum ScriptOrderType {
        kPreProcess,
        kStandard,
        kPostProcess
    };

    Effect(IRenderDelegate *delegate);
    ~Effect();

    bool attachEffect(IEffect *e, const IString *dir);
    CGtechnique findTechnique(const char *pass,
                              int offset,
                              int nmaterials,
                              bool hasTexture,
                              bool hasSphereMap,
                              bool useToon) const;
    void executeScriptExternal();
    bool hasTechniques(ScriptOrderType order) const;
    void executeProcess(const IModel *model, ScriptOrderType order);
    void executeTechniquePasses(const CGtechnique technique,
                                const GLenum mode,
                                const GLsizei count,
                                const GLenum type,
                                const GLvoid *ptr);
    void setModelMatrixParameters(const IModel *model,
                                  int extraCameraFlags = 0,
                                  int extraLightFlags = 0);
    void setZeroGeometryParameters(const IModel *model);
    void updateModelGeometryParameters(const Scene *scene, const IModel *model);
    void updateViewportParameters();

    bool isAttached() const { return m_effect != 0; }
    ScriptOutputType scriptOutput() const { return m_scriptOutput; }
    ScriptClassType scriptClass() const { return m_scriptClass; }
    ScriptOrderType scriptOrder() const { return m_scriptOrder; }

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
    MaterialTextureSemantic depthTexture;
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
              width(0),
              height(0),
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
            width = 0;
            height = 0;
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
        int width;
        int height;
        bool enterLoop;
    };
    typedef btAlignedObjectArray<ScriptState> Script;

    static bool testTechnique(const CGtechnique technique,
                              const char *pass,
                              int offset,
                              int nmaterials,
                              bool hasTexture,
                              bool hasSphereMap,
                              bool useToon);
    static bool containsSubset(const CGannotation annotation, int subset, int nmaterials);
    void setStateFromRenderColorTargetSemantic(const RenderColorTargetSemantic &semantic,
                                               const std::string &value,
                                               GLuint frameBufferObject,
                                               ScriptState::Type type,
                                               ScriptState &state);
    void setStateFromRenderDepthStencilTargetSemantic(const RenderDepthStencilTargetSemantic &semantic,
                                                      const std::string &value,
                                                      GLuint frameBufferObject,
                                                      ScriptState::Type type,
                                                      ScriptState &state);
    static void setStateFromParameter(const CGeffect effect,
                                      const std::string &value,
                                      CGtype testType,
                                      ScriptState::Type type,
                                      ScriptState &state);
    static void executePass(CGpass pass, const GLenum mode, const GLsizei count, const GLenum type, const GLvoid *ptr);
    void setFrameBufferTexture(const ScriptState &state);
    void executeScript(const Script *script, const GLenum mode, const GLsizei count, const GLenum type, const GLvoid *ptr);
    void addTechniquePasses(const CGtechnique technique);
    void setStandardsGlobal(const CGparameter parameter);
    void setTextureParameters(CGparameter parameter, const IString *dir);
    bool parsePassScript(const CGpass pass, GLuint frameBufferObject);
    bool parseTechniqueScript(const CGtechnique technique, GLuint &frameBufferObject, Passes &passes);
    void initializeBuffer();

#ifndef __APPLE__
    PFNGLDRAWBUFFERSPROC glDrawBuffers;
#endif /* __APPLE__ */
    IEffect *m_effect;
    IRenderDelegate *m_delegate;
    ScriptOutputType m_scriptOutput;
    ScriptClassType m_scriptClass;
    ScriptOrderType m_scriptOrder;
    Techniques m_techniques;
    TechniquePasses m_techniquePasses;
    Script m_externalScript;
    btAlignedObjectArray<GLuint> m_renderColorTargets;
    btHashMap<btHashPtr, Script> m_techniqueScripts;
    btHashMap<btHashPtr, Script> m_passScripts;
    btHashMap<btHashPtr, GLuint> m_techniqueFrameBuffers;
    GLuint m_verticesBuffer;
    GLuint m_indicesBuffer;

    VPVL2_DISABLE_COPY_AND_ASSIGN(Effect)
};

} /* namespace gl2 */
} /* namespace vpvl2 */

#endif
