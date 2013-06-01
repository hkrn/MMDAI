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

namespace {

using namespace vpvl2;

class NullBone : public IBone {
public:
    static inline IBone *sharedReference() {
        static NullBone bone;
        return &bone;
    }

    const IString *name() const { return 0; }
    int index() const { return -1; }
    IModel *parentModelRef() const { return 0; }
    IBone *parentBoneRef() const { return 0; }
    IBone *effectorBoneRef() const { return 0; }
    Transform worldTransform() const {  return Transform::getIdentity(); }
    Transform localTransform() const {  return Transform::getIdentity(); }
    void getLocalTransform(Transform &world2LocalTransform) const {
        world2LocalTransform = Transform::getIdentity();
    }
    void setLocalTransform(const Transform & /* value */) {}
    Vector3 origin() const { return kZeroV3; }
    Vector3 destinationOrigin() const { return kZeroV3; }
    Vector3 localTranslation() const { return kZeroV3; }
    Quaternion localRotation() const { return Quaternion::getIdentity(); }
    void getEffectorBones(Array<IBone *> & /* value */) const {}
    void setLocalTranslation(const Vector3 & /* value */) {}
    void setLocalRotation(const Quaternion & /* value */) {}
    bool isMovable() const { return false; }
    bool isRotateable() const { return false; }
    bool isVisible() const { return false; }
    bool isInteractive() const { return false; }
    bool hasInverseKinematics() const { return false; }
    bool hasFixedAxes() const { return false; }
    bool hasLocalAxes() const { return false; }
    Vector3 fixedAxis() const { return kZeroV3; }
    void getLocalAxes(Matrix3x3 & /* value */) const {}
    void setInverseKinematicsEnable(bool /* value */) {}
    bool isInverseKinematicsEnabled() const { return false; }
private:
    NullBone() {}
    ~NullBone() {}
};

class NullMaterial : public IMaterial {
public:
    static const Color kWhiteColor;
    static inline IMaterial *sharedReference() {
        static NullMaterial material;
        return &material;
    }

    IModel *parentModelRef() const { return 0; }
    const IString *name() const { return 0; }
    const IString *englishName() const { return 0; }
    const IString *userDataArea() const { return 0; }
    const IString *mainTexture() const { return 0; }
    const IString *sphereTexture() const { return 0; }
    const IString *toonTexture() const { return 0; }
    SphereTextureRenderMode sphereTextureRenderMode() const { return kNone; }
    Color ambient() const { return kZeroC; }
    Color diffuse() const { return kZeroC; }
    Color specular() const { return kZeroC; }
    Color edgeColor() const { return kZeroC; }
    Color mainTextureBlend() const { return kWhiteColor; }
    Color sphereTextureBlend() const { return kWhiteColor; }
    Color toonTextureBlend() const { return kWhiteColor; }
    IndexRange indexRange() const { return IndexRange(); }
    float32_t shininess() const { return 0; }
    IVertex::EdgeSizePrecision edgeSize() const { return 1; }
    int index() const { return -1; }
    int textureIndex() const { return -1; }
    int sphereTextureIndex() const { return -1; }
    int toonTextureIndex() const { return -1; }
    int sizeofIndices() const { return 0; }
    bool isSharedToonTextureUsed() const { return false; }
    bool isCullingDisabled() const { return true; }
    bool hasShadow() const { return false; }
    bool hasShadowMap() const { return false; }
    bool isSelfShadowEnabled() const { return hasShadowMap(); }
    bool isEdgeEnabled() const { return false; }

