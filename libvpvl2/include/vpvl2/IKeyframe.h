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

#ifndef VPVL2_IKEYFRAME_H_
#define VPVL2_IKEYFRAME_H_

#include "vpvl2/Common.h"

namespace vpvl2
{

class IString;

/**
 * 全ての種類のキーフレームが実装するインターフェースです。
 *
 */
class VPVL2_API IKeyframe
{
public:
    enum Type {
        kBone,
        kCamera,
        kLight,
        kMorph
    };
    virtual ~IKeyframe() {}

    /**
     * オンメモリ上のデータからキーフレームを構築します。
     *
     * データは estimateSize が返す値以上の領域を確保しなければなりません。
     *
     * @param data
     * @sa write
     * @sa estimateSize
     */
    virtual void read(const uint8_t *data) = 0;

    /**
     * キーフレームをオンメモリ上にのデータに書き出します。
     *
     * データは estimateSize が返す値以上の領域を確保しなければなりません。
     * 呼び出し後 data にはキーフレームのデータが書き込まれます。
     *
     * @param data
     * @sa read
     * @sa estimateSize
     */
    virtual void write(uint8_t *data) const = 0;

    /**
     * read/write で必要なデータ領域の大きさを返します。
     *
     * @sa read
     * @sa write
     * @return size_t
     */
    virtual size_t estimateSize() const = 0;

    /**
     * キーフレームの動作対象となる名前を返します。
     *
     * これはモデルに依存するボーンと表情のキーフレームで意味を持ちます。
     * モデルに依存しないカメラと照明のキーフレームでは null を返します。
     *
     * @return IString
     * @sa setName
     */
    virtual const IString *name() const = 0;

    /**
     * キーフレームのフレーム番号を返します。
     *
     * 返す値の型は float ですが、小数点はつきません。
     *
     * @return float
     * @sa setFrameIndex
     */
    virtual float frameIndex() const = 0;

    /**
     * キーフレームの動作対象となる名前を設定します。
     *
     * ボーンと表情のキーフレームでのみ機能します。
     * カメラと照明のキーフレームでは何もしません。
     *
     * @param IString
     * @sa name
     */
    virtual void setName(const IString *value) = 0;

    /**
     * キーフレームのフレーム番号を設定します。
     *
     * @param float
     * @sa frameIndex
     */
    virtual void setFrameIndex(float value) = 0;

    /**
     * キーフレームの型を返します。
     *
     * @return Type
     */
    virtual Type type() const = 0;
};

}

#endif

