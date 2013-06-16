/**

 Copyright (c) 2010-2013  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#include "BoneDialog.h"
#include "BoneMotionModel.h"
#include "BoneUIDelegate.h"
#include "MainWindow.h"

#include <QtGui/QtGui>
#include <vpvl2/qt/Util.h>

namespace vpvm
{

using namespace vpvl2::qt;

BoneUIDelegate::BoneUIDelegate(BoneMotionModel *bmm, QSettings *settings, MainWindow *parent) :
    QObject(parent),
    m_parentRef(parent),
    m_settingsRef(settings),
    m_boneMotionModelRef(bmm)
{
}

BoneUIDelegate::~BoneUIDelegate()
{
}

void BoneUIDelegate::resetBoneX()
{
    if (m_boneMotionModelRef->isBoneSelected()) {
        m_boneMotionModelRef->resetBone(BoneMotionModel::kX);
    }
    else {
        Util::warning(m_parentRef,
                      tr("The model or the bone is not selected."),
                      tr("Select a model or a bone to reset X position of the bone "
                         "(\"Model\" > \"Select model\" or double click a bone)"));
    }
}

void BoneUIDelegate::resetBoneY()
{
    if (m_boneMotionModelRef->isBoneSelected()) {
        m_boneMotionModelRef->resetBone(BoneMotionModel::kY);
    }
    else {
        Util::warning(m_parentRef,
                      tr("The model or the bone is not selected."),
                      tr("Select a model or a bone to reset Y position of the bone "
                         "(\"Model\" > \"Select model\" or double click a bone)"));
    }
}

void BoneUIDelegate::resetBoneZ()
{
    if (m_boneMotionModelRef->isBoneSelected()) {
        m_boneMotionModelRef->resetBone(BoneMotionModel::kZ);
    }
    else {
        Util::warning(m_parentRef,
                      tr("The model or the bone is not selected."),
                      tr("Select a model or a bone to reset Z position of the bone "
                         "(\"Model\" > \"Select model\" or double click a bone)"));
    }
}

void BoneUIDelegate::resetBoneRotation()
{
    if (m_boneMotionModelRef->isBoneSelected()) {
        m_boneMotionModelRef->resetBone(BoneMotionModel::kRotation);
    }
    else {
        Util::warning(m_parentRef,
                      tr("The model or the bone is not selected."),
                      tr("Select a model or a bone to reset rotation of the bone "
                         "(\"Model\" > \"Select model\" or double click a bone)"));
    }
}

void BoneUIDelegate::resetAllBones()
{
    if (m_boneMotionModelRef->isBoneSelected()) {
        m_boneMotionModelRef->resetAllBones();
    }
    else {
        Util::warning(m_parentRef,
                      tr("The model is not selected."),
                      tr("Select a model to reset bones (\"Model\" > \"Select model\")"));
    }
}

void BoneUIDelegate::openBoneDialog()
{
    if (m_boneMotionModelRef->isBoneSelected()) {
        BoneDialog *dialog = new BoneDialog(m_boneMotionModelRef, m_settingsRef, m_parentRef);
        dialog->exec();
    }
    else {
        Util::warning(m_parentRef,
                      tr("The model or the bone is not selected."),
                      tr("Select a model or a bone to open this dialog "
                         "(\"Model\" > \"Select model\" or double click a bone)"));
    }
}

} /* namespace vpvm */
