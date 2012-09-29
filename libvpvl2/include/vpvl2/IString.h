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

#ifndef VPVL2_ISTRING_H_
#define VPVL2_ISTRING_H_

#include "vpvl2/Common.h"

namespace vpvl2
{

/**
 * 文字列をあらわすインターフェースです。
 *
 * このインターフェースは libvpvl2 側では実装しないため、利用する側で実装する必要があります。
 *
 */
class VPVL2_API IString {
public:
    enum Codec {
        kShiftJIS,
        kUTF8,
        kUTF16,
        kMaxCodecType
    };
    virtual ~IString() {}

    /**
     * 文字列が始端と一致するかどうかを返します。
     *
     * @param IString
     * @return bool
     */
    virtual bool startsWith(const IString *value) const = 0;

    /**
     * 文字列が含まれているかどうかを返します。
     *
     * @param IString
     * @return bool
     */
    virtual bool contains(const IString *value) const = 0;

    /**
     * 文字列が終端と一致するかどうかを返します。
     *
     * @param IString
     * @return bool
     */
    virtual bool endsWith(const IString *value) const = 0;

    /**
     * 文字列を separator にもとづいて最大 maxTokens 分に分割します。
     *
     * maxTokens が 1 以上の場合は maxTokens 分まで分割し、それ以上の場合は例え separator があっても分割しません。
     * maxTokens が 0 の場合は中身の文字列をコピーして tokens に 1 つのみ含まれるようにします。
     * maxTokens が -1 以下の場合は制限なく全て分割します。
     * tokens は使用者側が解放します。
     *
     * @param IString
     * @param maxTokens
     * @param Array<IString *>
     */
    virtual void split(const IString *separator, int maxTokens, Array<IString *> &tokens) const = 0;

    /**
     * IString の完全なコピーを返します。
     *
     * @return IString
     */
    virtual IString *clone() const = 0;

    /**
     * IString のハッシュ値 (HashString) を返します。
     *
     * @return HashString
     */
    virtual const HashString toHashString() const = 0;

    /**
     * IString のインスタンスが value と同じであるかを比較します。
     *
     * 等しい場合は true を、等しくない場合は false を返します。
     *
     * @return bool
     */
    virtual bool equals(const IString *value) const = 0;

    /**
     * 文字列の文字単位の長さを返します。
     *
     * @return size_t
     */
    virtual size_t size() const = 0;

    /**
     * 文字列のバイト単位の長さを返します。
     *
     * @return size_t
     */
    virtual size_t length(IString::Codec codec) const = 0;

    /**
     * 文字列のバイト文字列を返します。
     *
     * これが返すデータは解放されないため、メモリ上に確保して返してはいけません。
     * メモリリークの原因になってしまいます。
     *
     * @return uint8_t
     */
    virtual const uint8_t *toByteArray() const = 0;
};

}

#endif

