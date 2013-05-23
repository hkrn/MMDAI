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
#ifndef VPVL2_INTERNAL_MODELHELPER_H_
#define VPVL2_INTERNAL_MODELHELPER_H_

#include "vpvl2/Common.h"
#include "vpvl2/IModel.h"
#include "vpvl2/internal/util.h"

namespace vpvl2
{
namespace internal
{

class ModelHelper {
public:
    static inline void transformVertex(const Transform &transform,
                                       const Vector3 &inPosition,
                                       const Vector3 &inNormal,
                                       Vector3 &outPosition,
                                       Vector3 &outNormal)
    {
        outPosition = transform * inPosition;
        outNormal = transform.getBasis() * inNormal;
    }
    static inline void transformVertex(const Transform &transformA,
                                       const Transform &transformB,
                                       const Vector3 &inPosition,
                                       const Vector3 &inNormal,
                                       Vector3 &outPosition,
                                       Vector3 &outNormal,
                                       const IVertex::WeightPrecision &weight)
    {
        const Vector3 &v1 = transformA * inPosition;
        const Vector3 &n1 = transformA.getBasis() * inNormal;
        const Vector3 &v2 = transformB * inPosition;
        const Vector3 &n2 = transformB.getBasis() * inNormal;
        const Scalar w(weight);
        outPosition.setInterpolate3(v2, v1, w);
        outNormal.setInterpolate3(n2, n1, w);
    }
    static inline bool hasBoneLoopChain(const IBone * /* parentBoneRef */, const IModel * /* baseModelRef */) {
        /* FIXME: implement this to stop loop chain */
        return false;
    }
    static inline bool hasModelLoopChain(const IModel *baseModelRef, const IModel *targetModelRef) {
        if (baseModelRef) {
            IModel *modelRef = baseModelRef->parentModelRef();
            while (modelRef) {
                if (modelRef == targetModelRef) {
                    return true;
                }
                modelRef = modelRef->parentModelRef();
            }
        }
        return false;
    }
    template<typename IndexType>
    static inline void swapIndices(IndexType *indicesPtr, const int nindices) {
        for (int i = 0; i < nindices; i += 3) {
            btSwap(indicesPtr[i], indicesPtr[i + 1]);
        }
    }
    template<typename T, typename I>
    static inline void getObjectRefs(const Array<T *> &objects, Array<I *> &value) {
        const int nobjects = objects.count();
        for (int i = 0; i < nobjects; i++) {
            T *object = objects[i];
            value.append(object);
        }
    }
    template<typename T, typename I>
    static inline I *findObjectAt(const Array<T *> &objects, int index) {
        return internal::checkBound(index, 0, objects.count()) ? objects[index] : 0;
    }
    template<typename T, typename I>
    static inline void addObject(IModel *modelRef, I *value, Array<T *> &objects) {
        if (value && value->index() == -1 && value->parentModelRef() == modelRef) {
            T *object = static_cast<T *>(value);
            object->setIndex(objects.count());
            objects.append(object);
        }
    }
    template<typename T, typename I>
    static inline void addObject2(IModel *modelRef, I *value, Array<I *> &objects) {
        if (value && value->index() == -1 && value->parentModelRef() == modelRef) {
            T *object = static_cast<T *>(value);
            object->setIndex(objects.count());
            objects.append(object);
        }
    }
    template<typename T, typename I>
    static inline void removeObject(IModel *modelRef, I *value, Array<T *> &objects) {
        if (value && value->parentModelRef() == modelRef) {
            T *object = static_cast<T *>(value);
            object->setIndex(-1);
            objects.remove(object);
        }
    }
    template<typename T, typename I>
    static inline void removeObject2(IModel *modelRef, I *value, Array<I *> &objects) {
        if (value && value->parentModelRef() == modelRef) {
            T *object = static_cast<T *>(value);
            object->setIndex(-1);
            objects.remove(object);
        }
    }

private:
    ModelHelper();
    ~ModelHelper();
};

} /* namespace internal */
} /* namespace vpvl2 */

#endif
