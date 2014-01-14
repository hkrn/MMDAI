/**

 Copyright (c) 2010-2014  hkrn

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

#include "BoneKeyframeRefObject.h"
#include "BoneMotionTrack.h"
#include "BoneRefObject.h"
#include "CameraKeyframeRefObject.h"
#include "CameraMotionTrack.h"
#include "CameraRefObject.h"
#include "LightKeyframeRefObject.h"
#include "LightMotionTrack.h"
#include "LightRefObject.h"
#include "ModelProxy.h"
#include "MorphKeyframeRefObject.h"
#include "MorphMotionTrack.h"
#include "MorphRefObject.h"
#include "MotionProxy.h"
#include "ProjectProxy.h"
#include "Util.h"

#include <cmath>
#include <QApplication>
#include <QtCore>
#include <QUndoStack>

#include <vpvl2/vpvl2.h>
#include <vpvl2/extensions/qt/String.h>

using namespace vpvl2;
using namespace vpvl2::extensions::qt;

namespace {

class BaseKeyframeCommand : public QUndoCommand {
public:
    BaseKeyframeCommand(const QList<BaseKeyframeRefObject *> &keyframes, QUndoCommand *parent)
        : QUndoCommand(parent),
          m_keyframes(keyframes)
    {
    }
    BaseKeyframeCommand(BaseKeyframeRefObject *keyframe, QUndoCommand *parent)
        : QUndoCommand(parent)
    {
        m_keyframes.append(keyframe);
    }
    ~BaseKeyframeCommand() {
        foreach (BaseKeyframeRefObject *keyframe, m_keyframes) {
            if (!keyframe->parentTrack()->contains(keyframe)) {
                /* move reference to BaseMotionTrack class */
                delete keyframe;
            }
        }
    }

protected:
    void addKeyframe() {
        foreach (BaseKeyframeRefObject *keyframe, m_keyframes) {
            BaseMotionTrack *track = keyframe->parentTrack();
            track->addKeyframe(keyframe, true);
        }
    }
    void removeKeyframe() {
        foreach (BaseKeyframeRefObject *keyframe, m_keyframes) {
            BaseMotionTrack *track = keyframe->parentTrack();
            track->removeKeyframe(keyframe, true);
        }
    }

    QList<BaseKeyframeRefObject *> m_keyframes;
};

class AddKeyframeCommand : public BaseKeyframeCommand {
public:
    AddKeyframeCommand(const QList<BaseKeyframeRefObject *> &keyframes, QUndoCommand *parent)
        : BaseKeyframeCommand(keyframes, parent)
    {
        setText(QApplication::tr("Register Keyframe(s)"));
    }
    AddKeyframeCommand(BaseKeyframeRefObject *keyframeRef, QUndoCommand *parent)
        : BaseKeyframeCommand(keyframeRef, parent)
    {
        setText(QApplication::tr("Register Keyframe(s)"));
    }
    ~AddKeyframeCommand() {
    }

    void undo() {
        removeKeyframe();
    }
    void redo() {
        addKeyframe();
    }
};

class RemoveKeyframeCommand : public BaseKeyframeCommand {
public:
    RemoveKeyframeCommand(const QList<BaseKeyframeRefObject *> &keyframes, QUndoCommand *parent)
        : BaseKeyframeCommand(keyframes, parent)
    {
        setText(QApplication::tr("Remove Keyframe(s)"));
    }
    RemoveKeyframeCommand(BaseKeyframeRefObject *keyframeRef, QUndoCommand *parent)
        : BaseKeyframeCommand(keyframeRef, parent)
    {
        setText(QApplication::tr("Remove Keyframe(s)"));
    }
    ~RemoveKeyframeCommand() {
    }

    void undo() {
        addKeyframe();
    }
    void redo() {
        removeKeyframe();
    }
};

class MergeKeyframeCommand : public QUndoCommand {
public:
    typedef QPair<BaseKeyframeRefObject *, BaseKeyframeRefObject *> Pair;

    MergeKeyframeCommand(const QList<Pair> &keyframes,
                         const qint64 &newTimeIndex,
                         const qint64 &oldTimeIndex,
                         QUndoCommand *parent)
        : QUndoCommand(parent),
          m_keyframes(keyframes),
          m_newTimeIndex(newTimeIndex),
          m_oldTimeIndex(oldTimeIndex)
    {
        setText(QApplication::tr("Merge Keyframe(s)"));
    }
    ~MergeKeyframeCommand() {
    }

    void undo() {
        foreach (const Pair &pair, m_keyframes) {
            Q_ASSERT(pair.first);
            BaseKeyframeRefObject *destinationKeyframe = pair.first;
            destinationKeyframe->setTimeIndex(m_oldTimeIndex);
            destinationKeyframe->parentTrack()->replaceTimeIndex(m_oldTimeIndex, m_newTimeIndex);
            if (BaseKeyframeRefObject *sourceKeyframe = pair.second) {
                sourceKeyframe->parentTrack()->addKeyframe(sourceKeyframe, true);
            }
        }
    }
    void redo() {
        foreach (const Pair &pair, m_keyframes) {
            Q_ASSERT(pair.first);
            if (BaseKeyframeRefObject *sourceKeyframe = pair.second) {
                sourceKeyframe->parentTrack()->removeKeyframe(sourceKeyframe, true);
            }
            BaseKeyframeRefObject *destionationKeyframe = pair.first;
            destionationKeyframe->setTimeIndex(m_newTimeIndex);
            destionationKeyframe->parentTrack()->replaceTimeIndex(m_newTimeIndex, m_oldTimeIndex);
        }
    }

private:
    const QList<Pair> m_keyframes;
    const qint64 m_newTimeIndex;
    const qint64 m_oldTimeIndex;
};

class UpdateBoneKeyframeCommand : public QUndoCommand {
public:
    UpdateBoneKeyframeCommand(const QList<BoneKeyframeRefObject *> &keyframeRefs, MotionProxy *motionProxy, QUndoCommand *parent)
        : QUndoCommand(parent),
          m_motionProxyRef(motionProxy)
    {
        initialize(keyframeRefs, motionProxy);
    }
    UpdateBoneKeyframeCommand(BoneKeyframeRefObject *keyframeRef, MotionProxy *motionProxy, QUndoCommand *parent)
        : QUndoCommand(parent),
          m_motionProxyRef(motionProxy)
    {
        QList<BoneKeyframeRefObject *> keyframeRefs;
        keyframeRefs.append(keyframeRef);
        initialize(keyframeRefs, motionProxy);
    }
    ~UpdateBoneKeyframeCommand() {
        foreach (BoneKeyframeRefObject *newKeyframe, m_newKeyframes) {
            if (!newKeyframe->parentTrack()->contains(newKeyframe)) {
                delete newKeyframe;
            }
        }
        m_motionProxyRef = 0;
    }

