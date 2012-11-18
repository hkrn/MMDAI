/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2012  hkrn                                    */
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

#ifndef VPVL2_QT_VERTEXARRAY_H_
#define VPVL2_QT_VERTEXARRAY_H_

#include <QtOpenGL/QtOpenGL>

namespace vpvl2
{
namespace qt
{

class VertexBundle {
public:
    VertexBundle()
        : glBindVertexArrayProcPtrRef(0),
          glDeleteVertexArraysProcPtrRef(0),
          glGenVertexArraysProcPtrRef(0),
          m_name(0)
    {
    }
    ~VertexBundle() {
        if (glDeleteVertexArraysProcPtrRef) {
            glDeleteVertexArraysProcPtrRef(1, &m_name);
            m_name = 0;
        }
    }

    void initialize(const QGLContext *context) {
#ifdef __APPLE__
        static const char *kBindVertexArray[] = {
            "glBindVertexArrayAPPLE",
            0
        };
        static const char *kDeleteVertexArrays[] = {
            "glDeleteVertexArraysAPPLE",
            0
        };
        static const char *kGenVertexArrays[] = {
            "glGenVertexArraysAPPLE",
            0
        };
#else
        static const char *kBindVertexArray[] = {
            "glBindVertexArray",
            0
        };
        static const char *kDeleteVertexArrays[] = {
            "glDeleteVertexArrays",
            0
        };
        static const char *kGenVertexArrays[] = {
            "glGenVertexArrays",
            0
        };
#endif
        glBindVertexArrayProcPtrRef = reinterpret_cast<glBindVertexArrayProcPtr>(
                    findProcAddress(context, kBindVertexArray));
        glDeleteVertexArraysProcPtrRef = reinterpret_cast<glDeleteVertexArraysProcPtr>(
                    findProcAddress(context, kDeleteVertexArrays));
        glGenVertexArraysProcPtrRef = reinterpret_cast<glGenVertexArraysProcPtr>(
                    findProcAddress(context, kGenVertexArrays));
    }

    bool bind() {
        if (glBindVertexArrayProcPtrRef) {
            glBindVertexArrayProcPtrRef(m_name);
            return true;
        }
        return false;
    }
    bool create() {
        if (glGenVertexArraysProcPtrRef) {
            glGenVertexArraysProcPtrRef(1, &m_name);
            return true;
        }
        return false;
    }
    bool release() {
        if (glBindVertexArrayProcPtrRef) {
            glBindVertexArrayProcPtrRef(0);
            return true;
        }
        return false;
    }

private:
    void *findProcAddress(const QGLContext *context, const char **candidates) const {
        const char *candidate = candidates[0];
        int i = 0;
        while (candidate) {
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
            void *address = reinterpret_cast<void *>(context->getProcAddress(candidate));
#else
            void *address = context->getProcAddress(candidate);
#endif
            if (address) {
                return address;
            }
            candidate = candidates[++i];
        }
        return 0;
    }

    typedef void (*glBindVertexArrayProcPtr)(GLuint id);
    typedef void (*glDeleteVertexArraysProcPtr)(GLsizei n, const GLuint *ids);
    typedef void (*glGenVertexArraysProcPtr)(GLsizei n, GLuint *ids);
    glBindVertexArrayProcPtr glBindVertexArrayProcPtrRef;
    glDeleteVertexArraysProcPtr glDeleteVertexArraysProcPtrRef;
    glGenVertexArraysProcPtr glGenVertexArraysProcPtrRef;
    GLuint m_name;
};

} /* namespace qt */
} /* namespace vpvl2 */

#endif // VPVL2_QT_VERTEXARRAY_H_
