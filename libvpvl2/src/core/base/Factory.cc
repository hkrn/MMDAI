/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAI project team nor the names of     */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#include "vpvl2/vpvl2.h"

#include "vpvl2/asset/Model.h"
#include "vpvl2/mvd/Motion.h"
#include "vpvl2/pmx/Model.h"
#include "vpvl2/mvd/BoneKeyframe.h"
#include "vpvl2/mvd/CameraKeyframe.h"
#include "vpvl2/mvd/EffectKeyframe.h"
#include "vpvl2/mvd/LightKeyframe.h"
#include "vpvl2/mvd/ModelKeyframe.h"
#include "vpvl2/mvd/MorphKeyframe.h"
#include "vpvl2/mvd/ProjectKeyframe.h"
#include "vpvl2/vmd/BoneKeyframe.h"
#include "vpvl2/vmd/CameraKeyframe.h"
#include "vpvl2/vmd/LightKeyframe.h"
#include "vpvl2/vmd/MorphKeyframe.h"
#include "vpvl2/vmd/Motion.h"

#ifdef VPVL2_LINK_VPVL
#include "vpvl2/pmd/Model.h"
#else
#include "vpvl2/pmd2/Model.h"
#endif

namespace vpvl2
{

struct Factory::PrivateContext
{
    PrivateContext(IEncoding *encodingRef)
        : encoding(encodingRef),
          motionPtr(0),
          mvdPtr(0),
          mvdBoneKeyframe(0),
          mvdCameraKeyframe(0),
          mvdLightKeyframe(0),
          mvdMorphKeyframe(0),
          vmdPtr(0),
          vmdBoneKeyframe(0),
          vmdCameraKeyframe(0),
          vmdLightKeyframe(0),
          vmdMorphKeyframe(0)
    {
    }
    ~PrivateContext() {
        delete motionPtr;
        motionPtr = 0;
        delete mvdPtr;
        mvdPtr = 0;
        delete mvdBoneKeyframe;
        mvdBoneKeyframe = 0;
        delete mvdCameraKeyframe;
        mvdCameraKeyframe = 0;
        delete mvdLightKeyframe;
        mvdLightKeyframe = 0;
        delete mvdMorphKeyframe;
        mvdMorphKeyframe = 0;
        delete vmdPtr;
        vmdPtr = 0;
        delete vmdBoneKeyframe;
        vmdBoneKeyframe = 0;
        delete vmdCameraKeyframe;
        vmdCameraKeyframe = 0;
        delete vmdLightKeyframe;
        vmdLightKeyframe = 0;
        delete vmdMorphKeyframe;
        vmdMorphKeyframe = 0;
    }

