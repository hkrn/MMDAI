#include <QtCore/QtCore>
#include <QtTest/QtTest>

#include <vpvl/vpvl.h>

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
    void parseBoneKeyFrame();
    void parseCameraKeyFrame();
    void parseFaceKeyFrame();
    void saveMotion();
    void boneInterpolation();
    void cameraInterpolation();
};

const char *TestVMDMotion::kTestString = "01234567890123";

TestVMDMotion::TestVMDMotion()
{
}

void TestVMDMotion::parseEmpty()
{
    vpvl::VMDMotion motion;
    vpvl::VMDMotionDataInfo info;
    QVERIFY(!motion.preparse(reinterpret_cast<const uint8_t *>(""), 0, info));
    QCOMPARE(motion.error(), vpvl::VMDMotion::kInvalidHeaderError);
}

void TestVMDMotion::parseMotion()
{
    QFile file("../../gtest/res/motion.vmd");
    if (file.open(QFile::ReadOnly)) {
        QByteArray bytes = file.readAll();
        const uint8_t *data = reinterpret_cast<const uint8_t *>(bytes.constData());
        size_t size = bytes.size();
        vpvl::VMDMotion motion;
        vpvl::VMDMotionDataInfo result;
        QVERIFY(motion.preparse(data, size, result));
        QVERIFY(motion.load(data, size));
        QCOMPARE(result.boneKeyFrameCount, size_t(motion.boneAnimation().countFrames()));
        QCOMPARE(result.cameraKeyFrameCount, motion.cameraAnimation().countFrames());
        QCOMPARE(result.faceKeyFrameCount, size_t(motion.faceAnimation().countFrames()));
        QCOMPARE(motion.error(), vpvl::VMDMotion::kNoError);
    }
    else {
        QSKIP("Require a motion to test this", SkipSingle);
    }
}

void TestVMDMotion::parseCamera()
{
    QFile file("../../gtest/res/camera.vmd");
    if (file.open(QFile::ReadOnly)) {
        QByteArray bytes = file.readAll();
        const uint8_t *data = reinterpret_cast<const uint8_t *>(bytes.constData());
        size_t size = bytes.size();
        vpvl::VMDMotion motion;
        vpvl::VMDMotionDataInfo result;
        QVERIFY(motion.preparse(data, size, result));
        QVERIFY(motion.load(data, size));
        QCOMPARE(result.boneKeyFrameCount, size_t(motion.boneAnimation().countFrames()));
        QCOMPARE(result.cameraKeyFrameCount, size_t(motion.cameraAnimation().countFrames()));
        QCOMPARE(result.faceKeyFrameCount, size_t(motion.faceAnimation().countFrames()));
        QCOMPARE(motion.error(), vpvl::VMDMotion::kNoError);
    }
    else {
        QSKIP("Require a motion to test this", SkipSingle);
    }
}

void TestVMDMotion::saveMotion()
{
    QFile file("../../gtest/res/motion.vmd");
    if (file.open(QFile::ReadOnly)) {
        QByteArray bytes = file.readAll();
        const uint8_t *data = reinterpret_cast<const uint8_t *>(bytes.constData());
        size_t size = bytes.size();
        vpvl::VMDMotion motion;
        motion.load(data, size);
        size_t newSize = motion.estimateSize();
        uint8_t *newData = new uint8_t[newSize];
        motion.save(newData);
        QFile file2("motion2.vmd");
        file2.open(QFile::WriteOnly);
        file2.write(reinterpret_cast<const char *>(newData), newSize);
        delete[] newData;
        QCOMPARE(newSize, size);
    }
    else {
        QSKIP("Require a motion to test this", SkipSingle);
    }
}

void TestVMDMotion::parseBoneKeyFrame()
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.writeRawData(kTestString, vpvl::BoneKeyFrame::kNameSize);
    stream << quint32(1)                   // frame index
           << 2.0f << 3.0f << 4.0f         // position
           << 5.0f << 6.0f << 7.0f << 8.0f // rotation
              ;
    stream.writeRawData(kTestString, vpvl::BoneKeyFrame::kTableSize);
    QCOMPARE(size_t(bytes.size()), vpvl::BoneKeyFrame::strideSize());
    vpvl::BoneKeyFrame frame;
    frame.read(reinterpret_cast<const uint8_t *>(bytes.constData()));
    QCOMPARE(QString(reinterpret_cast<const char *>(frame.name())), QString(kTestString));
    QCOMPARE(frame.frameIndex(), 1.0f);
