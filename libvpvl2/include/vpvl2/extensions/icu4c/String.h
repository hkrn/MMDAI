/**

 Copyright (c) 2010-2014  hkrn

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

#pragma once
#ifndef VPVL2_EXTENSIONS_ICU_STRING_H_
#define VPVL2_EXTENSIONS_ICU_STRING_H_

/* ICU */
#include <unicode/unistr.h>
#include <unicode/ucnv.h>

#include <string>
#include <vpvl2/IString.h>

namespace vpvl2
{
namespace extensions
{
namespace icu4c
{

class VPVL2_API String VPVL2_DECL_FINAL : public IString {
public:
    struct Converter {
        Converter()
            : shiftJIS(0),
              utf8(0),
              utf16(0)
        {
        }
        ~Converter() {
            ucnv_close(utf8);
            utf8 = 0;
            ucnv_close(utf16);
            utf16 = 0;
            ucnv_close(shiftJIS);
            shiftJIS = 0;
        }
        void initialize() {
            UErrorCode status = U_ZERO_ERROR;
            utf8  = ucnv_open("utf-8", &status);
            utf16 = ucnv_open("utf-16le", &status);
            shiftJIS  = ucnv_open("ibm-943_P15A-2003", &status);
        }
        UConverter *converterFromCodec(IString::Codec codec) const {
            switch (codec) {
            case IString::kShiftJIS:
                return shiftJIS;
            case IString::kUTF8:
                return utf8;
            case IString::kUTF16:
                return utf16;
            case IString::kMaxCodecType:
            default:
                return 0;
            }
        }
        UConverter *shiftJIS;
        UConverter *utf8;
        UConverter *utf16;
    };
    struct Less {
        /* use custom std::less alternative to prevent warning on MSVC */
        bool operator()(const UnicodeString &left, const UnicodeString &right) const {
            return left.compare(right) == -1;
        }
    };

    static IString *create(const std::string &value, const Converter *converterRef = 0);
    static std::string toStdString(const UnicodeString &value);

    explicit String(const UnicodeString &value, const Converter *converterRef = 0);
    ~String();

    bool startsWith(const IString *value) const;
    bool contains(const IString *value) const;
    bool endsWith(const IString *value) const;
    void split(const IString *separator, int maxTokens, Array<IString *> &tokens) const;
    IString *join(const Array<IString *> &tokens) const;
    IString *clone() const;
    const HashString toHashString() const;
    bool equals(const IString *value) const;
    UnicodeString value() const;
    std::string toStdString() const;
    const uint8 *toByteArray() const;
    vsize size() const;
    vsize length(Codec codec) const;

private:
    const Converter *m_converterRef;
    const UnicodeString m_value;
    Array<uint8> m_bytes;

    VPVL2_DISABLE_COPY_AND_ASSIGN(String)
};

} /* namespace icu */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif
