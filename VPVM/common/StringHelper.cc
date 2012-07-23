/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2012  hkrn                                    */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MMDAI project team nor the names of     */
/*   its contributors may be used to endorse or promote products     */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

#include "StringHelper.h"

#include <vpvl2/vpvl2.h>
#include <QtCore/QtCore>

namespace internal {

using namespace vpvl2;

String::String(const QString &s, IString::Codec codec)
    : m_bytes(s.toUtf8()),
      m_value(s),
      m_codec(codec)
{
}

String::~String() {
}

bool String::startsWith(const IString *value) const
{
    return m_value.startsWith(static_cast<const String *>(value)->m_value);
}

bool String::contains(const IString *value) const
{
    return m_value.contains(static_cast<const String *>(value)->m_value);
}

bool String::endsWith(const IString *value) const
{
    return m_value.endsWith(static_cast<const String *>(value)->m_value);
}

IString *String::clone() const
{
    return new String(m_value);
}

const HashString String::toHashString() const
{
    return HashString(m_bytes.constData());
}

bool String::equals(const IString *value) const
{
    return m_value == static_cast<const String *>(value)->value();
}

const QString &String::value() const
{
    return m_value;
}

const uint8_t *String::toByteArray() const
{
    return reinterpret_cast<const uint8_t *>(m_bytes.constData());
}

size_t String::length() const
{
    return m_bytes.length();
}

Encoding::Encoding()
    : m_sjis(QTextCodec::codecForName("Shift-JIS")),
      m_utf8(QTextCodec::codecForName("UTF-8")),
      m_utf16(QTextCodec::codecForName("UTF-16"))
{
}

Encoding::~Encoding() {
}

const IString *Encoding::stringConstant(ConstantType value) const
{
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

IString *Encoding::toString(const uint8_t *value, size_t size, IString::Codec codec) const
{
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

IString *Encoding::toString(const uint8_t *value, IString::Codec codec, size_t maxlen) const
{
    size_t size = qstrnlen(reinterpret_cast<const char *>(value), maxlen);
    return toString(value, size, codec);
}

uint8_t *Encoding::toByteArray(const IString *value, IString::Codec codec) const
{
    QByteArray bytes;
    if (value) {
        const String *s = static_cast<const String *>(value);
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
    }
    size_t size = bytes.length();
    uint8_t *data = new uint8_t[size + 1];
    memcpy(data, bytes.constData(), size);
    data[size] = 0;
    return data;
}

void Encoding::disposeByteArray(uint8_t *value) const
{
    delete[] value;
}

QTextCodec *getTextCodec()
{
    static QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
    return codec;
}

const QString &noneString()
{
    static const QString none = QCoreApplication::tr("(none)");
    return none;
}

const QByteArray toByteArrayFromQString(const QString &value)
{
    const QByteArray &bytes = getTextCodec()->fromUnicode(value);
    return bytes;
}

const QString toQStringFromBytes(const uint8_t *value)
{
    const QString &s = getTextCodec()->toUnicode(reinterpret_cast<const char *>(value));
    return s;
}

const QString toQStringFromString(const IString *value)
{
    const QString &s = value ? static_cast<const String *>(value)->value() : noneString();
    return s;
}

const QString toQStringFromModel(const IModel *value)
{
    const QString &s = value ? toQStringFromString(value->name()) : noneString();
    return s;
}

const QString toQStringFromMotion(const IMotion *value)
{
    const QString &s = value ? toQStringFromString(value->name()) : noneString();
    return s;
}

const QString toQStringFromBone(const IBone *value)
{
    const QString &s = value ? toQStringFromString(value->name()) : noneString();
    return s;
}

const QString toQStringFromMorph(const IMorph *value)
{
    const QString &s = value ? toQStringFromString(value->name()) : noneString();
    return s;
}

const QString toQStringFromBoneKeyframe(const IBoneKeyframe *value)
{
    const QString &s = value ? toQStringFromString(value->name()) : noneString();
    return s;
}

const QString toQStringFromMorphKeyframe(const IMorphKeyframe *value)
{
    const QString &s = value ? toQStringFromString(value->name()) : noneString();
    return s;
}

}
