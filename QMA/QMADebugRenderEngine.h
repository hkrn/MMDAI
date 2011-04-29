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

#ifndef QMADEBUGRENDERENGINE_H
#define QMADEBUGRENDERENGINE_H

#include <QtOpenGL>

#include <LinearMath/btIDebugDraw.h>

namespace MMDAI {
class PMDBone;
class SceneController;
}

class btConvexShape;
class btDiscreteDynamicsWorld;

class QMADebugRenderEngine : public btIDebugDraw
{
public:
    QMADebugRenderEngine(MMDAI::SceneController *controller);
    ~QMADebugRenderEngine();

    void initialize();
    void render();

    void drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color);
    void drawContactPoint(const btVector3 &PointOnB, const btVector3 &normalOnB, btScalar distance, int lifeTime, const btVector3 &color);
    void reportErrorWarning(const char *warningString);
    void draw3dText(const btVector3 &location, const char *textString);

    void setDebugMode(int debugMode) {
        m_debugMode = debugMode;
    }
    int getDebugMode() const {
        return m_debugMode;
    }
    inline void toggleRenderBones() {
        m_renderBones = !m_renderBones;
    }
    inline void toggleRenderRigidBodies() {
        m_renderRigidBodies = !m_renderRigidBodies;
    }

private:
    void drawCube();
    void drawSphere(int lats, int longs);
    void drawConvex(btConvexShape *shape);
    void renderRigidBodies();
    void renderModelBones();
    void renderModelBone(MMDAI::PMDBone *bone);

    MMDAI::SceneController *m_controller;
    btDiscreteDynamicsWorld *m_world;
    GLuint m_boxList;
    GLuint m_sphereList;
    int m_debugMode;
    bool m_boxListEnabled;
    bool m_sphereListEnabled;
    bool m_renderBones;
    bool m_renderRigidBodies;
};

#endif // QMADEBUGRENDERENGINE_H
