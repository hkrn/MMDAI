/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2012  hkrn                                    */
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

#ifndef VPVL2_PMX_CONSTRAINT_H_
#define VPVL2_PMX_CONSTRAINT_H_

#include "vpvl2/pmx/RigidBody.h"

class btGeneric6DofConstraint;
class btGeneric6DofSpringConstraint;

namespace vpvl2
{
namespace pmx
{

/**
 * @file
 * @author Nagoya Institute of Technology Department of Computer Science
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * Constraint class represents a consraint of a Polygon Model Data object.
 */

class VPVL2_API Constraint
{
public:
    Constraint();
    ~Constraint();

    static bool preparse(const uint8_t *data, size_t &rest, Model::DataInfo &info);

    void read(const uint8_t *data);
    void write(uint8_t *data) const;

    const uint8_t *name() const {
        return m_name;
    }
    btGeneric6DofConstraint *constraint() const {
        return reinterpret_cast<btGeneric6DofConstraint *>(m_constraint);
    }

private:
    uint8_t *m_name;
    btGeneric6DofSpringConstraint *m_constraint;
    Vector3 m_position;
    Vector3 m_rotation;
    Vector3 m_limitPositionFrom;
    Vector3 m_limitPositionTo;
    Vector3 m_limitRotationFrom;
    Vector3 m_limitRotationTo;
    float m_stiffness[6];
    int m_bodyA;
    int m_bodyB;

    VPVL2_DISABLE_COPY_AND_ASSIGN(Constraint)
};

typedef Array<Constraint*> ConstraintList;

} /* namespace pmx */
} /* namespace vpvl2 */

#endif

