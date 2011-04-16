/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2011  hkrn (libMMDAI)                         */
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

#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <QList>
#include <QRect>
#include <QString>
#include <QVariant>
#include <QtPlugin>

namespace MMDAI {

class SceneController;

} /* namespace */

class QMAPlugin : public QObject
{
public:
    QMAPlugin(QObject *parent = 0) : QObject(parent) {}
    virtual ~QMAPlugin() {}

    static const QString &getUpdateEvent() {
        static const QString kUpdateEvent = "MMDAI_GUI_UPDATE_EVENT";
        return kUpdateEvent;
    }
    static const QString &getPreRenderEvent() {
        static const QString kPreRenderEvent = "MMDAI_GUI_PRE_RENDER_EVENT";
        return kPreRenderEvent;
    }
    static const QString &getPostRenderEvent() {
        static const QString kPostRenderEvent = "MMDAI_GUI_POST_RENDER_EVENT";
        return kPostRenderEvent;
    }
    static bool isRenderEvent(const QString event) {
        return event.startsWith("MMDAI_GUI_");
    }
    static const QList<QVariant> &getEmptyArguments() {
        static const QList<QVariant> kEmptyArguments;
        return kEmptyArguments;
    }

    /* slots */
    virtual void load(MMDAI::SceneController *controller, const QString &baseName) = 0;
    virtual void unload() = 0;
    virtual void receiveCommand(const QString &command, const QList<QVariant> &arguments) = 0;
    virtual void receiveEvent(const QString &type, const QList<QVariant> &arguments) = 0;

    /* signals */
    virtual void commandPost(const QString &command, const QList<QVariant> &arguments) = 0;
    virtual void eventPost(const QString &type, const QList<QVariant> &arguments) = 0;

private:
    Q_DISABLE_COPY(QMAPlugin);
};

Q_DECLARE_INTERFACE(QMAPlugin, "com.github.hkrn.qma.PluginInterface/1.0")

#endif // PLUGININTERFACE_H
