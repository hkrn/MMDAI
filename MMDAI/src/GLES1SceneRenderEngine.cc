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

/* headers */

#if defined(OPENGLES1)
#include "MMDAI/MMDAI.h"
#include "MMDAI/GLES1SceneRenderEngine.h"

#define SHADOW_PCF                   /* use hardware PCF for shadow mapping */
#define SHADOW_AUTO_VIEW             /* automatically define depth frustum */
#define SHADOW_AUTO_VIEW_ANGLE 15.0f /* view angle for automatic depth frustum */

namespace MMDAI {

enum GLPMDModelBuffer {
    kEdgeVertices,
    kEdgeIndices,
    kShadowVertices,
    kShadowIndices,
    kModelVertices,
    kModelNormals,
    kModelTexCoords,
    kModelToonTexCoords,
    kModelSphereMapCoords,
    kModelSphereMap2Coords,
    kModelBufferMax,
};

class GLPMDModel : public PMDModel {
public:
    GLPMDModel(PMDRenderEngine *engine) : PMDModel(engine), m_materialVBO(NULL), m_nmaterials(0) {
    }
    ~GLPMDModel() {
        glDeleteBuffers(sizeof(m_modelVBO) / sizeof(GLuint), m_modelVBO);
        if (m_materialVBO != 0) {
            glDeleteBuffers(m_nmaterials, m_materialVBO);
            MMDAIMemoryRelease(m_materialVBO);
        }
        if (m_spheres) {
            for (unsigned int i = 0; i < m_nmaterials; i++) {
                MMDAIMemoryRelease(m_spheres[i]);
            }
            MMDAIMemoryRelease(m_spheres);
        }
        if (m_spheres2) {
            for (unsigned int i = 0; i < m_nmaterials; i++) {
                MMDAIMemoryRelease(m_spheres2[i]);
            }
            MMDAIMemoryRelease(m_spheres2);
        }
    }
    bool load(PMDModelLoader *loader, BulletPhysics *bullet) {
        if (!PMDModel::load(loader, bullet))
            return false;
        const unsigned int nvertices = getNumVertex();
        const unsigned short *surfaceData = getSurfacesPtr();
        const bool hasSingleSphereMap = this->hasSingleSphereMap();
        const bool hasMultipleSphereMap = this->hasMultipleSphereMap();
        m_nmaterials = getNumMaterial();
        m_materialVBO = static_cast<GLuint *>(MMDAIMemoryAllocate(m_nmaterials * sizeof(GLuint)));
        if (m_materialVBO == NULL)
            return false;
        m_spheres = static_cast<TexCoord **>(MMDAIMemoryAllocate(sizeof(void *) * m_nmaterials));
        if (m_spheres == NULL)
            return false;
        m_spheres2 = static_cast<TexCoord **>(MMDAIMemoryAllocate(sizeof(void *) * m_nmaterials));
        if (m_spheres2 == NULL)
            return false;
        glGenBuffers(sizeof(m_modelVBO) / sizeof(GLuint), m_modelVBO);
        glGenBuffers(m_nmaterials, m_materialVBO);
        // edge buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_modelVBO[kEdgeIndices]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, getNumSurfaceForEdge() * sizeof(GLushort), getSurfacesForEdgePtr(), GL_STATIC_DRAW);
        // shadow buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_modelVBO[kShadowIndices]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, getNumSurface() * sizeof(GLushort), getSurfacesPtr(), GL_STATIC_DRAW);
        // texture for model buffer
        glBindBuffer(GL_ARRAY_BUFFER, m_modelVBO[kModelTexCoords]);
        glBufferData(GL_ARRAY_BUFFER, getNumVertex() * sizeof(TexCoord), getTexCoordsPtr(), GL_STATIC_DRAW);
        // material indices
        for (unsigned int i = 0; i < m_nmaterials; i++) {
            PMDMaterial *m = getMaterialAt(i);
            const PMDTexture *tex = m->getTexture();
            m_spheres[i] = NULL;
            m_spheres2[i] = NULL;
            if (tex != NULL) {
                const PMDTextureNative *native = tex->getNative();
                if (native != NULL && hasSingleSphereMap && tex->isSphereMap()) {
                    TexCoord *coords = static_cast<TexCoord *>(MMDAIMemoryAllocate(sizeof(TexCoord) * nvertices));
                    if (coords != NULL) {
                        memset(coords, 0, sizeof(TexCoord) * nvertices);
                        m_spheres[i] = coords;
                    }
                }
            }
            if (hasMultipleSphereMap) {
                const PMDTexture *addtex = m->getAdditionalTexture();
                if (addtex != NULL) {
                    const PMDTextureNative *native = addtex->getNative();
                    if (native != NULL) {
                        TexCoord *coords = static_cast<TexCoord *>(MMDAIMemoryAllocate(sizeof(TexCoord) * nvertices));
                        if (coords != NULL) {
                            memset(coords, 0, sizeof(TexCoord) * nvertices);
                            m_spheres2[i] = coords;
                        }
                    }
                }
            }
            const int nsurfaces = getMaterialAt(i)->getNumSurface();
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_materialVBO[i]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, nsurfaces * sizeof(GLushort), surfaceData, GL_STATIC_DRAW);
            surfaceData += nsurfaces;
        }
        return true;
    }
    GLuint getMaterialVBOAt(GLuint index) const {
        return m_materialVBO[index];
    }
    GLuint getModelVBOAt(GLPMDModelBuffer index) const {
        return m_modelVBO[index];
    }
    TexCoord *getSphereCoordsAt(GLuint index) const {
        return m_spheres[index];
    }
    TexCoord *getSphereCoords2At(GLuint index) const {
        return m_spheres2[index];
    }
private:
    TexCoord **m_spheres;
    TexCoord **m_spheres2;
    GLuint *m_materialVBO;
    GLuint m_modelVBO[kModelBufferMax];
    GLuint m_nmaterials;
};

