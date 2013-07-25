#include "Common.h"

#include "vpvl2/vpvl2.h"
#include "vpvl2/extensions/icu4c/Encoding.h"
#include "vpvl2/pmx/Model.h"
#include "vpvl2/vmd/BoneAnimation.h"
#include "vpvl2/vmd/BoneKeyframe.h"
#include "vpvl2/vmd/CameraAnimation.h"
#include "vpvl2/vmd/CameraKeyframe.h"
#include "vpvl2/vmd/LightAnimation.h"
#include "vpvl2/vmd/LightKeyframe.h"
#include "vpvl2/vmd/MorphAnimation.h"
#include "vpvl2/vmd/MorphKeyframe.h"
#include "vpvl2/vmd/Motion.h"

#include "mock/Bone.h"
#include "mock/Model.h"
#include "mock/Morph.h"

using namespace ::testing;
using namespace vpvl2;
using namespace vpvl2::extensions::icu4c;
using namespace vpvl2::pmx;

namespace
{

const char *kTestString = "012345678901234";

static void CompareBoneInterpolationMatrix(const QuadWord p[], const vmd::BoneKeyframe &frame)
{
    QuadWord actual, expected = p[0];
    frame.getInterpolationParameter(vmd::BoneKeyframe::kBonePositionX, actual);
    ASSERT_TRUE(CompareVector(expected, actual));
    expected = p[1];
    frame.getInterpolationParameter(vmd::BoneKeyframe::kBonePositionY, actual);
    ASSERT_TRUE(CompareVector(expected, actual));
    expected = p[2];
    frame.getInterpolationParameter(vmd::BoneKeyframe::kBonePositionZ, actual);
    ASSERT_TRUE(CompareVector(expected, actual));
    expected = p[3];
    frame.getInterpolationParameter(vmd::BoneKeyframe::kBoneRotation, actual);
    ASSERT_TRUE(CompareVector(expected, actual));
}

static void CompareCameraInterpolationMatrix(const QuadWord p[], const vmd::CameraKeyframe &frame)
{
    QuadWord actual, expected = p[0];
    frame.getInterpolationParameter(vmd::CameraKeyframe::kCameraLookAtX, actual);
    ASSERT_TRUE(CompareVector(expected, actual));
    expected = p[1];
    frame.getInterpolationParameter(vmd::CameraKeyframe::kCameraLookAtY, actual);
    ASSERT_TRUE(CompareVector(expected, actual));
    expected = p[2];
    frame.getInterpolationParameter(vmd::CameraKeyframe::kCameraLookAtZ, actual);
    ASSERT_TRUE(CompareVector(expected, actual));
    expected = p[3];
    frame.getInterpolationParameter(vmd::CameraKeyframe::kCameraAngle, actual);
    ASSERT_TRUE(CompareVector(expected, actual));
    expected = p[4];
    frame.getInterpolationParameter(vmd::CameraKeyframe::kCameraDistance, actual);
    ASSERT_TRUE(CompareVector(expected, actual));
    expected = p[5];
    frame.getInterpolationParameter(vmd::CameraKeyframe::kCameraFov, actual);
    ASSERT_TRUE(CompareVector(expected, actual));
}

}

TEST(VMDMotionTest, ParseEmpty)
{
    Encoding encoding(0);
    Model model(&encoding);
    vmd::Motion motion(&model, &encoding);
    vmd::Motion::DataInfo info;
    // parsing empty should be error
    ASSERT_FALSE(motion.preparse(reinterpret_cast<const uint8 *>(""), 0, info));
    ASSERT_EQ(vmd::Motion::kInvalidHeaderError, motion.error());
}

TEST(VMDMotionTest, ParseFile)
{
    QFile file("motion.vmd");
    if (file.open(QFile::ReadOnly)) {
        QByteArray bytes = file.readAll();
        const uint8 *data = reinterpret_cast<const uint8 *>(bytes.constData());
        vsize size = bytes.size();
        Encoding encoding(0);
        Model model(&encoding);
        vmd::Motion motion(&model, &encoding);
        vmd::Motion::DataInfo result;
        // valid model motion should be loaded successfully
        ASSERT_TRUE(motion.preparse(data, size, result));
        ASSERT_TRUE(motion.load(data, size));
        ASSERT_EQ(vsize(motion.boneAnimation().countKeyframes()), result.boneKeyframeCount);
        ASSERT_EQ(vsize(motion.cameraAnimation().countKeyframes()), result.cameraKeyframeCount);
        ASSERT_EQ(vsize(motion.morphAnimation().countKeyframes()), result.morphKeyframeCount);
        ASSERT_EQ(vmd::Motion::kNoError, motion.error());
    }
}

