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

#ifndef PROJECT_H
#define PROJECT_H

#include <QColor>
#include <QHash>
#include <QObject>
#include <QPoint>
#include <QQmlListProperty>
#include <QScopedPointer>
#include <QString>
#include <QUrl>
#include <QUuid>

#include <vpvl2/extensions/icu4c/Encoding.h>
#include <vpvl2/extensions/XMLProject.h>

namespace vpvl2 {
class Factory;
class IEncoding;
class IModel;
class IMotion;
namespace extensions {
class World;
}
}

class BaseKeyframeRefObject;
class BoneRefObject;
class CameraRefObject;
class LightRefObject;
class ModelProxy;
class MorphRefObject;
class MotionProxy;
class RenderTarget;
class WorldProxy;

class QUndoGroup;
class QUndoStack;

class ProjectProxy : public QObject
{
    Q_OBJECT

    Q_ENUMS(AccelerationType)
    Q_ENUMS(DataType)
    Q_ENUMS(LanguageType)
    Q_ENUMS(MotionType)
    Q_ENUMS(ResetBoneType)
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged FINAL)
    Q_PROPERTY(QQmlListProperty<ModelProxy> availableModels READ availableModels NOTIFY availableModelsChanged FINAL)
    Q_PROPERTY(QQmlListProperty<MotionProxy> availableMotions READ availableMotions NOTIFY availableMotionsChanged FINAL)
    Q_PROPERTY(QQmlListProperty<QObject> availableParentBindingModels READ availableParentBindingModels NOTIFY availableParentBindingModelsChanged FINAL)
    Q_PROPERTY(QQmlListProperty<QObject> availableParentBindingBones READ availableParentBindingBones NOTIFY availableParentBindingBonesChanged FINAL)
    Q_PROPERTY(ModelProxy *currentModel READ currentModel WRITE setCurrentModel NOTIFY currentModelChanged FINAL)
    Q_PROPERTY(MotionProxy *currentMotion READ currentMotion WRITE setCurrentMotion NOTIFY currentMotionChanged FINAL)
    Q_PROPERTY(QUrl audioSource READ audioSource WRITE setAudioSource NOTIFY audioSourceChanged FINAL)
    Q_PROPERTY(QColor screenColor READ screenColor WRITE setScreenColor NOTIFY screenColorChanged FINAL)
    Q_PROPERTY(qreal currentTimeIndex READ currentTimeIndex NOTIFY currentTimeIndexChanged FINAL)
    Q_PROPERTY(qreal durationTimeIndex READ durationTimeIndex NOTIFY durationTimeIndexChanged FINAL)
    Q_PROPERTY(qreal durationMilliseconds READ durationMilliseconds NOTIFY durationTimeIndexChanged FINAL)
    Q_PROPERTY(QString errorString READ errorString NOTIFY errorStringChanged FINAL)
    Q_PROPERTY(CameraRefObject *camera READ camera NOTIFY cameraChanged FINAL)
    Q_PROPERTY(LightRefObject *light READ light NOTIFY lightChanged FINAL)
    Q_PROPERTY(WorldProxy *world READ world CONSTANT FINAL)
    Q_PROPERTY(AccelerationType accelerationType READ accelerationType WRITE setAccelerationType NOTIFY accelerationTypeChanged FINAL)
    Q_PROPERTY(LanguageType language READ language WRITE setLanguage NOTIFY languageChanged FINAL)
    Q_PROPERTY(bool loop READ isLoop WRITE setLoop NOTIFY loopChanged FINAL)
    Q_PROPERTY(bool dirty READ isDirty NOTIFY dirtyChanged FINAL)
    Q_PROPERTY(bool canUndo READ canUndo NOTIFY canUndoChanged FINAL)
    Q_PROPERTY(bool canRedo READ canRedo NOTIFY canRedoChanged FINAL)

