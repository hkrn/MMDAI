#include <QtTest/QtTest>
#include "../common.h"

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

using namespace vpvl2::pmx;
using namespace vpvl2::vmd;

class TestVMDMotion : public QObject
{
    Q_OBJECT

public:
    static const char *kTestString;

    TestVMDMotion();

private Q_SLOTS:
    void parseEmpty();
    void parseMotion();
    void parseCamera();
    void parseBoneKeyframe();
    void parseCameraKeyframe();
    void parseMorphKeyframe();
    void parseLightKeyframe();
    void saveBoneKeyframe();
    void saveCameraKeyframe();
    void saveMorphKeyframe();
    void saveLightKeyframe();
    void saveMotion();
    void boneInterpolation();
    void cameraInterpolation();
    void mutateBoneKeyframes() const;
    void mutateCameraKeyframes() const;
    void mutateLightKeyframes() const;
    void mutateMorphKeyframes() const;

private:
    void testBoneInterpolationMatrix(const QuadWord p[4], const BoneKeyframe &frame);
    void testCameraInterpolationMatrix(const QuadWord p[6], const CameraKeyframe &frame);
};

const char *TestVMDMotion::kTestString = "012345678901234";

static void CompareQuadWord(const QuadWord &actual, const QuadWord &expected)
{
    QCOMPARE(actual.x(), expected.x());
    QCOMPARE(actual.y(), expected.y());
    QCOMPARE(actual.z(), expected.z());
    QCOMPARE(actual.w(), expected.w());
}

TestVMDMotion::TestVMDMotion()
{
}

void TestVMDMotion::parseEmpty()
{
    Encoding encoding;
    Model model(&encoding);
    Motion motion(&model, &encoding);
    Motion::DataInfo info;
    // parsing empty should be error
    QVERIFY(!motion.preparse(reinterpret_cast<const uint8_t *>(""), 0, info));
    QCOMPARE(motion.error(), Motion::kInvalidHeaderError);
}

void TestVMDMotion::parseMotion()
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
        QVERIFY(motion.preparse(data, size, result));
        QVERIFY(motion.load(data, size));
        QCOMPARE(result.boneKeyframeCount, size_t(motion.boneAnimation().countKeyframes()));
        QCOMPARE(result.cameraKeyframeCount, size_t(motion.cameraAnimation().countKeyframes()));
        QCOMPARE(result.morphKeyframeCount, size_t(motion.morphAnimation().countKeyframes()));
        QCOMPARE(motion.error(), Motion::kNoError);
    }
    else {
        QSKIP("Require a motion to test this", SkipSingle);
    }
}

void TestVMDMotion::parseCamera()
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
        QVERIFY(motion.preparse(data, size, result));
        QVERIFY(motion.load(data, size));
        QCOMPARE(result.boneKeyframeCount, size_t(motion.boneAnimation().countKeyframes()));
        QCOMPARE(result.cameraKeyframeCount, size_t(motion.cameraAnimation().countKeyframes()));
        QCOMPARE(result.morphKeyframeCount, size_t(motion.morphAnimation().countKeyframes()));
        QCOMPARE(motion.error(), Motion::kNoError);
    }
    else {
        QSKIP("Require a motion to test this", SkipSingle);
    }
}

void TestVMDMotion::saveBoneKeyframe()
{
    Encoding encoding;
    String str(kTestString);
    BoneKeyframe frame(&encoding), newFrame(&encoding);
    Vector3 pos(1, 2, 3);
    Quaternion rot(4, 5, 6, 7);
    // initialize the bone frame to be copied
    frame.setFrameIndex(42);
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
    QVERIFY(newFrame.name()->equals(frame.name()));
    QCOMPARE(newFrame.frameIndex(), frame.frameIndex());
    QVERIFY(newFrame.position() == pos);
    QVERIFY(newFrame.rotation() == rot);
    testBoneInterpolationMatrix(p, frame);
    // cloned bone frame shold be copied with deep
    QScopedPointer<IBoneKeyframe> cloned(frame.clone());
    QVERIFY(cloned->name()->equals(frame.name()));
    QCOMPARE(cloned->frameIndex(), frame.frameIndex());
    QVERIFY(cloned->position() == pos);
    QVERIFY(cloned->rotation() == rot);
    testBoneInterpolationMatrix(p, *static_cast<BoneKeyframe *>(cloned.data()));
}

