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

#ifndef QMATEXTRENDERENGINE_H
#define QMATEXTRENDERENGINE_H

#include <QtOpenGL>
#include <QtCore/QSize>
#include <QtCore/QStringList>
#include <QtGui/QColor>
#include <QtGui/QFont>

class QMATextRenderEngine
{
public:
    static const int kWidth = 1024;
    static const int kHeight = 256;

    QMATextRenderEngine(const QFont &font);
    ~QMATextRenderEngine();

    void setFont(const QFont &font);
    void setColor(const QColor &foreground, const QColor &background);
    void setText(const QStringList &text);
    void render();

    inline const QSize size() {
        return m_size;
    }
    inline void setEnable(bool value) {
        m_enable = value;
    }
    inline bool isEnabled() {
        return m_enable;
    }

private:
    QFont m_font;
    QColor m_foreground;
    QColor m_background;
    QStringList m_text;
    QSize m_size;
    GLuint m_textureID;
    bool m_enable;

    Q_DISABLE_COPY(QMATextRenderEngine)
};

#endif // QMATEXTRENDERENGINE_H
