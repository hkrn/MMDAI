#include "BoneDialog.h"
#include "ui_BoneDialog.h"

#include "BoneMotionModel.h"
#include <vpvl/vpvl.h>

BoneDialog::BoneDialog(BoneMotionModel *bmm,
                       QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BoneDialog),
    m_boneMotionModel(bmm)
{
    ui->setupUi(this);
    vpvl::Bone *bone = m_boneMotionModel->selectedBone();
    setPosition(bone->position());
    setRotation(bone->rotation());
}

BoneDialog::~BoneDialog()
{
    delete ui;
}

void BoneDialog::setPosition(const btVector3 &pos)
{
    ui->XPositionSpinBox->setValue(pos.x());
    ui->YPositionSpinBox->setValue(pos.y());
    ui->ZPositionSpinBox->setValue(pos.z());
}

void BoneDialog::setRotation(const btQuaternion &rot)
{
    ui->XAxisSpinBox->setValue(vpvl::degree(rot.x()));
    ui->YAxisSpinBox->setValue(vpvl::degree(rot.y()));
    ui->ZAxisSpinBox->setValue(vpvl::degree(rot.z()));
}

void BoneDialog::on_XPositionSpinBox_valueChanged(double value)
{
    m_boneMotionModel->setPosition('X', value);
}

void BoneDialog::on_YPositionSpinBox_valueChanged(double value)
{
    m_boneMotionModel->setPosition('Y', value);
}

void BoneDialog::on_ZPositionSpinBox_valueChanged(double value)
{
    m_boneMotionModel->setPosition('Z', value);
}

void BoneDialog::on_XAxisSpinBox_valueChanged(double value)
{
    m_boneMotionModel->setRotation('X', vpvl::radian(value));
}

void BoneDialog::on_YAxisSpinBox_valueChanged(double value)
{
    m_boneMotionModel->setRotation('Y', vpvl::radian(value));
}

void BoneDialog::on_ZAxisSpinBox_valueChanged(double value)
{
    m_boneMotionModel->setRotation('Z', vpvl::radian(value));
}
