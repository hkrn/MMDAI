#ifndef HANDLES_H
#define HANDLES_H

#include <QtOpenGL/QtOpenGL>
#include <QtCore/QRectF>
#include <QtCore/QSize>

class Handles
{
public:
    struct Texture {
        QSize size;
        QRectF rect;
        GLuint textureID;
    };
    struct Handle {
        Texture enableMove;
        Texture disableMove;
        Texture enableRotate;
        Texture disableRotate;
    };

    Handles()
        : m_width(0),
          m_height(0),
          m_enableMove(false),
          m_enableRotate(false)
    {
    }
    ~Handles() {
        glDeleteTextures(1, &m_x.enableMove.textureID);
        glDeleteTextures(1, &m_y.enableMove.textureID);
        glDeleteTextures(1, &m_z.enableMove.textureID);
        glDeleteTextures(1, &m_x.disableMove.textureID);
        glDeleteTextures(1, &m_y.disableMove.textureID);
        glDeleteTextures(1, &m_z.disableMove.textureID);
        glDeleteTextures(1, &m_x.enableRotate.textureID);
        glDeleteTextures(1, &m_y.enableRotate.textureID);
        glDeleteTextures(1, &m_z.enableRotate.textureID);
        glDeleteTextures(1, &m_x.disableRotate.textureID);
        glDeleteTextures(1, &m_y.disableRotate.textureID);
        glDeleteTextures(1, &m_z.disableRotate.textureID);
    }

