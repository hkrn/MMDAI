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

#include <Cocoa/Cocoa.h>
#include <QuickLook/QuickLook.h>

extern "C" {

using namespace vpvl2;
using namespace vpvl2::extensions;
using namespace vpvl2::extensions::icu4c;
using namespace vpvl2::extensions::osx::ql4pmx;

OSStatus GeneratePreviewForURL(void *thisInterface,
                               QLPreviewRequestRef preview,
                               CFURLRef url,
                               CFStringRef contentTypeUTI,
                               CFDictionaryRef options);
void CancelPreviewGeneration(void *thisInterface, QLPreviewRequestRef preview);


OSStatus GeneratePreviewForURL(void * /* thisInterface */,
                               QLPreviewRequestRef preview,
                               CFURLRef url,
                               CFStringRef contentTypeUTI,
                               CFDictionaryRef /* options */)
{
    OSStatus status = noErr;
    @autoreleasepool {
        if (QLPreviewRequestIsCancelled(preview)) {
            return status;
        }
        NSDictionary *options = nil;
        NSString *stringPath = (NSString *) CFURLCopyFileSystemPath(url, kCFURLPOSIXPathStyle);
        CGContextRef bitmapContext = 0;
        CGImageRef image = 0;
        try {
            CFBundleRef bundle = QLPreviewRequestGetGeneratorBundle(preview);
            NSDictionary *info = (NSDictionary *) CFBundleGetInfoDictionary(bundle);
            Encoding encoding(0);
            Pose pose(&encoding);
            int width = [[info objectForKey:@"QLPreviewWidth"] intValue];
            int height = [[info objectForKey:@"QLPreviewHeight"] intValue];
            BundleContext context(bundle, width, height, 2);
            NSString *uti = (NSString *) contentTypeUTI;
            const char *modelPath = 0;
            if ([uti hasPrefix:@"com.github.hkrn.mmdai.uti.pm"]) {
                modelPath = [stringPath cStringUsingEncoding:NSUTF8StringEncoding];
            }
            else if ([uti isEqualToString:@"com.github.hkrn.mmdai.uti.vpd"]) {
                BundleContext::loadPose(bundle, stringPath, pose, modelPath);
            }
            if (modelPath && context.load(UnicodeString::fromUTF8(modelPath))) {
                IModel *model = context.currentModel();
                pose.bind(model);
                context.render();
                bitmapContext = context.createBitmapContext();
                image = CGBitmapContextCreateImage(bitmapContext);
                NSString *name = nil, *displayName = nil;
                NSURL *urlRef = (NSURL *) url;
                if (const IString *n = model->name()) {
                    name = [[NSString alloc] initWithUTF8String:reinterpret_cast<const char *>(n->toByteArray())];
                    displayName = [[NSString alloc] initWithFormat:@"%@ - %@", [name retain],
                                                                                [[urlRef lastPathComponent] retain]];
                }
                else {
                    displayName = [[NSString alloc] initWithString:[[urlRef lastPathComponent] retain]];
                }
                options = [[NSDictionary alloc] initWithObjectsAndKeys:[displayName retain],
                                                                        (NSString *)kQLPreviewPropertyDisplayNameKey, nil];
                [displayName release];
                [name release];
                CGContextRef previewContext = QLPreviewRequestCreateContext(preview,
                                                                            CGSizeMake(width, height),
                                                                            true,
                                                                            (CFDictionaryRef) options);
                CGContextDrawImage(previewContext, CGRectMake(0, 0, width, height), image);
                QLPreviewRequestFlushContext(preview, previewContext);
                CGContextRelease(previewContext);
            }
        } catch (std::exception e) {
            NSLog(@"%s", e.what());
        }
        CGImageRelease(image);
        CGContextRelease(bitmapContext);
        CFRelease(stringPath);
        [options release];
    }
    return status;
}

void CancelPreviewGeneration(void * /* thisInterface */, QLPreviewRequestRef /* preview */)
{
}

}