class GLPMDMaterial : public PMDMaterial {
public:
    GLPMDMaterial(PMDRenderEngine *engine) : PMDMaterial(engine) {
    }
private:
};

GLES1SceneRenderEngine::GLES1SceneRenderEngine(Preference *preference)
    : m_preference(preference),
    m_lightVec(btVector3(0.0f, 0.0f, 0.0f)),
    m_shadowMapAutoViewEyePoint(btVector3(0.0f, 0.0f, 0.0f)),
    m_shadowMapAutoViewRadius(0.0f),
    m_boxList(0),
    m_sphereList(0),
    m_depthTextureID(0),
    m_fboID(0),
    m_boxListEnabled(false),
    m_sphereListEnabled(false),
    m_enableShadowMapping(false),
    m_overrideModelViewMatrix(false),
    m_overrideProjectionMatrix(false),
    m_shadowMapInitialized(false)
{
}

GLES1SceneRenderEngine::~GLES1SceneRenderEngine()
{
}

PMDMaterial **GLES1SceneRenderEngine::allocateMaterials(int size)
{
    PMDMaterial **materials = reinterpret_cast<PMDMaterial **>(new GLPMDMaterial*[size]);
    for (int i = 0; i < size; i++) {
        materials[i] = new GLPMDMaterial(this);
    }
    return materials;
}

void GLES1SceneRenderEngine::releaseMaterials(PMDMaterial **materials, int size)
{
    for (int i = 0; i < size; i++) {
        delete materials[i];
    }
    delete[] materials;
}

PMDModel *GLES1SceneRenderEngine::allocateModel()
{
    return new GLPMDModel(this);
}

bool GLES1SceneRenderEngine::loadModel(PMDModel *model, PMDModelLoader *loader, BulletPhysics *bullet)
{
    return reinterpret_cast<GLPMDModel *>(model)->load(loader, bullet);
}

void GLES1SceneRenderEngine::releaseModel(PMDModel *model)
{
    delete model;
}


void GLES1SceneRenderEngine::drawCube()
{
#if 0
    static const GLfloat vertices [8][3] = {
        { -0.5f, -0.5f, 0.5f},
        { 0.5f, -0.5f, 0.5f},
        { 0.5f, 0.5f, 0.5f},
        { -0.5f, 0.5f, 0.5f},
        { 0.5f, -0.5f, -0.5f},
        { -0.5f, -0.5f, -0.5f},
        { -0.5f, 0.5f, -0.5f},
        { 0.5f, 0.5f, -0.5f}
    };

    glBegin(GL_POLYGON);
    glVertex3fv(vertices[0]);
    glVertex3fv(vertices[1]);
    glVertex3fv(vertices[2]);
    glVertex3fv(vertices[3]);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3fv(vertices[4]);
    glVertex3fv(vertices[5]);
    glVertex3fv(vertices[6]);
    glVertex3fv(vertices[7]);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3fv(vertices[1]);
    glVertex3fv(vertices[4]);
    glVertex3fv(vertices[7]);
    glVertex3fv(vertices[2]);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3fv(vertices[5]);
    glVertex3fv(vertices[0]);
    glVertex3fv(vertices[3]);
    glVertex3fv(vertices[6]);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3fv(vertices[3]);
    glVertex3fv(vertices[2]);
    glVertex3fv(vertices[7]);
    glVertex3fv(vertices[6]);
    glEnd();
    glBegin(GL_POLYGON);
    glVertex3fv(vertices[1]);
    glVertex3fv(vertices[0]);
    glVertex3fv(vertices[5]);
    glVertex3fv(vertices[4]);
    glEnd();
#endif
}

void GLES1SceneRenderEngine::drawSphere(int lats, int longs)
{
#if 0
    for (int i = 0; i <= lats; i++) {
        const double lat0 = BULLETPHYSICS_PI * (-0.5 + (double) (i - 1) / lats);
        const double z0 = sin(lat0);
        const double zr0 = cos(lat0);
        const double lat1 = BULLETPHYSICS_PI * (-0.5 + (double) i / lats);
        const double z1 = sin(lat1);
        const double zr1 = cos(lat1);

        glBegin(GL_QUAD_STRIP);
        for (int j = 0; j <= longs; j++) {
            const double lng = 2 * BULLETPHYSICS_PI * (double) (j - 1) / longs;
            const double x = cos(lng);
            const double y = sin(lng);

            glNormal3f((GLfloat)(x * zr0), (GLfloat)(y * zr0), (GLfloat)z0);
            glVertex3f((GLfloat)(x * zr0), (GLfloat)(y * zr0), (GLfloat)z0);
            glNormal3f((GLfloat)(x * zr1), (GLfloat)(y * zr1), (GLfloat)z1);
            glVertex3f((GLfloat)(x * zr1), (GLfloat)(y * zr1), (GLfloat)z1);
        }
        glEnd();
    }
#else
    lats = 0;
    longs = 0;
#endif
}