    void undo() {
        foreach (BoneKeyframeRefObject *keyframeRef, m_oldKeyframeRefs) {
            BoneMotionTrack *track = qobject_cast<BoneMotionTrack *>(keyframeRef->parentTrack());
            Q_ASSERT(track);
            track->replace(keyframeRef, m_old2newKeyframeRefs.value(keyframeRef), false);
            setLocalTransform(keyframeRef);
        }
        m_motionProxyRef->refreshBoneTracks();
    }
    void redo() {
        foreach (BoneKeyframeRefObject *newKeyframe, m_newKeyframes) {
            BoneMotionTrack *track = qobject_cast<BoneMotionTrack *>(newKeyframe->parentTrack());
            Q_ASSERT(track);
            track->replace(newKeyframe, m_new2oldKeyframeRefs.value(newKeyframe), false);
            setLocalTransform(newKeyframe);
        }
        m_motionProxyRef->refreshBoneTracks();
    }

private:
    void initialize(const QList<BoneKeyframeRefObject *> &keyframeRefs, const MotionProxy *motionProxyRef) {
        Q_ASSERT(motionProxyRef);
        const ModelProxy *modelProxyRef = motionProxyRef->parentModel();
        foreach (BoneKeyframeRefObject *keyframeRef, keyframeRefs) {
            BoneMotionTrack *track = qobject_cast<BoneMotionTrack *>(keyframeRef->parentTrack());
            QScopedPointer<BoneKeyframeRefObject> newKeyframe(new BoneKeyframeRefObject(track, keyframeRef->data()->clone()));
            IBoneKeyframe *newKeyframeRef = newKeyframe->data();
            BoneRefObject *boneRef = modelProxyRef->findBoneByName(track->name());
            newKeyframeRef->setLocalTranslation(boneRef->rawLocalTranslation());
            newKeyframeRef->setLocalOrientation(boneRef->rawLocalOrientation());
            m_old2newKeyframeRefs.insert(keyframeRef, newKeyframe.data());
            m_new2oldKeyframeRefs.insert(newKeyframe.data(), keyframeRef);
            m_newKeyframes.append(newKeyframe.take());
            m_oldKeyframeRefs.append(keyframeRef);
        }
        setText(QApplication::tr("Update Bone Keyframe(s)"));
    }
    void setLocalTransform(const BoneKeyframeRefObject *boneKeyframeRef) {
        if (BoneRefObject *boneRef = m_motionProxyRef->parentModel()->findBoneByName(boneKeyframeRef->name())) {
            IBoneKeyframe *keyframeRef = boneKeyframeRef->data();
            boneRef->setRawLocalTranslation(keyframeRef->localTranslation());
            boneRef->setRawLocalOrientation(keyframeRef->localOrientation());
        }
    }

    QHash<BoneKeyframeRefObject *, BoneKeyframeRefObject *> m_old2newKeyframeRefs;
    QHash<BoneKeyframeRefObject *, BoneKeyframeRefObject *> m_new2oldKeyframeRefs;
    QList<BoneKeyframeRefObject *> m_newKeyframes;
    QList<BoneKeyframeRefObject *> m_oldKeyframeRefs;
    MotionProxy *m_motionProxyRef;
};

class UpdateCameraKeyframeCommand : public QUndoCommand {
public:
    UpdateCameraKeyframeCommand(CameraKeyframeRefObject *keyframeRef, CameraRefObject *cameraRef, QUndoCommand *parent)
        : QUndoCommand(parent),
          m_newKeyframe(new CameraKeyframeRefObject(qobject_cast<CameraMotionTrack *>(keyframeRef->parentTrack()), keyframeRef->data()->clone())),
          m_keyframeRef(keyframeRef),
          m_cameraRef(cameraRef)
    {
        Q_ASSERT(m_keyframeRef);
        Q_ASSERT(m_cameraRef);
        ICameraKeyframe *newKeyframeRef = m_newKeyframe->data();
        newKeyframeRef->setAngle(Util::toVector3(m_cameraRef->angle()));
        newKeyframeRef->setDistance(m_cameraRef->distance());
        newKeyframeRef->setFov(m_cameraRef->fov());
        newKeyframeRef->setLookAt(Util::toVector3(m_cameraRef->lookAt()));
        setText(QApplication::tr("Update Camera Keyframe"));
    }
    ~UpdateCameraKeyframeCommand() {
        if(m_keyframeRef->parentTrack()->contains(m_newKeyframe.data())) {
            m_newKeyframe.take();
        }
        m_keyframeRef = 0;
        m_cameraRef = 0;
    }

    void undo() {
        CameraMotionTrack *track = qobject_cast<CameraMotionTrack *>(m_keyframeRef->parentTrack());
        Q_ASSERT(track);
        track->replace(m_keyframeRef, m_newKeyframe.data(), true);
        setCameraParameters(m_keyframeRef);
    }
    void redo() {
        CameraMotionTrack *track = qobject_cast<CameraMotionTrack *>(m_keyframeRef->parentTrack());
        Q_ASSERT(track);
        track->replace(m_newKeyframe.data(), m_keyframeRef, true);
        setCameraParameters(m_newKeyframe.data());
    }

private:
    void setCameraParameters(const CameraKeyframeRefObject *lightKeyframe) {
        if (m_cameraRef) {
            ICameraKeyframe *keyframeRef = lightKeyframe->data();
            m_cameraRef->setAngle(Util::fromVector3(keyframeRef->angle()));
            m_cameraRef->setDistance(keyframeRef->distance());
            m_cameraRef->setFov(keyframeRef->fov());
            m_cameraRef->setLookAt(Util::fromVector3(keyframeRef->lookAt()));
        }
    }

    QScopedPointer<CameraKeyframeRefObject> m_newKeyframe;
    CameraKeyframeRefObject *m_keyframeRef;
    CameraRefObject *m_cameraRef;
};

class UpdateLightKeyframeCommand : public QUndoCommand {
public:
    UpdateLightKeyframeCommand(LightKeyframeRefObject *keyframeRef, LightRefObject *lightRef, QUndoCommand *parent)
        : QUndoCommand(parent),
          m_newKeyframe(new LightKeyframeRefObject(qobject_cast<LightMotionTrack *>(keyframeRef->parentTrack()), keyframeRef->data()->clone())),
          m_keyframeRef(keyframeRef),
          m_lightRef(lightRef)
    {
        Q_ASSERT(m_keyframeRef);
        Q_ASSERT(m_lightRef);
        ILightKeyframe *newKeyframeRef = m_newKeyframe->data();
        newKeyframeRef->setColor(Util::toColor(m_lightRef->color()));
        newKeyframeRef->setDirection(Util::toVector3(m_lightRef->direction()));
        setText(QApplication::tr("Update Light Keyframe"));
    }
    ~UpdateLightKeyframeCommand() {
        if(m_keyframeRef->parentTrack()->contains(m_newKeyframe.data())) {
            m_newKeyframe.take();
        }
        m_keyframeRef = 0;
        m_lightRef = 0;
    }

    void undo() {
        LightMotionTrack *track = qobject_cast<LightMotionTrack *>(m_keyframeRef->parentTrack());
        Q_ASSERT(track);
        track->replace(m_keyframeRef, m_newKeyframe.data(), true);
        setLightParameters(m_keyframeRef);
    }
    void redo() {
        LightMotionTrack *track = qobject_cast<LightMotionTrack *>(m_keyframeRef->parentTrack());
        Q_ASSERT(track);
        track->replace(m_newKeyframe.data(), m_keyframeRef, true);
        setLightParameters(m_newKeyframe.data());
    }

private:
    void setLightParameters(const LightKeyframeRefObject *lightKeyframe) {
        if (m_lightRef) {
            ILightKeyframe *keyframeRef = lightKeyframe->data();
            m_lightRef->setColor(Util::fromColor(keyframeRef->color()));
            m_lightRef->setDirection(Util::fromVector3(keyframeRef->direction()));
        }
    }

    QScopedPointer<LightKeyframeRefObject> m_newKeyframe;
    LightKeyframeRefObject *m_keyframeRef;
    LightRefObject *m_lightRef;
};

