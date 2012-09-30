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

#ifndef VPVL2_ICU_STRING_H_
#define VPVL2_ICU_STRING_H_

#include <string>
#include <vector>

#include <vpvl2/IString.h>

/* ICU */
#include <unicode/unistr.h>
#include <unicode/regex.h>

namespace vpvl2
{
namespace icu
{

static const char *kDefaultEncoding = "utf8";

class String : public IString {
public:
    static inline const std::string toStdString(const UnicodeString &value) {
        size_t size = value.length(), length = value.extract(0, size, 0, kDefaultEncoding);
        std::vector<uint8_t> bytes(length + 1);
        value.extract(0, size, reinterpret_cast<char *>(bytes.data()), kDefaultEncoding);
        return std::string(bytes.begin(), bytes.end() - 1);
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

    String(const UnicodeString &value)
        : m_value(value),
          m_bytes(0)
    {
        size_t size = value.length(), length = value.extract(0, size, 0, kDefaultEncoding);
        m_bytes = new uint8_t[length + 1];
        value.extract(0, size, reinterpret_cast<char *>(m_bytes), kDefaultEncoding);
        m_bytes[length] = 0;
    }
    ~String() {
        delete[] m_bytes;
    }

    bool startsWith(const IString *value) const {
        return m_value.startsWith(static_cast<const String *>(value)->value());
    }
    bool contains(const IString *value) const {
        return m_value.indexOf(static_cast<const String *>(value)->value()) != -1;
    }
    bool endsWith(const IString *value) const {
        return m_value.endsWith(static_cast<const String *>(value)->value());
    }
    void split(const IString *separator, int maxTokens, Array<IString *> &tokens) const {
        tokens.clear();
        if (maxTokens > 0) {
            UErrorCode code = U_ZERO_ERROR;
            const UnicodeString &pattern = static_cast<const String *>(separator)->value();
            RegexMatcher matcher("\\Q" + pattern + "\\E", 0, code);
            Array<UnicodeString> words;
            words.resize(maxTokens);
            tokens.reserve(maxTokens);
            matcher.split(m_value, &words[0], maxTokens, code);
            int nwords = words.count();
            for (int i = 0; i < nwords; i++) {
                tokens.add(new String(words[i]));
            }
        }
        else if (maxTokens == 0) {
            tokens.add(new String(m_value));
        }
    }
    IString *clone() const {
        return new String(m_value);
    }
    const HashString toHashString() const {
        return HashString(reinterpret_cast<const char *>(m_bytes));
    }
    bool equals(const IString *value) const {
        return m_value == static_cast<const String *>(value)->value();
    }
    const UnicodeString &value() const {
        return m_value;
    }
    const uint8_t *toByteArray() const {
        return m_bytes;
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
    uint8_t *m_bytes;
};

} /* namespace icu */
} /* namespace vpvl2 */

#endif
