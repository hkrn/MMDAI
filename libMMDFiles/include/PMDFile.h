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

#ifndef MMDAI_PMDFILE_H_
#define MMDAI_PMDFILE_H_

/* disable alignment in this header */
#pragma pack(push, 1)

/* PMD_BONE_TYPE: bone type */
enum PMD_BONE_TYPE {
   ROTATE,          /* normal bone, can rotate by motion */
   ROTATE_AND_MOVE, /* normal bone, can rotate and move by motion */
   IK_DESTINATION,  /* IK bone, can move by motion */
   UNKNOWN,         /* unknown */
   UNDER_IK,        /* controlled under an IK chain */
   UNDER_ROTATE,    /* controlled by other bone: rotation copied from target bone */
   IK_TARGET,       /* IK target bone, directly manipulated by IK toward an IK destination */
   NO_DISP,         /* no display (bone end markers) */
   TWIST,           /* allow twisting rotation only */
   FOLLOW_ROTATE    /* follow the child bone's rotation by the specified rate */
};

/* PMD_FACE_TYPE: face type */
enum PMD_FACE_TYPE {
   PMD_FACE_BASE,    /* base face, defining all the default vertex positions controlled by the face motions */
   PMD_FACE_EYEBROW, /* eyebrow faces */
   PMD_FACE_EYE,     /* eye faces */
   PMD_FACE_LIP,     /* lip faces */
   PMD_FACE_OTHER    /* other faces */
};

/* PMDFile_Header: header */
typedef struct _PMDFileHeader {
   char magic[3];     /* magic string, should be "Pmd" */
   float version;     /* version number */
   char name[20];     /* model name */
   char comment[256]; /* model comment string */
} PMDFile_Header;

/* PMDFile_Vertex: vertex element */
typedef struct _PMDFile_Vertex {
   float pos[3];              /* position (x, y, z) */
   float normal[3];           /* normal vector (x, y, z) */
   float uv[2];               /* texture coordinate (u, v) */
   short boneID[2];           /* bone IDs for skinning */
   unsigned char boneWeight1; /* weight for boneID[0], (1-boneWeight1) for boneID[1] */
   unsigned char noEdgeFlag;  /* 1 if NO edge should be drawn for this vertex */
} PMDFile_Vertex;

/* PMDFile_Material: material element */
typedef struct _PMDFile_Material {
   float diffuse[3];              /* diffuse color */
   float alpha;                   /* alpha color */
   float shiness;                 /* shiness intensity */
   float specular[3];             /* specular color */
   float ambient[3];              /* ambient color */
   unsigned char toonID;          /* toon index: 0xff -> toon0.bmp, other -> toon(val+1).bmp */
   unsigned char edgeFlag;        /* 1 if edge should be drawn */
   unsigned int numSurfaceIndex;  /* number of surface indices for this material */
   char textureFile[20];          /* texture file name */
} PMDFile_Material;

/* PMDFile_Bone: bone element */
typedef struct _PMDFile_Bone {
   char name[20];      /* bone name */
   short parentBoneID; /* parent bone ID (-1 = none) */
   short childBoneID;  /* child bone ID (-1 = none) */
   unsigned char type; /* bone type (PMD_BONE_TYPE) */
   short targetBoneID; /* bone ID by which this bone if affected: IK bone (type 4), under_rotate bone (type 5) or co-rotate coef value (type 9) */
   float pos[3];       /* position from origin */
} PMDFile_Bone;

/* PMDFile_IK: IK element */
typedef struct _PMDFile_IK {
   short destBoneID;            /* bone of destination position */
   short targetBoneID;          /* bone to be directly manipulated by this IK */
   unsigned char numLink;       /* length of bone list affected by this IK */
   unsigned short numIteration; /* number of maximum iteration (IK value 1) */
   float angleConstraint;       /* maximum angle per IK step in radian (IK value 2) */
} PMDFile_IK;

/* PMDFile_Face_Vertex: face vertex element */
typedef struct _PMDFile_Face_Vertex {
   unsigned int vertexID; /* vertex index of this model to be controlled */
   /* if base face, this is index for model vertex index */
   /* if not base, this is index for base face vertices */
   float pos[3];           /* position to be placed if this face rate is 1.0 */
} PMDFile_Face_Vertex;

/* PMDFile_Face: face element */
typedef struct _PMDFile_Face {
   char name[20];           /* name of this face */
   unsigned int numVertex;  /* number of vertices controlled by this face */
   unsigned char type;      /* face type (PMD_FACE_TYPE) */
} PMDFile_Face;

/* PMDFile_RigidBody: Bullet Physics RigidBody element */
typedef struct _PMDFile_RigidBody {
   char name[20];                  /* name of this rigid body (unused) */
   unsigned short boneID;          /* related bone */
   unsigned char collisionGroupID; /* collision group in which this body belongs to */
   unsigned short collisionMask;   /* collision group mask */
   unsigned char shapeType;        /* shape type (0: sphere, 1: box, 2: capsule) */
   float width;                    /* width of this shape (valid for 0, 1, 2) */
   float height;                   /* height of this shape (valid for 1, 2) */
   float depth;                    /* depth of this shape (valied for 1) */
   float pos[3];                   /* position (x, y, z), relative to related bone */
   float rot[3];                   /* rotation (x, y, z), in radian */
   float mass;                     /* weight (valid for type 1 and 2) */
   float linearDamping;            /* linear damping coefficient */
   float angularDamping;           /* angular damping coefficient */
   float restitution;              /* restitution (recoil) coefficient */
   float friction;                 /* friction coefficient */
   unsigned char type;             /* type (0: kinematics, 1: simulated, 2: simulated+aligned) */
} PMDFile_RigidBody;

/* Bulletphysics Constraint element */
typedef struct _PMDFile_Constraint {
   char name[20];         /* name of this constraint (unused) */
   unsigned int bodyIDA;  /* ID of body A */
   unsigned int bodyIDB;  /* ID of body B */
   float pos[3];          /* position (x, y, z), relative to related bone */
   float rot[3];          /* rotation (x, y, z), in radian */
   float limitPosFrom[3]; /* position move limit from (x, y, z) */
   float limitPosTo[3];   /* position move limit to (x, y, z) */
   float limitRotFrom[3]; /* rotation angular limit from (x, y, z) (rad) */
   float limitRotTo[3];   /* rotation angular limit to (x, y, z) (rad) */
   float stiffness[6];    /* spring stiffness (x,y,z,rx,ry,rz) */
} PMDFile_Constraint;

/* restore alignment */
#pragma pack(pop)

#endif

