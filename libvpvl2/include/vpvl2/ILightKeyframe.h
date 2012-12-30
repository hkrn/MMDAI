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

#pragma once
#ifndef VPVL2_ILIGHTKEYFRAME_H_
#define VPVL2_ILIGHTKEYFRAME_H_

#include "vpvl2/IKeyframe.h"

namespace vpvl2
{

/**
 * 照明のキーフレームをあらわすインターフェースです。
 *
 */
class VPVL2_API ILightKeyframe : public virtual IKeyframe
{
public:
    virtual ~ILightKeyframe() {}

    /**
     * ILightKeyframe のインスタンスの完全なコピーを返します.
     *
     * @return ILightKeyframe
     */
    virtual ILightKeyframe *clone() const = 0;

    /**
     * 照明の色を返します.
     *
     * @return Vector3
     * @sa setColor
     */
    virtual Vector3 color() const = 0;

    /**
     * 照明の方向を返します.
     *
     * @return Vector3
     * @sa setDirection
     */
    virtual Vector3 direction() const = 0;

    /**
     * 照明の色を設定します.
     *
     * それぞれの値は 0.0 以上 1.0 以下でなければなりません。
     *
     * @param Vector3
     * @sa setColor
     */
    virtual void setColor(const Vector3 &value) = 0;

    /**
     * 照明の方向を設定します.
     *
     * それぞれの値は -1.0 以上 1.0 以下でなければなりません。
     *
     * @param Vector3
     * @sa setDirection
     */
    virtual void setDirection(const Vector3 &value) = 0;
};

} /* namespace vpvl2 */

#endif
