#include "QMAScenePreview.h"
#include "QMAPreference.h"

#include <MMDAI/MMDAI.h>

QMAScenePreview::QMAScenePreview(QMAPreference *preference, QWidget *parent) :
    QMAScenePlayer(preference, parent),
    m_gridListID(0)
{
}

QMAScenePreview::~QMAScenePreview()
{
}

void QMAScenePreview::initialize()
{
    QStringList args = qApp->arguments();
    QFile file("MMDAIUserData:/MMDAI.mdf");
    if (args.count() > 1) {
        QString filename = args.at(1);
        loadUserPreference(filename);
    }
    m_preference->load(file);
    m_timer.setSingleShot(false);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(updatePreview()));
}

void QMAScenePreview::loadPlugins()
{
}

void QMAScenePreview::start()
{
    addModel("/Users/hkrn/src/svn/MMDAgent/MMDAgent_Example/Model/lat/lat_white.pmd");
    MMDAI::PMDObject *object = m_controller->getObjectAt(0);
    object->getModel()->setGlobalAlpha(1.0);
    m_timer.start(m_interval);
}

bool QMAScenePreview::handleCommand(const QString &command, const QList<QVariant> &arguments)
{
    return QMAScenePlayer::handleCommand(command, arguments);
}

bool QMAScenePreview::handleEvent(const QString &type, const QList<QVariant> &arguments)
{
    return QMAScenePlayer::handleEvent(type, arguments);
}

void QMAScenePreview::initializeGL()
{
    const float color[] = { 1.0f, 1.0f, 1.0f };
    m_preference->setFloat3(MMDAI::kPreferenceCampusColor, color);
    m_controller->initialize(width(), height());
    m_controller->updateLight();
    m_debug->initialize();
}

void QMAScenePreview::updatePreview()
{
    m_controller->updateSkin();
    m_controller->updateDepthTextureViewParam();
    update();
}

void QMAScenePreview::paintGL()
{
    QMAScenePlayer::paintGL();
    drawGrid();
}

void QMAScenePreview::drawGrid()
{
    glDisable(GL_LIGHTING);
    if (m_gridListID) {
        glCallList(m_gridListID);
    }
    else {
        m_gridListID = glGenLists(1);
        glNewList(m_gridListID, GL_COMPILE);
        glColor3f(0.5f, 0.5f, 0.5f);
        GLfloat limit = 50.0f;
        // draw black grid
        for (int x = -limit; x <= limit; x += 5) {
            glBegin(GL_LINES);
            glVertex3f(x, 0.0, -limit);
            glVertex3f(x, 0.0, x == 0 ? 0.0 : limit);
            glEnd();
        }
        for (int z = -limit; z <= limit; z += 5) {
            glBegin(GL_LINES);
            glVertex3i(-limit, 0.0f, z);
            glVertex3i(z == 0 ? 0.0f : limit, 0.0f, z);
            glEnd();
        }
        // X coordinate
        glColor3f(1.0f, 0.0f, 0.0f);
        glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(limit, 0.0f, 0.0f);
        glEnd();
        // Y coordinate
        glColor3f(0.0f, 1.0f, 0.0f);
        glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, limit, 0.0f);
        glEnd();
        // Z coordinate
        glColor3f(0.0f, 0.0f, 1.0f);
        glBegin(GL_LINES);
        glVertex3f(0.0f, 0.0f, 0.0f);
        glVertex3f(0.0f, 0.0f, limit);
        glEnd();
        glEndList();
    }
    glEnable(GL_LIGHTING);
}
