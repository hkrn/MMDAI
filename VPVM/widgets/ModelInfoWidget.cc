/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2012  hkrn                                    */
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

#include "ModelInfoWidget.h"
#include "common/SceneLoader.h"
#include "common/util.h"

#include <QtGui/QtGui>
#include <vpvl2/vpvl2.h>

namespace vpvm
{

using namespace vpvl2;

ModelInfoWidget::ModelInfoWidget(QWidget *parent)
    : QWidget(parent),
      m_nameLabel(new QLabel()),
      m_nameValueLabel(new QLineEdit()),
      m_commentLabel(new QLabel()),
      m_commentValueLabel(new QTextEdit()),
      m_verticesCountLabel(new QLabel()),
      m_verticesCountValueLabel(new QLineEdit()),
      m_indicesCountLabel(new QLabel()),
      m_indicesCountValueLabel(new QLineEdit()),
      m_materialsCountLabel(new QLabel()),
      m_materialsCountValueLabel(new QLineEdit()),
      m_bonesCountLabel(new QLabel()),
      m_bonesCountValueLabel(new QLineEdit()),
      m_IKsCountLabel(new QLabel()),
      m_IKsCountValueLabel(new QLineEdit()),
      m_morphsCountLabel(new QLabel()),
      m_morphsCountValueLabel(new QLineEdit()),
      m_rigidBodiesCountLabel(new QLabel()),
      m_rigidBodiesCountValueLabel(new QLineEdit()),
      m_constrantsCountLabel(new QLabel()),
      m_constrantsCountValueLabel(new QLineEdit())
{
    /* モデル名 */
    m_nameValueLabel->setReadOnly(true);
    /* モデルのコメント */
    m_commentValueLabel->setReadOnly(true);
    /* 頂点数 */
    m_verticesCountValueLabel->setReadOnly(true);
    m_verticesCountValueLabel->setAlignment(Qt::AlignRight);
    /* インデックス数 */
    m_indicesCountValueLabel->setReadOnly(true);
    m_indicesCountValueLabel->setAlignment(Qt::AlignRight);
    /* 材質数 */
    m_materialsCountValueLabel->setReadOnly(true);
    m_materialsCountValueLabel->setAlignment(Qt::AlignRight);
    /* ボーン数 */
    m_bonesCountValueLabel->setReadOnly(true);
    m_bonesCountValueLabel->setAlignment(Qt::AlignRight);
    /* IK 数 */
    m_IKsCountValueLabel->setReadOnly(true);
    m_IKsCountValueLabel->setAlignment(Qt::AlignRight);
    /* モーフ数 */
    m_morphsCountValueLabel->setReadOnly(true);
    m_morphsCountValueLabel->setAlignment(Qt::AlignRight);
    /* 剛体数 */
    m_rigidBodiesCountValueLabel->setReadOnly(true);
    m_rigidBodiesCountValueLabel->setAlignment(Qt::AlignRight);
    /* コンストレイント(拘束条件)数 */
    m_constrantsCountValueLabel->setReadOnly(true);
    m_constrantsCountValueLabel->setAlignment(Qt::AlignRight);
    /* 構築 */
    QScopedPointer<QVBoxLayout> mainLayout(new QVBoxLayout());
    mainLayout->addWidget(m_nameLabel.data());
    mainLayout->addWidget(m_nameValueLabel.data());
    mainLayout->addWidget(m_commentLabel.data());
    mainLayout->addWidget(m_commentValueLabel.data());
    QScopedPointer<QGridLayout> gridLayout(new QGridLayout());
    gridLayout->addWidget(m_verticesCountLabel.data(), 0, 0);
    gridLayout->addWidget(m_verticesCountValueLabel.data(), 1, 0);
    gridLayout->addWidget(m_indicesCountLabel.data(), 0, 1);
    gridLayout->addWidget(m_indicesCountValueLabel.data(), 1, 1);
    gridLayout->addWidget(m_materialsCountLabel.data(), 2, 0);
    gridLayout->addWidget(m_materialsCountValueLabel.data(), 3, 0);
    gridLayout->addWidget(m_bonesCountLabel.data(), 2, 1);
    gridLayout->addWidget(m_bonesCountValueLabel.data(), 3, 1);
    gridLayout->addWidget(m_IKsCountLabel.data(), 4, 0);
    gridLayout->addWidget(m_IKsCountValueLabel.data(), 5, 0);
    gridLayout->addWidget(m_morphsCountLabel.data(), 4, 1);
    gridLayout->addWidget(m_morphsCountValueLabel.data(), 5, 1);
    gridLayout->addWidget(m_rigidBodiesCountLabel.data(), 6, 0);
    gridLayout->addWidget(m_rigidBodiesCountValueLabel.data(), 7, 0);
    gridLayout->addWidget(m_constrantsCountLabel.data(), 6, 1);
    gridLayout->addWidget(m_constrantsCountValueLabel.data(), 7, 1);
    mainLayout->addLayout(gridLayout.take());
    mainLayout->addStretch();
    setLayout(mainLayout.take());
    retranslate();
}

ModelInfoWidget::~ModelInfoWidget()
{
}

void ModelInfoWidget::retranslate()
{
    m_nameLabel->setText(tr("Name:"));
    m_commentLabel->setText(tr("Comment:"));
    m_verticesCountLabel->setText(tr("Number of Vertices:"));
    m_indicesCountLabel->setText(tr("Number of Indices:"));
    m_materialsCountLabel->setText(tr("Number of Materials:"));
    m_bonesCountLabel->setText(tr("Number of Bones:"));
    m_IKsCountLabel->setText(tr("Number of IKs:"));
    m_morphsCountLabel->setText(tr("Number of Morphs:"));
    m_rigidBodiesCountLabel->setText(tr("Number of Rigid bodies:"));
    m_constrantsCountLabel->setText(tr("Number of Joints:"));
}

void ModelInfoWidget::setModel(IModelSharedPtr model)
{
    if (model) {
        m_nameValueLabel->setText(toQStringFromModel(model.data()));
        m_commentValueLabel->setText(toQStringFromString(model->comment()));
        m_verticesCountValueLabel->setText(QString().sprintf("%d", model->count(IModel::kVertex)));
        m_indicesCountValueLabel->setText(QString().sprintf("%d", model->count(IModel::kIndex)));
        m_materialsCountValueLabel->setText(QString().sprintf("%d", model->count(IModel::kMaterial)));
        m_bonesCountValueLabel->setText(QString().sprintf("%d", model->count(IModel::kBone)));
        m_IKsCountValueLabel->setText(QString().sprintf("%d", model->count(IModel::kIK)));
        m_morphsCountValueLabel->setText(QString().sprintf("%d", model->count(IModel::kMorph)));
        m_rigidBodiesCountValueLabel->setText(QString().sprintf("%d", model->count(IModel::kRigidBody)));
        m_constrantsCountValueLabel->setText(QString().sprintf("%d", model->count(IModel::kJoint)));
        qDebug("Set a model to ModelInfoWidget: %s", qPrintable(toQStringFromModel(model.data())));
    }
    else {
        m_nameValueLabel->setText("N/A");
        m_commentValueLabel->setText("N/A");
        m_verticesCountValueLabel->setText("0");
        m_indicesCountValueLabel->setText("0");
        m_materialsCountValueLabel->setText("0");
        m_bonesCountValueLabel->setText("0");
        m_IKsCountValueLabel->setText("0");
        m_morphsCountValueLabel->setText("0");
        m_rigidBodiesCountValueLabel->setText("0");
        m_constrantsCountValueLabel->setText("0");
        qDebug("Reset ModelInfoWidget");
    }
}

} /* namespace vpvm */
