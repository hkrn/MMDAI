#include "Delegate.h"
#include "PlayerWidget.h"
#include "World.h"

#include <QtGui/QtGui>
#include <vpvl/vpvl.h>
#include <vpvl/gl/Renderer.h>

const QString PlayerWidget::kWindowTileFormat = tr("%1 - Rendering scene (FPS: %2)");

PlayerWidget::PlayerWidget(vpvl::VMDMotion *camera,
                           const QHash<QString, vpvl::PMDModel *> &models,
                           const QHash<QString, vpvl::XModel *> &assets,
                           const QHash<vpvl::PMDModel *, vpvl::VMDMotion *> &motions,
                           QWidget *parent) :
    QGLWidget(parent),
    m_renderer(0),
    m_delegate(0),
    m_world(0),
    m_camera(camera),
    m_models(models),
    m_assets(assets),
    m_motions(motions),
    m_frameCount(0),
    m_currentFPS(0),
    m_defaultFPS(60),
    m_interval(1000.0f / m_defaultFPS),
    m_internalTimerID(0)
{
    m_delegate = new Delegate(this);
    m_world = new World(m_defaultFPS);
    setAcceptDrops(false);
    setAutoFillBackground(false);
    setMinimumSize(540, 480);
    setWindowTitle(kWindowTileFormat.arg(qAppName()).arg(0));
}

PlayerWidget::~PlayerWidget()
{
    m_renderer = 0;
    m_camera = 0;
    m_frameCount = 0;
    m_currentFPS = 0;
    m_defaultFPS = 0;
    m_interval = 0;
    m_internalTimerID = 0;
}

void PlayerWidget::play()
{
    m_internalTimerID = startTimer(m_interval);
}

void PlayerWidget::stop()
{
    killTimer(m_internalTimerID);
    m_currentFPS = 0;
}

void PlayerWidget::initializeGL()
{
    m_renderer = new vpvl::gl::Renderer(m_delegate, width(), height(), m_defaultFPS);
    m_renderer->setDebugDrawer(m_world->mutableWorld());
    vpvl::Scene *scene = m_renderer->scene();
    scene->setViewMove(0);
    scene->setWorld(m_world->mutableWorld());
    scene->seek(0.0f);
    m_timer.start();
    play();
}

void PlayerWidget::paintGL()
{
    qglClearColor(Qt::white);
    m_renderer->initializeSurface();
    m_renderer->drawSurface();
    updateFPS();
}

void PlayerWidget::resizeGL(int w, int h)
{
    m_renderer->resize(w, h);
}

void PlayerWidget::closeEvent(QCloseEvent *event)
{
    stop();
    event->accept();
}

void PlayerWidget::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_internalTimerID) {
        vpvl::Scene *scene = m_renderer->scene();
        scene->updateModelView(0);
        scene->updateProjection(0);
        scene->update(0.5f);
        updateGL();
    }
}

void PlayerWidget::updateFPS()
{
    if (m_timer.elapsed() > 1000) {
        m_currentFPS = m_frameCount;
        m_frameCount = 0;
        m_timer.restart();
        setWindowTitle(kWindowTileFormat.arg(qAppName()).arg(m_currentFPS));
    }
    m_frameCount++;
}
