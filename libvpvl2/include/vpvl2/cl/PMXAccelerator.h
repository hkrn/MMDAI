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

#pragma once
#ifndef VPVL2_CL_PMXACCELERATOR_H_
#define VPVL2_CL_PMXACCELERATOR_H_

#include "vpvl2/Scene.h"
#include "vpvl2/extensions/gl/CommonMacros.h"

namespace vpvl2
{

class IApplicationContext;
class Scene;

namespace cl
{

class VPVL2_API PMXAccelerator VPVL2_DECL_FINAL
{
public:
    struct VertexBufferBridge {
        VertexBufferBridge(extensions::gl::GLuint n) : name(n), mem(0) {}
        extensions::gl::GLuint name;
        void *mem;
    };
    typedef Array<VertexBufferBridge> VertexBufferBridgeArray;

    PMXAccelerator(const Scene *sceneRef, IApplicationContext *applicationContextRef, IModel *modelRef, Scene::AccelerationType accelerationType);
    ~PMXAccelerator();

    bool isAvailable() const;
    void upload(VertexBufferBridgeArray &buffers, const IModel::IndexBuffer *indexBufferRef);
    void update(const IModel::DynamicVertexBuffer *dynamicBufferRef, const VertexBufferBridge &buffer, Vector3 &aabbMax, Vector3 &aabbMin);
    void release(VertexBufferBridgeArray &buffers) const;

private:
    struct PrivateContext;
    PrivateContext *m_context;
};

} /* namespace cl */
} /* namespace vpvl2 */

#endif
