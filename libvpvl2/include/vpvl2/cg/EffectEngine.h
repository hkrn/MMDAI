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

#pragma once
#ifndef VPVL2_CG_EFFECTENGINE_H_
#define VPVL2_CG_EFFECTENGINE_H_

#include "vpvl2/Common.h"
#include "vpvl2/IRenderContext.h"
#include "vpvl2/IRenderEngine.h"
#include "vpvl2/cg/Effect.h"

#include <string>

namespace vpvl2
{

class IModel;
class IRenderContext;
class IShadowMap;
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

class BaseParameter
{
public:
    BaseParameter();
    virtual ~BaseParameter();

    void addParameter(IEffect::IParameter *parameter);
    void invalidateParameter();
    IEffect::IParameter *baseParameter() const { return m_parameterRef; }

protected:
    static void connectParameter(IEffect::IParameter *sourceParameter, IEffect::IParameter *&destinationParameter);

    IEffect::IParameter *m_parameterRef;

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

class FloatParameter : public BaseParameter
{
public:
    FloatParameter();
    ~FloatParameter();

    void setValue(float value);

private:
    VPVL2_DISABLE_COPY_AND_ASSIGN(FloatParameter)
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

    void addParameter(IEffect::IParameter *parameterRef, const char *suffix);
    void invalidateParameter();
    void setMatrices(const IModel *model, int extraCameraFlags, int extraLightFlags);

private:
    void setParameter(const char *suffix,
                      IEffect::IParameter *sourceParameterRef,
                      IEffect::IParameter *&inverseRef,
                      IEffect::IParameter *&transposedRef,
                      IEffect::IParameter *&inversetransposedRef,
                      IEffect::IParameter *&baseParameterRef);
    void setMatrix(const IModel *model, IEffect::IParameter *parameterRef, int flags);

    const IRenderContext *m_renderContextRef;
    IEffect::IParameter *m_camera;
    IEffect::IParameter *m_cameraInversed;
    IEffect::IParameter *m_cameraTransposed;
    IEffect::IParameter *m_cameraInverseTransposed;
    IEffect::IParameter *m_light;
    IEffect::IParameter *m_lightInversed;
    IEffect::IParameter *m_lightTransposed;
    IEffect::IParameter *m_lightInverseTransposed;
    int m_flags;

    VPVL2_DISABLE_COPY_AND_ASSIGN(MatrixSemantic)
};

class MaterialSemantic : public BaseParameter
{
public:
    MaterialSemantic();
    ~MaterialSemantic();

    void addParameter(IEffect::IParameter *parameterRef);
    void invalidateParameter();
    void setGeometryColor(const Vector3 &value);
    void setGeometryValue(const Scalar &value);
    void setLightColor(const Vector3 &value);
    void setLightValue(const Scalar &value);

    IEffect::IParameter *geometryParameter() const { return m_geometry; } /* for test */
    IEffect::IParameter *lightParameter() const { return m_light; } /* for test */

private:
    IEffect::IParameter *m_geometry;
    IEffect::IParameter *m_light;

    VPVL2_DISABLE_COPY_AND_ASSIGN(MaterialSemantic)
};

class MaterialTextureSemantic : public BaseParameter
{
public:
    MaterialTextureSemantic();
    ~MaterialTextureSemantic();
    static bool hasMipmap(const IEffect::IParameter *textureParameterRef, const IEffect::IParameter *samplerParameterRef);

    void addParameter(const IEffect::IParameter *textureParameterRef, IEffect::IParameter *samplerParameterRef);
    void invalidateParameter();
    void setTexture(const HashPtr &key, const ITexture *value);
    void updateParameter(const HashPtr &key);
    bool isMipmapEnabled() const { return m_mipmap; }

private:
    Hash<HashPtr, const ITexture *> m_textures;
    bool m_mipmap;