class UpdateMorphKeyframeCommand : public QUndoCommand {
public:
    UpdateMorphKeyframeCommand(const QList<MorphKeyframeRefObject *> &keyframeRefs, MotionProxy *motionProxy, QUndoCommand *parent)
        : QUndoCommand(parent),
          m_motionProxyRef(motionProxy)
    {
        initialize(keyframeRefs, motionProxy);
    }
    UpdateMorphKeyframeCommand(MorphKeyframeRefObject *keyframeRef, MotionProxy *motionProxy, QUndoCommand *parent)
        : QUndoCommand(parent),
          m_motionProxyRef(motionProxy)
    {
        QList<MorphKeyframeRefObject *> keyframeRefs;
        keyframeRefs.append(keyframeRef);
        initialize(keyframeRefs, motionProxy);
    }
    ~UpdateMorphKeyframeCommand() {
        foreach (MorphKeyframeRefObject *newKeyframe, m_newKeyframes) {
            if (!newKeyframe->parentTrack()->contains(newKeyframe)) {
                delete newKeyframe;
            }
        }
        m_motionProxyRef = 0;
    }

    void undo() {
        foreach (MorphKeyframeRefObject *keyframeRef, m_oldKeyframeRefs) {
            MorphMotionTrack *track = qobject_cast<MorphMotionTrack *>(keyframeRef->parentTrack());
            Q_ASSERT(track);
            track->replace(keyframeRef, m_old2newKeyframeRefs.value(keyframeRef), false);
            setWeight(keyframeRef);
        }
        m_motionProxyRef->refreshMorphTracks();
    }
    void redo() {
        foreach (MorphKeyframeRefObject *newKeyframe, m_newKeyframes) {
            MorphMotionTrack *track = qobject_cast<MorphMotionTrack *>(newKeyframe->parentTrack());
            Q_ASSERT(track);
            track->replace(newKeyframe, m_new2oldKeyframeRefs.value(newKeyframe), false);
            setWeight(newKeyframe);
        }
        m_motionProxyRef->refreshMorphTracks();
    }

private:
    void initialize(const QList<MorphKeyframeRefObject *> &keyframeRefs, const MotionProxy *motionProxyRef) {
        Q_ASSERT(motionProxyRef);
        const ModelProxy *modelProxyRef = motionProxyRef->parentModel();
        foreach (MorphKeyframeRefObject *keyframeRef, keyframeRefs) {
            MorphMotionTrack *track = qobject_cast<MorphMotionTrack *>(keyframeRef->parentTrack());
            QScopedPointer<MorphKeyframeRefObject> newKeyframe(new MorphKeyframeRefObject(track, keyframeRef->data()->clone()));
            IMorphKeyframe *newKeyframeRef = newKeyframe->data();
            MorphRefObject *morphRef = modelProxyRef->findMorphByName(track->name());
            newKeyframeRef->setWeight(morphRef->weight());
            m_old2newKeyframeRefs.insert(keyframeRef, newKeyframe.data());
            m_new2oldKeyframeRefs.insert(newKeyframe.data(), keyframeRef);
            m_newKeyframes.append(newKeyframe.take());
            m_oldKeyframeRefs.append(keyframeRef);
        }
        setText(QApplication::tr("Update Morph Keyframe(s)"));
    }
    void setWeight(const MorphKeyframeRefObject *morphKeyframeRef) {
        if (MorphRefObject *morphRef = m_motionProxyRef->parentModel()->findMorphByName(morphKeyframeRef->name())) {
            IMorphKeyframe *keyframeRef = morphKeyframeRef->data();
            morphRef->setWeight(keyframeRef->weight());
        }
    }

    QHash<MorphKeyframeRefObject *, MorphKeyframeRefObject *> m_old2newKeyframeRefs;
    QHash<MorphKeyframeRefObject *, MorphKeyframeRefObject *> m_new2oldKeyframeRefs;
    QList<MorphKeyframeRefObject *> m_newKeyframes;
    QList<MorphKeyframeRefObject *> m_oldKeyframeRefs;
    MotionProxy *m_motionProxyRef;
};

class UpdateBoneKeyframeInterpolationCommand : public QUndoCommand {
public:
    UpdateBoneKeyframeInterpolationCommand(BoneKeyframeRefObject *keyframeRef,
                                           MotionProxy *motionProxyRef,
                                           const QVector4D &value,
                                           int type,
                                           QUndoCommand *parent)
        : QUndoCommand(parent),
          m_keyframeRef(keyframeRef),
          m_motionProxyRef(motionProxyRef),
          m_newValue(value.x(), value.y(), value.z(), value.w()),
          m_type(static_cast<IBoneKeyframe::InterpolationType>(type))
    {
        Q_ASSERT(m_keyframeRef && m_motionProxyRef);
        m_keyframeRef->data()->getInterpolationParameter(m_type, m_oldValue);
        setText(QApplication::tr("Update Bone Keyframe Interpolation Parameters"));
    }
    ~UpdateBoneKeyframeInterpolationCommand() {
        m_keyframeRef = 0;
        m_motionProxyRef = 0;
    }

    void undo() {
        m_keyframeRef->data()->setInterpolationParameter(m_type, m_oldValue);
    }
    void redo() {
        m_keyframeRef->data()->setInterpolationParameter(m_type, m_newValue);
    }

private:
    BoneKeyframeRefObject *m_keyframeRef;
    MotionProxy *m_motionProxyRef;
    const QuadWord m_newValue;
    const IBoneKeyframe::InterpolationType m_type;
    QuadWord m_oldValue;
};

class UpdateCameraKeyframeInterpolationCommand : public QUndoCommand {
public:
    UpdateCameraKeyframeInterpolationCommand(CameraKeyframeRefObject *keyframeRef,
                                             MotionProxy *motionProxyRef,
                                             const QVector4D &value,
                                             int type,
                                             QUndoCommand *parent)
        : QUndoCommand(parent),
          m_keyframeRef(keyframeRef),
          m_motionProxyRef(motionProxyRef),
          m_newValue(value.x(), value.y(), value.z(), value.w()),
          m_type(static_cast<ICameraKeyframe::InterpolationType>(type))
    {
        Q_ASSERT(m_keyframeRef && m_motionProxyRef);
        m_keyframeRef->data()->getInterpolationParameter(m_type, m_oldValue);
        setText(QApplication::tr("Update Camera Keyframe Interpolation Parameters"));
    }
    ~UpdateCameraKeyframeInterpolationCommand() {
        m_keyframeRef = 0;
        m_motionProxyRef = 0;
    }

    void undo() {
        m_keyframeRef->data()->setInterpolationParameter(m_type, m_oldValue);
    }
    void redo() {
        m_keyframeRef->data()->setInterpolationParameter(m_type, m_newValue);
    }

private:
    CameraKeyframeRefObject *m_keyframeRef;
    MotionProxy *m_motionProxyRef;
    const QuadWord m_newValue;
    const ICameraKeyframe::InterpolationType m_type;
    QuadWord m_oldValue;
};

class PasteKeyframesCommand : public QUndoCommand {
public:
    typedef QPair<QObject *, quint64> ProceedSetPair;
    typedef QSet<ProceedSetPair> ProceedSet;

