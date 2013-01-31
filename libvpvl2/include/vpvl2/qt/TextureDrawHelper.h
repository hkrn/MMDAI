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
#ifndef VPVL2_QT_TEXTUREDRAWHELPER_H_
#define VPVL2_QT_TEXTUREDRAWHELPER_H_

#include <vpvl2/IModel.h>
#include <vpvl2/qt/Common.h>

#include <QDir>
#include <QSize>
#include <QRect>
#include <QVector3D>

namespace vpvl2
{
namespace extensions {
namespace gl {
class VertexBundle;
class VertexBundleLayout;
}
}
class IRenderContext;

namespace qt
{
using namespace extensions::gl;

class VPVL2QTCOMMON_API TextureDrawHelper
{
public:
    TextureDrawHelper(const QSize &size);
    ~TextureDrawHelper();

    void load(const QDir &dir, const QRectF &baseTexCoord = QRectF(0.0, 0.0, 1.0, -1.0));
    void resize(const QSize &size);
    void draw(const QRectF &rect, intptr_t textureID);
    void draw(const QRect &rect, const QVector3D &pos, intptr_t textureID);
    void draw(const QRectF &rect, const QVector3D &pos, intptr_t textureID);
    QSize size() const;
    bool isAvailable() const;

private:
    class PrivateShaderProgram;
    void setVertices2D(const QRectF &rect, QVector2D *vertices2D) const;
    void updateVertexBuffer(const QRectF &rect);
    void bindVertexBundleLayout(bool bundle);
    void unbindVertexBundleLayout(bool bundle);

    QScopedPointer<PrivateShaderProgram> m_program;
    QScopedPointer<VertexBundle> m_bundle;
    QScopedPointer<VertexBundleLayout> m_layout;
    QSize m_size;
    bool m_linked;
};

} /* namespace qt */
} /* namespace vpvl2 */

#endif // VPVL2_QT_TEXTUREDRAWHELPER_H
