#include <gtest/gtest.h>
#include <QtCore/QtCore>

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
#include "Common.h"

#include <gmock/gmock.h>
#include "mock/Bone.h"
#include "mock/Model.h"
#include "mock/Morph.h"

using namespace ::testing;
using namespace vpvl2::pmx;
using namespace vpvl2::vmd;

namespace
{

const char *kTestString = "012345678901234";

static void CompareQuadWord(const QuadWord &actual, const QuadWord &expected)
{
    EXPECT_FLOAT_EQ(expected.x(), actual.x());
    EXPECT_FLOAT_EQ(expected.y(), actual.y());
    EXPECT_FLOAT_EQ(expected.z(), actual.z());
    EXPECT_FLOAT_EQ(expected.w(), actual.w());
}

static void CompareBoneInterpolationMatrix(const QuadWord p[], const BoneKeyframe &frame)
{
    QuadWord actual, expected = p[0];
    frame.getInterpolationParameter(BoneKeyframe::kX, actual);
    CompareQuadWord(actual, expected);
    expected = p[1];
    frame.getInterpolationParameter(BoneKeyframe::kY, actual);
    CompareQuadWord(actual, expected);
    expected = p[2];
    frame.getInterpolationParameter(BoneKeyframe::kZ, actual);
    CompareQuadWord(actual, expected);
    expected = p[3];
    frame.getInterpolationParameter(BoneKeyframe::kRotation, actual);
    CompareQuadWord(actual, expected);
}

static void CompareCameraInterpolationMatrix(const QuadWord p[], const CameraKeyframe &frame)
{
    QuadWord actual, expected = p[0];
    frame.getInterpolationParameter(CameraKeyframe::kX, actual);
    CompareQuadWord(actual, expected);
    expected = p[1];
    frame.getInterpolationParameter(CameraKeyframe::kY, actual);
    CompareQuadWord(actual, expected);
    expected = p[2];
    frame.getInterpolationParameter(CameraKeyframe::kZ, actual);
    CompareQuadWord(actual, expected);
    expected = p[3];
    frame.getInterpolationParameter(CameraKeyframe::kRotation, actual);
    CompareQuadWord(actual, expected);
    expected = p[4];
    frame.getInterpolationParameter(CameraKeyframe::kDistance, actual);
    CompareQuadWord(actual, expected);
    expected = p[5];
    frame.getInterpolationParameter(CameraKeyframe::kFov, actual);
    CompareQuadWord(actual, expected);
}

}

TEST(Motion, ParseEmpty)
{
    Encoding encoding;
    Model model(&encoding);
    Motion motion(&model, &encoding);
    Motion::DataInfo info;
    // parsing empty should be error
    EXPECT_FALSE(motion.preparse(reinterpret_cast<const uint8_t *>(""), 0, info));
    EXPECT_EQ(Motion::kInvalidHeaderError, motion.error());
}

TEST(Motion, ParseFile)
{
    QFile file("motion.vmd");
    if (file.open(QFile::ReadOnly)) {
        QByteArray bytes = file.readAll();
        const uint8_t *data = reinterpret_cast<const uint8_t *>(bytes.constData());
        size_t size = bytes.size();
        Encoding encoding;
        Model model(&encoding);
        Motion motion(&model, &encoding);
        Motion::DataInfo result;
        // valid model motion should be loaded successfully
        EXPECT_TRUE(motion.preparse(data, size, result));
        EXPECT_TRUE(motion.load(data, size));
        EXPECT_EQ(size_t(motion.boneAnimation().countKeyframes()), result.boneKeyframeCount);
        EXPECT_EQ(size_t(motion.cameraAnimation().countKeyframes()), result.cameraKeyframeCount);
        EXPECT_EQ(size_t(motion.morphAnimation().countKeyframes()), result.morphKeyframeCount);
        EXPECT_EQ(Motion::kNoError, motion.error());
    }
}

