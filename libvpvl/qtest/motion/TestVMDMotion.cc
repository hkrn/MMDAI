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
    void testBoneInterpolationMatrix(const btQuadWord p[4], const vpvl::BoneKeyFrame &frame);
    void testCameraInterpolationMatrix(const btQuadWord p[6], const vpvl::CameraKeyFrame &frame);
};

const char *TestVMDMotion::kTestString = "012345678901234";

static void CompareQuadWord(const btQuadWord &actual, const btQuadWord &expected)
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
    vpvl::VMDMotion motion;
    vpvl::VMDMotion::DataInfo info;
    QVERIFY(!motion.preparse(reinterpret_cast<const uint8_t *>(""), 0, info));
    QCOMPARE(motion.error(), vpvl::VMDMotion::kInvalidHeaderError);
}

void TestVMDMotion::parseMotion()
{
    QFile file("../../render/res/motion.vmd");
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

void TestVMDMotion::parseCamera()
{
    QFile file("../../render/res/camera.vmd");
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
    btQuadWord px(8, 9, 10, 11),
            py(12, 13, 14, 15),
            pz(16, 17, 18, 19),
            pr(20, 21, 22, 23),
            p[] = { px, py, pz, pr };
    frame.setInterpolationParameter(vpvl::BoneKeyFrame::kX, px);
    frame.setInterpolationParameter(vpvl::BoneKeyFrame::kY, py);
    frame.setInterpolationParameter(vpvl::BoneKeyFrame::kZ, pz);
    frame.setInterpolationParameter(vpvl::BoneKeyFrame::kRotation, pr);
    uint8_t data[vpvl::BoneKeyFrame::strideSize()];
    frame.write(data);
    newFrame.read(data);
    QCOMPARE(QString(reinterpret_cast<const char *>(newFrame.name())),
             QString(reinterpret_cast<const char *>(frame.name())));
    QCOMPARE(newFrame.frameIndex(), frame.frameIndex());
    QVERIFY(newFrame.position() == pos);
    QVERIFY(newFrame.rotation() == rot);
    testBoneInterpolationMatrix(p, frame);
    cloned = static_cast<vpvl::BoneKeyFrame *>(frame.clone());
    QCOMPARE(QString(reinterpret_cast<const char *>(cloned->name())),
             QString(reinterpret_cast<const char *>(frame.name())));
    QCOMPARE(cloned->frameIndex(), frame.frameIndex());
    QVERIFY(cloned->position() == pos);
    QVERIFY(cloned->rotation() == rot);
    testBoneInterpolationMatrix(p, *cloned);
    delete cloned;
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
    btQuadWord px(9, 10, 11, 12),
            py(13, 14, 15, 16),
            pz(17, 18, 19, 20),
            pr(21, 22, 23, 24),
            pd(25, 26, 27, 28),
            pf(29, 30, 31, 32),
            p[] = { px, py, pz, pr, pd, pf };
    frame.setInterpolationParameter(vpvl::CameraKeyFrame::kX, px);
    frame.setInterpolationParameter(vpvl::CameraKeyFrame::kY, py);
    frame.setInterpolationParameter(vpvl::CameraKeyFrame::kZ, pz);
    frame.setInterpolationParameter(vpvl::CameraKeyFrame::kRotation, pr);
    frame.setInterpolationParameter(vpvl::CameraKeyFrame::kDistance, pd);
    frame.setInterpolationParameter(vpvl::CameraKeyFrame::kFovy, pf);
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
    testCameraInterpolationMatrix(p, frame);
    cloned = static_cast<vpvl::CameraKeyFrame *>(frame.clone());
    QCOMPARE(cloned->frameIndex(), frame.frameIndex());
    QVERIFY(cloned->position() == frame.position());
    // for radian and degree calculation
    QVERIFY(qFuzzyCompare(cloned->angle().x(), frame.angle().x()));
    QVERIFY(qFuzzyCompare(cloned->angle().y(), frame.angle().y()));
    QVERIFY(qFuzzyCompare(cloned->angle().z(), frame.angle().z()));
    QVERIFY(cloned->distance() == frame.distance());
    QVERIFY(cloned->fovy() == frame.fovy());
    testCameraInterpolationMatrix(p, *cloned);
    delete cloned;
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
    delete cloned;
}

void TestVMDMotion::saveMotion()
{
    QFile file("../../render/res/motion.vmd");
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
    btQuadWord n;
    frame.getInterpolationParameter(vpvl::BoneKeyFrame::kX, n);
    CompareQuadWord(n, btQuadWord(0.0f, 0.0f, 0.0f, 0.0f));
    btQuadWord px(8, 9, 10, 11),
            py(12, 13, 14, 15),
            pz(16, 17, 18, 19),
            pr(20, 21, 22, 23),
            p[] = { px, py, pz, pr };
    frame.setInterpolationParameter(vpvl::BoneKeyFrame::kX, px);
    frame.setInterpolationParameter(vpvl::BoneKeyFrame::kY, py);
    frame.setInterpolationParameter(vpvl::BoneKeyFrame::kZ, pz);
    frame.setInterpolationParameter(vpvl::BoneKeyFrame::kRotation, pr);
    testBoneInterpolationMatrix(p, frame);
}

void TestVMDMotion::cameraInterpolation()
{
    vpvl::CameraKeyFrame frame;
    btQuadWord n;
    frame.getInterpolationParameter(vpvl::CameraKeyFrame::kX, n);
    CompareQuadWord(n, btQuadWord(0.0f, 0.0f, 0.0f, 0.0f));
    btQuadWord px(9, 10, 11, 12),
            py(13, 14, 15, 16),
            pz(17, 18, 19, 20),
            pr(21, 22, 23, 24),
            pd(25, 26, 27, 28),
            pf(29, 30, 31, 32),
            p[] = { px, py, pz, pr, pd, pf };
    frame.setInterpolationParameter(vpvl::CameraKeyFrame::kX, px);
    frame.setInterpolationParameter(vpvl::CameraKeyFrame::kY, py);
    frame.setInterpolationParameter(vpvl::CameraKeyFrame::kZ, pz);
    frame.setInterpolationParameter(vpvl::CameraKeyFrame::kRotation, pr);
    frame.setInterpolationParameter(vpvl::CameraKeyFrame::kDistance, pd);
    frame.setInterpolationParameter(vpvl::CameraKeyFrame::kFovy, pf);
    testCameraInterpolationMatrix(p, frame);
}

void TestVMDMotion::testBoneInterpolationMatrix(const btQuadWord p[], const vpvl::BoneKeyFrame &frame)
{
    btQuadWord actual, expected = p[0];
    frame.getInterpolationParameter(vpvl::BoneKeyFrame::kX, actual);
    CompareQuadWord(actual, expected);
    expected = p[1];
    frame.getInterpolationParameter(vpvl::BoneKeyFrame::kY, actual);
    CompareQuadWord(actual, expected);
    expected = p[2];
    frame.getInterpolationParameter(vpvl::BoneKeyFrame::kZ, actual);
    CompareQuadWord(actual, expected);
    expected = p[3];
    frame.getInterpolationParameter(vpvl::BoneKeyFrame::kRotation, actual);
    CompareQuadWord(actual, expected);
}

void TestVMDMotion::testCameraInterpolationMatrix(const btQuadWord p[], const vpvl::CameraKeyFrame &frame)
{
    btQuadWord actual, expected = p[0];
    frame.getInterpolationParameter(vpvl::CameraKeyFrame::kX, actual);
    CompareQuadWord(actual, expected);
    expected = p[1];
    frame.getInterpolationParameter(vpvl::CameraKeyFrame::kY, actual);
    CompareQuadWord(actual, expected);
    expected = p[2];
    frame.getInterpolationParameter(vpvl::CameraKeyFrame::kZ, actual);
    CompareQuadWord(actual, expected);
    expected = p[3];
    frame.getInterpolationParameter(vpvl::CameraKeyFrame::kRotation, actual);
    CompareQuadWord(actual, expected);
    expected = p[4];
    frame.getInterpolationParameter(vpvl::CameraKeyFrame::kDistance, actual);
    CompareQuadWord(actual, expected);
    expected = p[5];
    frame.getInterpolationParameter(vpvl::CameraKeyFrame::kFovy, actual);
    CompareQuadWord(actual, expected);
}

QTEST_APPLESS_MAIN(TestVMDMotion)

#include "TestVMDMotion.moc"

