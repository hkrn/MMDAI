#ifndef INTERPOLATIONWIDGET_H
#define INTERPOLATIONWIDGET_H

#include <QtGui/QWidget>
#include "TimelineTabWidget.h"

class BoneMotionModel;
class QComboBox;
class QHBoxLayout;

class InterpolationGraphWidget : public QWidget
{
    Q_OBJECT

public:
    static const int kCircleWidth = 5;

    explicit InterpolationGraphWidget(BoneMotionModel *bmm, QWidget *parent = 0);
    ~InterpolationGraphWidget();

public slots:
    void setX1(int value);
    void setX2(int value);
    void setY1(int value);
    void setY2(int value);

signals:
    void x1ValueDidChange(int value);
    void x2ValueDidChange(int value);
    void y1ValueDidChange(int value);
    void y2ValueDidChange(int value);

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void paintEvent(QPaintEvent *event);

private:
    void updateValues();

    BoneMotionModel *m_boneMotionModel;
    QPoint m_p1;
    QPoint m_p2;
};

class QSpinBox;

class InterpolationWidget : public QWidget
{
    Q_OBJECT

public:
    explicit InterpolationWidget(BoneMotionModel *bmm, QWidget *parent = 0);
    ~InterpolationWidget();

private slots:
    void setMode(TimelineTabWidget::Type mode);
    void disable();
    void resetInterpolation();

private:
    QSpinBox *createSpinBox(int defaultValue,
                            const char *signal,
                            const char *slot);

    QComboBox *m_comboBox;
    InterpolationGraphWidget *m_graphWidget;
};

#endif // INTERPOLATIONWIDGET_H
