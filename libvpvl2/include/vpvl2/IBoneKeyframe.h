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

#ifndef VPVL2_IBONEKEYFRAME_H_
#define VPVL2_IBONEKEYFRAME_H_

#include "vpvl2/IKeyframe.h"

namespace vpvl2
{

/**
 * ボーンのキーフレームをあらわすインターフェースです。
 *
 */
class VPVL2_API IBoneKeyframe : public virtual IKeyframe
{
public:
    struct InterpolationParameter
    {
        QuadWord x;
        QuadWord y;
        QuadWord z;
        QuadWord rotation;
    };
    enum InterpolationType
    {
        kX = 0,
        kY,
        kZ,
        kRotation,
        kMax
    };
    virtual ~IBoneKeyframe() {}

    /**
     * IBoneKeyframe のインスタンスの完全なコピーを返します。
     *
     * @return IBoneKeyframe
     */
    virtual IBoneKeyframe *clone() const = 0;

    /**
     * 補間パラメータを初期状態に設定します。
     *
     * @sa setInterpolationParameter
     * @sa getInterpolationParameter
     */
    virtual void setDefaultInterpolationParameter() = 0;

    /**
     * 指定された型の補間パラメータを設定します。
     *
     * 第２引数は以下で解釈されます。第２引数の値はそれぞれ 0 以上かつ 128 未満でなければなりません。
     *
     * - x = x1
     * - y = y1
     * - z = x2
     * - w = y2
     *
     * @param InterpolationType
     * @param QuadWord
     * @sa setDefaultInterpolationParameter
     * @sa getInterpolationParameter
     */
    virtual void setInterpolationParameter(InterpolationType type, const QuadWord &value) = 0;

    /**
     * 指定された型の補間パラメータを第二引数にコピーします。
     *
     * 第２引数にコピーされる値の設定順は setInterpolationParameter と同じです。
     *
     * @param InterpolationType
     * @param QuadWord
     * @sa setDefaultInterpolationParameter
     * @sa setInterpolationParameter
     */
    virtual void getInterpolationParameter(InterpolationType type, QuadWord &value) const = 0;

    /**
     * 移動量を返します。
     *
     * @return Vector3
     * @sa setPosition
     */
    virtual const Vector3 &position() const = 0;

    /**
     * 回転量を返します。
     *
     * @return Quaternion
     * @sa setRotation
     */
    virtual const Quaternion &rotation() const = 0;

    /**
     * 移動量を設定します。
     *
     * @param Vector3
     * @sa position
     */
    virtual void setPosition(const Vector3 &value) = 0;

    /**
     * 回転量を設定します。
     *
     * @param Quaternion
     * @sa rotation
     */
    virtual void setRotation(const Quaternion &value) = 0;
};

}

#endif

