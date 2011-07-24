#ifndef TRANSFORMWIDGET_H
#define TRANSFORMWIDGET_H

#include <QtCore/QModelIndex>
#include <QtGui/QPushButton>
#include <QtGui/QWidget>
#include <LinearMath/btVector3.h>

namespace vpvl {
class Bone;
class Face;
class PMDModel;
}

namespace Ui {
    class TransformWidget;
}

class QSettings;

class TransformButton : public QPushButton
{
    Q_OBJECT

public:
    enum Coordinate {
        kLocal,
        kGlobal,
        kView
    };

    explicit TransformButton(QWidget *parent = 0);
    ~TransformButton();

    void setBone(vpvl::Bone *value) { m_bone = value; }
    void setAngle(const btVector3 &angle) { m_angle = angle; }

public slots:
    void setMode(int value);

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);

private:
    vpvl::Bone *m_bone;
    btVector3 m_angle;
    QPoint m_pos;
    Coordinate m_mode;
};

class TransformWidget : public QWidget
{
    Q_OBJECT

public:
    enum ResetBoneType {
        kX,
        kY,
        kZ,
        kRotation
    };
    explicit TransformWidget(QSettings *settings, QWidget *parent = 0);
    ~TransformWidget();

    void resetBone(ResetBoneType type);

public slots:
    void setModel(vpvl::PMDModel *value);
    void setCameraPerspective(const btVector3 &pos, const btVector3 &angle, float fovy, float distance);

signals:
    void boneDidRegister(vpvl::Bone *bone);
    void faceDidRegister(vpvl::Face *face);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void on_faceWeightSlider_sliderMoved(int position);
    void on_faces_clicked(const QModelIndex &index);
    void on_faceWeightValue_returnPressed();
    void on_bones_pressed(const QModelIndex &index);
    void on_comboBox_currentIndexChanged(int index);
    void on_registerButton_clicked();

private:
    Ui::TransformWidget *ui;
    QSettings *m_settings;
};

#endif // TRANSFORMWIDGET_H
