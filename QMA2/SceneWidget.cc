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

enum __vpvlVertexBufferObjectType {
    kModelVertices,
    kModelNormals,
    kModelColors,
    kModelTexCoords,
    kModelToonTexCoords,
    kEdgeVertices,
    kEdgeIndices,
    kShadowIndices,
    kVertexBufferObjectMax
};

struct __vpvlPMDModelMaterialPrivate {
    GLuint primaryTextureID;
    GLuint secondTextureID;
    GLuint vertexBufferObject;
};

struct vpvl::PMDModelUserData {
    GLuint toonTextureID[vpvl::PMDModel::kSystemTextureMax];
    GLuint vertexBufferObjects[kVertexBufferObjectMax];
    bool hasSingleSphereMap;
    bool hasMultipleSphereMap;
    __vpvlPMDModelMaterialPrivate *materials;
};

struct vpvl::XModelUserData {
    GLuint listID;
    btHashMap<btHashString, GLuint> textures;
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

SceneWidget::SceneWidget(QSettings *settings, QWidget *parent) :
    QGLWidget(QGLFormat(QGL::SampleBuffers), parent),
    m_scene(0),
    m_stage(0),
    m_camera(0),
    m_selected(0),
    m_world(0),
    m_settings(settings),
    m_frameCount(0),
    m_currentFPS(0),
    m_defaultFPS(60),
    m_interval(1000.0f / m_defaultFPS),
    m_internalTimerID(0)
{
    m_world = new World(m_defaultFPS);
    setAcceptDrops(true);
    setAutoFillBackground(false);
}

SceneWidget::~SceneWidget()
{
    delete m_scene;
    m_scene = 0;
    unloadStage(m_stage);
    delete m_stage;
    m_stage = 0;
    delete m_camera;
    m_camera = 0;
    delete m_world;
    m_world = 0;
    m_selected = 0;
    qDeleteAll(m_motions);
    foreach (vpvl::PMDModel *model, m_models) {
        unloadModel(model);
        delete model;
    }
    m_models.clear();
}

void SceneWidget::setCurrentFPS(int value)
{
    if (value > 0) {
        m_defaultFPS = value;
        m_world->setCurrentFPS(value);
        m_scene->setCurrentFPS(value);
    }
}

const QString SceneWidget::toUnicodeModelName(const vpvl::PMDModel *model)
{
    static QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
    return codec->toUnicode(reinterpret_cast<const char *>(model->name()));
}

void SceneWidget::addModel()
{
    stopSceneUpdateTimer();
    QFileInfo fi(openFileDialog("lastPMDDirectory", tr("Open PMD file"), tr("PMD file (*.pmd)")));
    if (fi.exists()) {
        QProgressDialog *progress = getProgressDialog("Loading the model...", 0);
        addModelInternal(fi.fileName(), fi.dir());
        delete progress;
    }
    startSceneUpdateTimer();
}

void SceneWidget::insertMotionToAllModels()
{
    stopSceneUpdateTimer();
    QString fileName = openFileDialog("lastVMDDirectory", tr("Open VMD (for model) file"), tr("VMD file (*.vmd)"));
    if (QFile::exists(fileName)) {
        foreach (vpvl::PMDModel *model, m_models)
            addMotionInternal(model, fileName);
    }
    startSceneUpdateTimer();
}

void SceneWidget::insertMotionToSelectedModel()
{
    if (m_selected) {
        stopSceneUpdateTimer();
        QString fileName = openFileDialog("lastVMDDirectory", tr("Open VMD (for model) file"), tr("VMD file (*.vmd)"));
        if (QFile::exists(fileName))
            addMotionInternal(m_selected, fileName);
        startSceneUpdateTimer();
    }
    else {
        QMessageBox::warning(this, tr("The model is not selected."), tr("Select a motion to insert the motion"));
    }
}

void SceneWidget::setStage()
{
    stopSceneUpdateTimer();
    QFileInfo fi(openFileDialog("lastStageDirectory", tr("Open stage X file"), tr("DirectX mesh file (*.x)")));
    if (fi.exists()) {
        QProgressDialog *progress = getProgressDialog("Loading the stage...", 0);
        setStageInternal(fi.fileName(), fi.dir());
        delete progress;
    }
    startSceneUpdateTimer();
}

void SceneWidget::setCamera()
{
    stopSceneUpdateTimer();
    QString fileName = openFileDialog("lastCameraDirectory", tr("Open VMD (for camera) file"), tr("VMD file (*.vmd)"));
    if (QFile::exists(fileName))
        setCameraInternal(fileName);
    startSceneUpdateTimer();
}

void SceneWidget::deleteSelectedModel()
{
    const QString &key = m_models.key(m_selected);
    if (!key.isNull()) {
        emit modelDeleted(m_selected);
        m_scene->removeModel(m_selected);
        m_models.remove(key);
        delete m_selected;
        m_selected = 0;
    }
    else {
        QMessageBox::warning(this, tr("The model is not selected or exist."), tr("Select a model to delete"));
    }
}

void SceneWidget::resetCamera()
{
    m_scene->resetCamera();
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
                setStageInternal(stagePath.baseName(), stagePath.dir());
            }
        }
        startSceneUpdateTimer();
    }
}

