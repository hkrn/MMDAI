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

#include "StringHelper.h"

#include <vpvl2/vpvl2.h>
#include <QtCore/QtCore>
#include "CString.h"

namespace internal {

using namespace vpvl2;
using namespace vpvl2::qt;

QTextCodec *getTextCodec()
{
    static QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
    return codec;
}

const QString &noneString()
{
    static const QString none = QCoreApplication::tr("(none)");
    return none;
}

const QByteArray toByteArrayFromQString(const QString &value)
{
    const QByteArray &bytes = getTextCodec()->fromUnicode(value);
    return bytes;
}

const QString toQStringFromBytes(const uint8_t *value)
{
    const QString &s = getTextCodec()->toUnicode(reinterpret_cast<const char *>(value));
    return s;
}

const QString toQStringFromString(const IString *value)
{
    const QString &s = value ? static_cast<const CString *>(value)->value() : noneString();
    return s;
}

const QString toQStringFromModel(const IModel *value)
{
    const QString &s = value ? toQStringFromString(value->name()) : noneString();
    return s;
}

const QString toQStringFromMotion(const IMotion *value)
{
    const QString &s = value ? toQStringFromString(value->name()) : noneString();
    return s;
}

const QString toQStringFromBone(const IBone *value)
{
    const QString &s = value ? toQStringFromString(value->name()) : noneString();
    return s;
}

const QString toQStringFromMorph(const IMorph *value)
{
    const QString &s = value ? toQStringFromString(value->name()) : noneString();
    return s;
}

const QString toQStringFromBoneKeyframe(const IBoneKeyframe *value)
{
    const QString &s = value ? toQStringFromString(value->name()) : noneString();
    return s;
}

const QString toQStringFromMorphKeyframe(const IMorphKeyframe *value)
{
    const QString &s = value ? toQStringFromString(value->name()) : noneString();
    return s;
}

}
