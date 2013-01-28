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

#pragma once
#ifndef VPVL2_EXTENSIONS_ICU_ENCODING_H_
#define VPVL2_EXTENSIONS_ICU_ENCODING_H_

#include <vpvl2/IEncoding.h>
#include <vpvl2/extensions/icu4c/String.h>

#include <string.h> /* for strlen */

namespace vpvl2
{
namespace extensions
{
namespace icu4c
{

class Encoding : public IEncoding {
public:
    typedef Hash<HashInt, const String *> Dictionary;

    explicit Encoding(const Dictionary *dictionaryRef)
        : m_dictionaryRef(dictionaryRef),
          m_null(UnicodeString())
    {
        m_converter.open();
    }
    ~Encoding() {
    }

    const IString *stringConstant(ConstantType value) const {
        if (const String *const *s = m_dictionaryRef->find(value)) {
            return *s;
        }
        return &m_null;
    }
    IString *toString(const uint8_t *value, size_t size, IString::Codec codec) const {
        IString *s = 0;
        UErrorCode status = U_ZERO_ERROR;
        const char *str = reinterpret_cast<const char *>(value);
        switch (codec) {
        case IString::kShiftJIS:
            s = new String(UnicodeString(str, size, m_converter.shiftJIS, status));
            break;
        case IString::kUTF8:
            s = new String(UnicodeString(str, size, m_converter.utf8, status));
            break;
        case IString::kUTF16:
            s = new String(UnicodeString(str, size, m_converter.utf16, status));
            break;
        case IString::kMaxCodecType:
        default:
            break;
        }
        return s;
    }
    IString *toString(const uint8_t *value, IString::Codec codec, size_t maxlen) const {
        if (maxlen > 0 && value) {
            size_t size = strlen(reinterpret_cast<const char *>(value));
            return toString(value, std::min(maxlen, size), codec);
        }
        else {
            return new(std::nothrow) String(UnicodeString::fromUTF8(""));
        }
    }
    uint8_t *toByteArray(const IString *value, IString::Codec codec) const {
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
                size_t size = s->size(), newStringLength = src.extract(0, 0, converter, status);
                data = new (std::nothrow) uint8_t[newStringLength + 1];
                if (data) {
                    src.extract(reinterpret_cast<char *>(data), size, converter, status);
                    data[newStringLength] = 0;
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
    void disposeByteArray(uint8_t *value) const {
        delete[] value;
    }

    IString *createString(const UnicodeString &value) const {
        return new String(value, &m_converter);
    }

private:
    const Dictionary *m_dictionaryRef;
    const String m_null;
    String::Converter m_converter;

    VPVL2_DISABLE_COPY_AND_ASSIGN(Encoding)
};

} /* namespace icu */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif
