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

#ifndef VPVL2_IMORPH_H_
#define VPVL2_IMORPH_H_

#include "vpvl2/Common.h"

namespace vpvl2
{

class IString;

/**
 * モデルのモーフをあらわすインターフェースです。
 *
 */
class VPVL2_API IMorph
{
public:
    enum Category {
        kReserved,
        kEyeblow,
        kEye,
        kLip,
        kOther
    };

    virtual ~IMorph() {}

    /**
     * モーフの名前を返します。
     *
     * @return IString
     */
    virtual const IString *name() const = 0;

    /**
     * モーフの ID を返します。
     *
     * 常にユニークでなければなりません。
     *
     * @return int
     */
    virtual int index() const = 0;

    /**
     * モーフのカテゴリを返します。
     *
     * @return Category
     */
    virtual Category category() const = 0;

    /**
     * グループモーフに所属しているかを返します。
     *
     * @return bool
     */
    virtual bool hasParent() const = 0;

    /**
     * 係数を返します。
     *
     * @return float
     * @sa setWeight
     */
    virtual const Scalar &weight() const = 0;

    /**
     * 係数 value に基づいて変形を行います。
     *
     * value は 0.0 以上 1.0 以下でなければなりません。
     *
     * @param float
     * @sa weight
     */
    virtual void setWeight(const Scalar &value) = 0;
};

}

#endif
