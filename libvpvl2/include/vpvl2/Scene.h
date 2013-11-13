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
#ifndef VPVL2_SCENE_H_
#define VPVL2_SCENE_H_

#include "vpvl2/Common.h"
#include "vpvl2/IKeyframe.h"

class btDiscreteDynamicsWorld;

namespace vpvl2
{

class IBone;
class ICamera;
class IEffect;
class IEncoding;
class ILight;
class IModel;
class IMotion;
class IApplicationContext;
class IRenderEngine;
class IShadowMap;

class VPVL2_API Scene
{
public:
    enum AccelerationType {
        kSoftwareFallback,
        kOpenCLAccelerationType1,
        kVertexShaderAccelerationType1,
        kOpenCLAccelerationType2,
        kMaxAccelerationType
    };
    enum RenderEngineTypeFlags {
        kEffectCapable            = 0x1,
        kMaxRenderEngineTypeFlags = 0x2
    };
    enum UpdateTypeFlags {
        kUpdateModels         = 0x1,
        kUpdateRenderEngines  = 0x2,
        kUpdateCamera         = 0x4,
        kUpdateLight          = 0x8,
        kUpdateAll            = kUpdateModels | kUpdateRenderEngines | kUpdateCamera | kUpdateLight,
        kResetMotionState     = 0x10,
        kForceUpdateAllMorphs = 0x20,
        kMaxUpdateTypeFlags   = 0x40
    };
    struct Deleter {
        void operator()(IModel *model) const {
            Scene::deleteModelUnlessReferred(model);
        }
        void operator()(IMotion *motion) const {
            Scene::deleteMotionUnlessReferred(motion);
        }
        void operator()(IRenderEngine *engine) const {
            Scene::deleteRenderEngineUnlessReferred(engine);
        }
    };

    /**
     * Scene の初期化を行います.
     *
     * GLEW が有効な場合は内部で glewInit を呼び出して GLEW の初期化を行います。もし失敗した場合は opaque を通じて
     * glewInit が返したエラー番号を設定します。エラーを無視する場合は opaque を 0 に設定して渡してください。
     *
     * GLEW が無効の場合は何もしません。
     *
     * @brief initialize
     * @param opaque
     * @return
     */
    static bool initialize(void *opaque);

    /**
     * Scene が初期化済みかどうかを返します.
     *
     * GLEW が有効な状態は一度でも Scene#initialize が呼ばれた後にこの関数を呼び出した場合は true を、
     * 呼ばれる前は false を返します。GLEW が無効な状態の場合は常に true を返します。
     *
     * @brief isInitialized
     * @return
     */
    static bool isInitialized() VPVL2_DECL_NOEXCEPT;

    static void terminate();

    /**
     * Scene の OpenGL における初期状態を設定します.
     *
     * Scene#initialize が内部的に呼び出すため、通常この関数を呼び出す必要はありません。
     *
     * @brief setRequiredOpenGLState
     */
    static void setRequiredOpenGLState() VPVL2_DECL_NOEXCEPT;

    /**
     * OS 依存の OpenGL のコンテキストの値を返します.
     *
     * @brief opaqueCurrentPlatformOpenGLContext
     * @return
     */
    static void *opaqueCurrentPlatformOpenGLContext() VPVL2_DECL_NOEXCEPT;

    /**
     * OS 依存の OpenGL のデバイスの値を返します.
     *
     * @brief opaqueCurrentPlatformOpenGLDevice
     * @return
     */
    static void *opaqueCurrentPlatformOpenGLDevice() VPVL2_DECL_NOEXCEPT;

    /**
     * アクセラレータが有効かを返します.
     *
     * OpenCL つきでビルドした場合は常に true を返します。それ以外は常に false を返します。
     *
     * @brief isAcceleratorSupported
     * @return
     */
    static bool isAcceleratorSupported() VPVL2_DECL_NOEXCEPT;

    /**
     * 標準の FPS (Frames Per Second) を返します.
     *
     * MMD にあわせて常に 30 が返されます.
     *
     * @brief defaultFPS
     * @return
     */
    static Scalar defaultFPS() VPVL2_DECL_NOEXCEPT;

    /**
     * モデルから親の Scene 参照がなくなった場合はモデルを削除します.
     *
     * この関数はスマートポインタの初期化時に渡して使われることを想定しています。
     *
     * @brief deleteModelUnlessReferred
     * @param model
     */
    static void deleteModelUnlessReferred(IModel *model);

