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

#include "RenderOrderDialog.h"
#include "common/SceneLoader.h"

#include <QtGui/QtGui>

RenderOrderDialog::RenderOrderDialog(SceneLoader *loader, QWidget *parent)
    : QDialog(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    m_listWidget = new QListWidget();
    QDialogButtonBox *buttons = new QDialogButtonBox();
    connect(buttons, SIGNAL(accepted()), SLOT(accept()));
    connect(buttons, SIGNAL(rejected()), SLOT(reject()));
    connect(this, SIGNAL(accepted()), SLOT(emitSignal()));
    connect(this, SIGNAL(renderOrderListDidSet(QList<QUuid>)), loader, SLOT(setRenderOrderList(QList<QUuid>)));
    mainLayout->addWidget(buttons);
    mainLayout->addWidget(m_listWidget);
    setRenderOrder(loader);
    setLayout(mainLayout);
}

RenderOrderDialog::~RenderOrderDialog()
{
}

void RenderOrderDialog::emitSignal()
{
    QList<QUuid> value;
    int nitems = m_listWidget->count();
    for (int i = 0; i < nitems; i++) {
        QListWidgetItem *item = m_listWidget->item(i);
        const QVariant &var = item->data(0);
        value.append(QUuid(var.toString()));
    }
    emit renderOrderListDidSet(value);
}

void RenderOrderDialog::setRenderOrder(SceneLoader *loader)
{
    const QList<QUuid> &value = loader->renderOrderList();
    m_listWidget->clear();
    foreach (const QUuid &uuid, value) {
        QListWidgetItem *item = new QListWidgetItem(m_listWidget);
        item->setText(uuid);
        item->setData(0, QVariant(uuid));
    }
}
