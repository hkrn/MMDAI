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

#ifndef MODELPROXY_H
#define MODELPROXY_H

#include <QHash>
#include <QObject>
#include <QQmlListProperty>
#include <QQuaternion>
#include <QSet>
#include <QSharedPointer>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QUuid>
#include <QVector3D>

#include "ProjectProxy.h"

class QAbstractItemModel;
class QStringListModel;
class BoneRefObject;
class LabelRefObject;
class MorphRefObject;

namespace vpvl2 {
class IBone;
class ILabel;
class IModel;
class IMorph;
}

class ModelProxy : public QObject
{
    Q_OBJECT
    Q_ENUMS(AxisType)
    Q_ENUMS(TransformType)
    Q_PROPERTY(ProjectProxy *parentProject READ parentProject CONSTANT FINAL)
    Q_PROPERTY(ModelProxy *parentBindingModel READ parentBindingModel WRITE setParentBindingModel NOTIFY parentBindingModelChanged)
    Q_PROPERTY(BoneRefObject *parentBindingBone READ parentBindingBone WRITE setParentBindingBone NOTIFY parentBindingBoneChanged)
    Q_PROPERTY(MotionProxy *childMotion READ childMotion NOTIFY childMotionChanged)
    Q_PROPERTY(QUuid uuid READ uuid CONSTANT FINAL)
    Q_PROPERTY(QUrl fileUrl READ fileUrl CONSTANT FINAL)
    Q_PROPERTY(QUrl faviconUrl READ faviconUrl CONSTANT FINAL)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged FINAL)
    Q_PROPERTY(QString comment READ comment NOTIFY commentChanged FINAL)
    Q_PROPERTY(QQmlListProperty<LabelRefObject> availableLabels READ availableLabels CONSTANT FINAL)
    Q_PROPERTY(QQmlListProperty<BoneRefObject> availableBones READ availableBones CONSTANT FINAL)
    Q_PROPERTY(QQmlListProperty<MorphRefObject> availableMorphs READ availableMorphs CONSTANT FINAL)
    Q_PROPERTY(QQmlListProperty<BoneRefObject> targetBones READ targetBones CONSTANT FINAL)
    Q_PROPERTY(BoneRefObject *firstTargetBone READ firstTargetBone NOTIFY firstTargetBoneChanged FINAL)
    Q_PROPERTY(MorphRefObject *firstTargetMorph READ firstTargetMorph WRITE setFirstTargetMorph NOTIFY firstTargetMorphChanged FINAL)
    Q_PROPERTY(AxisType axisType READ axisType WRITE setAxisType NOTIFY axisTypeChanged FINAL)
    Q_PROPERTY(TransformType transformType READ transformType WRITE setTransformType NOTIFY transformTypeChanged FINAL)
    Q_PROPERTY(QVector3D translation READ translation WRITE setTranslation NOTIFY translationChanged FINAL)
    Q_PROPERTY(QQuaternion orientation READ orientation WRITE setOrientation NOTIFY orientationChanged FINAL)
    Q_PROPERTY(ProjectProxy::LanguageType language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(qreal scaleFactor READ scaleFactor WRITE setScaleFactor NOTIFY scaleFactorChanged FINAL)
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity NOTIFY opacityChanged FINAL)
    Q_PROPERTY(qreal edgeWidth READ edgeWidth WRITE setEdgeWidth NOTIFY edgeWidthChanged FINAL)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged FINAL)
    Q_PROPERTY(bool moving READ isMoving NOTIFY movingChanged FINAL)

public:
    enum AxisType {
        AxisX,
        AxisY,
        AxisZ
    };
    enum TransformType {
        LocalTransform,
        GlobalTransform,
        ViewTransform
    };

    ModelProxy(ProjectProxy *project,
               vpvl2::IModel *model,
               const QUuid &uuid,
               const QUrl &fileUrl,
               const QUrl &faviconUrl);
    ~ModelProxy();

    void initialize();
    void addBindingModel(ModelProxy *value);
    void removeBindingModel(ModelProxy *value);

    vpvl2::IModel *data() const;
    ProjectProxy *parentProject() const;
    ModelProxy *parentBindingModel() const;
    void setParentBindingModel(ModelProxy *value);
    BoneRefObject *parentBindingBone() const;
    void setParentBindingBone(BoneRefObject *value);
    MotionProxy *childMotion() const;
    void setChildMotion(MotionProxy *value);
    QUuid uuid() const;
    QUrl fileUrl() const;
    QUrl faviconUrl() const;
    QString name() const;
    QString comment() const;
    QQmlListProperty<LabelRefObject> availableLabels();
    QQmlListProperty<BoneRefObject> availableBones();
    QQmlListProperty<MorphRefObject> availableMorphs();
    QQmlListProperty<BoneRefObject> targetBones();
    QList<BoneRefObject *> allTargetBones() const;
    BoneRefObject *firstTargetBone() const;
    MorphRefObject *firstTargetMorph() const;
    void setFirstTargetMorph(MorphRefObject *value);
    AxisType axisType() const;
    void setAxisType(AxisType value);
    TransformType transformType() const;
    void setTransformType(TransformType value);
    ProjectProxy::LanguageType language() const;
    void setLanguage(ProjectProxy::LanguageType value);
    QVector3D translation() const;
    void setTranslation(const QVector3D &value);
    QQuaternion orientation() const;
    void setOrientation(const QQuaternion &value);
    qreal scaleFactor() const;
    void setScaleFactor(qreal value);
    qreal edgeWidth() const;
    void setEdgeWidth(qreal value);
    qreal opacity() const;
    void setOpacity(qreal value);
    QList<vpvl2::ILabel *> allLabels() const;
    bool isVisible() const;
    void setVisible(bool value);
    bool isMoving() const;

    QList<LabelRefObject *> allLabelRefs() const;
    QList<BoneRefObject *> allBoneRefs() const;
    QList<MorphRefObject *> allMorphRefs() const;

