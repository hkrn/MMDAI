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

#include "Encoding.h"
#include "CString.h"

#include <QtCore/QtCore>

using namespace vpvl2;

namespace vpvl2
{
namespace render
{
namespace qt
{

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
        static const CString s("左");
        return &s;
    }
    case kRight: {
        static const CString s("右");
        return &s;
    }
    case kFinger: {
        static const CString s("指");
        return &s;
    }
    case kElbow: {
        static const CString s("ひじ");
        return &s;
    }
    case kArm: {
        static const CString s("腕");
        return &s;
    }
    case kWrist: {
        static const CString s("手首");
        return &s;
    }
    case kCenter: {
        static const CString s("センター");
        return &s;
    }
    default: {
        static const CString s("");
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
        s = new CString(m_sjis->toUnicode(str, size));
        break;
    case IString::kUTF8:
        s = new CString(m_utf8->toUnicode(str, size));
        break;
    case IString::kUTF16:
        s = new CString(m_utf16->toUnicode(str, size));
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
    const CString *s = static_cast<const CString *>(value);
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

void Encoding::disposeByteArray(uint8_t *value) const
{
    delete[] value;
}

}
}
}

