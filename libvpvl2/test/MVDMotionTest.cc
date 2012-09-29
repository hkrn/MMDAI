#include "Common.h"

#include "vpvl2/pmx/Model.h"
#include "vpvl2/mvd/BoneKeyframe.h"
#include "vpvl2/mvd/BoneSection.h"
#include "vpvl2/mvd/CameraKeyframe.h"
#include "vpvl2/mvd/CameraSection.h"
#include "vpvl2/mvd/EffectKeyframe.h"
#include "vpvl2/mvd/EffectSection.h"
#include "vpvl2/mvd/LightKeyframe.h"
#include "vpvl2/mvd/LightSection.h"
#include "vpvl2/mvd/ModelKeyframe.h"
#include "vpvl2/mvd/ModelSection.h"
#include "vpvl2/mvd/MorphKeyframe.h"
#include "vpvl2/mvd/MorphSection.h"
#include "vpvl2/mvd/Motion.h"
#include "vpvl2/mvd/NameListSection.h"
#include "vpvl2/mvd/ProjectKeyframe.h"
#include "vpvl2/mvd/ProjectSection.h"

#include "mock/Bone.h"
#include "mock/Model.h"
#include "mock/Morph.h"

using namespace vpvl2::pmx;

namespace
{

static void CompareBoneInterpolationMatrix(const QuadWord p[], const mvd::BoneKeyframe &frame)
{
    QuadWord actual, expected = p[0];
    frame.getInterpolationParameter(mvd::BoneKeyframe::kX, actual);
    ASSERT_TRUE(testVector(expected, actual));
    expected = p[1];
    frame.getInterpolationParameter(mvd::BoneKeyframe::kY, actual);
    ASSERT_TRUE(testVector(expected, actual));
    expected = p[2];
    frame.getInterpolationParameter(mvd::BoneKeyframe::kZ, actual);
    ASSERT_TRUE(testVector(expected, actual));
    expected = p[3];
    frame.getInterpolationParameter(mvd::BoneKeyframe::kRotation, actual);
    ASSERT_TRUE(testVector(expected, actual));
}

static void CompareCameraInterpolationMatrix(const QuadWord p[], const mvd::CameraKeyframe &frame)
{
    QuadWord actual, expected = p[2];
    frame.getInterpolationParameter(mvd::CameraKeyframe::kX, actual);
    ASSERT_TRUE(testVector(expected, actual));
    expected = p[3];
    frame.getInterpolationParameter(mvd::CameraKeyframe::kRotation, actual);
    ASSERT_TRUE(testVector(expected, actual));
    expected = p[4];
    frame.getInterpolationParameter(mvd::CameraKeyframe::kDistance, actual);
    ASSERT_TRUE(testVector(expected, actual));
    expected = p[5];
    frame.getInterpolationParameter(mvd::CameraKeyframe::kFov, actual);
    ASSERT_TRUE(testVector(expected, actual));
}

}

TEST(MVDMotionTest, ParseEmpty)
{
    Encoding encoding;
    MockIModel model;
    mvd::Motion motion(&model, &encoding);
    mvd::Motion::DataInfo info;
    // parsing empty should be error
    ASSERT_FALSE(motion.preparse(reinterpret_cast<const uint8_t *>(""), 0, info));
    ASSERT_EQ(mvd::Motion::kInvalidHeaderError, motion.error());
}

TEST(MVDMotionTest, ParseModelMotion)
{
    QFile file("motion.mvd");
    if (file.open(QFile::ReadOnly)) {
        const QByteArray &bytes = file.readAll();
        const uint8_t *data = reinterpret_cast<const uint8_t *>(bytes.constData());
        size_t size = bytes.size();
        Encoding encoding;
        MockIModel model;
        MockIBone bone;
        MockIMorph morph;
        EXPECT_CALL(model, findBone(_)).Times(AnyNumber()).WillRepeatedly(Return(&bone));
        EXPECT_CALL(model, findMorph(_)).Times(AnyNumber()).WillRepeatedly(Return(&morph));
        mvd::Motion motion(&model, &encoding);
        mvd::Motion::DataInfo result;
        // valid model motion should be loaded successfully
        ASSERT_TRUE(motion.preparse(data, size, result));
        ASSERT_TRUE(motion.load(data, size));
        ASSERT_EQ(mvd::Motion::kNoError, motion.error());
    }
}

