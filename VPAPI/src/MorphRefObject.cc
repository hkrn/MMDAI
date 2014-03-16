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

#include "ModelProxy.h"
#include "MorphRefObject.h"

#include <vpvl2/vpvl2.h>
#include <vpvl2/extensions/qt/String.h>
#include <QtCore>

#include "BoneRefObject.h"
#include "LabelRefObject.h"
#include "MaterialRefObject.h"
#include "RigidBodyRefObject.h"
#include "VertexRefObject.h"
#include "Util.h"

using namespace vpvl2;
using namespace vpvl2::extensions::qt;

ChildGroupMorphRefObject::ChildGroupMorphRefObject(MorphRefObject *parent, IMorph::Group *valueRef)
    : m_parentMorphRef(parent),
      m_valueRef(valueRef)
{
    Q_ASSERT(m_parentMorphRef);
    Q_ASSERT(m_valueRef);
}

ChildGroupMorphRefObject::~ChildGroupMorphRefObject()
{
    m_parentMorphRef = 0;
    m_valueRef = 0;
}

QJsonValue ChildGroupMorphRefObject::toJson() const
{
    QJsonObject v;
    v.insert("targetMorph", targetMorph()->uuid().toString());
    v.insert("fixedWeight", fixedWeight());
    return v;
}

MorphRefObject *ChildGroupMorphRefObject::parentMorph() const
{
    return m_parentMorphRef;
}

QString ChildGroupMorphRefObject::name() const
{
    return targetMorph()->name();
}

int ChildGroupMorphRefObject::index() const
{
    Q_ASSERT(m_valueRef);
    return m_valueRef->index;
}

MorphRefObject *ChildGroupMorphRefObject::targetMorph() const
{
    Q_ASSERT(m_parentMorphRef);
    Q_ASSERT(m_valueRef);
    return m_parentMorphRef->parentModel()->resolveMorphRef(m_valueRef->morph);
}

void ChildGroupMorphRefObject::setTargetMorph(MorphRefObject *value)
{
    Q_ASSERT(value);
    Q_ASSERT(m_valueRef);
    if (targetMorph() != value) {
        m_valueRef->morph = value->data();
        m_parentMorphRef->setDirty(true);
        emit targetMorphChanged();
    }
}

qreal ChildGroupMorphRefObject::fixedWeight() const
{
    Q_ASSERT(m_valueRef);
    return m_valueRef->fixedWeight;
}

void ChildGroupMorphRefObject::setFixedWeight(const qreal &value)
{
    Q_ASSERT(m_valueRef);
    if (!qFuzzyCompare(fixedWeight(), value)) {
        m_valueRef->fixedWeight = value;
        m_parentMorphRef->setDirty(true);
        emit fixedWeightChanged();
    }
}

ChildVertexMorphRefObject::ChildVertexMorphRefObject(MorphRefObject *parentMorphRef, IMorph::Vertex *valueRef)
    : m_parentMorphRef(parentMorphRef),
      m_valueRef(valueRef)
{
    Q_ASSERT(m_parentMorphRef);
    Q_ASSERT(m_valueRef);
}

ChildVertexMorphRefObject::~ChildVertexMorphRefObject()
{
    m_parentMorphRef = 0;
    m_valueRef = 0;
}

QJsonValue ChildVertexMorphRefObject::toJson() const
{
    QJsonObject v;
    v.insert("targetVertex", targetVertex()->uuid().toString());
    v.insert("position", Util::toJson(position()));
    return v;
}

MorphRefObject *ChildVertexMorphRefObject::parentMorph() const
{
    return m_parentMorphRef;
}

QString ChildVertexMorphRefObject::name() const
{
    return targetVertex()->name();
}

int ChildVertexMorphRefObject::index() const
{
    Q_ASSERT(m_valueRef);
    return m_valueRef->index;
}

VertexRefObject *ChildVertexMorphRefObject::targetVertex() const
{
    Q_ASSERT(m_parentMorphRef);
    Q_ASSERT(m_valueRef);
    return m_parentMorphRef->parentModel()->resolveVertexRef(m_valueRef->vertex);
}

void ChildVertexMorphRefObject::setTargetVertex(VertexRefObject *value)
{
    Q_ASSERT(value);
    Q_ASSERT(m_valueRef);
    if (targetVertex() != value) {
        m_valueRef->vertex = value->data();
        m_parentMorphRef->setDirty(true);
        emit targetVertexChanged();
    }
}

