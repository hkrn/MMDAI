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

class TabWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TabWidget(QSettings *settings,
                       BoneMotionModel *bmm,
                       FaceMotionModel *fmm,
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
};

#endif // TABWIDGET_H
