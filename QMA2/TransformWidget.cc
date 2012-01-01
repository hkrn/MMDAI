/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2011  hkrn                                    */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAI project team nor the names of     */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#include "models/BoneMotionModel.h"
#include "models/FaceMotionModel.h"
#include "TransformWidget.h"
#include "ui_TransformWidget.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>
#include "common/util.h"

namespace
{

static BoneListModel *UICastBoneModel(Ui::TransformWidget *ui)
{
    return reinterpret_cast<BoneListModel *>(ui->bones->model());
}

static FaceListModel *UICastFaceModel(Ui::TransformWidget *ui)
{
    return reinterpret_cast<FaceListModel *>(ui->faces->model());
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
    BoneListModel *bmm = qobject_cast<BoneListModel *>(ui->bones->model());
    foreach (QItemSelectionRange range, selection) {
        QList<vpvl::Bone *> children = bmm->bonesByIndices(UISelectRowIndices(range));
        foreach (vpvl::Bone *child, children)
            bones.append(child);
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
    FaceListModel *fmm = qobject_cast<FaceListModel *>(ui->faces->model());
    foreach (QItemSelectionRange range, selection) {
        foreach (vpvl::Face *face, fmm->facesByIndices(UISelectRowIndices(range)))
            faces.append(face);
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

BoneListModel::BoneListModel(BoneMotionModel *model)
    : m_model(model)
{
    connect(m_model, SIGNAL(modelDidChange(vpvl::PMDModel*)), this, SLOT(changeModel(vpvl::PMDModel*)));
}

BoneListModel::~BoneListModel()
{
}

QVariant BoneListModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        switch(role) {
        case Qt::DisplayRole:
            const vpvl::Bone *bone = m_bones[index.row()];
            const QString name = internal::toQString(bone);
            return name;
        }
    }
    return QVariant();
}

int BoneListModel::rowCount(const QModelIndex & /* parent */) const
{
    return m_bones.count();
}

int BoneListModel::columnCount(const QModelIndex & /* parent */) const
{
    return 1;
}

void BoneListModel::selectBones(const QList<vpvl::Bone *> &bones)
{
    m_model->selectBones(bones);
}

void BoneListModel::changeModel(vpvl::PMDModel * /* model */)
{
    vpvl::PMDModel *model = m_model->selectedModel();
    if (model) {
        const vpvl::BoneList &bones = model->bones();
        const int nbones = bones.count();
        m_bones.clear();
        for (int i = 0; i < nbones; i++) {
            vpvl::Bone *bone = bones[i];
            if (bone->isMovable() || bone->isRotateable())
                m_bones.append(bone);
        }
    }
    reset();
}

QList<vpvl::Bone *> BoneListModel::bonesByIndices(const QModelIndexList &indices) const
{
    return bonesFromIndices(indices);
}

QList<vpvl::Bone *> BoneListModel::bonesFromIndices(const QModelIndexList &indices) const
{
    QList<vpvl::Bone *> ret;
    vpvl::PMDModel *model = m_model->selectedModel();
    if (!model)
        return ret;
    foreach (QModelIndex index, indices) {
        if (index.isValid())
            ret.append(m_bones[index.row()]);
    }
    return ret;
}

FaceListModel::FaceListModel(FaceMotionModel *model)
    : m_model(model)
{
    connect(m_model, SIGNAL(modelDidChange(vpvl::PMDModel*)), this, SLOT(changeModel(vpvl::PMDModel*)));
}

FaceListModel::~FaceListModel()
{
}

QVariant FaceListModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid()) {
        switch(role) {
        case Qt::DisplayRole:
            const vpvl::Face *bone = m_faces[index.row()];
            const QString name = internal::toQString(bone);
            return name;
        }
    }
    return QVariant();
}

int FaceListModel::rowCount(const QModelIndex & /* parent */) const
{
    return m_faces.count();
}

int FaceListModel::columnCount(const QModelIndex & /* parent */) const
{
    return 1;
}

void FaceListModel::selectFaces(const QList<vpvl::Face *> &faces)
{
    m_model->selectFaces(faces);
}

void FaceListModel::startTransform()
{
    m_model->saveTransform();
}

void FaceListModel::commitTransform()
{
    m_model->commitTransform();
}

