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

#ifndef MOTIONPROXY_H
#define MOTIONPROXY_H

#include <QObject>
#include <QQmlListProperty>
#include <QSet>
#include <QSharedPointer>
#include <QUrl>
#include <QUuid>

#include <vpvl2/IKeyframe.h>

class BaseMotionTrack;
class BaseKeyframeRefObject;
class BoneKeyframeRefObject;
class BoneMotionTrack;
class BoneRefObject;
class CameraKeyframeRefObject;
class CameraMotionTrack;
class CameraRefObject;
class LightKeyframeRefObject;
class LightMotionTrack;
class LightRefObject;
class ModelProxy;
class MorphKeyframeRefObject;
class MorphMotionTrack;
class MorphRefObject;
class ProjectProxy;
class QUndoCommand;
class QUndoStack;

namespace vpvl2 {
class IBoneKeyframe;
class IKeyframe;
class IMorphKeyframe;
class IMotion;
}

class MotionProxy : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ProjectProxy *parentProject READ parentProject CONSTANT FINAL)
    Q_PROPERTY(ModelProxy *parentModel READ parentModel CONSTANT FINAL)
    Q_PROPERTY(QUuid uuid READ uuid CONSTANT FINAL)
    Q_PROPERTY(QUrl fileUrl READ fileUrl CONSTANT FINAL)
    Q_PROPERTY(qreal maxTimeIndex READ maxTimeIndex NOTIFY maxTimeIndexChanged FINAL)
    Q_PROPERTY(qreal duration READ duration NOTIFY maxTimeIndexChanged FINAL)
    Q_PROPERTY(QQmlListProperty<BaseKeyframeRefObject> selectedKeyframes READ selectedKeyframes CONSTANT FINAL)
    Q_PROPERTY(bool canPaste READ canPaste NOTIFY canPasteChanged FINAL)

public:
    MotionProxy(ProjectProxy *projectRef,
                vpvl2::IMotion *motion,
                const QUuid &uuid,
                const QUrl &fileUrl,
                QUndoStack *undoStackRef);
    ~MotionProxy();

    void initialize();
    void setCameraMotionTrack(CameraMotionTrack *track);
    void setLightMotionTrack(LightMotionTrack *track);
    void refreshBoneTracks();
    void refreshMorphTracks();

    Q_INVOKABLE bool save(const QUrl &fileUrl);
    Q_INVOKABLE qreal differenceTimeIndex(qreal value) const;
    Q_INVOKABLE qreal differenceDuration(qreal value) const;
    Q_INVOKABLE BoneMotionTrack *findBoneMotionTrack(const BoneRefObject *value) const;
    Q_INVOKABLE BoneMotionTrack *findBoneMotionTrack(const QString &name) const;
    Q_INVOKABLE MorphMotionTrack *findMorphMotionTrack(const MorphRefObject *value) const;
    Q_INVOKABLE MorphMotionTrack *findMorphMotionTrack(const QString &name) const;
    Q_INVOKABLE BaseKeyframeRefObject *resolveKeyframeAt(const qint64 &timeIndex, QObject *opaque) const;

    vpvl2::IMotion *data() const;
    ProjectProxy *parentProject() const;
    ModelProxy *parentModel() const;
    QUuid uuid() const;
    QUrl fileUrl() const;
    qreal maxTimeIndex() const;
    qreal duration() const;
    QQmlListProperty<BaseKeyframeRefObject> selectedKeyframes();
    bool canPaste() const;

signals:
    void maxTimeIndexChanged();
    void keyframeDidAdd(BaseKeyframeRefObject *keyframe);
    void keyframeDidRemove(BaseKeyframeRefObject *keyframe);
    void keyframeDidReplace(BaseKeyframeRefObject *dst, BaseKeyframeRefObject *src);
    void timeIndexDidChange(BaseKeyframeRefObject *keyframe, qint64 newTimeIndex, qint64 oldTimeIndex);
    void canPasteChanged();
    void motionWillLoad(int numEstimatedKeyframes);
    void motionBeLoading(int numLoadedKeyframes, int numEstimatedKeyframes);
    void motionDidLoad(int numLoadedKeyframes, int numEstimatedKeyframes);

