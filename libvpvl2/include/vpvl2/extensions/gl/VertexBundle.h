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

#pragma once
#ifndef VPVL2_EXTENSIONS_GL_VERTEXBUNDLE_H_
#define VPVL2_EXTENSIONS_GL_VERTEXBUNDLE_H_

#include <vpvl2/Common.h>
#include <vpvl2/extensions/gl/CommonMacros.h>

namespace vpvl2
{
namespace extensions
{
namespace gl
{

class VPVL2_API VertexBundle {
public:
    enum Type {
        kVertexBuffer,
        kIndexBuffer,
        kMaxVertexBufferType
    };

    VertexBundle()
        : m_indexBuffer(0)
    {
    }
    ~VertexBundle() {
        const int nbuffers = m_vertexBuffers.count();
        for (int i = 0; i < nbuffers; i++) {
            const GLuint *value = m_vertexBuffers.value(i);
            glDeleteBuffers(1, value);
        }
        release(kIndexBuffer, 0);
    }

    void create(Type value, GLuint key, GLenum usage, const void *ptr, size_t size) {
        release(value, key);
        GLuint name = 0, target = type2target(value);
        glGenBuffers(1, &name);
        glBindBuffer(target, name);
        glBufferData(target, size, ptr, usage);
        glBindBuffer(target, 0);
        switch (value) {
        case kVertexBuffer:
            m_vertexBuffers.insert(key, name);
            break;
        case kIndexBuffer:
            m_indexBuffer = name;
            break;
        case kMaxVertexBufferType:
        default:
            break;
        }
    }
    void release(Type value, GLuint key) {
        switch (value) {
        case kVertexBuffer:
            if (const GLuint *buffer = m_vertexBuffers.find(key)) {
                glDeleteBuffers(1, buffer);
                m_vertexBuffers.remove(key);
            }
            break;
        case kIndexBuffer:
            glDeleteBuffers(1, &m_indexBuffer);
            break;
        case kMaxVertexBufferType:
        default:
            break;
        }
    }
    void bind(Type value, GLuint key) {
        switch (value) {
        case kVertexBuffer:
            if (const GLuint *bufferPtr = m_vertexBuffers.find(key)) {
                GLuint buffer = *bufferPtr;
                glBindBuffer(GL_ARRAY_BUFFER, buffer);
            }
            break;
        case kIndexBuffer:
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
            break;
        case kMaxVertexBufferType:
        default:
            break;
        }
    }
    void unbind(Type value) {
        switch (value) {
        case kVertexBuffer:
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            break;
        case kIndexBuffer:
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            break;
        case kMaxVertexBufferType:
        default:
            break;
        }
    }
    void *map(Type type, size_t offset, size_t size) {
        GLuint target = type2target(type);
#ifdef GL_CHROMIUM_map_sub
        return glMapBufferSubDataCHROMIUM(target, offset, size, GL_WRITE_ONLY);
#else /* GL_CHROMIUM_map_sub */
        (void) offset;
        (void) size;
        return glMapBuffer(target, GL_WRITE_ONLY);
#endif /* GL_CHROMIUM_map_sub */
    }
    void unmap(Type type, void *address) {
#ifdef GL_CHROMIUM_map_sub
        (void) type;
        glUnmapBufferSubDataCHROMIUM(address);
#else /* GL_CHROMIUM_map_sub */
        (void) address;
        GLuint target = type2target(type);
        glUnmapBuffer(target);
#endif /* GL_CHROMIUM_map_sub */
    }
    GLuint findName(GLuint key) const {
        if (const GLuint *value = m_vertexBuffers.find(key)) {
            return *value;
        }
        return 0;
    }

private:
    static GLuint type2target(Type value) {
        switch (value) {
        case kVertexBuffer:
            return GL_ARRAY_BUFFER;
        case kIndexBuffer:
            return GL_ELEMENT_ARRAY_BUFFER;
        case kMaxVertexBufferType:
        default:
            return 0;
        }
    }

    Hash<HashInt, GLuint> m_vertexBuffers;
    GLuint m_indexBuffer;

    VPVL2_DISABLE_COPY_AND_ASSIGN(VertexBundle)
};

} /* namespace gl */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif
