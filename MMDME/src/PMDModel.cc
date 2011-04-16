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

const float PMDModel::kMinBoneWeight = 0.0001f;
const float PMDModel::kMinFaceWeight = 0.001f;

const float PMDModel::kEdgeColorR = 0.0f;
const float PMDModel::kEdgeColorG = 0.0f;
const float PMDModel::kEdgeColorB = 0.0f;
const float PMDModel::kEdgeColorA = 1.0f;

/* PMDModel::initialize: initialize PMDModel */
void PMDModel::initialize()
{
    m_name = NULL;
    m_comment = NULL;
    m_bulletPhysics = NULL;

    m_vertexList = NULL;
    m_normalList = NULL;
    m_texCoordList = NULL;
    m_bone1List = NULL;
    m_bone2List = NULL;
    m_boneWeight1 = NULL;
    m_noEdgeFlag = NULL;
    m_surfaceList = NULL;
    m_material = NULL;
    m_boneList = NULL;
    m_IKList = NULL;
    m_faceList = NULL;
    m_rigidBodyList = NULL;
    m_constraintList = NULL;

    m_boneSkinningTrans = NULL;
    m_skinnedVertexList = NULL;
    m_skinnedNormalList = NULL;
    m_toonTexCoordList = NULL;
    m_edgeVertexList = NULL;
    m_surfaceListForEdge = NULL;
    m_toonTexCoordListForShadowMap = NULL;
    m_rotateBoneIDList = NULL;
    m_IKSimulated = NULL;

    m_numFaceDisplayNames = 0;
    m_faceDisplayNames = NULL;
    m_numBoneFrameNames = 0;
    m_boneFrameNames = NULL;
    m_numBoneDisplayNames = 0;
    m_boneDisplayIndices = NULL;
    m_boneDisplayNames = NULL;

    /* initial values for variables that should be kept at model change */
    m_enableSimulation = true;
    m_toon = false;
    m_globalAlpha = 1.0f;
    m_edgeOffset = 0.03f;
    m_selfShadowDrawing = false;
    m_selfShadowDensityCoef = 0.0f;
    m_edgeColor[0] = kEdgeColorR;
    m_edgeColor[1] = kEdgeColorG;
    m_edgeColor[2] = kEdgeColorB;
    m_edgeColor[3] = kEdgeColorA;
    m_rootBone.reset();
}

/* PMDModel::clear: free PMDModel */
void PMDModel::clear()
{
    delete [] m_vertexList;
    m_vertexList = NULL;
    delete [] m_normalList;
    m_normalList = NULL;
    MMDAIMemoryRelease(m_texCoordList);
    m_texCoordList = NULL;
    MMDAIMemoryRelease(m_bone1List);
    m_bone1List = NULL;
    MMDAIMemoryRelease(m_bone2List);
    m_bone2List = NULL;
    MMDAIMemoryRelease(m_boneWeight1);
    m_boneWeight1 = NULL;
    MMDAIMemoryRelease(m_noEdgeFlag);
    m_noEdgeFlag = NULL;
    MMDAIMemoryRelease(m_surfaceList);
    delete [] m_boneList;
    m_boneList = NULL;
    delete [] m_IKList;
    m_IKList = NULL;
    delete [] m_faceList;
    m_faceList = NULL;
    delete [] m_constraintList;
    m_constraintList = NULL;
    delete [] m_rigidBodyList;
    m_rigidBodyList = NULL;
    delete [] m_boneSkinningTrans;
    m_boneSkinningTrans = NULL;
    delete [] m_skinnedVertexList;
    m_skinnedVertexList = NULL;
    delete [] m_skinnedNormalList;
    m_skinnedNormalList = NULL;
    MMDAIMemoryRelease(m_toonTexCoordList);
    m_toonTexCoordList = NULL;
    delete [] m_edgeVertexList;
    m_edgeVertexList = NULL;
    MMDAIMemoryRelease(m_surfaceListForEdge);
    m_surfaceListForEdge = NULL;
    MMDAIMemoryRelease(m_toonTexCoordListForShadowMap);
    m_toonTexCoordListForShadowMap = NULL;
    MMDAIMemoryRelease(m_rotateBoneIDList);
    m_rotateBoneIDList = NULL;
    MMDAIMemoryRelease(m_IKSimulated);
    m_IKSimulated = NULL;
    MMDAIMemoryRelease(m_faceDisplayNames);
    m_faceDisplayNames = NULL;
    MMDAIMemoryRelease(m_boneFrameNames);
    m_boneFrameNames = NULL;
    MMDAIMemoryRelease(m_boneDisplayIndices);
    m_boneDisplayIndices = NULL;
    MMDAIMemoryRelease(m_boneDisplayNames);
    m_boneDisplayNames = NULL;
    MMDAIMemoryRelease(m_comment);
    m_comment = NULL;
    MMDAIMemoryRelease(m_name);
    m_name = NULL;

    if (m_material && m_engine) {
        m_engine->releaseMaterials(m_material, m_numMaterial);
        m_material = NULL;
    }

    for (uint32_t i = 0; i < kNSystemTextureFiles; i++)
        m_localToonTexture[i].release();
    m_name2bone.release();
    m_name2face.release();
}

