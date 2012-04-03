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

#include <vpvl2/vpvl2.h>

#if defined(VPVL2_ENABLE_NVIDIA_CG)
#include <vpvl/cg/Renderer.h>
using namespace vpvl2::cg;
#elif defined(VPVL2_ENABLE_GLSL)
#include <vpvl2/gl2/Renderer.h>
using namespace vpvl2;
using namespace vpvl2::gl2;
#else
#include <vpvl2/gl2/Renderer.h>
using namespace vpvl::gl;
#endif

#include <QtCore/QtCore>
#include <QtGui/QtGui>
#include <QtOpenGL/QtOpenGL>

#ifndef VPVL2_NO_BULLET
#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>
#else
VPVL2_DECLARE_HANDLE(btDiscreteDynamicsWorld)
#endif

#ifdef VPVL2_LINK_ASSIMP
#include <assimp.hpp>
#include <DefaultLogger.h>
#include <Logger.h>
#include <aiPostProcess.h>
#else
VPVL2_DECLARE_HANDLE(aiScene)
#endif

namespace
{
    static const int kWidth = 800;
    static const int kHeight = 600;
    static const int kFPS = 60;

    static const std::string kSystemTexturesDir = "../../QMA2/resources/images";
    static const std::string kShaderProgramsDir = "../../QMA2/resources/shaders/pmx";
    static const std::string kKernelProgramsDir = "../../QMA2/resources/kernels";
    static const std::string kModelDir = "render/res/miku";
    static const std::string kStageDir = "render/res/stage";
    static const std::string kMotion = "render/res/motion.vmd";
    static const std::string kCamera = "render/res/camera.vmd.404";
    static const std::string kModelName = "miku.pmx";
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

class String : public IString {
public:
    String(const QString &s) : m_value(s) {
    }
    ~String() {
    }

    IString *clone() const {
        return new String(m_value);
    }
    const HashString toHashString() const {
        const QByteArray &bytes = m_value.toUtf8();
        m_keys.insert(bytes);
        return HashString(bytes.constData());
    }
    bool equals(const IString *value) const {
        return m_value == static_cast<const String *>(value)->value();
    }
    const QString &value() const {
        return m_value;
    }
    const uint8_t *toByteArray() const {
        return reinterpret_cast<const uint8_t *>(m_value.toUtf8().constData());
    }
    size_t length() const {
        return m_value.length();
    }

private:
    mutable QSet<QByteArray> m_keys;
    QString m_value;
};

class Encoding : public IEncoding {
public:
    Encoding()
        : m_sjis(QTextCodec::codecForName("Shift-JIS")),
          m_utf8(QTextCodec::codecForName("UTF-8")),
          m_utf16(QTextCodec::codecForName("UTF-16"))
    {
    }
    ~Encoding() {
    }

    IString *toString(const uint8_t *value, size_t size, IString::Codec codec) const {
        IString *s = 0;
        const char *str = reinterpret_cast<const char *>(value);
        switch (codec) {
        case IString::kShiftJIS:
            s = new String(m_sjis->toUnicode(str, size));
            break;
        case IString::kUTF8:
            s = new String(m_utf8->toUnicode(str, size));
            break;
        case IString::kUTF16:
            s = new String(m_utf16->toUnicode(str, size));
            break;
        }
        return s;
    }
    IString *toString(const uint8_t *value, IString::Codec codec, size_t maxlen) const {
        size_t size = qstrnlen(reinterpret_cast<const char *>(value), maxlen);
        return toString(value, size, codec);
    }
    uint8_t *toByteArray(const IString *value, IString::Codec codec) const {
        const String *s = static_cast<const String *>(value);
        QByteArray bytes;
        switch (codec) {
        case IString::kShiftJIS:
            bytes = m_sjis->fromUnicode(s->value());
            break;
        case IString::kUTF8:
            bytes = m_utf8->fromUnicode(s->value());
            break;
        case IString::kUTF16:
            bytes = m_utf16->fromUnicode(s->value());
            break;
        }
        size_t size = bytes.length();
        uint8_t *data = new uint8_t[size + 1];
        memcpy(data, bytes.constData(), size);
        data[size] = 0;
        return data;
    }
    void disposeByteArray(uint8_t *value) const {
        delete[] value;
    }
    pmx::Model *m_model;

private:
    QTextCodec *m_sjis;
    QTextCodec *m_utf8;
    QTextCodec *m_utf16;
};

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

