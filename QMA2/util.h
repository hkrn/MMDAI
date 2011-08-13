#ifndef UTIL_H
#define UTIL_H

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

namespace internal
{

const QString kEmptyString = QString();

static inline QTextCodec *getTextCodec() {
    static QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
    return codec;
}

static inline const QString toQString(const uint8_t *value) {
    return getTextCodec()->toUnicode(reinterpret_cast<const char *>(value));
}

static inline const QString toQString(const vpvl::PMDModel *value) {
    return value ? toQString(value->name()) : kEmptyString;
}

static inline const QString toQString(const vpvl::Bone *value) {
    return value ? toQString(value->name()) : kEmptyString;
}

static inline const QString toQString(const vpvl::Face *value) {
    return value ? toQString(value->name()) : kEmptyString;
}

static inline const QString toQString(const vpvl::BoneKeyFrame *value) {
    return value ? toQString(value->name()) : kEmptyString;
}

static inline const QString toQString(const vpvl::FaceKeyFrame *value) {
    return value ? toQString(value->name()) : kEmptyString;
}

}

#endif // UTIL_H