public:
    enum AccelerationType {
        NoAcceleration        = vpvl2::Scene::kSoftwareFallback,
        OpenCLGPUAcceleration = vpvl2::Scene::kOpenCLAccelerationType1,
        VertexShaderSkinning  = vpvl2::Scene::kVertexShaderAccelerationType1,
        OpenCLCPUAcceleration = vpvl2::Scene::kOpenCLAccelerationType2,
        ParallelAcceleration,
        MaxAccelerationType
    };
    enum DataType {
        Bone,
        Morph,
        MaxDataType
    };
    enum LanguageType {
        DefaultLauguage = vpvl2::IEncoding::kDefaultLanguage,
        Japanese        = vpvl2::IEncoding::kJapanese,
        English         = vpvl2::IEncoding::kEnglish,
        MaxLanguageType
    };
    enum MotionType {
        ModelMotion,
        CameraMotion,
        LightMotion,
        MaxMotionType
    };
    enum ResetBoneType {
        TranslationAxisX,
        TranslationAxisY,
        TranslationAxisZ,
        TranslationAxisXYZ,
        Orientation,
        AllTranslationAndOrientation,
        MaxResetBoneType
    };

    explicit ProjectProxy(QObject *parent = 0);
    ~ProjectProxy();

    Q_INVOKABLE void initializeOnce();
    Q_INVOKABLE void createAsync();
    Q_INVOKABLE void loadAsync(const QUrl &fileUrl);
    Q_INVOKABLE bool save(const QUrl &fileUrl);
    Q_INVOKABLE qreal differenceTimeIndex(qreal value) const;
    Q_INVOKABLE qreal differenceDuration(qreal value) const;
    Q_INVOKABLE qreal secondsFromTimeIndex(qreal value) const;
    Q_INVOKABLE qreal millisecondsFromTimeIndex(qreal value) const;
    Q_INVOKABLE void resetBone(BoneRefObject *bone, ResetBoneType type);
    Q_INVOKABLE void resetMorph(MorphRefObject *morph);
    Q_INVOKABLE void updateParentBindingModel();

    ModelProxy *loadModel(const QUrl &fileUrl, const QUuid &uuid, bool skipConfirm);
    ModelProxy *createModelProxy(vpvl2::IModel *model, const QUuid &uuid, const QUrl &fileUrl, bool skipConfirm);
    MotionProxy *createMotionProxy(vpvl2::IMotion *motion, const QUuid &uuid, const QUrl &fileUrl, bool emitSignal);
    ModelProxy *resolveModelProxy(const vpvl2::IModel *value) const;
    MotionProxy *resolveMotionProxy(const vpvl2::IMotion *value) const;
    void internalDeleteAllMotions(bool deleteLater);
    void deleteMotion(MotionProxy *value, bool deleteLater);
    QVariant globalSetting(const QString &key, const QVariant &defaultValue = QVariant()) const;
    QVector3D globalSetting(const QString &key, const QVector3D &defaultValue) const;
    void setGlobalString(const QString &key, const QVariant &value);
    QVariant modelSetting(const ModelProxy *modelProxy, const QString &key, const QVariant &defaultValue = QVariant()) const;
    void setModelSetting(const ModelProxy *modelProxy, const QString &key, const QVariant &value) const;

    QList<ModelProxy *> modelProxies() const;
    QList<MotionProxy *> motionProxies() const;
    QQmlListProperty<ModelProxy> availableModels();
    QQmlListProperty<MotionProxy> availableMotions();
    QQmlListProperty<QObject> availableParentBindingModels();
    QQmlListProperty<QObject> availableParentBindingBones();
    QString title() const;
    void setTitle(const QString &value);
    ModelProxy *currentModel() const;
    void setCurrentModel(ModelProxy *value);
    MotionProxy *currentMotion() const;
    void setCurrentMotion(MotionProxy *value);
    QUrl audioSource() const;
    void setAudioSource(const QUrl &value);
    QColor screenColor() const;
    void setScreenColor(const QColor &value);
    LanguageType language() const;
    void setLanguage(LanguageType value);
    AccelerationType accelerationType() const;
    void setAccelerationType(AccelerationType value);
    bool isDirty() const;
    void setDirty(bool value);
    bool isPhysicsSimulationEnabled() const;
    void setPhysicsSimulationEnabled(bool value);
    bool isLoop() const;
    void setLoop(bool value);
    bool canUndo() const;
    bool canRedo() const;
    qreal currentTimeIndex() const;
    qreal durationTimeIndex() const;
    qreal durationMilliseconds() const;
    QString errorString() const;
    CameraRefObject *camera() const;
    LightRefObject *light() const;
    WorldProxy *world() const;

    vpvl2::IEncoding *encodingInstanceRef() const;
    vpvl2::Factory *factoryInstanceRef() const;
    vpvl2::extensions::XMLProject *projectInstanceRef() const;