/* PMDModel::PMDModel: constructor */
PMDModel::PMDModel(PMDRenderEngine *engine)
    : m_engine(engine)
{
    initialize();
}

/* PMDModel::~PMDModel: destructor */
PMDModel::~PMDModel()
{
    clear();
}

/* PMDModel::load: load from file name */
bool PMDModel::load(PMDModelLoader *loader, BulletPhysics *bullet)
{
    bool ret = parse(loader, bullet);
    return ret;
}

/* PMDModel::getBone: find bone data by name */
PMDBone *PMDModel::getBone(const char *name)
{
    PMDBone *match = static_cast<PMDBone *>(m_name2bone.findNearest(name));

    if (match && MMDAIStringEquals(match->getName(), name))
        return match;
    else
        return NULL;
}

/* PMDModel::getFace: find face data by name */
PMDFace *PMDModel::getFace(const char *name)
{
    PMDFace *match = static_cast<PMDFace *>(m_name2face.findNearest(name));

    if (match && MMDAIStringEquals(match->getName(), name))
        return match;
    else
        return NULL;
}

/* PMDModel::getChildBoneList: return list of child bones, in decent order */
int PMDModel::getChildBoneList(PMDBone **bone, uint16_t boneNum, PMDBone **childBoneList, uint16_t childBoneNumMax)
{
    uint16_t i, j;
    PMDBone *b;
    int n, k, l;
    bool updated;
    int iFrom, iTo;

    for (i = 0, n = 0; i < boneNum; i++) {
        b = bone[i];
        for (j = 0; j < m_numBone; j++) {
            if (m_boneList[j].getParentBone() == b) {
                if (n >= childBoneNumMax)
                    return -1;
                childBoneList[n] = &(m_boneList[j]);
                n++;
            }
        }
    }

    updated = true;
    iFrom = 0;
    iTo = n;
    while (updated) {
        updated = false;
        for (k = iFrom; k < iTo; k++) {
            b = childBoneList[k];
            for (j = 0; j < m_numBone; j++) {
                if (m_boneList[j].getParentBone() == b) {
                    for (l = 0; l < n; l++)
                        if (childBoneList[l] == &(m_boneList[j]))
                            break;
                    if (l < n)
                        continue;
                    if (n >= childBoneNumMax)
                        return -1;
                    childBoneList[n] = &(m_boneList[j]);
                    n++;
                    updated = true;
                }
            }
        }
        iFrom = iTo;
        iTo = n;
    }

    return n;
}

/* PMDModel::setPhysicsControl switch bone control by physics simulation */
void PMDModel::setPhysicsControl(bool flag)
{
    uint32_t i;

    m_enableSimulation = flag;
    /* when true, align all rigid bodies to corresponding bone by setting Kinematics flag */
    /* when false, all rigid bodies will have their own motion states according to the model definition */
    for (i = 0; i < m_numRigidBody; i++)
        m_rigidBodyList[i].setKinematic(!flag);
}

/* PMDModel::release: free PMDModel */
void PMDModel::release()
{
    clear();
}

} /* namespace */

