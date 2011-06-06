#include "gtest/gtest.h"
#include "vpvl/vpvl.h"
#include "vpvl/internal/PMDModel.h"

static void FileSlurp(const char *path, uint8_t *&data, size_t &size) {
    FILE *fp = fopen(path, "rb");
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    data = new uint8_t[size];
    fread(data, size, 1, fp);
    fclose(fp);
}

TEST(PMDModelTest, PreParseEmptyPMD) {
    vpvl::PMDModel model(reinterpret_cast<const uint8_t *>(""), 0);
    EXPECT_FALSE(model.preparse());
}

TEST(PMDModelTest, PreParseFullPMD) {
    uint8_t *data = 0;
    size_t size = 0;
    FileSlurp("test/res/miku.pmd", data, size);
    vpvl::PMDModel model(data, size);
    EXPECT_TRUE(model.preparse());
    vpvl::PMDModelDataInfo result = model.result();
    EXPECT_TRUE(result.basePtr != 0);
    EXPECT_TRUE(result.namePtr != 0);
    EXPECT_TRUE(result.commentPtr != 0);
    EXPECT_TRUE(result.verticesPtr != 0);
    EXPECT_TRUE(result.verticesCount != 0);
    EXPECT_TRUE(result.indicesPtr != 0);
    EXPECT_TRUE(result.indicesCount != 0);
    EXPECT_TRUE(result.materialsPtr != 0);
    EXPECT_TRUE(result.materialsCount != 0);
    EXPECT_TRUE(result.bonesPtr != 0);
    EXPECT_TRUE(result.bonesCount != 0);
    EXPECT_TRUE(result.IKsPtr != 0);
    EXPECT_TRUE(result.IKsCount != 0);
    EXPECT_TRUE(result.facesPtr != 0);
    EXPECT_TRUE(result.facesCount != 0);
    EXPECT_TRUE(result.faceDisplayNamesPtr != 0);
    EXPECT_TRUE(result.faceDisplayNamesCount != 0);
    EXPECT_TRUE(result.boneFrameNamesPtr != 0);
    EXPECT_TRUE(result.boneFrameNamesCount != 0);
    EXPECT_TRUE(result.boneDisplayNamesPtr != 0);
    EXPECT_TRUE(result.boneDisplayNamesCount != 0);
    EXPECT_TRUE(result.englishNamePtr != 0);
    EXPECT_TRUE(result.englishCommentPtr != 0);
    EXPECT_TRUE(result.englishBoneNamesPtr != 0);
    EXPECT_TRUE(result.englishFaceNamesPtr != 0);
    EXPECT_TRUE(result.englishBoneFramesPtr != 0);
    EXPECT_TRUE(result.toonTextureNamesPtr != 0);
    EXPECT_TRUE(result.rigidBodiesPtr != 0);
    EXPECT_TRUE(result.rigidBodiesCount != 0);
    EXPECT_TRUE(result.constraintsPtr != 0);
    EXPECT_TRUE(result.constranitsCount != 0);
    delete[] data;
}

TEST(PMDModelTest, ParseFullPMD) {
    uint8_t *data = 0;
    size_t size = 0;
    FileSlurp("test/res/miku.pmd", data, size);
    vpvl::PMDModel model(data, size);
    EXPECT_TRUE(model.load());
    vpvl::PMDModelDataInfo result = model.result();
    EXPECT_EQ(model.vertices().size(), result.verticesCount);
    EXPECT_EQ(model.materials().size(), result.materialsCount);
    EXPECT_EQ(model.bones().size(), result.bonesCount);
    EXPECT_EQ(model.IKs().size(), result.IKsCount);
    EXPECT_EQ(model.faces().size(), result.facesCount);
    EXPECT_EQ(model.rigidBodies().size(), result.rigidBodiesCount);
    EXPECT_EQ(model.constraints().size(), result.constranitsCount);
    EXPECT_STREQ("Miku Hatsune", reinterpret_cast<const char *>(model.englishName()));
}
