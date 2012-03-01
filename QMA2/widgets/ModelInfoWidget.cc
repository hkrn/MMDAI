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
#include <vpvl/vpvl.h>

using namespace vpvl;

ModelInfoWidget::ModelInfoWidget(QWidget *parent) :
    QWidget(parent),
    m_edgeColorDialog(0)
{
    m_nameLabel = new QLabel();
    m_nameValueLabel = new QLineEdit();
    m_nameValueLabel->setReadOnly(true);
    m_commentLabel = new QLabel();
    m_commentValueLabel = new QTextEdit();
    m_commentValueLabel->setReadOnly(true);
    m_verticesCountLabel = new QLabel();
    m_verticesCountValueLabel = new QLineEdit();
    m_verticesCountValueLabel->setReadOnly(true);
    m_verticesCountValueLabel->setAlignment(Qt::AlignRight);
    m_indicesCountLabel = new QLabel();
    m_indicesCountValueLabel = new QLineEdit();
    m_indicesCountValueLabel->setReadOnly(true);
    m_indicesCountValueLabel->setAlignment(Qt::AlignRight);
    m_materialsCountLabel = new QLabel();
    m_materialsCountValueLabel = new QLineEdit();
    m_materialsCountValueLabel->setReadOnly(true);
    m_materialsCountValueLabel->setAlignment(Qt::AlignRight);
    m_bonesCountLabel = new QLabel();
    m_bonesCountValueLabel = new QLineEdit();
    m_bonesCountValueLabel->setReadOnly(true);
    m_bonesCountValueLabel->setAlignment(Qt::AlignRight);
    m_IKsCountLabel = new QLabel();
    m_IKsCountValueLabel = new QLineEdit();
    m_IKsCountValueLabel->setReadOnly(true);
    m_IKsCountValueLabel->setAlignment(Qt::AlignRight);
    m_morphsCountLabel = new QLabel();
    m_morphsCountValueLabel = new QLineEdit();
    m_morphsCountValueLabel->setReadOnly(true);
    m_morphsCountValueLabel->setAlignment(Qt::AlignRight);
    m_rigidBodiesCountLabel = new QLabel();
    m_rigidBodiesCountValueLabel = new QLineEdit();
    m_rigidBodiesCountValueLabel->setReadOnly(true);
    m_rigidBodiesCountValueLabel->setAlignment(Qt::AlignRight);
    m_constrantsCountLabel = new QLabel();
    m_constrantsCountValueLabel = new QLineEdit();
    m_constrantsCountValueLabel->setReadOnly(true);
    m_constrantsCountValueLabel->setAlignment(Qt::AlignRight);
    m_edgeOffsetLabel = new QLabel();
    m_edgeOffsetSpinBox = new QDoubleSpinBox();
    connect(m_edgeOffsetSpinBox, SIGNAL(valueChanged(double)), SIGNAL(edgeOffsetDidChange(double)));
    m_edgeOffsetSpinBox->setEnabled(false);
    m_edgeOffsetSpinBox->setSingleStep(0.1);
    m_edgeOffsetSpinBox->setRange(0.0, 2.0);
    m_edgeColorDialogOpenButton = new QPushButton();
    connect(m_edgeColorDialogOpenButton, SIGNAL(clicked()), SLOT(openEdgeColorDialog()));
    m_projectiveShadowCheckbox = new QCheckBox();
    connect(m_projectiveShadowCheckbox, SIGNAL(toggled(bool)), SIGNAL(projectiveShadowDidChange(bool)));
    m_projectiveShadowCheckbox->setEnabled(false);
    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addWidget(m_nameLabel);
    mainLayout->addWidget(m_nameValueLabel);
    mainLayout->addWidget(m_commentLabel);
    mainLayout->addWidget(m_commentValueLabel);
    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->addWidget(m_verticesCountLabel, 0, 0);
    gridLayout->addWidget(m_verticesCountValueLabel, 1, 0);
    gridLayout->addWidget(m_indicesCountLabel, 0, 1);
    gridLayout->addWidget(m_indicesCountValueLabel, 1, 1);
    gridLayout->addWidget(m_materialsCountLabel, 2, 0);
    gridLayout->addWidget(m_materialsCountValueLabel, 3, 0);
    gridLayout->addWidget(m_bonesCountLabel, 2, 1);
    gridLayout->addWidget(m_bonesCountValueLabel, 3, 1);
    gridLayout->addWidget(m_IKsCountLabel, 4, 0);
    gridLayout->addWidget(m_IKsCountValueLabel, 5, 0);
    gridLayout->addWidget(m_morphsCountLabel, 4, 1);
    gridLayout->addWidget(m_morphsCountValueLabel, 5, 1);
    gridLayout->addWidget(m_rigidBodiesCountLabel, 6, 0);
    gridLayout->addWidget(m_rigidBodiesCountValueLabel, 7, 0);
    gridLayout->addWidget(m_constrantsCountLabel, 6, 1);
    gridLayout->addWidget(m_constrantsCountValueLabel, 7, 1);
    mainLayout->addLayout(gridLayout);
    QHBoxLayout *subLayout = new QHBoxLayout();
    subLayout->setAlignment(Qt::AlignCenter);
    subLayout->addWidget(m_edgeOffsetLabel);
    subLayout->addWidget(m_edgeOffsetSpinBox);
    subLayout->addWidget(m_edgeColorDialogOpenButton);
    mainLayout->addLayout(subLayout);
    mainLayout->addWidget(m_projectiveShadowCheckbox, 0, Qt::AlignCenter);
    mainLayout->addStretch();
    setLayout(mainLayout);
    retranslate();
    setModel(0, 0);
}

