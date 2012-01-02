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

#include <vpvl/vpvl.h>
#include <vpvl/cg/Renderer.h>
#include <vpvl/internal/gl2.h>

namespace vpvl
{

namespace cg
{

class EffectParameters
{
public:
    EffectParameters(CGeffect effect) : m_effect(effect) {
        m_controlObject = cgGetEffectParameterBySemantic(effect, "CONTROLOBJECT");
        m_lightDirection = getParameterBySemanticsAndAnnotation("DIRECTION", "Object", "Camera", false);
        m_lightDirection = getParameterBySemanticsAndAnnotation("POSITION", "Object", "Camera", false);
        m_lightDirection = getParameterBySemanticsAndAnnotation("DIRECTION", "Object", "Light", false);
        m_lightDirection = getParameterBySemanticsAndAnnotation("POSITION", "Object", "Light", false);
        m_lightAmbient = getParameterBySemanticsAndAnnotation("AMBIENT", "Object", "Light", false);
        m_lightDiffuse = getParameterBySemanticsAndAnnotation("DIFFUSE", "Object", "Light", false);
        m_lightSpecular = getParameterBySemanticsAndAnnotation("SPECULAR", "Object", "Light", false);
        m_lightEmission = getParameterBySemanticsAndAnnotation("EMISSION", "Object", "Light", false);
        m_materialAmbient = getParameterBySemanticsAndAnnotation("AMBIENT", "Object", "Geometry", false);
        m_materialDiffuse = getParameterBySemanticsAndAnnotation("DIFFUSE", "Object", "Geometry", false);
        m_materialSpecular = getParameterBySemanticsAndAnnotation("SPECULAR", "Object", "Geometry", false);
        m_materialEmission = getParameterBySemanticsAndAnnotation("EMISSION", "Object", "Geometry", false);
        m_materialShininess = getParameterBySemanticsAndAnnotation("SHININESS", "Object", "Geometry", false);
        m_materialSphereMap = cgGetEffectParameterBySemantic(effect, "MATERIALSPHEREMAP");
        m_materialTexture = cgGetEffectParameterBySemantic(effect, "MATERIALTEXTURE");
        m_opadd = cgCreateEffectParameter(effect, "opadd", CG_BOOL);
        m_parthf = cgCreateEffectParameter(effect, "parthf", CG_BOOL);
        m_projectionMatrix = cgGetEffectParameterBySemantic(effect, "PROJECTION");
        m_spadd = cgCreateEffectParameter(effect, "spadd", CG_BOOL);
        m_subsetCount = cgCreateEffectParameter(effect, "SubsetCount", CG_INT);
        m_transp = cgCreateEffectParameter(effect, "transp", CG_BOOL);
        m_useSphereMap = cgCreateEffectParameter(effect, "use_spheremap", CG_BOOL);
        m_useTexture = cgCreateEffectParameter(effect, "use_texture", CG_BOOL);
        m_useToon = cgCreateEffectParameter(effect, "use_toon", CG_BOOL);
        m_vertexCount = cgCreateEffectParameter(effect, "VertexCount", CG_INT);
        m_viewMatrix = getParameterBySemanticsAndAnnotation("VIEW", "Object", "Camera", true);
        m_viewport = cgGetEffectParameterBySemantic(effect, "VIEWPORT");
        m_viewProjectionMatrix = getParameterBySemanticsAndAnnotation("VIEWPROJECTION", "Object", "Camera", true);
        m_worldMatrix = getParameterBySemanticsAndAnnotation("WORLD", "Object", "Camera", true);
        m_worldViewMatrix = getParameterBySemanticsAndAnnotation("WORLDVIEW", "Object", "Camera", true);
        m_worldViewProjectionMatrix = getParameterBySemanticsAndAnnotation("WORLDVIEWPROJECTION", "Object", "Camera", true);
    }
    ~EffectParameters() {
        cgDestroyEffect(m_effect);
        m_effect = 0;
    }

    void setTechniqueAnnotation(CGtechnique technique) {
        CGannotation a = cgGetFirstTechniqueAnnotation(technique);
        while (a) {
            const char *name = cgGetAnnotationName(a);
            const char *value;
            CGbool boolean;
            if (strcmp(name, "Subset") == 0) {
                value = cgGetStringAnnotationValue(a);
            }
            else if (strcmp(name, "MMDPass") == 0) {
                value = cgGetStringAnnotationValue(a);
            }
            else if (strcmp(name, "UseTexture") == 0) {
                boolean = getBoolByAnnotation(a);
            }
            else if (strcmp(name, "UseSphereMap") == 0) {
                boolean = getBoolByAnnotation(a);
            }
            else if (strcmp(name, "UseToon") == 0) {
                boolean = getBoolByAnnotation(a);
            }
        }
    }