    PasteKeyframesCommand(MotionProxy *motionProxy,
                          const QList<BaseKeyframeRefObject *> &copiedKeyframes,
                          const IKeyframe::TimeIndex &offsetTimeIndex,
                          QList<BaseKeyframeRefObject *> *selectedKeyframes,
                          QUndoCommand *parent)
        : QUndoCommand(parent),
          m_copiedKeyframeRefs(copiedKeyframes),
          m_offsetTimeIndex(offsetTimeIndex),
          m_motionProxy(motionProxy),
          m_selectedKeyframeRefs(selectedKeyframes)
    {
        setText(QApplication::tr("Paster Keyframe(s)"));
    }
    ~PasteKeyframesCommand() {
    }

    void undo() {
        foreach (BaseKeyframeRefObject *keyframe, m_createdKeyframes) {
            BaseMotionTrack *track = keyframe->parentTrack();
            track->removeKeyframe(keyframe, false);
        }
        m_motionProxy->refresh();
        qDeleteAll(m_createdKeyframes);
        m_createdKeyframes.clear();
        m_selectedKeyframeRefs->clear();
        m_selectedKeyframeRefs->append(m_copiedKeyframeRefs);
    }
    void redo() {
        ProceedSet proceeded;
        foreach (BaseKeyframeRefObject *keyframe, m_copiedKeyframeRefs) {
            const qreal &timeIndex = keyframe->timeIndex() + m_offsetTimeIndex;
            BaseMotionTrack *track = keyframe->parentTrack();
            if (BaseKeyframeRefObject *clonedKeyframe = track->copy(keyframe, timeIndex, false)) {
                handleKeyframe(clonedKeyframe, proceeded);
                m_createdKeyframes.append(clonedKeyframe);
            }
        }
        m_motionProxy->refresh();
    }
    virtual void handleKeyframe(BaseKeyframeRefObject *value, ProceedSet &proceeded) {
        Q_UNUSED(value);
        Q_UNUSED(proceeded);
    }

protected:
    const QList<BaseKeyframeRefObject *> m_copiedKeyframeRefs;
    const QList<BaseKeyframeRefObject *> m_previousSelectedKeyframeRefs;
    const IKeyframe::TimeIndex m_offsetTimeIndex;
    MotionProxy *m_motionProxy;
    QList<BaseKeyframeRefObject *> m_createdKeyframes;
    QList<BaseKeyframeRefObject *> *m_selectedKeyframeRefs;
};

class InversedPasteKeyframesCommand : public PasteKeyframesCommand {
public:
    InversedPasteKeyframesCommand(MotionProxy *motionProxy,
                                  const QList<BaseKeyframeRefObject *> &copiedKeyframes,
                                  const IKeyframe::TimeIndex &offsetTimeIndex,
                                  QList<BaseKeyframeRefObject *> *selectedKeyframes,
                                  QUndoCommand *parent)
        : PasteKeyframesCommand(motionProxy, copiedKeyframes, offsetTimeIndex, selectedKeyframes, parent)
    {
        setText(QApplication::tr("Paster Keyframe(s) with Inversed"));
    }

    void handleKeyframe(BaseKeyframeRefObject *value, ProceedSet &proceeded) {
        ModelProxy *modelProxy = m_motionProxy->parentModel();
        if (BoneKeyframeRefObject *keyframe = qobject_cast<BoneKeyframeRefObject *>(value)) {
            const ProjectProxy *projectProxy = m_motionProxy->parentProject();
            const IEncoding *encodingRef = projectProxy->encodingInstanceRef();
            const QString &leftStringLiteral = Util::toQString(encodingRef->stringConstant(IEncoding::kLeft));
            const QString &rightStringLiteral = Util::toQString(encodingRef->stringConstant(IEncoding::kRight));
            const QString &name = keyframe->name();
            const quint64 &timeIndex = keyframe->timeIndex();
            bool isRight = name.startsWith(rightStringLiteral), isLeft = name.startsWith(leftStringLiteral);
            BoneRefObject *boneRef = modelProxy->findBoneByName(name);
            if (!proceeded.contains(ProceedSetPair(boneRef, timeIndex)) && (isRight || isLeft)) {
                QString replaced = name;
                if (isRight) {
                    replaced.replace(rightStringLiteral, leftStringLiteral);
                    proceeded.insert(ProceedSetPair(modelProxy->findBoneByName(replaced), timeIndex));
                }
                else if (isLeft) {
                    replaced.replace(leftStringLiteral, rightStringLiteral);
                    proceeded.insert(ProceedSetPair(modelProxy->findBoneByName(replaced), timeIndex));
                }
                const QVector3D &v = keyframe->localTranslation();
                keyframe->setLocalTranslation(QVector3D(-v.x(), v.y(), v.z()));
                const QQuaternion &q = keyframe->localOrientation();
                keyframe->setLocalOrientation(QQuaternion(q.x(), -q.y(), -q.z(), q.scalar()));
            }
        }
    }
};

}

MotionProxy::MotionProxy(ProjectProxy *projectRef,
                         IMotion *motion,
                         const QUuid &uuid,
                         const QUrl &fileUrl,
                         QUndoStack *undoStackRef)
    : QObject(projectRef),
      m_projectRef(projectRef),
      m_motion(motion),
      m_undoStackRef(undoStackRef),
      m_uuid(uuid),
      m_fileUrl(fileUrl)
{
    Q_ASSERT(m_projectRef);
    Q_ASSERT(m_undoStackRef);
    Q_ASSERT(!m_motion.isNull());
    Q_ASSERT(!m_uuid.isNull());
    connect(this, &MotionProxy::durationTimeIndexChanged, projectRef, &ProjectProxy::durationTimeIndexChanged);
    VPVL2_VLOG(1, "The motion " << uuid.toString().toStdString() << " is added");
}

MotionProxy::~MotionProxy()
{
    VPVL2_VLOG(1, "The motion " << uuid().toString().toStdString() << " will be deleted");
    qDeleteAll(m_boneMotionTrackBundle);
    m_boneMotionTrackBundle.clear();
    qDeleteAll(m_morphMotionTrackBundle);
    m_morphMotionTrackBundle.clear();
    m_cameraMotionTrackRef = 0;
    m_lightMotionTrackRef = 0;
    m_projectRef = 0;
}

void MotionProxy::setModelProxy(ModelProxy *modelProxy, const Factory *factoryRef)
{
    Q_ASSERT(modelProxy);
    vpvl2::IMotion *motionRef = m_motion.data();
    int numLoadedKeyframes = 0;
    int numBoneKeyframes = motionRef->countKeyframes(IKeyframe::kBoneKeyframe);
    int numMorphKeyframes = motionRef->countKeyframes(IKeyframe::kMorphKeyframe);
    int numEstimatedTotalKeyframes = numBoneKeyframes + numMorphKeyframes;
    emit motionWillLoad(numEstimatedTotalKeyframes);
    loadBoneTrackBundle(motionRef, numBoneKeyframes, numEstimatedTotalKeyframes, numLoadedKeyframes);
    loadMorphTrackBundle(motionRef, numMorphKeyframes, numEstimatedTotalKeyframes, numLoadedKeyframes);
    emit motionDidLoad(numLoadedKeyframes, numEstimatedTotalKeyframes);
    foreach (const BoneRefObject *bone, modelProxy->allBoneRefs()) {
        if (!findBoneMotionTrack(bone)) {
            QScopedPointer<IBoneKeyframe> keyframe(factoryRef->createBoneKeyframe(motionRef));
            keyframe->setDefaultInterpolationParameter();
            keyframe->setTimeIndex(0);
            keyframe->setLocalOrientation(bone->rawLocalOrientation());
            keyframe->setLocalTranslation(bone->rawLocalTranslation());
            keyframe->setName(bone->data()->name(IEncoding::kDefaultLanguage));
            BoneMotionTrack *track = addBoneTrack(bone->name());
            track->addKeyframe(track->convertBoneKeyframe(keyframe.take()), false);
        }
    }
    foreach (const MorphRefObject *morph, modelProxy->allMorphRefs()) {
        if (!findMorphMotionTrack(morph)) {
            QScopedPointer<IMorphKeyframe> keyframe(factoryRef->createMorphKeyframe(motionRef));
            keyframe->setTimeIndex(0);
            keyframe->setWeight(morph->weight());
            keyframe->setName(morph->data()->name(IEncoding::kDefaultLanguage));
            MorphMotionTrack *track = addMorphTrack(morph->name());
            track->addKeyframe(track->convertMorphKeyframe(keyframe.take()), false);
        }
    }
    refresh();
}

