#ifndef COMMON_H
#define COMMON_H

#include <QtCore/QtCore>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <tr1/tuple>
#include <vpvl2/vpvl2.h>
#include <vpvl2/internal/util.h>
#include <vpvl2/qt/CString.h>
#include <vpvl2/qt/Encoding.h>

using namespace ::testing;
using namespace std::tr1;
using namespace vpvl2;
using namespace vpvl2::qt;

namespace {

static const float kIdentity4x4[] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
};

static inline void AssertVector(const Vector3 &expected, const Vector3 &actual)
{
    ASSERT_FLOAT_EQ(expected.x(), actual.x());
    ASSERT_FLOAT_EQ(expected.y(), actual.y());
    ASSERT_FLOAT_EQ(expected.z(), actual.z());
}

static inline void AssertVector(const Vector4 &expected, const Vector4 &actual)
{
    ASSERT_FLOAT_EQ(expected.x(), actual.x());
    ASSERT_FLOAT_EQ(expected.y(), actual.y());
    ASSERT_FLOAT_EQ(expected.z(), actual.z());
    ASSERT_FLOAT_EQ(expected.w(), actual.w());
}

static inline void AssertVector(const Quaternion &expected, const Quaternion &actual)
{
    ASSERT_FLOAT_EQ(expected.x(), actual.x());
    ASSERT_FLOAT_EQ(expected.y(), actual.y());
    ASSERT_FLOAT_EQ(expected.z(), actual.z());
    ASSERT_FLOAT_EQ(expected.w(), actual.w());
}

static inline void AssertVector(const QuadWord &actual, const QuadWord &expected)
{
    ASSERT_FLOAT_EQ(expected.x(), actual.x());
    ASSERT_FLOAT_EQ(expected.y(), actual.y());
    ASSERT_FLOAT_EQ(expected.z(), actual.z());
    ASSERT_FLOAT_EQ(expected.w(), actual.w());
}

static inline void AssertMatrix(const float *expected, const float *actual)
{
    for (int i = 0; i < 16; i++) {
        ASSERT_FLOAT_EQ(expected[i], actual[i]) << "matrix value differs "
                                                << expected[i]
                                                << " but "
                                                << actual[i]
                                                << " at index "
                                                << i;
    }
}

}

#endif // COMMON_H
