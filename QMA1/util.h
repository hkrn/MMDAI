#ifndef UTIL_H
#define UTIL_H

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

namespace
{
const char *kEmptyString = QT_TR_NOOP("(none)");
}

namespace internal
{

static inline QTextCodec *getTextCodec() {
    static QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
    return codec;
}

static inline const QString noneString()
{
    return QApplication::tr(kEmptyString);
}

static inline const QByteArray fromQString(const QString &value) {
    return getTextCodec()->fromUnicode(value);
}

static inline const QString toQString(const uint8_t *value) {
    return getTextCodec()->toUnicode(reinterpret_cast<const char *>(value));
}

static inline const QString toQString(const vpvl::PMDModel *value) {
    return value ? toQString(value->name()) : noneString();
}

static inline const QString toQString(const vpvl::Bone *value) {
    return value ? toQString(value->name()) : noneString();
}

static inline const QString toQString(const vpvl::Face *value) {
    return value ? toQString(value->name()) : noneString();
}

static inline const QString toQString(const vpvl::BoneKeyFrame *value) {
    return value ? toQString(value->name()) : noneString();
}

static inline const QString toQString(const vpvl::FaceKeyFrame *value) {
    return value ? toQString(value->name()) : noneString();
}

}

#endif // UTIL_H
