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

#ifndef MORPHKEYFRAMEREFOBJECT_H
#define MORPHKEYFRAMEREFOBJECT_H

#include "BaseKeyframeRefObject.h"

class MorphMotionTrack;
class MorphRefObject;

namespace vpvl2 {
class IMorphKeyframe;
}

class MorphKeyframeRefObject : public BaseKeyframeRefObject
{
    Q_OBJECT

    Q_PROPERTY(MorphRefObject *parentMorph READ parentMorph CONSTANT FINAL)
    Q_PROPERTY(qreal weight READ weight WRITE setWeight NOTIFY weightChanged FINAL)

public:
    MorphKeyframeRefObject(MorphMotionTrack *trackRef, vpvl2::IMorphKeyframe *data);
    ~MorphKeyframeRefObject();

    BaseMotionTrack *parentTrack() const;
    MorphRefObject *parentMorph() const;
    QObject *opaque() const;
    QString name() const;
    void setName(const QString &value);
    qreal weight() const;
    void setWeight(const qreal &value);

    vpvl2::IMorphKeyframe *data() const;

protected:
    vpvl2::IKeyframe *baseKeyframeData() const;

signals:
    void weightChanged();

private:
    MorphMotionTrack *m_parentTrackRef;
    vpvl2::IMorphKeyframe *m_keyframeRef;
};

#endif // MORPHKEYFRAMEREFOBJECT_H