TEST(Motion, ParseCamera)
{
    QFile file("camera.vmd");
    if (file.open(QFile::ReadOnly)) {
        QByteArray bytes = file.readAll();
        const uint8_t *data = reinterpret_cast<const uint8_t *>(bytes.constData());
        size_t size = bytes.size();
        Encoding encoding;
        Model model(&encoding);
        Motion motion(&model, &encoding);
        Motion::DataInfo result;
        // valid camera motion should be loaded successfully
        EXPECT_TRUE(motion.preparse(data, size, result));
        EXPECT_TRUE(motion.load(data, size));
        EXPECT_EQ(size_t(motion.boneAnimation().countKeyframes()), result.boneKeyframeCount);
        EXPECT_EQ(size_t(motion.cameraAnimation().countKeyframes()), result.cameraKeyframeCount);
        EXPECT_EQ(size_t(motion.morphAnimation().countKeyframes()), result.morphKeyframeCount);
        EXPECT_EQ(Motion::kNoError, motion.error());
    }
}

TEST(Motion, SaveBoneKeyframe)
{
    Encoding encoding;
    String str(kTestString);
    BoneKeyframe frame(&encoding), newFrame(&encoding);
    Vector3 pos(1, 2, 3);
    Quaternion rot(4, 5, 6, 7);
    // initialize the bone frame to be copied
    frame.setTimeIndex(42);
    frame.setName(&str);
    frame.setPosition(pos);
    frame.setRotation(rot);
    QuadWord px(8, 9, 10, 11),
            py(12, 13, 14, 15),
            pz(16, 17, 18, 19),
            pr(20, 21, 22, 23);
    QuadWord p[] = { px, py, pz, pr };
    frame.setInterpolationParameter(BoneKeyframe::kX, px);
    frame.setInterpolationParameter(BoneKeyframe::kY, py);
    frame.setInterpolationParameter(BoneKeyframe::kZ, pz);
    frame.setInterpolationParameter(BoneKeyframe::kRotation, pr);
    // write a bone frame to data and read it
    uint8_t data[BoneKeyframe::strideSize()];
    frame.write(data);
    newFrame.read(data);
    // compare read bone frame
    EXPECT_TRUE(newFrame.name()->equals(frame.name()));
    EXPECT_EQ(frame.timeIndex(), newFrame.timeIndex());
    EXPECT_TRUE(newFrame.position() == pos);
    EXPECT_TRUE(newFrame.rotation() == rot);
    CompareBoneInterpolationMatrix(p, frame);
    // cloned bone frame shold be copied with deep
    QScopedPointer<IBoneKeyframe> cloned(frame.clone());
    EXPECT_TRUE(cloned->name()->equals(frame.name()));
    EXPECT_EQ(frame.timeIndex(), cloned->timeIndex());
    EXPECT_TRUE(cloned->position() == pos);
    EXPECT_TRUE(cloned->rotation() == rot);
    CompareBoneInterpolationMatrix(p, *static_cast<BoneKeyframe *>(cloned.data()));
}

