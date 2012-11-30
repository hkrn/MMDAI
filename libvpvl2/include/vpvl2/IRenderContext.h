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

#pragma once
#ifndef VPVL2_IRENDERDELEGATE_H_
#define VPVL2_IRENDERDELEGATE_H_

#include "vpvl2/Common.h"

namespace vpvl2
{

class IEffect;
class IModel;
class IString;

namespace extensions
{
namespace gl
{
class FrameBufferObject;
}
}

using namespace extensions::gl;

class VPVL2_API IRenderContext
{
public:
    enum LogLevel {
        kLogInfo,
        kLogWarning,
        kMaxLogLevel
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
        kZPlotWithSkinningVertexShader,
        kModelEffectTechniques,
        kMaxShaderType
    };
    enum KernelType {
        kModelSkinningKernel,
        kMaxKernelType
    };
    enum MousePositionType {
        kMouseCursorPosition,
        kMouseLeftPressPosition,
        kMouseMiddlePressPosition,
        kMouseRightPressPosition,
        kMaxMousePositionType
    };
    enum ProfileType {
        kProfileUploadModelProcess,
        kProfileRenderModelProcess,
        kProfileRenderModelMaterialDrawCall,
        kProfileRenderEdgeProcess,
        kProfileRenderEdgeMateiralDrawCall,
        kProfileRenderShadowProcess,
        kProfileRenderShadowMaterialDrawCall,
        kProfileRenderZPlotProcess,
        kProfileRenderZPlotMaterialDrawCall,
        kProfileUpdateModelProcess,
        kMaxProfileType
    };
    enum MatrixTypeFlags {
        kWorldMatrix        = 0x001,
        kViewMatrix         = 0x002,
        kProjectionMatrix   = 0x004,
        kInverseMatrix      = 0x008,
        kTransposeMatrix    = 0x010,
        kCameraMatrix       = 0x020,
        kLightMatrix        = 0x040,
        kShadowMatrix       = 0x080,
        kMaxMatrixTypeFlags = 0x100
    };
    enum TextureTypeFlags {
        kTexture2D             = 0x1,
        kToonTexture           = 0x2,
        kTexture3D             = 0x4,
        kTextureCube           = 0x8,
        kGenerateTextureMipmap = 0x10,
        kMaxTextureTypeFlags   = 0x20
    };
    struct Texture {
        Texture()
            : async(true),
              width(0),
              height(0),
              object(0)
        {
        }
        ~Texture() {
            async = true;
            width = 0;
            height = 0;
            object = 0;
        }

        bool async;
        int width;
        int height;
        void *object;
    };

    virtual ~IRenderContext() {}

    /**
     * モデルのアップロード時のみに有効な局所的なオブジェクトを作成します.
     *
     * 作成したオブジェクトは userData に格納する必要があります。
     * 何も格納する必要がない場合は処理をスキップすることが出来ます。
     * 処理中は例外を投げないように処理を行う必要があります。
     *
     * @sa releaseContext
     * @param model
     * @param context
     */
    virtual void allocateUserData(const IModel *model, void *&userData) = 0;

    /**
     * allocateContext で作成したオブジェクトを破棄します.
     *
     * userData にデータが格納されているため、delete 等で破棄してください。
     * 破棄したら可能であれば userData を 0 にセットしてください。
     * 処理中は例外を投げないように処理を行う必要があります。
     *
     * @sa allocateContext
     * @param model
     * @param context
     */
    virtual void releaseUserData(const IModel *model, void *&userData) = 0;

    /**
     * モデルのテクスチャをサーバ (GPU) にアップロードします.
     *
     * アップロードしたテクスチャの識別子を texture に格納してください。
     * OpenGL の場合は GLuint の値をセットします。
     * 処理中は例外を投げないように処理を行う必要があります。
     *
     * @param name
     * @param dir
     * @param type
     * @param texture
     * @param context
     * @return bool
     */
    virtual bool uploadTexture(const IString *name, const IString *dir, int flags, Texture &texture, void *context) = 0;

    /**
     * 取得する型に応じた行列を取得します.
     *
     * flags 変数に呼び出し側が取得したい行列の形式をビットの組み合わせで渡します。
     * 実装側は呼び出し側の要求に従って行列の結果を value に格納する必要があります。
     *
     * @param value
     * @param mdoel
     * @param flags
     */
    virtual void getMatrix(float value[16], const IModel *model, int flags) const = 0;

    /**
     * 指定されたフォーマットと可変引数を用いてロギングを行います.
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
     * 指定された形式のエフェクトのソースを読み込みます.
     *
     * シェーダのソースの読み込みを行います。失敗した場合は返り値として 0 を渡してください。
     * model の type メソッドを用いて読み込むシェーダの切り替えを行います。
     * 処理中は例外を投げないように処理を行う必要があります。
     * このメソッドは Cg 専用でオフスクリーンに割り当てられたエフェクトの読み込みに利用します。
     *
     * @param type
     * @param path
     * @return IString
     */
    virtual IString *loadShaderSource(ShaderType type, const IString *path) = 0;

    /**
     * 指定された形式の (OpenGL の) シェーダのソースを読み込みます.
     *
     * シェーダのソースの読み込みを行います。失敗した場合は返り値として 0 を渡してください。
     * model の type メソッドを用いて読み込むシェーダの切り替えを行います。
     * 処理中は例外を投げないように処理を行う必要があります。
     *
     * @param type
     * @param model
     * @param dir
     * @param context
     * @return IString
     */
    virtual IString *loadShaderSource(ShaderType type, const IModel *model, const IString *dir, void *context) = 0;