    void setName(const IString * /* value */) {}
    void setEnglishName(const IString * /* value */) {}
    void setUserDataArea(const IString * /* value */) {}
    void setMainTexture(const IString * /* value */) {}
    void setSphereTexture(const IString * /* value */) {}
    void setToonTexture(const IString * /* value */) {}
    void setSphereTextureRenderMode(SphereTextureRenderMode /* value */) {}
    void setAmbient(const Color & /* value */) {}
    void setDiffuse(const Color & /* value */) {}
    void setSpecular(const Color & /* value */) {}
    void setEdgeColor(const Color & /* value */) {}
    void setIndexRange(const IndexRange & /* value */) {}
    void setShininess(float32_t /* value */) {}
    void setEdgeSize(const IVertex::EdgeSizePrecision & /* value */) {}
    void setMainTextureIndex(int /* value */) {}
    void setSphereTextureIndex(int /* value */) {}
    void setToonTextureIndex(int /* value */) {}
    void setIndices(int /* value */) {}
    void setFlags(int /* value */) {}

private:
    NullMaterial() {}
    ~NullMaterial() {}
};
const Color NullMaterial::kWhiteColor = Color(1, 1, 1, 1);

}

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
        const int nBoneKeyframes = source->countKeyframes(IKeyframe::kBoneKeyframe);
        QuadWord value;
        Array<IKeyframe *> boneKeyframes, cameraKeyframes, lightKeyframes, morphKeyframes;
        boneKeyframes.reserve(nBoneKeyframes);
        for (int i = 0; i < nBoneKeyframes; i++) {
            mvd::BoneKeyframe *keyframeTo = mvdBoneKeyframe = new mvd::BoneKeyframe(motion);
            const IBoneKeyframe *keyframeFrom = source->findBoneKeyframeRefAt(i);
            keyframeTo->setTimeIndex(keyframeFrom->timeIndex());
            keyframeTo->setName(keyframeFrom->name());
            keyframeTo->setLocalTranslation(keyframeFrom->localTranslation());
            keyframeTo->setLocalRotation(keyframeFrom->localRotation());
            keyframeTo->setDefaultInterpolationParameter();
            keyframeFrom->getInterpolationParameter(IBoneKeyframe::kBonePositionX, value);
            keyframeTo->setInterpolationParameter(IBoneKeyframe::kBonePositionX, value);
            keyframeFrom->getInterpolationParameter(IBoneKeyframe::kBonePositionY, value);
            keyframeTo->setInterpolationParameter(IBoneKeyframe::kBonePositionY, value);
            keyframeFrom->getInterpolationParameter(IBoneKeyframe::kBonePositionZ, value);
            keyframeTo->setInterpolationParameter(IBoneKeyframe::kBonePositionZ, value);
            keyframeFrom->getInterpolationParameter(IBoneKeyframe::kBoneRotation, value);
            keyframeTo->setInterpolationParameter(IBoneKeyframe::kBoneRotation, value);
            boneKeyframes.append(keyframeTo);
        }
        motion->setAllKeyframes(boneKeyframes, IKeyframe::kBoneKeyframe);
        const int nCameraKeyframes = source->countKeyframes(IKeyframe::kCameraKeyframe);
        cameraKeyframes.resize(nCameraKeyframes);
        for (int i = 0; i < nCameraKeyframes; i++) {
            mvd::CameraKeyframe *keyframeTo = mvdCameraKeyframe = new mvd::CameraKeyframe(motion);
            const ICameraKeyframe *keyframeFrom = source->findCameraKeyframeRefAt(i);
            keyframeTo->setTimeIndex(keyframeFrom->timeIndex());
            keyframeTo->setLookAt(keyframeFrom->lookAt());
            keyframeTo->setAngle(keyframeFrom->angle());
            keyframeTo->setFov(keyframeFrom->fov());
            keyframeTo->setDistance(keyframeFrom->distance());
            keyframeTo->setPerspective(keyframeFrom->isPerspective());
            keyframeTo->setDefaultInterpolationParameter();
            keyframeFrom->getInterpolationParameter(ICameraKeyframe::kCameraLookAtX, value);
            keyframeTo->setInterpolationParameter(ICameraKeyframe::kCameraLookAtX, value);
            keyframeFrom->getInterpolationParameter(ICameraKeyframe::kCameraAngle, value);
            keyframeTo->setInterpolationParameter(ICameraKeyframe::kCameraAngle, value);
            keyframeFrom->getInterpolationParameter(ICameraKeyframe::kCameraFov, value);
            keyframeTo->setInterpolationParameter(ICameraKeyframe::kCameraFov, value);
            keyframeFrom->getInterpolationParameter(ICameraKeyframe::kCameraDistance, value);
            keyframeTo->setInterpolationParameter(ICameraKeyframe::kCameraDistance, value);
            cameraKeyframes.append(keyframeTo);
        }
        motion->setAllKeyframes(cameraKeyframes, IKeyframe::kCameraKeyframe);
        const int nLightKeyframes = source->countKeyframes(IKeyframe::kLightKeyframe);
        lightKeyframes.reserve(nLightKeyframes);
        for (int i = 0; i < nLightKeyframes; i++) {
            mvd::LightKeyframe *keyframeTo = mvdLightKeyframe = new mvd::LightKeyframe(motion);
            const ILightKeyframe *keyframeFrom = source->findLightKeyframeRefAt(i);
            keyframeTo->setTimeIndex(keyframeFrom->timeIndex());
            keyframeTo->setColor(keyframeFrom->color());
            keyframeTo->setDirection(keyframeFrom->direction());
            keyframeTo->setEnable(true);
            lightKeyframes.append(keyframeTo);
        }
        motion->setAllKeyframes(lightKeyframes, IKeyframe::kLightKeyframe);
        const int nMorphKeyframes = source->countKeyframes(IKeyframe::kMorphKeyframe);
        morphKeyframes.reserve(nMorphKeyframes);
        for (int i = 0; i < nMorphKeyframes; i++) {
            mvd::MorphKeyframe *keyframeTo = mvdMorphKeyframe = new mvd::MorphKeyframe(motion);
            const IMorphKeyframe *keyframeFrom = source->findMorphKeyframeRefAt(i);
            keyframeTo->setTimeIndex(keyframeFrom->timeIndex());
            keyframeTo->setName(keyframeFrom->name());
            keyframeTo->setWeight(keyframeFrom->weight());
            keyframeTo->setDefaultInterpolationParameter();
            morphKeyframes.append(keyframeTo);
        }
        motion->setAllKeyframes(morphKeyframes, IKeyframe::kMorphKeyframe);
        mvdBoneKeyframe = 0;
        mvdCameraKeyframe = 0;
        mvdLightKeyframe = 0;
        mvdMorphKeyframe = 0;
        mvdPtr = 0;
        return motion;
    }
    vmd::Motion *createVMDFromMVD(mvd::Motion *source) const {
        vmd::Motion *motion = vmdPtr = new vmd::Motion(source->parentModelRef(), encoding);
        const int nBoneKeyframes = source->countKeyframes(IKeyframe::kBoneKeyframe);
        QuadWord value;
        Array<IKeyframe *> boneKeyframes, cameraKeyframes, lightKeyframes, morphKeyframes;
        boneKeyframes.reserve(nBoneKeyframes);
        for (int i = 0; i < nBoneKeyframes; i++) {
            vmd::BoneKeyframe *keyframeTo = vmdBoneKeyframe = new vmd::BoneKeyframe(encoding);
            const IBoneKeyframe *keyframeFrom = source->findBoneKeyframeRefAt(i);
            keyframeTo->setTimeIndex(keyframeFrom->timeIndex());
            keyframeTo->setName(keyframeFrom->name());
            keyframeTo->setLocalTranslation(keyframeFrom->localTranslation());
            keyframeTo->setLocalRotation(keyframeFrom->localRotation());
            keyframeTo->setDefaultInterpolationParameter();
            keyframeFrom->getInterpolationParameter(IBoneKeyframe::kBonePositionX, value);
            keyframeTo->setInterpolationParameter(IBoneKeyframe::kBonePositionX, value);
            keyframeFrom->getInterpolationParameter(IBoneKeyframe::kBonePositionY, value);
            keyframeTo->setInterpolationParameter(IBoneKeyframe::kBonePositionY, value);
            keyframeFrom->getInterpolationParameter(IBoneKeyframe::kBonePositionZ, value);
            keyframeTo->setInterpolationParameter(IBoneKeyframe::kBonePositionZ, value);
            keyframeFrom->getInterpolationParameter(IBoneKeyframe::kBoneRotation, value);
            keyframeTo->setInterpolationParameter(IBoneKeyframe::kBoneRotation, value);
            boneKeyframes.append(keyframeTo);
        }
        motion->setAllKeyframes(boneKeyframes, IKeyframe::kBoneKeyframe);
        const int nCameraKeyframes = source->countKeyframes(IKeyframe::kCameraKeyframe);
        cameraKeyframes.reserve(nCameraKeyframes);
        for (int i = 0; i < nCameraKeyframes; i++) {
            vmd::CameraKeyframe *keyframeTo = vmdCameraKeyframe = new vmd::CameraKeyframe();
            const ICameraKeyframe *keyframeFrom = source->findCameraKeyframeRefAt(i);
            keyframeTo->setTimeIndex(keyframeFrom->timeIndex());
            keyframeTo->setLookAt(keyframeFrom->lookAt());
            keyframeTo->setAngle(keyframeFrom->angle());
            keyframeTo->setFov(keyframeFrom->fov());
            keyframeTo->setDistance(keyframeFrom->distance());
            keyframeTo->setPerspective(keyframeFrom->isPerspective());
            keyframeTo->setDefaultInterpolationParameter();
            keyframeFrom->getInterpolationParameter(ICameraKeyframe::kCameraLookAtX, value);
            keyframeTo->setInterpolationParameter(ICameraKeyframe::kCameraLookAtX, value);
            keyframeTo->setInterpolationParameter(ICameraKeyframe::kCameraLookAtY, value);
            keyframeTo->setInterpolationParameter(ICameraKeyframe::kCameraLookAtZ, value);
            keyframeFrom->getInterpolationParameter(ICameraKeyframe::kCameraAngle, value);
            keyframeTo->setInterpolationParameter(ICameraKeyframe::kCameraAngle, value);
            keyframeFrom->getInterpolationParameter(ICameraKeyframe::kCameraFov, value);
            keyframeTo->setInterpolationParameter(ICameraKeyframe::kCameraFov, value);
            keyframeFrom->getInterpolationParameter(ICameraKeyframe::kCameraDistance, value);
            keyframeTo->setInterpolationParameter(ICameraKeyframe::kCameraDistance, value);
            cameraKeyframes.append(keyframeTo);
        }
        motion->setAllKeyframes(cameraKeyframes, IKeyframe::kCameraKeyframe);
        const int nLightKeyframes = source->countKeyframes(IKeyframe::kLightKeyframe);
        lightKeyframes.reserve(nLightKeyframes);
        for (int i = 0; i < nLightKeyframes; i++) {
            vmd::LightKeyframe *keyframeTo = vmdLightKeyframe = new vmd::LightKeyframe();
            const ILightKeyframe *keyframeFrom = source->findLightKeyframeRefAt(i);
            keyframeTo->setTimeIndex(keyframeFrom->timeIndex());
            keyframeTo->setColor(keyframeFrom->color());
            keyframeTo->setDirection(keyframeFrom->direction());
            lightKeyframes.append(keyframeTo);
        }
        motion->setAllKeyframes(lightKeyframes, IKeyframe::kLightKeyframe);
        /* TODO: interpolation */
        const int nMorphKeyframes = source->countKeyframes(IKeyframe::kMorphKeyframe);
        morphKeyframes.reserve(nMorphKeyframes);
        for (int i = 0; i < nMorphKeyframes; i++) {
            vmd::MorphKeyframe *keyframeTo = vmdMorphKeyframe = new vmd::MorphKeyframe(encoding);
            const IMorphKeyframe *keyframeFrom = source->findMorphKeyframeRefAt(i);
            keyframeTo->setTimeIndex(keyframeFrom->timeIndex());
            keyframeTo->setName(keyframeFrom->name());
            keyframeTo->setWeight(keyframeFrom->weight());
            morphKeyframes.append(keyframeTo);
        }
        motion->setAllKeyframes(morphKeyframes, IKeyframe::kMorphKeyframe);
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
        return IModel::kPMXModel;
    }
    else if (size >= 3 && memcmp(data, "Pmd", 3) == 0) {
        return IModel::kPMDModel;
    }
    else {
        return IModel::kAssetModel;
    }
}

