/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn                                    */
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

#ifndef VPVL_CG_RENDERER_H_
#define VPVL_CG_RENDERER_H_

#include <string>
#include "vpvl/Asset.h"
#include "vpvl/gl2/Renderer.h"

#ifdef __APPLE__
#include <Cg/cg.h>
#include <Cg/cgGL.h>
#else
#include <cg.h>
#include <cgGL.h>
#endif

namespace vpvl
{

namespace cg
{

struct PMDModelUserData;

class VPVL_API IDelegate : public gl2::Renderer::IDelegate
{
public:
    virtual bool loadEffect(vpvl::PMDModel *model, const std::string &dir, std::string &source) = 0;
};

/**
 * @file
 * @author Nagoya Institute of Technology Department of Computer Science
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * Bone class represents a bone of a Polygon Model Data object.
 */

class VPVL_API Renderer : public vpvl::gl2::Renderer
{
public:
    Renderer(IDelegate *delegate, int width, int height, int fps);
    virtual ~Renderer();

    void uploadModel(vpvl::PMDModel *model, const std::string &dir);
    void deleteModel(vpvl::PMDModel *&model);
    void renderModel(const vpvl::PMDModel *model);

private:
    void renderModel0(const vpvl::cg::PMDModelUserData *userData, const vpvl::PMDModel *model);

    CGcontext m_context;

    VPVL_DISABLE_COPY_AND_ASSIGN(Renderer)
};

}
}

#endif

