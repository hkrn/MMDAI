/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2009-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                2010-2012  hkrn                                    */
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

#include <vpvl/vpvl.h>

#if defined(VPVL_ENABLE_NVIDIA_CG)
#include <vpvl/cg/Renderer.h>
using namespace vpvl::cg;
#elif defined(VPVL_ENABLE_GLSL)
#include <vpvl/gl2/Renderer.h>
using namespace vpvl::gl2;
#else
#include <vpvl/gl/Renderer.h>
using namespace vpvl::gl;
#endif

#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtOpenGL/QtOpenGL>

#ifndef VPVL_NO_BULLET
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#else
VPVL_DECLARE_HANDLE(btDiscreteDynamicsWorld)
#endif

#ifdef VPVL_LINK_ASSIMP
#include <assimp.hpp>
#include <DefaultLogger.h>
#include <Logger.h>
#include <aiPostProcess.h>
#else
VPVL_DECLARE_HANDLE(aiScene)
#endif

namespace
{
    static const int kWidth = 800;
    static const int kHeight = 600;
    static const int kFPS = 60;

    static const std::string kSystemTexturesDir = "../../QMA2/resources/images";
    static const std::string kShaderProgramsDir = "../../QMA2/resources/shaders";
    static const std::string kKernelProgramsDir = "../../QMA2/resources/kernels";
    static const std::string kModelDir = "render/res/lat";
    static const std::string kStageDir = "render/res/stage";
    static const std::string kMotion = "render/res/motion.vmd";
    static const std::string kCamera = "render/res/camera.vmd.404";
    static const std::string kModelName = "normal.pmd";
    static const std::string kStageName = "stage.x";
    static const std::string kStage2Name = "stage2.x";

    typedef QScopedPointer<uint8_t, QScopedPointerArrayDeleter<uint8_t> > ByteArrayPtr;
}

namespace internal
{

static const std::string concatPath(const std::string &dir, const std::string &name) {
    return std::string(QDir(dir.c_str()).absoluteFilePath(name.c_str()).toLocal8Bit());
}

static bool slurpFile(const std::string &path, QByteArray &bytes) {
    QFile file(path.c_str());
    if (file.open(QFile::ReadOnly)) {
        bytes = file.readAll();
        file.close();
        return true;
    }
    else {
        qWarning("slurpFile error at %s: %s", path.c_str(), qPrintable(file.errorString()));
        return false;
    }
}

class Delegate : public Renderer::IDelegate
{
public:
    Delegate(QGLWidget *widget)
        : m_widget(widget),
          m_hardwareSkinning(false)
    {
    }
    ~Delegate()
    {
    }

