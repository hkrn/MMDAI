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

#include <QObject>
#include <QUuid>

class LabelRefObject;

namespace vpvl2 {
class IMorph;
}

class MorphRefObject : public QObject
{
    Q_OBJECT
    Q_ENUMS(Category)
    Q_PROPERTY(LabelRefObject *parentLabel READ parentLabel CONSTANT FINAL)
    Q_PROPERTY(QUuid uuid READ uuid CONSTANT FINAL)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged FINAL)
    Q_PROPERTY(Category category READ category WRITE setCategory NOTIFY categoryChanged FINAL)
    Q_PROPERTY(int index READ index CONSTANT FINAL)
    Q_PROPERTY(qreal weight READ weight WRITE setWeight NOTIFY weightChanged FINAL)
    Q_PROPERTY(qreal originWeight READ originWeight NOTIFY originWeightChanged FINAL)

public:
    enum Category {
        Unknown,
        Eye,
        Lip,
        Eyeblow,
        Other
    };

    MorphRefObject(LabelRefObject *labelRef, vpvl2::IMorph *morphRef, const QUuid &uuid);
    ~MorphRefObject();

    void setOriginWeight(const qreal &value);

    vpvl2::IMorph *data() const;
    LabelRefObject *parentLabel() const;
    QUuid uuid() const;
    QString name() const;
    void setName(const QString &value);
    Category category() const;
    void setCategory(const Category &value);
    int index() const;
    qreal weight() const;
    void setWeight(const qreal &value);
    qreal originWeight() const;

public slots:
    Q_INVOKABLE void sync();

signals:
    void nameChanged();
    void categoryChanged();
    void weightChanged();
    void originWeightChanged();
    void morphDidSync();

private:
    LabelRefObject *m_parentLabelRef;
    vpvl2::IMorph *m_morphRef;
    const QUuid m_uuid;
    qreal m_originWeight;
};

#endif // MORPHREFOBJECT_H
