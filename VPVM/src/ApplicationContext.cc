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

#include "ApplicationContext.h"

#include <QtCore>
#include <QOpenGLContext>
#include <QQuickWindow>
#include <vpvl2/vpvl2.h>
#include <vpvl2/gl/Texture2D.h>
#include <vpvl2/extensions/qt/String.h>
#include "ModelProxy.h"
#include "ProjectProxy.h"

using namespace vpvl2;
using namespace vpvl2::extensions;
using namespace vpvl2::extensions::qt;

namespace {

struct Resolver : IApplicationContext::FunctionResolver {
    bool hasExtension(const char *name) const {
        if (const bool *ptr = supportedExtensionsCache.find(name)) {
            return *ptr;
        }
        else if (coreProfile) {
            GLint nextensions;
            glGetIntegerv(kGL_NUM_EXTENSIONS, &nextensions);
            const std::string &needle = std::string("GL_") + name;
            typedef const GLubyte * (GLAPIENTRY * PFNGLGETSTRINGIPROC) (gl::GLenum pname, gl::GLuint index);
            PFNGLGETSTRINGIPROC getStringi = reinterpret_cast<PFNGLGETSTRINGIPROC>(resolveSymbol("glGetStringi"));
            for (int i = 0; i < nextensions; i++) {
                if (needle == reinterpret_cast<const char *>(getStringi(GL_EXTENSIONS, i))) {
                    supportedExtensionsCache.insert(name, true);
                    return true;
                }
            }
            supportedExtensionsCache.insert(name, false);
            return false;
        }
        else if (const char *extensions = reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS))) {
            bool found = strstr(extensions, name) != NULL;
            supportedExtensionsCache.insert(name, found);
            return found;
        }
        else {
            supportedExtensionsCache.insert(name, false);
            return false;
        }
    }
    void *resolveSymbol(const char *name) const {
        if (void *const *ptr = addressesCache.find(name)) {
            return *ptr;
        }
        else {
#ifdef Q_OS_WIN32
            void *address = reinterpret_cast<void *>(m_library.resolve(name));
#else
            void *address = reinterpret_cast<void *>(QOpenGLContext::currentContext()->getProcAddress(name));
#endif
            addressesCache.insert(name, address);
            return address;
        }
    }
    int query(QueryType type) const {
        switch (type) {
        case kQueryVersion: {
            return gl::makeVersion(reinterpret_cast<const char *>(glGetString(GL_VERSION)));
        }
        case kQueryShaderVersion: {
            return gl::makeVersion(reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION)));
        }
        case kQueryCoreProfile: {
            return coreProfile;
        }
        default:
            return 0;
        }
    }

#ifdef Q_OS_WIN32
    Resolver()
        : coreProfile(false)
    {
#ifdef QT_NO_DEBUG
        m_library.setFileName("libGLESv2");
#else
        m_library.setFileName("libGLESv2d");
#endif
        m_library.load();
        Q_ASSERT(m_library.isLoaded());
    }
    mutable QLibrary m_library;
#else
    Resolver()
        : coreProfile(false)
    {
    }
#endif
    ~Resolver() {}
    static const GLenum kGL_NUM_EXTENSIONS = 0x821D;
    mutable Hash<HashString, bool> supportedExtensionsCache;
    mutable Hash<HashString, void *> addressesCache;
    bool coreProfile;
};
Q_GLOBAL_STATIC(Resolver, g_functionResolverInstance)

}

ApplicationContext::ApplicationContext(const ProjectProxy *proxy, const StringMap *stringMap, bool isCoreProfile)
    : BaseApplicationContext(proxy->projectInstanceRef(), proxy->encodingInstanceRef(), stringMap),
      m_baseTime(QTime::currentTime()),
      m_orderIndex(1)
{
    g_functionResolverInstance->coreProfile = isCoreProfile;
}

ApplicationContext::~ApplicationContext()
{
}

void *ApplicationContext::findProcedureAddress(const void **candidatesPtr) const
{
    const char **names = reinterpret_cast<const char **>(candidatesPtr);
    QOpenGLContext *context = QOpenGLContext::currentContext();
    while (const char *nameRef = *names) {
        QFunctionPointer function = context->getProcAddress(nameRef);
        if (function) {
            return reinterpret_cast<void *>(function);
        }
        names++;
    }
    return 0;
}

void ApplicationContext::getToonColor(const IString * /* name */, Color & /* value */, void * /* userData */)
{
}

void ApplicationContext::getTime(float32 &value, bool sync) const
{
    if (sync) {
        value = m_sceneRef->currentTimeIndex() / Scene::defaultFPS();
    }
    else {
        value = m_baseTime.msecsTo(QTime::currentTime()) / 1000.0f;
    }
}

void ApplicationContext::getElapsed(float32 &value, bool sync) const
{
    if (sync) {
        value = m_sceneRef->currentTimeIndex() / Scene::defaultFPS();
    }
    else {
        const QTime &currentTime = QTime::currentTime();
        value = (m_elapsedTime.isNull() ? 0 : m_elapsedTime.msecsTo(currentTime)) / 1000.0f;
        m_elapsedTime = currentTime;
    }
}

