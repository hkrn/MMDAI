#include "Common.h"

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

#include "vpvl2/mvd/Motion.h"

#include "mock/Bone.h"
#include "mock/Model.h"
#include "mock/Morph.h"

using namespace vpvl2::pmx;

namespace
{

const char *kTestString = "012345678901234";

static void CompareBoneInterpolationMatrix(const QuadWord p[], const vmd::BoneKeyframe &frame)
{
    QuadWord actual, expected = p[0];
    frame.getInterpolationParameter(vmd::BoneKeyframe::kX, actual);
    AssertVector(actual, expected);
    expected = p[1];
    frame.getInterpolationParameter(vmd::BoneKeyframe::kY, actual);
    AssertVector(actual, expected);
    expected = p[2];
    frame.getInterpolationParameter(vmd::BoneKeyframe::kZ, actual);
    AssertVector(actual, expected);
    expected = p[3];
    frame.getInterpolationParameter(vmd::BoneKeyframe::kRotation, actual);
    AssertVector(actual, expected);
}

static void CompareCameraInterpolationMatrix(const QuadWord p[], const vmd::CameraKeyframe &frame)
{
    QuadWord actual, expected = p[0];
    frame.getInterpolationParameter(vmd::CameraKeyframe::kX, actual);
    AssertVector(actual, expected);
    expected = p[1];
    frame.getInterpolationParameter(vmd::CameraKeyframe::kY, actual);
    AssertVector(actual, expected);
    expected = p[2];
    frame.getInterpolationParameter(vmd::CameraKeyframe::kZ, actual);
    AssertVector(actual, expected);
    expected = p[3];
    frame.getInterpolationParameter(vmd::CameraKeyframe::kRotation, actual);
    AssertVector(actual, expected);
    expected = p[4];
    frame.getInterpolationParameter(vmd::CameraKeyframe::kDistance, actual);
    AssertVector(actual, expected);
    expected = p[5];
    frame.getInterpolationParameter(vmd::CameraKeyframe::kFov, actual);
    AssertVector(actual, expected);
}

}

TEST(MotionTest, ParseEmpty)
{
    Encoding encoding;
    Model model(&encoding);
    vmd::Motion motion(&model, &encoding);
    vmd::Motion::DataInfo info;
    // parsing empty should be error
    ASSERT_FALSE(motion.preparse(reinterpret_cast<const uint8_t *>(""), 0, info));
    ASSERT_EQ(vmd::Motion::kInvalidHeaderError, motion.error());
}

TEST(MotionTest, ParseFile)
{
    QFile file("motion.vmd");
    if (file.open(QFile::ReadOnly)) {
        QByteArray bytes = file.readAll();
        const uint8_t *data = reinterpret_cast<const uint8_t *>(bytes.constData());
        size_t size = bytes.size();
        Encoding encoding;
        Model model(&encoding);
        vmd::Motion motion(&model, &encoding);
        vmd::Motion::DataInfo result;
        // valid model motion should be loaded successfully
        ASSERT_TRUE(motion.preparse(data, size, result));
        ASSERT_TRUE(motion.load(data, size));
        ASSERT_EQ(size_t(motion.boneAnimation().countKeyframes()), result.boneKeyframeCount);
        ASSERT_EQ(size_t(motion.cameraAnimation().countKeyframes()), result.cameraKeyframeCount);
        ASSERT_EQ(size_t(motion.morphAnimation().countKeyframes()), result.morphKeyframeCount);
        ASSERT_EQ(vmd::Motion::kNoError, motion.error());
    }
}

TEST(MotionTest, ParseCamera)
{
    QFile file("camera.vmd");
    if (file.open(QFile::ReadOnly)) {
        QByteArray bytes = file.readAll();
        const uint8_t *data = reinterpret_cast<const uint8_t *>(bytes.constData());
        size_t size = bytes.size();
        Encoding encoding;
        Model model(&encoding);
        vmd::Motion motion(&model, &encoding);
        vmd::Motion::DataInfo result;
        // valid camera motion should be loaded successfully
        ASSERT_TRUE(motion.preparse(data, size, result));
        ASSERT_TRUE(motion.load(data, size));
        ASSERT_EQ(size_t(motion.boneAnimation().countKeyframes()), result.boneKeyframeCount);
        ASSERT_EQ(size_t(motion.cameraAnimation().countKeyframes()), result.cameraKeyframeCount);
        ASSERT_EQ(size_t(motion.morphAnimation().countKeyframes()), result.morphKeyframeCount);
        ASSERT_EQ(vmd::Motion::kNoError, motion.error());
    }
}

