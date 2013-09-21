#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "gl_vpvl2.h"

#if defined(__APPLE__)
#include <mach-o/dyld.h>

static void* AppleGLGetProcAddress (const GLubyte *name)
{
  static const struct mach_header* image = NULL;
  NSSymbol symbol;
  char* symbolName;
  if (NULL == image)
  {
    image = NSAddImage("/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL", NSADDIMAGE_OPTION_RETURN_ON_ERROR);
  }
  /* prepend a '_' for the Unix C symbol mangling convention */
  symbolName = malloc(strlen((const char*)name) + 2);
  strcpy(symbolName+1, (const char*)name);
  symbolName[0] = '_';
  symbol = NULL;
  /* if (NSIsSymbolNameDefined(symbolName))
	 symbol = NSLookupAndBindSymbol(symbolName); */
  symbol = image ? NSLookupSymbolInImage(image, symbolName, NSLOOKUPSYMBOLINIMAGE_OPTION_BIND | NSLOOKUPSYMBOLINIMAGE_OPTION_RETURN_ON_ERROR) : NULL;
  free(symbolName);
  return symbol ? NSAddressOfSymbol(symbol) : NULL;
}
#endif /* __APPLE__ */

#if defined(__sgi) || defined (__sun)
#include <dlfcn.h>
#include <stdio.h>

static void* SunGetProcAddress (const GLubyte* name)
{
  static void* h = NULL;
  static void* gpa;

  if (h == NULL)
  {
    if ((h = dlopen(NULL, RTLD_LAZY | RTLD_LOCAL)) == NULL) return NULL;
    gpa = dlsym(h, "glXGetProcAddress");
  }

  if (gpa != NULL)
    return ((void*(*)(const GLubyte*))gpa)(name);
  else
    return dlsym(h, (const char*)name);
}
#endif /* __sgi || __sun */

#if defined(_WIN32)

#ifdef _MSC_VER
#pragma warning(disable: 4055)
#pragma warning(disable: 4054)
#endif

static int TestPointer(const PROC pTest)
{
	ptrdiff_t iTest;
	if(!pTest) return 0;
	iTest = (ptrdiff_t)pTest;
	
	if(iTest == 1 || iTest == 2 || iTest == 3 || iTest == -1) return 0;
	
	return 1;
}

static PROC WinGetProcAddress(const char *name)
{
	HMODULE glMod = NULL;
	PROC pFunc = wglGetProcAddress((LPCSTR)name);
	if(TestPointer(pFunc))
	{
		return pFunc;
	}
	glMod = GetModuleHandleA("OpenGL32.dll");
	return (PROC)GetProcAddress(glMod, (LPCSTR)name);
}
	
#define IntGetProcAddress(name) WinGetProcAddress(name)
#else
	#if defined(__APPLE__)
		#define IntGetProcAddress(name) AppleGLGetProcAddress(name)
	#else
		#if defined(__sgi) || defined(__sun)
			#define IntGetProcAddress(name) SunGetProcAddress(name)
		#else /* GLX */
		    #include <GL/glx.h>

			#define IntGetProcAddress(name) (*glXGetProcAddressARB)((const GLubyte*)name)
		#endif
	#endif
#endif

int vpvl2_ogl_ext_ARB_debug_output = vpvl2_ogl_LOAD_FAILED;
int vpvl2_ogl_ext_ARB_depth_buffer_float = vpvl2_ogl_LOAD_FAILED;
int vpvl2_ogl_ext_ARB_draw_elements_base_vertex = vpvl2_ogl_LOAD_FAILED;
int vpvl2_ogl_ext_ARB_framebuffer_object = vpvl2_ogl_LOAD_FAILED;
int vpvl2_ogl_ext_ARB_half_float_pixel = vpvl2_ogl_LOAD_FAILED;
int vpvl2_ogl_ext_ARB_map_buffer_range = vpvl2_ogl_LOAD_FAILED;
int vpvl2_ogl_ext_ARB_occlusion_query = vpvl2_ogl_LOAD_FAILED;
int vpvl2_ogl_ext_ARB_occlusion_query2 = vpvl2_ogl_LOAD_FAILED;
int vpvl2_ogl_ext_ARB_sampler_objects = vpvl2_ogl_LOAD_FAILED;
int vpvl2_ogl_ext_ARB_texture_float = vpvl2_ogl_LOAD_FAILED;
int vpvl2_ogl_ext_ARB_texture_rg = vpvl2_ogl_LOAD_FAILED;
int vpvl2_ogl_ext_ARB_texture_storage = vpvl2_ogl_LOAD_FAILED;
int vpvl2_ogl_ext_ARB_vertex_array_object = vpvl2_ogl_LOAD_FAILED;
int vpvl2_ogl_ext_APPLE_vertex_array_object = vpvl2_ogl_LOAD_FAILED;
int vpvl2_ogl_ext_EXT_framebuffer_blit = vpvl2_ogl_LOAD_FAILED;
int vpvl2_ogl_ext_EXT_framebuffer_multisample = vpvl2_ogl_LOAD_FAILED;
int vpvl2_ogl_ext_EXT_framebuffer_object = vpvl2_ogl_LOAD_FAILED;

void (CODEGEN_FUNCPTR *vpvl2__ptrc_glDebugMessageCallbackARB)(GLDEBUGPROCARB, const void *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glDebugMessageControlARB)(GLenum, GLenum, GLenum, GLsizei, const GLuint *, GLboolean) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glDebugMessageInsertARB)(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar *) = NULL;
GLuint (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetDebugMessageLogARB)(GLuint, GLsizei, GLenum *, GLenum *, GLuint *, GLenum *, GLsizei *, GLchar *) = NULL;

static int Load_ARB_debug_output()
{
  int numFailed = 0;
  vpvl2__ptrc_glDebugMessageCallbackARB = (void (CODEGEN_FUNCPTR *)(GLDEBUGPROCARB, const void *))IntGetProcAddress("glDebugMessageCallbackARB");
  if(!vpvl2__ptrc_glDebugMessageCallbackARB) numFailed++;
  vpvl2__ptrc_glDebugMessageControlARB = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum, GLsizei, const GLuint *, GLboolean))IntGetProcAddress("glDebugMessageControlARB");
  if(!vpvl2__ptrc_glDebugMessageControlARB) numFailed++;
  vpvl2__ptrc_glDebugMessageInsertARB = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLuint, GLenum, GLsizei, const GLchar *))IntGetProcAddress("glDebugMessageInsertARB");
  if(!vpvl2__ptrc_glDebugMessageInsertARB) numFailed++;
  vpvl2__ptrc_glGetDebugMessageLogARB = (GLuint (CODEGEN_FUNCPTR *)(GLuint, GLsizei, GLenum *, GLenum *, GLuint *, GLenum *, GLsizei *, GLchar *))IntGetProcAddress("glGetDebugMessageLogARB");
  if(!vpvl2__ptrc_glGetDebugMessageLogARB) numFailed++;
  return numFailed;
}

void (CODEGEN_FUNCPTR *vpvl2__ptrc_glDrawElementsBaseVertex)(GLenum, GLsizei, GLenum, const GLvoid *, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glDrawElementsInstancedBaseVertex)(GLenum, GLsizei, GLenum, const GLvoid *, GLsizei, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glDrawRangeElementsBaseVertex)(GLenum, GLuint, GLuint, GLsizei, GLenum, const GLvoid *, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultiDrawElementsBaseVertex)(GLenum, const GLsizei *, GLenum, const GLvoid *const*, GLsizei, const GLint *) = NULL;

static int Load_ARB_draw_elements_base_vertex()
{
  int numFailed = 0;
  vpvl2__ptrc_glDrawElementsBaseVertex = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, GLenum, const GLvoid *, GLint))IntGetProcAddress("glDrawElementsBaseVertex");
  if(!vpvl2__ptrc_glDrawElementsBaseVertex) numFailed++;
  vpvl2__ptrc_glDrawElementsInstancedBaseVertex = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, GLenum, const GLvoid *, GLsizei, GLint))IntGetProcAddress("glDrawElementsInstancedBaseVertex");
  if(!vpvl2__ptrc_glDrawElementsInstancedBaseVertex) numFailed++;
  vpvl2__ptrc_glDrawRangeElementsBaseVertex = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint, GLuint, GLsizei, GLenum, const GLvoid *, GLint))IntGetProcAddress("glDrawRangeElementsBaseVertex");
  if(!vpvl2__ptrc_glDrawRangeElementsBaseVertex) numFailed++;
  vpvl2__ptrc_glMultiDrawElementsBaseVertex = (void (CODEGEN_FUNCPTR *)(GLenum, const GLsizei *, GLenum, const GLvoid *const*, GLsizei, const GLint *))IntGetProcAddress("glMultiDrawElementsBaseVertex");
  if(!vpvl2__ptrc_glMultiDrawElementsBaseVertex) numFailed++;
  return numFailed;
}

void (CODEGEN_FUNCPTR *vpvl2__ptrc_glBindFramebuffer)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glBindRenderbuffer)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glBlitFramebuffer)(GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum) = NULL;
GLenum (CODEGEN_FUNCPTR *vpvl2__ptrc_glCheckFramebufferStatus)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glDeleteFramebuffers)(GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glDeleteRenderbuffers)(GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glFramebufferRenderbuffer)(GLenum, GLenum, GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glFramebufferTexture1D)(GLenum, GLenum, GLenum, GLuint, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glFramebufferTexture2D)(GLenum, GLenum, GLenum, GLuint, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glFramebufferTexture3D)(GLenum, GLenum, GLenum, GLuint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glFramebufferTextureLayer)(GLenum, GLenum, GLuint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGenFramebuffers)(GLsizei, GLuint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGenRenderbuffers)(GLsizei, GLuint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGenerateMipmap)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetFramebufferAttachmentParameteriv)(GLenum, GLenum, GLenum, GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetRenderbufferParameteriv)(GLenum, GLenum, GLint *) = NULL;
GLboolean (CODEGEN_FUNCPTR *vpvl2__ptrc_glIsFramebuffer)(GLuint) = NULL;
GLboolean (CODEGEN_FUNCPTR *vpvl2__ptrc_glIsRenderbuffer)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRenderbufferStorage)(GLenum, GLenum, GLsizei, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRenderbufferStorageMultisample)(GLenum, GLsizei, GLenum, GLsizei, GLsizei) = NULL;

static int Load_ARB_framebuffer_object()
{
  int numFailed = 0;
  vpvl2__ptrc_glBindFramebuffer = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glBindFramebuffer");
  if(!vpvl2__ptrc_glBindFramebuffer) numFailed++;
  vpvl2__ptrc_glBindRenderbuffer = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glBindRenderbuffer");
  if(!vpvl2__ptrc_glBindRenderbuffer) numFailed++;
  vpvl2__ptrc_glBlitFramebuffer = (void (CODEGEN_FUNCPTR *)(GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum))IntGetProcAddress("glBlitFramebuffer");
  if(!vpvl2__ptrc_glBlitFramebuffer) numFailed++;
  vpvl2__ptrc_glCheckFramebufferStatus = (GLenum (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glCheckFramebufferStatus");
  if(!vpvl2__ptrc_glCheckFramebufferStatus) numFailed++;
  vpvl2__ptrc_glDeleteFramebuffers = (void (CODEGEN_FUNCPTR *)(GLsizei, const GLuint *))IntGetProcAddress("glDeleteFramebuffers");
  if(!vpvl2__ptrc_glDeleteFramebuffers) numFailed++;
  vpvl2__ptrc_glDeleteRenderbuffers = (void (CODEGEN_FUNCPTR *)(GLsizei, const GLuint *))IntGetProcAddress("glDeleteRenderbuffers");
  if(!vpvl2__ptrc_glDeleteRenderbuffers) numFailed++;
  vpvl2__ptrc_glFramebufferRenderbuffer = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum, GLuint))IntGetProcAddress("glFramebufferRenderbuffer");
  if(!vpvl2__ptrc_glFramebufferRenderbuffer) numFailed++;
  vpvl2__ptrc_glFramebufferTexture1D = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum, GLuint, GLint))IntGetProcAddress("glFramebufferTexture1D");
  if(!vpvl2__ptrc_glFramebufferTexture1D) numFailed++;
  vpvl2__ptrc_glFramebufferTexture2D = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum, GLuint, GLint))IntGetProcAddress("glFramebufferTexture2D");
  if(!vpvl2__ptrc_glFramebufferTexture2D) numFailed++;
  vpvl2__ptrc_glFramebufferTexture3D = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum, GLuint, GLint, GLint))IntGetProcAddress("glFramebufferTexture3D");
  if(!vpvl2__ptrc_glFramebufferTexture3D) numFailed++;
  vpvl2__ptrc_glFramebufferTextureLayer = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLuint, GLint, GLint))IntGetProcAddress("glFramebufferTextureLayer");
  if(!vpvl2__ptrc_glFramebufferTextureLayer) numFailed++;
  vpvl2__ptrc_glGenFramebuffers = (void (CODEGEN_FUNCPTR *)(GLsizei, GLuint *))IntGetProcAddress("glGenFramebuffers");
  if(!vpvl2__ptrc_glGenFramebuffers) numFailed++;
  vpvl2__ptrc_glGenRenderbuffers = (void (CODEGEN_FUNCPTR *)(GLsizei, GLuint *))IntGetProcAddress("glGenRenderbuffers");
  if(!vpvl2__ptrc_glGenRenderbuffers) numFailed++;
  vpvl2__ptrc_glGenerateMipmap = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glGenerateMipmap");
  if(!vpvl2__ptrc_glGenerateMipmap) numFailed++;
  vpvl2__ptrc_glGetFramebufferAttachmentParameteriv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum, GLint *))IntGetProcAddress("glGetFramebufferAttachmentParameteriv");
  if(!vpvl2__ptrc_glGetFramebufferAttachmentParameteriv) numFailed++;
  vpvl2__ptrc_glGetRenderbufferParameteriv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint *))IntGetProcAddress("glGetRenderbufferParameteriv");
  if(!vpvl2__ptrc_glGetRenderbufferParameteriv) numFailed++;
  vpvl2__ptrc_glIsFramebuffer = (GLboolean (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glIsFramebuffer");
  if(!vpvl2__ptrc_glIsFramebuffer) numFailed++;
  vpvl2__ptrc_glIsRenderbuffer = (GLboolean (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glIsRenderbuffer");
  if(!vpvl2__ptrc_glIsRenderbuffer) numFailed++;
  vpvl2__ptrc_glRenderbufferStorage = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLsizei, GLsizei))IntGetProcAddress("glRenderbufferStorage");
  if(!vpvl2__ptrc_glRenderbufferStorage) numFailed++;
  vpvl2__ptrc_glRenderbufferStorageMultisample = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, GLenum, GLsizei, GLsizei))IntGetProcAddress("glRenderbufferStorageMultisample");
  if(!vpvl2__ptrc_glRenderbufferStorageMultisample) numFailed++;
  return numFailed;
}

void (CODEGEN_FUNCPTR *vpvl2__ptrc_glFlushMappedBufferRange)(GLenum, GLintptr, GLsizeiptr) = NULL;
void * (CODEGEN_FUNCPTR *vpvl2__ptrc_glMapBufferRange)(GLenum, GLintptr, GLsizeiptr, GLbitfield) = NULL;

static int Load_ARB_map_buffer_range()
{
  int numFailed = 0;
  vpvl2__ptrc_glFlushMappedBufferRange = (void (CODEGEN_FUNCPTR *)(GLenum, GLintptr, GLsizeiptr))IntGetProcAddress("glFlushMappedBufferRange");
  if(!vpvl2__ptrc_glFlushMappedBufferRange) numFailed++;
  vpvl2__ptrc_glMapBufferRange = (void * (CODEGEN_FUNCPTR *)(GLenum, GLintptr, GLsizeiptr, GLbitfield))IntGetProcAddress("glMapBufferRange");
  if(!vpvl2__ptrc_glMapBufferRange) numFailed++;
  return numFailed;
}

void (CODEGEN_FUNCPTR *vpvl2__ptrc_glBeginQueryARB)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glDeleteQueriesARB)(GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glEndQueryARB)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGenQueriesARB)(GLsizei, GLuint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetQueryObjectivARB)(GLuint, GLenum, GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetQueryObjectuivARB)(GLuint, GLenum, GLuint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetQueryivARB)(GLenum, GLenum, GLint *) = NULL;
GLboolean (CODEGEN_FUNCPTR *vpvl2__ptrc_glIsQueryARB)(GLuint) = NULL;

static int Load_ARB_occlusion_query()
{
  int numFailed = 0;
  vpvl2__ptrc_glBeginQueryARB = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glBeginQueryARB");
  if(!vpvl2__ptrc_glBeginQueryARB) numFailed++;
  vpvl2__ptrc_glDeleteQueriesARB = (void (CODEGEN_FUNCPTR *)(GLsizei, const GLuint *))IntGetProcAddress("glDeleteQueriesARB");
  if(!vpvl2__ptrc_glDeleteQueriesARB) numFailed++;
  vpvl2__ptrc_glEndQueryARB = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glEndQueryARB");
  if(!vpvl2__ptrc_glEndQueryARB) numFailed++;
  vpvl2__ptrc_glGenQueriesARB = (void (CODEGEN_FUNCPTR *)(GLsizei, GLuint *))IntGetProcAddress("glGenQueriesARB");
  if(!vpvl2__ptrc_glGenQueriesARB) numFailed++;
  vpvl2__ptrc_glGetQueryObjectivARB = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLint *))IntGetProcAddress("glGetQueryObjectivARB");
  if(!vpvl2__ptrc_glGetQueryObjectivARB) numFailed++;
  vpvl2__ptrc_glGetQueryObjectuivARB = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLuint *))IntGetProcAddress("glGetQueryObjectuivARB");
  if(!vpvl2__ptrc_glGetQueryObjectuivARB) numFailed++;
  vpvl2__ptrc_glGetQueryivARB = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint *))IntGetProcAddress("glGetQueryivARB");
  if(!vpvl2__ptrc_glGetQueryivARB) numFailed++;
  vpvl2__ptrc_glIsQueryARB = (GLboolean (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glIsQueryARB");
  if(!vpvl2__ptrc_glIsQueryARB) numFailed++;
  return numFailed;
}

void (CODEGEN_FUNCPTR *vpvl2__ptrc_glBindSampler)(GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glDeleteSamplers)(GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGenSamplers)(GLsizei, GLuint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetSamplerParameterIiv)(GLuint, GLenum, GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetSamplerParameterIuiv)(GLuint, GLenum, GLuint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetSamplerParameterfv)(GLuint, GLenum, GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetSamplerParameteriv)(GLuint, GLenum, GLint *) = NULL;
GLboolean (CODEGEN_FUNCPTR *vpvl2__ptrc_glIsSampler)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glSamplerParameterIiv)(GLuint, GLenum, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glSamplerParameterIuiv)(GLuint, GLenum, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glSamplerParameterf)(GLuint, GLenum, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glSamplerParameterfv)(GLuint, GLenum, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glSamplerParameteri)(GLuint, GLenum, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glSamplerParameteriv)(GLuint, GLenum, const GLint *) = NULL;

static int Load_ARB_sampler_objects()
{
  int numFailed = 0;
  vpvl2__ptrc_glBindSampler = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint))IntGetProcAddress("glBindSampler");
  if(!vpvl2__ptrc_glBindSampler) numFailed++;
  vpvl2__ptrc_glDeleteSamplers = (void (CODEGEN_FUNCPTR *)(GLsizei, const GLuint *))IntGetProcAddress("glDeleteSamplers");
  if(!vpvl2__ptrc_glDeleteSamplers) numFailed++;
  vpvl2__ptrc_glGenSamplers = (void (CODEGEN_FUNCPTR *)(GLsizei, GLuint *))IntGetProcAddress("glGenSamplers");
  if(!vpvl2__ptrc_glGenSamplers) numFailed++;
  vpvl2__ptrc_glGetSamplerParameterIiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLint *))IntGetProcAddress("glGetSamplerParameterIiv");
  if(!vpvl2__ptrc_glGetSamplerParameterIiv) numFailed++;
  vpvl2__ptrc_glGetSamplerParameterIuiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLuint *))IntGetProcAddress("glGetSamplerParameterIuiv");
  if(!vpvl2__ptrc_glGetSamplerParameterIuiv) numFailed++;
  vpvl2__ptrc_glGetSamplerParameterfv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLfloat *))IntGetProcAddress("glGetSamplerParameterfv");
  if(!vpvl2__ptrc_glGetSamplerParameterfv) numFailed++;
  vpvl2__ptrc_glGetSamplerParameteriv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLint *))IntGetProcAddress("glGetSamplerParameteriv");
  if(!vpvl2__ptrc_glGetSamplerParameteriv) numFailed++;
  vpvl2__ptrc_glIsSampler = (GLboolean (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glIsSampler");
  if(!vpvl2__ptrc_glIsSampler) numFailed++;
  vpvl2__ptrc_glSamplerParameterIiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, const GLint *))IntGetProcAddress("glSamplerParameterIiv");
  if(!vpvl2__ptrc_glSamplerParameterIiv) numFailed++;
  vpvl2__ptrc_glSamplerParameterIuiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, const GLuint *))IntGetProcAddress("glSamplerParameterIuiv");
  if(!vpvl2__ptrc_glSamplerParameterIuiv) numFailed++;
  vpvl2__ptrc_glSamplerParameterf = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLfloat))IntGetProcAddress("glSamplerParameterf");
  if(!vpvl2__ptrc_glSamplerParameterf) numFailed++;
  vpvl2__ptrc_glSamplerParameterfv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, const GLfloat *))IntGetProcAddress("glSamplerParameterfv");
  if(!vpvl2__ptrc_glSamplerParameterfv) numFailed++;
  vpvl2__ptrc_glSamplerParameteri = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLint))IntGetProcAddress("glSamplerParameteri");
  if(!vpvl2__ptrc_glSamplerParameteri) numFailed++;
  vpvl2__ptrc_glSamplerParameteriv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, const GLint *))IntGetProcAddress("glSamplerParameteriv");
  if(!vpvl2__ptrc_glSamplerParameteriv) numFailed++;
  return numFailed;
}

void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexStorage1D)(GLenum, GLsizei, GLenum, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexStorage2D)(GLenum, GLsizei, GLenum, GLsizei, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexStorage3D)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei) = NULL;

static int Load_ARB_texture_storage()
{
  int numFailed = 0;
  vpvl2__ptrc_glTexStorage1D = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, GLenum, GLsizei))IntGetProcAddress("glTexStorage1D");
  if(!vpvl2__ptrc_glTexStorage1D) numFailed++;
  vpvl2__ptrc_glTexStorage2D = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, GLenum, GLsizei, GLsizei))IntGetProcAddress("glTexStorage2D");
  if(!vpvl2__ptrc_glTexStorage2D) numFailed++;
  vpvl2__ptrc_glTexStorage3D = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, GLenum, GLsizei, GLsizei, GLsizei))IntGetProcAddress("glTexStorage3D");
  if(!vpvl2__ptrc_glTexStorage3D) numFailed++;
  return numFailed;
}

void (CODEGEN_FUNCPTR *vpvl2__ptrc_glBindVertexArray)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glDeleteVertexArrays)(GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGenVertexArrays)(GLsizei, GLuint *) = NULL;
GLboolean (CODEGEN_FUNCPTR *vpvl2__ptrc_glIsVertexArray)(GLuint) = NULL;

