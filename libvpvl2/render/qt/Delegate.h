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

#ifndef VPVL2_RENDER_QT_DELEGATE_H_
#define VPVL2_RENDER_QT_DELEGATE_H_

#include <vpvl2/IRenderDelegate.h>

#include <QtOpenGL/QtOpenGL>

namespace vpvl2
{
class Scene;

namespace render
{
namespace qt
{

class Delegate : public IRenderDelegate, protected QGLFunctions
{
public:
    struct TextureCache {
        int width;
        int height;
        GLuint id;
    };
    struct PrivateContext {
        QHash<QString, TextureCache> textureCache;
    };

    static QString readAllAsync(const QString &path);
    static QImage loadImageAsync(const QString &path);

    Delegate(const QSettings *settings, const Scene *scene, QGLWidget *context);
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
    IModel *offscreenEffectOwner(const IEffect *effect) const;
    void setRenderTarget(const void *targets, const int ntargets);
    void bindRenderTarget(void *texture, size_t width, size_t height, bool enableAA);
    void bindRenderDepthStencilTarget(void *texture, void *depth, void *stencil, size_t width, size_t height, bool enableAA);
    void releaseRenderTarget(void *texture, size_t width, size_t height, bool enableAA);
    void releaseRenderDepthStencilTarget(void *texture, void *depth, void *stencil, size_t width, size_t height, bool enableAA);

    void updateMatrices(const QSize &size);
    void setCameraModelMatrix(const QMatrix4x4 &value);
    void setLightMatrices(const QMatrix4x4 &world, const QMatrix4x4 &view, const QMatrix4x4 &projection);
    void setMousePosition(const Vector3 &value, bool pressed, MousePositionType type);
    void addModelPath(const IModel *model, const QString &filename);
    const QString findModelPath(const IModel *model) const;
    const QString effectFilePath(const IModel *model, const IString *dir) const;
    IModel *effectOwner(const IEffect *effect) const;
    const QString effectOwnerName(const IEffect *effect) const;
    void setEffectOwner(const IEffect *effect, IModel *model);
    void createRenderTargets();
    void bindOffscreenRenderTarget(GLuint textureID, size_t width, size_t height, bool enableAA);
    void releaseOffscreenRenderTarget(GLuint textureID, size_t width, size_t height, bool enableAA);

private:
    class FrameBufferObject;

    static const QString createPath(const IString *dir, const QString &name);
    static const QString createPath(const IString *dir, const IString *name);
    static void setTextureID(const TextureCache &cache, bool isToon, Texture &output);
    static void addTextureCache(PrivateContext *context, const QString &path, const TextureCache &texture);
    bool uploadTextureInternal(const QString &path, Texture &texture, bool isToon, bool mipmap, void *context);
    void getToonColorInternal(const QString &path, Color &value);
    FrameBufferObject *findRenderTarget(const GLuint textureID, size_t width, size_t height);

    const QSettings *m_settings;
    const Scene *m_scene;
    const QDir m_systemDir;
    mutable QMutex m_model2PathLock;
    mutable QMutex m_effectOwnersLock;
    mutable QMutex m_effect2modelsLock;
    QGLWidget *m_context;
    QSize m_viewport;
    QHash<const IModel *, QString> m_model2Paths;
    QHash<GLuint, QString> m_texture2Paths;
    QHash<GLuint, QMovie *> m_texture2Movies;
    QHash<GLuint, FrameBufferObject *> m_renderTargets;
    QHash<const IEffect *, QString> m_effectOwners;
    QHash<const IEffect *, IModel *> m_effect2models;
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
    GLuint m_previousTextureID;
    int m_msaaSamples;
};

}
}
}

#endif
