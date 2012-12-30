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

#include "vpvl2/qt/CString.h"

#include <QtCore/QString>
#include <QtCore/QStringList>

namespace vpvl2
{
namespace qt
{

CString::CString(const QString &s)
    : m_bytes(s.toUtf8()),
      m_value(s)
{
}

CString::~CString()
{
}

bool CString::startsWith(const IString *value) const
{
    return m_value.startsWith(static_cast<const CString *>(value)->m_value);
}

bool CString::contains(const IString *value) const
{
    return m_value.contains(static_cast<const CString *>(value)->m_value);
}

bool CString::endsWith(const IString *value) const
{
    return m_value.endsWith(static_cast<const CString *>(value)->m_value);
}

void CString::split(const IString *separator, int maxTokens, Array<IString *> &tokens) const
{
    const QString &sep = static_cast<const CString *>(separator)->value();
    if (maxTokens > 1) {
        const QStringList &words = m_value.split(sep);
        int nwords = words.size();
        int mwords = qMin(nwords, maxTokens), i = 0;
        for (i = 0; i < mwords; i++) {
            const QString &word = words[i];
            tokens.add(new CString(word));
        }
        if (nwords > maxTokens) {
            QStringList concat;
            for (int j = i; j < nwords; j++) {
                concat.append(words[j]);
            }
            tokens.add(new CString(concat.join("")));
        }
    }
    else if (maxTokens == 0) {
        tokens.add(new CString(m_value));
    }
    else {
        const QStringList &words = m_value.split(sep);
        foreach (const QString &word, words) {
            tokens.add(new CString(word));
        }
    }
}

IString *CString::clone() const
{
    return new CString(m_value);
}

const HashString CString::toHashString() const
{
    return HashString(m_bytes.constData());
}

bool CString::equals(const IString *value) const
{
    return m_value == static_cast<const CString *>(value)->m_value;
}

QString CString::value() const
{
    return m_value;
}

const uint8_t *CString::toByteArray() const
{
    return reinterpret_cast<const uint8_t *>(m_bytes.constData());
}

size_t CString::size() const
{
    return m_value.length();
}

size_t CString::length(Codec /*codec*/) const
{
    return m_bytes.length();
}

}
}
