/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2011  hkrn                                    */
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

#include "SceneWidget.h"

#include <QtGui/QtGui>
#include <btBulletDynamicsCommon.h>
#include <vpvl/vpvl.h>
#include "util.h"

namespace internal
{

typedef QScopedPointer<uint8_t, QScopedPointerArrayDeleter<uint8_t> > ByteArrayPtr;

class Delegate : public vpvl::gl::IDelegate
{
public:
    Delegate(QGLWidget *widget)
        : m_widget(widget)
    {}
    ~Delegate() {}

    bool loadTexture(const std::string &path, GLuint &textureID) {
        QString pathString = QString::fromUtf8(path.c_str());
        if (pathString.endsWith(".tga", Qt::CaseInsensitive)) {
            uint8_t *rawData = 0;
            QImage image = loadTGA(pathString, rawData);
            textureID = m_widget->bindTexture(QGLWidget::convertToGLFormat(image));
            delete[] rawData;
        }
        else {
            QImage image(pathString);
            textureID = m_widget->bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
        }
        qDebug("Loaded a texture (ID=%d): \"%s\"", textureID, pathString.toUtf8().constData());
        return textureID != 0;
    }
    bool loadToonTexture(const std::string &name, const std::string &dir, GLuint &textureID) {
        QString path = QString::fromUtf8(dir.c_str()) + "/" + QString::fromUtf8(name.c_str());
        if (!QFile::exists(path))
            path = QString(":/textures/%1").arg(name.c_str());
        return loadTexture(std::string(path.toUtf8()), textureID);
    }
    const std::string toUnicode(const uint8_t *value) {
        return std::string(toQString(value).toUtf8());
    }

private:
    static QImage loadTGA(const QString &path, uint8_t *&rawData) {
        QFile file(path);
        if (file.open(QFile::ReadOnly) && file.size() > 18) {
            QByteArray data = file.readAll();
            uint8_t *ptr = reinterpret_cast<uint8_t *>(data.data());
            uint8_t field = *reinterpret_cast<uint8_t *>(ptr);
            uint8_t type = *reinterpret_cast<uint8_t *>(ptr + 2);
            if (type != 2 /* full color */ && type != 10 /* full color + RLE */) {
                qWarning("Loaded TGA image type is not full color: %s", path.toUtf8().constData());
                return QImage();
            }
            uint16_t width = *reinterpret_cast<uint16_t *>(ptr + 12);
            uint16_t height = *reinterpret_cast<uint16_t *>(ptr + 14);
            uint8_t depth = *reinterpret_cast<uint8_t *>(ptr + 16); /* 24 or 32 */
            uint8_t flags = *reinterpret_cast<uint8_t *>(ptr + 17);
            if (width == 0 || height == 0 || (depth != 24 && depth != 32)) {
                qWarning("Invalid TGA image (width=%d, height=%d, depth=%d): %s",
                         width, height, depth, path.toUtf8().constData());
                return QImage();
            }
            int component = depth >> 3;
            uint8_t *body = ptr + 18 + field;
            /* if RLE compressed, uncompress it */
            size_t datalen = width * height * component;
            ByteArrayPtr uncompressedPtr(new uint8_t[datalen]);
            if (type == 10) {
                uint8_t *uncompressed = uncompressedPtr.data();
                uint8_t *src = body;
                uint8_t *dst = uncompressed;
                while (static_cast<size_t>(dst - uncompressed) < datalen) {
                    int16_t len = (*src & 0x7f) + 1;
                    if (*src & 0x80) {
                        src++;
                        for (int i = 0; i < len; i++) {
                            memcpy(dst, src, component);
                            dst += component;
                        }
                        src += component;
                    }
                    else {
                        src++;
                        memcpy(dst, src, component * len);
                        dst += component * len;
                        src += component * len;
                    }
                }
                /* will load from uncompressed data */
                body = uncompressed;
            }
            /* prepare texture data area */
            datalen = (width * height) << 2;
            rawData = new uint8_t[datalen];
            ptr = rawData;
            for (uint16_t h = 0; h < height; h++) {
                uint8_t *line = NULL;
                if (flags & 0x20) /* from up to bottom */
                    line = body + h * width * component;
                else /* from bottom to up */
                    line = body + (height - 1 - h) * width * component;
                for (uint16_t w = 0; w < width; w++) {
                    uint32_t index = 0;
                    if (flags & 0x10)/* from right to left */
                        index = (width - 1 - w) * component;
                    else /* from left to right */
                        index = w * component;
                    /* BGR or BGRA -> ARGB */
                    *ptr++ = line[index + 2];
                    *ptr++ = line[index + 1];
                    *ptr++ = line[index + 0];
                    *ptr++ = (depth == 32) ? line[index + 3] : 255;
                }
            }
            return QImage(rawData, width, height, QImage::Format_ARGB32);
        }
        else {
            qWarning("Cannot open file %s: %s", path.toUtf8().constData(),
                     file.errorString().toUtf8().constData());
            return QImage();
        }
    }