QVector3D ChildVertexMorphRefObject::position() const
{
    return Util::fromVector3(m_valueRef->position);
}

void ChildVertexMorphRefObject::setPosition(const QVector3D &value)
{
    if (!qFuzzyCompare(position(), value)) {
        m_valueRef->position = Util::toVector3(value);
        m_parentMorphRef->setDirty(true);
        emit positionChanged();
    }
}

ChildBoneMorphRefObject::ChildBoneMorphRefObject(MorphRefObject *parentMorphRef, IMorph::Bone *valueRef)
    : m_parentMorphRef(parentMorphRef),
      m_valueRef(valueRef)
{
    Q_ASSERT(m_parentMorphRef);
    Q_ASSERT(m_valueRef);
}

ChildBoneMorphRefObject::~ChildBoneMorphRefObject()
{
    m_parentMorphRef = 0;
    m_valueRef = 0;
}

QJsonValue ChildBoneMorphRefObject::toJson() const
{
    QJsonObject v;
    v.insert("targetBone", targetBone()->uuid().toString());
    v.insert("position", Util::toJson(position()));
    v.insert("rotation", Util::toJson(rotation()));
    return v;
}

MorphRefObject *ChildBoneMorphRefObject::parentMorph() const
{
    return m_parentMorphRef;
}

QString ChildBoneMorphRefObject::name() const
{
    return targetBone()->name();
}

int ChildBoneMorphRefObject::index() const
{
    Q_ASSERT(m_valueRef);
    return m_valueRef->index;
}

BoneRefObject *ChildBoneMorphRefObject::targetBone() const
{
    Q_ASSERT(m_parentMorphRef);
    Q_ASSERT(m_valueRef);
    return m_parentMorphRef->parentModel()->resolveBoneRef(m_valueRef->bone);
}

void ChildBoneMorphRefObject::setTargetBone(BoneRefObject *value)
{
    Q_ASSERT(value);
    Q_ASSERT(m_valueRef);
    if (targetBone() != value) {
        m_valueRef->bone = value->data();
        m_parentMorphRef->setDirty(true);
        emit targetBoneChanged();
    }
}

QVector3D ChildBoneMorphRefObject::position() const
{
    Q_ASSERT(m_valueRef);
    return Util::fromVector3(m_valueRef->position);
}

void ChildBoneMorphRefObject::setPosition(const QVector3D &value)
{
    Q_ASSERT(m_valueRef);
    if (!qFuzzyCompare(position(), value)) {
        m_valueRef->position = Util::toVector3(value);
        m_parentMorphRef->setDirty(true);
        emit positionChanged();
    }
}

QQuaternion ChildBoneMorphRefObject::rotation() const
{
    Q_ASSERT(m_valueRef);
    return Util::fromQuaternion(m_valueRef->rotation);
}

void ChildBoneMorphRefObject::setRotation(const QQuaternion &value)
{
    Q_ASSERT(m_valueRef);
    if (!qFuzzyCompare(rotation(), value)) {
        m_valueRef->rotation = Util::toQuaternion(value);
        m_parentMorphRef->setDirty(true);
        emit rotationChanged();
    }
}

ChildUVMorphRefObject::ChildUVMorphRefObject(MorphRefObject *parentMorphRef, IMorph::UV *valueRef)
    : m_parentMorphRef(parentMorphRef),
      m_valueRef(valueRef)
{
    Q_ASSERT(m_parentMorphRef);
    Q_ASSERT(m_valueRef);
}

ChildUVMorphRefObject::~ChildUVMorphRefObject()
{
    m_parentMorphRef = 0;
    m_valueRef = 0;
}

QJsonValue ChildUVMorphRefObject::toJson() const
{
    QJsonObject v;
    v.insert("targetVertex", targetVertex()->uuid().toString());
    v.insert("position", Util::toJson(position()));
    return v;
}

MorphRefObject *ChildUVMorphRefObject::parentMorph() const
{
    return m_parentMorphRef;
}

QString ChildUVMorphRefObject::name() const
{
    return targetVertex()->name();
}

int ChildUVMorphRefObject::index() const
{
    Q_ASSERT(m_valueRef);
    return m_valueRef->index;
}

