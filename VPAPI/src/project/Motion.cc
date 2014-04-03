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
#include <vpvl2/extensions/qt/String.h>
#include <project/Project.h>
#include <project/Motion.h>
#include <project/BoneKeyframe.h>
#include <project/CameraKeyframe.h>
#include <project/EffectKeyframe.h>
#include <project/LightKeyframe.h>
#include <project/ModelKeyframe.h>
#include <project/MorphKeyframe.h>
#include <project/ProjectKeyframe.h>

#include <QSqlQuery>

using namespace vpvl2;
using namespace vpvl2::extensions::qt;

namespace project
{

typedef QHash<QString, IBoneKeyframe *> BoneTrack;
typedef QMap<IKeyframe::TimeIndex, BoneTrack>  BoneTrackAnimation;
typedef QMap<IKeyframe::LayerIndex, BoneTrackAnimation> BoneTrackAnimationBundle;
typedef QMap<IKeyframe::TimeIndex, ICameraKeyframe *> CameraTrackAnimation;
typedef QMap<IKeyframe::LayerIndex, CameraTrackAnimation> CameraTrackAnimationBundle;
typedef QHash<QString, IEffectKeyframe *> EffectTrack;
typedef QMap<IKeyframe::TimeIndex, EffectTrack> EffectTrackAnimation;
typedef QMap<IKeyframe::LayerIndex, EffectTrackAnimation> EffectTrackAnimationBundle;
typedef QMap<IKeyframe::TimeIndex, ILightKeyframe *> LightTrackAnimation;
typedef QMap<IKeyframe::LayerIndex, LightTrackAnimation> LightTrackAnimationBundle;
typedef QMap<IKeyframe::TimeIndex, IModelKeyframe *> ModelTrackAnimation;
typedef QMap<IKeyframe::LayerIndex, ModelTrackAnimation> ModelTrackAnimationBundle;
typedef QHash<QString, IMorphKeyframe *> MorphTrack;
typedef QMap<IKeyframe::TimeIndex, MorphTrack> MorphTrackAnimation;
typedef QMap<IKeyframe::LayerIndex, MorphTrackAnimation> MorphTrackAnimationBundle;
typedef QMap<IKeyframe::TimeIndex, IProjectKeyframe *> ProjectTrackAnimation;
typedef QMap<IKeyframe::LayerIndex, ProjectTrackAnimation> ProjectTrackAnimationBundle;

struct Motion::PrivateContext {
    PrivateContext(Project *parent)
        : parentProjectRef(parent)
    {
    }
    ~PrivateContext() {
        for (BoneTrackAnimationBundle::ConstIterator it = boneBundle.begin(), end = boneBundle.end(); it != end; ++it) {
            for (BoneTrackAnimation::ConstIterator it2 = it.value().begin(), end2 = it.value().end(); it2 != end2; ++it2) {
                qDeleteAll(*it2);
            }
        }
        for (CameraTrackAnimationBundle::ConstIterator it = cameraBundle.begin(), end = cameraBundle.end(); it != end; ++it) {
            qDeleteAll(*it);
        }
        for (EffectTrackAnimationBundle::ConstIterator it = effectBundle.begin(), end = effectBundle.end(); it != end; ++it) {
            for (EffectTrackAnimation::ConstIterator it2 = it.value().begin(), end2 = it.value().end(); it2 != end2; ++it2) {
                qDeleteAll(*it2);
            }
        }
        for (LightTrackAnimationBundle::ConstIterator it = lightBundle.begin(), end = lightBundle.end(); it != end; ++it) {
            qDeleteAll(*it);
        }
        for (ModelTrackAnimationBundle::ConstIterator it = modelBundle.begin(), end = modelBundle.end(); it != end; ++it) {
            qDeleteAll(*it);
        }
        for (MorphTrackAnimationBundle::ConstIterator it = morphBundle.begin(), end = morphBundle.end(); it != end; ++it) {
            for (MorphTrackAnimation::ConstIterator it2 = it.value().begin(), end2 = it.value().end(); it2 != end2; ++it2) {
                qDeleteAll(*it2);
            }
        }
        for (ProjectTrackAnimationBundle::ConstIterator it = projectBundle.begin(), end = projectBundle.end(); it != end; ++it) {
            qDeleteAll(*it);
        }
        parentProjectRef = 0;
    }