void ApplicationContext::uploadAnimatedTexture(float32 /* offset */, float32 /* speed */, float32 /* seek */, void * /* texture */)
{
}

bool ApplicationContext::mapFile(const std::string &path, MapBuffer *buffer) const
{
    QScopedPointer<QFile> file(new QFile(QString::fromStdString(path)));
    if (file->open(QFile::ReadOnly | QFile::Unbuffered)) {
        bool ok = true;
        size_t size = 0;
#ifdef VPVL2_USE_MMAP
        size = file->size();
        buffer->address = file->map(0, size);
        ok = buffer->address != 0;
#else
        const QByteArray &bytes = file->readAll();
        size = bytes.size();
        buffer->address = new uint8_t[size];
        memcpy(buffer->address, bytes.constData(), size);
#endif
        buffer->size = size;
        buffer->opaque = reinterpret_cast<intptr_t>(file.take());
        return ok;
    }
    VPVL2_LOG(WARNING, "Cannot load " << qPrintable(file->fileName()) << ": " << qPrintable(file->errorString()));
    return false;
}

bool ApplicationContext::unmapFile(MapBuffer *buffer) const
{
    if (QFile *file = reinterpret_cast<QFile *>(buffer->opaque)) {
#ifdef VPVL2_USE_MMAP
        file->unmap(buffer->address);
#else
        delete[] buffer->address;
#endif
        file->close();
        delete file;
        return true;
    }
    return false;
}

bool ApplicationContext::existsFile(const std::string &path) const
{
    return QFile::exists(QString::fromStdString(path));
}

bool ApplicationContext::extractFilePath(const std::string &path, std::string &fileName, std::string &basename) const
{
    QFileInfo finfo(QString::fromStdString(path));
    fileName = finfo.fileName().toStdString();
    basename = finfo.baseName().toStdString();
    return true;
}

bool ApplicationContext::extractModelNameFromFileName(const std::string &path, std::string &modelName) const
{
    QRegExp regexp("");
    if (regexp.indexIn(QString::fromStdString(path))) {
        modelName = regexp.cap(1).toStdString();
        return true;
    }
    return false;
}

bool ApplicationContext::uploadTextureOpaque(const uint8 *data, vsize size, const std::string &key, int flags, ModelContext *context, ITexture *&texturePtr)
{
    QImage image;
    image.loadFromData(data, size);
    if (!uploadTextureQt(image.convertToFormat(QImage::Format_ARGB32), key, flags, context, texturePtr)) {
        return context->uploadTexture(data, size, key, flags, texturePtr);
    }
    return true;
}

bool ApplicationContext::uploadTextureOpaque(const std::string &path, int flags, ModelContext *context, ITexture *&texturePtr)
{
    QImage image;
    image.load(QString::fromStdString(path));
    if (!uploadTextureQt(image.convertToFormat(QImage::Format_ARGB32), path, flags, context, texturePtr)) {
        return context->uploadTexture(path, flags, texturePtr);
    }
    return true;
}

IApplicationContext::FunctionResolver *ApplicationContext::sharedFunctionResolverInstance() const
{
    return g_functionResolverInstance;
}

bool ApplicationContext::uploadTextureQt(const QImage &image, const std::string &key, int flags, ModelContext *modelContext, ITexture *&texturePtr)
{
    /* use Qt's pluggable image loader (jpg/png is loaded with libjpeg/libpng) */
    if (!image.isNull()) {
        const Vector3 size(image.width(), image.height(), 1);
        static const gl::BaseSurface::Format kFormat(gl::kGL_BGRA, gl::kGL_RGBA8, gl::kGL_UNSIGNED_INT_8_8_8_8_REV, gl::Texture2D::kGL_TEXTURE_2D);
        texturePtr = modelContext->createTexture(image.constBits(), kFormat, size, (flags & kGenerateTextureMipmap) != 0);
        VPVL2_VLOG(2, "Created a texture: texture=" << texturePtr);
        return modelContext->storeTexture(key, flags, texturePtr);
    }
    else {
        VPVL2_LOG(WARNING, "Cannot load an image texture with QImage: " << key);
        return false;
    }
}