VertexRefObject *ChildUVMorphRefObject::targetVertex() const
{
    Q_ASSERT(m_parentMorphRef);
    Q_ASSERT(m_valueRef);
    return m_parentMorphRef->parentModel()->resolveVertexRef(m_valueRef->vertex);
}

void ChildUVMorphRefObject::setTargetVertex(VertexRefObject *value)
{
    Q_ASSERT(m_parentMorphRef);
    Q_ASSERT(m_valueRef);
    if (targetVertex() != value) {
        m_valueRef->vertex = value->data();
        m_parentMorphRef->setDirty(true);
        emit targetVertexChanged();
    }
}

QVector4D ChildUVMorphRefObject::position() const
{
    Q_ASSERT(m_valueRef);
    return Util::fromVector4(m_valueRef->position);
}

void ChildUVMorphRefObject::setPosition(const QVector4D &value)
{
    Q_ASSERT(m_valueRef);
    if (position() != value) {
        m_valueRef->position = Util::toVector4(value);
        m_parentMorphRef->setDirty(true);
        emit positionChanged();
    }
}

ChildMaterialMorphRefObject::ChildMaterialMorphRefObject(MorphRefObject *parentMorphRef, IMorph::Material *valueRef)
    : m_parentMorphRef(parentMorphRef),
      m_valueRef(valueRef)
{
    Q_ASSERT(m_parentMorphRef);
    Q_ASSERT(m_valueRef);
}

ChildMaterialMorphRefObject::~ChildMaterialMorphRefObject()
{
    m_parentMorphRef = 0;
    m_valueRef = 0;
}

QJsonValue ChildMaterialMorphRefObject::toJson() const
{
    QJsonObject v;
    v.insert("ambient", Util::toJson(ambient()));
    v.insert("diffuse", Util::toJson(diffuse()));
    v.insert("specualr", Util::toJson(specular()));
    v.insert("edgeColor", Util::toJson(edgeColor()));
    v.insert("mainTextureCoefficient", Util::toJson(mainTextureCoefficient()));
    v.insert("sphereTextureCoefficient", Util::toJson(sphereTextureCoefficient()));
    v.insert("toonTextureCoefficient", Util::toJson(toonTextureCoefficient()));
    v.insert("shininess", shininess());
    v.insert("edgeSize", edgeSize());
    v.insert("operation", operation());
    return v;
}

MorphRefObject *ChildMaterialMorphRefObject::parentMorph() const
{
    return m_parentMorphRef;
}

QString ChildMaterialMorphRefObject::name() const
{
    /* FIXME: implement this */
    return QString();
}

int ChildMaterialMorphRefObject::index() const
{
    Q_ASSERT(m_valueRef);
    return m_valueRef->index;
}

QColor ChildMaterialMorphRefObject::ambient() const
{
    Q_ASSERT(m_valueRef);
    return Util::fromColorRGB(m_valueRef->ambient);
}

void ChildMaterialMorphRefObject::setAmbient(const QColor &value)
{
    Q_ASSERT(m_valueRef);
    if (ambient() != value) {
        m_valueRef->ambient = Util::toColorRGB(value);
        m_parentMorphRef->setDirty(true);
        emit ambientChanged();
    }
}

QColor ChildMaterialMorphRefObject::diffuse() const
{
    Q_ASSERT(m_valueRef);
    return Util::fromColorRGBA(m_valueRef->diffuse);
}

void ChildMaterialMorphRefObject::setDiffuse(const QColor &value)
{
    Q_ASSERT(m_valueRef);
    if (diffuse() != value) {
        m_valueRef->diffuse = Util::toColorRGBA(value);
        m_parentMorphRef->setDirty(true);
        emit diffuseChanged();
    }
}

QColor ChildMaterialMorphRefObject::specular() const
{
    Q_ASSERT(m_valueRef);
    return Util::fromColorRGB(m_valueRef->specular);
}

void ChildMaterialMorphRefObject::setSpecular(const QColor &value)
{
    Q_ASSERT(m_valueRef);
    if (specular() != value) {
        m_valueRef->specular = Util::toColorRGB(value);
        m_parentMorphRef->setDirty(true);
        emit specularChanged();
    }
}

QColor ChildMaterialMorphRefObject::edgeColor() const
{
    Q_ASSERT(m_valueRef);
    return Util::fromColorRGBA(m_valueRef->edgeColor);
}

