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

#pragma once
#ifndef VPVL2_FX_EFFECTENGINE_H_
#define VPVL2_FX_EFFECTENGINE_H_

#include "vpvl2/Common.h"
#include "vpvl2/IApplicationContext.h"
#include "vpvl2/IRenderEngine.h"
#include "vpvl2/fx/Effect.h"
#include "vpvl2/extensions/gl/FrameBufferObject.h"

#include <string>

namespace vpvl2
{

class IModel;
class IApplicationContext;
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

namespace fx
{

class BaseParameter
{
public:
    BaseParameter();
    virtual ~BaseParameter();

    virtual void addParameter(IEffect::Parameter *parameter);
    virtual void invalidate();
    IEffect::Parameter *parameterRef() const;

protected:
    static void connectParameter(IEffect::Parameter *sourceParameter, IEffect::Parameter *&destinationParameter);

    IEffect::Parameter *m_parameterRef;

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
    MatrixSemantic(const IApplicationContext *applicationContextRef, int flags);
    ~MatrixSemantic();

    void setParameter(IEffect::Parameter *parameterRef, const char *suffix);
    void invalidate();
    void setMatrices(const IModel *model, int extraCameraFlags, int extraLightFlags);

private:
    void setMatrixParameters(const char *suffix,
                             IEffect::Parameter *sourceParameterRef,
                             IEffect::Parameter *&inverseRef,
                             IEffect::Parameter *&transposedRef,
                             IEffect::Parameter *&inversetransposedRef,
                             IEffect::Parameter *&baseParameterRef);
    void setMatrix(const IModel *model, IEffect::Parameter *parameterRef, int flags);

    const IApplicationContext *m_applicationContextRef;
    IEffect::Parameter *m_camera;
    IEffect::Parameter *m_cameraInversed;
    IEffect::Parameter *m_cameraTransposed;
    IEffect::Parameter *m_cameraInverseTransposed;
    IEffect::Parameter *m_light;
    IEffect::Parameter *m_lightInversed;
    IEffect::Parameter *m_lightTransposed;
    IEffect::Parameter *m_lightInverseTransposed;
    int m_flags;

    VPVL2_DISABLE_COPY_AND_ASSIGN(MatrixSemantic)
};

class MaterialSemantic : public BaseParameter
{
public:
    MaterialSemantic();
    ~MaterialSemantic();

    void addParameter(IEffect::Parameter *parameterRef);
    void invalidate();
    void setGeometryColor(const Color &value);
    void setGeometryValue(const Scalar &value);
    void setLightColor(const Color &value);
    void setLightValue(const Scalar &value);

    IEffect::Parameter *geometryParameter() const { return m_geometry; } /* for test */
    IEffect::Parameter *lightParameter() const { return m_light; } /* for test */

private:
    IEffect::Parameter *m_geometry;
    IEffect::Parameter *m_light;

    VPVL2_DISABLE_COPY_AND_ASSIGN(MaterialSemantic)
};

class MaterialTextureSemantic : public BaseParameter
{
public:
    MaterialTextureSemantic();
    ~MaterialTextureSemantic();
    static bool hasMipmap(const IEffect::Parameter *textureParameterRef, const IEffect::Parameter *samplerParameterRef);

    void addTextureParameter(const IEffect::Parameter *textureParameterRef, IEffect::Parameter *samplerParameterRef);
    void invalidate();
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

    void setTexture(extensions::gl::GLuint value);
};

class GeometrySemantic : public BaseParameter
{
public:
    GeometrySemantic();
    ~GeometrySemantic();

    void addParameter(IEffect::Parameter *parameterRef);
    void invalidate();
    void setCameraValue(const Vector3 &value);
    void setLightValue(const Vector3 &value);

    IEffect::Parameter *cameraParameter() const { return m_camera; } /* for test */
    IEffect::Parameter *lightParameter() const { return m_light; } /* for test */

private:
    IEffect::Parameter *m_camera;
    IEffect::Parameter *m_light;