TEST(Motion, SaveCameraKeyframe)
{
    CameraKeyframe frame, newFrame;
    Vector3 pos(1, 2, 3), angle(4, 5, 6);
    // initialize the camera frame to be copied
    frame.setTimeIndex(42);
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
    frame.setInterpolationParameter(CameraKeyframe::kX, px);
    frame.setInterpolationParameter(CameraKeyframe::kY, py);
    frame.setInterpolationParameter(CameraKeyframe::kZ, pz);
    frame.setInterpolationParameter(CameraKeyframe::kRotation, pr);
    frame.setInterpolationParameter(CameraKeyframe::kDistance, pd);
    frame.setInterpolationParameter(CameraKeyframe::kFov, pf);
    // write a camera frame to data and read it
    uint8_t data[CameraKeyframe::strideSize()];
    frame.write(data);
    newFrame.read(data);
    EXPECT_EQ(frame.timeIndex(), newFrame.timeIndex());
    EXPECT_TRUE(newFrame.position() == frame.position());
    // compare read camera frame
    // for radian and degree calculation
    EXPECT_TRUE(qFuzzyCompare(newFrame.angle().x(), frame.angle().x()));
    EXPECT_TRUE(qFuzzyCompare(newFrame.angle().y(), frame.angle().y()));
    EXPECT_TRUE(qFuzzyCompare(newFrame.angle().z(), frame.angle().z()));
    EXPECT_TRUE(newFrame.distance() == frame.distance());
    EXPECT_TRUE(newFrame.fov() == frame.fov());
    CompareCameraInterpolationMatrix(p, frame);
    // cloned camera frame shold be copied with deep
    QScopedPointer<ICameraKeyframe> cloned(frame.clone());
    EXPECT_EQ(frame.timeIndex(), cloned->timeIndex());
    EXPECT_TRUE(cloned->position() == frame.position());
    // for radian and degree calculation
    EXPECT_TRUE(qFuzzyCompare(cloned->angle().x(), frame.angle().x()));
    EXPECT_TRUE(qFuzzyCompare(cloned->angle().y(), frame.angle().y()));
    EXPECT_TRUE(qFuzzyCompare(cloned->angle().z(), frame.angle().z()));
    EXPECT_TRUE(cloned->distance() == frame.distance());
    EXPECT_TRUE(cloned->fov() == frame.fov());
    CompareCameraInterpolationMatrix(p, *static_cast<CameraKeyframe *>(cloned.data()));
}

TEST(Motion, SaveMorphKeyframe)
{
    Encoding encoding;
    String str(kTestString);
    MorphKeyframe frame(&encoding), newFrame(&encoding);
    // initialize the morph frame to be copied
    frame.setName(&str);
    frame.setTimeIndex(42);
    frame.setWeight(0.5);
    // write a morph frame to data and read it
    uint8_t data[MorphKeyframe::strideSize()];
    frame.write(data);
    newFrame.read(data);
    // compare read morph frame
    EXPECT_TRUE(newFrame.name()->equals(frame.name()));
    EXPECT_EQ(frame.timeIndex(), newFrame.timeIndex());
    EXPECT_EQ(frame.weight(), newFrame.weight());
    // cloned morph frame shold be copied with deep
    QScopedPointer<IMorphKeyframe> cloned(frame.clone());
    EXPECT_TRUE(cloned->name()->equals(frame.name()));
    EXPECT_EQ(frame.timeIndex(), cloned->timeIndex());
    EXPECT_EQ(frame.weight(), cloned->weight());
}

TEST(Motion, SaveLightKeyframe)
{
    LightKeyframe frame, newFrame;
    Vector3 color(0.1, 0.2, 0.3), direction(4, 5, 6);
    // initialize the light frame to be copied
    frame.setTimeIndex(42);
    frame.setColor(color);
    frame.setDirection(direction);
    // write a light frame to data and read it
    uint8_t data[LightKeyframe::strideSize()];
    frame.write(data);
    newFrame.read(data);
    // compare read light frame
    EXPECT_EQ(frame.timeIndex(), newFrame.timeIndex());
    EXPECT_TRUE(newFrame.color() == frame.color());
    EXPECT_TRUE(newFrame.direction() == frame.direction());
    // cloned morph frame shold be copied with deep
    QScopedPointer<ILightKeyframe> cloned(frame.clone());
    EXPECT_EQ(frame.timeIndex(), cloned->timeIndex());
    EXPECT_TRUE(cloned->color() == frame.color());
    EXPECT_TRUE(cloned->direction() == frame.direction());
}

TEST(Motion, SaveMotion)
{
    QFile file("motion.vmd");
    if (file.open(QFile::ReadOnly)) {
        QByteArray bytes = file.readAll();
        const uint8_t *data = reinterpret_cast<const uint8_t *>(bytes.constData());
        size_t size = bytes.size();
        Encoding encoding;
        Model model(&encoding);
        Motion motion(&model, &encoding);
        motion.load(data, size);
        size_t newSize = motion.estimateSize();
        QScopedArrayPointer<uint8_t> newData(new uint8_t[newSize]);
        motion.save(newData.data());
        // using temporary file and should be deleted
        QTemporaryFile file2;
        file2.setAutoRemove(true);
        file2.write(reinterpret_cast<const char *>(newData.data()), newSize);
        // just compare written size
        EXPECT_EQ(size, newSize);
    }
}

