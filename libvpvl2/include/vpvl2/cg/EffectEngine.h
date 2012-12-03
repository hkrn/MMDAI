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

#pragma once
#ifndef VPVL2_CG_ENGINECOMMON_H_
#define VPVL2_CG_ENGINECOMMON_H_

#include "vpvl2/Common.h"
#include "vpvl2/IRenderContext.h"
#include "vpvl2/IRenderEngine.h"
#include "vpvl2/cg/Effect.h"

#include <string>

#if defined(VPVL2_LINK_QT)
#include <QtOpenGL/QGLFunctions>
#endif /* VPVL_LINK_QT */

namespace vpvl2
{

class IModel;
class IRenderContext;
class IString;
class Scene;

namespace extensions
{
namespace gl
{
class FrameBufferObject;
}
}

namespace cg
{

using namespace extensions::gl;

class Util
{
public:
    static bool toBool(const CGannotation annotation);
    static int toInt(const CGannotation annotation);
    static float toFloat(const CGannotation annotation);
    static bool isPassEquals(const CGannotation annotation, const char *target);
    static bool isIntegerParameter(const CGparameter parameter);
    static const std::string trim(const std::string &value);

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
    CGparameter baseParameter() const { return m_baseParameter; } /* for test */

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
    MatrixSemantic(const IRenderContext *renderContextRef, int flags);
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

    const IRenderContext *m_renderContextRef;
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

    CGparameter geometryParameter() const { return m_geometry; } /* for test */
    CGparameter lightParameter() const { return m_light; } /* for test */

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

    CGparameter cameraParameter() const { return m_camera; } /* for test */
    CGparameter lightParameter() const { return m_light; } /* for test */

private:
    CGparameter m_camera;
    CGparameter m_light;

    VPVL2_DISABLE_COPY_AND_ASSIGN(GeometrySemantic)
};

class TimeSemantic : public BaseParameter
{
public:
    TimeSemantic(const IRenderContext *renderContextRef);
    ~TimeSemantic();

    void addParameter(CGparameter parameter);
    void update();

    CGparameter syncEnabledParameter() const { return m_syncEnabled; } /* for test */
    CGparameter syncDisabledParameter() const { return m_syncDisabled; } /* for test */

private:
    const IRenderContext *m_renderContextRef;
    CGparameter m_syncEnabled;
    CGparameter m_syncDisabled;

    VPVL2_DISABLE_COPY_AND_ASSIGN(TimeSemantic)
};

class ControlObjectSemantic : public BaseParameter
{
public:
    ControlObjectSemantic(const IEffect *effect, const Scene *scene, const IRenderContext *renderContextRef);
    ~ControlObjectSemantic();

    void addParameter(CGparameter parameter);
    void update(const IModel *self);

private:
    void setParameter(const IModel *model, const CGparameter parameter);

    const Scene *m_sceneRef;
    const IRenderContext *m_renderContextRef;
    const IEffect *m_effectRef;
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
        Texture(int w, int h, int d, CGparameter p, CGparameter s, GLuint i, GLenum f)
            : width(w),
              height(h),
              depth(d),
              parameter(p),
              sampler(s),
              id(i),
              format(f)
        {
        }
        Texture(const IRenderContext::Texture &t, int d, CGparameter p, CGparameter s)
            : width(t.width),
              height(t.height),
              depth(d),
              parameter(p),
              sampler(s),
              id(*static_cast<const GLuint *>(t.object)),
              format(t.format)
        {
        }
        int width;
        int height;
        int depth;
        CGparameter parameter;
        CGparameter sampler;
        GLuint id;
        GLenum format;
    };

    RenderColorTargetSemantic(IRenderContext *renderContextRef);
    ~RenderColorTargetSemantic();

