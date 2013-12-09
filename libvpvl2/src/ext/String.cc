/**

 Copyright (c) 2010-2013  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#include <vpvl2/extensions/icu4c/String.h>
#include <vpvl2/internal/util.h>

namespace vpvl2
{
namespace extensions
{
namespace icu4c
{

IString *String::create(const std::string &value, const Converter *converterRef)
{
    return new String(UnicodeString::fromUTF8(value), converterRef);
}

std::string String::toStdString(const UnicodeString &value)
{
    std::string str;
    UErrorCode status = U_ZERO_ERROR;
    vsize length = value.extract(0, 0, static_cast<UConverter *>(0), status);
    str.resize(length);
    status = U_ZERO_ERROR;
    value.extract(&str[0], int32_t(str.size()), 0, status);
    return str;
}

String::String(const UnicodeString &value, const Converter *converterRef)
    : m_converterRef(converterRef),
      m_value(value)
{
    UErrorCode status = U_ZERO_ERROR;
    int32_t length = value.extract(0, 0, static_cast<UConverter *>(0), status);
    status = U_ZERO_ERROR;
    m_bytes.resize(length + 1);
    value.extract(reinterpret_cast<char *>(&m_bytes[0]),
            m_bytes.count(),
            static_cast<UConverter *>(converterRef ? converterRef->utf8 : 0), status);
}

String::~String()
{
    m_converterRef = 0;
}

bool String::startsWith(const IString *value) const
{
    return m_value.startsWith(static_cast<const String *>(value)->value()) == TRUE;
}

bool String::contains(const IString *value) const
{
    return m_value.indexOf(static_cast<const String *>(value)->value()) != -1;
}

bool String::endsWith(const IString *value) const
{
    return m_value.endsWith(static_cast<const String *>(value)->value()) == TRUE;
}

void String::split(const IString *separator, int maxTokens, Array<IString *> &tokens) const
{
    tokens.clear();
    if (maxTokens > 0) {
        const UnicodeString &sep = static_cast<const String *>(separator)->value();
        int32 offset = 0, pos = 0, size = sep.length(), nwords = 0;
        while ((pos = m_value.indexOf(sep, offset)) >= 0) {
            tokens.append(new String(m_value.tempSubString(offset, pos - offset), m_converterRef));
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
            tokens[lastArrayOffset] = new String(s2 + sp + m_value.tempSubString(offset), m_converterRef);
            internal::deleteObject(s);
        }
    }
    else if (maxTokens == 0) {
        tokens.append(new String(m_value, m_converterRef));
    }
    else {
        const UnicodeString &sep = static_cast<const String *>(separator)->value();
        int32 offset = 0, pos = 0, size = sep.length();
        while ((pos = m_value.indexOf(sep, offset)) >= 0) {
            tokens.append(new String(m_value.tempSubString(offset, pos - offset), m_converterRef));
            offset = pos + size;
        }
        tokens.append(new String(m_value.tempSubString(offset), m_converterRef));
    }
}

IString *String::join(const Array<IString *> &tokens) const
{
    UnicodeString s;
    const int ntokens = tokens.count();
    for (int i = 0 ; i < ntokens; i++) {
        const IString *token = tokens[i];
        s.append(static_cast<const String *>(token)->value());
        if (i != ntokens - 1) {
            s.append(m_value);
        }
    }
    return new String(s, m_converterRef);
}

IString *String::clone() const
{
    return new String(m_value, m_converterRef);
}

const HashString String::toHashString() const
{
    return HashString(reinterpret_cast<const char *>(&m_bytes[0]));
}

bool String::equals(const IString *value) const
{
    return value && m_value.compare(static_cast<const String *>(value)->value()) == 0;
}

UnicodeString String::value() const
{
    return m_value;
}

std::string String::toStdString() const
{
    return toStdString(m_value);
}

const uint8 *String::toByteArray() const
{
    return &m_bytes[0];
}

vsize String::size() const
{
    return m_value.length();
}

vsize String::length(Codec codec) const
{
    if (m_converterRef) {
        UErrorCode status = U_ZERO_ERROR;
        UConverter *converter = 0;
        switch (codec) {
        case kShiftJIS:
            converter = m_converterRef->shiftJIS;
            break;
        case kUTF8:
            converter = m_converterRef->utf8;
            break;
        case kUTF16:
            converter = m_converterRef->utf16;
            break;
        case kMaxCodecType:
        default:
            break;
        }
        return m_value.extract(0, 0, converter, status);
    }
    else {
        const char *codecString = "utf-8";
        switch (codec) {
        case kShiftJIS:
            codecString = "shift_jis";
            break;
        case kUTF8:
            codecString = "utf-8";
            break;
        case kUTF16:
            codecString = "utf-16le";
            break;
        case kMaxCodecType:
        default:
            break;
        }
        return m_value.extract(0, m_value.length(), 0, codecString);
    }
}

} /* namespace icu4c */
} /* namespace extensions */
} /* namespace vpvl2 */