void ChildMaterialMorphRefObject::setEdgeColor(const QColor &value)
{
    Q_ASSERT(m_valueRef);
    if (edgeColor() != value) {
        m_valueRef->edgeColor = Util::toColorRGBA(value);
        m_parentMorphRef->setDirty(true);
        emit edgeColorChanged();
    }
}

QVector4D ChildMaterialMorphRefObject::mainTextureCoefficient() const
{
    Q_ASSERT(m_valueRef);
    return Util::fromVector4(m_valueRef->textureWeight);
}

void ChildMaterialMorphRefObject::setMainTextureCoefficient(const QVector4D &value)
{
    Q_ASSERT(m_valueRef);
    if (!qFuzzyCompare(mainTextureCoefficient(), value)) {
        m_valueRef->textureWeight = Util::toVector4(value);
        m_parentMorphRef->setDirty(true);
        emit mainTextureCoefficientChanged();
    }
}

QVector4D ChildMaterialMorphRefObject::sphereTextureCoefficient() const
{
    Q_ASSERT(m_valueRef);
    return Util::fromVector4(m_valueRef->sphereTextureWeight);
}

void ChildMaterialMorphRefObject::setSphereTextureCoefficient(const QVector4D &value)
{
    Q_ASSERT(m_valueRef);
    if (!qFuzzyCompare(sphereTextureCoefficient(), value)) {
        m_valueRef->sphereTextureWeight = Util::toVector4(value);
        m_parentMorphRef->setDirty(true);
        emit sphereTextureCoefficientChanged();
    }
}

QVector4D ChildMaterialMorphRefObject::toonTextureCoefficient() const
{
    Q_ASSERT(m_valueRef);
    return Util::fromVector4(m_valueRef->toonTextureWeight);
}

void ChildMaterialMorphRefObject::setToonTextureCoefficient(const QVector4D &value)
{
    Q_ASSERT(m_valueRef);
    if (!qFuzzyCompare(toonTextureCoefficient(), value)) {
        m_valueRef->toonTextureWeight = Util::toVector4(value);
        m_parentMorphRef->setDirty(true);
        emit toonTextureCoefficientChanged();
    }
}

qreal ChildMaterialMorphRefObject::shininess() const
{
    Q_ASSERT(m_valueRef);
    return m_valueRef->shininess;
}

void ChildMaterialMorphRefObject::setShininess(const qreal &value)
{
    Q_ASSERT(m_valueRef);
    if (!qFuzzyCompare(shininess(), value)) {
        m_valueRef->shininess = value;
        m_parentMorphRef->setDirty(true);
        emit shininessChanged();
    }
}

qreal ChildMaterialMorphRefObject::edgeSize() const
{
    Q_ASSERT(m_valueRef);
    return m_valueRef->edgeSize;
}

void ChildMaterialMorphRefObject::setEdgeSize(const qreal &value)
{
    Q_ASSERT(m_valueRef);
    if (!qFuzzyCompare(edgeSize(), value)) {
        m_valueRef->edgeSize = value;
        m_parentMorphRef->setDirty(true);
        emit edgeSizeChanged();
    }
}

ChildMaterialMorphRefObject::OperationType ChildMaterialMorphRefObject::operation() const
{
    Q_ASSERT(m_valueRef);
    return static_cast<OperationType>(m_valueRef->operation);
}

void ChildMaterialMorphRefObject::setOperation(const OperationType &value)
{
    Q_ASSERT(m_valueRef);
    if (operation() != value) {
        m_valueRef->operation = static_cast<uint8>(value);
        m_parentMorphRef->setDirty(true);
        emit operationChanged();
    }
}

ChildFlipMorphRefObject::ChildFlipMorphRefObject(MorphRefObject *parentMorphRef, IMorph::Flip *valueRef)
    : m_parentMorphRef(parentMorphRef),
      m_valueRef(valueRef)
{
    Q_ASSERT(m_parentMorphRef);
    Q_ASSERT(m_valueRef);
}

ChildFlipMorphRefObject::~ChildFlipMorphRefObject()
{
    m_parentMorphRef = 0;
    m_valueRef = 0;
}

QJsonValue ChildFlipMorphRefObject::toJson() const
{
    QJsonObject v;
    v.insert("targetMorph", targetMorph()->uuid().toString());
    v.insert("fixedWeight", fixedWeight());
    return v;
}

MorphRefObject *ChildFlipMorphRefObject::parentMorph() const
{
    return m_parentMorphRef;
}

QString ChildFlipMorphRefObject::name() const
{
    return targetMorph()->name();
}

int ChildFlipMorphRefObject::index() const
{
    Q_ASSERT(m_valueRef);
    return m_valueRef->index;
}

MorphRefObject *ChildFlipMorphRefObject::targetMorph() const
{
    Q_ASSERT(m_parentMorphRef);
    Q_ASSERT(m_valueRef);
    return m_parentMorphRef->parentModel()->resolveMorphRef(m_valueRef->morph);
}

void ChildFlipMorphRefObject::setTargetMorph(MorphRefObject *value)
{
    Q_ASSERT(value);
    Q_ASSERT(m_valueRef);
    if (targetMorph() != value) {
        m_valueRef->morph = value->data();
        m_parentMorphRef->setDirty(true);
        emit targetMorphChanged();
    }
}

qreal ChildFlipMorphRefObject::fixedWeight() const
{
    Q_ASSERT(m_valueRef);
    return m_valueRef->fixedWeight;
}

void ChildFlipMorphRefObject::setFixedWeight(const qreal &value)
{
    Q_ASSERT(m_valueRef);
    if (!qFuzzyCompare(fixedWeight(), value)) {
        m_valueRef->fixedWeight = value;
        m_parentMorphRef->setDirty(true);
        emit fixedWeightChanged();
    }
}

ChildImpulseMorphRefObject::ChildImpulseMorphRefObject(MorphRefObject *parentMorphRef, IMorph::Impulse *valueRef)
    : m_parentMorphRef(parentMorphRef),
      m_valueRef(valueRef)
{
    Q_ASSERT(m_parentMorphRef);
    Q_ASSERT(m_valueRef);
}

ChildImpulseMorphRefObject::~ChildImpulseMorphRefObject()
{
    m_parentMorphRef = 0;
    m_valueRef = 0;
}

QJsonValue ChildImpulseMorphRefObject::toJson() const
{
    QJsonObject v;
    v.insert("targetRigidBody", targetRigidBody()->uuid().toString());
    v.insert("velocity", Util::toJson(velocity()));
    v.insert("torque", Util::toJson(torque()));
    v.insert("local", isLocal());
    return v;
}

MorphRefObject *ChildImpulseMorphRefObject::parentMorph() const
{
    return m_parentMorphRef;
}

QString ChildImpulseMorphRefObject::name() const
{
    return targetRigidBody()->name();
}

int ChildImpulseMorphRefObject::index() const
{
    Q_ASSERT(m_valueRef);
    return m_valueRef->index;
}

RigidBodyRefObject *ChildImpulseMorphRefObject::targetRigidBody() const
{
    Q_ASSERT(m_parentMorphRef);
    Q_ASSERT(m_valueRef);
    return m_parentMorphRef->parentModel()->resolveRigidBodyRef(m_valueRef->rigidBody);
}

void ChildImpulseMorphRefObject::setTargetRigidBody(RigidBodyRefObject *value)
{
    Q_ASSERT(m_valueRef);
    if (targetRigidBody() != value) {
        m_valueRef->rigidBody = value->data();
        m_parentMorphRef->setDirty(true);
        emit targetRigidBodyChanged();
    }
}

QVector3D ChildImpulseMorphRefObject::velocity() const
{
    Q_ASSERT(m_valueRef);
    return Util::fromVector3(m_valueRef->velocity);
}

void ChildImpulseMorphRefObject::setVelocity(const QVector3D &value)
{
    Q_ASSERT(m_valueRef);
    if (!qFuzzyCompare(velocity(), value)) {
        m_valueRef->velocity = Util::toVector3(value);
        m_parentMorphRef->setDirty(true);
        emit velocityChanged();
    }
}

QVector3D ChildImpulseMorphRefObject::torque() const
{
    Q_ASSERT(m_valueRef);
    return Util::fromVector3(m_valueRef->velocity);
}

void ChildImpulseMorphRefObject::setTorque(const QVector3D &value)
{
    Q_ASSERT(m_valueRef);
    if (!qFuzzyCompare(torque(), value)) {
        m_valueRef->torque = Util::toVector3(value);
        m_parentMorphRef->setDirty(true);
        emit torqueChanged();
    }
}

bool ChildImpulseMorphRefObject::isLocal() const
{
    Q_ASSERT(m_valueRef);
    return m_valueRef->isLocal;
}

void ChildImpulseMorphRefObject::setLocal(bool value)
{
    Q_ASSERT(m_valueRef);
    if (isLocal() != value) {
        m_valueRef->isLocal = value;
        m_parentMorphRef->setDirty(true);
        emit localChanged();
    }
}

MorphRefObject::MorphRefObject(ModelProxy *modelRef, IMorph *morphRef, const QUuid &uuid)
    : QObject(modelRef),
      m_parentModelRef(modelRef),
      m_parentLabelRef(0),
      m_morphRef(morphRef),
      m_uuid(uuid),
      m_originWeight(0),
      m_dirty(false)
{
    Q_ASSERT(m_parentModelRef);
    Q_ASSERT(m_morphRef);
    Q_ASSERT(!m_uuid.isNull());
    connect(m_parentModelRef, &ModelProxy::languageChanged, this, &MorphRefObject::nameChanged);
    connect(this, &MorphRefObject::morphDidSync, this, &MorphRefObject::weightChanged);
}

MorphRefObject::~MorphRefObject()
{
    qDeleteAll(m_groupMorphs);
    m_groupMorphs.clear();
    qDeleteAll(m_vertexMorphs);
    m_vertexMorphs.clear();
    qDeleteAll(m_boneMorphs);
    m_boneMorphs.clear();
    qDeleteAll(m_uvsMorphs);
    m_uvsMorphs.clear();
    qDeleteAll(m_materialMorphs);
    m_materialMorphs.clear();
    qDeleteAll(m_flipMorphs);
    m_flipMorphs.clear();
    qDeleteAll(m_impulseMorphs);
    m_impulseMorphs.clear();
    m_parentLabelRef = 0;
    m_morphRef = 0;
    m_originWeight = 0;
}

void MorphRefObject::initialize()
{
    initializeAllGroupMorphs();
    initializeAllVertexMorphs();
    initializeAllMaterialMorphs();
    initializeAllBoneMorphs();
    initializeAllUVMorphs();
    initializeAllFlipMorphs();
    initializeAllImpulseMorphs();
}

void MorphRefObject::setOriginWeight(const qreal &value)
{
    if (!qFuzzyCompare(value, m_originWeight)) {
        m_originWeight = value;
        setDirty(true);
        emit originWeightChanged();
    }
}

QJsonValue MorphRefObject::toJson() const
{
    QJsonObject v;
    v.insert("uuid", uuid().toString());
    v.insert("name", name());
    v.insert("category", category());
    v.insert("type", type());
    QJsonArray children;
    switch (type()) {
    case Group:
        foreach (ChildGroupMorphRefObject *item, m_groupMorphs) {
            children.append(item->toJson());
        }
        break;
    case Vertex:
        foreach (ChildVertexMorphRefObject *item, m_vertexMorphs) {
            children.append(item->toJson());
        }
        break;
    case Material:
        foreach (ChildMaterialMorphRefObject *item, m_materialMorphs) {
            children.append(item->toJson());
        }
        break;
    case Bone:
        foreach (ChildBoneMorphRefObject *item, m_boneMorphs) {
            children.append(item->toJson());
        }
        break;
    case Texcoord:
    case UVA1:
    case UVA2:
    case UVA3:
    case UVA4:
        foreach (ChildUVMorphRefObject *item, m_uvsMorphs) {
            children.append(item->toJson());
        }
        break;
    case Flip:
        foreach (ChildFlipMorphRefObject *item, m_flipMorphs) {
            children.append(item->toJson());
        }
        break;
    case Impulse:
        foreach (ChildImpulseMorphRefObject *item, m_impulseMorphs) {
            children.append(item->toJson());
        }
        break;
    default:
        break;
    }
    v.insert("children", children);
    return v;
}

vpvl2::IMorph *MorphRefObject::data() const
{
    return m_morphRef;
}

ModelProxy *MorphRefObject::parentModel() const
{
    return m_parentModelRef;
}

LabelRefObject *MorphRefObject::parentLabel() const
{
    return m_parentLabelRef;
}

