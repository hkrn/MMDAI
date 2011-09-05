#ifndef DELEGATE_H
#define DELEGATE_H

#include <GL/glew.h>
#include <QtCore/QString>
#include <QtGui/QImage>
#include <vpvl/gl/Renderer.h>

class QGLWidget;

class Delegate : public vpvl::gl::IDelegate
{
public:
    Delegate(QGLWidget *wiget);
    ~Delegate();

    bool loadTexture(const std::string &path, GLuint &textureID);
    bool loadToonTexture(const std::string &name, const std::string &dir, GLuint &textureID);
    void log(LogLevel level, const char *format...);
    const std::string toUnicode(const uint8_t *value);

private:
    static QImage loadTGA(const QString &path, uint8_t *&rawData);

    QGLWidget *m_widget;
};

#endif // DELEGATE_H
