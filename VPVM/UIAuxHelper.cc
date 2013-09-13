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

#include "UIAuxHelper.h"

#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>

UIAuxHelper::UIAuxHelper(QObject *parent)
    : QObject(parent)
{
}

UIAuxHelper::~UIAuxHelper()
{
}

UIAuxHelper::ConfirmResponseType UIAuxHelper::confirmSaving()
{
    QMessageBox dialog;
    dialog.setText(tr("The project has been modified."));
    dialog.setInformativeText(tr("Do you want to save your changes?"));
    dialog.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    dialog.setDefaultButton(QMessageBox::Save);
    switch (dialog.exec()) {
    case QMessageBox::Save:
        return Save;
    case QMessageBox::Cancel:
        return Cancel;
    case QMessageBox::Discard:
    default:
        return Discard;
    }
}

QUrl UIAuxHelper::openSaveDialog(const QString &title,
                                 const QString &suffix,
                                 const QStringList &nameFilters)
{
    QFileDialog dialog;
    dialog.setWindowTitle(title);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilters(nameFilters);
    dialog.setDefaultSuffix(suffix);
    if (dialog.exec() == QDialog::Accepted) {
        const QStringList &selectedFiles = dialog.selectedFiles();
        return QUrl::fromLocalFile(selectedFiles.value(0));
    }
    return QUrl();
}

QString UIAuxHelper::slurpLicenseText(const QString &name)
{
    QFile file(QString(":licenses/%1").arg(name));
    if (file.open(QFile::ReadOnly)) {
        return file.readAll();
    }
    return QString();
}

void UIAuxHelper::openAboutQt()
{
    qApp->aboutQt();
}
