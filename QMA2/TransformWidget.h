#ifndef TRANSFORMWIDGET_H
#define TRANSFORMWIDGET_H

#include <QtCore/QModelIndex>
#include <QtGui/QLabel>
#include <QtGui/QWidget>

namespace vpvl {
class Bone;
class PMDModel;
}

namespace Ui {
    class TransformWidget;
}

class TransformLabel : public QLabel
{
    Q_OBJECT

public:
    explicit TransformLabel(QWidget *parent = 0);
    ~TransformLabel();

    void setBone(vpvl::Bone *value) { m_bone = value; }

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);

private:
    vpvl::Bone *m_bone;
    QPoint m_pos;
};

class TransformWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TransformWidget(QWidget *parent = 0);
    ~TransformWidget();

public slots:
    void setModel(vpvl::PMDModel *value);

private slots:
    void on_faceWeightSlider_sliderMoved(int position);
    void on_faces_clicked(const QModelIndex &index);
    void on_faceWeightValue_returnPressed();
    void on_bones_clicked(const QModelIndex &index);

private:
    Ui::TransformWidget *ui;
};

#endif // TRANSFORMWIDGET_H
