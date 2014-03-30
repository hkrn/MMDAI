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

#include <vpvl2/vpvl2.h>
#include <vpvl2/extensions/vpdb/Motion.h>
#include <vpvl2/extensions/vpdb/BoneKeyframe.h>
#include <vpvl2/extensions/vpdb/CameraKeyframe.h>
#include <vpvl2/extensions/vpdb/EffectKeyframe.h>
#include <vpvl2/extensions/vpdb/LightKeyframe.h>
#include <vpvl2/extensions/vpdb/ModelKeyframe.h>
#include <vpvl2/extensions/vpdb/MorphKeyframe.h>
#include <vpvl2/extensions/vpdb/ProjectKeyframe.h>

namespace vpvl2
{
namespace VPVL2_VERSION_NS
{
namespace extensions
{
namespace vpdb
{

struct Motion::PrivateContext {
    PrivateContext(Project *parent)
        : m_parentProjectRef(parent)
    {
    }
    ~PrivateContext() {
        m_parentProjectRef = 0;
    }

    Project *m_parentProjectRef;
};

Motion::Motion(Project *parent)
    : m_context(new PrivateContext(parent))
{
}

Motion::~Motion()
{
    delete m_context;
    m_context = 0;
}

bool Motion::load(const uint8 * /* data */, vsize /* size */)
{
    return false;
}

void Motion::save(uint8 * /* data */) const
{
}

vsize Motion::estimateSize() const
{
    return 0;
}

IMotion::Error Motion::error() const
{
    return kNoError;
}

IModel *Motion::parentModelRef() const
{
    return 0;
}

void Motion::refresh()
{
}

void Motion::seekSeconds(const float64 &seconds)
{
}

void Motion::seekSceneSeconds(const float64 &seconds, Scene *scene, int flags)
{
}

void Motion::seekTimeIndex(const IKeyframe::TimeIndex &timeIndex)
{
}

void Motion::seekSceneTimeIndex(const IKeyframe::TimeIndex &timeIndex, Scene *scene, int flags)
{
}

void Motion::reset()
{
}

float64 Motion::durationSeconds() const
{
    return 0;
}

IKeyframe::TimeIndex Motion::durationTimeIndex() const
{
    return 0;
}

bool Motion::isReachedTo(const IKeyframe::TimeIndex &timeIndex) const
{
    return false;
}

IBoneKeyframe *Motion::createBoneKeyframe()
{
    return new BoneKeyframe(this);
}

ICameraKeyframe *Motion::createCameraKeyframe()
{
    return new CameraKeyframe(this);
}

IEffectKeyframe *Motion::createEffectKeyframe()
{
    return new EffectKeyframe(this);
}

ILightKeyframe *Motion::createLightKeyframe()
{
    return new LightKeyframe(this);
}

IModelKeyframe *Motion::createModelKeyframe()
{
    return new ModelKeyframe(this);
}

IMorphKeyframe *Motion::createMorphKeyframe()
{
    return new MorphKeyframe(this);
}

IProjectKeyframe *Motion::createProjectKeyframe()
{
    return new ProjectKeyframe(this);
}

void Motion::addKeyframe(IKeyframe *value)
{
}

int Motion::countKeyframes(IKeyframe::Type value) const
{
}

IKeyframe::LayerIndex Motion::countLayers(const IString *name, IKeyframe::Type type) const
{
    return 0;
}

void Motion::getKeyframeRefs(const IKeyframe::TimeIndex &timeIndex,
                             const IKeyframe::LayerIndex &layerIndex,
                             IKeyframe::Type type,
                             Array<IKeyframe *> &keyframes)
{
}

IBoneKeyframe *Motion::findBoneKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                           const IString *name,
                                           const IKeyframe::LayerIndex &layerIndex) const
{
    return 0;
}

IBoneKeyframe *Motion::findBoneKeyframeRefAt(int index) const
{
    return 0;
}

ICameraKeyframe *Motion::findCameraKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                               const IKeyframe::LayerIndex &layerIndex) const
{
    return 0;
}

ICameraKeyframe *Motion::findCameraKeyframeRefAt(int index) const
{
    return 0;
}

IEffectKeyframe *Motion::findEffectKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                               const IString *name,
                                               const IKeyframe::LayerIndex &layerIndex) const
{
    return 0;
}

IEffectKeyframe *Motion::findEffectKeyframeRefAt(int index) const
{
    return 0;
}

ILightKeyframe *Motion::findLightKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                                     const IKeyframe::LayerIndex &layerIndex) const
{
    return 0;
}

ILightKeyframe *Motion::findLightKeyframeRefAt(int index) const
{
    return 0;
}

IModelKeyframe *Motion::findModelKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                             const IKeyframe::LayerIndex &layerIndex) const
{
    return 0;
}

IModelKeyframe *Motion::findModelKeyframeRefAt(int index) const
{
    return 0;
}

IMorphKeyframe *Motion::findMorphKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                             const IString *name,
                                             const IKeyframe::LayerIndex &layerIndex) const
{
    return 0;
}

IMorphKeyframe *Motion::findMorphKeyframeRefAt(int index) const
{
    return 0;
}

IProjectKeyframe *Motion::findProjectKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                                 const IKeyframe::LayerIndex &layerIndex) const
{
    return 0;
}

IProjectKeyframe *Motion::findProjectKeyframeRefAt(int index) const
{
    return 0;
}

void Motion::replaceKeyframe(IKeyframe *value, bool alsoDelete)
{
}

void Motion::removeKeyframe(IKeyframe *value)
{
}

void Motion::deleteKeyframe(IKeyframe *&value)
{
}

void Motion::update(IKeyframe::Type type)
{
}

void Motion::getAllKeyframeRefs(Array<IKeyframe *> &value, IKeyframe::Type type)
{
}

void Motion::setAllKeyframes(const Array<IKeyframe *> &value, IKeyframe::Type type)
{
}

void Motion::createFirstKeyframesUnlessFound()
{
}

IMotion *Motion::clone() const
{
    return 0;
}

Scene *Motion::parentSceneRef() const
{
    return 0;
}

const IString *Motion::name() const
{
    return 0;
}

IMotion::FormatType Motion::type() const
{
    return kMVDFormat;
}

} /* namespace vpdb */
} /* namespace extensions */
} /* namespace VPVL2_VERSION_NS */
} /* namespace vpvl2 */
