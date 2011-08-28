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
    void saveBoneKeyFrame();
    void saveCameraKeyFrame();
    void saveFaceKeyFrame();
    void saveMotion();
    void boneInterpolation();
    void cameraInterpolation();

private:
    void testBoneInterpolationMatrix(const int8_t matrix[16], const vpvl::BoneKeyFrame &frame);
    void testCameraInterpolationMatrix(const int8_t matrix[24], const vpvl::CameraKeyFrame &frame);
};

const char *TestVMDMotion::kTestString = "01234567890123";

TestVMDMotion::TestVMDMotion()
{
}

void TestVMDMotion::parseEmpty()
{
    vpvl::VMDMotion motion;
    vpvl::VMDMotion::DataInfo info;
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
        vpvl::VMDMotion::DataInfo result;
        QVERIFY(motion.preparse(data, size, result));
        QVERIFY(motion.load(data, size));
        QCOMPARE(result.boneKeyFrameCount, size_t(motion.boneAnimation().countKeyFrames()));
        QCOMPARE(result.cameraKeyFrameCount, motion.cameraAnimation().countKeyFrames());
        QCOMPARE(result.faceKeyFrameCount, size_t(motion.faceAnimation().countKeyFrames()));
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
        vpvl::VMDMotion::DataInfo result;
        QVERIFY(motion.preparse(data, size, result));
        QVERIFY(motion.load(data, size));
        QCOMPARE(result.boneKeyFrameCount, size_t(motion.boneAnimation().countKeyFrames()));
        QCOMPARE(result.cameraKeyFrameCount, size_t(motion.cameraAnimation().countKeyFrames()));
        QCOMPARE(result.faceKeyFrameCount, size_t(motion.faceAnimation().countKeyFrames()));
        QCOMPARE(motion.error(), vpvl::VMDMotion::kNoError);
    }
    else {
        QSKIP("Require a motion to test this", SkipSingle);
    }
}

void TestVMDMotion::saveBoneKeyFrame()
{
    vpvl::BoneKeyFrame frame, newFrame, *cloned;
    btVector3 pos(1, 2, 3);
    btQuaternion rot(4, 5, 6, 7);
    frame.setFrameIndex(42);
    frame.setName(reinterpret_cast<const uint8_t *>(kTestString));
    frame.setPosition(pos);
    frame.setRotation(rot);
    frame.setInterpolationParameter(vpvl::BoneKeyFrame::kX,         8,  9, 10, 11);
    frame.setInterpolationParameter(vpvl::BoneKeyFrame::kY,        12, 13, 14, 15);
    frame.setInterpolationParameter(vpvl::BoneKeyFrame::kZ,        16, 17, 18, 19);
    frame.setInterpolationParameter(vpvl::BoneKeyFrame::kRotation, 20, 21, 22, 23);
    uint8_t data[vpvl::BoneKeyFrame::strideSize()];
    frame.write(data);
    newFrame.read(data);
    QCOMPARE(QString(reinterpret_cast<const char *>(newFrame.name())),
             QString(reinterpret_cast<const char *>(frame.name())));
    QCOMPARE(newFrame.frameIndex(), frame.frameIndex());
    QVERIFY(newFrame.position() == pos);
    QVERIFY(newFrame.rotation() == rot);
    int8_t matrix[] = {
         8,  9, 10, 11,
        12, 13, 14, 15,
        16, 17, 18, 19,
        20, 21, 22, 23
    };
    testBoneInterpolationMatrix(matrix, frame);
    cloned = static_cast<vpvl::BoneKeyFrame *>(frame.clone());
    QCOMPARE(QString(reinterpret_cast<const char *>(cloned->name())),
             QString(reinterpret_cast<const char *>(frame.name())));
    QCOMPARE(cloned->frameIndex(), frame.frameIndex());
    QVERIFY(cloned->position() == pos);
    QVERIFY(cloned->rotation() == rot);
    testBoneInterpolationMatrix(matrix, *cloned);
}

void TestVMDMotion::saveCameraKeyFrame()
{
    vpvl::CameraKeyFrame frame, newFrame, *cloned;
    btVector3 pos(1, 2, 3), angle(4, 5, 6);
    frame.setFrameIndex(42);
    frame.setPosition(pos);
    frame.setAngle(angle);
    frame.setDistance(7);
    frame.setFovy(8);
    frame.setInterpolationParameter(vpvl::CameraKeyFrame::kX,         9, 10, 11, 12);
    frame.setInterpolationParameter(vpvl::CameraKeyFrame::kY,        13, 14, 15, 16);
    frame.setInterpolationParameter(vpvl::CameraKeyFrame::kZ,        17, 18, 19, 20);
    frame.setInterpolationParameter(vpvl::CameraKeyFrame::kRotation, 21, 22, 23, 24);
    frame.setInterpolationParameter(vpvl::CameraKeyFrame::kDistance, 25, 26, 27, 28);
    frame.setInterpolationParameter(vpvl::CameraKeyFrame::kFovy,     29, 30, 31, 32);
    uint8_t data[vpvl::CameraKeyFrame::strideSize()];
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
    int8_t matrix[] = {
         9, 10, 11, 12,
        13, 14, 15, 16,
        17, 18, 19, 20,
        21, 22, 23, 24,
        25, 26, 27, 28,
        29, 30, 31, 32
    };
    testCameraInterpolationMatrix(matrix, frame);
    cloned = static_cast<vpvl::CameraKeyFrame *>(frame.clone());
    QCOMPARE(cloned->frameIndex(), frame.frameIndex());
    QVERIFY(cloned->position() == frame.position());
    // for radian and degree calculation
    QVERIFY(qFuzzyCompare(cloned->angle().x(), frame.angle().x()));
    QVERIFY(qFuzzyCompare(cloned->angle().y(), frame.angle().y()));
    QVERIFY(qFuzzyCompare(cloned->angle().z(), frame.angle().z()));
    QVERIFY(cloned->distance() == frame.distance());
    QVERIFY(cloned->fovy() == frame.fovy());
    testCameraInterpolationMatrix(matrix, *cloned);
}