TEST(MotionTest, SaveBoneKeyframe)
{
    Encoding encoding;
    CString str(kTestString);
    vmd::BoneKeyframe frame(&encoding), newFrame(&encoding);
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
    frame.setInterpolationParameter(vmd::BoneKeyframe::kX, px);
    frame.setInterpolationParameter(vmd::BoneKeyframe::kY, py);
    frame.setInterpolationParameter(vmd::BoneKeyframe::kZ, pz);
    frame.setInterpolationParameter(vmd::BoneKeyframe::kRotation, pr);
    // write a bone frame to data and read it
    uint8_t data[vmd::BoneKeyframe::strideSize()];
    frame.write(data);
    newFrame.read(data);
    // compare read bone frame
    ASSERT_TRUE(newFrame.name()->equals(frame.name()));
    ASSERT_EQ(frame.timeIndex(), newFrame.timeIndex());
    ASSERT_TRUE(newFrame.position() == pos);
    ASSERT_TRUE(newFrame.rotation() == rot);
    CompareBoneInterpolationMatrix(p, frame);
    // cloned bone frame shold be copied with deep
    QScopedPointer<IBoneKeyframe> cloned(frame.clone());
    ASSERT_TRUE(cloned->name()->equals(frame.name()));
    ASSERT_EQ(frame.timeIndex(), cloned->timeIndex());
    ASSERT_TRUE(cloned->position() == pos);
    ASSERT_TRUE(cloned->rotation() == rot);
    CompareBoneInterpolationMatrix(p, *static_cast<vmd::BoneKeyframe *>(cloned.data()));
}

TEST(MotionTest, SaveCameraKeyframe)
{
    vmd::CameraKeyframe frame, newFrame;
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
    frame.setInterpolationParameter(vmd::CameraKeyframe::kX, px);
    frame.setInterpolationParameter(vmd::CameraKeyframe::kY, py);
    frame.setInterpolationParameter(vmd::CameraKeyframe::kZ, pz);
    frame.setInterpolationParameter(vmd::CameraKeyframe::kRotation, pr);
    frame.setInterpolationParameter(vmd::CameraKeyframe::kDistance, pd);
    frame.setInterpolationParameter(vmd::CameraKeyframe::kFov, pf);
    // write a camera frame to data and read it
    uint8_t data[vmd::CameraKeyframe::strideSize()];
    frame.write(data);
    newFrame.read(data);
    ASSERT_EQ(frame.timeIndex(), newFrame.timeIndex());
    ASSERT_TRUE(newFrame.position() == frame.position());
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
    ASSERT_TRUE(cloned->position() == frame.position());
    // for radian and degree calculation
    ASSERT_TRUE(qFuzzyCompare(cloned->angle().x(), frame.angle().x()));
    ASSERT_TRUE(qFuzzyCompare(cloned->angle().y(), frame.angle().y()));
    ASSERT_TRUE(qFuzzyCompare(cloned->angle().z(), frame.angle().z()));
    ASSERT_TRUE(cloned->distance() == frame.distance());
    ASSERT_TRUE(cloned->fov() == frame.fov());
    CompareCameraInterpolationMatrix(p, *static_cast<vmd::CameraKeyframe *>(cloned.data()));
}

