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

#include "MaterialRefObject.h"
#include "ModelProxy.h"
#include "Util.h"

#include <QtCore>
#include <vpvl2/vpvl2.h>
#include <vpvl2/extensions/qt/String.h>

using namespace vpvl2;
using namespace vpvl2::extensions::qt;

MaterialRefObject::MaterialRefObject(ModelProxy *parentModelRef,
                                     vpvl2::IMaterial *materialRef,
                                     const QUuid &uuid)
    : m_parentModelRef(parentModelRef),
      m_materialRef(materialRef),
      m_uuid(uuid),
      m_dirty(false)
{
    Q_ASSERT(m_parentModelRef);
    Q_ASSERT(m_materialRef);
    Q_ASSERT(!m_uuid.isNull());
}

MaterialRefObject::~MaterialRefObject()
{
    m_parentModelRef = 0;
    m_materialRef = 0;
}

QJsonValue MaterialRefObject::toJson() const
{
    QJsonObject v;
    v.insert("uuid", uuid().toString());
    v.insert("name", name());
    v.insert("mainTexturePath", mainTexturePath());
    v.insert("sphereTexturePath", sphereTexturePath());
    v.insert("toonTexturePath", toonTexturePath());
    v.insert("userAreaData", userAreaData());
    v.insert("ambient", Util::toJson(ambient()));
    v.insert("diffuse", Util::toJson(diffuse()));
    v.insert("specular", Util::toJson(specular()));
    v.insert("edgeColor", Util::toJson(edgeColor()));
    v.insert("sphereTextureType", sphereTextureType());
    v.insert("shininess", shininess());
    v.insert("edgeSize", edgeSize());
    v.insert("toonTextureIndex", toonTextureIndex());
    v.insert("cullingDisabled", isCullingDisabled());
    v.insert("castingShadowEnabled", isCastingShadowEnabled());
    v.insert("castingShadowMapEnabled", isCastingShadowMapEnabled());
    v.insert("edgeEnabled", isEdgeEnabled());
    v.insert("vertexColorEnabled", isVertexColorEnabled());
    return v;
}

IMaterial *MaterialRefObject::data() const
{
    Q_ASSERT(m_materialRef);
    return m_materialRef;
}

ModelProxy *MaterialRefObject::parentModel() const
{
    Q_ASSERT(m_parentModelRef);
    return m_parentModelRef;
}

QUuid MaterialRefObject::uuid() const
{
    return m_uuid;
}

int MaterialRefObject::index() const
{
    Q_ASSERT(m_materialRef);
    return m_materialRef->index();
}

QString MaterialRefObject::name() const
{
    Q_ASSERT(m_parentModelRef);
    Q_ASSERT(m_materialRef);
    IEncoding::LanguageType language = static_cast<IEncoding::LanguageType>(m_parentModelRef->language());
    return Util::toQString(m_materialRef->name(language));
}

void MaterialRefObject::setName(const QString &value)
{
    Q_ASSERT(m_parentModelRef);
    Q_ASSERT(m_materialRef);
    if (name() != value) {
        IEncoding::LanguageType language = static_cast<IEncoding::LanguageType>(m_parentModelRef->language());
        QScopedPointer<IString> s(String::create(value.toStdString()));
        m_parentModelRef->renameObject(this, value);
        m_materialRef->setName(s.data(), language);
        setDirty(true);
        emit nameChanged();
    }
}

QString MaterialRefObject::mainTexturePath() const
{
    Q_ASSERT(m_materialRef);
    return Util::toQString(m_materialRef->mainTexture());
}

void MaterialRefObject::setMainTexturePath(const QString &value)
{
    Q_ASSERT(m_materialRef);
    if (mainTexturePath() != value) {
        QScopedPointer<IString> s(String::create(value.toStdString()));
        m_materialRef->setMainTexture(s.data());
        setDirty(true);
        emit mainTexturePathChanged();
    }
}

QString MaterialRefObject::sphereTexturePath() const
{
    Q_ASSERT(m_materialRef);
    return Util::toQString(m_materialRef->sphereTexture());
}

void MaterialRefObject::setSphereTexturePath(const QString &value)
{
    Q_ASSERT(m_materialRef);
    if (mainTexturePath() != value) {
        QScopedPointer<IString> s(String::create(value.toStdString()));
        m_materialRef->setSphereTexture(s.data());
        setDirty(true);
        emit sphereTexturePathChanged();
    }
}

QString MaterialRefObject::toonTexturePath() const
{
    Q_ASSERT(m_materialRef);
    return Util::toQString(m_materialRef->toonTexture());
}

void MaterialRefObject::setToonTexturePath(const QString &value)
{
    Q_ASSERT(m_materialRef);
    if (toonTexturePath() != value) {
        QScopedPointer<IString> s(String::create(value.toStdString()));
        m_materialRef->setToonTexture(s.data());
        setDirty(true);
        emit toonTexturePathChanged();
    }
}

QString MaterialRefObject::userAreaData() const
{
    Q_ASSERT(m_materialRef);
    return Util::toQString(m_materialRef->userDataArea());
}

void MaterialRefObject::setUserAreaData(const QString &value)
{
    Q_ASSERT(m_materialRef);
    if (toonTexturePath() != value) {
        QScopedPointer<IString> s(String::create(value.toStdString()));
        m_materialRef->setUserDataArea(s.data());
        setDirty(true);
        emit userAreaDataChanged();
    }
}

QColor MaterialRefObject::ambient() const
{
    Q_ASSERT(m_materialRef);
    return Util::fromColorRGBA(m_materialRef->ambient());
}

void MaterialRefObject::setAmbient(const QColor &value)
{
    Q_ASSERT(m_materialRef);
    if (ambient() != value) {
        m_materialRef->setAmbient(Util::toColorRGBA(value));
        setDirty(true);
        emit ambientChanged();
    }
}

QColor MaterialRefObject::diffuse() const
{
    Q_ASSERT(m_materialRef);
    return Util::fromColorRGBA(m_materialRef->diffuse());
}

void MaterialRefObject::setDiffuse(const QColor &value)
{
    Q_ASSERT(m_materialRef);
    if (diffuse() != value) {
        m_materialRef->setDiffuse(Util::toColorRGBA(value));
        setDirty(true);
        emit diffuseChanged();
    }
}

QColor MaterialRefObject::specular() const
{
    Q_ASSERT(m_materialRef);
    return Util::fromColorRGBA(m_materialRef->specular());
}

void MaterialRefObject::setSpecular(const QColor &value)
{
    Q_ASSERT(m_materialRef);
    if (specular() != value) {
        m_materialRef->setSpecular(Util::toColorRGBA(value));
        setDirty(true);
        emit specularChanged();
    }
}

QColor MaterialRefObject::edgeColor() const
{
    Q_ASSERT(m_materialRef);
    return Util::fromColorRGBA(m_materialRef->edgeColor());
}

void MaterialRefObject::setEdgeColor(const QColor &value)
{
    Q_ASSERT(m_materialRef);
    if (edgeColor() != value) {
        m_materialRef->setEdgeColor(Util::toColorRGBA(value));
        setDirty(true);
        emit edgeColorChanged();
    }
}

MaterialRefObject::SphereTextureType MaterialRefObject::sphereTextureType() const
{
    Q_ASSERT(m_materialRef);
    return static_cast<SphereTextureType>(m_materialRef->sphereTextureRenderMode());
}

void MaterialRefObject::setSphereTextureType(const SphereTextureType &value)
{
    if (sphereTextureType() != value) {
        m_materialRef->setSphereTextureRenderMode(static_cast<IMaterial::SphereTextureRenderMode>(value));
        setDirty(true);
        emit sphereTextureTypeChanged();
    }
}

qreal MaterialRefObject::shininess() const
{
    Q_ASSERT(m_materialRef);
    return m_materialRef->shininess();
}

void MaterialRefObject::setShininess(const qreal &value)
{
    Q_ASSERT(m_materialRef);
    if (!qFuzzyCompare(shininess(), value)) {
        m_materialRef->setShininess(value);
        setDirty(true);
        emit shininessChanged();
    }
}

qreal MaterialRefObject::edgeSize() const
{
    Q_ASSERT(m_materialRef);
    return m_materialRef->edgeSize();
}