void SceneWidget::initializeGL()
{
    m_scene = new vpvl::Scene(width(), height(), m_defaultFPS);
    m_scene->setWorld(m_world->mutableWorld());
    startSceneUpdateTimer();
    m_timer.start();
}

void SceneWidget::mousePressEvent(QMouseEvent *event)
{
    m_prevPos = event->pos();
}

void SceneWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        Qt::KeyboardModifiers modifiers = event->modifiers();
        QPoint diff = event->pos() - m_prevPos;
        if (modifiers & Qt::ControlModifier && modifiers & Qt::ShiftModifier) {
            btVector4 direction = m_scene->lightDirection();
            btVector3 d(direction.x(), direction.y(), direction.z());
            btQuaternion rx(0.0f, diff.y() * vpvl::radian(0.1f), 0.0f),
                    ry(0.0f, diff.x() * vpvl::radian(0.1f), 0.0f);
            d = d * btMatrix3x3(rx * ry);
            direction.setValue(d.x(), d.y(), d.z(), direction.w());
            m_scene->setLight(m_scene->lightColor(), direction);
        }
        else if (modifiers & Qt::ShiftModifier) {
            translateInternal(diff.x() * -0.1f, diff.y() * 0.1f);
        }
        else {
            rotateInternal(diff.y() * 0.5f, diff.x() * 0.5f);
        }
        m_prevPos = event->pos();
    }
}

void SceneWidget::paintEvent(QPaintEvent * /* event */)
{
    makeCurrent();
    initializeSurface();
    drawSurface();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    drawInformativeText(painter);
    painter.end();
    updateFPS();
}

void SceneWidget::resizeGL(int w, int h)
{
    m_scene->setWidth(w);
    m_scene->setHeight(h);
}

void SceneWidget::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_internalTimerID) {
        m_scene->updateModelView(0);
        m_scene->updateProjection(0);
        m_scene->update(0.5f);
        update();
    }
}

void SceneWidget::wheelEvent(QWheelEvent *event)
{
    zoom(event->delta() > 0, event->modifiers());
}

void SceneWidget::initializeSurface()
{
    glClearStencil(0);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GEQUAL, 0.05f);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);
    setLighting();
}