TEST(Motion, ParseBoneKeyframe)
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.writeRawData(kTestString, BoneKeyframe::kNameSize);
    stream << quint32(1)                   // frame index
           << 2.0f << 3.0f << 4.0f         // position
           << 5.0f << 6.0f << 7.0f << 8.0f // rotation
              ;
    for (int i = 0; i < BoneKeyframe::kTableSize; i++)
        stream << quint8(0);
    EXPECT_EQ(BoneKeyframe::strideSize(), size_t(bytes.size()));
    Encoding encoding;
    BoneKeyframe frame(&encoding);
    String str(kTestString);
    frame.read(reinterpret_cast<const uint8_t *>(bytes.constData()));
    EXPECT_TRUE(frame.name()->equals(&str));
    EXPECT_EQ(IKeyframe::TimeIndex(1.0), frame.timeIndex());
#ifdef VPVL2_COORDINATE_OPENGL
    EXPECT_TRUE(frame.position() == Vector3(2.0f, 3.0f, -4.0f));
    EXPECT_TRUE(frame.rotation() == Quaternion(-5.0f, -6.0f, 7.0f, 8.0f));
#else
    EXPECT_TRUE(frame.position() == Vector3(2.0f, 3.0f, 4.0f));
    EXPECT_TRUE(frame.rotation() == Quaternion(5.0f, 6.0f, 7.0f, 8.0f));
#endif
}

TEST(Motion, ParseCameraKeyframe)
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
    for (int i = 0; i < CameraKeyframe::kTableSize; i++)
        stream << quint8(0);
    stream << quint32(8)           // view angle (fovy)
           << quint8(1)            // no perspective
              ;
    EXPECT_EQ(CameraKeyframe::strideSize(), size_t(bytes.size()));
    CameraKeyframe frame;
    frame.read(reinterpret_cast<const uint8_t *>(bytes.constData()));
    EXPECT_EQ(IKeyframe::TimeIndex(1.0), frame.timeIndex());
#ifdef VPVL2_COORDINATE_OPENGL
    EXPECT_EQ(-1.0f, frame.distance());
    EXPECT_TRUE(frame.position() == Vector3(2.0f, 3.0f, -4.0f));
    EXPECT_TRUE(frame.angle() == Vector3(-degree(5.0f), -degree(6.0f), degree(7.0f)));
#else
    EXPECT_EQ(1.0f, frame.distance());
    EXPECT_TRUE(frame.position() == Vector3(2.0f, 3.0f, 4.0f));
    EXPECT_TRUE(frame.angle() == Vector3(degree(5.0f), degree(6.0f), degree(7.0f)));
#endif
    EXPECT_EQ(8.0f, frame.fov());
    // TODO: perspective flag
}

TEST(Motion, ParseMorphKeyframe)
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.writeRawData(kTestString, MorphKeyframe::kNameSize);
    stream << quint32(1) // frame index
           << 0.5f       // weight
              ;
    EXPECT_EQ(MorphKeyframe::strideSize(), size_t(bytes.size()));
    Encoding encoding;
    MorphKeyframe frame(&encoding);
    String str(kTestString);
    frame.read(reinterpret_cast<const uint8_t *>(bytes.constData()));
    EXPECT_TRUE(frame.name()->equals(&str));
    EXPECT_EQ(IKeyframe::TimeIndex(1.0), frame.timeIndex());
    EXPECT_EQ(IMorph::WeightPrecision(0.5), frame.weight());
}

TEST(Motion, ParseLightKeyframe)
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << quint32(1)           // frame index
           << 0.2f << 0.3f << 0.4f // color
           << 0.5f << 0.6f << 0.7f // direction
              ;
    EXPECT_EQ(LightKeyframe::strideSize(), size_t(bytes.size()));
    LightKeyframe frame;
    frame.read(reinterpret_cast<const uint8_t *>(bytes.constData()));
    EXPECT_EQ(IKeyframe::TimeIndex(1.0), frame.timeIndex());
    EXPECT_TRUE(frame.color() == Vector3(0.2f, 0.3f, 0.4f));