void MotionProxy::setCameraMotionTrack(CameraMotionTrack *track, const Factory *factoryRef)
{
    Q_ASSERT(track && factoryRef);
    IMotion *motionRef = data();
    const int nkeyframes = motionRef->countKeyframes(track->type());
    for (int i = 0; i < nkeyframes; i++) {
        ICameraKeyframe *keyframe = motionRef->findCameraKeyframeRefAt(i);
        track->add(track->convertCameraKeyframe(keyframe), false);
    }
    if (!track->findKeyframeByTimeIndex(0)) {
        QScopedPointer<ICamera> cameraRef(m_projectRef->projectInstanceRef()->createCamera());
        QScopedPointer<ICameraKeyframe> keyframe(factoryRef->createCameraKeyframe(data()));
        keyframe->setDefaultInterpolationParameter();
        keyframe->setAngle(cameraRef->angle());
        keyframe->setDistance(cameraRef->distance());
        keyframe->setFov(cameraRef->fov());
        keyframe->setLookAt(cameraRef->lookAt());
        track->addKeyframe(track->convertCameraKeyframe(keyframe.take()), false);
    }
    track->refresh();
    bindTrackSignals(track);
    m_cameraMotionTrackRef = track;
}

void MotionProxy::setLightMotionTrack(LightMotionTrack *track, const Factory *factoryRef)
{
    Q_ASSERT(track && factoryRef);
    IMotion *motionRef = data();
    const int nkeyframes = motionRef->countKeyframes(track->type());
    for (int i = 0; i < nkeyframes; i++) {
        ILightKeyframe *keyframe = motionRef->findLightKeyframeRefAt(i);
        track->add(track->convertLightKeyframe(keyframe), false);
    }
    if (!track->findKeyframeByTimeIndex(0)) {
        QScopedPointer<ILight> lightRef(m_projectRef->projectInstanceRef()->createLight());
        QScopedPointer<ILightKeyframe> keyframe(factoryRef->createLightKeyframe(data()));
        keyframe->setColor(lightRef->color());
        keyframe->setDirection(lightRef->direction());
        track->addKeyframe(track->convertLightKeyframe(keyframe.take()), true);
    }
    track->refresh();
    bindTrackSignals(track);
    m_lightMotionTrackRef = track;
}

void MotionProxy::refreshBoneTracks()
{
    QHashIterator<QString, BoneMotionTrack *> it(m_boneMotionTrackBundle);
    while (it.hasNext()) {
        it.next();
        it.value()->sort();
    }
    m_motion->update(IKeyframe::kBoneKeyframe);
}

void MotionProxy::refreshMorphTracks()
{
    QHashIterator<QString, MorphMotionTrack *> it(m_morphMotionTrackBundle);
    while (it.hasNext()) {
        it.next();
        it.value()->sort();
    }
    m_motion->update(IKeyframe::kMorphKeyframe);
}

bool MotionProxy::save(const QUrl &fileUrl)
{
    if (fileUrl.isEmpty() || !fileUrl.isValid()) {
        /* do nothing if url is empty or invalid */
        VPVL2_VLOG(2, "fileUrl is empty or invalid: url=" << fileUrl.toString().toStdString());
        return false;
    }
    bool result = false;
    QByteArray bytes;
    bytes.resize(m_motion->estimateSize());
    m_motion->save(reinterpret_cast<uint8_t *>(bytes.data()));
    QSaveFile saveFile(fileUrl.toLocalFile());
    if (saveFile.open(QFile::WriteOnly | QFile::Unbuffered)) {
        saveFile.write(bytes);
        result = saveFile.commit();
    }
    return result;
}

qreal MotionProxy::differenceTimeIndex(qreal value) const
{
    return m_motion->duration() - value;
}

qreal MotionProxy::differenceDuration(qreal value) const
{
    return differenceTimeIndex(value) * Scene::defaultFPS();
}

BoneMotionTrack *MotionProxy::findBoneMotionTrack(const BoneRefObject *value) const
{
    return value ? findBoneMotionTrack(value->name()) : 0;
}

BoneMotionTrack *MotionProxy::findBoneMotionTrack(const QString &name) const
{
    return m_boneMotionTrackBundle.value(name);
}

MorphMotionTrack *MotionProxy::findMorphMotionTrack(const MorphRefObject *value) const
{
    return value ? findMorphMotionTrack(value->name()) : 0;
}

MorphMotionTrack *MotionProxy::findMorphMotionTrack(const QString &name) const
{
    return m_morphMotionTrackBundle.value(name);
}

BaseKeyframeRefObject *MotionProxy::resolveKeyframeAt(const qint64 &timeIndex, QObject *opaque) const
{
    if (const BoneRefObject *boneRef = qobject_cast<const BoneRefObject *>(opaque)) {
        const BoneMotionTrack *track = findBoneMotionTrack(boneRef);
        return track ? track->findKeyframeByTimeIndex(timeIndex) : 0;
    }
    else if (const CameraRefObject *cameraRef = qobject_cast<const CameraRefObject *>(opaque)) {
        const CameraMotionTrack *track = cameraRef->track();
        return track->findKeyframeByTimeIndex(timeIndex);
    }
    else if (const LightRefObject *lightRef = qobject_cast<const LightRefObject *>(opaque)) {
        const LightMotionTrack *track = lightRef->track();
        return track->findKeyframeByTimeIndex(timeIndex);
    }
    else if (const MorphRefObject *morphRef = qobject_cast<const MorphRefObject *>(opaque)) {
        const MorphMotionTrack *track = findMorphMotionTrack(morphRef);
        return track ? track->findKeyframeByTimeIndex(timeIndex) : 0;
    }
    return 0;
}

void MotionProxy::applyParentModel()
{
    if (ModelProxy *modelProxy = m_projectRef->resolveModelProxy(m_motion->parentModelRef())) {
        m_motion->setParentModelRef(modelProxy->data());
    }
}

