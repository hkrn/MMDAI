#ifndef PLAYERWIDGET_H
#define PLAYERWIDGET_H

#include <QtCore/QElapsedTimer>
#include <QtCore/QHash>
#include <GL/glew.h>
#include <QtOpenGL/QGLWidget>

namespace vpvl {
namespace gl {
class Renderer;
}
class PMDModel;
class Scene;
class VMDMotion;
class XModel;
}

class Delegate;
class SceneLoader;
class World;

class PlayerWidget : public QGLWidget
{
    Q_OBJECT
public:
    static const QString kWindowTileFormat;

    explicit PlayerWidget(SceneLoader *loader, QWidget *parent = 0);
    ~PlayerWidget();

    void play();
    void stop();

protected:
    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);
    void closeEvent(QCloseEvent *event);
    void timerEvent(QTimerEvent *event);

private:
    void updateFPS();

    vpvl::gl::Renderer *m_renderer;
    Delegate *m_delegate;
    SceneLoader *m_parentLoader;
    SceneLoader *m_thisLoader;
    World *m_world;
    QElapsedTimer m_timer;
    int m_frameCount;
    int m_currentFPS;
    int m_defaultFPS;
    int m_interval;
    int m_internalTimerID;
};

#endif // PLAYERWIDGET_H