void GLES1SceneRenderEngine::drawConvex(btConvexShape *shape)
{
#if 0
    btShapeHull *hull = new btShapeHull(shape);
    hull->buildHull(shape->getMargin());

    if (hull->numTriangles () > 0) {
        int index = 0;
        const unsigned int *idx = hull->getIndexPointer();
        const btVector3 *vtx = hull->getVertexPointer();
        glBegin (GL_TRIANGLES);
        for (int i = 0; i < hull->numTriangles (); i++) {
            const int i1 = index++;
            const int i2 = index++;
            const int i3 = index++;
            btAssert(i1 < hull->numIndices () &&
                     i2 < hull->numIndices () &&
                     i3 < hull->numIndices ());

            const int index1 = idx[i1];
            const int index2 = idx[i2];
            const int index3 = idx[i3];
            btAssert(index1 < hull->numVertices () &&
                     index2 < hull->numVertices () &&
                     index3 < hull->numVertices ());
            const btVector3 v1 = vtx[index1];
            const btVector3 v2 = vtx[index2];
            const btVector3 v3 = vtx[index3];
            btVector3 normal = (v3 - v1).cross(v2 - v1);
            normal.normalize ();

            glNormal3f(normal.getX(), normal.getY(), normal.getZ());
            glVertex3f (v1.x(), v1.y(), v1.z());
            glVertex3f (v2.x(), v2.y(), v2.z());
            glVertex3f (v3.x(), v3.y(), v3.z());
        }
        glEnd ();
    }

    delete hull;
#else
    shape = NULL;
#endif
}

void GLES1SceneRenderEngine::renderRigidBodies(BulletPhysics *bullet)
{
#if 0
    GLfloat color[] = {0.8f, 0.8f, 0.0f, 1.0f};
    GLint polygonMode[2] = { 0, 0 };
    btRigidBody* body = NULL;
    btScalar m[16];
    btCollisionShape* shape = NULL;
    btVector3 halfExtent;
    btDiscreteDynamicsWorld *world = bullet->getWorld();
    const btSphereShape* sphereShape;
    float radius;
    const int numObjects = world->getNumCollisionObjects();

    /* draw in wire frame */
    glGetIntegerv(GL_POLYGON_MODE, polygonMode);
    if (polygonMode[1] != GL_LINE)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDisable(GL_TEXTURE_2D);

    for (int i = 0; i < numObjects; i++) {
        /* set color */
        color[1] = 0.8f / (float) ((i % 5) + 1) + 0.2f;
        glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
        /* draw */
        {
            body = btRigidBody::upcast(world->getCollisionObjectArray()[i]);
            body->getWorldTransform().getOpenGLMatrix(m);
            shape = body->getCollisionShape();
            glPushMatrix();
            glMultMatrixf(m);
            switch (shape->getShapeType()) {
            case BOX_SHAPE_PROXYTYPE: {
                    const btBoxShape* boxShape = static_cast<const btBoxShape*>(shape);
                    halfExtent = boxShape->getHalfExtentsWithMargin();
                    glScalef(2 * halfExtent[0], 2 * halfExtent[1], 2 * halfExtent[2]);
                    if (m_boxListEnabled) {
                        glCallList(m_boxList);
                    }
                    else {
                        m_boxList = glGenLists(1);
                        glNewList(m_boxList, GL_COMPILE);
                        drawCube();
                        glEndList();
                        m_boxListEnabled = true;
                    }
                    break;
                }
            case SPHERE_SHAPE_PROXYTYPE: {
                    sphereShape = static_cast<const btSphereShape*>(shape);
                    radius = sphereShape->getMargin(); /* radius doesn't include the margin, so draw with margin */
                    glScalef(radius, radius, radius);
                    if (m_sphereListEnabled) {
                        glCallList(m_sphereList);
                    }
                    else {
                        m_sphereList = glGenLists(1);
                        glNewList(m_sphereList, GL_COMPILE);
                        drawSphere(10, 10);
                        glEndList();
                        m_sphereListEnabled = true;
                    }
                    break;
                }
            default:
                if (shape->isConvex()) {
                    drawConvex(static_cast<btConvexShape*>(shape));
                }
            }
            glPopMatrix();
        }
    }
    if (polygonMode[1] != GL_LINE) {
        glPolygonMode(GL_FRONT_AND_BACK, polygonMode[1]);
    }
#else
    bullet = NULL;
#endif
}

void GLES1SceneRenderEngine::renderBone(PMDBone *bone)
{
    btScalar m[16];
    PMDBone *parentBone = bone->getParentBone();
    const btTransform *trans = bone->getTransform();
    const unsigned char type = bone->getType();
    const bool isSimulated = bone->isSimulated();

    /* do not draw IK target bones if the IK chain is under simulation */
    if (type == IK_TARGET && parentBone && parentBone->isSimulated())
        return;

    trans->getOpenGLMatrix(m);

    /* draw node */
    glPushMatrix();
    glMultMatrixf(m);
    if (type != NO_DISP) { /* do not draw invisible bone nodes */
        if (isSimulated) {
            /* under physics simulation */
            glColor4f(0.8f, 0.8f, 0.0f, 1.0f);
            glScalef(0.1, 0.1, 0.1);
        }
        else {
            switch (type) {
            case IK_DESTINATION:
                glColor4f(0.7f, 0.2f, 0.2f, 1.0f);
                glScalef(0.25, 0.25, 0.25);
                break;
            case UNDER_IK:
                glColor4f(0.8f, 0.5f, 0.0f, 1.0f);
                glScalef(0.15, 0.15, 0.15);
                break;
            case IK_TARGET:
                glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
                glScalef(0.15, 0.15, 0.15);
                break;
            case UNDER_ROTATE:
            case TWIST:
            case FOLLOW_ROTATE:
                glColor4f(0.0f, 0.8f, 0.2f, 1.0f);
                glScalef(0.15, 0.15, 0.15);
                break;
            default:
                if (bone->hasMotionIndependency()) {
                    glColor4f(0.0f, 1.0f, 1.0f, 1.0f);
                    glScalef(0.25, 0.25, 0.25);
                } else {
                    glColor4f(0.0f, 0.5f, 1.0f, 1.0f);
                    glScalef(0.15, 0.15, 0.15);
                }
                break;
            }
        }
        drawCube();
    }
    glPopMatrix();

    if (!parentBone || type == IK_DESTINATION)
        return;

    /* draw line from parent */
    glPushMatrix();
    if (type == NO_DISP) {
        glColor4f(0.5f, 0.4f, 0.5f, 1.0f);
    }
    else if (isSimulated) {
        glColor4f(0.7f, 0.7f, 0.0f, 1.0f);
    }
    else if (type == UNDER_IK || type == IK_TARGET) {
        glColor4f(0.8f, 0.5f, 0.3f, 1.0f);
    }
    else {
        glColor4f(0.5f, 0.6f, 1.0f, 1.0f);
    }

    const btVector3 vertices[] = {
        parentBone->getTransform()->getOrigin(),
        trans->getOrigin()
    };
    const int indices[] = {
        1, 0
            };
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, sizeof(btVector3), vertices);
    glDrawElements(GL_LINES, sizeof(indices) / sizeof(int), GL_UNSIGNED_SHORT, indices);
    glDisableClientState(GL_VERTEX_ARRAY);

    glPopMatrix();
}

