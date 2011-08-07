#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QtGui/QTabWidget>

namespace Ui {
    class TabWidget;
}

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
                       FaceMotionModel *fmm,
                       QWidget *parent = 0);
    ~TabWidget();

    CameraPerspectiveWidget *cameraPerspectiveWidget();
    FaceWidget *faceWidget();

protected:
    void closeEvent(QCloseEvent *event);

private:
    QSettings *m_settings;
    CameraPerspectiveWidget *m_camera;
    FaceWidget *m_face;
    InterpolationWidget *m_interpolation;
};

#endif // TABWIDGET_H
