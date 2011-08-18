#include "TransformWidget.h"
#include "ui_TransformWidget.h"
#include "BoneMotionModel.h"
#include "FaceMotionModel.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>
#include "util.h"

namespace {

static BoneMotionModel *UICastBoneModel(Ui::TransformWidget *ui)
{
    return reinterpret_cast<BoneMotionModel *>(ui->bones->model());
}

static FaceMotionModel *UICastFaceModel(Ui::TransformWidget *ui)
{
    return reinterpret_cast<FaceMotionModel *>(ui->faces->model());
}

static void UIGetRotateButtons(QList<TransformButton *> &buttons,
                               const Ui::TransformWidget *ui)
{
    buttons.append(ui->rx);
    buttons.append(ui->ry);
    buttons.append(ui->rz);
}

static void UIGetTransformButtons(QList<TransformButton *> &buttons,
                                  const Ui::TransformWidget *ui)
{
    buttons.append(ui->tx);
    buttons.append(ui->ty);
    buttons.append(ui->tz);
}

static const QList<TransformButton *> UIGetAllButtons(const Ui::TransformWidget *ui)
{
    QList<TransformButton *> buttons;
    UIGetRotateButtons(buttons, ui);
    UIGetTransformButtons(buttons, ui);
    return buttons;
}

static QModelIndexList UISelectRowIndices(const QItemSelectionRange &range)
{
    QModelIndexList newIndices;
    foreach (QModelIndex index, range.indexes()) {
        if (index.column() == 0)
            newIndices.append(index);
    }
    return newIndices;
}

static QList<vpvl::Bone *> UISelectBonesBySelection(const Ui::TransformWidget *ui, const QItemSelection &selection)
{
    QList<vpvl::Bone *> bones;
    BoneMotionModel *bmm = qobject_cast<BoneMotionModel *>(ui->bones->model());
    foreach (QItemSelectionRange range, selection) {
        foreach (vpvl::Bone *bone, bmm->bonesFromIndices(UISelectRowIndices(range))) {
            if (bone)
                bones.append(bone);
        }
    }
    return bones;
}

static QList<vpvl::Bone *> UISelectBones(const Ui::TransformWidget *ui)
{
    return UISelectBonesBySelection(ui, ui->bones->selectionModel()->selection());
}

static QList<vpvl::Face *> UISelectFacesBySelection(const Ui::TransformWidget *ui, const QItemSelection &selection)
{
    QList<vpvl::Face *> faces;
    FaceMotionModel *fmm = qobject_cast<FaceMotionModel *>(ui->faces->model());
    foreach (QItemSelectionRange range, selection) {
        foreach (vpvl::Face *face, fmm->facesFromIndices(UISelectRowIndices(range))) {
            if (face)
                faces.append(face);
        }
    }
    return faces;
}

static QList<vpvl::Face *> UISelectFaces(const Ui::TransformWidget *ui)
{
    return UISelectFacesBySelection(ui, ui->faces->selectionModel()->selection());
}

static void UIToggleBoneButtons(const Ui::TransformWidget *ui, bool movable, bool rotateable)
{
    QList<TransformButton *> buttons;
    UIGetTransformButtons(buttons, ui);
    foreach (TransformButton *button, buttons)
        button->setEnabled(movable);
    buttons.clear();
    UIGetRotateButtons(buttons, ui);
    foreach (TransformButton *button, buttons)
        button->setEnabled(rotateable);
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
            m_boneMotionModel->rotate(coordinate, vpvl::radian(value));
            break;
        case 't':
            m_boneMotionModel->transform(coordinate, value);
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
    UIGetTransformButtons(buttons, ui);
    UIGetRotateButtons(buttons, ui);
    foreach (TransformButton *button, buttons)
        button->setBoneMotionModel(bmm);
    connect(ui->bones->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(on_bones_selectionChanged(QItemSelection,QItemSelection)));
    connect(ui->faces->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(on_faces_selectionChanged(QItemSelection,QItemSelection)));
    restoreGeometry(m_settings->value("transformWidget/geometry").toByteArray());
}

TransformWidget::~TransformWidget()
{
    delete ui;
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

void TransformWidget::on_faces_selectionChanged(const QItemSelection &selected,
                                                const QItemSelection & /* deselected */)
{
    QList<vpvl::Face *> faces = UISelectFacesBySelection(ui, selected);
    UICastFaceModel(ui)->selectFaces(faces);
    float weight = 0.0f;
    if (!faces.isEmpty()) {
        vpvl::Face *face = faces.last();
        if (face)
            weight = face->weight();
    }
    ui->faceWeightSpinBox->setValue(weight);
    ui->faceWeightSlider->setValue(weight * 100.0f);
}

void TransformWidget::on_bones_selectionChanged(const QItemSelection &selected,
                                                const QItemSelection &deselected)
{
    bool movable = true, rotateable = true;
    m_selectedBones.merge(selected, QItemSelectionModel::Select);
    m_selectedBones.merge(deselected, QItemSelectionModel::Deselect);
    QList<vpvl::Bone *> selectedBones = UISelectBonesBySelection(ui, m_selectedBones);
    UICastBoneModel(ui)->selectBones(selectedBones);
    foreach (vpvl::Bone *bone, selectedBones) {
        movable = movable && bone->isMovable();
        rotateable = rotateable && bone->isRotateable();
    }
    UIToggleBoneButtons(ui, movable, rotateable);
}

void TransformWidget::on_faceWeightSlider_valueChanged(int value)
{
    float weight = value / 100.0f;
    ui->faceWeightSpinBox->setValue(weight);
    UICastFaceModel(ui)->setWeight(weight);
}

void TransformWidget::on_faceWeightSpinBox_valueChanged(double value)
{
    float weight = value;
    ui->faceWeightSlider->setValue(weight * 100.0f);
    UICastFaceModel(ui)->setWeight(weight);
}

void TransformWidget::on_comboBox_currentIndexChanged(int index)
{
    foreach (TransformButton *button, UIGetAllButtons(ui))
        button->setMode(index);
}

void TransformWidget::on_registerButton_clicked()
{
    foreach (vpvl::Bone *bone, UISelectBones(ui))
        emit boneDidRegister(bone);
    foreach (vpvl::Face *face, UISelectFaces(ui))
        emit faceDidRegister(face);
}
