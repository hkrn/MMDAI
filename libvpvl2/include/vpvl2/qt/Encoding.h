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

#ifndef VPVL2_QT_ENCODING_H_
#define VPVL2_QT_ENCODING_H_

#include "vpvl2/IEncoding.h"
#include "vpvl2/IString.h"

#include <QHash>
#include <QString>

class QTextCodec;

namespace vpvl2
{
namespace qt
{

class CString;

class Encoding : public IEncoding
{
public:
    typedef QHash<ConstantType, const CString *> Dictionary;
    Encoding(const Dictionary &dictionary);
    ~Encoding();

    const IString *stringConstant(ConstantType value) const;
    IString *toString(const uint8_t *value, size_t size, IString::Codec codec) const;
    IString *toString(const uint8_t *value, IString::Codec codec, size_t maxlen) const;
    uint8_t *toByteArray(const IString *value, IString::Codec codec) const;
    void disposeByteArray(uint8_t *value) const;

private:
    Dictionary m_dictionary;
    QTextCodec *m_sjis;
    QTextCodec *m_utf8;
    QTextCodec *m_utf16;

    VPVL2_DISABLE_COPY_AND_ASSIGN(Encoding)
};

} /* namespace qt */
} /* namespace vpvl2 */

#endif /* VPVL2_QT_ENCODING_H_ */
