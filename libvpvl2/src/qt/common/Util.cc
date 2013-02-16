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

#include <vpvl2/qt/Util.h>
#include <vpvl2/vpvl2.h>

#include <QTextCodec>
#include <QDebug>
#include <QVector3D>
#include <QQuaternion>

static inline void VPVM2QtCommonInitializeResources()
{
    Q_INIT_RESOURCE(libvpvl2qtcommon);
}

static inline void VPVM2QtCommonCleanupResources()
{
    Q_CLEANUP_RESOURCE(libvpvl2qtcommon);
}

namespace vpvl2
{
namespace qt
{

void Util::initializeResources()
{
    VPVM2QtCommonInitializeResources();
}

void Util::cleanupResources()
{
    VPVM2QtCommonCleanupResources();
}

QString Util::toQString(const UnicodeString &value)
{
    return QString::fromUtf16(reinterpret_cast<const ushort *>(value.getBuffer()), value.length());
}

UnicodeString Util::fromQString(const QString &value)
{
    return UnicodeString(reinterpret_cast<const UChar *>(value.utf16()), value.size());
}

QTextCodec *Util::getTextCodec()
{
    static QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
    return codec;
}

const QString Util::noneString()
{
    static const QString none = QCoreApplication::tr("(none)");
    return none;
}

QByteArray Util::toByteArrayFromQString(const QString &value)
{
    const QByteArray &bytes = getTextCodec()->fromUnicode(value);
    return bytes;
}

QString Util::toQStringFromBytes(const uint8_t *value)
{
    const QString &s = getTextCodec()->toUnicode(reinterpret_cast<const char *>(value));
    return s;
}

QString Util::toQStringFromString(const IString *value)
{
    const QString &s = value ? Util::toQString(static_cast<const String *>(value)->value()) : noneString();
    return s;
}

QString Util::toQStringFromModel(const IModel *value)
{
    const QString &s = value ? toQStringFromString(value->name()) : noneString();
    return s;
}

QString Util::toQStringFromMotion(const IMotion *value)
{
    const QString &s = value ? toQStringFromString(value->name()) : noneString();
    return s;
}

QString Util::toQStringFromBone(const IBone *value)
{
    const QString &s = value ? toQStringFromString(value->name()) : noneString();
    return s;
}

QString Util::toQStringFromMorph(const IMorph *value)
{
    const QString &s = value ? toQStringFromString(value->name()) : noneString();
    return s;
}

QString Util::toQStringFromBoneKeyframe(const IBoneKeyframe *value)
{
    const QString &s = value ? toQStringFromString(value->name()) : noneString();
    return s;
}
QString Util::toQStringFromMorphKeyframe(const IMorphKeyframe *value)
{
    const QString &s = value ? toQStringFromString(value->name()) : noneString();
    return s;
}

void Util::dumpBones(const IModel *model)
{
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

void Util::dumpBoneKeyFrame(const IBoneKeyframe *frame, int index)
{
    const Vector3 &p = frame->localPosition();
    const Quaternion &q = frame->localRotation();
    qDebug().nospace() << "index=" << index
                       << " timeIndex=" << frame->timeIndex()
                       << " name=" << toQStringFromBoneKeyframe(frame)
                       << " position=" << QVector3D(p.x(), p.y(), p.z())
                       << " rotation=" << QQuaternion(q.w(), q.x(), q.y(), q.z());
}

void Util::dumpBoneKeyFrames(const IMotion *motion)
{
    const int nframes = motion->countKeyframes(IKeyframe::kBoneKeyframe);
    for (int i = 0; i < nframes; i++)
        dumpBoneKeyFrame(motion->findBoneKeyframeAt(i), i);
}

} /* namespace qt */
} /* namespace vpvl2 */