    void setMatrix(const vpvl::Scene *scene) {
        float matrix4x4[16];
        scene->getModelViewMatrix(matrix4x4);
        setMatrixParameter(m_viewMatrix, matrix4x4);
        scene->getProjectionMatrix(matrix4x4);
        setMatrixParameter(m_projectionMatrix, matrix4x4);
    }
    void setLight(const vpvl::Scene *scene) {
        setParameter4fv(m_lightPosition, scene->lightPosition());
    }
    void setMaterial(const vpvl::Material *material, const vpvl::gl2::PMDModelMaterialPrivate &materialPrivate, bool enableToon) {
        Color average, ambient, diffuse, specular;
        float alpha = material->opacity();
        ambient = material->ambient();
        ambient.setW(ambient.w() * alpha);
        diffuse = material->diffuse();
        diffuse.setW(diffuse.w() * alpha);
        specular = material->specular();
        specular.setW(specular.w() * alpha);
        setParameter4fv(m_materialAmbient, ambient);
        setParameter4fv(m_materialDiffuse, diffuse);
        setParameter4fv(m_materialSpecular, specular);
        setParameter1f(m_materialShininess, material->shiness());
        cgGLSetTextureParameter(m_materialTexture, materialPrivate.mainTextureID);
        cgGLSetParameter1f(m_spadd, material->isMainSphereAdd() || material->isSubSphereAdd());
        cgGLSetParameter1f(m_useSphereMap, material->isMainSphereAdd() || material->isMainSphereModulate()
                           || material->isSubSphereAdd() || material->isSubSphereModulate());
        cgGLSetParameter1f(m_useTexture, materialPrivate.mainTextureID || materialPrivate.subTextureID);
        cgGLSetParameter1f(m_useToon, enableToon);
        cgGLSetParameter1f(m_parthf, 0);
        cgGLSetParameter1f(m_transp, 0);
    }
    void setViewport(vpvl::Scene *scene) {
        CGparameter p = m_viewport;
        float width = scene->width(), height = scene->height();
        while (p) {
            cgGLSetParameter2f(p, width, height);
            p = cgGetNextParameter(p);
        }
    }
    void setVertexCount(int size) {
        setParameter1f(m_vertexCount, size);
    }
    void setSubsetCount(int size) {
        setParameter1f(m_subsetCount, size);
    }

private:
    void setParameter1f(CGparameter p, float value) {
        cgGLSetParameter1f(p, value);
    }
    void setParameter4fv(CGparameter p, const float *value) {
        cgGLSetParameter4fv(p, value);
    }
    void setMatrixParameter(CGparameter p, const float *matrix) {
        cgGLSetMatrixParameterfc(p, matrix);
    }
    CGbool getBoolByAnnotation(CGannotation a) {
        int nbools = 0;
        const CGbool *bools = cgGetBoolAnnotationValues(a, &nbools);
        return nbools == 1 ? bools[0] : CG_FALSE;
    }
    CGparameter getParameterBySemanticsAndAnnotation(const char *semantics, const char *key, const char *target, bool isDefault) {
        CGparameter p = cgGetEffectParameterBySemantic(m_effect, semantics);
        CGannotation a = cgGetFirstParameterAnnotation(p);
        while (a) {
            const char *name = cgGetAnnotationName(a);
            const char *value = cgGetStringAnnotationValue(a);
            if (strcmp(name, key) == 0 && strcmp(value, target) == 0)
                return p;
            a = cgGetNextAnnotation(a);
        }
        return isDefault ? p : 0;
    }