    void load(QGLWidget *widget) {
        QImage image;
        image.load(":icons/x-enable-move.png");
        m_x.enableMove.size = image.size();
        m_x.enableMove.textureID = widget->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
        image.load(":icons/x-enable-rotate.png");
        m_x.enableRotate.size = image.size();
        m_x.enableRotate.textureID = widget->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
        image.load(":icons/y-enable-move.png");
        m_y.enableMove.size = image.size();
        m_y.enableMove.textureID = widget->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
        image.load(":icons/y-enable-rotate.png");
        m_y.enableRotate.size = image.size();
        m_y.enableRotate.textureID = widget->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
        image.load(":icons/z-enable-move.png");
        m_z.enableMove.size = image.size();
        m_z.enableMove.textureID = widget->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
        image.load(":icons/z-enable-rotate.png");
        m_z.enableRotate.size = image.size();
        m_z.enableRotate.textureID = widget->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
        image.load(":icons/x-disable-move.png");
        m_x.disableMove.size = image.size();
        m_x.disableMove.textureID = widget->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
        image.load(":icons/x-disable-rotate.png");
        m_x.disableRotate.size = image.size();
        m_x.disableRotate.textureID = widget->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
        image.load(":icons/y-disable-move.png");
        m_y.disableMove.size = image.size();
        m_y.disableMove.textureID = widget->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
        image.load(":icons/y-disable-rotate.png");
        m_y.disableRotate.size = image.size();
        m_y.disableRotate.textureID = widget->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
        image.load(":icons/z-disable-move.png");
        m_z.disableMove.size = image.size();
        m_z.disableMove.textureID = widget->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
        image.load(":icons/z-disable-rotate.png");
        m_z.disableRotate.size = image.size();
        m_z.disableRotate.textureID = widget->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
    }
    void resize(int width, int height) {
        qreal baseX = width - 240, xoffset = 80, yoffset = 96;
        m_width = width;
        m_height = height;
        if (m_enableMove) {
            m_x.enableMove.rect.setTopLeft(QPointF(baseX, yoffset));
            m_x.enableMove.rect.setSize(m_x.enableMove.size);
            m_y.enableMove.rect.setTopLeft(QPointF(baseX + xoffset, yoffset));
            m_y.enableMove.rect.setSize(m_y.enableMove.size);
            m_z.enableMove.rect.setTopLeft(QPointF(baseX + xoffset * 2, yoffset));
            m_z.enableMove.rect.setSize(m_z.enableMove.size);
        }
        else {
            m_x.disableMove.rect.setTopLeft(QPointF(baseX, yoffset));
            m_x.disableMove.rect.setSize(m_x.disableMove.size);
            m_y.disableMove.rect.setTopLeft(QPointF(baseX + xoffset, yoffset));
            m_y.disableMove.rect.setSize(m_y.disableMove.size);
            m_z.disableMove.rect.setTopLeft(QPointF(baseX + xoffset * 2, yoffset));
            m_z.disableMove.rect.setSize(m_z.disableMove.size);
        }
        if (m_enableRotate) {
            m_x.enableRotate.rect.setTopLeft(QPointF(baseX, 0));
            m_x.enableRotate.rect.setSize(m_x.enableRotate.size);
            m_y.enableRotate.rect.setTopLeft(QPointF(baseX + xoffset, 0));
            m_y.enableRotate.rect.setSize(m_y.enableRotate.size);
            m_z.enableRotate.rect.setTopLeft(QPointF(baseX + xoffset * 2, 0));
            m_z.enableRotate.rect.setSize(m_z.enableRotate.size);
        }
        else {
            m_x.disableRotate.rect.setTopLeft(QPointF(baseX, 0));
            m_x.disableRotate.rect.setSize(m_x.disableRotate.size);
            m_y.disableRotate.rect.setTopLeft(QPointF(baseX + xoffset, 0));
            m_y.disableRotate.rect.setSize(m_y.disableRotate.size);
            m_z.disableRotate.rect.setTopLeft(QPointF(baseX + xoffset * 2, 0));
            m_z.disableRotate.rect.setSize(m_z.disableRotate.size);
        }
    }
    bool testHit(const QPoint &p, QRectF &rect) {
        bool hitMove = true, hitRotate = true;
        QPoint pos(p.x(), m_height - p.y());
        if (m_enableMove) {
            if (m_x.enableMove.rect.contains(pos))
                rect = m_x.enableMove.rect;
            else if (m_y.enableMove.rect.contains(pos))
                rect = m_y.enableMove.rect;
            else if (m_z.enableMove.rect.contains(pos))
                rect = m_z.enableMove.rect;
            else
                hitMove = false;
        }
        else {
            if (m_x.disableMove.rect.contains(pos))
                rect = m_x.disableMove.rect;
            else if (m_y.disableMove.rect.contains(pos))
                rect = m_y.disableMove.rect;
            else if (m_z.disableMove.rect.contains(pos))
                rect = m_z.disableMove.rect;
            else
                hitMove = false;
        }
        if (m_enableRotate) {
            if (m_x.enableRotate.rect.contains(pos))
                rect = m_x.enableRotate.rect;
            else if (m_y.enableRotate.rect.contains(pos))
                rect = m_y.enableRotate.rect;
            else if (m_z.enableRotate.rect.contains(pos))
                rect = m_z.enableRotate.rect;
            else
                hitRotate = false;
        }
        else {
            if (m_x.disableRotate.rect.contains(pos))
                rect = m_x.disableRotate.rect;
            else if (m_y.disableRotate.rect.contains(pos))
                rect = m_y.disableRotate.rect;
            else if (m_z.disableRotate.rect.contains(pos))
                rect = m_z.disableRotate.rect;
            else
                hitRotate = false;
        }
        return hitMove || hitRotate;
    }
    void draw(QGLWidget *widget) {
        QPainter painter(widget);
        painter.beginNativePainting();
        glEnable(GL_MULTISAMPLE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_LIGHTING);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluOrtho2D(0, m_width, 0, m_height);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        if (m_enableMove) {
            widget->drawTexture(m_x.enableMove.rect, m_x.enableMove.textureID);
            widget->drawTexture(m_y.enableMove.rect, m_y.enableMove.textureID);
            widget->drawTexture(m_z.enableMove.rect, m_z.enableMove.textureID);
        }
        else {
            widget->drawTexture(m_x.disableMove.rect, m_x.disableMove.textureID);
            widget->drawTexture(m_y.disableMove.rect, m_y.disableMove.textureID);
            widget->drawTexture(m_z.disableMove.rect, m_z.disableMove.textureID);
        }
        if (m_enableRotate) {
            widget->drawTexture(m_x.enableRotate.rect, m_x.enableRotate.textureID);
            widget->drawTexture(m_y.enableRotate.rect, m_y.enableRotate.textureID);
            widget->drawTexture(m_z.enableRotate.rect, m_z.enableRotate.textureID);
        }
        else {
            widget->drawTexture(m_x.disableRotate.rect, m_x.disableRotate.textureID);
            widget->drawTexture(m_y.disableRotate.rect, m_y.disableRotate.textureID);
            widget->drawTexture(m_z.disableRotate.rect, m_z.disableRotate.textureID);
        }
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
        painter.endNativePainting();
    }

    void setMovable(bool value) {
        m_enableMove = value;
    }
    void setRotateable(bool value) {
        m_enableRotate = value;
    }

private:
    Handle m_x;
    Handle m_y;
    Handle m_z;
    int m_width;
    int m_height;
    bool m_enableMove;
    bool m_enableRotate;
};

#endif // HANDLES_H