    void addParameter(CGparameter parameter,
                      CGparameter sampler,
                      const IString *dir,
                      bool enableResourceName,
                      bool enableAllTextureTypes);
    const Texture *findTexture(const char *name) const;
    CGparameter findParameter(const char *name) const;

protected:
    bool isMimapEnabled(const CGparameter parameter) const;
    void getTextureFormat(const CGparameter parameter,
                          GLenum &internal,
                          GLenum &format,
                          GLenum &type) const;
    virtual void generateTexture2D(const CGparameter parameter,
                                   const CGparameter sampler,
                                   GLuint texture,
                                   size_t width,
                                   size_t height,
                                   GLenum &format);
    virtual void generateTexture3D(const CGparameter parameter,
                                   const CGparameter sampler,
                                   GLuint texture,
                                   size_t width,
                                   size_t height,
                                   size_t depth);
    void getSize2(const CGparameter parameter, size_t &width, size_t &height) const;
    void getSize3(const CGparameter parameter, size_t &width, size_t &height, size_t &depth) const;

private:
    GLuint generateTexture2D0(const CGparameter parameter, const CGparameter sampler);
    GLuint generateTexture3D0(const CGparameter parameter, const CGparameter sampler);

    IRenderContext *m_renderContextRef;
    Array<CGparameter> m_parameters;
    Array<GLuint> m_textures;
    Hash<HashString, Texture> m_name2textures;
    Hash<HashString, CGparameter> m_path2parameters;

    VPVL2_DISABLE_COPY_AND_ASSIGN(RenderColorTargetSemantic)
};

class RenderDepthStencilTargetSemantic : public RenderColorTargetSemantic
{
public:
    struct Buffer {
        Buffer(int w, int h, CGparameter p)
            : width(w),
              height(h),
              parameter(p)
        {
        }
        int width;
        int height;
        CGparameter parameter;
    };

    RenderDepthStencilTargetSemantic(IRenderContext *renderContextRef);
    ~RenderDepthStencilTargetSemantic();

    void addParameter(CGparameter parameter);
    const Buffer *findRenderBuffer(const char *name) const;

private:
    Hash<HashString, Buffer> m_buffers;

    VPVL2_DISABLE_COPY_AND_ASSIGN(RenderDepthStencilTargetSemantic)
};

class OffscreenRenderTargetSemantic : public RenderColorTargetSemantic
{
public:
    OffscreenRenderTargetSemantic(Effect *effectRef, IRenderContext *renderContextRef);
    ~OffscreenRenderTargetSemantic();

    void addParameter(CGparameter parameter, CGparameter sampler, const IString *dir);

protected:
    void generateTexture2D(const CGparameter parameter,
                           const CGparameter sampler,
                           GLuint texture,
                           size_t width,
                           size_t height,
                           GLenum &format);

private:
    Effect *m_effectRef;
    VPVL2_DISABLE_COPY_AND_ASSIGN(OffscreenRenderTargetSemantic)
};

class AnimatedTextureSemantic : public BaseParameter
{
public:
    AnimatedTextureSemantic(IRenderContext *renderContextRef);
    ~AnimatedTextureSemantic();

    void addParameter(CGparameter parameter);
    void update(const RenderColorTargetSemantic &renderColorTarget);

private:
    IRenderContext *m_renderContextRef;
    Array<CGparameter> m_parameters;

    VPVL2_DISABLE_COPY_AND_ASSIGN(AnimatedTextureSemantic)
};

class TextureValueSemantic : public BaseParameter
{
public:
    TextureValueSemantic();
    ~TextureValueSemantic();

    void addParameter(CGparameter parameter);
    void update();

private:
    Array<CGparameter> m_parameters;

    VPVL2_DISABLE_COPY_AND_ASSIGN(TextureValueSemantic)
};

class EffectEngine
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
    struct ScriptState {
        ScriptState();
        ~ScriptState();

        void reset();
        void setFromState(const ScriptState &other);
        void setFromTexture(const RenderColorTargetSemantic::Texture *t);
        void setFromBuffer(const RenderDepthStencilTargetSemantic::Buffer *b);

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
        CGparameter sampler;
        CGpass pass;
        GLuint texture;
        GLuint depthBuffer;
        GLuint stencilBuffer;
        GLenum textureFormat;
        size_t width;
        size_t height;
        bool enterLoop;
        bool isRenderTargetBound;
    };
    typedef btAlignedObjectArray<ScriptState> Script;

    EffectEngine(const Scene *scene,
                 const IString *dir,
                 Effect *effect,
                 IRenderContext *renderContextRef);
    virtual ~EffectEngine();