TEST(MotionTest, SaveMorphKeyframe)
{
    Encoding encoding;
    CString str(kTestString);
    vmd::MorphKeyframe frame(&encoding), newFrame(&encoding);
    // initialize the morph frame to be copied
    frame.setName(&str);
    frame.setTimeIndex(42);
    frame.setWeight(0.5);
    // write a morph frame to data and read it
    uint8_t data[vmd::MorphKeyframe::strideSize()];
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

TEST(MotionTest, SaveLightKeyframe)
{
    vmd::LightKeyframe frame, newFrame;
    Vector3 color(0.1, 0.2, 0.3), direction(4, 5, 6);
    // initialize the light frame to be copied
    frame.setTimeIndex(42);
    frame.setColor(color);
    frame.setDirection(direction);
    // write a light frame to data and read it
    uint8_t data[vmd::LightKeyframe::strideSize()];
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

TEST(MotionTest, SaveMotion)
{
    QFile file("motion.vmd");
    if (file.open(QFile::ReadOnly)) {
        QByteArray bytes = file.readAll();
        const uint8_t *data = reinterpret_cast<const uint8_t *>(bytes.constData());
        size_t size = bytes.size();
        Encoding encoding;
        Model model(&encoding);
        vmd::Motion motion(&model, &encoding);
        motion.load(data, size);
        size_t newSize = motion.estimateSize();
        QScopedArrayPointer<uint8_t> newData(new uint8_t[newSize]);
        motion.save(newData.data());
        // using temporary file and should be deleted
        QTemporaryFile file2;
        file2.setAutoRemove(true);
        file2.write(reinterpret_cast<const char *>(newData.data()), newSize);
        // just compare written size
        ASSERT_EQ(size, newSize);
    }
}

TEST(MotionTest, ParseBoneKeyframe)
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
    ASSERT_EQ(vmd::BoneKeyframe::strideSize(), size_t(bytes.size()));
    Encoding encoding;
    vmd::BoneKeyframe frame(&encoding);
    CString str(kTestString);
    frame.read(reinterpret_cast<const uint8_t *>(bytes.constData()));
    ASSERT_TRUE(frame.name()->equals(&str));
    ASSERT_EQ(IKeyframe::TimeIndex(1.0), frame.timeIndex());
#ifdef VPVL2_COORDINATE_OPENGL
    ASSERT_TRUE(frame.position() == Vector3(2.0f, 3.0f, -4.0f));
    ASSERT_TRUE(frame.rotation() == Quaternion(-5.0f, -6.0f, 7.0f, 8.0f));
#else
    ASSERT_TRUE(frame.position() == Vector3(2.0f, 3.0f, 4.0f));
    ASSERT_TRUE(frame.rotation() == Quaternion(5.0f, 6.0f, 7.0f, 8.0f));
#endif
}

TEST(MotionTest, ParseCameraKeyframe)
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
    ASSERT_EQ(vmd::CameraKeyframe::strideSize(), size_t(bytes.size()));
    vmd::CameraKeyframe frame;
    frame.read(reinterpret_cast<const uint8_t *>(bytes.constData()));
    ASSERT_EQ(IKeyframe::TimeIndex(1.0), frame.timeIndex());
#ifdef VPVL2_COORDINATE_OPENGL
    ASSERT_EQ(-1.0f, frame.distance());
    ASSERT_TRUE(frame.position() == Vector3(2.0f, 3.0f, -4.0f));
    ASSERT_TRUE(frame.angle() == Vector3(-degree(5.0f), -degree(6.0f), degree(7.0f)));
#else
    ASSERT_EQ(1.0f, frame.distance());
    ASSERT_TRUE(frame.position() == Vector3(2.0f, 3.0f, 4.0f));
    ASSERT_TRUE(frame.angle() == Vector3(degree(5.0f), degree(6.0f), degree(7.0f)));
#endif
    ASSERT_EQ(8.0f, frame.fov());
    // TODO: perspective flag
}

TEST(MotionTest, ParseMorphKeyframe)
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.writeRawData(kTestString, vmd::MorphKeyframe::kNameSize);
    stream << quint32(1) // frame index
           << 0.5f       // weight
              ;
    ASSERT_EQ(vmd::MorphKeyframe::strideSize(), size_t(bytes.size()));
    Encoding encoding;
    vmd::MorphKeyframe frame(&encoding);
    CString str(kTestString);
    frame.read(reinterpret_cast<const uint8_t *>(bytes.constData()));
    ASSERT_TRUE(frame.name()->equals(&str));
    ASSERT_EQ(IKeyframe::TimeIndex(1.0), frame.timeIndex());
    ASSERT_EQ(IMorph::WeightPrecision(0.5), frame.weight());
}