TEST(VMDMotionTest, ParseCamera)
{
    QFile file("camera.vmd");
    if (file.open(QFile::ReadOnly)) {
        QByteArray bytes = file.readAll();
        const uint8 *data = reinterpret_cast<const uint8 *>(bytes.constData());
        vsize size = bytes.size();
        Encoding encoding(0);
        Model model(&encoding);
        vmd::Motion motion(&model, &encoding);
        vmd::Motion::DataInfo result;
        // valid camera motion should be loaded successfully
        ASSERT_TRUE(motion.preparse(data, size, result));
        ASSERT_TRUE(motion.load(data, size));
        ASSERT_EQ(vsize(motion.boneAnimation().countKeyframes()), result.boneKeyframeCount);
        ASSERT_EQ(vsize(motion.cameraAnimation().countKeyframes()), result.cameraKeyframeCount);
        ASSERT_EQ(vsize(motion.morphAnimation().countKeyframes()), result.morphKeyframeCount);
        ASSERT_EQ(vmd::Motion::kNoError, motion.error());
    }
}

TEST(VMDMotionTest, SaveBoneKeyframe)
{
    Encoding encoding(0);
    String str(kTestString);
    vmd::BoneKeyframe frame(&encoding), newFrame(&encoding);
    Vector3 pos(1, 2, 3);
    Quaternion rot(4, 5, 6, 7);
    // initialize the bone frame to be copied
    frame.setTimeIndex(42);
    frame.setName(&str);
    frame.setLocalTranslation(pos);
    frame.setLocalRotation(rot);
    QuadWord px(8, 9, 10, 11),
            py(12, 13, 14, 15),
            pz(16, 17, 18, 19),
            pr(20, 21, 22, 23);
    QuadWord p[] = { px, py, pz, pr };
    frame.setInterpolationParameter(vmd::BoneKeyframe::kBonePositionX, px);
    frame.setInterpolationParameter(vmd::BoneKeyframe::kBonePositionY, py);
    frame.setInterpolationParameter(vmd::BoneKeyframe::kBonePositionZ, pz);
    frame.setInterpolationParameter(vmd::BoneKeyframe::kBoneRotation, pr);
    // write a bone frame to data and read it
    uint8 data[vmd::BoneKeyframe::strideSize()];
    frame.write(data);
    newFrame.read(data);
    // compare read bone frame
    ASSERT_TRUE(newFrame.name()->equals(frame.name()));
    ASSERT_EQ(frame.timeIndex(), newFrame.timeIndex());
    ASSERT_TRUE(newFrame.localTranslation() == pos);
    ASSERT_TRUE(newFrame.localRotation() == rot);
    CompareBoneInterpolationMatrix(p, frame);
    // cloned bone frame shold be copied with deep
    QScopedPointer<IBoneKeyframe> cloned(frame.clone());
    ASSERT_TRUE(cloned->name()->equals(frame.name()));
    ASSERT_EQ(frame.timeIndex(), cloned->timeIndex());
    ASSERT_TRUE(cloned->localTranslation() == pos);
    ASSERT_TRUE(cloned->localRotation() == rot);
    CompareBoneInterpolationMatrix(p, *static_cast<vmd::BoneKeyframe *>(cloned.data()));
}

TEST(VMDMotionTest, SaveCameraKeyframe)
{
    vmd::CameraKeyframe frame, newFrame;
    Vector3 pos(1, 2, 3), angle(4, 5, 6);
    // initialize the camera frame to be copied
    frame.setTimeIndex(42);
    frame.setLookAt(pos);
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
    frame.setInterpolationParameter(vmd::CameraKeyframe::kCameraLookAtX, px);
    frame.setInterpolationParameter(vmd::CameraKeyframe::kCameraLookAtY, py);
    frame.setInterpolationParameter(vmd::CameraKeyframe::kCameraLookAtZ, pz);
    frame.setInterpolationParameter(vmd::CameraKeyframe::kCameraAngle, pr);
    frame.setInterpolationParameter(vmd::CameraKeyframe::kCameraDistance, pd);
    frame.setInterpolationParameter(vmd::CameraKeyframe::kCameraFov, pf);
    // write a camera frame to data and read it
    uint8 data[vmd::CameraKeyframe::strideSize()];
    frame.write(data);
    newFrame.read(data);
    ASSERT_EQ(frame.timeIndex(), newFrame.timeIndex());
    ASSERT_TRUE(newFrame.lookAt() == frame.lookAt());
    // compare read camera frame
    // for radian and degree calculation
    ASSERT_TRUE(qFuzzyCompare(newFrame.angle().x(), frame.angle().x()));
    ASSERT_TRUE(qFuzzyCompare(newFrame.angle().y(), frame.angle().y()));
    ASSERT_TRUE(qFuzzyCompare(newFrame.angle().z(), frame.angle().z()));
    ASSERT_TRUE(newFrame.distance() == frame.distance());
    ASSERT_TRUE(newFrame.fov() == frame.fov());
    CompareCameraInterpolationMatrix(p, frame);
    // cloned camera frame shold be copied with deep
    QScopedPointer<ICameraKeyframe> cloned(frame.clone());
    ASSERT_EQ(frame.timeIndex(), cloned->timeIndex());
    ASSERT_TRUE(cloned->lookAt() == frame.lookAt());
    // for radian and degree calculation
    ASSERT_TRUE(qFuzzyCompare(cloned->angle().x(), frame.angle().x()));
    ASSERT_TRUE(qFuzzyCompare(cloned->angle().y(), frame.angle().y()));
    ASSERT_TRUE(qFuzzyCompare(cloned->angle().z(), frame.angle().z()));
    ASSERT_TRUE(cloned->distance() == frame.distance());
    ASSERT_TRUE(cloned->fov() == frame.fov());
    CompareCameraInterpolationMatrix(p, *static_cast<vmd::CameraKeyframe *>(cloned.data()));
}

