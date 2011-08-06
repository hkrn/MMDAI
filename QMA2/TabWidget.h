#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QtGui/QTabWidget>

namespace Ui {
    class TabWidget;
}

class CameraPerspectiveWidget;
class FaceWidget;
class QSettings;

class TabWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TabWidget(QSettings *settings, QWidget *parent = 0);
    ~TabWidget();

    CameraPerspectiveWidget *cameraPerspectiveWidget();
    FaceWidget *faceWidget();

protected:
    void closeEvent(QCloseEvent *event);

private:
    QSettings *m_settings;
    CameraPerspectiveWidget *m_camera;
    FaceWidget *m_face;
};

#endif // TABWIDGET_H
