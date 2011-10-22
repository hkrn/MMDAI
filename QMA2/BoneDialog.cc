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
    m_boneMotionModel->saveState();
}

BoneDialog::~BoneDialog()
{
    delete ui;
}

void BoneDialog::setPosition(const vpvl::Vector3 &pos)
{
    ui->XPositionSpinBox->setValue(pos.x());
    ui->YPositionSpinBox->setValue(pos.y());
    ui->ZPositionSpinBox->setValue(pos.z());
}

void BoneDialog::setRotation(const vpvl::Quaternion &rot)
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

void BoneDialog::on_buttonBox_accepted()
{
    m_boneMotionModel->discardState();
}

void BoneDialog::on_buttonBox_rejected()
{
    m_boneMotionModel->restoreState();
}