TEST(MVDMotionTest, ParseCameraMotion)
{
    QFile file("camera.mvd");
    if (file.open(QFile::ReadOnly)) {
        const QByteArray &bytes = file.readAll();
        const uint8_t *data = reinterpret_cast<const uint8_t *>(bytes.constData());
        size_t size = bytes.size();
        Encoding encoding;
        mvd::Motion motion(0, &encoding);
        mvd::Motion::DataInfo result;
        // valid camera motion should be loaded successfully
        ASSERT_TRUE(motion.preparse(data, size, result));
        ASSERT_TRUE(motion.load(data, size));
        ASSERT_EQ(mvd::Motion::kNoError, motion.error());
    }
}

TEST(MVDMotionTest, SaveBoneKeyframe)
{
    Encoding encoding;
    String str("This is test.");
    mvd::NameListSection nameList(&encoding);
    mvd::BoneKeyframe frame(&nameList), newFrame(&nameList);
    Vector3 pos(1, 2, 3);
    Quaternion rot(4, 5, 6, 7);
    // initialize the bone frame to be copied
    frame.setLayerIndex(42);
    frame.setTimeIndex(42);
    frame.setName(&str);
    frame.setPosition(pos);
    frame.setRotation(rot);
    QuadWord px(8, 9, 10, 11),
            py(12, 13, 14, 15),
            pz(16, 17, 18, 19),
            pr(20, 21, 22, 23);
    QuadWord p[] = { px, py, pz, pr };
    frame.setInterpolationParameter(mvd::BoneKeyframe::kX, px);
    frame.setInterpolationParameter(mvd::BoneKeyframe::kY, py);
    frame.setInterpolationParameter(mvd::BoneKeyframe::kZ, pz);
    frame.setInterpolationParameter(mvd::BoneKeyframe::kRotation, pr);
    // write a bone frame to data and read it
    QScopedArrayPointer<uint8_t> ptr(new uint8_t[frame.estimateSize()]);
    frame.write(ptr.data());
    newFrame.read(ptr.data());
    // compare read bone frame
    ASSERT_EQ(0, nameList.key(&str));
    ASSERT_EQ(frame.timeIndex(), newFrame.timeIndex());
    ASSERT_EQ(frame.layerIndex(), newFrame.layerIndex());
    ASSERT_TRUE(testVector(pos, newFrame.position()));
    ASSERT_TRUE(testVector(rot, newFrame.rotation()));
    CompareBoneInterpolationMatrix(p, frame);
    // cloned bone frame shold be copied with deep
    QScopedPointer<IBoneKeyframe> cloned(frame.clone());
    // ASSERT_TRUE(cloned->name()->equals(frame.name()));
    ASSERT_EQ(frame.layerIndex(), cloned->layerIndex());
    ASSERT_EQ(frame.timeIndex(), cloned->timeIndex());
    ASSERT_TRUE(testVector(pos, cloned->position()));
    ASSERT_TRUE(testVector(rot, cloned->rotation()));
    CompareBoneInterpolationMatrix(p, *static_cast<mvd::BoneKeyframe *>(cloned.data()));
}