void TestVMDMotion::saveCameraKeyframe()
{
    CameraKeyframe frame, newFrame;
    Vector3 pos(1, 2, 3), angle(4, 5, 6);
    // initialize the camera frame to be copied
    frame.setFrameIndex(42);
    frame.setPosition(pos);
    frame.setAngle(angle);
    frame.setDistance(7);
    frame.setFovy(8);
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
    frame.setInterpolationParameter(CameraKeyframe::kFovy, pf);
    // write a camera frame to data and read it
    uint8_t data[CameraKeyframe::strideSize()];
    frame.write(data);
    newFrame.read(data);
    QCOMPARE(newFrame.frameIndex(), frame.frameIndex());
    QVERIFY(newFrame.position() == frame.position());
    // compare read camera frame
    // for radian and degree calculation
    QVERIFY(qFuzzyCompare(newFrame.angle().x(), frame.angle().x()));
    QVERIFY(qFuzzyCompare(newFrame.angle().y(), frame.angle().y()));
    QVERIFY(qFuzzyCompare(newFrame.angle().z(), frame.angle().z()));
    QVERIFY(newFrame.distance() == frame.distance());
    QVERIFY(newFrame.fovy() == frame.fovy());
    testCameraInterpolationMatrix(p, frame);
    // cloned camera frame shold be copied with deep
    QScopedPointer<ICameraKeyframe> cloned(frame.clone());
    QCOMPARE(cloned->frameIndex(), frame.frameIndex());
    QVERIFY(cloned->position() == frame.position());
    // for radian and degree calculation
    QVERIFY(qFuzzyCompare(cloned->angle().x(), frame.angle().x()));
    QVERIFY(qFuzzyCompare(cloned->angle().y(), frame.angle().y()));
    QVERIFY(qFuzzyCompare(cloned->angle().z(), frame.angle().z()));
    QVERIFY(cloned->distance() == frame.distance());
    QVERIFY(cloned->fovy() == frame.fovy());
    testCameraInterpolationMatrix(p, *static_cast<CameraKeyframe *>(cloned.data()));
}

void TestVMDMotion::saveMorphKeyframe()
{
    Encoding encoding;
    String str(kTestString);
    MorphKeyframe frame(&encoding), newFrame(&encoding);
    // initialize the morph frame to be copied
    frame.setName(&str);
    frame.setFrameIndex(42);
    frame.setWeight(0.5);
    // write a morph frame to data and read it
    uint8_t data[MorphKeyframe::strideSize()];
    frame.write(data);
    newFrame.read(data);
    // compare read morph frame
    QVERIFY(newFrame.name()->equals(frame.name()));
    QCOMPARE(newFrame.frameIndex(), frame.frameIndex());
    QCOMPARE(newFrame.weight(), frame.weight());
    // cloned morph frame shold be copied with deep
    QScopedPointer<IMorphKeyframe> cloned(frame.clone());
    QVERIFY(cloned->name()->equals(frame.name()));
    QCOMPARE(cloned->frameIndex(), frame.frameIndex());
    QCOMPARE(cloned->weight(), frame.weight());
}

void TestVMDMotion::saveLightKeyframe()
{
    LightKeyframe frame, newFrame;
    Vector3 color(0.1, 0.2, 0.3), direction(4, 5, 6);
    // initialize the light frame to be copied
    frame.setFrameIndex(42);
    frame.setColor(color);
    frame.setDirection(direction);
    // write a light frame to data and read it
    uint8_t data[LightKeyframe::strideSize()];
    frame.write(data);
    newFrame.read(data);
    // compare read light frame
    QCOMPARE(newFrame.frameIndex(), frame.frameIndex());
    QVERIFY(newFrame.color() == frame.color());
    QVERIFY(newFrame.direction() == frame.direction());
    // cloned morph frame shold be copied with deep
    QScopedPointer<ILightKeyframe> cloned(frame.clone());
    QCOMPARE(cloned->frameIndex(), frame.frameIndex());
    QVERIFY(cloned->color() == frame.color());
    QVERIFY(cloned->direction() == frame.direction());
}

void TestVMDMotion::saveMotion()
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
        QCOMPARE(newSize, size);
    }
    else {
        QSKIP("Require a motion to test this", SkipSingle);
    }
}