#ifdef VPVL2_COORDINATE_OPENGL
    EXPECT_TRUE(frame.direction() == Vector3(0.5f, 0.6f, -0.7f));
#else
    EXPECT_TRUE(frame.direction() == Vector3(0.5f, 0.6f, 0.7f));
#endif
}

TEST(Motion, BoneInterpolation)
{
    Encoding encoding;
    BoneKeyframe frame(&encoding);
    QuadWord n;
    frame.getInterpolationParameter(BoneKeyframe::kX, n);
    CompareQuadWord(n, QuadWord(0.0f, 0.0f, 0.0f, 0.0f));
    QuadWord px(8, 9, 10, 11),
            py(12, 13, 14, 15),
            pz(16, 17, 18, 19),
            pr(20, 21, 22, 23);
    QuadWord p[] = { px, py, pz, pr };
    frame.setInterpolationParameter(BoneKeyframe::kX, px);
    frame.setInterpolationParameter(BoneKeyframe::kY, py);
    frame.setInterpolationParameter(BoneKeyframe::kZ, pz);
    frame.setInterpolationParameter(BoneKeyframe::kRotation, pr);
    CompareBoneInterpolationMatrix(p, frame);
}

TEST(Motion, CameraInterpolation)
{
    CameraKeyframe frame;
    QuadWord n;
    frame.getInterpolationParameter(CameraKeyframe::kX, n);
    CompareQuadWord(n, QuadWord(0.0f, 0.0f, 0.0f, 0.0f));
    QuadWord px(9, 10, 11, 12),
            py(13, 14, 15, 16),
            pz(17, 18, 19, 20),
            pr(21, 22, 23, 24),
            pd(25, 26, 27, 28),
            pf(29, 30, 31, 32);
    QuadWord p[] = { px, py, pz, pr, pd, pf };
    frame.setInterpolationParameter(CameraKeyframe::kX, px);
    frame.setInterpolationParameter(CameraKeyframe::kY, py);
    frame.setInterpolationParameter(CameraKeyframe::kZ, pz);
    frame.setInterpolationParameter(CameraKeyframe::kRotation, pr);
    frame.setInterpolationParameter(CameraKeyframe::kDistance, pd);
    frame.setInterpolationParameter(CameraKeyframe::kFov, pf);
    CompareCameraInterpolationMatrix(p, frame);
}

TEST(Motion, AddAndRemoveBoneKeyframes)
{
    Encoding encoding;
    String name("bone");
    MockIModel model;
    MockIBone bone;
    Motion motion(&model, &encoding);
    EXPECT_EQ(0, motion.countKeyframes(IKeyframe::kBone));
    // mock bone
    EXPECT_CALL(model, findBone(_)).Times(AtLeast(1)).WillRepeatedly(Return(&bone));
    QScopedPointer<IBoneKeyframe> frame(new BoneKeyframe(&encoding));
    frame->setTimeIndex(42);
    frame->setName(&name);
    {
        // add a bone keyframe (don't forget updating motion!)
        motion.addKeyframe(frame.data());
        motion.update(IKeyframe::kBone);
        EXPECT_EQ(1, motion.countKeyframes(IKeyframe::kBone));
        // boudary check of findBoneKeyframeAt
        EXPECT_EQ(static_cast<IBoneKeyframe *>(0), motion.findBoneKeyframeAt(-1));
        EXPECT_EQ(frame.data(), motion.findBoneKeyframeAt(0));
        EXPECT_EQ(static_cast<IBoneKeyframe *>(0), motion.findBoneKeyframeAt(1));
        // find a bone keyframe with timeIndex and name
        EXPECT_EQ(frame.take(), motion.findBoneKeyframe(42, &name));
    }
    QScopedPointer<IBoneKeyframe> frame2(new BoneKeyframe(&encoding));
    frame2->setTimeIndex(42);
    frame2->setName(&name);
    {
        // replaced bone frame should be one keyframe (don't forget updating motion!)
        motion.replaceKeyframe(frame2.data());
        motion.update(IKeyframe::kBone);
        EXPECT_EQ(1, motion.countKeyframes(IKeyframe::kBone));
        // no longer be find previous bone keyframe
        EXPECT_EQ(frame2.data(), motion.findBoneKeyframe(42, &name));
    }
    {
        IKeyframe *keyframeToDelete = frame2.take();
        // delete bone keyframe and set it null (don't forget updating motion!)
        motion.deleteKeyframe(keyframeToDelete);
        motion.update(IKeyframe::kBone);
        // bone keyframes should be empty
        EXPECT_EQ(0, motion.countKeyframes(IKeyframe::kBone));
        EXPECT_EQ(static_cast<IBoneKeyframe *>(0), motion.findBoneKeyframe(42, &name));
        EXPECT_EQ(static_cast<IKeyframe *>(0), keyframeToDelete);
    }
}

