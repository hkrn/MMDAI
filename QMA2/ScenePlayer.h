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

#ifndef SCENEPLAYER_H
#define SCENEPLAYER_H

#include <QtCore/QElapsedTimer>
#include <QtCore/QTimer>
#include <QtGui/QProgressDialog>

namespace vpvl {
class PMDModel;
}

class AudioPlayer;
class SceneWidget;
class PlaySettingDialog;

class ScenePlayer : public QObject
{
    Q_OBJECT

public:
    ScenePlayer(SceneWidget *sceneWidget, PlaySettingDialog *dialog);
    ~ScenePlayer();

    void start();
    bool isActive() const;

public slots:
    void stop();

signals:
    void renderFrameDidStart();
    void renderFrameDidStop();
    /* motionDidSeek は int 型な点に注意 (他は float 型) */
    void motionDidSeek(int frameIndex);

private slots:
    void renderSceneFrameFixed();
    void renderSceneFrameVariant();
    void advanceAudioFrame(float step);

private:
    void renderSceneFrame0(float step);

    QElapsedTimer m_elapsed;
    QTimer m_renderTimer;
    SceneWidget *m_sceneWidget;
    PlaySettingDialog *m_dialog;
    QProgressDialog *m_progress;
    QString m_format;
    QByteArray m_buffer;
    AudioPlayer *m_player;
    vpvl::PMDModel *m_selected;
    float m_prevFrameIndex;
    float m_frameStep;
    float m_totalStep;
    float m_audioFrameIndex;
    float m_prevAudioFrameIndex;
    int m_countForFPS;
    int m_currentFPS;
    int m_prevSceneFPS;
};

#endif // SCENEPLAYER_H
