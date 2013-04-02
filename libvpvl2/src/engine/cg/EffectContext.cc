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

#include "vpvl2/vpvl2.h"
#include "vpvl2/IRenderContext.h"
#include "vpvl2/cg/Effect.h"
#include "vpvl2/cg/EffectContext.h"
#include "vpvl2/internal/util.h"

#include <Cg/cgGL.h>

namespace {

static CGbool VPVL2CGFXSetStateDisable(CGstateassignment value, int compare, GLenum name)
{
    int nvalues;
    if (const CGbool *values = cgGetBoolStateAssignmentValues(value, &nvalues)) {
        if (values[0] == compare)
            glDisable(name);
    }
    return CG_TRUE;
}

static CGbool VPVL2CGFXSetStateEnable(CGstateassignment value, int compare, GLenum name)
{
    int nvalues;
    if (const CGbool *values = cgGetBoolStateAssignmentValues(value, &nvalues)) {
        if (values[0] == compare)
            glEnable(name);
    }
    return CG_TRUE;
}

static CGbool VPVL2CGFXAlphaBlendEnableSet(CGstateassignment value)
{
    return VPVL2CGFXSetStateDisable(value, CG_FALSE, GL_BLEND);
}

static CGbool VPVL2CGFXAlphaBlendEnableReset(CGstateassignment value)
{
    return VPVL2CGFXSetStateEnable(value, CG_FALSE, GL_BLEND);
}

static CGbool VPVL2CGFXAlphaTestEnableSet(CGstateassignment value)
{
    return VPVL2CGFXSetStateEnable(value, GL_TRUE, GL_ALPHA_TEST);
}

static CGbool VPVL2CGFXAlphaTestEnableReset(CGstateassignment value)
{
    return VPVL2CGFXSetStateDisable(value, GL_TRUE, GL_ALPHA_TEST);
}

static CGbool VPVL2CGFXBlendFuncSet(CGstateassignment value)
{
    int nvalues;
    if (const int *values = cgGetIntStateAssignmentValues(value, &nvalues)) {
        if (nvalues == 2 && values[0] != GL_SRC_ALPHA && values[1] != GL_ONE_MINUS_SRC_ALPHA)
            glBlendFunc(values[0], values[1]);
    }
    return CG_TRUE;
}

static CGbool VPVL2CGFXBlendFuncReset(CGstateassignment value)
{
    int nvalues;
    if (const int *values = cgGetIntStateAssignmentValues(value, &nvalues)) {
        if (nvalues == 2 && values[0] != GL_SRC_ALPHA && values[1] != GL_ONE_MINUS_SRC_ALPHA)
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    return CG_TRUE;
}

static CGbool VPVL2CGFXCullFaceSet(CGstateassignment value)
{
    int nvalues;
    if (const int *values = cgGetIntStateAssignmentValues(value, &nvalues)) {
        if (values[0] != GL_BACK)
            glCullFace(values[0]);
    }
    return CG_TRUE;
}

static CGbool VPVL2CGFXCullFaceReset(CGstateassignment value)
{
    int nvalues;
    if (const int *values = cgGetIntStateAssignmentValues(value, &nvalues)) {
        if (values[0] != GL_BACK)
            glCullFace(GL_BACK);
    }
    return CG_TRUE;
}

static CGbool VPVL2CGFXCullFaceEnableSet(CGstateassignment value)
{
    return VPVL2CGFXSetStateDisable(value, CG_FALSE, GL_CULL_FACE);
}

static CGbool VPVL2CGFXCullFaceEnableReset(CGstateassignment value)
{
    return VPVL2CGFXSetStateEnable(value, CG_FALSE, GL_CULL_FACE);
}

static CGbool VPVL2CGFXDepthTestEnableSet(CGstateassignment value)
{
    return VPVL2CGFXSetStateDisable(value, CG_FALSE, GL_DEPTH_TEST);
}

static CGbool VPVL2CGFXDepthTestEnableReset(CGstateassignment value)
{
    return VPVL2CGFXSetStateEnable(value, CG_FALSE, GL_DEPTH_TEST);
}

static CGbool VPVL2CGFXZWriteEnableSet(CGstateassignment value)
{
    int nvalues;
    if (const CGbool *values = cgGetBoolStateAssignmentValues(value, &nvalues)) {
        if (values[0] == CG_FALSE)
            glDepthMask(GL_FALSE);
    }
    return CG_TRUE;
}

static CGbool VPVL2CGFXZWriteEnableReset(CGstateassignment value)
{
    int nvalues;
    if (const CGbool *values = cgGetBoolStateAssignmentValues(value, &nvalues)) {
        if (values[0] == CG_FALSE)
            glDepthMask(GL_TRUE);
    }
    return CG_TRUE;
}

}

namespace vpvl2
{
namespace cg
{

EffectContext::EffectContext()
    : m_context(cgCreateContext())
{
    cgSetParameterSettingMode(m_context, CG_DEFERRED_PARAMETER_SETTING);
    cgGLSetDebugMode(CG_FALSE);
    cgGLSetManageTextureParameters(m_context, CG_TRUE);
    cgGLRegisterStates(m_context);
    /* override state callbacks to override state default parameters */
    CGstate alphaBlendEnableState = cgGetNamedState(m_context, "AlphaBlendEnable");
    cgSetStateCallbacks(alphaBlendEnableState, VPVL2CGFXAlphaBlendEnableSet, VPVL2CGFXAlphaBlendEnableReset, 0);
    CGstate alphaTestEnableState = cgGetNamedState(m_context, "AlphaTestEnable");
    cgSetStateCallbacks(alphaTestEnableState, VPVL2CGFXAlphaTestEnableSet, VPVL2CGFXAlphaTestEnableReset, 0);
    CGstate blendFuncState = cgGetNamedState(m_context, "BlendFunc");
    cgSetStateCallbacks(blendFuncState, VPVL2CGFXBlendFuncSet, VPVL2CGFXBlendFuncReset, 0);
    CGstate cullFaceState = cgGetNamedState(m_context, "CullFace");
    CGstate cullModeState = cgGetNamedState(m_context, "CullMode");
    cgSetStateCallbacks(cullFaceState, VPVL2CGFXCullFaceSet, VPVL2CGFXCullFaceReset, 0);
    cgSetStateCallbacks(cullModeState, VPVL2CGFXCullFaceSet, VPVL2CGFXCullFaceReset, 0);
    CGstate cullFaceEnableState = cgGetNamedState(m_context, "CullFaceEnable");
    cgSetStateCallbacks(cullFaceEnableState, VPVL2CGFXCullFaceEnableSet, VPVL2CGFXCullFaceEnableReset, 0);
    CGstate depthTestEnableState = cgGetNamedState(m_context, "DepthTestEnable");
    CGstate zenableState = cgGetNamedState(m_context, "ZEnable");
    cgSetStateCallbacks(depthTestEnableState, VPVL2CGFXDepthTestEnableSet, VPVL2CGFXDepthTestEnableReset, 0);
    cgSetStateCallbacks(zenableState, VPVL2CGFXDepthTestEnableSet, VPVL2CGFXDepthTestEnableReset, 0);
    CGstate zwriteEnableState = cgGetNamedState(m_context, "ZWriteEnable");
    cgSetStateCallbacks(zwriteEnableState, VPVL2CGFXZWriteEnableSet, VPVL2CGFXZWriteEnableReset, 0);
}

EffectContext::~EffectContext()
{
    m_compilerArguments.releaseAll();
    cgDestroyContext(m_context);
    m_context = 0;
}

void EffectContext::getEffectArguments(const IRenderContext *renderContext, Array<const char *> &arguments)
{
    m_compilerArguments.releaseAll();
    renderContext->getEffectCompilerArguments(m_compilerArguments);
    arguments.clear();
    const int narguments = m_compilerArguments.count();
    for (int i = 0; i < narguments; i++) {
        if (IString *s = m_compilerArguments[i]) {
            arguments.append(internal::cstr(s));
        }
    }
    const char constVPVM[] = "-DVPVM";
    arguments.append(constVPVM);
    static const char constVersion[] = "-DVPVL2_VERSION=" VPVL2_VERSION_STRING;
    arguments.append(constVersion);
    arguments.append(0);
}

IEffect *EffectContext::compileFromFile(const IString *pathRef, IRenderContext *renderContextRef)
{
    CGeffect effect = 0;
    if (pathRef) {
        Array<const char *> arguments;
        getEffectArguments(renderContextRef, arguments);
        effect = cgCreateEffectFromFile(m_context, internal::cstr(pathRef), &arguments[0]);
    }
    return new cg::Effect(this, renderContextRef, effect);
}

IEffect *EffectContext::compileFromSource(const IString *source, IRenderContext *renderContextRef)
{
    CGeffect effect = 0;
    if (source) {
        Array<const char *> arguments;
        getEffectArguments(renderContextRef, arguments);
        effect = cgCreateEffect(m_context, internal::cstr(source), &arguments[0]);
    }
    return new cg::Effect(this, renderContextRef, effect);
}

CGcontext EffectContext::internalContext() const
{
    return m_context;
}

} /* namespace cg */
} /* namespace vpvl2 */