    /**
     * 指定された形式の (OpenCL の) カーネルのソースを読み込みます.
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
     * 指定された文字列を IString に変換します.
     *
     * 処理中は例外を投げないように処理を行う必要があります。
     *
     * @param str
     * @return IString
     */
    virtual IString *toUnicode(const uint8_t *str) const = 0;

    /**
     * 指定された OpenGL の拡張が存在するかを返します.
     *
     * @brief hasExtension
     * @param name
     * @return
     * @sa findProcedureAddress
     */
    virtual bool hasExtension(const void *namePtr) const = 0;

    /**
     * OpenGL の拡張の関数ポインタを返します.
     *
     * 複数指定された文字列の引数のうち最初に見つかった関数ポインタを返すように実装する必要があります。
     * 引数の終端 (sentinel) に必ず 0 が入ります。
     *
     * @brief findProcedureAddress
     * @param candidatesPtr
     * @return
     * @sa hasExtension
     */
    virtual void *findProcedureAddress(const void **candidatesPtr) const = 0;

    /**
     * プロファイルのセッションを開始します.
     *
     * arg の値は type の値によって変化します。タイマーを使ってベンチマークを行う実装を想定しています。
     *
     * @brief startProfileSession
     * @param type
     * @param arg
     * @sa stopProfileSession
     */
    virtual void startProfileSession(ProfileType type, const void *arg) = 0;

    /**
     * プロファイルのセッションを終了します.
     *
     * arg の値は type の値によって変化します。タイマーを使ってベンチマークを行う実装を想定しています。
     *
     * @brief stopProfileSession
     * @param type
     * @param arg
     * @sa startProfileSession
     */
    virtual void stopProfileSession(ProfileType type, const void *arg) = 0;

#ifdef VPVL2_ENABLE_NVIDIA_CG
    /**
     * トゥーン色を取得します.
     *
     * 実装側はトゥーン色の結果を value 変数に格納する必要があります。
     * このメソッドは Cg 専用で、トゥーンテクスチャのみ uploadTexture に代わって呼び出します。
     *
     * @param name
     * @param dir
     * @param value
     * @param context
     */
    virtual void getToonColor(const IString *name, const IString *dir, Color &value, void *context) = 0;

    /**
     * ビューポートの大きさを取得します.
     *
     * 実装側は value 変数に width を x に、height を y に設定して格納する必要があります。
     * このメソッドは Cg 専用です。
     *
     * @param value
     */
    virtual void getViewport(Vector3 &value) const = 0;

    /**
     * マウス座標を取得します.
     *
     * 実装側は type 変数に従ってマウス座標を x と y に、クリックされた時間を z に、
     * クリックされたかの真偽値を w に格納する必要があります。
     * このメソッドは Cg 専用です。
     *
     * @param value
     * @param type
     */
    virtual void getMousePosition(Vector4 &value, MousePositionType type) const = 0;

    /**
     * フレーム位置あるいは起動開始からの秒数を取得します.
     *
     * 実装側は sync が true ならフレーム位置を秒数単位に変換した値を、false なら
     * 起動開始からの秒数を value 変数に格納する必要があります。
     * このメソッドは Cg 専用です。
     *
     * @param value
     * @param sync
     */
    virtual void getTime(float &value, bool sync) const = 0;

    /**
     * 前回からの描画の秒数を取得します.
     *
     * 実装側は sync が true なら前回の描写の経過秒数を、false なら
     * 起動開始からの秒数を value 変数に格納する必要があります。
     * このメソッドは Cg 専用です。
     *
     * @param value
     * @param sync
     */
    virtual void getElapsed(float &value, bool sync) const = 0;

    /**
     * アニメーションテクスチャを更新します.
     *
     * 実装側は引数にしたがってアニメーションテクスチャを更新する必要があります。
     * texture 変数に作成時のテクスチャの識別子が格納されているため、
     * これを各 3D API のテクスチャのオブジェクトにキャストしてください。
     * このメソッドは Cg 専用です。
     *
     * @param offset
     * @param speed
     * @param seek
     * @param texture
     * @param sync
     */
    virtual void uploadAnimatedTexture(float offset, float speed, float seek, void *texture) = 0;

    /**
     * モデル名からモデルのインスタンスを返します.
     *
     * ポインタ参照を返すため、delete で解放してはいけません。
     * このメソッドは Cg 専用です。
     *
     * @brief findModel
     * @param name
     * @return IModel
     * @sa effectOwner
     */
    virtual IModel *findModel(const IString *name) const = 0;

    /**
     * エフェクトのインスタンスからモデルのインスタンスを返します.
     *
     * ポインタ参照を返すため、delete で解放してはいけません。
     * このメソッドは Cg 専用です。
     *
     * @brief effectOwner
     * @param name
     * @return IModel
     * @sa findModel
     */
    virtual IModel *effectOwner(const IEffect *effect) const = 0;

    /**
     * 描画するレンダーターゲットのカラーバッファの配列を設定します.
     *
     * void* で渡されるため、static_cast を用いて GLuint の配列にキャストする必要があります。
     * OpenGL の glDrawBuffers に渡すことを想定して作られています。このメソッドは Cg 専用です。
     *
     * @brief setRenderColorTargets
     * @param targets
     * @param ntargets
     */
    virtual void setRenderColorTargets(const void *targets, const int ntargets) = 0;

    /**
     * エフェクト毎に使われるレンダーターゲットに対するフレームバッファを作成します.
     *
     * このメソッドは Cg 専用です。
     *
     * @brief createFrameBufferObject
     * @return
     */
    virtual FrameBufferObject *createFrameBufferObject() = 0;

#endif /* VPVL2_ENABLE_NVIDIA_CG */
};

} /* namespace vpvl2 */

#endif
