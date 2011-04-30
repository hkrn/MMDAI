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