void MaterialRefObject::setEdgeSize(const qreal &value)
{
    Q_ASSERT(m_materialRef);
    if (!qFuzzyCompare(edgeSize(), value)) {
        m_materialRef->setEdgeSize(value);
        setDirty(true);
        emit edgeSizeChanged();
    }
}

int MaterialRefObject::toonTextureIndex() const
{
    Q_ASSERT(m_materialRef);
    return m_materialRef->toonTextureIndex();
}

void MaterialRefObject::setToonTextureIndex(int value)
{
    if (toonTextureIndex() != value) {
        m_materialRef->setToonTextureIndex(value);
        setDirty(true);
        emit toonTextureIndexChanged();
    }
}

bool MaterialRefObject::isSharedToonTextureEnabled() const
{
    Q_ASSERT(m_materialRef);
    return m_materialRef->isSharedToonTextureUsed();
}

void MaterialRefObject::setSharedToonTextureEnabled(bool value)
{
    if (isSharedToonTextureEnabled() != value) {
        m_materialRef->setSharedToonTextureUsed(value);
        setDirty(true);
        emit sharedToonTextureEnabledChanged();
    }
}

bool MaterialRefObject::isCullingDisabled() const
{
    Q_ASSERT(m_materialRef);
    return m_materialRef->isCullingDisabled();
}

void MaterialRefObject::setCullingDisabled(bool value)
{
    Q_ASSERT(m_materialRef);
    if (isCullingDisabled() != value) {
        m_materialRef->setCullingDisabled(value);
        setDirty(true);
        emit cullingDisabledChanged();
    }
}

bool MaterialRefObject::isCastingShadowEnabled() const
{
    Q_ASSERT(m_materialRef);
    return m_materialRef->isCastingShadowEnabled();
}

void MaterialRefObject::setCastingShadowEnabled(bool value)
{
    Q_ASSERT(m_materialRef);
    if (isCastingShadowEnabled() != value) {
        m_materialRef->setCastingShadowEnabled(value);
        setDirty(true);
        emit castingShadowEnabledChanged();
    }
}

bool MaterialRefObject::isCastingShadowMapEnabled() const
{
    Q_ASSERT(m_materialRef);
    return m_materialRef->isCastingShadowMapEnabled();
}

void MaterialRefObject::setCastingShadowMapEnabled(bool value)
{
    Q_ASSERT(m_materialRef);
    if (isCastingShadowMapEnabled() != value) {
        m_materialRef->setCastingShadowMapEnabled(value);
        setDirty(true);
        emit castingShadowMapEnabledChanged();
    }
}

bool MaterialRefObject::isShadowMapEnabled() const
{
    Q_ASSERT(m_materialRef);
    return m_materialRef->isShadowMapEnabled();
}

void MaterialRefObject::setShadowMapEnabled(bool value)
{
    Q_ASSERT(m_materialRef);
    if (isShadowMapEnabled() != value) {
        m_materialRef->setShadowMapEnabled(value);
        setDirty(true);
        emit shadowMapEnabledChanged();
    }
}

bool MaterialRefObject::isEdgeEnabled() const
{
    Q_ASSERT(m_materialRef);
    return m_materialRef->isEdgeEnabled();
}

void MaterialRefObject::setEdgeEnabled(bool value)
{
    Q_ASSERT(m_materialRef);
    if (isEdgeEnabled() != value) {
        m_materialRef->setEdgeEnabled(value);
        setDirty(true);
        emit edgeEnabledChanged();
    }
}

bool MaterialRefObject::isVertexColorEnabled() const
{
    Q_ASSERT(m_materialRef);
    return m_materialRef->isVertexColorEnabled();
}

void MaterialRefObject::setVertexColorEnabled(bool value)
{
    Q_ASSERT(m_materialRef);
    if (isVertexColorEnabled() != value) {
        m_materialRef->setVertexColorEnabled(value);
        setDirty(true);
        emit vertexColorEnabledChanged();
    }
}

bool MaterialRefObject::isDirty() const
{
    return m_dirty;
}

void MaterialRefObject::setDirty(bool value)
{
    if (isDirty() != value) {
        m_dirty = value;
        emit dirtyChanged();
        if (value) {
            m_parentModelRef->markDirty();
        }
    }
}
