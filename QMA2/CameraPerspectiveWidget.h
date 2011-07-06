#ifndef PERSPECTIONWIDGET_H
#define PERSPECTIONWIDGET_H

#include <QtGui/QWidget>

class btVector3;

class CameraPerspectiveWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CameraPerspectiveWidget(QWidget *parent = 0);

public slots:
    void setCameraPerspectiveFront();
    void setCameraPerspectiveBack();
    void setCameraPerspectiveTop();
    void setCameraPerspectiveLeft();
    void setCameraPerspectiveRight();

signals:
    void cameraPerspectiveDidChange(btVector3 *pos, btVector3 *angle, float *fovy, float *distance);
};

#endif // PERSPECTIONWIDGET_H
