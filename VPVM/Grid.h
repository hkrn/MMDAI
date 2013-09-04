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

#ifndef GRID_H_
#define GRID_H_

#include <vpvl2/Common.h>
#include <glm/gtc/type_ptr.hpp>

#include <QColor>
#include <QVector4D>
#include <QScopedPointer>

class ProjectProxy;

namespace vpvl2 {
class IModel;
class Scene;
namespace extensions {
namespace gl {
class ShaderProgram;
class VertexBundle;
class VertexBundleLayout;
}
}
}

class Grid : public QObject {
    Q_OBJECT

public:
    Q_PROPERTY(QVector4D size READ size WRITE setSize NOTIFY sizeChanged)
    Q_PROPERTY(QColor lineColor READ lineColor WRITE setLineColor NOTIFY lineColorChanged)
    Q_PROPERTY(QColor axisXColor READ axisXColor WRITE setAxisXColor NOTIFY axisXColorChanged)
    Q_PROPERTY(QColor axisYColor READ axisYColor WRITE setAxisYColor NOTIFY axisYColorChanged)
    Q_PROPERTY(QColor axisZColor READ axisZColor WRITE setAxisZColor NOTIFY axisZColorChanged)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged)

    Grid(QObject *parent = 0);
    ~Grid();

    void load();
    void draw(const glm::mat4 &mvp);

    QVector4D size() const;
    void setSize(const QVector4D &value);
    QColor lineColor() const;
    void setLineColor(const QColor &value);
    QColor axisXColor() const;
    void setAxisXColor(const QColor &value);
    QColor axisYColor() const;
    void setAxisYColor(const QColor &value);
    QColor axisZColor() const;
    void setAxisZColor(const QColor &value);
    bool isVisible() const;
    void setVisible(bool value);

signals:
    void sizeChanged();
    void lineColorChanged();
    void axisXColorChanged();
    void axisYColorChanged();
    void axisZColorChanged();
    void visibleChanged();

private:
    class PrivateShaderProgram;
    struct Vertex {
        Vertex() {}
        Vertex(const vpvl2::Vector3 &p, const QColor &c)
            : position(p),
              color(c.redF(), c.greenF(), c.blueF())
        {
        }
        vpvl2::Vector3 position;
        vpvl2::Vector3 color;
    };
    void addLine(const vpvl2::Vector3 &from,
                 const vpvl2::Vector3 &to,
                 const QColor &color,
                 vpvl2::Array<Vertex> &vertices,
                 vpvl2::Array<vpvl2::uint8> &indices,
                 vpvl2::uint8 &index);
    void bindVertexBundle(bool bundle);
    void releaseVertexBundle(bool bundle);

    ProjectProxy *m_parentProjectProxyRef;
    QScopedPointer<PrivateShaderProgram> m_program;
    QScopedPointer<vpvl2::extensions::gl::VertexBundle> m_bundle;
    QScopedPointer<vpvl2::extensions::gl::VertexBundleLayout> m_layout;
    QVector4D m_size;
    QColor m_lineColor;
    QColor m_axisXColor;
    QColor m_axisYColor;
    QColor m_axisZColor;
    int m_nindices;
    bool m_visible;

    Q_DISABLE_COPY(Grid)
};

#endif // GRID_H
