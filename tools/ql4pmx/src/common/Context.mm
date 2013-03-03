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
#include <vpvl2/extensions/World.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

using namespace vpvl2;
using namespace vpvl2::extensions;

namespace vpvl2 {
namespace extensions {
namespace osx {
namespace ql4pmx {

RenderContext::RenderContext(Scene *sceneRef, StringMap *configRef)
    : BaseRenderContext(sceneRef, configRef)
{
}

RenderContext::~RenderContext()
{
}

void *RenderContext::findProcedureAddress(const void **candidatesPtr) const {
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
    ModelContext *modelcontext = static_cast<ModelContext *>(context);
    if (modelcontext && modelcontext->findTextureCache(path, texture)) {
        return true;
    }
    NSString *newPath = toNSString(path);
    BOOL isDirectory = NO;
    BOOL fileExists = [[NSFileManager defaultManager] fileExistsAtPath:newPath isDirectory:&isDirectory];
    if (!fileExists || (fileExists && isDirectory)) {
        return true;
    }
    NSImage *image = [[NSImage alloc] initWithContentsOfFile:newPath];
    NSSize size = [image size];
    size_t width = size.width, height = size.height, stride = width * 4;
    if (CGImage *imageRef = [image CGImageForProposedRect:nil context:nil hints:nil]) {
        uint8_t *rawData = new uint8_t[stride * size_t(size.height) * sizeof(uint8_t)];
        CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
        CGContextRef bitmapContext = CGBitmapContextCreate(rawData,
                                                           width,
                                                           height,
                                                           CGImageGetBitsPerComponent(imageRef),
                                                           stride,
                                                           colorSpace,
                                                           kCGImageAlphaPremultipliedLast);
        CGContextDrawImage(bitmapContext, CGRectMake(0, 0, width, height), imageRef);
        GLuint textureID = createTexture(rawData, glm::ivec3(width, height, 0), GL_RGBA,
                                         GL_UNSIGNED_INT_8_8_8_8_REV, texture.mipmap, texture.toon, false);
        texture.size.setValue(width, height, 0);
        texture.opaque = textureID;
        if (modelcontext) {
            TextureCache cache(texture);
            modelcontext->addTextureCache(path, cache);
        }
        bool ok = texture.ok = textureID != 0;
        CGContextRelease(bitmapContext);
        CGColorSpaceRelease(colorSpace);
        [image release];
        [newPath release];
        return ok;
    }
    return false;
}

NSString *RenderContext::toNSString(const UnicodeString &value)
{
    return [[NSString alloc] initWithCharacters:value.getBuffer() length:value.length()];
}

BundleContext::BundleContext(int w, int h)
    : m_mesaContext(0),
      m_world(new World()),
      m_encoding(new Encoding(&m_dictionary)),
      m_factory(new Factory(m_encoding.get())),
      m_scene(new Scene(true)),
      m_width(w),
      m_height(h),
      m_renderBuffer(0),
      m_tempRenderBuffer(0)
{
    m_mesaContext = OSMesaCreateContextExt(GL_RGBA, 24, 8, 0, 0);
    m_renderBuffer = new uint8_t[w * h * 4];
    if (m_mesaContext && OSMesaMakeCurrent(m_mesaContext, m_renderBuffer, GL_UNSIGNED_BYTE, m_width, m_height) && Scene::initialize(0)) {
        NSBundle *mainBundle = [NSBundle mainBundle];
        NSString *resourceURL = [mainBundle resourcePath];
        NSString *toonDirectoryPath = [resourceURL stringByAppendingPathComponent:@"toon"];
        NSString *kernelsDirectoryPath = [resourceURL stringByAppendingPathComponent:@"kernels"];
        NSString *shadersDirectoryPath = [resourceURL stringByAppendingPathComponent:@"shaders"];
        m_settings.insert(std::make_pair(UnicodeString::fromUTF8("dir.system.toon"),
                                         UnicodeString::fromUTF8([toonDirectoryPath UTF8String])));
        m_settings.insert(std::make_pair(UnicodeString::fromUTF8("dir.system.kernels"),
                                         UnicodeString::fromUTF8([kernelsDirectoryPath UTF8String])));
        m_settings.insert(std::make_pair(UnicodeString::fromUTF8("dir.system.shaders"),
                                         UnicodeString::fromUTF8([shadersDirectoryPath UTF8String])));
        m_renderContext.reset(new RenderContext(m_scene.get(), &m_settings));
    }
    else {
        release();
    }
}

BundleContext::~BundleContext()
{
    release();
}

void BundleContext::render(const UnicodeString &modelPath)
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
    }
    m_scene->seek(0, Scene::kUpdateAll);
    m_scene->update(Scene::kUpdateAll | Scene::kResetMotionState);
    m_renderContext->updateCameraMatrices(glm::vec2(m_width, m_height));
    draw();
}

CGContextRef BundleContext::createBitmapContext()
{
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef source = CGBitmapContextCreate(m_renderBuffer,
                                                m_width,
                                                m_height,
                                                8,
                                                4 * m_width,
                                                colorSpace,
                                                kCGImageAlphaPremultipliedLast);
    delete[] m_tempRenderBuffer;
    m_tempRenderBuffer = new uint8_t[m_width * m_height * 4];
    CGContextRef dest = CGBitmapContextCreate(m_tempRenderBuffer,
                                              m_width,
                                              m_height,
                                              8,
                                              4 * m_width,
                                              colorSpace,
                                              kCGImageAlphaPremultipliedLast);
    CGContextTranslateCTM(dest, 0, m_height);
    CGContextScaleCTM(dest, 1, -1);
    CGImageRef image = CGBitmapContextCreateImage(source);
    CGContextDrawImage(dest, CGRectMake(0, 0, m_width, m_height), image);
    CGImageRelease(image);
    CGContextRelease(source);
    CGColorSpaceRelease(colorSpace);
    return dest;
}

CGSize BundleContext::size() const
{
    return CGSizeMake(m_width, m_height);
}

void BundleContext::draw()
{
    glViewport(0, 0, m_width, m_height);
    glClearColor(0, 0, 0, 0);
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
    delete[] m_renderBuffer;
    m_renderBuffer = 0;
    delete[] m_tempRenderBuffer;
    m_tempRenderBuffer = 0;
    if (m_mesaContext) {
        OSMesaDestroyContext(m_mesaContext);
        m_mesaContext = 0;
    }
}

} /* namespace ql4pmx */
} /* namespace osx */
} /* namespace extensions */
} /* namespace vpvl2 */
