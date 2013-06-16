/**

 Copyright (c) 2010-2013  hkrn

 All rights reserved.

 Redistribution and use in source and binary forms, with or
 without modification, are permitted provided that the following
 conditions are met:

 - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above
   copyright notice, this list of conditions and the following
   disclaimer in the documentation and/or other materials provided
   with the distribution.
 - Neither the name of the MMDAI project team nor the names of
   its contributors may be used to endorse or promote products
   derived from this software without specific prior written
   permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.

*/

#include "vpvl2/qt/CustomGLContext.h"

#include <QtCore/QtCore>

#import <Cocoa/Cocoa.h>
#import <OpenGL/CGLRenderers.h>

namespace vpvl2
{
namespace qt
{

void *CustomGLContext::chooseMacVisual(GDHandle handle)
{
    QVarLengthArray<NSOpenGLPixelFormatAttribute> attrs;
    attrs.append(NSOpenGLPFAStencilSize);
    attrs.append(8);
    attrs.append(NSOpenGLPFAAlphaSize);
    attrs.append(8);
    attrs.append(NSOpenGLPFAColorSize);
    attrs.append(32);
    attrs.append(NSOpenGLPFADepthSize);
    attrs.append(32);
    if (format().sampleBuffers()) {
        attrs.append(NSOpenGLPFAMultisample);
        attrs.append(NSOpenGLPFASampleBuffers);
        attrs.append(1);
        attrs.append(NSOpenGLPFASamples);
        attrs.append(4);
    }
    if (format().doubleBuffer()) {
        attrs.append(NSOpenGLPFADoubleBuffer);
    }
    if (format().directRendering()) {
        attrs.append(NSOpenGLPFAAccelerated);
    }
    else {
        attrs.append(NSOpenGLPFARendererID);
        attrs.append(kCGLRendererGenericFloatID);
    }
    attrs.append(NSOpenGLPFAOpenGLProfile);
    if (format().profile() == QGLFormat::CoreProfile) {
        attrs.append(NSOpenGLProfileVersion3_2Core);
    }
    else {
        attrs.append(NSOpenGLProfileVersionLegacy);
    }
    attrs.append(0);
    NSOpenGLPixelFormat *fmt = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs.constData()];
    return fmt != nil ? fmt : QGLContext::chooseMacVisual(handle);
}

} /* namespace qt */
} /* namespace vpvl2 */

