#include "GL/glew.h"

namespace nvFX {

PFNGLACTIVETEXTUREPROC glActiveTexture = 0;
PFNGLALPHAFUNCPROC glAlphaFunc = 0;
PFNGLATTACHOBJECTARBPROC glAttachObjectARB = 0;
PFNGLATTACHSHADERPROC glAttachShader = 0;
PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation = 0;
PFNGLBINDBUFFERPROC glBindBuffer = 0;
PFNGLBINDBUFFERBASEPROC glBindBufferBase = 0;
PFNGLBINDBUFFERRANGEPROC glBindBufferRange = 0;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = 0;
PFNGLBINDIMAGETEXTUREPROC glBindImageTexture = 0;
PFNGLBINDPROGRAMPIPELINEPROC glBindProgramPipeline = 0;
PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer = 0;
PFNGLBINDSAMPLERPROC glBindSampler = 0;
PFNGLBINDTEXTUREPROC glBindTexture = 0;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray = 0;
PFNGLBLENDCOLORPROC glBlendColor = 0;
PFNGLBLENDEQUATIONSEPARATEPROC glBlendEquationSeparate = 0;
PFNGLBLENDFUNCPROC glBlendFunc = 0;
PFNGLBLENDFUNCSEPARATEPROC glBlendFuncSeparate = 0;
PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer = 0;
PFNGLBUFFERDATAPROC glBufferData = 0;
PFNGLBUFFERSUBDATAPROC glBufferSubData = 0;
PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus = 0;
PFNGLCLEARPROC glClear = 0;
PFNGLCLEARCOLORPROC glClearColor = 0;
PFNGLCOLORMASKPROC glColorMask = 0;
PFNGLCOMPILESHADERPROC glCompileShader = 0;
PFNGLCOVERFILLPATHNVPROC glCoverFillPathNV = 0;
PFNGLCOVERSTROKEPATHNVPROC glCoverStrokePathNV = 0;
PFNGLCREATEPROGRAMPROC glCreateProgram = 0;
PFNGLCREATESHADERPROC glCreateShader = 0;
PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB = 0;
PFNGLCULLFACEPROC glCullFace = 0;
PFNGLDELETEBUFFERSPROC glDeleteBuffers = 0;
PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers = 0;
PFNGLDELETEOBJECTARBPROC glDeleteObjectARB = 0;
PFNGLDELETEPATHSNVPROC glDeletePathsNV = 0;
PFNGLDELETEPROGRAMPROC glDeleteProgram = 0;
PFNGLDELETEPROGRAMPIPELINESPROC glDeleteProgramPipelines = 0;
PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers = 0;
PFNGLDELETESHADERPROC glDeleteShader = 0;
PFNGLDELETETEXTURESPROC glDeleteTextures = 0;
PFNGLDEPTHFUNCPROC glDepthFunc = 0;
PFNGLDEPTHMASKPROC glDepthMask = 0;
PFNGLDETACHOBJECTARBPROC glDetachObjectARB = 0;
PFNGLDETACHSHADERPROC glDetachShader = 0;
PFNGLDISABLEPROC glDisable = 0;
PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray = 0;
PFNGLDRAWARRAYSPROC glDrawArrays = 0;
PFNGLDRAWBUFFERPROC glDrawBuffer = 0;
PFNGLDRAWBUFFERSPROC glDrawBuffers = 0;
PFNGLENABLEPROC glEnable = 0;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = 0;
PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer = 0;
PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D = 0;
PFNGLFRONTFACEPROC glFrontFace = 0;
PFNGLGENBUFFERSPROC glGenBuffers = 0;
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers = 0;
PFNGLGENPATHSNVPROC glGenPathsNV = 0;
PFNGLGENPROGRAMPIPELINESPROC glGenProgramPipelines = 0;
PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers = 0;
PFNGLGENTEXTURESPROC glGenTextures = 0;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = 0;
PFNGLGETACTIVEUNIFORMPROC glGetActiveUniform = 0;
PFNGLGETACTIVEUNIFORMBLOCKIVPROC glGetActiveUniformBlockiv = 0;
PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation = 0;
PFNGLGETERRORPROC glGetError = 0;
PFNGLGETINFOLOGARBPROC glGetInfoLogARB = 0;
PFNGLGETINTEGERVPROC glGetIntegerv = 0;
PFNGLGETPROGRAMBINARYPROC glGetProgramBinary = 0;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = 0;
PFNGLGETPROGRAMPIPELINEIVPROC glGetProgramPipelineiv = 0;
PFNGLGETPROGRAMSTAGEIVPROC glGetProgramStageiv = 0;
PFNGLGETPROGRAMIVPROC glGetProgramiv = 0;
PFNGLGETRENDERBUFFERPARAMETERIVPROC glGetRenderbufferParameteriv = 0;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = 0;
PFNGLGETSHADERIVPROC glGetShaderiv = 0;
PFNGLGETSTRINGPROC glGetString = 0;
PFNGLGETSUBROUTINEINDEXPROC glGetSubroutineIndex = 0;
PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC glGetSubroutineUniformLocation = 0;
PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndex = 0;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = 0;
PFNGLISPROGRAMPIPELINEPROC glIsProgramPipeline = 0;
PFNGLLINEWIDTHPROC glLineWidth = 0;
PFNGLLINKPROGRAMPROC glLinkProgram = 0;
PFNGLLOGICOPPROC glLogicOp = 0;
PFNGLMAPBUFFERPROC glMapBuffer = 0;
PFNGLMAPBUFFERRANGEPROC glMapBufferRange = 0;
PFNGLPATHPARAMETERFNVPROC glPathParameterfNV = 0;
PFNGLPATHPARAMETERINVPROC glPathParameteriNV = 0;
PFNGLPATHSTENCILDEPTHOFFSETNVPROC glPathStencilDepthOffsetNV = 0;
PFNGLPATHSTRINGNVPROC glPathStringNV = 0;
PFNGLPOINTSIZEPROC glPointSize = 0;
PFNGLPOLYGONMODEPROC glPolygonMode = 0;
PFNGLPOLYGONOFFSETPROC glPolygonOffset = 0;
PFNGLPROGRAMPARAMETERIPROC glProgramParameteri = 0;
PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage = 0;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC glRenderbufferStorageMultisample = 0;
PFNGLRENDERBUFFERSTORAGEMULTISAMPLECOVERAGENVPROC glRenderbufferStorageMultisampleCoverageNV = 0;
PFNGLSHADERSOURCEPROC glShaderSource = 0;
PFNGLSTENCILFILLPATHINSTANCEDNVPROC glStencilFillPathInstancedNV = 0;
PFNGLSTENCILFILLPATHNVPROC glStencilFillPathNV = 0;
PFNGLSTENCILFUNCSEPARATEPROC glStencilFuncSeparate = 0;
PFNGLSTENCILOPSEPARATEPROC glStencilOpSeparate = 0;
PFNGLSTENCILSTROKEPATHNVPROC glStencilStrokePathNV = 0;
PFNGLTEXIMAGE1DPROC glTexImage1D = 0;
PFNGLTEXIMAGE2DPROC glTexImage2D = 0;
PFNGLTEXIMAGE2DMULTISAMPLEPROC glTexImage2DMultisample = 0;
PFNGLTEXIMAGE3DPROC glTexImage3D = 0;
PFNGLTEXPARAMETERFPROC glTexParameterf = 0;
PFNGLTEXPARAMETERIPROC glTexParameteri = 0;
PFNGLTEXTUREPARAMETERIEXTPROC glTextureParameteriEXT = 0;
PFNGLUNIFORM1FPROC glUniform1f = 0;
PFNGLUNIFORM1FVPROC glUniform1fv = 0;
PFNGLUNIFORM1IPROC glUniform1i = 0;
PFNGLUNIFORM1IVPROC glUniform1iv = 0;
PFNGLUNIFORM2FVPROC glUniform2fv = 0;
PFNGLUNIFORM2IVPROC glUniform2iv = 0;
PFNGLUNIFORM3FVPROC glUniform3fv = 0;
PFNGLUNIFORM3IVPROC glUniform3iv = 0;
PFNGLUNIFORM4FVPROC glUniform4fv = 0;
PFNGLUNIFORM4IVPROC glUniform4iv = 0;
PFNGLUNIFORMBLOCKBINDINGPROC glUniformBlockBinding = 0;
PFNGLUNIFORMSUBROUTINESUIVPROC glUniformSubroutinesuiv = 0;
PFNGLUNMAPBUFFERPROC glUnmapBuffer = 0;
PFNGLUSEPROGRAMPROC glUseProgram = 0;
PFNGLUSEPROGRAMSTAGESPROC glUseProgramStages = 0;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = 0;
PFNGLVIEWPORTPROC glViewport = 0;

PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = 0;
PFNGLPROGRAMUNIFORM1IPROC glProgramUniform1i = 0;
PFNGLPROGRAMUNIFORM1IVPROC glProgramUniform1iv = 0;
PFNGLPROGRAMUNIFORM1FVPROC glProgramUniform1fv = 0;
PFNGLPROGRAMUNIFORM2IVPROC glProgramUniform2iv = 0;
PFNGLPROGRAMUNIFORM2FVPROC glProgramUniform2fv = 0;
PFNGLPROGRAMUNIFORM3IVPROC glProgramUniform3iv = 0;
PFNGLPROGRAMUNIFORM3FVPROC glProgramUniform3fv = 0;
PFNGLPROGRAMUNIFORM4IVPROC glProgramUniform4iv = 0;
PFNGLPROGRAMUNIFORM4FVPROC glProgramUniform4fv = 0;
PFNGLPROGRAMUNIFORMMATRIX4FVPROC glProgramUniformMatrix4fv = 0;
PFNGLXGETPROCADDRESSPROC glXGetProcAddress = 0;

static inline GLenum __glGetError()
{
    return 0;
}

void initializeOpenGLFunctions(const FunctionResolver *resolver)
{
    glActiveTexture = reinterpret_cast<PFNGLACTIVETEXTUREPROC>(resolver->resolve("glActiveTexture"));
    glAlphaFunc = reinterpret_cast<PFNGLALPHAFUNCPROC>(resolver->resolve("glAlphaFunc"));
    glAttachObjectARB = reinterpret_cast<PFNGLATTACHOBJECTARBPROC>(resolver->resolve("glAttachObjectARB"));
    glAttachShader = reinterpret_cast<PFNGLATTACHSHADERPROC>(resolver->resolve("glAttachShader"));
    glBindAttribLocation = reinterpret_cast<PFNGLBINDATTRIBLOCATIONPROC>(resolver->resolve("glBindAttribLocation"));
    glBindBuffer = reinterpret_cast<PFNGLBINDBUFFERPROC>(resolver->resolve("glBindBuffer"));
    glBindFramebuffer = reinterpret_cast<PFNGLBINDFRAMEBUFFERPROC>(resolver->resolve("glBindFramebuffer"));
    glBindRenderbuffer = reinterpret_cast<PFNGLBINDRENDERBUFFERPROC>(resolver->resolve("glBindRenderbuffer"));
    glBindTexture = reinterpret_cast<PFNGLBINDTEXTUREPROC>(resolver->resolve("glBindTexture"));
    glBlendColor = reinterpret_cast<PFNGLBLENDCOLORPROC>(resolver->resolve("glBlendColor"));
    glBlendEquationSeparate = reinterpret_cast<PFNGLBLENDEQUATIONSEPARATEPROC>(resolver->resolve("glBlendEquationSeparate"));
    glBlendFunc = reinterpret_cast<PFNGLBLENDFUNCPROC>(resolver->resolve("glBlendFunc"));
    glBlendFuncSeparate = reinterpret_cast<PFNGLBLENDFUNCSEPARATEPROC>(resolver->resolve("glBlendFuncSeparate"));
    glBlitFramebuffer = reinterpret_cast<PFNGLBLITFRAMEBUFFERPROC>(resolver->resolve("glBlitFramebuffer"));
    glBufferData = reinterpret_cast<PFNGLBUFFERDATAPROC>(resolver->resolve("glBufferData"));
    glBufferSubData = reinterpret_cast<PFNGLBUFFERSUBDATAPROC>(resolver->resolve("glBufferSubData"));
    glCheckFramebufferStatus = reinterpret_cast<PFNGLCHECKFRAMEBUFFERSTATUSPROC>(resolver->resolve("glCheckFramebufferStatus"));
    glClear = reinterpret_cast<PFNGLCLEARPROC>(resolver->resolve("glClear"));
    glClearColor = reinterpret_cast<PFNGLCLEARCOLORPROC>(resolver->resolve("glClearColor"));
    glColorMask = reinterpret_cast<PFNGLCOLORMASKPROC>(resolver->resolve("glColorMask"));
    glCompileShader = reinterpret_cast<PFNGLCOMPILESHADERPROC>(resolver->resolve("glCompileShader"));
    glCreateProgram = reinterpret_cast<PFNGLCREATEPROGRAMPROC>(resolver->resolve("glCreateProgram"));
    glCreateShader = reinterpret_cast<PFNGLCREATESHADERPROC>(resolver->resolve("glCreateShader"));
    glCreateShaderObjectARB = reinterpret_cast<PFNGLCREATESHADEROBJECTARBPROC>(resolver->resolve("glCreateShaderObjectARB"));
    glCullFace = reinterpret_cast<PFNGLCULLFACEPROC>(resolver->resolve("glCullFace"));
    glDeleteBuffers = reinterpret_cast<PFNGLDELETEBUFFERSPROC>(resolver->resolve("glDeleteBuffers"));
    glDeleteFramebuffers = reinterpret_cast<PFNGLDELETEFRAMEBUFFERSPROC>(resolver->resolve("glDeleteFramebuffers"));
    glDeleteObjectARB = reinterpret_cast<PFNGLDELETEOBJECTARBPROC>(resolver->resolve("glDeleteObjectARB"));
    glDeleteProgram = reinterpret_cast<PFNGLDELETEPROGRAMPROC>(resolver->resolve("glDeleteProgram"));
    glDeleteRenderbuffers = reinterpret_cast<PFNGLDELETERENDERBUFFERSPROC>(resolver->resolve("glDeleteRenderbuffers"));
    glDeleteShader = reinterpret_cast<PFNGLDELETESHADERPROC>(resolver->resolve("glDeleteShader"));
    glDeleteTextures = reinterpret_cast<PFNGLDELETETEXTURESPROC>(resolver->resolve("glDeleteTextures"));
    glDepthFunc = reinterpret_cast<PFNGLDEPTHFUNCPROC>(resolver->resolve("glDepthFunc"));
    glDepthMask = reinterpret_cast<PFNGLDEPTHMASKPROC>(resolver->resolve("glDepthMask"));
    glDetachObjectARB = reinterpret_cast<PFNGLDETACHOBJECTARBPROC>(resolver->resolve("glDetachObjectARB"));
    glDetachShader = reinterpret_cast<PFNGLDETACHSHADERPROC>(resolver->resolve("glDetachShader"));
    glDisable = reinterpret_cast<PFNGLDISABLEPROC>(resolver->resolve("glDisable"));
    glDisableVertexAttribArray = reinterpret_cast<PFNGLDISABLEVERTEXATTRIBARRAYPROC>(resolver->resolve("glDisableVertexAttribArray"));
    glDrawArrays = reinterpret_cast<PFNGLDRAWARRAYSPROC>(resolver->resolve("glDrawArrays"));
    glDrawBuffer = reinterpret_cast<PFNGLDRAWBUFFERPROC>(resolver->resolve("glDrawBuffer"));
    glDrawBuffers = reinterpret_cast<PFNGLDRAWBUFFERSPROC>(resolver->resolve("glDrawBuffers"));
    glEnable = reinterpret_cast<PFNGLENABLEPROC>(resolver->resolve("glEnable"));
    glEnableVertexAttribArray = reinterpret_cast<PFNGLENABLEVERTEXATTRIBARRAYPROC>(resolver->resolve("glEnableVertexAttribArray"));
    glFramebufferRenderbuffer = reinterpret_cast<PFNGLFRAMEBUFFERRENDERBUFFERPROC>(resolver->resolve("glFramebufferRenderbuffer"));
    glFramebufferTexture2D = reinterpret_cast<PFNGLFRAMEBUFFERTEXTURE2DPROC>(resolver->resolve("glFramebufferTexture2D"));
    glFrontFace = reinterpret_cast<PFNGLFRONTFACEPROC>(resolver->resolve("glFrontFace"));
    glGenBuffers = reinterpret_cast<PFNGLGENBUFFERSPROC>(resolver->resolve("glGenBuffers"));
    glGenFramebuffers = reinterpret_cast<PFNGLGENFRAMEBUFFERSPROC>(resolver->resolve("glGenFramebuffers"));
    glGenRenderbuffers = reinterpret_cast<PFNGLGENRENDERBUFFERSPROC>(resolver->resolve("glGenRenderbuffers"));
    glGenTextures = reinterpret_cast<PFNGLGENTEXTURESPROC>(resolver->resolve("glGenTextures"));
    glGetActiveUniform = reinterpret_cast<PFNGLGETACTIVEUNIFORMPROC>(resolver->resolve("glGetActiveUniform"));
    glGetAttribLocation = reinterpret_cast<PFNGLGETATTRIBLOCATIONPROC>(resolver->resolve("glGetAttribLocation"));
    glGetError = __glGetError; // reinterpret_cast<PFNGLGETERRORPROC>(resolver->resolve("glGetError"));
    glGetInfoLogARB = reinterpret_cast<PFNGLGETINFOLOGARBPROC>(resolver->resolve("glGetInfoLogARB"));
    glGetIntegerv = reinterpret_cast<PFNGLGETINTEGERVPROC>(resolver->resolve("glGetIntegerv"));
    glGetProgramInfoLog = reinterpret_cast<PFNGLGETPROGRAMINFOLOGPROC>(resolver->resolve("glGetProgramInfoLog"));
    glGetProgramiv = reinterpret_cast<PFNGLGETPROGRAMIVPROC>(resolver->resolve("glGetProgramiv"));
    glGetRenderbufferParameteriv = reinterpret_cast<PFNGLGETRENDERBUFFERPARAMETERIVPROC>(resolver->resolve("glGetRenderbufferParameteriv"));
    glGetShaderInfoLog = reinterpret_cast<PFNGLGETSHADERINFOLOGPROC>(resolver->resolve("glGetShaderInfoLog"));
    glGetShaderiv = reinterpret_cast<PFNGLGETSHADERIVPROC>(resolver->resolve("glGetShaderiv"));
    glGetString = reinterpret_cast<PFNGLGETSTRINGPROC>(resolver->resolve("glGetString"));
    glGetUniformLocation = reinterpret_cast<PFNGLGETUNIFORMLOCATIONPROC>(resolver->resolve("glGetUniformLocation"));
    glLineWidth = reinterpret_cast<PFNGLLINEWIDTHPROC>(resolver->resolve("glLineWidth"));
    glLinkProgram = reinterpret_cast<PFNGLLINKPROGRAMPROC>(resolver->resolve("glLinkProgram"));
    glLogicOp = reinterpret_cast<PFNGLLOGICOPPROC>(resolver->resolve("glLogicOp"));
    glMapBuffer = reinterpret_cast<PFNGLMAPBUFFERPROC>(resolver->resolve("glMapBuffer"));
    glPointSize = reinterpret_cast<PFNGLPOINTSIZEPROC>(resolver->resolve("glPointSize"));
    glPolygonMode = reinterpret_cast<PFNGLPOLYGONMODEPROC>(resolver->resolve("glPolygonMode"));
    glPolygonOffset = reinterpret_cast<PFNGLPOLYGONOFFSETPROC>(resolver->resolve("glPolygonOffset"));
    glProgramParameteri = reinterpret_cast<PFNGLPROGRAMPARAMETERIPROC>(resolver->resolve("glProgramParameteri"));
    glRenderbufferStorage = reinterpret_cast<PFNGLRENDERBUFFERSTORAGEPROC>(resolver->resolve("glRenderbufferStorage"));
    glRenderbufferStorageMultisample = reinterpret_cast<PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC>(resolver->resolve("glRenderbufferStorageMultisample"));
    glShaderSource = reinterpret_cast<PFNGLSHADERSOURCEPROC>(resolver->resolve("glShaderSource"));
    glStencilFuncSeparate = reinterpret_cast<PFNGLSTENCILFUNCSEPARATEPROC>(resolver->resolve("glStencilFuncSeparate"));
    glStencilOpSeparate = reinterpret_cast<PFNGLSTENCILOPSEPARATEPROC>(resolver->resolve("glStencilOpSeparate"));
    glTexImage1D = reinterpret_cast<PFNGLTEXIMAGE1DPROC>(resolver->resolve("glTexImage1D"));
    glTexImage2D = reinterpret_cast<PFNGLTEXIMAGE2DPROC>(resolver->resolve("glTexImage2D"));
    glTexImage3D = reinterpret_cast<PFNGLTEXIMAGE3DPROC>(resolver->resolve("glTexImage3D"));
    glTexParameterf = reinterpret_cast<PFNGLTEXPARAMETERFPROC>(resolver->resolve("glTexParameterf"));
    glTexParameteri = reinterpret_cast<PFNGLTEXPARAMETERIPROC>(resolver->resolve("glTexParameteri"));
    glUniform1f = reinterpret_cast<PFNGLUNIFORM1FPROC>(resolver->resolve("glUniform1f"));
    glUniform1fv = reinterpret_cast<PFNGLUNIFORM1FVPROC>(resolver->resolve("glUniform1fv"));
    glUniform1i = reinterpret_cast<PFNGLUNIFORM1IPROC>(resolver->resolve("glUniform1i"));
    glUniform1iv = reinterpret_cast<PFNGLUNIFORM1IVPROC>(resolver->resolve("glUniform1iv"));
    glUniform2fv = reinterpret_cast<PFNGLUNIFORM2FVPROC>(resolver->resolve("glUniform2fv"));
    glUniform2iv = reinterpret_cast<PFNGLUNIFORM2IVPROC>(resolver->resolve("glUniform2iv"));
    glUniform3fv = reinterpret_cast<PFNGLUNIFORM3FVPROC>(resolver->resolve("glUniform3fv"));
    glUniform3iv = reinterpret_cast<PFNGLUNIFORM3IVPROC>(resolver->resolve("glUniform3iv"));
    glUniform4fv = reinterpret_cast<PFNGLUNIFORM4FVPROC>(resolver->resolve("glUniform4fv"));
    glUniform4iv = reinterpret_cast<PFNGLUNIFORM4IVPROC>(resolver->resolve("glUniform4iv"));
    glUniformMatrix4fv = reinterpret_cast<PFNGLUNIFORMMATRIX4FVPROC>(resolver->resolve("glUniformMatrix4fv"));
    glUnmapBuffer = reinterpret_cast<PFNGLUNMAPBUFFERPROC>(resolver->resolve("glUnmapBuffer"));
    glUseProgram = reinterpret_cast<PFNGLUSEPROGRAMPROC>(resolver->resolve("glUseProgram"));
    glVertexAttribPointer = reinterpret_cast<PFNGLVERTEXATTRIBPOINTERPROC>(resolver->resolve("glVertexAttribPointer"));
    glViewport = reinterpret_cast<PFNGLVIEWPORTPROC>(resolver->resolve("glViewport"));

    int version = resolver->queryVersion();
    if (version < FunctionResolver::makeVersion(3, 0) && resolver->hasExtension("GL_APPLE_vertex_array_object")) {
        glBindVertexArray = reinterpret_cast<PFNGLBINDVERTEXARRAYPROC>(resolver->resolve("glBindVertexArrayAPPLE"));
        glGenVertexArrays = reinterpret_cast<PFNGLGENVERTEXARRAYSPROC>(resolver->resolve("glGenVertexArraysAPPLE"));
    }
    else if (version >= FunctionResolver::makeVersion(3, 0) || resolver->hasExtension("GL_ARB_vertex_array_object")) {
        glBindVertexArray = reinterpret_cast<PFNGLBINDVERTEXARRAYPROC>(resolver->resolve("glBindVertexArray"));
        glGenVertexArrays = reinterpret_cast<PFNGLGENVERTEXARRAYSPROC>(resolver->resolve("glGenVertexArrays"));
    }
    if (version >= FunctionResolver::makeVersion(3, 1) || resolver->hasExtension("ARB_uniform_buffer_object")) {
        glBindBufferBase = reinterpret_cast<PFNGLBINDBUFFERBASEPROC>(resolver->resolve("glBindBufferBase"));
        glBindBufferRange = reinterpret_cast<PFNGLBINDBUFFERRANGEPROC>(resolver->resolve("glBindBufferRange"));
        glGetActiveUniformBlockiv = reinterpret_cast<PFNGLGETACTIVEUNIFORMBLOCKIVPROC>(resolver->resolve("glGetActiveUniformBlockiv"));
        glGetUniformBlockIndex = reinterpret_cast<PFNGLGETUNIFORMBLOCKINDEXPROC>(resolver->resolve("glGetUniformBlockIndex"));
        glUniformBlockBinding = reinterpret_cast<PFNGLUNIFORMBLOCKBINDINGPROC>(resolver->resolve("glUniformBlockBinding"));
    }
    if (version >= FunctionResolver::makeVersion(3, 2) || resolver->hasExtension("ARB_map_buffer_range")) {
        glMapBufferRange = reinterpret_cast<PFNGLMAPBUFFERRANGEPROC>(resolver->resolve("glMapBufferRange"));
    }
    if (version >= FunctionResolver::makeVersion(3, 2) || resolver->hasExtension("ARB_texture_multisample")) {
        glTexImage2DMultisample = reinterpret_cast<PFNGLTEXIMAGE2DMULTISAMPLEPROC>(resolver->resolve("glTexImage2DMultisample"));
    }
    if (version >= FunctionResolver::makeVersion(3, 3)) {
        glBindSampler = reinterpret_cast<PFNGLBINDSAMPLERPROC>(resolver->resolve("glBindSampler"));
    }
    if (version >= FunctionResolver::makeVersion(4, 2) || resolver->hasExtension("ARB_shader_image_load_store")) {
        glBindImageTexture = reinterpret_cast<PFNGLBINDIMAGETEXTUREPROC>(resolver->resolve("glBindImageTexture"));
    }
    if (resolver->hasExtension("ARB_separate_shader_objects")) {
        glBindProgramPipeline = reinterpret_cast<PFNGLBINDPROGRAMPIPELINEPROC>(resolver->resolve("glBindProgramPipeline"));
        glDeleteProgramPipelines = reinterpret_cast<PFNGLDELETEPROGRAMPIPELINESPROC>(resolver->resolve("glDeleteProgramPipelines"));
        glGenProgramPipelines = reinterpret_cast<PFNGLGENPROGRAMPIPELINESPROC>(resolver->resolve("glGenProgramPipelines"));
        glGetProgramPipelineiv = reinterpret_cast<PFNGLGETPROGRAMPIPELINEIVPROC>(resolver->resolve("glGetProgramPipelineiv"));
        glIsProgramPipeline = reinterpret_cast<PFNGLISPROGRAMPIPELINEPROC>(resolver->resolve("glIsProgramPipeline"));
        glProgramUniform1i = reinterpret_cast<PFNGLPROGRAMUNIFORM1IPROC>(resolver->resolve("glProgramUniform1i"));
        glProgramUniform1iv = reinterpret_cast<PFNGLPROGRAMUNIFORM1IVPROC>(resolver->resolve("glProgramUniform1iv"));
        glProgramUniform1fv = reinterpret_cast<PFNGLPROGRAMUNIFORM1FVPROC>(resolver->resolve("glProgramUniform1fv"));
        glProgramUniform2iv = reinterpret_cast<PFNGLPROGRAMUNIFORM2IVPROC>(resolver->resolve("glProgramUniform2iv"));
        glProgramUniform2fv = reinterpret_cast<PFNGLPROGRAMUNIFORM2FVPROC>(resolver->resolve("glProgramUniform2fv"));
        glProgramUniform3iv = reinterpret_cast<PFNGLPROGRAMUNIFORM3IVPROC>(resolver->resolve("glProgramUniform3iv"));
        glProgramUniform3fv = reinterpret_cast<PFNGLPROGRAMUNIFORM3FVPROC>(resolver->resolve("glProgramUniform3fv"));
        glProgramUniform4iv = reinterpret_cast<PFNGLPROGRAMUNIFORM4IVPROC>(resolver->resolve("glProgramUniform4iv"));
        glProgramUniform4fv = reinterpret_cast<PFNGLPROGRAMUNIFORM4FVPROC>(resolver->resolve("glProgramUniform4fv"));
        glProgramUniformMatrix4fv = reinterpret_cast<PFNGLPROGRAMUNIFORMMATRIX4FVPROC>(resolver->resolve("glProgramUniformMatrix4fv"));
        glUseProgramStages = reinterpret_cast<PFNGLUSEPROGRAMSTAGESPROC>(resolver->resolve("glUseProgramStages"));
    }
    if (resolver->hasExtension("ARB_shader_subroutine")) {
        glGetProgramStageiv = reinterpret_cast<PFNGLGETPROGRAMSTAGEIVPROC>(resolver->resolve("glGetProgramStageiv"));
        glGetSubroutineIndex = reinterpret_cast<PFNGLGETSUBROUTINEINDEXPROC>(resolver->resolve("glGetSubroutineIndex"));
        glGetSubroutineUniformLocation = reinterpret_cast<PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC>(resolver->resolve("glGetSubroutineUniformLocation"));
        glUniformSubroutinesuiv = reinterpret_cast<PFNGLUNIFORMSUBROUTINESUIVPROC>(resolver->resolve("glUniformSubroutinesuiv"));
    }
    if (resolver->hasExtension("ARB_get_program_binary")) {
        glGetProgramBinary = reinterpret_cast<PFNGLGETPROGRAMBINARYPROC>(resolver->resolve("glGetProgramBinary"));
    }
    if (resolver->hasExtension("EXT_direct_state_access")) {
        glTextureParameteriEXT = reinterpret_cast<PFNGLTEXTUREPARAMETERIEXTPROC>(resolver->resolve("glTextureParameteriEXT"));
    }
    if (resolver->hasExtension("NV_framebuffer_multisample_coverage")) {
        glRenderbufferStorageMultisampleCoverageNV = reinterpret_cast<PFNGLRENDERBUFFERSTORAGEMULTISAMPLECOVERAGENVPROC>(resolver->resolve("glRenderbufferStorageMultisampleCoverageNV"));
    }
    if (resolver->hasExtension("NV_path_rendering")) {
        glCoverFillPathNV = reinterpret_cast<PFNGLCOVERFILLPATHNVPROC>(resolver->resolve("glCoverFillPathNV"));
        glCoverStrokePathNV = reinterpret_cast<PFNGLCOVERSTROKEPATHNVPROC>(resolver->resolve("glCoverStrokePathNV"));
        glDeletePathsNV = reinterpret_cast<PFNGLDELETEPATHSNVPROC>(resolver->resolve("glDeletePathsNV"));
        glGenPathsNV = reinterpret_cast<PFNGLGENPATHSNVPROC>(resolver->resolve("glGenPathsNV"));
        glPathParameterfNV = reinterpret_cast<PFNGLPATHPARAMETERFNVPROC>(resolver->resolve("glPathParameterfNV"));
        glPathParameteriNV = reinterpret_cast<PFNGLPATHPARAMETERINVPROC>(resolver->resolve("glPathParameteriNV"));
        glPathStencilDepthOffsetNV = reinterpret_cast<PFNGLPATHSTENCILDEPTHOFFSETNVPROC>(resolver->resolve("glPathStencilDepthOffsetNV"));
        glPathStringNV = reinterpret_cast<PFNGLPATHSTRINGNVPROC>(resolver->resolve("glPathStringNV"));
        glStencilFillPathInstancedNV = reinterpret_cast<PFNGLSTENCILFILLPATHINSTANCEDNVPROC>(resolver->resolve("glStencilFillPathInstancedNV"));
        glStencilFillPathNV = reinterpret_cast<PFNGLSTENCILFILLPATHNVPROC>(resolver->resolve("glStencilFillPathNV"));
        glStencilStrokePathNV = reinterpret_cast<PFNGLSTENCILSTROKEPATHNVPROC>(resolver->resolve("glStencilStrokePathNV"));
    }
}

} /* namespace nvFX */

