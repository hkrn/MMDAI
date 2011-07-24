#include "TransformWidget.h"
#include "ui_TransformWidget.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>
#include "util.h"

namespace internal {

static const QString kNotSelected = QT_TR_NOOP("0: Not Selected");

class BoneModel : public QAbstractListModel {
public:
    BoneModel(QObject *parent = 0)
        : QAbstractListModel(parent), m_selectedBone(0) {
        setModel(0);
    }

    void setModel(vpvl::PMDModel *value) {
        QString format("%1: %2");
        m_bones.clear();
        m_bones.append(QPair<QString, vpvl::Bone *>(kNotSelected, 0));
        if (value) {
            vpvl::BoneList bones = value->bones();
            uint32_t nBones = bones.size();
            for (uint32_t i = 0; i < nBones; i++) {
                vpvl::Bone *bone = bones.at(i);
                if (bone->isVisible()) {
                    QString name = toQString(bone);
                    m_bones.append(QPair<QString, vpvl::Bone *>(format.arg(i + 1).arg(name), bone));
                }
            }
        }
        m_model = value;
        reset();
    }

    void resetBone(TransformWidget::ResetBoneType type) {
        if (m_selectedBone) {
            btVector3 pos = m_selectedBone->position();
            btQuaternion rot = m_selectedBone->rotation();
            switch (type) {
            case TransformWidget::kX:
                pos.setX(0.0f);
                m_selectedBone->setPosition(pos);
                break;
            case TransformWidget::kY:
                pos.setY(0.0f);
                m_selectedBone->setPosition(pos);
                break;
            case TransformWidget::kZ:
                pos.setZ(0.0f);
                m_selectedBone->setPosition(pos);
                break;
            case TransformWidget::kRotation:
                rot.setValue(0.0f, 0.0f, 0.0f, 1.0f);
                m_selectedBone->setRotation(rot);
                break;
            default:
                qFatal("Unexpected reset bone type: %d", type);
            }
        }
    }

    bool hasModel() { return m_model != 0; }

    vpvl::Bone *selectBone(int rowIndex) {
        vpvl::Bone *bone = m_selectedBone = m_bones[rowIndex].second;
        return bone;
    }

    QList<vpvl::Bone *> bonesFromIndices(const QModelIndexList &indices) {
        QList<vpvl::Bone *> bones;
        foreach (QModelIndex index, indices) {
            bones.append(index.isValid() ? m_bones[index.row()].second : 0);
        }
        return bones;
    }

    int rowCount(const QModelIndex & /* parent */) const {
        return m_bones.size();
    }

    QVariant data(const QModelIndex &index, int role) const {
        if (!index.isValid() || role != Qt::DisplayRole)
            return QVariant();
        return m_bones[index.row()].first;
    }

private:
    vpvl::PMDModel *m_model;
    vpvl::Bone *m_selectedBone;
    QList< QPair<QString, vpvl::Bone *> > m_bones;
};


class FaceModel : public QAbstractListModel {
public:
    FaceModel(QObject *parent = 0)
        : QAbstractListModel(parent), m_selectedFace(0) {
        setModel(0);
    }

    void setModel(vpvl::PMDModel *value) {
        QString format("%1: %2");
        m_faces.clear();
        m_faces.append(QPair<QString, vpvl::Face *>(kNotSelected, 0));
        if (value) {
            vpvl::FaceList faces = value->faces();
            uint32_t nFaces = faces.size();
            for (uint32_t i = 0; i < nFaces; i++) {
                vpvl::Face *face = faces.at(i);
                if (face->type() != vpvl::Face::kBase) {
                    QString name = toQString(face);
                    m_faces.append(QPair<QString, vpvl::Face *>(format.arg(i).arg(name), face));
                }
            }
        }
        m_model = value;
        reset();
    }

    bool hasModel() { return m_model != 0; }

    vpvl::Face *selectFace(int rowIndex) {
        vpvl::Face *face = m_selectedFace = m_faces[rowIndex].second;
        return face;
    }

    QList<vpvl::Face *> facesFromIndices(const QModelIndexList &indices) {
        QList<vpvl::Face *> faces;
        foreach (QModelIndex index, indices) {
            faces.append(index.isValid() ? m_faces[index.row()].second : 0);
        }
        return faces;
    }