TEST(MVDMotionTest, SaveCameraKeyframe)
{
    mvd::CameraKeyframe frame, newFrame;
    Vector3 pos(1, 2, 3), angle(4, 5, 6);
    // initialize the camera frame to be copied
    frame.setTimeIndex(42);
    frame.setLayerIndex(42);
    frame.setPosition(pos);
    frame.setAngle(angle);
    frame.setDistance(7);
    frame.setFov(8);
    QuadWord px(9, 10, 11, 12),
            py(13, 14, 15, 16),
            pz(17, 18, 19, 20),
            pr(21, 22, 23, 24),
            pd(25, 26, 27, 28),
            pf(29, 30, 31, 32);
    QuadWord p[] = { px, py, pz, pr, pd, pf };
    frame.setInterpolationParameter(mvd::CameraKeyframe::kX, px);
    frame.setInterpolationParameter(mvd::CameraKeyframe::kY, py);
    frame.setInterpolationParameter(mvd::CameraKeyframe::kZ, pz);
    frame.setInterpolationParameter(mvd::CameraKeyframe::kRotation, pr);
    frame.setInterpolationParameter(mvd::CameraKeyframe::kDistance, pd);
    frame.setInterpolationParameter(mvd::CameraKeyframe::kFov, pf);
    // write a camera frame to data and read it
    QScopedArrayPointer<uint8_t> ptr(new uint8_t[frame.estimateSize()]);
    frame.write(ptr.data());
    newFrame.read(ptr.data());
    ASSERT_EQ(frame.timeIndex(), newFrame.timeIndex());
    ASSERT_EQ(frame.layerIndex(), newFrame.layerIndex());
    ASSERT_TRUE(testVector(frame.position(), newFrame.position()));
    // compare read camera frame
    // for radian and degree calculation
    ASSERT_FLOAT_EQ(newFrame.angle().x(), frame.angle().x());
    ASSERT_FLOAT_EQ(newFrame.angle().y(), frame.angle().y());
    ASSERT_FLOAT_EQ(newFrame.angle().z(), frame.angle().z());
    ASSERT_FLOAT_EQ(newFrame.distance(), frame.distance());
    ASSERT_FLOAT_EQ(newFrame.fov(), frame.fov());
    CompareCameraInterpolationMatrix(p, frame);
    // cloned camera frame shold be copied with deep
    QScopedPointer<ICameraKeyframe> cloned(frame.clone());
    ASSERT_EQ(frame.timeIndex(), cloned->timeIndex());
    ASSERT_EQ(frame.layerIndex(), cloned->layerIndex());
    ASSERT_TRUE(testVector(frame.position(), cloned->position()));
    // for radian and degree calculation
    ASSERT_FLOAT_EQ(cloned->angle().x(), frame.angle().x());
    ASSERT_FLOAT_EQ(cloned->angle().y(), frame.angle().y());
    ASSERT_FLOAT_EQ(cloned->angle().z(), frame.angle().z());
    ASSERT_FLOAT_EQ(cloned->distance(), frame.distance());
    ASSERT_FLOAT_EQ(cloned->fov(), frame.fov());
    CompareCameraInterpolationMatrix(p, *static_cast<mvd::CameraKeyframe *>(cloned.data()));
}

TEST(MVDMotionTest, SaveMorphKeyframe)
{
    Encoding encoding;
    String str("This is a test.");
    mvd::NameListSection nameList(&encoding);
    mvd::MorphKeyframe frame(&nameList), newFrame(&nameList);
    // initialize the morph frame to be copied
    frame.setName(&str);
    frame.setTimeIndex(42);
    frame.setWeight(0.5);
    // write a morph frame to data and read it
    QScopedArrayPointer<uint8_t> ptr(new uint8_t[frame.estimateSize()]);
    frame.write(ptr.data());
    newFrame.read(ptr.data());
    // compare read morph frame
    ASSERT_EQ(0, nameList.key(&str));
    ASSERT_EQ(frame.timeIndex(), newFrame.timeIndex());
    ASSERT_EQ(frame.weight(), newFrame.weight());
    // cloned morph frame shold be copied with deep
    QScopedPointer<IMorphKeyframe> cloned(frame.clone());
    // ASSERT_TRUE(cloned->name()->equals(frame.name()));
    ASSERT_EQ(frame.timeIndex(), cloned->timeIndex());
    ASSERT_EQ(frame.weight(), cloned->weight());
}

/*
TEST(MVDMotionTest, SaveLightKeyframe)
{
    mvd::LightKeyframe frame, newFrame;
    Vector3 color(0.1, 0.2, 0.3), direction(4, 5, 6);
    // initialize the light frame to be copied
    frame.setTimeIndex(42);
    frame.setColor(color);
    frame.setDirection(direction);
    // write a light frame to data and read it
    uint8_t data[mvd::LightKeyframe::strideSize()];
    frame.write(data);
    newFrame.read(data);
    // compare read light frame
    ASSERT_EQ(frame.timeIndex(), newFrame.timeIndex());
    ASSERT_TRUE(newFrame.color() == frame.color());
    ASSERT_TRUE(newFrame.direction() == frame.direction());
    // cloned morph frame shold be copied with deep
    QScopedPointer<ILightKeyframe> cloned(frame.clone());
    ASSERT_EQ(frame.timeIndex(), cloned->timeIndex());
    ASSERT_TRUE(cloned->color() == frame.color());
    ASSERT_TRUE(cloned->direction() == frame.direction());
}
*/