#ifdef VPVL_COORDINATE_OPENGL
    QVERIFY(frame.position() == btVector3(2.0f, 3.0f, -4.0f));
    QVERIFY(frame.rotation() == btQuaternion(-5.0f, -6.0f, 7.0f, 8.0f));
#else
    QVERIFY(frame.position() == btVector3(2.0f, 3.0f, 4.0f));
    QVERIFY(frame.rotation() == btQuaternion(5.0f, 6.0f, 7.0f, 8.0f));
#endif
}

void TestVMDMotion::parseCameraKeyFrame()
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
    stream.writeRawData("", vpvl::CameraKeyFrame::kTableSize);
    stream << quint32(8)           // view angle (fovy)
           << quint8(1)            // no perspective
              ;
    QCOMPARE(size_t(bytes.size()), vpvl::CameraKeyFrame::strideSize());
    vpvl::CameraKeyFrame frame;
    frame.read(reinterpret_cast<const uint8_t *>(bytes.constData()));
    QCOMPARE(frame.frameIndex(), 1.0f);
#ifdef VPVL_COORDINATE_OPENGL
    QCOMPARE(frame.distance(), -1.0f);
    QVERIFY(frame.position() == btVector3(2.0f, 3.0f, -4.0f));
    QVERIFY(frame.angle() == btVector3(-vpvl::degree(5.0f), -vpvl::degree(6.0f), vpvl::degree(7.0f)));
#else
    QCOMPARE(frame.distance(), 1.0f);
    QVERIFY(frame.position() == btVector3(2.0f, 3.0f, 4.0f));
    QVERIFY(frame.angle() == btVector3(vpvl::degree(5.0f), vpvl::degree(6.0f), vpvl::degree(7.0f)));
#endif
    QCOMPARE(frame.fovy(), 8.0f);
    // TODO: perspective flag
}

void TestVMDMotion::parseFaceKeyFrame()
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
    stream.setByteOrder(QDataStream::LittleEndian);
    stream.writeRawData(kTestString, vpvl::FaceKeyFrame::kNameSize);
    stream << quint32(1) // frame index
           << 0.5f       // weight
              ;
    QCOMPARE(size_t(bytes.size()), vpvl::FaceKeyFrame::strideSize());
    vpvl::FaceKeyFrame frame;
    frame.read(reinterpret_cast<const uint8_t *>(bytes.constData()));
    QCOMPARE(QString(reinterpret_cast<const char *>(frame.name())), QString(kTestString));
    QCOMPARE(frame.frameIndex(), 1.0f);
    QCOMPARE(frame.weight(), 0.5f);
}

void TestVMDMotion::boneInterpolation()
{
    vpvl::BoneKeyFrame frame;
    int8_t x1, x2, y1, y2;
    frame.getInterpolationParameter(vpvl::BoneKeyFrame::kX, x1, x2, y1, y2);
    QCOMPARE(x1, int8_t(0));
    QCOMPARE(x2, int8_t(0));
    QCOMPARE(y1, int8_t(0));
    QCOMPARE(y2, int8_t(0));
    frame.setInterpolationParameter(vpvl::BoneKeyFrame::kX, 1, 2, 3, 4);
    frame.setInterpolationParameter(vpvl::BoneKeyFrame::kY, 5, 6, 7, 8);
    frame.setInterpolationParameter(vpvl::BoneKeyFrame::kZ, 9, 10, 11, 12);
    frame.setInterpolationParameter(vpvl::BoneKeyFrame::kRotation, 13, 14, 15, 16);
    frame.getInterpolationParameter(vpvl::BoneKeyFrame::kX, x1, x2, y1, y2);
    QCOMPARE(x1, int8_t(1));
    QCOMPARE(x2, int8_t(2));
    QCOMPARE(y1, int8_t(3));
    QCOMPARE(y2, int8_t(4));
    frame.getInterpolationParameter(vpvl::BoneKeyFrame::kY, x1, x2, y1, y2);
    QCOMPARE(x1, int8_t(5));
    QCOMPARE(x2, int8_t(6));
    QCOMPARE(y1, int8_t(7));
    QCOMPARE(y2, int8_t(8));
    frame.getInterpolationParameter(vpvl::BoneKeyFrame::kZ, x1, x2, y1, y2);
    QCOMPARE(x1, int8_t(9));
    QCOMPARE(x2, int8_t(10));
    QCOMPARE(y1, int8_t(11));
    QCOMPARE(y2, int8_t(12));
    frame.getInterpolationParameter(vpvl::BoneKeyFrame::kRotation, x1, x2, y1, y2);
    QCOMPARE(x1, int8_t(13));
    QCOMPARE(x2, int8_t(14));
    QCOMPARE(y1, int8_t(15));
    QCOMPARE(y2, int8_t(16));
}

