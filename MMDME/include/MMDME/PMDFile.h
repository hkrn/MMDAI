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

#ifndef MMDME_PMDFILE_H_
#define MMDME_PMDFILE_H_

#include "MMDME/Common.h"

namespace MMDAI {

#pragma pack(push, 1)

enum PMD_BONE_TYPE {
    ROTATE,
    ROTATE_AND_MOVE,
    IK_DESTINATION,
    UNKNOWN,
    UNDER_IK,
    UNDER_ROTATE,
    IK_TARGET,
    NO_DISP,
    TWIST,
    FOLLOW_ROTATE
};

enum PMD_FACE_TYPE {
    PMD_FACE_BASE,
    PMD_FACE_EYEBROW,
    PMD_FACE_EYE,
    PMD_FACE_LIP,
    PMD_FACE_OTHER
};

typedef struct _PMDFileHeader {
    char magic[3];
    float version;
    char name[20];
    char comment[256];
} PMDFile_Header;

typedef struct _PMDFile_Vertex {
    float pos[3];
    float normal[3];
    float uv[2];
    int16_t boneID[2];
    uint8_t boneWeight1;
    uint8_t noEdgeFlag;
} PMDFile_Vertex;

typedef struct _PMDFile_Material {
    float diffuse[3];
    float alpha;
    float shiness;
    float specular[3];
    float ambient[3];
    uint8_t toonID;
    uint8_t edgeFlag;
    uint32_t numSurfaceIndex;
    char textureFile[20];
} PMDFile_Material;

typedef struct _PMDFile_Bone {
    char name[20];
    int16_t parentBoneID;
    int16_t childBoneID;
    uint8_t type;
    int16_t targetBoneID;
    float pos[3];
} PMDFile_Bone;

typedef struct _PMDFile_IK {
    int16_t destBoneID;
    int16_t targetBoneID;
    uint8_t numLink;
    uint16_t numIteration;
    float angleConstraint;
} PMDFile_IK;

typedef struct _PMDFile_Face_Vertex {
    uint32_t vertexID;
    float pos[3];
} PMDFile_Face_Vertex;

typedef struct _PMDFile_Face {
    char name[20];
    uint32_t numVertex;
    uint8_t type;
} PMDFile_Face;

typedef struct _PMDFile_RigidBody {
    char name[20];
    uint16_t boneID;
    uint8_t collisionGroupID;
    uint16_t collisionMask;
    uint8_t shapeType;
    float width;
    float height;
    float depth;
    float pos[3];
    float rot[3];
    float mass;
    float linearDamping;
    float angularDamping;
    float restitution;
    float friction;
    uint8_t type;
} PMDFile_RigidBody;

typedef struct _PMDFile_Constraint {
    char name[20];
    uint32_t bodyIDA;
    uint32_t bodyIDB;
    float pos[3];
    float rot[3];
    float limitPosFrom[3];
    float limitPosTo[3];
    float limitRotFrom[3];
    float limitRotTo[3];
    float stiffness[6];
} PMDFile_Constraint;

#pragma pack(pop)

} /* namespace */

#endif