TEST(Motion, AddAndRemoveCameraKeyframes)
{
    Encoding encoding;
    Model model(&encoding);
    Motion motion(&model, &encoding);
    EXPECT_EQ(0, motion.countKeyframes(IKeyframe::kCamera));
    QScopedPointer<ICameraKeyframe> frame(new CameraKeyframe());
    frame->setTimeIndex(42);
    frame->setDistance(42);
    {
        // add a camera keyframe (don't forget updating motion!)
        motion.addKeyframe(frame.data());
        motion.update(IKeyframe::kCamera);
        EXPECT_EQ(1, motion.countKeyframes(IKeyframe::kCamera));
        // boudary check of findCameraKeyframeAt
        EXPECT_EQ(static_cast<ICameraKeyframe *>(0), motion.findCameraKeyframeAt(-1));
        EXPECT_EQ(frame.data(), motion.findCameraKeyframeAt(0));
        EXPECT_EQ(static_cast<ICameraKeyframe *>(0), motion.findCameraKeyframeAt(1));
        // find a camera keyframe with timeIndex
        EXPECT_EQ(frame.take(), motion.findCameraKeyframe(42));
    }
    QScopedPointer<ICameraKeyframe> frame2(new CameraKeyframe());
    frame2->setTimeIndex(42);
    frame2->setDistance(84);
    {
        // replaced camera frame should be one keyframe (don't forget updating motion!)
        motion.replaceKeyframe(frame2.data());
        motion.update(IKeyframe::kCamera);
        EXPECT_EQ(1, motion.countKeyframes(IKeyframe::kCamera));
        // no longer be find previous camera keyframe
        EXPECT_EQ(84.0f, motion.findCameraKeyframe(42)->distance());
    }
    {
        IKeyframe *keyframeToDelete = frame2.take();
        // delete camera keyframe and set it null (don't forget updating motion!)
        motion.deleteKeyframe(keyframeToDelete);
        motion.update(IKeyframe::kCamera);
        // camera keyframes should be empty
        EXPECT_EQ(0, motion.countKeyframes(IKeyframe::kCamera));
        EXPECT_EQ(static_cast<ICameraKeyframe *>(0), motion.findCameraKeyframe(42));
        EXPECT_EQ(static_cast<IKeyframe *>(0), keyframeToDelete);
    }
}

