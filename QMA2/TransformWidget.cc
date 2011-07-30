#include "TransformWidget.h"
#include "ui_TransformWidget.h"
#include "BoneMotionModel.h"
#include "FaceMotionModel.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>
#include "util.h"

namespace {

static BoneMotionModel *castBoneModel(Ui::TransformWidget *ui)
{
    return reinterpret_cast<BoneMotionModel *>(ui->bones->model());
}

static FaceMotionModel *castFaceModel(Ui::TransformWidget *ui)
{
    return reinterpret_cast<FaceMotionModel *>(ui->faces->model());
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

}

TransformButton::TransformButton(QWidget *parent) :
    QPushButton(parent),
    m_boneMotionModel(0),
    m_angle(0.0f, 0.0f, 0.0f)
{
}

TransformButton::~TransformButton()
{
}

void TransformButton::setMode(int value)
{
    m_boneMotionModel->setMode(value);
}

void TransformButton::mousePressEvent(QMouseEvent *event)
{
    m_pos = event->pos();
    m_drag = mapToGlobal(m_pos);
    m_cursor = cursor();
    QBitmap transparent(32, 32);
    transparent.fill(Qt::color0);
    qApp->setOverrideCursor(QCursor(transparent, transparent));
}

void TransformButton::mouseMoveEvent(QMouseEvent *event)
{
    if (m_boneMotionModel->isBoneSelected() && !m_pos.isNull()) {
        const QString name = objectName();
        const QPoint p = m_pos - event->pos();
        char type = name[0].toAscii();
        char coordinate = name[1].toAscii();
        float value = p.y() * 0.1;
        switch (type) {
        case 'r':
            m_boneMotionModel->setRotation(coordinate, vpvl::radian(value));
            break;
        case 't':
            m_boneMotionModel->setPosition(coordinate, value);
            break;
        }
        m_pos = event->pos();
    }
}

void TransformButton::mouseReleaseEvent(QMouseEvent * /* event */)
{
    qApp->setOverrideCursor(m_cursor);
    cursor().setPos(m_drag);
    m_pos.setX(0);
    m_pos.setY(0);
}

TransformWidget::TransformWidget(QSettings *settings,
                                 BoneMotionModel *bmm,
                                 FaceMotionModel *fmm,
                                 QWidget *parent)
    : QWidget(parent),
      ui(new Ui::TransformWidget),
      m_settings(settings)
{
    QList<TransformButton *> buttons;
    ui->setupUi(this);
    ui->bones->setModel(bmm);
    ui->faces->setModel(fmm);
    transformButtons(buttons, ui);
    rotateButtons(buttons, ui);
    foreach (TransformButton *button, buttons)
        button->setBoneMotionModel(bmm);
    connect(ui->bones->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(on_bones_clicked(QModelIndex)));
    connect(ui->faces->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(on_faces_clicked(QModelIndex)));
    restoreGeometry(m_settings->value("transformWidget/geometry").toByteArray());
}

TransformWidget::~TransformWidget()
{
    delete ui;
}

static void setFaceValue(Ui::TransformWidget *ui, float value)
{
    ui->faceWeightValue->setText(QString("%1").arg(value, 0, 'g', 2));
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

void TransformWidget::on_bones_clicked(const QModelIndex &index)
{
    QList<TransformButton *> buttons;
    vpvl::Bone *bone = castBoneModel(ui)->selectBone(index.row());
    if (bone) {
        bool movable = bone->isMovable(), rotateable = bone->isRotateable();
        transformButtons(buttons, ui);
        foreach (TransformButton *button, buttons)
            button->setEnabled(movable);
        buttons.clear();
        rotateButtons(buttons, ui);
        foreach (TransformButton *button, buttons)
            button->setEnabled(rotateable);
    }
    else {
        transformButtons(buttons, ui);
        foreach (TransformButton *button, buttons)
            button->setEnabled(false);
        buttons.clear();
        rotateButtons(buttons, ui);
        foreach (TransformButton *button, buttons)
            button->setEnabled(false);
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