    VPVL2_DISABLE_COPY_AND_ASSIGN(MaterialTextureSemantic)
};

class TextureUnit : public BaseParameter
{
public:
    TextureUnit();
    ~TextureUnit();

    void setTexture(GLuint value);
};

class GeometrySemantic : public BaseParameter
{
public:
    GeometrySemantic();
    ~GeometrySemantic();

    void addParameter(IEffect::IParameter *parameterRef);
    void invalidateParameter();
    void setCameraValue(const Vector3 &value);
    void setLightValue(const Vector3 &value);

    IEffect::IParameter *cameraParameter() const { return m_camera; } /* for test */
    IEffect::IParameter *lightParameter() const { return m_light; } /* for test */

private:
    IEffect::IParameter *m_camera;
    IEffect::IParameter *m_light;

    VPVL2_DISABLE_COPY_AND_ASSIGN(GeometrySemantic)
};

class TimeSemantic : public BaseParameter
{
public:
    TimeSemantic(const IRenderContext *renderContextRef);
    ~TimeSemantic();

    void addParameter(IEffect::IParameter *parameterRef);
    void invalidateParameter();
    void update();

    IEffect::IParameter *syncEnabledParameter() const { return m_syncEnabled; } /* for test */
    IEffect::IParameter *syncDisabledParameter() const { return m_syncDisabled; } /* for test */

private:
    const IRenderContext *m_renderContextRef;
    IEffect::IParameter *m_syncEnabled;
    IEffect::IParameter *m_syncDisabled;

    VPVL2_DISABLE_COPY_AND_ASSIGN(TimeSemantic)
};

class ControlObjectSemantic : public BaseParameter
{
public:
    ControlObjectSemantic(const Scene *sceneRef, const IRenderContext *renderContextRef);
    ~ControlObjectSemantic();

    void addParameter(IEffect::IParameter *parameterRef);
    void invalidateParameter();
    void update(const IModel *self);

private:
    void setParameter(const IModel *model, IEffect::IParameter *parameterRef);

    const Scene *m_sceneRef;
    const IRenderContext *m_renderContextRef;
    Array<IEffect::IParameter *> m_parameterRefs;

    VPVL2_DISABLE_COPY_AND_ASSIGN(ControlObjectSemantic)
};

class RenderColorTargetSemantic : public BaseParameter
{
public:
    struct Texture {
        Texture(FrameBufferObject *fbo,
                ITexture *tex,
                IEffect::IParameter *p,
                IEffect::IParameter *s)
            : frameBufferObjectRef(fbo),
              textureRef(tex),
              textureParameterRef(p),
              samplerParameterRef(s)
        {
        }
        ~Texture() {
            frameBufferObjectRef = 0;
            textureRef = 0;
            textureParameterRef = 0;
            samplerParameterRef = 0;
        }
        FrameBufferObject *frameBufferObjectRef;
        ITexture *textureRef;
        IEffect::IParameter *textureParameterRef;
        IEffect::IParameter *samplerParameterRef;
    };

    static bool tryGetTextureFlags(const IEffect::IParameter *textureParameterRef,
                                   const IEffect::IParameter *samplerParameterRef,
                                   bool enableAllTextureTypes,
                                   int &flags);

    RenderColorTargetSemantic(IRenderContext *renderContextRef);
    ~RenderColorTargetSemantic();

    void addParameter(IEffect::IParameter *textureParameterRef,
                      IEffect::IParameter *samplerParameterRef,
                      FrameBufferObject *frameBufferObjectRef,
                      const IString *dir,
                      bool enableResourceName,
                      bool enableAllTextureTypes);
    void invalidateParameter();
    const Texture *findTexture(const char *name) const;
    IEffect::IParameter *findParameter(const char *name) const;
    int countParameters() const;

protected:
    Array<IEffect::IParameter *> m_parameters;