public slots:
    Q_INVOKABLE ModelProxy *findModel(const QUuid &uuid);
    Q_INVOKABLE MotionProxy *findMotion(const QUuid &uuid);
    Q_INVOKABLE bool loadModel(const QUrl &fileUrl);
    Q_INVOKABLE void addModel(ModelProxy *value);
    Q_INVOKABLE void deleteModel(ModelProxy *value);
    Q_INVOKABLE bool loadMotion(const QUrl &fileUrl, ModelProxy *modelProxy, MotionType type);
    Q_INVOKABLE bool loadPose(const QUrl &fileUrl, ModelProxy *modelProxy);
    Q_INVOKABLE void seek(qreal timeIndex);
    Q_INVOKABLE void rewind();
    Q_INVOKABLE void refresh();
    Q_INVOKABLE void ray(qreal x, qreal y, int width, int height);
    Q_INVOKABLE void undo();
    Q_INVOKABLE void redo();
    void internalAddModel(ModelProxy *value, bool selected, bool isProject);
    void internalDeleteModel(ModelProxy *value);
    void internalCreateAsync();
    void internalLoadAsync();
    void update(int flags);
    void reset();

signals:
    void projectDidRelease();
    void projectWillCreate();
    void projectDidCreate();
    void projectWillLoad();
    void projectDidLoad();
    void projectWillSave();
    void projectDidSave();
    void modelWillLoad(ModelProxy *model);
    void modelDidLoad(ModelProxy *model, bool skipConfirm);
    void modelDidAdd(ModelProxy *model, bool isProject);
    void modelWillRemove(ModelProxy *model);
    void modelDidRemove(ModelProxy *model);
    void motionWillLoad(MotionProxy *motion);
    void motionDidLoad(MotionProxy *motion);
    void motionWillDelete(MotionProxy *motion);
    void poseDidLoad(ModelProxy *model);
    void modelBoneDidPick(BoneRefObject *bone);
    void modelDidCommitUploading();
    void modelDidCommitDeleting();
    void enqueuedModelsDidDelete();
    void parentBindingDidUpdate();
    void rewindDidPerform();
    void availableModelsChanged();
    void availableMotionsChanged();
    void availableParentBindingModelsChanged();
    void availableParentBindingBonesChanged();
    void titleChanged();
    void currentTimeIndexChanged();
    void currentModelChanged();
    void currentMotionChanged();
    void audioSourceChanged();
    void screenColorChanged();
    void durationTimeIndexChanged();
    void errorStringChanged();
    void cameraChanged();
    void lightChanged();
    void accelerationTypeChanged();
    void languageChanged();
    void loopChanged();
    void dirtyChanged();
    void undoDidPerform();
    void redoDidPerform();
    void canUndoChanged();
    void canRedoChanged();

private:
    static void resetIKEffectorBones(BoneRefObject *bone);
    void createProjectInstance();
    void assignCamera();
    void assignLight();
    void internalSeek(const qreal &timeIndex, bool forceUpdate);
    void updateOriginValues();
    void setErrorString(const QString &value);
    void release();

    vpvl2::extensions::icu4c::Encoding::Dictionary m_dictionary;
    QScopedPointer<vpvl2::IEncoding> m_encoding;
    QScopedPointer<vpvl2::Factory> m_factory;
    QScopedPointer<vpvl2::extensions::XMLProject::IDelegate> m_delegate;
    QScopedPointer<vpvl2::extensions::XMLProject> m_project;
    QScopedPointer<CameraRefObject> m_cameraRefObject;
    QScopedPointer<LightRefObject> m_lightRefObject;
    QScopedPointer<WorldProxy> m_worldProxy;
    QScopedPointer<QUndoGroup> m_undoGroup;
    QHash<const vpvl2::IModel *, ModelProxy *> m_instance2ModelProxyRefs;
    QHash<const vpvl2::IMotion *, MotionProxy *> m_instance2MotionProxyRefs;
    QHash<QUuid, ModelProxy *> m_uuid2ModelProxyRefs;
    QHash<QUuid, MotionProxy *> m_uuid2MotionProxyRefs;
    QHash<MotionProxy *, QUndoStack *> m_motion2UndoStacks;
    QList<ModelProxy *> m_modelProxies;
    QList<MotionProxy *> m_motionProxies;
    QList<QObject *> m_parentModelProxyRefs;
    QList<QObject *> m_parentModelBoneRefs;
    QUrl m_fileUrl;
    QString m_title;
    QColor m_screenColor;
    ModelProxy *m_currentModelRef;
    MotionProxy *m_currentMotionRef;
    QString m_errorString;
    qreal m_currentTimeIndex;
    AccelerationType m_accelerationType;
    LanguageType m_language;
    bool m_initialized;
};

#endif // PROJECT_H