    /**
     * モーションから親の Scene 参照がなくなった場合はモーションを削除します.
     *
     * この関数はスマートポインタの初期化時に渡して使われることを想定しています。
     *
     * @brief deleteMotionUnlessReferred
     * @param motion
     */
    static void deleteMotionUnlessReferred(IMotion *motion);

    /**
     * レンダリングエンジンから親の Model 参照がなくなった場合はレンダリングエンジンを削除します.
     *
     * この関数はスマートポインタの初期化時に渡して使われることを想定しています。
     *
     * @brief deleteRenderEngineUnlessReferred
     * @param engine
     */
    static void deleteRenderEngineUnlessReferred(IRenderEngine *engine);

    explicit Scene(bool ownMemory);
    virtual ~Scene();

    /**
     * レンダリングエンジンのインスタンスを作成します.
     *
     * IRenderEngine インターフェースとして返されますが、flags 引数とモデルの型によって作成される実体は異なります。
     * エフェクト効果が必要な場合は flags 引数に Scene::kEffectCapable を渡してください。
     *
     * 作成されたレンダリングエンジンは Scene の参照に追加されないため、別途 addModel で Scene の参照に
     * 追加する必要があります。
     *
     * @brief createRenderEngine
     * @param applicationContextRef
     * @param model
     * @param flags
     * @return
     */
    IRenderEngine *createRenderEngine(IApplicationContext *applicationContextRef, IModel *model, int flags);

    /**
     * モデルとレンダリングエンジンの参照を追加します.
     *
     * モデルの参照とレンダリングエンジンの参照両方必要です。どちらかひとつでも NULL の場合は追加されず、何も行われません。
     *
     * @brief addModel
     * @param model
     * @param engine
     */
    void addModel(IModel *model, IRenderEngine *engine, int priority);

    /**
     * モーションの参照を追加します.
     *
     * 引数が NULL の場合は何もしません。
     *
     * @brief addMotion
     * @param motion
     */
    void addMotion(IMotion *motion);

    /**
     * カメラのインスタンスを作成します.
     *
     * @brief createCamera
     * @return
     */
    ICamera *createCamera();

    /**
     * 照明（一方向光源）のインスタンスを作成します.
     *
     * @brief createLight
     * @return
     */
    ILight *createLight();

    /**
     * エフェクトのソースの文字列からエフェクトのインスタンスを作成します.
     *
     * エフェクトの作成は時間がかかるので、別途呼び出し用のスレッドを作成して実行してください。
     *
     * VPVL2_ENABLE_NVIDIA_CG を無効にしてビルドした場合は何もしません。
     *
     * @brief createEffectFromSource
     * @param source
     * @param applicationContextRef
     * @return
     */
    IEffect *createEffectFromSource(const IString *source, IApplicationContext *applicationContextRef);

    /**
     * エフェクトのソースファイルからエフェクトのインスタンスを作成します.
     *
     * エフェクトの作成は時間がかかるので、別途呼び出し用のスレッドを作成して実行してください。
     *
     * VPVL2_ENABLE_NVIDIA_CG を無効にしてビルドした場合は何もしません。
     *
     * @brief createEffectFromFile
     * @param path
     * @param applicationContextRef
     * @return
     */
    IEffect *createEffectFromFile(const IString *path, IApplicationContext *applicationContextRef);

    /**
     * モデルにあるエフェクトからエフェクトのインスタンスを作成します.
     *
     * エフェクトの作成は時間がかかるので、別途呼び出し用のスレッドを作成して実行してください。
     *
     * VPVL2_ENABLE_NVIDIA_CG を無効にしてビルドした場合は何もしません。
     *
     * @brief createEffectFromModel
     * @param model
     * @param dir
     * @param applicationContextRef
     * @return
     */
    IEffect *createEffectFromModel(const IModel *model, const IString *dir, IApplicationContext *applicationContextRef);

    /**
     * フォールバックとして使われるエフェクトを作成します.
     *
     * 初回時の作成は時間がかかりますが、２回目以降は Scene にキャッシュした結果を返します。
     * そのため、返されるエフェクトは Scene で参照を持っているため、delete で削除してはいけません。
     *
     * VPVL2_ENABLE_NVIDIA_CG を無効にしてビルドした場合は何もしません。
     *
     * @brief createDefaultStandardEffectRef
     * @param applicationContextRef
     * @return
     */
    IEffect *createDefaultStandardEffectRef(IApplicationContext *applicationContextRef);

    /**
     * モデルと紐付けられたレンダリングエンジンから Scene の参照を解除します.
     *
     * 引数が NULL の場合は何もしません。
     *
     * @brief removeModel
     * @param model
     */
    void removeModel(IModel *model);