void MotionProxy::addKeyframe(QObject *opaque, const qint64 &timeIndex, QUndoCommand *parent)
{
    BaseKeyframeRefObject *keyframe = 0;
    if (const BoneRefObject *bone = qobject_cast<const BoneRefObject *>(opaque)) {
        keyframe = addBoneKeyframe(bone);
        if (keyframe) {
            int length = findBoneMotionTrack(bone->name())->length();
            VPVL2_VLOG(2, "insert type=BONE timeIndex=" << timeIndex << " name=" << bone->name().toStdString() << " length=" << length);
        }
    }
    else if (const CameraRefObject *camera = qobject_cast<const CameraRefObject *>(opaque)) {
        keyframe = addCameraKeyframe(camera);
        VPVL2_VLOG(2, "insert type=CAMERA timeIndex=" << timeIndex << " length=" << camera->track()->length());
    }
    else if (const LightRefObject *light = qobject_cast<const LightRefObject *>(opaque)) {
        keyframe = addLightKeyframe(light);
        VPVL2_VLOG(2, "insert type=LIGHT timeIndex=" << timeIndex << " length=" << light->track()->length());
    }
    else if (const MorphRefObject *morph = qobject_cast<const MorphRefObject *>(opaque)) {
        keyframe = addMorphKeyframe(morph);
        if (keyframe) {
            int length = findMorphMotionTrack(morph->name())->length();
            VPVL2_VLOG(2, "insert type=MORPH timeIndex=" << timeIndex << " name=" << morph->name().toStdString() << " length=" << length);
        }
    }
    if (keyframe) {
        keyframe->setTimeIndex(timeIndex);
        m_undoStackRef->push(new AddKeyframeCommand(keyframe, parent));
        m_projectRef->setDirty(true);
    }
}

void MotionProxy::updateKeyframe(QObject *opaque, const qint64 &timeIndex, QUndoCommand *parent)
{
    if (BoneKeyframeRefObject *boneKeyframeRef = qobject_cast<BoneKeyframeRefObject *>(opaque)) {
        m_undoStackRef->push(new UpdateBoneKeyframeCommand(boneKeyframeRef, this, parent));
        m_projectRef->setDirty(true);
        VPVL2_VLOG(2, "update type=BONE timeIndex=" << timeIndex << " name=" << boneKeyframeRef->name().toStdString());
    }
    else if (CameraKeyframeRefObject *cameraKeyframeRef = qobject_cast<CameraKeyframeRefObject *>(opaque)) {
        m_undoStackRef->push(new UpdateCameraKeyframeCommand(cameraKeyframeRef, m_projectRef->camera(), parent));
        m_projectRef->setDirty(true);
        VPVL2_VLOG(2, "update type=CAMERA timeIndex=" << timeIndex);
    }
    else if (LightKeyframeRefObject *lightKeyframeRef = qobject_cast<LightKeyframeRefObject *>(opaque)) {
        m_undoStackRef->push(new UpdateLightKeyframeCommand(lightKeyframeRef, m_projectRef->light(), parent));
        m_projectRef->setDirty(true);
        VPVL2_VLOG(2, "update type=LIGHT timeIndex=" << timeIndex);
    }
    else if (MorphKeyframeRefObject *morphKeyframeRef = qobject_cast<MorphKeyframeRefObject *>(opaque)) {
        m_undoStackRef->push(new UpdateMorphKeyframeCommand(morphKeyframeRef, this, parent));
        m_projectRef->setDirty(true);
        VPVL2_VLOG(2, "update type=MORPH timeIndex=" << timeIndex << " name=" << morphKeyframeRef->name().toStdString());
    }
    else if (const BoneRefObject *boneRef = qobject_cast<BoneRefObject *>(opaque)) {
        updateOrAddKeyframeFromBone(boneRef, timeIndex, parent);
    }
    else if (CameraRefObject *cameraRef = qobject_cast<CameraRefObject *>(opaque)) {
        updateOrAddKeyframeFromCamera(cameraRef, timeIndex, parent);
    }
    else if (LightRefObject *lightRef = qobject_cast<LightRefObject *>(opaque)) {
        updateOrAddKeyframeFromLight(lightRef, timeIndex, parent);
    }
    else if (const MorphRefObject *morphRef = qobject_cast<MorphRefObject *>(opaque)) {
        updateOrAddKeyframeFromMorph(morphRef, timeIndex, parent);
    }
}

void MotionProxy::updateKeyframeInterpolation(QObject *opaque, const QVector4D &value, int type, QUndoCommand *parent)
{
    if (BoneKeyframeRefObject *boneKeyframeRef = qobject_cast<BoneKeyframeRefObject *>(opaque)) {
        m_undoStackRef->push(new UpdateBoneKeyframeInterpolationCommand(boneKeyframeRef, this, value, type, parent));
        m_projectRef->setDirty(true);
        VPVL2_VLOG(2, "updateInterpolation type=BONE timeIndex=" << boneKeyframeRef->timeIndex());
    }
    else if (CameraKeyframeRefObject *cameraKeyframeRef = qobject_cast<CameraKeyframeRefObject *>(opaque)) {
        m_undoStackRef->push(new UpdateCameraKeyframeInterpolationCommand(cameraKeyframeRef, this, value, type, parent));
        m_projectRef->setDirty(true);
        VPVL2_VLOG(2, "updateInterpolation type=CAMERA timeIndex=" << cameraKeyframeRef->timeIndex());
    }
}

void MotionProxy::removeKeyframe(QObject *opaque, QUndoCommand *parent)
{
    if (BaseKeyframeRefObject *keyframe = qobject_cast<BaseKeyframeRefObject *>(opaque)) {
        m_undoStackRef->push(new RemoveKeyframeCommand(keyframe, parent));
        m_projectRef->setDirty(true);
    }
}

void MotionProxy::removeAllSelectedKeyframes(QUndoCommand *parent)
{
    QList<BaseKeyframeRefObject *> keyframes;
    foreach (BaseKeyframeRefObject *keyframe, m_selectedKeyframeRefs) {
        if (keyframe->timeIndex() > 0) {
            keyframes.append(keyframe);
        }
    }
    removeKeyframes(keyframes, parent);
}

void MotionProxy::removeAllKeyframesAt(const qint64 &timeIndex, QUndoCommand *parent)
{
    QList<qint64> timeIndices;
    timeIndices.append(timeIndex);
    removeAllKeyframesIn(timeIndices, parent);
}

void MotionProxy::removeAllKeyframesIn(const QList<qint64> &timeIndices, QUndoCommand *parent)
{
    QList<BaseKeyframeRefObject *> keyframes;
    foreach (const qint64 &timeIndex, timeIndices) {
        if (timeIndex > 0) {
            QHashIterator<QString, BoneMotionTrack *> it(m_boneMotionTrackBundle);
            while (it.hasNext()) {
                it.next();
                if (BaseKeyframeRefObject *keyframe = it.value()->findKeyframeByTimeIndex(timeIndex)) {
                    keyframes.append(keyframe);
                }
            }
            QHashIterator<QString, MorphMotionTrack *> it2(m_morphMotionTrackBundle);
            while (it2.hasNext()) {
                it2.next();
                if (BaseKeyframeRefObject *keyframe = it2.value()->findKeyframeByTimeIndex(timeIndex)) {
                    keyframes.append(keyframe);
                }
            }
            if (m_cameraMotionTrackRef) {
                if (CameraKeyframeRefObject *keyframe = qobject_cast<CameraKeyframeRefObject *>(m_cameraMotionTrackRef->findKeyframeByTimeIndex(timeIndex))) {
                    keyframes.append(keyframe);
                }
            }
            if (m_lightMotionTrackRef) {
                if (LightKeyframeRefObject *keyframe = qobject_cast<LightKeyframeRefObject *>(m_lightMotionTrackRef->findKeyframeByTimeIndex(timeIndex))) {
                    keyframes.append(keyframe);
                }
            }
        }
    }
    removeKeyframes(keyframes, parent);
}

void MotionProxy::copyKeyframes()
{
    m_copiedKeyframeRefs = m_selectedKeyframeRefs;
    emit canPasteChanged();
}