static int Load_ARB_vertex_array_object()
{
  int numFailed = 0;
  vpvl2__ptrc_glBindVertexArray = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glBindVertexArray");
  if(!vpvl2__ptrc_glBindVertexArray) numFailed++;
  vpvl2__ptrc_glDeleteVertexArrays = (void (CODEGEN_FUNCPTR *)(GLsizei, const GLuint *))IntGetProcAddress("glDeleteVertexArrays");
  if(!vpvl2__ptrc_glDeleteVertexArrays) numFailed++;
  vpvl2__ptrc_glGenVertexArrays = (void (CODEGEN_FUNCPTR *)(GLsizei, GLuint *))IntGetProcAddress("glGenVertexArrays");
  if(!vpvl2__ptrc_glGenVertexArrays) numFailed++;
  vpvl2__ptrc_glIsVertexArray = (GLboolean (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glIsVertexArray");
  if(!vpvl2__ptrc_glIsVertexArray) numFailed++;
  return numFailed;
}

void (CODEGEN_FUNCPTR *vpvl2__ptrc_glBindVertexArrayAPPLE)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glDeleteVertexArraysAPPLE)(GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGenVertexArraysAPPLE)(GLsizei, GLuint *) = NULL;
GLboolean (CODEGEN_FUNCPTR *vpvl2__ptrc_glIsVertexArrayAPPLE)(GLuint) = NULL;

static int Load_APPLE_vertex_array_object()
{
  int numFailed = 0;
  vpvl2__ptrc_glBindVertexArrayAPPLE = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glBindVertexArrayAPPLE");
  if(!vpvl2__ptrc_glBindVertexArrayAPPLE) numFailed++;
  vpvl2__ptrc_glDeleteVertexArraysAPPLE = (void (CODEGEN_FUNCPTR *)(GLsizei, const GLuint *))IntGetProcAddress("glDeleteVertexArraysAPPLE");
  if(!vpvl2__ptrc_glDeleteVertexArraysAPPLE) numFailed++;
  vpvl2__ptrc_glGenVertexArraysAPPLE = (void (CODEGEN_FUNCPTR *)(GLsizei, GLuint *))IntGetProcAddress("glGenVertexArraysAPPLE");
  if(!vpvl2__ptrc_glGenVertexArraysAPPLE) numFailed++;
  vpvl2__ptrc_glIsVertexArrayAPPLE = (GLboolean (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glIsVertexArrayAPPLE");
  if(!vpvl2__ptrc_glIsVertexArrayAPPLE) numFailed++;
  return numFailed;
}

void (CODEGEN_FUNCPTR *vpvl2__ptrc_glBlitFramebufferEXT)(GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum) = NULL;

static int Load_EXT_framebuffer_blit()
{
  int numFailed = 0;
  vpvl2__ptrc_glBlitFramebufferEXT = (void (CODEGEN_FUNCPTR *)(GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLint, GLbitfield, GLenum))IntGetProcAddress("glBlitFramebufferEXT");
  if(!vpvl2__ptrc_glBlitFramebufferEXT) numFailed++;
  return numFailed;
}

void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRenderbufferStorageMultisampleEXT)(GLenum, GLsizei, GLenum, GLsizei, GLsizei) = NULL;

static int Load_EXT_framebuffer_multisample()
{
  int numFailed = 0;
  vpvl2__ptrc_glRenderbufferStorageMultisampleEXT = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, GLenum, GLsizei, GLsizei))IntGetProcAddress("glRenderbufferStorageMultisampleEXT");
  if(!vpvl2__ptrc_glRenderbufferStorageMultisampleEXT) numFailed++;
  return numFailed;
}

void (CODEGEN_FUNCPTR *vpvl2__ptrc_glBindFramebufferEXT)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glBindRenderbufferEXT)(GLenum, GLuint) = NULL;
GLenum (CODEGEN_FUNCPTR *vpvl2__ptrc_glCheckFramebufferStatusEXT)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glDeleteFramebuffersEXT)(GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glDeleteRenderbuffersEXT)(GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glFramebufferRenderbufferEXT)(GLenum, GLenum, GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glFramebufferTexture1DEXT)(GLenum, GLenum, GLenum, GLuint, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glFramebufferTexture2DEXT)(GLenum, GLenum, GLenum, GLuint, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glFramebufferTexture3DEXT)(GLenum, GLenum, GLenum, GLuint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGenFramebuffersEXT)(GLsizei, GLuint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGenRenderbuffersEXT)(GLsizei, GLuint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGenerateMipmapEXT)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetFramebufferAttachmentParameterivEXT)(GLenum, GLenum, GLenum, GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetRenderbufferParameterivEXT)(GLenum, GLenum, GLint *) = NULL;
GLboolean (CODEGEN_FUNCPTR *vpvl2__ptrc_glIsFramebufferEXT)(GLuint) = NULL;
GLboolean (CODEGEN_FUNCPTR *vpvl2__ptrc_glIsRenderbufferEXT)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRenderbufferStorageEXT)(GLenum, GLenum, GLsizei, GLsizei) = NULL;

static int Load_EXT_framebuffer_object()
{
  int numFailed = 0;
  vpvl2__ptrc_glBindFramebufferEXT = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glBindFramebufferEXT");
  if(!vpvl2__ptrc_glBindFramebufferEXT) numFailed++;
  vpvl2__ptrc_glBindRenderbufferEXT = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glBindRenderbufferEXT");
  if(!vpvl2__ptrc_glBindRenderbufferEXT) numFailed++;
  vpvl2__ptrc_glCheckFramebufferStatusEXT = (GLenum (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glCheckFramebufferStatusEXT");
  if(!vpvl2__ptrc_glCheckFramebufferStatusEXT) numFailed++;
  vpvl2__ptrc_glDeleteFramebuffersEXT = (void (CODEGEN_FUNCPTR *)(GLsizei, const GLuint *))IntGetProcAddress("glDeleteFramebuffersEXT");
  if(!vpvl2__ptrc_glDeleteFramebuffersEXT) numFailed++;
  vpvl2__ptrc_glDeleteRenderbuffersEXT = (void (CODEGEN_FUNCPTR *)(GLsizei, const GLuint *))IntGetProcAddress("glDeleteRenderbuffersEXT");
  if(!vpvl2__ptrc_glDeleteRenderbuffersEXT) numFailed++;
  vpvl2__ptrc_glFramebufferRenderbufferEXT = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum, GLuint))IntGetProcAddress("glFramebufferRenderbufferEXT");
  if(!vpvl2__ptrc_glFramebufferRenderbufferEXT) numFailed++;
  vpvl2__ptrc_glFramebufferTexture1DEXT = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum, GLuint, GLint))IntGetProcAddress("glFramebufferTexture1DEXT");
  if(!vpvl2__ptrc_glFramebufferTexture1DEXT) numFailed++;
  vpvl2__ptrc_glFramebufferTexture2DEXT = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum, GLuint, GLint))IntGetProcAddress("glFramebufferTexture2DEXT");
  if(!vpvl2__ptrc_glFramebufferTexture2DEXT) numFailed++;
  vpvl2__ptrc_glFramebufferTexture3DEXT = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum, GLuint, GLint, GLint))IntGetProcAddress("glFramebufferTexture3DEXT");
  if(!vpvl2__ptrc_glFramebufferTexture3DEXT) numFailed++;
  vpvl2__ptrc_glGenFramebuffersEXT = (void (CODEGEN_FUNCPTR *)(GLsizei, GLuint *))IntGetProcAddress("glGenFramebuffersEXT");
  if(!vpvl2__ptrc_glGenFramebuffersEXT) numFailed++;
  vpvl2__ptrc_glGenRenderbuffersEXT = (void (CODEGEN_FUNCPTR *)(GLsizei, GLuint *))IntGetProcAddress("glGenRenderbuffersEXT");
  if(!vpvl2__ptrc_glGenRenderbuffersEXT) numFailed++;
  vpvl2__ptrc_glGenerateMipmapEXT = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glGenerateMipmapEXT");
  if(!vpvl2__ptrc_glGenerateMipmapEXT) numFailed++;
  vpvl2__ptrc_glGetFramebufferAttachmentParameterivEXT = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum, GLint *))IntGetProcAddress("glGetFramebufferAttachmentParameterivEXT");
  if(!vpvl2__ptrc_glGetFramebufferAttachmentParameterivEXT) numFailed++;
  vpvl2__ptrc_glGetRenderbufferParameterivEXT = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint *))IntGetProcAddress("glGetRenderbufferParameterivEXT");
  if(!vpvl2__ptrc_glGetRenderbufferParameterivEXT) numFailed++;
  vpvl2__ptrc_glIsFramebufferEXT = (GLboolean (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glIsFramebufferEXT");
  if(!vpvl2__ptrc_glIsFramebufferEXT) numFailed++;
  vpvl2__ptrc_glIsRenderbufferEXT = (GLboolean (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glIsRenderbufferEXT");
  if(!vpvl2__ptrc_glIsRenderbufferEXT) numFailed++;
  vpvl2__ptrc_glRenderbufferStorageEXT = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLsizei, GLsizei))IntGetProcAddress("glRenderbufferStorageEXT");
  if(!vpvl2__ptrc_glRenderbufferStorageEXT) numFailed++;
  return numFailed;
}

void (CODEGEN_FUNCPTR *vpvl2__ptrc_glAccum)(GLenum, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glAlphaFunc)(GLenum, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glBegin)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glBitmap)(GLsizei, GLsizei, GLfloat, GLfloat, GLfloat, GLfloat, const GLubyte *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glBlendFunc)(GLenum, GLenum) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glCallList)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glCallLists)(GLsizei, GLenum, const GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glClear)(GLbitfield) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glClearAccum)(GLfloat, GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glClearColor)(GLfloat, GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glClearDepth)(GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glClearIndex)(GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glClearStencil)(GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glClipPlane)(GLenum, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glColor3b)(GLbyte, GLbyte, GLbyte) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glColor3bv)(const GLbyte *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glColor3d)(GLdouble, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glColor3dv)(const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glColor3f)(GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glColor3fv)(const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glColor3i)(GLint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glColor3iv)(const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glColor3s)(GLshort, GLshort, GLshort) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glColor3sv)(const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glColor3ub)(GLubyte, GLubyte, GLubyte) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glColor3ubv)(const GLubyte *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glColor3ui)(GLuint, GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glColor3uiv)(const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glColor3us)(GLushort, GLushort, GLushort) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glColor3usv)(const GLushort *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glColor4b)(GLbyte, GLbyte, GLbyte, GLbyte) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glColor4bv)(const GLbyte *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glColor4d)(GLdouble, GLdouble, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glColor4dv)(const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glColor4f)(GLfloat, GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glColor4fv)(const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glColor4i)(GLint, GLint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glColor4iv)(const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glColor4s)(GLshort, GLshort, GLshort, GLshort) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glColor4sv)(const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glColor4ub)(GLubyte, GLubyte, GLubyte, GLubyte) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glColor4ubv)(const GLubyte *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glColor4ui)(GLuint, GLuint, GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glColor4uiv)(const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glColor4us)(GLushort, GLushort, GLushort, GLushort) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glColor4usv)(const GLushort *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glColorMask)(GLboolean, GLboolean, GLboolean, GLboolean) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glColorMaterial)(GLenum, GLenum) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glCopyPixels)(GLint, GLint, GLsizei, GLsizei, GLenum) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glCullFace)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glDeleteLists)(GLuint, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glDepthFunc)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glDepthMask)(GLboolean) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glDepthRange)(GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glDisable)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glDrawBuffer)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glDrawPixels)(GLsizei, GLsizei, GLenum, GLenum, const GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glEdgeFlag)(GLboolean) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glEdgeFlagv)(const GLboolean *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glEnable)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glEnd)() = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glEndList)() = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glEvalCoord1d)(GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glEvalCoord1dv)(const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glEvalCoord1f)(GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glEvalCoord1fv)(const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glEvalCoord2d)(GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glEvalCoord2dv)(const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glEvalCoord2f)(GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glEvalCoord2fv)(const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glEvalMesh1)(GLenum, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glEvalMesh2)(GLenum, GLint, GLint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glEvalPoint1)(GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glEvalPoint2)(GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glFeedbackBuffer)(GLsizei, GLenum, GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glFinish)() = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glFlush)() = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glFogf)(GLenum, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glFogfv)(GLenum, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glFogi)(GLenum, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glFogiv)(GLenum, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glFrontFace)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glFrustum)(GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble) = NULL;
GLuint (CODEGEN_FUNCPTR *vpvl2__ptrc_glGenLists)(GLsizei) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetBooleanv)(GLenum, GLboolean *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetClipPlane)(GLenum, GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetDoublev)(GLenum, GLdouble *) = NULL;
GLenum (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetError)() = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetFloatv)(GLenum, GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetIntegerv)(GLenum, GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetLightfv)(GLenum, GLenum, GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetLightiv)(GLenum, GLenum, GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetMapdv)(GLenum, GLenum, GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetMapfv)(GLenum, GLenum, GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetMapiv)(GLenum, GLenum, GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetMaterialfv)(GLenum, GLenum, GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetMaterialiv)(GLenum, GLenum, GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetPixelMapfv)(GLenum, GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetPixelMapuiv)(GLenum, GLuint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetPixelMapusv)(GLenum, GLushort *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetPolygonStipple)(GLubyte *) = NULL;
const GLubyte * (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetString)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetTexEnvfv)(GLenum, GLenum, GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetTexEnviv)(GLenum, GLenum, GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetTexGendv)(GLenum, GLenum, GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetTexGenfv)(GLenum, GLenum, GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetTexGeniv)(GLenum, GLenum, GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetTexImage)(GLenum, GLint, GLenum, GLenum, GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetTexLevelParameterfv)(GLenum, GLint, GLenum, GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetTexLevelParameteriv)(GLenum, GLint, GLenum, GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetTexParameterfv)(GLenum, GLenum, GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetTexParameteriv)(GLenum, GLenum, GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glHint)(GLenum, GLenum) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glIndexMask)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glIndexd)(GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glIndexdv)(const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glIndexf)(GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glIndexfv)(const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glIndexi)(GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glIndexiv)(const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glIndexs)(GLshort) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glIndexsv)(const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glInitNames)() = NULL;
GLboolean (CODEGEN_FUNCPTR *vpvl2__ptrc_glIsEnabled)(GLenum) = NULL;
GLboolean (CODEGEN_FUNCPTR *vpvl2__ptrc_glIsList)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glLightModelf)(GLenum, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glLightModelfv)(GLenum, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glLightModeli)(GLenum, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glLightModeliv)(GLenum, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glLightf)(GLenum, GLenum, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glLightfv)(GLenum, GLenum, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glLighti)(GLenum, GLenum, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glLightiv)(GLenum, GLenum, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glLineStipple)(GLint, GLushort) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glLineWidth)(GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glListBase)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glLoadIdentity)() = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glLoadMatrixd)(const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glLoadMatrixf)(const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glLoadName)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glLogicOp)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMap1d)(GLenum, GLdouble, GLdouble, GLint, GLint, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMap1f)(GLenum, GLfloat, GLfloat, GLint, GLint, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMap2d)(GLenum, GLdouble, GLdouble, GLint, GLint, GLdouble, GLdouble, GLint, GLint, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMap2f)(GLenum, GLfloat, GLfloat, GLint, GLint, GLfloat, GLfloat, GLint, GLint, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMapGrid1d)(GLint, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMapGrid1f)(GLint, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMapGrid2d)(GLint, GLdouble, GLdouble, GLint, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMapGrid2f)(GLint, GLfloat, GLfloat, GLint, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMaterialf)(GLenum, GLenum, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMaterialfv)(GLenum, GLenum, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMateriali)(GLenum, GLenum, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMaterialiv)(GLenum, GLenum, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMatrixMode)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultMatrixd)(const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultMatrixf)(const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glNewList)(GLuint, GLenum) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glNormal3b)(GLbyte, GLbyte, GLbyte) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glNormal3bv)(const GLbyte *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glNormal3d)(GLdouble, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glNormal3dv)(const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glNormal3f)(GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glNormal3fv)(const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glNormal3i)(GLint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glNormal3iv)(const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glNormal3s)(GLshort, GLshort, GLshort) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glNormal3sv)(const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glOrtho)(GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glPassThrough)(GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glPixelMapfv)(GLenum, GLsizei, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glPixelMapuiv)(GLenum, GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glPixelMapusv)(GLenum, GLsizei, const GLushort *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glPixelStoref)(GLenum, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glPixelStorei)(GLenum, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glPixelTransferf)(GLenum, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glPixelTransferi)(GLenum, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glPixelZoom)(GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glPointSize)(GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glPolygonMode)(GLenum, GLenum) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glPolygonStipple)(const GLubyte *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glPopAttrib)() = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glPopMatrix)() = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glPopName)() = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glPushAttrib)(GLbitfield) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glPushMatrix)() = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glPushName)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRasterPos2d)(GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRasterPos2dv)(const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRasterPos2f)(GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRasterPos2fv)(const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRasterPos2i)(GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRasterPos2iv)(const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRasterPos2s)(GLshort, GLshort) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRasterPos2sv)(const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRasterPos3d)(GLdouble, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRasterPos3dv)(const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRasterPos3f)(GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRasterPos3fv)(const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRasterPos3i)(GLint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRasterPos3iv)(const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRasterPos3s)(GLshort, GLshort, GLshort) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRasterPos3sv)(const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRasterPos4d)(GLdouble, GLdouble, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRasterPos4dv)(const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRasterPos4f)(GLfloat, GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRasterPos4fv)(const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRasterPos4i)(GLint, GLint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRasterPos4iv)(const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRasterPos4s)(GLshort, GLshort, GLshort, GLshort) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRasterPos4sv)(const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glReadBuffer)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glReadPixels)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRectd)(GLdouble, GLdouble, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRectdv)(const GLdouble *, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRectf)(GLfloat, GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRectfv)(const GLfloat *, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRecti)(GLint, GLint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRectiv)(const GLint *, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRects)(GLshort, GLshort, GLshort, GLshort) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRectsv)(const GLshort *, const GLshort *) = NULL;
GLint (CODEGEN_FUNCPTR *vpvl2__ptrc_glRenderMode)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRotated)(GLdouble, GLdouble, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glRotatef)(GLfloat, GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glScaled)(GLdouble, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glScalef)(GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glScissor)(GLint, GLint, GLsizei, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glSelectBuffer)(GLsizei, GLuint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glShadeModel)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glStencilFunc)(GLenum, GLint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glStencilMask)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glStencilOp)(GLenum, GLenum, GLenum) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexCoord1d)(GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexCoord1dv)(const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexCoord1f)(GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexCoord1fv)(const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexCoord1i)(GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexCoord1iv)(const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexCoord1s)(GLshort) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexCoord1sv)(const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexCoord2d)(GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexCoord2dv)(const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexCoord2f)(GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexCoord2fv)(const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexCoord2i)(GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexCoord2iv)(const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexCoord2s)(GLshort, GLshort) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexCoord2sv)(const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexCoord3d)(GLdouble, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexCoord3dv)(const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexCoord3f)(GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexCoord3fv)(const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexCoord3i)(GLint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexCoord3iv)(const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexCoord3s)(GLshort, GLshort, GLshort) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexCoord3sv)(const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexCoord4d)(GLdouble, GLdouble, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexCoord4dv)(const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexCoord4f)(GLfloat, GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexCoord4fv)(const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexCoord4i)(GLint, GLint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexCoord4iv)(const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexCoord4s)(GLshort, GLshort, GLshort, GLshort) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexCoord4sv)(const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexEnvf)(GLenum, GLenum, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexEnvfv)(GLenum, GLenum, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexEnvi)(GLenum, GLenum, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexEnviv)(GLenum, GLenum, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexGend)(GLenum, GLenum, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexGendv)(GLenum, GLenum, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexGenf)(GLenum, GLenum, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexGenfv)(GLenum, GLenum, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexGeni)(GLenum, GLenum, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexGeniv)(GLenum, GLenum, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexImage1D)(GLenum, GLint, GLint, GLsizei, GLint, GLenum, GLenum, const GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexImage2D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexParameterf)(GLenum, GLenum, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexParameterfv)(GLenum, GLenum, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexParameteri)(GLenum, GLenum, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexParameteriv)(GLenum, GLenum, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTranslated)(GLdouble, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTranslatef)(GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertex2d)(GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertex2dv)(const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertex2f)(GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertex2fv)(const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertex2i)(GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertex2iv)(const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertex2s)(GLshort, GLshort) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertex2sv)(const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertex3d)(GLdouble, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertex3dv)(const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertex3f)(GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertex3fv)(const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertex3i)(GLint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertex3iv)(const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertex3s)(GLshort, GLshort, GLshort) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertex3sv)(const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertex4d)(GLdouble, GLdouble, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertex4dv)(const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertex4f)(GLfloat, GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertex4fv)(const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertex4i)(GLint, GLint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertex4iv)(const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertex4s)(GLshort, GLshort, GLshort, GLshort) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertex4sv)(const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glViewport)(GLint, GLint, GLsizei, GLsizei) = NULL;

GLboolean (CODEGEN_FUNCPTR *vpvl2__ptrc_glAreTexturesResident)(GLsizei, const GLuint *, GLboolean *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glArrayElement)(GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glBindTexture)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glColorPointer)(GLint, GLenum, GLsizei, const GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glCopyTexImage1D)(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glCopyTexImage2D)(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glCopyTexSubImage1D)(GLenum, GLint, GLint, GLint, GLint, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glCopyTexSubImage2D)(GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glDeleteTextures)(GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glDisableClientState)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glDrawArrays)(GLenum, GLint, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glDrawElements)(GLenum, GLsizei, GLenum, const GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glEdgeFlagPointer)(GLsizei, const GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glEnableClientState)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGenTextures)(GLsizei, GLuint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetPointerv)(GLenum, GLvoid **) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glIndexPointer)(GLenum, GLsizei, const GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glIndexub)(GLubyte) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glIndexubv)(const GLubyte *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glInterleavedArrays)(GLenum, GLsizei, const GLvoid *) = NULL;
GLboolean (CODEGEN_FUNCPTR *vpvl2__ptrc_glIsTexture)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glNormalPointer)(GLenum, GLsizei, const GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glPolygonOffset)(GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glPopClientAttrib)() = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glPrioritizeTextures)(GLsizei, const GLuint *, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glPushClientAttrib)(GLbitfield) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexCoordPointer)(GLint, GLenum, GLsizei, const GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexSubImage1D)(GLenum, GLint, GLint, GLsizei, GLenum, GLenum, const GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexSubImage2D)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexPointer)(GLint, GLenum, GLsizei, const GLvoid *) = NULL;

void (CODEGEN_FUNCPTR *vpvl2__ptrc_glBlendColor)(GLfloat, GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glBlendEquation)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glCopyTexSubImage3D)(GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glDrawRangeElements)(GLenum, GLuint, GLuint, GLsizei, GLenum, const GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexImage3D)(GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glTexSubImage3D)(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *) = NULL;

void (CODEGEN_FUNCPTR *vpvl2__ptrc_glActiveTexture)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glClientActiveTexture)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glCompressedTexImage1D)(GLenum, GLint, GLenum, GLsizei, GLint, GLsizei, const GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glCompressedTexImage2D)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glCompressedTexImage3D)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glCompressedTexSubImage1D)(GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, const GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glCompressedTexSubImage2D)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glCompressedTexSubImage3D)(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetCompressedTexImage)(GLenum, GLint, GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glLoadTransposeMatrixd)(const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glLoadTransposeMatrixf)(const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultTransposeMatrixd)(const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultTransposeMatrixf)(const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultiTexCoord1d)(GLenum, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultiTexCoord1dv)(GLenum, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultiTexCoord1f)(GLenum, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultiTexCoord1fv)(GLenum, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultiTexCoord1i)(GLenum, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultiTexCoord1iv)(GLenum, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultiTexCoord1s)(GLenum, GLshort) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultiTexCoord1sv)(GLenum, const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultiTexCoord2d)(GLenum, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultiTexCoord2dv)(GLenum, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultiTexCoord2f)(GLenum, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultiTexCoord2fv)(GLenum, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultiTexCoord2i)(GLenum, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultiTexCoord2iv)(GLenum, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultiTexCoord2s)(GLenum, GLshort, GLshort) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultiTexCoord2sv)(GLenum, const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultiTexCoord3d)(GLenum, GLdouble, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultiTexCoord3dv)(GLenum, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultiTexCoord3f)(GLenum, GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultiTexCoord3fv)(GLenum, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultiTexCoord3i)(GLenum, GLint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultiTexCoord3iv)(GLenum, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultiTexCoord3s)(GLenum, GLshort, GLshort, GLshort) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultiTexCoord3sv)(GLenum, const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultiTexCoord4d)(GLenum, GLdouble, GLdouble, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultiTexCoord4dv)(GLenum, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultiTexCoord4f)(GLenum, GLfloat, GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultiTexCoord4fv)(GLenum, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultiTexCoord4i)(GLenum, GLint, GLint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultiTexCoord4iv)(GLenum, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultiTexCoord4s)(GLenum, GLshort, GLshort, GLshort, GLshort) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultiTexCoord4sv)(GLenum, const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glSampleCoverage)(GLfloat, GLboolean) = NULL;