    QGLWidget *m_widget;
};

class World {
public:
    World(int defaultFPS)
        : m_dispatcher(&m_config),
          m_broadphase(btVector3(-400.0f, -400.0f, -400.0f), btVector3(400.0f, 400.0, 400.0f), 1024),
          m_world(&m_dispatcher, &m_broadphase, &m_solver, &m_config)
    {
        m_world.setGravity(btVector3(0.0f, -9.8f * 2.0f, 0.0f));
        setCurrentFPS(defaultFPS);
    }
    ~World()
    {
    }

    btDiscreteDynamicsWorld *mutableWorld() {
        return &m_world;
    }
    void setCurrentFPS(int value) {
        m_world.getSolverInfo().m_numIterations = static_cast<int>(10.0f * 60.0f / value);
    }

private:
    btDefaultCollisionConfiguration m_config;
    btCollisionDispatcher m_dispatcher;
    btAxisSweep3 m_broadphase;
    btSequentialImpulseConstraintSolver m_solver;
    btDiscreteDynamicsWorld m_world;
};

class Grid {
public:
    static const int kLimit = 50;

    Grid() : m_vbo(0), m_cbo(0), m_ibo(0), m_list(0) {}
    ~Grid() {
        m_vertices.clear();
        m_colors.clear();
        m_indices.clear();
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
        glDeleteBuffers(1, &m_cbo);
        m_cbo = 0;
        glDeleteBuffers(1, &m_ibo);
        m_ibo = 0;
        glDeleteLists(m_list, 1);
        m_list = 0;
    }

