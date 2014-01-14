/**

 Copyright (c) 2010-2014  hkrn

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
#ifndef VPVL2_GL_VERTEXBUNDLE_H_
#define VPVL2_GL_VERTEXBUNDLE_H_

#include <vpvl2/gl/Global.h>

namespace vpvl2
{
namespace gl
{

class VertexBundle VPVL2_DECL_FINAL {
public:
    static const GLenum kGL_STREAM_DRAW = 0x88E0;
    static const GLenum kGL_STATIC_DRAW = 0x88E4;
    static const GLenum kGL_DYNAMIC_DRAW = 0x88E8;
    static const GLenum kGL_ARRAY_BUFFER = 0x8892;
    static const GLenum kGL_ELEMENT_ARRAY_BUFFER = 0x8893;
    static const GLenum kGL_WRITE_ONLY = 0x88B9;
    static const GLenum kGL_MAP_WRITE_BIT = 0x0002;
    static const GLenum kGL_TRANSFORM_FEEDBACK_BUFFER = 0x8C8E;
    static const GLenum kGL_RASTERIZER_DISCARD = 0x8C89;
    static const GLenum kGL_INTERLEAVED_ATTRIBS = 0x8C8C;
    static const GLenum kGL_SEPARATE_ATTRIBS = 0x8C8D;
    static const GLenum kGL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN = 0x8C88;

    enum Type {
        kVertexBuffer,
        kIndexBuffer,
        kMaxVertexBufferType
    };

    VertexBundle(const IApplicationContext::FunctionResolver *resolver)
        : genBuffers(reinterpret_cast<PFNGLGENBUFFERSPROC>(resolver->resolveSymbol("glGenBuffers"))),
          bindBuffer(reinterpret_cast<PFNGLBINDBUFFERPROC>(resolver->resolveSymbol("glBindBuffer"))),
          bufferData(reinterpret_cast<PFNGLBUFFERDATAPROC>(resolver->resolveSymbol("glBufferData"))),
          bufferSubData(reinterpret_cast<PFNGLBUFFERSUBDATAPROC>(resolver->resolveSymbol("glBufferSubData"))),
          deleteBuffers(reinterpret_cast<PFNGLDELETEBUFFERSPROC>(resolver->resolveSymbol("glDeleteBuffers"))),
          bindBufferBase(0),
          transformFeedbackVaryings(0),
          getTransformFeedbackBarying(0),
          beginTransformFeedback(0),
          endTransformFeedback(0),
          genQueries(reinterpret_cast<PFNGLGENQUERIESPROC>(resolver->resolveSymbol("glGenQueries"))),
          beginQuery(reinterpret_cast<PFNGLBEGINQUERYPROC>(resolver->resolveSymbol("glBeginQuery"))),
          endQuery(reinterpret_cast<PFNGLENDQUERYPROC>(resolver->resolveSymbol("glEndQuery"))),
          getQueryObjectiv(reinterpret_cast<PFNGLGETQUERYOBJECTIVPROC>(resolver->resolveSymbol("glGetQueryObjectiv"))),
          deleteQueries(reinterpret_cast<PFNGLDELETEQUERIESPROC>(resolver->resolveSymbol("glDeleteQueries"))),
          mapBuffer(reinterpret_cast<PFNGLMAPBUFFERPROC>(resolver->resolveSymbol("glMapBuffer"))),
          unmapBuffer(reinterpret_cast<PFNGLUNMAPBUFFERPROC>(resolver->resolveSymbol("glUnmapBuffer"))),
          mapBufferRange(0),
          m_indexBuffer(0),
          m_query(0)
    {
        if (resolver->hasExtension("ARB_map_buffer_range")) {
            mapBufferRange = reinterpret_cast<PFNGLMAPBUFFERRANGEPROC>(resolver->resolveSymbol("glMapBufferRange"));
        }
        if (resolver->query(IApplicationContext::FunctionResolver::kQueryVersion) >= gl::makeVersion(3, 0)) {
            bindBufferBase = reinterpret_cast<PFNGLBINDBUFFERBASEPROC>(resolver->resolveSymbol("glBindBufferBase"));
            transformFeedbackVaryings = reinterpret_cast<PFNGLTRANSFORMFEEDBACKVARYINGSPROC>(resolver->resolveSymbol("glTransformFeedbackVaryings"));
            getTransformFeedbackBarying = reinterpret_cast<PFNGLGETTRANSFORMFEEDBACKVARYINGPROC>(resolver->resolveSymbol("glGetTransformFeedbackVarying"));
            beginTransformFeedback = reinterpret_cast<PFNGLBEGINTRANSFORMFEEDBACKPROC>(resolver->resolveSymbol("glBeginTransformFeedback"));
            endTransformFeedback = reinterpret_cast<PFNGLENDTRANSFORMFEEDBACKPROC>(resolver->resolveSymbol("glEndTransformFeedback"));
        }
        else if (resolver->hasExtension("EXT_transform_feedback")) {
            bindBufferBase = reinterpret_cast<PFNGLBINDBUFFERBASEPROC>(resolver->resolveSymbol("glBindBufferBaseEXT"));
            transformFeedbackVaryings = reinterpret_cast<PFNGLTRANSFORMFEEDBACKVARYINGSPROC>(resolver->resolveSymbol("glTransformFeedbackVaryingsEXT"));
            getTransformFeedbackBarying = reinterpret_cast<PFNGLGETTRANSFORMFEEDBACKVARYINGPROC>(resolver->resolveSymbol("glGetTransformFeedbackVaryingEXT"));
            beginTransformFeedback = reinterpret_cast<PFNGLBEGINTRANSFORMFEEDBACKPROC>(resolver->resolveSymbol("glBeginTransformFeedbackEXT"));
            endTransformFeedback = reinterpret_cast<PFNGLENDTRANSFORMFEEDBACKPROC>(resolver->resolveSymbol("glEndTransformFeedbackEXT"));
        }
    }
    ~VertexBundle() {
        const int numVertexBuffers = m_vertexBuffers.count();
        for (int i = 0; i < numVertexBuffers; i++) {
            const GLuint *value = m_vertexBuffers.value(i);
            deleteBuffers(1, value);
        }
        if (m_query) {
            deleteQueries(1, &m_query);
        }
        release(kIndexBuffer, 0);
    }

    void create(Type value, GLuint key, GLenum usage, const void *ptr, vsize size) {
        release(value, key);
        switch (value) {
        case kVertexBuffer: {
            GLenum target = type2target(value);
            m_vertexBuffers.insert(key, internalCreate(target, usage, ptr, size));
            break;
        }
        case kIndexBuffer: {
            m_indexBuffer = internalCreate(type2target(value), usage, ptr, size);
            break;
        }
        case kMaxVertexBufferType:
        default:
            break;
        }
    }
    void release(Type value, GLuint key) {
        switch (value) {
        case kVertexBuffer: {
            if (const GLuint *buffer = m_vertexBuffers.find(key)) {
                deleteBuffers(1, buffer);
                m_vertexBuffers.remove(key);
            }
            break;
        }
        case kIndexBuffer: {
            if (m_indexBuffer) {
                deleteBuffers(1, &m_indexBuffer);
                m_indexBuffer = 0;
            }
            break;
        }
        case kMaxVertexBufferType:
        default:
            break;
        }
    }
    void bind(Type value, GLuint key) {
        switch (value) {
        case kVertexBuffer: {
            if (const GLuint *bufferPtr = m_vertexBuffers.find(key)) {
                GLuint buffer = *bufferPtr;
                bindBuffer(kGL_ARRAY_BUFFER, buffer);
            }
            break;
        }
        case kIndexBuffer: {
            bindBuffer(kGL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
            break;
        }
        case kMaxVertexBufferType:
        default:
            break;
        }
    }
    void setFeedbackOutput(GLuint program, const Array<const char *> &names, GLenum type) {
        VPVL2_DCHECK(names.count() > 0);
        transformFeedbackVaryings(program, names.count(), &names[0], type);
    }
    void beginTransform(GLenum type, GLuint key) {
        if (const GLuint *bufferPtr = m_vertexBuffers.find(key)) {
            GLuint buffer = *bufferPtr;
            bindBufferBase(kGL_TRANSFORM_FEEDBACK_BUFFER, 0, buffer);
            beginTransformFeedback(type);
        }
    }
    void endTransform() {
        endTransformFeedback();
    }
    void beginFeedbackQuery() {
        if (!m_query) {
            genQueries(1, &m_query);
        }
        beginQuery(kGL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, m_query);
    }
    GLint endFeedbackQuery() {
        GLint written = 0;
        if (m_query) {
            endQuery(kGL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
            getQueryObjectiv(m_query, kGL_QUERY_RESULT, &written);
        }
        return written;
    }
    void unbind(Type type) {
        GLuint target = type2target(type);
        bindBuffer(target, 0);
    }
    void allocate(Type type, GLuint usage, vsize size, const void *data) {
        GLuint target = type2target(type);
        bufferData(target, size, data, usage);
    }
    void write(Type type, vsize offset, vsize size, const void *data) {
        GLuint target = type2target(type);
        bufferSubData(target, offset, size, data);
    }
    void *map(Type type, vsize offset, vsize size) {
        GLuint target = type2target(type);
#if defined(GL_CHROMIUM_map_sub)
        return glMapBufferSubDataCHROMIUM(target, offset, size, GL_WRITE_ONLY);
#elif defined(VPVL2_ENABLE_GLES2)
        (void) target;
        (void) offset;
        m_bytes.resize(int(size));
        return &m_bytes[0];
#else /* GL_CHROMIUM_map_sub */
        if (mapBufferRange) {
            return mapBufferRange(target, offset, size, kGL_MAP_WRITE_BIT);
        }
        else {
            return mapBuffer(target, kGL_WRITE_ONLY);
        }
