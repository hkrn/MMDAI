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

#include <vpvl2/vpvl2.h>
#include <vpvl2/extensions/qt/String.h>

using namespace vpvl2;
using namespace vpvl2::extensions::qt;

MaterialRefObject::MaterialRefObject(ModelProxy *parentModelRef,
                                     vpvl2::IMaterial *materialRef,
                                     const QUuid &uuid)
    : m_parentModelRef(parentModelRef),
      m_materialRef(materialRef),
      m_uuid(uuid)
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
        m_materialRef->setName(s.data(), language);
        m_parentModelRef->markDirty();
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
        m_parentModelRef->markDirty();
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
        m_parentModelRef->markDirty();
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
        m_parentModelRef->markDirty();
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
        m_parentModelRef->markDirty();
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
        m_parentModelRef->markDirty();
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
        m_parentModelRef->markDirty();
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
        m_parentModelRef->markDirty();
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
        m_parentModelRef->markDirty();
        emit edgeColorChanged();
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
        m_parentModelRef->markDirty();
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
        m_parentModelRef->markDirty();
        emit edgeSizeChanged();
    }
}

bool MaterialRefObject::isCullingDisabled() const
{
    return m_materialRef->isCullingDisabled();
}

void MaterialRefObject::setCullingDisabled(bool value)
{
    if (isCullingDisabled() != value) {
        m_materialRef->setCullingDisabled(value);
        m_parentModelRef->markDirty();
        emit cullingDisabledChanged();
    }
}

bool MaterialRefObject::isCastingShadowEnabled() const
{
    return m_materialRef->isCastingShadowEnabled();
}

void MaterialRefObject::setCastingShadowEnabled(bool value)
{
    if (isCastingShadowEnabled() != value) {
        m_materialRef->setCastingShadowEnabled(value);
        m_parentModelRef->markDirty();
        emit castingShadowEnabledChanged();
    }
}

bool MaterialRefObject::isCastingShadowMapEnabled() const
{
    return m_materialRef->isCastingShadowMapEnabled();
}

void MaterialRefObject::setCastingShadowMapEnabled(bool value)
{
    if (isCastingShadowMapEnabled() != value) {
        m_materialRef->setCastingShadowMapEnabled(value);
        m_parentModelRef->markDirty();
        emit castingShadowMapEnabledChanged();
    }
}

bool MaterialRefObject::isShadowMapEnabled() const
{
    return m_materialRef->isShadowMapEnabled();
}

void MaterialRefObject::setShadowMapEnabled(bool value)
{
    if (isShadowMapEnabled() != value) {
        m_materialRef->setShadowMapEnabled(value);
        m_parentModelRef->markDirty();
        emit shadowMapEnabledChanged();
    }
}

bool MaterialRefObject::isEdgeEnabled() const
{
    return m_materialRef->isEdgeEnabled();
}

void MaterialRefObject::setEdgeEnabled(bool value)
{
    if (isEdgeEnabled() != value) {
        m_materialRef->setEdgeEnabled(value);
        m_parentModelRef->markDirty();
        emit edgeEnabledChanged();
    }
}

bool MaterialRefObject::isVertexColorEnabled() const
{
    return m_materialRef->isVertexColorEnabled();
}

void MaterialRefObject::setVertexColorEnabled(bool value)
{
    if (isVertexColorEnabled() != value) {
        m_materialRef->setVertexColorEnabled(value);
        m_parentModelRef->markDirty();
        emit vertexColorEnabledChanged();
    }
}