    /**
     * モデルと紐付けられたレンダリングエンジンから Scene の参照を解除したうえで削除します.
     *
     * コンストラクタの引数で ownMemory を true にした場合は実体を削除します。
     * 引数が NULL の場合は何もしません。
     *
     * @brief deleteModel
     * @param model
     */
    void deleteModel(IModel *&model);

    /**
     * モーションから Scene の参照を解除します.
     *
     * 引数が NULL の場合は何もしません。
     *
     * @brief removeMotion
     * @param motion
     */
    void removeMotion(IMotion *motion);

    /**
     * モーションから Scene の参照を解除したうえで削除します.
     *
     * コンストラクタの引数で ownMemory を true にした場合は実体を削除します。
     * 引数が NULL の場合は何もしません。
     *
     * @brief deleteMotion
     * @param motion
     */
    void deleteMotion(IMotion *&motion);

    /**
     * Scene にある全てのモーションを delta 分進めます.
     *
     * flags 引数によって適用されるモーションが異なります。
     *
     * :kUpdateModels|モデルのモーション全て
     * :kUpdateCamera|カメラモーション
     * :kUpdateLight|照明のモーション
     * :kUpdateAll|上記すべて
     *
     * @brief advance
     * @param delta
     * @param flags
     */
    void advance(const IKeyframe::TimeIndex &delta, int flags);

    /**
     * Scene にある全てのモーションを timeIndex の箇所に移動します.
     *
     * flags 引数によって適用されるモーションが異なります。
     *
     * :kUpdateModels|モデルのモーション全て
     * :kUpdateCamera|カメラモーション
     * :kUpdateLight|照明のモーション
     * :kUpdateAll|上記すべて
     *
     * @brief seek
     * @param timeIndex
     * @param flags
     */
    void seek(const IKeyframe::TimeIndex &timeIndex, int flags);

    /**
     * モデルとそのレンダリングエンジンの状態を更新します.
     *
     * 引数が NULL の場合は何もしません。
     *
     * @brief updateModel
     * @param model
     */
    void updateModel(IModel *model) const;

    /**
     * Scene にある全てのモデルまたはカメラの状態を更新します.
     *
     * flags 引数によって適用されるモーションが異なります。
     *
     * :kUpdateCamera|カメラモーション
     * :kUpdateRenderEngine|レンダリングエンジン
     * :kUpdateAll|上記すべて
     *
     * @brief update
     * @param flags
     */
    void update(int flags);

    /**
     * レンダリングエンジンをエフェクトのプロセス毎に分けて取得します.
     *
     * @brief getRenderEnginesByRenderOrder
     * @param enginesForPreProcess
     * @param enginesForStandard
     * @param enginesForPostProcess
     * @param nextPostEffects
     */
    void getRenderEnginesByRenderOrder(Array<IRenderEngine *> &enginesForPreProcess,
                                       Array<IRenderEngine *> &enginesForStandard,
                                       Array<IRenderEngine *> &enginesForPostProcess,
                                       Hash<HashPtr, IEffect *> &nextPostEffects) const;

    /**
     * Scene インスタンスを再初期化します.
     *
     * @brief reset
     */
    void reset();

    /**
     * Scene の FPS を設定します.
     *
     * @brief setPreferredFPS
     * @param value
     */
    void setPreferredFPS(const Scalar &value) VPVL2_DECL_NOEXCEPT;

    /**
     * Scene にある全てのモーションが timeIndex まで進められているかを返します.
     *
     * @brief isReachedTo
     * @param timeIndex
     * @return
     */
    bool isReachedTo(const IKeyframe::TimeIndex &timeIndex) const VPVL2_DECL_NOEXCEPT;

    /**
     * Scene にあるモーションの継続時間（全てのモーションが完了するまでの timeIndex）を返します.
     *
     * @brief duration
     * @return
     */
    IKeyframe::TimeIndex duration() const VPVL2_DECL_NOEXCEPT;

    /**
     * Scene が持つ全てのモデルの参照を返します.
     *
     * 返されたモデルの配列は Scene がメモリ管理を行なっているため、delete で解放してはいけません。
     *
     * @brief models
     * @return
     */
    void getModelRefs(Array<IModel *> &value) const;

    /**
     * Scene が持つ全てのモーションの参照を返します.
     *
     * 返されたモーションの配列は Scene がメモリ管理を行なっているため、delete で解放してはいけません。
     *
     * @brief models
     * @return
     */
    void getMotionRefs(Array<IMotion *> &value) const;

