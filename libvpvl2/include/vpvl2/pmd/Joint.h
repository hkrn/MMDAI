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

#ifndef VPVL2_PMD_JOINT_H_
#define VPVL2_PMD_JOINT_H_

#include "vpvl2/Common.h"
#include "vpvl2/internal/BaseJoint.h"
#include "vpvl2/pmd/Model.h"

class btGeneric6DofSpringConstraint;
class btRigidBody;

namespace vpvl2
{

class IEncoding;
class IString;

namespace pmd
{

class VPVL2_API Joint : public internal::BaseJoint
{
public:
    static const int kNameSize = 20;

    Joint(IEncoding *encodingRef);
    ~Joint();

    static bool preparse(uint8_t *&ptr, size_t &rest, Model::DataInfo &info);
    static bool loadJoints(const Array<Joint *> &joints, const Array<RigidBody *> &rigidBodies);
    static size_t estimateTotalSize(const Array<Joint *> &joints, const Model::DataInfo &info);

    void read(const uint8_t *data, const Model::DataInfo &info, size_t &size);
    void write(uint8_t *data, const Model::DataInfo &info) const;
    size_t estimateSize(const Model::DataInfo &info) const;

private:
    IEncoding *m_encodingRef;
    VPVL2_DISABLE_COPY_AND_ASSIGN(Joint)
};

}
}

#endif