TEST(MotionTest, ParseLightKeyframe)
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << quint32(1)           // frame index
           << 0.2f << 0.3f << 0.4f // color
           << 0.5f << 0.6f << 0.7f // direction
              ;
    ASSERT_EQ(vmd::LightKeyframe::strideSize(), size_t(bytes.size()));
    vmd::LightKeyframe frame;
    frame.read(reinterpret_cast<const uint8_t *>(bytes.constData()));
    ASSERT_EQ(IKeyframe::TimeIndex(1.0), frame.timeIndex());
    ASSERT_TRUE(frame.color() == Vector3(0.2f, 0.3f, 0.4f));
#ifdef VPVL2_COORDINATE_OPENGL
    ASSERT_TRUE(frame.direction() == Vector3(0.5f, 0.6f, -0.7f));
#else
    ASSERT_TRUE(frame.direction() == Vector3(0.5f, 0.6f, 0.7f));
#endif
}

TEST(MotionTest, BoneInterpolation)
{
    Encoding encoding;
    vmd::BoneKeyframe frame(&encoding);
    QuadWord n;
    frame.getInterpolationParameter(vmd::BoneKeyframe::kX, n);
    AssertVector(n, QuadWord(0.0f, 0.0f, 0.0f, 0.0f));
    QuadWord px(8, 9, 10, 11),
            py(12, 13, 14, 15),
            pz(16, 17, 18, 19),
            pr(20, 21, 22, 23);
    QuadWord p[] = { px, py, pz, pr };
    frame.setInterpolationParameter(vmd::BoneKeyframe::kX, px);
    frame.setInterpolationParameter(vmd::BoneKeyframe::kY, py);
    frame.setInterpolationParameter(vmd::BoneKeyframe::kZ, pz);
    frame.setInterpolationParameter(vmd::BoneKeyframe::kRotation, pr);
    CompareBoneInterpolationMatrix(p, frame);
}

TEST(MotionTest, CameraInterpolation)
{
    vmd::CameraKeyframe frame;
    QuadWord n;
    frame.getInterpolationParameter(vmd::CameraKeyframe::kX, n);
    AssertVector(n, QuadWord(0.0f, 0.0f, 0.0f, 0.0f));
    QuadWord px(9, 10, 11, 12),
            py(13, 14, 15, 16),
            pz(17, 18, 19, 20),
            pr(21, 22, 23, 24),
            pd(25, 26, 27, 28),
            pf(29, 30, 31, 32);
    QuadWord p[] = { px, py, pz, pr, pd, pf };
    frame.setInterpolationParameter(vmd::CameraKeyframe::kX, px);
    frame.setInterpolationParameter(vmd::CameraKeyframe::kY, py);
    frame.setInterpolationParameter(vmd::CameraKeyframe::kZ, pz);
    frame.setInterpolationParameter(vmd::CameraKeyframe::kRotation, pr);
    frame.setInterpolationParameter(vmd::CameraKeyframe::kDistance, pd);
    frame.setInterpolationParameter(vmd::CameraKeyframe::kFov, pf);
    CompareCameraInterpolationMatrix(p, frame);
}

TEST(MotionTest, AddAndRemoveBoneKeyframes)
{
    Encoding encoding;
    CString name("bone");
    MockIModel model;
    MockIBone bone;
    vmd::Motion motion(&model, &encoding);
    ASSERT_EQ(0, motion.countKeyframes(IKeyframe::kBone));
    // mock bone
    EXPECT_CALL(model, findBone(_)).Times(AtLeast(1)).WillRepeatedly(Return(&bone));
    QScopedPointer<IBoneKeyframe> frame(new vmd::BoneKeyframe(&encoding));
    frame->setTimeIndex(42);
    frame->setName(&name);
    {
        // add a bone keyframe (don't forget updating motion!)
        motion.addKeyframe(frame.data());
        motion.update(IKeyframe::kBone);
        ASSERT_EQ(1, motion.countKeyframes(IKeyframe::kBone));
        // boudary check of findBoneKeyframeAt
        ASSERT_EQ(static_cast<IBoneKeyframe *>(0), motion.findBoneKeyframeAt(-1));
        ASSERT_EQ(frame.data(), motion.findBoneKeyframeAt(0));
        ASSERT_EQ(static_cast<IBoneKeyframe *>(0), motion.findBoneKeyframeAt(1));
        // find a bone keyframe with timeIndex and name
        ASSERT_EQ(frame.take(), motion.findBoneKeyframe(42, &name));
    }
    QScopedPointer<IBoneKeyframe> frame2(new vmd::BoneKeyframe(&encoding));
    frame2->setTimeIndex(42);
    frame2->setName(&name);
    {
        // replaced bone frame should be one keyframe (don't forget updating motion!)
        motion.replaceKeyframe(frame2.data());
        motion.update(IKeyframe::kBone);
        ASSERT_EQ(1, motion.countKeyframes(IKeyframe::kBone));
        // no longer be find previous bone keyframe
        ASSERT_EQ(frame2.data(), motion.findBoneKeyframe(42, &name));
    }
    {
        IKeyframe *keyframeToDelete = frame2.take();
        // delete bone keyframe and set it null (don't forget updating motion!)
        motion.deleteKeyframe(keyframeToDelete);
        motion.update(IKeyframe::kBone);
        // bone keyframes should be empty
        ASSERT_EQ(0, motion.countKeyframes(IKeyframe::kBone));
        ASSERT_EQ(static_cast<IBoneKeyframe *>(0), motion.findBoneKeyframe(42, &name));
        ASSERT_EQ(static_cast<IKeyframe *>(0), keyframeToDelete);
    }
}