    VPVL2_DISABLE_COPY_AND_ASSIGN(GeometrySemantic)
};

class TimeSemantic : public BaseParameter
{
public:
    TimeSemantic(const IApplicationContext *applicationContextRef);
    ~TimeSemantic();

    void addParameter(IEffect::Parameter *parameterRef);
    void invalidate();
    void update();

    IEffect::Parameter *syncEnabledParameter() const { return m_syncEnabled; } /* for test */
    IEffect::Parameter *syncDisabledParameter() const { return m_syncDisabled; } /* for test */

private:
    const IApplicationContext *m_applicationContextRef;
    IEffect::Parameter *m_syncEnabled;
    IEffect::Parameter *m_syncDisabled;

    VPVL2_DISABLE_COPY_AND_ASSIGN(TimeSemantic)
};

class ControlObjectSemantic : public BaseParameter
{
public:
    ControlObjectSemantic(const Scene *sceneRef, const IApplicationContext *applicationContextRef);
    ~ControlObjectSemantic();

    void addParameter(IEffect::Parameter *parameterRef);
    void invalidate();
    void update(const IModel *self);

private:
    void setParameter(const IModel *model, IEffect::Parameter *parameterRef);
    void setModelBoneMorphParameter(const IModel *model, const IEffect::Annotation *annotationRef, IEffect::Parameter *parameterRef);
    void setAssetParameter(const IModel *model, const IEffect::Annotation *annotationRef, IEffect::Parameter *parameterRef);
    void setModelParameter(const IModel *model, IEffect::Parameter *parameterRef);
    void setNullParameter(IEffect::Parameter *parameterRef);

    const Scene *m_sceneRef;
    const IApplicationContext *m_applicationContextRef;
    Array<IEffect::Parameter *> m_parameterRefs;

    VPVL2_DISABLE_COPY_AND_ASSIGN(ControlObjectSemantic)
};

class RenderColorTargetSemantic : public BaseParameter
{
public:
    struct TextureReference {
        TextureReference(extensions::gl::FrameBufferObject *fbo,
                ITexture *tex,
                IEffect::Parameter *p,
                IEffect::Parameter *s)
            : frameBufferObjectRef(fbo),
              textureRef(tex),
              textureParameterRef(p),
              samplerParameterRef(s)
        {
        }
        ~TextureReference() {
            frameBufferObjectRef = 0;
            textureRef = 0;
            textureParameterRef = 0;
            samplerParameterRef = 0;
        }
        extensions::gl::FrameBufferObject *frameBufferObjectRef;
        ITexture *textureRef;
        IEffect::Parameter *textureParameterRef;
        IEffect::Parameter *samplerParameterRef;
    };

    static bool tryGetTextureFlags(const IEffect::Parameter *textureParameterRef,
                                   const IEffect::Parameter *samplerParameterRef,
                                   bool enableAllTextureTypes,
                                   int &flags);

    RenderColorTargetSemantic(IApplicationContext *applicationContextRef);
    ~RenderColorTargetSemantic();

    void addFrameBufferObjectParameter(IEffect::Parameter *textureParameterRef,
                                       IEffect::Parameter *samplerParameterRef,
                                       extensions::gl::FrameBufferObject *frameBufferObjectRef,
                                       void *userData,
                                       bool enableResourceName,
                                       bool enableAllTextureTypes);
    void invalidate();
    const TextureReference *findTexture(const char *name) const;
    IEffect::Parameter *findParameter(const char *name) const;
    int countParameters() const;

protected:
    typedef void (GLAPIENTRY * PFNGLGENERATEMIPMAPPROC) (extensions::gl::GLenum target);
    PFNGLGENERATEMIPMAPPROC generateMipmap;

    IApplicationContext *m_applicationContextRef;
    Array<IEffect::Parameter *> m_parameters;