#endif /* GL_CHROMIUM_map_sub */
    }
    void unmap(Type type, void *address) {
#ifdef GL_CHROMIUM_map_sub
        (void) type;
        glUnmapBufferSubDataCHROMIUM(address);
#elif defined(VPVL2_ENABLE_GLES2)
        (void) address;
        GLuint target = type2target(type);
        bufferSubData(target, 0, m_bytes.count(), &m_bytes[0]);
#else /* GL_CHROMIUM_map_sub */
        (void) address;
        GLuint target = type2target(type);
        unmapBuffer(target);
#endif /* GL_CHROMIUM_map_sub */
    }
    GLuint findName(GLuint key) const {
        if (const GLuint *value = m_vertexBuffers.find(key)) {
            return *value;
        }
        return 0;
    }
    inline void dumpFeedbackOutput(GLuint program, int nindices) const {
        char name[128];
        GLsizei length(0), size(0);
        GLenum type(0);
        for (int i = 0; i < nindices; i++) {
            getTransformFeedbackBarying(program, i, sizeof(name), &length, &size, &type, name);
            VPVL2_VLOG(1, "name=" << name << " length=" << length << " size=" << size << " type=" << type);
        }
    }

private:
    GLuint internalCreate(GLenum target, GLenum usage, const void *ptr, vsize size) {
        GLuint name;
        genBuffers(1, &name);
        bindBuffer(target, name);
        bufferData(target, size, ptr, usage);
        bindBuffer(target, 0);
        return name;
    }
    static GLuint type2target(Type value) {
        switch (value) {
        case kVertexBuffer: {
            return kGL_ARRAY_BUFFER;
        }
        case kIndexBuffer: {
            return kGL_ELEMENT_ARRAY_BUFFER;
        }
        case kMaxVertexBufferType:
        default:
            return 0;
        }
    }

    typedef void (GLAPIENTRY * PFNGLGENBUFFERSPROC) (GLsizei n, GLuint* buffers);
    typedef void (GLAPIENTRY * PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
    typedef void (GLAPIENTRY * PFNGLBUFFERDATAPROC) (GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
    typedef void (GLAPIENTRY * PFNGLBUFFERSUBDATAPROC) (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data);
    typedef void (GLAPIENTRY * PFNGLDELETEBUFFERSPROC) (GLsizei n, const GLuint* buffers);
    typedef void (GLAPIENTRY * PFNGLBINDBUFFERBASEPROC) (GLenum target, GLuint index, GLuint buffer);
    typedef void (GLAPIENTRY * PFNGLTRANSFORMFEEDBACKVARYINGSPROC) (GLuint program, GLsizei count, const GLchar * const* varyings, GLenum bufferMode);
    typedef void (GLAPIENTRY * PFNGLGETTRANSFORMFEEDBACKVARYINGPROC) (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name);
    typedef void (GLAPIENTRY * PFNGLBEGINTRANSFORMFEEDBACKPROC) (GLenum primitiveMode);
    typedef void (GLAPIENTRY * PFNGLENDTRANSFORMFEEDBACKPROC) ();
    typedef void (GLAPIENTRY * PFNGLGENQUERIESPROC) (GLsizei n, GLuint* ids);
    typedef void (GLAPIENTRY * PFNGLBEGINQUERYPROC) (GLenum target, GLuint id);
    typedef void (GLAPIENTRY * PFNGLENDQUERYPROC) (GLenum target);
    typedef void (GLAPIENTRY * PFNGLGETQUERYOBJECTIVPROC) (GLuint id, GLenum pname, GLint* params);
    typedef void (GLAPIENTRY * PFNGLDELETEQUERIESPROC) (GLsizei n, const GLuint* ids);
    typedef GLvoid* (GLAPIENTRY * PFNGLMAPBUFFERPROC) (GLenum target, GLenum access);
    typedef GLboolean (GLAPIENTRY * PFNGLUNMAPBUFFERPROC) (GLenum target);
    typedef GLvoid * (GLAPIENTRY * PFNGLMAPBUFFERRANGEPROC) (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
    PFNGLGENBUFFERSPROC genBuffers;
    PFNGLBINDBUFFERPROC bindBuffer;
    PFNGLBUFFERDATAPROC bufferData;
    PFNGLBUFFERSUBDATAPROC bufferSubData;
    PFNGLDELETEBUFFERSPROC deleteBuffers;
    PFNGLBINDBUFFERBASEPROC bindBufferBase;
    PFNGLTRANSFORMFEEDBACKVARYINGSPROC transformFeedbackVaryings;
    PFNGLGETTRANSFORMFEEDBACKVARYINGPROC getTransformFeedbackBarying;
    PFNGLBEGINTRANSFORMFEEDBACKPROC beginTransformFeedback;
    PFNGLENDTRANSFORMFEEDBACKPROC endTransformFeedback;
    PFNGLGENQUERIESPROC genQueries;
    PFNGLBEGINQUERYPROC beginQuery;
    PFNGLENDQUERYPROC endQuery;
    PFNGLGETQUERYOBJECTIVPROC getQueryObjectiv;
    PFNGLDELETEQUERIESPROC deleteQueries;
    PFNGLMAPBUFFERPROC mapBuffer;
    PFNGLUNMAPBUFFERPROC unmapBuffer;
    PFNGLMAPBUFFERRANGEPROC mapBufferRange;

    Hash<HashInt, GLuint> m_vertexBuffers;
    GLuint m_indexBuffer;
    GLuint m_query;
#ifdef VPVL2_ENABLE_GLES2
    Array<uint8_t> m_bytes;
#endif

    VPVL2_DISABLE_COPY_AND_ASSIGN(VertexBundle)
};

} /* namespace gl */
} /* namespace vpvl2 */

#endif