    virtual void generateTexture2D(IEffect::IParameter *textureParameterRef,
                                   IEffect::IParameter *samplerParameterRef,
                                   const Vector3 &size,
                                   FrameBufferObject *frameBufferObjectRef,
                                   BaseSurface::Format &format);
    virtual void generateTexture3D(IEffect::IParameter *textureParamaterRef,
                                   IEffect::IParameter *samplerParameterRef,
                                   const Vector3 &size,
                                   FrameBufferObject *frameBufferObjectRef);
    void getSize2(const IEffect::IParameter *parameterRef, size_t &width, size_t &height) const;
    void getSize3(const IEffect::IParameter *parameterRef, size_t &width, size_t &height, size_t &depth) const;
    ITexture *lastTextureRef() const;

private:
    void generateTexture2D0(IEffect::IParameter *textureRef,
                            IEffect::IParameter *samplerRef,
                            FrameBufferObject *frameBufferObjectRef);
    void generateTexture3D0(IEffect::IParameter *textureRef,
                            IEffect::IParameter *samplerRef,
                            FrameBufferObject *frameBufferObjectRef);

    IRenderContext *m_renderContextRef;
    PointerArray<ITexture> m_textures;
    Hash<HashString, Texture> m_name2textures;
    Hash<HashString, IEffect::IParameter *> m_path2parameterRefs;

    VPVL2_DISABLE_COPY_AND_ASSIGN(RenderColorTargetSemantic)
};

class RenderDepthStencilTargetSemantic : public RenderColorTargetSemantic
{
public:
    struct Buffer {
        Buffer(FrameBufferObject *fbo,
               FrameBufferObject::BaseRenderBuffer *renderBuffer,
               IEffect::IParameter *p)
            : frameBufferObjectRef(fbo),
              renderBufferRef(renderBuffer),
              parameterRef(p)
        {
        }
        ~Buffer() {
            frameBufferObjectRef = 0;
            renderBufferRef = 0;
        }
        FrameBufferObject *frameBufferObjectRef;
        FrameBufferObject::BaseRenderBuffer *renderBufferRef;
        IEffect::IParameter *parameterRef;
    };

    RenderDepthStencilTargetSemantic(IRenderContext *renderContextRef);
    ~RenderDepthStencilTargetSemantic();

    void addParameter(IEffect::IParameter *parameterRef, FrameBufferObject *frameBufferObjectRef);
    void invalidateParameter();
    const Buffer *findDepthStencilBuffer(const char *name) const;

private:
    Array<FrameBufferObject::BaseRenderBuffer *> m_renderBuffers;
    Hash<HashString, Buffer> m_buffers;

    VPVL2_DISABLE_COPY_AND_ASSIGN(RenderDepthStencilTargetSemantic)
};

class OffscreenRenderTargetSemantic : public RenderColorTargetSemantic
{
public:
    OffscreenRenderTargetSemantic(IRenderContext *renderContextRef);
    ~OffscreenRenderTargetSemantic();

protected:
    void generateTexture2D(IEffect::IParameter *textureParameterRef,
                           IEffect::IParameter *samplerParameterRef,
                           const Vector3 &size,
                           FrameBufferObject *frameBufferObjectRef,
                           BaseSurface::Format &format);

private:
    VPVL2_DISABLE_COPY_AND_ASSIGN(OffscreenRenderTargetSemantic)
};

class AnimatedTextureSemantic : public BaseParameter
{
public:
    AnimatedTextureSemantic(IRenderContext *renderContextRef);
    ~AnimatedTextureSemantic();

    void addParameter(IEffect::IParameter *parameterRef);
    void invalidateParameter();
    void update(const RenderColorTargetSemantic &renderColorTarget);

private:
    IRenderContext *m_renderContextRef;
    Array<IEffect::IParameter *> m_parameterRefs;

    VPVL2_DISABLE_COPY_AND_ASSIGN(AnimatedTextureSemantic)
};

class TextureValueSemantic : public BaseParameter
{
public:
    TextureValueSemantic();
    ~TextureValueSemantic();