    CGeffect m_effect;
    CGparameter m_worldMatrix;
    CGparameter m_viewMatrix;
    CGparameter m_projectionMatrix;
    CGparameter m_worldViewMatrix;
    CGparameter m_viewProjectionMatrix;
    CGparameter m_worldViewProjectionMatrix;
    CGparameter m_cameraDirection;
    CGparameter m_cameraPosition;
    CGparameter m_lightDirection;
    CGparameter m_lightPosition;
    CGparameter m_lightAmbient;
    CGparameter m_lightDiffuse;
    CGparameter m_lightSpecular;
    CGparameter m_lightEmission;
    CGparameter m_materialAmbient;
    CGparameter m_materialDiffuse;
    CGparameter m_materialSpecular;
    CGparameter m_materialEmission;
    CGparameter m_materialShininess;
    CGparameter m_materialTexture;
    CGparameter m_materialSphereMap;
    CGparameter m_viewport;
    CGparameter m_controlObject;
    CGparameter m_parthf;
    CGparameter m_spadd;
    CGparameter m_transp;
    CGparameter m_useTexture;
    CGparameter m_useSphereMap;
    CGparameter m_useToon;
    CGparameter m_opadd;
    CGparameter m_vertexCount;
    CGparameter m_subsetCount;
};

struct PMDModelUserData : vpvl::gl2::PMDModelUserData
{
    EffectParameters *parameters;
};

Renderer::Renderer(IDelegate *delegate, int width, int height, int fps)
    : vpvl::gl2::Renderer(delegate, width, height, fps),
      m_context(0)
{
    cgGLSetDebugMode(CG_TRUE);
    m_context = cgCreateContext();
}

Renderer::~Renderer()
{
    cgDestroyContext(m_context);
    m_context = 0;
}

void Renderer::uploadModel(vpvl::PMDModel *model, const std::string &dir)
{
    vpvl::cg::PMDModelUserData *userData = new vpvl::cg::PMDModelUserData();
    vpvl::cg::Renderer::IDelegate *delegate = static_cast<vpvl::cg::Renderer::IDelegate *>(m_delegate);
    std::string source;
    userData->parameters = 0;
    if (delegate->loadEffect(model, dir, source)) {
        CGeffect effect = cgCreateEffect(m_context, source.c_str(), NULL);
        if (effect) {
            userData->parameters = new EffectParameters(effect);
        }
        else {
            const char *message = cgGetErrorString(cgGetError());
            m_delegate->log(Renderer::kLogWarning,
                            "Loading an effect (%s) error: %s",
                            m_delegate->toUnicode(model->name()).c_str(), message);
        }
    }
    uploadModel0(userData, model, dir);
}

void Renderer::deleteModel(vpvl::PMDModel *&model)
{
    vpvl::cg::PMDModelUserData *userData = static_cast<vpvl::cg::PMDModelUserData *>(model->userData());
    delete userData->parameters;
    userData->parameters = 0;
    deleteModel0(userData, model);
}

void Renderer::renderModel(const vpvl::PMDModel *model)
{
    vpvl::cg::PMDModelUserData *userData = static_cast<vpvl::cg::PMDModelUserData *>(model->userData());
    userData->parameters ? renderModel0(userData, model) : vpvl::gl2::Renderer::renderModel(model);
}

void Renderer::renderModel0(const vpvl::cg::PMDModelUserData *userData, const vpvl::PMDModel *model)
{
    EffectParameters *p = userData->parameters;
    size_t stride = model->strideSize(vpvl::PMDModel::kVerticesStride), vsize = model->vertices().count();

    glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[vpvl::gl2::kModelVertices]);
    glVertexPointer(3, GL_FLOAT, vsize * stride, reinterpret_cast<const GLvoid *>(stride));
    stride = model->strideOffset(vpvl::PMDModel::kNormalsStride);
    glNormalPointer(GL_FLOAT, vsize * stride, reinterpret_cast<const GLvoid *>(stride));
    glClientActiveTexture(GL_TEXTURE0);
    stride = model->strideOffset(vpvl::PMDModel::kTextureCoordsStride);
    glTexCoordPointer(2, GL_FLOAT, vsize * stride, reinterpret_cast<const GLvoid *>(stride));

    p->setMatrix(m_scene);
    p->setLight(m_scene);

    const bool enableToon = model->isToonEnabled();
    // toon
    if (enableToon) {
        stride = model->strideOffset(vpvl::PMDModel::kToonTextureStride);
        glClientActiveTexture(GL_TEXTURE1);
        glTexCoordPointer(2, GL_FLOAT, vsize * stride, reinterpret_cast<const GLvoid *>(stride));
    }

    const vpvl::MaterialList &materials = model->materials();
    const vpvl::gl2::PMDModelMaterialPrivate *materialPrivates = userData->materials;
    const int nmaterials = materials.count();
    size_t offset = 0;
    p->setViewport(m_scene);
    p->setVertexCount(vsize);
    p->setSubsetCount(nmaterials);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, userData->vertexBufferObjects[vpvl::gl2::kShadowIndices]);
    for (int i = 0; i < nmaterials; i++) {
        const vpvl::Material *material = materials[i];
        const vpvl::gl2::PMDModelMaterialPrivate &materialPrivate = materialPrivates[i];
        const int nindices = material->countIndices();
        material->opacity() < 1.0f ? glDisable(GL_CULL_FACE) : glEnable(GL_CULL_FACE);
        p->setMaterial(material, materialPrivate, enableToon);
        glDrawElements(GL_TRIANGLES, nindices, GL_UNSIGNED_SHORT, reinterpret_cast<const GLvoid *>(offset));
        offset += (nindices << 1);
    }
    glEnable(GL_CULL_FACE);
}

}

}
