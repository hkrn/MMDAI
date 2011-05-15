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

#ifndef QMAWINDOW_H
#define QMAWINDOW_H

#include <QtCore/QSettings>
#include <QtCore/QString>
#include <QtGui/QApplication>
#include <QtGui/QFileOpenEvent>
#include <QtGui/QMainWindow>

class QMALicenseWidget;
class QMAPreference;
class QMAScenePlayer;
class QMALogViewWidget;

class QMAApplication : public QApplication
{
    Q_OBJECT

public:
    QMAApplication(int &argc, char **argv) : QApplication(argc, argv) {}
    QMAApplication(int &argc, char **argv, bool GUIenabled) : QApplication(argc, argv, GUIenabled) {}
    QMAApplication(int &argc, char **argv, Type type) : QApplication(argc, argv, type) {}
#if defined(Q_WS_X11)
    QApplication(Display* dpy, Qt::HANDLE visual = 0, Qt::HANDLE cmap = 0) : QApplication(dpy, visual, cmap) {}
    QApplication(Display *dpy, int &argc, char **argv, Qt::HANDLE visual = 0, Qt::HANDLE cmap= 0) : QApplication(dpy, argc, argv, visual, cmap) {}
#endif

signals:
    void fileFound(const QString &filename);

protected:
    bool event(QEvent *event)
    {
        if (event->type() == QEvent::FileOpen) {
            QFileOpenEvent *foe = static_cast<QFileOpenEvent*>(event);
            emit fileFound(foe->file());
        }
        return QApplication::event(event);
    }
};

class QMAWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit QMAWindow(QWidget *parent = 0);
    ~QMAWindow();

    void initialize();
    void loadPlugins();
    void start();

public slots:
    void reload(const QString &filename);

protected:
    void keyPressEvent(QKeyEvent *event);
    void closeEvent(QCloseEvent *event);

private slots:
    void toggleFullScreen();
    void showLogWindow();
    void resizeScene();
    void about();
    void receiveEvent(const QString &type, const QList<QVariant> &arguments);

private:
    void setEdgeThin(float value);
    void createActions();
    void createMenu(QMenuBar *menubar);
    void mergeMenu();
    void readSetting();
    void writeSetting();

    QMenuBar *m_menuBar;
    QSettings m_settings;
    QMAPreference *m_preference;
    QMAScenePlayer *m_scene;
    QMALogViewWidget *m_logWidget;
    QMALicenseWidget *m_licenseWidget;
    QHash<QString, QMenu*> m_menu;
    QAction *m_showLogAction;
    QAction *m_toggleFullScreenAction;
    QAction *m_resize640x480Action;
    QAction *m_resize800x480Action;
    QAction *m_resize1024x768Action;
    QAction *m_resize1280x800Action;
    QAction *m_exitAction;
    QAction *m_aboutAction;
    QAction *m_aboutQtAction;

    bool m_isFullScreen;

    Q_DISABLE_COPY(QMAWindow)
};

#endif // QMAWINDOW_H
