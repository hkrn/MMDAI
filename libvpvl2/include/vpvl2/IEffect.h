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
#ifndef VPVL2_IEFFECT_H_
#define VPVL2_IEFFECT_H_

#include "vpvl2/Common.h"

namespace vpvl2
{

class IString;
class ITexture;

namespace extensions
{
namespace gl
{
class FrameBufferObject;
}
}

class IEffect
{
public:
    class Annotation;
    class Pass;
    class SamplerState;
    class Parameter {
    public:
        enum Type {
            kUnknownParameterType,
            kBoolean,
            kBool1,
            kBool2,
            kBool3,
            kBool4,
            kInteger,
            kInt1,
            kInt2,
            kInt3,
            kInt4,
            kFloat,
            kFloat1,
            kFloat2,
            kFloat3,
            kFloat4,
            kFloat2x2,
            kFloat3x3,
            kFloat4x4,
            kString,
            kTexture,
            kSampler,
            kSampler1D,
            kSampler2D,
            kSampler3D,
            kSamplerCube,
            kMaxType
        };
        enum VariableType {
            kUnknownVariableType,
            kConstant,
            kUniform,
            kVarying
        };
        virtual ~Parameter() {}
        virtual IEffect *parentEffectRef() const = 0;
        virtual const Annotation *annotationRef(const char *name) const = 0;
        virtual const char *name() const = 0;
        virtual const char *semantic() const = 0;
        virtual Type type() const = 0;
        virtual VariableType variableType() const = 0;
        virtual void connect(Parameter *destinationParameter) = 0;
        virtual void reset() = 0;
        virtual void getValue(int &value) const = 0;
        virtual void getValue(float32 &value) const = 0;
        virtual void getValue(Vector3 &value) const = 0;
        virtual void getValue(Vector4 &value) const = 0;
        virtual void getMatrix(float32 *value) const = 0;
        virtual void getArrayDimension(int &value) const = 0;
        virtual void getArrayTotalSize(int &value) const = 0;
        virtual void getTextureRef(intptr_t &value) const = 0;
        virtual void getSamplerStateRefs(Array<SamplerState *> &value) const = 0;
        virtual void setValue(bool value) = 0;
        virtual void setValue(int value) = 0;
        virtual void setValue(float32 value) = 0;
        virtual void setValue(const Vector3 &value) = 0;
        virtual void setValue(const Vector4 &value) = 0;
        virtual void setValue(const Vector4 *value) = 0;
        virtual void setMatrix(const float32 *value) = 0;
        virtual void setMatrices(const float32 *value, size_t size) = 0;
        virtual void setSampler(const ITexture *value) = 0;
        virtual void setTexture(const ITexture *value) = 0;
        virtual void setTexture(intptr_t value) = 0;
    };
    class Technique {
    public:
        virtual ~Technique() {}
        virtual IEffect *parentEffectRef() const = 0;
        virtual Pass *findPass(const char *name) const = 0;
        virtual const Annotation *annotationRef(const char *name) const = 0;
        virtual const char *name() const = 0;
        virtual void getPasses(Array<Pass *> &passes) const = 0;
        virtual void setOverridePass(Pass *pass) = 0;
    };
    class Pass {
    public:
        virtual ~Pass() {}
        virtual Technique *parentTechniqueRef() const = 0;
        virtual const Annotation *annotationRef(const char *name) const = 0;
        virtual const char *name() const = 0;
        virtual bool isRenderable() const = 0;
        virtual void setState() = 0;
        virtual void resetState() = 0;
        virtual void setupOverrides(const IEffect *effectRef) = 0;
        virtual void setupOverrides(const Array<Pass *> &passes) = 0;
    };
    class SamplerState {
    public:
        virtual ~SamplerState() {}
        virtual const char *name() const = 0;
        virtual Parameter::Type type() const = 0;
        virtual Parameter *parameterRef() const = 0;
        virtual void getValue(int &value) const = 0;
    };
    class Annotation {
    public:
        virtual ~Annotation() {}
        virtual bool booleanValue() const = 0;
        virtual int integerValue() const = 0;
        virtual const int *integerValues(int *size) const = 0;
        virtual float32 floatValue() const = 0;
        virtual const float32 *floatValues(int *size) const = 0;
        virtual const char *stringValue() const = 0;
    };

    enum VertexAttributeType {
        kUnknownVertexAttribute = -1,
        kPositionVertexAttribute,
        kNormalVertexAttribute,
        kTextureCoordVertexAttribute,
        kBoneIndexVertexAttribute,
        kBoneWeightVertexAttribute,
        kUVA1VertexAttribute,
        kUVA2VertexAttribute,
        kUVA3VertexAttribute,
        kUVA4VertexAttribute,
        kMaxVertexAttribute
    };
    enum ScriptOrderType {
        kPreProcess,
        kStandard,
        kStandardOffscreen,
        kPostProcess,
        kAutoDetection,
        kDefault,
        kMaxScriptOrderType
    };
    struct OffscreenRenderTarget {
        ITexture *textureRef;
        IEffect::Parameter *textureParameterRef;
        IEffect::Parameter *samplerParameterRef;
    };
    virtual ~IEffect() {}

    /**
     * CGcontext のインスタンスを返します.
     *
     * void* で返すため、static_cast で CGcontext にキャストする必要があります。
     *
     * @brief internalContext
     * @return
     */
    virtual void *internalContext() const = 0;