TEST(MotionTest, AddAndRemoveCameraKeyframes)
{
    Encoding encoding;
    Model model(&encoding);
    vmd::Motion motion(&model, &encoding);
    ASSERT_EQ(0, motion.countKeyframes(IKeyframe::kCamera));
    QScopedPointer<ICameraKeyframe> frame(new vmd::CameraKeyframe());
    frame->setTimeIndex(42);
    frame->setDistance(42);
    {
        // add a camera keyframe (don't forget updating motion!)
        motion.addKeyframe(frame.data());
        motion.update(IKeyframe::kCamera);
        ASSERT_EQ(1, motion.countKeyframes(IKeyframe::kCamera));
        // boudary check of findCameraKeyframeAt
        ASSERT_EQ(static_cast<ICameraKeyframe *>(0), motion.findCameraKeyframeAt(-1));
        ASSERT_EQ(frame.data(), motion.findCameraKeyframeAt(0));
        ASSERT_EQ(static_cast<ICameraKeyframe *>(0), motion.findCameraKeyframeAt(1));
        // find a camera keyframe with timeIndex
        ASSERT_EQ(frame.take(), motion.findCameraKeyframe(42));
    }
    QScopedPointer<ICameraKeyframe> frame2(new vmd::CameraKeyframe());
    frame2->setTimeIndex(42);
    frame2->setDistance(84);
    {
        // replaced camera frame should be one keyframe (don't forget updating motion!)
        motion.replaceKeyframe(frame2.data());
        motion.update(IKeyframe::kCamera);
        ASSERT_EQ(1, motion.countKeyframes(IKeyframe::kCamera));
        // no longer be find previous camera keyframe
        ASSERT_EQ(84.0f, motion.findCameraKeyframe(42)->distance());
    }
    {
        IKeyframe *keyframeToDelete = frame2.take();
        // delete camera keyframe and set it null (don't forget updating motion!)
        motion.deleteKeyframe(keyframeToDelete);
        motion.update(IKeyframe::kCamera);
        // camera keyframes should be empty
        ASSERT_EQ(0, motion.countKeyframes(IKeyframe::kCamera));
        ASSERT_EQ(static_cast<ICameraKeyframe *>(0), motion.findCameraKeyframe(42));
        ASSERT_EQ(static_cast<IKeyframe *>(0), keyframeToDelete);
    }
}