void MorphRefObject::setParentLabel(LabelRefObject *value)
{
    if (parentLabel() != value) {
        m_parentLabelRef = value;
        emit parentLabelChanged();
    }
}

QQmlListProperty<ChildGroupMorphRefObject>MorphRefObject::groups()
{
    initializeAllGroupMorphs();
    return QQmlListProperty<ChildGroupMorphRefObject>(this, m_groupMorphs);
}

QQmlListProperty<ChildVertexMorphRefObject> MorphRefObject::vertices()
{
    initializeAllVertexMorphs();
    return QQmlListProperty<ChildVertexMorphRefObject>(this, m_vertexMorphs);
}

QQmlListProperty<ChildBoneMorphRefObject> MorphRefObject::bones()
{
    initializeAllBoneMorphs();
    return QQmlListProperty<ChildBoneMorphRefObject>(this, m_boneMorphs);
}

QQmlListProperty<ChildUVMorphRefObject> MorphRefObject::uvs()
{
    initializeAllUVMorphs();
    return QQmlListProperty<ChildUVMorphRefObject>(this, m_uvsMorphs);
}

QQmlListProperty<ChildMaterialMorphRefObject> MorphRefObject::materials()
{
    initializeAllMaterialMorphs();
    return QQmlListProperty<ChildMaterialMorphRefObject>(this, m_materialMorphs);
}

QQmlListProperty<ChildFlipMorphRefObject> MorphRefObject::flips()
{
    initializeAllFlipMorphs();
    return QQmlListProperty<ChildFlipMorphRefObject>(this, m_flipMorphs);
}

QQmlListProperty<ChildImpulseMorphRefObject> MorphRefObject::impluses()
{
    initializeAllImpulseMorphs();
    return QQmlListProperty<ChildImpulseMorphRefObject>(this, m_impulseMorphs);
}

QUuid MorphRefObject::uuid() const
{
    Q_ASSERT(!m_uuid.isNull());
    return m_uuid;
}

QString MorphRefObject::name() const
{
    Q_ASSERT(m_parentModelRef);
    Q_ASSERT(m_morphRef);
    ModelProxy *parentModel = m_parentModelRef;
    IEncoding::LanguageType language = static_cast<IEncoding::LanguageType>(parentModel->language());
    const IString *name = m_morphRef->name(language);
    return Util::toQString((name && name->size() > 0) ? name : m_morphRef->name(IEncoding::kDefaultLanguage));
}

void MorphRefObject::setName(const QString &value)
{
    Q_ASSERT(m_parentModelRef);
    Q_ASSERT(m_morphRef);
    if (name() != value) {
        ModelProxy *parentModel = m_parentModelRef;
        IEncoding::LanguageType language = static_cast<IEncoding::LanguageType>(parentModel->language());
        QScopedPointer<IString> s(String::create(value.toStdString()));
        m_parentModelRef->renameObject(this, value);
        m_morphRef->setName(s.data(), language);
        setDirty(true);
        emit nameChanged();
    }
}

MorphRefObject::Category MorphRefObject::category() const
{
    Q_ASSERT(m_morphRef);
    return static_cast<Category>(m_morphRef->category());
}

void MorphRefObject::setCategory(const Category &value)
{
    Q_ASSERT(m_parentModelRef);
    Q_ASSERT(m_morphRef);
    if (category() != value) {
        m_morphRef->setCategory(static_cast<IMorph::Category>(value));
        setDirty(true);
    }
}

MorphRefObject::Type MorphRefObject::type() const
{
    Q_ASSERT(m_morphRef);
    return static_cast<Type>(m_morphRef->type());
}

void MorphRefObject::setType(const Type &value)
{
    Q_ASSERT(m_parentModelRef);
    Q_ASSERT(m_morphRef);
    if (type() != value) {
        m_morphRef->setType(static_cast<IMorph::Type>(value));
        setDirty(true);
    }
}

int MorphRefObject::index() const
{
    Q_ASSERT(m_morphRef);
    return m_morphRef->index();
}

qreal MorphRefObject::weight() const
{
    Q_ASSERT(m_morphRef);
    return m_morphRef->weight();
}

void MorphRefObject::setWeight(const qreal &value)
{
    Q_ASSERT(m_morphRef);
    if (!qFuzzyCompare(value, weight())) {
        m_morphRef->setWeight(static_cast<IMorph::WeightPrecision>(value));
        emit weightChanged();
    }
}