void GLES1SceneRenderEngine::renderBones(PMDModel *model)
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);

    /* draw bones */
    const int nbones = model->getNumBone();
    PMDBone *bones = model->getBonesPtr();
    for (int i = 0; i < nbones; i++)
        renderBone(&bones[i]);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

static void MMDAIGLGenSphereCoords(TexCoord **ptr,
                                   const btVector3 *positions,
                                   const btVector3 *normals,
                                   const unsigned int count)
{
    TexCoord *p = *ptr;
    for (unsigned int i = 0; i <  count; i++) {
        const btVector3 position = positions[i];
        const btVector3 normal = normals[i];
        const btVector3 r = position - 2 * normal.dot(position) * normal;
        const btScalar x = r.x();
        const btScalar y = r.y();
        const btScalar z = r.z() + 1.0;
        const btScalar m = 2.0 * sqrt(x * x + y * y + z * z);
        TexCoord *coord = &p[i];
        coord->u = x / m + 0.5;
        coord->v = y / m + 0.5;
    }
}

/* needs multi-texture function on OpenGL: */
/* texture unit 0: model texture */
/* texture unit 1: toon texture for toon shading */
/* texture unit 2: additional sphere map texture, if exist */
void GLES1SceneRenderEngine::renderModel(PMDModel *ptr)
{
    GLPMDModel *model = reinterpret_cast<GLPMDModel *>(ptr);
    const btVector3 *vertices = model->getSkinnedVerticesPtr();
    const btVector3 *normals = model->getSkinnedNormalsPtr();
    const unsigned int nvertices = model->getNumVertex();
    if (!vertices || !normals)
        return;

#ifndef MMDFILES_CONVERTCOORDINATESYSTEM
    glPushMatrix();
    glScalef(1.0f, 1.0f, -1.0f); /* from left-hand to right-hand */
    glCullFace(GL_FRONT);
#endif

    /* activate texture unit 0 */
    glActiveTexture(GL_TEXTURE0);
    glClientActiveTexture(GL_TEXTURE0);

    /* set lists */
    size_t vsize = nvertices  * sizeof(btVector3);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, model->getModelVBOAt(kModelVertices));
    glBufferData(GL_ARRAY_BUFFER, vsize, model->getSkinnedVerticesPtr(), GL_DYNAMIC_DRAW);
    glVertexPointer(3, GL_FLOAT, sizeof(btVector3), NULL);
    glBindBuffer(GL_ARRAY_BUFFER, model->getModelVBOAt(kModelNormals));
    glBufferData(GL_ARRAY_BUFFER, vsize, model->getSkinnedNormalsPtr(), GL_DYNAMIC_DRAW);
    glNormalPointer(GL_FLOAT, sizeof(btVector3), NULL);

    /* set model texture coordinates to texture unit 0 */
    glClientActiveTexture(GL_TEXTURE0);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, model->getModelVBOAt(kModelTexCoords));
    glTexCoordPointer(2, GL_FLOAT, 0, NULL);

    const bool enableToon = model->getToonFlag();
    const bool hasSingleSphereMap = model->hasSingleSphereMap();
    const bool hasMultipleSphereMap = model->hasMultipleSphereMap();

    if (enableToon) {
        /* set toon texture coordinates to texture unit 1 */
        glActiveTexture(GL_TEXTURE1);
        glEnable(GL_TEXTURE_2D);
        glClientActiveTexture(GL_TEXTURE1);
        glBindBuffer(GL_ARRAY_BUFFER, model->getModelVBOAt(kModelToonTexCoords));
        if (model->isSelfShadowEnabled()) {
            /* when drawing a shadow part in shadow mapping, force toon texture coordinates to (0, 0) */
            glBufferData(GL_ARRAY_BUFFER, nvertices * sizeof(TexCoord), model->getToonTexCoordsForSelfShadowPtr(), GL_DYNAMIC_DRAW);
            glTexCoordPointer(2, GL_FLOAT, 0, NULL);
        }
        else {
            glBufferData(GL_ARRAY_BUFFER, nvertices * sizeof(TexCoord), model->getToonTexCoordsPtr(), GL_DYNAMIC_DRAW);
            glTexCoordPointer(2, GL_FLOAT, 0, NULL);
        }
        glActiveTexture(GL_TEXTURE0);
        glClientActiveTexture(GL_TEXTURE0);
    }

    /* calculate alpha value, applying model global alpha */
    const float modelAlpha = model->getGlobalAlpha();

    /* render per material */
    const int nmaterials = model->getNumMaterial();
    for (int i = 0; i < nmaterials; i++) {
        float c[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        PMDMaterial *m = model->getMaterialAt(i);
        /* set colors */
        double alpha = m->getAlpha();
        c[3] = alpha * modelAlpha;
        if (c[3] > 0.99f) {
            c[3] = 1.0f; /* clamp to 1.0 */
        }
        if (enableToon) {
            /* use averaged color of diffuse and ambient for both */
            m->copyAvgcol(c);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, &(c[0]));
            m->copySpecular(c);
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, &(c[0]));
        }
        else {
            /* use each color */
            m->copyDiffuse(c);
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, &(c[0]));
            m->copyAmbient(c);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, &(c[0]));
            m->copySpecular(c);
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, &(c[0]));
        }
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, m->getShiness());

        /* disable face culling for transparent materials */
        if (alpha < 1.0f) {
            glDisable(GL_CULL_FACE);
        }
        else {
            glEnable(GL_CULL_FACE);
        }

        /* if using multiple texture units, set current unit to 0 */
        if (enableToon || hasMultipleSphereMap) {
            glActiveTexture(GL_TEXTURE0);
        }

        const PMDTexture *tex = m->getTexture();
        if (tex != NULL) {
            /* bind model texture */
            const PMDTextureNative *native = tex->getNative();
            if (native != NULL) {
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, native->id);
                if (hasSingleSphereMap && tex->isSphereMap()) {
                    /* this is sphere map */
                    /* enable texture coordinate generation */
                    if (tex->isSphereMapAdd())
                        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
                    TexCoord *coords = model->getSphereCoordsAt(i);
                    if (coords != NULL) {
                        MMDAIGLGenSphereCoords(&coords, vertices, normals, nvertices);
                        glBindBuffer(GL_ARRAY_BUFFER, model->getModelVBOAt(kModelSphereMapCoords));
                        glBufferData(GL_ARRAY_BUFFER, nvertices * sizeof(TexCoord), coords, GL_DYNAMIC_DRAW);
                        glTexCoordPointer(2, GL_FLOAT, 0, NULL);
                    }
                }
            }
            else {
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        }
        else {
            glDisable(GL_TEXTURE_2D);
        }

        if (enableToon) {
            /* set toon texture for texture unit 1 */
            const PMDTextureNative *native = model->getToonTextureAt(m->getToonID())->getNative();
            if (native != NULL) {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, native->id);
                /* set GL_CLAMP_TO_EDGE for toon texture to avoid texture interpolation at edge */
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }
        }

        if (hasMultipleSphereMap) {
            const PMDTexture *addtex = m->getAdditionalTexture();
            if (addtex) {
                /* this material has additional sphere map texture, bind it at texture unit 2 */
                glActiveTexture(GL_TEXTURE2);
                glEnable(GL_TEXTURE_2D);
                if (addtex->isSphereMapAdd()) {
                    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
                }
                else {
                    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
                }
                const PMDTextureNative *native = addtex->getNative();
                if (native != NULL) {
                    glBindTexture(GL_TEXTURE_2D, native->id);
                    TexCoord *coords = model->getSphereCoords2At(i);
                    if (coords != NULL) {
                        MMDAIGLGenSphereCoords(&coords, vertices, normals, nvertices);
                        glClientActiveTexture(GL_TEXTURE2);
                        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
                        glBindBuffer(GL_ARRAY_BUFFER, model->getModelVBOAt(kModelSphereMap2Coords));
                        glBufferData(GL_ARRAY_BUFFER, nvertices * sizeof(TexCoord), coords, GL_DYNAMIC_DRAW);
                        glTexCoordPointer(2, GL_FLOAT, 0, NULL);
                    }
                }
                else {
                    glBindTexture(GL_TEXTURE_2D, 0);
                }
            }
            else {
                /* disable generation */
                glActiveTexture(GL_TEXTURE2);
                glDisable(GL_TEXTURE_2D);
            }
        }

        /* draw elements */
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->getMaterialVBOAt(i));
        glDrawElements(GL_TRIANGLES, m->getNumSurface(), GL_UNSIGNED_SHORT, NULL);

        /* reset some parameters */
        if (tex && tex->isSphereMap() && tex->isSphereMapAdd()) {
            if (enableToon)
                glActiveTexture(GL_TEXTURE0);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        }
    }

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    if (enableToon) {
        glClientActiveTexture(GL_TEXTURE0);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glClientActiveTexture(GL_TEXTURE1);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        if (hasMultipleSphereMap) {
            glClientActiveTexture(GL_TEXTURE2);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }
        glActiveTexture(GL_TEXTURE0);
    } else {
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        if (hasMultipleSphereMap) {
            glClientActiveTexture(GL_TEXTURE2);
            glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }
    }

    if (enableToon) {
        glActiveTexture(GL_TEXTURE1);
        glDisable(GL_TEXTURE_2D);
    }
    if (hasMultipleSphereMap) {
        glActiveTexture(GL_TEXTURE2);
        glDisable(GL_TEXTURE_2D);
    }
    glActiveTexture(GL_TEXTURE0);

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
#ifndef MMDFILES_CONVERTCOORDINATESYSTEM
    glCullFace(GL_BACK);
    glPopMatrix();
