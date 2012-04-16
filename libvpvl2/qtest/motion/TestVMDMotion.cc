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
    uint8_t data[BoneKeyframe::strideSize()];
    frame.write(data);
    newFrame.read(data);
    QVERIFY(newFrame.name()->equals(frame.name()));
    QCOMPARE(newFrame.frameIndex(), frame.frameIndex());
    QVERIFY(newFrame.position() == pos);
    QVERIFY(newFrame.rotation() == rot);
    testBoneInterpolationMatrix(p, frame);
    IBoneKeyframe *cloned = reinterpret_cast<IBoneKeyframe *>(frame.clone());
    QVERIFY(cloned->name()->equals(frame.name()));
    QCOMPARE(cloned->frameIndex(), frame.frameIndex());
    QVERIFY(cloned->position() == pos);
    QVERIFY(cloned->rotation() == rot);
    testBoneInterpolationMatrix(p, *static_cast<BoneKeyframe *>(cloned));
    delete cloned;
}

void TestVMDMotion::saveCameraKeyframe()
{
    CameraKeyframe frame, newFrame;
    Vector3 pos(1, 2, 3), angle(4, 5, 6);
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
    uint8_t data[CameraKeyframe::strideSize()];
    frame.write(data);
    newFrame.read(data);
    QCOMPARE(newFrame.frameIndex(), frame.frameIndex());
    QVERIFY(newFrame.position() == frame.position());
    // for radian and degree calculation
    QVERIFY(qFuzzyCompare(newFrame.angle().x(), frame.angle().x()));
    QVERIFY(qFuzzyCompare(newFrame.angle().y(), frame.angle().y()));
    QVERIFY(qFuzzyCompare(newFrame.angle().z(), frame.angle().z()));
    QVERIFY(newFrame.distance() == frame.distance());
    QVERIFY(newFrame.fovy() == frame.fovy());
    testCameraInterpolationMatrix(p, frame);
    ICameraKeyframe *cloned = reinterpret_cast<ICameraKeyframe *>(frame.clone());
    QCOMPARE(cloned->frameIndex(), frame.frameIndex());
    QVERIFY(cloned->position() == frame.position());
    // for radian and degree calculation
    QVERIFY(qFuzzyCompare(cloned->angle().x(), frame.angle().x()));
    QVERIFY(qFuzzyCompare(cloned->angle().y(), frame.angle().y()));
    QVERIFY(qFuzzyCompare(cloned->angle().z(), frame.angle().z()));
    QVERIFY(cloned->distance() == frame.distance());
    QVERIFY(cloned->fovy() == frame.fovy());
    testCameraInterpolationMatrix(p, *static_cast<CameraKeyframe *>(cloned));
    delete cloned;
}

void TestVMDMotion::saveMorphKeyframe()
{
    Encoding encoding;
    String str(kTestString);
    MorphKeyframe frame(&encoding), newFrame(&encoding), *cloned;
    frame.setName(&str);
    frame.setFrameIndex(42);
    frame.setWeight(0.5);
    uint8_t data[MorphKeyframe::strideSize()];
    frame.write(data);
    newFrame.read(data);
    QVERIFY(newFrame.name()->equals(frame.name()));
    QCOMPARE(newFrame.frameIndex(), frame.frameIndex());
    QCOMPARE(newFrame.weight(), frame.weight());
    cloned = static_cast<MorphKeyframe *>(frame.clone());
    QVERIFY(cloned->name()->equals(frame.name()));
    QCOMPARE(cloned->frameIndex(), frame.frameIndex());
    QCOMPARE(cloned->weight(), frame.weight());
    delete cloned;
}

void TestVMDMotion::saveLightKeyframe()
{
    LightKeyframe frame, newFrame;
    Vector3 color(0.1, 0.2, 0.3), direction(4, 5, 6);
    frame.setFrameIndex(42);
    frame.setColor(color);
    frame.setDirection(direction);
    uint8_t data[LightKeyframe::strideSize()];
    frame.write(data);
    newFrame.read(data);
    QCOMPARE(newFrame.frameIndex(), frame.frameIndex());
    QVERIFY(newFrame.color() == frame.color());
    QVERIFY(newFrame.direction() == frame.direction());
    ILightKeyframe *cloned = reinterpret_cast<ILightKeyframe *>(frame.clone());
    QCOMPARE(cloned->frameIndex(), frame.frameIndex());
    QVERIFY(cloned->color() == frame.color());
    QVERIFY(cloned->direction() == frame.direction());
    delete cloned;
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
        uint8_t *newData = new uint8_t[newSize];
        motion.save(newData);
        QTemporaryFile file2;
        file2.setAutoRemove(true);
        file2.write(reinterpret_cast<const char *>(newData), newSize);
        delete[] newData;
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

