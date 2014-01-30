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

#ifndef MORPHREFOBJECT_H
#define MORPHREFOBJECT_H

#include <QColor>
#include <QObject>
#include <QQmlListProperty>
#include <QQuaternion>
#include <QVector3D>
#include <QVector4D>
#include <QUuid>
#include <vpvl2/IMorph.h>

class BoneRefObject;
class LabelRefObject;
class MaterialRefObject;
class ModelProxy;
class MorphRefObject;
class RigidBodyRefObject;
class VertexRefObject;

class ChildGroupMorphRefObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(MorphRefObject *parentMorph READ parentMorph CONSTANT FINAL)
    Q_PROPERTY(QString name READ name CONSTANT FINAL)
    Q_PROPERTY(int index READ index CONSTANT FINAL)
    Q_PROPERTY(MorphRefObject *targetMorph READ targetMorph WRITE setTargetMorph NOTIFY targetMorphChanged FINAL)
    Q_PROPERTY(qreal fixedWeight READ fixedWeight WRITE setFixedWeight NOTIFY fixedWeightChanged FINAL)

public:
    ChildGroupMorphRefObject(MorphRefObject *parentMorphRef, vpvl2::IMorph::Group *valueRef);
    ~ChildGroupMorphRefObject();

    MorphRefObject *parentMorph() const;
    QString name() const;
    int index() const;
    MorphRefObject *targetMorph() const;
    void setTargetMorph(MorphRefObject *value);
    qreal fixedWeight() const;
    void setFixedWeight(const qreal &value);

signals:
    void targetMorphChanged();
    void fixedWeightChanged();

private:
    MorphRefObject *m_parentMorphRef;
    vpvl2::IMorph::Group *m_valueRef;
};


class ChildVertexMorphRefObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(MorphRefObject *parentMorph READ parentMorph CONSTANT FINAL)
    Q_PROPERTY(QString name READ name CONSTANT FINAL)
    Q_PROPERTY(int index READ index CONSTANT FINAL)
    Q_PROPERTY(VertexRefObject *targetVertex READ targetVertex WRITE setTargetVertex NOTIFY targetVertexChanged FINAL)
    Q_PROPERTY(QVector3D position READ position WRITE setPosition NOTIFY positionChanged FINAL)

public:
    ChildVertexMorphRefObject(MorphRefObject *parentMorphRef, vpvl2::IMorph::Vertex *valueRef);
    ~ChildVertexMorphRefObject();

    MorphRefObject *parentMorph() const;
    QString name() const;
    int index() const;
    VertexRefObject *targetVertex() const;
    void setTargetVertex(VertexRefObject *value);
    QVector3D position() const;
    void setPosition(const QVector3D &value);

signals:
    void targetVertexChanged();
    void positionChanged();

private:
    MorphRefObject *m_parentMorphRef;
    vpvl2::IMorph::Vertex *m_valueRef;
};

class ChildBoneMorphRefObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(MorphRefObject *parentMorph READ parentMorph CONSTANT FINAL)
    Q_PROPERTY(QString name READ name CONSTANT FINAL)
    Q_PROPERTY(int index READ index CONSTANT FINAL)
    Q_PROPERTY(BoneRefObject *targetBone READ targetBone WRITE setTargetBone NOTIFY targetBoneChanged FINAL)
    Q_PROPERTY(QVector3D position READ position WRITE setPosition NOTIFY positionChanged FINAL)
    Q_PROPERTY(QQuaternion rotation READ rotation WRITE setRotation NOTIFY rotationChanged FINAL)

public:
    ChildBoneMorphRefObject(MorphRefObject *parentMorphRef, vpvl2::IMorph::Bone *valueRef);
    ~ChildBoneMorphRefObject();

    MorphRefObject *parentMorph() const;
    QString name() const;
    int index() const;
    BoneRefObject *targetBone() const;
    void setTargetBone(BoneRefObject *value);
    QVector3D position() const;
    void setPosition(const QVector3D &value);
    QQuaternion rotation() const;
    void setRotation(const QQuaternion &value);

signals:
    void targetBoneChanged();
    void positionChanged();
    void rotationChanged();

private:
    MorphRefObject *m_parentMorphRef;
    vpvl2::IMorph::Bone *m_valueRef;
};

class ChildUVMorphRefObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(MorphRefObject *parentMorph READ parentMorph CONSTANT FINAL)
    Q_PROPERTY(QString name READ name CONSTANT FINAL)
    Q_PROPERTY(int index READ index CONSTANT FINAL)
    Q_PROPERTY(VertexRefObject *targetVertex READ targetVertex WRITE setTargetVertex NOTIFY targetVertexChanged FINAL)
    Q_PROPERTY(QVector4D position READ position WRITE setPosition NOTIFY positionChanged FINAL)

public:
    ChildUVMorphRefObject(MorphRefObject *parentMorphRef, vpvl2::IMorph::UV *valueRef);
    ~ChildUVMorphRefObject();

    MorphRefObject *parentMorph() const;
    QString name() const;
    int index() const;
    VertexRefObject *targetVertex() const;
    void setTargetVertex(VertexRefObject *value);
    QVector4D position() const;
    void setPosition(const QVector4D &value);

signals:
    void targetVertexChanged();
    void positionChanged();

private:
    MorphRefObject *m_parentMorphRef;
    vpvl2::IMorph::UV *m_valueRef;
};

class ChildMaterialMorphRefObject : public QObject
{
    Q_OBJECT
    Q_ENUMS(OperationType)
    Q_PROPERTY(MorphRefObject *parentMorph READ parentMorph CONSTANT FINAL)
    Q_PROPERTY(QString name READ name CONSTANT FINAL)
    Q_PROPERTY(int index READ index CONSTANT FINAL)
    Q_PROPERTY(QColor ambient READ ambient WRITE setAmbient NOTIFY ambientChanged FINAL)
    Q_PROPERTY(QColor diffuse READ diffuse WRITE setDiffuse NOTIFY diffuseChanged FINAL)
    Q_PROPERTY(QColor specular READ specular WRITE setSpecular NOTIFY specularChanged FINAL)
    Q_PROPERTY(QColor edgeColor READ edgeColor WRITE setEdgeColor NOTIFY edgeColorChanged FINAL)
    Q_PROPERTY(QVector4D mainTextureCoefficient READ mainTextureCoefficient WRITE setMainTextureCoefficient NOTIFY mainTextureCoefficientChanged FINAL)
    Q_PROPERTY(QVector4D sphereTextureCoefficient READ sphereTextureCoefficient WRITE setSphereTextureCoefficient NOTIFY sphereTextureCoefficientChanged FINAL)
    Q_PROPERTY(QVector4D toonTextureCoefficient READ toonTextureCoefficient WRITE setToonTextureCoefficient NOTIFY toonTextureCoefficientChanged FINAL)
    Q_PROPERTY(qreal shininess READ shininess WRITE setShininess NOTIFY shininessChanged FINAL)
    Q_PROPERTY(qreal edgeSize READ edgeSize WRITE setEdgeSize NOTIFY edgeSizeChanged FINAL)
    Q_PROPERTY(OperationType operation READ operation WRITE setOperation NOTIFY operationChanged FINAL)

public:
    enum OperationType {
        Multiply,
        Additive
    };

    ChildMaterialMorphRefObject(MorphRefObject *parentMorphRef, vpvl2::IMorph::Material *valueRef);
    ~ChildMaterialMorphRefObject();

    MorphRefObject *parentMorph() const;
    QString name() const;
    int index() const;
    QColor ambient() const;
    void setAmbient(const QColor &value);
    QColor diffuse() const;
    void setDiffuse(const QColor &value);
    QColor specular() const;
    void setSpecular(const QColor &value);
    QColor edgeColor() const;
    void setEdgeColor(const QColor &value);
    QVector4D mainTextureCoefficient() const;
    void setMainTextureCoefficient(const QVector4D &value);
    QVector4D sphereTextureCoefficient() const;
    void setSphereTextureCoefficient(const QVector4D &value);
    QVector4D toonTextureCoefficient() const;
    void setToonTextureCoefficient(const QVector4D &value);
    qreal shininess() const;
    void setShininess(const qreal &value);
    qreal edgeSize() const;
    void setEdgeSize(const qreal &value);
    OperationType operation() const;
    void setOperation(const OperationType &value);

signals:
    void ambientChanged();
    void diffuseChanged();
    void specularChanged();
    void edgeColorChanged();
    void mainTextureCoefficientChanged();
    void sphereTextureCoefficientChanged();
    void toonTextureCoefficientChanged();
    void shininessChanged();
    void edgeSizeChanged();
    void operationChanged();

private:
    MorphRefObject *m_parentMorphRef;
    vpvl2::IMorph::Material *m_valueRef;
};

class ChildFlipMorphRefObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(MorphRefObject *parentMorph READ parentMorph CONSTANT FINAL)
    Q_PROPERTY(QString name READ name CONSTANT FINAL)
    Q_PROPERTY(int index READ index CONSTANT FINAL)
    Q_PROPERTY(MorphRefObject *targetMorph READ targetMorph WRITE setTargetMorph NOTIFY targetMorphChanged FINAL)
    Q_PROPERTY(qreal fixedWeight READ fixedWeight WRITE setFixedWeight NOTIFY fixedWeightChanged FINAL)

public:
    ChildFlipMorphRefObject(MorphRefObject *parentMorphRef, vpvl2::IMorph::Flip *valueRef);
    ~ChildFlipMorphRefObject();

    MorphRefObject *parentMorph() const;
    QString name() const;
    int index() const;
    MorphRefObject *targetMorph() const;
    void setTargetMorph(MorphRefObject *value);
    qreal fixedWeight() const;
    void setFixedWeight(const qreal &value);

signals:
    void targetMorphChanged();
    void fixedWeightChanged();

private:
    MorphRefObject *m_parentMorphRef;
    vpvl2::IMorph::Flip *m_valueRef;
};

class ChildImpulseMorphRefObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(MorphRefObject *parentMorph READ parentMorph CONSTANT FINAL)
    Q_PROPERTY(int index READ index CONSTANT FINAL)
    Q_PROPERTY(RigidBodyRefObject *targetRigidBody READ targetRigidBody WRITE setTargetRigidBody NOTIFY targetRigidBodyChanged FINAL)
    Q_PROPERTY(QVector3D velocity READ velocity WRITE setVelocity NOTIFY velocityChanged FINAL)
    Q_PROPERTY(QVector3D torque READ torque WRITE setTorque NOTIFY torqueChanged FINAL)
    Q_PROPERTY(bool local READ isLocal WRITE setLocal NOTIFY localChanged FINAL)

public:
    ChildImpulseMorphRefObject(MorphRefObject *parentMorphRef, vpvl2::IMorph::Impulse *valueRef);
    ~ChildImpulseMorphRefObject();

    MorphRefObject *parentMorph() const;
    QString name() const;
    int index() const;
    RigidBodyRefObject *targetRigidBody() const;
    void setTargetRigidBody(RigidBodyRefObject *value);
    QVector3D velocity() const;
    void setVelocity(const QVector3D &value);
    QVector3D torque() const;
    void setTorque(const QVector3D &value);
    bool isLocal() const;
    void setLocal(bool value);

signals:
    void targetRigidBodyChanged();
    void velocityChanged();
    void torqueChanged();
    void localChanged();

private:
    MorphRefObject *m_parentMorphRef;
    vpvl2::IMorph::Impulse *m_valueRef;
};

class MorphRefObject : public QObject
{
    Q_OBJECT
    Q_ENUMS(Category)
    Q_ENUMS(Type)
    Q_PROPERTY(ModelProxy *parentModel READ parentModel CONSTANT FINAL)
    Q_PROPERTY(LabelRefObject *parentLabel READ parentLabel CONSTANT FINAL)
    Q_PROPERTY(QQmlListProperty<ChildGroupMorphRefObject> groups READ groups NOTIFY groupsChanged FINAL)
    Q_PROPERTY(QQmlListProperty<ChildVertexMorphRefObject> vertices READ vertices NOTIFY verticesChanged FINAL)
    Q_PROPERTY(QQmlListProperty<ChildBoneMorphRefObject> bones READ bones NOTIFY bonesChanged FINAL)
    Q_PROPERTY(QQmlListProperty<ChildUVMorphRefObject> uvs READ uvs NOTIFY uvsChanged FINAL)
    Q_PROPERTY(QQmlListProperty<ChildMaterialMorphRefObject> materials READ materials NOTIFY materialsChanged FINAL)
    Q_PROPERTY(QQmlListProperty<ChildFlipMorphRefObject> flips READ flips NOTIFY flipsChanged)
    Q_PROPERTY(QQmlListProperty<ChildImpulseMorphRefObject> impluses READ impluses NOTIFY implusesChanged FINAL)
    Q_PROPERTY(QUuid uuid READ uuid CONSTANT FINAL)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)
    Q_PROPERTY(Category category READ category WRITE setCategory NOTIFY categoryChanged FINAL)
    Q_PROPERTY(Type type READ type WRITE setType NOTIFY typeChanged FINAL)
    Q_PROPERTY(int index READ index CONSTANT FINAL)
    Q_PROPERTY(qreal weight READ weight WRITE setWeight NOTIFY weightChanged FINAL)
    Q_PROPERTY(qreal originWeight READ originWeight NOTIFY originWeightChanged FINAL)
    Q_PROPERTY(bool dirty READ isDirty NOTIFY dirtyChanged FINAL)

