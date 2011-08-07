#include <QtCore/QtCore>
#include <QtTest/QtTest>

#include <vpvl/vpvl.h>

class TestVMDMotion : public QObject
{
    Q_OBJECT

public:
    TestVMDMotion();

private Q_SLOTS:
    void parseEmpty();
    void parseMotion();
    void parseCamera();
    void saveMotion();
};

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

QTEST_APPLESS_MAIN(TestVMDMotion)

#include "TestVMDMotion.moc"