void MotionProxy::pasteKeyframes(const qint64 &timeIndex, bool inversed, QUndoCommand *parent)
{
    if (inversed) {
        m_undoStackRef->push(new InversedPasteKeyframesCommand(this, m_copiedKeyframeRefs, timeIndex, &m_selectedKeyframeRefs, parent));
        m_projectRef->setDirty(true);
    }
    else {
        m_undoStackRef->push(new PasteKeyframesCommand(this, m_copiedKeyframeRefs, timeIndex, &m_selectedKeyframeRefs, parent));
        m_projectRef->setDirty(true);
    }
    VPVL2_VLOG(2, "paste timeIndex=" << timeIndex << " inversed=" << inversed);
}

void MotionProxy::cutKeyframes(QUndoCommand *parent)
{
    Q_UNUSED(parent)
    // TODO: implement this
}

IMotion *MotionProxy::data() const
{
    return m_motion.data();
}

ProjectProxy *MotionProxy::parentProject() const
{
    return m_projectRef;
}

ModelProxy *MotionProxy::parentModel() const
{
    return m_projectRef->resolveModelProxy(m_motion->parentModelRef());
}

QUuid MotionProxy::uuid() const
{
    return m_uuid;
}

QUrl MotionProxy::fileUrl() const
{
    return m_fileUrl;
}

qreal MotionProxy::durationTimeIndex() const
{
    return differenceTimeIndex(0);
}

qreal MotionProxy::duration() const
{
    return differenceDuration(0);
}

QQmlListProperty<BaseKeyframeRefObject> MotionProxy::selectedKeyframes()
{
    return QQmlListProperty<BaseKeyframeRefObject>(this, m_selectedKeyframeRefs);
}

bool MotionProxy::canPaste() const
{
    return m_copiedKeyframeRefs.size() > 0;
}

void MotionProxy::mergeKeyframes(const QList<QObject *> &keyframes, const qint64 &newTimeIndex, const qint64 &oldTimeIndex, QUndoCommand *parent)
{
    QList<MergeKeyframeCommand::Pair> keyframeRefs;
    foreach (QObject *item, keyframes) {
        BaseKeyframeRefObject *keyframe = qobject_cast<BaseKeyframeRefObject *>(item);
        Q_ASSERT(keyframe);
        BaseMotionTrack *track = keyframe->parentTrack();
        if (BaseKeyframeRefObject *src = track->findKeyframeByTimeIndex(newTimeIndex)) {
            keyframeRefs.append(MergeKeyframeCommand::Pair(keyframe, src));
        }
        else {
            keyframeRefs.append(MergeKeyframeCommand::Pair(keyframe, 0));
        }
    }
    if (!keyframeRefs.isEmpty()) {
        m_undoStackRef->push(new MergeKeyframeCommand(keyframeRefs, newTimeIndex, oldTimeIndex, parent));
        m_projectRef->setDirty(true);
        VPVL2_VLOG(2, "merge newTimeIndex=" << newTimeIndex << " oldTimeIndex=" << oldTimeIndex << " length=" << keyframeRefs.size());
    }
}

void MotionProxy::refresh()
{
    refreshBoneTracks();
    refreshMorphTracks();
    emit durationTimeIndexChanged();
}

BoneKeyframeRefObject *MotionProxy::addBoneKeyframe(const BoneRefObject *value) const
{
    BoneKeyframeRefObject *keyframe = 0;
    if (BoneMotionTrack *track = findBoneMotionTrack(value->name())) {
        Factory *factoryRef = m_projectRef->factoryInstanceRef();
        QScopedPointer<IBoneKeyframe> keyframePtr(factoryRef->createBoneKeyframe(m_motion.data()));
        keyframePtr->setDefaultInterpolationParameter();
        keyframePtr->setName(value->data()->name(IEncoding::kDefaultLanguage));
        keyframe = track->convertBoneKeyframe(keyframePtr.take());
    }
    return keyframe;
}

CameraKeyframeRefObject *MotionProxy::addCameraKeyframe(const CameraRefObject *value) const
{
    CameraKeyframeRefObject *keyframe = 0;
    Factory *factoryRef = m_projectRef->factoryInstanceRef();
    QScopedPointer<ICameraKeyframe> keyframePtr(factoryRef->createCameraKeyframe(m_motion.data()));
    ICamera *cameraRef = value->data();
    keyframePtr->setDefaultInterpolationParameter();
    keyframePtr->setAngle(cameraRef->angle());
    keyframePtr->setDistance(cameraRef->distance());
    keyframePtr->setFov(cameraRef->fov());
    keyframePtr->setLookAt(cameraRef->lookAt());
    keyframe = value->track()->convertCameraKeyframe(keyframePtr.take());
    return keyframe;
}

LightKeyframeRefObject *MotionProxy::addLightKeyframe(const LightRefObject *value) const
{
    LightKeyframeRefObject *keyframe = 0;
    Factory *factoryRef = m_projectRef->factoryInstanceRef();
    QScopedPointer<ILightKeyframe> keyframePtr(factoryRef->createLightKeyframe(m_motion.data()));
    ILight *lightRef = value->data();
    keyframePtr->setColor(lightRef->color());
    keyframePtr->setDirection(lightRef->direction());
    keyframe = value->track()->convertLightKeyframe(keyframePtr.take());
    return keyframe;
}

MorphKeyframeRefObject *MotionProxy::addMorphKeyframe(const MorphRefObject *value) const
{
    MorphKeyframeRefObject *keyframe = 0;
    if (MorphMotionTrack *track = findMorphMotionTrack(value->name())) {
        Factory *factoryRef = m_projectRef->factoryInstanceRef();
        QScopedPointer<IMorphKeyframe> keyframePtr(factoryRef->createMorphKeyframe(m_motion.data()));
        keyframePtr->setName(value->data()->name(IEncoding::kDefaultLanguage));
        keyframe = track->convertMorphKeyframe(keyframePtr.take());
    }
    return keyframe;
}

void MotionProxy::updateOrAddKeyframeFromBone(const BoneRefObject *boneRef, const qint64 &timeIndex, QUndoCommand *parent)
{
    const QString &name = boneRef->name();
    if (BoneMotionTrack *track = findBoneMotionTrack(name)) {
        if (BaseKeyframeRefObject *boneKeyframeRef = track->findKeyframeByTimeIndex(timeIndex)) {
            updateKeyframe(boneKeyframeRef, timeIndex, parent);
        }
        else {
            Factory *factoryRef = m_projectRef->factoryInstanceRef();
            QScopedPointer<IBoneKeyframe> newKeyframe(factoryRef->createBoneKeyframe(m_motion.data()));
            newKeyframe->setDefaultInterpolationParameter();
            newKeyframe->setName(boneRef->data()->name(IEncoding::kDefaultLanguage));
            newKeyframe->setTimeIndex(timeIndex);
            BoneKeyframeRefObject *newKeyframe2 = track->convertBoneKeyframe(newKeyframe.take());
            QScopedPointer<QUndoCommand> parentCommand(new QUndoCommand(parent));
            new AddKeyframeCommand(newKeyframe2, parentCommand.data());
            new UpdateBoneKeyframeCommand(newKeyframe2, this, parentCommand.data());
            m_undoStackRef->push(parentCommand.take());
            m_projectRef->setDirty(true);
            VPVL2_VLOG(2, "insert+update type=BONE timeIndex=" << timeIndex << " name=" << track->name().toStdString() << " length=" << track->length());
        }
    }
}