void ApplicationContext::uploadEnqueuedModelProxies(ProjectProxy *projectProxy, QList<ModelProxyPair> &succeededModelProxies, QList<ModelProxyPair> &failedModelProxies)
{
    XMLProject *projectRef = projectProxy->projectInstanceRef();
    succeededModelProxies.clear();
    failedModelProxies.clear();
    while (!m_uploadingModels.isEmpty()) {
        const ModelProxyPair &pair = m_uploadingModels.dequeue();
        ModelProxy *modelProxy = pair.first;
        const QFileInfo fileInfo(modelProxy->fileUrl().toLocalFile());
        const String dir(fileInfo.absoluteDir().absolutePath());
        ModelContext context(this, 0, &dir);
        IModel *modelRef = modelProxy->data();
        IRenderEngineSmartPtr engine(projectRef->createRenderEngine(this, modelRef, Scene::kEffectCapable));
        engine->setUpdateOptions(IRenderEngine::kParallelUpdate);
        IEffect *effectRef = 0;
        engine->setEffect(effectRef, IEffect::kAutoDetection, &context);
        if (engine->upload(&context)) {
            parseOffscreenSemantic(effectRef, &dir);
            modelRef->setEdgeWidth(1.0f);
            const XMLProject::UUID &uuid = modelProxy->uuid().toString().toStdString();
            /* remove model reference from project first to add model/engine correctly after loading project */
            projectRef->removeModel(modelRef);
            projectRef->addModel(modelRef, engine.release(), uuid, m_orderIndex++);
            projectProxy->setModelSetting(modelProxy, QString::fromStdString(XMLProject::kSettingNameKey), modelProxy->name());
            projectProxy->setModelSetting(modelProxy, QString::fromStdString(XMLProject::kSettingURIKey), modelProxy->fileUrl().toLocalFile());
            if (!pair.second) {
                projectProxy->setModelSetting(modelProxy, "selected", false);
            }
            addModelFilePath(modelRef, fileInfo.absoluteFilePath().toStdString());
            setEffectModelRef(effectRef, modelRef);
            succeededModelProxies.append(pair);
        }
        else {
            failedModelProxies.append(pair);
        }
    }
}

void ApplicationContext::uploadEnqueuedEffects(ProjectProxy *projectProxy, QList<ModelProxy *> &succeededEffects, QList<ModelProxy *> &failedEffects)
{
    XMLProject *projectRef = projectProxy->projectInstanceRef();
    succeededEffects.clear();
    failedEffects.clear();
    while (!m_uploadingEffects.isEmpty()) {
        ModelProxy *modelProxy = m_uploadingEffects.dequeue();
        const QFileInfo fileInfo(modelProxy->fileUrl().toLocalFile());
        if (IEffect *effectRef = createEffectRef(fileInfo.absoluteFilePath().toStdString())) {
            const String dir(fileInfo.absoluteDir().absolutePath());
            ModelContext context(this, 0, &dir);
            IModel *modelRef = modelProxy->data();
            IRenderEngineSmartPtr engine(projectRef->createRenderEngine(this, modelRef, Scene::kEffectCapable));
            engine->setEffect(effectRef, IEffect::kAutoDetection, &context);
            modelRef->setName(effectRef->name(), IEncoding::kDefaultLanguage);
            parseOffscreenSemantic(effectRef, &dir);
            createEffectParameterUIWidgets(effectRef);
            const XMLProject::UUID &uuid = modelProxy->uuid().toString().toStdString();
            /* remove model reference from project first to add model/engine correctly after loading project */
            projectRef->removeModel(modelRef);
            projectRef->addModel(modelRef, engine.release(), uuid, m_orderIndex++);
            addModelFilePath(modelRef, fileInfo.absoluteFilePath().toStdString());
            succeededEffects.append(modelProxy);
        }
        else {
            projectProxy->deleteModel(modelProxy);
            failedEffects.append(modelProxy);
        }
    }
}

QList<ModelProxy *> ApplicationContext::deleteEnqueuedModelProxies(ProjectProxy *projectProxy)
{
    QList<ModelProxy *> deletedModelProxies;
    XMLProject *projectRef = projectProxy->projectInstanceRef();
    while (!m_deletingModels.isEmpty()) {
        ModelProxy *modelProxy = m_deletingModels.dequeue();
        IModel *modelRef = modelProxy->data();
        /* Failed loading effect will have null IRenderEngine instance case  */
        if (IRenderEngine *engine = projectRef->findRenderEngine(modelRef)) {
            projectRef->removeModel(modelRef);
            engine->release();
            delete engine;
        }
        else {
            projectRef->removeModel(modelRef);
        }
        deletedModelProxies.append(modelProxy);
    }
    return deletedModelProxies;
}

void ApplicationContext::enqueueUploadingModel(ModelProxy *model, bool isProject)
{
    m_uploadingModels.enqueue(ModelProxyPair(model, isProject));
}

void ApplicationContext::enqueueUploadingEffect(ModelProxy *model)
{
    m_uploadingEffects.enqueue(model);
}

void ApplicationContext::enqueueDeletingModelProxy(ModelProxy *model)
{
    m_deletingModels.enqueue(model);
}

void ApplicationContext::resetOrderIndex(int startOrderIndex)
{
    m_orderIndex = startOrderIndex;
}

QOpenGLFramebufferObjectFormat ApplicationContext::framebufferObjectFormat(const QQuickWindow *win)
{
    QOpenGLFramebufferObjectFormat format;
    format.setAttachment(QOpenGLFramebufferObject::CombinedDepthStencil);
    format.setSamples(win->format().samples());
    return format;
}