    virtual void generateTexture2D(IEffect::Parameter *textureParameterRef,
                                   IEffect::Parameter *samplerParameterRef,
                                   const Vector3 &size,
                                   extensions::gl::FrameBufferObject *frameBufferObjectRef,
                                   extensions::gl::BaseSurface::Format &format);
    virtual void generateTexture3D(IEffect::Parameter *textureParamaterRef,
                                   IEffect::Parameter *samplerParameterRef,
                                   const Vector3 &size,
                                   extensions::gl::FrameBufferObject *frameBufferObjectRef);
    void getSize2(const IEffect::Parameter *parameterRef, vsize &width, vsize &height) const;
    void getSize3(const IEffect::Parameter *parameterRef, vsize &width, vsize &height, vsize &depth) const;
    ITexture *lastTextureRef() const;

private:
    void generateTexture2D0(IEffect::Parameter *textureRef,
                            IEffect::Parameter *samplerRef,
                            extensions::gl::FrameBufferObject *frameBufferObjectRef);
    void generateTexture3D0(IEffect::Parameter *textureRef,
                            IEffect::Parameter *samplerRef,
                            extensions::gl::FrameBufferObject *frameBufferObjectRef);

    PointerArray<ITexture> m_textures;
    Hash<HashString, TextureReference> m_name2textures;
    Hash<HashString, IEffect::Parameter *> m_path2parameterRefs;

    VPVL2_DISABLE_COPY_AND_ASSIGN(RenderColorTargetSemantic)
};

class RenderDepthStencilTargetSemantic : public RenderColorTargetSemantic
{
public:
    struct Buffer {
        Buffer(extensions::gl::FrameBufferObject *fbo,
               extensions::gl::FrameBufferObject::BaseRenderBuffer *renderBuffer,
               IEffect::Parameter *p)
            : frameBufferObjectRef(fbo),
              renderBufferRef(renderBuffer),
              parameterRef(p)
        {
        }
        ~Buffer() {
            frameBufferObjectRef = 0;
            renderBufferRef = 0;
        }
        extensions::gl::FrameBufferObject *frameBufferObjectRef;
        extensions::gl::FrameBufferObject::BaseRenderBuffer *renderBufferRef;
        IEffect::Parameter *parameterRef;
    };

    RenderDepthStencilTargetSemantic(IApplicationContext *applicationContextRef);
    ~RenderDepthStencilTargetSemantic();

    void addFrameBufferObjectParameter(IEffect::Parameter *parameterRef, extensions::gl::FrameBufferObject *frameBufferObjectRef);
    void invalidate();
    const Buffer *findDepthStencilBuffer(const char *name) const;

private:
    Array<extensions::gl::FrameBufferObject::BaseRenderBuffer *> m_renderBuffers;
    Hash<HashString, Buffer> m_buffers;

    VPVL2_DISABLE_COPY_AND_ASSIGN(RenderDepthStencilTargetSemantic)
};

class OffscreenRenderTargetSemantic : public RenderColorTargetSemantic
{
public:
    OffscreenRenderTargetSemantic(IApplicationContext *applicationContextRef);
    ~OffscreenRenderTargetSemantic();

protected:
    void generateTexture2D(IEffect::Parameter *textureParameterRef,
                           IEffect::Parameter *samplerParameterRef,
                           const Vector3 &size,
                           extensions::gl::FrameBufferObject *frameBufferObjectRef,
                           extensions::gl::BaseSurface::Format &format);

private:
    VPVL2_DISABLE_COPY_AND_ASSIGN(OffscreenRenderTargetSemantic)
};

class AnimatedTextureSemantic : public BaseParameter
{
public:
    AnimatedTextureSemantic(IApplicationContext *applicationContextRef);
    ~AnimatedTextureSemantic();

    void addParameter(IEffect::Parameter *parameterRef);
    void invalidate();
    void update(const RenderColorTargetSemantic &renderColorTarget);

private:
    IApplicationContext *m_applicationContextRef;
    Array<IEffect::Parameter *> m_parameterRefs;

    VPVL2_DISABLE_COPY_AND_ASSIGN(AnimatedTextureSemantic)
};

class TextureValueSemantic : public BaseParameter
{
public:
    TextureValueSemantic(IApplicationContext *resolver);
    ~TextureValueSemantic();

