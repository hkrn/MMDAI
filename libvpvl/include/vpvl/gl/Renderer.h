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

#ifndef VPVL_GL_RENDERER_H_
#define VPVL_GL_RENDERER_H_

#include <string>
#include "vpvl/Common.h"

#ifdef VPVL_USE_ALLEGRO5
#include <allegro5/allegro5.h>
#include <allegro5/allegro_opengl.h>
#include <GL/glu.h>
#else
#include <GL/glew.h>
#endif

class btDynamicsWorld;
class btIDebugDraw;

namespace vpvl
{

class PMDModel;
class Scene;
class XModel;

namespace gl
{

class VPVL_EXPORT IDelegate
{
public:
    virtual bool loadTexture(const std::string &path, GLuint &textureID) = 0;
    virtual bool loadToonTexture(const std::string &name, const std::string &dir, GLuint &textureID) = 0;
    virtual const std::string toUnicode(const uint8_t *value) = 0;
};

/**
 * @file
 * @author Nagoya Institute of Technology Department of Computer Science
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * Bone class represents a bone of a Polygon Model Data object.
 */

class VPVL_EXPORT Renderer
{
public:
    static bool initializeGLEW(GLenum &err);

    Renderer(IDelegate *delegate, int width, int height, int fps);
    virtual ~Renderer();

    vpvl::Scene *scene() const {
        return m_scene;
    }
    vpvl::PMDModel *selectedModel() const {
        return m_selected;
    }
    void setSelectedModel(vpvl::PMDModel *value) {
        m_selected = value;
    }
    bool isDisplayBones() {
        return m_displayBones;
    }
    void toggleDisplayBones() {
        m_displayBones = !m_displayBones;
    }

    const btVector4 &lightColor() const {
        return m_lightColor;
    }
    void setLightColor(const btVector4 &value) {
        m_lightColor = value;
    }
    const btVector4 &lightPosition() const {
        return m_lightPosition;
    }
    void setLightPosition(const btVector4 &value) {
        m_lightPosition = value;
    }
    btScalar lightIntensity() const {
        return m_lightIntensity;
    }
    void setLightIntensity(btScalar value) {
        m_lightIntensity = value;
    }
    btScalar ambient() const {
        return m_ambient;
    }
    void setAmbient(btScalar value) {
        m_ambient = value;
    }
    btScalar diffuse() const {
        return m_diffuse;
    }
    void setDiffuse(btScalar value) {
        m_diffuse = value;
    }
    btScalar specular() const {
        return m_specular;
    }
    void setSpecular(btScalar value) {
        m_specular = value;
    }

    btIDebugDraw *debugDrawer() const {
        return m_debugDrawer;
    }

    void initializeSurface();
    void resize(int width, int height);
    void pickBones(int px, int py, float approx, vpvl::BoneList &pickBones);
    void getObjectCoordinate(int px, int py, btVector3 &coordinate);
    void setLighting();
    void setDebugDrawer(btDynamicsWorld *world);
    void loadModel(vpvl::PMDModel *model, const std::string &dir);
    void unloadModel(const vpvl::PMDModel *model);
    void drawModel(const vpvl::PMDModel *model);
    void drawModelEdge(const vpvl::PMDModel *model);
    void drawModelShadow(const vpvl::PMDModel *model);
    void drawModelBones();
    void drawModelBones(const vpvl::PMDModel *model);
    void loadAsset(vpvl::XModel *model, const std::string &dir);
    void unloadAsset(const vpvl::XModel *model);
    void drawAsset(const vpvl::XModel *model);
    void drawSurface();

private:
    vpvl::Scene *m_scene;
    vpvl::PMDModel *m_selected;
    vpvl::gl::IDelegate *m_delegate;
    Array<vpvl::XModel *> m_assets;
    btDynamicsWorld *m_world;
    btIDebugDraw *m_debugDrawer;
    btVector4 m_lightColor;
    btVector4 m_lightPosition;
    btScalar m_lightIntensity;
    btScalar m_ambient;
    btScalar m_diffuse;
    btScalar m_specular;
    bool m_displayBones;
    int m_width;
    int m_height;

    VPVL_DISABLE_COPY_AND_ASSIGN(Renderer)
};

}
}

#endif
