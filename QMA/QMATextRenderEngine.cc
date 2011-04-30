#include "QMATextRenderEngine.h"

#include <QtGui>

QMATextRenderEngine::QMATextRenderEngine(const QFont &font)
    : m_font(font),
      m_foreground(QColor::fromRgbF(1.0, 1.0, 1.0, 1.0)),
      m_background(QColor::fromRgbF(0.0, 0.0, 0.0, 0.75)),
      m_size(0, 0),
      m_textureID(0),
      m_enable(false)
{
    m_font.setPointSize(32);
}

QMATextRenderEngine::~QMATextRenderEngine()
{
    if (glIsTexture(m_textureID)) {
        glDeleteTextures(1, &m_textureID);
        m_textureID = 0;
    }
}

void QMATextRenderEngine::setFont(const QFont &font)
{
    m_font = font;
}

void QMATextRenderEngine::setColor(const QColor &foreground, const QColor &background)
{
    m_foreground = foreground;
    m_background = background;
}

void QMATextRenderEngine::setText(const QStringList &text)
{
    QFontMetrics metrics(m_font);
    int x = 0, y = 0, height = metrics.height(), nlines = text.count();
    foreach (QString line, text) {
        int width = metrics.width(line);
        if (width > x)
            x = width;
        y += height;
    }
    y = height;
    QImage image(kWidth, kWidth, QImage::Format_ARGB32_Premultiplied);
    QPainter painter(&image);
    painter.setPen(m_foreground);
    painter.setFont(m_font);
#if 0
    painter.fillRect(0, 0, kWidth, height * nlines + 10, m_background);
#else
    painter.fillRect(0, 0, kWidth, kHeight, m_background);
#endif
    foreach (QString line, text) {
        painter.drawText(0, y, line);
        y += height;
    }
    painter.end();
    if (glIsTexture(m_textureID))
        glDeleteTextures(1, &m_textureID);
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kWidth, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.constBits());
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
    m_size.setWidth(kWidth);
    m_size.setHeight(height * nlines + 10);
    m_text = text;
}

void QMATextRenderEngine::render()
{
    if (m_enable) {
        GLdouble pos = 15.0;
        glDisable(GL_CULL_FACE);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, m_textureID);
        glPushMatrix();
        glTranslated(0.0, -5.0, 0.0);
        glBegin(GL_POLYGON);
        glTexCoord2d(0.0, 1.0);
        glVertex2d(-pos, -pos);
        glTexCoord2d(1.0, 1.0);
        glVertex2d(pos, -pos);
        glTexCoord2d(1.0, 0.0);
        glVertex2d(pos, pos);
        glTexCoord2d(0.0, 0.0);
        glVertex2d(-pos, pos);
        glEnd();
        glPopMatrix();
        glEnable(GL_CULL_FACE);
        glDisable(GL_TEXTURE_2D);
    }
}
