#ifndef UTIL_H
#define UTIL_H

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

namespace internal
{

static inline QTextCodec *getTextCodec() {
    static QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
    return codec;
}

static inline const QString toQString(const uint8_t *value) {
    return getTextCodec()->toUnicode(reinterpret_cast<const char *>(value));
}

static inline const QString toQString(const vpvl::PMDModel *value) {
    return toQString(value->name());
}

static inline const QString toQString(const vpvl::Bone *value) {
    return toQString(value->name());
}

static inline const QString toQString(const vpvl::Face *value) {
    return toQString(value->name());
}

static inline const QString toQString(const vpvl::BoneKeyFrame *value) {
    return toQString(value->name());
}

static inline const QString toQString(const vpvl::FaceKeyFrame *value) {
    return toQString(value->name());
}

}

#endif // UTIL_H