vpvl::PMDModel *SceneWidget::addModelInternal(const QString &baseName, const QDir &dir)
{
    QFile file(dir.absoluteFilePath(baseName));
    vpvl::PMDModel *model = 0;
    if (file.open(QFile::ReadOnly)) {
        QByteArray data = file.readAll();
        model = new vpvl::PMDModel(reinterpret_cast<const uint8_t *>(data.constData()), data.size());
        if (model->load()) {
            loadModel(model, dir);
            m_scene->addModel(model);
            QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
            QString key = codec->toUnicode(reinterpret_cast<const char *>(model->name()));
            if (m_models.contains(key)) {
                int i = 0;
                while (true) {
                    QString tmpKey = QString("%1%2").arg(key).arg(i);
                    if (!m_models.contains(tmpKey))
                        key = tmpKey;
                }
            }
            m_models[key] = model;
            emit modelAdded(model);
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
        motion = new vpvl::VMDMotion(reinterpret_cast<const uint8_t *>(data.constData()), data.size());
        if (motion->load()) {
            model->addMotion(motion);
            m_motions.append(motion);
            emit motionAdded(motion);
        }
        else {
            delete motion;
            motion = 0;
        }
    }
    return motion;
}

vpvl::VMDMotion *SceneWidget::setCameraInternal(const QString &path)
{
    QFile file(path);
    vpvl::VMDMotion *motion = 0;
    if (file.open(QFile::ReadOnly)) {
        QByteArray data = file.readAll();
        motion = new vpvl::VMDMotion(reinterpret_cast<const uint8_t *>(data.constData()), data.size());
        if (motion->load()) {
            delete m_camera;
            m_camera = motion;
            m_scene->setCameraMotion(motion);
            emit cameraMotionSet(motion);
        }
        else {
            delete motion;
            motion = 0;
        }
    }
    return motion;
}

vpvl::XModel *SceneWidget::setStageInternal(const QString &baseName, const QDir &dir)
{
    QFile file(dir.absoluteFilePath(baseName));
    vpvl::XModel *model = 0;
    if (file.open(QFile::ReadOnly)) {
        QByteArray data = file.readAll();
        model = new vpvl::XModel(reinterpret_cast<const uint8_t *>(data.constData()), data.size());
        if (model->load()) {
            if (m_stage) {
                unloadStage(m_stage);
                delete m_stage;
            }
            loadStage(model, dir);
            m_stage = model;
            emit stageSet(model);
        }
        else {
            delete model;
            model = 0;
        }
    }
    return model;
}

void SceneWidget::rotateInternal(float x, float y)
{
    btVector3 angle = m_scene->angle();
    angle.setValue(angle.x() + x, angle.y() + y, angle.z());
    m_scene->setCamera(m_scene->position(), angle, m_scene->fovy(), m_scene->distance());
}

void SceneWidget::translateInternal(float x, float y)
{
    btVector3 position = m_scene->position();
    position.setValue(position.x() + x, position.y() + y, position.z());
    m_scene->setCamera(position, m_scene->angle(), m_scene->fovy(), m_scene->distance());
}

void SceneWidget::setLighting()
{
    btVector4 color(1.0f, 1.0f, 1.0f, 1.0f), direction(0.5f, 1.0f, 0.5f, 0.0f);
    btScalar diffuseValue, ambientValue, specularValue, lightIntensity = 0.6;

    // use MMD like cartoon
#if 0
    diffuseValue = 0.2f;
    ambientValue = lightIntensity * 2.0f;
    specularValue = 0.4f;
#else
    diffuseValue = 0.0f;
    ambientValue = lightIntensity * 2.0f;
    specularValue = lightIntensity;
#endif

    btVector3 diffuse = color * diffuseValue;
    btVector3 ambient = color * ambientValue;
    btVector3 specular = color * specularValue;
    diffuse.setW(1.0f);
    ambient.setW(1.0f);
    specular.setW(1.0f);

    glLightfv(GL_LIGHT0, GL_POSITION, static_cast<const btScalar *>(direction));
    glLightfv(GL_LIGHT0, GL_DIFFUSE, static_cast<const btScalar *>(diffuse));
    glLightfv(GL_LIGHT0, GL_AMBIENT, static_cast<const btScalar *>(ambient));
    glLightfv(GL_LIGHT0, GL_SPECULAR, static_cast<const btScalar *>(specular));
    m_scene->setLight(color, direction);
}

bool SceneWidget::loadTexture(const QString &path, GLuint &textureID)
{
    qDebug() << path;
    QImage image(path);
    textureID = bindTexture(QGLWidget::convertToGLFormat(image.rgbSwapped()));
    return textureID != 0;
}

bool SceneWidget::loadToonTexture(const QString &name, const QDir &model, GLuint &textureID)
{
    QString path = model.absoluteFilePath(name);
    if (!QFile::exists(path)) {
        path = m_system.absoluteFilePath(name);
        if (!QFile::exists(path))
            return false;
    }
    return loadTexture(path, textureID);
}

void SceneWidget::loadModel(vpvl::PMDModel *model, const QDir &dir)
{
    const vpvl::MaterialList materials = model->materials();
    const uint32_t nMaterials = materials.size();
    GLuint textureID = 0;
    vpvl::PMDModelUserData *userData = new vpvl::PMDModelUserData;
    __vpvlPMDModelMaterialPrivate *materialPrivates = new __vpvlPMDModelMaterialPrivate[nMaterials];
    uint16_t *indicesPtr = const_cast<uint16_t *>(model->indicesPointer());
    bool hasSingleSphere = false, hasMultipleSphere = false;
    for (uint32_t i = 0; i < nMaterials; i++) {
        const vpvl::Material *material = materials[i];
        const uint8_t *primary = material->primaryTextureName();
        const uint8_t *second = material->secondTextureName();
        __vpvlPMDModelMaterialPrivate &materialPrivate = materialPrivates[i];
        materialPrivate.primaryTextureID = 0;
        materialPrivate.secondTextureID = 0;
        const uint32_t nIndices = material->countIndices();
        glGenBuffers(1, &materialPrivate.vertexBufferObject);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, materialPrivate.vertexBufferObject);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, nIndices * sizeof(uint16_t), indicesPtr, GL_STATIC_DRAW);
        indicesPtr += nIndices;
        if (*primary) {
            if (loadTexture(dir.absoluteFilePath(reinterpret_cast<const char *>(primary)), textureID))
                materialPrivate.primaryTextureID = textureID;
        }
        if (*second) {
            if (loadTexture(dir.absoluteFilePath(reinterpret_cast<const char *>(second)), textureID))
                materialPrivate.secondTextureID = textureID;
        }
        hasSingleSphere |= material->isSpherePrimary() && !material->isSphereAuxSecond();
        hasMultipleSphere |= material->isSphereAuxSecond();
    }
    userData->hasSingleSphereMap = hasSingleSphere;
    userData->hasMultipleSphereMap = hasMultipleSphere;
    glGenBuffers(kVertexBufferObjectMax, userData->vertexBufferObjects);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, userData->vertexBufferObjects[kEdgeIndices]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->edgeIndicesCount() * model->stride(vpvl::PMDModel::kEdgeIndicesStride),
                 model->edgeIndicesPointer(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, userData->vertexBufferObjects[kShadowIndices]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model->indices().size() * model->stride(vpvl::PMDModel::kIndicesStride),
                 model->indicesPointer(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelTexCoords]);
    glBufferData(GL_ARRAY_BUFFER, model->vertices().size() * model->stride(vpvl::PMDModel::kTextureCoordsStride),
                 model->textureCoordsPointer(), GL_STATIC_DRAW);
    if (loadToonTexture("toon0.bmp", dir, textureID))
        userData->toonTextureID[0] = textureID;
    for (uint32_t i = 0; i < vpvl::PMDModel::kSystemTextureMax - 1; i++) {
        const uint8_t *name = model->toonTexture(i);
        if (loadToonTexture(reinterpret_cast<const char *>(name), dir, textureID))
            userData->toonTextureID[i + 1] = textureID;
    }
    userData->materials = materialPrivates;
    model->setUserData(userData);
}