    /**
     * Scene が持つ全てのレンダリングエンジンの参照を返します.
     *
     * 返されたレンダリングエンジンの配列は Scene がメモリ管理を行なっているため、delete で解放してはいけません。
     *
     * @brief models
     * @return
     */
    void getRenderEngineRefs(Array<IRenderEngine *> &value) const;

    /**
     * モデル名からモデルの参照を返します.
     *
     * 見つかった場合は該当するモデルの参照を返し、見つからなかった場合は NULL を返します。
     * 返されたモデルの参照は Scene がメモリ管理を行なっているため、delete で解放してはいけません。
     *
     * @brief findModel
     * @param name
     * @return
     */
    IModel *findModel(const IString *name) const VPVL2_DECL_NOEXCEPT;

    /**
     * モデルの参照からレンダリングエンジンの参照を返します.
     *
     * 見つかった場合は該当するレンダリングエンジンの参照を返し、見つからなかった場合は NULL を返します。
     * 返されたレンダリングエンジンの参照は Scene がメモリ管理を行なっているため、delete で解放してはいけません。
     *
     * @brief findModel
     * @param name
     * @return
     */
    IRenderEngine *findRenderEngine(const IModel *model) const VPVL2_DECL_NOEXCEPT;

    /**
     * モデルの配列を描画順に従ってソートします.
     *
     * @brief sort
     */
    void sort();

    /**
     * 現在のカメラ及び照明含む全てのモーションにおける時間位置を返します.
     *
     * @brief currentTimeIndex
     * @return
     */
    IKeyframe::TimeIndex currentTimeIndex() const VPVL2_DECL_NOEXCEPT;

    /**
     * Scene が管理する照明のインスタンスの参照を返します.
     *
     * 返された照明のインスタンスの参照は Scene が管理しているため、delete で解放してはいけません。
     *
     * @brief light
     * @return
     */
    ILight *lightRef() const VPVL2_DECL_NOEXCEPT;

    /**
     * Scene が管理するカメラのインスタンスの参照を返します.
     *
     * 返されたカメラのインスタンスの参照は Scene が管理しているため、delete で解放してはいけません。
     *
     * @brief camera
     * @return
     */
    ICamera *cameraRef() const VPVL2_DECL_NOEXCEPT;

    /**
     * IShadowMap のインスタンスの参照を返します.
     *
     * Scene がメモリ管理する ICamera/ILight と異なり、IShadowMap は Scene でメモリ管理しません。
     * そのため、 IShadowMap のインスタンスのメモリ管理は呼び出し側で行う必要があります。
     *
     * @brief shadowMapRef
     * @return
     */
    IShadowMap *shadowMapRef() const VPVL2_DECL_NOEXCEPT;

    /**
     * IShadowMap のインスタンスの参照を設定します.
     *
     * @brief setShadowMapRef
     * @param value
     */
    void setShadowMapRef(IShadowMap *value) VPVL2_DECL_NOEXCEPT;

    /**
     * Scene の FPS を返します.
     *
     * @brief preferredFPS
     * @return
     */
    Scalar preferredFPS() const VPVL2_DECL_NOEXCEPT;

    /**
     * アクセレーションの型を返します.
     *
     * @brief accelerationType
     * @return
     */
    AccelerationType accelerationType() const VPVL2_DECL_NOEXCEPT;

    /**
     * アクセレーションの型を設定します.
     *
     * アクセレーションの設定は設定後の createRenderEngine で有効になるため、すでに createRenderEngine で
     * 作成されたレンダリングエンジンについては設定前の状態になります。
     *
     * @brief setAccelerationType
     * @param value
     */
    void setAccelerationType(AccelerationType value) VPVL2_DECL_NOEXCEPT;

    /**
     * 物理世界のインスタンスの参照を設定します.
     *
     * 物理世界が新しく設定された場合は全てのモデルにその物理世界に対して追加されます。
     * すでに物理世界が設定されていて新たに設定される場合は全てのモデルから前の物理世界の参照を削除し、
     * 新たに設定される物理世界の参照に設定するように変更されます。
     *
     * @brief setWorldRef
     * @param worldRef
     */
    void setWorldRef(btDiscreteDynamicsWorld *worldRef) VPVL2_DECL_NOEXCEPT;

private:
    VPVL2_DISABLE_COPY_AND_ASSIGN(Scene)
    struct PrivateContext;
    PrivateContext *m_context;
};

} /* namespace vpvl2 */

#endif
