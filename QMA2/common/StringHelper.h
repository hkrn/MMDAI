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

#ifndef STRINGHELPER_H
#define STRINGHELPER_H

#include <QtCore/QString>

#include <vpvl2/IEncoding.h>
#include <vpvl2/IString.h>

class QTextCodec;

namespace vpvl2 {
class IBone;
class IBoneKeyframe;
class IModel;
class IMorph;
class IMorphKeyframe;
class IMotion;
class IString;
}

namespace internal {

using namespace vpvl2;

class String : public IString {
public:
    explicit String(const QString &s, IString::Codec codec = IString::kUTF8);
    ~String();

    bool startsWith(const IString *value) const;
    bool contains(const IString *value) const;
    bool endsWith(const IString *value) const;
    IString *clone() const;
    const HashString toHashString() const;
    bool equals(const IString *value) const;
    const QString &value() const;
    const uint8_t *toByteArray() const;
    size_t length() const;

private:
    const QByteArray m_bytes;
    const QString m_value;
    const IString::Codec m_codec;
};

class Encoding : public IEncoding {
public:
    Encoding();
    ~Encoding();

    const IString *stringConstant(ConstantType value) const;
    IString *toString(const uint8_t *value, size_t size, IString::Codec codec) const;
    IString *toString(const uint8_t *value, IString::Codec codec, size_t maxlen) const;
    uint8_t *toByteArray(const IString *value, IString::Codec codec) const;
    void disposeByteArray(uint8_t *value) const;

private:
    const QTextCodec *m_sjis;
    const QTextCodec *m_utf8;
    const QTextCodec *m_utf16;
};

QTextCodec *getTextCodec();
const QByteArray toByteArrayFromQString(const QString &value);
const QString toQStringFromBytes(const uint8_t *value);
const QString toQStringFromString(const vpvl2::IString *value);
const QString toQStringFromModel(const vpvl2::IModel *value);
const QString toQStringFromMotion(const vpvl2::IMotion *value);
const QString toQStringFromBone(const vpvl2::IBone *value);
const QString toQStringFromMorph(const vpvl2::IMorph *value);
const QString toQStringFromBoneKeyframe(const vpvl2::IBoneKeyframe *value);
const QString toQStringFromMorphKeyframe(const vpvl2::IMorphKeyframe *value);

}

#endif