    void initialize() {
        // draw black grid
        btVector3 lineColor(0.5f, 0.5f, 0.5f);
        uint16_t index = 0;
        for (int x = -kLimit; x <= kLimit; x += 5)
            addLine(btVector3(x, 0.0, -kLimit), btVector3(x, 0.0, x == 0 ? 0.0 : kLimit), lineColor, index);
        for (int z = -kLimit; z <= kLimit; z += 5)
            addLine(btVector3(-kLimit, 0.0f, z), btVector3(z == 0 ? 0.0f : kLimit, 0.0f, z), lineColor, index);
        // X coordinate (red)
        addLine(btVector3(0.0f, 0.0f, 0.0f), btVector3(kLimit, 0.0f, 0.0f), btVector3(1.0f, 0.0f, 0.0f), index);
        // Y coordinate (green)
        addLine(btVector3(0.0f, 0.0f, 0.0f), btVector3(0.0f, kLimit, 0.0f), btVector3(0.0f, 1.0f, 0.0f), index);
        // Z coordinate (blue)
        addLine(btVector3(0.0f, 0.0f, 0.0f), btVector3(0.0f, 0.0f, kLimit), btVector3(0.0f, 0.0f, 1.0f), index);
        m_list = glGenLists(1);
        glGenBuffers(1, &m_vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(btVector3), &m_vertices[0], GL_STATIC_DRAW);
        glGenBuffers(1, &m_cbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_cbo);
        glBufferData(GL_ARRAY_BUFFER, m_colors.size() * sizeof(btVector3), &m_colors[0], GL_STATIC_DRAW);
        glGenBuffers(1, &m_ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(uint16_t), &m_indices[0], GL_STATIC_DRAW);
        // start compiling to render with list cache
        glNewList(m_list, GL_COMPILE);
        glDisable(GL_LIGHTING);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
        glVertexPointer(3, GL_FLOAT, sizeof(btVector3), 0);
        glBindBuffer(GL_ARRAY_BUFFER, m_cbo);
        glColorPointer(3, GL_FLOAT, sizeof(btVector3), 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
        glDrawElements(GL_LINES, m_indices.size(), GL_UNSIGNED_SHORT, 0);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glEnable(GL_LIGHTING);
        glEndList();
    }

    void draw() const {
        glCallList(m_list);
    }

private:
    void addLine(const btVector3 &from, const btVector3 &to, const btVector3 &color, uint16_t &index) {
        m_vertices.push_back(from);
        m_vertices.push_back(to);
        m_colors.push_back(color);
        m_colors.push_back(color);
        m_indices.push_back(index);
        index++;
        m_indices.push_back(index);
        index++;
    }

    btAlignedObjectArray<btVector3> m_vertices;
    btAlignedObjectArray<btVector3> m_colors;
    btAlignedObjectArray<uint16_t> m_indices;
    GLuint m_vbo;
    GLuint m_cbo;
    GLuint m_ibo;
    GLuint m_list;
};

}

SceneWidget::SceneWidget(QWidget *parent) :
    QGLWidget(QGLFormat(QGL::SampleBuffers), parent),
    m_camera(0),
    m_delegate(0),
    m_grid(0),
    m_world(0),
    m_settings(0),
    m_gridListID(0),
    m_frameCount(0),
    m_currentFPS(0),
    m_defaultFPS(60),
    m_interval(1000.0f / m_defaultFPS),
    m_internalTimerID(0)
{
    m_delegate = new internal::Delegate(this);
    m_grid = new internal::Grid();
    m_world = new internal::World(m_defaultFPS);
    setAcceptDrops(true);
    setAutoFillBackground(false);
    setMinimumSize(540, 480);
}

SceneWidget::~SceneWidget()
{
    delete m_camera;
    m_camera = 0;
    delete m_grid;
    m_grid = 0;
    delete m_world;
    m_world = 0;
    glDeleteLists(m_gridListID, 1);
    m_gridListID = 0;
    qDeleteAll(m_motions);
    foreach (vpvl::PMDModel *model, m_models) {
        m_renderer->unloadModel(model);
        delete model;
    }
    m_models.clear();
    m_assets.clear();
    delete m_renderer;
    m_renderer = 0;
    delete m_delegate;
    m_delegate = 0;
}

void SceneWidget::setCurrentFPS(int value)
{
    if (value > 0) {
        m_defaultFPS = value;
        m_world->setCurrentFPS(value);
        m_renderer->scene()->setCurrentFPS(value);
    }
}

void SceneWidget::resetAllBones()
{
    resetAllBones(selectedModel());
}

void SceneWidget::resetAllBones(vpvl::PMDModel *model)
{
    model->smearAllBonesToDefault(0.0f);
}

void SceneWidget::addModel()
{
    stopSceneUpdateTimer();
    QFileInfo fi(openFileDialog("sceneWidget/lastPMDDirectory", tr("Open PMD file"), tr("PMD file (*.pmd)")));
    if (fi.exists()) {
        QProgressDialog *progress = getProgressDialog("Loading the model...", 0);
        vpvl::PMDModel *model = addModelInternal(fi.fileName(), fi.dir());
        if (model)
            setSelectedModel(model);
        else
            QMessageBox::warning(this, tr("Loading model error"),
                                 tr("%1 cannot be loaded").arg(fi.fileName()));
        delete progress;
    }
    startSceneUpdateTimer();
}

void SceneWidget::insertMotionToAllModels()
{
    stopSceneUpdateTimer();
    QString fileName = openFileDialog("sceneWidget/lastVMDDirectory", tr("Open VMD (for model) file"), tr("VMD file (*.vmd)"));
    if (QFile::exists(fileName)) {
        foreach (vpvl::PMDModel *model, m_models) {
            if (!addMotionInternal(model, fileName)) {
                QMessageBox::warning(this, tr("Loading model motion error"),
                                     tr("%1 cannot be loaded").arg(QFileInfo(fileName).fileName()));
                break;
            }
        }
    }
    startSceneUpdateTimer();
}

void SceneWidget::insertMotionToSelectedModel()
{
    vpvl::PMDModel *selected = m_renderer->selectedModel();
    if (selected) {
        stopSceneUpdateTimer();
        QString fileName = openFileDialog("sceneWidget/lastVMDDirectory", tr("Open VMD (for model) file"), tr("VMD file (*.vmd)"));
        if (QFile::exists(fileName)) {
            if (!addMotionInternal(selected, fileName))
                QMessageBox::warning(this, tr("Loading model motion error"),
                                     tr("%1 cannot be loaded").arg(QFileInfo(fileName).fileName()));
        }
        startSceneUpdateTimer();
    }
    else {
        QMessageBox::warning(this, tr("The model is not selected."), tr("Select a model to insert the motion"));
    }
}

void SceneWidget::setModelPose()
{
    vpvl::PMDModel *selected = m_renderer->selectedModel();
    if (selected) {
        stopSceneUpdateTimer();
        QString fileName = openFileDialog("sceneWidget/lastVPDDirectory", tr("Open VPD file"), tr("VPD file (*.vpd)"));
        if (QFile::exists(fileName)) {
            vpvl::VPDPose *pose = setModelPoseInternal(selected, fileName);
            if (!pose)
                QMessageBox::warning(this, tr("Loading model pose error"),
                                     tr("%1 cannot be loaded").arg(QFileInfo(fileName).fileName()));
            else
                delete pose;
        }
        startSceneUpdateTimer();
    }
    else {
        QMessageBox::warning(this, tr("The model is not selected."), tr("Select a model to set the pose"));
    }
}

void SceneWidget::addAsset()
{
    stopSceneUpdateTimer();
    QFileInfo fi(openFileDialog("sceneWidget/lastAssetDirectory", tr("Open X file"), tr("DirectX mesh file (*.x)")));
    if (fi.exists()) {
        QProgressDialog *progress = getProgressDialog("Loading the stage...", 0);
        if (!addAssetInternal(fi.fileName(), fi.dir())) {
            QMessageBox::warning(this, tr("Loading stage error"),
                                 tr("%1 cannot be loaded").arg(fi.fileName()));
        }
        delete progress;
    }
    startSceneUpdateTimer();
}

void SceneWidget::setCamera()
{
    stopSceneUpdateTimer();
    QString fileName = openFileDialog("sceneWidget/lastCameraDirectory", tr("Open VMD (for camera) file"), tr("VMD file (*.vmd)"));
    if (QFile::exists(fileName)) {
        if (!setCameraInternal(fileName))
            QMessageBox::warning(this, tr("Loading camera motion error"),
                                 tr("%1 cannot be loaded").arg(QFileInfo(fileName).fileName()));
    }
    startSceneUpdateTimer();
}

void SceneWidget::deleteSelectedModel()
{
    vpvl::PMDModel *selected = m_renderer->selectedModel();
    const QString &key = m_models.key(selected);
    if (!key.isNull()) {
        emit modelDidDelete(selected);
        m_renderer->unloadModel(selected);
        m_renderer->scene()->removeModel(selected);
        m_models.remove(key);
        delete selected;
        m_renderer->setSelectedModel(0);
        emit modelDidSelect(0);
    }
    else {
        QMessageBox::warning(this, tr("The model is not selected or exist."), tr("Select a model to delete"));
    }
}

void SceneWidget::resetCamera()
{
    vpvl::Scene *scene = m_renderer->scene();
    scene->resetCamera();
    emit cameraPerspectiveDidSet(scene->position(), scene->angle(), scene->fovy(), scene->distance());
}

void SceneWidget::setCameraPerspective(btVector3 *pos, btVector3 *angle, float *fovy, float *distance)
{
    vpvl::Scene *scene = m_renderer->scene();
    btVector3 posValue, angleValue;
    float fovyValue, distanceValue;
    posValue = !pos ? scene->position() : *pos;
    angleValue = !angle ? scene->angle() : *angle;
    fovyValue = !fovy ? scene->fovy() : *fovy;
    distanceValue = !distance ? scene->distance() : *distance;
    scene->setCameraPerspective(posValue, angleValue, fovyValue, distanceValue);
    emit cameraPerspectiveDidSet(posValue, angleValue, fovyValue, distanceValue);
}

void SceneWidget::rotate(float x, float y)
{
    vpvl::Scene *scene = m_renderer->scene();
    btVector3 pos = scene->position(), angle = scene->angle();
    float fovy = scene->fovy(), distance = scene->distance();
    angle.setValue(angle.x() + x, angle.y() + y, angle.z());
    scene->setCameraPerspective(pos, angle, fovy, distance);
    emit cameraPerspectiveDidSet(pos, angle, fovy, distance);
}

void SceneWidget::translate(float x, float y)
{
    vpvl::Scene *scene = m_renderer->scene();
    btVector3 pos = scene->position(), angle = scene->angle();
    float fovy = scene->fovy(), distance = scene->distance();
    pos.setValue(pos.x() + x, pos.y() + y, pos.z());
    scene->setCameraPerspective(pos, angle, fovy, distance);
    emit cameraPerspectiveDidSet(pos, angle, fovy, distance);
}

void SceneWidget::zoom(bool up, const Qt::KeyboardModifiers &modifiers)
{
    vpvl::Scene *scene = m_renderer->scene();
    btVector3 pos = scene->position(), angle = scene->angle();
    float fovy = scene->fovy(), distance = scene->distance();
    float fovyStep = 1.0f, distanceStep = 4.0f;
    if (modifiers & Qt::ControlModifier && modifiers & Qt::ShiftModifier) {
        fovy = up ? fovy - fovyStep : fovy + fovyStep;
    }
    else {
        if (modifiers & Qt::ControlModifier)
            distanceStep *= 5.0f;
        else if (modifiers & Qt::ShiftModifier)
            distanceStep *= 0.2f;
        if (distanceStep != 0.0f)
            distance = up ? distance - distanceStep : distance + distanceStep;
    }
    scene->setCameraPerspective(pos, angle, fovy, distance);
    emit cameraPerspectiveDidSet(pos, angle, fovy, distance);
}

void SceneWidget::closeEvent(QCloseEvent *event)
{
    stopSceneUpdateTimer();
    event->accept();
}

void SceneWidget::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void SceneWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}