ModelInfoWidget::~ModelInfoWidget()
{
}

void ModelInfoWidget::retranslate()
{
    m_nameLabel->setText(tr("Name:"));
    m_commentLabel->setText(tr("Comment:"));
    m_verticesCountLabel->setText(tr("Number of vertices:"));
    m_indicesCountLabel->setText(tr("Number of indices:"));
    m_materialsCountLabel->setText(tr("Number of materials:"));
    m_bonesCountLabel->setText(tr("Number of bones:"));
    m_IKsCountLabel->setText(tr("Number of IKs:"));
    m_morphsCountLabel->setText(tr("Number of morphs:"));
    m_rigidBodiesCountLabel->setText(tr("Number of rigid bodies:"));
    m_constrantsCountLabel->setText(tr("Number of constraints:"));
    m_edgeOffsetLabel->setText(tr("Edge offset:"));
    m_edgeColorDialogOpenButton->setText(tr("Color"));
    m_projectiveShadowCheckbox->setText(tr("Enable projective shadow"));
}

void ModelInfoWidget::openEdgeColorDialog()
{
    createEdgeColorDialog();
    m_edgeColorDialog->show();
}

void ModelInfoWidget::setModel(PMDModel *model, SceneLoader *loader)
{
    if (model) {
        createEdgeColorDialog();
        m_nameValueLabel->setText(internal::toQString(model->name()));
        m_commentValueLabel->setText(internal::toQString(model->comment()));
        m_verticesCountValueLabel->setText(QString().sprintf("%d", model->vertices().count()));
        m_indicesCountValueLabel->setText(QString().sprintf("%d", model->indices().count()));
        m_materialsCountValueLabel->setText(QString().sprintf("%d", model->materials().count()));
        m_bonesCountValueLabel->setText(QString().sprintf("%d", model->bones().count()));
        m_IKsCountValueLabel->setText(QString().sprintf("%d", model->IKs().count()));
        m_morphsCountValueLabel->setText(QString().sprintf("%d", model->faces().count()));
        m_rigidBodiesCountValueLabel->setText(QString().sprintf("%d", model->rigidBodies().count()));
        m_constrantsCountValueLabel->setText(QString().sprintf("%d", model->constraints().count()));
        m_edgeOffsetSpinBox->setValue(model->edgeOffset());
        m_edgeOffsetSpinBox->setEnabled(true);
        m_projectiveShadowCheckbox->setEnabled(true);
        const Vector3 &color = model->edgeColor();
        QColor c;
        c.setRedF(color.x());
        c.setGreenF(color.y());
        c.setBlueF(color.z());
        c.setAlphaF(1.0);
        m_edgeColorDialog->setCurrentColor(c);
        if (loader) {
            m_projectiveShadowCheckbox->setChecked(loader->isProjectiveShadowEnabled(model));
        }
        else {
            m_projectiveShadowCheckbox->setChecked(false);
        }
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
        m_edgeOffsetSpinBox->setValue(0.0f);
        m_edgeOffsetSpinBox->setEnabled(false);
        m_projectiveShadowCheckbox->setEnabled(false);
    }
}

void ModelInfoWidget::createEdgeColorDialog()
{
    if (!m_edgeColorDialog) {
        m_edgeColorDialog = new QColorDialog(this);
        connect(m_edgeColorDialog, SIGNAL(currentColorChanged(QColor)), SIGNAL(edgeColorDidChange(QColor)));
        m_edgeColorDialog->setOption(QColorDialog::NoButtons);
    }
}