ACTION_P(FindBone, bones)
{
    MockIBone *bone = new MockIBone();
    EXPECT_CALL(*bone, name()).Times(AnyNumber()).WillRepeatedly(Return(arg0));
    bones->data()->append(bone);
    return bone;
}

ACTION_P(FindMorph, morphs)
{
    MockIMorph *morph = new MockIMorph();
    EXPECT_CALL(*morph, name()).Times(AnyNumber()).WillRepeatedly(Return(arg0));
    morphs->data()->append(morph);
    return morph;
}

TEST(MVDMotionTest, SaveModelMotion)
{
    QFile file("motion.mvd");
    if (file.open(QFile::ReadOnly)) {
        const QByteArray &bytes = file.readAll();
        const uint8_t *data = reinterpret_cast<const uint8_t *>(bytes.constData());
        size_t size = bytes.size();
        Encoding encoding;
        MockIModel model;
        QScopedPointer<QList<IBone *>, ScopedPointerListDeleter> bones(new QList<IBone *>());
        QScopedPointer<QList<IMorph *>, ScopedPointerListDeleter> morphs(new QList<IMorph *>());
        EXPECT_CALL(model, findBone(_)).Times(AtLeast(1)).WillRepeatedly(FindBone(&bones));
        EXPECT_CALL(model, findMorph(_)).Times(AtLeast(1)).WillRepeatedly(FindMorph(&morphs));
        mvd::Motion motion(&model, &encoding);
        ASSERT_TRUE(motion.load(data, size));
        size_t newSize = motion.estimateSize();
        //ASSERT_EQ(size, newSize);
        QScopedArrayPointer<uint8_t> newData(new uint8_t[newSize]);
        uint8_t *ptr = newData.data();
        mvd::Motion motion2(&model, &encoding);
        motion.save(ptr);
        ASSERT_TRUE(motion2.load(ptr, newSize));
        ASSERT_EQ(motion.countKeyframes(IKeyframe::kBone), motion2.countKeyframes(IKeyframe::kBone));
        ASSERT_EQ(motion.countKeyframes(IKeyframe::kMorph), motion2.countKeyframes(IKeyframe::kMorph));
        ASSERT_EQ(motion.countKeyframes(IKeyframe::kModel), motion2.countKeyframes(IKeyframe::kModel));
    }
}

TEST(MVDMotionTest, SaveCameraMotion)
{
    QFile file("camera.mvd");
    if (file.open(QFile::ReadOnly)) {
        const QByteArray &bytes = file.readAll();
        const uint8_t *data = reinterpret_cast<const uint8_t *>(bytes.constData());
        size_t size = bytes.size();
        Encoding encoding;
        mvd::Motion motion(0, &encoding);
        ASSERT_TRUE(motion.load(data, size));
        size_t newSize = motion.estimateSize();
        QScopedArrayPointer<uint8_t> newData(new uint8_t[newSize]);
        uint8_t *ptr = newData.data();
        mvd::Motion motion2(0, &encoding);
        motion.save(ptr);
        ASSERT_TRUE(motion2.load(ptr, newSize));
        ASSERT_EQ(motion.countKeyframes(IKeyframe::kCamera), motion2.countKeyframes(IKeyframe::kCamera));
    }
}

