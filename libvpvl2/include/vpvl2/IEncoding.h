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
#ifndef VPVL2_IENCODING_H_
#define VPVL2_IENCODING_H_

#include "vpvl2/Common.h"
#include "vpvl2/IString.h"

namespace vpvl2
{

/**
 * 文字コード変換を担当し、IString のインスタンスを生成するためのインターフェースです。
 *
 * このインターフェースは libvpvl2 側では実装しないため、利用する側で実装する必要があります。
 * IEncoding は以下の文字コードを変換する実装が必要です。
 *
 * - Shift_JIS
 * - UTF-8
 * - UTF-16
 *
 */
class VPVL2_API IEncoding
{
public:
    enum ConstantType {
        kLeft,
        kRight,
        kFinger,
        kElbow,
        kArm,
        kWrist,
        kCenter,
        kAsterisk,
        kSPHExtension,
        kSPAExtension,
        kRightKnee,
        kLeftKnee,
        kRootBone,
        kScaleBoneAsset,
        kOpacityMorphAsset,
        kMaxConstantType
    };

    virtual ~IEncoding() {}

    /**
     * 長さ size を持つ bytes 文字列を codec に基づいて変換し、IString として返します.
     *
     * libvpvl2 側で delete を呼ぶため、スタック上ではなくヒープ上で作成してください。
     *
     * @param value
     * @param size
     * @param codec
     * @return IString
     */
    virtual IString *toString(const uint8_t *value, size_t size, IString::Codec codec) const = 0;

    /**
     * 最大長 maxlen であることが保証されている bytes 文字列を codec に基づいて変換し、IString として返します.
     *
     * 文字列の長さは strnlen などを使って文字列の長さを知る必要があります。
     * libvpvl2 側で delete を呼ぶため、スタック上ではなくヒープ上で作成してください。
     *
     * @param value
     * @param codec
     * @param maxlen
     * @return IString
     */
    virtual IString *toString(const uint8_t *value, IString::Codec codec, size_t maxlen) const = 0;

    /**
     * value を codec に基づいて変換し、バッファとして返します.
     *
     * libvpvl2 側で toByteArray が呼ばれた後領域を使用後必ず disposeByteArray を呼びます。
     *
     * @param value
     * @param codec
     * @return uint8_t
     * @sa disposeByteArray
     */
    virtual uint8_t *toByteArray(const IString *value, IString::Codec codec) const = 0;

    /**
     * value をメモリ上から解放します.
     *
     * value は必ず toByteArray から呼ばれたものが渡されます。
     *
     * @param value
     * @sa toByteArray
     */
    virtual void disposeByteArray(uint8_t *value) const = 0;

    /**
     * 指定された定数から不変の文字列を返します.
     *
     * 返す文字列は IEncoding を継承するクラスがスタックとして保持するか、あるいはヒープ上で管理していることを想定しています。
     * そのため、new してそのまま返さないようにしてください。メモリリークの原因となってしまいます。
     *
     * @return IString
     */
    virtual const IString *stringConstant(ConstantType value) const = 0;
};

} /* namespace vpvl2 */

#endif
