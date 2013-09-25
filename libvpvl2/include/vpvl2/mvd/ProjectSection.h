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

#pragma once
#ifndef VPVL2_MVD_PROJECTSECTION_H_
#define VPVL2_MVD_PROJECTSECTION_H_

#include "vpvl2/mvd/BaseSection.h"

namespace vpvl2
{
class IEncoding;

namespace mvd
{

class VPVL2_API ProjectSection VPVL2_DECL_FINAL : public BaseSection
{
public:
    ProjectSection(Motion *motionRef);
    ~ProjectSection();

    static bool preparse(uint8 *&ptr, vsize &rest, Motion::DataInfo &info);

    void read(const uint8 *data);
    void seek(const IKeyframe::TimeIndex &timeIndex);
    void write(uint8 *data) const;
    vsize estimateSize() const;
    vsize countKeyframes() const;
    void update();
    void addKeyframe(IKeyframe *keyframe);
    void removeKeyframe(IKeyframe *keyframe);
    void deleteKeyframe(IKeyframe *&keyframe);
    void getKeyframes(const IKeyframe::TimeIndex &timeIndex,
                      const IKeyframe::LayerIndex &layerIndex,
                      Array<IKeyframe *> &keyframes) const;
    void getAllKeyframes(Array<IKeyframe *> &keyframes) const;
    void setAllKeyframes(const Array<IKeyframe *> &value);
    IKeyframe::LayerIndex countLayers() const;
    IProjectKeyframe *findKeyframe(const IKeyframe::TimeIndex &timeIndex,
                                   const IKeyframe::LayerIndex &layerIndex) const;
    IProjectKeyframe *findKeyframeAt(int index) const;

private:
    class PrivateContext;
    PrivateContext *m_context;

    VPVL2_DISABLE_COPY_AND_ASSIGN(ProjectSection)
};

} /* namespace mvd */
} /* namespace vpvl2 */

#endif