TEST(MVDMotionTest, BoneInterpolation)
{
    Encoding encoding;
    mvd::NameListSection nameList(&encoding);
    mvd::BoneKeyframe frame(&nameList);
    QuadWord n;
    frame.getInterpolationParameter(mvd::BoneKeyframe::kX, n);
    ASSERT_TRUE(testVector(mvd::Motion::InterpolationTable::kDefaultParameter, n));
    QuadWord px(8, 9, 10, 11),
            py(12, 13, 14, 15),
            pz(16, 17, 18, 19),
            pr(20, 21, 22, 23);
    QuadWord p[] = { px, py, pz, pr };
    frame.setInterpolationParameter(mvd::BoneKeyframe::kX, px);
    frame.setInterpolationParameter(mvd::BoneKeyframe::kY, py);
    frame.setInterpolationParameter(mvd::BoneKeyframe::kZ, pz);
    frame.setInterpolationParameter(mvd::BoneKeyframe::kRotation, pr);
    CompareBoneInterpolationMatrix(p, frame);
}

TEST(MVDMotionTest, CameraInterpolation)
{
    mvd::CameraKeyframe frame;
    QuadWord n;
    frame.getInterpolationParameter(mvd::CameraKeyframe::kX, n);
    ASSERT_TRUE(testVector(mvd::Motion::InterpolationTable::kDefaultParameter, n));
    QuadWord px(9, 10, 11, 12),
            py(13, 14, 15, 16),
            pz(17, 18, 19, 20),
            pr(21, 22, 23, 24),
            pd(25, 26, 27, 28),
            pf(29, 30, 31, 32);
    QuadWord p[] = { px, py, pz, pr, pd, pf };
    frame.setInterpolationParameter(mvd::CameraKeyframe::kX, px);
    frame.setInterpolationParameter(mvd::CameraKeyframe::kY, py);
    frame.setInterpolationParameter(mvd::CameraKeyframe::kZ, pz);
    frame.setInterpolationParameter(mvd::CameraKeyframe::kRotation, pr);
    frame.setInterpolationParameter(mvd::CameraKeyframe::kDistance, pd);
    frame.setInterpolationParameter(mvd::CameraKeyframe::kFov, pf);
    CompareCameraInterpolationMatrix(p, frame);
}

TEST(MVDMotionTest, AddAndRemoveBoneKeyframes)
{
    Encoding encoding;
    String name("bone");
    MockIModel model;
    MockIBone bone;
    mvd::Motion motion(&model, &encoding);
    ASSERT_EQ(0, motion.countKeyframes(IKeyframe::kBone));
    // mock bone
    EXPECT_CALL(model, findBone(_)).Times(AtLeast(1)).WillRepeatedly(Return(&bone));
    QScopedPointer<IBoneKeyframe> keyframePtr(new mvd::BoneKeyframe(motion.nameListSection()));
    keyframePtr->setTimeIndex(42);
    keyframePtr->setName(&name);
    {
        // add a bone keyframe
        motion.addKeyframe(keyframePtr.data());
        IKeyframe *keyframe = keyframePtr.take();
        ASSERT_EQ(1, motion.countKeyframes(IKeyframe::kBone));
        // boudary check of findBoneKeyframeAt
        ASSERT_EQ(static_cast<IBoneKeyframe *>(0), motion.findBoneKeyframeAt(-1));
        ASSERT_EQ(keyframe, motion.findBoneKeyframeAt(0));
        ASSERT_EQ(static_cast<IBoneKeyframe *>(0), motion.findBoneKeyframeAt(1));
        // layer index 0 must be used
        ASSERT_EQ(1, motion.countLayers(&name, IKeyframe::kBone));
        ASSERT_EQ(0, motion.findMorphKeyframe(42, &name, 1));
        // find a bone keyframe with timeIndex and name
        ASSERT_EQ(keyframe, motion.findBoneKeyframe(42, &name, 0));
    }
    keyframePtr.reset(new mvd::BoneKeyframe(motion.nameListSection()));
    keyframePtr->setTimeIndex(42);
    keyframePtr->setName(&name);
    {
        // replaced bone frame should be one keyframe
        motion.replaceKeyframe(keyframePtr.data());
        IKeyframe *keyframeToDelete = keyframePtr.take();
        ASSERT_EQ(1, motion.countKeyframes(IKeyframe::kBone));
        // no longer be find previous bone keyframe
        ASSERT_EQ(keyframeToDelete, motion.findBoneKeyframe(42, &name, 0));
        // delete bone keyframe and set it null
        motion.deleteKeyframe(keyframeToDelete);
        // bone keyframes should be empty
        ASSERT_EQ(0, motion.countKeyframes(IKeyframe::kBone));
        ASSERT_EQ(static_cast<IBoneKeyframe *>(0), motion.findBoneKeyframe(42, &name, 0));
        ASSERT_EQ(static_cast<IKeyframe *>(0), keyframeToDelete);
    }
}

