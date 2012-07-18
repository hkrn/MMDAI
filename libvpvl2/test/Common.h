#include <QtCore/QtCore>
#include <vpvl2/vpvl2.h>

using namespace vpvl2;

class String : public IString {
public:
    explicit String(const QString &s, IString::Codec codec = IString::kUTF8)
        : m_bytes(s.toUtf8()),
          m_value(s),
          m_codec(codec)
    {
    }
    ~String() {
    }

    bool startsWith(const IString *value) const {
        return m_value.startsWith(static_cast<const String *>(value)->m_value);
    }
    bool contains(const IString *value) const {
        return m_value.contains(static_cast<const String *>(value)->m_value);
    }
    bool endsWith(const IString *value) const {
        return m_value.endsWith(static_cast<const String *>(value)->m_value);
    }
    IString *clone() const {
        return new String(m_value);
    }
    const HashString toHashString() const {
        return HashString(m_bytes.constData());
    }
    bool equals(const IString *value) const {
        return m_value == static_cast<const String *>(value)->value();
    }
    const QString &value() const {
        return m_value;
    }
    const uint8_t *toByteArray() const {
        return reinterpret_cast<const uint8_t *>(m_bytes.constData());
    }
    size_t length() const {
        return m_bytes.length();
    }

private:
    const QByteArray m_bytes;
    const QString m_value;
    const IString::Codec m_codec;
};

class Encoding : public IEncoding {
public:
    Encoding()
        : m_sjis(QTextCodec::codecForName("Shift-JIS")),
          m_utf8(QTextCodec::codecForName("UTF-8")),
          m_utf16(QTextCodec::codecForName("UTF-16"))
    {
    }
    ~Encoding() {
    }

    const IString *stringConstant(ConstantType value) const {
        switch (value) {
        case kLeft: {
            static const String s("左");
            return &s;
        }
        case kRight: {
            static const String s("右");
            return &s;
        }
        case kFinger: {
            static const String s("指");
            return &s;
        }
        case kElbow: {
            static const String s("ひじ");
            return &s;
        }
        case kArm: {
            static const String s("腕");
            return &s;
        }
        case kWrist: {
            static const String s("手首");
            return &s;
        }
        default: {
            static const String s("");
            return &s;
        }
        }
    }
    IString *toString(const uint8_t *value, size_t size, IString::Codec codec) const {
        IString *s = 0;
        const char *str = reinterpret_cast<const char *>(value);
        switch (codec) {
        case IString::kShiftJIS:
            s = new String(m_sjis->toUnicode(str, size), codec);
            break;
        case IString::kUTF8:
            s = new String(m_utf8->toUnicode(str, size), codec);
            break;
        case IString::kUTF16:
            s = new String(m_utf16->toUnicode(str, size), codec);
            break;
        }
        return s;
    }
    IString *toString(const uint8_t *value, IString::Codec codec, size_t maxlen) const {
        size_t size = qstrnlen(reinterpret_cast<const char *>(value), maxlen);
        return toString(value, size, codec);
    }
    uint8_t *toByteArray(const IString *value, IString::Codec codec) const {
        const String *s = static_cast<const String *>(value);
        QByteArray bytes;
        switch (codec) {
        case IString::kShiftJIS:
            bytes = m_sjis->fromUnicode(s->value());
            break;
        case IString::kUTF8:
            bytes = m_utf8->fromUnicode(s->value());
            break;
        case IString::kUTF16:
            bytes = m_utf16->fromUnicode(s->value());
            break;
        }
        size_t size = bytes.length();
        uint8_t *data = new uint8_t[size + 1];
        memcpy(data, bytes.constData(), size);
        data[size] = 0;
        return data;
    }
    void disposeByteArray(uint8_t *value) const {
        delete[] value;
    }

private:
    const QTextCodec *m_sjis;
    const QTextCodec *m_utf8;
    const QTextCodec *m_utf16;
};