    /**
     * CGeffect のインスタンスを返します.
     *
     * void* で返すため、static_cast で CGeffect にキャストする必要があります。
     *
     * @brief internalPointer
     * @return
     */
    virtual void *internalPointer() const = 0;

    /**
     * オフスクリーンターゲットの配列を取得します.
     *
     * @brief getOffscreenRenderTargets
     * @param value
     */
    virtual void getOffscreenRenderTargets(Array<OffscreenRenderTarget> &value) const = 0;

    /**
     * UIWidget セマンティクスがあるパラメータの配列を返します.
     *
     * void* で返すため、個々に static_cast で CGparameter にキャストする必要があります。
     * 現在実装していないため機能していません。
     *
     * @brief getInteractiveParameters
     * @param value
     */
    virtual void getInteractiveParameters(Array<IEffect::Parameter *> &value) const = 0;

    /**
     * 親の IEffect インスタンスのポインタ参照を返します.
     *
     * @brief parentEffect
     * @return
     */
    virtual IEffect *parentEffectRef() const = 0;

    /**
     * 親の IEffect インスタンスのポインタ参照を設定します.
     *
     * @brief parentEffect
     * @param value
     */
    virtual void setParentEffectRef(IEffect *value) = 0;

    /**
     * 親のフレームバッファの参照を返します.
     *
     * エフェクト作成後必ず createFrameBufferObject を呼び出す必要があります。
     * 呼び出されていない場合は NULL を返します。
     *
     * @brief parentFrameBufferObject
     * @return
     */
    virtual extensions::gl::FrameBufferObject *parentFrameBufferObject() const = 0;

    /**
     * フレームバッファを作成します.
     *
     * 別スレッドで処理する際に正しくエフェクトを作成できるようにするため、
     *　フレームバッファの作成は呼び出し側で行う必要があります。
     *
     * @brief createFrameBufferObject
     */
    virtual void createFrameBufferObject() = 0;

    /**
     * スクリプトの描画タイミングを返します.
     *
     * @brief scriptOrderType
     * @return
     */
    virtual ScriptOrderType scriptOrderType() const = 0;

    /**
     * オフスクリーンレンダーターゲットを追加します.
     *
     * @brief addOffscreenRenderTarget
     * @param textureRef
     * @param textureParameterRef
     * @param samplerParameterRef
     */
    virtual void addOffscreenRenderTarget(ITexture *textureRef, Parameter *textureParameterRef, Parameter *samplerParameterRef) = 0;

    /**
     * 接頭子に UI とつくユーザが操作可能なパラメータを追加します.
     *
     * @brief addInteractiveParameter
     * @param value
     */
    virtual void addInteractiveParameter(IEffect::Parameter *value) = 0;

    /**
     * レンダーターゲットを追加します.
     *
     * @brief addRenderColorTargetIndex
     * @param targetIndex
     */
    virtual void addRenderColorTargetIndex(int targetIndex) = 0;

    /**
     * レンダーターゲットを削除します.
     *
     * @brief removeRenderColorTargetIndex
     * @param targetIndex
     */
    virtual void removeRenderColorTargetIndex(int targetIndex) = 0;

    /**
     * 現在設定されているレンダーターゲットを全て削除します.
     *
     * @brief clearRenderColorTargetIndices
     */
    virtual void clearRenderColorTargetIndices() = 0;

    /**
     * スクリプト順序を設定します.
     *
     * @brief setScriptOrderType
     * @param value
     */
    virtual void setScriptOrderType(ScriptOrderType value) = 0;

    /**
     * 指定されたレンダーターゲットの番号が存在するかを返します.
     *
     * @brief hasRenderColorTargetIndex
     * @param targetIndex
     * @return
     */
    virtual bool hasRenderColorTargetIndex(int targetIndex) const = 0;

    /**
     * パラメータ名から OpenGL でいう uniform のパラメータを返します.
     *
     * 存在する場合は非 NULL の IEffect::IParameter のインスタンスを返します.
     *
     * @brief findUniformParameter
     * @param name
     * @return
     */
    virtual IEffect::Parameter *findUniformParameter(const char *name) const = 0;

    /**
     * テクニック名からテクニックのインスタンスを返します.
     *
     * 存在する場合は非 NULL の IEffect::ITechnique のインスタンスを返します.
     *
     * @brief findTechnique
     * @param name
     * @return
     */
    virtual IEffect::Technique *findTechnique(const char *name) const = 0;

    /**
     * エフェクトに設定されている全てのパラメータを引数に設定します.
     *
     * @brief getParameterRefs
     * @param parameters
     */
    virtual void getParameterRefs(Array<Parameter *> &parameters) const = 0;

    /**
     * エフェクトに設定されている全てのテクニックを引数に設定します.
     *
     * @brief getTechniqueRefs
     * @param techniques
     */
    virtual void getTechniqueRefs(Array<Technique *> &techniques) const = 0;

    virtual void setVertexAttributePointer(VertexAttributeType vtype, Parameter::Type ptype, vsize stride, const void *ptr) = 0;

    virtual void activateVertexAttribute(VertexAttributeType vtype) = 0;

    virtual void deactivateVertexAttribute(VertexAttributeType vtype) = 0;

    virtual void setupOverride(const IEffect *effectRef) = 0;

    virtual const IString *name() const = 0;

    virtual void setName(const IString *value) = 0;

    virtual bool isEnabled() const = 0;

    virtual void setEnabled(bool value) = 0;

    virtual const char *errorString() const = 0;
};

} /* namespace vpvl2 */

#endif