TEST(VMDMotionTest, SaveMorphKeyframe)
{
    Encoding encoding(0);
    String str(kTestString);
    vmd::MorphKeyframe frame(&encoding), newFrame(&encoding);
    // initialize the morph frame to be copied
    frame.setName(&str);
    frame.setTimeIndex(42);
    frame.setWeight(0.5);
    // write a morph frame to data and read it
    uint8 data[vmd::MorphKeyframe::strideSize()];
    frame.write(data);
    newFrame.read(data);
    // compare read morph frame
    ASSERT_TRUE(newFrame.name()->equals(frame.name()));
    ASSERT_EQ(frame.timeIndex(), newFrame.timeIndex());
    ASSERT_EQ(frame.weight(), newFrame.weight());
    // cloned morph frame shold be copied with deep
    QScopedPointer<IMorphKeyframe> cloned(frame.clone());
    ASSERT_TRUE(cloned->name()->equals(frame.name()));
    ASSERT_EQ(frame.timeIndex(), cloned->timeIndex());
    ASSERT_EQ(frame.weight(), cloned->weight());
}

TEST(VMDMotionTest, SaveLightKeyframe)
{
    vmd::LightKeyframe frame, newFrame;
    Vector3 color(0.1, 0.2, 0.3), direction(4, 5, 6);
    // initialize the light frame to be copied
    frame.setTimeIndex(42);
    frame.setColor(color);
    frame.setDirection(direction);
    // write a light frame to data and read it
    uint8 data[vmd::LightKeyframe::strideSize()];
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

TEST(VMDMotionTest, SaveMotion)
{
    QFile file("motion.vmd");
    if (file.open(QFile::ReadOnly)) {
        QByteArray bytes = file.readAll();
        const uint8 *data = reinterpret_cast<const uint8 *>(bytes.constData());
        vsize size = bytes.size();
        Encoding encoding(0);
        Model model(&encoding);
        vmd::Motion motion(&model, &encoding);
        motion.load(data, size);
        vsize newSize = motion.estimateSize();
        // ASSERT_EQ(size, newSize);
        QScopedArrayPointer<uint8> newData(new uint8[newSize]);
        uint8 *ptr = newData.data();
        vmd::Motion motion2(&model, &encoding);
        motion.save(ptr);
        ASSERT_TRUE(motion2.load(ptr, newSize));
        ASSERT_EQ(motion.countKeyframes(IKeyframe::kBoneKeyframe), motion2.countKeyframes(IKeyframe::kBoneKeyframe));
        ASSERT_EQ(motion.countKeyframes(IKeyframe::kMorphKeyframe), motion2.countKeyframes(IKeyframe::kMorphKeyframe));
        ASSERT_EQ(motion.countKeyframes(IKeyframe::kModelKeyframe), motion2.countKeyframes(IKeyframe::kModelKeyframe));
    }
}

TEST(VMDMotionTest, CloneMotion)
{
    QFile file("motion.vmd");
    if (file.open(QFile::ReadOnly)) {
        QByteArray bytes = file.readAll();
        const uint8 *data = reinterpret_cast<const uint8 *>(bytes.constData());
        vsize size = bytes.size();
        Encoding encoding(0);
        Model model(&encoding);
        vmd::Motion motion(&model, &encoding);
        motion.load(data, size);
        QByteArray bytes2(motion.estimateSize(), 0);
        motion.save(reinterpret_cast<uint8 *>(bytes2.data()));
        QScopedPointer<IMotion> motion2(motion.clone());
        QByteArray bytes3(motion2->estimateSize(), 0);
        //motion2->save(reinterpret_cast<uint8_t *>(bytes3.data()));
        //ASSERT_STREQ(bytes2.constData(), bytes3.constData());
    }
}

TEST(VMDMotionTest, ParseBoneKeyframe)
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.writeRawData(kTestString, vmd::BoneKeyframe::kNameSize);
    stream << quint32(1)                   // frame index
           << 2.0f << 3.0f << 4.0f         // position
           << 5.0f << 6.0f << 7.0f << 8.0f // rotation
              ;
    for (int i = 0; i < vmd::BoneKeyframe::kTableSize; i++)
        stream << quint8(0);
    ASSERT_EQ(vmd::BoneKeyframe::strideSize(), vsize(bytes.size()));
    Encoding encoding(0);
    vmd::BoneKeyframe frame(&encoding);
    String str(kTestString);
    frame.read(reinterpret_cast<const uint8 *>(bytes.constData()));
    ASSERT_TRUE(frame.name()->equals(&str));
    ASSERT_EQ(IKeyframe::TimeIndex(1.0), frame.timeIndex());
#ifdef VPVL2_COORDINATE_OPENGL
    ASSERT_TRUE(frame.localTranslation() == Vector3(2.0f, 3.0f, -4.0f));
    ASSERT_TRUE(frame.localRotation() == Quaternion(-5.0f, -6.0f, 7.0f, 8.0f));
#else
    ASSERT_TRUE(frame.localPosition() == Vector3(2.0f, 3.0f, 4.0f));
    ASSERT_TRUE(frame.localRotation() == Quaternion(5.0f, 6.0f, 7.0f, 8.0f));
#endif
}