void SceneWidget::dragMoveEvent(QDragMoveEvent *event)
{
    event->acceptProposedAction();
}

void SceneWidget::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        const QList<QUrl> urls = mimeData->urls();
        const Qt::KeyboardModifiers modifiers = event->keyboardModifiers();
        stopSceneUpdateTimer();
        foreach (const QUrl url, urls) {
            QString path = url.toLocalFile();
            if (path.endsWith(".pmd", Qt::CaseInsensitive)) {
                if (modifiers & Qt::ControlModifier) {
                    QFileInfo modelPath(path);
                    addModelInternal(modelPath.baseName(), modelPath.dir());
                }
            }
            else if (path.endsWith(".x", Qt::CaseInsensitive)) {
                QFileInfo stagePath(path);
                addAssetInternal(stagePath.baseName(), stagePath.dir());
            }
            qDebug() << "Proceeded a dropped file:" << path;
        }
        startSceneUpdateTimer();
    }
}

void SceneWidget::initializeGL()
{
    GLenum err;
    if (!vpvl::gl::Renderer::initializeGLEW(err))
        qFatal("Cannot initialize GLEW: %s", glewGetErrorString(err));
    else
        qDebug("GLEW version: %s", glewGetString(GLEW_VERSION));
    qDebug("VPVL version: %s (%d)", VPVL_VERSION_STRING, VPVL_VERSION);
    m_renderer = new vpvl::gl::Renderer(m_delegate, width(), height(), m_defaultFPS);
    vpvl::Scene *scene = m_renderer->scene();
    scene->setViewMove(0);
    // scene->setWorld(m_world->mutableWorld());
    m_grid->initialize();
    m_timer.start();
    startSceneUpdateTimer();
}