IMotion::Type Factory::findMotionType(const uint8_t *data, size_t size)
{
    if (size >= sizeof(vmd::Motion::kSignature) &&
            memcmp(data, vmd::Motion::kSignature, sizeof(vmd::Motion::kSignature) - 1) == 0) {
        return IMotion::kVMDMotion;
    }
    else if (size >= sizeof(mvd::Motion::kSignature) &&
             memcmp(data, mvd::Motion::kSignature, sizeof(mvd::Motion::kSignature) - 1) == 0) {
        return IMotion::kMVDMotion;
    }
    else {
        return IMotion::kUnknownMotion;
    }
}

IBone *Factory::sharedNullBoneRef()
{
    return NullBone::sharedReference();
}

IMaterial *Factory::sharedNullMaterialRef()
{
    return NullMaterial::sharedReference();
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

IModel *Factory::newModel(IModel::Type type) const
{
    switch (type) {
    case IModel::kAssetModel:
        return new asset::Model(m_context->encoding);
    case IModel::kPMDModel:
#ifdef VPVL2_LINK_VPVL
        return new pmd::Model(m_context->encoding);
#else
        return new pmd2::Model(m_context->encoding);
#endif
    case IModel::kPMXModel:
        return new pmx::Model(m_context->encoding);
    default:
        return 0;
    }
}

IModel *Factory::createModel(const uint8_t *data, size_t size, bool &ok) const
{
    IModel *model = newModel(findModelType(data, size));
    ok = model ? model->load(data, size) : false;
    return model;
}

IMotion *Factory::newMotion(IMotion::Type type, IModel *modelRef) const
{
    switch (type) {
    case IMotion::kVMDMotion:
        return new vmd::Motion(modelRef, m_context->encoding);
    case IMotion::kMVDMotion:
        return new mvd::Motion(modelRef, m_context->encoding);
    default:
        return 0;
    }
}

IMotion *Factory::createMotion(const uint8_t *data, size_t size, IModel *model, bool &ok) const
{
    IMotion *motion = newMotion(findMotionType(data, size), model);
    ok = motion ? motion->load(data, size) : false;
    return motion;
}

IBoneKeyframe *Factory::createBoneKeyframe(const IMotion *motion) const
{
    if (motion) {
        switch (motion->type()) {
        case IMotion::kMVDMotion:
            return new mvd::BoneKeyframe(static_cast<const mvd::Motion *>(motion));
        case IMotion::kVMDMotion:
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
        case IMotion::kMVDMotion:
            return new mvd::CameraKeyframe(static_cast<const mvd::Motion *>(motion));
        case IMotion::kVMDMotion:
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
        case IMotion::kMVDMotion:
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
        case IMotion::kMVDMotion:
            return new mvd::LightKeyframe(static_cast<const mvd::Motion *>(motion));
        case IMotion::kVMDMotion:
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
        case IMotion::kMVDMotion:
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
        case IMotion::kMVDMotion:
            return new mvd::MorphKeyframe(static_cast<const mvd::Motion *>(motion));
        case IMotion::kVMDMotion:
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
        case IMotion::kMVDMotion:
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
        else if (sourceType == IMotion::kVMDMotion && destType == IMotion::kMVDMotion) {
            return m_context->createMVDFromVMD(static_cast<vmd::Motion *>(source));
        }
        else if (sourceType == IMotion::kMVDMotion && destType == IMotion::kVMDMotion) {
            return m_context->createVMDFromMVD(static_cast<mvd::Motion *>(source));
        }
    }
    return 0;
}

}