    void setWeight(float value) {
        if (m_selectedFace)
            m_selectedFace->setWeight(value);
    }

    int rowCount(const QModelIndex & /* parent */) const {
        return m_faces.size();
    }
    QVariant data(const QModelIndex &index, int role) const {
        if (!index.isValid() || role != Qt::DisplayRole)
            return QVariant();
        return m_faces[index.row()].first;
    }

private:
    vpvl::PMDModel *m_model;
    vpvl::Face *m_selectedFace;
    QList< QPair<QString, vpvl::Face *> > m_faces;
};

}

TransformButton::TransformButton(QWidget *parent) :
    QPushButton(parent),
    m_bone(0),
    m_angle(0.0f, 0.0f, 0.0f),
    m_mode(kLocal)
{
}

TransformButton::~TransformButton()
{
}

void TransformButton::setMode(int value)
{
    switch (value) {
    case 0:
        m_mode = kLocal;
        break;
    case 1:
        m_mode = kGlobal;
        break;
    case 2:
        m_mode = kView;
        break;
    }
}

void TransformButton::mousePressEvent(QMouseEvent *event)
{
    m_pos = event->pos();
}

void TransformButton::mouseMoveEvent(QMouseEvent *event)
{
    if (m_bone && !m_pos.isNull()) {
        const QString name = objectName();
        const QPoint p = m_pos - event->pos();
        char type = name[0].toAscii();
        char coordinate = name[1].toAscii();
        if (type == 'r') {
            btQuaternion current = m_bone->rotation(), rot, dest;
            float value = vpvl::radian(p.y() * 0.1f);
            switch (coordinate) {
            case 'x':
                rot.setEulerZYX(0, 0, value);
                break;
            case 'y':
                rot.setEulerZYX(0, value, 0);
                break;
            case 'z':
                rot.setEulerZYX(value, 0, 0);
                break;
            default:
                qFatal("Unexpected coordinate value: %c", coordinate);
            }
            btQuaternion view;
            view.setEulerZYX(vpvl::radian(m_angle.z()), vpvl::radian(m_angle.y()), vpvl::radian(m_angle.x()));
            // local coordinate
            switch (m_mode) {
            case kLocal:
                dest = current * rot;
                break;
            default:
                break;
            }
            m_bone->setRotation(dest);
        }
        else if (type == 't') {
            btVector3 current = m_bone->position(), pos, dest;
            float value = p.y() * 0.1;
            switch (coordinate) {
            case 'x':
                pos.setValue(value, 0, 0);
                break;
            case 'y':
                pos.setValue(0, value, 0);
                break;
            case 'z':
                pos.setValue(0, 0, value);
                break;
            default:
                qFatal("Unexpected coordinate value: %c", coordinate);
            }
            // local coordinate
            switch (m_mode) {
            case kLocal:
                dest = btTransform(m_bone->rotation(), current) * pos;
                break;
            case kGlobal:
                dest = current + pos;
                break;
            default:
                break;
            }
            m_bone->setPosition(dest);
        }
        m_pos = event->pos();
    }
}

void TransformButton::mouseReleaseEvent(QMouseEvent * /* event */)
{
    m_pos.setX(0);
    m_pos.setY(0);
}

static internal::BoneModel *castBoneModel(Ui::TransformWidget *ui)
{
    return reinterpret_cast<internal::BoneModel *>(ui->bones->model());
}

static internal::FaceModel *castFaceModel(Ui::TransformWidget *ui)
{
    return reinterpret_cast<internal::FaceModel *>(ui->faces->model());
}

TransformWidget::TransformWidget(QSettings *settings, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TransformWidget),
    m_settings(settings)
{
    ui->setupUi(this);
    ui->bones->setModel(new internal::BoneModel(this));
    ui->faces->setModel(new internal::FaceModel(this));
    restoreGeometry(m_settings->value("transformWidget/geometry").toByteArray());
}

TransformWidget::~TransformWidget()
{
    delete ui;
}

void TransformWidget::resetBone(ResetBoneType type)
{
    castBoneModel(ui)->resetBone(type);
}

