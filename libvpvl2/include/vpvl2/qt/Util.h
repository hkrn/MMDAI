/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
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

#ifndef VPVL2_RENDER_QT_UTIL_H_
#define VPVL2_RENDER_QT_UTIL_H_

#include <vpvl2/IModel.h>
#include <vpvl2/IMotion.h>
#include <vpvl2/IBone.h>
#include <vpvl2/IBoneKeyframe.h>
#include <vpvl2/IMorph.h>
#include <vpvl2/IMorphKeyframe.h>
#include <vpvl2/qt/Common.h>
#include <vpvl2/extensions/icu/String.h>

#include <QApplication>
#include <QDir>
#include <QSettings>
#include <QString>
#include <QTextCodec>
#include <QQuaternion>
#include <QVector3D>
#include <QMatrix4x4>
#include <QMessageBox>
#include <QFileDialog>

namespace vpvl2
{
namespace qt
{
using namespace extensions::icu;

class VPVL2QTCOMMON_API Util
{
public:
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
    static QString toQString(const UnicodeString &value) {
        const std::string &s = String::toStdString(value);
        return QString::fromUtf8(s.c_str(), s.length());
    }
    static UnicodeString fromQString(const QString &value) {
        const QByteArray &bytes = value.toUtf8();
        return UnicodeString::fromUTF8(StringPiece(bytes.constData(), bytes.length()));
    }
    static QTextCodec *getTextCodec() {
        static QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
        return codec;
    }
    static const QString noneString() {
        static const QString none = QCoreApplication::tr("(none)");
        return none;
    }
    static QByteArray toByteArrayFromQString(const QString &value) {
        const QByteArray &bytes = getTextCodec()->fromUnicode(value);
        return bytes;
    }
    static QString toQStringFromBytes(const uint8_t *value) {
        const QString &s = getTextCodec()->toUnicode(reinterpret_cast<const char *>(value));
        return s;
    }
    static QString toQStringFromString(const IString *value) {
        const QString &s = value ? Util::toQString(static_cast<const String *>(value)->value()) : noneString();
        return s;
    }
    static QString toQStringFromModel(const IModel *value) {
        const QString &s = value ? toQStringFromString(value->name()) : noneString();
        return s;
    }
    static QString toQStringFromMotion(const IMotion *value) {
        const QString &s = value ? toQStringFromString(value->name()) : noneString();
        return s;
    }
    static QString toQStringFromBone(const IBone *value) {
        const QString &s = value ? toQStringFromString(value->name()) : noneString();
        return s;
    }
    static QString toQStringFromMorph(const IMorph *value) {
        const QString &s = value ? toQStringFromString(value->name()) : noneString();
        return s;
    }
    static QString toQStringFromBoneKeyframe(const IBoneKeyframe *value) {
        const QString &s = value ? toQStringFromString(value->name()) : noneString();
        return s;
    }
    static QString toQStringFromMorphKeyframe(const IMorphKeyframe *value) {
        const QString &s = value ? toQStringFromString(value->name()) : noneString();
        return s;
    }
    static void dumpBones(const IModel *model) {
        Array<IBone *> bones;
        model->getBoneRefs(bones);
        const int nbones = bones.count();
        for (int i = 0; i < nbones; i++) {
            IBone *bone = bones[i];
            const Transform &transform = bone->worldTransform();
            const Vector3 &p = transform.getOrigin();
            const Quaternion &q = transform.getRotation();
            qDebug().nospace() << "index=" << i
                               << " name=" << toQStringFromBone(bone)
                               << " position=" << QVector3D(p.x(), p.y(), p.z())
                               << " rotation=" << QQuaternion(q.w(), q.x(), q.y(), q.z());
        }
    }
    static void dumpBoneKeyFrame(const IBoneKeyframe *frame, int index = 0) {
        const Vector3 &p = frame->localPosition();
        const Quaternion &q = frame->localRotation();
        qDebug().nospace() << "index=" << index
                           << " timeIndex=" << frame->timeIndex()
                           << " name=" << toQStringFromBoneKeyframe(frame)
                           << " position=" << QVector3D(p.x(), p.y(), p.z())
                           << " rotation=" << QQuaternion(q.w(), q.x(), q.y(), q.z());
    }
    static void dumpBoneKeyFrames(const IMotion *motion) {
        const int nframes = motion->countKeyframes(IKeyframe::kBoneKeyframe);
        for (int i = 0; i < nframes; i++)
            dumpBoneKeyFrame(motion->findBoneKeyframeAt(i), i);
    }
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
    static const QString openFileDialog(const QString &name,
                                        const QString &desc,
                                        const QString &exts,
                                        QSettings *settings) {
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
                                               QSettings *settings) {
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

private:
    VPVL2_MAKE_STATIC_CLASS(Util)
};

} /* namespace qt */
} /* namespace vpvl2 */

#endif
