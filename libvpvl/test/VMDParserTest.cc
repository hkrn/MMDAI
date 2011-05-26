#include "gtest/gtest.h"
#include "vpvl/vpvl.h"
#include "vpvl/internal/VMDMotion.h"

static void FileSlurp(const char *path, char *&data, size_t &size) {
    FILE *fp = fopen(path, "rb");
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    data = new char[size];
    fread(data, size, 1, fp);
    fclose(fp);
}

TEST(VMDMotionTest, PreParseEmptyVMD) {
    vpvl::VMDMotion model("", 0);
    EXPECT_FALSE(model.preparse());
}

TEST(VMDMotionTest, PreParseMotionVMD) {
    char *data = 0;
    size_t size = 0;
    FileSlurp("test/res/motion.vmd", data, size);
    vpvl::VMDMotion model(data, size);
    EXPECT_TRUE(model.preparse());
    vpvl::VMDMotionDataInfo result = model.result();
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
    char *data = 0;
    size_t size = 0;
    FileSlurp("test/res/camera.vmd", data, size);
    vpvl::VMDMotion model(data, size);
    EXPECT_TRUE(model.preparse());
    vpvl::VMDMotionDataInfo result = model.result();
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