void SceneWidget::unloadModel(const vpvl::PMDModel *model)
{
    if (model) {
        const vpvl::MaterialList materials = model->materials();
        const uint32_t nMaterials = materials.size();
        vpvl::PMDModelUserData *userData = model->userData();
        for (uint32_t i = 0; i < nMaterials; i++) {
            __vpvlPMDModelMaterialPrivate &materialPrivate = userData->materials[i];
            deleteTexture(materialPrivate.primaryTextureID);
            deleteTexture(materialPrivate.secondTextureID);
            glDeleteBuffers(1, &materialPrivate.vertexBufferObject);
        }
        for (uint32_t i = 1; i < vpvl::PMDModel::kSystemTextureMax; i++) {
            deleteTexture(userData->toonTextureID[i]);
        }
        glDeleteBuffers(kVertexBufferObjectMax, userData->vertexBufferObjects);
        delete[] userData->materials;
        delete userData;
    }
}

void SceneWidget::drawModel(const vpvl::PMDModel *model)
{
#ifndef VPVL_COORDINATE_OPENGL
    glPushMatrix();
    glScalef(1.0f, 1.0f, -1.0f);
    glCullFace(GL_FRONT);
#endif

    const vpvl::PMDModelUserData *userData = model->userData();
    size_t stride = model->stride(vpvl::PMDModel::kNormalsStride), vsize = model->vertices().size();
    glActiveTexture(GL_TEXTURE0);
    glClientActiveTexture(GL_TEXTURE0);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelVertices]);
    glVertexPointer(3, GL_FLOAT, model->stride(vpvl::PMDModel::kVerticesStride), 0);
    glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelNormals]);
    glBufferData(GL_ARRAY_BUFFER, vsize * stride, model->normalsPointer(), GL_DYNAMIC_DRAW);
    glNormalPointer(GL_FLOAT, stride, 0);
    glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelTexCoords]);
    glTexCoordPointer(2, GL_FLOAT, model->stride(vpvl::PMDModel::kTextureCoordsStride), 0);

    const bool enableToon = true;
    // toon
    if (enableToon) {
        glActiveTexture(GL_TEXTURE1);
        glEnable(GL_TEXTURE_2D);
        glClientActiveTexture(GL_TEXTURE1);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, userData->vertexBufferObjects[kModelToonTexCoords]);
        // shadow map
        stride = model->stride(vpvl::PMDModel::kToonTextureStride);
        if (false)
            glBufferData(GL_ARRAY_BUFFER, 0, 0, GL_DYNAMIC_DRAW);
        else
            glBufferData(GL_ARRAY_BUFFER, vsize * stride, model->toonTextureCoordsPointer(), GL_DYNAMIC_DRAW);
        glTexCoordPointer(2, GL_FLOAT, stride, 0);
        glActiveTexture(GL_TEXTURE0);
        glClientActiveTexture(GL_TEXTURE0);
    }
    bool hasSingleSphereMap = false, hasMultipleSphereMap = false;
    // first sphere map
    if (userData->hasSingleSphereMap) {
        glEnable(GL_TEXTURE_2D);
        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        glDisable(GL_TEXTURE_2D);
        hasSingleSphereMap = true;
    }
    // second sphere map
    if (userData->hasMultipleSphereMap) {
        glActiveTexture(GL_TEXTURE2);
        glEnable(GL_TEXTURE_2D);
        glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
        glDisable(GL_TEXTURE_2D);
        glActiveTexture(GL_TEXTURE0);
        hasMultipleSphereMap = true;
    }

    const vpvl::MaterialList materials = model->materials();
    const __vpvlPMDModelMaterialPrivate *materialPrivates = userData->materials;
    const uint32_t nMaterials = materials.size();
    btVector4 average, ambient, diffuse, specular;
    for (uint32_t i = 0; i < nMaterials; i++) {
        const vpvl::Material *material = materials[i];
        const __vpvlPMDModelMaterialPrivate &materialPrivate = materialPrivates[i];
        // toon
        const float alpha = material->alpha();
        if (enableToon) {
            average = material->averageColor();
            average.setW(average.w() * alpha);
            specular = material->specular();
            specular.setW(specular.w() * alpha);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, static_cast<const GLfloat *>(average));
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, static_cast<const GLfloat *>(specular));
        }
        else {
            ambient = material->ambient();
            ambient.setW(ambient.w() * alpha);
            diffuse = material->diffuse();
            diffuse.setW(diffuse.w() * alpha);
            specular = material->specular();
            specular.setW(specular.w() * alpha);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, static_cast<const GLfloat *>(ambient));
            glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, static_cast<const GLfloat *>(diffuse));
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, static_cast<const GLfloat *>(specular));
        }
        glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material->shiness());
        material->alpha() < 1.0f ? glDisable(GL_CULL_FACE) : glEnable(GL_CULL_FACE);
        glActiveTexture(GL_TEXTURE0);
        // has texture
        if (materialPrivate.primaryTextureID > 0) {
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, materialPrivate.primaryTextureID);
            if (hasSingleSphereMap) {
                // is sphere map
                if (material->isSpherePrimary() || material->isSphereAuxPrimary()) {
                    // is second sphere map
                    if (material->isSphereAuxPrimary())
                        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
                    glEnable(GL_TEXTURE_GEN_S);
                    glEnable(GL_TEXTURE_GEN_T);
                }
                else {
                    glDisable(GL_TEXTURE_GEN_S);
                    glDisable(GL_TEXTURE_GEN_T);
                }
            }
        }
        else {
            glDisable(GL_TEXTURE_2D);
        }
        // toon
        if (enableToon) {
            const GLuint textureID = userData->toonTextureID[material->toonID()];
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        if (hasMultipleSphereMap) {
            // second sphere
            glActiveTexture(GL_TEXTURE2);
            glEnable(GL_TEXTURE_2D);
            if (materialPrivate.secondTextureID > 0) {
                // is second sphere
                if (material->isSphereAuxSecond())
                    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
                else
                    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
                glBindTexture(GL_TEXTURE_2D, materialPrivate.secondTextureID);
                glEnable(GL_TEXTURE_GEN_S);
                glEnable(GL_TEXTURE_GEN_T);
            }
            else {
                glBindTexture(GL_TEXTURE_2D, 0);
            }
        }
        // draw
        const uint32_t nIndices = material->countIndices();
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, materialPrivate.vertexBufferObject);
        glDrawElements(GL_TRIANGLES, nIndices, GL_UNSIGNED_SHORT, 0);
        // is aux sphere map
        if (material->isSphereAuxPrimary()) {
            glActiveTexture(GL_TEXTURE0);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    // toon
    if (enableToon) {
        glClientActiveTexture(GL_TEXTURE0);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        // first sphere map
        if (hasSingleSphereMap) {
            glActiveTexture(GL_TEXTURE0);
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
        }
        glClientActiveTexture(GL_TEXTURE1);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        // second sphere map
        if (hasMultipleSphereMap) {
            glActiveTexture(GL_TEXTURE2);
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
        }
    }
    else {
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        // first sphere map
        if (hasSingleSphereMap) {
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
        }
        // second sphere map
        if (hasMultipleSphereMap) {
            glActiveTexture(GL_TEXTURE2);
            glDisable(GL_TEXTURE_GEN_S);
            glDisable(GL_TEXTURE_GEN_T);
        }
    }
    glActiveTexture(GL_TEXTURE0);
    // first or second sphere map
    if (hasSingleSphereMap || hasMultipleSphereMap) {
        glDisable(GL_TEXTURE_GEN_S);
        glDisable(GL_TEXTURE_GEN_T);
    }
    // toon
    if (enableToon) {
        glActiveTexture(GL_TEXTURE1);
        glDisable(GL_TEXTURE_2D);
    }
    // second sphere map
    if (hasMultipleSphereMap) {
        glActiveTexture(GL_TEXTURE2);
        glDisable(GL_TEXTURE_2D);
    }
    glActiveTexture(GL_TEXTURE0);
    glDisable(GL_TEXTURE_2D);
    glEnable(GL_CULL_FACE);

#ifndef VPVL_COORDINATE_OPENGL
    glPopMatrix();
    glCullFace(GL_BACK);
#endif
}

