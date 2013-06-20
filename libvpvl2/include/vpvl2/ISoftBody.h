/**

 Copyright (c) 2010-2013  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#pragma once
#ifndef VPVL2_ISOFTBODY_H_
#define VPVL2_ISOFTBODY_H_

#include "vpvl2/Common.h"

namespace vpvl2
{

class IString;

class VPVL2_API ISoftBody {
public:
    enum ShapeType {
        kTriMesh,
        kRope
    };
    enum AeroModelType {
        kVPoint,
        kVTwoSided,
        kVOneSided,
        kFTwoSided,
        kFOneSided
    };

    virtual ~ISoftBody() {}

    /**
     * ソフトボディの名前を返します.
     *
     * @return IString
     */
    virtual const IString *name() const = 0;

    /**
     * ソフトボディの名前を返します.
     *
     * @return IString
     */
    virtual const IString *englishName() const = 0;

    /**
     * ソフトボディの ID を返します.
     *
     * 常にユニークな値で返します。
     *
     * @return int
     */
    virtual int index() const = 0;

    virtual ShapeType shapeType() const = 0;

    virtual uint8 collideGroup() const = 0;

    virtual uint16 uncollideGroupFlags() const = 0;

    virtual bool hasBLink() const = 0;

    virtual bool hasCluster() const = 0;

    virtual bool hasLink() const = 0;

    virtual int distanceBLink() const = 0;

    virtual int numCluster() const = 0;

    virtual float mass() const = 0;

    virtual float collisionMargin() const = 0;

    virtual AeroModelType aeroModelType() const = 0;
};

} /* namespace vpvl2 */

#endif

