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

#ifndef VPVL2_RENDER_QT_UTIL_H_
#define VPVL2_RENDER_QT_UTIL_H_

#include <vpvl2/qt/Common.h>
#include <vpvl2/extensions/icu4c/Encoding.h>

#include <QApplication>
#include <QDir>
#include <QSettings>
#include <QString>
#include <QMessageBox>
#include <QFileDialog>

namespace vpvl2
{

class IBone;
class IBoneKeyframe;
class IModel;
class IMorph;
class IMorphKeyframe;
class IMotion;

namespace qt
{
using namespace extensions::icu4c;

class VPVL2QTCOMMON_API Util
{
public:
    static bool initializeOnce(const char *argv0);
    static void terminate();
    static void loadDictionary(Encoding::Dictionary *dictionary);
    static QString toQString(const UnicodeString &value);
    static UnicodeString fromQString(const QString &value);
    static QTextCodec *getTextCodec();
    static const QString noneString();
    static QByteArray toByteArrayFromQString(const QString &value);
    static QString toQStringFromBytes(const uint8 *value);
    static QString toQStringFromString(const IString *value);
    static QString toQStringFromModel(const IModel *value);
    static QString toQStringFromMotion(const IMotion *value);
    static QString toQStringFromBone(const IBone *value);
    static QString toQStringFromMorph(const IMorph *value);
    static QString toQStringFromBoneKeyframe(const IBoneKeyframe *value);
    static QString toQStringFromMorphKeyframe(const IMorphKeyframe *value);
    static void dumpBones(const IModel *model);
    static void dumpBoneKeyFrame(const IBoneKeyframe *frame, int index = 0);
    static void dumpBoneKeyFrames(const IMotion *motion);

    static int warning(QWidget *parent,
                       const QString &title,
                       const QString &text,
                       const QString &detail = "",
                       QMessageBox::StandardButtons buttons = QMessageBox::Ok)
    {
        QScopedPointer<QMessageBox> mbox(new QMessageBox(parent));
        if (parent)
            mbox->setWindowModality(Qt::WindowModal);
        mbox->setWindowTitle(QString("%1 - %2").arg(qAppName()).arg(title));
        mbox->setText(text);
        if (!detail.isEmpty())
            mbox->setInformativeText(detail);
        mbox->setIcon(QMessageBox::Warning);
        mbox->setStandardButtons(buttons);
        return mbox->exec();
    }
    static const inline QString openFileDialog(const QString &name,
                                               const QString &desc,
                                               const QString &exts,
                                               QSettings *settings)
    {
        /* ファイルが選択されている場合はファイルが格納されているディレクトリを指す絶対パスを設定に保存しておく */
        const QString &path = settings->value(name, QDir::homePath()).toString();
        const QString &fileName = QFileDialog::getOpenFileName(0, desc, path, exts);
        if (!fileName.isEmpty()) {
            QDir dir(fileName);
            dir.cdUp();
            settings->setValue(name, dir.absolutePath());
        }
        return fileName;
    }
    static const inline QString openSaveDialog(const QString &name,
                                               const QString &desc,
                                               const QString &exts,
                                               const QString &defaultFilename,
                                               QSettings *settings)
    {
        const QDir base(settings->value(name, QDir::homePath()).toString());
        const QString &path = base.absoluteFilePath(defaultFilename);
        const QString &fileName = QFileDialog::getSaveFileName(0, desc, path, exts);
        if (!fileName.isEmpty()) {
            QDir dir(fileName);
            dir.cdUp();
            settings->setValue(name, dir.absolutePath());
        }
        return fileName;
    }
    template<typename T>
    static bool compare(const QList<T *> &left, const QList<T *> &right) {
        const int nitems = left.size();
        if (nitems == 0) {
            return true;
        }
        else if (nitems == right.size()) {
            /* 中身の全てのポインタのアドレスが両方の配列で同じであるかどうかを確認 */
            return memcmp(&left[0], &right[0], sizeof(void *) * nitems) == 0;
        }
        return false;
    }

private:
    VPVL2_MAKE_STATIC_CLASS(Util)
};

} /* namespace qt */
} /* namespace vpvl2 */

#endif