void TestVMDMotion::saveFaceKeyFrame()
{
    vpvl::FaceKeyFrame frame, newFrame, *cloned;
    frame.setFrameIndex(42);
    frame.setName(reinterpret_cast<const uint8_t *>(kTestString));
    frame.setWeight(0.5);
    uint8_t data[vpvl::FaceKeyFrame::strideSize()];
    frame.write(data);
    newFrame.read(data);
    QCOMPARE(QString(reinterpret_cast<const char *>(newFrame.name())),
             QString(reinterpret_cast<const char *>(frame.name())));
    QCOMPARE(newFrame.frameIndex(), frame.frameIndex());
    QCOMPARE(newFrame.weight(), frame.weight());
    cloned = static_cast<vpvl::FaceKeyFrame *>(frame.clone());
    QCOMPARE(QString(reinterpret_cast<const char *>(cloned->name())),
             QString(reinterpret_cast<const char *>(frame.name())));
    QCOMPARE(cloned->frameIndex(), frame.frameIndex());
    QCOMPARE(cloned->weight(), frame.weight());
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
    const int8_t matrix[] = {
         1,  2,  3,  4,
         5,  6,  7,  8,
         9, 10, 11, 12,
        13, 14, 15, 16
    };
    testBoneInterpolationMatrix(matrix, frame);
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
    const int8_t matrix[] = {
         1,  2,  3,  4,
         5,  6,  7,  8,
         9, 10, 11, 12,
        13, 14, 15, 16,
        17, 18, 19, 20,
        21, 22, 23, 24
    };
    testCameraInterpolationMatrix(matrix, frame);
}

void TestVMDMotion::testBoneInterpolationMatrix(const int8_t matrix[], const vpvl::BoneKeyFrame &frame)
{
    int8_t x1, y1, x2, y2;
    frame.getInterpolationParameter(vpvl::BoneKeyFrame::kX, x1, x2, y1, y2);
    QCOMPARE(x1, matrix[0]);
    QCOMPARE(x2, matrix[1]);
    QCOMPARE(y1, matrix[2]);
    QCOMPARE(y2, matrix[3]);
    frame.getInterpolationParameter(vpvl::BoneKeyFrame::kY, x1, x2, y1, y2);
    QCOMPARE(x1, matrix[4]);
    QCOMPARE(x2, matrix[5]);
    QCOMPARE(y1, matrix[6]);
    QCOMPARE(y2, matrix[7]);
    frame.getInterpolationParameter(vpvl::BoneKeyFrame::kZ, x1, x2, y1, y2);
    QCOMPARE(x1, matrix[8]);
    QCOMPARE(x2, matrix[9]);
    QCOMPARE(y1, matrix[10]);
    QCOMPARE(y2, matrix[11]);
    frame.getInterpolationParameter(vpvl::BoneKeyFrame::kRotation, x1, x2, y1, y2);
    QCOMPARE(x1, matrix[12]);
    QCOMPARE(x2, matrix[13]);
    QCOMPARE(y1, matrix[14]);
    QCOMPARE(y2, matrix[15]);
}

void TestVMDMotion::testCameraInterpolationMatrix(const int8_t matrix[], const vpvl::CameraKeyFrame &frame)
{
    int8_t x1, y1, x2, y2;
    frame.getInterpolationParameter(vpvl::CameraKeyFrame::kX, x1, x2, y1, y2);
    QCOMPARE(x1, matrix[0]);
    QCOMPARE(x2, matrix[1]);
    QCOMPARE(y1, matrix[2]);
    QCOMPARE(y2, matrix[3]);
    frame.getInterpolationParameter(vpvl::CameraKeyFrame::kY, x1, x2, y1, y2);
    QCOMPARE(x1, matrix[4]);
    QCOMPARE(x2, matrix[5]);
    QCOMPARE(y1, matrix[6]);
    QCOMPARE(y2, matrix[7]);
    frame.getInterpolationParameter(vpvl::CameraKeyFrame::kZ, x1, x2, y1, y2);
    QCOMPARE(x1, matrix[8]);
    QCOMPARE(x2, matrix[9]);
    QCOMPARE(y1, matrix[10]);
    QCOMPARE(y2, matrix[11]);
    frame.getInterpolationParameter(vpvl::CameraKeyFrame::kRotation, x1, x2, y1, y2);
    QCOMPARE(x1, matrix[12]);
    QCOMPARE(x2, matrix[13]);
    QCOMPARE(y1, matrix[14]);
    QCOMPARE(y2, matrix[15]);
    frame.getInterpolationParameter(vpvl::CameraKeyFrame::kDistance, x1, x2, y1, y2);
    QCOMPARE(x1, matrix[16]);
    QCOMPARE(x2, matrix[17]);
    QCOMPARE(y1, matrix[18]);
    QCOMPARE(y2, matrix[19]);
    frame.getInterpolationParameter(vpvl::CameraKeyFrame::kFovy, x1, x2, y1, y2);
    QCOMPARE(x1, matrix[20]);
    QCOMPARE(x2, matrix[21]);
    QCOMPARE(y1, matrix[22]);
    QCOMPARE(y2, matrix[23]);
}

QTEST_APPLESS_MAIN(TestVMDMotion)

#include "TestVMDMotion.moc"

