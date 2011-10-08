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
    enum Flags {
        kEnable = 1,
        kDisable = 2,
        kMove = 4,
        kRotate = 8,
        kX = 16,
        kY = 32,
        kZ = 64
    };

    Handles()
        : m_width(0),
          m_height(0),
          m_enableMove(false),
          m_enableRotate(false),
          m_visible(true)
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
        qreal baseX = width - 104, baseY = 4, xoffset = 32, yoffset = 40;
        m_width = width;
        m_height = height;
        m_x.enableMove.rect.setTopLeft(QPointF(baseX, baseY));
        m_x.enableMove.rect.setSize(m_x.enableMove.size);
        m_y.enableMove.rect.setTopLeft(QPointF(baseX + xoffset, baseY));
        m_y.enableMove.rect.setSize(m_y.enableMove.size);
        m_z.enableMove.rect.setTopLeft(QPointF(baseX + xoffset * 2, baseY));
        m_z.enableMove.rect.setSize(m_z.enableMove.size);
        m_x.disableMove.rect.setTopLeft(QPointF(baseX, baseY));
        m_x.disableMove.rect.setSize(m_x.disableMove.size);
        m_y.disableMove.rect.setTopLeft(QPointF(baseX + xoffset, baseY));
        m_y.disableMove.rect.setSize(m_y.disableMove.size);
        m_z.disableMove.rect.setTopLeft(QPointF(baseX + xoffset * 2,  baseY));
        m_z.disableMove.rect.setSize(m_z.disableMove.size);
        m_x.enableRotate.rect.setTopLeft(QPointF(baseX, baseY + yoffset));
        m_x.enableRotate.rect.setSize(m_x.enableRotate.size);
        m_y.enableRotate.rect.setTopLeft(QPointF(baseX + xoffset, baseY + yoffset));
        m_y.enableRotate.rect.setSize(m_y.enableRotate.size);
        m_z.enableRotate.rect.setTopLeft(QPointF(baseX + xoffset * 2, baseY + yoffset));
        m_z.enableRotate.rect.setSize(m_z.enableRotate.size);
        m_x.disableRotate.rect.setTopLeft(QPointF(baseX, baseY + yoffset));
        m_x.disableRotate.rect.setSize(m_x.disableRotate.size);
        m_y.disableRotate.rect.setTopLeft(QPointF(baseX + xoffset, baseY + yoffset));
        m_y.disableRotate.rect.setSize(m_y.disableRotate.size);
        m_z.disableRotate.rect.setTopLeft(QPointF(baseX + xoffset * 2, baseY + yoffset));
        m_z.disableRotate.rect.setSize(m_z.disableRotate.size);
    }
    bool testHit(const QPoint &p, int &flags, QRectF &rect) {
        QPoint pos(p.x(), m_height - p.y());
        flags = 0;
        if (m_enableMove) {
            if (m_x.enableMove.rect.contains(pos)) {
                rect = m_x.enableMove.rect;
                flags = kEnable | kMove | kX;
            }
            else if (m_y.enableMove.rect.contains(pos)) {
                rect = m_y.enableMove.rect;
                flags = kEnable | kMove | kY;
            }
            else if (m_z.enableMove.rect.contains(pos)) {
                rect = m_z.enableMove.rect;
                flags = kEnable | kMove | kZ;
            }
        }
        else {
            if (m_x.disableMove.rect.contains(pos)) {
                rect = m_x.disableMove.rect;
                flags = kDisable | kMove | kX;
            }
            else if (m_y.disableMove.rect.contains(pos)) {
                rect = m_y.disableMove.rect;
                flags = kDisable | kMove | kY;
            }
            else if (m_z.disableMove.rect.contains(pos)) {
                rect = m_z.disableMove.rect;
                flags = kDisable | kMove | kZ;
            }
        }
        if (m_enableRotate) {
            if (m_x.enableRotate.rect.contains(pos)) {
                rect = m_x.enableRotate.rect;
                flags = kEnable | kRotate | kX;
            }
            else if (m_y.enableRotate.rect.contains(pos)) {
                rect = m_y.enableRotate.rect;
                flags = kEnable | kRotate | kY;
            }
            else if (m_z.enableRotate.rect.contains(pos)) {
                rect = m_z.enableRotate.rect;
                flags = kEnable | kRotate | kZ;
            }
        }
        else {
            if (m_x.disableRotate.rect.contains(pos)) {
                rect = m_x.disableRotate.rect;
                flags = kDisable | kRotate | kX;
            }
            else if (m_y.disableRotate.rect.contains(pos)) {
                rect = m_y.disableRotate.rect;
                flags = kDisable | kRotate | kY;
            }
            else if (m_z.disableRotate.rect.contains(pos)) {
                rect = m_z.disableRotate.rect;
                flags = kDisable | kRotate | kZ;
            }
        }
        return flags != 0;
    }
    void draw(QGLWidget *widget) {
        if (!m_visible)
            return;
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
    void setVisible(bool value) {
        m_visible = value;
    }

private:
    Handle m_x;
    Handle m_y;
    Handle m_z;
    int m_width;
    int m_height;
    bool m_enableMove;
    bool m_enableRotate;
    bool m_visible;
};

#endif // HANDLES_H
