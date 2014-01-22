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

#ifndef MATERIALREFOBJECT_H
#define MATERIALREFOBJECT_H

#include <QColor>
#include <QObject>
#include <QUuid>
#include <vpvl2/Common.h>

class ModelProxy;

namespace vpvl2 {
class IMaterial;
}

class MaterialRefObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(ModelProxy *parentModel READ parentModel CONSTANT FINAL)
    Q_PROPERTY(QUuid uuid READ uuid CONSTANT FINAL)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)
    Q_PROPERTY(QString mainTexturePath READ mainTexturePath WRITE setMainTexturePath NOTIFY mainTexturePathChanged FINAL)
    Q_PROPERTY(QString sphereTexturePath READ sphereTexturePath WRITE setSphereTexturePath NOTIFY sphereTexturePathChanged FINAL)
    Q_PROPERTY(QString toonTexturePath READ toonTexturePath WRITE setToonTexturePath NOTIFY toonTexturePathChanged FINAL)
    Q_PROPERTY(QString userAreaData READ userAreaData WRITE setUserAreaData NOTIFY userAreaDataChanged FINAL)
    Q_PROPERTY(QColor ambient READ ambient WRITE setAmbient NOTIFY ambientChanged FINAL)
    Q_PROPERTY(QColor diffuse READ diffuse WRITE setDiffuse NOTIFY diffuseChanged FINAL)
    Q_PROPERTY(QColor specular READ specular WRITE setSpecular NOTIFY specularChanged FINAL)
    Q_PROPERTY(QColor edgeColor READ edgeColor WRITE setEdgeColor NOTIFY edgeColorChanged FINAL)
    Q_PROPERTY(qreal shininess READ shininess WRITE setShininess NOTIFY shininessChanged FINAL)
    Q_PROPERTY(qreal edgeSize READ edgeSize WRITE setEdgeSize NOTIFY edgeSizeChanged FINAL)
    Q_PROPERTY(bool cullingDisabled READ isCullingDisabled WRITE setCullingDisabled NOTIFY cullingDisabledChanged FINAL)
    Q_PROPERTY(bool castingShadowEnabled READ isCastingShadowEnabled WRITE setCastingShadowEnabled NOTIFY castingShadowEnabledChanged FINAL)
    Q_PROPERTY(bool castingShadowMapEnabled READ isCastingShadowMapEnabled WRITE setCastingShadowMapEnabled NOTIFY castingShadowMapEnabledChanged FINAL)
    Q_PROPERTY(bool shadowMapEnabled READ isShadowMapEnabled WRITE setShadowMapEnabled NOTIFY shadowMapEnabledChanged FINAL)
    Q_PROPERTY(bool edgeEnabled READ isEdgeEnabled WRITE setEdgeEnabled NOTIFY edgeEnabledChanged FINAL)
    Q_PROPERTY(bool vertexColorEnabled READ isVertexColorEnabled WRITE setVertexColorEnabled NOTIFY vertexColorEnabledChanged)

public:
    MaterialRefObject(ModelProxy *parentModelRef, vpvl2::IMaterial *materialRef, const QUuid &uuid);
    ~MaterialRefObject();

    vpvl2::IMaterial *data() const;
    ModelProxy *parentModel() const;
    QUuid uuid() const;
    QString name() const;
    void setName(const QString &value);
    QString mainTexturePath() const;
    void setMainTexturePath(const QString &value);
    QString sphereTexturePath() const;
    void setSphereTexturePath(const QString &value);
    QString toonTexturePath() const;
    void setToonTexturePath(const QString &value);
    QString userAreaData() const;
    void setUserAreaData(const QString &value);
    QColor ambient() const;
    void setAmbient(const QColor &value);
    QColor diffuse() const;
    void setDiffuse(const QColor &value);
    QColor specular() const;
    void setSpecular(const QColor &value);
    QColor edgeColor() const;
    void setEdgeColor(const QColor &value);
    qreal shininess() const;
    void setShininess(const qreal &value);
    qreal edgeSize() const;
    void setEdgeSize(const qreal &value);
    bool isCullingDisabled() const;
    void setCullingDisabled(bool value);
    bool isCastingShadowEnabled() const;
    void setCastingShadowEnabled(bool value);
    bool isCastingShadowMapEnabled() const;
    void setCastingShadowMapEnabled(bool value);
    bool isShadowMapEnabled() const;
    void setShadowMapEnabled(bool value);
    bool isEdgeEnabled() const;
    void setEdgeEnabled(bool value);
    bool isVertexColorEnabled() const;
    void setVertexColorEnabled(bool value);

signals:
    void nameChanged();
    void mainTexturePathChanged();
    void sphereTexturePathChanged();
    void toonTexturePathChanged();
    void userAreaDataChanged();
    void ambientChanged();
    void diffuseChanged();
    void specularChanged();
    void edgeColorChanged();
    void shininessChanged();
    void edgeSizeChanged();
    void cullingDisabledChanged();
    void castingShadowEnabledChanged();
    void castingShadowMapEnabledChanged();
    void shadowMapEnabledChanged();
    void edgeEnabledChanged();
    void vertexColorEnabledChanged();

private:
    ModelProxy *m_parentModelRef;
    vpvl2::IMaterial *m_materialRef;
    const QUuid m_uuid;
};

#endif
