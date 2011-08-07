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
    QFile file("../../../gtest/res/motion.vmd");
    if (file.open(QFile::ReadOnly)) {
        QByteArray bytes = file.readAll();
        const uint8_t *data = reinterpret_cast<const uint8_t *>(bytes.constData());
        size_t size = bytes.size();
        vpvl::VMDMotion motion;
        vpvl::VMDMotionDataInfo result;
        QVERIFY(motion.preparse(data, size, result));
        QVERIFY(motion.load(data, size));
        QCOMPARE(result.boneKeyFrameCount, size_t(motion.boneAnimation().frames().size()));
        QCOMPARE(result.cameraKeyFrameCount, size_t(motion.cameraAnimation().frames().size()));
        QCOMPARE(result.faceKeyFrameCount, size_t(motion.faceAnimation().frames().size()));
        QCOMPARE(motion.error(), vpvl::VMDMotion::kNoError);
    }
    else {
        QSKIP("Require a motion to test this", SkipSingle);
    }
}

void TestVMDMotion::parseCamera()
{
    QFile file("../../../gtest/res/camera.vmd");
    if (file.open(QFile::ReadOnly)) {
        QByteArray bytes = file.readAll();
        const uint8_t *data = reinterpret_cast<const uint8_t *>(bytes.constData());
        size_t size = bytes.size();
        vpvl::VMDMotion motion;
        vpvl::VMDMotionDataInfo result;
        QVERIFY(motion.preparse(data, size, result));
        QVERIFY(motion.load(data, size));
        QCOMPARE(result.boneKeyFrameCount, size_t(motion.boneAnimation().frames().size()));
        QCOMPARE(result.cameraKeyFrameCount, size_t(motion.cameraAnimation().frames().size()));
        QCOMPARE(result.faceKeyFrameCount, size_t(motion.faceAnimation().frames().size()));
        QCOMPARE(motion.error(), vpvl::VMDMotion::kNoError);
    }
    else {
        QSKIP("Require a motion to test this", SkipSingle);
    }
}

void TestVMDMotion::saveMotion()
{
    QFile file("../../../gtest/res/motion.vmd");
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
    QCOMPARE(size_t(bytes.size()), vpvl::BoneKeyFrame::stride());
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
    QCOMPARE(size_t(bytes.size()), vpvl::CameraKeyFrame::stride());
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
    QCOMPARE(size_t(bytes.size()), vpvl::FaceKeyFrame::stride());
    vpvl::FaceKeyFrame frame;
    frame.read(reinterpret_cast<const uint8_t *>(bytes.constData()));
    QCOMPARE(QString(reinterpret_cast<const char *>(frame.name())), QString(kTestString));
    QCOMPARE(frame.frameIndex(), 1.0f);
    QCOMPARE(frame.weight(), 0.5f);
}

QTEST_APPLESS_MAIN(TestVMDMotion)

#include "TestVMDMotion.moc"