    mvd::Motion *createMVDFromVMD(vmd::Motion *source) const {
        mvd::Motion *motion = mvdPtr = new mvd::Motion(source->parentModelRef(), encoding);
        mvd::NameListSection *nameList = motion->nameListSection();
        const int nBoneKeyframes = source->countKeyframes(IKeyframe::kBone);
        QuadWord value;
        for (int i = 0; i < nBoneKeyframes; i++) {
            mvd::BoneKeyframe *keyframeTo = mvdBoneKeyframe = new mvd::BoneKeyframe(motion);
            const IBoneKeyframe *keyframeFrom = source->findBoneKeyframeAt(i);
            keyframeTo->setTimeIndex(keyframeFrom->timeIndex());
            keyframeTo->setName(keyframeFrom->name());
            keyframeTo->setLocalPosition(keyframeFrom->localPosition());
            keyframeTo->setLocalRotation(keyframeFrom->localRotation());
            keyframeTo->setDefaultInterpolationParameter();
            keyframeFrom->getInterpolationParameter(IBoneKeyframe::kX, value);
            keyframeTo->setInterpolationParameter(IBoneKeyframe::kX, value);
            keyframeFrom->getInterpolationParameter(IBoneKeyframe::kY, value);
            keyframeTo->setInterpolationParameter(IBoneKeyframe::kY, value);
            keyframeFrom->getInterpolationParameter(IBoneKeyframe::kZ, value);
            keyframeTo->setInterpolationParameter(IBoneKeyframe::kZ, value);
            keyframeFrom->getInterpolationParameter(IBoneKeyframe::kRotation, value);
            keyframeTo->setInterpolationParameter(IBoneKeyframe::kRotation, value);
            motion->addKeyframe(keyframeTo);
        }
        const int nCameraKeyframes = source->countKeyframes(IKeyframe::kCamera);
        for (int i = 0; i < nCameraKeyframes; i++) {
            mvd::CameraKeyframe *keyframeTo = mvdCameraKeyframe = new mvd::CameraKeyframe(motion);
            const ICameraKeyframe *keyframeFrom = source->findCameraKeyframeAt(i);
            keyframeTo->setTimeIndex(keyframeFrom->timeIndex());
            keyframeTo->setLookAt(keyframeFrom->lookAt());
            keyframeTo->setAngle(keyframeFrom->angle());
            keyframeTo->setFov(keyframeFrom->fov());
            keyframeTo->setDistance(keyframeFrom->distance());
            keyframeTo->setPerspective(keyframeFrom->isPerspective());
            keyframeTo->setDefaultInterpolationParameter();
            keyframeFrom->getInterpolationParameter(ICameraKeyframe::kX, value);
            keyframeTo->setInterpolationParameter(ICameraKeyframe::kX, value);
            keyframeFrom->getInterpolationParameter(ICameraKeyframe::kRotation, value);
            keyframeTo->setInterpolationParameter(ICameraKeyframe::kRotation, value);
            keyframeFrom->getInterpolationParameter(ICameraKeyframe::kFov, value);
            keyframeTo->setInterpolationParameter(ICameraKeyframe::kFov, value);
            keyframeFrom->getInterpolationParameter(ICameraKeyframe::kDistance, value);
            keyframeTo->setInterpolationParameter(ICameraKeyframe::kDistance, value);
            motion->addKeyframe(keyframeTo);
        }
        const int nLightKeyframes = source->countKeyframes(IKeyframe::kLight);
        for (int i = 0; i < nLightKeyframes; i++) {
            mvd::LightKeyframe *keyframeTo = mvdLightKeyframe = new mvd::LightKeyframe(motion);
            const ILightKeyframe *keyframeFrom = source->findLightKeyframeAt(i);
            keyframeTo->setTimeIndex(keyframeFrom->timeIndex());
            keyframeTo->setColor(keyframeFrom->color());
            keyframeTo->setDirection(keyframeFrom->direction());
            keyframeTo->setEnable(true);
            motion->addKeyframe(keyframeTo);
        }
        const int nMorphKeyframes = source->countKeyframes(IKeyframe::kMorph);
        for (int i = 0; i < nMorphKeyframes; i++) {
            mvd::MorphKeyframe *keyframeTo = mvdMorphKeyframe = new mvd::MorphKeyframe(motion);
            const IMorphKeyframe *keyframeFrom = source->findMorphKeyframeAt(i);
            keyframeTo->setTimeIndex(keyframeFrom->timeIndex());
            keyframeTo->setName(keyframeFrom->name());
            keyframeTo->setWeight(keyframeFrom->weight());
            keyframeTo->setDefaultInterpolationParameter();
            motion->addKeyframe(keyframeTo);
        }
        mvdBoneKeyframe = 0;
        mvdCameraKeyframe = 0;
        mvdLightKeyframe = 0;
        mvdMorphKeyframe = 0;
        mvdPtr = 0;
        return motion;
    }
    vmd::Motion *createVMDFromMVD(mvd::Motion *source) const {
        vmd::Motion *motion = vmdPtr = new vmd::Motion(source->parentModelRef(), encoding);
        const int nBoneKeyframes = source->countKeyframes(IKeyframe::kBone);
        QuadWord value;
        for (int i = 0; i < nBoneKeyframes; i++) {
            vmd::BoneKeyframe *keyframeTo = vmdBoneKeyframe = new vmd::BoneKeyframe(encoding);
            const IBoneKeyframe *keyframeFrom = source->findBoneKeyframeAt(i);
            keyframeTo->setTimeIndex(keyframeFrom->timeIndex());
            keyframeTo->setName(keyframeFrom->name());
            keyframeTo->setLocalPosition(keyframeFrom->localPosition());
            keyframeTo->setLocalRotation(keyframeFrom->localRotation());
            keyframeTo->setDefaultInterpolationParameter();
            keyframeFrom->getInterpolationParameter(IBoneKeyframe::kX, value);
            keyframeTo->setInterpolationParameter(IBoneKeyframe::kX, value);
            keyframeFrom->getInterpolationParameter(IBoneKeyframe::kY, value);
            keyframeTo->setInterpolationParameter(IBoneKeyframe::kY, value);
            keyframeFrom->getInterpolationParameter(IBoneKeyframe::kZ, value);
            keyframeTo->setInterpolationParameter(IBoneKeyframe::kZ, value);
            keyframeFrom->getInterpolationParameter(IBoneKeyframe::kRotation, value);
            keyframeTo->setInterpolationParameter(IBoneKeyframe::kRotation, value);
            motion->addKeyframe(keyframeTo);
        }
        const int nCameraKeyframes = source->countKeyframes(IKeyframe::kCamera);
        for (int i = 0; i < nCameraKeyframes; i++) {
            vmd::CameraKeyframe *keyframeTo = vmdCameraKeyframe = new vmd::CameraKeyframe();
            const ICameraKeyframe *keyframeFrom = source->findCameraKeyframeAt(i);
            keyframeTo->setTimeIndex(keyframeFrom->timeIndex());
            keyframeTo->setLookAt(keyframeFrom->lookAt());
            keyframeTo->setAngle(keyframeFrom->angle());
            keyframeTo->setFov(keyframeFrom->fov());
            keyframeTo->setDistance(keyframeFrom->distance());
            keyframeTo->setPerspective(keyframeFrom->isPerspective());
            keyframeTo->setDefaultInterpolationParameter();
            keyframeFrom->getInterpolationParameter(ICameraKeyframe::kX, value);
            keyframeTo->setInterpolationParameter(ICameraKeyframe::kX, value);
            keyframeTo->setInterpolationParameter(ICameraKeyframe::kY, value);
            keyframeTo->setInterpolationParameter(ICameraKeyframe::kZ, value);
            keyframeFrom->getInterpolationParameter(ICameraKeyframe::kRotation, value);
            keyframeTo->setInterpolationParameter(ICameraKeyframe::kRotation, value);
            keyframeFrom->getInterpolationParameter(ICameraKeyframe::kFov, value);
            keyframeTo->setInterpolationParameter(ICameraKeyframe::kFov, value);
            keyframeFrom->getInterpolationParameter(ICameraKeyframe::kDistance, value);
            keyframeTo->setInterpolationParameter(ICameraKeyframe::kDistance, value);
            motion->addKeyframe(keyframeTo);
        }
        const int nLightKeyframes = source->countKeyframes(IKeyframe::kLight);
        for (int i = 0; i < nLightKeyframes; i++) {
            vmd::LightKeyframe *keyframeTo = vmdLightKeyframe = new vmd::LightKeyframe();
            const ILightKeyframe *keyframeFrom = source->findLightKeyframeAt(i);
            keyframeTo->setTimeIndex(keyframeFrom->timeIndex());
            keyframeTo->setColor(keyframeFrom->color());
            keyframeTo->setDirection(keyframeFrom->direction());
            motion->addKeyframe(keyframeTo);
        }
        /* TODO: interpolation */
        const int nMorphKeyframes = source->countKeyframes(IKeyframe::kMorph);
        for (int i = 0; i < nMorphKeyframes; i++) {
            vmd::MorphKeyframe *keyframeTo = vmdMorphKeyframe = new vmd::MorphKeyframe(encoding);
            const IMorphKeyframe *keyframeFrom = source->findMorphKeyframeAt(i);
            keyframeTo->setTimeIndex(keyframeFrom->timeIndex());
            keyframeTo->setName(keyframeFrom->name());
            keyframeTo->setWeight(keyframeFrom->weight());
            motion->addKeyframe(keyframeTo);
        }
        vmdBoneKeyframe = 0;
        vmdCameraKeyframe = 0;
        vmdLightKeyframe = 0;
        vmdMorphKeyframe = 0;
        vmdPtr = 0;
        return motion;
    }

