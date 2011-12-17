/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2011  hkrn                                    */
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

#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QtGui/QTabWidget>

namespace Ui {
    class TabWidget;
}

class AssetWidget;
class BoneMotionModel;
class CameraPerspectiveWidget;
class FaceMotionModel;
class FaceWidget;
class InterpolationWidget;
class QSettings;
class SceneMotionModel;

class TabWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TabWidget(QSettings *settings,
                       BoneMotionModel *bmm,
                       FaceMotionModel *fmm,
                       SceneMotionModel *smm,
                       QWidget *parent = 0);
    ~TabWidget();

    AssetWidget *assetWidget() const { return m_asset; }
    CameraPerspectiveWidget *cameraPerspectiveWidget() const { return m_camera; }
    FaceWidget *faceWidget() const { return m_face; }
    InterpolationWidget *interpolationWidget() const { return m_interpolation; }

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void retranslate();

private:
    QTabWidget *m_tabWidget;
    QSettings *m_settings;
    AssetWidget *m_asset;
    CameraPerspectiveWidget *m_camera;
    FaceWidget *m_face;
    InterpolationWidget *m_interpolation;

    Q_DISABLE_COPY(TabWidget)
};

#endif // TABWIDGET_H
