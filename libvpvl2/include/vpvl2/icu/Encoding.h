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

#ifndef VPVL2_ICU_ENCODING_H_
#define VPVL2_ICU_ENCODING_H_

#include <vpvl2/IEncoding.h>
#include <vpvl2/icu/String.h>

/* ICU */
#include <unicode/unistr.h>

namespace vpvl2
{
namespace icu
{

class Encoding : public IEncoding {
public:
    Encoding()
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
        case kCenter: {
            static const String s("センター");
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
            s = new String(UnicodeString(str, size, "shift_jis"));
            break;
        case IString::kUTF8:
            s = new String(UnicodeString(str, size, "utf-8"));
            break;
        case IString::kUTF16:
            s = new String(UnicodeString(str, size, "utf-16le"));
            break;
        default:
            break;
        }
        return s;
    }
    IString *toString(const uint8_t *value, IString::Codec codec, size_t maxlen) const {
        size_t size = strlen(reinterpret_cast<const char *>(value));
        return toString(value, btMin(maxlen, size), codec);
    }
    uint8_t *toByteArray(const IString *value, IString::Codec codec) const {
        if (value) {
            const String *s = static_cast<const String *>(value);
            const UnicodeString &src = s->value();
            const char *codecTo = 0;
            switch (codec) {
            case IString::kShiftJIS:
                codecTo = "shift_jis";
                break;
            case IString::kUTF8:
                codecTo = "utf-8";
                break;
            case IString::kUTF16:
                codecTo = "utf-16le";
                break;
            default:
                break;
            }
            size_t size = s->length(), newStringLength = src.extract(0, size, 0, codecTo);
            uint8_t *data = new uint8_t[newStringLength + 1];
            src.extract(0, size, reinterpret_cast<char *>(data), codecTo);
            data[newStringLength] = 0;
            return data;
        }
        return 0;
    }
    void disposeByteArray(uint8_t *value) const {
        delete[] value;
    }
};

} /* namespace icu */
} /* namespace vpvl2 */

#endif