TEST(VMDMotionTest, ParseCameraKeyframe)
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << quint32(1)           // frame index
           << 1.0f                 // distance
           << 2.0f << 3.0f << 4.0f // position
           << 5.0f << 6.0f << 7.0f // angle
              ;
    for (int i = 0; i < vmd::CameraKeyframe::kTableSize; i++)
        stream << quint8(0);
    stream << quint32(8)           // view angle (fovy)
           << quint8(1)            // no perspective
              ;
    ASSERT_EQ(vmd::CameraKeyframe::strideSize(), vsize(bytes.size()));
    vmd::CameraKeyframe frame;
    frame.read(reinterpret_cast<const uint8 *>(bytes.constData()));
    ASSERT_EQ(IKeyframe::TimeIndex(1.0), frame.timeIndex());
#ifdef VPVL2_COORDINATE_OPENGL
    ASSERT_EQ(-1.0f, frame.distance());
    ASSERT_TRUE(frame.lookAt() == Vector3(2.0f, 3.0f, -4.0f));
    ASSERT_TRUE(frame.angle() == Vector3(-btDegrees(5.0f), -btDegrees(6.0f), btDegrees(7.0f)));
#else
    ASSERT_EQ(1.0f, frame.distance());
    ASSERT_TRUE(frame.position() == Vector3(2.0f, 3.0f, 4.0f));
    ASSERT_TRUE(frame.angle() == Vector3(btDegrees(5.0f), btDegrees(6.0f), btDegrees(7.0f)));
#endif
    ASSERT_EQ(8.0f, frame.fov());
    // TODO: perspective flag
}

TEST(VMDMotionTest, ParseMorphKeyframe)
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.writeRawData(kTestString, vmd::MorphKeyframe::kNameSize);
    stream << quint32(1) // frame index
           << 0.5f       // weight
              ;
    ASSERT_EQ(vmd::MorphKeyframe::strideSize(), vsize(bytes.size()));
    Encoding encoding(0);
    vmd::MorphKeyframe frame(&encoding);
    String str(kTestString);
    frame.read(reinterpret_cast<const uint8 *>(bytes.constData()));
    ASSERT_TRUE(frame.name()->equals(&str));
    ASSERT_EQ(IKeyframe::TimeIndex(1.0), frame.timeIndex());
    ASSERT_EQ(IMorph::WeightPrecision(0.5), frame.weight());
}

TEST(VMDMotionTest, ParseLightKeyframe)
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << quint32(1)           // frame index
           << 0.2f << 0.3f << 0.4f // color
           << 0.5f << 0.6f << 0.7f // direction
              ;
    ASSERT_EQ(vmd::LightKeyframe::strideSize(), vsize(bytes.size()));
    vmd::LightKeyframe frame;
    frame.read(reinterpret_cast<const uint8 *>(bytes.constData()));
    ASSERT_EQ(IKeyframe::TimeIndex(1.0), frame.timeIndex());
    ASSERT_TRUE(frame.color() == Vector3(0.2f, 0.3f, 0.4f));