void TestVMDMotion::parseBoneKeyframe()
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
    QCOMPARE(size_t(bytes.size()), BoneKeyframe::strideSize());
    Encoding encoding;
    BoneKeyframe frame(&encoding);
    String str(kTestString);
    frame.read(reinterpret_cast<const uint8_t *>(bytes.constData()));
    QVERIFY(frame.name()->equals(&str));
    QCOMPARE(frame.frameIndex(), 1.0f);
#ifdef VPVL2_COORDINATE_OPENGL
    QVERIFY(frame.position() == Vector3(2.0f, 3.0f, -4.0f));
    QVERIFY(frame.rotation() == Quaternion(-5.0f, -6.0f, 7.0f, 8.0f));
#else
    QVERIFY(frame.position() == Vector3(2.0f, 3.0f, 4.0f));
    QVERIFY(frame.rotation() == Quaternion(5.0f, 6.0f, 7.0f, 8.0f));
#endif
}

void TestVMDMotion::parseCameraKeyframe()
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
    QCOMPARE(size_t(bytes.size()), CameraKeyframe::strideSize());
    CameraKeyframe frame;
    frame.read(reinterpret_cast<const uint8_t *>(bytes.constData()));
    QCOMPARE(frame.frameIndex(), 1.0f);
#ifdef VPVL2_COORDINATE_OPENGL
    QCOMPARE(frame.distance(), -1.0f);
    QVERIFY(frame.position() == Vector3(2.0f, 3.0f, -4.0f));
    QVERIFY(frame.angle() == Vector3(-degree(5.0f), -degree(6.0f), degree(7.0f)));
#else
    QCOMPARE(frame.distance(), 1.0f);
    QVERIFY(frame.position() == Vector3(2.0f, 3.0f, 4.0f));
    QVERIFY(frame.angle() == Vector3(degree(5.0f), degree(6.0f), degree(7.0f)));
#endif
    QCOMPARE(frame.fovy(), 8.0f);
    // TODO: perspective flag
}

void TestVMDMotion::parseMorphKeyframe()
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.writeRawData(kTestString, MorphKeyframe::kNameSize);
    stream << quint32(1) // frame index
           << 0.5f       // weight
              ;
    QCOMPARE(size_t(bytes.size()), MorphKeyframe::strideSize());
    Encoding encoding;
    MorphKeyframe frame(&encoding);
    String str(kTestString);
    frame.read(reinterpret_cast<const uint8_t *>(bytes.constData()));
    QVERIFY(frame.name()->equals(&str));
    QCOMPARE(frame.frameIndex(), 1.0f);
    QCOMPARE(frame.weight(), 0.5f);
}

void TestVMDMotion::parseLightKeyframe()
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream << quint32(1)           // frame index
           << 0.2f << 0.3f << 0.4f // color
           << 0.5f << 0.6f << 0.7f // direction
              ;
    QCOMPARE(size_t(bytes.size()), LightKeyframe::strideSize());
    LightKeyframe frame;
    frame.read(reinterpret_cast<const uint8_t *>(bytes.constData()));
    QCOMPARE(frame.frameIndex(), 1.0f);
    QVERIFY(frame.color() == Vector3(0.2f, 0.3f, 0.4f));
#ifdef VPVL2_COORDINATE_OPENGL
    QVERIFY(frame.direction() == Vector3(-0.5f, -0.6f, 0.7f));
#else
    QVERIFY(frame.direction() == Vector3(0.5f, 0.6f, 0.7f));
#endif
}

void TestVMDMotion::boneInterpolation()
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
    testBoneInterpolationMatrix(p, frame);
}

void TestVMDMotion::cameraInterpolation()
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
    frame.setInterpolationParameter(CameraKeyframe::kFovy, pf);
    testCameraInterpolationMatrix(p, frame);
}

