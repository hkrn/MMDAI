#pragma once
#ifndef NVFX_GLEW_MOCK_H_
#define NVFX_GLEW_MOCK_H_

#include <stddef.h>

/* GLEW mock for nvFX */
#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif
#ifndef GLAPI
#define GLAPI extern
#endif
#ifndef GLAPIENTRY
#define GLAPIENTRY
#endif

namespace nvFX
{

typedef void GLvoid;
typedef char GLchar;
typedef char GLcharARB;
typedef int GLboolean;
typedef int GLbitfield;
typedef int GLint;
typedef int GLsizei;
typedef unsigned int GLenum;
typedef unsigned int GLuint;
typedef unsigned int GLhandleARB;
typedef float GLfloat;
typedef float GLclampf;

typedef ptrdiff_t GLsizeiptr;
typedef ptrdiff_t GLintptr;

static const GLenum GL_ACTIVE_PROGRAM = 0x8259;
static const GLenum GL_ACTIVE_SUBROUTINES = 0x8DE5;
static const GLenum GL_ACTIVE_SUBROUTINE_MAX_LENGTH = 0x8E48;
static const GLenum GL_ACTIVE_SUBROUTINE_UNIFORMS = 0x8DE6;
static const GLenum GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS = 0x8E47;
static const GLenum GL_ACTIVE_SUBROUTINE_UNIFORM_MAX_LENGTH = 0x8E49;
static const GLenum GL_ACTIVE_UNIFORM_BLOCKS = 0x8A36;
static const GLenum GL_ALPHA = 0x1906;
static const GLenum GL_ALPHA_TEST = 0x0BC0;
static const GLenum GL_ALPHA_TEST_FUNC = 0x0BC1;
static const GLenum GL_ALPHA_TEST_REF = 0x0BC2;
static const GLenum GL_ARB_shader_subroutine = 1;
static const GLenum GL_ARRAY_BUFFER = 0x8892;
static const GLenum GL_BACK = 0x0405;
static const GLenum GL_BGRA = 0x80E1;
static const GLenum GL_BLEND = 0x0BE2;
static const GLenum GL_BLEND_COLOR = 0x8005;
static const GLenum GL_BLEND_DST = 0x0BE0;
static const GLenum GL_BLEND_DST_ALPHA = 0x80CA;
static const GLenum GL_BLEND_DST_RGB = 0x80C8;
static const GLenum GL_BLEND_EQUATION = 0x8009;
static const GLenum GL_BLEND_EQUATION_ALPHA = 0x883D;
static const GLenum GL_BLEND_EQUATION_RGB = GL_BLEND_EQUATION;
// GL_BLEND_EQUATION_RGBA_NV is not found
static const GLenum GL_BLEND_SRC = 0x0BE1;
static const GLenum GL_BLEND_SRC_ALPHA = 0x80CB;
static const GLenum GL_BLEND_SRC_RGB = 0x80C9;
static const GLenum GL_CLAMP = 0x2900;
static const GLenum GL_CLAMP_TO_EDGE = 0x812F;
static const GLenum GL_COLOR_ATTACHMENT0 = 0x8CE0;
static const GLenum GL_COLOR_ATTACHMENT0_EXT = 0x8CE0;
static const GLenum GL_COLOR_ATTACHMENT1 = 0x8CE1;
static const GLenum GL_COLOR_ATTACHMENT2 = 0x8CE2;
static const GLenum GL_COLOR_ATTACHMENT3 = 0x8CE3;
static const GLenum GL_COLOR_ATTACHMENT4 = 0x8CE4;
static const GLenum GL_COLOR_ATTACHMENT5 = 0x8CE5;
static const GLenum GL_COLOR_ATTACHMENT6 = 0x8CE6;
static const GLenum GL_COLOR_BUFFER_BIT = 0x00004000;
static const GLenum GL_COLOR_LOGIC_OP = 0x0BF2;
static const GLenum GL_COLOR_WRITEMASK = 0x0C23;
// GL_COMBINED_LINE_STIPPLE_NV is not found
static const GLenum GL_COMPILE_STATUS = 0x8B81;
static const GLenum GL_COMPUTE_SHADER = 0x91B9;
static const GLenum GL_CULL_FACE = 0x0B44;
static const GLenum GL_CULL_FACE_MODE = 0x0B45;
static const GLenum GL_CURRENT_PROGRAM = 0x8B8D;
static const GLenum GL_DEPTH24_STENCIL8 = 0x88F0;
// GL_DEPTH32F_STENCIL8_ARB is not found
static const GLenum GL_DEPTH_ATTACHMENT = 0x8D00;
static const GLenum GL_DEPTH_BOUNDS_EXT = 0x8891;
static const GLenum GL_DEPTH_BOUNDS_TEST_EXT = 0x8890;
static const GLenum GL_DEPTH_BUFFER_BIT = 0x00000100;
static const GLenum GL_DEPTH_CLAMP = 0x864F;
static const GLenum GL_DEPTH_COMPONENT = 0x1902;
// GL_DEPTH_COMPONENT32F_ARB is not found
static const GLenum GL_DEPTH_COMPONENT32F_NV = 0x8DAB;
static const GLenum GL_DEPTH_FUNC = 0x0B74;
static const GLenum GL_DEPTH_TEST = 0x0B71;
static const GLenum GL_DEPTH_TEXTURE_MODE = 0x884B;
static const GLenum GL_DEPTH_WRITEMASK = 0x0B72;
static const GLenum GL_DITHER = 0x0BD0;
static const GLenum GL_DRAW_FRAMEBUFFER = 0x8CA9;
static const GLenum GL_DYNAMIC_DRAW = 0x88E8;
static const GLenum GL_FALSE = 0;
static const GLenum GL_FLOAT = 0x1406;
static const GLenum GL_FLOAT_RG16_NV = 0x8886;
static const GLenum GL_FLOAT_RG32_NV = 0x8887;
static const GLenum GL_FLOAT_RGBA16_NV = 0x888A;
static const GLenum GL_FRAGMENT_SHADER = 0x8B30;
static const GLenum GL_FRAGMENT_SHADER_ARB = 0x8B30;
static const GLenum GL_FRAMEBUFFER = 0x8D40;
static const GLenum GL_FRAMEBUFFER_COMPLETE = 0x8CD5;
static const GLenum GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT = 0x8CD6;
// GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS is not found
static const GLenum GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT = 0x8CD9;
static const GLenum GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER = 0x8CDB;
static const GLenum GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT = 0x8CDA;
static const GLenum GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT = 0x8CD7;
static const GLenum GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER = 0x8CDC;
static const GLenum GL_FRAMEBUFFER_SRGB = 0x8DB9;
static const GLenum GL_FRAMEBUFFER_UNSUPPORTED = 0x8CDD;
static const GLenum GL_FRONT = 0x0404;
static const GLenum GL_FRONT_FACE = 0x0B46;
static const GLenum GL_GEOMETRY_SHADER_ARB = 0x8DD9;
static const GLenum GL_INT = 0x1404;
static const GLenum GL_INTENSITY16F_ARB = 0x881D;
static const GLenum GL_INTENSITY32F_ARB = 0x8817;
static const GLenum GL_INTENSITY8 = 0x804B;
static const GLenum GL_INVALID_ENUM = 0x0500;
static const GLenum GL_INVALID_OPERATION = 0x0502;
static const GLenum GL_INVALID_VALUE = 0x0501;
static const GLenum GL_LINEAR = 0x2601;
static const GLenum GL_LINEAR_MIPMAP_LINEAR = 0x2703;
static const GLenum GL_LINEAR_MIPMAP_NEAREST = 0x2701;
static const GLenum GL_LINE_SMOOTH = 0x0B20;
static const GLenum GL_LINE_STIPPLE = 0x0B24;
static const GLenum GL_LINE_STIPPLE_PATTERN = 0x0B25;
static const GLenum GL_LINE_STIPPLE_REPEAT = 0x0B26;
static const GLenum GL_LINE_WIDTH = 0x0B21;
static const GLenum GL_LINK_STATUS = 0x8B82;
static const GLenum GL_LOGIC_OP_MODE = 0x0BF0;
static const GLenum GL_LUMINANCE = 0x1909;
static const GLenum GL_LUMINANCE8_ALPHA8 = 0x8045;
static const GLenum GL_LUMINANCE_ALPHA = 0x190A;
static const GLenum GL_LUMINANCE_ALPHA16F_ARB = 0x881F;
static const GLenum GL_LUMINANCE_ALPHA32F_ARB = 0x8819;
static const GLenum GL_MAP_INVALIDATE_RANGE_BIT = 0x0004;
static const GLenum GL_MAP_UNSYNCHRONIZED_BIT = 0x0020;
static const GLenum GL_MAP_WRITE_BIT = 0x0002;
static const GLenum GL_MAX_VERTEX_ATTRIBS = 0x8869;
static const GLenum GL_MULTISAMPLE = 0x809D;
static const GLenum GL_NEAREST = 0x2600;
static const GLenum GL_NEAREST_MIPMAP_LINEAR = 0x2702;
static const GLenum GL_NEAREST_MIPMAP_NEAREST = 0x2700;
static const GLenum GL_NONE = 0;
static const GLenum GL_NO_ERROR = 0;
static const GLenum GL_NV_float_buffer = 1;
static const GLenum GL_OUT_OF_MEMORY = 0x0505;
static const GLenum GL_PATH_CLIENT_LENGTH_NV = 0x907F;
static const GLenum GL_PATH_COVER_DEPTH_FUNC_NV = 0x90BF;
static const GLenum GL_PATH_DASH_OFFSET_NV = 0x907E;
static const GLenum GL_PATH_DASH_OFFSET_RESET_NV = 0x90B4;
static const GLenum GL_PATH_FILL_COVER_MODE_NV = 0x9082;
static const GLenum GL_PATH_FILL_MASK_NV = 0x9081;
static const GLenum GL_PATH_FILL_MODE_NV = 0x9080;
static const GLenum GL_PATH_FOG_GEN_MODE_NV = 0x90AC;
static const GLenum GL_PATH_FORMAT_SVG_NV = 0x9070;
static const GLenum GL_PATH_INITIAL_DASH_CAP_NV = 0x907C;
static const GLenum GL_PATH_INITIAL_END_CAP_NV = 0x9077;
static const GLenum GL_PATH_JOIN_STYLE_NV = 0x9079;
static const GLenum GL_PATH_MITER_LIMIT_NV = 0x907A;
static const GLenum GL_PATH_STENCIL_DEPTH_OFFSET_FACTOR_NV = 0x90BD;
static const GLenum GL_PATH_STENCIL_DEPTH_OFFSET_UNITS_NV = 0x90BE;
static const GLenum GL_PATH_STENCIL_FUNC_NV = 0x90B7;
static const GLenum GL_PATH_STROKE_COVER_MODE_NV = 0x9083;
static const GLenum GL_PATH_STROKE_MASK_NV = 0x9084;
static const GLenum GL_PATH_STROKE_WIDTH_NV = 0x9075;
static const GLenum GL_PATH_TERMINAL_DASH_CAP_NV = 0x907D;
static const GLenum GL_PATH_TERMINAL_END_CAP_NV = 0x9078;
// GL_PER_DRAW_BUFFER_BLEND_ENABLE_NV is not found
// GL_PER_DRAW_BUFFER_BLEND_EQUATION_NV is not found
// GL_PER_DRAW_BUFFER_BLEND_FUNCTION_NV is not found
// GL_PER_DRAW_BUFFER_COLOR_WRITEMASK_NV is not found
static const GLenum GL_POINT_SIZE = 0x0B11;
static const GLenum GL_POINT_SMOOTH = 0x0B10;
static const GLenum GL_POINT_SPRITE = 0x8861;
static const GLenum GL_POINT_SPRITE_COORD_ORIGIN = 0x8CA0;
static const GLenum GL_POLYGON_MODE = 0x0B40;
static const GLenum GL_POLYGON_OFFSET_FACTOR = 0x8038;
static const GLenum GL_POLYGON_OFFSET_FILL = 0x8037;
static const GLenum GL_POLYGON_OFFSET_LINE = 0x2A02;
static const GLenum GL_POLYGON_OFFSET_POINT = 0x2A01;
static const GLenum GL_POLYGON_OFFSET_UNITS = 0x2A00;
static const GLenum GL_POLYGON_SMOOTH = 0x0B41;
static const GLenum GL_POLYGON_STIPPLE = 0x0B42;
static const GLenum GL_PROGRAM_SEPARABLE = 0x8258;
static const GLenum GL_R16 = 0x822A;
static const GLenum GL_R16F = 0x822D;
static const GLenum GL_R16I = 0x8233;
static const GLenum GL_R16UI = 0x8234;
static const GLenum GL_R32F = 0x822E;
static const GLenum GL_R32I = 0x8235;
static const GLenum GL_R32UI = 0x8236;
static const GLenum GL_R8 = 0x8229;
static const GLenum GL_R8I = 0x8231;
static const GLenum GL_R8UI = 0x8232;
static const GLenum GL_RASTERIZER_DISCARD = 0x8C89;
static const GLenum GL_READ_FRAMEBUFFER = 0x8CA8;
static const GLenum GL_READ_ONLY = 0x88B8;
static const GLenum GL_READ_WRITE = 0x88BA;
static const GLenum GL_RED = 0x1903;
static const GLenum GL_RED_INTEGER = 0x8D94;
static const GLenum GL_RENDERBUFFER = 0x8D41;
static const GLenum GL_RENDERBUFFER_COLOR_SAMPLES_NV = 0x8E10;
static const GLenum GL_RENDERBUFFER_COVERAGE_SAMPLES_NV = 0x8CAB;
static const GLenum GL_RENDERBUFFER_SAMPLES = 0x8CAB;
static const GLenum GL_REPEAT = 0x2901;
static const GLenum GL_RG = 0x8227;
static const GLenum GL_RG16 = 0x822C;
static const GLenum GL_RG16F = 0x822F;
static const GLenum GL_RG16I = 0x8239;
static const GLenum GL_RG16UI = 0x823A;
static const GLenum GL_RG32F = 0x8230;
static const GLenum GL_RG32I = 0x823B;
static const GLenum GL_RG32UI = 0x823C;
static const GLenum GL_RG8 = 0x822B;
static const GLenum GL_RG8I = 0x8237;
static const GLenum GL_RG8UI = 0x8238;
static const GLenum GL_RGB = 0x1907;
static const GLenum GL_RGB16F_ARB = 0x881B;
static const GLenum GL_RGB32F_ARB = 0x8815;
static const GLenum GL_RGB8 = 0x8051;
static const GLenum GL_RGBA = 0x1908;
static const GLenum GL_RGBA16F_ARB = 0x881A;
static const GLenum GL_RGBA32F_ARB = 0x8814;
static const GLenum GL_RGBA8 = 0x8058;
static const GLenum GL_RGBA8UI = 0x8D7C;
static const GLenum GL_RGBA_INTEGER = 0x8D99;
static const GLenum GL_RG_INTEGER = 0x8228;
static const GLenum GL_SAMPLE_ALPHA_TO_COVERAGE = 0x809E;
static const GLenum GL_SAMPLE_ALPHA_TO_ONE = 0x809F;
static const GLenum GL_SAMPLE_MASK = 0x8E51;
static const GLenum GL_SAMPLE_MASK_VALUE = 0x8E52;
static const GLenum GL_STACK_OVERFLOW = 0x0503;
static const GLenum GL_STACK_UNDERFLOW = 0x0504;
static const GLenum GL_STENCIL_ATTACHMENT = 0x8D20;
static const GLenum GL_STENCIL_BACK_FAIL = 0x8801;
static const GLenum GL_STENCIL_BACK_FUNC = 0x8800;
static const GLenum GL_STENCIL_BACK_PASS_DEPTH_FAIL = 0x8802;
static const GLenum GL_STENCIL_BACK_PASS_DEPTH_PASS = 0x8803;
static const GLenum GL_STENCIL_BACK_REF = 0x8CA3;
static const GLenum GL_STENCIL_BACK_VALUE_MASK = 0x8CA4;
static const GLenum GL_STENCIL_BACK_WRITEMASK = 0x8CA5;
static const GLenum GL_STENCIL_BUFFER_BIT = 0x00000400;
static const GLenum GL_STENCIL_FAIL = 0x0B94;
// GL_STENCIL_FRONT_FAIL is not found
// GL_STENCIL_FRONT_FUNC is not found
// GL_STENCIL_FRONT_PASS_DEPTH_FAIL is not found
// GL_STENCIL_FRONT_PASS_DEPTH_PASS is not found
// GL_STENCIL_FRONT_REF is not found
// GL_STENCIL_FRONT_VALUE_MASK is not found
// GL_STENCIL_FRONT_WRITEMASK is not found
static const GLenum GL_STENCIL_FUNC = 0x0B92;
static const GLenum GL_STENCIL_PASS_DEPTH_FAIL = 0x0B95;
static const GLenum GL_STENCIL_PASS_DEPTH_PASS = 0x0B96;
static const GLenum GL_STENCIL_REF = 0x0B97;
static const GLenum GL_STENCIL_TEST = 0x0B90;
static const GLenum GL_STENCIL_VALUE_MASK = 0x0B93;
static const GLenum GL_STENCIL_WRITEMASK = 0x0B98;
static const GLenum GL_TESS_CONTROL_SHADER = 0x8E88;
static const GLenum GL_TESS_EVALUATION_SHADER = 0x8E87;
static const GLenum GL_TEXTURE0 = 0x84C0;
static const GLenum GL_TEXTURE_1D = 0x0DE0;
static const GLenum GL_TEXTURE_2D = 0x0DE1;
static const GLenum GL_TEXTURE_2D_MULTISAMPLE = 0x9100;
static const GLenum GL_TEXTURE_3D = 0x806F;
static const GLenum GL_TEXTURE_CUBE_MAP = 0x8513;
static const GLenum GL_TEXTURE_MAG_FILTER = 0x2800;
static const GLenum GL_TEXTURE_MIN_FILTER = 0x2801;
static const GLenum GL_TEXTURE_RECTANGLE = 0x84F5;
static const GLenum GL_TEXTURE_WRAP_R = 0x8072;
static const GLenum GL_TEXTURE_WRAP_S = 0x2802;
static const GLenum GL_TEXTURE_WRAP_T = 0x2803;
static const GLenum GL_TRIANGLE_STRIP = 0x0005;
static const GLenum GL_TRUE = 1;
static const GLenum GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS = 0x8A42;
static const GLenum GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES = 0x8A43;
static const GLenum GL_UNIFORM_BLOCK_BINDING = 0x8A3F;
static const GLenum GL_UNIFORM_BUFFER = 0x8A11;
static const GLenum GL_UNSIGNED_BYTE = 0x1401;
static const GLenum GL_UNSIGNED_INT = 0x1405;
static const GLenum GL_UTF8_NV = 0x909A;
static const GLenum GL_VERTEX_SHADER = 0x8B31;
static const GLenum GL_VERTEX_SHADER_ARB = 0x8B31;
static const GLenum GL_VERTEX_SHADER_BIT = 0x00000001;
static const GLenum GL_WRITE_ONLY = 0x88B9;

typedef void (APIENTRYP PFNGLACTIVETEXTUREPROC) (GLenum texture);
typedef void (APIENTRYP PFNGLALPHAFUNCPROC)(GLenum, GLfloat);
typedef void (APIENTRYP PFNGLATTACHOBJECTARBPROC) (GLhandleARB containerObj, GLhandleARB obj);
typedef void (APIENTRYP PFNGLATTACHSHADERPROC) (GLuint program, GLuint shader);
typedef void (APIENTRYP PFNGLBINDATTRIBLOCATIONPROC) (GLuint program, GLuint index, const GLchar* name);
typedef void (APIENTRYP PFNGLBINDBUFFERPROC) (GLenum target, GLuint buffer);
typedef void (APIENTRYP PFNGLBINDBUFFERBASEPROC) (GLenum target, GLuint index, GLuint buffer);
typedef void (APIENTRYP PFNGLBINDBUFFERRANGEPROC) (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size);
typedef void (APIENTRYP PFNGLBINDFRAMEBUFFERPROC) (GLenum target, GLuint framebuffer);
typedef void (APIENTRYP PFNGLBINDIMAGETEXTUREPROC) (GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
typedef void (APIENTRYP PFNGLBINDPROGRAMPIPELINEPROC) (GLuint pipeline);
typedef void (APIENTRYP PFNGLBINDRENDERBUFFERPROC) (GLenum target, GLuint renderbuffer);
typedef void (APIENTRYP PFNGLBINDSAMPLERPROC) (GLuint unit, GLuint sampler);
typedef void (APIENTRYP PFNGLBINDTEXTUREPROC)(GLenum, GLuint);
typedef void (APIENTRYP PFNGLBINDVERTEXARRAYPROC) (GLuint array);
typedef void (APIENTRYP PFNGLBLENDCOLORPROC) (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
typedef void (APIENTRYP PFNGLBLENDEQUATIONSEPARATEPROC) (GLenum, GLenum);
typedef void (APIENTRYP PFNGLBLENDFUNCPROC)(GLenum, GLenum);
typedef void (APIENTRYP PFNGLBLENDFUNCSEPARATEPROC) (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
typedef void (APIENTRYP PFNGLBLITFRAMEBUFFERPROC) (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter);
typedef void (APIENTRYP PFNGLBUFFERDATAPROC) (GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
typedef void (APIENTRYP PFNGLBUFFERSUBDATAPROC) (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data);
typedef GLenum (APIENTRYP PFNGLCHECKFRAMEBUFFERSTATUSPROC) (GLenum target);
typedef void (APIENTRYP PFNGLCLEARPROC)(GLbitfield);
typedef void (APIENTRYP PFNGLCLEARCOLORPROC)(GLfloat, GLfloat, GLfloat, GLfloat);
typedef void (APIENTRYP PFNGLCOLORMASKPROC)(GLboolean, GLboolean, GLboolean, GLboolean);
typedef void (APIENTRYP PFNGLCOMPILESHADERPROC) (GLuint shader);
typedef void (APIENTRYP PFNGLCOVERFILLPATHNVPROC) (GLuint path, GLenum coverMode);
typedef void (APIENTRYP PFNGLCOVERSTROKEPATHNVPROC) (GLuint name, GLenum coverMode);
typedef GLuint (APIENTRYP PFNGLCREATEPROGRAMPROC) (void);
typedef GLuint (APIENTRYP PFNGLCREATESHADERPROC) (GLenum type);
typedef GLhandleARB (APIENTRYP PFNGLCREATESHADEROBJECTARBPROC) (GLenum shaderType);
typedef void (APIENTRYP PFNGLCULLFACEPROC)(GLenum);
typedef void (APIENTRYP PFNGLDELETEBUFFERSPROC) (GLsizei n, const GLuint* buffers);
typedef void (APIENTRYP PFNGLDELETEFRAMEBUFFERSPROC) (GLsizei n, const GLuint* framebuffers);
typedef void (APIENTRYP PFNGLDELETEOBJECTARBPROC) (GLhandleARB obj);
typedef void (APIENTRYP PFNGLDELETEPATHSNVPROC) (GLuint path, GLsizei range);
typedef void (APIENTRYP PFNGLDELETEPROGRAMPROC) (GLuint program);
typedef void (APIENTRYP PFNGLDELETEPROGRAMPIPELINESPROC) (GLsizei n, const GLuint* pipelines);
typedef void (APIENTRYP PFNGLDELETERENDERBUFFERSPROC) (GLsizei n, const GLuint* renderbuffers);
typedef void (APIENTRYP PFNGLDELETESHADERPROC) (GLuint shader);
typedef void (APIENTRYP PFNGLDELETETEXTURESPROC)(GLsizei, const GLuint *);
typedef void (APIENTRYP PFNGLDEPTHFUNCPROC)(GLenum);
typedef void (APIENTRYP PFNGLDEPTHMASKPROC)(GLboolean);
typedef void (APIENTRYP PFNGLDETACHOBJECTARBPROC) (GLhandleARB containerObj, GLhandleARB attachedObj);
typedef void (APIENTRYP PFNGLDETACHSHADERPROC) (GLuint program, GLuint shader);
typedef void (APIENTRYP PFNGLDISABLEPROC)(GLenum);
typedef void (APIENTRYP PFNGLDISABLEVERTEXATTRIBARRAYPROC) (GLuint);
typedef void (APIENTRYP PFNGLDRAWARRAYSPROC)(GLenum, GLint, GLsizei);
typedef void (APIENTRYP PFNGLDRAWBUFFERPROC)(GLenum);
typedef void (APIENTRYP PFNGLDRAWBUFFERSPROC) (GLsizei n, const GLenum* bufs);
typedef void (APIENTRYP PFNGLENABLEPROC)(GLenum);
typedef void (APIENTRYP PFNGLENABLEVERTEXATTRIBARRAYPROC) (GLuint);
typedef void (APIENTRYP PFNGLFRAMEBUFFERRENDERBUFFERPROC) (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void (APIENTRYP PFNGLFRAMEBUFFERTEXTURE2DPROC) (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
typedef void (APIENTRYP PFNGLFRONTFACEPROC)(GLenum);
typedef void (APIENTRYP PFNGLGENBUFFERSPROC) (GLsizei n, GLuint* buffers);
typedef void (APIENTRYP PFNGLGENFRAMEBUFFERSPROC) (GLsizei n, GLuint* framebuffers);
typedef GLuint (APIENTRYP PFNGLGENPATHSNVPROC) (GLsizei range);
typedef void (APIENTRYP PFNGLGENPROGRAMPIPELINESPROC) (GLsizei n, GLuint* pipelines);
typedef void (APIENTRYP PFNGLGENRENDERBUFFERSPROC) (GLsizei n, GLuint* renderbuffers);
typedef void (APIENTRYP PFNGLGENTEXTURESPROC)(GLsizei, GLuint *);
typedef void (APIENTRYP PFNGLGENVERTEXARRAYSPROC) (GLsizei n, GLuint* arrays);
typedef void (APIENTRYP PFNGLGETACTIVEUNIFORMPROC) (GLuint program, GLuint index, GLsizei maxLength, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
typedef void (APIENTRYP PFNGLGETACTIVEUNIFORMBLOCKIVPROC) (GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params);
typedef GLint (APIENTRYP PFNGLGETATTRIBLOCATIONPROC) (GLuint program, const GLchar* name);
typedef GLenum (APIENTRYP PFNGLGETERRORPROC)();
typedef void (APIENTRYP PFNGLGETINFOLOGARBPROC) (GLhandleARB obj, GLsizei maxLength, GLsizei* length, GLcharARB *infoLog);
typedef void (APIENTRYP PFNGLGETINTEGERVPROC)(GLenum, GLint *);
typedef void (APIENTRYP PFNGLGETPROGRAMINFOLOGPROC) (GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef void (APIENTRYP PFNGLGETPROGRAMPIPELINEIVPROC) (GLuint pipeline, GLenum pname, GLint* params);
typedef void (APIENTRYP PFNGLGETPROGRAMSTAGEIVPROC) (GLuint program, GLenum shadertype, GLenum pname, GLint* values);
typedef void (APIENTRYP PFNGLGETPROGRAMIVPROC) (GLuint program, GLenum pname, GLint* param);
typedef void (APIENTRYP PFNGLGETRENDERBUFFERPARAMETERIVPROC) (GLenum target, GLenum pname, GLint* params);
typedef void (APIENTRYP PFNGLGETSHADERINFOLOGPROC) (GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef void (APIENTRYP PFNGLGETSHADERIVPROC) (GLuint shader, GLenum pname, GLint* param);
typedef GLuint (APIENTRYP PFNGLGETSUBROUTINEINDEXPROC) (GLuint program, GLenum shadertype, const GLchar* name);
typedef GLint (APIENTRYP PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC) (GLuint program, GLenum shadertype, const GLchar* name);
typedef GLuint (APIENTRYP PFNGLGETUNIFORMBLOCKINDEXPROC) (GLuint program, const GLchar* uniformBlockName);
typedef GLint (APIENTRYP PFNGLGETUNIFORMLOCATIONPROC) (GLuint program, const GLchar* name);
typedef GLboolean (APIENTRYP PFNGLISPROGRAMPIPELINEPROC) (GLuint pipeline);
typedef void (APIENTRYP PFNGLLINEWIDTHPROC)(GLfloat);
typedef void (APIENTRYP PFNGLLINKPROGRAMPROC) (GLuint program);
typedef void (APIENTRYP PFNGLLOGICOPPROC)(GLenum);
typedef GLvoid* (APIENTRYP PFNGLMAPBUFFERPROC) (GLenum target, GLenum access);
typedef GLvoid * (APIENTRYP PFNGLMAPBUFFERRANGEPROC) (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);
typedef void (APIENTRYP PFNGLPATHPARAMETERFNVPROC) (GLuint path, GLenum pname, GLfloat value);
typedef void (APIENTRYP PFNGLPATHPARAMETERINVPROC) (GLuint path, GLenum pname, GLint value);
typedef void (APIENTRYP PFNGLPATHSTENCILDEPTHOFFSETNVPROC) (GLfloat factor, GLfloat units);
typedef void (APIENTRYP PFNGLPATHSTRINGNVPROC) (GLuint path, GLenum format, GLsizei length, const void* pathString);
typedef void (APIENTRYP PFNGLPOINTSIZEPROC)(GLfloat);
typedef void (APIENTRYP PFNGLPOLYGONMODEPROC)(GLenum, GLenum);
typedef void (APIENTRYP PFNGLPOLYGONOFFSETPROC)(GLfloat, GLfloat);
typedef void (APIENTRYP PFNGLPROGRAMPARAMETERIPROC) (GLuint program, GLenum pname, GLint value);
typedef void (APIENTRYP PFNGLRENDERBUFFERSTORAGEPROC) (GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (APIENTRYP PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC) (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (APIENTRYP PFNGLRENDERBUFFERSTORAGEMULTISAMPLECOVERAGENVPROC) (GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLenum internalformat, GLsizei width, GLsizei height);
typedef void (APIENTRYP PFNGLSHADERSOURCEPROC) (GLuint shader, GLsizei count, const GLchar** strings, const GLint* lengths);
typedef void (APIENTRYP PFNGLSTENCILFILLPATHINSTANCEDNVPROC) (GLsizei numPaths, GLenum pathNameType, const void* paths, GLuint pathBase, GLenum fillMode, GLuint mask, GLenum transformType, const GLfloat *transformValues);
typedef void (APIENTRYP PFNGLSTENCILFILLPATHNVPROC) (GLuint path, GLenum fillMode, GLuint mask);
typedef void (APIENTRYP PFNGLSTENCILFUNCSEPARATEPROC) (GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask);
typedef void (APIENTRYP PFNGLSTENCILOPSEPARATEPROC) (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
typedef void (APIENTRYP PFNGLSTENCILSTROKEPATHNVPROC) (GLuint path, GLint reference, GLuint mask);
typedef void (APIENTRYP PFNGLTEXIMAGE1DPROC)(GLenum, GLint, GLint, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
typedef void (APIENTRYP PFNGLTEXIMAGE2DPROC)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const GLvoid *);
typedef void (APIENTRYP PFNGLTEXIMAGE2DMULTISAMPLEPROC) (GLenum target, GLsizei samples, GLint internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations);
typedef void (APIENTRYP PFNGLTEXIMAGE3DPROC) (GLenum target, GLint level, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
typedef void (APIENTRYP PFNGLTEXPARAMETERFPROC)(GLenum, GLenum, GLfloat);
typedef void (APIENTRYP PFNGLTEXPARAMETERIPROC)(GLenum, GLenum, GLint);
typedef void (APIENTRYP PFNGLTEXTUREPARAMETERIEXTPROC) (GLuint texture, GLenum target, GLenum pname, GLint param);
typedef void (APIENTRYP PFNGLUNIFORM1FPROC) (GLint location, GLfloat v0);
typedef void (APIENTRYP PFNGLUNIFORM1FVPROC) (GLint location, GLsizei count, const GLfloat* value);
typedef void (APIENTRYP PFNGLUNIFORM1IPROC) (GLint location, GLint v0);
typedef void (APIENTRYP PFNGLUNIFORM1IVPROC) (GLint location, GLsizei count, const GLint* value);
typedef void (APIENTRYP PFNGLUNIFORM2FVPROC) (GLint location, GLsizei count, const GLfloat* value);
typedef void (APIENTRYP PFNGLUNIFORM2IVPROC) (GLint location, GLsizei count, const GLint* value);
typedef void (APIENTRYP PFNGLUNIFORM3FVPROC) (GLint location, GLsizei count, const GLfloat* value);
typedef void (APIENTRYP PFNGLUNIFORM3IVPROC) (GLint location, GLsizei count, const GLint* value);
typedef void (APIENTRYP PFNGLUNIFORM4FVPROC) (GLint location, GLsizei count, const GLfloat* value);
typedef void (APIENTRYP PFNGLUNIFORM4IVPROC) (GLint location, GLsizei count, const GLint* value);
typedef void (APIENTRYP PFNGLUNIFORMBLOCKBINDINGPROC) (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding);
typedef void (APIENTRYP PFNGLUNIFORMSUBROUTINESUIVPROC) (GLenum shadertype, GLsizei count, const GLuint* indices);
typedef GLboolean (APIENTRYP PFNGLUNMAPBUFFERPROC) (GLenum target);
typedef void (APIENTRYP PFNGLUSEPROGRAMPROC) (GLuint program);
typedef void (APIENTRYP PFNGLUSEPROGRAMSTAGESPROC) (GLuint pipeline, GLbitfield stages, GLuint program);
typedef void (APIENTRYP PFNGLVERTEXATTRIBPOINTERPROC) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer);
typedef void (APIENTRYP PFNGLVIEWPORTPROC)(GLint, GLint, GLsizei, GLsizei);
extern PFNGLACTIVETEXTUREPROC glActiveTexture;
extern PFNGLALPHAFUNCPROC glAlphaFunc;
extern PFNGLATTACHOBJECTARBPROC glAttachObjectARB;
extern PFNGLATTACHSHADERPROC glAttachShader;
extern PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation;
extern PFNGLBINDBUFFERPROC glBindBuffer;
extern PFNGLBINDBUFFERBASEPROC glBindBufferBase;
extern PFNGLBINDBUFFERRANGEPROC glBindBufferRange;
extern PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
extern PFNGLBINDIMAGETEXTUREPROC glBindImageTexture;
extern PFNGLBINDPROGRAMPIPELINEPROC glBindProgramPipeline;
extern PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
extern PFNGLBINDSAMPLERPROC glBindSampler;
extern PFNGLBINDTEXTUREPROC glBindTexture;
extern PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
extern PFNGLBLENDCOLORPROC glBlendColor;
extern PFNGLBLENDEQUATIONSEPARATEPROC glBlendEquationSeparate;
extern PFNGLBLENDFUNCPROC glBlendFunc;
extern PFNGLBLENDFUNCSEPARATEPROC glBlendFuncSeparate;
extern PFNGLBLITFRAMEBUFFERPROC glBlitFramebuffer;
extern PFNGLBUFFERDATAPROC glBufferData;
extern PFNGLBUFFERSUBDATAPROC glBufferSubData;
extern PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
extern PFNGLCLEARPROC glClear;
extern PFNGLCLEARCOLORPROC glClearColor;
extern PFNGLCOLORMASKPROC glColorMask;
extern PFNGLCOMPILESHADERPROC glCompileShader;
extern PFNGLCOVERFILLPATHNVPROC glCoverFillPathNV;
extern PFNGLCOVERSTROKEPATHNVPROC glCoverStrokePathNV;
extern PFNGLCREATEPROGRAMPROC glCreateProgram;
extern PFNGLCREATESHADERPROC glCreateShader;
extern PFNGLCREATESHADEROBJECTARBPROC glCreateShaderObjectARB;
extern PFNGLCULLFACEPROC glCullFace;
extern PFNGLDELETEBUFFERSPROC glDeleteBuffers;
extern PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
extern PFNGLDELETEOBJECTARBPROC glDeleteObjectARB;
extern PFNGLDELETEPATHSNVPROC glDeletePathsNV;
extern PFNGLDELETEPROGRAMPROC glDeleteProgram;
extern PFNGLDELETEPROGRAMPIPELINESPROC glDeleteProgramPipelines;
extern PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
extern PFNGLDELETESHADERPROC glDeleteShader;
extern PFNGLDELETETEXTURESPROC glDeleteTextures;
extern PFNGLDEPTHFUNCPROC glDepthFunc;
extern PFNGLDEPTHMASKPROC glDepthMask;
extern PFNGLDETACHOBJECTARBPROC glDetachObjectARB;
extern PFNGLDETACHSHADERPROC glDetachShader;
extern PFNGLDISABLEPROC glDisable;
extern PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
extern PFNGLDRAWARRAYSPROC glDrawArrays;
extern PFNGLDRAWBUFFERPROC glDrawBuffer;
extern PFNGLDRAWBUFFERSPROC glDrawBuffers;
extern PFNGLENABLEPROC glEnable;
extern PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
extern PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;
extern PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
extern PFNGLFRONTFACEPROC glFrontFace;
extern PFNGLGENBUFFERSPROC glGenBuffers;
extern PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
extern PFNGLGENPATHSNVPROC glGenPathsNV;
extern PFNGLGENPROGRAMPIPELINESPROC glGenProgramPipelines;
extern PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
extern PFNGLGENTEXTURESPROC glGenTextures;
extern PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
extern PFNGLGETACTIVEUNIFORMPROC glGetActiveUniform;
extern PFNGLGETACTIVEUNIFORMBLOCKIVPROC glGetActiveUniformBlockiv;
extern PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
extern PFNGLGETERRORPROC glGetError;
extern PFNGLGETINFOLOGARBPROC glGetInfoLogARB;
extern PFNGLGETINTEGERVPROC glGetIntegerv;
extern PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
extern PFNGLGETPROGRAMPIPELINEIVPROC glGetProgramPipelineiv;
extern PFNGLGETPROGRAMSTAGEIVPROC glGetProgramStageiv;
extern PFNGLGETPROGRAMIVPROC glGetProgramiv;
extern PFNGLGETRENDERBUFFERPARAMETERIVPROC glGetRenderbufferParameteriv;
extern PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
extern PFNGLGETSHADERIVPROC glGetShaderiv;
extern PFNGLGETSUBROUTINEINDEXPROC glGetSubroutineIndex;
extern PFNGLGETSUBROUTINEUNIFORMLOCATIONPROC glGetSubroutineUniformLocation;
extern PFNGLGETUNIFORMBLOCKINDEXPROC glGetUniformBlockIndex;
extern PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
extern PFNGLISPROGRAMPIPELINEPROC glIsProgramPipeline;
extern PFNGLLINEWIDTHPROC glLineWidth;
extern PFNGLLINKPROGRAMPROC glLinkProgram;
extern PFNGLLOGICOPPROC glLogicOp;
extern PFNGLMAPBUFFERPROC glMapBuffer;
extern PFNGLMAPBUFFERRANGEPROC glMapBufferRange;
extern PFNGLPATHPARAMETERFNVPROC glPathParameterfNV;
extern PFNGLPATHPARAMETERINVPROC glPathParameteriNV;
extern PFNGLPATHSTENCILDEPTHOFFSETNVPROC glPathStencilDepthOffsetNV;
extern PFNGLPATHSTRINGNVPROC glPathStringNV;
extern PFNGLPOINTSIZEPROC glPointSize;
extern PFNGLPOLYGONMODEPROC glPolygonMode;
extern PFNGLPOLYGONOFFSETPROC glPolygonOffset;
extern PFNGLPROGRAMPARAMETERIPROC glProgramParameteri;
extern PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLEPROC glRenderbufferStorageMultisample;
extern PFNGLRENDERBUFFERSTORAGEMULTISAMPLECOVERAGENVPROC glRenderbufferStorageMultisampleCoverageNV;
extern PFNGLSHADERSOURCEPROC glShaderSource;
extern PFNGLSTENCILFILLPATHINSTANCEDNVPROC glStencilFillPathInstancedNV;
extern PFNGLSTENCILFILLPATHNVPROC glStencilFillPathNV;
extern PFNGLSTENCILFUNCSEPARATEPROC glStencilFuncSeparate;
extern PFNGLSTENCILOPSEPARATEPROC glStencilOpSeparate;
extern PFNGLSTENCILSTROKEPATHNVPROC glStencilStrokePathNV;
extern PFNGLTEXIMAGE1DPROC glTexImage1D;
extern PFNGLTEXIMAGE2DPROC glTexImage2D;
extern PFNGLTEXIMAGE2DMULTISAMPLEPROC glTexImage2DMultisample;
extern PFNGLTEXIMAGE3DPROC glTexImage3D;
extern PFNGLTEXPARAMETERFPROC glTexParameterf;
extern PFNGLTEXPARAMETERIPROC glTexParameteri;
extern PFNGLTEXTUREPARAMETERIEXTPROC glTextureParameteriEXT;
extern PFNGLUNIFORM1FPROC glUniform1f;
extern PFNGLUNIFORM1FVPROC glUniform1fv;
extern PFNGLUNIFORM1IPROC glUniform1i;
extern PFNGLUNIFORM1IVPROC glUniform1iv;
extern PFNGLUNIFORM2FVPROC glUniform2fv;
extern PFNGLUNIFORM2IVPROC glUniform2iv;
extern PFNGLUNIFORM3FVPROC glUniform3fv;
extern PFNGLUNIFORM3IVPROC glUniform3iv;
extern PFNGLUNIFORM4FVPROC glUniform4fv;
extern PFNGLUNIFORM4IVPROC glUniform4iv;
extern PFNGLUNIFORMBLOCKBINDINGPROC glUniformBlockBinding;
extern PFNGLUNIFORMSUBROUTINESUIVPROC glUniformSubroutinesuiv;
extern PFNGLUNMAPBUFFERPROC glUnmapBuffer;
extern PFNGLUSEPROGRAMPROC glUseProgram;
extern PFNGLUSEPROGRAMSTAGESPROC glUseProgramStages;
extern PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
extern PFNGLVIEWPORTPROC glViewport;

typedef void (APIENTRYP PFNGLUNIFORMMATRIX4FVPROC) (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM1IPROC) (GLuint program, GLint location, GLint v0);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM1IVPROC) (GLuint program, GLint location, GLsizei count, const GLint *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM1FVPROC) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM2IVPROC) (GLuint program, GLint location, GLsizei count, const GLint *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM2FVPROC) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM3IVPROC) (GLuint program, GLint location, GLsizei count, const GLint *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM3FVPROC) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM4IVPROC) (GLuint program, GLint location, GLsizei count, const GLint *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORM4FVPROC) (GLuint program, GLint location, GLsizei count, const GLfloat *value);
typedef void (APIENTRYP PFNGLPROGRAMUNIFORMMATRIX4FVPROC) (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

extern PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
extern PFNGLPROGRAMUNIFORM1IPROC glProgramUniform1i;
extern PFNGLPROGRAMUNIFORM1IVPROC glProgramUniform1iv;
extern PFNGLPROGRAMUNIFORM1FVPROC glProgramUniform1fv;
extern PFNGLPROGRAMUNIFORM2IVPROC glProgramUniform2iv;
extern PFNGLPROGRAMUNIFORM2FVPROC glProgramUniform2fv;
extern PFNGLPROGRAMUNIFORM3IVPROC glProgramUniform3iv;
extern PFNGLPROGRAMUNIFORM3FVPROC glProgramUniform3fv;
extern PFNGLPROGRAMUNIFORM4IVPROC glProgramUniform4iv;
extern PFNGLPROGRAMUNIFORM4FVPROC glProgramUniform4fv;
extern PFNGLPROGRAMUNIFORMMATRIX4FVPROC glProgramUniformMatrix4fv;

// glDispatchComputeARB is not found
// glGetProcAddress is not found
// glPROGRAMUNIFORM is not found
// glPROGRAMUNIFORMMATRIXV is not found
// glPROGRAMUNIFORMV is not found
// glXGetProcAddress is not found

struct FunctionResolver {
    virtual ~FunctionResolver() {}
    virtual bool hasExtension(const char *name) const = 0;
    virtual void *resolve(const char *name) const = 0;
};

GLAPI void initializeOpenGLFunctions(const FunctionResolver *resolver);

} /* namespace nvFX */

#endif