#ifdef VPVL2_COORDINATE_OPENGL
    ASSERT_TRUE(frame.direction() == Vector3(0.5f, 0.6f, -0.7f));
#else
    ASSERT_TRUE(frame.direction() == Vector3(0.5f, 0.6f, 0.7f));
#endif
}

TEST(VMDMotionTest, BoneInterpolation)
{
    Encoding encoding(0);
    vmd::BoneKeyframe frame(&encoding);
    QuadWord n;
    frame.getInterpolationParameter(vmd::BoneKeyframe::kBonePositionX, n);
    ASSERT_TRUE(CompareVector(QuadWord(0.0f, 0.0f, 0.0f, 0.0f), n));
    QuadWord px(8, 9, 10, 11),
            py(12, 13, 14, 15),
            pz(16, 17, 18, 19),
            pr(20, 21, 22, 23);
    QuadWord p[] = { px, py, pz, pr };
    frame.setInterpolationParameter(vmd::BoneKeyframe::kBonePositionX, px);
    frame.setInterpolationParameter(vmd::BoneKeyframe::kBonePositionY, py);
    frame.setInterpolationParameter(vmd::BoneKeyframe::kBonePositionZ, pz);
    frame.setInterpolationParameter(vmd::BoneKeyframe::kBoneRotation, pr);
    CompareBoneInterpolationMatrix(p, frame);
}

TEST(VMDMotionTest, CameraInterpolation)
{
    vmd::CameraKeyframe frame;
    QuadWord n;
    frame.getInterpolationParameter(vmd::CameraKeyframe::kCameraLookAtX, n);
    ASSERT_TRUE(CompareVector(QuadWord(0.0f, 0.0f, 0.0f, 0.0f), n));
    QuadWord px(9, 10, 11, 12),
            py(13, 14, 15, 16),
            pz(17, 18, 19, 20),
            pr(21, 22, 23, 24),
            pd(25, 26, 27, 28),
            pf(29, 30, 31, 32);
    QuadWord p[] = { px, py, pz, pr, pd, pf };
    frame.setInterpolationParameter(vmd::CameraKeyframe::kCameraLookAtX, px);
    frame.setInterpolationParameter(vmd::CameraKeyframe::kCameraLookAtY, py);
    frame.setInterpolationParameter(vmd::CameraKeyframe::kCameraLookAtZ, pz);
    frame.setInterpolationParameter(vmd::CameraKeyframe::kCameraAngle, pr);
    frame.setInterpolationParameter(vmd::CameraKeyframe::kCameraDistance, pd);
    frame.setInterpolationParameter(vmd::CameraKeyframe::kCameraFov, pf);
    CompareCameraInterpolationMatrix(p, frame);
}

TEST(VMDMotionTest, AddAndRemoveBoneKeyframes)
{
    Encoding encoding(0);
    String name("bone");
    MockIModel model;
    MockIBone bone;
    vmd::Motion motion(&model, &encoding);
    ASSERT_EQ(0, motion.countKeyframes(IKeyframe::kBoneKeyframe));
    // mock bone
    EXPECT_CALL(model, findBoneRef(_)).Times(AtLeast(1)).WillRepeatedly(Return(&bone));
    QScopedPointer<IBoneKeyframe> keyframePtr(new vmd::BoneKeyframe(&encoding));
    keyframePtr->setTimeIndex(42);
    keyframePtr->setName(&name);
    {
        // The frame that the layer index is not zero should not be added
        QScopedPointer<IBoneKeyframe> frame42(new vmd::BoneKeyframe(&encoding));
        frame42->setLayerIndex(42);
        motion.addKeyframe(frame42.data());
        motion.update(IKeyframe::kBoneKeyframe);
        ASSERT_EQ(0, motion.countKeyframes(IKeyframe::kBoneKeyframe));
    }
    {
        // add a bone keyframe (don't forget updating motion!)
        motion.addKeyframe(keyframePtr.data());
        IKeyframe *keyframe = keyframePtr.take();
        motion.update(IKeyframe::kBoneKeyframe);
        ASSERT_EQ(1, motion.countKeyframes(IKeyframe::kBoneKeyframe));
        // boudary check of findBoneKeyframeAt
        ASSERT_EQ(static_cast<IBoneKeyframe *>(0), motion.findBoneKeyframeRefAt(-1));
        ASSERT_EQ(keyframe, motion.findBoneKeyframeRefAt(0));
        ASSERT_EQ(static_cast<IBoneKeyframe *>(0), motion.findBoneKeyframeRefAt(1));
        // layer index 0 must be used
        ASSERT_EQ(1, motion.countLayers(&name, IKeyframe::kBoneKeyframe));
        ASSERT_EQ(0, motion.findMorphKeyframeRef(42, &name, 1));
        // find a bone keyframe with timeIndex and name
        ASSERT_EQ(keyframe, motion.findBoneKeyframeRef(42, &name, 0));
    }
    keyframePtr.reset(new vmd::BoneKeyframe(&encoding));
    keyframePtr->setTimeIndex(42);
    keyframePtr->setName(&name);
    {
        // replaced bone frame should be one keyframe (don't forget updating motion!)
        motion.replaceKeyframe(keyframePtr.data(), true);
        IKeyframe *keyframeToDelete = keyframePtr.take();
        motion.update(IKeyframe::kBoneKeyframe);
        ASSERT_EQ(1, motion.countKeyframes(IKeyframe::kBoneKeyframe));
        // no longer be find previous bone keyframe
        ASSERT_EQ(keyframeToDelete, motion.findBoneKeyframeRef(42, &name, 0));
        // delete bone keyframe and set it null (don't forget updating motion!)
        motion.deleteKeyframe(keyframeToDelete);
        motion.update(IKeyframe::kBoneKeyframe);
        // bone keyframes should be empty
        ASSERT_EQ(0, motion.countKeyframes(IKeyframe::kBoneKeyframe));
        ASSERT_EQ(static_cast<IBoneKeyframe *>(0), motion.findBoneKeyframeRef(42, &name, 0));
        ASSERT_EQ(static_cast<IKeyframe *>(0), keyframeToDelete);
    }
}