    bool uploadTexture(const std::string &path, GLuint &textureID, bool isToon) {
        QString pathString = QString::fromLocal8Bit(path.c_str());
        if (!QFileInfo(pathString).exists()) {
            return false;
        }
        uint8_t *rawData = 0;
        QImage image = QImage(pathString).rgbSwapped();
        QGLContext::BindOptions options = QGLContext::LinearFilteringBindOption|QGLContext::InvertedYBindOption;
        textureID = m_widget->bindTexture(QGLWidget::convertToGLFormat(image), GL_TEXTURE_2D,
                                          image.depth() == 32 ? GL_RGBA : GL_RGB, options);
        delete[] rawData;
        if (!isToon) {
            glTexParameteri(textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        qDebug("Loaded a texture (ID=%d): \"%s\"", textureID, qPrintable(pathString));
        return textureID != 0;
    }
    bool uploadToonTexture(const std::string &name, const std::string &dir, GLuint &textureID) {
        QFileInfo info((dir + "/" + name).c_str());
        if (!info.exists()) {
            info.setFile((kSystemTexturesDir + "/" + name).c_str());
            if (!info.exists()) {
                log(Renderer::kLogWarning, "%s is not found, skipped...", qPrintable(info.fileName()));
                return false;
            }
        }
        return uploadTexture(std::string(info.absoluteFilePath().toUtf8()), textureID, true);
    }
    void log(Renderer::LogLevel /* level */, const char *format, ...) {
        va_list ap;
        va_start(ap, format);
        vfprintf(stderr, format, ap);
        fprintf(stderr, "%s", "\n");
        va_end(ap);
    }
    bool loadEffect(vpvl::PMDModel * /* model */, const std::string & /* dir */, std::string & /* source */) {
        return false;
    }
    const std::string loadKernel(Renderer::KernelType type) {
        std::string file;
        switch (type) {
        case Renderer::kModelSkinningKernel:
            file = "skinning.cl";
            break;
        }
        QByteArray bytes;
        std::string path = kKernelProgramsDir + "/" + file;
        if (slurpFile(path, bytes)) {
            log(Renderer::kLogInfo, "Loaded a kernel: %s", path.c_str());
            return std::string(reinterpret_cast<const char *>(bytes.constData()), bytes.size());
        }
        else {
            return std::string();
        }
    }
    const std::string loadShader(Renderer::ShaderType type) {
        std::string file;
        switch (type) {
        case Renderer::kAssetVertexShader:
            file = "asset.vsh";
            break;
        case Renderer::kAssetFragmentShader:
            file = "asset.fsh";
            break;
        case Renderer::kEdgeVertexShader:
            file = m_hardwareSkinning ? "pmd/edge_hws.vsh" : "pmd/edge.vsh";
            break;
        case Renderer::kEdgeFragmentShader:
            file = "pmd/edge.fsh";
            break;
        case Renderer::kModelVertexShader:
            file = m_hardwareSkinning ? "pmd/model_hws.vsh" : "pmd/model.vsh";
            break;
        case Renderer::kModelFragmentShader:
            file = "pmd/model.fsh";
            break;
        case Renderer::kShadowVertexShader:
            file = "pmd/shadow.vsh";
            break;
        case Renderer::kShadowFragmentShader:
            file = "pmd/shadow.fsh";
            break;
        case Renderer::kZPlotVertexShader:
            file = "pmd/zplot.vsh";
            break;
        case Renderer::kZPlotFragmentShader:
            file = "pmd/zplot.fsh";
            break;
        }
        QByteArray bytes;
        std::string path = kShaderProgramsDir + "/" + file;
        if (slurpFile(path, bytes)) {
            log(Renderer::kLogInfo, "Loaded a shader: %s", path.c_str());
            return std::string(reinterpret_cast<const char *>(bytes.constData()), bytes.size());
        }
        else {
            return std::string();
        }
    }
    const std::string toUnicode(const uint8_t *value) {
        QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
        QString s = codec->toUnicode(reinterpret_cast<const char *>(value));
        return std::string(s.toUtf8());
    }

    void setShaderSkinningEnable(bool value) {
        m_hardwareSkinning = value;
    }

private:
    QGLWidget *m_widget;
    bool m_hardwareSkinning;
};

}

class UI : public QGLWidget
{
public:
    UI()
        : QGLWidget(QGLFormat(QGL::SampleBuffers), 0),
          m_world(0),
      #ifndef VPVL_NO_BULLET
          m_dispatcher(&m_config),
          m_broadphase(btVector3(-400.0f, -400.0f, -400.0f), btVector3(400.0f, 400.0f, 400.0f), 1024),
      #endif /* VPVL_NO_BULLET */
          m_delegate(this),
          m_renderer(0),
          m_prevElapsed(0)
    {
        m_renderer = new Renderer(&m_delegate, kWidth, kHeight, kFPS);
#ifndef VPVL_NO_BULLET
        m_world = new btDiscreteDynamicsWorld(&m_dispatcher, &m_broadphase, &m_solver, &m_config);
        m_world->setGravity(btVector3(0.0f, -9.8f * 2.0f, 0.0f));
        m_world->getSolverInfo().m_numIterations = static_cast<int>(10.0f * (30.0f / vpvl::Scene::kFPS));
#endif /* VPVL_NO_BULLET */
    }
    ~UI() {
#ifdef VPVL_LINK_ASSIMP
        Assimp::DefaultLogger::kill();
#endif
        delete m_renderer;
        delete m_world;
    }

