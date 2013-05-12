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
#ifndef VPVL2_ILABEL_H_
#define VPVL2_ILABEL_H_

#include "vpvl2/Common.h"

namespace vpvl2
{

class IBone;
class IModel;
class IMorph;
class IString;

/**
 * モデルのラベル（表示枠）をあらわすインターフェースです。
 *
 */
class VPVL2_API ILabel
{
public:
    virtual ~ILabel() {}

    /**
     * ラベルの名前を返します.
     *
     * @return IString
     */
    virtual const IString *name() const = 0;

    /**
     * ラベルの英名を返します.
     *
     * @return IString
     */
    virtual const IString *englishName() const = 0;

    /**
     * 親のモデルのインスタンスを返します.
     *
     * @brief parentModelRef
     * @return IModel
     */
    virtual IModel *parentModelRef() const = 0;

    /**
     * 特別枠かどうかを返します.
     *
     * @brief isSpecial
     * @return
     */
    virtual bool isSpecial() const = 0;

    /**
     * ラベルに含まれる枠数を返します.
     *
     * @brief count
     * @return
     */
    virtual int count() const = 0;

    /**
     * インデックスに対応するボーンのインスタンスを返します.
     *
     * インデックスに対応するボーンが見つかった場合は 0 以外の IBone のインスタンスを返します。
     *　見つからない場合は 0 を返します。ポインタの参照を返すため、delete を行なってはいけません。
     *
     * @brief bone
     * @param index
     * @return IBone
     */
    virtual IBone *bone(int index) const = 0;

    /**
     * インデックスに対応するモーフのインスタンスを返します.
     *
     * インデックスに対応するモーフが見つかった場合は 0 以外の IMorph のインスタンスを返します。
     *　見つからない場合は 0 を返します。ポインタの参照を返すため、delete を行なってはいけません。
     *
     * @brief morph
     * @param index
     * @return IMorph
     */
    virtual IMorph *morph(int index) const = 0;

    /**
     * ラベルの ID を返します.
     *
     * ラベル毎にそれぞれ独立し、かつ重複しない値を返します。
     *
     * @brief index
     * @return
     */
    virtual int index() const = 0;
};

} /* namespace vpvl2 */

#endif

