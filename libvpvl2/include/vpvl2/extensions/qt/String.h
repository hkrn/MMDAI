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
#include <QTextCodec>

namespace vpvl2
{
namespace VPVL2_VERSION_NS
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

    String(const QString &value, const QTextCodec *converter = QTextCodec::codecForName("UTF-8"))
        : m_value(value),
          m_converter(converter),
          m_utf8(value.toStdString())
    {
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
                tokens.append(new String(s, m_converter));
            }
            tokens.resize(maxTokens);
        }
        else if (maxTokens == 0) {
            tokens.append(new String(m_value, m_converter));
        }
        else {
            foreach (const QString &s, m_value.split(static_cast<const String *>(separator)->value())) {
                tokens.append(new String(s, m_converter));
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
        return new String(s, m_converter);
    }
    IString *clone() const {
        return new String(m_value, m_converter);
    }
    const HashString toHashString() const {
        /* first argument of HashString's construct must be on memory after calling this (toHashString) */
        return HashString(m_utf8.c_str());
    }
    bool equals(const IString *value) const {
        return value && m_value == static_cast<const String *>(value)->value();
    }
    QString value() const {
        return m_value;
    }
    std::string toStdString() const {
        return m_utf8;
    }
    const uint8 *toByteArray() const {
        return reinterpret_cast<const uint8 *>(m_utf8.c_str());
    }
    vsize size() const {
        return m_utf8.size();
    }

private:
    const QString m_value;
    const QTextCodec *m_converter;
    const std::string m_utf8;

    VPVL2_DISABLE_COPY_AND_ASSIGN(String)
};

} /* namespace qt */
} /* namespace extensions */
} /* namespace VPVL2_VERSION_NS */
using namespace VPVL2_VERSION_NS;

} /* namespace vpvl2 */

#endif