void TestVMDMotion::mutateBoneKeyframes() const
{
    QSKIP("this need mock", SkipSingle);
    Encoding encoding;
    String s("bone"), s2("bone2");
    Model model(&encoding);
    Motion motion(&model, &encoding);
    QCOMPARE(motion.countKeyframes(IKeyframe::kBone), 0);
    QScopedPointer<IBoneKeyframe> frame(new BoneKeyframe(&encoding));
    frame->setFrameIndex(42);
    frame->setName(&s);
    // add a bone keyframe
    motion.addKeyframe(frame.data());
    motion.update(IKeyframe::kBone);
    QCOMPARE(motion.countKeyframes(IKeyframe::kBone), 1);
    // boudary check of findBoneKeyframeAt
    QCOMPARE(motion.findBoneKeyframeAt(-1), static_cast<IBoneKeyframe *>(0));
    QCOMPARE(motion.findBoneKeyframeAt(0), frame.data());
    QCOMPARE(motion.findBoneKeyframeAt(1), static_cast<IBoneKeyframe *>(0));
    // find a bone keyframe with frameIndex and name
    QCOMPARE(motion.findBoneKeyframe(42, &s), frame.take());
    QScopedPointer<IBoneKeyframe> frame2(new BoneKeyframe(&encoding));
    frame2->setFrameIndex(42);
    frame2->setName(&s2);
    // replaced bone frame should be one keyframe
    motion.replaceKeyframe(frame2.data());
    QCOMPARE(motion.countKeyframes(IKeyframe::kBone), 1);
    // no longer be find previous bone keyframe
    QCOMPARE(motion.findBoneKeyframe(42, &s), static_cast<IBoneKeyframe *>(0));
    QCOMPARE(motion.findBoneKeyframe(42, &s2), frame2.data());
    IKeyframe *keyframeToDelete = frame2.take();
    // delete bone keyframe and set it null (don't forget updating motion!)
    motion.deleteKeyframe(keyframeToDelete);
    motion.update(IKeyframe::kBone);
    // bone keyframes should be empty
    QCOMPARE(motion.countKeyframes(IKeyframe::kBone), 0);
    QCOMPARE(motion.findBoneKeyframe(42, &s2), static_cast<IBoneKeyframe *>(0));
    QCOMPARE(keyframeToDelete, static_cast<IKeyframe *>(0));
}

void TestVMDMotion::mutateCameraKeyframes() const
{
    Encoding encoding;
    Model model(&encoding);
    Motion motion(&model, &encoding);
    QCOMPARE(motion.countKeyframes(IKeyframe::kCamera), 0);
    QScopedPointer<ICameraKeyframe> frame(new CameraKeyframe());
    frame->setFrameIndex(42);
    frame->setDistance(42);
    // add a camera keyframe
    motion.addKeyframe(frame.data());
    QCOMPARE(motion.countKeyframes(IKeyframe::kCamera), 1);
    // boudary check of findCameraKeyframeAt
    QCOMPARE(motion.findCameraKeyframeAt(-1), static_cast<ICameraKeyframe *>(0));
    QCOMPARE(motion.findCameraKeyframeAt(0), frame.data());
    QCOMPARE(motion.findCameraKeyframeAt(1), static_cast<ICameraKeyframe *>(0));
    // find a camera keyframe with frameIndex
    QCOMPARE(motion.findCameraKeyframe(42), frame.take());
    QScopedPointer<ICameraKeyframe> frame2(new CameraKeyframe());
    frame2->setFrameIndex(42);
    frame2->setDistance(84);
    // replaced camera frame should be one keyframe
    motion.replaceKeyframe(frame2.data());
    QCOMPARE(motion.countKeyframes(IKeyframe::kCamera), 1);
    // no longer be find previous camera keyframe
    QCOMPARE(motion.findCameraKeyframe(42)->distance(), 84.0f);
    IKeyframe *keyframeToDelete = frame2.take();
    // delete camera keyframe and set it null
    motion.deleteKeyframe(keyframeToDelete);
    // camera keyframes should be empty
    QCOMPARE(motion.countKeyframes(IKeyframe::kCamera), 0);
    QCOMPARE(motion.findCameraKeyframe(42), static_cast<ICameraKeyframe *>(0));
    QCOMPARE(keyframeToDelete, static_cast<IKeyframe *>(0));
}

