#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtCore/QSettings>
#include <QtGui/QMainWindow>

namespace vpvl {
class Asset;
class PMDModel;
}

class LicenseWidget;
class SceneWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

    bool validateLibraryVersion();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void selectCurrentModel();
    void setCurrentModel(vpvl::PMDModel *value);
    void addModel(vpvl::PMDModel *model);
    void deleteModel(vpvl::PMDModel *model);
    void addAsset(vpvl::Asset *asset);
    void deleteAsset(vpvl::Asset *asset);
    void updateFPS(int fps);

private:
    void startSceneUpdate();
    void stopSceneUpdate();
    const QString buildWindowTitle();
    const QString buildWindowTitle(int fps);
    void connectWidgets();
    void updateInformation();
    void buildMenuBar();
    void retranslate();

    QSettings m_settings;
    LicenseWidget *m_licenseWidget;
    SceneWidget *m_sceneWidget;
    vpvl::PMDModel *m_model;
    int m_currentFPS;

    QAction *m_actionAddModel;
    QAction *m_actionAddAsset;
    QAction *m_actionInsertToAllModels;
    QAction *m_actionInsertToSelectedModel;
    QAction *m_actionSetCamera;
    QAction *m_actionExit;
    QAction *m_actionAbout;
    QAction *m_actionAboutQt;
    QAction *m_actionPlay;
    QAction *m_actionPause;
    QAction *m_actionStop;
    QAction *m_actionZoomIn;
    QAction *m_actionZoomOut;
    QAction *m_actionRotateUp;
    QAction *m_actionRotateDown;
    QAction *m_actionRotateLeft;
    QAction *m_actionRotateRight;
    QAction *m_actionTranslateUp;
    QAction *m_actionTranslateDown;
    QAction *m_actionTranslateLeft;
    QAction *m_actionTranslateRight;
    QAction *m_actionResetCamera;
    QAction *m_actionRevertSelectedModel;
    QAction *m_actionDeleteSelectedModel;
    QAction *m_actionShowBones;
    QMenuBar *m_menuBar;
    QMenu *m_menuFile;
    QMenu *m_menuScene;
    QMenu *m_menuModel;
    QMenu *m_menuRetainModels;
    QMenu *m_menuRetainAssets;
    QMenu *m_menuHelp;
};

#endif // MAINWINDOW_H
