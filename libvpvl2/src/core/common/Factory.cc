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

#include "vpvl2/vpvl2.h"

#include "vpvl2/asset/Model.h"
#include "vpvl2/mvd/Motion.h"
#include "vpvl2/pmd/Model.h"
#include "vpvl2/pmx/Model.h"
#include "vpvl2/mvd/BoneKeyframe.h"
#include "vpvl2/mvd/CameraKeyframe.h"
#include "vpvl2/mvd/LightKeyframe.h"
#include "vpvl2/mvd/MorphKeyframe.h"
#include "vpvl2/vmd/BoneKeyframe.h"
#include "vpvl2/vmd/CameraKeyframe.h"
#include "vpvl2/vmd/LightKeyframe.h"
#include "vpvl2/vmd/MorphKeyframe.h"
#include "vpvl2/vmd/Motion.h"

namespace vpvl2
{

struct Factory::PrivateContext
{
    PrivateContext(IEncoding *encoding)
        : encoding(encoding),
          motion(0)
    {
    }
    ~PrivateContext() {
        delete motion;
        motion = 0;
    }

    IEncoding *encoding;
    IMotion *motion;
};

IModel::Type Factory::findModelType(const uint8_t *data, size_t size)
{
    if (size >= 4 && memcmp(data, "PMX ", 4) == 0) {
        return IModel::kPMX;
    }
    else if (size >= 3 && memcmp(data, "Pmd", 3) == 0) {
        return IModel::kPMD;
    }
    else {
        return IModel::kAsset;
    }
}

IMotion::Type Factory::findMotionType(const uint8_t *data, size_t size)
{
    if (size >= sizeof(vmd::Motion::kSignature) &&
            memcmp(data, vmd::Motion::kSignature, sizeof(vmd::Motion::kSignature) - 1) == 0) {
        return IMotion::kVMD;
    }
    else if (size >= sizeof(mvd::Motion::kSignature) &&
             memcmp(data, mvd::Motion::kSignature, sizeof(mvd::Motion::kSignature) - 1) == 0) {
        return IMotion::kMVD;
    }
    else {
        return IMotion::kUnknown;
    }
}

Factory::Factory(IEncoding *encoding)
    : m_context(0)
{
    m_context = new PrivateContext(encoding);
}

Factory::~Factory()
{
    delete m_context;
    m_context = 0;
}

IModel *Factory::createModel(IModel::Type type) const
{
    switch (type) {
    case IModel::kAsset:
        return new asset::Model(m_context->encoding);
    case IModel::kPMD:
        return new pmd::Model(m_context->encoding);
    case IModel::kPMX:
        return new pmx::Model(m_context->encoding);
    default:
        return 0;
    }
}

IModel *Factory::createModel(const uint8_t *data, size_t size, bool &ok) const
{
    IModel *model = createModel(findModelType(data, size));
    ok = model ? model->load(data, size) : false;
    return model;
}

IMotion *Factory::createMotion(IMotion::Type type, IModel *model) const
{
    switch (type) {
    case IMotion::kVMD:
        return new vmd::Motion(model, m_context->encoding);
    case IMotion::kMVD:
        return new mvd::Motion(model, m_context->encoding);
    default:
        return 0;
    }
}

IMotion *Factory::createMotion(const uint8_t *data, size_t size, IModel *model, bool &ok) const
{
    IMotion *motion = createMotion(findMotionType(data, size), model);
    ok = motion ? motion->load(data, size) : false;
    return motion;
}

IBoneKeyframe *Factory::createBoneKeyframe(const IMotion *motion) const
{
    if (motion) {
        switch (motion->type()) {
        case IMotion::kMVD:
            return new mvd::BoneKeyframe(static_cast<const mvd::Motion *>(motion)->nameListSection());
        case IMotion::kVMD:
            return new vmd::BoneKeyframe(m_context->encoding);
        default:
            break;
        }
    }
    return 0;
}

ICameraKeyframe *Factory::createCameraKeyframe(const IMotion *motion) const
{
    if (motion) {
        switch (motion->type()) {
        case IMotion::kMVD:
            return new mvd::CameraKeyframe();
        case IMotion::kVMD:
            return new vmd::CameraKeyframe();
        default:
            break;
        }
    }
    return 0;
}

ILightKeyframe *Factory::createLightKeyframe(const IMotion *motion) const
{
    if (motion) {
        switch (motion->type()) {
        case IMotion::kMVD:
            return new mvd::LightKeyframe();
        case IMotion::kVMD:
            return new vmd::LightKeyframe();
        default:
            break;
        }
    }
    return 0;
}

IMorphKeyframe *Factory::createMorphKeyframe(const IMotion *motion) const
{
    if (motion) {
        switch (motion->type()) {
        case IMotion::kMVD:
            return new mvd::MorphKeyframe(static_cast<const mvd::Motion *>(motion)->nameListSection());
        case IMotion::kVMD:
            return new vmd::MorphKeyframe(m_context->encoding);
        default:
            break;
        }
    }
    return 0;
}

IMotion *Factory::convertMotion(IMotion *source, IMotion::Type destType) const
{
    IMotion::Type sourceType = source->type();
    if (sourceType == destType) {
        return source->clone();
    }
    else if (sourceType == IMotion::kVMD && destType == IMotion::kMVD) {
    }
    else if (sourceType == IMotion::kMVD && destType == IMotion::kVMD) {
    }
    return 0;
}

}