void TestVMDMotion::cameraInterpolation()
{
    vpvl::CameraKeyFrame frame;
    int8_t x1, x2, y1, y2;
    frame.getInterpolationParameter(vpvl::CameraKeyFrame::kX, x1, x2, y1, y2);
    QCOMPARE(x1, int8_t(0));
    QCOMPARE(x2, int8_t(0));
    QCOMPARE(y1, int8_t(0));
    QCOMPARE(y2, int8_t(0));
    frame.setInterpolationParameter(vpvl::CameraKeyFrame::kX, 1, 2, 3, 4);
    frame.setInterpolationParameter(vpvl::CameraKeyFrame::kY, 5, 6, 7, 8);
    frame.setInterpolationParameter(vpvl::CameraKeyFrame::kZ, 9, 10, 11, 12);
    frame.setInterpolationParameter(vpvl::CameraKeyFrame::kRotation, 13, 14, 15, 16);
    frame.setInterpolationParameter(vpvl::CameraKeyFrame::kDistance, 17, 18, 19, 20);
    frame.setInterpolationParameter(vpvl::CameraKeyFrame::kFovy, 21, 22, 23, 24);
    frame.getInterpolationParameter(vpvl::CameraKeyFrame::kX, x1, x2, y1, y2);
    QCOMPARE(x1, int8_t(1));
    QCOMPARE(x2, int8_t(2));
    QCOMPARE(y1, int8_t(3));
    QCOMPARE(y2, int8_t(4));
    frame.getInterpolationParameter(vpvl::CameraKeyFrame::kY, x1, x2, y1, y2);
    QCOMPARE(x1, int8_t(5));
    QCOMPARE(x2, int8_t(6));
    QCOMPARE(y1, int8_t(7));
    QCOMPARE(y2, int8_t(8));
    frame.getInterpolationParameter(vpvl::CameraKeyFrame::kZ, x1, x2, y1, y2);
    QCOMPARE(x1, int8_t(9));
    QCOMPARE(x2, int8_t(10));
    QCOMPARE(y1, int8_t(11));
    QCOMPARE(y2, int8_t(12));
    frame.getInterpolationParameter(vpvl::CameraKeyFrame::kRotation, x1, x2, y1, y2);
    QCOMPARE(x1, int8_t(13));
    QCOMPARE(x2, int8_t(14));
    QCOMPARE(y1, int8_t(15));
    QCOMPARE(y2, int8_t(16));
    frame.getInterpolationParameter(vpvl::CameraKeyFrame::kDistance, x1, x2, y1, y2);
    QCOMPARE(x1, int8_t(17));
    QCOMPARE(x2, int8_t(18));
    QCOMPARE(y1, int8_t(19));
    QCOMPARE(y2, int8_t(20));
    frame.getInterpolationParameter(vpvl::CameraKeyFrame::kFovy, x1, x2, y1, y2);
    QCOMPARE(x1, int8_t(21));
    QCOMPARE(x2, int8_t(22));
    QCOMPARE(y1, int8_t(23));
    QCOMPARE(y2, int8_t(24));
}

QTEST_APPLESS_MAIN(TestVMDMotion)

#include "TestVMDMotion.moc"

