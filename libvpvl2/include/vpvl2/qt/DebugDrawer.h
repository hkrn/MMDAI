/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
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

#ifndef VPVL2_QT_DEBUGDRAWER_H_
#define VPVL2_QT_DEBUGDRAWER_H_

#include <vpvl2/Common.h>
#include <LinearMath/btIDebugDraw.h>

#include <QScopedPointer>
#include <QSet>
#include <QVarLengthArray>

class btDiscreteDynamicsWorld;
class btCollisionShape;
class QSettings;

namespace vpvl2
{
class IBone;
class IModel;
class IRenderContext;

namespace extensions {
class World;
namespace gl {
class ShaderProgram;
class VertexBundle;
class VertexBundleLayout;
}
}

namespace qt
{

using namespace vpvl2::extensions;
using namespace vpvl2::extensions::gl;

class VPVL2_API DebugDrawer : public btIDebugDraw
{
public:
    typedef QSet<const IBone *> BoneSet;
    static const Scalar kLength;
    static const Vector3 kRed;
    static const Vector3 kGreen;
    static const Vector3 kBlue;

    DebugDrawer(const IRenderContext *renderContextRef, QSettings *settingsRef);
    ~DebugDrawer();

    void draw3dText(const btVector3 & /* location */, const char *textString);
    void drawContactPoint(const btVector3 &PointOnB,
                          const btVector3 &normalOnB,
                          btScalar distance,
                          int /* lifeTime */,
                          const btVector3 &color);
    void drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color);
    void drawLine(const btVector3 &from,
                  const btVector3 &to,
                  const btVector3 &fromColor,
                  const btVector3 & /* toColor */);
    void reportErrorWarning(const char *warningString);
    int getDebugMode() const;
    void setDebugMode(int debugMode);

    void load();
    void setVisible(bool value);
    void drawShape(btDiscreteDynamicsWorld *world,
                   btCollisionShape *shape,
                   const btTransform &transform,
                   const btVector3 &color);
    void drawWorld(World *world, int flags = DBG_DrawWireframe | DBG_DrawAabb | DBG_DrawConstraints);

    void drawModelBones(const IModel *model, const BoneSet &selectedBones);
    void drawMovableBone(const IBone *bone, const IModel *model);
    void drawBoneTransform(const IBone *bone, const IModel *model, int mode);

private:
    class PrivateShaderProgram;
    struct Vertex {
        Vertex() {}
        Vertex(const Vector3 &v, const Vector3 &c)
            : position(v),
              color(c)
        {
        }
        Vector3 position;
        Vector3 color;
    };
    static const IBone *findSpecialBone(const IModel *model);
    void drawBone(const IBone *bone, const BoneSet &selected, const BoneSet &IK, bool skipDrawingLine);
    void beginDrawing(const IModel *model);
    void flushDrawing();
    void bindVertexBundle(bool bundle);
    void releaseVertexBundle(bool bundle);

    const IRenderContext *m_renderContextRef;
    const QSettings *m_settingsRef;
    QVarLengthArray<Vertex> m_vertices;
    QVarLengthArray<int> m_indices;
    QScopedPointer<PrivateShaderProgram> m_program;
    QScopedPointer<VertexBundle> m_bundle;
    QScopedPointer<VertexBundleLayout> m_layout;
    int m_flags;
    int m_index;
    bool m_visible;

    VPVL2_DISABLE_COPY_AND_ASSIGN(DebugDrawer)
};

} /* namespace qt */
} /* namespace vpvl2 */

#endif // DEBUGDRAWER_H
