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

#ifndef VPVL2_IBONE_H_
#define VPVL2_IBONE_H_

#include "vpvl2/Common.h"

namespace vpvl2
{

class IString;

/**
 * モデルのボーンをあらわすインターフェースです。
 *
 */
class VPVL2_API IBone
{
public:
    virtual ~IBone() {}

    /**
     * ボーン名を返します。
     *
     * @return IString
     */
    virtual const IString *name() const = 0;

    /**
     * ボーンの ID を返します。
     *
     * 常にユニークでなければなりません。
     *
     * @return int
     */
    virtual int index() const = 0;

    /**
     * ローカル行列を返します。
     *
     * これはスキニング処理で使われます。
     * ボーンの位置を求めるにはこれが返す値の Transform::getOrigin によって求めることが出来ます。
     *
     * @return Transform
     */
    virtual const Transform &localTransform() const = 0;

    /**
     * 現在のボーンの移動量を返します。
     *
     * 初期状態は vpvl2::kZeroV3 と同等です。
     *
     * @return Vector3
     */
    virtual const Vector3 &position() const = 0;

    /**
     * 現在のボーンの回転量を返します。
     *
     * 初期状態は Quaternion::getIdentity() と同等です。
     *
     * @return Quaternion
     */
    virtual const Quaternion &rotation() const = 0;

    /**
     * ボーンの移動量を設定します。
     *
     * @param Vector3
     */
    virtual void setPosition(const Vector3 &value) = 0;

    /**
     * ボーンの回転量を設定します。
     *
     * @param Quaternion
     */
    virtual void setRotation(const Quaternion &value) = 0;
};

}

#endif

