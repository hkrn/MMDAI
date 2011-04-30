/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn (libMMDAI)                         */
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

#ifndef MMDME_PMDTEXTURE_H_
#define MMDME_PMDTEXTURE_H_

#include "MMDME/Common.h"
#include "MMDME/PMDRenderEngine.h"

namespace MMDAI {

class PMDTexture
{
public:
    static bool loadTGAImage(const unsigned char *data, unsigned char **ptr, int *pwidth, int *pheight);

    PMDTexture();
    ~PMDTexture();

    void loadBytes(const unsigned char *data,
                   size_t size,
                   int width,
                   int height,
                   int components,
                   bool isSphereMap,
                   bool isSphereMapAdd);
    void release();

    inline void setRenderEngine(PMDRenderEngine *value) {
        m_engine = value;
    }
    inline PMDTextureNative *getNative() const {
        return m_native;
    }
    inline bool isSPH() const {
        return m_isSphereMap;
    }
    inline bool isSPA() const {
        return m_isSphereMapAdd;
    }

private:
    PMDRenderEngine *m_engine;
    PMDTextureNative *m_native;
    bool m_isSphereMap;
    bool m_isSphereMapAdd;
    int m_width;
    int m_height;
    unsigned char m_components;
    unsigned char *m_textureData;

    MMDME_DISABLE_COPY_AND_ASSIGN(PMDTexture);
};

} /* namespace */

#endif