    bool attachEffect(IEffect *e, const IString *dir);
    CGtechnique findTechnique(const char *pass,
                              int offset,
                              int nmaterials,
                              bool hasTexture,
                              bool hasSphereMap,
                              bool useToon) const;
    void executeScriptExternal();
    bool hasTechniques(IEffect::ScriptOrderType order) const;
    void executeProcess(const IModel *model,
                        const IEffect *nextPostEffectRef,
                        IEffect::ScriptOrderType order);
    void executeTechniquePasses(const CGtechnique technique,
                                const IEffect *nextPostEffectRef,
                                const GLenum mode,
                                const GLsizei count,
                                const GLenum type,
                                const GLvoid *ptr);
    void setModelMatrixParameters(const IModel *model,
                                  int extraCameraFlags = 0,
                                  int extraLightFlags = 0);
    void setZeroGeometryParameters(const IModel *model);
    void updateModelGeometryParameters(const Scene *scene, const IModel *model);
    void updateSceneParameters();
    bool validateStandard() const;
    const Script *findTechniqueScript(const CGtechnique technique) const;
    const Script *findPassScript(const CGpass pass) const;

    IEffect *effect() const { return m_effectRef; }
    ScriptOutputType scriptOutput() const { return m_scriptOutput; }
    ScriptClassType scriptClass() const { return m_scriptClass; }
    IEffect::ScriptOrderType scriptOrder() const { return m_scriptOrder; }

    const Techniques &techniques() const { return m_techniques; } /* for test */

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

protected:
    virtual void drawPrimitives(const GLenum mode,
                                const GLsizei count,
                                const GLenum type,
                                const GLvoid *ptr) const = 0;

private:
    class RectRenderEngine;

    static bool testTechnique(const CGtechnique technique,
                              const char *pass,
                              int offset,
                              int nmaterials,
                              bool hasTexture,
                              bool hasSphereMap,
                              bool useToon);
    static bool containsSubset(const CGannotation annotation, int subset, int nmaterials);
    void setScriptStateFromRenderColorTargetSemantic(const RenderColorTargetSemantic &semantic,
                                                     const std::string &value,
                                                     ScriptState::Type type,
                                                     ScriptState &state);
    void setScriptStateFromRenderDepthStencilTargetSemantic(const RenderDepthStencilTargetSemantic &semantic,
                                                            const std::string &value,
                                                            ScriptState::Type type,
                                                            ScriptState &state);
    static void setScriptStateFromParameter(const CGeffect effect,
                                            const std::string &value,
                                            CGtype testType,
                                            ScriptState::Type type,
                                            ScriptState &state);
    void executePass(CGpass pass,
                     const GLenum mode,
                     const GLsizei count,
                     const GLenum type,
                     const GLvoid *ptr) const;
    void setRenderColorTargetFromScriptState(const ScriptState &state, const IEffect *nextPostEffectRef);
    void setRenderDepthStencilTargetFromScriptState(const ScriptState &state, const IEffect *nextPostEffectRef);
    void executeScript(const Script *script,
                       const IEffect *nextPostEffectRef,
                       const GLenum mode,
                       const GLsizei count,
                       const GLenum type,
                       const GLvoid *ptr);
    void addTechniquePasses(const CGtechnique technique);
    void setStandardsGlobal(const CGparameter parameter, bool &ownTechniques);
    void setTextureParameters(CGparameter parameter, const IString *dir);
    bool parsePassScript(const CGpass pass);
    bool parseTechniqueScript(const CGtechnique technique, Passes &passes);

    Effect *m_effectRef;
    IRenderContext *m_renderContextRef;
    RectRenderEngine *m_rectRenderEngine;
    FrameBufferObject *m_frameBufferObjectRef;
    ScriptOutputType m_scriptOutput;
    ScriptClassType m_scriptClass;
    IEffect::ScriptOrderType m_scriptOrder;
    Techniques m_techniques;
    TechniquePasses m_techniquePasses;
    Script m_externalScript;
    btHashMap<btHashInt, const RenderColorTargetSemantic::Texture *> m_target2textureRefs;
    btHashMap<btHashInt, const RenderDepthStencilTargetSemantic::Buffer *> m_target2bufferRefs;
    btAlignedObjectArray<GLuint> m_renderColorTargets;
    btHashMap<btHashPtr, Script> m_techniqueScripts;
    btHashMap<btHashPtr, Script> m_passScripts;

    VPVL2_DISABLE_COPY_AND_ASSIGN(EffectEngine)
};

} /* namespace gl2 */
} /* namespace vpvl2 */

#endif