void MotionProxy::updateOrAddKeyframeFromCamera(CameraRefObject *cameraRef, const qint64 &timeIndex, QUndoCommand *parent)
{
    CameraMotionTrack *track = cameraRef->track();
    if (CameraKeyframeRefObject *cameraKeyframeRef = qobject_cast<CameraKeyframeRefObject *>(track->findKeyframeByTimeIndex(timeIndex))) {
        updateKeyframe(cameraKeyframeRef, timeIndex, parent);
    }
    else {
        Factory *factoryRef = m_projectRef->factoryInstanceRef();
        QScopedPointer<ICameraKeyframe> newKeyframe(factoryRef->createCameraKeyframe(m_motion.data()));
        newKeyframe->setDefaultInterpolationParameter();
        newKeyframe->setTimeIndex(timeIndex);
        CameraKeyframeRefObject *newKeyframe2 = track->convertCameraKeyframe(newKeyframe.take());
        QScopedPointer<QUndoCommand> parentCommand(new QUndoCommand(parent));
        new AddKeyframeCommand(newKeyframe2, parentCommand.data());
        new UpdateCameraKeyframeCommand(newKeyframe2, cameraRef, parentCommand.data());
        m_undoStackRef->push(parentCommand.take());
        m_projectRef->setDirty(true);
        VPVL2_VLOG(2, "insert+update type=CAMERA timeIndex=" << timeIndex << " length=" << track->length());
    }
}

void MotionProxy::updateOrAddKeyframeFromLight(LightRefObject *lightRef, const qint64 &timeIndex, QUndoCommand *parent)
{
    LightMotionTrack *track = lightRef->track();
    if (LightKeyframeRefObject *cameraKeyframeRef = qobject_cast<LightKeyframeRefObject *>(track->findKeyframeByTimeIndex(timeIndex))) {
        updateKeyframe(cameraKeyframeRef, timeIndex, parent);
    }
    else {
        Factory *factoryRef = m_projectRef->factoryInstanceRef();
        QScopedPointer<ILightKeyframe> newKeyframe(factoryRef->createLightKeyframe(m_motion.data()));
        newKeyframe->setTimeIndex(timeIndex);
        LightKeyframeRefObject *newKeyframe2 = track->convertLightKeyframe(newKeyframe.take());
        QScopedPointer<QUndoCommand> parentCommand(new QUndoCommand(parent));
        new AddKeyframeCommand(newKeyframe2, parentCommand.data());
        new UpdateLightKeyframeCommand(newKeyframe2, lightRef, parentCommand.data());
        m_undoStackRef->push(parentCommand.take());
        m_projectRef->setDirty(true);
        VPVL2_VLOG(2, "insert+update type=LIGHT timeIndex=" << timeIndex << " length=" << track->length());
    }
}

void MotionProxy::updateOrAddKeyframeFromMorph(const MorphRefObject *morphRef, const qint64 &timeIndex, QUndoCommand *parent)
{
    const QString &name = morphRef->name();
    if (MorphMotionTrack *track = findMorphMotionTrack(name)) {
        if (MorphKeyframeRefObject *morphKeyframeRef = qobject_cast<MorphKeyframeRefObject *>(track->findKeyframeByTimeIndex(timeIndex))) {
            updateKeyframe(morphKeyframeRef, timeIndex, parent);
        }
        else {
            Factory *factoryRef = m_projectRef->factoryInstanceRef();
            QScopedPointer<IMorphKeyframe> newKeyframe(factoryRef->createMorphKeyframe(m_motion.data()));
            newKeyframe->setName(morphRef->data()->name(IEncoding::kDefaultLanguage));
            newKeyframe->setTimeIndex(timeIndex);
            MorphKeyframeRefObject *newKeyframe2 = track->convertMorphKeyframe(newKeyframe.take());
            QScopedPointer<QUndoCommand> parentCommand(new QUndoCommand(parent));
            new AddKeyframeCommand(newKeyframe2, parentCommand.data());
            new UpdateMorphKeyframeCommand(newKeyframe2, this, parentCommand.data());
            m_undoStackRef->push(parentCommand.take());
            m_projectRef->setDirty(true);
            VPVL2_VLOG(2, "insert+update type=MORPH timeIndex=" << timeIndex << " name=" << track->name().toStdString() << " length=" << track->length());
        }
    }
}

void MotionProxy::loadBoneTrackBundle(IMotion *motionRef,
                                      int numBoneKeyframes,
                                      int numEstimatedKeyframes,
                                      int &numLoadedKeyframes)
{
    for (int i = 0; i < numBoneKeyframes; i++) {
        IBoneKeyframe *keyframe = motionRef->findBoneKeyframeRefAt(i);
        const QString &key = Util::toQString(keyframe->name());
        BoneMotionTrack *track = 0;
        if (m_boneMotionTrackBundle.contains(key)) {
            track = m_boneMotionTrackBundle.value(key);
        }
        else {
            track = addBoneTrack(key);
        }
        Q_ASSERT(track);
        track->add(track->convertBoneKeyframe(keyframe), false);
        emit motionBeLoading(numLoadedKeyframes++, numEstimatedKeyframes);
    }
}

void MotionProxy::loadMorphTrackBundle(IMotion *motionRef, int numMorphKeyframes, int numEstimatedKeyframes, int &numLoadedKeyframes)
{
    for (int i = 0; i < numMorphKeyframes; i++) {
        IMorphKeyframe *keyframe = motionRef->findMorphKeyframeRefAt(i);
        const QString &key = Util::toQString(keyframe->name());
        MorphMotionTrack *track = 0;
        if (m_morphMotionTrackBundle.contains(key)) {
            track = m_morphMotionTrackBundle.value(key);
        }
        else {
            track = addMorphTrack(key);
        }
        Q_ASSERT(track);
        track->add(track->convertMorphKeyframe(keyframe), false);
        emit motionBeLoading(numLoadedKeyframes++, numEstimatedKeyframes);
    }
}

void MotionProxy::removeKeyframes(const QList<BaseKeyframeRefObject *> &keyframes, QUndoCommand *parent)
{
    if (!keyframes.isEmpty()) {
        m_undoStackRef->push(new RemoveKeyframeCommand(keyframes, parent));
        m_projectRef->setDirty(true);
        VPVL2_VLOG(2, "remove length=" << keyframes.size());
    }
}

BoneMotionTrack *MotionProxy::addBoneTrack(const QString &key)
{
    BoneMotionTrack *track = new BoneMotionTrack(this, key);
    bindTrackSignals(track);
    m_boneMotionTrackBundle.insert(key, track);
    return track;
}

MorphMotionTrack *MotionProxy::addMorphTrack(const QString &key)
{
    MorphMotionTrack *track = new MorphMotionTrack(this, key);
    bindTrackSignals(track);
    m_morphMotionTrackBundle.insert(key, track);
    return track;
}

void MotionProxy::bindTrackSignals(BaseMotionTrack *track)
{
    connect(track, &BaseMotionTrack::keyframeDidAdd, this, &MotionProxy::keyframeDidAdd);
    connect(track, &BaseMotionTrack::keyframeDidRemove, this, &MotionProxy::keyframeDidRemove);
    connect(track, &BaseMotionTrack::keyframeDidSwap, this, &MotionProxy::keyframeDidReplace);
    connect(track, &BaseMotionTrack::timeIndexDidChange, this, &MotionProxy::timeIndexDidChange);
}
