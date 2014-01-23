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

#ifndef VECTOR3REFOBJECT_H_
#define VECTOR3REFOBJECT_H_

#include <QObject>
#include <QDebug>
#include <QVector3D>

class Vector3RefObject : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVector3D value READ value WRITE setValue NOTIFY xChanged NOTIFY yChanged NOTIFY zChanged NOTIFY valueChanged FINAL)
    Q_PROPERTY(qreal x READ x WRITE setX NOTIFY xChanged NOTIFY valueChanged FINAL)
    Q_PROPERTY(qreal y READ y WRITE setY NOTIFY yChanged NOTIFY valueChanged FINAL)
    Q_PROPERTY(qreal z READ z WRITE setZ NOTIFY zChanged NOTIFY valueChanged FINAL)

public:
    Vector3RefObject(QObject *parent = 0)
        : QObject(parent)
    {
    }
    ~Vector3RefObject() {
    }

    qreal x() const { return m_value.x(); }
    qreal y() const { return m_value.y(); }
    qreal z() const { return m_value.z(); }
    QVector3D value() const { return m_value; }

    void setX(const qreal &value) {
        if (!qFuzzyCompare(x(), value)) {
            m_value.setX(value);
            emit xChanged();
            emit valueChanged();
        }
    }
    void setY(const qreal &value) {
        if (!qFuzzyCompare(y(), value)) {
            m_value.setY(value);
            emit yChanged();
            emit valueChanged();
        }
    }
    void setZ(const qreal &value) {
        if (!qFuzzyCompare(z(), value)) {
            m_value.setZ(value);
            emit zChanged();
            emit valueChanged();
        }
    }
    void setValue(const QVector3D &value) {
        if (!qFuzzyCompare(m_value, value)) {
            m_value = value;
            emit xChanged();
            emit yChanged();
            emit zChanged();
            emit valueChanged();
        }
    }

signals:
    void xChanged();
    void yChanged();
    void zChanged();
    void valueChanged();

private:
    QVector3D m_value;
};

#endif
