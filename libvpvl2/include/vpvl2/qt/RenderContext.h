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
#ifndef VPVL2_QT_RENDERCONTEXT_H_
#define VPVL2_QT_RENDERCONTEXT_H_

#include <vpvl2/qt/Common.h>
#include <vpvl2/extensions/BaseRenderContext.h>

#include <QElapsedTimer>
#include <QSet>
#include <QSharedPointer>
#include <QString>

class QImage;
class QMovie;

namespace nv {
class Stream;
}

namespace vpvl2
{
namespace extensions
{
class Archive;
}

namespace qt
{
using namespace extensions;

typedef QSharedPointer<Archive> ArchiveSharedPtr;
typedef QSharedPointer<IEffect> IEffectSharedPtr;
typedef QSharedPointer<IModel> IModelSharedPtr;
typedef QSharedPointer<IMotion> IMotionSharedPtr;
typedef QSharedPointer<IRenderEngine> IRenderEnginePtr;

using namespace extensions;

class VPVL2_API RenderContext : public BaseRenderContext
{
public:
    static QSet<QString> loadableTextureExtensions();

    RenderContext(Scene *sceneRef, const StringMap *settingsRef);
    ~RenderContext();

    void *findProcedureAddress(const void **candidatesPtr) const;
    bool mapFile(const UnicodeString &path, MapBuffer *buffer) const;
    bool unmapFile(MapBuffer *buffer) const;
    bool existsFile(const UnicodeString &path) const;
    void removeModel(IModel *model);

#ifdef VPVL2_ENABLE_NVIDIA_CG
    void getToonColor(const IString *name, const IString *dir, Color &value, void *context);
    void getTime(float &value, bool sync) const;
    void getElapsed(float &value, bool sync) const;
    void uploadAnimatedTexture(float offset, float speed, float seek, void *texture);
#endif

private:
    static QString createQPath(const IString *dir, const IString *name);
    bool uploadTextureNVTT(const QString &suffix, const QString &path, QScopedPointer<nv::Stream> &stream,
                           Texture &texture, ModelContext *modelContext);
    bool uploadTextureInternal(const UnicodeString &path, Texture &texture, void *context);
    bool generateTextureFromImage(const QImage &image, const QString &path,
                                  Texture &texture, ModelContext *modelContext);
    void getToonColorInternal(const QString &path, bool isSystem, Color &value, bool &ok);
    QHash<GLuint, QSharedPointer<QMovie> > m_texture2Movies;
    QHash<GLuint, QString> m_texture2Paths;
    QElapsedTimer m_timer;

    VPVL2_DISABLE_COPY_AND_ASSIGN(RenderContext)
};

} /* namespace qt */
} /* namespace vpvl2 */

/* workaround for moc generated file */
#ifdef Q_MOC_OUTPUT_REVISION
using namespace vpvl2;
using namespace vpvl2::qt;
#endif

#endif /* VPVL2_QT_RENDERCONTEXT_H_ */
