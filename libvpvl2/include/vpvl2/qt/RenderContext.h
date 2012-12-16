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
#ifndef VPVL2_QT_RENDERCONTEXT_H_
#define VPVL2_QT_RENDERCONTEXT_H_

#include "vpvl2/Common.h"
#include "vpvl2/IEffect.h"
#include "vpvl2/IRenderContext.h"

#include <QtCore/QtCore>
#include <QtOpenGL/QtOpenGL>

#include <glm/glm.hpp>

#ifdef VPVL2_ENABLE_NVIDIA_CG
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#endif

namespace nv {
class Stream;
}

namespace vpvl2
{

namespace extensions
{
namespace gl
{
class FrameBufferObject;
}
}

class Scene;

namespace qt
{

using namespace extensions::gl;

class Archive;
typedef QSharedPointer<Archive> ArchiveSharedPtr;
typedef QSharedPointer<IEffect> IEffectSharedPtr;

class RenderContext : public IRenderContext, protected QGLFunctions
{
public:
    struct TextureCache {
        TextureCache() {}
        TextureCache(int w, int h, GLuint i, GLenum f)
            : width(w),
              height(h),
              id(i),
              format(f)
        {
        }
        int width;
        int height;
        GLuint id;
        GLenum format;
    };
    struct InternalTexture {
        InternalTexture(RenderContext::Texture *r, bool m, bool t)
            : ref(r),
              isToon(t),
              isSystem(false),
              mipmap(m),
              ok(false)
        {
        }
        void assign(const RenderContext::TextureCache &cache) {
            ref->width = cache.width;
            ref->height = cache.height;
            ref->format = cache.format;
            *static_cast<GLuint *>(ref->object) = cache.id;
        }
        RenderContext::Texture *ref;
        bool isToon;
        bool isSystem;
        bool mipmap;
        bool ok;
    };
    struct InternalContext {
        QHash<QString, TextureCache> textureCache;
        void addTextureCache(const QString &path, const RenderContext::TextureCache &cache) {
            textureCache.insert(path, cache);
        }
        bool findTextureCache(const QString &path, RenderContext::InternalTexture &texture) {
            if (textureCache.contains(path)) {
                texture.assign(textureCache[path]);
                texture.ok = true;
                return true;
            }
            return false;
        }
    };

    static QSet<QString> loadableTextureExtensions();
    static QString readAllAsync(const QString &path);
    static QImage loadImageAsync(const QString &path);

    RenderContext(const QHash<QString, QString> &settings, Scene *scene);
    ~RenderContext();

    void allocateUserData(const IModel *model, void *&context);
    void releaseUserData(const IModel *model, void *&context);
    bool uploadTexture(const IString *name, const IString *dir, int flags, Texture &texture, void *context);
    void getToonColor(const IString *name, const IString *dir, Color &value, void * /* context */);
    void uploadAnimatedTexture(float offset, float speed, float seek, void *texture);
    void getMatrix(float value[], const IModel *model, int flags) const;
    void getViewport(Vector3 &value) const;
    void getTime(float &value, bool sync) const;
    void getElapsed(float &value, bool sync) const;
    void getMousePosition(Vector4 &value, MousePositionType type) const;
    void log(void *context, LogLevel level, const char *format, va_list ap);
    IString *loadKernelSource(KernelType type, void *context);
    IString *loadShaderSource(ShaderType type, const IString *path);
    IString *loadShaderSource(ShaderType type, const IModel *model, const IString *dir, void *context);
    IString *toUnicode(const uint8_t *value) const;
    bool hasExtension(const void *namePtr) const;
    void *findProcedureAddress(const void **candidatesPtr) const;
    void startProfileSession(ProfileType type, const void *arg);
    void stopProfileSession(ProfileType type, const void *arg);

