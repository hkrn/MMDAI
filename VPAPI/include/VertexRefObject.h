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

#ifndef VERTEXREFOBJECT_H
#define VERTEXREFOBJECT_H

#include <QObject>
#include <QUuid>
#include <QVector3D>
#include <QVector4D>
#include <vpvl2/IVertex.h>

class BoneRefObject;
class ModelProxy;

class VertexRefObject : public QObject
{
    Q_OBJECT
    Q_ENUMS(Type)
    Q_PROPERTY(ModelProxy *parentModel READ parentModel CONSTANT FINAL)
    Q_PROPERTY(QUuid uuid READ uuid CONSTANT FINAL)
    Q_PROPERTY(int index READ index CONSTANT FINAL)
    Q_PROPERTY(QVector3D origin READ origin WRITE setOrigin NOTIFY originChanged FINAL)
    Q_PROPERTY(QVector3D normal READ normal WRITE setNormal NOTIFY normalChanged FINAL)
    Q_PROPERTY(QVector3D textureCoord READ textureCoord WRITE setTextureCoord NOTIFY textureCoordChanged FINAL)
    Q_PROPERTY(QVector3D sdefC READ sdefC WRITE setSdefC NOTIFY sdefCChanged FINAL)
    Q_PROPERTY(QVector3D sdefR0 READ sdefR0 WRITE setSdefR0 NOTIFY sdefR0Changed FINAL)
    Q_PROPERTY(QVector3D sdefR1 READ sdefR1 WRITE setSdefR1 NOTIFY sdefR1Changed FINAL)
    Q_PROPERTY(qreal edgeSize READ edgeSize WRITE setEdgeSize NOTIFY edgeSizeChanged FINAL)
    Q_PROPERTY(Type type READ type WRITE setType NOTIFY typeChanged FINAL)

public:
    enum Type {
        Bdef1 = vpvl2::IVertex::kBdef1,
        Bdef2 = vpvl2::IVertex::kBdef2,
        Bdef4 = vpvl2::IVertex::kBdef4,
        Sdef  = vpvl2::IVertex::kSdef,
        Qdef  = vpvl2::IVertex::kQdef
    };

    VertexRefObject(ModelProxy *parentModelRef, vpvl2::IVertex *vertexRef, const QUuid &uuid);
    ~VertexRefObject();

    Q_INVOKABLE QVector4D originUV(int index);
    Q_INVOKABLE void setOriginUV(int index, const QVector4D &value);
    Q_INVOKABLE QVector4D morphUV(int index);
    Q_INVOKABLE void setMorphUV(int index, const QVector4D &value);
    Q_INVOKABLE BoneRefObject *bone(int index);
    Q_INVOKABLE void setBone(int index, BoneRefObject *value);
    Q_INVOKABLE qreal weight(int index);
    Q_INVOKABLE void setWeight(int index, const qreal &value);

    vpvl2::IVertex *data() const;
    ModelProxy *parentModel() const;
    QUuid uuid() const;
    int index() const;
    QVector3D origin() const;
    void setOrigin(const QVector3D &value);
    QVector3D normal() const;
    void setNormal(const QVector3D &value);
    QVector3D textureCoord() const;
    void setTextureCoord(const QVector3D &value);
    QVector3D sdefC() const;
    void setSdefC(const QVector3D &value);
    QVector3D sdefR0() const;
    void setSdefR0(const QVector3D &value);
    QVector3D sdefR1() const;
    void setSdefR1(const QVector3D &value);
    qreal edgeSize() const;
    void setEdgeSize(const qreal &value);
    Type type() const;
    void setType(const Type &value);

signals:
    void originChanged();
    void normalChanged();
    void textureCoordChanged();
    void sdefCChanged();
    void sdefR0Changed();
    void sdefR1Changed();
    void edgeSizeChanged();
    void typeChanged();
    void originUVDidChange(int index, const QVector4D &newValue, const QVector4D &oldValue);
    void morphUVDidChange(int index, const QVector4D &newValue, const QVector4D &oldValue);
    void boneDidChange(int index, BoneRefObject *newValue, BoneRefObject *oldValue);
    void weightDidChange(int index, const qreal &newValue, const qreal &oldValue);

private:
    ModelProxy *m_parentModelRef;
    vpvl2::IVertex *m_vertexRef;
    const QUuid m_uuid;
};

#endif