public slots:
    Q_INVOKABLE void applyParentModel();
    Q_INVOKABLE void addKeyframe(QObject *opaque, const qint64 &timeIndex, QUndoCommand *parent = 0);
    Q_INVOKABLE void updateKeyframe(QObject *opaque, const qint64 &timeIndex, QUndoCommand *parent = 0);
    Q_INVOKABLE void updateKeyframeInterpolation(QObject *opaque, const QVector4D &value, int type, QUndoCommand *parent = 0);
    Q_INVOKABLE void removeKeyframe(QObject *opaque, QUndoCommand *parent = 0);
    Q_INVOKABLE void removeAllSelectedKeyframes(QUndoCommand *parent = 0);
    Q_INVOKABLE void removeAllKeyframesAt(const qint64 &timeIndex, QUndoCommand *parent = 0);
    Q_INVOKABLE void removeAllKeyframesIn(const QList<qint64> &timeIndices, QUndoCommand *parent = 0);
    Q_INVOKABLE void copyKeyframes();
    Q_INVOKABLE void pasteKeyframes(const qint64 &timeIndex, bool inversed, QUndoCommand *parent = 0);
    Q_INVOKABLE void cutKeyframes(QUndoCommand *parent = 0);
    Q_INVOKABLE void mergeKeyframes(const QList<QObject *> &keyframes, const qint64 &newTimeIndex, const qint64 &oldTimeIndex, QUndoCommand *parent = 0);
    Q_INVOKABLE void refresh();

private:
    BoneKeyframeRefObject *addBoneKeyframe(const BoneRefObject *value) const;
    CameraKeyframeRefObject *addCameraKeyframe(const CameraRefObject *value) const;
    LightKeyframeRefObject *addLightKeyframe(const LightRefObject *value) const;
    MorphKeyframeRefObject *addMorphKeyframe(const MorphRefObject *value) const;
    void updateOrAddKeyframeFromBone(const BoneRefObject *boneRef, const qint64 &timeIndex, QUndoCommand *parent);
    void updateOrAddKeyframeFromCamera(CameraRefObject *cameraRef, const qint64 &timeIndex, QUndoCommand *parent);
    void updateOrAddKeyframeFromLight(LightRefObject *lightRef, const qint64 &timeIndex, QUndoCommand *parent);
    void updateOrAddKeyframeFromMorph(const MorphRefObject *morphRef, const qint64 &timeIndex, QUndoCommand *parent);
    void loadBoneTrackBundle(vpvl2::IMotion *motionRef, int numBoneKeyframes, int numEstimatedKeyframes, int &numLoadedKeyframes);
    void loadMorphTrackBundle(vpvl2::IMotion *motionRef, int numMorphKeyframes, int numEstimatedKeyframes, int &numLoadedKeyframes);
    void removeKeyframes(const QList<BaseKeyframeRefObject *> &keyframes, QUndoCommand *parent);
    void bindTrackSignals(BaseMotionTrack *track);

    typedef QHash<QString, BoneMotionTrack *> BoneMotionTrackBundle;
    typedef QHash<QString, MorphMotionTrack *> MorphMotionTrackBundle;
    ProjectProxy *m_projectRef;
    CameraMotionTrack *m_cameraMotionTrackRef;
    LightMotionTrack *m_lightMotionTrackRef;
    QScopedPointer<vpvl2::IMotion> m_motion;
    BoneMotionTrackBundle m_boneMotionTrackBundle;
    MorphMotionTrackBundle m_morphMotionTrackBundle;
    QList<BaseKeyframeRefObject *> m_selectedKeyframeRefs;
    QList<BaseKeyframeRefObject *> m_copiedKeyframeRefs;
    QUndoStack *m_undoStackRef;
    QUuid m_uuid;
    QUrl m_fileUrl;
};

#endif // MOTIONPROXY_H
