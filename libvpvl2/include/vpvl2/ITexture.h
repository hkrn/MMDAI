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
#ifndef VPVL2_ITEXTURE_H_
#define VPVL2_ITEXTURE_H_

#include "vpvl2/Common.h"

namespace vpvl2
{

/**
 * テクスチャをあらわすインターフェースです。
 *
 */
class VPVL2_API ITexture
{
public:
    virtual ~ITexture() {}

    /**
     * テクスチャの実体を作成します.
     *
     * @brief create
     */
    virtual void create() = 0;

    /**
     * 現在のテクスチャをコンテキストに紐付けします.
     *
     * @brief bind
     */
    virtual void bind() = 0;

    virtual void fillPixels(const void *pixels) = 0;

    virtual void allocate(const void *pixels) = 0;

    virtual void write(const void *pixels) = 0;

    virtual void getParameters(unsigned int key, int *values) const = 0;

    virtual void getParameters(unsigned int key, float *values) const = 0;

    virtual void setParameter(unsigned int key, int value) = 0;

    virtual void setParameter(unsigned int key, float value) = 0;

    virtual void generateMipmaps() = 0;

    /**
     * テクスチャの大きさを引数に基づいて変更します.
     *
     * @brief resize
     * @param size
     */
    virtual void resize(const Vector3 &size) = 0;

    /**
     * 現在のテクスチャをコンテキストから解除します.
     *
     * @brief unbind
     */
    virtual void unbind() = 0;

    /**
     * テクスチャの実体を解放します.
     *
     * @brief release
     */
    virtual void release() = 0;

    /**
     * テクスチャの大きさを返します.
     *
     * @brief size
     * @return
     */
    virtual Vector3 size() const = 0;

    /**
     * テクスチャの実体を返します.
     *
     * 中身は使用する API によって異なり、OpenGL の場合は GLuint になります。
     *
     * @brief data
     * @return
     */
    virtual intptr_t data() const = 0;

    /**
     * サンプラーの実体を返します.
     *
     * 中身は使用する API によって異なり、OpenGL の場合は GLuint になります。
     *
     * @brief sampler
     * @return
     */
    virtual intptr_t sampler() const = 0;

    /**
     * フォーマットの実体を返します.
     *
     * 中身は使用する API によって異なり、OpenGL の場合は vpvl2::extensions::gl::BaseSurface::Format になります。
     *
     * @brief format
     * @return
     */
    virtual intptr_t format() const = 0;
};

} /* namespace vpvl2 */

#endif
