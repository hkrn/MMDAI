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

#include <vpvl/vpvl.h>

#if defined(VPVL_USE_NVIDIA_CG)
#include <vpvl/cg/Renderer.h>
using namespace vpvl::cg;
#elif defined(VPVL_USE_GLSL)
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

static const std::string kSystemDir = "render/res/system";
static const std::string kModelDir = "render/res/lat";
static const std::string kStageDir = "render/res/stage";
static const std::string kMotion = "render/res/motion.vmd.404";
static const std::string kCamera = "render/res/camera.vmd.404";
static const std::string kModelName = "normal.pmd";
static const std::string kStageName = "stage.x";
static const std::string kStage2Name = "stage2.x";

typedef QScopedPointer<uint8_t, QScopedPointerArrayDeleter<uint8_t> > ByteArrayPtr;

QImage LoadTGA(const QString &path, uint8_t *&rawData) {
    QFile file(path);
    if (file.open(QFile::ReadOnly) && file.size() > 18) {
        QByteArray data = file.readAll();
        uint8_t *ptr = reinterpret_cast<uint8_t *>(data.data());
        uint8_t field = *reinterpret_cast<uint8_t *>(ptr);
        uint8_t type = *reinterpret_cast<uint8_t *>(ptr + 2);
        if (type != 2 /* full color */ && type != 10 /* full color + RLE */) {
            qWarning("Loaded TGA image type is not full color: %s", qPrintable(path));
            return QImage();
        }
        uint16_t width = *reinterpret_cast<uint16_t *>(ptr + 12);
        uint16_t height = *reinterpret_cast<uint16_t *>(ptr + 14);
        uint8_t depth = *reinterpret_cast<uint8_t *>(ptr + 16); /* 24 or 32 */
        uint8_t flags = *reinterpret_cast<uint8_t *>(ptr + 17);
        if (width == 0 || height == 0 || (depth != 24 && depth != 32)) {
            qWarning("Invalid TGA image (width=%d, height=%d, depth=%d): %s",
                     width, height, depth, qPrintable(path));
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
        qWarning("Cannot open file %s: %s", qPrintable(path), qPrintable(file.errorString()));
        return QImage();
    }
}
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

class Delegate : public IDelegate
{
public:
    Delegate(QGLWidget *widget, const std::string &system)
        : m_widget(widget),
          m_system(system)
    {
    }
    ~Delegate()
    {
    }

    bool loadTexture(const std::string &path, GLuint &textureID) {
        QString pathString = QString::fromLocal8Bit(path.c_str());
        if (!QFileInfo(pathString).exists()) {
            return false;
        }
        uint8_t *rawData = 0;
        QImage image = pathString.endsWith(".tga", Qt::CaseInsensitive)
                ? LoadTGA(pathString, rawData) : QImage(pathString).rgbSwapped();
        QGLContext::BindOptions options = QGLContext::LinearFilteringBindOption|QGLContext::InvertedYBindOption;
        textureID = m_widget->bindTexture(QGLWidget::convertToGLFormat(image), GL_TEXTURE_2D,
                                          image.depth() == 32 ? GL_RGBA : GL_RGB, options);
        delete[] rawData;
        qDebug("Loaded a texture (ID=%d): \"%s\"", textureID, qPrintable(pathString));
        return textureID != 0;
    }
    bool loadToonTexture(const std::string &name, const std::string &dir, GLuint &textureID) {
        QFileInfo info((dir + "/" + name).c_str());
        if (!info.exists()) {
            info.setFile((m_system + "/" + name).c_str());
            if (!info.exists()) {
                log(kLogWarning, "%s is not found, skipped...", qPrintable(info.fileName()));
                return false;
            }
        }
        return loadTexture(std::string(info.absoluteFilePath().toUtf8()), textureID);
    }
    void log(LogLevel /* level */, const char *format, ...) {
        va_list ap;
        va_start(ap, format);
        vfprintf(stderr, format, ap);
        fprintf(stderr, "%s", "\n");
        va_end(ap);
    }
#ifdef VPVL_USE_NVIDIA_CG
    bool loadEffect(vpvl::PMDModel *model, const std::string &dir, std::string &source) {
        return false;
    }
#endif
#ifdef VPVL_GL2_RENDERER_H_
    const std::string loadShader(ShaderType type) {
        std::string file;
        switch (type) {
        case kAssetVertexShader:
            file = "asset.vsh";
            break;
        case kAssetFragmentShader:
            file = "asset.fsh";
            break;
        case kEdgeVertexShader:
            file = "edge.vsh";
            break;
        case kEdgeFragmentShader:
            file = "edge.fsh";
            break;
        case kModelVertexShader:
            file = "model.vsh";
            break;
        case kModelFragmentShader:
            file = "model.fsh";
            break;
        case kShadowVertexShader:
            file = "shadow.vsh";
            break;
        case kShadowFragmentShader:
            file = "shadow.fsh";
            break;
        }
        QByteArray bytes;
        std::string path = m_system + "/" + file;
        if (slurpFile(path, bytes)) {
            log(kLogInfo, "Loaded a shader: %s", path.c_str());
            return std::string(reinterpret_cast<const char *>(bytes.constData()), bytes.size());
        }
        else {
            return std::string();
        }
    }
#endif
    const std::string toUnicode(const uint8_t *value) {
        QTextCodec *codec = QTextCodec::codecForName("Shift-JIS");
        QString s = codec->toUnicode(reinterpret_cast<const char *>(value));
        return std::string(s.toUtf8());
    }

private:
    QGLWidget *m_widget;
    std::string m_system;
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
          m_delegate(this, kSystemDir),
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
        btVector3 angle = scene->angle();
        angle.setValue(angle.x() + x, angle.y() + y, angle.z());
        scene->setCameraPerspective(scene->position(), angle, scene->fovy(), scene->distance());
    }
    void translate(float x, float y) {
        vpvl::Scene *scene = m_renderer->scene();
        btVector3 pos = scene->position();
        pos.setValue(pos.x() + x, pos.y() + y, pos.z());
        scene->setCameraPerspective(pos, scene->angle(), scene->fovy(), scene->distance());
    }

protected:
    virtual void initializeGL() {
        GLenum err;
        if (!Renderer::initializeGLEW(err))
            qFatal("Unable to init GLEW: %s", glewGetErrorString(err));

#ifdef VPVL_GL2_RENDERER_H_
        m_renderer->createPrograms();
#endif
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
        scene->updateModelView(0);
        scene->updateProjection(0);
        scene->advanceMotion(diff);
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
        float fovy = scene->fovy(), distance = scene->distance();
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
        scene->setCameraPerspective(scene->position(), scene->angle(), fovy, distance);
    }
    virtual void resizeGL(int w, int h) {
        m_renderer->resize(w, h);
    }
    virtual void paintGL() {
        glClearColor(0, 0, 1, 1);
        m_renderer->initializeSurface();
        m_renderer->drawSurface();
    }

private:
    bool loadScene() {
        QByteArray bytes;
        vpvl::PMDModel *model = new vpvl::PMDModel();
        if (!internal::slurpFile(internal::concatPath(kModelDir, kModelName), bytes) ||
                !model->load(reinterpret_cast<const uint8_t *>(bytes.constData()), bytes.size())) {
            m_delegate.log(IDelegate::kLogWarning, "Failed parsing the model");
            delete model;
            return false;
        }
        vpvl::Scene *scene = m_renderer->scene();
        //scene.setCamera(btVector3(0.0f, 50.0f, 0.0f), btVector3(0.0f, 0.0f, 0.0f), 60.0f, 50.0f);
        scene->setWorld(m_world);

        m_renderer->loadModel(model, kModelDir);
#ifdef VPVL_LINK_ASSIMP
        Assimp::Logger::LogSeverity severity = Assimp::Logger::VERBOSE;
        Assimp::DefaultLogger::create("", severity, aiDefaultLogStream_STDOUT);
        loadAsset(kStageDir, kStageName);
        loadAsset(kStageDir, kStage2Name);
#endif

        if (!internal::slurpFile(kMotion, bytes) ||
                !m_motion.load(reinterpret_cast<const uint8_t *>(bytes.constData()), bytes.size()))
            m_delegate.log(IDelegate::kLogWarning, "Failed parsing the model motion, skipped...");
        else
            model->addMotion(&m_motion);

        if (!internal::slurpFile(kCamera, bytes) ||
                !m_camera.load(reinterpret_cast<const uint8_t *>(bytes.constData()), bytes.size()))
            m_delegate.log(IDelegate::kLogWarning, "Failed parsing the camera motion, skipped...");
        else
            scene->setCameraMotion(&m_camera);

        const vpvl::Color &color = scene->lightColor();
        const vpvl::Scalar &intensity = 0.6f;
    #if 0 // MMD like toon
        const vpvl::Vector3 &a = color * intensity * 2.0f;
        const vpvl::Vector3 &d = color * 0.0f;
        const vpvl::Vector3 &s = color * intensity;
    #else // no toon
        const vpvl::Vector3 &a = color;
        const vpvl::Vector3 &d = color * intensity;
        const vpvl::Vector3 &s = color;
    #endif
        const vpvl::Color ambient(a.x(), a.y(), a.z(), 1.0f);
        const vpvl::Color diffuse(d.x(), d.y(), d.z(), 1.0f);
        const vpvl::Color specular(s.x(), s.y(), s.z(), 1.0f);
        scene->setLightComponent(ambient, diffuse, specular);

        return true;
    }
    vpvl::Asset *loadAsset(const std::string &dir, const std::string &name) {
        vpvl::Asset *asset = new vpvl::Asset();
        const std::string path = internal::concatPath(dir, name);
        if (asset->load(path.c_str())) {
            m_renderer->loadAsset(asset, dir);
            return asset;
        }
        else {
            m_delegate.log(IDelegate::kLogWarning,
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