static void rotateButtons(QList<TransformButton *> &buttons,
                          const Ui::TransformWidget *ui)
{
    buttons.append(ui->rx);
    buttons.append(ui->ry);
    buttons.append(ui->rz);
}

static void transformButtons(QList<TransformButton *> &buttons,
                             const Ui::TransformWidget *ui)
{
    buttons.append(ui->tx);
    buttons.append(ui->ty);
    buttons.append(ui->tz);
}

static const QList<TransformButton *> allButtons(const Ui::TransformWidget *ui)
{
    QList<TransformButton *> buttons;
    rotateButtons(buttons, ui);
    transformButtons(buttons, ui);
    return buttons;
}

static void setFaceValue(Ui::TransformWidget *ui, float value)
{
    ui->faceWeightValue->setText(QString("%1").arg(value, 0, 'g', 2));
}

void TransformWidget::setModel(vpvl::PMDModel *value)
{
    castBoneModel(ui)->setModel(value);
    castFaceModel(ui)->setModel(value);
}

void TransformWidget::setCameraPerspective(const btVector3 & /* pos */,
                                           const btVector3 &angle,
                                           float /* fovy */,
                                           float /* distance */)
{
    reinterpret_cast<TransformButton *>(ui->rx)->setAngle(angle);
    reinterpret_cast<TransformButton *>(ui->ry)->setAngle(angle);
    reinterpret_cast<TransformButton *>(ui->rz)->setAngle(angle);
}

void TransformWidget::closeEvent(QCloseEvent *event)
{
    m_settings->setValue("transformWidget/geometry", saveGeometry());
    event->accept();
}

void TransformWidget::on_faceWeightSlider_sliderMoved(int position)
{
    setFaceValue(ui, position / 100.0f);
    castFaceModel(ui)->setWeight(position / 100.0f);
}

void TransformWidget::on_faces_clicked(const QModelIndex &index)
{
    vpvl::Face *face = castFaceModel(ui)->selectFace(index.row());
    if (face) {
        float weight = face->weight();
        setFaceValue(ui, weight);
        ui->faceWeightSlider->setValue(weight * 100.0f);
    }
}

void TransformWidget::on_faceWeightValue_returnPressed()
{
    bool ok = false;
    float value = ui->faceWeightValue->text().toFloat(&ok);
    if (ok) {
        value = qMin(1.0f, qMax(0.0f, value));
        castFaceModel(ui)->setWeight(value);
        ui->faceWeightSlider->setValue(value * 100.0f);
    }
    setFaceValue(ui, value);
}

void TransformWidget::on_bones_pressed(const QModelIndex &index)
{
    QList<TransformButton *> buttons;
    vpvl::Bone *bone = castBoneModel(ui)->selectBone(index.row());
    if (bone) {
        bool movable = bone->isMovable(), rotateable = bone->isRotateable();
        transformButtons(buttons, ui);
        foreach (TransformButton *button, buttons) {
            button->setEnabled(movable);
            button->setBone(bone);
        }
        buttons.clear();
        rotateButtons(buttons, ui);
        foreach (TransformButton *button, buttons) {
            button->setEnabled(rotateable);
            button->setBone(bone);
        }
    }
    else {
        transformButtons(buttons, ui);
        foreach (TransformButton *button, buttons) {
            button->setEnabled(false);
        }
        buttons.clear();
        rotateButtons(buttons, ui);
        foreach (TransformButton *button, buttons) {
            button->setEnabled(false);
        }
    }
}

void TransformWidget::on_comboBox_currentIndexChanged(int index)
{
    foreach (TransformButton *button, allButtons(ui))
        button->setMode(index);
}

void TransformWidget::on_registerButton_clicked()
{
    foreach (vpvl::Bone *bone, castBoneModel(ui)->bonesFromIndices(ui->bones->selectionModel()->selectedIndexes())) {
        if (bone)
            emit boneDidRegister(bone);
    }
    foreach (vpvl::Face *face, castFaceModel(ui)->facesFromIndices(ui->faces->selectionModel()->selectedIndexes())) {
        if (face)
            emit faceDidRegister(face);
    }
}
