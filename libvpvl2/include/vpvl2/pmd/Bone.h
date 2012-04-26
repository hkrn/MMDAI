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

#ifndef VPVL2_PMD_BONE_H_
#define VPVL2_PMD_BONE_H_

#include "vpvl2/Common.h"
#include "vpvl2/IBone.h"
#include "vpvl2/IEncoding.h"
#include "vpvl2/IString.h"

#include "vpvl/Bone.h"

namespace vpvl2
{
namespace pmd
{

class VPVL2_API Bone : public IBone
{
public:
    Bone(vpvl::Bone *bone, IEncoding *encoding);
    ~Bone();

    const IString *name() const;
    int index() const;
    IBone *parentBone() const { return m_parentBone; }
    const Transform &localTransform() const;
    const Vector3 &origin() const;
    const Vector3 destinationOrigin() const;
    const Vector3 &position() const;
    const Quaternion &rotation() const;
    void setPosition(const Vector3 &value);
    void setRotation(const Quaternion &value);
    bool isMovable() const;
    bool isRotateable() const;
    bool isVisible() const;
    bool hasFixedAxes() const;
    bool hasLocalAxes() const;
    void getFixedAxes(Matrix3x3 &value) const;
    void getLocalAxes(Matrix3x3 &value) const;

    void setParentBone(vpvl::Bone * value);
    void setChildBone(vpvl::Bone *value);

private:
    IEncoding *m_encoding;
    IString *m_name;
    IBone *m_parentBone;
    IBone *m_childBone;
    vpvl::Bone *m_bone;
};

}
}

#endif
