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

#pragma once
#ifndef VPVL2_EXTENSIONS_OSX_QL4PMX_RENDERCONTEXT_H_
#define VPVL2_EXTENSIONS_OSX_QL4PMX_RENDERCONTEXT_H_

#include <vpvl2/extensions/BaseRenderContext.h>
#include <vpvl2/extensions/icu4c/Encoding.h>
#include <vpvl2/extensions/icu4c/StringMap.h>

#include <GL/osmesa.h>
#include <Foundation/Foundation.h>

namespace vpvl2
{

class IModel;

namespace extensions
{

class Pose;

namespace osx
{
namespace ql4pmx
{

class RenderContext : public vpvl2::extensions::BaseRenderContext {
public:
    RenderContext(vpvl2::Scene *sceneRef, IEncoding *encodingRef, icu4c::StringMap *configRef);
    ~RenderContext();

    void *findProcedureAddress(const void **candidatesPtr) const;
    bool mapFile(const UnicodeString &path, MapBuffer *buffer) const;
    bool unmapFile(MapBuffer *buffer) const;
    bool existsFile(const UnicodeString &path) const;

private:
    bool uploadTextureInternal(const UnicodeString &path, Texture &texture, void *context);
    static NSString *toNSString(const UnicodeString &value);
};

VPVL2_MAKE_SMARTPTR(RenderContext);

class BundleContext {
public:
    static const CGFloat kScaleFactor;
    static void initialize();
    static void loadDictionary(icu4c::Encoding::Dictionary &dictionary);

    BundleContext(CFBundleRef bundle, int w, int h, CGFloat scaleFactor = kScaleFactor);
    ~BundleContext();

    bool load(const UnicodeString &modelPath);
    void render();
    CGContextRef createBitmapContext();
    CGSize size() const;
    IModel *currentModel() const;
    IEncoding *encodingRef() const;

    static NSString *bundleResourcePath(CFBundleRef bundle);
    static void loadPose(CFBundleRef bundle, NSString *path, Pose &pose, const char *&modelPath);

private:
    void loadDictionary(const char *path);
    void draw();
    void release();

    OSMesaContext m_mesaContext;
    NSData *m_icuCommonData;
    vpvl2::extensions::icu4c::StringMap m_settings;
    vpvl2::extensions::icu4c::Encoding::Dictionary m_dictionary;
    vpvl2::extensions::WorldSmartPtr m_world;
    vpvl2::extensions::icu4c::EncodingSmartPtr m_encoding;
    vpvl2::extensions::FactorySmartPtr m_factory;
    vpvl2::extensions::SceneSmartPtr m_scene;
    RenderContextSmartPtr m_renderContext;
    Array<uint8_t> m_renderBuffer;
    Array<uint8_t> m_tempRenderBuffer;
    CGFloat m_scaleFactor;
    int m_renderWidth;
    int m_renderHeight;
    int m_imageWidth;
    int m_imageHeight;
};

} /* namespace ql4pmx */
} /* namespace osx */
} /* namespace extensions */
} /* namespace vpvl2 */

#endif