#endif
}

void GLES1SceneRenderEngine::renderEdge(PMDModel *ptr)
{
    GLPMDModel *model = reinterpret_cast<GLPMDModel *>(ptr);
    const btVector3 *vertices = model->getVerticesPtr();
    const bool enableToon = model->getToonFlag();
    const unsigned int nsurfaces = model->getNumSurfaceForEdge();
    if (!vertices || !enableToon || nsurfaces == 0)
        return;

#ifndef MMDFILES_CONVERTCOORDINATESYSTEM
    glPushMatrix();
    glScalef(1.0f, 1.0f, -1.0f);
    glCullFace(GL_BACK);
#else
    /* draw back surface only */
    glCullFace(GL_FRONT);
#endif

    /* calculate alpha value */
    const float modelAlpha = model->getGlobalAlpha();
    const float *edgeColors = model->getEdgeColors();

    glDisable(GL_LIGHTING);
    glBindBuffer(GL_ARRAY_BUFFER, model->getModelVBOAt(kEdgeVertices));
    glBufferData(GL_ARRAY_BUFFER, model->getNumVertex() * sizeof(btVector3), model->getEdgeVerticesPtr(), GL_DYNAMIC_DRAW);
    glVertexPointer(3, GL_FLOAT, sizeof(btVector3), NULL);
    glColor4f(edgeColors[0], edgeColors[1], edgeColors[2], edgeColors[3] * modelAlpha);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->getModelVBOAt(kEdgeIndices));
    glDrawElements(GL_TRIANGLES, nsurfaces, GL_UNSIGNED_SHORT, NULL);
    glDisableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_LIGHTING);

    /* draw front again */
