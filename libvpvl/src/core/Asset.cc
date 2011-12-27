/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2011  hkrn                                    */
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

#include "vpvl/vpvl.h"

#ifdef VPVL_LINK_ASSIMP

#include <assimp.hpp>
#include <aiPostProcess.h>
#include <aiScene.h>

namespace vpvl
{

bool Asset::isSupported()
{
    return true;
}

Asset::Asset()
    : m_importer(0),
      m_scene(0),
      m_parentBone(0),
      m_position(0.0f, 0.0f, 0.0f),
      m_rotation(0.0f, 0.0f, 0.0f, 1.0f),
      m_scale(10.0f),
      m_opacity(1.0f),
      m_flags(aiProcessPreset_TargetRealtime_Quality | aiProcess_FlipUVs),
      m_name(0)
{
    m_importer = new Assimp::Importer();
    m_importer->SetPropertyInteger(AI_CONFIG_PP_SBP_REMOVE, aiPrimitiveType_LINE | aiPrimitiveType_POINT);
}

Asset::~Asset()
{
    delete[] m_name;
    m_name = 0;
    m_flags = 0;
    m_scale = 0.0f;
    m_opacity = 0.0f;
    m_position.setZero();
    m_rotation.setValue(0.0f, 0.0f, 0.0f, 1.0f);
    m_scene = 0;
    delete m_importer;
    m_importer = 0;
}

bool Asset::load(const char *path)
{
    m_scene = m_importer->ReadFile(path, m_flags);
    return m_scene != NULL;
}

bool Asset::load(const uint8_t *data, size_t size)
{
    m_scene = m_importer->ReadFileFromMemory(data, size, m_flags);
    return m_scene != NULL;
}

void Asset::setName(const char *name)
{
    delete[] m_name;
    m_name = new char[strlen(name) + 1];
    strcpy(m_name, name);
}

#else /* VPVL_LINK_ASSIMP */

namespace vpvl
{

bool Asset::isSupported()
{
    return false;
}

Asset::Asset()
    : m_importer(0),
      m_scene(0),
      m_position(0.0f, 0.0f, 0.0f),
      m_rotation(0.0f, 0.0f, 0.0f, 1.0f),
      m_scale(10.0f),
      m_opacity(1.0f),
      m_flags(0),
      m_name(0)
{
}

Asset::~Asset()
{
    m_name = 0;
    m_flags = 0;
    m_scale = 0.0f;
    m_opacity = 0.0f;
    m_position.setZero();
    m_rotation.setValue(0.0f, 0.0f, 0.0f, 1.0f);
    m_scene = 0;
    m_importer = 0;
}

bool Asset::load(const char * /* path */)
{
    return false;
}

bool Asset::load(const uint8_t * /* data */, size_t /* size */)
{
    return false;
}

void Asset::setName(const char * /* name */)
{
}

#endif /* VPVL_LINK_ASSIMP */

void Asset::setPosition(const Vector3 &value)
{
    m_position = value;
}

void Asset::setRotation(const Quaternion &value)
{
    m_rotation = value;
}

void Asset::setScaleFactor(const Scalar &value)
{
    m_scale = value;
}

void Asset::setOpacity(const Scalar &value)
{
    m_opacity = value;
}

void Asset::setLoadFlags(uint32_t value)
{
    m_flags = value;
}

void Asset::setParentBone(Bone *value)
{
    m_parentBone = value;
}

void Asset::setUserData(AssetUserData *value)
{
    m_userData = value;
}

} /* namespace vpvl */