void (CODEGEN_FUNCPTR *vpvl2__ptrc_glBlendFuncSeparate)(GLenum, GLenum, GLenum, GLenum) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glFogCoordPointer)(GLenum, GLsizei, const GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glFogCoordd)(GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glFogCoorddv)(const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glFogCoordf)(GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glFogCoordfv)(const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultiDrawArrays)(GLenum, const GLint *, const GLsizei *, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glMultiDrawElements)(GLenum, const GLsizei *, GLenum, const GLvoid *const*, GLsizei) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glPointParameterf)(GLenum, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glPointParameterfv)(GLenum, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glPointParameteri)(GLenum, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glPointParameteriv)(GLenum, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glSecondaryColor3b)(GLbyte, GLbyte, GLbyte) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glSecondaryColor3bv)(const GLbyte *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glSecondaryColor3d)(GLdouble, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glSecondaryColor3dv)(const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glSecondaryColor3f)(GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glSecondaryColor3fv)(const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glSecondaryColor3i)(GLint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glSecondaryColor3iv)(const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glSecondaryColor3s)(GLshort, GLshort, GLshort) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glSecondaryColor3sv)(const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glSecondaryColor3ub)(GLubyte, GLubyte, GLubyte) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glSecondaryColor3ubv)(const GLubyte *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glSecondaryColor3ui)(GLuint, GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glSecondaryColor3uiv)(const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glSecondaryColor3us)(GLushort, GLushort, GLushort) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glSecondaryColor3usv)(const GLushort *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glSecondaryColorPointer)(GLint, GLenum, GLsizei, const GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glWindowPos2d)(GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glWindowPos2dv)(const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glWindowPos2f)(GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glWindowPos2fv)(const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glWindowPos2i)(GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glWindowPos2iv)(const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glWindowPos2s)(GLshort, GLshort) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glWindowPos2sv)(const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glWindowPos3d)(GLdouble, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glWindowPos3dv)(const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glWindowPos3f)(GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glWindowPos3fv)(const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glWindowPos3i)(GLint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glWindowPos3iv)(const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glWindowPos3s)(GLshort, GLshort, GLshort) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glWindowPos3sv)(const GLshort *) = NULL;

void (CODEGEN_FUNCPTR *vpvl2__ptrc_glBeginQuery)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glBindBuffer)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glBufferData)(GLenum, GLsizeiptr, const GLvoid *, GLenum) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glBufferSubData)(GLenum, GLintptr, GLsizeiptr, const GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glDeleteBuffers)(GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glDeleteQueries)(GLsizei, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glEndQuery)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGenBuffers)(GLsizei, GLuint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGenQueries)(GLsizei, GLuint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetBufferParameteriv)(GLenum, GLenum, GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetBufferPointerv)(GLenum, GLenum, GLvoid **) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetBufferSubData)(GLenum, GLintptr, GLsizeiptr, GLvoid *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetQueryObjectiv)(GLuint, GLenum, GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetQueryObjectuiv)(GLuint, GLenum, GLuint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetQueryiv)(GLenum, GLenum, GLint *) = NULL;
GLboolean (CODEGEN_FUNCPTR *vpvl2__ptrc_glIsBuffer)(GLuint) = NULL;
GLboolean (CODEGEN_FUNCPTR *vpvl2__ptrc_glIsQuery)(GLuint) = NULL;
void * (CODEGEN_FUNCPTR *vpvl2__ptrc_glMapBuffer)(GLenum, GLenum) = NULL;
GLboolean (CODEGEN_FUNCPTR *vpvl2__ptrc_glUnmapBuffer)(GLenum) = NULL;

void (CODEGEN_FUNCPTR *vpvl2__ptrc_glAttachShader)(GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glBindAttribLocation)(GLuint, GLuint, const GLchar *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glBlendEquationSeparate)(GLenum, GLenum) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glCompileShader)(GLuint) = NULL;
GLuint (CODEGEN_FUNCPTR *vpvl2__ptrc_glCreateProgram)() = NULL;
GLuint (CODEGEN_FUNCPTR *vpvl2__ptrc_glCreateShader)(GLenum) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glDeleteProgram)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glDeleteShader)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glDetachShader)(GLuint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glDisableVertexAttribArray)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glDrawBuffers)(GLsizei, const GLenum *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glEnableVertexAttribArray)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetActiveAttrib)(GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetActiveUniform)(GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetAttachedShaders)(GLuint, GLsizei, GLsizei *, GLuint *) = NULL;
GLint (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetAttribLocation)(GLuint, const GLchar *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetProgramInfoLog)(GLuint, GLsizei, GLsizei *, GLchar *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetProgramiv)(GLuint, GLenum, GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetShaderInfoLog)(GLuint, GLsizei, GLsizei *, GLchar *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetShaderSource)(GLuint, GLsizei, GLsizei *, GLchar *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetShaderiv)(GLuint, GLenum, GLint *) = NULL;
GLint (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetUniformLocation)(GLuint, const GLchar *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetUniformfv)(GLuint, GLint, GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetUniformiv)(GLuint, GLint, GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetVertexAttribPointerv)(GLuint, GLenum, GLvoid **) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetVertexAttribdv)(GLuint, GLenum, GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetVertexAttribfv)(GLuint, GLenum, GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glGetVertexAttribiv)(GLuint, GLenum, GLint *) = NULL;
GLboolean (CODEGEN_FUNCPTR *vpvl2__ptrc_glIsProgram)(GLuint) = NULL;
GLboolean (CODEGEN_FUNCPTR *vpvl2__ptrc_glIsShader)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glLinkProgram)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glShaderSource)(GLuint, GLsizei, const GLchar *const*, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glStencilFuncSeparate)(GLenum, GLenum, GLint, GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glStencilMaskSeparate)(GLenum, GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glStencilOpSeparate)(GLenum, GLenum, GLenum, GLenum) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glUniform1f)(GLint, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glUniform1fv)(GLint, GLsizei, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glUniform1i)(GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glUniform1iv)(GLint, GLsizei, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glUniform2f)(GLint, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glUniform2fv)(GLint, GLsizei, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glUniform2i)(GLint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glUniform2iv)(GLint, GLsizei, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glUniform3f)(GLint, GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glUniform3fv)(GLint, GLsizei, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glUniform3i)(GLint, GLint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glUniform3iv)(GLint, GLsizei, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glUniform4f)(GLint, GLfloat, GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glUniform4fv)(GLint, GLsizei, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glUniform4i)(GLint, GLint, GLint, GLint, GLint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glUniform4iv)(GLint, GLsizei, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glUniformMatrix2fv)(GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glUniformMatrix3fv)(GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glUniformMatrix4fv)(GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glUseProgram)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glValidateProgram)(GLuint) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttrib1d)(GLuint, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttrib1dv)(GLuint, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttrib1f)(GLuint, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttrib1fv)(GLuint, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttrib1s)(GLuint, GLshort) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttrib1sv)(GLuint, const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttrib2d)(GLuint, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttrib2dv)(GLuint, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttrib2f)(GLuint, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttrib2fv)(GLuint, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttrib2s)(GLuint, GLshort, GLshort) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttrib2sv)(GLuint, const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttrib3d)(GLuint, GLdouble, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttrib3dv)(GLuint, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttrib3f)(GLuint, GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttrib3fv)(GLuint, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttrib3s)(GLuint, GLshort, GLshort, GLshort) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttrib3sv)(GLuint, const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttrib4Nbv)(GLuint, const GLbyte *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttrib4Niv)(GLuint, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttrib4Nsv)(GLuint, const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttrib4Nub)(GLuint, GLubyte, GLubyte, GLubyte, GLubyte) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttrib4Nubv)(GLuint, const GLubyte *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttrib4Nuiv)(GLuint, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttrib4Nusv)(GLuint, const GLushort *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttrib4bv)(GLuint, const GLbyte *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttrib4d)(GLuint, GLdouble, GLdouble, GLdouble, GLdouble) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttrib4dv)(GLuint, const GLdouble *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttrib4f)(GLuint, GLfloat, GLfloat, GLfloat, GLfloat) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttrib4fv)(GLuint, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttrib4iv)(GLuint, const GLint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttrib4s)(GLuint, GLshort, GLshort, GLshort, GLshort) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttrib4sv)(GLuint, const GLshort *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttrib4ubv)(GLuint, const GLubyte *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttrib4uiv)(GLuint, const GLuint *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttrib4usv)(GLuint, const GLushort *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glVertexAttribPointer)(GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid *) = NULL;

void (CODEGEN_FUNCPTR *vpvl2__ptrc_glUniformMatrix2x3fv)(GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glUniformMatrix2x4fv)(GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glUniformMatrix3x2fv)(GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glUniformMatrix3x4fv)(GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glUniformMatrix4x2fv)(GLint, GLsizei, GLboolean, const GLfloat *) = NULL;
void (CODEGEN_FUNCPTR *vpvl2__ptrc_glUniformMatrix4x3fv)(GLint, GLsizei, GLboolean, const GLfloat *) = NULL;

