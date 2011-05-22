#include "gtest/gtest.h"
#include "vpvl/vpvl.h"
#include "vpvl/internal/PMDParser.h"

TEST(PMDParserTest, PreParseEmptyPMD) {
    vpvl::PMDParser parser("", 0);
    EXPECT_FALSE(parser.preparse());
}
