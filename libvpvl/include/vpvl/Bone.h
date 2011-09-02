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

#ifndef VPVL_BONE_H_
#define VPVL_BONE_H_

#include "vpvl/Common.h"

namespace vpvl
{

class Bone;
typedef Array<Bone*> BoneList;

/**
 * @file
 * @author Nagoya Institute of Technology Department of Computer Science
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * Bone class represents a bone of a Polygon Model Data object.
 */

class VPVL_API Bone
{
public:

    /**
     * Type of bone kinds.
     */
    enum Type
    {
        kRotate,
        kRotateAndMove,
        kIKDestination,
        kUnknown,
        kUnderIK,
        kUnderRotate,
        kIKTarget,
        kInvisible,
        kTwist,
        kFollowRotate
    };

    /**
     * Constructor
     */
    Bone();
    ~Bone();

    /**
     * The max length of a bone name
     */
    static const int kNameSize = 20;

    /**
     * Get the center bone name in Shift_JIS, not in english.
     *
     * @return The center bone name in Shift_JIS
     */
    static const uint8_t *centerBoneName();

    /**
     * Get the center bone from a bone list that is not empty.
     *
     * This method search a bone named of centerBoneName() first.
     * If there is no bone found, returns the first bone.
     *
     * @param bones All bones of the model
     * @return The center bone
     * @see centerBoneName
     */
    static Bone *centerBone(const BoneList *bones);

    /**
     * Stride length of a bone structure.
     *
     * @return Stride length
     */
    static size_t stride();

    /**
     * Read and parse the buffer with id and sets it's result to the class.
     *
     * This method just set ID and value from buffer. You should call build method
     * to get a bone object from ID.
     *
     * @param data The buffer to read and parse
     * @param id An ID of this bone
     * @param Size of the buffer
     * @see build
     */
    void read(const uint8_t *data, int16_t id);

    /**
     * Reset all bone states.
     *
     * This method resets below properties.
     * - position
     * - rotation
     * - localTransform
     */
    void reset();

    /**
     * Resolve parent, child and target bone from all bones.
     *
     * @param bones All bones of the model
     * @param rootBone Root bone of the model
     */
    void build(BoneList *bones, Bone *rootBone);

    /**
     * Update rotation from the bone.
     *
     * This method affects only the bone of kUnderRotate or kFollowRotate.
     */
    void updateRotation();

    /**
     * Update local transform from the bone.
     *
     * This method same as calling updateTransform with current rotation property.
     */
    void updateTransform();

    /**
     * Update local transform from the bone with the quaternion.
     *
     * @param q The quaternion value to rotate
     */
    void updateTransform(const btQuaternion &q);

    /**
     * Get transform value for skinning
     *
     * @param tr The result value of transform
     */
    void getSkinTransform(btTransform &tr);

    /**
     * Get the name of this bone.
     *
     * @return the name of this bone
     */
    const uint8_t *name() const {
        return m_name;
    }

    /**
     * Get the name of this bone in English.
     *
     * @return the name of this bone in English.
     */
    const uint8_t *englishName() const {
        return m_englishName;
    }

    /**
     * Get an index of the category of this bone.
     *
     * @return an index of the category of this bone
     */
    uint8_t categoryIndex() const {
        return m_categoryIndex;
    }

    /**
     * Get the kind of this bone.
     *
     * @return the kind of this bone
     */
    Type type() const {
        return m_type;
    }

    /**
     * Get the ID of this bone.
     *
     * @return bone ID
     */
    int16_t id() const {
        return m_id;
    }

    /**
     * Get the parent bone of this bone.
     *
     * @return parent bone
     */
    const Bone *parent() const {
        return m_parentBone;
    }

    /**
     * Get the child bone of this bone.
     *
     * @return child bone
     */
    const Bone *child() const {
        return m_childBone;
    }

    /**
     * Get the target bone of this bone.
     *
     * @return target bone
     */
    const Bone *target() const {
        return m_targetBone;
    }

    /**
     * Get local transform object of this bone.
     *
     * @return local transform object
     */
    const btTransform &localTransform() const {
        return m_localTransform;
    }

    /**
     * Get offset of this bone.
     *
     * @return offset of this bone
     */
    const btVector3 &offset() const {
        return m_offset;
    }

