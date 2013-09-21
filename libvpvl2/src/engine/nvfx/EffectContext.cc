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
#include "vpvl2/nvfx/Effect.h"
#include "vpvl2/nvfx/EffectContext.h"
#include "vpvl2/internal/util.h"

/* prevent compile error */
#ifndef GLhandleARB
#define GLhandleARB void *
#endif
#include <FxParser.h>

namespace vpvl2
{
namespace nvfx
{

static void handleErrorCallback(const char *message)
{
    VPVL2_LOG(WARNING, message);
}

static void handleMessageCallback(const char *message)
{
    VPVL2_LOG(INFO, message);
}

EffectContext::EffectContext()
{
    nvFX::setErrorCallback(handleErrorCallback);
    nvFX::setMessageCallback(handleMessageCallback);
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
