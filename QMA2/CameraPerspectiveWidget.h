#ifndef PERSPECTIONWIDGET_H
#define PERSPECTIONWIDGET_H

#include <QtGui/QWidget>
#include <vpvl/Common.h>

class QPushButton;

class CameraPerspectiveWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CameraPerspectiveWidget(QWidget *parent = 0);

public slots:
    void retranslate();
    void setCameraPerspectiveFront();
    void setCameraPerspectiveBack();
    void setCameraPerspectiveTop();
    void setCameraPerspectiveLeft();
    void setCameraPerspectiveRight();

signals:
    void cameraPerspectiveDidChange(vpvl::Vector3 *pos, vpvl::Vector3 *angle, float *fovy, float *distance);

private:
    QPushButton *m_frontLabel;
    QPushButton *m_backLabel;
    QPushButton *m_topLabel;
    QPushButton *m_leftLabel;
    QPushButton *m_rightLabel;
    QPushButton *m_cameraLabel;
};

#endif // PERSPECTIONWIDGET_H