static int Load_Version_2_1()
{
  int numFailed = 0;
  vpvl2__ptrc_glAccum = (void (CODEGEN_FUNCPTR *)(GLenum, GLfloat))IntGetProcAddress("glAccum");
  if(!vpvl2__ptrc_glAccum) numFailed++;
  vpvl2__ptrc_glAlphaFunc = (void (CODEGEN_FUNCPTR *)(GLenum, GLfloat))IntGetProcAddress("glAlphaFunc");
  if(!vpvl2__ptrc_glAlphaFunc) numFailed++;
  vpvl2__ptrc_glBegin = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glBegin");
  if(!vpvl2__ptrc_glBegin) numFailed++;
  vpvl2__ptrc_glBitmap = (void (CODEGEN_FUNCPTR *)(GLsizei, GLsizei, GLfloat, GLfloat, GLfloat, GLfloat, const GLubyte *))IntGetProcAddress("glBitmap");
  if(!vpvl2__ptrc_glBitmap) numFailed++;
  vpvl2__ptrc_glBlendFunc = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum))IntGetProcAddress("glBlendFunc");
  if(!vpvl2__ptrc_glBlendFunc) numFailed++;
  vpvl2__ptrc_glCallList = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glCallList");
  if(!vpvl2__ptrc_glCallList) numFailed++;
  vpvl2__ptrc_glCallLists = (void (CODEGEN_FUNCPTR *)(GLsizei, GLenum, const GLvoid *))IntGetProcAddress("glCallLists");
  if(!vpvl2__ptrc_glCallLists) numFailed++;
  vpvl2__ptrc_glClear = (void (CODEGEN_FUNCPTR *)(GLbitfield))IntGetProcAddress("glClear");
  if(!vpvl2__ptrc_glClear) numFailed++;
  vpvl2__ptrc_glClearAccum = (void (CODEGEN_FUNCPTR *)(GLfloat, GLfloat, GLfloat, GLfloat))IntGetProcAddress("glClearAccum");
  if(!vpvl2__ptrc_glClearAccum) numFailed++;
  vpvl2__ptrc_glClearColor = (void (CODEGEN_FUNCPTR *)(GLfloat, GLfloat, GLfloat, GLfloat))IntGetProcAddress("glClearColor");
  if(!vpvl2__ptrc_glClearColor) numFailed++;
  vpvl2__ptrc_glClearDepth = (void (CODEGEN_FUNCPTR *)(GLdouble))IntGetProcAddress("glClearDepth");
  if(!vpvl2__ptrc_glClearDepth) numFailed++;
  vpvl2__ptrc_glClearIndex = (void (CODEGEN_FUNCPTR *)(GLfloat))IntGetProcAddress("glClearIndex");
  if(!vpvl2__ptrc_glClearIndex) numFailed++;
  vpvl2__ptrc_glClearStencil = (void (CODEGEN_FUNCPTR *)(GLint))IntGetProcAddress("glClearStencil");
  if(!vpvl2__ptrc_glClearStencil) numFailed++;
  vpvl2__ptrc_glClipPlane = (void (CODEGEN_FUNCPTR *)(GLenum, const GLdouble *))IntGetProcAddress("glClipPlane");
  if(!vpvl2__ptrc_glClipPlane) numFailed++;
  vpvl2__ptrc_glColor3b = (void (CODEGEN_FUNCPTR *)(GLbyte, GLbyte, GLbyte))IntGetProcAddress("glColor3b");
  if(!vpvl2__ptrc_glColor3b) numFailed++;
  vpvl2__ptrc_glColor3bv = (void (CODEGEN_FUNCPTR *)(const GLbyte *))IntGetProcAddress("glColor3bv");
  if(!vpvl2__ptrc_glColor3bv) numFailed++;
  vpvl2__ptrc_glColor3d = (void (CODEGEN_FUNCPTR *)(GLdouble, GLdouble, GLdouble))IntGetProcAddress("glColor3d");
  if(!vpvl2__ptrc_glColor3d) numFailed++;
  vpvl2__ptrc_glColor3dv = (void (CODEGEN_FUNCPTR *)(const GLdouble *))IntGetProcAddress("glColor3dv");
  if(!vpvl2__ptrc_glColor3dv) numFailed++;
  vpvl2__ptrc_glColor3f = (void (CODEGEN_FUNCPTR *)(GLfloat, GLfloat, GLfloat))IntGetProcAddress("glColor3f");
  if(!vpvl2__ptrc_glColor3f) numFailed++;
  vpvl2__ptrc_glColor3fv = (void (CODEGEN_FUNCPTR *)(const GLfloat *))IntGetProcAddress("glColor3fv");
  if(!vpvl2__ptrc_glColor3fv) numFailed++;
  vpvl2__ptrc_glColor3i = (void (CODEGEN_FUNCPTR *)(GLint, GLint, GLint))IntGetProcAddress("glColor3i");
  if(!vpvl2__ptrc_glColor3i) numFailed++;
  vpvl2__ptrc_glColor3iv = (void (CODEGEN_FUNCPTR *)(const GLint *))IntGetProcAddress("glColor3iv");
  if(!vpvl2__ptrc_glColor3iv) numFailed++;
  vpvl2__ptrc_glColor3s = (void (CODEGEN_FUNCPTR *)(GLshort, GLshort, GLshort))IntGetProcAddress("glColor3s");
  if(!vpvl2__ptrc_glColor3s) numFailed++;
  vpvl2__ptrc_glColor3sv = (void (CODEGEN_FUNCPTR *)(const GLshort *))IntGetProcAddress("glColor3sv");
  if(!vpvl2__ptrc_glColor3sv) numFailed++;
  vpvl2__ptrc_glColor3ub = (void (CODEGEN_FUNCPTR *)(GLubyte, GLubyte, GLubyte))IntGetProcAddress("glColor3ub");
  if(!vpvl2__ptrc_glColor3ub) numFailed++;
  vpvl2__ptrc_glColor3ubv = (void (CODEGEN_FUNCPTR *)(const GLubyte *))IntGetProcAddress("glColor3ubv");
  if(!vpvl2__ptrc_glColor3ubv) numFailed++;
  vpvl2__ptrc_glColor3ui = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint, GLuint))IntGetProcAddress("glColor3ui");
  if(!vpvl2__ptrc_glColor3ui) numFailed++;
  vpvl2__ptrc_glColor3uiv = (void (CODEGEN_FUNCPTR *)(const GLuint *))IntGetProcAddress("glColor3uiv");
  if(!vpvl2__ptrc_glColor3uiv) numFailed++;
  vpvl2__ptrc_glColor3us = (void (CODEGEN_FUNCPTR *)(GLushort, GLushort, GLushort))IntGetProcAddress("glColor3us");
  if(!vpvl2__ptrc_glColor3us) numFailed++;
  vpvl2__ptrc_glColor3usv = (void (CODEGEN_FUNCPTR *)(const GLushort *))IntGetProcAddress("glColor3usv");
  if(!vpvl2__ptrc_glColor3usv) numFailed++;
  vpvl2__ptrc_glColor4b = (void (CODEGEN_FUNCPTR *)(GLbyte, GLbyte, GLbyte, GLbyte))IntGetProcAddress("glColor4b");
  if(!vpvl2__ptrc_glColor4b) numFailed++;
  vpvl2__ptrc_glColor4bv = (void (CODEGEN_FUNCPTR *)(const GLbyte *))IntGetProcAddress("glColor4bv");
  if(!vpvl2__ptrc_glColor4bv) numFailed++;
  vpvl2__ptrc_glColor4d = (void (CODEGEN_FUNCPTR *)(GLdouble, GLdouble, GLdouble, GLdouble))IntGetProcAddress("glColor4d");
  if(!vpvl2__ptrc_glColor4d) numFailed++;
  vpvl2__ptrc_glColor4dv = (void (CODEGEN_FUNCPTR *)(const GLdouble *))IntGetProcAddress("glColor4dv");
  if(!vpvl2__ptrc_glColor4dv) numFailed++;
  vpvl2__ptrc_glColor4f = (void (CODEGEN_FUNCPTR *)(GLfloat, GLfloat, GLfloat, GLfloat))IntGetProcAddress("glColor4f");
  if(!vpvl2__ptrc_glColor4f) numFailed++;
  vpvl2__ptrc_glColor4fv = (void (CODEGEN_FUNCPTR *)(const GLfloat *))IntGetProcAddress("glColor4fv");
  if(!vpvl2__ptrc_glColor4fv) numFailed++;
  vpvl2__ptrc_glColor4i = (void (CODEGEN_FUNCPTR *)(GLint, GLint, GLint, GLint))IntGetProcAddress("glColor4i");
  if(!vpvl2__ptrc_glColor4i) numFailed++;
  vpvl2__ptrc_glColor4iv = (void (CODEGEN_FUNCPTR *)(const GLint *))IntGetProcAddress("glColor4iv");
  if(!vpvl2__ptrc_glColor4iv) numFailed++;
  vpvl2__ptrc_glColor4s = (void (CODEGEN_FUNCPTR *)(GLshort, GLshort, GLshort, GLshort))IntGetProcAddress("glColor4s");
  if(!vpvl2__ptrc_glColor4s) numFailed++;
  vpvl2__ptrc_glColor4sv = (void (CODEGEN_FUNCPTR *)(const GLshort *))IntGetProcAddress("glColor4sv");
  if(!vpvl2__ptrc_glColor4sv) numFailed++;
  vpvl2__ptrc_glColor4ub = (void (CODEGEN_FUNCPTR *)(GLubyte, GLubyte, GLubyte, GLubyte))IntGetProcAddress("glColor4ub");
  if(!vpvl2__ptrc_glColor4ub) numFailed++;
  vpvl2__ptrc_glColor4ubv = (void (CODEGEN_FUNCPTR *)(const GLubyte *))IntGetProcAddress("glColor4ubv");
  if(!vpvl2__ptrc_glColor4ubv) numFailed++;
  vpvl2__ptrc_glColor4ui = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint, GLuint, GLuint))IntGetProcAddress("glColor4ui");
  if(!vpvl2__ptrc_glColor4ui) numFailed++;
  vpvl2__ptrc_glColor4uiv = (void (CODEGEN_FUNCPTR *)(const GLuint *))IntGetProcAddress("glColor4uiv");
  if(!vpvl2__ptrc_glColor4uiv) numFailed++;
  vpvl2__ptrc_glColor4us = (void (CODEGEN_FUNCPTR *)(GLushort, GLushort, GLushort, GLushort))IntGetProcAddress("glColor4us");
  if(!vpvl2__ptrc_glColor4us) numFailed++;
  vpvl2__ptrc_glColor4usv = (void (CODEGEN_FUNCPTR *)(const GLushort *))IntGetProcAddress("glColor4usv");
  if(!vpvl2__ptrc_glColor4usv) numFailed++;
  vpvl2__ptrc_glColorMask = (void (CODEGEN_FUNCPTR *)(GLboolean, GLboolean, GLboolean, GLboolean))IntGetProcAddress("glColorMask");
  if(!vpvl2__ptrc_glColorMask) numFailed++;
  vpvl2__ptrc_glColorMaterial = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum))IntGetProcAddress("glColorMaterial");
  if(!vpvl2__ptrc_glColorMaterial) numFailed++;
  vpvl2__ptrc_glCopyPixels = (void (CODEGEN_FUNCPTR *)(GLint, GLint, GLsizei, GLsizei, GLenum))IntGetProcAddress("glCopyPixels");
  if(!vpvl2__ptrc_glCopyPixels) numFailed++;
  vpvl2__ptrc_glCullFace = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glCullFace");
  if(!vpvl2__ptrc_glCullFace) numFailed++;
  vpvl2__ptrc_glDeleteLists = (void (CODEGEN_FUNCPTR *)(GLuint, GLsizei))IntGetProcAddress("glDeleteLists");
  if(!vpvl2__ptrc_glDeleteLists) numFailed++;
  vpvl2__ptrc_glDepthFunc = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glDepthFunc");
  if(!vpvl2__ptrc_glDepthFunc) numFailed++;
  vpvl2__ptrc_glDepthMask = (void (CODEGEN_FUNCPTR *)(GLboolean))IntGetProcAddress("glDepthMask");
  if(!vpvl2__ptrc_glDepthMask) numFailed++;
  vpvl2__ptrc_glDepthRange = (void (CODEGEN_FUNCPTR *)(GLdouble, GLdouble))IntGetProcAddress("glDepthRange");
  if(!vpvl2__ptrc_glDepthRange) numFailed++;
  vpvl2__ptrc_glDisable = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glDisable");
  if(!vpvl2__ptrc_glDisable) numFailed++;
  vpvl2__ptrc_glDrawBuffer = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glDrawBuffer");
  if(!vpvl2__ptrc_glDrawBuffer) numFailed++;
  vpvl2__ptrc_glDrawPixels = (void (CODEGEN_FUNCPTR *)(GLsizei, GLsizei, GLenum, GLenum, const GLvoid *))IntGetProcAddress("glDrawPixels");
  if(!vpvl2__ptrc_glDrawPixels) numFailed++;
  vpvl2__ptrc_glEdgeFlag = (void (CODEGEN_FUNCPTR *)(GLboolean))IntGetProcAddress("glEdgeFlag");
  if(!vpvl2__ptrc_glEdgeFlag) numFailed++;
  vpvl2__ptrc_glEdgeFlagv = (void (CODEGEN_FUNCPTR *)(const GLboolean *))IntGetProcAddress("glEdgeFlagv");
  if(!vpvl2__ptrc_glEdgeFlagv) numFailed++;
  vpvl2__ptrc_glEnable = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glEnable");
  if(!vpvl2__ptrc_glEnable) numFailed++;
  vpvl2__ptrc_glEnd = (void (CODEGEN_FUNCPTR *)())IntGetProcAddress("glEnd");
  if(!vpvl2__ptrc_glEnd) numFailed++;
  vpvl2__ptrc_glEndList = (void (CODEGEN_FUNCPTR *)())IntGetProcAddress("glEndList");
  if(!vpvl2__ptrc_glEndList) numFailed++;
  vpvl2__ptrc_glEvalCoord1d = (void (CODEGEN_FUNCPTR *)(GLdouble))IntGetProcAddress("glEvalCoord1d");
  if(!vpvl2__ptrc_glEvalCoord1d) numFailed++;
  vpvl2__ptrc_glEvalCoord1dv = (void (CODEGEN_FUNCPTR *)(const GLdouble *))IntGetProcAddress("glEvalCoord1dv");
  if(!vpvl2__ptrc_glEvalCoord1dv) numFailed++;
  vpvl2__ptrc_glEvalCoord1f = (void (CODEGEN_FUNCPTR *)(GLfloat))IntGetProcAddress("glEvalCoord1f");
  if(!vpvl2__ptrc_glEvalCoord1f) numFailed++;
  vpvl2__ptrc_glEvalCoord1fv = (void (CODEGEN_FUNCPTR *)(const GLfloat *))IntGetProcAddress("glEvalCoord1fv");
  if(!vpvl2__ptrc_glEvalCoord1fv) numFailed++;
  vpvl2__ptrc_glEvalCoord2d = (void (CODEGEN_FUNCPTR *)(GLdouble, GLdouble))IntGetProcAddress("glEvalCoord2d");
  if(!vpvl2__ptrc_glEvalCoord2d) numFailed++;
  vpvl2__ptrc_glEvalCoord2dv = (void (CODEGEN_FUNCPTR *)(const GLdouble *))IntGetProcAddress("glEvalCoord2dv");
  if(!vpvl2__ptrc_glEvalCoord2dv) numFailed++;
  vpvl2__ptrc_glEvalCoord2f = (void (CODEGEN_FUNCPTR *)(GLfloat, GLfloat))IntGetProcAddress("glEvalCoord2f");
  if(!vpvl2__ptrc_glEvalCoord2f) numFailed++;
  vpvl2__ptrc_glEvalCoord2fv = (void (CODEGEN_FUNCPTR *)(const GLfloat *))IntGetProcAddress("glEvalCoord2fv");
  if(!vpvl2__ptrc_glEvalCoord2fv) numFailed++;
  vpvl2__ptrc_glEvalMesh1 = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint))IntGetProcAddress("glEvalMesh1");
  if(!vpvl2__ptrc_glEvalMesh1) numFailed++;
  vpvl2__ptrc_glEvalMesh2 = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLint, GLint))IntGetProcAddress("glEvalMesh2");
  if(!vpvl2__ptrc_glEvalMesh2) numFailed++;
  vpvl2__ptrc_glEvalPoint1 = (void (CODEGEN_FUNCPTR *)(GLint))IntGetProcAddress("glEvalPoint1");
  if(!vpvl2__ptrc_glEvalPoint1) numFailed++;
  vpvl2__ptrc_glEvalPoint2 = (void (CODEGEN_FUNCPTR *)(GLint, GLint))IntGetProcAddress("glEvalPoint2");
  if(!vpvl2__ptrc_glEvalPoint2) numFailed++;
  vpvl2__ptrc_glFeedbackBuffer = (void (CODEGEN_FUNCPTR *)(GLsizei, GLenum, GLfloat *))IntGetProcAddress("glFeedbackBuffer");
  if(!vpvl2__ptrc_glFeedbackBuffer) numFailed++;
  vpvl2__ptrc_glFinish = (void (CODEGEN_FUNCPTR *)())IntGetProcAddress("glFinish");
  if(!vpvl2__ptrc_glFinish) numFailed++;
  vpvl2__ptrc_glFlush = (void (CODEGEN_FUNCPTR *)())IntGetProcAddress("glFlush");
  if(!vpvl2__ptrc_glFlush) numFailed++;
  vpvl2__ptrc_glFogf = (void (CODEGEN_FUNCPTR *)(GLenum, GLfloat))IntGetProcAddress("glFogf");
  if(!vpvl2__ptrc_glFogf) numFailed++;
  vpvl2__ptrc_glFogfv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLfloat *))IntGetProcAddress("glFogfv");
  if(!vpvl2__ptrc_glFogfv) numFailed++;
  vpvl2__ptrc_glFogi = (void (CODEGEN_FUNCPTR *)(GLenum, GLint))IntGetProcAddress("glFogi");
  if(!vpvl2__ptrc_glFogi) numFailed++;
  vpvl2__ptrc_glFogiv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLint *))IntGetProcAddress("glFogiv");
  if(!vpvl2__ptrc_glFogiv) numFailed++;
  vpvl2__ptrc_glFrontFace = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glFrontFace");
  if(!vpvl2__ptrc_glFrontFace) numFailed++;
  vpvl2__ptrc_glFrustum = (void (CODEGEN_FUNCPTR *)(GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble))IntGetProcAddress("glFrustum");
  if(!vpvl2__ptrc_glFrustum) numFailed++;
  vpvl2__ptrc_glGenLists = (GLuint (CODEGEN_FUNCPTR *)(GLsizei))IntGetProcAddress("glGenLists");
  if(!vpvl2__ptrc_glGenLists) numFailed++;
  vpvl2__ptrc_glGetBooleanv = (void (CODEGEN_FUNCPTR *)(GLenum, GLboolean *))IntGetProcAddress("glGetBooleanv");
  if(!vpvl2__ptrc_glGetBooleanv) numFailed++;
  vpvl2__ptrc_glGetClipPlane = (void (CODEGEN_FUNCPTR *)(GLenum, GLdouble *))IntGetProcAddress("glGetClipPlane");
  if(!vpvl2__ptrc_glGetClipPlane) numFailed++;
  vpvl2__ptrc_glGetDoublev = (void (CODEGEN_FUNCPTR *)(GLenum, GLdouble *))IntGetProcAddress("glGetDoublev");
  if(!vpvl2__ptrc_glGetDoublev) numFailed++;
  vpvl2__ptrc_glGetError = (GLenum (CODEGEN_FUNCPTR *)())IntGetProcAddress("glGetError");
  if(!vpvl2__ptrc_glGetError) numFailed++;
  vpvl2__ptrc_glGetFloatv = (void (CODEGEN_FUNCPTR *)(GLenum, GLfloat *))IntGetProcAddress("glGetFloatv");
  if(!vpvl2__ptrc_glGetFloatv) numFailed++;
  vpvl2__ptrc_glGetIntegerv = (void (CODEGEN_FUNCPTR *)(GLenum, GLint *))IntGetProcAddress("glGetIntegerv");
  if(!vpvl2__ptrc_glGetIntegerv) numFailed++;
  vpvl2__ptrc_glGetLightfv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLfloat *))IntGetProcAddress("glGetLightfv");
  if(!vpvl2__ptrc_glGetLightfv) numFailed++;
  vpvl2__ptrc_glGetLightiv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint *))IntGetProcAddress("glGetLightiv");
  if(!vpvl2__ptrc_glGetLightiv) numFailed++;
  vpvl2__ptrc_glGetMapdv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLdouble *))IntGetProcAddress("glGetMapdv");
  if(!vpvl2__ptrc_glGetMapdv) numFailed++;
  vpvl2__ptrc_glGetMapfv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLfloat *))IntGetProcAddress("glGetMapfv");
  if(!vpvl2__ptrc_glGetMapfv) numFailed++;
  vpvl2__ptrc_glGetMapiv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint *))IntGetProcAddress("glGetMapiv");
  if(!vpvl2__ptrc_glGetMapiv) numFailed++;
  vpvl2__ptrc_glGetMaterialfv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLfloat *))IntGetProcAddress("glGetMaterialfv");
  if(!vpvl2__ptrc_glGetMaterialfv) numFailed++;
  vpvl2__ptrc_glGetMaterialiv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint *))IntGetProcAddress("glGetMaterialiv");
  if(!vpvl2__ptrc_glGetMaterialiv) numFailed++;
  vpvl2__ptrc_glGetPixelMapfv = (void (CODEGEN_FUNCPTR *)(GLenum, GLfloat *))IntGetProcAddress("glGetPixelMapfv");
  if(!vpvl2__ptrc_glGetPixelMapfv) numFailed++;
  vpvl2__ptrc_glGetPixelMapuiv = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint *))IntGetProcAddress("glGetPixelMapuiv");
  if(!vpvl2__ptrc_glGetPixelMapuiv) numFailed++;
  vpvl2__ptrc_glGetPixelMapusv = (void (CODEGEN_FUNCPTR *)(GLenum, GLushort *))IntGetProcAddress("glGetPixelMapusv");
  if(!vpvl2__ptrc_glGetPixelMapusv) numFailed++;
  vpvl2__ptrc_glGetPolygonStipple = (void (CODEGEN_FUNCPTR *)(GLubyte *))IntGetProcAddress("glGetPolygonStipple");
  if(!vpvl2__ptrc_glGetPolygonStipple) numFailed++;
  vpvl2__ptrc_glGetString = (const GLubyte * (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glGetString");
  if(!vpvl2__ptrc_glGetString) numFailed++;
  vpvl2__ptrc_glGetTexEnvfv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLfloat *))IntGetProcAddress("glGetTexEnvfv");
  if(!vpvl2__ptrc_glGetTexEnvfv) numFailed++;
  vpvl2__ptrc_glGetTexEnviv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint *))IntGetProcAddress("glGetTexEnviv");
  if(!vpvl2__ptrc_glGetTexEnviv) numFailed++;
  vpvl2__ptrc_glGetTexGendv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLdouble *))IntGetProcAddress("glGetTexGendv");
  if(!vpvl2__ptrc_glGetTexGendv) numFailed++;
  vpvl2__ptrc_glGetTexGenfv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLfloat *))IntGetProcAddress("glGetTexGenfv");
  if(!vpvl2__ptrc_glGetTexGenfv) numFailed++;
  vpvl2__ptrc_glGetTexGeniv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint *))IntGetProcAddress("glGetTexGeniv");
  if(!vpvl2__ptrc_glGetTexGeniv) numFailed++;
  vpvl2__ptrc_glGetTexImage = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLenum, GLenum, GLvoid *))IntGetProcAddress("glGetTexImage");
  if(!vpvl2__ptrc_glGetTexImage) numFailed++;
  vpvl2__ptrc_glGetTexLevelParameterfv = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLenum, GLfloat *))IntGetProcAddress("glGetTexLevelParameterfv");
  if(!vpvl2__ptrc_glGetTexLevelParameterfv) numFailed++;
  vpvl2__ptrc_glGetTexLevelParameteriv = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLenum, GLint *))IntGetProcAddress("glGetTexLevelParameteriv");
  if(!vpvl2__ptrc_glGetTexLevelParameteriv) numFailed++;
  vpvl2__ptrc_glGetTexParameterfv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLfloat *))IntGetProcAddress("glGetTexParameterfv");
  if(!vpvl2__ptrc_glGetTexParameterfv) numFailed++;
  vpvl2__ptrc_glGetTexParameteriv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint *))IntGetProcAddress("glGetTexParameteriv");
  if(!vpvl2__ptrc_glGetTexParameteriv) numFailed++;
  vpvl2__ptrc_glHint = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum))IntGetProcAddress("glHint");
  if(!vpvl2__ptrc_glHint) numFailed++;
  vpvl2__ptrc_glIndexMask = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glIndexMask");
  if(!vpvl2__ptrc_glIndexMask) numFailed++;
  vpvl2__ptrc_glIndexd = (void (CODEGEN_FUNCPTR *)(GLdouble))IntGetProcAddress("glIndexd");
  if(!vpvl2__ptrc_glIndexd) numFailed++;
  vpvl2__ptrc_glIndexdv = (void (CODEGEN_FUNCPTR *)(const GLdouble *))IntGetProcAddress("glIndexdv");
  if(!vpvl2__ptrc_glIndexdv) numFailed++;
  vpvl2__ptrc_glIndexf = (void (CODEGEN_FUNCPTR *)(GLfloat))IntGetProcAddress("glIndexf");
  if(!vpvl2__ptrc_glIndexf) numFailed++;
  vpvl2__ptrc_glIndexfv = (void (CODEGEN_FUNCPTR *)(const GLfloat *))IntGetProcAddress("glIndexfv");
  if(!vpvl2__ptrc_glIndexfv) numFailed++;
  vpvl2__ptrc_glIndexi = (void (CODEGEN_FUNCPTR *)(GLint))IntGetProcAddress("glIndexi");
  if(!vpvl2__ptrc_glIndexi) numFailed++;
  vpvl2__ptrc_glIndexiv = (void (CODEGEN_FUNCPTR *)(const GLint *))IntGetProcAddress("glIndexiv");
  if(!vpvl2__ptrc_glIndexiv) numFailed++;
  vpvl2__ptrc_glIndexs = (void (CODEGEN_FUNCPTR *)(GLshort))IntGetProcAddress("glIndexs");
  if(!vpvl2__ptrc_glIndexs) numFailed++;
  vpvl2__ptrc_glIndexsv = (void (CODEGEN_FUNCPTR *)(const GLshort *))IntGetProcAddress("glIndexsv");
  if(!vpvl2__ptrc_glIndexsv) numFailed++;
  vpvl2__ptrc_glInitNames = (void (CODEGEN_FUNCPTR *)())IntGetProcAddress("glInitNames");
  if(!vpvl2__ptrc_glInitNames) numFailed++;
  vpvl2__ptrc_glIsEnabled = (GLboolean (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glIsEnabled");
  if(!vpvl2__ptrc_glIsEnabled) numFailed++;
  vpvl2__ptrc_glIsList = (GLboolean (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glIsList");
  if(!vpvl2__ptrc_glIsList) numFailed++;
  vpvl2__ptrc_glLightModelf = (void (CODEGEN_FUNCPTR *)(GLenum, GLfloat))IntGetProcAddress("glLightModelf");
  if(!vpvl2__ptrc_glLightModelf) numFailed++;
  vpvl2__ptrc_glLightModelfv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLfloat *))IntGetProcAddress("glLightModelfv");
  if(!vpvl2__ptrc_glLightModelfv) numFailed++;
  vpvl2__ptrc_glLightModeli = (void (CODEGEN_FUNCPTR *)(GLenum, GLint))IntGetProcAddress("glLightModeli");
  if(!vpvl2__ptrc_glLightModeli) numFailed++;
  vpvl2__ptrc_glLightModeliv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLint *))IntGetProcAddress("glLightModeliv");
  if(!vpvl2__ptrc_glLightModeliv) numFailed++;
  vpvl2__ptrc_glLightf = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLfloat))IntGetProcAddress("glLightf");
  if(!vpvl2__ptrc_glLightf) numFailed++;
  vpvl2__ptrc_glLightfv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, const GLfloat *))IntGetProcAddress("glLightfv");
  if(!vpvl2__ptrc_glLightfv) numFailed++;
  vpvl2__ptrc_glLighti = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint))IntGetProcAddress("glLighti");
  if(!vpvl2__ptrc_glLighti) numFailed++;
  vpvl2__ptrc_glLightiv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, const GLint *))IntGetProcAddress("glLightiv");
  if(!vpvl2__ptrc_glLightiv) numFailed++;
  vpvl2__ptrc_glLineStipple = (void (CODEGEN_FUNCPTR *)(GLint, GLushort))IntGetProcAddress("glLineStipple");
  if(!vpvl2__ptrc_glLineStipple) numFailed++;
  vpvl2__ptrc_glLineWidth = (void (CODEGEN_FUNCPTR *)(GLfloat))IntGetProcAddress("glLineWidth");
  if(!vpvl2__ptrc_glLineWidth) numFailed++;
  vpvl2__ptrc_glListBase = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glListBase");
  if(!vpvl2__ptrc_glListBase) numFailed++;
  vpvl2__ptrc_glLoadIdentity = (void (CODEGEN_FUNCPTR *)())IntGetProcAddress("glLoadIdentity");
  if(!vpvl2__ptrc_glLoadIdentity) numFailed++;
  vpvl2__ptrc_glLoadMatrixd = (void (CODEGEN_FUNCPTR *)(const GLdouble *))IntGetProcAddress("glLoadMatrixd");
  if(!vpvl2__ptrc_glLoadMatrixd) numFailed++;
  vpvl2__ptrc_glLoadMatrixf = (void (CODEGEN_FUNCPTR *)(const GLfloat *))IntGetProcAddress("glLoadMatrixf");
  if(!vpvl2__ptrc_glLoadMatrixf) numFailed++;
  vpvl2__ptrc_glLoadName = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glLoadName");
  if(!vpvl2__ptrc_glLoadName) numFailed++;
  vpvl2__ptrc_glLogicOp = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glLogicOp");
  if(!vpvl2__ptrc_glLogicOp) numFailed++;
  vpvl2__ptrc_glMap1d = (void (CODEGEN_FUNCPTR *)(GLenum, GLdouble, GLdouble, GLint, GLint, const GLdouble *))IntGetProcAddress("glMap1d");
  if(!vpvl2__ptrc_glMap1d) numFailed++;
  vpvl2__ptrc_glMap1f = (void (CODEGEN_FUNCPTR *)(GLenum, GLfloat, GLfloat, GLint, GLint, const GLfloat *))IntGetProcAddress("glMap1f");
  if(!vpvl2__ptrc_glMap1f) numFailed++;
  vpvl2__ptrc_glMap2d = (void (CODEGEN_FUNCPTR *)(GLenum, GLdouble, GLdouble, GLint, GLint, GLdouble, GLdouble, GLint, GLint, const GLdouble *))IntGetProcAddress("glMap2d");
  if(!vpvl2__ptrc_glMap2d) numFailed++;
  vpvl2__ptrc_glMap2f = (void (CODEGEN_FUNCPTR *)(GLenum, GLfloat, GLfloat, GLint, GLint, GLfloat, GLfloat, GLint, GLint, const GLfloat *))IntGetProcAddress("glMap2f");
  if(!vpvl2__ptrc_glMap2f) numFailed++;
  vpvl2__ptrc_glMapGrid1d = (void (CODEGEN_FUNCPTR *)(GLint, GLdouble, GLdouble))IntGetProcAddress("glMapGrid1d");
  if(!vpvl2__ptrc_glMapGrid1d) numFailed++;
  vpvl2__ptrc_glMapGrid1f = (void (CODEGEN_FUNCPTR *)(GLint, GLfloat, GLfloat))IntGetProcAddress("glMapGrid1f");
  if(!vpvl2__ptrc_glMapGrid1f) numFailed++;
  vpvl2__ptrc_glMapGrid2d = (void (CODEGEN_FUNCPTR *)(GLint, GLdouble, GLdouble, GLint, GLdouble, GLdouble))IntGetProcAddress("glMapGrid2d");
  if(!vpvl2__ptrc_glMapGrid2d) numFailed++;
  vpvl2__ptrc_glMapGrid2f = (void (CODEGEN_FUNCPTR *)(GLint, GLfloat, GLfloat, GLint, GLfloat, GLfloat))IntGetProcAddress("glMapGrid2f");
  if(!vpvl2__ptrc_glMapGrid2f) numFailed++;
  vpvl2__ptrc_glMaterialf = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLfloat))IntGetProcAddress("glMaterialf");
  if(!vpvl2__ptrc_glMaterialf) numFailed++;
  vpvl2__ptrc_glMaterialfv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, const GLfloat *))IntGetProcAddress("glMaterialfv");
  if(!vpvl2__ptrc_glMaterialfv) numFailed++;
  vpvl2__ptrc_glMateriali = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint))IntGetProcAddress("glMateriali");
  if(!vpvl2__ptrc_glMateriali) numFailed++;
  vpvl2__ptrc_glMaterialiv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, const GLint *))IntGetProcAddress("glMaterialiv");
  if(!vpvl2__ptrc_glMaterialiv) numFailed++;
  vpvl2__ptrc_glMatrixMode = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glMatrixMode");
  if(!vpvl2__ptrc_glMatrixMode) numFailed++;
  vpvl2__ptrc_glMultMatrixd = (void (CODEGEN_FUNCPTR *)(const GLdouble *))IntGetProcAddress("glMultMatrixd");
  if(!vpvl2__ptrc_glMultMatrixd) numFailed++;
  vpvl2__ptrc_glMultMatrixf = (void (CODEGEN_FUNCPTR *)(const GLfloat *))IntGetProcAddress("glMultMatrixf");
  if(!vpvl2__ptrc_glMultMatrixf) numFailed++;
  vpvl2__ptrc_glNewList = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum))IntGetProcAddress("glNewList");
  if(!vpvl2__ptrc_glNewList) numFailed++;
  vpvl2__ptrc_glNormal3b = (void (CODEGEN_FUNCPTR *)(GLbyte, GLbyte, GLbyte))IntGetProcAddress("glNormal3b");
  if(!vpvl2__ptrc_glNormal3b) numFailed++;
  vpvl2__ptrc_glNormal3bv = (void (CODEGEN_FUNCPTR *)(const GLbyte *))IntGetProcAddress("glNormal3bv");
  if(!vpvl2__ptrc_glNormal3bv) numFailed++;
  vpvl2__ptrc_glNormal3d = (void (CODEGEN_FUNCPTR *)(GLdouble, GLdouble, GLdouble))IntGetProcAddress("glNormal3d");
  if(!vpvl2__ptrc_glNormal3d) numFailed++;
  vpvl2__ptrc_glNormal3dv = (void (CODEGEN_FUNCPTR *)(const GLdouble *))IntGetProcAddress("glNormal3dv");
  if(!vpvl2__ptrc_glNormal3dv) numFailed++;
  vpvl2__ptrc_glNormal3f = (void (CODEGEN_FUNCPTR *)(GLfloat, GLfloat, GLfloat))IntGetProcAddress("glNormal3f");
  if(!vpvl2__ptrc_glNormal3f) numFailed++;
  vpvl2__ptrc_glNormal3fv = (void (CODEGEN_FUNCPTR *)(const GLfloat *))IntGetProcAddress("glNormal3fv");
  if(!vpvl2__ptrc_glNormal3fv) numFailed++;
  vpvl2__ptrc_glNormal3i = (void (CODEGEN_FUNCPTR *)(GLint, GLint, GLint))IntGetProcAddress("glNormal3i");
  if(!vpvl2__ptrc_glNormal3i) numFailed++;
  vpvl2__ptrc_glNormal3iv = (void (CODEGEN_FUNCPTR *)(const GLint *))IntGetProcAddress("glNormal3iv");
  if(!vpvl2__ptrc_glNormal3iv) numFailed++;
  vpvl2__ptrc_glNormal3s = (void (CODEGEN_FUNCPTR *)(GLshort, GLshort, GLshort))IntGetProcAddress("glNormal3s");
  if(!vpvl2__ptrc_glNormal3s) numFailed++;
  vpvl2__ptrc_glNormal3sv = (void (CODEGEN_FUNCPTR *)(const GLshort *))IntGetProcAddress("glNormal3sv");
  if(!vpvl2__ptrc_glNormal3sv) numFailed++;
  vpvl2__ptrc_glOrtho = (void (CODEGEN_FUNCPTR *)(GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble))IntGetProcAddress("glOrtho");
  if(!vpvl2__ptrc_glOrtho) numFailed++;
  vpvl2__ptrc_glPassThrough = (void (CODEGEN_FUNCPTR *)(GLfloat))IntGetProcAddress("glPassThrough");
  if(!vpvl2__ptrc_glPassThrough) numFailed++;
  vpvl2__ptrc_glPixelMapfv = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, const GLfloat *))IntGetProcAddress("glPixelMapfv");
  if(!vpvl2__ptrc_glPixelMapfv) numFailed++;
  vpvl2__ptrc_glPixelMapuiv = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, const GLuint *))IntGetProcAddress("glPixelMapuiv");
  if(!vpvl2__ptrc_glPixelMapuiv) numFailed++;
  vpvl2__ptrc_glPixelMapusv = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, const GLushort *))IntGetProcAddress("glPixelMapusv");
  if(!vpvl2__ptrc_glPixelMapusv) numFailed++;
  vpvl2__ptrc_glPixelStoref = (void (CODEGEN_FUNCPTR *)(GLenum, GLfloat))IntGetProcAddress("glPixelStoref");
  if(!vpvl2__ptrc_glPixelStoref) numFailed++;
  vpvl2__ptrc_glPixelStorei = (void (CODEGEN_FUNCPTR *)(GLenum, GLint))IntGetProcAddress("glPixelStorei");
  if(!vpvl2__ptrc_glPixelStorei) numFailed++;
  vpvl2__ptrc_glPixelTransferf = (void (CODEGEN_FUNCPTR *)(GLenum, GLfloat))IntGetProcAddress("glPixelTransferf");
  if(!vpvl2__ptrc_glPixelTransferf) numFailed++;
  vpvl2__ptrc_glPixelTransferi = (void (CODEGEN_FUNCPTR *)(GLenum, GLint))IntGetProcAddress("glPixelTransferi");
  if(!vpvl2__ptrc_glPixelTransferi) numFailed++;
  vpvl2__ptrc_glPixelZoom = (void (CODEGEN_FUNCPTR *)(GLfloat, GLfloat))IntGetProcAddress("glPixelZoom");
  if(!vpvl2__ptrc_glPixelZoom) numFailed++;
  vpvl2__ptrc_glPointSize = (void (CODEGEN_FUNCPTR *)(GLfloat))IntGetProcAddress("glPointSize");
  if(!vpvl2__ptrc_glPointSize) numFailed++;
  vpvl2__ptrc_glPolygonMode = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum))IntGetProcAddress("glPolygonMode");
  if(!vpvl2__ptrc_glPolygonMode) numFailed++;
  vpvl2__ptrc_glPolygonStipple = (void (CODEGEN_FUNCPTR *)(const GLubyte *))IntGetProcAddress("glPolygonStipple");
  if(!vpvl2__ptrc_glPolygonStipple) numFailed++;
  vpvl2__ptrc_glPopAttrib = (void (CODEGEN_FUNCPTR *)())IntGetProcAddress("glPopAttrib");
  if(!vpvl2__ptrc_glPopAttrib) numFailed++;
  vpvl2__ptrc_glPopMatrix = (void (CODEGEN_FUNCPTR *)())IntGetProcAddress("glPopMatrix");
  if(!vpvl2__ptrc_glPopMatrix) numFailed++;
  vpvl2__ptrc_glPopName = (void (CODEGEN_FUNCPTR *)())IntGetProcAddress("glPopName");
  if(!vpvl2__ptrc_glPopName) numFailed++;
  vpvl2__ptrc_glPushAttrib = (void (CODEGEN_FUNCPTR *)(GLbitfield))IntGetProcAddress("glPushAttrib");
  if(!vpvl2__ptrc_glPushAttrib) numFailed++;
  vpvl2__ptrc_glPushMatrix = (void (CODEGEN_FUNCPTR *)())IntGetProcAddress("glPushMatrix");
  if(!vpvl2__ptrc_glPushMatrix) numFailed++;
  vpvl2__ptrc_glPushName = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glPushName");
  if(!vpvl2__ptrc_glPushName) numFailed++;
  vpvl2__ptrc_glRasterPos2d = (void (CODEGEN_FUNCPTR *)(GLdouble, GLdouble))IntGetProcAddress("glRasterPos2d");
  if(!vpvl2__ptrc_glRasterPos2d) numFailed++;
  vpvl2__ptrc_glRasterPos2dv = (void (CODEGEN_FUNCPTR *)(const GLdouble *))IntGetProcAddress("glRasterPos2dv");
  if(!vpvl2__ptrc_glRasterPos2dv) numFailed++;
  vpvl2__ptrc_glRasterPos2f = (void (CODEGEN_FUNCPTR *)(GLfloat, GLfloat))IntGetProcAddress("glRasterPos2f");
  if(!vpvl2__ptrc_glRasterPos2f) numFailed++;
  vpvl2__ptrc_glRasterPos2fv = (void (CODEGEN_FUNCPTR *)(const GLfloat *))IntGetProcAddress("glRasterPos2fv");
  if(!vpvl2__ptrc_glRasterPos2fv) numFailed++;
  vpvl2__ptrc_glRasterPos2i = (void (CODEGEN_FUNCPTR *)(GLint, GLint))IntGetProcAddress("glRasterPos2i");
  if(!vpvl2__ptrc_glRasterPos2i) numFailed++;
  vpvl2__ptrc_glRasterPos2iv = (void (CODEGEN_FUNCPTR *)(const GLint *))IntGetProcAddress("glRasterPos2iv");
  if(!vpvl2__ptrc_glRasterPos2iv) numFailed++;
  vpvl2__ptrc_glRasterPos2s = (void (CODEGEN_FUNCPTR *)(GLshort, GLshort))IntGetProcAddress("glRasterPos2s");
  if(!vpvl2__ptrc_glRasterPos2s) numFailed++;
  vpvl2__ptrc_glRasterPos2sv = (void (CODEGEN_FUNCPTR *)(const GLshort *))IntGetProcAddress("glRasterPos2sv");
  if(!vpvl2__ptrc_glRasterPos2sv) numFailed++;
  vpvl2__ptrc_glRasterPos3d = (void (CODEGEN_FUNCPTR *)(GLdouble, GLdouble, GLdouble))IntGetProcAddress("glRasterPos3d");
  if(!vpvl2__ptrc_glRasterPos3d) numFailed++;
  vpvl2__ptrc_glRasterPos3dv = (void (CODEGEN_FUNCPTR *)(const GLdouble *))IntGetProcAddress("glRasterPos3dv");
  if(!vpvl2__ptrc_glRasterPos3dv) numFailed++;
  vpvl2__ptrc_glRasterPos3f = (void (CODEGEN_FUNCPTR *)(GLfloat, GLfloat, GLfloat))IntGetProcAddress("glRasterPos3f");
  if(!vpvl2__ptrc_glRasterPos3f) numFailed++;
  vpvl2__ptrc_glRasterPos3fv = (void (CODEGEN_FUNCPTR *)(const GLfloat *))IntGetProcAddress("glRasterPos3fv");
  if(!vpvl2__ptrc_glRasterPos3fv) numFailed++;
  vpvl2__ptrc_glRasterPos3i = (void (CODEGEN_FUNCPTR *)(GLint, GLint, GLint))IntGetProcAddress("glRasterPos3i");
  if(!vpvl2__ptrc_glRasterPos3i) numFailed++;
  vpvl2__ptrc_glRasterPos3iv = (void (CODEGEN_FUNCPTR *)(const GLint *))IntGetProcAddress("glRasterPos3iv");
  if(!vpvl2__ptrc_glRasterPos3iv) numFailed++;
  vpvl2__ptrc_glRasterPos3s = (void (CODEGEN_FUNCPTR *)(GLshort, GLshort, GLshort))IntGetProcAddress("glRasterPos3s");
  if(!vpvl2__ptrc_glRasterPos3s) numFailed++;
  vpvl2__ptrc_glRasterPos3sv = (void (CODEGEN_FUNCPTR *)(const GLshort *))IntGetProcAddress("glRasterPos3sv");
  if(!vpvl2__ptrc_glRasterPos3sv) numFailed++;
  vpvl2__ptrc_glRasterPos4d = (void (CODEGEN_FUNCPTR *)(GLdouble, GLdouble, GLdouble, GLdouble))IntGetProcAddress("glRasterPos4d");
  if(!vpvl2__ptrc_glRasterPos4d) numFailed++;
  vpvl2__ptrc_glRasterPos4dv = (void (CODEGEN_FUNCPTR *)(const GLdouble *))IntGetProcAddress("glRasterPos4dv");
  if(!vpvl2__ptrc_glRasterPos4dv) numFailed++;
  vpvl2__ptrc_glRasterPos4f = (void (CODEGEN_FUNCPTR *)(GLfloat, GLfloat, GLfloat, GLfloat))IntGetProcAddress("glRasterPos4f");
  if(!vpvl2__ptrc_glRasterPos4f) numFailed++;
  vpvl2__ptrc_glRasterPos4fv = (void (CODEGEN_FUNCPTR *)(const GLfloat *))IntGetProcAddress("glRasterPos4fv");
  if(!vpvl2__ptrc_glRasterPos4fv) numFailed++;
  vpvl2__ptrc_glRasterPos4i = (void (CODEGEN_FUNCPTR *)(GLint, GLint, GLint, GLint))IntGetProcAddress("glRasterPos4i");
  if(!vpvl2__ptrc_glRasterPos4i) numFailed++;
  vpvl2__ptrc_glRasterPos4iv = (void (CODEGEN_FUNCPTR *)(const GLint *))IntGetProcAddress("glRasterPos4iv");
  if(!vpvl2__ptrc_glRasterPos4iv) numFailed++;
  vpvl2__ptrc_glRasterPos4s = (void (CODEGEN_FUNCPTR *)(GLshort, GLshort, GLshort, GLshort))IntGetProcAddress("glRasterPos4s");
  if(!vpvl2__ptrc_glRasterPos4s) numFailed++;
  vpvl2__ptrc_glRasterPos4sv = (void (CODEGEN_FUNCPTR *)(const GLshort *))IntGetProcAddress("glRasterPos4sv");
  if(!vpvl2__ptrc_glRasterPos4sv) numFailed++;
  vpvl2__ptrc_glReadBuffer = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glReadBuffer");
  if(!vpvl2__ptrc_glReadBuffer) numFailed++;
  vpvl2__ptrc_glReadPixels = (void (CODEGEN_FUNCPTR *)(GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, GLvoid *))IntGetProcAddress("glReadPixels");
  if(!vpvl2__ptrc_glReadPixels) numFailed++;
  vpvl2__ptrc_glRectd = (void (CODEGEN_FUNCPTR *)(GLdouble, GLdouble, GLdouble, GLdouble))IntGetProcAddress("glRectd");
  if(!vpvl2__ptrc_glRectd) numFailed++;
  vpvl2__ptrc_glRectdv = (void (CODEGEN_FUNCPTR *)(const GLdouble *, const GLdouble *))IntGetProcAddress("glRectdv");
  if(!vpvl2__ptrc_glRectdv) numFailed++;
  vpvl2__ptrc_glRectf = (void (CODEGEN_FUNCPTR *)(GLfloat, GLfloat, GLfloat, GLfloat))IntGetProcAddress("glRectf");
  if(!vpvl2__ptrc_glRectf) numFailed++;
  vpvl2__ptrc_glRectfv = (void (CODEGEN_FUNCPTR *)(const GLfloat *, const GLfloat *))IntGetProcAddress("glRectfv");
  if(!vpvl2__ptrc_glRectfv) numFailed++;
  vpvl2__ptrc_glRecti = (void (CODEGEN_FUNCPTR *)(GLint, GLint, GLint, GLint))IntGetProcAddress("glRecti");
  if(!vpvl2__ptrc_glRecti) numFailed++;
  vpvl2__ptrc_glRectiv = (void (CODEGEN_FUNCPTR *)(const GLint *, const GLint *))IntGetProcAddress("glRectiv");
  if(!vpvl2__ptrc_glRectiv) numFailed++;
  vpvl2__ptrc_glRects = (void (CODEGEN_FUNCPTR *)(GLshort, GLshort, GLshort, GLshort))IntGetProcAddress("glRects");
  if(!vpvl2__ptrc_glRects) numFailed++;
  vpvl2__ptrc_glRectsv = (void (CODEGEN_FUNCPTR *)(const GLshort *, const GLshort *))IntGetProcAddress("glRectsv");
  if(!vpvl2__ptrc_glRectsv) numFailed++;
  vpvl2__ptrc_glRenderMode = (GLint (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glRenderMode");
  if(!vpvl2__ptrc_glRenderMode) numFailed++;
  vpvl2__ptrc_glRotated = (void (CODEGEN_FUNCPTR *)(GLdouble, GLdouble, GLdouble, GLdouble))IntGetProcAddress("glRotated");
  if(!vpvl2__ptrc_glRotated) numFailed++;
  vpvl2__ptrc_glRotatef = (void (CODEGEN_FUNCPTR *)(GLfloat, GLfloat, GLfloat, GLfloat))IntGetProcAddress("glRotatef");
  if(!vpvl2__ptrc_glRotatef) numFailed++;
  vpvl2__ptrc_glScaled = (void (CODEGEN_FUNCPTR *)(GLdouble, GLdouble, GLdouble))IntGetProcAddress("glScaled");
  if(!vpvl2__ptrc_glScaled) numFailed++;
  vpvl2__ptrc_glScalef = (void (CODEGEN_FUNCPTR *)(GLfloat, GLfloat, GLfloat))IntGetProcAddress("glScalef");
  if(!vpvl2__ptrc_glScalef) numFailed++;
  vpvl2__ptrc_glScissor = (void (CODEGEN_FUNCPTR *)(GLint, GLint, GLsizei, GLsizei))IntGetProcAddress("glScissor");
  if(!vpvl2__ptrc_glScissor) numFailed++;
  vpvl2__ptrc_glSelectBuffer = (void (CODEGEN_FUNCPTR *)(GLsizei, GLuint *))IntGetProcAddress("glSelectBuffer");
  if(!vpvl2__ptrc_glSelectBuffer) numFailed++;
  vpvl2__ptrc_glShadeModel = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glShadeModel");
  if(!vpvl2__ptrc_glShadeModel) numFailed++;
  vpvl2__ptrc_glStencilFunc = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLuint))IntGetProcAddress("glStencilFunc");
  if(!vpvl2__ptrc_glStencilFunc) numFailed++;
  vpvl2__ptrc_glStencilMask = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glStencilMask");
  if(!vpvl2__ptrc_glStencilMask) numFailed++;
  vpvl2__ptrc_glStencilOp = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum))IntGetProcAddress("glStencilOp");
  if(!vpvl2__ptrc_glStencilOp) numFailed++;
  vpvl2__ptrc_glTexCoord1d = (void (CODEGEN_FUNCPTR *)(GLdouble))IntGetProcAddress("glTexCoord1d");
  if(!vpvl2__ptrc_glTexCoord1d) numFailed++;
  vpvl2__ptrc_glTexCoord1dv = (void (CODEGEN_FUNCPTR *)(const GLdouble *))IntGetProcAddress("glTexCoord1dv");
  if(!vpvl2__ptrc_glTexCoord1dv) numFailed++;
  vpvl2__ptrc_glTexCoord1f = (void (CODEGEN_FUNCPTR *)(GLfloat))IntGetProcAddress("glTexCoord1f");
  if(!vpvl2__ptrc_glTexCoord1f) numFailed++;
  vpvl2__ptrc_glTexCoord1fv = (void (CODEGEN_FUNCPTR *)(const GLfloat *))IntGetProcAddress("glTexCoord1fv");
  if(!vpvl2__ptrc_glTexCoord1fv) numFailed++;
  vpvl2__ptrc_glTexCoord1i = (void (CODEGEN_FUNCPTR *)(GLint))IntGetProcAddress("glTexCoord1i");
  if(!vpvl2__ptrc_glTexCoord1i) numFailed++;
  vpvl2__ptrc_glTexCoord1iv = (void (CODEGEN_FUNCPTR *)(const GLint *))IntGetProcAddress("glTexCoord1iv");
  if(!vpvl2__ptrc_glTexCoord1iv) numFailed++;
  vpvl2__ptrc_glTexCoord1s = (void (CODEGEN_FUNCPTR *)(GLshort))IntGetProcAddress("glTexCoord1s");
  if(!vpvl2__ptrc_glTexCoord1s) numFailed++;
  vpvl2__ptrc_glTexCoord1sv = (void (CODEGEN_FUNCPTR *)(const GLshort *))IntGetProcAddress("glTexCoord1sv");
  if(!vpvl2__ptrc_glTexCoord1sv) numFailed++;
  vpvl2__ptrc_glTexCoord2d = (void (CODEGEN_FUNCPTR *)(GLdouble, GLdouble))IntGetProcAddress("glTexCoord2d");
  if(!vpvl2__ptrc_glTexCoord2d) numFailed++;
  vpvl2__ptrc_glTexCoord2dv = (void (CODEGEN_FUNCPTR *)(const GLdouble *))IntGetProcAddress("glTexCoord2dv");
  if(!vpvl2__ptrc_glTexCoord2dv) numFailed++;
  vpvl2__ptrc_glTexCoord2f = (void (CODEGEN_FUNCPTR *)(GLfloat, GLfloat))IntGetProcAddress("glTexCoord2f");
  if(!vpvl2__ptrc_glTexCoord2f) numFailed++;
  vpvl2__ptrc_glTexCoord2fv = (void (CODEGEN_FUNCPTR *)(const GLfloat *))IntGetProcAddress("glTexCoord2fv");
  if(!vpvl2__ptrc_glTexCoord2fv) numFailed++;
  vpvl2__ptrc_glTexCoord2i = (void (CODEGEN_FUNCPTR *)(GLint, GLint))IntGetProcAddress("glTexCoord2i");
  if(!vpvl2__ptrc_glTexCoord2i) numFailed++;
  vpvl2__ptrc_glTexCoord2iv = (void (CODEGEN_FUNCPTR *)(const GLint *))IntGetProcAddress("glTexCoord2iv");
  if(!vpvl2__ptrc_glTexCoord2iv) numFailed++;
  vpvl2__ptrc_glTexCoord2s = (void (CODEGEN_FUNCPTR *)(GLshort, GLshort))IntGetProcAddress("glTexCoord2s");
  if(!vpvl2__ptrc_glTexCoord2s) numFailed++;
  vpvl2__ptrc_glTexCoord2sv = (void (CODEGEN_FUNCPTR *)(const GLshort *))IntGetProcAddress("glTexCoord2sv");
  if(!vpvl2__ptrc_glTexCoord2sv) numFailed++;
  vpvl2__ptrc_glTexCoord3d = (void (CODEGEN_FUNCPTR *)(GLdouble, GLdouble, GLdouble))IntGetProcAddress("glTexCoord3d");
  if(!vpvl2__ptrc_glTexCoord3d) numFailed++;
  vpvl2__ptrc_glTexCoord3dv = (void (CODEGEN_FUNCPTR *)(const GLdouble *))IntGetProcAddress("glTexCoord3dv");
  if(!vpvl2__ptrc_glTexCoord3dv) numFailed++;
  vpvl2__ptrc_glTexCoord3f = (void (CODEGEN_FUNCPTR *)(GLfloat, GLfloat, GLfloat))IntGetProcAddress("glTexCoord3f");
  if(!vpvl2__ptrc_glTexCoord3f) numFailed++;
  vpvl2__ptrc_glTexCoord3fv = (void (CODEGEN_FUNCPTR *)(const GLfloat *))IntGetProcAddress("glTexCoord3fv");
  if(!vpvl2__ptrc_glTexCoord3fv) numFailed++;
  vpvl2__ptrc_glTexCoord3i = (void (CODEGEN_FUNCPTR *)(GLint, GLint, GLint))IntGetProcAddress("glTexCoord3i");
  if(!vpvl2__ptrc_glTexCoord3i) numFailed++;
  vpvl2__ptrc_glTexCoord3iv = (void (CODEGEN_FUNCPTR *)(const GLint *))IntGetProcAddress("glTexCoord3iv");
  if(!vpvl2__ptrc_glTexCoord3iv) numFailed++;
  vpvl2__ptrc_glTexCoord3s = (void (CODEGEN_FUNCPTR *)(GLshort, GLshort, GLshort))IntGetProcAddress("glTexCoord3s");
  if(!vpvl2__ptrc_glTexCoord3s) numFailed++;
  vpvl2__ptrc_glTexCoord3sv = (void (CODEGEN_FUNCPTR *)(const GLshort *))IntGetProcAddress("glTexCoord3sv");
  if(!vpvl2__ptrc_glTexCoord3sv) numFailed++;
  vpvl2__ptrc_glTexCoord4d = (void (CODEGEN_FUNCPTR *)(GLdouble, GLdouble, GLdouble, GLdouble))IntGetProcAddress("glTexCoord4d");
  if(!vpvl2__ptrc_glTexCoord4d) numFailed++;
  vpvl2__ptrc_glTexCoord4dv = (void (CODEGEN_FUNCPTR *)(const GLdouble *))IntGetProcAddress("glTexCoord4dv");
  if(!vpvl2__ptrc_glTexCoord4dv) numFailed++;
  vpvl2__ptrc_glTexCoord4f = (void (CODEGEN_FUNCPTR *)(GLfloat, GLfloat, GLfloat, GLfloat))IntGetProcAddress("glTexCoord4f");
  if(!vpvl2__ptrc_glTexCoord4f) numFailed++;
  vpvl2__ptrc_glTexCoord4fv = (void (CODEGEN_FUNCPTR *)(const GLfloat *))IntGetProcAddress("glTexCoord4fv");
  if(!vpvl2__ptrc_glTexCoord4fv) numFailed++;
  vpvl2__ptrc_glTexCoord4i = (void (CODEGEN_FUNCPTR *)(GLint, GLint, GLint, GLint))IntGetProcAddress("glTexCoord4i");
  if(!vpvl2__ptrc_glTexCoord4i) numFailed++;
  vpvl2__ptrc_glTexCoord4iv = (void (CODEGEN_FUNCPTR *)(const GLint *))IntGetProcAddress("glTexCoord4iv");
  if(!vpvl2__ptrc_glTexCoord4iv) numFailed++;
  vpvl2__ptrc_glTexCoord4s = (void (CODEGEN_FUNCPTR *)(GLshort, GLshort, GLshort, GLshort))IntGetProcAddress("glTexCoord4s");
  if(!vpvl2__ptrc_glTexCoord4s) numFailed++;
  vpvl2__ptrc_glTexCoord4sv = (void (CODEGEN_FUNCPTR *)(const GLshort *))IntGetProcAddress("glTexCoord4sv");
  if(!vpvl2__ptrc_glTexCoord4sv) numFailed++;
  vpvl2__ptrc_glTexEnvf = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLfloat))IntGetProcAddress("glTexEnvf");
  if(!vpvl2__ptrc_glTexEnvf) numFailed++;
  vpvl2__ptrc_glTexEnvfv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, const GLfloat *))IntGetProcAddress("glTexEnvfv");
  if(!vpvl2__ptrc_glTexEnvfv) numFailed++;
  vpvl2__ptrc_glTexEnvi = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint))IntGetProcAddress("glTexEnvi");
  if(!vpvl2__ptrc_glTexEnvi) numFailed++;
  vpvl2__ptrc_glTexEnviv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, const GLint *))IntGetProcAddress("glTexEnviv");
  if(!vpvl2__ptrc_glTexEnviv) numFailed++;
  vpvl2__ptrc_glTexGend = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLdouble))IntGetProcAddress("glTexGend");
  if(!vpvl2__ptrc_glTexGend) numFailed++;
  vpvl2__ptrc_glTexGendv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, const GLdouble *))IntGetProcAddress("glTexGendv");
  if(!vpvl2__ptrc_glTexGendv) numFailed++;
  vpvl2__ptrc_glTexGenf = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLfloat))IntGetProcAddress("glTexGenf");
  if(!vpvl2__ptrc_glTexGenf) numFailed++;
  vpvl2__ptrc_glTexGenfv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, const GLfloat *))IntGetProcAddress("glTexGenfv");
  if(!vpvl2__ptrc_glTexGenfv) numFailed++;
  vpvl2__ptrc_glTexGeni = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint))IntGetProcAddress("glTexGeni");
  if(!vpvl2__ptrc_glTexGeni) numFailed++;
  vpvl2__ptrc_glTexGeniv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, const GLint *))IntGetProcAddress("glTexGeniv");
  if(!vpvl2__ptrc_glTexGeniv) numFailed++;
  vpvl2__ptrc_glTexImage1D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLsizei, GLint, GLenum, GLenum, const GLvoid *))IntGetProcAddress("glTexImage1D");
  if(!vpvl2__ptrc_glTexImage1D) numFailed++;
  vpvl2__ptrc_glTexImage2D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *))IntGetProcAddress("glTexImage2D");
  if(!vpvl2__ptrc_glTexImage2D) numFailed++;
  vpvl2__ptrc_glTexParameterf = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLfloat))IntGetProcAddress("glTexParameterf");
  if(!vpvl2__ptrc_glTexParameterf) numFailed++;
  vpvl2__ptrc_glTexParameterfv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, const GLfloat *))IntGetProcAddress("glTexParameterfv");
  if(!vpvl2__ptrc_glTexParameterfv) numFailed++;
  vpvl2__ptrc_glTexParameteri = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint))IntGetProcAddress("glTexParameteri");
  if(!vpvl2__ptrc_glTexParameteri) numFailed++;
  vpvl2__ptrc_glTexParameteriv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, const GLint *))IntGetProcAddress("glTexParameteriv");
  if(!vpvl2__ptrc_glTexParameteriv) numFailed++;
  vpvl2__ptrc_glTranslated = (void (CODEGEN_FUNCPTR *)(GLdouble, GLdouble, GLdouble))IntGetProcAddress("glTranslated");
  if(!vpvl2__ptrc_glTranslated) numFailed++;
  vpvl2__ptrc_glTranslatef = (void (CODEGEN_FUNCPTR *)(GLfloat, GLfloat, GLfloat))IntGetProcAddress("glTranslatef");
  if(!vpvl2__ptrc_glTranslatef) numFailed++;
  vpvl2__ptrc_glVertex2d = (void (CODEGEN_FUNCPTR *)(GLdouble, GLdouble))IntGetProcAddress("glVertex2d");
  if(!vpvl2__ptrc_glVertex2d) numFailed++;
  vpvl2__ptrc_glVertex2dv = (void (CODEGEN_FUNCPTR *)(const GLdouble *))IntGetProcAddress("glVertex2dv");
  if(!vpvl2__ptrc_glVertex2dv) numFailed++;
  vpvl2__ptrc_glVertex2f = (void (CODEGEN_FUNCPTR *)(GLfloat, GLfloat))IntGetProcAddress("glVertex2f");
  if(!vpvl2__ptrc_glVertex2f) numFailed++;
  vpvl2__ptrc_glVertex2fv = (void (CODEGEN_FUNCPTR *)(const GLfloat *))IntGetProcAddress("glVertex2fv");
  if(!vpvl2__ptrc_glVertex2fv) numFailed++;
  vpvl2__ptrc_glVertex2i = (void (CODEGEN_FUNCPTR *)(GLint, GLint))IntGetProcAddress("glVertex2i");
  if(!vpvl2__ptrc_glVertex2i) numFailed++;
  vpvl2__ptrc_glVertex2iv = (void (CODEGEN_FUNCPTR *)(const GLint *))IntGetProcAddress("glVertex2iv");
  if(!vpvl2__ptrc_glVertex2iv) numFailed++;
  vpvl2__ptrc_glVertex2s = (void (CODEGEN_FUNCPTR *)(GLshort, GLshort))IntGetProcAddress("glVertex2s");
  if(!vpvl2__ptrc_glVertex2s) numFailed++;
  vpvl2__ptrc_glVertex2sv = (void (CODEGEN_FUNCPTR *)(const GLshort *))IntGetProcAddress("glVertex2sv");
  if(!vpvl2__ptrc_glVertex2sv) numFailed++;
  vpvl2__ptrc_glVertex3d = (void (CODEGEN_FUNCPTR *)(GLdouble, GLdouble, GLdouble))IntGetProcAddress("glVertex3d");
  if(!vpvl2__ptrc_glVertex3d) numFailed++;
  vpvl2__ptrc_glVertex3dv = (void (CODEGEN_FUNCPTR *)(const GLdouble *))IntGetProcAddress("glVertex3dv");
  if(!vpvl2__ptrc_glVertex3dv) numFailed++;
  vpvl2__ptrc_glVertex3f = (void (CODEGEN_FUNCPTR *)(GLfloat, GLfloat, GLfloat))IntGetProcAddress("glVertex3f");
  if(!vpvl2__ptrc_glVertex3f) numFailed++;
  vpvl2__ptrc_glVertex3fv = (void (CODEGEN_FUNCPTR *)(const GLfloat *))IntGetProcAddress("glVertex3fv");
  if(!vpvl2__ptrc_glVertex3fv) numFailed++;
  vpvl2__ptrc_glVertex3i = (void (CODEGEN_FUNCPTR *)(GLint, GLint, GLint))IntGetProcAddress("glVertex3i");
  if(!vpvl2__ptrc_glVertex3i) numFailed++;
  vpvl2__ptrc_glVertex3iv = (void (CODEGEN_FUNCPTR *)(const GLint *))IntGetProcAddress("glVertex3iv");
  if(!vpvl2__ptrc_glVertex3iv) numFailed++;
  vpvl2__ptrc_glVertex3s = (void (CODEGEN_FUNCPTR *)(GLshort, GLshort, GLshort))IntGetProcAddress("glVertex3s");
  if(!vpvl2__ptrc_glVertex3s) numFailed++;
  vpvl2__ptrc_glVertex3sv = (void (CODEGEN_FUNCPTR *)(const GLshort *))IntGetProcAddress("glVertex3sv");
  if(!vpvl2__ptrc_glVertex3sv) numFailed++;
  vpvl2__ptrc_glVertex4d = (void (CODEGEN_FUNCPTR *)(GLdouble, GLdouble, GLdouble, GLdouble))IntGetProcAddress("glVertex4d");
  if(!vpvl2__ptrc_glVertex4d) numFailed++;
  vpvl2__ptrc_glVertex4dv = (void (CODEGEN_FUNCPTR *)(const GLdouble *))IntGetProcAddress("glVertex4dv");
  if(!vpvl2__ptrc_glVertex4dv) numFailed++;
  vpvl2__ptrc_glVertex4f = (void (CODEGEN_FUNCPTR *)(GLfloat, GLfloat, GLfloat, GLfloat))IntGetProcAddress("glVertex4f");
  if(!vpvl2__ptrc_glVertex4f) numFailed++;
  vpvl2__ptrc_glVertex4fv = (void (CODEGEN_FUNCPTR *)(const GLfloat *))IntGetProcAddress("glVertex4fv");
  if(!vpvl2__ptrc_glVertex4fv) numFailed++;
  vpvl2__ptrc_glVertex4i = (void (CODEGEN_FUNCPTR *)(GLint, GLint, GLint, GLint))IntGetProcAddress("glVertex4i");
  if(!vpvl2__ptrc_glVertex4i) numFailed++;
  vpvl2__ptrc_glVertex4iv = (void (CODEGEN_FUNCPTR *)(const GLint *))IntGetProcAddress("glVertex4iv");
  if(!vpvl2__ptrc_glVertex4iv) numFailed++;
  vpvl2__ptrc_glVertex4s = (void (CODEGEN_FUNCPTR *)(GLshort, GLshort, GLshort, GLshort))IntGetProcAddress("glVertex4s");
  if(!vpvl2__ptrc_glVertex4s) numFailed++;
  vpvl2__ptrc_glVertex4sv = (void (CODEGEN_FUNCPTR *)(const GLshort *))IntGetProcAddress("glVertex4sv");
  if(!vpvl2__ptrc_glVertex4sv) numFailed++;
  vpvl2__ptrc_glViewport = (void (CODEGEN_FUNCPTR *)(GLint, GLint, GLsizei, GLsizei))IntGetProcAddress("glViewport");
  if(!vpvl2__ptrc_glViewport) numFailed++;
  vpvl2__ptrc_glAreTexturesResident = (GLboolean (CODEGEN_FUNCPTR *)(GLsizei, const GLuint *, GLboolean *))IntGetProcAddress("glAreTexturesResident");
  if(!vpvl2__ptrc_glAreTexturesResident) numFailed++;
  vpvl2__ptrc_glArrayElement = (void (CODEGEN_FUNCPTR *)(GLint))IntGetProcAddress("glArrayElement");
  if(!vpvl2__ptrc_glArrayElement) numFailed++;
  vpvl2__ptrc_glBindTexture = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glBindTexture");
  if(!vpvl2__ptrc_glBindTexture) numFailed++;
  vpvl2__ptrc_glColorPointer = (void (CODEGEN_FUNCPTR *)(GLint, GLenum, GLsizei, const GLvoid *))IntGetProcAddress("glColorPointer");
  if(!vpvl2__ptrc_glColorPointer) numFailed++;
  vpvl2__ptrc_glCopyTexImage1D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLint))IntGetProcAddress("glCopyTexImage1D");
  if(!vpvl2__ptrc_glCopyTexImage1D) numFailed++;
  vpvl2__ptrc_glCopyTexImage2D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLenum, GLint, GLint, GLsizei, GLsizei, GLint))IntGetProcAddress("glCopyTexImage2D");
  if(!vpvl2__ptrc_glCopyTexImage2D) numFailed++;
  vpvl2__ptrc_glCopyTexSubImage1D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLint, GLint, GLsizei))IntGetProcAddress("glCopyTexSubImage1D");
  if(!vpvl2__ptrc_glCopyTexSubImage1D) numFailed++;
  vpvl2__ptrc_glCopyTexSubImage2D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei))IntGetProcAddress("glCopyTexSubImage2D");
  if(!vpvl2__ptrc_glCopyTexSubImage2D) numFailed++;
  vpvl2__ptrc_glDeleteTextures = (void (CODEGEN_FUNCPTR *)(GLsizei, const GLuint *))IntGetProcAddress("glDeleteTextures");
  if(!vpvl2__ptrc_glDeleteTextures) numFailed++;
  vpvl2__ptrc_glDisableClientState = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glDisableClientState");
  if(!vpvl2__ptrc_glDisableClientState) numFailed++;
  vpvl2__ptrc_glDrawArrays = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLsizei))IntGetProcAddress("glDrawArrays");
  if(!vpvl2__ptrc_glDrawArrays) numFailed++;
  vpvl2__ptrc_glDrawElements = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, GLenum, const GLvoid *))IntGetProcAddress("glDrawElements");
  if(!vpvl2__ptrc_glDrawElements) numFailed++;
  vpvl2__ptrc_glEdgeFlagPointer = (void (CODEGEN_FUNCPTR *)(GLsizei, const GLvoid *))IntGetProcAddress("glEdgeFlagPointer");
  if(!vpvl2__ptrc_glEdgeFlagPointer) numFailed++;
  vpvl2__ptrc_glEnableClientState = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glEnableClientState");
  if(!vpvl2__ptrc_glEnableClientState) numFailed++;
  vpvl2__ptrc_glGenTextures = (void (CODEGEN_FUNCPTR *)(GLsizei, GLuint *))IntGetProcAddress("glGenTextures");
  if(!vpvl2__ptrc_glGenTextures) numFailed++;
  vpvl2__ptrc_glGetPointerv = (void (CODEGEN_FUNCPTR *)(GLenum, GLvoid **))IntGetProcAddress("glGetPointerv");
  if(!vpvl2__ptrc_glGetPointerv) numFailed++;
  vpvl2__ptrc_glIndexPointer = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, const GLvoid *))IntGetProcAddress("glIndexPointer");
  if(!vpvl2__ptrc_glIndexPointer) numFailed++;
  vpvl2__ptrc_glIndexub = (void (CODEGEN_FUNCPTR *)(GLubyte))IntGetProcAddress("glIndexub");
  if(!vpvl2__ptrc_glIndexub) numFailed++;
  vpvl2__ptrc_glIndexubv = (void (CODEGEN_FUNCPTR *)(const GLubyte *))IntGetProcAddress("glIndexubv");
  if(!vpvl2__ptrc_glIndexubv) numFailed++;
  vpvl2__ptrc_glInterleavedArrays = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, const GLvoid *))IntGetProcAddress("glInterleavedArrays");
  if(!vpvl2__ptrc_glInterleavedArrays) numFailed++;
  vpvl2__ptrc_glIsTexture = (GLboolean (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glIsTexture");
  if(!vpvl2__ptrc_glIsTexture) numFailed++;
  vpvl2__ptrc_glNormalPointer = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, const GLvoid *))IntGetProcAddress("glNormalPointer");
  if(!vpvl2__ptrc_glNormalPointer) numFailed++;
  vpvl2__ptrc_glPolygonOffset = (void (CODEGEN_FUNCPTR *)(GLfloat, GLfloat))IntGetProcAddress("glPolygonOffset");
  if(!vpvl2__ptrc_glPolygonOffset) numFailed++;
  vpvl2__ptrc_glPopClientAttrib = (void (CODEGEN_FUNCPTR *)())IntGetProcAddress("glPopClientAttrib");
  if(!vpvl2__ptrc_glPopClientAttrib) numFailed++;
  vpvl2__ptrc_glPrioritizeTextures = (void (CODEGEN_FUNCPTR *)(GLsizei, const GLuint *, const GLfloat *))IntGetProcAddress("glPrioritizeTextures");
  if(!vpvl2__ptrc_glPrioritizeTextures) numFailed++;
  vpvl2__ptrc_glPushClientAttrib = (void (CODEGEN_FUNCPTR *)(GLbitfield))IntGetProcAddress("glPushClientAttrib");
  if(!vpvl2__ptrc_glPushClientAttrib) numFailed++;
  vpvl2__ptrc_glTexCoordPointer = (void (CODEGEN_FUNCPTR *)(GLint, GLenum, GLsizei, const GLvoid *))IntGetProcAddress("glTexCoordPointer");
  if(!vpvl2__ptrc_glTexCoordPointer) numFailed++;
  vpvl2__ptrc_glTexSubImage1D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLsizei, GLenum, GLenum, const GLvoid *))IntGetProcAddress("glTexSubImage1D");
  if(!vpvl2__ptrc_glTexSubImage1D) numFailed++;
  vpvl2__ptrc_glTexSubImage2D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *))IntGetProcAddress("glTexSubImage2D");
  if(!vpvl2__ptrc_glTexSubImage2D) numFailed++;
  vpvl2__ptrc_glVertexPointer = (void (CODEGEN_FUNCPTR *)(GLint, GLenum, GLsizei, const GLvoid *))IntGetProcAddress("glVertexPointer");
  if(!vpvl2__ptrc_glVertexPointer) numFailed++;
  vpvl2__ptrc_glBlendColor = (void (CODEGEN_FUNCPTR *)(GLfloat, GLfloat, GLfloat, GLfloat))IntGetProcAddress("glBlendColor");
  if(!vpvl2__ptrc_glBlendColor) numFailed++;
  vpvl2__ptrc_glBlendEquation = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glBlendEquation");
  if(!vpvl2__ptrc_glBlendEquation) numFailed++;
  vpvl2__ptrc_glCopyTexSubImage3D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLint, GLint, GLint, GLint, GLsizei, GLsizei))IntGetProcAddress("glCopyTexSubImage3D");
  if(!vpvl2__ptrc_glCopyTexSubImage3D) numFailed++;
  vpvl2__ptrc_glDrawRangeElements = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint, GLuint, GLsizei, GLenum, const GLvoid *))IntGetProcAddress("glDrawRangeElements");
  if(!vpvl2__ptrc_glDrawRangeElements) numFailed++;
  vpvl2__ptrc_glTexImage3D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLsizei, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *))IntGetProcAddress("glTexImage3D");
  if(!vpvl2__ptrc_glTexImage3D) numFailed++;
  vpvl2__ptrc_glTexSubImage3D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLenum, const GLvoid *))IntGetProcAddress("glTexSubImage3D");
  if(!vpvl2__ptrc_glTexSubImage3D) numFailed++;
  vpvl2__ptrc_glActiveTexture = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glActiveTexture");
  if(!vpvl2__ptrc_glActiveTexture) numFailed++;
  vpvl2__ptrc_glClientActiveTexture = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glClientActiveTexture");
  if(!vpvl2__ptrc_glClientActiveTexture) numFailed++;
  vpvl2__ptrc_glCompressedTexImage1D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLenum, GLsizei, GLint, GLsizei, const GLvoid *))IntGetProcAddress("glCompressedTexImage1D");
  if(!vpvl2__ptrc_glCompressedTexImage1D) numFailed++;
  vpvl2__ptrc_glCompressedTexImage2D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *))IntGetProcAddress("glCompressedTexImage2D");
  if(!vpvl2__ptrc_glCompressedTexImage2D) numFailed++;
  vpvl2__ptrc_glCompressedTexImage3D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLenum, GLsizei, GLsizei, GLsizei, GLint, GLsizei, const GLvoid *))IntGetProcAddress("glCompressedTexImage3D");
  if(!vpvl2__ptrc_glCompressedTexImage3D) numFailed++;
  vpvl2__ptrc_glCompressedTexSubImage1D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLsizei, GLenum, GLsizei, const GLvoid *))IntGetProcAddress("glCompressedTexSubImage1D");
  if(!vpvl2__ptrc_glCompressedTexSubImage1D) numFailed++;
  vpvl2__ptrc_glCompressedTexSubImage2D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *))IntGetProcAddress("glCompressedTexSubImage2D");
  if(!vpvl2__ptrc_glCompressedTexSubImage2D) numFailed++;
  vpvl2__ptrc_glCompressedTexSubImage3D = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLint, GLint, GLsizei, GLsizei, GLsizei, GLenum, GLsizei, const GLvoid *))IntGetProcAddress("glCompressedTexSubImage3D");
  if(!vpvl2__ptrc_glCompressedTexSubImage3D) numFailed++;
  vpvl2__ptrc_glGetCompressedTexImage = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLvoid *))IntGetProcAddress("glGetCompressedTexImage");
  if(!vpvl2__ptrc_glGetCompressedTexImage) numFailed++;
  vpvl2__ptrc_glLoadTransposeMatrixd = (void (CODEGEN_FUNCPTR *)(const GLdouble *))IntGetProcAddress("glLoadTransposeMatrixd");
  if(!vpvl2__ptrc_glLoadTransposeMatrixd) numFailed++;
  vpvl2__ptrc_glLoadTransposeMatrixf = (void (CODEGEN_FUNCPTR *)(const GLfloat *))IntGetProcAddress("glLoadTransposeMatrixf");
  if(!vpvl2__ptrc_glLoadTransposeMatrixf) numFailed++;
  vpvl2__ptrc_glMultTransposeMatrixd = (void (CODEGEN_FUNCPTR *)(const GLdouble *))IntGetProcAddress("glMultTransposeMatrixd");
  if(!vpvl2__ptrc_glMultTransposeMatrixd) numFailed++;
  vpvl2__ptrc_glMultTransposeMatrixf = (void (CODEGEN_FUNCPTR *)(const GLfloat *))IntGetProcAddress("glMultTransposeMatrixf");
  if(!vpvl2__ptrc_glMultTransposeMatrixf) numFailed++;
  vpvl2__ptrc_glMultiTexCoord1d = (void (CODEGEN_FUNCPTR *)(GLenum, GLdouble))IntGetProcAddress("glMultiTexCoord1d");
  if(!vpvl2__ptrc_glMultiTexCoord1d) numFailed++;
  vpvl2__ptrc_glMultiTexCoord1dv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLdouble *))IntGetProcAddress("glMultiTexCoord1dv");
  if(!vpvl2__ptrc_glMultiTexCoord1dv) numFailed++;
  vpvl2__ptrc_glMultiTexCoord1f = (void (CODEGEN_FUNCPTR *)(GLenum, GLfloat))IntGetProcAddress("glMultiTexCoord1f");
  if(!vpvl2__ptrc_glMultiTexCoord1f) numFailed++;
  vpvl2__ptrc_glMultiTexCoord1fv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLfloat *))IntGetProcAddress("glMultiTexCoord1fv");
  if(!vpvl2__ptrc_glMultiTexCoord1fv) numFailed++;
  vpvl2__ptrc_glMultiTexCoord1i = (void (CODEGEN_FUNCPTR *)(GLenum, GLint))IntGetProcAddress("glMultiTexCoord1i");
  if(!vpvl2__ptrc_glMultiTexCoord1i) numFailed++;
  vpvl2__ptrc_glMultiTexCoord1iv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLint *))IntGetProcAddress("glMultiTexCoord1iv");
  if(!vpvl2__ptrc_glMultiTexCoord1iv) numFailed++;
  vpvl2__ptrc_glMultiTexCoord1s = (void (CODEGEN_FUNCPTR *)(GLenum, GLshort))IntGetProcAddress("glMultiTexCoord1s");
  if(!vpvl2__ptrc_glMultiTexCoord1s) numFailed++;
  vpvl2__ptrc_glMultiTexCoord1sv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLshort *))IntGetProcAddress("glMultiTexCoord1sv");
  if(!vpvl2__ptrc_glMultiTexCoord1sv) numFailed++;
  vpvl2__ptrc_glMultiTexCoord2d = (void (CODEGEN_FUNCPTR *)(GLenum, GLdouble, GLdouble))IntGetProcAddress("glMultiTexCoord2d");
  if(!vpvl2__ptrc_glMultiTexCoord2d) numFailed++;
  vpvl2__ptrc_glMultiTexCoord2dv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLdouble *))IntGetProcAddress("glMultiTexCoord2dv");
  if(!vpvl2__ptrc_glMultiTexCoord2dv) numFailed++;
  vpvl2__ptrc_glMultiTexCoord2f = (void (CODEGEN_FUNCPTR *)(GLenum, GLfloat, GLfloat))IntGetProcAddress("glMultiTexCoord2f");
  if(!vpvl2__ptrc_glMultiTexCoord2f) numFailed++;
  vpvl2__ptrc_glMultiTexCoord2fv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLfloat *))IntGetProcAddress("glMultiTexCoord2fv");
  if(!vpvl2__ptrc_glMultiTexCoord2fv) numFailed++;
  vpvl2__ptrc_glMultiTexCoord2i = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint))IntGetProcAddress("glMultiTexCoord2i");
  if(!vpvl2__ptrc_glMultiTexCoord2i) numFailed++;
  vpvl2__ptrc_glMultiTexCoord2iv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLint *))IntGetProcAddress("glMultiTexCoord2iv");
  if(!vpvl2__ptrc_glMultiTexCoord2iv) numFailed++;
  vpvl2__ptrc_glMultiTexCoord2s = (void (CODEGEN_FUNCPTR *)(GLenum, GLshort, GLshort))IntGetProcAddress("glMultiTexCoord2s");
  if(!vpvl2__ptrc_glMultiTexCoord2s) numFailed++;
  vpvl2__ptrc_glMultiTexCoord2sv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLshort *))IntGetProcAddress("glMultiTexCoord2sv");
  if(!vpvl2__ptrc_glMultiTexCoord2sv) numFailed++;
  vpvl2__ptrc_glMultiTexCoord3d = (void (CODEGEN_FUNCPTR *)(GLenum, GLdouble, GLdouble, GLdouble))IntGetProcAddress("glMultiTexCoord3d");
  if(!vpvl2__ptrc_glMultiTexCoord3d) numFailed++;
  vpvl2__ptrc_glMultiTexCoord3dv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLdouble *))IntGetProcAddress("glMultiTexCoord3dv");
  if(!vpvl2__ptrc_glMultiTexCoord3dv) numFailed++;
  vpvl2__ptrc_glMultiTexCoord3f = (void (CODEGEN_FUNCPTR *)(GLenum, GLfloat, GLfloat, GLfloat))IntGetProcAddress("glMultiTexCoord3f");
  if(!vpvl2__ptrc_glMultiTexCoord3f) numFailed++;
  vpvl2__ptrc_glMultiTexCoord3fv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLfloat *))IntGetProcAddress("glMultiTexCoord3fv");
  if(!vpvl2__ptrc_glMultiTexCoord3fv) numFailed++;
  vpvl2__ptrc_glMultiTexCoord3i = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLint))IntGetProcAddress("glMultiTexCoord3i");
  if(!vpvl2__ptrc_glMultiTexCoord3i) numFailed++;
  vpvl2__ptrc_glMultiTexCoord3iv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLint *))IntGetProcAddress("glMultiTexCoord3iv");
  if(!vpvl2__ptrc_glMultiTexCoord3iv) numFailed++;
  vpvl2__ptrc_glMultiTexCoord3s = (void (CODEGEN_FUNCPTR *)(GLenum, GLshort, GLshort, GLshort))IntGetProcAddress("glMultiTexCoord3s");
  if(!vpvl2__ptrc_glMultiTexCoord3s) numFailed++;
  vpvl2__ptrc_glMultiTexCoord3sv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLshort *))IntGetProcAddress("glMultiTexCoord3sv");
  if(!vpvl2__ptrc_glMultiTexCoord3sv) numFailed++;
  vpvl2__ptrc_glMultiTexCoord4d = (void (CODEGEN_FUNCPTR *)(GLenum, GLdouble, GLdouble, GLdouble, GLdouble))IntGetProcAddress("glMultiTexCoord4d");
  if(!vpvl2__ptrc_glMultiTexCoord4d) numFailed++;
  vpvl2__ptrc_glMultiTexCoord4dv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLdouble *))IntGetProcAddress("glMultiTexCoord4dv");
  if(!vpvl2__ptrc_glMultiTexCoord4dv) numFailed++;
  vpvl2__ptrc_glMultiTexCoord4f = (void (CODEGEN_FUNCPTR *)(GLenum, GLfloat, GLfloat, GLfloat, GLfloat))IntGetProcAddress("glMultiTexCoord4f");
  if(!vpvl2__ptrc_glMultiTexCoord4f) numFailed++;
  vpvl2__ptrc_glMultiTexCoord4fv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLfloat *))IntGetProcAddress("glMultiTexCoord4fv");
  if(!vpvl2__ptrc_glMultiTexCoord4fv) numFailed++;
  vpvl2__ptrc_glMultiTexCoord4i = (void (CODEGEN_FUNCPTR *)(GLenum, GLint, GLint, GLint, GLint))IntGetProcAddress("glMultiTexCoord4i");
  if(!vpvl2__ptrc_glMultiTexCoord4i) numFailed++;
  vpvl2__ptrc_glMultiTexCoord4iv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLint *))IntGetProcAddress("glMultiTexCoord4iv");
  if(!vpvl2__ptrc_glMultiTexCoord4iv) numFailed++;
  vpvl2__ptrc_glMultiTexCoord4s = (void (CODEGEN_FUNCPTR *)(GLenum, GLshort, GLshort, GLshort, GLshort))IntGetProcAddress("glMultiTexCoord4s");
  if(!vpvl2__ptrc_glMultiTexCoord4s) numFailed++;
  vpvl2__ptrc_glMultiTexCoord4sv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLshort *))IntGetProcAddress("glMultiTexCoord4sv");
  if(!vpvl2__ptrc_glMultiTexCoord4sv) numFailed++;
  vpvl2__ptrc_glSampleCoverage = (void (CODEGEN_FUNCPTR *)(GLfloat, GLboolean))IntGetProcAddress("glSampleCoverage");
  if(!vpvl2__ptrc_glSampleCoverage) numFailed++;
  vpvl2__ptrc_glBlendFuncSeparate = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum, GLenum))IntGetProcAddress("glBlendFuncSeparate");
  if(!vpvl2__ptrc_glBlendFuncSeparate) numFailed++;
  vpvl2__ptrc_glFogCoordPointer = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizei, const GLvoid *))IntGetProcAddress("glFogCoordPointer");
  if(!vpvl2__ptrc_glFogCoordPointer) numFailed++;
  vpvl2__ptrc_glFogCoordd = (void (CODEGEN_FUNCPTR *)(GLdouble))IntGetProcAddress("glFogCoordd");
  if(!vpvl2__ptrc_glFogCoordd) numFailed++;
  vpvl2__ptrc_glFogCoorddv = (void (CODEGEN_FUNCPTR *)(const GLdouble *))IntGetProcAddress("glFogCoorddv");
  if(!vpvl2__ptrc_glFogCoorddv) numFailed++;
  vpvl2__ptrc_glFogCoordf = (void (CODEGEN_FUNCPTR *)(GLfloat))IntGetProcAddress("glFogCoordf");
  if(!vpvl2__ptrc_glFogCoordf) numFailed++;
  vpvl2__ptrc_glFogCoordfv = (void (CODEGEN_FUNCPTR *)(const GLfloat *))IntGetProcAddress("glFogCoordfv");
  if(!vpvl2__ptrc_glFogCoordfv) numFailed++;
  vpvl2__ptrc_glMultiDrawArrays = (void (CODEGEN_FUNCPTR *)(GLenum, const GLint *, const GLsizei *, GLsizei))IntGetProcAddress("glMultiDrawArrays");
  if(!vpvl2__ptrc_glMultiDrawArrays) numFailed++;
  vpvl2__ptrc_glMultiDrawElements = (void (CODEGEN_FUNCPTR *)(GLenum, const GLsizei *, GLenum, const GLvoid *const*, GLsizei))IntGetProcAddress("glMultiDrawElements");
  if(!vpvl2__ptrc_glMultiDrawElements) numFailed++;
  vpvl2__ptrc_glPointParameterf = (void (CODEGEN_FUNCPTR *)(GLenum, GLfloat))IntGetProcAddress("glPointParameterf");
  if(!vpvl2__ptrc_glPointParameterf) numFailed++;
  vpvl2__ptrc_glPointParameterfv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLfloat *))IntGetProcAddress("glPointParameterfv");
  if(!vpvl2__ptrc_glPointParameterfv) numFailed++;
  vpvl2__ptrc_glPointParameteri = (void (CODEGEN_FUNCPTR *)(GLenum, GLint))IntGetProcAddress("glPointParameteri");
  if(!vpvl2__ptrc_glPointParameteri) numFailed++;
  vpvl2__ptrc_glPointParameteriv = (void (CODEGEN_FUNCPTR *)(GLenum, const GLint *))IntGetProcAddress("glPointParameteriv");
  if(!vpvl2__ptrc_glPointParameteriv) numFailed++;
  vpvl2__ptrc_glSecondaryColor3b = (void (CODEGEN_FUNCPTR *)(GLbyte, GLbyte, GLbyte))IntGetProcAddress("glSecondaryColor3b");
  if(!vpvl2__ptrc_glSecondaryColor3b) numFailed++;
  vpvl2__ptrc_glSecondaryColor3bv = (void (CODEGEN_FUNCPTR *)(const GLbyte *))IntGetProcAddress("glSecondaryColor3bv");
  if(!vpvl2__ptrc_glSecondaryColor3bv) numFailed++;
  vpvl2__ptrc_glSecondaryColor3d = (void (CODEGEN_FUNCPTR *)(GLdouble, GLdouble, GLdouble))IntGetProcAddress("glSecondaryColor3d");
  if(!vpvl2__ptrc_glSecondaryColor3d) numFailed++;
  vpvl2__ptrc_glSecondaryColor3dv = (void (CODEGEN_FUNCPTR *)(const GLdouble *))IntGetProcAddress("glSecondaryColor3dv");
  if(!vpvl2__ptrc_glSecondaryColor3dv) numFailed++;
  vpvl2__ptrc_glSecondaryColor3f = (void (CODEGEN_FUNCPTR *)(GLfloat, GLfloat, GLfloat))IntGetProcAddress("glSecondaryColor3f");
  if(!vpvl2__ptrc_glSecondaryColor3f) numFailed++;
  vpvl2__ptrc_glSecondaryColor3fv = (void (CODEGEN_FUNCPTR *)(const GLfloat *))IntGetProcAddress("glSecondaryColor3fv");
  if(!vpvl2__ptrc_glSecondaryColor3fv) numFailed++;
  vpvl2__ptrc_glSecondaryColor3i = (void (CODEGEN_FUNCPTR *)(GLint, GLint, GLint))IntGetProcAddress("glSecondaryColor3i");
  if(!vpvl2__ptrc_glSecondaryColor3i) numFailed++;
  vpvl2__ptrc_glSecondaryColor3iv = (void (CODEGEN_FUNCPTR *)(const GLint *))IntGetProcAddress("glSecondaryColor3iv");
  if(!vpvl2__ptrc_glSecondaryColor3iv) numFailed++;
  vpvl2__ptrc_glSecondaryColor3s = (void (CODEGEN_FUNCPTR *)(GLshort, GLshort, GLshort))IntGetProcAddress("glSecondaryColor3s");
  if(!vpvl2__ptrc_glSecondaryColor3s) numFailed++;
  vpvl2__ptrc_glSecondaryColor3sv = (void (CODEGEN_FUNCPTR *)(const GLshort *))IntGetProcAddress("glSecondaryColor3sv");
  if(!vpvl2__ptrc_glSecondaryColor3sv) numFailed++;
  vpvl2__ptrc_glSecondaryColor3ub = (void (CODEGEN_FUNCPTR *)(GLubyte, GLubyte, GLubyte))IntGetProcAddress("glSecondaryColor3ub");
  if(!vpvl2__ptrc_glSecondaryColor3ub) numFailed++;
  vpvl2__ptrc_glSecondaryColor3ubv = (void (CODEGEN_FUNCPTR *)(const GLubyte *))IntGetProcAddress("glSecondaryColor3ubv");
  if(!vpvl2__ptrc_glSecondaryColor3ubv) numFailed++;
  vpvl2__ptrc_glSecondaryColor3ui = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint, GLuint))IntGetProcAddress("glSecondaryColor3ui");
  if(!vpvl2__ptrc_glSecondaryColor3ui) numFailed++;
  vpvl2__ptrc_glSecondaryColor3uiv = (void (CODEGEN_FUNCPTR *)(const GLuint *))IntGetProcAddress("glSecondaryColor3uiv");
  if(!vpvl2__ptrc_glSecondaryColor3uiv) numFailed++;
  vpvl2__ptrc_glSecondaryColor3us = (void (CODEGEN_FUNCPTR *)(GLushort, GLushort, GLushort))IntGetProcAddress("glSecondaryColor3us");
  if(!vpvl2__ptrc_glSecondaryColor3us) numFailed++;
  vpvl2__ptrc_glSecondaryColor3usv = (void (CODEGEN_FUNCPTR *)(const GLushort *))IntGetProcAddress("glSecondaryColor3usv");
  if(!vpvl2__ptrc_glSecondaryColor3usv) numFailed++;
  vpvl2__ptrc_glSecondaryColorPointer = (void (CODEGEN_FUNCPTR *)(GLint, GLenum, GLsizei, const GLvoid *))IntGetProcAddress("glSecondaryColorPointer");
  if(!vpvl2__ptrc_glSecondaryColorPointer) numFailed++;
  vpvl2__ptrc_glWindowPos2d = (void (CODEGEN_FUNCPTR *)(GLdouble, GLdouble))IntGetProcAddress("glWindowPos2d");
  if(!vpvl2__ptrc_glWindowPos2d) numFailed++;
  vpvl2__ptrc_glWindowPos2dv = (void (CODEGEN_FUNCPTR *)(const GLdouble *))IntGetProcAddress("glWindowPos2dv");
  if(!vpvl2__ptrc_glWindowPos2dv) numFailed++;
  vpvl2__ptrc_glWindowPos2f = (void (CODEGEN_FUNCPTR *)(GLfloat, GLfloat))IntGetProcAddress("glWindowPos2f");
  if(!vpvl2__ptrc_glWindowPos2f) numFailed++;
  vpvl2__ptrc_glWindowPos2fv = (void (CODEGEN_FUNCPTR *)(const GLfloat *))IntGetProcAddress("glWindowPos2fv");
  if(!vpvl2__ptrc_glWindowPos2fv) numFailed++;
  vpvl2__ptrc_glWindowPos2i = (void (CODEGEN_FUNCPTR *)(GLint, GLint))IntGetProcAddress("glWindowPos2i");
  if(!vpvl2__ptrc_glWindowPos2i) numFailed++;
  vpvl2__ptrc_glWindowPos2iv = (void (CODEGEN_FUNCPTR *)(const GLint *))IntGetProcAddress("glWindowPos2iv");
  if(!vpvl2__ptrc_glWindowPos2iv) numFailed++;
  vpvl2__ptrc_glWindowPos2s = (void (CODEGEN_FUNCPTR *)(GLshort, GLshort))IntGetProcAddress("glWindowPos2s");
  if(!vpvl2__ptrc_glWindowPos2s) numFailed++;
  vpvl2__ptrc_glWindowPos2sv = (void (CODEGEN_FUNCPTR *)(const GLshort *))IntGetProcAddress("glWindowPos2sv");
  if(!vpvl2__ptrc_glWindowPos2sv) numFailed++;
  vpvl2__ptrc_glWindowPos3d = (void (CODEGEN_FUNCPTR *)(GLdouble, GLdouble, GLdouble))IntGetProcAddress("glWindowPos3d");
  if(!vpvl2__ptrc_glWindowPos3d) numFailed++;
  vpvl2__ptrc_glWindowPos3dv = (void (CODEGEN_FUNCPTR *)(const GLdouble *))IntGetProcAddress("glWindowPos3dv");
  if(!vpvl2__ptrc_glWindowPos3dv) numFailed++;
  vpvl2__ptrc_glWindowPos3f = (void (CODEGEN_FUNCPTR *)(GLfloat, GLfloat, GLfloat))IntGetProcAddress("glWindowPos3f");
  if(!vpvl2__ptrc_glWindowPos3f) numFailed++;
  vpvl2__ptrc_glWindowPos3fv = (void (CODEGEN_FUNCPTR *)(const GLfloat *))IntGetProcAddress("glWindowPos3fv");
  if(!vpvl2__ptrc_glWindowPos3fv) numFailed++;
  vpvl2__ptrc_glWindowPos3i = (void (CODEGEN_FUNCPTR *)(GLint, GLint, GLint))IntGetProcAddress("glWindowPos3i");
  if(!vpvl2__ptrc_glWindowPos3i) numFailed++;
  vpvl2__ptrc_glWindowPos3iv = (void (CODEGEN_FUNCPTR *)(const GLint *))IntGetProcAddress("glWindowPos3iv");
  if(!vpvl2__ptrc_glWindowPos3iv) numFailed++;
  vpvl2__ptrc_glWindowPos3s = (void (CODEGEN_FUNCPTR *)(GLshort, GLshort, GLshort))IntGetProcAddress("glWindowPos3s");
  if(!vpvl2__ptrc_glWindowPos3s) numFailed++;
  vpvl2__ptrc_glWindowPos3sv = (void (CODEGEN_FUNCPTR *)(const GLshort *))IntGetProcAddress("glWindowPos3sv");
  if(!vpvl2__ptrc_glWindowPos3sv) numFailed++;
  vpvl2__ptrc_glBeginQuery = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glBeginQuery");
  if(!vpvl2__ptrc_glBeginQuery) numFailed++;
  vpvl2__ptrc_glBindBuffer = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glBindBuffer");
  if(!vpvl2__ptrc_glBindBuffer) numFailed++;
  vpvl2__ptrc_glBufferData = (void (CODEGEN_FUNCPTR *)(GLenum, GLsizeiptr, const GLvoid *, GLenum))IntGetProcAddress("glBufferData");
  if(!vpvl2__ptrc_glBufferData) numFailed++;
  vpvl2__ptrc_glBufferSubData = (void (CODEGEN_FUNCPTR *)(GLenum, GLintptr, GLsizeiptr, const GLvoid *))IntGetProcAddress("glBufferSubData");
  if(!vpvl2__ptrc_glBufferSubData) numFailed++;
  vpvl2__ptrc_glDeleteBuffers = (void (CODEGEN_FUNCPTR *)(GLsizei, const GLuint *))IntGetProcAddress("glDeleteBuffers");
  if(!vpvl2__ptrc_glDeleteBuffers) numFailed++;
  vpvl2__ptrc_glDeleteQueries = (void (CODEGEN_FUNCPTR *)(GLsizei, const GLuint *))IntGetProcAddress("glDeleteQueries");
  if(!vpvl2__ptrc_glDeleteQueries) numFailed++;
  vpvl2__ptrc_glEndQuery = (void (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glEndQuery");
  if(!vpvl2__ptrc_glEndQuery) numFailed++;
  vpvl2__ptrc_glGenBuffers = (void (CODEGEN_FUNCPTR *)(GLsizei, GLuint *))IntGetProcAddress("glGenBuffers");
  if(!vpvl2__ptrc_glGenBuffers) numFailed++;
  vpvl2__ptrc_glGenQueries = (void (CODEGEN_FUNCPTR *)(GLsizei, GLuint *))IntGetProcAddress("glGenQueries");
  if(!vpvl2__ptrc_glGenQueries) numFailed++;
  vpvl2__ptrc_glGetBufferParameteriv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint *))IntGetProcAddress("glGetBufferParameteriv");
  if(!vpvl2__ptrc_glGetBufferParameteriv) numFailed++;
  vpvl2__ptrc_glGetBufferPointerv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLvoid **))IntGetProcAddress("glGetBufferPointerv");
  if(!vpvl2__ptrc_glGetBufferPointerv) numFailed++;
  vpvl2__ptrc_glGetBufferSubData = (void (CODEGEN_FUNCPTR *)(GLenum, GLintptr, GLsizeiptr, GLvoid *))IntGetProcAddress("glGetBufferSubData");
  if(!vpvl2__ptrc_glGetBufferSubData) numFailed++;
  vpvl2__ptrc_glGetQueryObjectiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLint *))IntGetProcAddress("glGetQueryObjectiv");
  if(!vpvl2__ptrc_glGetQueryObjectiv) numFailed++;
  vpvl2__ptrc_glGetQueryObjectuiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLuint *))IntGetProcAddress("glGetQueryObjectuiv");
  if(!vpvl2__ptrc_glGetQueryObjectuiv) numFailed++;
  vpvl2__ptrc_glGetQueryiv = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint *))IntGetProcAddress("glGetQueryiv");
  if(!vpvl2__ptrc_glGetQueryiv) numFailed++;
  vpvl2__ptrc_glIsBuffer = (GLboolean (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glIsBuffer");
  if(!vpvl2__ptrc_glIsBuffer) numFailed++;
  vpvl2__ptrc_glIsQuery = (GLboolean (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glIsQuery");
  if(!vpvl2__ptrc_glIsQuery) numFailed++;
  vpvl2__ptrc_glMapBuffer = (void * (CODEGEN_FUNCPTR *)(GLenum, GLenum))IntGetProcAddress("glMapBuffer");
  if(!vpvl2__ptrc_glMapBuffer) numFailed++;
  vpvl2__ptrc_glUnmapBuffer = (GLboolean (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glUnmapBuffer");
  if(!vpvl2__ptrc_glUnmapBuffer) numFailed++;
  vpvl2__ptrc_glAttachShader = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint))IntGetProcAddress("glAttachShader");
  if(!vpvl2__ptrc_glAttachShader) numFailed++;
  vpvl2__ptrc_glBindAttribLocation = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint, const GLchar *))IntGetProcAddress("glBindAttribLocation");
  if(!vpvl2__ptrc_glBindAttribLocation) numFailed++;
  vpvl2__ptrc_glBlendEquationSeparate = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum))IntGetProcAddress("glBlendEquationSeparate");
  if(!vpvl2__ptrc_glBlendEquationSeparate) numFailed++;
  vpvl2__ptrc_glCompileShader = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glCompileShader");
  if(!vpvl2__ptrc_glCompileShader) numFailed++;
  vpvl2__ptrc_glCreateProgram = (GLuint (CODEGEN_FUNCPTR *)())IntGetProcAddress("glCreateProgram");
  if(!vpvl2__ptrc_glCreateProgram) numFailed++;
  vpvl2__ptrc_glCreateShader = (GLuint (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glCreateShader");
  if(!vpvl2__ptrc_glCreateShader) numFailed++;
  vpvl2__ptrc_glDeleteProgram = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glDeleteProgram");
  if(!vpvl2__ptrc_glDeleteProgram) numFailed++;
  vpvl2__ptrc_glDeleteShader = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glDeleteShader");
  if(!vpvl2__ptrc_glDeleteShader) numFailed++;
  vpvl2__ptrc_glDetachShader = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint))IntGetProcAddress("glDetachShader");
  if(!vpvl2__ptrc_glDetachShader) numFailed++;
  vpvl2__ptrc_glDisableVertexAttribArray = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glDisableVertexAttribArray");
  if(!vpvl2__ptrc_glDisableVertexAttribArray) numFailed++;
  vpvl2__ptrc_glDrawBuffers = (void (CODEGEN_FUNCPTR *)(GLsizei, const GLenum *))IntGetProcAddress("glDrawBuffers");
  if(!vpvl2__ptrc_glDrawBuffers) numFailed++;
  vpvl2__ptrc_glEnableVertexAttribArray = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glEnableVertexAttribArray");
  if(!vpvl2__ptrc_glEnableVertexAttribArray) numFailed++;
  vpvl2__ptrc_glGetActiveAttrib = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *))IntGetProcAddress("glGetActiveAttrib");
  if(!vpvl2__ptrc_glGetActiveAttrib) numFailed++;
  vpvl2__ptrc_glGetActiveUniform = (void (CODEGEN_FUNCPTR *)(GLuint, GLuint, GLsizei, GLsizei *, GLint *, GLenum *, GLchar *))IntGetProcAddress("glGetActiveUniform");
  if(!vpvl2__ptrc_glGetActiveUniform) numFailed++;
  vpvl2__ptrc_glGetAttachedShaders = (void (CODEGEN_FUNCPTR *)(GLuint, GLsizei, GLsizei *, GLuint *))IntGetProcAddress("glGetAttachedShaders");
  if(!vpvl2__ptrc_glGetAttachedShaders) numFailed++;
  vpvl2__ptrc_glGetAttribLocation = (GLint (CODEGEN_FUNCPTR *)(GLuint, const GLchar *))IntGetProcAddress("glGetAttribLocation");
  if(!vpvl2__ptrc_glGetAttribLocation) numFailed++;
  vpvl2__ptrc_glGetProgramInfoLog = (void (CODEGEN_FUNCPTR *)(GLuint, GLsizei, GLsizei *, GLchar *))IntGetProcAddress("glGetProgramInfoLog");
  if(!vpvl2__ptrc_glGetProgramInfoLog) numFailed++;
  vpvl2__ptrc_glGetProgramiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLint *))IntGetProcAddress("glGetProgramiv");
  if(!vpvl2__ptrc_glGetProgramiv) numFailed++;
  vpvl2__ptrc_glGetShaderInfoLog = (void (CODEGEN_FUNCPTR *)(GLuint, GLsizei, GLsizei *, GLchar *))IntGetProcAddress("glGetShaderInfoLog");
  if(!vpvl2__ptrc_glGetShaderInfoLog) numFailed++;
  vpvl2__ptrc_glGetShaderSource = (void (CODEGEN_FUNCPTR *)(GLuint, GLsizei, GLsizei *, GLchar *))IntGetProcAddress("glGetShaderSource");
  if(!vpvl2__ptrc_glGetShaderSource) numFailed++;
  vpvl2__ptrc_glGetShaderiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLint *))IntGetProcAddress("glGetShaderiv");
  if(!vpvl2__ptrc_glGetShaderiv) numFailed++;
  vpvl2__ptrc_glGetUniformLocation = (GLint (CODEGEN_FUNCPTR *)(GLuint, const GLchar *))IntGetProcAddress("glGetUniformLocation");
  if(!vpvl2__ptrc_glGetUniformLocation) numFailed++;
  vpvl2__ptrc_glGetUniformfv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLfloat *))IntGetProcAddress("glGetUniformfv");
  if(!vpvl2__ptrc_glGetUniformfv) numFailed++;
  vpvl2__ptrc_glGetUniformiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLint *))IntGetProcAddress("glGetUniformiv");
  if(!vpvl2__ptrc_glGetUniformiv) numFailed++;
  vpvl2__ptrc_glGetVertexAttribPointerv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLvoid **))IntGetProcAddress("glGetVertexAttribPointerv");
  if(!vpvl2__ptrc_glGetVertexAttribPointerv) numFailed++;
  vpvl2__ptrc_glGetVertexAttribdv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLdouble *))IntGetProcAddress("glGetVertexAttribdv");
  if(!vpvl2__ptrc_glGetVertexAttribdv) numFailed++;
  vpvl2__ptrc_glGetVertexAttribfv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLfloat *))IntGetProcAddress("glGetVertexAttribfv");
  if(!vpvl2__ptrc_glGetVertexAttribfv) numFailed++;
  vpvl2__ptrc_glGetVertexAttribiv = (void (CODEGEN_FUNCPTR *)(GLuint, GLenum, GLint *))IntGetProcAddress("glGetVertexAttribiv");
  if(!vpvl2__ptrc_glGetVertexAttribiv) numFailed++;
  vpvl2__ptrc_glIsProgram = (GLboolean (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glIsProgram");
  if(!vpvl2__ptrc_glIsProgram) numFailed++;
  vpvl2__ptrc_glIsShader = (GLboolean (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glIsShader");
  if(!vpvl2__ptrc_glIsShader) numFailed++;
  vpvl2__ptrc_glLinkProgram = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glLinkProgram");
  if(!vpvl2__ptrc_glLinkProgram) numFailed++;
  vpvl2__ptrc_glShaderSource = (void (CODEGEN_FUNCPTR *)(GLuint, GLsizei, const GLchar *const*, const GLint *))IntGetProcAddress("glShaderSource");
  if(!vpvl2__ptrc_glShaderSource) numFailed++;
  vpvl2__ptrc_glStencilFuncSeparate = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLint, GLuint))IntGetProcAddress("glStencilFuncSeparate");
  if(!vpvl2__ptrc_glStencilFuncSeparate) numFailed++;
  vpvl2__ptrc_glStencilMaskSeparate = (void (CODEGEN_FUNCPTR *)(GLenum, GLuint))IntGetProcAddress("glStencilMaskSeparate");
  if(!vpvl2__ptrc_glStencilMaskSeparate) numFailed++;
  vpvl2__ptrc_glStencilOpSeparate = (void (CODEGEN_FUNCPTR *)(GLenum, GLenum, GLenum, GLenum))IntGetProcAddress("glStencilOpSeparate");
  if(!vpvl2__ptrc_glStencilOpSeparate) numFailed++;
  vpvl2__ptrc_glUniform1f = (void (CODEGEN_FUNCPTR *)(GLint, GLfloat))IntGetProcAddress("glUniform1f");
  if(!vpvl2__ptrc_glUniform1f) numFailed++;
  vpvl2__ptrc_glUniform1fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, const GLfloat *))IntGetProcAddress("glUniform1fv");
  if(!vpvl2__ptrc_glUniform1fv) numFailed++;
  vpvl2__ptrc_glUniform1i = (void (CODEGEN_FUNCPTR *)(GLint, GLint))IntGetProcAddress("glUniform1i");
  if(!vpvl2__ptrc_glUniform1i) numFailed++;
  vpvl2__ptrc_glUniform1iv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, const GLint *))IntGetProcAddress("glUniform1iv");
  if(!vpvl2__ptrc_glUniform1iv) numFailed++;
  vpvl2__ptrc_glUniform2f = (void (CODEGEN_FUNCPTR *)(GLint, GLfloat, GLfloat))IntGetProcAddress("glUniform2f");
  if(!vpvl2__ptrc_glUniform2f) numFailed++;
  vpvl2__ptrc_glUniform2fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, const GLfloat *))IntGetProcAddress("glUniform2fv");
  if(!vpvl2__ptrc_glUniform2fv) numFailed++;
  vpvl2__ptrc_glUniform2i = (void (CODEGEN_FUNCPTR *)(GLint, GLint, GLint))IntGetProcAddress("glUniform2i");
  if(!vpvl2__ptrc_glUniform2i) numFailed++;
  vpvl2__ptrc_glUniform2iv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, const GLint *))IntGetProcAddress("glUniform2iv");
  if(!vpvl2__ptrc_glUniform2iv) numFailed++;
  vpvl2__ptrc_glUniform3f = (void (CODEGEN_FUNCPTR *)(GLint, GLfloat, GLfloat, GLfloat))IntGetProcAddress("glUniform3f");
  if(!vpvl2__ptrc_glUniform3f) numFailed++;
  vpvl2__ptrc_glUniform3fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, const GLfloat *))IntGetProcAddress("glUniform3fv");
  if(!vpvl2__ptrc_glUniform3fv) numFailed++;
  vpvl2__ptrc_glUniform3i = (void (CODEGEN_FUNCPTR *)(GLint, GLint, GLint, GLint))IntGetProcAddress("glUniform3i");
  if(!vpvl2__ptrc_glUniform3i) numFailed++;
  vpvl2__ptrc_glUniform3iv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, const GLint *))IntGetProcAddress("glUniform3iv");
  if(!vpvl2__ptrc_glUniform3iv) numFailed++;
  vpvl2__ptrc_glUniform4f = (void (CODEGEN_FUNCPTR *)(GLint, GLfloat, GLfloat, GLfloat, GLfloat))IntGetProcAddress("glUniform4f");
  if(!vpvl2__ptrc_glUniform4f) numFailed++;
  vpvl2__ptrc_glUniform4fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, const GLfloat *))IntGetProcAddress("glUniform4fv");
  if(!vpvl2__ptrc_glUniform4fv) numFailed++;
  vpvl2__ptrc_glUniform4i = (void (CODEGEN_FUNCPTR *)(GLint, GLint, GLint, GLint, GLint))IntGetProcAddress("glUniform4i");
  if(!vpvl2__ptrc_glUniform4i) numFailed++;
  vpvl2__ptrc_glUniform4iv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, const GLint *))IntGetProcAddress("glUniform4iv");
  if(!vpvl2__ptrc_glUniform4iv) numFailed++;
  vpvl2__ptrc_glUniformMatrix2fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glUniformMatrix2fv");
  if(!vpvl2__ptrc_glUniformMatrix2fv) numFailed++;
  vpvl2__ptrc_glUniformMatrix3fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glUniformMatrix3fv");
  if(!vpvl2__ptrc_glUniformMatrix3fv) numFailed++;
  vpvl2__ptrc_glUniformMatrix4fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glUniformMatrix4fv");
  if(!vpvl2__ptrc_glUniformMatrix4fv) numFailed++;
  vpvl2__ptrc_glUseProgram = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glUseProgram");
  if(!vpvl2__ptrc_glUseProgram) numFailed++;
  vpvl2__ptrc_glValidateProgram = (void (CODEGEN_FUNCPTR *)(GLuint))IntGetProcAddress("glValidateProgram");
  if(!vpvl2__ptrc_glValidateProgram) numFailed++;
  vpvl2__ptrc_glVertexAttrib1d = (void (CODEGEN_FUNCPTR *)(GLuint, GLdouble))IntGetProcAddress("glVertexAttrib1d");
  if(!vpvl2__ptrc_glVertexAttrib1d) numFailed++;
  vpvl2__ptrc_glVertexAttrib1dv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLdouble *))IntGetProcAddress("glVertexAttrib1dv");
  if(!vpvl2__ptrc_glVertexAttrib1dv) numFailed++;
  vpvl2__ptrc_glVertexAttrib1f = (void (CODEGEN_FUNCPTR *)(GLuint, GLfloat))IntGetProcAddress("glVertexAttrib1f");
  if(!vpvl2__ptrc_glVertexAttrib1f) numFailed++;
  vpvl2__ptrc_glVertexAttrib1fv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLfloat *))IntGetProcAddress("glVertexAttrib1fv");
  if(!vpvl2__ptrc_glVertexAttrib1fv) numFailed++;
  vpvl2__ptrc_glVertexAttrib1s = (void (CODEGEN_FUNCPTR *)(GLuint, GLshort))IntGetProcAddress("glVertexAttrib1s");
  if(!vpvl2__ptrc_glVertexAttrib1s) numFailed++;
  vpvl2__ptrc_glVertexAttrib1sv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLshort *))IntGetProcAddress("glVertexAttrib1sv");
  if(!vpvl2__ptrc_glVertexAttrib1sv) numFailed++;
  vpvl2__ptrc_glVertexAttrib2d = (void (CODEGEN_FUNCPTR *)(GLuint, GLdouble, GLdouble))IntGetProcAddress("glVertexAttrib2d");
  if(!vpvl2__ptrc_glVertexAttrib2d) numFailed++;
  vpvl2__ptrc_glVertexAttrib2dv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLdouble *))IntGetProcAddress("glVertexAttrib2dv");
  if(!vpvl2__ptrc_glVertexAttrib2dv) numFailed++;
  vpvl2__ptrc_glVertexAttrib2f = (void (CODEGEN_FUNCPTR *)(GLuint, GLfloat, GLfloat))IntGetProcAddress("glVertexAttrib2f");
  if(!vpvl2__ptrc_glVertexAttrib2f) numFailed++;
  vpvl2__ptrc_glVertexAttrib2fv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLfloat *))IntGetProcAddress("glVertexAttrib2fv");
  if(!vpvl2__ptrc_glVertexAttrib2fv) numFailed++;
  vpvl2__ptrc_glVertexAttrib2s = (void (CODEGEN_FUNCPTR *)(GLuint, GLshort, GLshort))IntGetProcAddress("glVertexAttrib2s");
  if(!vpvl2__ptrc_glVertexAttrib2s) numFailed++;
  vpvl2__ptrc_glVertexAttrib2sv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLshort *))IntGetProcAddress("glVertexAttrib2sv");
  if(!vpvl2__ptrc_glVertexAttrib2sv) numFailed++;
  vpvl2__ptrc_glVertexAttrib3d = (void (CODEGEN_FUNCPTR *)(GLuint, GLdouble, GLdouble, GLdouble))IntGetProcAddress("glVertexAttrib3d");
  if(!vpvl2__ptrc_glVertexAttrib3d) numFailed++;
  vpvl2__ptrc_glVertexAttrib3dv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLdouble *))IntGetProcAddress("glVertexAttrib3dv");
  if(!vpvl2__ptrc_glVertexAttrib3dv) numFailed++;
  vpvl2__ptrc_glVertexAttrib3f = (void (CODEGEN_FUNCPTR *)(GLuint, GLfloat, GLfloat, GLfloat))IntGetProcAddress("glVertexAttrib3f");
  if(!vpvl2__ptrc_glVertexAttrib3f) numFailed++;
  vpvl2__ptrc_glVertexAttrib3fv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLfloat *))IntGetProcAddress("glVertexAttrib3fv");
  if(!vpvl2__ptrc_glVertexAttrib3fv) numFailed++;
  vpvl2__ptrc_glVertexAttrib3s = (void (CODEGEN_FUNCPTR *)(GLuint, GLshort, GLshort, GLshort))IntGetProcAddress("glVertexAttrib3s");
  if(!vpvl2__ptrc_glVertexAttrib3s) numFailed++;
  vpvl2__ptrc_glVertexAttrib3sv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLshort *))IntGetProcAddress("glVertexAttrib3sv");
  if(!vpvl2__ptrc_glVertexAttrib3sv) numFailed++;
  vpvl2__ptrc_glVertexAttrib4Nbv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLbyte *))IntGetProcAddress("glVertexAttrib4Nbv");
  if(!vpvl2__ptrc_glVertexAttrib4Nbv) numFailed++;
  vpvl2__ptrc_glVertexAttrib4Niv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLint *))IntGetProcAddress("glVertexAttrib4Niv");
  if(!vpvl2__ptrc_glVertexAttrib4Niv) numFailed++;
  vpvl2__ptrc_glVertexAttrib4Nsv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLshort *))IntGetProcAddress("glVertexAttrib4Nsv");
  if(!vpvl2__ptrc_glVertexAttrib4Nsv) numFailed++;
  vpvl2__ptrc_glVertexAttrib4Nub = (void (CODEGEN_FUNCPTR *)(GLuint, GLubyte, GLubyte, GLubyte, GLubyte))IntGetProcAddress("glVertexAttrib4Nub");
  if(!vpvl2__ptrc_glVertexAttrib4Nub) numFailed++;
  vpvl2__ptrc_glVertexAttrib4Nubv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLubyte *))IntGetProcAddress("glVertexAttrib4Nubv");
  if(!vpvl2__ptrc_glVertexAttrib4Nubv) numFailed++;
  vpvl2__ptrc_glVertexAttrib4Nuiv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLuint *))IntGetProcAddress("glVertexAttrib4Nuiv");
  if(!vpvl2__ptrc_glVertexAttrib4Nuiv) numFailed++;
  vpvl2__ptrc_glVertexAttrib4Nusv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLushort *))IntGetProcAddress("glVertexAttrib4Nusv");
  if(!vpvl2__ptrc_glVertexAttrib4Nusv) numFailed++;
  vpvl2__ptrc_glVertexAttrib4bv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLbyte *))IntGetProcAddress("glVertexAttrib4bv");
  if(!vpvl2__ptrc_glVertexAttrib4bv) numFailed++;
  vpvl2__ptrc_glVertexAttrib4d = (void (CODEGEN_FUNCPTR *)(GLuint, GLdouble, GLdouble, GLdouble, GLdouble))IntGetProcAddress("glVertexAttrib4d");
  if(!vpvl2__ptrc_glVertexAttrib4d) numFailed++;
  vpvl2__ptrc_glVertexAttrib4dv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLdouble *))IntGetProcAddress("glVertexAttrib4dv");
  if(!vpvl2__ptrc_glVertexAttrib4dv) numFailed++;
  vpvl2__ptrc_glVertexAttrib4f = (void (CODEGEN_FUNCPTR *)(GLuint, GLfloat, GLfloat, GLfloat, GLfloat))IntGetProcAddress("glVertexAttrib4f");
  if(!vpvl2__ptrc_glVertexAttrib4f) numFailed++;
  vpvl2__ptrc_glVertexAttrib4fv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLfloat *))IntGetProcAddress("glVertexAttrib4fv");
  if(!vpvl2__ptrc_glVertexAttrib4fv) numFailed++;
  vpvl2__ptrc_glVertexAttrib4iv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLint *))IntGetProcAddress("glVertexAttrib4iv");
  if(!vpvl2__ptrc_glVertexAttrib4iv) numFailed++;
  vpvl2__ptrc_glVertexAttrib4s = (void (CODEGEN_FUNCPTR *)(GLuint, GLshort, GLshort, GLshort, GLshort))IntGetProcAddress("glVertexAttrib4s");
  if(!vpvl2__ptrc_glVertexAttrib4s) numFailed++;
  vpvl2__ptrc_glVertexAttrib4sv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLshort *))IntGetProcAddress("glVertexAttrib4sv");
  if(!vpvl2__ptrc_glVertexAttrib4sv) numFailed++;
  vpvl2__ptrc_glVertexAttrib4ubv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLubyte *))IntGetProcAddress("glVertexAttrib4ubv");
  if(!vpvl2__ptrc_glVertexAttrib4ubv) numFailed++;
  vpvl2__ptrc_glVertexAttrib4uiv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLuint *))IntGetProcAddress("glVertexAttrib4uiv");
  if(!vpvl2__ptrc_glVertexAttrib4uiv) numFailed++;
  vpvl2__ptrc_glVertexAttrib4usv = (void (CODEGEN_FUNCPTR *)(GLuint, const GLushort *))IntGetProcAddress("glVertexAttrib4usv");
  if(!vpvl2__ptrc_glVertexAttrib4usv) numFailed++;
  vpvl2__ptrc_glVertexAttribPointer = (void (CODEGEN_FUNCPTR *)(GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid *))IntGetProcAddress("glVertexAttribPointer");
  if(!vpvl2__ptrc_glVertexAttribPointer) numFailed++;
  vpvl2__ptrc_glUniformMatrix2x3fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glUniformMatrix2x3fv");
  if(!vpvl2__ptrc_glUniformMatrix2x3fv) numFailed++;
  vpvl2__ptrc_glUniformMatrix2x4fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glUniformMatrix2x4fv");
  if(!vpvl2__ptrc_glUniformMatrix2x4fv) numFailed++;
  vpvl2__ptrc_glUniformMatrix3x2fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glUniformMatrix3x2fv");
  if(!vpvl2__ptrc_glUniformMatrix3x2fv) numFailed++;
  vpvl2__ptrc_glUniformMatrix3x4fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glUniformMatrix3x4fv");
  if(!vpvl2__ptrc_glUniformMatrix3x4fv) numFailed++;
  vpvl2__ptrc_glUniformMatrix4x2fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glUniformMatrix4x2fv");
  if(!vpvl2__ptrc_glUniformMatrix4x2fv) numFailed++;
  vpvl2__ptrc_glUniformMatrix4x3fv = (void (CODEGEN_FUNCPTR *)(GLint, GLsizei, GLboolean, const GLfloat *))IntGetProcAddress("glUniformMatrix4x3fv");
  if(!vpvl2__ptrc_glUniformMatrix4x3fv) numFailed++;
  return numFailed;
}