void SceneWidget::drawModelEdge(const vpvl::PMDModel *model)
{
#ifdef VPVL_COORDINATE_OPENGL
    glCullFace(GL_FRONT);
#else
    glPushMatrix();
    glScalef(1.0f, 1.0f, -1.0f);
    glCullFace(GL_BACK);
#endif

    const float alpha = 1.0f;
    const size_t stride = model->stride(vpvl::PMDModel::kEdgeVerticesStride);
    const vpvl::PMDModelUserData *modelPrivate = model->userData();
    btVector4 color;

    if (model == m_selected)
        color.setValue(1.0f, 0.0f, 0.0f, alpha);
    else
        color.setValue(0.0f, 0.0f, 0.0f, alpha);

    glDisable(GL_LIGHTING);
    glEnableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, modelPrivate->vertexBufferObjects[kEdgeVertices]);
    glBufferData(GL_ARRAY_BUFFER, model->vertices().size() * stride, model->edgeVerticesPointer(), GL_DYNAMIC_DRAW);
    glVertexPointer(3, GL_FLOAT, stride, 0);
    glColor4fv(static_cast<const btScalar *>(color));
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelPrivate->vertexBufferObjects[kEdgeIndices]);
    glDrawElements(GL_TRIANGLES, model->edgeIndicesCount(), GL_UNSIGNED_SHORT, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_LIGHTING);

#ifdef VPVL_COORDINATE_OPENGL
    glCullFace(GL_BACK);
#else
    glPopMatrix();
    glCullFace(GL_FRONT);
#endif
}

void SceneWidget::drawModelShadow(const vpvl::PMDModel *model)
{
    const size_t stride = model->stride(vpvl::PMDModel::kVerticesStride);
    const vpvl::PMDModelUserData *modelPrivate = model->userData();
    glDisable(GL_CULL_FACE);
    glEnableClientState(GL_VERTEX_ARRAY);
    glBindBuffer(GL_ARRAY_BUFFER, modelPrivate->vertexBufferObjects[kModelVertices]);
    glBufferData(GL_ARRAY_BUFFER, model->vertices().size() * stride, model->verticesPointer(), GL_DYNAMIC_DRAW);
    glVertexPointer(3, GL_FLOAT, stride, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, modelPrivate->vertexBufferObjects[kShadowIndices]);
    glDrawElements(GL_TRIANGLES, model->indices().size(), GL_UNSIGNED_SHORT, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDisableClientState(GL_VERTEX_ARRAY);
    glEnable(GL_CULL_FACE);
}

void SceneWidget::loadStage(vpvl::XModel *model, const QDir &dir)
{
    vpvl::XModelUserData *userData = new vpvl::XModelUserData;
    userData->listID = glGenLists(1);
    glNewList(userData->listID, GL_COMPILE);
#ifndef VPVL_COORDINATE_OPENGL
    glPushMatrix();
    glScalef(1.0f, 1.0f, -1.0f);
    glCullFace(GL_FRONT);
#endif
    QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
    const btAlignedObjectArray<vpvl::XModelFaceIndex> &faces = model->faces();
    const btAlignedObjectArray<btVector3> &vertices = model->vertices();
    const btAlignedObjectArray<btVector3> &textureCoords = model->textureCoords();
    const btAlignedObjectArray<btVector3> &normals = model->normals();
    const btAlignedObjectArray<btVector4> &colors = model->colors();
    const bool hasMaterials = model->countMatreials() > 0;
    const bool hasTextureCoords = textureCoords.size() > 0;
    const bool hasNormals = normals.size();
    const bool hasColors = colors.size();
    uint32_t nFaces = faces.size();
    uint32_t prevIndex = UINT32_MAX;
    glEnable(GL_TEXTURE_2D);
    for (uint32_t i = 0; i < nFaces; i++) {
        const vpvl::XModelFaceIndex &face = faces[i];
        const btVector4 &value = face.value;
        const uint32_t count = face.count;
        const uint32_t currentIndex = face.index;
        if (hasMaterials && prevIndex != currentIndex) {
            const vpvl::XMaterial *material = model->materialAt(currentIndex);
            glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, static_cast<const GLfloat *>(material->color()));
            glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, static_cast<const GLfloat *>(material->emmisive()));
            glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, static_cast<const GLfloat *>(material->specular()));
            glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, material->power());
            QString textureName = codec->toUnicode(material->textureName());
            if (!textureName.isEmpty()) {
                btHashString key(material->textureName());
                GLuint *textureID = userData->textures[key];
                if (!textureID) {
                    GLuint value;
                    if (loadTexture(dir.absoluteFilePath(textureName), value)) {
                        userData->textures.insert(key, value);
                        glBindTexture(GL_TEXTURE_2D, value);
                    }
                }
                else {
                    glBindTexture(GL_TEXTURE_2D, *textureID);
                }
            }
            else {
                glBindTexture(GL_TEXTURE_2D, 0);
            }
            prevIndex = currentIndex;
        }
        switch (count) {
        case 3:
            glBegin(GL_TRIANGLES);
            break;
        case 4:
            glBegin(GL_QUADS);
            break;
        }
        for (uint32_t j = 0; j < count; j++) {
            const uint32_t x = static_cast<const uint32_t>(value[j]);
            if (hasTextureCoords)
                glTexCoord2fv(textureCoords[x]);
            if (hasColors)
                glColor4fv(colors[x]);
            if (hasNormals)
                glNormal3fv(normals[x]);
            glVertex3fv(vertices[x]);
        }
        glEnd();
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
#ifndef VPVL_COORDINATE_OPENGL
    glPopMatrix();
    glCullFace(GL_BACK);
