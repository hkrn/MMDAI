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

#ifndef VPVL_ASSET_H_
#define VPVL_ASSET_H_

#include "vpvl/Common.h"

struct aiScene;

namespace Assimp
{
class Importer;
}

namespace vpvl
{

/**
 * @file
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * Asset class represents an accessory
 */

class Bone;
class PMDModel;

class VPVL_API Asset
{
public:
    class UserData {
    public:
        UserData() {}
        virtual ~UserData() {}
    };

    static bool isSupported();

    Asset();
    ~Asset();

    bool load(const char *path);
    bool load(const uint8_t *data, size_t size);
    void setName(const char *name);

    const char *name() const {
        return m_name;
    }
    const Vector3 &position() const {
        return m_position;
    }
    const Quaternion &rotation() const {
        return m_rotation;
    }
    const Scalar &scaleFactor() const {
        return m_scale;
    }
    const Scalar &opacity() const {
        return m_opacity;
    }
    uint32_t loadFlags() const {
        return m_flags;
    }
    PMDModel *parentModel() const {
        return m_parentModel;
    }
    Bone *parentBone() const {
        return m_parentBone;
    }
    UserData *userData() const {
        return m_userData;
    }
    const aiScene *getScene() const {
        return m_scene;
    }

    void setPosition(const Vector3 &value);
    void setRotation(const Quaternion &value);
    void setScaleFactor(const Scalar &value);
    void setOpacity(const Scalar &value);
    void setLoadFlags(uint32_t value);
    void setParentModel(PMDModel *value);
    void setParentBone(Bone *value);
    void setUserData(UserData *value);

private:
    Assimp::Importer *m_importer;
    const aiScene *m_scene;
    UserData *m_userData;
    PMDModel *m_parentModel;
    Bone *m_parentBone;
    Vector3 m_position;
    Quaternion m_rotation;
    Scalar m_scale;
    Scalar m_opacity;
    uint32_t m_flags;
    char *m_name;

    VPVL_DISABLE_COPY_AND_ASSIGN(Asset)
};

typedef Array<Asset*> AssetList;

} /* namespace vpvl */

#endif
