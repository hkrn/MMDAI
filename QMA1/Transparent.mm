#import <Cocoa/Cocoa.h>

void UISetGLContextTransparent(bool value)
{
    GLint opacity = value ? 0 : 1;
    NSOpenGLContext *context = [NSOpenGLContext currentContext];
    [context setValues:&opacity forParameter:NSOpenGLCPSurfaceOpacity];
}
