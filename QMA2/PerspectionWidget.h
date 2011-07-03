#ifndef PERSPECTIONWIDGET_H
#define PERSPECTIONWIDGET_H

#include <QtGui/QWidget>

class btVector3;

class PerspectionWidget : public QWidget
{
    Q_OBJECT
public:
    explicit PerspectionWidget(QWidget *parent = 0);

public slots:
    void setPerspectionFront();
    void setPerspectionBack();
    void setPerspectionTop();
    void setPerspectionLeft();
    void setPerspectionRight();

signals:
    void perspectionDidChange(btVector3 *pos, btVector3 *angle, float *fovy, float *distance);
};

#endif // PERSPECTIONWIDGET_H