void SceneWidget::mousePressEvent(QMouseEvent *event)
{
    vpvl::PMDModel *selected = m_renderer->selectedModel();
    m_prevPos = event->pos();
    if (selected) {
        vpvl::BoneList bones;
        //m_renderer->pickBones(event->pos().x(), event->pos().y(), 0.5f, bones);
        for (int i = 0; i < bones.size(); i++) {
            vpvl::Bone *bone = bones[i];
            qDebug() << internal::toQString(bone);
        }
    }
}

void SceneWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        Qt::KeyboardModifiers modifiers = event->modifiers();
        QPoint diff = event->pos() - m_prevPos;
        if (modifiers & Qt::ControlModifier && modifiers & Qt::ShiftModifier) {
            vpvl::Scene *scene = m_renderer->scene();
            btVector4 direction = scene->lightDirection();
            btVector3 d(direction.x(), direction.y(), direction.z());
            btQuaternion rx(0.0f, diff.y() * vpvl::radian(0.1f), 0.0f),
                    ry(0.0f, diff.x() * vpvl::radian(0.1f), 0.0f);
            d = d * btMatrix3x3(rx * ry);
            direction.setValue(d.x(), d.y(), d.z(), direction.w());
            scene->setLight(scene->lightColor(), direction);
        }
        else if (modifiers & Qt::ShiftModifier) {
            translate(diff.x() * -0.1f, diff.y() * 0.1f);
        }
        else {
            rotate(diff.y() * 0.5f, diff.x() * 0.5f);
        }
        m_prevPos = event->pos();
    }
}

