#ifndef TRANSFORMWIDGET_H
#define TRANSFORMWIDGET_H

#include <QtCore/QModelIndex>
#include <QtGui/QItemSelection>
#include <QtGui/QPushButton>
#include <QtGui/QWidget>
#include <vpvl/Common.h>

namespace vpvl {
class Bone;
class Face;
class PMDModel;
}

namespace Ui {
class TransformWidget;
}

class BoneMotionModel;
class FaceMotionModel;
class QSettings;

class BoneListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    BoneListModel(BoneMotionModel *model);
    ~BoneListModel();

    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;

    QList<vpvl::Bone *> bonesByIndices(const QModelIndexList &indices) const;
    QList<vpvl::Bone *> bonesFromIndices(const QModelIndexList &indices) const;
    void selectBones(const QList<vpvl::Bone *> &bones);

private slots:
    void changeModel(vpvl::PMDModel *model);

private:
    BoneMotionModel *m_model;
    QList<vpvl::Bone *> m_bones;
};

class TransformButton : public QPushButton
{
    Q_OBJECT

public:
    explicit TransformButton(QWidget *parent = 0);
    ~TransformButton();

    void setAngle(const vpvl::Vector3 &value) { m_angle = value; }
    void setBoneMotionModel(BoneMotionModel *value) { m_boneMotionModel = value; }

public slots:
    void setMode(int value);

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);

private:
    BoneMotionModel *m_boneMotionModel;
    vpvl::Vector3 m_angle;
    QCursor m_cursor;
    QPoint m_drag;
    QPoint m_pos;
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
    explicit TransformWidget(QSettings *settings,
                             BoneMotionModel *bmm,
                             FaceMotionModel *fmm,
                             QWidget *parent = 0);
    ~TransformWidget();

public slots:
    void setCameraPerspective(const vpvl::Vector3 &pos, const vpvl::Vector3 &angle, float fovy, float distance);

signals:
    void boneDidRegister(vpvl::Bone *bone);
    void faceDidRegister(vpvl::Face *face);
    void bonesDidSelect(const QList<vpvl::Bone *> &bones);
    void facesDidSelect(const QList<vpvl::Face *> &faces);

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void on_faces_selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void on_bones_selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void on_faceWeightSlider_valueChanged(int value);
    void on_faceWeightSpinBox_valueChanged(double value);
    void on_faceWeightSlider_sliderPressed();
    void on_faceWeightSlider_sliderReleased();
    void on_comboBox_currentIndexChanged(int index);
    void on_registerButton_clicked();

private:
    Ui::TransformWidget *ui;
    BoneListModel *m_bmm;
    QSettings *m_settings;
    QItemSelection m_selectedBones;
};

#endif // TRANSFORMWIDGET_H
