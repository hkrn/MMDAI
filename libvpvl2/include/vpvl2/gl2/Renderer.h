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

#ifndef VPVL2_GL2_RENDERER_H_
#define VPVL2_GL2_RENDERER_H_

#include <string>
#include "vpvl2/Common.h"

#ifdef VPVL2_LINK_ASSIMP
#include <aiMaterial.h>
#include <aiScene.h>
#endif /* VPVL_LINK_ASSIMP */

#ifdef VPVL2_LINK_QT
#include <QtOpenGL/QtOpenGL>
#endif /* VPVL_LINK_QT */

#ifdef VPVL2_BUILD_IOS
#include <OpenGLES/ES2/gl.h>
#else
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/CGLCurrent.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif /* __APPLE__ */
#endif /* VPVL_BUILD_IOS */

class btDynamicsWorld;
class btIDebugDraw;

namespace vpvl2
{

class Asset;

namespace pmx
{
class Model;
}

namespace gl2
{

class Accelerator;
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

#ifdef VPVL2_LINK_QT
class VPVL2_API Renderer : protected QGLFunctions
#else
class VPVL2_API Renderer
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
    enum KernelType {
        kModelSkinningKernel
    };

    class VPVL2_API IDelegate
    {
    public:
        virtual bool uploadTexture(const std::string &path, const std::string &dir, GLuint &textureID, bool isToon) = 0;
        virtual bool uploadToonTexture(int sharedTextureIndex, GLuint &textureID) = 0;
        virtual void log(LogLevel level, const char *format, ...) = 0;
        virtual const std::string loadShader(ShaderType type) = 0;
        virtual const std::string loadKernel(KernelType type) = 0;
        virtual const std::string toUnicode(const uint8_t *value) = 0;
        virtual const std::string toUnicode(const IString *value) = 0;
    };

    Renderer(IDelegate *delegate, int width, int height, int fps);
    virtual ~Renderer();

    bool createShaderPrograms();
    void initializeSurface();
    void resize(int width, int height);
    void uploadModel(pmx::Model *model, const std::string &dir);
    void deleteModel(pmx::Model *&model);
    void updateAllModel();
    void updateModel(pmx::Model *model);
    void renderModel(const pmx::Model *model);
    void renderModelEdge(const pmx::Model *model);
    void renderModelShadow(const pmx::Model *model);

    void clear();
    void renderAllModels();
    void renderProjectiveShadow();

    void setModelViewMatrix(const float value[16]);
    void setProjectionMatrix(const float value[16]);
    void setLightColor(const Vector3 &value);
    void setLightPosition(const Vector3 &value);

    static bool isAcceleratorSupported();
    bool isAcceleratorAvailable() const;
    bool initializeAccelerator();

protected:
    void uploadModel0(pmx::Model::UserData *userData, pmx::Model *model, const std::string &dir);

    IDelegate *m_delegate;
    Array<Asset *> m_assets;

private:
    Array<pmx::Model *> m_models;
    ModelProgram *m_modelProgram;
    ShadowProgram *m_shadowProgram;
    Accelerator *m_accelerator;
    Vector3 m_lightColor;
    Vector3 m_lightPosition;
    float m_modelViewMatrix[16];
    float m_projectionMatrix[16];

    VPVL2_DISABLE_COPY_AND_ASSIGN(Renderer)
};

}
}

#endif

