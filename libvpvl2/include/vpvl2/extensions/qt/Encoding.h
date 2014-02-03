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
#ifndef VPVL2_EXTENSIONS_QT_ENCODING_H_
#define VPVL2_EXTENSIONS_QT_ENCODING_H_

#include <vpvl2/config.h>
#include <vpvl2/IEncoding.h>
#include <vpvl2/extensions/qt/String.h>
#include <QTextCodec>

namespace vpvl2
{
namespace extensions
{
namespace qt
{

class Encoding VPVL2_DECL_FINAL : public IEncoding
{
public:
    typedef Hash<HashInt, const IString *> Dictionary;

    static bool initializeOnce() {
        /* do nothing */
        return true;
    }

    Encoding(const Dictionary *dictionaryRef)
        : m_dictionaryRef(dictionaryRef),
          m_null(QString())
    {
    }
    ~Encoding() {
    }

    const IString *stringConstant(ConstantType value) const {
        if (const IString *const *s = m_dictionaryRef->find(value)) {
            return *s;
        }
        return &m_null;
    }
    IString *toString(const uint8 *value, vsize size, IString::Codec codec) const {
        QTextCodec *converter = detectTextCodec(codec);
        Q_ASSERT(converter);
        QString us = converter->toUnicode(reinterpret_cast<const char *>(value), size);
        /* remove head and trail spaces and 0x1a (appended by PMDEditor) */
        us.replace(QChar(0x1a), QChar());
        us = us.trimmed();
        IString *s = new (std::nothrow) String(us, converter);
        return s;
    }
    IString *toString(const uint8 *value, IString::Codec codec, vsize maxlen) const {
        if (maxlen > 0 && value) {
            vsize size = qstrlen(reinterpret_cast<const char *>(value));
            return toString(value, qMin(maxlen, size), codec);
        }
        else {
            return new(std::nothrow) String(QString());
        }
    }
    vsize estimateSize(const IString *value, IString::Codec codec) const {
        const String *s = static_cast<const String *>(value);
        const QTextCodec *converter = detectTextCodec(codec);
        Q_ASSERT(converter);
        vsize size = s ? converter->fromUnicode(s->value()).size() : 0;
        return size;
    }
    uint8 *toByteArray(const IString *value, IString::Codec codec) const {
        uint8 *data = 0;
        if (const String *s = static_cast<const String *>(value)) {
            QTextCodec *converter = detectTextCodec(codec);
            Q_ASSERT(converter);
            QByteArray bytes = converter->fromUnicode(s->value());
            data = new (std::nothrow) uint8[bytes.size() + 1];
            Q_CHECK_PTR(data);
            qstrcpy(reinterpret_cast<char *>(data), bytes);
        }
        else {
            data = new (std::nothrow) uint8[1];
            Q_CHECK_PTR(data);
            data[0] = 0;
        }
        return data;
    }
    void disposeByteArray(uint8 *&value) const {
        delete[] value;
        value = 0;
    }
    IString::Codec detectCodec(const char *data, vsize length) const {
        const QString &name = QTextCodec::codecForUtfText(QByteArray(data, length))->name();
        if (name == "UTF-8") {
            return IString::kUTF8;
        }
        else if (name == "UTF-16LE") {
            return IString::kUTF16;
        }
        else if (name == "Shift-JIS") {
            return IString::kShiftJIS;
        }
        return IString::kUTF8;
    }
    IString *createString(const QString &value) const {
        return new String(value);
    }

private:
    static QTextCodec *detectTextCodec(IString::Codec type) {
        switch (type) {
        case IString::kShiftJIS:
            return QTextCodec::codecForName("Shift-JIS");
        case IString::kUTF8:
            return QTextCodec::codecForName("UTF-8");
        case IString::kUTF16:
            return QTextCodec::codecForName("UTF-16LE");
        case IString::kMaxCodecType:
        default:
            return 0;
        }
    }

    const Dictionary *m_dictionaryRef;
    const String m_null;

    VPVL2_DISABLE_COPY_AND_ASSIGN(Encoding)
};

} /* namespace qt */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif
