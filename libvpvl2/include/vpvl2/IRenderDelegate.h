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

#ifndef VPVL2_IRENDERDELEGATE_H_
#define VPVL2_IRENDERDELEGATE_H_

#include "vpvl2/Common.h"

namespace vpvl2
{

class IModel;
class IString;

class VPVL2_API IRenderDelegate
{
public:
    enum LogLevel {
        kLogInfo,
        kLogWarning
    };
    enum ShaderType {
        kEdgeVertexShader,
        kEdgeFragmentShader,
        kModelVertexShader,
        kModelFragmentShader,
        kShadowVertexShader,
        kShadowFragmentShader,
        kZPlotVertexShader,
        kZPlotFragmentShader,
        kEdgeWithSkinningVertexShader,
        kModelWithSkinningVertexShader,
        kShadowWithSkinningVertexShader,
        kZPlotWithSkinningVertexShader
    };
    enum KernelType {
        kModelSkinningKernel
    };
    virtual ~IRenderDelegate() {}

    /**
     * モデルのアップロード時のみに有効な局所的なオブジェクトを作成します。
     *
     * 作成したオブジェクトは context に格納する必要があります。
     * 何も格納する必要がない場合は処理をスキップすることが出来ます。
     * 処理中は例外を投げないように処理を行う必要があります。
     *
     * @sa releaseContext
     * @param model
     * @param context
     */
    virtual void allocateContext(const IModel *model, void *&context) = 0;

    /**
     * allocateContext で作成したオブジェクトを破棄します。
     *
     * context にデータが格納されているため、delete 等で破棄してください。
     * 破棄したら可能であれば context を 0 にセットしてください。
     * 処理中は例外を投げないように処理を行う必要があります。
     *
     * @sa allocateContext
     * @param model
     * @param context
     */
    virtual void releaseContext(const IModel *model, void *&context) = 0;

    /**
     * モデルのテクスチャをサーバ (GPU) にアップロードします。
     *
     * アップロードしたテクスチャの識別子を texture に格納してください。
     * OpenGL の場合は GLuint の値をセットします。
     * 処理中は例外を投げないように処理を行う必要があります。
     *
     * @param context
     * @param name
     * @param dir
     * @param texture
     * @param isToon
     * @return bool
     */
    virtual bool uploadTexture(void *context, const IString *name, const IString *dir, void *texture, bool isToon) = 0;

    /**
     * モデルのトゥーンテクスチャをサーバ (GPU) にアップロードします。
     *
     * 基本的な処理戦略は uploadTexture と同じです。
     *
     * @sa uploadTexture
     * @param context
     * @param name
     * @param dir
     * @param texture
     * @return bool
     */
    virtual bool uploadToonTexture(void *context, const char *name, const IString *dir, void *texture) = 0;

    /**
     * モデルのトゥーンテクスチャをサーバ (GPU) にアップロードします。
     *
     * 基本的な処理戦略は uploadTexture と同じです。
     *
     * @sa uploadTexture
     * @param context
     * @param name
     * @param dir
     * @param texture
     * @return bool
     */
    virtual bool uploadToonTexture(void *context, const IString *name, const IString *dir, void *texture) = 0;

    /**
     * モデルのトゥーンテクスチャをサーバ (GPU) にアップロードします。
     *
     * 基本的な実装戦略は uploadTexture と同じですが、
     * システム全体におけるトゥーンテクスチャ番号しか渡されないため、
     * アプリケーションにトゥーンテクスチャをリソースとして含め、
     * それをアップロードする必要があります。
     *
     * @sa uploadTexture
     * @param context
     * @param int
     * @param texture
     * @return bool
     */
    virtual bool uploadToonTexture(void *context, int index, void *texture) = 0;

    /**
     * 指定されたフォーマットと可変引数を用いてロギングを行います。
     *
     * ロギングは任意の出力先に書き出しを行います。この処理は無視することが出来ます。
     * 処理中は例外を投げないように処理を行う必要があります。
     *
     * @param context
     * @param level
     * @param format
     * @param ap
     */
    virtual void log(void *context, LogLevel level, const char *format, va_list ap) = 0;

    /**
     * 指定された形式の (OpenGL の) シェーダのソースを読み込みます。
     *
     * シェーダのソースの読み込みを行います。失敗した場合は返り値として 0 を渡してください。
     * model の type メソッドを用いて読み込むシェーダの切り替えを行います。
     * 処理中は例外を投げないように処理を行う必要があります。
     *
     * @param type
     * @param model
     * @param context
     * @return IString
     */
    virtual IString *loadShaderSource(ShaderType type, const IModel *model, void *context) = 0;

    /**
     * 指定された形式の (OpenCL の) カーネルのソースを読み込みます。
     *
     * カーネルのソースの読み込みを行います。失敗した場合は返り値として 0 を渡してください。
     * 処理中は例外を投げないように処理を行う必要があります。
     *
     * @param type
     * @param context
     * @return IString
     */
    virtual IString *loadKernelSource(KernelType type, void *context) = 0;

    /**
     * 指定された文字列を IString に変換します。
     *
     * 処理中は例外を投げないように処理を行う必要があります。
     *
     * @param str
     * @return IString
     */
    virtual IString *toUnicode(const uint8_t *str) const = 0;
};

} /* namespace vpvl2 */

#endif
