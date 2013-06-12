/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2013  hkrn                                    */
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

#include "vpvl2/extensions/osx/ql4pmx/Context.h"

#include <vpvl2/vpvl2.h>
#include <vpvl2/extensions/Pose.h>
#include <vpvl2/extensions/World.h>
#include <unicode/udata.h> /* for udata_setCommonData */

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <Cocoa/Cocoa.h>

#include <fstream>

using namespace vpvl2;
using namespace vpvl2::extensions;
using namespace vpvl2::extensions::icu4c;

namespace vpvl2 {
namespace extensions {
namespace osx {
namespace ql4pmx {

RenderContext::RenderContext(Scene *sceneRef, IEncoding *encodingRef, StringMap *configRef)
    : BaseRenderContext(sceneRef, encodingRef, configRef)
{
}

RenderContext::~RenderContext()
{
}

void *RenderContext::findProcedureAddress(const void **candidatesPtr) const
{
    const char **candidates = reinterpret_cast<const char **>(candidatesPtr);
    const char *candidate = candidates[0];
    int i = 0;
    while (candidate) {
        void *address = reinterpret_cast<void *>(OSMesaGetProcAddress(candidate));
        if (address) {
            return address;
        }
        candidate = candidates[++i];
    }
    return 0;
}

bool RenderContext::mapFile(const UnicodeString &path, MapBuffer *buffer) const
{
    int fd = ::open(String::toStdString(path).c_str(), O_RDONLY);
    if (fd == -1) {
        return false;
    }
    struct stat sb;
    if (::fstat(fd, &sb) == -1) {
        return false;
    }
    uint8_t *address = static_cast<uint8_t *>(::mmap(0, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0));
    if (address == reinterpret_cast<uint8_t *>(-1)) {
        return false;
    }
    buffer->address = address;
    buffer->size = sb.st_size;
    return true;
}

bool RenderContext::unmapFile(MapBuffer *buffer) const
{
    if (uint8_t *address = buffer->address) {
        ::munmap(address, buffer->size);
        return true;
    }
    return false;
}

bool RenderContext::existsFile(const UnicodeString &path) const
{
    NSString *newPath = toNSString(path);
    BOOL exists = [[NSFileManager defaultManager] fileExistsAtPath:newPath];
    [newPath release];
    return exists;
}

bool RenderContext::uploadTextureInternal(const UnicodeString &path, Texture &texture, void *context)
{
    ModelContext *modelContext = static_cast<ModelContext *>(context);
    if (modelContext && modelContext->findTextureCache(path, texture)) {
        return true;
    }
    return modelContext->uploadTextureFile(path, texture);
}

NSString *RenderContext::toNSString(const UnicodeString &value)
{
    return [[NSString alloc] initWithCharacters:value.getBuffer() length:value.length()];
}

const CGFloat BundleContext::kScaleFactor = 4;

void BundleContext::initialize()
{
    static bool g_initialized = false;
    if (!g_initialized) {
        google::InstallFailureSignalHandler();
        google::InitGoogleLogging("ql4pmx.qlgenerator");
        google::LogToStderr();
        FLAGS_v = 2;
        g_initialized = true;
    }
}

BundleContext::BundleContext(CFBundleRef bundle, int w, int h, CGFloat scaleFactor)
    : m_mesaContext(0),
      m_icuCommonData(nil),
      m_world(new World()),
      m_scene(new Scene(true)),
      m_scaleFactor(scaleFactor),
      m_renderWidth(w * scaleFactor),
      m_renderHeight(h * scaleFactor),
      m_imageWidth(w),
      m_imageHeight(h)
{
    m_mesaContext = OSMesaCreateContextExt(GL_RGBA, 24, 0, 0, 0);
    m_renderBuffer.reserve(m_renderWidth * m_renderHeight * 4);
    if (m_mesaContext && OSMesaMakeCurrent(m_mesaContext, &m_renderBuffer[0], GL_UNSIGNED_BYTE, m_renderWidth, m_renderHeight) && Scene::initialize(0)) {
        NSString *resourcePath = bundleResourcePath(bundle);
        NSString *dataDirectoryPath = [resourcePath stringByAppendingPathComponent:@"data"];
        NSString *toonDirectoryPath = [resourcePath stringByAppendingPathComponent:@"images"];
        NSString *kernelsDirectoryPath = [resourcePath stringByAppendingPathComponent:@"kernels"];
        NSString *shadersDirectoryPath = [resourcePath stringByAppendingPathComponent:@"shaders"];
        m_icuCommonData = [[NSData alloc] initWithContentsOfFile:[dataDirectoryPath stringByAppendingPathComponent:@"icudt50l.dat"]];
        m_settings.insert(std::make_pair(UnicodeString::fromUTF8("dir.system.toon"),
                                         UnicodeString::fromUTF8([toonDirectoryPath UTF8String])));
        m_settings.insert(std::make_pair(UnicodeString::fromUTF8("dir.system.kernels"),
                                         UnicodeString::fromUTF8([kernelsDirectoryPath UTF8String])));
        m_settings.insert(std::make_pair(UnicodeString::fromUTF8("dir.system.shaders"),
                                         UnicodeString::fromUTF8([shadersDirectoryPath UTF8String])));
        loadDictionary([[dataDirectoryPath stringByAppendingPathComponent:@"words.dic"] UTF8String]);
        [resourcePath release];
        UErrorCode status = U_ZERO_ERROR;
        udata_setCommonData([m_icuCommonData bytes], &status);
        m_encoding.reset(new Encoding(&m_dictionary));
        m_factory.reset(new Factory(m_encoding.get()));
        m_renderContext.reset(new RenderContext(m_scene.get(), m_encoding.get(), &m_settings));
        m_renderContext->initialize(false);
    }
    else {
        release();
    }
}

BundleContext::~BundleContext()
{
    release();
}

bool BundleContext::load(const UnicodeString &modelPath)
{
    int indexOf = modelPath.lastIndexOf("/");
    String dir(modelPath.tempSubString(0, indexOf));
    RenderContext::MapBuffer modelBuffer(m_renderContext.get());
    bool ok = false;
    if (m_renderContext->mapFile(modelPath, &modelBuffer)) {
        IModelSmartPtr model(m_factory->createModel(modelBuffer.address, modelBuffer.size, ok));
        IRenderEngineSmartPtr engine(m_scene->createRenderEngine(m_renderContext.get(), model.get(), 0));
        IEffect *effectRef = 0;
        m_renderContext->addModelPath(model.get(), modelPath);
        if (engine->upload(&dir)) {
            m_renderContext->parseOffscreenSemantic(effectRef, &dir);
            model->setEdgeWidth(1.0f);
            m_scene->addModel(model.release(), engine.release(), 0);
        }
        else {
            ok = false;
        }
    }
    return ok;
}

void BundleContext::render()
{
    m_scene->seek(0, Scene::kUpdateAll);
    m_scene->update(Scene::kUpdateAll);
    m_renderContext->updateCameraMatrices(glm::vec2(m_renderWidth, m_renderHeight));
    draw();
}

CGContextRef BundleContext::createBitmapContext()
{
    size_t bitsPerComponent = 8;
    CGColorSpaceRef colorSpace = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
    CGContextRef source = CGBitmapContextCreate(&m_renderBuffer[0],
            m_renderWidth,
            m_renderHeight,
            bitsPerComponent,
            m_renderWidth * 4,
            colorSpace,
            kCGImageAlphaPremultipliedLast);
    size_t stride = m_imageWidth * 4;
    m_tempRenderBuffer.resize(stride * m_imageHeight);
    CGContextRef dest = CGBitmapContextCreate(&m_tempRenderBuffer[0],
            m_imageWidth,
            m_imageHeight,
            bitsPerComponent,
            stride,
            colorSpace,
            kCGImageAlphaPremultipliedLast);
    CGContextTranslateCTM(dest, 0, m_imageHeight);
    CGContextScaleCTM(dest, 1 / m_scaleFactor, -1 / m_scaleFactor);
    CGImageRef image = CGBitmapContextCreateImage(source);
    CGContextDrawImage(dest, CGRectMake(0, 0, m_renderWidth, m_renderHeight), image);
    CGImageRelease(image);
    CGContextRelease(source);
    CGColorSpaceRelease(colorSpace);
    return dest;
}

CGSize BundleContext::size() const
{
    return CGSizeMake(m_imageWidth, m_imageHeight);
}

IModel *BundleContext::currentModel() const
{
    Array<IModel *> models;
    m_scene->getModelRefs(models);
    return models.count() > 0 ? models[0] : 0;
}

IEncoding *BundleContext::encodingRef() const
{
    return m_encoding.get();
}

NSString *BundleContext::bundleResourcePath(CFBundleRef bundle)
{
    CFURLRef resourceURL = CFBundleCopyResourcesDirectoryURL(bundle);
    UInt8 bufferPath[PATH_MAX];
    CFURLGetFileSystemRepresentation(resourceURL, TRUE, bufferPath, sizeof(bufferPath));
    CFRelease(resourceURL);
    NSString *resourcePath = [[NSString alloc] initWithUTF8String:reinterpret_cast<const char *>(bufferPath)];
    return resourcePath;
}

void BundleContext::loadPose(CFBundleRef bundle, NSString *path, Pose &pose, const char *&modelPath)
{
    NSFileManager *fm = [NSFileManager defaultManager];
    NSString *basePath = BundleContext::bundleResourcePath(bundle);
    NSString *referenceModelXPath = [basePath stringByAppendingPathComponent:@"model/reference.pmx"];
    if ([fm fileExistsAtPath:referenceModelXPath]) {
        modelPath = [referenceModelXPath cStringUsingEncoding:NSUTF8StringEncoding];
    }
    else {
        NSString *referenceModelDPath = [basePath stringByAppendingPathComponent:@"model/reference.pmd"];
        if ([fm fileExistsAtPath:referenceModelDPath]) {
            modelPath = [referenceModelDPath cStringUsingEncoding:NSUTF8StringEncoding];
        }
    }
    if (modelPath) {
        NSError *error = nil;
        NSString *data = [NSString stringWithContentsOfFile:path
                encoding:NSShiftJISStringEncoding
                error:&error];
        if (data != nil) {
            const char *s = [data UTF8String];
            std::string str(s);
            std::istringstream stream(str);
            pose.load(stream);
        }
    }
}

void BundleContext::loadDictionary(const char *path)
{
    typedef std::map<std::string, IEncoding::ConstantType> String2ConstantType;
    String2ConstantType str2const;
    str2const.insert(std::make_pair("constants.arm", IEncoding::kArm));
    str2const.insert(std::make_pair("constants.asterisk", IEncoding::kAsterisk));
    str2const.insert(std::make_pair("constants.center", IEncoding::kCenter));
    str2const.insert(std::make_pair("constants.elbow", IEncoding::kElbow));
    str2const.insert(std::make_pair("constants.finger", IEncoding::kFinger));
    str2const.insert(std::make_pair("constants.left", IEncoding::kLeft));
    str2const.insert(std::make_pair("constants.leftknee", IEncoding::kLeftKnee));
    str2const.insert(std::make_pair("constants.opacity", IEncoding::kOpacityMorphAsset));
    str2const.insert(std::make_pair("constants.right", IEncoding::kRight));
    str2const.insert(std::make_pair("constants.rightknee", IEncoding::kRightKnee));
    str2const.insert(std::make_pair("constants.root", IEncoding::kRootBone));
    str2const.insert(std::make_pair("constants.scale", IEncoding::kScaleBoneAsset));
    str2const.insert(std::make_pair("constants.spaextension", IEncoding::kSPAExtension));
    str2const.insert(std::make_pair("constants.sphextension", IEncoding::kSPHExtension));
    str2const.insert(std::make_pair("constants.wrist", IEncoding::kWrist));
    std::fstream stream(path, std::ios::in);
    std::string line, key, value;
    while (std::getline(stream, line)) {
        std::istringstream lstream(line);
        std::getline(lstream, key, '=');
        std::getline(lstream, value);
        String2ConstantType::const_iterator it = str2const.find(Pose::trim(key).c_str());
        if (it != str2const.end()) {
            m_dictionary.insert(it->second, new String(UnicodeString::fromUTF8(Pose::trim(value))));
        }
    }
}

void BundleContext::draw()
{
    glViewport(0, 0, m_renderWidth, m_renderHeight);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    Array<IRenderEngine *> enginesForPreProcess, enginesForStandard, enginesForPostProcess;
    Hash<HashPtr, IEffect *> nextPostEffects;
    m_scene->getRenderEnginesByRenderOrder(enginesForPreProcess,
                                           enginesForStandard,
                                           enginesForPostProcess,
                                           nextPostEffects);
    for (int i = 0, nengines = enginesForStandard.count(); i < nengines; i++) {
        IRenderEngine *engine = enginesForStandard[i];
        engine->renderModel();
        engine->renderEdge();
    }
}

void BundleContext::release()
{
    if (m_mesaContext) {
        OSMesaDestroyContext(m_mesaContext);
        m_mesaContext = 0;
    }
    m_dictionary.releaseAll();
    [m_icuCommonData release];
    m_icuCommonData  = nil;
}

} /* namespace ql4pmx */
} /* namespace osx */
} /* namespace extensions */
} /* namespace vpvl2 */
