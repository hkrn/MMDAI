/**

 Copyright (c) 2010-2013  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#include "vpvl2/vpvl2.h"
#include "vpvl2/IApplicationContext.h"

#include "vpvl2/extensions/gl/Global.h"
#include "vpvl2/nvfx/Effect.h"
#include "vpvl2/nvfx/EffectContext.h"
#include "vpvl2/internal/util.h"

#include "../vendor/nvFX/GL/glew.h"

/* prevent compile error */
#ifndef GLhandleARB
#define GLhandleARB void *
#endif
#include <FxParser.h>

using namespace vpvl2;
using namespace vpvl2::extensions;

namespace {

VPVL2_DECL_TLS static bool g_initialized = false;

static void AppendShaderHeader(nvFX::IContainer *container, const IApplicationContext::FunctionResolver *resolver)
{
    static const char kAppendingShaderHeader[] =
            "#if defined(GL_ES) || __VERSION__ >= 150\n"
            "precision highp float;\n"
            "#else\n"
            "#define highp\n"
            "#define mediump\n"
            "#define lowp\n"
            "#endif\n"
            "#if __VERSION__ >= 130\n"
            "#define vpvl2FXGetTexturePixel2D(samp, uv) texture(samp, (uv))\n"
            "#else\n"
            "#define vpvl2FXGetTexturePixel2D(samp, uv) texture2D(samp, (uv))\n"
            "#define layout(expr)\n"
            "#endif\n"
            "#if __VERSION__ >= 400\n"
            "#define vpvl2FXFMA(v, m, a) fma((v), (m), (a))\n"
            "#else\n"
            "#define vpvl2FXFMA(v, m, a) ((v) * (m) + (a))\n"
            "#endif\n"
            "#define vpvl2FXSaturate(v) clamp((v), 0.0, 1.0)\n"
            ;
    char appendingHeader[1024];
    if (resolver->query(IApplicationContext::FunctionResolver::kQueryCoreProfile) != 0) {
        static const char kFormat[] = "#version %d core\n%s";
        internal::snprintf(appendingHeader, sizeof(appendingHeader), kFormat, resolver->query(IApplicationContext::FunctionResolver::kQueryShaderVersion), kAppendingShaderHeader);
    }
    else {
        static const char kFormat[] = "#version 120\n%s";
        internal::snprintf(appendingHeader, sizeof(appendingHeader), kFormat, kAppendingShaderHeader);
    }
    int i = 0;
    while (nvFX::IShader *shader = container->findShader(i++)) {
        nvFX::TargetType type = shader->getType();
        const char *name = shader->getName();
        if (*name == '\0' && type == nvFX::TGLSL) {
            shader->getExInterface()->addHeaderCode(appendingHeader);
        }
    }
}

static void handleErrorCallback(const char *message)
{
    VPVL2_LOG(WARNING, message);
}

static void handleMessageCallback(const char *message)
{
    VPVL2_LOG(INFO, message);
}

static void discardsMessageCallback(const char * /* message */)
{
}

struct FunctionResolverProxy : nvFX::FunctionResolver {
    FunctionResolverProxy(const IApplicationContext::FunctionResolver *resolver)
        : m_resolver(resolver)
    {
    }
    ~FunctionResolverProxy() {
    }

    bool hasExtension(const char *name) const {
        return m_resolver->hasExtension(name);
    }
    void *resolve(const char *name) const {
        return m_resolver->resolveSymbol(name);
    }
    int queryVersion() const {
        return m_resolver->query(IApplicationContext::FunctionResolver::kQueryVersion);
    }

    const IApplicationContext::FunctionResolver *m_resolver;
};

}

namespace vpvl2
{
namespace nvfx
{

bool EffectContext::initializeGLEW(const IApplicationContext::FunctionResolver *resolver)
{
    if (!g_initialized) {
        FunctionResolverProxy proxy(resolver);
        nvFX::initializeOpenGLFunctions(&proxy);
        g_initialized = true;
    }
    return g_initialized;
}

void EffectContext::enableMessageCallback()
{
    nvFX::setErrorCallback(handleErrorCallback);
    nvFX::setMessageCallback(handleMessageCallback);
}

void EffectContext::disableMessageCallback()
{
    nvFX::setErrorCallback(discardsMessageCallback);
    nvFX::setMessageCallback(discardsMessageCallback);
}

EffectContext::EffectContext()
{
    enableMessageCallback();
}

EffectContext::~EffectContext()
{
}

void EffectContext::getEffectArguments(const IApplicationContext * /* applicationContextRef */, Array<const char *> &arguments)
{
    arguments.clear();
}

IEffect *EffectContext::compileFromFile(const IString *pathRef, IApplicationContext *applicationContextRef)
{
    nvFX::IContainer *container = 0;
    if (pathRef) {
        container = nvFX::IContainer::create();
        if (nvFX::loadEffectFromFile(container, internal::cstr(pathRef, 0))) {
            AppendShaderHeader(container, applicationContextRef->sharedFunctionResolverInstance());
            return new nvfx::Effect(this, applicationContextRef, container);
        }
    }
    return 0;
}

IEffect *EffectContext::compileFromSource(const IString *source, IApplicationContext *applicationContextRef)
{
    nvFX::IContainer *container = 0;
    if (source) {
        container = nvFX::IContainer::create();
        if (nvFX::loadEffect(container, internal::cstr(source, 0))) {
            AppendShaderHeader(container, applicationContextRef->sharedFunctionResolverInstance());
            return new nvfx::Effect(this, applicationContextRef, container);
        }
    }
    return 0;
}

void *EffectContext::internalContext() const
{
    return 0;
}

} /* namespace nvfx */
} /* namespace vpvl2 */
