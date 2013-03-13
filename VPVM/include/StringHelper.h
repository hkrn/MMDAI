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

#ifndef VPVM_STRINGHELPER_H
#define VPVM_STRINGHELPER_H

#include <vpvl2/IEncoding.h>
#include <vpvl2/IString.h>

#include <QString>

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

namespace vpvm {

using namespace vpvl2;

QTextCodec *getTextCodec();
const QByteArray toByteArrayFromQString(const QString &value);
const QString toQStringFromBytes(const uint8_t *value);
const QString toQStringFromString(const IString *value);
const QString toQStringFromModel(const IModel *value);
const QString toQStringFromMotion(const IMotion *value);
const QString toQStringFromBone(const IBone *value);
const QString toQStringFromMorph(const IMorph *value);
const QString toQStringFromBoneKeyframe(const IBoneKeyframe *value);
const QString toQStringFromMorphKeyframe(const IMorphKeyframe *value);

} /* namespace vpvm */

#endif