    void rotate(float x, float y) {
        vpvl::Scene *scene = m_renderer->scene();
        btVector3 angle = scene->cameraAngle();
        angle.setValue(angle.x() + x, angle.y() + y, angle.z());
        scene->setCameraPerspective(scene->cameraPosition(), angle, scene->fovy(), scene->cameraDistance());
    }
    void translate(float x, float y) {
        vpvl::Scene *scene = m_renderer->scene();
        btVector3 pos = scene->cameraPosition();
        pos.setValue(pos.x() + x, pos.y() + y, pos.z());
        scene->setCameraPerspective(pos, scene->cameraAngle(), scene->fovy(), scene->cameraDistance());
    }

protected:
    virtual void initializeGL() {
        bool shaderSkinning = false;
        m_delegate.setShaderSkinningEnable(shaderSkinning);
        m_renderer->scene()->setSoftwareSkinningEnable(!shaderSkinning);
#ifdef VPVL_ENABLE_OPENCL
        if (m_renderer->initializeAccelerator())
            m_renderer->scene()->setSoftwareSkinningEnable(false);
#endif
#ifdef VPVL_GL2_RENDERER_H_
        if (!m_renderer->createShaderPrograms() || !m_renderer->createShadowFrameBuffers())
            exit(-1);
#endif
        m_renderer->initializeSurface();
        if (!loadScene())
            qFatal("Unable to load scene");

        resize(kWidth, kHeight);
        startTimer(1000.0f / (vpvl::Scene::kFPS * 2));
        m_timer.start();
    }
    virtual void timerEvent(QTimerEvent *) {
        float elapsed = m_timer.elapsed() / static_cast<float>(vpvl::Scene::kFPS);
        float diff = elapsed - m_prevElapsed;
        m_prevElapsed = elapsed;
        if (diff < 0)
            diff = elapsed;
        vpvl::Scene *scene = m_renderer->scene();
        scene->updateModelView();
        scene->updateProjection();
        scene->advanceMotion(diff);
        m_renderer->updateAllModel();
        updateGL();
    }
    virtual void mousePressEvent(QMouseEvent *event) {
        m_prevPos = event->pos();
    }
    virtual void mouseMoveEvent(QMouseEvent *event) {
        if (event->buttons() & Qt::LeftButton) {
            Qt::KeyboardModifiers modifiers = event->modifiers();
            QPoint diff = event->pos() - m_prevPos;
            if (modifiers & Qt::ShiftModifier) {
                translate(diff.x() * -0.1f, diff.y() * 0.1f);
            }
            else {
                rotate(diff.y() * 0.5f, diff.x() * 0.5f);
            }
            m_prevPos = event->pos();
        }
    }
    virtual void wheelEvent(QWheelEvent *event) {
        Qt::KeyboardModifiers modifiers = event->modifiers();
        vpvl::Scene *scene = m_renderer->scene();
        float fovy = scene->fovy(), distance = scene->cameraDistance();
        float fovyStep = 1.0f, distanceStep = 4.0f;
        if (modifiers & Qt::ControlModifier && modifiers & Qt::ShiftModifier) {
            fovy = event->delta() > 0 ? fovy - fovyStep : fovy + fovyStep;
        }
        else {
            if (modifiers & Qt::ControlModifier)
                distanceStep *= 5.0f;
            else if (modifiers & Qt::ShiftModifier)
                distanceStep *= 0.2f;
            if (distanceStep != 0.0f)
                distance = event->delta() > 0 ? distance - distanceStep : distance + distanceStep;
        }
        scene->setCameraPerspective(scene->cameraPosition(), scene->cameraAngle(), fovy, distance);
    }
    virtual void resizeGL(int w, int h) {
        m_renderer->resize(w, h);
    }
    virtual void paintGL() {
        glClearColor(0, 0, 1, 1);
        //m_renderer->renderZPlot();
        m_renderer->clear();
        m_renderer->renderProjectiveShadow();
        m_renderer->renderAllModels();
        m_renderer->renderAllAssets();
    }

private:
    bool loadScene() {
        QByteArray bytes;
        vpvl::PMDModel *model = new vpvl::PMDModel();
        if (!internal::slurpFile(internal::concatPath(kModelDir, kModelName), bytes) ||
                !model->load(reinterpret_cast<const uint8_t *>(bytes.constData()), bytes.size())) {
            m_delegate.log(Renderer::kLogWarning, "Failed parsing the model");
            delete model;
            return false;
        }
        vpvl::Scene *scene = m_renderer->scene();
        //scene.setCamera(btVector3(0.0f, 50.0f, 0.0f), btVector3(0.0f, 0.0f, 0.0f), 60.0f, 50.0f);
        scene->setWorld(m_world);

        m_renderer->uploadModel(model, kModelDir);
        model->setEdgeOffset(0.5f);
#ifdef VPVL_LINK_ASSIMP
        Assimp::Logger::LogSeverity severity = Assimp::Logger::VERBOSE;
        Assimp::DefaultLogger::create("", severity, aiDefaultLogStream_STDOUT);
        loadAsset(kStageDir, kStageName);
        loadAsset(kStageDir, kStage2Name);
#endif

        if (!internal::slurpFile(kMotion, bytes) ||
                !m_motion.load(reinterpret_cast<const uint8_t *>(bytes.constData()), bytes.size()))
            m_delegate.log(Renderer::kLogWarning, "Failed parsing the model motion, skipped...");
        else
            model->addMotion(&m_motion);

        if (!internal::slurpFile(kCamera, bytes) ||
                !m_camera.load(reinterpret_cast<const uint8_t *>(bytes.constData()), bytes.size()))
            m_delegate.log(Renderer::kLogWarning, "Failed parsing the camera motion, skipped...");
        else
            scene->setCameraMotion(&m_camera);
        m_renderer->updateAllModel();

        return true;
    }
    vpvl::Asset *loadAsset(const std::string &dir, const std::string &name) {
        vpvl::Asset *asset = new vpvl::Asset();
        const std::string path = internal::concatPath(dir, name);
        if (asset->load(path.c_str())) {
            m_renderer->uploadAsset(asset, dir);
            return asset;
        }
        else {
            m_delegate.log(Renderer::kLogWarning,
                           "Failed parsing the asset %s, skipped...",
                           path.c_str());
            return 0;
        }
    }

    QElapsedTimer m_timer;
    QPoint m_prevPos;
    btDiscreteDynamicsWorld *m_world;
#ifndef VPVL_NO_BULLET
    btDefaultCollisionConfiguration m_config;
    btCollisionDispatcher m_dispatcher;
    btAxisSweep3 m_broadphase;
    btSequentialImpulseConstraintSolver m_solver;
#endif /* VPVL_NO_BULLET */
    internal::Delegate m_delegate;
    Renderer *m_renderer;
    vpvl::VMDMotion m_motion;
    vpvl::VMDMotion m_camera;
    float m_prevElapsed;
};

