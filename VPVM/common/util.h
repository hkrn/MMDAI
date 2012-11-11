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

#ifndef VPVM_UTIL_H
#define VPVM_UTIL_H

#include <QtGui/QtGui>
#include <vpvl2/vpvl2.h>
#include "StringHelper.h"

#if !defined(NDEBUG) && !defined(QMA_DEBUG)
#include <vpvl2/pmd2/Bone.h>
#include <vpvl2/pmd2/Joint.h>
#include <vpvl2/pmd2/Label.h>
#include <vpvl2/pmd2/Model.h>
#include <vpvl2/pmd2/Morph.h>
#include <vpvl2/pmd2/RigidBody.h>
#include <vpvl2/pmd2/Vertex.h>
#include <vpvl2/pmx/Bone.h>
#include <vpvl2/pmx/Joint.h>
#include <vpvl2/pmx/Label.h>
#include <vpvl2/pmx/Material.h>
#include <vpvl2/pmx/Model.h>
#include <vpvl2/pmx/Morph.h>
#include <vpvl2/pmx/RigidBody.h>
#include <vpvl2/pmx/Vertex.h>
#include <vpvl2/vmd/BoneAnimation.h>
#include <vpvl2/vmd/BoneKeyframe.h>
#include <vpvl2/vmd/CameraAnimation.h>
#include <vpvl2/vmd/CameraKeyframe.h>
#include <vpvl2/vmd/LightAnimation.h>
#include <vpvl2/vmd/LightKeyframe.h>
#include <vpvl2/vmd/MorphAnimation.h>
#include <vpvl2/vmd/MorphKeyframe.h>
#include <vpvl2/vmd/Motion.h>
#define QMA_DEBUG
#endif

namespace vpvm
{

using namespace vpvl2;

static inline void dumpBones(IModel *model)
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

static inline void dumpBoneKeyFrame(const IBoneKeyframe *frame, int index = 0)
{
    const Vector3 &p = frame->localPosition();
    const Quaternion &q = frame->localRotation();
    qDebug().nospace() << "index=" << index
                       << " timeIndex=" << frame->timeIndex()
                       << " name=" << toQStringFromBoneKeyframe(frame)
                       << " position=" << QVector3D(p.x(), p.y(), p.z())
                       << " rotation=" << QQuaternion(q.w(), q.x(), q.y(), q.z());
}

static inline void dumpBoneKeyFrames(const IMotion *motion)
{
    const int nframes = motion->countKeyframes(IKeyframe::kBone);
    for (int i = 0; i < nframes; i++)
        dumpBoneKeyFrame(motion->findBoneKeyframeAt(i), i);
}

static inline const QMatrix4x4 toMatrix4x4(float matrixf[16])
{
    qreal matrixd[16];
    for (int i = 0; i < 16; i++)
        matrixd[i] = matrixf[i];
    return QMatrix4x4(matrixd);
}

static inline int warning(QWidget *parent,
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

static inline const QString openFileDialog(const QString &name,
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

static const inline Vector3 vec2vec(const QVector4D &value)
{
    Vector3 v(value.x(), value.y(), value.z());
    return v;
}

template<typename T>
static inline bool CompareGenericList(const QList<T *> &left, const QList<T *> &right)
{
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

} /* namespace vpvm */

#endif // UTIL_H