    Project *parentProjectRef;
    mutable BoneTrackAnimationBundle boneBundle;
    mutable CameraTrackAnimationBundle cameraBundle;
    mutable EffectTrackAnimationBundle effectBundle;
    mutable LightTrackAnimationBundle lightBundle;
    mutable ModelTrackAnimationBundle modelBundle;
    mutable MorphTrackAnimationBundle morphBundle;
    mutable ProjectTrackAnimationBundle projectBundle;
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
    switch (value->type()) {
    case IKeyframe::kBoneKeyframe:
        break;
    case IKeyframe::kCameraKeyframe:
        break;
    case IKeyframe::kLightKeyframe:
        break;
    case IKeyframe::kModelKeyframe:
        break;
    case IKeyframe::kMorphKeyframe:
        break;
    case IKeyframe::kProjectKeyframe:
        break;
    default:
        break;
    }
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
    const QString &n = static_cast<const String *>(name)->value();
    if (m_context->boneBundle.contains(layerIndex)) {
        const BoneTrackAnimation &a = m_context->boneBundle.value(layerIndex);
        if (a.contains(timeIndex)) {
            const BoneTrack &t = a.value(timeIndex);
            if (t.contains(n)) {
                return t.value(n);
            }
        }
    }
    return 0;
}

IBoneKeyframe *Motion::findBoneKeyframeRefAt(int index) const
{
    return 0;
}

ICameraKeyframe *Motion::findCameraKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                               const IKeyframe::LayerIndex &layerIndex) const
{
    if (m_context->cameraBundle.contains(layerIndex)) {
        const CameraTrackAnimation &a = m_context->cameraBundle.value(timeIndex);
        if (a.contains(timeIndex)) {
            return a.value(timeIndex);
        }
    }
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
    const QString &n = static_cast<const String *>(name)->value();
    if (m_context->effectBundle.contains(layerIndex)) {
        const EffectTrackAnimation &a = m_context->effectBundle.value(layerIndex);
        if (a.contains(timeIndex)) {
            const EffectTrack &t = a.value(timeIndex);
            if (t.contains(n)) {
                return t.value(n);
            }
        }
    }
    return 0;
}

IEffectKeyframe *Motion::findEffectKeyframeRefAt(int index) const
{
    return 0;
}

ILightKeyframe *Motion::findLightKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                             const IKeyframe::LayerIndex &layerIndex) const
{
    if (m_context->lightBundle.contains(layerIndex)) {
        const LightTrackAnimation &a = m_context->lightBundle.value(timeIndex);
        if (a.contains(timeIndex)) {
            return a.value(timeIndex);
        }
    }
    return 0;
}

ILightKeyframe *Motion::findLightKeyframeRefAt(int index) const
{
    return 0;
}

IModelKeyframe *Motion::findModelKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                             const IKeyframe::LayerIndex &layerIndex) const
{
    if (m_context->modelBundle.contains(layerIndex)) {
        const ModelTrackAnimation &a = m_context->modelBundle.value(timeIndex);
        if (a.contains(timeIndex)) {
            return a.value(timeIndex);
        }
    }
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
    const QString &n = static_cast<const String *>(name)->value();
    if (m_context->morphBundle.contains(layerIndex)) {
        const MorphTrackAnimation &a = m_context->morphBundle.value(layerIndex);
        if (a.contains(timeIndex)) {
            const MorphTrack &t = a.value(timeIndex);
            if (t.contains(n)) {
                return t.value(n);
            }
        }
    }
    return 0;
}

IMorphKeyframe *Motion::findMorphKeyframeRefAt(int index) const
{
    return 0;
}

IProjectKeyframe *Motion::findProjectKeyframeRef(const IKeyframe::TimeIndex &timeIndex,
                                                 const IKeyframe::LayerIndex &layerIndex) const
{
    if (m_context->projectBundle.contains(layerIndex)) {
        const ProjectTrackAnimation &a = m_context->projectBundle.value(timeIndex);
        if (a.contains(timeIndex)) {
            return a.value(timeIndex);
        }
    }
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
    removeKeyframe(value);
    delete value;
    value = 0;
}

void Motion::update(IKeyframe::Type /* type */)
{
}

void Motion::getAllKeyframeRefs(Array<IKeyframe *> &value, IKeyframe::Type type)
{
    switch (type) {
    case IKeyframe::kBoneKeyframe:
        break;
    case IKeyframe::kCameraKeyframe:
        break;
    case IKeyframe::kLightKeyframe:
        break;
    case IKeyframe::kModelKeyframe:
        break;
    case IKeyframe::kMorphKeyframe:
        break;
    case IKeyframe::kProjectKeyframe:
        break;
    default:
        break;
    }
}

void Motion::setAllKeyframes(const Array<IKeyframe *> &value, IKeyframe::Type type)
{
    switch (type) {
    case IKeyframe::kBoneKeyframe:
        break;
    case IKeyframe::kCameraKeyframe:
        break;
    case IKeyframe::kLightKeyframe:
        break;
    case IKeyframe::kModelKeyframe:
        break;
    case IKeyframe::kMorphKeyframe:
        break;
    case IKeyframe::kProjectKeyframe:
        break;
    default:
        break;
    }
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

} /* namespace project */