    /**
     * Get original position of this bone.
     *
     * This method returns constant origin position so it doesn't reflect
     * transform of the bone. If you want to get transformed origin position,
     * you should call "localTransform().getOrigin()".
     *
     * @return original position of this bone
     */
    const btVector3 &originPosition() const {
        return m_originPosition;
    }

    /**
     * Get position of this bone.
     *
     * @return position of this bone
     */
    const btVector3 &position() const {
        return m_position;
    }

    /**
     * Get rotation of this bone.
     *
     * @return rotation of this bone
     */
    const btQuaternion &rotation() const {
        return m_rotation;
    }

    /**
     * Get the bone is constrainted only X coordinate.
     *
     * @return true if this bone is constrainted
     */
    bool isConstraintedXCoordinateForIK() const {
        return m_constraintedXCoordinateForIK;
    }

    /**
     * Get this bone is simulated.
     *
     * @return true if this bone is simulated
     */
    bool isSimulated() const {
        return m_simulated;
    }

    /**
     * Get this bone is independent.
     *
     * @return true if this bone is independent
     */
    bool hasMotionIndependency() const {
        return m_motionIndepent;
    }

    /**
     * Get this bone has parent.
     *
     * @return true if this bone has parent
     */
    bool hasParent() const {
        return m_hasParent;
    }

    /**
     * Set the name of this bone.
     *
     * @param value the name of this bone
     */
    void setName(const uint8_t *value) {
        copyBytesSafe(m_name, value, sizeof(m_name));
    }

    /**
     * Set the name of this bone in English.
     *
     * @param value the name of this bone in English
     */
    void setEnglishName(const uint8_t *value) {
        copyBytesSafe(m_englishName, value, sizeof(m_englishName));
    }

    /**
     * Set an index of the category of this bone.
     *
     * @param value an index of the category of this bone
     */
    void setCategoryIndex(uint8_t value) {
        m_categoryIndex = value;
    }

    /**
     * Set transform object
     *
     * @param value transform object
     */
    void setLocalTransform(const btTransform &value) {
        m_localTransform = value;
    }

    /**
     * Set offset of this bone.
     *
     * @param value offset of this bone
     */
    void setOffset(const btVector3 &value) {
        m_offset = value;
    }

    /**
     * Set position of this bone.
     *
     * @param value position of this bone
     */
    void setPosition(const btVector3 &value) {
        m_position = value;
    }

    /**
     * Set rotation of this bone.
     *
     * @param value rotation of this bone
     */
    void setRotation(const btQuaternion &value) {
        m_rotation = value;
    }

    /**
     * Set this bone is simulated.
     *
     * @param value whether this bone is simulated
     */
    void setSimulated(bool value) {
        m_simulated = value;
    }

    /**
     * Get whether this bone is movable
     *
     * @return true if this bone is movable
     */
    bool isMovable() const {
        switch (m_type) {
        case kRotateAndMove:
        case kIKDestination:
        case kUnderIK:
            return true;
        default:
            return false;
        }
    }

    /**
     * Get whether this bone is rotateable
     *
     * @return true if this bone is rotateable
     */
    bool isRotateable() const {
        return isVisible();
    }

    /**
     * Get whether this bone is visible
     *
     * @return true if this bone is visible
     */
    bool isVisible() const {
        switch (m_type) {
        case kUnknown:
        case kIKTarget:
        case kInvisible:
        case kFollowRotate:
            return false;
        default:
            return true;
        }
    }

private:
    uint8_t m_name[kNameSize];
    uint8_t m_englishName[kNameSize];
    uint8_t m_categoryIndex;
    int16_t m_id;
    Type m_type;
    btTransform m_localTransform;
    btTransform m_transformMoveToOrigin;
    btVector3 m_originPosition;
    btVector3 m_position;
    btVector3 m_offset;
    btQuaternion m_rotation;
    float m_rotateCoef;
    Bone *m_parentBone;
    Bone *m_childBone;
    Bone *m_targetBone;
    int16_t m_parentBoneID;
    int16_t m_childBoneID;
    int16_t m_targetBoneID;
    bool m_hasParent;
    bool m_parentIsRoot;
    bool m_constraintedXCoordinateForIK;
    bool m_simulated;
    bool m_motionIndepent;

    VPVL_DISABLE_COPY_AND_ASSIGN(Bone)
};

} /* namespace vpvl */

#endif
