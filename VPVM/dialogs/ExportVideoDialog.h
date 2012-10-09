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

#ifndef VPVM_EXPORTVIDEODIALOG_H
#define VPVM_EXPORTVIDEODIALOG_H

#include <QtGui/QDialog>

class QCheckBox;
class QComboBox;
class QLineEdit;
class QPushButton;
class QSettings;
class QSpinBox;

namespace vpvm
{

class MainWindow;
class SceneLoader;
class SceneWidget;

class ExportVideoDialog : public QDialog
{
    Q_OBJECT

public:
    ExportVideoDialog(SceneLoader *loader,
                      const QSize &min,
                      const QSize &max,
                      QSettings *settings);
    ~ExportVideoDialog();

    void setImageConfiguration(bool value);

    const QString backgroundAudio() const;
    int sceneWidth() const;
    int sceneHeight() const;
    int fromIndex() const;
    int toIndex() const;
    int videoBitrate() const;
    int sceneFPS() const;
    bool includesGrid() const;

signals:
    void settingsDidSave();

protected:
    void showEvent(QShowEvent *event);

private  slots:
    void openFileDialog();
    void saveSettings();

private:
    SceneLoader *m_loader;
    QSettings *m_settings;
    QLineEdit *m_pathEdit;
    QPushButton *m_openFileButton;
    QSpinBox *m_widthBox;
    QSpinBox *m_heightBox;
    QSpinBox *m_fromIndexBox;
    QSpinBox *m_toIndexBox;
    QSpinBox *m_videoBitrateBox;
    QComboBox *m_sceneFPSBox;
    QCheckBox *m_includeGridBox;

    Q_DISABLE_COPY(ExportVideoDialog)
};

} /* namespace vpvm */

#endif // EXPORTVIDEODIALOG_H
