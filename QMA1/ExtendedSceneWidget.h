/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2012  hkrn                                    */
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

#ifndef EXTENDEDSCENEWIDGET_H
#define EXTENDEDSCENEWIDGET_H

#include "SceneWidget.h"

class Script;
class TiledStage;

class ExtendedSceneWidget : public SceneWidget
{
    Q_OBJECT

public:
    explicit ExtendedSceneWidget(QSettings *settings, QWidget *parent = 0);
    ~ExtendedSceneWidget();

    TiledStage *tiledStage() const { return m_tiledStage; }
    Script *script() const { return m_script; }
    bool isTransparentEnabled() const { return m_enableTransparent; }

public slots:
    void clear();
    void loadScript();
    void loadScript(const QString &filename);
    void setEmptyMotion(vpvl2::IModel *model);
    void setTransparentEnable(bool value) { m_enableTransparent = value; }

signals:
    void scriptDidLoaded(const QString &filename);
    void motionDidFinished(const QList<vpvl2::IMotion *> &motions);

protected:
    void dropEvent(QDropEvent *event);
    void initializeGL();
    void paintGL();

private:
    Script *m_script;
    TiledStage *m_tiledStage;
    bool m_enableTransparent;

    Q_DISABLE_COPY(ExtendedSceneWidget)
};

#endif // SCENEWIDGET_H
