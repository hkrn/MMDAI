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

#include "widgets/LicenseWidget.h"

#include <QtGui/QtGui>
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
#include <QtWidgets/QtWidgets>
#endif
#include <vpvl2/config.h>

/* lupdate cannot parse tr() syntax correctly */

namespace {

static const int kNameIndex = 0;
static const int kLicenseIndex = 1;
static const int kWebsiteIndex = 2;

}

namespace vpvm
{

LicenseWidget::LicenseWidget(QWidget *parent)
    : QWidget(parent),
      m_model(new QStandardItemModel(0, 3)),
      m_text(new QTextEdit())
{
    QScopedPointer<QVBoxLayout> layout(new QVBoxLayout());
    QScopedPointer<QLabel> copyrightLabel(new QLabel());
    copyrightLabel->setText(QString("<h3>%1 %2+alpha</h3>"
                                    "<div style='font-size:10px'><p>"
                                    "Copyright (C) 2009-2011 Nagoya Institute of Technology Department of Computer Science (MMDAgent)<br>"
                                    "Copyright (C) 2010-2012 hkrn (MMDAI and VPVM)"
                                    "</p>"
                                    "<p>%3</p><p>%4</p></div>")
                            .arg(qApp->applicationName())
                            .arg(qApp->applicationVersion())
                            .arg(vpvm::LicenseWidget::tr("MMDAI2 (will be VPVM) is an application to edit or create a motion compatible with MMD ("
                                                         "<a href='http://www.geocities.jp/higuchuu4/index.htm'>MikuMikuDance</a> "
                                                         "created by Yuu Higuchi). This doesn't intend to be the successor of MMD."))
                            .arg(vpvm::LicenseWidget::tr("Below table is a list of libraries MMDAI2 uses. "
                                                         "Double click a row to show the license text or open the website")));
    copyrightLabel->setWordWrap(true);
    layout->addWidget(copyrightLabel.take());
    QTreeView *tree = new QTreeView();
    m_model->setHeaderData(0, Qt::Horizontal, vpvm::LicenseWidget::tr("Name"));
    m_model->setHeaderData(1, Qt::Horizontal, vpvm::LicenseWidget::tr("License"));
    m_model->setHeaderData(2, Qt::Horizontal, vpvm::LicenseWidget::tr("Website"));
    connect(tree, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(handleDoubleClick(QModelIndex)));
    tree->setRootIsDecorated(false);
    tree->setAlternatingRowColors(true);
    tree->setModel(m_model.data());
    tree->setEditTriggers(QTreeView::NoEditTriggers);
    layout->addWidget(tree);
    QScopedPointer<QLabel> aboutIconLabel(new QLabel());
    aboutIconLabel->setText(
                "<div style='font-size:10px'>" +
                vpvm::LicenseWidget::tr("MIKU Hatsune and other CV series are product of CRYPTON FUTURE MEDIA, INC.<br>"
                                        "VOCALOID is the trademark of YAMAHA Corporation.") + "</div>");
    aboutIconLabel->setWordWrap(true);
    aboutIconLabel->setOpenExternalLinks(true);
    aboutIconLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    layout->addWidget(aboutIconLabel.take());
    setWindowTitle(QString(vpvm::LicenseWidget::tr("About %1")).arg(qApp->applicationName()));
    resize(600, 450);
    setLayout(layout.take());

    addLibrary("Glyph Icons", "CC 3.0", "http://glyphicons.com/", "GlyphIcons");
#ifdef VPVL2_LINK_INTEL_TBB
    addLibrary("TBB", "GPL", "http://threadingbuildingblocks.org/", "TBB");
#endif
#ifdef VPVL2_LINK_NVTT
    addLibrary("NVIDIA texture tools", "MIT", "http://code.google.com/p/nvidia-texture-tools/", "nvtt");
#endif /* VPVL2_LINK_NVTT */
#ifdef VPVL2_LINK_DEVIL
    addLibrary("libpng", "zlib", "http://libpng.org", "libpng");
    addLibrary("libjpeg", "Custom", "http://ijg.org", "libjpeg");
    addLibrary("DevIL", "LGPL", "http://openil.sf.net", "DevIL");
#endif /* VPVL2_LINK_DEVIL */
#ifdef VPVL2_ENABLE_NVIDIA_CG
    addLibrary("Cg", "EULA", "http://www.nvidia.com/", "Cg");
#endif /* VPVL2_ENABLE_CG */
    addLibrary("minizip", "zlib", "http://www.winimage.com/zLibDll/minizip.html", "minizip");
    addLibrary("PortAudio", "MIT", "http://portaudio.com", "PortAudio");
    addLibrary("libav", "LGPL", "http://libav.org", "libav");
    addLibrary("zlib", "zlib", "http://zlib.net", "zlib");
    addLibrary("libiconv", "LGPL", "http://www.gnu.org/software/libiconv/", "libiconv");
    addLibrary("libxml2", "MIT", "http://xmlsoft.org", "libxml2");
#ifdef VPVL2_LINK_ASSIMP
    addLibrary("Open Asset Import Library", "New BSD", "http://assimp.sf.net", "Assimp");
#endif /* VPVL2_LINK_ASSIMP */
    addLibrary("BulletPhysics", "zlib", "http://bulletphysics.org/wordpress/", "Bullet");
    addLibrary("MMDAgent", "New BSD", "http://mmdagent.jp", "MMDAgent");
    addLibrary("libvpvl", "New BSD", "https://github.com/hkrn/MMDAI/", "libvpvl");
}

LicenseWidget::~LicenseWidget()
{
}

void LicenseWidget::addLibrary(const QString &name,
                               const QString &license,
                               const QString &website,
                               const QString &path)
{
    m_path.insert(name, path);
    m_model->insertRow(0);
    m_model->setData(m_model->index(0, kNameIndex), name);
    m_model->setData(m_model->index(0, kLicenseIndex), license);
    m_model->setData(m_model->index(0, kWebsiteIndex), website);
}

void LicenseWidget::handleDoubleClick(const QModelIndex &index)
{
    QVariant value;
    switch (index.column()) {
    case kNameIndex:
    case kLicenseIndex:
    {
        const QString &name = m_model->data(m_model->index(index.row(), 0)).toString();
        QFile file(QString(":/licenses/%1").arg(m_path[name]));
        if (file.exists() && file.open(QFile::ReadOnly | QFile::Text)) {
            QTextStream stream(&file);
            m_text->setWindowTitle(vpvm::LicenseWidget::tr("%1's license").arg(name));
            m_text->setReadOnly(true);
            m_text->setHtml(QString("<pre>%1</pre>").arg(stream.readAll()));
            m_text->resize(600, 500);
            m_text->setLineWrapMode(QTextEdit::NoWrap);
            m_text->show();
        }
        break;
    }
    case kWebsiteIndex:
        value = m_model->data(index);
        QDesktopServices::openUrl(value.toString());
        break;
    }
}

} /* namespace vpvm */