qreal MorphRefObject::originWeight() const
{
    return m_originWeight;
}

void MorphRefObject::sync()
{
    emit morphDidSync();
}

bool MorphRefObject::isDirty() const
{
    return m_dirty;
}

void MorphRefObject::setDirty(bool value)
{
    if (isDirty() != value) {
        m_dirty = value;
        emit dirtyChanged();
        if (value) {
            m_parentModelRef->markDirty();
        }
    }
}

void MorphRefObject::initializeAllGroupMorphs()
{
    if (m_groupMorphs.isEmpty()) {
        Array<IMorph::Group *> values;
        m_morphRef->getGroupMorphs(values);
        const int nvalues = values.count();
        for (int i = 0; i < nvalues; i++) {
            IMorph::Group *value = values[i];
            m_groupMorphs.append(new ChildGroupMorphRefObject(this, value));
        }
        qSort(m_groupMorphs.begin(), m_groupMorphs.end(), Util::LessThan());
    }
}

void MorphRefObject::initializeAllVertexMorphs()
{
    if (m_vertexMorphs.isEmpty()) {
        Array<IMorph::Vertex *> values;
        m_morphRef->getVertexMorphs(values);
        const int nvalues = values.count();
        for (int i = 0; i < nvalues; i++) {
            IMorph::Vertex *value = values[i];
            m_vertexMorphs.append(new ChildVertexMorphRefObject(this, value));
        }
        qSort(m_vertexMorphs.begin(), m_vertexMorphs.end(), Util::LessThan());
    }
}

void MorphRefObject::initializeAllBoneMorphs()
{
    if (m_boneMorphs.isEmpty()) {
        Array<IMorph::Bone *> values;
        m_morphRef->getBoneMorphs(values);
        const int nvalues = values.count();
        for (int i = 0; i < nvalues; i++) {
            IMorph::Bone *value = values[i];
            m_boneMorphs.append(new ChildBoneMorphRefObject(this, value));
        }
        qSort(m_boneMorphs.begin(), m_boneMorphs.end(), Util::LessThan());
    }
}

void MorphRefObject::initializeAllUVMorphs()
{
    if (m_uvsMorphs.isEmpty()) {
        Array<IMorph::UV *> values;
        m_morphRef->getUVMorphs(values);
        const int nvalues = values.count();
        for (int i = 0; i < nvalues; i++) {
            IMorph::UV *value = values[i];
            m_uvsMorphs.append(new ChildUVMorphRefObject(this, value));
        }
        qSort(m_uvsMorphs.begin(), m_uvsMorphs.end(), Util::LessThan());
    }
}

void MorphRefObject::initializeAllMaterialMorphs()
{
    if (m_materialMorphs.isEmpty()) {
        Array<IMorph::Material *> values;
        m_morphRef->getMaterialMorphs(values);
        const int nvalues = values.count();
        for (int i = 0; i < nvalues; i++) {
            IMorph::Material *value = values[i];
            m_materialMorphs.append(new ChildMaterialMorphRefObject(this, value));
        }
        qSort(m_materialMorphs.begin(), m_materialMorphs.end(), Util::LessThan());
    }
}

void MorphRefObject::initializeAllFlipMorphs()
{
    if (m_flipMorphs.isEmpty()) {
        Array<IMorph::Flip *> values;
        m_morphRef->getFlipMorphs(values);
        const int nvalues = values.count();
        for (int i = 0; i < nvalues; i++) {
            IMorph::Flip *value = values[i];
            m_flipMorphs.append(new ChildFlipMorphRefObject(this, value));
        }
        qSort(m_flipMorphs.begin(), m_flipMorphs.end(), Util::LessThan());
    }
}

void MorphRefObject::initializeAllImpulseMorphs()
{
    if (m_impulseMorphs.isEmpty()) {
        Array<IMorph::Impulse *> values;
        m_morphRef->getImpulseMorphs(values);
        const int nvalues = values.count();
        for (int i = 0; i < nvalues; i++) {
            IMorph::Impulse *value = values[i];
            m_impulseMorphs.append(new ChildImpulseMorphRefObject(this, value));
        }
        qSort(m_impulseMorphs.begin(), m_impulseMorphs.end(), Util::LessThan());
    }
}