TEST(MotionTest, AddAndRemoveLightKeyframes)
{
    Encoding encoding;
    Model model(&encoding);
    vmd::Motion motion(&model, &encoding);
    ASSERT_EQ(0, motion.countKeyframes(IKeyframe::kCamera));
    QScopedPointer<ILightKeyframe> frame(new vmd::LightKeyframe());
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
        // find a light keyframe with timeIndex
        ASSERT_EQ(frame.take(), motion.findLightKeyframe(42));
    }
    QScopedPointer<ILightKeyframe> frame2(new vmd::LightKeyframe());
    frame2->setTimeIndex(42);
    frame2->setColor(Vector3(0, 0, 1));
    {
        // replaced light frame should be one keyframe (don't forget updating motion!)
        motion.replaceKeyframe(frame2.data());
        motion.update(IKeyframe::kLight);
        ASSERT_EQ(1, motion.countKeyframes(IKeyframe::kLight));
        // no longer be find previous light keyframe
        ASSERT_EQ(1.0f, motion.findLightKeyframe(42)->color().z());
    }
    {
        IKeyframe *keyframeToDelete = frame2.take();
        // delete light keyframe and set it null (don't forget updating motion!)
        motion.deleteKeyframe(keyframeToDelete);
        motion.update(IKeyframe::kLight);
        // light keyframes should be empty
        ASSERT_EQ(0, motion.countKeyframes(IKeyframe::kLight));
        ASSERT_EQ(static_cast<ILightKeyframe *>(0), motion.findLightKeyframe(42));
        ASSERT_EQ(static_cast<IKeyframe *>(0), keyframeToDelete);
    }
}

TEST(MotionTest, AddAndRemoveMorphKeyframes)
{
    Encoding encoding;
    CString name("morph");
    MockIModel model;
    MockIMorph morph;
    vmd::Motion motion(&model, &encoding);
    ASSERT_EQ(0, motion.countKeyframes(IKeyframe::kMorph));
    // mock morph
    EXPECT_CALL(model, findMorph(_)).Times(AtLeast(1)).WillRepeatedly(Return(&morph));
    QScopedPointer<IMorphKeyframe> frame(new vmd::MorphKeyframe(&encoding));
    frame->setTimeIndex(42);
    frame->setName(&name);
    {
        // add a morph keyframe (don't forget updating motion!)
        motion.addKeyframe(frame.data());
        motion.update(IKeyframe::kMorph);
        ASSERT_EQ(1, motion.countKeyframes(IKeyframe::kMorph));
        // boudary check of findMorphKeyframeAt
        ASSERT_EQ(static_cast<IMorphKeyframe *>(0), motion.findMorphKeyframeAt(-1));
        ASSERT_EQ(frame.data(), motion.findMorphKeyframeAt(0));
        ASSERT_EQ(static_cast<IMorphKeyframe *>(0), motion.findMorphKeyframeAt(1));
        ASSERT_EQ(frame.take(), motion.findMorphKeyframe(42, &name));
    }
    QScopedPointer<IMorphKeyframe> frame2(new vmd::MorphKeyframe(&encoding));
    frame2->setTimeIndex(42);
    frame2->setName(&name);
    {
        // replaced morph frame should be one keyframe (don't forget updating motion!)
        motion.replaceKeyframe(frame2.data());
        motion.update(IKeyframe::kMorph);
        ASSERT_EQ(1, motion.countKeyframes(IKeyframe::kMorph));
        // no longer be find previous morph keyframe
        ASSERT_EQ(frame2.data(), motion.findMorphKeyframe(42, &name));
    }
    {
        IKeyframe *keyframeToDelete = frame2.take();
        // delete light keyframe and set it null (don't forget updating motion!)
        motion.deleteKeyframe(keyframeToDelete);
        motion.update(IKeyframe::kMorph);
        // morph keyframes should be empty
        ASSERT_EQ(0, motion.countKeyframes(IKeyframe::kMorph));
        ASSERT_EQ(static_cast<IMorphKeyframe *>(0), motion.findMorphKeyframe(42, &name));
        ASSERT_EQ(static_cast<IKeyframe *>(0), keyframeToDelete);
    }
}

TEST(MotionTest, ParseMVD)
{
    QFile file("motion.mvd");
    if (file.open(QFile::ReadOnly)) {
        QByteArray bytes = file.readAll();
        const uint8_t *data = reinterpret_cast<const uint8_t *>(bytes.constData());
        size_t size = bytes.size();
        Encoding encoding;
        MockIModel model;
        MockIBone bone;
        MockIMorph morph;
        EXPECT_CALL(model, findBone(_)).Times(AtLeast(1)).WillRepeatedly(Return(&bone));
        EXPECT_CALL(model, findMorph(_)).Times(AtLeast(1)).WillRepeatedly(Return(&morph));
        mvd::Motion motion(&model, &encoding);
        mvd::Motion::DataInfo result;
        // valid model motion should be loaded successfully
        ASSERT_TRUE(motion.preparse(data, size, result));
        ASSERT_TRUE(motion.load(data, size));
    }
}