    void addParameter(IEffect::IParameter *parameterRef);
    void invalidateParameter();
    void update();

private:
    Array<IEffect::IParameter *> m_parameterRefs;

    VPVL2_DISABLE_COPY_AND_ASSIGN(TextureValueSemantic)
};

class SelfShadowSemantic : public BaseParameter
{
public:
    SelfShadowSemantic();
    ~SelfShadowSemantic();

    void addParameter(IEffect::IParameter *parameterRef);
    void invalidateParameter();
    void updateParameter(const IShadowMap *shadowMapRef);

private:
    IEffect::IParameter *m_center;
    IEffect::IParameter *m_size;
    IEffect::IParameter *m_distance;
    IEffect::IParameter *m_rate;

    VPVL2_DISABLE_COPY_AND_ASSIGN(SelfShadowSemantic)
};

class EffectEngine
{
public:
    typedef Array<IEffect::ITechnique *> Techniques;
    typedef btAlignedObjectArray<IEffect::IPass *> Passes;
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
        const RenderColorTargetSemantic::Texture *renderColorTargetTextureRef;
        const RenderDepthStencilTargetSemantic::Buffer *renderDepthStencilBufferRef;
        IEffect::IParameter *parameter;
        IEffect::IPass *pass;
        bool enterLoop;
        bool isRenderTargetBound;
    };
    typedef btAlignedObjectArray<ScriptState> Script;
    struct DrawPrimitiveCommand {
        DrawPrimitiveCommand(GLenum mode, GLsizei count, GLenum type, const uint8_t *ptr, size_t offset, size_t stride)
            : mode(mode),
              count(count),
              type(type),
              ptr(ptr),
              offset(offset),
              stride(stride),
              start(0),
              end(0)
        {
        }
        DrawPrimitiveCommand()
            : mode(GL_TRIANGLES),
              count(0),
              type(GL_UNSIGNED_INT),
              ptr(0),
              offset(0),
              stride(sizeof(int)),
              start(0),
              end(0)
        {
        }
        GLenum mode;
        GLsizei count;
        GLenum type;
        const uint8_t *ptr;
        size_t offset;
        size_t stride;
        int start;
        int end;
    };

    EffectEngine(Scene *sceneRef, IRenderContext *renderContextRef);
    virtual ~EffectEngine();

    bool setEffect(IEffect *effect, const IString *dir, bool isDefaultStandardEffect);
    void invalidateEffect();
    IEffect::ITechnique *findTechnique(const char *pass,
                                       int offset,
                                       int nmaterials,
                                       bool hasTexture,
                                       bool hasSphereMap,
                                       bool useToon) const;
    void executeScriptExternal();
    bool hasTechniques(IEffect::ScriptOrderType order) const;
    void executeProcess(const IModel *model,
                        IEffect *nextPostEffectRef,
                        IEffect::ScriptOrderType order);
    void executeTechniquePasses(const IEffect::ITechnique *technique,
                                const DrawPrimitiveCommand &command,
                                IEffect *nextPostEffectRef);
    void setModelMatrixParameters(const IModel *model,
                                  int extraCameraFlags = 0,
                                  int extraLightFlags = 0);
    void setDefaultStandardEffectRef(IEffect *effectRef);
    void setZeroGeometryParameters(const IModel *model);
    void updateModelGeometryParameters(const Scene *scene, const IModel *model);
    void updateSceneParameters();
    bool isStandardEffect() const;
    const Script *findTechniqueScript(const IEffect::ITechnique *technique) const;
    const Script *findPassScript(const IEffect::IPass *pass) const;

