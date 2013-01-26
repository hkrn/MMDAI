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
#ifndef VPVL2_EXTENSIONS_ICU_STRING_H_
#define VPVL2_EXTENSIONS_ICU_STRING_H_

#include <string>
#include <vpvl2/IString.h>

/* ICU */
#include <unicode/unistr.h>

namespace vpvl2
{
namespace extensions
{
namespace icu
{

static const char *kDefaultEncoding = "utf8";

class String : public IString {
public:
    static inline const std::string toStdString(const UnicodeString &value) {
        Array<uint8_t> bytes;
        size_t size = value.length(), length = value.extract(0, size, 0, kDefaultEncoding);
        bytes.resize(length + 1);
        value.extract(0, size, reinterpret_cast<char *>(&bytes[0]), kDefaultEncoding);
        return std::string(&bytes[0], &bytes[bytes.count() - 1]);
    }
    static inline bool toBoolean(const UnicodeString &value) {
        return value == "true" || value == "1" || value == "y" || value == "yes";
    }
    static inline int toInt(const UnicodeString &value, int def = 0) {
        int v = int(strtol(toStdString(value).c_str(), 0, 10));
        return v != 0 ? v : def;
    }
    static inline double toDouble(const UnicodeString &value, double def = 0.0) {
        double v = strtod(toStdString(value).c_str(), 0);
        return v != 0.0 ? float(v) : def;
    }

    explicit String(const UnicodeString &value)
        : m_value(value)
    {
        size_t size = value.length(), length = value.extract(0, size, 0, kDefaultEncoding);
        m_bytes.resize(length + 1);
        value.extract(0, size, reinterpret_cast<char *>(&m_bytes[0]), kDefaultEncoding);
        m_bytes[length] = 0;
    }
    ~String() {
    }

    bool startsWith(const IString *value) const {
        return m_value.startsWith(static_cast<const String *>(value)->value()) == TRUE;
    }
    bool contains(const IString *value) const {
        return m_value.indexOf(static_cast<const String *>(value)->value()) != -1;
    }
    bool endsWith(const IString *value) const {
        return m_value.endsWith(static_cast<const String *>(value)->value()) == TRUE;
    }
    void split(const IString *separator, int maxTokens, Array<IString *> &tokens) const {
        tokens.clear();
        if (maxTokens > 0) {
            const UnicodeString &sep = static_cast<const String *>(separator)->value();
            int32_t offset = 0, pos = 0, size = sep.length(), nwords = 0;
            while ((pos = m_value.indexOf(sep, offset)) >= 0) {
                tokens.add(new String(m_value.tempSubString(offset, pos - offset)));
                offset = pos + size;
                nwords++;
                if (nwords >= maxTokens) {
                    break;
                }
            }
            if (maxTokens - nwords == 0) {
                int lastArrayOffset = tokens.count() - 1;
                IString *s = tokens[lastArrayOffset];
                const UnicodeString &s2 = static_cast<const String *>(s)->value();
                const UnicodeString &sp = static_cast<const String *>(separator)->value();
                tokens[lastArrayOffset] = new String(s2 + sp + m_value.tempSubString(offset));
                delete s;
            }
        }
        else if (maxTokens == 0) {
            tokens.add(new String(m_value));
        }
        else {
            const UnicodeString &sep = static_cast<const String *>(separator)->value();
            int32_t offset = 0, pos = 0, size = sep.length();
            while ((pos = m_value.indexOf(sep, offset)) >= 0) {
                tokens.add(new String(m_value.tempSubString(offset, pos - offset)));
                offset = pos + size;
            }
            tokens.add(new String(m_value.tempSubString(offset)));
        }
    }
    IString *clone() const {
        return new String(m_value);
    }
    const HashString toHashString() const {
        return HashString(reinterpret_cast<const char *>(&m_bytes[0]));
    }
    bool equals(const IString *value) const {
        return (m_value == static_cast<const String *>(value)->value()) == TRUE;
    }
    UnicodeString value() const {
        return m_value;
    }
    const uint8_t *toByteArray() const {
        return &m_bytes[0];
    }
    size_t size() const {
        return m_value.length();
    }
    size_t length(Codec codec) const {
        switch (codec) {
        case kShiftJIS:
            return m_value.extract(0, m_value.length(), 0, "shift_jis");
        case kUTF8:
            return m_value.extract(0, m_value.length(), 0, "shift_jis");
        case kUTF16:
            return m_value.extract(0, m_value.length(), 0, "shift_jis");
        case kMaxCodecType:
        default:
            return 0;
        }
    }

private:
    const UnicodeString m_value;
    Array<uint8_t> m_bytes;

    VPVL2_DISABLE_COPY_AND_ASSIGN(String)
};

} /* namespace icu */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif
