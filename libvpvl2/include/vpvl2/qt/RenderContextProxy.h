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
#ifndef VPVL2_QT_RENDERCONTEXTPROXY_H_
#define VPVL2_QT_RENDERCONTEXTPROXY_H_

#include "vpvl2/IEffect.h"
#include "vpvl2/qt/Common.h"
#include <QtCore/QHash>

namespace vpvl2
{

namespace extensions
{
namespace gl
{
class FrameBufferObject;
}
}

namespace qt
{

using namespace extensions::gl;

class VPVL2QTCOMMON_API RenderContextProxy
{
public:
    static void initialize();
    static FrameBufferObject *newFrameBufferObject(size_t width, size_t height, int samples);
    static FrameBufferObject *createFrameBufferObject(size_t width, size_t height, int samples, bool enableAA);
    static void bindOffscreenRenderTarget(unsigned int textureID, unsigned int textureFormat, FrameBufferObject *fbo);
    static void releaseOffscreenRenderTarget(FrameBufferObject *fbo);
    static void deleteAllRenderTargets(QHash<unsigned int, FrameBufferObject *> &renderTargets);

private:
    RenderContextProxy();
    ~RenderContextProxy();
    VPVL2_DISABLE_COPY_AND_ASSIGN(RenderContextProxy)
};

} /* namespace qt */
} /* namespace vpvl2 */

#endif /* VPVL2_QT_CSTRING_H_ */