#ifndef MMDFILES_CONVERTCOORDINATESYSTEM
    glPopMatrix();
    glCullFace(GL_FRONT);
#else
    glCullFace(GL_BACK);
#endif
}

void GLES1SceneRenderEngine::renderShadow(PMDModel *ptr)
{
    GLPMDModel *model = reinterpret_cast<GLPMDModel *>(ptr);
    const btVector3 *vertices = model->getVerticesPtr();
    if (!vertices)
        return;

    glDisable(GL_CULL_FACE);
    glEnableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, model->getModelVBOAt(kShadowVertices));
    glBufferData(GL_ARRAY_BUFFER, model->getNumVertex() * sizeof(btVector3), model->getSkinnedVerticesPtr(), GL_DYNAMIC_DRAW);
    glVertexPointer(3, GL_FLOAT, sizeof(btVector3), NULL);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->getModelVBOAt(kShadowIndices));
    glDrawElements(GL_TRIANGLES, model->getNumSurface(), GL_UNSIGNED_SHORT, NULL);
    glDisableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_CULL_FACE);
}

PMDTextureNative *GLES1SceneRenderEngine::allocateTexture(const unsigned char *data,
                                                          const int width,
                                                          const int height,
                                                          const int components)
{
    /* generate texture */
    GLuint format = 0;
    PMDTextureNative *native = new PMDTextureNative;
    glGenTextures(1, &native->id);
    glBindTexture(GL_TEXTURE_2D, native->id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    if (components == 3) {
        format = GL_RGB;
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    } else {
        format = GL_RGBA;
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    }
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    return native;
}

void GLES1SceneRenderEngine::releaseTexture(PMDTextureNative *native)
{
    if (native != NULL) {
        glDeleteTextures(1, &native->id);
        delete native;
    }
}

void GLES1SceneRenderEngine::renderModelCached(PMDModel *model, PMDRenderCacheNative **ptr)
{
    model = NULL;
    *ptr = NULL;
}

void GLES1SceneRenderEngine::renderTileTexture(PMDTexture *texture,
                                               const float *color,
                                               const float *normal,
                                               const float *vertices1,
                                               const float *vertices2,
                                               const float *vertices3,
                                               const float *vertices4,
                                               const float nX,
                                               const float nY,
                                               const bool cullFace,
                                               PMDRenderCacheNative **ptr)
{
    *ptr = NULL;

    /* register rendering command */
    if (!cullFace)
        glDisable(GL_CULL_FACE);

    glEnable(GL_TEXTURE_2D);
    glPushMatrix();
#if 0
    glNormal3f(normal[0], normal[1], normal[2]);
    glBindTexture(GL_TEXTURE_2D, texture->getNative()->id);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0, nY);
    glVertex3fv(vertices1);
    glTexCoord2f(nX, nY);
    glVertex3fv(vertices2);
    glTexCoord2f(nX, 0.0);
    glVertex3fv(vertices3);
    glTexCoord2f(0.0, 0.0);
    glVertex3fv(vertices4);
    glEnd();
#else
    const float *vertices[] = {
        vertices1,
        vertices2,
        vertices3,
        vertices4
    };
    const float *normals[] = {
        normal,
        normal,
        normal,
        normal
    };
    const float coords[] = {
        0.0, nY,
        nX, nY,
        nX, 0.0,
        0.0, 0.0
    };
    const int indices[] = {
        0, 1, 2, 2, 1, 3
            };
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glBindTexture(GL_TEXTURE_2D, texture->getNative()->id);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, color);
    glVertexPointer(3, GL_FLOAT, 0, vertices);
    glNormalPointer(GL_FLOAT, 0, normals);
    glTexCoordPointer(2, GL_FLOAT, 0, coords);
    glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(int), GL_UNSIGNED_SHORT, indices);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
#endif
    glPopMatrix();
    glDisable(GL_TEXTURE_2D);

    if (!cullFace)
        glEnable(GL_CULL_FACE);
}

void GLES1SceneRenderEngine::deleteCache(PMDRenderCacheNative **ptr)
{
    PMDRenderCacheNative *native = *ptr;
    if (native != NULL) {
        delete native;
        *ptr = 0;
    }
}