public:
    enum Category {
        Unknown = vpvl2::IMorph::kBase,
        Eye     = vpvl2::IMorph::kEye,
        Lip     = vpvl2::IMorph::kLip,
        Eyeblow = vpvl2::IMorph::kEyeblow,
        Other   = vpvl2::IMorph::kOther
    };
    enum Type {
        Group    = vpvl2::IMorph::kGroupMorph,
        Vertex   = vpvl2::IMorph::kVertexMorph,
        Bone     = vpvl2::IMorph::kBoneMorph,
        Texcoord = vpvl2::IMorph::kTexCoordMorph,
        UVA1     = vpvl2::IMorph::kUVA1Morph,
        UVA2     = vpvl2::IMorph::kUVA2Morph,
        UVA3     = vpvl2::IMorph::kUVA3Morph,
        UVA4     = vpvl2::IMorph::kUVA4Morph,
        Material = vpvl2::IMorph::kMaterialMorph,
        Flip     = vpvl2::IMorph::kFlipMorph,
        Impulse  = vpvl2::IMorph::kImpulseMorph
    };

    MorphRefObject(ModelProxy *modelRef, LabelRefObject *labelRef, vpvl2::IMorph *morphRef, const QUuid &uuid);
    ~MorphRefObject();

    void setOriginWeight(const qreal &value);

    vpvl2::IMorph *data() const;
    ModelProxy *parentModel() const;
    LabelRefObject *parentLabel() const;
    QQmlListProperty<ChildGroupMorphRefObject> groups();
    QQmlListProperty<ChildVertexMorphRefObject> vertices();
    QQmlListProperty<ChildBoneMorphRefObject> bones();
    QQmlListProperty<ChildUVMorphRefObject> uvs();
    QQmlListProperty<ChildMaterialMorphRefObject> materials();
    QQmlListProperty<ChildFlipMorphRefObject> flips();
    QQmlListProperty<ChildImpulseMorphRefObject> impluses();
    QUuid uuid() const;
    QString name() const;
    void setName(const QString &value);
    Category category() const;
    void setCategory(const Category &value);
    Type type() const;
    void setType(const Type &value);
    int index() const;
    qreal weight() const;
    void setWeight(const qreal &value);
    qreal originWeight() const;
    bool isDirty() const;
    void setDirty(bool value);

public slots:
    Q_INVOKABLE void sync();

signals:
    void groupsChanged();
    void verticesChanged();
    void bonesChanged();
    void uvsChanged();
    void materialsChanged();
    void flipsChanged();
    void implusesChanged();
    void nameChanged();
    void categoryChanged();
    void typeChanged();
    void weightChanged();
    void originWeightChanged();
    void morphDidSync();
    void dirtyChanged();

private:
    QList<ChildGroupMorphRefObject *> m_groupMorphs;
    QList<ChildVertexMorphRefObject *> m_vertexMorphs;
    QList<ChildBoneMorphRefObject *> m_boneMorphs;
    QList<ChildUVMorphRefObject *> m_uvsMorphs;
    QList<ChildMaterialMorphRefObject *> m_materialMorphs;
    QList<ChildFlipMorphRefObject *> m_flipMorphs;
    QList<ChildImpulseMorphRefObject *> m_impulseMorphs;
    ModelProxy *m_parentModelRef;
    LabelRefObject *m_parentLabelRef;
    vpvl2::IMorph *m_morphRef;
    const QUuid m_uuid;
    qreal m_originWeight;
    bool m_dirty;
};

#endif // MORPHREFOBJECT_H