    void addParameter(IEffect::Parameter *parameterRef);
    void invalidate();
    void update();

private:
    typedef void (GLAPIENTRY * PFNGLBINDTEXTUREPROC) (extensions::gl::GLenum target, extensions::gl::GLuint texture);
    typedef void (GLAPIENTRY * PFNGLGETTEXIMAGEPROC) (extensions::gl::GLenum target, extensions::gl::GLint level, extensions::gl::GLenum format, extensions::gl::GLenum type, extensions::gl::GLvoid *pixels);
    PFNGLBINDTEXTUREPROC bindTexture;
    PFNGLGETTEXIMAGEPROC getTexImage;

    Array<IEffect::Parameter *> m_parameterRefs;

    VPVL2_DISABLE_COPY_AND_ASSIGN(TextureValueSemantic)
};

class SelfShadowSemantic : public BaseParameter
{
public:
    SelfShadowSemantic();
    ~SelfShadowSemantic();

    void addParameter(IEffect::Parameter *parameterRef);
    void invalidate();
    void updateParameter(const IShadowMap *shadowMapRef);

private:
    IEffect::Parameter *m_center;
    IEffect::Parameter *m_size;
    IEffect::Parameter *m_distance;
    IEffect::Parameter *m_rate;

    VPVL2_DISABLE_COPY_AND_ASSIGN(SelfShadowSemantic)
};

class MatricesParameter : public BaseParameter
{
public:
    MatricesParameter();
    ~MatricesParameter();

    void setValues(const float32 *value, size_t size);
};

class VPVL2_API EffectEngine
{
public:
    typedef Array<IEffect::Technique *> Techniques;
    typedef btAlignedObjectArray<IEffect::Pass *> Passes;
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
        const RenderColorTargetSemantic::TextureReference *renderColorTargetTextureRef;
        const RenderDepthStencilTargetSemantic::Buffer *renderDepthStencilBufferRef;
        IEffect::Parameter *parameter;
        IEffect::Pass *pass;
        bool enterLoop;
        bool isRenderTargetBound;
    };
    typedef btAlignedObjectArray<ScriptState> Script;
    struct DrawPrimitiveCommand {
        DrawPrimitiveCommand(extensions::gl::GLenum mode,
                             extensions::gl::GLsizei count,
                             extensions::gl::GLenum type,
                             const uint8 *ptr, vsize offset, vsize stride)
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
            : mode(extensions::gl::kGL_TRIANGLES),
              count(0),
              type(extensions::gl::kGL_UNSIGNED_INT),
              ptr(0),
              offset(0),
              stride(sizeof(int)),
              start(0),
              end(0)
        {
        }
        extensions::gl::GLenum mode;
        extensions::gl::GLsizei count;
        extensions::gl::GLenum type;
        const uint8 *ptr;
        vsize offset;
        vsize stride;
        int start;
        int end;
    };

    EffectEngine(Scene *sceneRef, IApplicationContext *applicationContextRef);
    virtual ~EffectEngine();