TEST(MVDMotionTest, AddAndRemoveCameraKeyframes)
{
    Encoding encoding;
    Model model(&encoding);
    mvd::Motion motion(&model, &encoding);
    ASSERT_EQ(0, motion.countKeyframes(IKeyframe::kCamera));
    QScopedPointer<ICameraKeyframe> keyframePtr(new mvd::CameraKeyframe());
    keyframePtr->setTimeIndex(42);
    keyframePtr->setDistance(42);
    {
        // add a camera keyframe
        motion.addKeyframe(keyframePtr.data());
        IKeyframe *keyframe = keyframePtr.take();
        ASSERT_EQ(1, motion.countKeyframes(IKeyframe::kCamera));
        // boudary check of findCameraKeyframeAt
        ASSERT_EQ(static_cast<ICameraKeyframe *>(0), motion.findCameraKeyframeAt(-1));
        ASSERT_EQ(keyframe, motion.findCameraKeyframeAt(0));
        ASSERT_EQ(static_cast<ICameraKeyframe *>(0), motion.findCameraKeyframeAt(1));
        // layer index 0 must be used
        ASSERT_EQ(1, motion.countLayers(0, IKeyframe::kCamera));
        ASSERT_EQ(0, motion.findCameraKeyframe(42, 1));
        // find a camera keyframe with timeIndex
        ASSERT_EQ(keyframe, motion.findCameraKeyframe(42, 0));
    }
    keyframePtr.reset(new mvd::CameraKeyframe());
    keyframePtr->setTimeIndex(42);
    keyframePtr->setDistance(84);
    {
        // replaced camera frame should be one keyframe
        motion.replaceKeyframe(keyframePtr.data());
        IKeyframe *keyframeToDelete = keyframePtr.take();
        ASSERT_EQ(1, motion.countKeyframes(IKeyframe::kCamera));
        // no longer be find previous camera keyframe
        ASSERT_EQ(84.0f, motion.findCameraKeyframe(42, 0)->distance());
        // delete camera keyframe and set it null (don't forget updating motion!)
        motion.deleteKeyframe(keyframeToDelete);
        // camera keyframes should be empty
        ASSERT_EQ(0, motion.countKeyframes(IKeyframe::kCamera));
        ASSERT_EQ(static_cast<ICameraKeyframe *>(0), motion.findCameraKeyframe(42, 0));
        ASSERT_EQ(static_cast<IKeyframe *>(0), keyframeToDelete);
    }
}

/*
TEST(MVDMotionTest, AddAndRemoveLightKeyframes)
{
    Encoding encoding;
    Model model(&encoding);
    mvd::Motion motion(&model, &encoding);
    ASSERT_EQ(0, motion.countKeyframes(IKeyframe::kLight));
    QScopedPointer<ILightKeyframe> frame(new mvd::LightKeyframe());
    frame->setTimeIndex(42);
    frame->setColor(Vector3(1, 0, 0));
    {
        // add a light keyframe (don't forget updating motion!)
        motion.addKeyframe(frame.data());
        motion.update(IKeyframe::kLight);
        ASSERT_EQ(1, motion.countKeyframes(IKeyframe::kLight));
        // boudary check of findLightKeyframeAt
        ASSERT_EQ(static_cast<ILightKeyframe *>(0), motion.findLightKeyframeAt(-1));
        ASSERT_EQ(frame.data(), motion.findLightKeyframeAt(0));
        ASSERT_EQ(static_cast<ILightKeyframe *>(0), motion.findLightKeyframeAt(1));
        // layer index 0 must be used
        ASSERT_EQ(1, motion.countLayers(0, IKeyframe::kLight));
        ASSERT_EQ(0, motion.findLightKeyframe(42, 1));
        // find a light keyframe with timeIndex
        ASSERT_EQ(frame.take(), motion.findLightKeyframe(42, 0));
    }
    QScopedPointer<ILightKeyframe> frame2(new mvd::LightKeyframe());
    frame2->setTimeIndex(42);
    frame2->setColor(Vector3(0, 0, 1));
    {
        // replaced light frame should be one keyframe (don't forget updating motion!)
        motion.replaceKeyframe(frame2.data());
        motion.update(IKeyframe::kLight);
        ASSERT_EQ(1, motion.countKeyframes(IKeyframe::kLight));
        // no longer be find previous light keyframe
        ASSERT_EQ(1.0f, motion.findLightKeyframe(42, 0)->color().z());
    }
    {
        IKeyframe *keyframeToDelete = frame2.take();
        // delete light keyframe and set it null (don't forget updating motion!)
        motion.deleteKeyframe(keyframeToDelete);
        motion.update(IKeyframe::kLight);
        // light keyframes should be empty
        ASSERT_EQ(0, motion.countKeyframes(IKeyframe::kLight));
        ASSERT_EQ(static_cast<ILightKeyframe *>(0), motion.findLightKeyframe(42, 0));
        ASSERT_EQ(static_cast<IKeyframe *>(0), keyframeToDelete);
    }
}
*/

