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

#ifndef VPVL2_ICAMERAKEYFRAME_H_
#define VPVL2_ICAMERAKEYFRAME_H_

#include "vpvl2/IKeyframe.h"

namespace vpvl2
{

/**
 * カメラのキーフレームをあらわすインターフェースです。
 *
 */
class VPVL2_API ICameraKeyframe : public virtual IKeyframe
{
public:
    struct InterpolationParameter
    {
        QuadWord x;
        QuadWord y;
        QuadWord z;
        QuadWord rotation;
        QuadWord distance;
        QuadWord fovy;
    };
    enum InterpolationType
    {
        kX = 0,
        kY,
        kZ,
        kRotation,
        kDistance,
        kFovy,
        kMax
    };
    virtual ~ICameraKeyframe() {}

    /**
     * ICameraKeyframe のインスタンスの完全なコピーを返します。
     *
     * @return ICameraKeyframe
     */
    virtual ICameraKeyframe *clone() const = 0;

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
     * - x = x1
     * - y = y1
     * - z = x2
     * - w = y2
     *
     * @param InterpolationType
     * @param QuadWord
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
     * @sa setInterpolationParameter
     */
    virtual void getInterpolationParameter(InterpolationType type, QuadWord &value) const = 0;

    /**
     * カメラの注視点を返します。
     *
     * @return Vector3
     * @sa setPosition
     */
    virtual const Vector3 &position() const = 0;

    /**
     * カメラのアングル（オイラー角）を返します。
     *
     * @return Vector3
     * @sa setAngle
     */
    virtual const Vector3 &angle() const = 0;

    /**
     * カメラの視野距離を返します。
     *
     * @return float
     * @sa setDistance
     */
    virtual float distance() const = 0;

    /**
     * カメラの視野角を返します。
     *
     * @return float
     * @sa setFovy
     */
    virtual float fovy() const = 0;

    /**
     * カメラが透視であるかを返します。
     *
     * @return bool
     * @sa setPerspective
     */
    virtual bool isPerspective() const = 0;

    /**
     * カメラの注視点を設定します。
     *
     * @param Vector3
     * @sa position
     */
    virtual void setPosition(const Vector3 &value) = 0;

    /**
     * カメラのアングル（オイラー角）を設定します。
     *
     * @param Vector3
     * @sa angle
     */
    virtual void setAngle(const Vector3 &value) = 0;

    /**
     * カメラの視野距離を設定します。
     *
     * @param Vector3
     * @sa distance
     */
    virtual void setDistance(float value) = 0;

    /**
     * カメラの視野角を設定します。
     *
     * @param Vector3
     * @sa fovy
     */
    virtual void setFovy(float value) = 0;

    /**
     * 透視にするかを設定します。
     *
     * @param bool
     * @sa isPerspective
     */
    virtual void setPerspective(bool value) = 0;
};

}

#endif

