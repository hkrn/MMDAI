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

#include <vpvl2/extensions/icu4c/Encoding.h>

#include <string.h> /* for strlen */

#ifdef WIN32
#define strncasecmp _strnicmp
#endif

namespace vpvl2
{
namespace extensions
{
namespace icu4c
{

Encoding::Encoding(const Dictionary *dictionaryRef)
    : m_dictionaryRef(dictionaryRef),
      m_null(UnicodeString()),
      m_detector(0)
{
    UErrorCode status = U_ZERO_ERROR;
    m_detector = ucsdet_open(&status);
    m_converter.initialize();
}

Encoding::~Encoding()
{
    ucsdet_close(m_detector);
    m_detector = 0;
    m_dictionaryRef = 0;
}

const IString *Encoding::stringConstant(ConstantType value) const
{
    if (const String *const *s = m_dictionaryRef->find(value)) {
        return *s;
    }
    return &m_null;
}

IString *Encoding::toString(const uint8_t *value, size_t size, IString::Codec codec) const
{
    const char *str = reinterpret_cast<const char *>(value);
    UConverter *converter = 0;
    switch (codec) {
    case IString::kShiftJIS:
        converter = m_converter.shiftJIS;
        break;
    case IString::kUTF8:
        converter = m_converter.utf8;
        break;
    case IString::kUTF16:
        converter = m_converter.utf16;
        break;
    case IString::kMaxCodecType:
    default:
        break;
    }
    IString *s = 0;
    if (converter) {
        UErrorCode status = U_ZERO_ERROR;
        UnicodeString us(str, size, converter, status);
        /* remove head and trail spaces and 0x1a (appended by PMDEditor) */
        s = new (std::nothrow) String(us.trim().findAndReplace(UChar(0x1a), UChar()), &m_converter);
    }
    return s;
}

IString *Encoding::toString(const uint8_t *value, IString::Codec codec, size_t maxlen) const
{
    if (maxlen > 0 && value) {
        size_t size = strlen(reinterpret_cast<const char *>(value));
        return toString(value, std::min(maxlen, size), codec);
    }
    else {
        return new(std::nothrow) String(UnicodeString(), &m_converter);
    }
}

uint8_t *Encoding::toByteArray(const IString *value, IString::Codec codec) const
{
    uint8_t *data = 0;
    if (value) {
        const String *s = static_cast<const String *>(value);
        const UnicodeString &src = s->value();
        UConverter *converter = 0;
        switch (codec) {
        case IString::kShiftJIS:
            converter = m_converter.shiftJIS;
            break;
        case IString::kUTF8:
            converter = m_converter.utf8;
            break;
        case IString::kUTF16:
            converter = m_converter.utf16;
            break;
        case IString::kMaxCodecType:
        default:
            break;
        }
        if (converter) {
            UErrorCode status = U_ZERO_ERROR;
            size_t newStringLength = src.extract(0, 0, converter, status) + 1;
            data = new (std::nothrow) uint8_t[newStringLength];
            if (data) {
                status = U_ZERO_ERROR;
                src.extract(reinterpret_cast<char *>(data), newStringLength, converter, status);
            }
        }
    }
    else {
        data = new (std::nothrow) uint8_t[1];
        if (data) {
            data[0] = 0;
        }
    }
    return data;
}

void Encoding::disposeByteArray(uint8_t *value) const
{
    delete[] value;
}

IString::Codec Encoding::detectCodec(const char *data, size_t length) const
{
    UErrorCode status = U_ZERO_ERROR;
    ucsdet_setText(m_detector, data, length, &status);
    const UCharsetMatch *match = ucsdet_detect(m_detector, &status);
    const char *charset = ucsdet_getName(match, &status);
    IString::Codec codec = IString::kUTF8;
    struct CodecMap {
        IString::Codec codec;
        const char *name;
    };
    static const CodecMap codecMap[] = {
        { IString::kShiftJIS, "shift_jis" },
        { IString::kUTF8,     "utf-8"     },
        { IString::kUTF16,    "utf-16"    }
    };
    for (size_t i = 0; i < sizeof(codecMap) / sizeof(codecMap[0]); i++) {
        const CodecMap &item = codecMap[i];
        if (strncasecmp(charset, item.name, strlen(item.name)) == 0) {
            codec = item.codec;
        }
    }
    return codec;
}

IString *Encoding::createString(const UnicodeString &value) const
{
    return new String(value, &m_converter);
}

} /* namespace icu4c */
} /* namespace extensions */
} /* namespace vpvl2 */
