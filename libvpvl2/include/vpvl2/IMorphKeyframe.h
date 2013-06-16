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
#ifndef VPVL2_IMORPHKEYFRAME_H_
#define VPVL2_IMORPHKEYFRAME_H_

#include "vpvl2/IKeyframe.h"
#include "vpvl2/IMorph.h"

namespace vpvl2
{

/**
 * モーフのキーフレームをあらわすインターフェースです。
 *
 */
class VPVL2_API IMorphKeyframe : public IKeyframe
{
public:
    virtual ~IMorphKeyframe() {}

    /**
     * IMorphKeyframe のインスタンスの完全なコピーを返します.
     *
     * @return IBoneKeyframe
     */
    virtual IMorphKeyframe *clone() const = 0;

    /**
     * 変形係数を返します.
     *
     * 返す値は 0.0 以上 1.0 以下になります。
     *
     * @return float
     * @sa setWeight
     */
    virtual IMorph::WeightPrecision weight() const = 0;

    /**
     * 変形係数を設定します.
     *
     * 設定する値は 0.0 以上 1.0 以下でなければなりません。
     *
     * @param float
     * @sa weight
     */
    virtual void setWeight(const IMorph::WeightPrecision &value) = 0;
};

} /* namespace vpvl2 */

#endif