    bool uploadTexture(const std::string &path, const std::string &dir, GLuint &textureID, bool isToon) {
        const QString &pathString = QString::fromLocal8Bit((dir + "/" + path).c_str());
        const QFileInfo info(pathString);
        if (info.isDir() || !info.exists())
            return false;
        const QImage &image = QImage(pathString).rgbSwapped();
        QGLContext::BindOptions options = QGLContext::LinearFilteringBindOption|QGLContext::InvertedYBindOption;
        textureID = m_widget->bindTexture(QGLWidget::convertToGLFormat(image), GL_TEXTURE_2D, GL_RGBA, options);
        if (!isToon) {
            glTexParameteri(textureID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(textureID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        }
        qDebug("Loaded a texture (ID=%d): \"%s\"", textureID, qPrintable(pathString));
        return textureID != 0;
    }

    bool uploadToonTexture(int sharedToonTextureIndex, GLuint &textureID) {
        switch (sharedToonTextureIndex) {
        case 0:
            return uploadTexture("toon01.bmp", kSystemTexturesDir, textureID, true);
        case 1:
            return uploadTexture("toon02.bmp", kSystemTexturesDir, textureID, true);
        case 2:
            return uploadTexture("toon03.bmp", kSystemTexturesDir, textureID, true);
        case 3:
            return uploadTexture("toon04.bmp", kSystemTexturesDir, textureID, true);
        case 4:
            return uploadTexture("toon05.bmp", kSystemTexturesDir, textureID, true);
        case 5:
            return uploadTexture("toon06.bmp", kSystemTexturesDir, textureID, true);
        case 6:
            return uploadTexture("toon07.bmp", kSystemTexturesDir, textureID, true);
        case 7:
            return uploadTexture("toon08.bmp", kSystemTexturesDir, textureID, true);
        case 8:
            return uploadTexture("toon09.bmp", kSystemTexturesDir, textureID, true);
        case 9:
            return uploadTexture("toon10.bmp", kSystemTexturesDir, textureID, true);
        default:
            return false;
        }
    }

    void log(Renderer::LogLevel /* level */, const char *format, ...) {
        va_list ap;
        va_start(ap, format);
        vfprintf(stderr, format, ap);
        fprintf(stderr, "%s", "\n");
        va_end(ap);
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
            file = m_hardwareSkinning ? "edge_hws.vsh" : "edge.vsh";
            break;
        case Renderer::kEdgeFragmentShader:
            file = "edge.fsh";
            break;
        case Renderer::kModelVertexShader:
            file = m_hardwareSkinning ? "model_hws.vsh" : "model.vsh";
            break;
        case Renderer::kModelFragmentShader:
            file = "model.fsh";
            break;
        case Renderer::kShadowVertexShader:
            file = "shadow.vsh";
            break;
        case Renderer::kShadowFragmentShader:
            file = "shadow.fsh";
            break;
        case Renderer::kZPlotVertexShader:
            file = "zplot.vsh";
            break;
        case Renderer::kZPlotFragmentShader:
            file = "zplot.fsh";
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
    const std::string toUnicode(const IString *value) {
        if (value) {
            const QString &s = static_cast<const String *>(value)->value();
            return std::string(s.toUtf8());
        }
        return "";
    }

    void setShaderSkinningEnable(bool value) {
        m_hardwareSkinning = value;
    }

private:
    QGLWidget *m_widget;
    bool m_hardwareSkinning;
};

}

QDebug operator<<(QDebug debug, const Vector3 &v)
{
    debug.nospace() << "(x=" << v.x() << ", y=" << v.y() << ", z=" << v.z() << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const Color &v)
{
    debug.nospace() << "(r=" << v.x() << ", g=" << v.y() << ", b=" << v.z() << ", a=" << v.w() << ")";
    return debug;
}

QDebug operator<<(QDebug debug, const IString *str)
{
    if (str) {
        debug.nospace() << reinterpret_cast<const internal::String *>(str)->value();
    }
    else {
        debug.nospace() << "\"\"";
    }
    return debug;
}

QDebug operator<<(QDebug debug, const pmx::Bone *bone)
{
    debug.nospace()
            << "Bone id=" << bone->id()
            << " name=" << bone->name()
            << " english=" << bone->englishName()
            << " origin=" << bone->origin();
    if (bone->parentBone())
        debug << " parent=" << bone->parentBone()->name();
    debug << " index=" << bone->index() << "\n" << " offset=" << bone->origin();
    if (bone->hasIKLinks()) {
        debug << " targetBone=" << bone->targetBone()->name()
              << " constraintAngle=" << bone->constraintAngle();
    }
    if (bone->hasPositionInherence()) {
        debug << " parentPositionInherenceBone=" << bone->parentInherenceBone()->name()
              << " weight=" << bone->weight();
    }
    if (bone->hasRotationInherence()) {
        debug << " parentRotationInherenceBone=" << bone->parentInherenceBone()->name()
              << " weight=" << bone->weight();
    }
    if (bone->isAxisFixed())
        debug << " axis=" << bone->axis();
    if (bone->hasLocalAxis())
        debug << " axisX=" << bone->axisX() << " axisZ=" << bone->axisZ();
    return debug.space();
}

QDebug operator<<(QDebug debug, const pmx::Material *material)
{
    debug.nospace()
            << "Material name=" << material->name()
            << " english=" << material->englishName()
            << " mainTexture=" << material->mainTexture()
            << " sphereTexture=" << material->sphereTexture()
            << " toonTexture=" << material->toonTexture()
            << "\n"
            << " ambient=" << material->ambient()
            << " diffuse=" << material->diffuse()
            << " specular=" << material->specular()
            << " edgeColor=" << material->edgeColor()
            << "\n"
            << " shininess=" << material->shininess()
            << " edgeSize=" << material->edgeSize()
            << " indices=" << material->indices()
            << " isSharedToonTextureUsed=" << material->isSharedToonTextureUsed()
            << " isCullDisabled=" << material->isCullFaceDisabled()
            << " hasShadow=" << material->hasShadow()
            << " isShadowMapDrawin=" << material->isShadowMapDrawn()
            << " isEdgeDrawn=" << material->isEdgeDrawn()
               ;
    return debug.space();
}

QDebug operator<<(QDebug debug, const pmx::Morph *morph)
{
    debug.nospace()
            << "Morph name=" << morph->name()
            << " english=" << morph->englishName()
               ;
    return debug.space();
}

class UI : public QGLWidget
{
public:
    static const qreal kCameraNear = 0.5;
    static const qreal kCameraFar = 10000.0;

    UI()
        : QGLWidget(QGLFormat(QGL::SampleBuffers), 0),
          m_rotation(Quaternion::getIdentity()),
          m_position(0.0, 10.0, 0.0),
          m_angle(kZeroV3),
          m_fovy(30.0),
          m_distance(50.0),
          m_world(0),
          m_encoding(0),
          m_model(0),
          m_motion(0),
      #ifndef VPVL_NO_BULLET
          m_dispatcher(&m_config),
          m_broadphase(Vector3(-10000.0f, -10000.0f, -10000.0f), Vector3(10000.0f, 10000.0f, 10000.0f), 1024),
      #endif /* VPVL_NO_BULLET */
          m_delegate(this),
          m_renderer(0),
          m_prevElapsed(0),
          m_currentFrameIndex(0)
    {
        internal::Encoding *encoding = new internal::Encoding();
        m_encoding = encoding;
        m_model = new pmx::Model(encoding);
        encoding->m_model = m_model;
        m_renderer = new Renderer(&m_delegate, kWidth, kHeight, kFPS);
#ifndef VPVL_NO_BULLET
        m_world = new btDiscreteDynamicsWorld(&m_dispatcher, &m_broadphase, &m_solver, &m_config);
        m_world->setGravity(btVector3(0.0f, -9.8f * 2.0f, 0.0f));
        m_world->getSolverInfo().m_numIterations = static_cast<int>(10.0f);
#endif /* VPVL_NO_BULLET */
    }
    ~UI() {
#ifdef VPVL_LINK_ASSIMP
        Assimp::DefaultLogger::kill();
#endif
        delete m_renderer;
        delete m_world;
        delete m_encoding;
        delete m_motion;
    }

    void rotate(float x, float y) {
        m_angle.setX(m_angle.x() + x);
        m_angle.setY(m_angle.y() + y);
        const Quaternion rx(Vector3(1.0, 0.0, 0.0), btRadians(m_angle.x())),
                ry(Vector3(0.0, 1.0, 0.0), btRadians(m_angle.y())),
                rz(Vector3(0.0, 0.0, 1.0), btRadians(m_angle.z()));
        m_rotation = rz * rx * ry;
    }
    void translate(float x, float y) {
        m_position.setX(m_position.x() + x);
        m_position.setY(m_position.y() + y);
    }

protected:
    void initializeGL() {
        bool shaderSkinning = false;
        m_delegate.setShaderSkinningEnable(shaderSkinning);
        //m_renderer->scene()->setSoftwareSkinningEnable(!shaderSkinning);
#if 0
        if (m_renderer->initializeAccelerator())
            m_renderer->scene()->setSoftwareSkinningEnable(false);
#endif
#ifdef VPVL2_GL2_RENDERER_H_
        if (!m_renderer->createShaderPrograms())
            exit(-1);
#endif
        m_renderer->initializeSurface();
        if (!loadScene())
            qFatal("Unable to load scene");

        resize(kWidth, kHeight);
        startTimer(1000.0f / 60.0f);
        m_timer.start();
    }
    void timerEvent(QTimerEvent *) {
        float elapsed = m_timer.elapsed() / static_cast<float>(60.0f);
        float diff = elapsed - m_prevElapsed;
        m_prevElapsed = elapsed;
        if (diff < 0)
            diff = elapsed;
        m_renderer->updateAllModel();
        updateGL();
    }
    void mousePressEvent(QMouseEvent *event) {
        m_prevPos = event->pos();
    }
    void mouseMoveEvent(QMouseEvent *event) {
        if (event->buttons() & Qt::LeftButton) {
            Qt::KeyboardModifiers modifiers = event->modifiers();
            const QPoint &diff = event->pos() - m_prevPos;
            if (modifiers & Qt::ShiftModifier) {
                translate(diff.x() * -0.1f, diff.y() * 0.1f);
            }
            else {
                rotate(diff.y() * 0.5f, diff.x() * 0.5f);
            }
            m_prevPos = event->pos();
        }
    }
    void keyPressEvent(QKeyEvent *event) {
        if (m_motion && event->modifiers() & Qt::SHIFT) {
            switch (event->key()) {
            case Qt::Key_Left:
                m_currentFrameIndex -= 1.0f;
                btSetMax(m_currentFrameIndex, 0.0f);
                m_motion->seek(m_currentFrameIndex);
                break;
            case Qt::Key_Right:
                m_currentFrameIndex += 1.0;
                btSetMin(m_currentFrameIndex, m_motion->maxFrameIndex());
                m_motion->seek(m_currentFrameIndex);
                break;
            }
            qDebug() << m_currentFrameIndex;
            if (m_motion->isReachedTo(m_motion->maxFrameIndex()))
                m_motion->reset();
            m_model->update();
        }
    }
    void wheelEvent(QWheelEvent *event) {
        Qt::KeyboardModifiers modifiers = event->modifiers();
        if (modifiers & Qt::ControlModifier && modifiers & Qt::ShiftModifier) {
            const qreal fovyStep = 1.0;
            m_fovy = qMax(event->delta() > 0 ? m_fovy - fovyStep : m_fovy + fovyStep, 0.0);
        }
        else {
            qreal distanceStep = 4.0;
            if (modifiers & Qt::ControlModifier)
                distanceStep *= 5.0f;
            else if (modifiers & Qt::ShiftModifier)
                distanceStep *= 0.2f;
            if (distanceStep != 0.0f)
                m_distance = event->delta() > 0 ? m_distance - distanceStep : m_distance + distanceStep;
        }
    }
    void resizeGL(int w, int h) {
        m_renderer->resize(w, h);
    }
    void paintGL() {
        glClearColor(0, 0, 1, 1);
        m_renderer->clear();
        updateModelViewMatrix();
        updateProjectionMatrix();
        m_renderer->renderProjectiveShadow();
        m_renderer->renderAllModels();
    }

private:
    void updateModelViewMatrix() {
        float matrixf[16];
        m_modelviewMatrix.setIdentity();
        m_modelviewMatrix.setRotation(m_rotation);
        Vector3 position = m_modelviewMatrix.getBasis() * -m_position;
        position.setZ(position.z() - m_distance);
        m_modelviewMatrix.setOrigin(position);
        m_modelviewMatrix.getOpenGLMatrix(matrixf);
        m_renderer->setModelViewMatrix(matrixf);
    }
    void updateProjectionMatrix() {
        qreal matrixd[16];
        float matrixf[16];
        m_projectionMatrix.setToIdentity();
        m_projectionMatrix.perspective(m_fovy, kWidth / float(kHeight), kCameraNear, kCameraFar);
        m_projectionMatrix.copyDataTo(matrixd);
        for (int i = 0; i < 16; i++)
            matrixf[i] = matrixd[i];
        m_renderer->setProjectionMatrix(matrixf);
    }
    bool loadScene() {
        QByteArray bytes;
        if (!internal::slurpFile(internal::concatPath(kModelDir, kModelName), bytes)) {
            m_delegate.log(Renderer::kLogWarning, "Failed loading the model");
            return false;
        }
        if (!m_model->load(reinterpret_cast<const uint8_t *>(bytes.constData()), bytes.size())) {
            m_delegate.log(Renderer::kLogWarning, "Failed parsing the model: %d", m_model->error());
            return false;
        }

        /*
        vpvl::Scene *scene = m_renderer->scene();
        //scene.setCamera(btVector3(0.0f, 50.0f, 0.0f), btVector3(0.0f, 0.0f, 0.0f), 60.0f, 50.0f);
        scene->setWorld(m_world);
        */

        m_renderer->uploadModel(m_model, kModelDir);
        //m_model->setEdgeOffset(0.5f);
#ifdef VPVL_LINK_ASSIMP
        Assimp::Logger::LogSeverity severity = Assimp::Logger::VERBOSE;
        Assimp::DefaultLogger::create("", severity, aiDefaultLogStream_STDOUT);
        loadAsset(kStageDir, kStageName);
        loadAsset(kStageDir, kStage2Name);
#endif

#if 1
        for (int i = 0; i < m_model->materials().count(); i++)
            qDebug() << m_model->materials().at(i);
        for (int i = 0; i < m_model->bones().count(); i++)
            qDebug() << m_model->bones().at(i);
        for (int i = 0; i < m_model->morphs().count(); i++)
            qDebug() << m_model->morphs().at(i);
        /*
        for (int i = 0; i < m_model->rigidBodies().count(); i++)
            qDebug("rbody%d: %s", i, m_delegate.toUnicode(m_model->rigidBodies()[i]->name()).c_str());
        for (int i = 0; i < m_model->joints().count(); i++)
            qDebug("joint%d: %s", i, m_delegate.toUnicode(m_model->joints()[i]->name()).c_str());
            */
#endif

        if (internal::slurpFile(kMotion, bytes)) {
            m_motion = new vmd::Motion(m_model, m_encoding);
            qDebug() << "Loaded motion:" << m_motion->load(reinterpret_cast<const uint8_t *>(bytes.constData()), bytes.size());
            qDebug() << "maxFrameIndex:" << m_motion->maxFrameIndex();
            m_motion->seek(0.0);
            m_model->update();
        }
        else {
            m_delegate.log(Renderer::kLogWarning, "Failed parsing the model motion, skipped...");
        }

        /*
        if (!internal::slurpFile(kCamera, bytes) ||
                !m_camera.load(reinterpret_cast<const uint8_t *>(bytes.constData()), bytes.size()))
            m_delegate.log(Renderer::kLogWarning, "Failed parsing the camera motion, skipped...");
        else
            scene->setCameraMotion(&m_camera);
            */
        m_renderer->updateAllModel();

        return true;
    }
#ifdef VPVL2_LINK_ASSIMP
    Asset *loadAsset(const std::string &dir, const std::string &name) {
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
#endif

    QElapsedTimer m_timer;
    QPoint m_prevPos;
    QMatrix4x4 m_projectionMatrix;
    Transform m_modelviewMatrix;
    Quaternion m_rotation;
    Vector3 m_position;
    Vector3 m_angle;
    qreal m_fovy;
    qreal m_distance;
    btDiscreteDynamicsWorld *m_world;
    IEncoding *m_encoding;
    pmx::Model *m_model;
    IMotion *m_motion;
#ifndef VPVL_NO_BULLET
    btDefaultCollisionConfiguration m_config;
    btCollisionDispatcher m_dispatcher;
    btAxisSweep3 m_broadphase;
    btSequentialImpulseConstraintSolver m_solver;
#endif /* VPVL_NO_BULLET */
    internal::Delegate m_delegate;
    Renderer *m_renderer;
    //VMDMotion m_camera;
    float m_prevElapsed;
    float m_currentFrameIndex;
};