void TestVMDMotion::mutateLightKeyframes() const
{
    Encoding encoding;
    Model model(&encoding);
    Motion motion(&model, &encoding);
    QCOMPARE(motion.countKeyframes(IKeyframe::kCamera), 0);
    QScopedPointer<ILightKeyframe> frame(new LightKeyframe());
    frame->setFrameIndex(42);
    frame->setColor(Vector3(1, 0, 0));
    // add a light keyframe
    motion.addKeyframe(frame.data());
    QCOMPARE(motion.countKeyframes(IKeyframe::kLight), 1);
    // boudary check of findLightKeyframeAt
    QCOMPARE(motion.findLightKeyframeAt(-1), static_cast<ILightKeyframe *>(0));
    QCOMPARE(motion.findLightKeyframeAt(0), frame.data());
    QCOMPARE(motion.findLightKeyframeAt(1), static_cast<ILightKeyframe *>(0));
    // find a light keyframe with frameIndex
    QCOMPARE(motion.findLightKeyframe(42), frame.take());
    QScopedPointer<ILightKeyframe> frame2(new LightKeyframe());
    frame2->setFrameIndex(42);
    frame2->setColor(Vector3(0, 0, 1));
    // replaced light frame should be one keyframe
    motion.replaceKeyframe(frame2.data());
    QCOMPARE(motion.countKeyframes(IKeyframe::kLight), 1);
    // no longer be find previous light keyframe
    QCOMPARE(motion.findLightKeyframe(42)->color().z(), 1.0f);
    IKeyframe *keyframeToDelete = frame2.take();
    // delete light keyframe and set it null
    motion.deleteKeyframe(keyframeToDelete);
    // light keyframes should be empty
    QCOMPARE(motion.countKeyframes(IKeyframe::kLight), 0);
    QCOMPARE(motion.findLightKeyframe(42), static_cast<ILightKeyframe *>(0));
    QCOMPARE(keyframeToDelete, static_cast<IKeyframe *>(0));
}

void TestVMDMotion::mutateMorphKeyframes() const
{
    QSKIP("this need mock", SkipSingle);
    Encoding encoding;
    String s("morph"), s2("morph2");
    Model model(&encoding);
    Motion motion(&model, &encoding);
    QCOMPARE(motion.countKeyframes(IKeyframe::kMorph), 0);
    QScopedPointer<IMorphKeyframe> frame(new MorphKeyframe(&encoding));
    frame->setFrameIndex(42);
    frame->setName(&s);
    // add a morph keyframe
    motion.addKeyframe(frame.data());
    motion.update(IKeyframe::kMorph);
    QCOMPARE(motion.countKeyframes(IKeyframe::kMorph), 1);
    // boudary check of findMorphKeyframeAt
    QCOMPARE(motion.findMorphKeyframeAt(-1), static_cast<IMorphKeyframe *>(0));
    QCOMPARE(motion.findMorphKeyframeAt(0), frame.data());
    QCOMPARE(motion.findMorphKeyframeAt(1), static_cast<IMorphKeyframe *>(0));
    QCOMPARE(motion.findMorphKeyframe(42, &s), frame.take());
    QScopedPointer<IMorphKeyframe> frame2(new MorphKeyframe(&encoding));
    frame2->setFrameIndex(42);
    frame2->setName(&s2);
    // replaced morph frame should be one keyframe
    motion.replaceKeyframe(frame2.data());
    QCOMPARE(motion.countKeyframes(IKeyframe::kMorph), 1);
    // no longer be find previous morph keyframe
    QCOMPARE(motion.findMorphKeyframe(42, &s), static_cast<IMorphKeyframe *>(0));
    QCOMPARE(motion.findMorphKeyframe(42, &s2), frame2.data());
    IKeyframe *keyframeToDelete = frame2.take();
    // delete light keyframe and set it null (don't forget updating motion!)
    motion.deleteKeyframe(keyframeToDelete);
    motion.update(IKeyframe::kMorph);
    // morph keyframes should be empty
    QCOMPARE(motion.countKeyframes(IKeyframe::kMorph), 0);
    QCOMPARE(motion.findMorphKeyframe(42, &s2), static_cast<IMorphKeyframe *>(0));
    QCOMPARE(keyframeToDelete, static_cast<IKeyframe *>(0));
}

void TestVMDMotion::testBoneInterpolationMatrix(const QuadWord p[], const BoneKeyframe &frame)
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

void TestVMDMotion::testCameraInterpolationMatrix(const QuadWord p[], const CameraKeyframe &frame)
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
    frame.getInterpolationParameter(CameraKeyframe::kFovy, actual);
    CompareQuadWord(actual, expected);
}

QTEST_APPLESS_MAIN(TestVMDMotion)

#include "TestVMDMotion.moc"

