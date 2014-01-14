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
#ifndef VPVL2_EXTENSIONS_QT_STRING_H_
#define VPVL2_EXTENSIONS_QT_STRING_H_

#include <vpvl2/config.h>
#include <vpvl2/IString.h>
#include <QString>
#include <QStringList>

namespace vpvl2
{
namespace extensions
{
namespace qt
{

class String VPVL2_DECL_FINAL : public IString
{
public:
    static IString *create(const std::string &value) {
        return new String(QString::fromStdString(value));
    }
    static std::string toStdString(const QString &value) {
        return value.toStdString();
    }

    String(const QString &value)
        : m_value(value)
    {
        const QByteArray &bytes = value.toUtf8();
        m_bytes.resize(bytes.length() + 1);
        qstrcpy(reinterpret_cast<char *>(&m_bytes[0]), bytes.constData());
    }
    ~String() {
    }

    bool startsWith(const IString *value) const {
        return m_value.startsWith(static_cast<const String *>(value)->value());
    }
    bool contains(const IString *value) const {
        return m_value.contains(static_cast<const String *>(value)->value());
    }
    bool endsWith(const IString *value) const {
        return m_value.endsWith(static_cast<const String *>(value)->value());
    }
    void split(const IString *separator, int maxTokens, Array<IString *> &tokens) const {
        tokens.clear();
        if (maxTokens > 0) {
            foreach (const QString &s, m_value.split(static_cast<const String *>(separator)->value())) {
                tokens.append(new String(s));
            }
            tokens.resize(maxTokens);
        }
        else if (maxTokens == 0) {
            tokens.append(new String(m_value));
        }
        else {
            foreach (const QString &s, m_value.split(static_cast<const String *>(separator)->value())) {
                tokens.append(new String(s));
            }
        }
    }
    IString *join(const Array<IString *> &tokens) const {
        QString s;
        const int ntokens = tokens.count();
        for (int i = 0 ; i < ntokens; i++) {
            const IString *token = tokens[i];
            s.append(static_cast<const String *>(token)->value());
            if (i != ntokens - 1) {
                s.append(m_value);
            }
        }
        return new String(s);
    }
    IString *clone() const {
        return new String(m_value);
    }
    const HashString toHashString() const {
        return HashString(reinterpret_cast<const char *>(&m_bytes[0]));
    }
    bool equals(const IString *value) const {
        return value && m_value == static_cast<const String *>(value)->value();
    }
    QString value() const {
        return m_value;
    }
    std::string toStdString() const {
        return toStdString(m_value);
    }
    const uint8 *toByteArray() const {
        return &m_bytes[0];
    }
    vsize size() const {
        return m_value.length();
    }
    vsize length(Codec /* codec */) const {
        return m_value.length();
    }

private:
    const QString m_value;
    Array<uint8> m_bytes;

    VPVL2_DISABLE_COPY_AND_ASSIGN(String)
};

} /* namespace qt */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif

