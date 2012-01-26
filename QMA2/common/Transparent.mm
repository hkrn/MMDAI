#import <Cocoa/Cocoa.h>

void UISetGLContextTransparent()
{
    GLint opacity = 0;
    NSOpenGLContext *context = [NSOpenGLContext currentContext];
    [context setValues:&opacity forParameter:NSOpenGLCPSurfaceOpacity];
}
