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

#include <vpvl/vpvl.h>
#include <vpvl/cg/Renderer.h>
#include <vpvl/internal/gl2.h>

namespace vpvl
{

namespace cg
{

struct EffectParameters
{
    CGparameter worldMatrix;
    CGparameter viewMatrix;
    CGparameter projectionMatrix;
    CGparameter worldViewMatrix;
    CGparameter viewProjectionMatrix;
    CGparameter worldViewProjectionMatrix;
    CGparameter ambient;
    CGparameter diffuse;
    CGparameter specular;
    CGparameter emission;
    CGparameter shininess;
    CGparameter position;
    CGparameter direction;
    CGparameter materialTexture;
    CGparameter materialSphereMap;
    CGparameter viewport;
    CGparameter controlObject;
    CGparameter parthf;
    CGparameter spadd;
    CGparameter transp;
    CGparameter useTexture;
    CGparameter useSphereMap;
    CGparameter useToon;
    CGparameter opadd;
    CGparameter vertexCount;
    CGparameter subsetCount;
};

struct PMDModelUserData : vpvl::gl2::PMDModelUserData
{
    CGeffect effect;
    EffectParameters parameters;
};

Renderer::Renderer(IDelegate *delegate, int width, int height, int fps)
    : vpvl::gl2::Renderer(delegate, width, height, fps),
      m_context(0)
{
    m_context = cgCreateContext();
}

Renderer::~Renderer()
{
    cgDestroyContext(m_context);
    m_context = 0;
}

void Renderer::loadModel(vpvl::PMDModel *model, const std::string &dir)
{
    vpvl::cg::PMDModelUserData *userData = new vpvl::cg::PMDModelUserData();
    vpvl::cg::IDelegate *delegate = static_cast<vpvl::cg::IDelegate *>(m_delegate);
    std::string source;
    userData->effect = 0;
    if (delegate->loadEffect(model, dir, source)) {
        CGeffect effect = cgCreateEffect(m_context, source.c_str(), NULL);
        if (effect) {
            EffectParameters &p = userData->parameters;
            p.ambient = cgGetEffectParameterBySemantic(effect, "AMBIENT");
            p.controlObject = cgGetEffectParameterBySemantic(effect, "CONTROLOBJECT");
            p.diffuse = cgGetEffectParameterBySemantic(effect, "DIFFUSE");
            p.direction = cgGetEffectParameterBySemantic(effect, "DIRECTION");
            p.emission = cgGetEffectParameterBySemantic(effect, "EMISSION");
            p.materialSphereMap = cgGetEffectParameterBySemantic(effect, "MATERIALSPHEREMAP");
            p.materialTexture = cgGetEffectParameterBySemantic(effect, "MATERIALTEXTURE");
            p.opadd = cgCreateEffectParameter(effect, "opadd", CG_BOOL);
            p.parthf = cgCreateEffectParameter(effect, "parthf", CG_BOOL);
            p.position = cgGetEffectParameterBySemantic(effect, "POSITION");
            p.projectionMatrix = cgGetEffectParameterBySemantic(effect, "PROJECTION");
            p.shininess = cgGetEffectParameterBySemantic(effect, "SHININESS");
            p.spadd = cgCreateEffectParameter(effect, "spadd", CG_BOOL);
            p.specular = cgGetEffectParameterBySemantic(effect, "SPECULAR");
            p.subsetCount = cgCreateEffectParameter(effect, "SubsetCount", CG_INT);
            p.transp = cgCreateEffectParameter(effect, "transp", CG_BOOL);
            p.useSphereMap = cgCreateEffectParameter(effect, "use_spheremap", CG_BOOL);
            p.useTexture = cgCreateEffectParameter(effect, "use_texture", CG_BOOL);
            p.useToon = cgCreateEffectParameter(effect, "use_toon", CG_BOOL);
            p.vertexCount = cgCreateEffectParameter(effect, "VertexCount", CG_INT);
            p.viewMatrix = cgGetEffectParameterBySemantic(effect, "VIEW");
            p.viewport = cgGetEffectParameterBySemantic(effect, "VIEWPORT");
            p.viewProjectionMatrix = cgGetEffectParameterBySemantic(effect, "VIEWPROJECTION");
            p.worldMatrix = cgGetEffectParameterBySemantic(effect, "WORLD");
            p.worldViewMatrix = cgGetEffectParameterBySemantic(effect, "WORLDVIEW");
            p.worldViewProjectionMatrix = cgGetEffectParameterBySemantic(effect, "WORLDVIEWPROJECTION");
            userData->effect = effect;
        }
    }
    loadModel0(userData, model, dir);
}

void Renderer::unloadModel(const vpvl::PMDModel *model)
{
    vpvl::cg::PMDModelUserData *userData = static_cast<vpvl::cg::PMDModelUserData *>(model->userData());
    if (userData->effect) {
        cgDestroyEffect(userData->effect);
        userData->effect = 0;
    }
    unloadModel0(userData, model);
}

void Renderer::drawModel(const vpvl::PMDModel *model)
{
    vpvl::cg::PMDModelUserData *userData = static_cast<vpvl::cg::PMDModelUserData *>(model->userData());
    userData->effect ? drawModel0(userData, model) : vpvl::gl2::Renderer::drawModel(model);
}

void Renderer::drawModel0(const vpvl::cg::PMDModelUserData *userData, const vpvl::PMDModel *model)
{
}

}

}