void SceneWidget::paintGL()
{
    qglClearColor(Qt::white);
    m_renderer->initializeSurface();
    m_renderer->drawSurface();
    drawGrid();
    updateFPS();
    emit surfaceDidUpdate();
}

void SceneWidget::resizeGL(int w, int h)
{
    m_renderer->resize(w, h);
}

void SceneWidget::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_internalTimerID) {
        vpvl::Scene *scene = m_renderer->scene();
        scene->updateModelView(0);
        scene->updateProjection(0);
        scene->update(0.5f);
        updateGL();
    }
}

void SceneWidget::wheelEvent(QWheelEvent *event)
{
    zoom(event->delta() > 0, event->modifiers());
}

vpvl::XModel *SceneWidget::addAssetInternal(const QString &baseName, const QDir &dir)
{
    QFile file(dir.absoluteFilePath(baseName));
    vpvl::XModel *model = 0;
    if (file.open(QFile::ReadOnly)) {
        QByteArray data = file.readAll();
        model = new vpvl::XModel();
        if (model->load(reinterpret_cast<const uint8_t *>(data.constData()), data.size())) {
            QString key = baseName;
            if (m_assets.contains(key)) {
                int i = 0;
                while (true) {
                    QString tmpKey = QString("%1%2").arg(key).arg(i);
                    if (!m_assets.contains(tmpKey))
                        key = tmpKey;
                    i++;
                }
            }
            m_renderer->loadAsset(model, std::string(dir.absolutePath().toUtf8()));
            m_assets[key] = model;
            emit assetDidAdd(model);
        }
        else {
            delete model;
            model = 0;
        }
    }
    return model;
}