typedef int (*PFN_LOADFUNCPOINTERS)();
typedef struct vpvl2_ogl_StrToExtMap_s
{
  char *extensionName;
  int *extensionVariable;
  PFN_LOADFUNCPOINTERS LoadExtension;
} vpvl2_ogl_StrToExtMap;

static vpvl2_ogl_StrToExtMap ExtensionMap[17] = {
  {"GL_ARB_debug_output", &vpvl2_ogl_ext_ARB_debug_output, Load_ARB_debug_output},
  {"GL_ARB_depth_buffer_float", &vpvl2_ogl_ext_ARB_depth_buffer_float, NULL},
  {"GL_ARB_draw_elements_base_vertex", &vpvl2_ogl_ext_ARB_draw_elements_base_vertex, Load_ARB_draw_elements_base_vertex},
  {"GL_ARB_framebuffer_object", &vpvl2_ogl_ext_ARB_framebuffer_object, Load_ARB_framebuffer_object},
  {"GL_ARB_half_float_pixel", &vpvl2_ogl_ext_ARB_half_float_pixel, NULL},
  {"GL_ARB_map_buffer_range", &vpvl2_ogl_ext_ARB_map_buffer_range, Load_ARB_map_buffer_range},
  {"GL_ARB_occlusion_query", &vpvl2_ogl_ext_ARB_occlusion_query, Load_ARB_occlusion_query},
  {"GL_ARB_occlusion_query2", &vpvl2_ogl_ext_ARB_occlusion_query2, NULL},
  {"GL_ARB_sampler_objects", &vpvl2_ogl_ext_ARB_sampler_objects, Load_ARB_sampler_objects},
  {"GL_ARB_texture_float", &vpvl2_ogl_ext_ARB_texture_float, NULL},
  {"GL_ARB_texture_rg", &vpvl2_ogl_ext_ARB_texture_rg, NULL},
  {"GL_ARB_texture_storage", &vpvl2_ogl_ext_ARB_texture_storage, Load_ARB_texture_storage},
  {"GL_ARB_vertex_array_object", &vpvl2_ogl_ext_ARB_vertex_array_object, Load_ARB_vertex_array_object},
  {"GL_APPLE_vertex_array_object", &vpvl2_ogl_ext_APPLE_vertex_array_object, Load_APPLE_vertex_array_object},
  {"GL_EXT_framebuffer_blit", &vpvl2_ogl_ext_EXT_framebuffer_blit, Load_EXT_framebuffer_blit},
  {"GL_EXT_framebuffer_multisample", &vpvl2_ogl_ext_EXT_framebuffer_multisample, Load_EXT_framebuffer_multisample},
  {"GL_EXT_framebuffer_object", &vpvl2_ogl_ext_EXT_framebuffer_object, Load_EXT_framebuffer_object},
};