void FaceListModel::setWeight(float value)
{
    m_model->setWeight(value);
}

void FaceListModel::changeModel(vpvl::PMDModel * /* model */)
{
    vpvl::PMDModel *model = m_model->selectedModel();
    if (model) {
        const vpvl::FaceList &faces = model->faces();
        const int nfaces = faces.count();
        m_faces.clear();
        for (int i = 0; i < nfaces; i++) {
            vpvl::Face *face = faces[i];
            if (face->type() != vpvl::Face::kBase)
                m_faces.append(face);
        }
    }
    reset();
}

QList<vpvl::Face *> FaceListModel::facesByIndices(const QModelIndexList &indices) const
{
    return facesFromIndices(indices);
}

QList<vpvl::Face *> FaceListModel::facesFromIndices(const QModelIndexList &indices) const
{
    QList<vpvl::Face *> ret;
    vpvl::PMDModel *model = m_model->selectedModel();
    if (!model)
        return ret;
    foreach (QModelIndex index, indices) {
        if (index.isValid())
            ret.append(m_faces[index.row()]);
    }
    return ret;
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

void TransformButton::mousePressEvent(QMouseEvent *event)
{
    m_pos = event->pos();
    m_drag = mapToGlobal(m_pos);
    m_cursor = cursor();
    m_boneMotionModel->saveTransform();
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
            m_boneMotionModel->rotate(coordinate, 'L', vpvl::radian(value));
            break;
        case 't':
            m_boneMotionModel->translate(coordinate, 'L', value);
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
    m_boneMotionModel->commitTransform();
}

void TransformButton::setMode(int value)
{
    m_mode = value;
}

TransformWidget::TransformWidget(QSettings *settings,
                                 BoneMotionModel *bmm,
                                 FaceMotionModel *fmm,
                                 QWidget *parent)
    : QWidget(parent),
      ui(new Ui::TransformWidget),
      m_boneList(0),
      m_faceList(0),
      m_settings(settings)
{
    QList<TransformButton *> buttons;
    m_boneList = new BoneListModel(bmm);
    m_faceList = new FaceListModel(fmm);
    ui->setupUi(this);
    ui->bones->setModel(m_boneList);
    ui->faces->setModel(m_faceList);
    UIGetTransformButtons(buttons, ui);
    UIGetRotateButtons(buttons, ui);
    foreach (TransformButton *button, buttons)
        button->setBoneMotionModel(bmm);
    connect(ui->bones->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(bonesSelectionChanged(QItemSelection,QItemSelection)));
    connect(ui->faces->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(facesSelectionChanged(QItemSelection,QItemSelection)));
    restoreGeometry(m_settings->value("transformWidget/geometry").toByteArray());
}

TransformWidget::~TransformWidget()
{
    delete m_boneList;
    delete m_faceList;
    delete ui;
}

void TransformWidget::setCameraPerspective(const vpvl::Vector3 & /* pos */,
                                           const vpvl::Vector3 &angle,
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

void TransformWidget::facesSelectionChanged(const QItemSelection &selected,
                                            const QItemSelection & /* deselected */)
{
    QList<vpvl::Face *> selectedFaces = UISelectFacesBySelection(ui, selected);
    UICastFaceModel(ui)->selectFaces(selectedFaces);
    float weight = 0.0f;
    if (!selectedFaces.isEmpty()) {
        vpvl::Face *face = selectedFaces.last();
        if (face)
            weight = face->weight();
    }
    ui->faceWeightSpinBox->setValue(weight);
    ui->faceWeightSlider->setValue(weight * 100.0f);
}

void TransformWidget::bonesSelectionChanged(const QItemSelection &selected,
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

void TransformWidget::on_faceWeightSlider_sliderPressed()
{
    UICastFaceModel(ui)->startTransform();
}

void TransformWidget::on_faceWeightSlider_sliderReleased()
{
    UICastFaceModel(ui)->commitTransform();
}

void TransformWidget::on_comboBox_currentIndexChanged(int index)
{
    int mode = 0;
    switch (index) {
    case 0:
        mode = 'L';
        break;
    case 1:
        mode = 'G';
        break;
    case 2:
        mode = 'V';
        break;
    }
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