TEST(MVDMotionTest, AddAndRemoveMorphKeyframes)
{
    Encoding encoding;
    String name("morph");
    MockIModel model;
    MockIMorph morph;
    mvd::Motion motion(&model, &encoding);
    ASSERT_EQ(0, motion.countKeyframes(IKeyframe::kMorph));
    // mock morph
    EXPECT_CALL(model, findMorph(_)).Times(AtLeast(1)).WillRepeatedly(Return(&morph));
    QScopedPointer<IMorphKeyframe> keyframePtr(new mvd::MorphKeyframe(motion.nameListSection()));
    keyframePtr->setTimeIndex(42);
    keyframePtr->setName(&name);
    {
        // add a morph keyframe
        motion.addKeyframe(keyframePtr.data());
        IKeyframe *keyframe = keyframePtr.take();
        ASSERT_EQ(1, motion.countKeyframes(IKeyframe::kMorph));
        // boudary check of findMorphKeyframeAt
        ASSERT_EQ(static_cast<IMorphKeyframe *>(0), motion.findMorphKeyframeAt(-1));
        ASSERT_EQ(keyframe, motion.findMorphKeyframeAt(0));
        ASSERT_EQ(static_cast<IMorphKeyframe *>(0), motion.findMorphKeyframeAt(1));
        // layer index 0 must be used
        ASSERT_EQ(1, motion.countLayers(&name, IKeyframe::kMorph));
        ASSERT_EQ(0, motion.findMorphKeyframe(42, &name, 1));
        ASSERT_EQ(keyframe, motion.findMorphKeyframe(42, &name, 0));
    }
    keyframePtr.reset(new mvd::MorphKeyframe(motion.nameListSection()));
    keyframePtr->setTimeIndex(42);
    keyframePtr->setName(&name);
    {
        // replaced morph frame should be one keyframe
        motion.replaceKeyframe(keyframePtr.data());
        IKeyframe *keyframeToDelete = keyframePtr.take();
        ASSERT_EQ(1, motion.countKeyframes(IKeyframe::kMorph));
        // no longer be find previous morph keyframe
        ASSERT_EQ(keyframeToDelete, motion.findMorphKeyframe(42, &name, 0));
        // delete light keyframe and set it null
        motion.deleteKeyframe(keyframeToDelete);
        // morph keyframes should be empty
        ASSERT_EQ(0, motion.countKeyframes(IKeyframe::kMorph));
        ASSERT_EQ(static_cast<IMorphKeyframe *>(0), motion.findMorphKeyframe(42, &name, 0));
        ASSERT_EQ(static_cast<IKeyframe *>(0), keyframeToDelete);
    }
}

TEST(MVDMotionTest, AddAndRemoveNullKeyframe)
{
    /* should happen nothing */
    Encoding encoding;
    MockIModel model;
    IKeyframe *nullKeyframe = 0;
    mvd::Motion motion(&model, &encoding);
    motion.addKeyframe(nullKeyframe);
    motion.replaceKeyframe(nullKeyframe);
    motion.deleteKeyframe(nullKeyframe);
}
