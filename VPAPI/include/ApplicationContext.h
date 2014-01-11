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
#ifndef APPLICATIONCONTEXT_H_
#define APPLICATIONCONTEXT_H_

#include <QFileSystemWatcher>
#include <QOpenGLFramebufferObject>
#include <QQueue>
#include <QTime>
#include <vpvl2/extensions/BaseApplicationContext.h>

class ModelProxy;
class ProjectProxy;
class QQuickWindow;

namespace vpvl2 {
namespace extensions {
class StringMap;
}
}

class ApplicationContext : public QObject, public vpvl2::extensions::BaseApplicationContext
{
    Q_OBJECT

public:
    typedef QPair<ModelProxy *, bool> ModelProxyPair;
    static QOpenGLFramebufferObjectFormat framebufferObjectFormat(const QQuickWindow *win);

    ApplicationContext(const ProjectProxy *proxy, const vpvl2::extensions::StringMap *stringMap, bool isCoreProfile);
    ~ApplicationContext();

    void *findProcedureAddress(const void **candidatesPtr) const;
    void getToonColor(const vpvl2::IString * /* name */, vpvl2::Color & /* value */, void * /* userData */);
    void getTime(vpvl2::float32 &value, bool sync) const;
    void getElapsed(vpvl2::float32 &value, bool sync) const;
    void uploadAnimatedTexture(vpvl2::float32 /* offset */, vpvl2::float32 /* speed */, vpvl2::float32 /* seek */, void * /* texture */);
    bool mapFile(const std::string &path, MapBuffer *buffer) const;
    bool unmapFile(MapBuffer *buffer) const;
    bool existsFile(const std::string &path) const;
    bool extractFilePath(const std::string &path, std::string &fileName, std::string &basename) const;
    bool extractModelNameFromFileName(const std::string &path, std::string &modelName) const;
    bool uploadTextureOpaque(const vpvl2::uint8 *data, vpvl2::vsize size, const std::string &key, int flags, ModelContext *context, vpvl2::ITexture *&texturePtr);
    bool uploadTextureOpaque(const std::string &path, int flags, ModelContext *context, vpvl2::ITexture *&texturePtr);
    FunctionResolver *sharedFunctionResolverInstance() const;
#ifdef QT_OPENGL_ES_2
    vpvl2::gl::BaseSurface::Format defaultTextureFormat() const;
#endif

    bool uploadTextureQt(const QImage &image, const std::string &key, int flags, ModelContext *modelContext, vpvl2::ITexture *&texturePtr);
    void uploadEnqueuedModelProxies(ProjectProxy *projectProxy, QList<ModelProxyPair> &succeededModelProxies, QList<ModelProxyPair> &failedModelProxies);
    void uploadEnqueuedEffects(ProjectProxy *projectProxy, QList<ModelProxy *> &succeededEffects, QList<ModelProxy *> &failedEffects);
    QList<ModelProxy *> deleteEnqueuedModelProxies(ProjectProxy *projectProxy);
    void deleteAllModelProxies(ProjectProxy *projectProxy);
    void enqueueUploadingModel(ModelProxy *model, bool isProject);
    void enqueueUploadingEffect(ModelProxy *model);
    void enqueueDeletingModelProxy(ModelProxy *model);
    void resetOrderIndex(int startOrderIndex);
    void reloadTexture(const QString &filePath);
    void reloadEffect(const QString &filePath);
    void reloadFile(const QString &filePath);

signals:
    void fileDidChange(const QString &filePath);

private:
    void addTextureWatch(const vpvl2::IModel *modelRef, const ModelContext &context);
    void removeTextureWatch(const vpvl2::IModel *modelRef);
    void deleteModelProxy(ModelProxy *modelProxy, ProjectProxy *projectProxyRef);

    QFileSystemWatcher m_fileSystemWatcher;
    QHash<const vpvl2::IModel *, BaseApplicationContext::ModelContext::TextureRefCacheMap> m_textureCacheRefs;
    QHash<const QString, vpvl2::ITexture *> m_filePath2TextureRefs;
    QHash<const QString, vpvl2::IEffect *> m_filePath2EffectRefs;
    QQueue<ModelProxyPair> m_uploadingModels;
    QQueue<ModelProxy *> m_uploadingEffects;
    QQueue<ModelProxy *> m_deletingModels;
    mutable QTime m_elapsedTime;
    QTime m_baseTime;
    int m_orderIndex;
};

#endif
