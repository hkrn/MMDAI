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
#ifndef VPVL2_ICAMERA_H_
#define VPVL2_ICAMERA_H_

#include "vpvl2/Common.h"

namespace vpvl2
{

class IMotion;

class VPVL2_API ICamera
{
public:
    class PropertyEventListener {
    public:
        virtual ~PropertyEventListener() {}
        virtual void lookAtWillChange(const Vector3 &value, ICamera *camera) = 0;
        virtual void angleWillChange(const Vector3 &value, ICamera *camera) = 0;
        virtual void fovWillChange(const Scalar &value, ICamera *camera) = 0;
        virtual void distanceWillChange(const Scalar &value, ICamera *camera) = 0;
        virtual void motionWillChange(IMotion *value, ICamera *camera) = 0;
    };

    virtual ~ICamera() {}

    virtual void addEventListenerRef(PropertyEventListener *value) = 0;
    virtual void removeEventListenerRef(PropertyEventListener *value) = 0;
    virtual void getEventListenerRefs(Array<PropertyEventListener *> &value) = 0;

    /**
     * カメラのビュー行列を返します.
     *
     * @brief modelViewTransform
     * @return Transform
     */
    virtual Transform modelViewTransform() const = 0;

    /**
     * カメラの注視点の位置を返します.
     *
     * @brief lookAt
     * @return Vector3
     * @sa position
     * @sa setLookAt
     */
    virtual Vector3 lookAt() const = 0;

    /**
     * カメラの位置を返します.
     *
     * カメラの位置はカメラの注視点に視野距離を Z 軸に対して加算した値となります。
     *
     * @brief position
     * @return Vector3
     * @sa lookAt
     */
    virtual Vector3 position() const = 0;

    /**
     * カメラのアングル角度（全てラジアン）を返します.
     *
     * @brief angle
     * @return Vector3
     * @sa setAngle
     */
    virtual Vector3 angle() const = 0;

    /**
     * カメラの視野角（ラジアンではなく角度）を返します.
     *
     * @brief fov
     * @return Scalar
     * @sa setFov
     */
    virtual Scalar fov() const = 0;

    /**
     * カメラの視野距離を返します.
     *
     * @brief distance
     * @return Scalar
     * @sa setDistance
     */
    virtual Scalar distance() const = 0;

    /**
     * @brief znear
     * @return Scalar
     * @sa setZNear
     */
    virtual Scalar znear() const = 0;

    /**
     * @brief zfar
     * @return Scalar
     * @sa setZFar
     */
    virtual Scalar zfar() const = 0;

    /**
     * 現在設定されているカメラモーションのインスタンスを返します.
     *
     * カメラモーションが設定されている場合は 0 以外の値を返します。
     * メモリ上の所有権は ICamera にあるため、delete で解放してはいけません。
     * 初期値は 0 (カメラモーション未設定) です。
     *
     * @brief motion
     * @return IMotion
     * @sa setMotion
     */
    virtual IMotion *motion() const = 0;

    /**
     * カメラの注視点を設定します.
     *
     * @brief setLookAt
     * @param value
     * @sa lookAt
     */
    virtual void setLookAt(const Vector3 &value) = 0;

    /**
     * カメラのアングル角度を設定します.
     *
     * @brief setAngle
     * @param value
     * @sa angle
     */
    virtual void setAngle(const Vector3 &value) = 0;

    /**
     * カメラの視野角（ラジアンではなく角度）を設定します.
     *
     * @brief setFov
     * @param value
     * @sa fov
     */
    virtual void setFov(Scalar value) = 0;

    /**
     * カメラの視野距離を設定します.
     *
     * @brief setDistance
     * @param value
     * @sa distance
     */
    virtual void setDistance(Scalar value) = 0;

    /**
     * @brief setZNear
     * @param value
     * @sa znear
     */
    virtual void setZNear(Scalar value) = 0;

    /**
     * @brief setZFar
     * @param value
     * @sa zfar
     */
    virtual void setZFar(Scalar value) = 0;

    /**
     * カメラモーションを設定します.
     *
     * このメソッドを呼び出して渡したカメラモーションのメモリ上の所有権が ICamera に移動するため、
     * もし先にメモリ解放を行う場合は先にこのメソッドの引数に 0 を渡してメモリ所有権を解放して
     * 二重解放を行わないようにする必要があります。
     *
     * @brief setMotion
     * @param value
     * @sa motion
     */
    virtual void setMotion(IMotion *value) = 0;

    /**
     * 別の ICamera のインスタンスを自身にコピーします.
     *
     * @brief copyFrom
     * @param value
     */
    virtual void copyFrom(const ICamera *value) = 0;

    /**
     * カメラ設定を初期値にリセットします.
     *
     * カメラ設定の初期値は以下になります。
     * lookAt = (0, 10, 0)
     * angle  = (0, 0, 0)
     * fov = 30
     * distance = 50
     *
     * @brief resetDefault
     */
    virtual void resetDefault() = 0;
};

} /* namespace vpvl2 */

#endif
