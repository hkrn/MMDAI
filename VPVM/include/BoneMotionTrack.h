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

#ifndef BONEMOTIONTRACK_H
#define BONEMOTIONTRACK_H

#include <QHash>
#include "BaseMotionTrack.h"

class BaseKeyframeRefObject;
class BoneKeyframeRefObject;
class MotionProxy;

namespace vpvl2 {
class IBoneKeyframe;
}

class BoneMotionTrack : public BaseMotionTrack
{
    Q_OBJECT

public:
    BoneMotionTrack(MotionProxy *motionProxy, const QString &name);
    ~BoneMotionTrack();

    BaseKeyframeRefObject *copy(BaseKeyframeRefObject *value, const qint64 &timeIndex, bool doUpdate);
    BaseKeyframeRefObject *convert(vpvl2::IKeyframe *value);
    BoneKeyframeRefObject *convertBoneKeyframe(vpvl2::IBoneKeyframe *keyframe);
    void addKeyframe(BoneKeyframeRefObject *keyframe, bool doUpdate);
    void addKeyframe(QObject *value, bool doUpdate);
    void removeKeyframe(BoneKeyframeRefObject *keyframe, bool doUpdate);
    void removeKeyframe(QObject *value, bool doUpdate);
    void replace(BoneKeyframeRefObject *dst, BoneKeyframeRefObject *src, bool doUpdate);
    vpvl2::IKeyframe::Type type() const;
};

#endif // BONEMOTIONTRACK_H
