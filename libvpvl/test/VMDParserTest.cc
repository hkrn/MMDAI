#include "gtest/gtest.h"
#include "vpvl/vpvl.h"

static void FileSlurp(const char *path, uint8_t *&data, size_t &size) {
    FILE *fp = fopen(path, "rb");
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    data = new uint8_t[size];
    fread(data, size, 1, fp);
    fclose(fp);
}

TEST(VMDMotionTest, PreParseEmptyVMD) {
    vpvl::VMDMotion motion(reinterpret_cast<const uint8_t *>(""), 0);
    EXPECT_FALSE(motion.preparse());
}

TEST(VMDMotionTest, PreParseMotionVMD) {
    uint8_t *data = 0;
    size_t size = 0;
    FileSlurp("test/res/motion.vmd", data, size);
    vpvl::VMDMotion motion(data, size);
    EXPECT_TRUE(motion.preparse());
    vpvl::VMDMotionDataInfo result = motion.result();
    EXPECT_TRUE(result.basePtr != 0);
    EXPECT_TRUE(result.namePtr != 0);
    EXPECT_TRUE(result.boneKeyFramePtr != 0);
    EXPECT_TRUE(result.boneKeyFrameCount != 0);
    EXPECT_TRUE(result.faceKeyFramePtr != 0);
    EXPECT_TRUE(result.faceKeyFrameCount != 0);
    EXPECT_TRUE(result.cameraKeyFramePtr != 0);
    EXPECT_TRUE(result.cameraKeyFrameCount == 0);
    delete[] data;
}

TEST(VMDMotionTest, PreParseCameraVMD) {
    uint8_t *data = 0;
    size_t size = 0;
    FileSlurp("test/res/camera.vmd", data, size);
    vpvl::VMDMotion motion(data, size);
    EXPECT_TRUE(motion.preparse());
    vpvl::VMDMotionDataInfo result = motion.result();
    EXPECT_TRUE(result.basePtr != 0);
    EXPECT_TRUE(result.namePtr != 0);
    EXPECT_TRUE(result.boneKeyFramePtr != 0);
    EXPECT_TRUE(result.boneKeyFrameCount == 0);
    EXPECT_TRUE(result.faceKeyFramePtr != 0);
    EXPECT_TRUE(result.faceKeyFrameCount == 0);
    EXPECT_TRUE(result.cameraKeyFramePtr != 0);
    EXPECT_TRUE(result.cameraKeyFrameCount != 0);
    delete[] data;
}

TEST(VMDMotionTest, ParseMotionVMD) {
    uint8_t *data = 0;
    size_t size = 0;
    FileSlurp("test/res/motion.vmd", data, size);
    vpvl::VMDMotion motion(data, size);
    EXPECT_TRUE(motion.load());
    vpvl::VMDMotionDataInfo result = motion.result();
    EXPECT_EQ(motion.bone().frames().size(), result.boneKeyFrameCount);
    EXPECT_EQ(motion.face().frames().size(), result.faceKeyFrameCount);
    EXPECT_EQ(motion.camera().frames().size(), result.cameraKeyFrameCount);
    delete[] data;
}

TEST(VMDMotionTest, ParseCameraVMD) {
    uint8_t *data = 0;
    size_t size = 0;
    FileSlurp("test/res/camera.vmd", data, size);
    vpvl::VMDMotion motion(data, size);
    EXPECT_TRUE(motion.load());
    vpvl::VMDMotionDataInfo result = motion.result();
    EXPECT_EQ(motion.bone().frames().size(), result.boneKeyFrameCount);
    EXPECT_EQ(motion.face().frames().size(), result.faceKeyFrameCount);
    EXPECT_EQ(motion.camera().frames().size(), result.cameraKeyFrameCount);
    delete[] data;
}
