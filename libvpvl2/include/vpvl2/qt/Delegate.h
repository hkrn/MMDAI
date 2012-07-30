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

#ifndef VPVL2_QT_DELEGATE_H_
#define VPVL2_QT_DELEGATE_H_

#include "vpvl2/Common.h"
#include "vpvl2/IEffect.h"
#include "vpvl2/IRenderDelegate.h"

#include <QtOpenGL/QtOpenGL>

namespace vpvl2
{
class Scene;

namespace qt
{
class Archive;

class Delegate : public IRenderDelegate, protected QGLFunctions
{
public:
    struct TextureCache {
        TextureCache() {}
        TextureCache(int w, int h, GLuint i)
            : width(w),
              height(h),
              id(i)
        {
        }
        int width;
        int height;
        GLuint id;
    };
    struct PrivateContext {
        QHash<QString, TextureCache> textureCache;
    };
    typedef QPair<QRegExp, vpvl2::IEffect *> EffectAttachment;
    typedef struct {
        IEffect::OffscreenRenderTarget renderTarget;
        QList<EffectAttachment> attachments;
        GLuint textureID;
    } OffscreenRenderTarget;

    static QString readAllAsync(const QString &path);
    static QImage loadImageAsync(const QString &path);

    Delegate(const QHash<QString, QString> &settings, Scene *scene, QGLWidget *context);
    ~Delegate();

    void allocateContext(const IModel *model, void *&context);
    void releaseContext(const IModel *model, void *&context);
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
    IModel *effectOwner(const IEffect *effect) const;
    IModel *findModel(const IString *name) const;
    void setRenderColorTargets(const void *targets, const int ntargets);
    void bindRenderColorTarget(void *texture, size_t width, size_t height, int index, bool enableAA);
    void bindRenderDepthStencilTarget(void *texture, void *depth, void *stencil, size_t width, size_t height, bool enableAA);
    void releaseRenderColorTarget(void *texture, size_t width, size_t height, int index, bool enableAA);
    void releaseRenderDepthStencilTarget(void *texture, void *depth, void *stencil, size_t width, size_t height, bool enableAA);

    void setArchive(Archive *value);
    void setScenePtr(Scene *value);
    void updateMatrices(const QSizeF &size);
    void getCameraMatrices(QMatrix4x4 &world, QMatrix4x4 &view, QMatrix4x4 &projection);
    void setCameraModelMatrix(const QMatrix4x4 &value);
    void getLightMatrices(QMatrix4x4 &world, QMatrix4x4 &view, QMatrix4x4 &projection);
    void setLightMatrices(const QMatrix4x4 &world, const QMatrix4x4 &view, const QMatrix4x4 &projection);
    void setMousePosition(const Vector3 &value, bool pressed, MousePositionType type);
    void addModelPath(IModel *model, const QString &filename);
    const QString findModelPath(const IModel *model) const;
    const QString effectFilePath(const IModel *model, const IString *dir) const;
    const QString effectOwnerName(const IEffect *effect) const;
    void setEffectOwner(const IEffect *effect, IModel *model);
    void createRenderTargets();
    void bindOffscreenRenderTarget(GLuint textureID, size_t width, size_t height, bool enableAA);
    void releaseOffscreenRenderTarget(GLuint textureID, size_t width, size_t height, bool enableAA);
    void parseOffscreenSemantic(IEffect *effect, const QDir &dir);
    IModel *offscreenEffectOwner(const IEffect *effect) const;
    IEffect *createEffectAsync(const IString *path);
    IEffect *createEffectAsync(IModel *model, const IString *dir);

    const QList<OffscreenRenderTarget> &offscreenRenderTargets() const { return m_offscreens; }

private:
    class FrameBufferObject;

    static const QString createPath(const IString *dir, const QString &name);
    static const QString createPath(const IString *dir, const IString *name);
    static void setTextureID(const TextureCache &cache, bool isToon, Texture &output);
    static void addTextureCache(PrivateContext *context, const QString &path, const TextureCache &texture);
    static QImage loadTGA(const QString &path, QScopedArrayPointer<uint8_t> &dataPtr);
    static QImage loadTGA(QByteArray data, QScopedArrayPointer<uint8_t> &dataPtr);
    static QGLContext::BindOptions textureBindOptions(bool enableMipmap);
    bool uploadTextureInternal(const QString &path, Texture &texture, bool isToon, bool mipmap, void *context);
    void getToonColorInternal(const QString &path, Color &value);
    FrameBufferObject *findRenderTarget(const GLuint textureID, size_t width, size_t height);

    const QHash<QString, QString> m_settings;
    const QDir m_systemDir;
    Scene *m_scene;
    mutable QMutex m_model2PathLock;
    mutable QMutex m_effectOwnersLock;
    mutable QMutex m_effect2modelsLock;
    mutable QMutex m_effectCachesLock;
    QGLWidget *m_context;
    Archive *m_archive;
    QSizeF m_viewport;
    QList<OffscreenRenderTarget> m_offscreens;
    QHash<const IModel *, QString> m_model2Paths;
    QHash<const QString, IModel *> m_filename2Models;
    QHash<GLuint, QString> m_texture2Paths;
    QHash<GLuint, QMovie *> m_texture2Movies;
    QHash<GLuint, FrameBufferObject *> m_renderTargets;
    QHash<const QString, IEffect *> m_effectCaches;
    QHash<const IEffect *, QString> m_effectOwners;
    QHash<const IEffect *, IModel *> m_effect2models;
    QList<FrameBufferObject *> m_previousFrameBufferPtrs;
    QMatrix4x4 m_lightWorldMatrix;
    QMatrix4x4 m_lightViewMatrix;
    QMatrix4x4 m_lightProjectionMatrix;
    QMatrix4x4 m_cameraModelMatrix;
    QMatrix4x4 m_cameraViewMatrix;
    QMatrix4x4 m_cameraProjectionMatrix;
    QElapsedTimer m_timer;
    Vector4 m_mouseCursorPosition;
    Vector4 m_mouseLeftPressPosition;
    Vector4 m_mouseMiddlePressPosition;
    Vector4 m_mouseRightPressPosition;
    int m_msaaSamples;

    VPVL2_DISABLE_COPY_AND_ASSIGN(Delegate)
};

}
}

#endif