vpvl::PMDModel *SceneWidget::addModelInternal(const QString &baseName, const QDir &dir)
{
    QFile file(dir.absoluteFilePath(baseName));
    vpvl::PMDModel *model = 0;
    if (file.open(QFile::ReadOnly)) {
        QByteArray data = file.readAll();
        model = new vpvl::PMDModel();
        if (model->load(reinterpret_cast<const uint8_t *>(data.constData()), data.size())) {
            m_renderer->loadModel(model, std::string(dir.absolutePath().toUtf8()));
            m_renderer->scene()->addModel(model);
            QString key = internal::toQString(model);
            qDebug() << key << baseName;
            if (m_models.contains(key)) {
                int i = 0;
                while (true) {
                    QString tmpKey = QString("%1%2").arg(key).arg(i);
                    if (!m_models.contains(tmpKey))
                        key = tmpKey;
                    i++;
                }
            }
            m_models[key] = model;
            emit modelDidAdd(model);
        }
        else {
            delete model;
            model = 0;
        }
    }
    return model;
}

vpvl::VMDMotion *SceneWidget::addMotionInternal(vpvl::PMDModel *model, const QString &path)
{
    QFile file(path);
    vpvl::VMDMotion *motion = 0;
    if (file.open(QFile::ReadOnly)) {
        QByteArray data = file.readAll();
        motion = new vpvl::VMDMotion();
        if (motion->load(reinterpret_cast<const uint8_t *>(data.constData()), data.size())) {
            model->addMotion(motion);
            m_motions.append(motion);
            emit motionDidAdd(motion, model);
        }
        else {
            delete motion;
            motion = 0;
        }
    }
    return motion;
}

vpvl::VPDPose *SceneWidget::setModelPoseInternal(vpvl::PMDModel *model, const QString &path)
{
    QFile file(path);
    vpvl::VPDPose *pose = 0;
    if (file.open(QFile::ReadOnly)) {
        QByteArray data = file.readAll();
        pose = new vpvl::VPDPose();
        if (pose->load(reinterpret_cast<const uint8_t *>(data.constData()), data.size())) {
            pose->makePose(model);
            emit modelDidMakePose(pose, model);
        }
        else {
            delete pose;
            pose = 0;
        }
    }
    return pose;
}

vpvl::VMDMotion *SceneWidget::setCameraInternal(const QString &path)
{
    QFile file(path);
    vpvl::VMDMotion *motion = 0;
    if (file.open(QFile::ReadOnly)) {
        QByteArray data = file.readAll();
        motion = new vpvl::VMDMotion();
        if (motion->load(reinterpret_cast<const uint8_t *>(data.constData()), data.size())) {
            delete m_camera;
            m_camera = motion;
            m_renderer->scene()->setCameraMotion(motion);
            emit cameraMotionDidSet(motion);
        }
        else {
            delete motion;
            motion = 0;
        }
    }
    return motion;
}

void SceneWidget::drawGrid()
{
    m_grid->draw();
}

void SceneWidget::updateFPS()
{
    if (m_timer.elapsed() > 1000) {
        m_currentFPS = m_frameCount;
        m_frameCount = 0;
        m_timer.restart();
        emit fpsDidUpdate(m_currentFPS);
    }
    m_frameCount++;
}

void SceneWidget::startSceneUpdateTimer()
{
    m_internalTimerID = startTimer(m_interval);
}

void SceneWidget::stopSceneUpdateTimer()
{
    killTimer(m_internalTimerID);
    m_currentFPS = 0;
}

QProgressDialog *SceneWidget::getProgressDialog(const QString &label, int max)
{
    QProgressDialog *progress = new QProgressDialog(label, tr("Cancel"), 0, max, this);
    progress->setMinimumDuration(0);
    progress->setValue(0);
    progress->setWindowModality(Qt::WindowModal);
    return progress;
}

const QString SceneWidget::openFileDialog(const QString &name, const QString &desc, const QString &exts)
{
    const QString path = m_settings->value(name).toString();
    const QString fileName = QFileDialog::getOpenFileName(this, desc, path, exts);
    if (!fileName.isEmpty()) {
        QDir dir(fileName);
        dir.cdUp();
        m_settings->setValue(name, dir.absolutePath());
    }
    return fileName;
}