/* setup: initialize and setup Renderer */
bool GLES1SceneRenderEngine::setup()
{
    /* set clear color */
    float campusColor[3];
    m_preference->getFloat3(kPreferenceCampusColor, campusColor);
    glClearColor(campusColor[0], campusColor[1], campusColor[2], 0.0f);
    glClearStencil(0);

    /* enable depth test */
    glEnable(GL_DEPTH_TEST);

    /* enable texture */
    glEnable(GL_TEXTURE_2D);

    /* enable face culling */
    glEnable(GL_CULL_FACE);
    /* not Renderer the back surface */
    glCullFace(GL_BACK);

    /* enable alpha blending */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    /* enable alpha test, to avoid zero-alpha surfaces to depend on the Renderering order */
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GEQUAL, 0.05f);

    /* enable lighting */
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);

    /* initialization for shadow mapping */
    setShadowMapping();

    return true;
}

/* GLES1SceneRenderEngine::initializeShadowMap: initialize OpenGL for shadow mapping */
void GLES1SceneRenderEngine::initializeShadowMap()
{
}

/* GLES1SceneRenderEngine::setShadowMapping: switch shadow mapping */
void GLES1SceneRenderEngine::setShadowMapping()
{
}

void GLES1SceneRenderEngine::prerender(PMDObject **objects,
                                       int size)
{
    (void) objects;
    (void) size;
    /* clear Renderering buffer */
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

/* GLES1SceneRenderEngine::render: Render all */
void GLES1SceneRenderEngine::render(PMDObject **objects, int size, Stage *stage)
{
    int i = 0;

    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);

    /* set model viwe matrix */
    applyModelViewMatrix();

    /* stage and shadhow */
    glPushMatrix();
    /* background */
    stage->renderBackground();
    /* enable stencil */
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 1, ~0);
    /* make stencil tag true */
    glStencilOp(GL_KEEP, GL_KEEP , GL_REPLACE);
    /* Renderer floor */
    stage->renderFloor();
    /* Renderer shadow stencil */
    glColorMask(0, 0, 0, 0) ;
    glDepthMask(0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 1, ~0);
    /* increment 1 pixel stencil */
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
    /* Render model */
    glDisable(GL_DEPTH_TEST);
    for (i = 0; i < size; i++) {
        PMDObject *object = objects[i];
        if (!object->isEnable())
            continue;
        glPushMatrix();
        glMultMatrixf(stage->getShadowMatrix());
        renderShadow(object->getPMDModel());
        glPopMatrix();
    }
    glEnable(GL_DEPTH_TEST);
    glColorMask(1, 1, 1, 1);
    glDepthMask(1);
    /* if stencil is 2, Renderer shadow with blend on */
    glStencilFunc(GL_EQUAL, 2, ~0);
    glDisable(GL_LIGHTING);
    glColor4f(0.1f, 0.1f, 0.1f, m_preference->getFloat(kPreferenceShadowMappingSelfDensity));
    glDisable(GL_DEPTH_TEST);
    stage->renderFloor();
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_LIGHTING);
    glPopMatrix();

    /* Render model */
    for (i = 0; i < size; i++) {
        PMDObject *object = objects[i];
        if (!object->isEnable())
            continue;
        PMDModel *model = object->getPMDModel();
        renderModel(model);
        renderEdge(model);
    }

}

/* GLES1SceneRenderEngine::pickModel: pick up a model at the screen position */
int GLES1SceneRenderEngine::pickModel(PMDObject **objects,
                                      int size,
                                      int x,
                                      int y,
                                      int width,
                                      int height,
                                      double scale,
                                      int *allowDropPicked)
{
#if 0
    int i;

    GLuint selectionBuffer[512];
    GLint viewport[4];

    GLint hits;
    GLuint *data;
    GLuint minDepth = 0, minDepthAllowDrop = 0;
    int minID, minIDAllowDrop;
    GLuint depth;
    int id;

    /* get current viewport */
    glGetIntegerv(GL_VIEWPORT, viewport);
    /* set selection buffer */
    glSelectBuffer(512, selectionBuffer);
    /* begin selection mode */
    glRenderMode(GL_SELECT);
    /* save projection matrix */
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    /* set projection matrix for picking */
    glLoadIdentity();
    /* apply picking matrix */
    gluPickMatrix(x, viewport[3] - y, 15.0, 15.0, viewport);
    /* apply normal projection matrix */
    applyProjectionMatrix(width, height, scale);
    /* switch to model view mode */
    glMatrixMode(GL_MODELVIEW);
    /* initialize name buffer */
    glInitNames();
    glPushName(0);

    /* draw models with selection names */
    for (i = 0; i < size; i++) {
        PMDObject *object = objects[i];
        if (!object->isEnable())
            continue;
        glLoadName(i);
        renderShadow(object->getPMDModel());
    }

    /* restore projection matrix */
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    /* switch to model view mode */
    glMatrixMode(GL_MODELVIEW);
    /* end selection mode and get number of hits */
    hits = glRenderMode(GL_RENDER);
    if (hits == 0)
        return -1;

    data = &(selectionBuffer[0]);
    minID = -1;
    minIDAllowDrop = -1;
    for (i = 0; i < hits; i++) {
        depth = *(data + 1);
        id = *(data + 3);
        if (minID == -1 || minDepth > depth) {
            minDepth = depth;
            minID = id;
        }
        if (allowDropPicked && objects[id]->allowMotionFileDrop()) {
            if (minIDAllowDrop == -1 || minDepthAllowDrop > depth) {
                minDepthAllowDrop = depth;
                minIDAllowDrop = id;
            }
        }
        data += *data + 3;
    }
    if (allowDropPicked)
        *allowDropPicked = minIDAllowDrop;

    return minID;
#else
    objects = NULL;
    size = 0;
    x = 0;
    y = 0;
    width = 0;
    height = 0;
    scale = 0.0f;
    allowDropPicked = NULL;
    return -1;
#endif
}

