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

#ifndef DEBUGDRAWER_H
#define DEBUGDRAWER_H

#include "common/SceneWidget.h"
#include "util.h"

#include <QtCore/QObject>
#include <btBulletDynamicsCommon.h>
#include <vpvl/vpvl.h>

namespace internal {

class DebugDrawer : public btIDebugDraw
{
public:
    DebugDrawer(SceneWidget *sceneWidget)
        : m_sceneWidget(sceneWidget),
          m_world(0)
    {}
    virtual ~DebugDrawer() {}

    void draw3dText(const btVector3 & /* location */, const char *textString) {
        fprintf(stderr, "[INFO]: %s\n", textString);
    }
    void drawContactPoint(const btVector3 &PointOnB, const btVector3 &normalOnB, btScalar distance, int /* lifeTime */, const btVector3 &color) {
        const btVector3 to = PointOnB + normalOnB * distance;
        glBegin(GL_LINES);
        glColor3fv(color);
        glVertex3fv(PointOnB);
        glVertex3fv(to);
        glEnd();
    }
    void drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color) {
        glBegin(GL_LINES);
        glColor3fv(color);
        glVertex3fv(from);
        glVertex3fv(to);
        glEnd();
    }
    void drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &fromColor, const btVector3 &toColor) {
        glBegin(GL_LINES);
        glColor3fv(fromColor);
        glVertex3fv(from);
        glColor3fv(toColor);
        glVertex3fv(to);
        glEnd();
    }
    void reportErrorWarning(const char *warningString) {
        fprintf(stderr, "[ERROR]: %s\n", warningString);
    }
    int getDebugMode() const {
        return m_flags;
    }
    void setDebugMode(int debugMode) {
        m_flags = debugMode;
    }

    void render() {
        if (m_world)
            m_world->debugDrawWorld();
    }
    void setWorld(btDynamicsWorld *value) {
        m_world = value;
    }

    void drawModelBones(const vpvl::PMDModel *model, bool /* drawSpheres */, bool /* drawLines */) {
        if (!model)
            return;

        glUseProgram(0);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);

        btAlignedObjectArray<btVector3> vertices;
        static const int indices[] = {
            0, 1, 5, 3, 0, 2, 5, 4, 1, 2, 3, 4
        };

        float matrix[16];
        vpvl::Transform mv = m_sceneWidget->scene()->modelViewTransform();
        mv.getOpenGLMatrix(matrix);

        const vpvl::BoneList &bones = model->bones();
        const int nbones = bones.count();
        btTransform tr = btTransform::getIdentity();
        for (int i = 0; i < nbones; i++) {
            const vpvl::Bone *bone = bones[i], *parent = bone->parent();
            if (!parent || bone->parent()->id() == 0 ||
                    !bone->isVisible() || !parent->isVisible())
                continue;
            //vpvl::Bone::Type type = bone->type();
            const btTransform &boneTransform = bone->localTransform(),
                    &parentTransform = parent->localTransform();
            const btVector3 &origin = boneTransform.getOrigin(),
                    &parentOrigin = parentTransform.getOrigin();
            glPushMatrix();
            glLoadMatrixf(matrix);
            vertices.clear();
            vertices.push_back(origin);
            btScalar s = btMin(0.25, parentOrigin.distance(origin) * 0.1);
            tr.setOrigin(btVector3(s, s, 0.0));
            vertices.push_back(tr * origin);
            tr.setOrigin(btVector3(0.0, s, s));
            vertices.push_back(tr * origin);
            tr.setOrigin(btVector3(-s, s, 0.0));
            vertices.push_back(tr * origin);
            tr.setOrigin(btVector3(0.0, s, -s));
            vertices.push_back(tr * origin);
            vertices.push_back(parentOrigin);
            glEnableClientState(GL_VERTEX_ARRAY);
            glVertexPointer(3, GL_FLOAT, sizeof(btVector3), &vertices[0]);
            glColor3f(0, 0, 1);
            glDrawElements(GL_LINE_LOOP, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_INT, indices);
            glDisableClientState(GL_VERTEX_ARRAY);
            glPopMatrix();
        }

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
    }
    void drawBoneTransform(vpvl::Bone *bone) {
        if (bone) {
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_LIGHTING);
            glPushMatrix();
            const btTransform &t = bone->localTransform();
            btScalar orthoLen = 1.0f;
            if (bone->hasParent()) {
                const btTransform &pt = bone->parent()->localTransform();
                orthoLen = btMin(orthoLen, pt.getOrigin().distance(t.getOrigin()));
            }
            drawTransform(t, orthoLen);
            glPopMatrix();
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_LIGHTING);
        }
    }
    void drawPosition(const vpvl::Vector3 &pos) {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glPushMatrix();
        drawSphere(pos, 0.25f, vpvl::Vector3(1, 0, 0));
        glPopMatrix();
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
    }

private:
    SceneWidget *m_sceneWidget;
    btDynamicsWorld *m_world;
    int m_flags;

    Q_DISABLE_COPY(DebugDrawer)
};

}

#endif // DEBUGDRAWER_H