TEST(Motion, AddAndRemoveLightKeyframes)
{
    Encoding encoding;
    Model model(&encoding);
    Motion motion(&model, &encoding);
    EXPECT_EQ(0, motion.countKeyframes(IKeyframe::kCamera));
    QScopedPointer<ILightKeyframe> frame(new LightKeyframe());
    frame->setTimeIndex(42);
    frame->setColor(Vector3(1, 0, 0));
    {
        // add a light keyframe (don't forget updating motion!)
        motion.addKeyframe(frame.data());
        motion.update(IKeyframe::kLight);
        EXPECT_EQ(1, motion.countKeyframes(IKeyframe::kLight));
        // boudary check of findLightKeyframeAt
        EXPECT_EQ(static_cast<ILightKeyframe *>(0), motion.findLightKeyframeAt(-1));
        EXPECT_EQ(frame.data(), motion.findLightKeyframeAt(0));
        EXPECT_EQ(static_cast<ILightKeyframe *>(0), motion.findLightKeyframeAt(1));
        // find a light keyframe with timeIndex
        EXPECT_EQ(frame.take(), motion.findLightKeyframe(42));
    }
    QScopedPointer<ILightKeyframe> frame2(new LightKeyframe());
    frame2->setTimeIndex(42);
    frame2->setColor(Vector3(0, 0, 1));
    {
        // replaced light frame should be one keyframe (don't forget updating motion!)
        motion.replaceKeyframe(frame2.data());
        motion.update(IKeyframe::kLight);
        EXPECT_EQ(1, motion.countKeyframes(IKeyframe::kLight));
        // no longer be find previous light keyframe
        EXPECT_EQ(1.0f, motion.findLightKeyframe(42)->color().z());
    }
    {
        IKeyframe *keyframeToDelete = frame2.take();
        // delete light keyframe and set it null (don't forget updating motion!)
        motion.deleteKeyframe(keyframeToDelete);
        motion.update(IKeyframe::kLight);
        // light keyframes should be empty
        EXPECT_EQ(0, motion.countKeyframes(IKeyframe::kLight));
        EXPECT_EQ(static_cast<ILightKeyframe *>(0), motion.findLightKeyframe(42));
        EXPECT_EQ(static_cast<IKeyframe *>(0), keyframeToDelete);
    }
}

TEST(Motion, AddAndRemoveMorphKeyframes)
{
    Encoding encoding;
    String name("morph");
    MockIModel model;
    MockIMorph morph;
    Motion motion(&model, &encoding);
    EXPECT_EQ(0, motion.countKeyframes(IKeyframe::kMorph));
    // mock morph
    EXPECT_CALL(model, findMorph(_)).Times(AtLeast(1)).WillRepeatedly(Return(&morph));
    QScopedPointer<IMorphKeyframe> frame(new MorphKeyframe(&encoding));
    frame->setTimeIndex(42);
    frame->setName(&name);
    {
        // add a morph keyframe (don't forget updating motion!)
        motion.addKeyframe(frame.data());
        motion.update(IKeyframe::kMorph);
        EXPECT_EQ(1, motion.countKeyframes(IKeyframe::kMorph));
        // boudary check of findMorphKeyframeAt
        EXPECT_EQ(static_cast<IMorphKeyframe *>(0), motion.findMorphKeyframeAt(-1));
        EXPECT_EQ(frame.data(), motion.findMorphKeyframeAt(0));
        EXPECT_EQ(static_cast<IMorphKeyframe *>(0), motion.findMorphKeyframeAt(1));
        EXPECT_EQ(frame.take(), motion.findMorphKeyframe(42, &name));
    }
    QScopedPointer<IMorphKeyframe> frame2(new MorphKeyframe(&encoding));
    frame2->setTimeIndex(42);
    frame2->setName(&name);
    {
        // replaced morph frame should be one keyframe (don't forget updating motion!)
        motion.replaceKeyframe(frame2.data());
        motion.update(IKeyframe::kMorph);
        EXPECT_EQ(1, motion.countKeyframes(IKeyframe::kMorph));
        // no longer be find previous morph keyframe
        EXPECT_EQ(frame2.data(), motion.findMorphKeyframe(42, &name));
    }
    {
        IKeyframe *keyframeToDelete = frame2.take();
        // delete light keyframe and set it null (don't forget updating motion!)
        motion.deleteKeyframe(keyframeToDelete);
        motion.update(IKeyframe::kMorph);
        // morph keyframes should be empty
        EXPECT_EQ(0, motion.countKeyframes(IKeyframe::kMorph));
        EXPECT_EQ(static_cast<IMorphKeyframe *>(0), motion.findMorphKeyframe(42, &name));
        EXPECT_EQ(static_cast<IKeyframe *>(0), keyframeToDelete);
    }
}