/* GLES1SceneRenderEngine::updateLigithing: update light */
void GLES1SceneRenderEngine::updateLighting()
{
    const float lightIntensity = m_preference->getFloat(kPreferenceLightIntensity);
    float lightColor[3];
    float lightDirection[4];
    float lightDiffuse[4];
    float lightSpecular[4];
    float lightAmbient[4];
    int i = 0;
    float diffuse = 0, ambinet = 0, specular = 0;

    m_preference->getFloat3(kPreferenceLightColor, lightColor);
    m_preference->getFloat4(kPreferenceLightDirection, lightDirection);
    if (!m_preference->getBool(kPreferenceUseMMDLikeCartoon)) {
        /* MMDAgent original cartoon */
        diffuse = 0.2f;
        ambinet = lightIntensity * 2.0f;
        specular = 0.4f;
    } else if (m_preference->getBool(kPreferenceUseCartoonRendering)) {
        /* like MikuMikuDance */
        diffuse = 0.0f;
        ambinet = lightIntensity * 2.0f;
        specular = lightIntensity;
    } else {
        /* no toon */
        diffuse = lightIntensity;
        ambinet = 1.0f;
        specular = 1.0f; /* OpenGL default */
    }

    for (i = 0; i < 3; i++)
        lightDiffuse[i] = lightColor[i] * diffuse;
    lightDiffuse[3] = 1.0f;
    for (i = 0; i < 3; i++)
        lightAmbient[i] = lightColor[i] * ambinet;
    lightAmbient[3] = 1.0f;
    for (i = 0; i < 3; i++)
        lightSpecular[i] = lightColor[i] * specular;
    lightSpecular[3] = 1.0f;
    
    glLightfv(GL_LIGHT0, GL_POSITION, lightDirection);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
    
    /* update light direction vector */
    m_lightVec = btVector3(lightDirection[0], lightDirection[1], lightDirection[2]);
    m_lightVec.normalize();
}

/* GLES1SceneRenderEngine::updateProjectionMatrix: update view information */
void GLES1SceneRenderEngine::updateProjectionMatrix(const int width,
                                                    const int height,
                                                    const double scale)
{
    glViewport(0, 0, width, height);
    /* camera setting */
    glMatrixMode(GL_PROJECTION);
    applyProjectionMatrix(width, height, scale);
    glMatrixMode(GL_MODELVIEW);
}

static void MMDAIGLFrustum(float *result, float left, float right, float bottom, float top, float near, float far)
{
    const float a = (right + left) / (right - left);
    const float b = (top + bottom) / (top - bottom);
    const float c = ((far + near) / (far - near)) * -1;
    const float d = ((-2 * far * near) / (far - near));
    const float e = (2 * near) / (right - left);
    const float f = (2 * near) / (top - bottom);
    const float matrix[16] = {
        e, 0, 0, 0,
        0, f, 0, 0,
        a, b, c, -1,
        0, 0, d, 0
    };
    memcpy(result, matrix, sizeof(matrix));
}

/* GLES1SceneRenderEngine::applyProjectionMatirx: update projection matrix */
void GLES1SceneRenderEngine::applyProjectionMatrix(const int width,
                                                   const int height,
                                                   const double scale)
{
    if (width == 0 || height == 0)
        return;
    if (m_overrideProjectionMatrix) {
        glLoadMatrixf(m_newProjectionMatrix);
        m_overrideProjectionMatrix = false;
    }
    else {
        const float aspect = (float) height / (float) width;
        const float ratio = (scale == 0.0) ? 1.0f : 1.0f / scale;
        float result[16];
        glLoadIdentity();
        MMDAIGLFrustum(result, - ratio, ratio, - aspect * ratio, aspect * ratio, RENDER_VIEWPOINT_FRUSTUM_NEAR, RENDER_VIEWPOINT_FRUSTUM_FAR);
        glMultMatrixf(result);
    }
}

void GLES1SceneRenderEngine::applyModelViewMatrix()
{
    glLoadIdentity();
    if (m_overrideModelViewMatrix) {
        glLoadMatrixf(m_newModelViewMatrix);
        m_overrideModelViewMatrix = false;
    }
    else {
        glMultMatrixf(m_rotMatrix);
    }
}

void GLES1SceneRenderEngine::updateModelViewMatrix(const btTransform &transMatrix,
                                                   const btTransform &transMatrixInv)
{
    transMatrix.getOpenGLMatrix(m_rotMatrix);
    transMatrixInv.getOpenGLMatrix(m_rotMatrixInv);
}

void GLES1SceneRenderEngine::setModelViewMatrix(const btScalar modelView[16])
{
    m_overrideModelViewMatrix = true;
    memcpy(m_newModelViewMatrix, modelView, sizeof(m_newModelViewMatrix));
}

void GLES1SceneRenderEngine::setProjectionMatrix(const btScalar projection[16])
{
    m_overrideProjectionMatrix = true;
    memcpy(m_newProjectionMatrix, projection, sizeof(m_newProjectionMatrix));
}

void GLES1SceneRenderEngine::setShadowMapAutoView(const btVector3 &eyePoint,
                                                  const float radius)
{
    m_shadowMapAutoViewEyePoint = btVector3(eyePoint);
    m_shadowMapAutoViewRadius = radius;
}

} /* namespace */

#endif

