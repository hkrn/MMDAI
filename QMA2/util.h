/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2011  hkrn                                    */
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

#ifndef UTIL_H
#define UTIL_H

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>

namespace internal
{

static inline QTextCodec *getTextCodec() {
    static QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
    return codec;
}

static inline const QString &noneString()
{
    static const QString none = QApplication::tr("(none)");
    return none;
}

static inline const QByteArray fromQString(const QString &value) {
    return getTextCodec()->fromUnicode(value);
}

static inline const QString toQString(const uint8_t *value) {
    return getTextCodec()->toUnicode(reinterpret_cast<const char *>(value));
}

static inline const QString toQString(const vpvl::Asset *value) {
    return value ? toQString(reinterpret_cast<const uint8_t *>(value->name())) : noneString();
}

static inline const QString toQString(const vpvl::PMDModel *value) {
    return value ? toQString(value->name()) : noneString();
}

static inline const QString toQString(const vpvl::Bone *value) {
    return value ? toQString(value->name()) : noneString();
}

static inline const QString toQString(const vpvl::Face *value) {
    return value ? toQString(value->name()) : noneString();
}

static inline const QString toQString(const vpvl::BoneKeyFrame *value) {
    return value ? toQString(value->name()) : noneString();
}

static inline const QString toQString(const vpvl::FaceKeyFrame *value) {
    return value ? toQString(value->name()) : noneString();
}

}

#endif // UTIL_H