    IEffect *effect() const { return m_effectRef; }
    ScriptOutputType scriptOutput() const { return m_scriptOutput; }
    ScriptClassType scriptClass() const { return m_scriptClass; }
    IEffect::ScriptOrderType scriptOrder() const {
        return m_effectRef ? m_effectRef->scriptOrderType() : IEffect::kStandard;
    }

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
    FloatParameter edgeWidth;
    Float4Parameter addingTexture;
    Float4Parameter addingSphere;
    Float4Parameter multiplyTexture;
    Float4Parameter multiplySphere;
    GeometrySemantic position;
    GeometrySemantic direction;
    MaterialTextureSemantic materialTexture;
    MaterialTextureSemantic materialSphereMap;
    TextureUnit depthTexture;
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
    SelfShadowSemantic selfShadow;
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
    virtual void drawPrimitives(const DrawPrimitiveCommand &command) const = 0;
    virtual void rebindVertexBundle() = 0;

private:
    class RectangleRenderEngine;

    static bool testTechnique(const IEffect::ITechnique *technique,
                              const char *pass,
                              int offset,
                              int nmaterials,
                              bool hasTexture,
                              bool hasSphereMap,
                              bool useToon);
    static bool containsSubset(const IEffect::IAnnotation *annotation, int subset, int nmaterials);

    void setScriptStateFromRenderColorTargetSemantic(const RenderColorTargetSemantic &semantic,
                                                     const std::string &value,
                                                     ScriptState::Type type,
                                                     ScriptState &state);
    void setScriptStateFromRenderDepthStencilTargetSemantic(const RenderDepthStencilTargetSemantic &semantic,
                                                            const std::string &value,
                                                            ScriptState::Type type,
                                                            ScriptState &state);
    static void setScriptStateFromParameter(const IEffect *effectRef,
                                            const std::string &value,
                                            IEffect::IParameter::Type testType,
                                            ScriptState::Type type,
                                            ScriptState &state);
    void executePass(IEffect::IPass *pass, const DrawPrimitiveCommand &command) const;
    void setRenderColorTargetFromScriptState(const ScriptState &state, IEffect *nextPostEffectRef);
    void setRenderDepthStencilTargetFromScriptState(const ScriptState &state, const IEffect *nextPostEffectRef);
    void executeScript(const Script *script,
                       const DrawPrimitiveCommand &command,
                       IEffect *nextPostEffectRef,
                       bool &isPassExecuted);
    void addTechniquePasses(IEffect::ITechnique *technique);
    void clearTechniquePasses();
    void setStandardsGlobal(const IEffect::IParameter *parameterRef, bool &ownTechniques);
    void parseSamplerStateParameter(IEffect::IParameter *samplerParameter,
                                    FrameBufferObject *frameBufferObjectRef,
                                    const IString *dir);
    void addSharedTextureParameter(IEffect::IParameter *textureParameterRef,
                                   FrameBufferObject *frameBufferObjectRef,
                                   RenderColorTargetSemantic &semantic);
    bool parsePassScript(IEffect::IPass *pass);
    bool parseTechniqueScript(const IEffect::ITechnique *technique, Passes &passes);

    IEffect *m_effectRef;
    IEffect *m_defaultStandardEffectRef;
    IRenderContext *m_renderContextRef;
    RectangleRenderEngine *m_rectangleRenderEngine;
    FrameBufferObject *m_frameBufferObjectRef;
    ScriptOutputType m_scriptOutput;
    ScriptClassType m_scriptClass;
    Techniques m_techniques;
    Techniques m_defaultTechniques;
    TechniquePasses m_techniquePasses;
    Script m_externalScript;
    Hash<HashInt, const RenderColorTargetSemantic::Texture *> m_target2TextureRefs;
    Hash<HashInt, const RenderDepthStencilTargetSemantic::Buffer *> m_target2BufferRefs;
    btHashMap<btHashPtr, Script> m_techniqueScripts;
    btHashMap<btHashPtr, Script> m_passScripts;

    VPVL2_DISABLE_COPY_AND_ASSIGN(EffectEngine)
};

} /* namespace gl2 */
} /* namespace vpvl2 */

#endif
