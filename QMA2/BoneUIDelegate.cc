#include "BoneUIDelegate.h"
#include "BoneDialog.h"
#include "BoneMotionModel.h"
#include "MainWindow.h"
#include <QtGui/QtGui>

BoneUIDelegate::BoneUIDelegate(BoneMotionModel *bmm, MainWindow *parent) :
    QObject(parent),
    m_parent(parent),
    m_boneMotionModel(bmm)
{
}

BoneUIDelegate::~BoneUIDelegate()
{
}

void BoneUIDelegate::resetBoneX()
{
    if (m_boneMotionModel->isBoneSelected()) {
        m_boneMotionModel->resetBone(BoneMotionModel::kX);
    }
    else {
        QMessageBox::warning(m_parent, tr("The model or the bone is not selected."),
                             tr("Select a model or a bone to reset X position of the bone"));
    }
}

void BoneUIDelegate::resetBoneY()
{
    if (m_boneMotionModel->isBoneSelected()) {
        m_boneMotionModel->resetBone(BoneMotionModel::kY);
    }
    else {
        QMessageBox::warning(m_parent, tr("The model or the bone is not selected."),
                             tr("Select a model or a bone to reset Y position of the bone"));
    }
}

void BoneUIDelegate::resetBoneZ()
{
    if (m_boneMotionModel->isBoneSelected()) {
        m_boneMotionModel->resetBone(BoneMotionModel::kZ);
    }
    else {
        QMessageBox::warning(m_parent, tr("The model or the bone is not selected."),
                             tr("Select a model or a bone to reset Z position of the bone"));
    }
}

void BoneUIDelegate::resetBoneRotation()
{
    if (m_boneMotionModel->isBoneSelected()) {
        m_boneMotionModel->resetBone(BoneMotionModel::kRotation);
    }
    else {
        QMessageBox::warning(m_parent, tr("The model or the bone is not selected."),
                             tr("Select a model or a bone to reset rotation of the bone"));
    }
}

void BoneUIDelegate::resetAllBones()
{
    if (m_boneMotionModel->isBoneSelected()) {
        m_boneMotionModel->resetAllBones();
    }
    else {
        QMessageBox::warning(m_parent, tr("The model is not selected."), tr("Select a model to reset bones"));
    }
}

void BoneUIDelegate::openBoneDialog()
{
    if (m_boneMotionModel->isBoneSelected()) {
        BoneDialog *dialog = new BoneDialog(m_boneMotionModel, m_parent);
        dialog->exec();
    }
    else {
        QMessageBox::warning(m_parent, tr("The model or the bone is not selected."),
                             tr("Select a model or a bone to open this dialog"));
    }
}