static int g_extensionMapSize = 17;

static vpvl2_ogl_StrToExtMap *FindExtEntry(const char *extensionName)
{
  int loop;
  vpvl2_ogl_StrToExtMap *currLoc = ExtensionMap;
  for(loop = 0; loop < g_extensionMapSize; ++loop, ++currLoc)
  {
  	if(strcmp(extensionName, currLoc->extensionName) == 0)
  		return currLoc;
  }
  
  return NULL;
}

static void ClearExtensionVars()
{
  vpvl2_ogl_ext_ARB_debug_output = vpvl2_ogl_LOAD_FAILED;
  vpvl2_ogl_ext_ARB_depth_buffer_float = vpvl2_ogl_LOAD_FAILED;
  vpvl2_ogl_ext_ARB_draw_elements_base_vertex = vpvl2_ogl_LOAD_FAILED;
  vpvl2_ogl_ext_ARB_framebuffer_object = vpvl2_ogl_LOAD_FAILED;
  vpvl2_ogl_ext_ARB_half_float_pixel = vpvl2_ogl_LOAD_FAILED;
  vpvl2_ogl_ext_ARB_map_buffer_range = vpvl2_ogl_LOAD_FAILED;
  vpvl2_ogl_ext_ARB_occlusion_query = vpvl2_ogl_LOAD_FAILED;
  vpvl2_ogl_ext_ARB_occlusion_query2 = vpvl2_ogl_LOAD_FAILED;
  vpvl2_ogl_ext_ARB_sampler_objects = vpvl2_ogl_LOAD_FAILED;
  vpvl2_ogl_ext_ARB_texture_float = vpvl2_ogl_LOAD_FAILED;
  vpvl2_ogl_ext_ARB_texture_rg = vpvl2_ogl_LOAD_FAILED;
  vpvl2_ogl_ext_ARB_texture_storage = vpvl2_ogl_LOAD_FAILED;
  vpvl2_ogl_ext_ARB_vertex_array_object = vpvl2_ogl_LOAD_FAILED;
  vpvl2_ogl_ext_APPLE_vertex_array_object = vpvl2_ogl_LOAD_FAILED;
  vpvl2_ogl_ext_EXT_framebuffer_blit = vpvl2_ogl_LOAD_FAILED;
  vpvl2_ogl_ext_EXT_framebuffer_multisample = vpvl2_ogl_LOAD_FAILED;
  vpvl2_ogl_ext_EXT_framebuffer_object = vpvl2_ogl_LOAD_FAILED;
}