TEST(VMDMotionTest, AddAndRemoveCameraKeyframes)
{
    Encoding encoding(0);
    Model model(&encoding);
    vmd::Motion motion(&model, &encoding);
    ASSERT_EQ(0, motion.countKeyframes(IKeyframe::kCameraKeyframe));
    QScopedPointer<ICameraKeyframe> keyframePtr(new vmd::CameraKeyframe());
    keyframePtr->setTimeIndex(42);
    keyframePtr->setDistance(42);
    {
        // The frame that the layer index is not zero should not be added
        QScopedPointer<ICameraKeyframe> frame42(new vmd::CameraKeyframe());
        frame42->setLayerIndex(42);
        motion.addKeyframe(frame42.data());
        motion.update(IKeyframe::kCameraKeyframe);
        ASSERT_EQ(0, motion.countKeyframes(IKeyframe::kCameraKeyframe));
    }
    {
        // add a camera keyframe (don't forget updating motion!)
        motion.addKeyframe(keyframePtr.data());
        IKeyframe *keyframe = keyframePtr.take();
        motion.update(IKeyframe::kCameraKeyframe);
        ASSERT_EQ(1, motion.countKeyframes(IKeyframe::kCameraKeyframe));
        // boudary check of findCameraKeyframeAt
        ASSERT_EQ(static_cast<ICameraKeyframe *>(0), motion.findCameraKeyframeRefAt(-1));
        ASSERT_EQ(keyframe, motion.findCameraKeyframeRefAt(0));
        ASSERT_EQ(static_cast<ICameraKeyframe *>(0), motion.findCameraKeyframeRefAt(1));
        // layer index 0 must be used
        ASSERT_EQ(1, motion.countLayers(0, IKeyframe::kCameraKeyframe));
        ASSERT_EQ(0, motion.findCameraKeyframeRef(42, 1));
        // find a camera keyframe with timeIndex
        ASSERT_EQ(keyframe, motion.findCameraKeyframeRef(42, 0));
    }
    keyframePtr.reset(new vmd::CameraKeyframe());
    keyframePtr->setTimeIndex(42);
    keyframePtr->setDistance(84);
    {
        // replaced camera frame should be one keyframe (don't forget updating motion!)
        motion.replaceKeyframe(keyframePtr.data(), true);
        IKeyframe *keyframeToDelete = keyframePtr.take();
        motion.update(IKeyframe::kCameraKeyframe);
        ASSERT_EQ(1, motion.countKeyframes(IKeyframe::kCameraKeyframe));
        // no longer be find previous camera keyframe
        ASSERT_EQ(84.0f, motion.findCameraKeyframeRef(42, 0)->distance());
        // delete camera keyframe and set it null (don't forget updating motion!)
        motion.deleteKeyframe(keyframeToDelete);
        motion.update(IKeyframe::kCameraKeyframe);
        // camera keyframes should be empty
        ASSERT_EQ(0, motion.countKeyframes(IKeyframe::kCameraKeyframe));
        ASSERT_EQ(static_cast<ICameraKeyframe *>(0), motion.findCameraKeyframeRef(42, 0));
        ASSERT_EQ(static_cast<IKeyframe *>(0), keyframeToDelete);
    }
}

TEST(VMDMotionTest, AddAndRemoveLightKeyframes)
{
    Encoding encoding(0);
    Model model(&encoding);
    vmd::Motion motion(&model, &encoding);
    ASSERT_EQ(0, motion.countKeyframes(IKeyframe::kLightKeyframe));
    QScopedPointer<ILightKeyframe> keyframePtr(new vmd::LightKeyframe());
    keyframePtr->setTimeIndex(42);
    keyframePtr->setColor(Vector3(1, 0, 0));
    {
        // The frame that the layer index is not zero should not be added
        QScopedPointer<ILightKeyframe> frame42(new vmd::LightKeyframe());
        frame42->setLayerIndex(42);
        motion.addKeyframe(frame42.data());
        motion.update(IKeyframe::kLightKeyframe);
        ASSERT_EQ(0, motion.countKeyframes(IKeyframe::kLightKeyframe));
    }
    {
        // add a light keyframe (don't forget updating motion!)
        motion.addKeyframe(keyframePtr.data());
        IKeyframe *keyframe = keyframePtr.take();
        motion.update(IKeyframe::kLightKeyframe);
        ASSERT_EQ(1, motion.countKeyframes(IKeyframe::kLightKeyframe));
        // boudary check of findLightKeyframeAt
        ASSERT_EQ(static_cast<ILightKeyframe *>(0), motion.findLightKeyframeRefAt(-1));
        ASSERT_EQ(keyframe, motion.findLightKeyframeRefAt(0));
        ASSERT_EQ(static_cast<ILightKeyframe *>(0), motion.findLightKeyframeRefAt(1));
        // layer index 0 must be used
        ASSERT_EQ(1, motion.countLayers(0, IKeyframe::kLightKeyframe));
        ASSERT_EQ(0, motion.findLightKeyframeRef(42, 1));
        // find a light keyframe with timeIndex
        ASSERT_EQ(keyframe, motion.findLightKeyframeRef(42, 0));
    }
    keyframePtr.reset(new vmd::LightKeyframe());
    keyframePtr->setTimeIndex(42);
    keyframePtr->setColor(Vector3(0, 0, 1));
    {
        // replaced light frame should be one keyframe (don't forget updating motion!)
        motion.replaceKeyframe(keyframePtr.data(), true);
        IKeyframe *keyframeToDelete = keyframePtr.take();
        motion.update(IKeyframe::kLightKeyframe);
        ASSERT_EQ(1, motion.countKeyframes(IKeyframe::kLightKeyframe));
        // no longer be find previous light keyframe
        ASSERT_EQ(1.0f, motion.findLightKeyframeRef(42, 0)->color().z());
        // delete light keyframe and set it null (don't forget updating motion!)
        motion.deleteKeyframe(keyframeToDelete);
        motion.update(IKeyframe::kLightKeyframe);
        // light keyframes should be empty
        ASSERT_EQ(0, motion.countKeyframes(IKeyframe::kLightKeyframe));
        ASSERT_EQ(static_cast<ILightKeyframe *>(0), motion.findLightKeyframeRef(42, 0));
        ASSERT_EQ(static_cast<IKeyframe *>(0), keyframeToDelete);
    }
}