#endif
    glEndList();
    model->setUserData(userData);
}

void SceneWidget::unloadStage(const vpvl::XModel *model)
{
    if (model) {
        vpvl::XModelUserData *userData = model->userData();
        glDeleteLists(userData->listID, 1);
        btHashMap<btHashString, GLuint> &textures = userData->textures;
        uint32_t nTextures = textures.size();
        for (uint32_t i = 0; i < nTextures; i++)
            deleteTexture(*textures.getAtIndex(i));
        textures.clear();
        delete userData;
    }
}

void SceneWidget::drawStage(const vpvl::XModel *model)
{
    if (model)
        glCallList(model->userData()->listID);
}

void SceneWidget::drawSurface()
{
    float matrix[16];
    glViewport(0, 0, width(), height());
    glMatrixMode(GL_PROJECTION);
    m_scene->getProjectionMatrix(matrix);
    glLoadMatrixf(matrix);
    glMatrixMode(GL_MODELVIEW);
    m_scene->getModelViewMatrix(matrix);
    glLoadMatrixf(matrix);
    glClearColor(0.0f, 0.0f, 0.25f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    drawStage(m_stage);
    // initialize rendering states
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 1, ~0);
    glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
    glColorMask(0, 0, 0, 0);
    glDepthMask(0);
    glDisable(GL_DEPTH_TEST);
    glPushMatrix();
    // render shadow
    size_t size = 0;
    vpvl::PMDModel **models = m_scene->getRenderingOrder(size);
    for (size_t i = 0; i < size; i++) {
        vpvl::PMDModel *model = models[i];
        drawModelShadow(model);
    }
    glPopMatrix();
    glColorMask(1, 1, 1, 1);
    glDepthMask(1);
    glStencilFunc(GL_EQUAL, 2, ~0);
    glDisable(GL_STENCIL_TEST);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    // render model and edge
    for (size_t i = 0; i < size; i++) {
        vpvl::PMDModel *model = models[i];
        drawModel(model);
        drawModelEdge(model);
    }
}

void SceneWidget::updateFPS()
{
    if (m_timer.elapsed() > 1000) {
        m_currentFPS = m_frameCount;
        m_frameCount = 0;
        m_timer.restart();
    }
    m_frameCount++;
}

void SceneWidget::zoom(bool up, const Qt::KeyboardModifiers &modifiers)
{
    float fovy = m_scene->fovy(), distance = m_scene->distance();
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
    m_scene->setCamera(m_scene->position(), m_scene->angle(), fovy, distance);
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

void SceneWidget::drawInformativeText(QPainter &painter)
{
    painter.setPen(Qt::white);
    if (m_currentFPS > 0)
        painter.drawText(10, 15, QString("FPS: %1").arg(m_currentFPS));
    if (m_selected) {
        QString name = toUnicodeModelName(m_selected);
        painter.drawText(10, 35, QString("Model: %1").arg(name));
    }
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
