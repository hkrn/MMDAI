/**

 Copyright (c) 2010-2013  hkrn

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
#ifndef VPVL2_ILIGHT_H_
#define VPVL2_ILIGHT_H_

#include "vpvl2/Common.h"

namespace vpvl2
{

class IMotion;

class VPVL2_API ILight
{
public:
    virtual ~ILight() {}

    /**
     * 照明色を返します.
     *
     * @brief color
     * @return
     */
    virtual Vector3 color() const = 0;

    /**
     * 照明方向を返します.
     *
     * @brief direction
     * @return
     */
    virtual Vector3 direction() const = 0;

    /**
     * トゥーンが有効かどうかを返します.
     *
     * @brief isToonEnabled
     * @return
     */
    virtual bool isToonEnabled() const = 0;

    /**
     * 現在設定されている照明のモーションを返します.
     *
     * 設定されている場合は非 NULL の値を、設定されていない場合は NULL を返します。
     *
     * @brief motion
     * @return
     */
    virtual IMotion *motion() const = 0;

    /**
     * 照明色を設定します.
     *
     * @brief setColor
     * @param value
     */
    virtual void setColor(const Vector3 &value) = 0;

    /**
     * 照明方向を設定します.
     *
     * @brief setDirection
     * @param value
     */
    virtual void setDirection(const Vector3 &value) = 0;

    /**
     * 照明のモーションを設定します.
     *
     * @brief setMotion
     * @param value
     */
    virtual void setMotion(IMotion *value) = 0;

    /**
     * トゥーンの有効無効を設定します.
     *
     * @brief setToonEnable
     * @param value
     */
    virtual void setToonEnable(bool value) = 0;

    /**
     * 引数から照明のパラメータをコピーします.
     *
     * コピーされるのは照明色と照明方向とトゥーンの有効無効のみで、
     * モーションはコピーされません。
     *
     * @brief copyFrom
     * @param value
     */
    virtual void copyFrom(const ILight *value) = 0;

    /**
     * 照明のパラメータを初期値に戻します.
     *
     * @brief resetDefault
     */
    virtual void resetDefault() = 0;
};

} /* namespace vpvl2 */

#endif
