/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2011  hkrn                                    */
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

#ifndef DELEGATE_H
#define DELEGATE_H

#include <GL/glew.h>
#include <QtCore/QString>
#include <QtGui/QImage>

#ifdef VPVL_USE_GLSL
#include <vpvl/gl2/Renderer.h>
#else
#include <vpvl/gl/Renderer.h>
#endif

class QGLWidget;

#ifdef VPVL_USE_GLSL
class Delegate : public vpvl::gl2::IDelegate
{
public:
    explicit Delegate(QGLWidget *wiget);
    ~Delegate();

    bool loadTexture(const std::string &path, GLuint &textureID);
    bool loadToonTexture(const std::string &name, const std::string &dir, GLuint &textureID);
    void log(LogLevel level, const char *format...);
    const std::string loadShader(ShaderType type);
    const std::string toUnicode(const uint8_t *value);

private:
    static QImage loadTGA(const QString &path, uint8_t *&rawData);

    QGLWidget *m_widget;

    Q_DISABLE_COPY(Delegate)
};
#else
class Delegate : public vpvl::gl::IDelegate
{
public:
    explicit Delegate(QGLWidget *wiget);
    ~Delegate();

    bool loadTexture(const std::string &path, GLuint &textureID);
    bool loadToonTexture(const std::string &name, const std::string &dir, GLuint &textureID);
    void log(LogLevel level, const char *format...);
    const std::string toUnicode(const uint8_t *value);

private:
    static QImage loadTGA(const QString &path, uint8_t *&rawData);

    QGLWidget *m_widget;

    Q_DISABLE_COPY(Delegate)
};
#endif

#endif // DELEGATE_H
