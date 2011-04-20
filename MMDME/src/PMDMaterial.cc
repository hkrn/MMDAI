/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2010  Nagoya Institute of Technology          */
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
/* - Neither the name of the MMDAgent project team nor the names of  */
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

/* headers */

#include "MMDME/MMDME.h"

namespace MMDAI {

PMDMaterial::PMDMaterial(PMDRenderEngine *engine)
    : m_alpha(0.0f),
    m_shiness(0.0f),
    m_numSurface(0),
    m_toonID(0),
    m_edgeFlag(false),
    m_engine(engine)
{
}

PMDMaterial::~PMDMaterial()
{
    release();
    m_engine = NULL;
}

void PMDMaterial::release()
{
    for (int i = 0; i < 3; i++) {
        m_diffuse[i] = 0.0f;
        m_ambient[i] = 0.0f;
        m_avgcol[i] = 0.0f;
        m_specular[i] = 0.0f;
    }
    m_alpha = 0.0f;
    m_shiness = 0.0f;
    m_numSurface = 0;
    m_toonID = 0;
    m_edgeFlag = false;
    m_texture.release();
    m_additionalTexture.release();
}

bool PMDMaterial::setup(const PMDFile_Material *m, IModelLoader *loader)
{
    release();

    /* colors */
    for (int i = 0; i < 3; i++) {
        m_diffuse[i] = m->diffuse[i];
        m_ambient[i] = m->ambient[i];
        /* calculate average color of diffuse and ambient for toon rendering */
        m_avgcol[i] = (m_diffuse[i] + m_ambient[i]) * 0.5f;
        m_specular[i] = m->specular[i];
    }
    m_alpha = m->alpha;
    m_shiness = m->shiness;

    /* number of surface indices whose material should be assigned by this */
    m_numSurface = m->numSurfaceIndex;

    /* toon texture ID */
    if (m->toonID == 0xff)
        m_toonID = 0;
    else
        m_toonID = m->toonID + 1;
    /* edge drawing flag */
    m_edgeFlag = m->edgeFlag ? true : false;

    /* load model texture */
    char name[21], raw[21];
    MMDAIStringCopySafe(name, m->textureFile, sizeof(name));
    MMDAIStringCopySafe(raw, name, sizeof(raw));
    if (MMDAIStringLength(name) > 0) {
        char *ptr = strchr(name, '*');
        m_texture.setRenderEngine(m_engine);
        m_additionalTexture.setRenderEngine(m_engine);
        if (ptr) {
            /* has extra sphere map */
            *ptr = '\0';
            if (!loader->loadModelTexture(name, &m_texture))
                return false;
            if (!loader->loadModelTexture(ptr + 1, &m_additionalTexture))
                return false;
        } else {
            if (!loader->loadModelTexture(name, &m_texture))
                return false;
        }
    }

    MMDAILogDebug("name=\"%s\", ambient=(%.2f, %.2f, %.2f), diffuse=(%.2f, %.2f, %.2f), specular=(%.2f, %.2f, %.2f), "
                  "alpha=%.2f, shiness=%.2f, numSurface=%d, toonID=%d, edge=%d", raw, m_ambient[0], m_ambient[1], m_ambient[2],
                  m_diffuse[0], m_diffuse[1], m_diffuse[2], m_specular[0], m_specular[1], m_specular[2], m_alpha, m_shiness,
                  m_numSurface, m_toonID, m_edgeFlag);

    return true;
}

} /* namespace */