TEST(VMDMotionTest, AddAndRemoveMorphKeyframes)
{
    Encoding encoding(0);
    String name("morph");
    MockIModel model;
    MockIMorph morph;
    vmd::Motion motion(&model, &encoding);
    ASSERT_EQ(0, motion.countKeyframes(IKeyframe::kMorphKeyframe));
    // mock morph
    EXPECT_CALL(model, findMorphRef(_)).Times(AtLeast(1)).WillRepeatedly(Return(&morph));
    QScopedPointer<IMorphKeyframe> keyframePtr(new vmd::MorphKeyframe(&encoding));
    keyframePtr->setTimeIndex(42);
    keyframePtr->setName(&name);
    {
        // The frame that the layer index is not zero should not be added
        QScopedPointer<IMorphKeyframe> frame42(new vmd::MorphKeyframe(&encoding));
        frame42->setLayerIndex(42);
        motion.addKeyframe(frame42.data());
        motion.update(IKeyframe::kMorphKeyframe);
        ASSERT_EQ(0, motion.countKeyframes(IKeyframe::kMorphKeyframe));
    }
    {
        // add a morph keyframe (don't forget updating motion!)
        motion.addKeyframe(keyframePtr.data());
        IKeyframe *keyframe = keyframePtr.take();
        motion.update(IKeyframe::kMorphKeyframe);
        ASSERT_EQ(1, motion.countKeyframes(IKeyframe::kMorphKeyframe));
        // boudary check of findMorphKeyframeAt
        ASSERT_EQ(static_cast<IMorphKeyframe *>(0), motion.findMorphKeyframeRefAt(-1));
        ASSERT_EQ(keyframe, motion.findMorphKeyframeRefAt(0));
        ASSERT_EQ(static_cast<IMorphKeyframe *>(0), motion.findMorphKeyframeRefAt(1));
        // layer index 0 must be used
        ASSERT_EQ(1, motion.countLayers(&name, IKeyframe::kMorphKeyframe));
        ASSERT_EQ(0, motion.findMorphKeyframeRef(42, &name, 1));
        ASSERT_EQ(keyframe, motion.findMorphKeyframeRef(42, &name, 0));
    }
    keyframePtr.reset(new vmd::MorphKeyframe(&encoding));
    keyframePtr->setTimeIndex(42);
    keyframePtr->setName(&name);
    {
        // replaced morph frame should be one keyframe (don't forget updating motion!)
        motion.replaceKeyframe(keyframePtr.data(), true);
        IKeyframe *keyframeToDelete = keyframePtr.take();
        motion.update(IKeyframe::kMorphKeyframe);
        ASSERT_EQ(1, motion.countKeyframes(IKeyframe::kMorphKeyframe));
        // no longer be find previous morph keyframe
        ASSERT_EQ(keyframeToDelete, motion.findMorphKeyframeRef(42, &name, 0));
        // delete light keyframe and set it null (don't forget updating motion!)
        motion.deleteKeyframe(keyframeToDelete);
        motion.update(IKeyframe::kMorphKeyframe);
        // morph keyframes should be empty
        ASSERT_EQ(0, motion.countKeyframes(IKeyframe::kMorphKeyframe));
        ASSERT_EQ(static_cast<IMorphKeyframe *>(0), motion.findMorphKeyframeRef(42, &name, 0));
        ASSERT_EQ(static_cast<IKeyframe *>(0), keyframeToDelete);
    }
}

TEST(VMDMotionTest, AddAndRemoveNullKeyframe)
{
    /* should happen nothing */
    Encoding encoding(0);
    MockIModel model;
    IKeyframe *nullKeyframe = 0;
    vmd::Motion motion(&model, &encoding);
    motion.addKeyframe(nullKeyframe);
    motion.replaceKeyframe(nullKeyframe, true);
    motion.deleteKeyframe(nullKeyframe);
}

class VMDMotionAllKeyframesTest : public TestWithParam<IKeyframe::Type> {};

TEST_P(VMDMotionAllKeyframesTest, SetAndGetAllKeyframes)
{
    Encoding::Dictionary dictionary;
    Encoding encoding(&dictionary);
    MockIModel model;
    MockIBone bone; /* no interest call */
    EXPECT_CALL(model, findBoneRef(_)).Times(AnyNumber()).WillRepeatedly(Return(&bone));
    MockIMorph morph; /* no interest call */
    EXPECT_CALL(model, findMorphRef(_)).Times(AnyNumber()).WillRepeatedly(Return(&morph));
    vmd::Motion motion(&model, &encoding);
    Array<IKeyframe *> source, dest;
    IKeyframe::Type type = GetParam();
    QScopedPointer<vmd::BoneKeyframe> boneKeyframe(new vmd::BoneKeyframe(&encoding));
    boneKeyframe->setName(encoding.stringConstant(IEncoding::kMaxConstantType));
    source.append(boneKeyframe.data());
    QScopedPointer<vmd::CameraKeyframe> cameraKeyframe(new vmd::CameraKeyframe());
    source.append(cameraKeyframe.data());
    QScopedPointer<vmd::LightKeyframe> lightKeyframe(new vmd::LightKeyframe());
    source.append(lightKeyframe.data());
    QScopedPointer<vmd::MorphKeyframe> morphKeyframe(new vmd::MorphKeyframe(&encoding));
    morphKeyframe->setName(encoding.stringConstant(IEncoding::kMaxConstantType));
    source.append(morphKeyframe.data());
    motion.setAllKeyframes(source, type);
    boneKeyframe.take();
    cameraKeyframe.take();
    lightKeyframe.take();
    morphKeyframe.take();
    motion.getAllKeyframeRefs(dest, type);
    ASSERT_EQ(1, dest.count());
    ASSERT_EQ(type, dest[0]->type());
}

INSTANTIATE_TEST_CASE_P(VMDMotionInstance, VMDMotionAllKeyframesTest,
                        Values(IKeyframe::kBoneKeyframe, IKeyframe::kCameraKeyframe,
                               IKeyframe::kLightKeyframe, IKeyframe::kMorphKeyframe));