    bool setEffect(IEffect *effect, void *userData, bool isDefaultStandardEffect);
    void invalidate();
    IEffect::Technique *findTechnique(const char *pass,
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
    void executeTechniquePasses(const IEffect::Technique *technique,
                                const DrawPrimitiveCommand &command,
                                IEffect *nextPostEffectRef);
    void setModelMatrixParameters(const IModel *model,
                                  int extraCameraFlags = 0,
                                  int extraLightFlags = 0);
    void setDefaultStandardEffectRef(IEffect *effectRef);
    void setZeroGeometryParameters(const IModel *model);
    void updateModelLightParameters(const Scene *scene, const IModel *model);
    void updateSceneParameters();
    bool isStandardEffect() const;
    const Script *findTechniqueScript(const IEffect::Technique *technique) const;
    const Script *findPassScript(const IEffect::Pass *pass) const;

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
    MaterialTextureSemantic materialToonTexture;
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
    MatricesParameter boneMatrices;
    /* special parameters */
    BooleanParameter parthf;
    BooleanParameter spadd;
    BooleanParameter spsub;
    BooleanParameter transp;
    BooleanParameter useTexture;
    BooleanParameter useSpheremap;
    BooleanParameter useToon;
    BooleanParameter opadd;
    IntegerParameter vertexCount;
    IntegerParameter subsetCount;

protected:
    typedef void (GLAPIENTRY * PFNGLCLEARPROC) (extensions::gl::GLbitfield mask);
    typedef void (GLAPIENTRY * PFNGLCLEARCOLORPROC) (extensions::gl::GLclampf red, extensions::gl::GLclampf green, extensions::gl::GLclampf blue, extensions::gl::GLclampf alpha);
    typedef void (GLAPIENTRY * PFNGLCLEARDEPTHPROC) (extensions::gl::GLclampd depth);
    PFNGLCLEARPROC clear;
    PFNGLCLEARCOLORPROC clearColor;
    PFNGLCLEARDEPTHPROC clearDepth;

    virtual void drawPrimitives(const DrawPrimitiveCommand &command) const = 0;
    virtual void rebindVertexBundle() = 0;

private:
    class RectangleRenderEngine;

    static bool containsSubset(const IEffect::Annotation *annotation, int subset, int nmaterials);
    static bool testTechnique(const IEffect::Technique *technique,
                              const char *pass,
                              int offset,
                              int nmaterials,
                              bool hasTexture,
                              bool hasSphereMap,
                              bool useToon);
    static IEffect::Technique *findTechniqueIn(const Techniques &techniques,
                                                const char *pass,
                                                int offset,
                                                int nmaterials,
                                                bool hasTexture,
                                                bool hasSphereMap,
                                                bool useToon);

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
                                            IEffect::Parameter::Type testType,
                                            ScriptState::Type type,
                                            ScriptState &state);
    void executePass(IEffect::Pass *pass, const DrawPrimitiveCommand &command) const;
    void setRenderColorTargetFromScriptState(const ScriptState &state);
    void setRenderDepthStencilTargetFromScriptState(const ScriptState &state);
    void setDefaultRenderTarget(const Vector3 &viewport);
    void executeScript(const Script *script,
                       const DrawPrimitiveCommand &command,
                       IEffect *nextPostEffectRef,
                       bool &isPassExecuted);
    void addTechniquePasses(IEffect::Technique *technique);
    void clearTechniquePasses();
    void setStandardsGlobal(const IEffect::Parameter *parameterRef, bool &ownTechniques);
    void parseSamplerStateParameter(IEffect::Parameter *samplerParameter,
                                    extensions::gl::FrameBufferObject *frameBufferObjectRef,
                                    void *userData);
    void addSharedTextureParameter(IEffect::Parameter *textureParameterRef,
                                   extensions::gl::FrameBufferObject *frameBufferObjectRef,
                                   RenderColorTargetSemantic &semantic);
    bool parsePassScript(IEffect::Pass *pass);
    bool parseTechniqueScript(const IEffect::Technique *technique, Passes &passes);

    IEffect *m_effectRef;
    IEffect *m_defaultStandardEffectRef;
    IApplicationContext *m_applicationContextRef;
    RectangleRenderEngine *m_rectangleRenderEngine;
    extensions::gl::FrameBufferObject *m_frameBufferObjectRef;
    ScriptOutputType m_scriptOutput;
    ScriptClassType m_scriptClass;
    Techniques m_techniques;
    Techniques m_defaultTechniques;
    TechniquePasses m_techniquePasses;
    Script m_externalScript;
    Hash<HashInt, const RenderColorTargetSemantic::TextureReference *> m_target2TextureRefs;
    Hash<HashInt, const RenderDepthStencilTargetSemantic::Buffer *> m_target2BufferRefs;
    btHashMap<btHashPtr, Script> m_techniqueScripts;
    btHashMap<btHashPtr, Script> m_passScripts;

    VPVL2_DISABLE_COPY_AND_ASSIGN(EffectEngine)
};

} /* namespace fx */
} /* namespace vpvl2 */

#endif
