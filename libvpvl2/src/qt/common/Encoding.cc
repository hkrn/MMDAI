/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
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

#include "vpvl2/qt/Encoding.h"
#include "vpvl2/qt/CString.h"

#include <QtCore/QtCore>

namespace vpvl2
{
namespace qt
{

Encoding::Encoding(const Dictionary &dictionary)
    : m_dictionary(dictionary),
      m_sjisCodecRef(QTextCodec::codecForName("Shift-JIS")),
      m_utf8CodecRef(QTextCodec::codecForName("UTF-8")),
      m_utf16CodecRef(QTextCodec::codecForName("UTF-16"))
{
}

Encoding::~Encoding()
{
}

const IString *Encoding::stringConstant(ConstantType value) const
{
    if (m_dictionary.contains(value)) {
        return m_dictionary[value];
    }
    else {
        static const CString s("");
        return &s;
    }
}

IString *Encoding::toString(const uint8_t *value, size_t size, IString::Codec codec) const
{
    IString *s = 0;
    if (value) {
        const char *str = reinterpret_cast<const char *>(value);
        switch (codec) {
        case IString::kShiftJIS:
            s = new CString(m_sjisCodecRef->toUnicode(str, size));
            break;
        case IString::kUTF8:
            s = new CString(m_utf8CodecRef->toUnicode(str, size));
            break;
        case IString::kUTF16:
            s = new CString(m_utf16CodecRef->toUnicode(str, size));
            break;
        default:
            break;
        }
    }
    else {
        s = new CString("");
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
    if (value) {
        const CString *s = static_cast<const CString *>(value);
        QByteArray bytes;
        switch (codec) {
        case IString::kShiftJIS:
            bytes = m_sjisCodecRef->fromUnicode(s->value());
            break;
        case IString::kUTF8:
            bytes = m_utf8CodecRef->fromUnicode(s->value());
            break;
        case IString::kUTF16:
            bytes = m_utf16CodecRef->fromUnicode(s->value());
            break;
        default:
            break;
        }
        size_t size = bytes.length();
        uint8_t *data = new uint8_t[size + 1];
        memcpy(data, bytes.constData(), size);
        data[size] = 0;
        return data;
    }
    else {
        uint8_t *s = new uint8_t[1];
        s[0] = 0;
        return s;
    }
}

void Encoding::disposeByteArray(uint8_t *value) const
{
    delete[] value;
}

}
}

