/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2011  hkrn (libMMDAI)                         */
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

#include "QMALogViewWidget.h"

#include <QtGui>

#include "QMALogger.h"

QMALogViewWidget::QMALogViewWidget(QWidget *parent) :
    QWidget(parent)
{
  QMALogger *logger = QMALogger::getInstance();
  connect(logger, SIGNAL(lineWritten(QString)), this, SLOT(addLine(QString)));
  m_text = new QTextEdit();
  m_text->setReadOnly(true);
  m_text->setLineWrapMode(QTextEdit::NoWrap);
  QVBoxLayout *layout = new QVBoxLayout();
  layout->addWidget(m_text);
  layout->setMargin(10);
  setLayout(layout);
  setWindowTitle("QtMMDAI Log window");
  resize(QSize(640, 320));
}

void QMALogViewWidget::closeEvent(QCloseEvent *event)
{
  event->accept();
}

void QMALogViewWidget::showEvent(QShowEvent *event)
{
  Q_UNUSED(event);
  m_text->setPlainText(m_contents.join(""));
}

void QMALogViewWidget::addLine(const QString &line)
{
  m_contents.append(line);
  if (isVisible())
    m_text->setPlainText(m_contents.join(""));
}
