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

#ifndef VPVL2_SCENE_H_
#define VPVL2_SCENE_H_

#include "vpvl2/Common.h"

namespace vpvl2
{

class IEncoding;
class IModel;
class IMotion;
class IRenderDelegate;
class IRenderEngine;

class VPVL2_API Scene
{
public:
    class IMatrices {
    public:
        virtual ~IMatrices() {}

        virtual void getModel(float value[16]) const = 0;
        virtual void getView(float value[16]) const = 0;
        virtual void getProjection(float value[16]) const = 0;
        virtual void getModelView(float value[16]) const = 0;
        virtual void getModelViewProjection(float value[16]) const = 0;
        virtual void getNormal(float value[16]) const = 0;
        virtual void setModel(float value[16]) = 0;
        virtual void setView(float value[16]) = 0;
        virtual void setProjection(float value[16]) = 0;
        virtual void setModelView(float value[16]) = 0;
        virtual void setModelViewProjection(float value[16]) = 0;
        virtual void setNormal(float value[9]) = 0;
    };
    class ILight {
    public:
        virtual ~ILight() {}

        virtual const Vector3 &color() const = 0;
        virtual const Vector3 &direction() const = 0;
        virtual IMotion *motion() const = 0;
        virtual void setColor(const Vector3 &value) = 0;
        virtual void setDirection(const Vector3 &value) = 0;
        virtual void setMotion(IMotion *value) = 0;
        virtual void copyFrom(ILight *value) = 0;
        virtual void resetDefault() = 0;
    };
    class ICamera {
    public:
        virtual ~ICamera() {}

        virtual const Transform &modelViewTransform() const = 0;
        virtual const Vector3 &position() const = 0;
        virtual const Vector3 &angle() const = 0;
        virtual Scalar fovy() const = 0;
        virtual Scalar distance() const = 0;
        virtual Scalar znear() const = 0;
        virtual Scalar zfar() const = 0;
        virtual IMotion *motion() const = 0;
        virtual void setPosition(const Vector3 &value) = 0;
        virtual void setAngle(const Vector3 &value) = 0;
        virtual void setFovy(Scalar value) = 0;
        virtual void setDistance(Scalar value) = 0;
        virtual void setZNear(Scalar value) = 0;
        virtual void setZFar(Scalar value) = 0;
        virtual void setMotion(IMotion *value) = 0;
        virtual void copyFrom(ICamera *value) = 0;
        virtual void resetDefault() = 0;
    };

    static bool isAcceleratorSupported();
    static const Scalar &defaultFPS();

    Scene();
    virtual ~Scene();

    IRenderEngine *createRenderEngine(vpvl2::IRenderDelegate *delegate, IModel *model) const;
    void addModel(IModel *model, IRenderEngine *engine);
    void addMotion(IMotion *motion);
    void deleteModel(vpvl2::IModel *&model);
    void removeMotion(IMotion *motion);
    void advance(float delta);
    void seek(float frameIndex);
    void updateModels();
    void updateCamera();
    void setPreferredFPS(const Scalar &value);
    bool isReachedTo(float frameIndex) const;
    float maxFrameIndex() const;
    const Array<IModel *> &models() const;
    const Array<IMotion *> &motions() const;
    const Array<IRenderEngine *> &renderEngines() const;
    IRenderEngine *renderEngine(IModel *model) const;
    IMatrices *matrices() const;
    ILight *light() const;
    ICamera *camera() const;
    const Scalar &preferredFPS() const;

private:
    struct PrivateContext;
    PrivateContext *m_context;
};

} /* namespace vpvl2 */

#endif