static void LoadExtByName(const char *extensionName)
{
	vpvl2_ogl_StrToExtMap *entry = NULL;
	entry = FindExtEntry(extensionName);
	if(entry)
	{
		if(entry->LoadExtension)
		{
			int numFailed = entry->LoadExtension();
			if(numFailed == 0)
			{
				*(entry->extensionVariable) = vpvl2_ogl_LOAD_SUCCEEDED;
			}
			else
			{
				*(entry->extensionVariable) = vpvl2_ogl_LOAD_SUCCEEDED + numFailed;
			}
		}
		else
		{
			*(entry->extensionVariable) = vpvl2_ogl_LOAD_SUCCEEDED;
		}
	}
}


static void ProcExtsFromExtString(const char *strExtList)
{
	size_t iExtListLen = strlen(strExtList);
	const char *strExtListEnd = strExtList + iExtListLen;
	const char *strCurrPos = strExtList;
	char strWorkBuff[256];

	while(*strCurrPos)
	{
		/*Get the extension at our position.*/
		int iStrLen = 0;
		const char *strEndStr = strchr(strCurrPos, ' ');
		int iStop = 0;
		if(strEndStr == NULL)
		{
			strEndStr = strExtListEnd;
			iStop = 1;
		}

		iStrLen = (int)((ptrdiff_t)strEndStr - (ptrdiff_t)strCurrPos);

		if(iStrLen > 255)
			return;

		strncpy(strWorkBuff, strCurrPos, iStrLen);
		strWorkBuff[iStrLen] = '\0';

		LoadExtByName(strWorkBuff);

		strCurrPos = strEndStr + 1;
		if(iStop) break;
	}
}