    IEncoding *encoding;
    IMotion *motionPtr;
    mutable mvd::Motion *mvdPtr;
    mutable mvd::BoneKeyframe *mvdBoneKeyframe;
    mutable mvd::CameraKeyframe *mvdCameraKeyframe;
    mutable mvd::LightKeyframe *mvdLightKeyframe;
    mutable mvd::MorphKeyframe *mvdMorphKeyframe;
    mutable vmd::Motion *vmdPtr;
    mutable vmd::BoneKeyframe *vmdBoneKeyframe;
    mutable vmd::CameraKeyframe *vmdCameraKeyframe;
    mutable vmd::LightKeyframe *vmdLightKeyframe;
    mutable vmd::MorphKeyframe *vmdMorphKeyframe;
};

IModel::Type Factory::findModelType(const uint8_t *data, size_t size)
{
    if (size >= 4 && memcmp(data, "PMX ", 4) == 0) {
        return IModel::kPMX;
    }
    else if (size >= 3 && memcmp(data, "Pmd", 3) == 0) {
        return IModel::kPMD;
    }
    else {
        return IModel::kAsset;
    }
}

IMotion::Type Factory::findMotionType(const uint8_t *data, size_t size)
{
    if (size >= sizeof(vmd::Motion::kSignature) &&
            memcmp(data, vmd::Motion::kSignature, sizeof(vmd::Motion::kSignature) - 1) == 0) {
        return IMotion::kVMD;
    }
    else if (size >= sizeof(mvd::Motion::kSignature) &&
             memcmp(data, mvd::Motion::kSignature, sizeof(mvd::Motion::kSignature) - 1) == 0) {
        return IMotion::kMVD;
    }
    else {
        return IMotion::kUnknown;
    }
}

Factory::Factory(IEncoding *encoding)
    : m_context(0)
{
    m_context = new PrivateContext(encoding);
}

Factory::~Factory()
{
    delete m_context;
    m_context = 0;
}

IModel *Factory::createModel(IModel::Type type) const
{
    switch (type) {
    case IModel::kAsset:
        return new asset::Model(m_context->encoding);
    case IModel::kPMD:
#ifdef VPVL2_LINK_VPVL
        return new pmd::Model(m_context->encoding);
#else
        return new pmd2::Model(m_context->encoding);
#endif
    case IModel::kPMX:
        return new pmx::Model(m_context->encoding);
    default:
        return 0;
    }
}

IModel *Factory::createModel(const uint8_t *data, size_t size, bool &ok) const
{
    IModel *model = createModel(findModelType(data, size));
    ok = model ? model->load(data, size) : false;
    return model;
}

IMotion *Factory::createMotion(IMotion::Type type, IModel *modelRef) const
{
    switch (type) {
    case IMotion::kVMD:
        return new vmd::Motion(modelRef, m_context->encoding);
    case IMotion::kMVD:
        return new mvd::Motion(modelRef, m_context->encoding);
    default:
        return 0;
    }
}

IMotion *Factory::createMotion(const uint8_t *data, size_t size, IModel *model, bool &ok) const
{
    IMotion *motion = createMotion(findMotionType(data, size), model);
    ok = motion ? motion->load(data, size) : false;
    return motion;
}

IBoneKeyframe *Factory::createBoneKeyframe(const IMotion *motion) const
{
    if (motion) {
        switch (motion->type()) {
        case IMotion::kMVD:
            return new mvd::BoneKeyframe(static_cast<const mvd::Motion *>(motion));
        case IMotion::kVMD:
            return new vmd::BoneKeyframe(m_context->encoding);
        default:
            break;
        }
    }
    return 0;
}

ICameraKeyframe *Factory::createCameraKeyframe(const IMotion *motion) const
{
    if (motion) {
        switch (motion->type()) {
        case IMotion::kMVD:
            return new mvd::CameraKeyframe(static_cast<const mvd::Motion *>(motion));
        case IMotion::kVMD:
            return new vmd::CameraKeyframe();
        default:
            break;
        }
    }
    return 0;
}

IEffectKeyframe *Factory::createEffectKeyframe(const IMotion *motion) const
{
    if (motion) {
        switch (motion->type()) {
        case IMotion::kMVD:
            return new mvd::EffectKeyframe(static_cast<const mvd::Motion *>(motion));
        default:
            break;
        }
    }
    return 0;
}

ILightKeyframe *Factory::createLightKeyframe(const IMotion *motion) const
{
    if (motion) {
        switch (motion->type()) {
        case IMotion::kMVD:
            return new mvd::LightKeyframe(static_cast<const mvd::Motion *>(motion));
        case IMotion::kVMD:
            return new vmd::LightKeyframe();
        default:
            break;
        }
    }
    return 0;
}

IModelKeyframe *Factory::createModelKeyframe(const IMotion *motion) const
{
    if (motion) {
        switch (motion->type()) {
        case IMotion::kMVD:
            return new mvd::ModelKeyframe(static_cast<const mvd::Motion *>(motion), 0);
        default:
            break;
        }
    }
    return 0;
}

IMorphKeyframe *Factory::createMorphKeyframe(const IMotion *motion) const
{
    if (motion) {
        switch (motion->type()) {
        case IMotion::kMVD:
            return new mvd::MorphKeyframe(static_cast<const mvd::Motion *>(motion));
        case IMotion::kVMD:
            return new vmd::MorphKeyframe(m_context->encoding);
        default:
            break;
        }
    }
    return 0;
}

IProjectKeyframe *Factory::createProjectKeyframe(const IMotion *motion) const
{
    if (motion) {
        switch (motion->type()) {
        case IMotion::kMVD:
            return new mvd::ProjectKeyframe(static_cast<const mvd::Motion *>(motion));
        default:
            break;
        }
    }
    return 0;
}

IMotion *Factory::convertMotion(IMotion *source, IMotion::Type destType) const
{
    if (source) {
        IMotion::Type sourceType = source->type();
        if (sourceType == destType) {
            return source->clone();
        }
        else if (sourceType == IMotion::kVMD && destType == IMotion::kMVD) {
            return m_context->createMVDFromVMD(static_cast<vmd::Motion *>(source));
        }
        else if (sourceType == IMotion::kMVD && destType == IMotion::kVMD) {
            return m_context->createVMDFromMVD(static_cast<mvd::Motion *>(source));
        }
    }
    return 0;
}

}
