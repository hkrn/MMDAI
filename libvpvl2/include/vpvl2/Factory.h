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

#ifndef VPVL2_FACTORY_H_
#define VPVL2_FACTORY_H_

#include "vpvl2/Common.h"
#include "vpvl2/IModel.h"
#include "vpvl2/IMotion.h"

namespace vpvl2
{

class IBoneKeyframe;
class ICameraKeyframe;
class ILightKeyframe;
class IMorphKeyframe;
class IEncoding;
class IModel;
class IMotion;

/**
 * モデルやモーション、キーフレームのインスタンスを作成するファクトリパターンに基づいたクラスです。
 * いずれのメソッドも指定されたインターフェースを継承するインスタンスを返します。
 *
 */
class VPVL2_API Factory
{
public:
    static IModel::Type findModelType(const uint8_t *data, size_t size);
    static IMotion::Type findMotionType(const uint8_t *data, size_t size);

    Factory(IEncoding *encoding);
    ~Factory();

    /**
     * type (IModel::Type) に基づいた空の Model インスタンスを作成します。
     *
     * IModel::Type 以外の値を指定した場合は null を返します。
     *
     * @param type
     * @return IModel
     */
    IModel *createModel(IModel::Type type) const;

    /**
     * オンメモリ上にあるデータとその長さを元に読み込み済みの Model インスタンスを作成します。
     *
     * 読み込みに成功した場合第３引数の ok が true に、失敗した場合は false にセットされます。
     * 読み込みの成功可否にかかわらず IModel インスタンスを返します。
     *
     * @param data
     * @param size
     * @param ok
     * @return IModel
     */
    IModel *createModel(const uint8_t *data, size_t size, bool &ok) const;

    /**
     * 空の Motion インスタンスを返します。
     *
     * @return IMotion
     */
    IMotion *createMotion(vpvl2::IMotion::Type type, IModel *model) const;

    /**
     * オンメモリ上にあるデータとその長さを元に読み込み済みの Motion インスタンスを作成します。
     *
     * 読み込みに成功した場合第４引数の ok が true に、失敗した場合は false にセットされます。
     * 読み込みの成功可否にかかわらず IMotion インスタンスを返します。
     *
     * 第３引数は通常モーションの動作対象となる IModel インスタンスを指定しますが、
     * カメラまたは照明といったモデル非依存のモーションの場合は null を設定してください。
     *
     * @param data
     * @param size
     * @param model
     * @param ok
     * @return IMotion
     */
    IMotion *createMotion(const uint8_t *data, size_t size, IModel *model, bool &ok) const;

    /**
     * IBoneKeyframe (ボーンのキーフレーム) のインスタンスを返します。
     *
     * @return IBoneKeyframe
     */
    IBoneKeyframe *createBoneKeyframe() const;

    /**
     * ICameraKeyframe (カメラのキーフレーム) のインスタンスを返します。
     *
     * @return ICameraKeyframe
     */
    ICameraKeyframe *createCameraKeyframe() const;

    /**
     * ILightKeyframe (照明のキーフレーム) のインスタンスを返します。
     *
     * @return ILightKeyframe
     */
    ILightKeyframe *createLightKeyframe() const;

    /**
     * IMorphKeyframe (モーフのキーフレーム) のインスタンスを返します。
     *
     * @return IMorphKeyframe
     */
    IMorphKeyframe *createMorphKeyframe() const;

private:
    struct PrivateContext;
    PrivateContext *m_context;
};

} /* namespace vpvl2 */

#endif