int vpvl2_ogl_LoadFunctions()
{
  int numFailed = 0;
  ClearExtensionVars();
  
  vpvl2__ptrc_glGetString = (const GLubyte * (CODEGEN_FUNCPTR *)(GLenum))IntGetProcAddress("glGetString");
  if(!vpvl2__ptrc_glGetString) return vpvl2_ogl_LOAD_FAILED;
  
  ProcExtsFromExtString((const char *)vpvl2__ptrc_glGetString(GL_EXTENSIONS));
  numFailed = Load_Version_2_1();
  
  if(numFailed == 0)
  	return vpvl2_ogl_LOAD_SUCCEEDED;
  else
  	return vpvl2_ogl_LOAD_SUCCEEDED + numFailed;
}

static int g_major_version = 0;
static int g_minor_version = 0;

static void ParseVersionFromString(int *pOutMajor, int *pOutMinor, const char *strVersion)
{
	const char *strDotPos = NULL;
	int iLength = 0;
	char strWorkBuff[10];
	*pOutMinor = 0;
	*pOutMajor = 0;

	strDotPos = strchr(strVersion, '.');
	if(!strDotPos)
		return;

	iLength = (int)((ptrdiff_t)strDotPos - (ptrdiff_t)strVersion);
	strncpy(strWorkBuff, strVersion, iLength);
	strWorkBuff[iLength] = '\0';

	*pOutMajor = atoi(strWorkBuff);
	strDotPos = strchr(strVersion + iLength + 1, ' ');
	if(!strDotPos)
	{
		/*No extra data. Take the whole rest of the string.*/
		strcpy(strWorkBuff, strVersion + iLength + 1);
	}
	else
	{
		/*Copy only up until the space.*/
		int iLengthMinor = (int)((ptrdiff_t)strDotPos - (ptrdiff_t)strVersion);
		iLengthMinor = iLengthMinor - (iLength + 1);
		strncpy(strWorkBuff, strVersion + iLength + 1, iLengthMinor);
		strWorkBuff[iLengthMinor] = '\0';
	}

	*pOutMinor = atoi(strWorkBuff);
}

static void GetGLVersion()
{
	ParseVersionFromString(&g_major_version, &g_minor_version, (const char*)glGetString(GL_VERSION));
}

int vpvl2_ogl_GetMajorVersion()
{
	if(g_major_version == 0)
		GetGLVersion();
	return g_major_version;
}

int vpvl2_ogl_GetMinorVersion()
{
	if(g_major_version == 0) //Yes, check the major version to get the minor one.
		GetGLVersion();
	return g_minor_version;
}

int vpvl2_ogl_IsVersionGEQ(int majorVersion, int minorVersion)
{
	if(g_major_version == 0)
		GetGLVersion();
		
	if(majorVersion > g_major_version) return 1;
	if(majorVersion < g_major_version) return 0;
	if(minorVersion >= g_minor_version) return 1;
	return 0;
}

