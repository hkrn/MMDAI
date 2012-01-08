/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2012  hkrn                                    */
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

#ifndef VPVL_GL2_RENDERER_H_
#define VPVL_GL2_RENDERER_H_

#include <string>
#include "vpvl/Common.h"

#ifdef VPVL_LINK_ASSIMP
#include <aiMaterial.h>
#include <aiScene.h>
#endif

#ifdef VPVL_LINK_QT
#include <QtOpenGL/QtOpenGL>
#else
#ifdef VPVL_BUILD_IOS
#include <OpenGLES/ES2/gl.h>
#else
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif /* __APPLE__ */
#endif /* VPVL_BUILD_IOS */
#endif /* VPVL_LINK_QT */

class btDynamicsWorld;
class btIDebugDraw;

namespace vpvl
{

class Asset;
class Bone;
class PMDModel;
class Project;
class Scene;

namespace gl2
{

class AssetProgram;
class EdgeProgram;
class ModelProgram;
class ObjectProgram;
class ShadowProgram;
class ZPlotProgram;

/**
 * @file
 * @author Nagoya Institute of Technology Department of Computer Science
 * @author hkrn
 *
 * @section DESCRIPTION
 *
 * Bone class represents a bone of a Polygon Model Data object.
 */

#ifdef VPVL_LINK_QT
class VPVL_API Renderer : protected QGLFunctions
#else
class VPVL_API Renderer
#endif
{
public:
    enum LogLevel {
        kLogInfo,
        kLogWarning
    };
    enum ShaderType {
        kEdgeVertexShader,
        kEdgeFragmentShader,
        kModelVertexShader,
        kModelFragmentShader,
        kAssetVertexShader,
        kAssetFragmentShader,
        kShadowVertexShader,
        kShadowFragmentShader,
        kZPlotVertexShader,
        kZPlotFragmentShader
    };
    class VPVL_API IDelegate
    {
    public:
        virtual bool uploadTexture(const std::string &path, GLuint &textureID, bool isToon) = 0;
        virtual bool uploadToonTexture(const std::string &name, const std::string &dir, GLuint &textureID) = 0;
        virtual void log(LogLevel level, const char *format, ...) = 0;
        virtual const std::string loadShader(ShaderType type) = 0;
        virtual const std::string toUnicode(const uint8_t *value) = 0;
    };

    Renderer(IDelegate *delegate, int width, int height, int fps);
    virtual ~Renderer();

    vpvl::Scene *scene() const {
        return m_scene;
    }
    vpvl::PMDModel *selectedModel() const {
        return m_selected;
    }
    void setSelectedModel(PMDModel *value) {
        m_selected = value;
    }

    bool createPrograms();
    void initializeSurface();
    bool createShadowFrameBuffers();
    void resize(int width, int height);
    void uploadModel(PMDModel *model, const std::string &dir);
    void deleteModel(PMDModel *&model);
    void updateAllModel();
    void updateModel(PMDModel *model);
    void renderModel(const PMDModel *model);
    void renderModelEdge(const PMDModel *model);
    void renderModelShadow(const PMDModel *model);
    void renderModelZPlot(const PMDModel *model);
    void renderAsset(const Asset *asset);
    void uploadAsset(Asset *asset, const std::string &dir);
    void deleteAsset(Asset *&asset);
    void releaseProject(Project *project);

    void clear();
    void renderAllAssets();
    void renderAllModels();
    void renderProjectiveShadow();
    void renderZPlot();

protected:
    void uploadModel0(PMDModel::UserData *userData, PMDModel *model, const std::string &dir);
    void uploadAsset0(Asset::UserData *userData, Asset *asset, const std::string &dir);

    IDelegate *m_delegate;
    vpvl::Scene *m_scene;
    Array<vpvl::Asset *> m_assets;

private:
#ifdef VPVL_LINK_ASSIMP
    void uploadAssetRecurse(const aiScene *scene, const aiNode *node, Asset::UserData *userData);
    void deleteAssetRecurse(const aiScene *scene, const aiNode *node, Asset::UserData *userData);
    void renderAssetRecurse(const aiScene *scene, const aiNode *node, const Asset *asset);
    void setAssetMaterial(const aiMaterial *material, const vpvl::Asset *asset, AssetProgram *program);
#endif

    EdgeProgram *m_edgeProgram;
    ModelProgram *m_modelProgram;
    ShadowProgram *m_shadowProgram;
    ZPlotProgram *m_zplotProgram;
    vpvl::PMDModel *m_selected;
    GLuint m_depthTextureID;
    GLuint m_frameBufferID;

    VPVL_DISABLE_COPY_AND_ASSIGN(Renderer)
};

}
}

#endif

