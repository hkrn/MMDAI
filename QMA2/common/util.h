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

#ifndef UTIL_H
#define UTIL_H

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

namespace internal
{

const static vpvl::Vector3 kWorldAabbSize(vpvl::Scene::kFrustumFar, vpvl::Scene::kFrustumFar, vpvl::Scene::kFrustumFar);

static inline QTextCodec *getTextCodec()
{
    static QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
    return codec;
}

static inline const QString &noneString()
{
    static const QString none = QApplication::tr("(none)");
    return none;
}

static inline const QByteArray fromQString(const QString &value)
{
    return getTextCodec()->fromUnicode(value);
}

static inline const QString toQString(const uint8_t *value)
{
    return getTextCodec()->toUnicode(reinterpret_cast<const char *>(value));
}

static inline const QString toQString(const vpvl::Asset *value)
{
    return value ? QString(value->name()) : noneString();
}

static inline const QString toQString(const vpvl::PMDModel *value)
{
    return value ? toQString(value->name()) : noneString();
}

static inline const QString toQString(const vpvl::Bone *value)
{
    return value ? toQString(value->name()) : noneString();
}

static inline const QString toQString(const vpvl::Face *value)
{
    return value ? toQString(value->name()) : noneString();
}

static inline const QString toQString(const vpvl::BoneKeyFrame *value)
{
    return value ? toQString(value->name()) : noneString();
}

static inline const QString toQString(const vpvl::FaceKeyFrame *value)
{
    return value ? toQString(value->name()) : noneString();
}

static inline void dumpBones(vpvl::PMDModel *model)
{
    const vpvl::BoneList &bones = model->bones();
    const int nbones = bones.count();
    for (int i = 0; i < nbones; i++) {
        vpvl::Bone *bone = bones[i];
        const vpvl::Transform &transform = bone->localTransform();
        const vpvl::Vector3 &p = transform.getOrigin();
        const vpvl::Quaternion &q = transform.getRotation();
        qDebug().nospace() << "index=" << i
                           << " name=" << internal::toQString(bone)
                           << " position=" << QVector3D(p.x(), p.y(), p.z())
                           << " rotation=" << QQuaternion(q.w(), q.x(), q.y(), q.z());
    }
}

static inline void dumpBoneKeyFrame(const vpvl::BoneKeyFrame *frame, int index = 0)
{
    const vpvl::Vector3 &p = frame->position();
    const vpvl::Quaternion &q = frame->rotation();
    qDebug().nospace() << "index=" << index
                       << " frameIndex=" << frame->frameIndex()
                       << " name=" << internal::toQString(frame)
                       << " position=" << QVector3D(p.x(), p.y(), p.z())
                       << " rotation=" << QQuaternion(q.w(), q.x(), q.y(), q.z());
}

static inline void dumpBoneAnimation(const vpvl::BoneAnimation &animation)
{
    const int nframes = animation.countKeyFrames();
    for (int i = 0; i < nframes; i++)
        dumpBoneKeyFrame(static_cast<vpvl::BoneKeyFrame *>(animation.frameAt(i)), i);
}

static inline void dumpBoneKeyFrames(const vpvl::BaseKeyFrameList &frames)
{
    const int nframes = frames.count();
    for (int i = 0; i < nframes; i++)
        dumpBoneKeyFrame(static_cast<vpvl::BoneKeyFrame *>(frames[i]), i);
}


static inline const QMatrix4x4 toMatrix4x4(float matrixf[16])
{
    qreal matrixd[16];
    for (int i = 0; i < 16; i++)
        matrixd[i] = matrixf[i];
    return QMatrix4x4(matrixd);
}

}

#endif // UTIL_H