    void setArchive(ArchiveSharedPtr value);
    void setSceneRef(Scene *value);
    void updateCameraMatrices(const QSizeF &size);
    void getCameraMatrices(glm::mat4 &world, glm::mat4 &view, glm::mat4 &projection);
    void setCameraModelMatrix(const glm::mat4 &value);
    void getLightMatrices(glm::mat4 &world, glm::mat4 &view, glm::mat4 &projection);
    void setLightMatrices(const glm::mat4 &world, const glm::mat4 &view, const glm::mat4 &projection);
    void setMousePosition(const Vector3 &value, bool pressed, MousePositionType type);
    void addModelPath(IModel *model, const QString &filename);
    const QString findModelPath(const IModel *model) const;
    const QString effectFilePath(const IModel *model, const IString *dir) const;
    const QString effectOwnerName(const IEffect *effect) const;
    void setEffectOwner(const IEffectSharedPtr &effect, IModel *model);
    void removeModel(IModel *model);
    void initialize(bool enableMSAA);

#ifdef VPVL2_ENABLE_NVIDIA_CG
    typedef QPair<QRegExp, IEffect *> EffectAttachment;
    struct OffscreenTexture {
        OffscreenTexture(const IEffect::OffscreenRenderTarget &r, const QList<EffectAttachment> &a)
            : renderTarget(r),
              attachments(a),
              textureID(cgGLGetTextureParameter(static_cast<CGparameter>(r.samplerParameter))),
              textureFormat(r.format)
        {
        }
        IEffect::OffscreenRenderTarget renderTarget;
        QList<EffectAttachment> attachments;
        GLuint textureID;
        GLenum textureFormat;
    };
    IModel *effectOwner(const IEffect *effect) const;
    IModel *findModel(const IString *name) const;
    void setRenderColorTargets(const void *targets, const int ntargets);
    FrameBufferObject *createFrameBufferObject();
    bool hasFrameBufferObjectBound() const;
    void bindOffscreenRenderTarget(const OffscreenTexture &rt, bool enableAA);
    void releaseOffscreenRenderTarget(const OffscreenTexture &rt, bool enableAA);
    void parseOffscreenSemantic(IEffect *effect, const QDir &dir);
    void renderOffscreen(const QSize &size);
    IModel *offscreenEffectOwner(const IEffect *effect) const;
    IEffectSharedPtr createEffectAsync(const IString *path);
    IEffectSharedPtr createEffectAsync(IModel *model, const IString *dir);
    const QList<OffscreenTexture> &offscreenTextures() const { return m_offscreenTextures; }
#endif

private:
    QImage createImageFromArchive(const QFileInfo &info);
    bool uploadTextureInternal(const QString &path,
                               InternalTexture &internalTexture,
                               void *context);
    bool uploadTextureNVTT(const QString &suffix,
                           const QString &path,
                           QScopedPointer<nv::Stream> &stream,
                           InternalTexture &internalTexture,
                           InternalContext *internalContext);
    bool generateTextureFromImage(const QImage &image,
                                  const QString &path,
                                  InternalTexture &internalTexture,
                                  InternalContext *internalContext);
    void getToonColorInternal(const QString &path, bool isSystem, Color &value, bool &ok);
    FrameBufferObject *findRenderTarget(const GLuint textureID, size_t width, size_t height, bool enableAA);

    Scene *m_sceneRef;
    const QHash<QString, QString> m_settings;
    const QDir m_systemDir;
    mutable QMutex m_model2PathLock;
    mutable QMutex m_effectOwnersLock;
    mutable QMutex m_effect2modelsLock;
    mutable QMutex m_effectCachesLock;
    ArchiveSharedPtr m_archive;
    QHash<const IModel *, QString> m_model2Paths;
    QHash<const QString, IModel *> m_filename2Models;
    QHash<GLuint, QString> m_texture2Paths;
    QHash<GLuint, QMovie *> m_texture2Movies;
    QHash<GLuint, FrameBufferObject *> m_renderTargets;
    QHash<const QString, IEffectSharedPtr> m_effectCaches;
    QHash<const IEffect *, QString> m_effectOwners;
    QHash<const IEffect *, IModel *> m_effect2models;
    QList<FrameBufferObject *> m_previousFrameBufferPtrs;
    QElapsedTimer m_timer;
    QSet<QString> m_loadableExtensions;
    QSet<QString> m_extensions;
    typedef QPair<IRenderContext::ProfileType, const void *> ProfilerKey;
    QHash<ProfilerKey, QElapsedTimer> m_profilerTimers;
    QString m_shaderSourcePrefix;
    glm::mat4 m_lightWorldMatrix;
    glm::mat4 m_lightViewMatrix;
    glm::mat4 m_lightProjectionMatrix;
    glm::mat4 m_cameraWorldMatrix;
    glm::mat4 m_cameraViewMatrix;
    glm::mat4 m_cameraProjectionMatrix;
    glm::vec4 m_mouseCursorPosition;
    glm::vec4 m_mouseLeftPressPosition;
    glm::vec4 m_mouseMiddlePressPosition;
    glm::vec4 m_mouseRightPressPosition;
    glm::vec2 m_viewport;
    int m_msaaSamples;
    bool m_frameBufferObjectBound;

#ifdef VPVL2_ENABLE_NVIDIA_CG
    QList<OffscreenTexture> m_offscreenTextures;
#endif

    VPVL2_DISABLE_COPY_AND_ASSIGN(RenderContext)
};

} /* namespace qt */
} /* namespace vpvl2 */

#endif /* VPVL2_QT_RENDERCONTEXT_H_ */
