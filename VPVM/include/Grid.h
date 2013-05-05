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

#ifndef VPVM_GRID_H_
#define VPVM_GRID_H_

#include "SceneLoader.h"

#include <vpvl2/Common.h>
#include <vpvl2/IModel.h>
#include <vpvl2/Scene.h>
#include <vpvl2/extensions/gl/VertexBundle.h>
#include <vpvl2/extensions/gl/VertexBundleLayout.h>
#include <glm/gtc/type_ptr.hpp>

namespace vpvl2 {
namespace extensions {
namespace gl {
class ShaderProgram;
class VertexBundle;
class VertexBundleLayout;
}
}
}

namespace vpvm {

class Grid {
public:
    Grid();
    ~Grid();

    void load();
    void draw(const SceneLoader *loader, bool visible);

    void setGridSize(const Vector4 &value) { m_size = value; }
    void setLineColor(const Vector3 &value) { m_lineColor = value; }
    void setAxisXColor(const Vector3 &value) { m_axisXColor = value; }
    void setAxisYColor(const Vector3 &value) { m_axisYColor = value; }
    void setAxisZColor(const Vector3 &value) { m_axisZColor = value; }

private:
    class PrivateShaderProgram;
    struct Vertex {
        Vertex() {}
        Vertex(const Vector3 &p, const Vector3 &c)
            : position(p),
              color(c)
        {
        }
        Vector3 position;
        Vector3 color;
    };
    void addLine(const Vector3 &from,
                 const Vector3 &to,
                 const Vector3 &color,
                 Array<Vertex> &vertices,
                 Array<uint8_t> &indices,
                 uint8_t &index);
    void bindVertexBundle(bool bundle);
    void releaseVertexBundle(bool bundle);

    QScopedPointer<PrivateShaderProgram> m_program;
    QScopedPointer<vpvl2::extensions::gl::VertexBundle> m_bundle;
    QScopedPointer<vpvl2::extensions::gl::VertexBundleLayout> m_layout;
    Vector4 m_size;
    Vector3 m_lineColor;
    Vector3 m_axisXColor;
    Vector3 m_axisYColor;
    Vector3 m_axisZColor;
    int m_nindices;

    Q_DISABLE_COPY(Grid)
};

} /* namespace vpvm */

#endif // GRID_H
