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

#ifndef EXPORTVIDEODIALOG_H
#define EXPORTVIDEODIALOG_H

#include <QtGui/QDialog>

class MainWindow;
class QCheckBox;
class QSettings;
class QSpinBox;
class SceneWidget;

class ExportVideoDialog : public QDialog
{
    Q_OBJECT

public:
    ExportVideoDialog(MainWindow *parent, QSettings *settings, SceneWidget *scene);
    ~ExportVideoDialog();

    int sceneWidth() const;
    int sceneHeight() const;
    int fromIndex() const;
    int toIndex() const;
    bool includesGrid() const;

signals:
    void settingsDidSave();

private  slots:
    void saveSettings();

private:
    QSpinBox *m_widthBox;
    QSpinBox *m_heightBox;
    QSpinBox *m_fromIndexBox;
    QSpinBox *m_toIndexBox;
    QCheckBox *m_includeGridBox;
    QSettings *m_settings;

    Q_DISABLE_COPY(ExportVideoDialog)
};

#endif // EXPORTVIDEODIALOG_H
