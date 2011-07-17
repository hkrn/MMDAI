#include "TransformWidget.h"
#include "ui_TransformWidget.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

class BoneModel : public QAbstractListModel {
public:
    BoneModel(QObject *parent = 0)
        : QAbstractListModel(parent) {
        setModel(0);
    }

    void setModel(vpvl::PMDModel *value) {
        m_bones.clear();
        QString format("%1: %2");
        if (value) {
            vpvl::BoneList bones = value->bones();
            uint32_t nBones = bones.size();
            QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
            for (uint32_t i = 0; i < nBones; i++) {
                vpvl::Bone *bone = bones.at(i);
                QString name = codec->toUnicode(reinterpret_cast<const char *>(bone->name()));
                m_bones.append(QPair<QString, vpvl::Bone *>(format.arg(i).arg(name), bone));
            }
        }
        m_model = value;
        reset();
    }
    bool hasModel() { return m_model != 0; }
    vpvl::Bone *selectBone(int rowIndex) {
        vpvl::Bone *bone = m_selectedBone = m_bones[rowIndex].second;
        return bone;
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
        : QAbstractListModel(parent) {
        setModel(0);
    }

    void setModel(vpvl::PMDModel *value) {
        m_faces.clear();
        QString format("%1: %2");
        if (value) {
            vpvl::FaceList faces = value->faces();
            uint32_t nFaces = faces.size();
            QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
            for (uint32_t i = 0; i < nFaces; i++) {
                vpvl::Face *face = faces.at(i);
                if (face->type() != vpvl::Face::kBase) {
                    QString name = codec->toUnicode(reinterpret_cast<const char *>(face->name()));
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
    qDebug() << event->pos();
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

void TransformButton::mouseReleaseEvent(QMouseEvent *event)
{
    qDebug() << event->pos();
    m_pos.setX(0);
    m_pos.setY(0);
}

TransformWidget::TransformWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TransformWidget)
{
    ui->setupUi(this);
    ui->bones->setModel(new BoneModel());
    ui->faces->setModel(new FaceModel());
}

TransformWidget::~TransformWidget()
{
    delete ui;
}

static BoneModel *castBoneModel(Ui::TransformWidget *ui)
{
    return reinterpret_cast<BoneModel *>(ui->bones->model());
}

static FaceModel *castFaceModel(Ui::TransformWidget *ui)
{
    return reinterpret_cast<FaceModel *>(ui->faces->model());
}

static const QList<TransformButton *> buttons(Ui::TransformWidget *ui)
{
    QList<TransformButton *> buttons;
    buttons.append(ui->rx);
    buttons.append(ui->ry);
    buttons.append(ui->rz);
    buttons.append(ui->tx);
    buttons.append(ui->ty);
    buttons.append(ui->tz);
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

void TransformWidget::on_faceWeightSlider_sliderMoved(int position)
{
    setFaceValue(ui, position / 100.0f);
    castFaceModel(ui)->setWeight(position / 100.0f);
}

void TransformWidget::on_faces_clicked(const QModelIndex &index)
{
    vpvl::Face *face = castFaceModel(ui)->selectFace(index.row());
    float weight = face->weight();
    setFaceValue(ui, weight);
    ui->faceWeightSlider->setValue(weight * 100.0f);
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

void TransformWidget::on_bones_clicked(const QModelIndex &index)
{
    vpvl::Bone *bone = castBoneModel(ui)->selectBone(index.row());
    foreach (TransformButton *button, buttons(ui))
        button->setBone(bone);
}

void TransformWidget::on_comboBox_currentIndexChanged(int index)
{
    foreach (TransformButton *button, buttons(ui))
        button->setMode(index);
}
