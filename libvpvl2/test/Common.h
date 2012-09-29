#ifndef COMMON_H
#define COMMON_H

#include <QtCore/QtCore>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <tr1/tuple>
#include <vpvl2/vpvl2.h>
#include <vpvl2/internal/util.h>
#include <vpvl2/icu/String.h>
#include <vpvl2/icu/Encoding.h>

using namespace ::testing;
using namespace std::tr1;
using namespace vpvl2;
using namespace vpvl2::icu;

namespace vpvl2 {

inline std::ostream& operator <<(std::ostream &os, const Vector3 &value)
{
    return os << "Vector3(x=" << value.x()
              << ", y=" << value.y()
              << ", z=" << value.z()
              << ")";
}

inline std::ostream& operator <<(std::ostream &os, const Vector4 &value)
{
    return os << "Vector4(x=" << value.x()
              << ", y=" << value.y()
              << ", z=" << value.z()
              << ", w=" << value.w()
              << ")";
}

inline std::ostream& operator <<(std::ostream &os, const Quaternion &value)
{
    return os << "Quaternion(x=" << value.x()
              << ", y=" << value.y()
              << ", z=" << value.z()
              << ", w=" << value.w()
              << ")";
}

inline std::ostream& operator <<(std::ostream &os, const QuadWord &value)
{
    return os << "QuadWord(x=" << value.x()
              << ", y=" << value.y()
              << ", z=" << value.z()
              << ", w=" << value.w()
              << ")";
}

}

namespace {

static const float kIdentity4x4[] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
};

static const float kEpsilon = 0.000001;

static inline AssertionResult testVector(const Vector3 &expected, const Vector3 &actual)
{
    if (!btEqual(expected.x() - actual.x(), kEpsilon))
        return AssertionFailure() << "X is not equal: expected is " << expected << " but actual is " << actual;
    if (!btEqual(expected.y() - actual.y(), kEpsilon))
        return AssertionFailure() << "Y is not equal: expected is " << expected << " but actual is " << actual;
    if (!btEqual(expected.z() - actual.z(), kEpsilon))
        return AssertionFailure() << "Z is not equal: expected is " << expected << " but actual is " << actual;
    return AssertionSuccess();
}

static inline AssertionResult testVector(const Vector4 &expected, const Vector4 &actual)
{
    if (!btEqual(expected.x() - actual.x(), kEpsilon))
        return AssertionFailure() << "X is not equal: expected is " << expected << " but actual is " << actual;
    if (!btEqual(expected.y() - actual.y(), kEpsilon))
        return AssertionFailure() << "Y is not equal: expected is " << expected << " but actual is " << actual;
    if (!btEqual(expected.z() - actual.z(), kEpsilon))
        return AssertionFailure() << "Z is not equal: expected is " << expected << " but actual is " << actual;
    if (!btEqual(expected.w() - actual.w(), kEpsilon))
        return AssertionFailure() << "W is not equal: expected is " << expected << " but actual is " << actual;
    return AssertionSuccess();
}

static inline AssertionResult testVector(const Quaternion &expected, const Quaternion &actual)
{
    if (!btEqual(expected.x() - actual.x(), kEpsilon))
        return AssertionFailure() << "X is not equal: expected is " << expected << " but actual is " << actual;
    if (!btEqual(expected.y() - actual.y(), kEpsilon))
        return AssertionFailure() << "Y is not equal: expected is " << expected << " but actual is " << actual;
    if (!btEqual(expected.z() - actual.z(), kEpsilon))
        return AssertionFailure() << "Z is not equal: expected is " << expected << " but actual is " << actual;
    if (!btEqual(expected.w() - actual.w(), kEpsilon))
        return AssertionFailure() << "W is not equal: expected is " << expected << " but actual is " << actual;
    return AssertionSuccess();
}

static inline AssertionResult testVector(const QuadWord &actual, const QuadWord &expected)
{
    if (!btEqual(expected.x() - actual.x(), kEpsilon))
        return AssertionFailure() << "X is not equal: expected is " << expected << " but actual is " << actual;
    if (!btEqual(expected.y() - actual.y(), kEpsilon))
        return AssertionFailure() << "Y is not equal: expected is " << expected << " but actual is " << actual;
    if (!btEqual(expected.z() - actual.z(), kEpsilon))
        return AssertionFailure() << "Z is not equal: expected is " << expected << " but actual is " << actual;
    if (!btEqual(expected.w() - actual.w(), kEpsilon))
        return AssertionFailure() << "W is not equal: expected is " << expected << " but actual is " << actual;
    return AssertionSuccess();
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

struct ScopedPointerListDeleter {
    template<typename T>
    static inline void cleanup(QList<T *> *list) {
        qDeleteAll(*list);
    }
};

}

#endif // COMMON_H