signals:
    void parentBindingModelChanged();
    void parentBindingBoneChanged();
    void childMotionChanged();
    void nameChanged();
    void commentChanged();
    void firstTargetBoneChanged();
    void firstTargetMorphChanged();
    void axisTypeChanged();
    void transformTypeChanged();
    void languageChanged();
    void translationChanged();
    void orientationChanged();
    void scaleFactorChanged();
    void opacityChanged();
    void edgeWidthChanged();
    void visibleChanged();
    void movingChanged();
    void targetBonesDidBeginTransform();
    void targetBonesDidTranslate();
    void targetBonesDidRotate();
    void targetBonesDidCommitTransform();
    void boneDidSelect(BoneRefObject *bone);
    void morphDidSelect(MorphRefObject *morph);
    void modelDidRefresh();
    void modelWillLoad(int numEstimatedObjects);
    void modelBeLoading(int numLoadedObjects, int numEstimatedObjects);
    void modelDidLoad(int numLoadedObjects, int numEstimatedObjects);

public slots:
    Q_INVOKABLE void selectOpaqueObject(QObject *value);
    Q_INVOKABLE void selectBone(BoneRefObject *value);
    Q_INVOKABLE void beginTranslate(qreal startY);
    Q_INVOKABLE void translate(qreal value);
    Q_INVOKABLE void endTranslate();
    Q_INVOKABLE void beginRotate(qreal startY);
    Q_INVOKABLE void rotate(qreal angle);
    Q_INVOKABLE void endRotate();
    Q_INVOKABLE void resetTargets();
    Q_INVOKABLE void release();
    Q_INVOKABLE void refresh();
    Q_INVOKABLE BoneRefObject *resolveBoneRef(const vpvl2::IBone *value) const;
    Q_INVOKABLE BoneRefObject *findBoneByName(const QString &name) const;
    Q_INVOKABLE BoneRefObject *findBoneByUuid(const QUuid &uuid) const;
    Q_INVOKABLE MorphRefObject *resolveMorphRef(const vpvl2::IMorph *value) const;
    Q_INVOKABLE MorphRefObject *findMorphByName(const QString &name) const;
    Q_INVOKABLE MorphRefObject *findMorphByUuid(const QUuid &uuid) const;
    Q_INVOKABLE QList<QObject *> findMorphsByCategory(int type) const;

private:
    void buildLabelHash(const vpvl2::IModel *model,
                        QHash<vpvl2::IBone *, vpvl2::ILabel *> &bone2label,
                        QHash<vpvl2::IMorph *, vpvl2::ILabel *> &morph2label);
    void setAllBones(const vpvl2::Array<vpvl2::ILabel *> &labelRefs, int numEstimatedObjects, int &numLoadedObjects);
    void setAllMorphs(const vpvl2::Array<vpvl2::ILabel *> &labelRefs, const vpvl2::IModel *model, int numEstimatedObjects, int &numLoadedObjects);
    void saveTransformState();
    void clearTransformState();
    vpvl2::IEncoding::LanguageType languageType() const;

    ProjectProxy *m_parentProjectRef;
    MotionProxy *m_childMotionRef;
    QScopedPointer<vpvl2::IModel> m_model;
    typedef QPair<vpvl2::Vector3, vpvl2::Quaternion> InternalTransform;
    QHash<BoneRefObject *, InternalTransform> m_transformState;
    const QUuid m_uuid;
    const QUrl m_fileUrl;
    const QUrl m_faviconUrl;
    QHash<const vpvl2::IBone *, BoneRefObject *> m_bone2Refs;
    QHash<const vpvl2::IMorph *, MorphRefObject *> m_morph2Refs;
    QHash<QString, BoneRefObject *> m_name2boneRefs;
    QHash<QUuid, BoneRefObject *> m_uuid2boneRefs;
    QHash<QString, MorphRefObject *> m_name2morphRefs;
    QHash<QUuid, MorphRefObject *> m_uuid2morphRefs;
    QList<LabelRefObject *> m_allLabels;
    QList<BoneRefObject *> m_allBones;
    QList<BoneRefObject *> m_availableBoneRefs;
    QList<BoneRefObject *> m_targetBoneRefs;
    QList<MorphRefObject *> m_allMorphs;
    QList<ModelProxy *> m_bindingModels;
    MorphRefObject * m_targetMorphRef;
    AxisType m_boneAxisType;
    TransformType m_boneTransformType;
    ProjectProxy::LanguageType m_language;
    qreal m_baseY;
    bool m_moving;
};

#endif // MODELPROXY_H
