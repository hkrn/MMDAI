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

#include <vpvl2/vpvl2.h>
#include <vpvl2/internal/util.h>
#include <vpvl2/extensions/fx/EffectFX5.h>

#include <vpvl2/extensions/gl/ShaderProgram.h>

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#pragma clang diagnostic ignored "-Wunused-function"
#endif
extern "C" {
#include <toGLSL.h>
}
#ifdef __clang__
#pragma clang diagnostic push
#endif

namespace {

using namespace vpvl2;

#pragma pack(push, 1)

/**
 * based on Effect Binary Format spec in Effects11
 * @see http://blogs.msdn.com/b/chuckw/archive/2012/10/24/effects-for-direct3d-11-update.aspx
 */

enum VariableType {
    kInvalidVariable,
    kNumericVariable,
    kObjectVariable,
    kStructVariable,
    kInterfaceVariable
};

enum ScalarType {
    kInvalidScalar,
    kFloatScalar,
    kIntegerScalar,
    kUIntegerScalar,
    kBooleanScalar,
    kCount
};

enum ObjectType {
    kInvalidObject,
    kString,
    kBlend,
    kDepthStencil,
    kRasterizer,
    kPixelShader,
    kVertexShader,
    kGeometryShader,
    kGeometryShaderStreamOutput,
    kTexture,
    kTexture1D,
    kTexture1DArray,
    kTexture2D,
    kTexture2DArray,
    kTexture2DMultisample,
    kTexture2DMultisampleArray,
    kTexture3D,
    kTextureCube,
    kConstantBuffer,
    kRenderColorTarget,
    kRenderDepthStencilTarget,
    kSampler,
    kBuffer,
    kTextureCubeArray,
    kCountObject,
    kPixelShader5,
    kVertexShader5,
    kGeometryShader5,
    kComputeShader5,
    kHullShader5,
    kDomainShader5,
    kRWTexture1D,
    kRWTexture1DArray,
    kRWTexture2D,
    kRWTexture2DArray,
    kRWTexture3D,
    kRWBuffer,
    kByteAddressBuffer,
    kRWByteAddressBuffer,
    kStructuredBuffer,
    kRWStructuredBuffer,
    kRWStructuredBufferAlloc,
    kRWStructuredBufferConsume,
    kAppendStructuredBuffer,
    kConsumeStructuredBuffer,
    kObjectTypeBool = 0x100,
    kObjectTypeFloat,
    kObjectTypeUInt8,
    kObjectTypeUInt32
};

enum NumericLayoutType {
    kScalarLayout,
    kVectorLayout,
    kMatrixLayout
};

enum AssignmentType {
    kInvalidAssignment,
    kConstantAssignment,
    kVariableAssignment,
    kConstantIndexAssignment,
    kVariableIndexAssignment,
    kExpressionIndexAssignment,
    kExpressionAssignment,
    kInlineShaderAssignment
};

struct VariableCounterBlock {
    uint32_t numConstants;
    uint32_t numNumericVariables;
    uint32_t numObjectVariables;
};

struct Header {
    uint32_t signature;
    VariableCounterBlock effect;
    VariableCounterBlock pool;
    uint32_t numTechniques;
    uint32_t unstructuredSize;
    uint32_t numStrings;
    uint32_t numShaders;
    uint32_t numDepthStencilBlocks;
    uint32_t numBlendStateBlocks;
    uint32_t numRasterizerStateBlocks;
    uint32_t numSamplers;
    uint32_t numRenderColorTargets;
    uint32_t numRenderDepthStencilTargets;
    uint32_t numTotalShaders;
    uint32_t numInlineShaders;
    uint32_t numGroups;
    uint32_t numUnorderedAccessViews;
    uint32_t numInterfaceVariables;
    uint32_t numInterfaceVariableElements;
    uint32_t numClassInstanceElements;
    bool isValidSignature() const {
        return signature == 0xfeff1001 || signature == 0xfeff1011 || signature == 0xfeff2001;
    }
};

struct BaseElement {
    uint32_t offsetName;
};

struct ConstantBufferElement : BaseElement {
    uint32_t size;
    uint32_t flags;
    uint32_t numVariables;
    uint32_t explicitBindPoint;
};

struct TypedBaseElement : BaseElement {
    uint32_t offsetType;
};

struct AnnotationElement : TypedBaseElement {
};

struct NumericVarableElement : TypedBaseElement {
    uint32_t offsetSemantic;
    uint32_t offsetSelf;
    uint32_t offsetDefaultValue;
    uint32_t flags;
};

struct InterfaceVariableElement : TypedBaseElement {
    uint32_t offsetDefaultValue;
    uint32_t flags;
};

struct InterfaceInitalizer {
    uint32_t offsetInstanceName;
    uint32_t arrayIndex;
};

struct ObjectVariable : TypedBaseElement {
    uint32_t offsetSemantic;
    uint32_t explicitBindPoint;
};

struct TypeBlock {
    uint32_t offsetTypeName;
    uint32_t variableType;
    uint32_t numElements;
    uint32_t numTotalSize;
    uint32_t stride;
    uint32_t packedSize;
};

struct StructMemberBlock {
    uint32_t offsetName;
    uint32_t offsetSemantic;
    uint32_t offsetParentStruct;
    uint32_t offsetType;
};

struct NumericType {
    uint32_t layout        : 3;
    uint32_t scalarType    : 5;
    uint32_t rows          : 3;
    uint32_t columns       : 3;
    uint32_t isColumnMajor : 1;
    uint32_t isPackedArray : 1;
};

struct TypeInheritanceBlock {
    uint32_t offsetBaseClass;
    uint32_t numInterfaces;
};

struct GroupElement : BaseElement {
    uint32_t numTechniques;
};

struct TechniqueElement : BaseElement {
    uint32_t numPasses;
};

struct PassElement : BaseElement {
    uint32_t numAssignments;
};

struct AssignmentBlock {
    uint32_t keyIndex;
    uint32_t valueIndex;
    uint32_t type;
    uint32_t offsetInitializer;
};

struct InlineShaderBlock {
    uint32_t offsetShader;
    uint32_t offsetStreamOutputDeclaration;
};

struct ConstantBlock {
    uint32_t type;
    uint32_t value;
};

struct ConstantIndexBlock {
    uint32_t offsetArrayName;
    uint32_t index;
};

struct VariableIndexBlock {
    uint32_t offsetArrayName;
    uint32_t offsetIndexVariableName;
};

#pragma pack(pop)

struct StateAssignmentValue {
    const char *const name;
    const uint32_t value;
};

enum StateAssignmentType {
    kInvalidStateAssignment,
    kPassStateAssignment,
    kRasterizerStateAssignment,
    kDepthStencilStateAssignment,
    kBlendStateAssignment,
    kSampleStateAssignment
};

struct StateAssignment {
    const char *const name;
    StateAssignmentType blockType;
    ObjectType objectType;
    uint32_t numColumnsRequired;
    uint32_t numMaxIndicesAllowed;
    bool canUseBothVectorAndScalar;
    const StateAssignmentValue *const rightValue;
    void set() {
    }
    void reset() {
    }
};

static const StateAssignmentValue g_nullSAV[] = {
    { "nullptr", 0 },
    { 0, 0 }
};
static const StateAssignmentValue g_boolSAV[] = {
    { "false", GL_FALSE },
    { "true",  GL_TRUE },
    { 0, 0 }
};
static const StateAssignmentValue g_depthWriteMaskSAV[] = {
    { "ZERO", GL_ZERO },
    { "ALL",  0 },
    { 0, 0 }
};
static const StateAssignmentValue g_filterModeSAV[] = {
    { "MIN_MAG_MIP_POINT", 0 },
    { "MIN_MAG_POINT_MIP_LINEAR", 0 },
    { "MIN_POINT_MAG_LINEAR_MIP_POINT", 0 },
    { "MIN_POINT_MAG_MIP_LINEAR", 0 },
    { "MIN_LINEAR_MAG_MIP_POINT", 0 },
    { "MIN_LINEAR_MAG_POINT_MIP_LINEAR", 0 },
    { "MIN_MAG_LINEAR_MIP_POINT", 0 },
    { "MIN_MAG_MIP_LINEAR", 0 },
    { "ANISOTROPIC", 0 },
    { "COMPARISON_MIN_MAG_MIP_POINT", 0 },
    { "COMPARISON_MIN_MAG_POINT_MIP_LINEAR", 0 },
    { "COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT", 0 },
    { "COMPARISON_MIN_POINT_MAG_MIP_LINEAR", 0 },
    { "COMPARISON_MIN_LINEAR_MAG_MIP_POINT", 0 },
    { "COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR", 0 },
    { "COMPARISON_MIN_MAG_LINEAR_MIP_POINT", 0 },
    { "COMPARISON_MIN_MAG_MIP_LINEAR", 0 },
    { "COMPARISON_ANISOTROPIC", 0 },
    { 0, 0 }
};
static const StateAssignmentValue g_blendSAV[] = {
    { "ZERO",             GL_ZERO },
    { "ONE",              GL_ONE },
    { "SRC_COLOR",        GL_SRC_COLOR },
    { "INV_SRC_COLOR",    GL_ONE_MINUS_SRC_COLOR },
    { "SRC_ALPHA",        GL_SRC_ALPHA },
    { "INV_SRC_ALPHA",    GL_ONE_MINUS_SRC_ALPHA },
    { "DEST_ALPHA",       GL_DST_ALPHA },
    { "INV_DEST_ALPHA",   GL_ONE_MINUS_DST_ALPHA },
    { "DEST_COLOR",       GL_DST_COLOR },
    { "INV_DEST_COLOR",   GL_ONE_MINUS_DST_COLOR },
    { "SRC_ALPHA_SAT",    GL_SRC_ALPHA_SATURATE },
    { "BLEND_FACTOR",     0 },
    { "INV_BLEND_FACTOR", 0 },
    { "SRC1_COLOR",       GL_SRC1_COLOR },
    { "INV_SRC1_COLOR",   GL_ONE_MINUS_SRC1_COLOR },
    { "SRC1_ALPHA",       GL_SRC1_ALPHA },
    { "INV_SRC1_ALPHA",   GL_ONE_MINUS_SRC1_ALPHA },
    { 0, 0 }
};
static const StateAssignmentValue g_textureAddressSAV[] = {
    { "CLAMP", GL_CLAMP },
    { "WRAP", GL_WRAP_BORDER },
    { "MIRROR", GL_MIRRORED_REPEAT },
    { "BORDER", GL_MIRROR_CLAMP_TO_BORDER_EXT },
    { "MIRROR_ONCE", GL_MIRROR_CLAMP_EXT },
    { 0, 0 }
};
static const StateAssignmentValue g_cullSAV[] = {
    { "NONE", GL_NONE },
    { "FRONT", GL_FRONT },
    { "BACK", GL_BACK },
    { 0, 0 }
};
static const StateAssignmentValue g_compareSAV[] = {
    { "NEVER", GL_NEVER },
    { "LESS", GL_LESS },
    { "EQUAL", GL_EQUAL },
    { "LESS_EQUAL", GL_LEQUAL },
    { "GREATER", GL_GREATER },
    { "NOT_EQUAL", GL_NOTEQUAL },
    { "GREATER_EQUAL", GL_GEQUAL },
    { "ALWAYS", GL_ALWAYS },
    { 0, 0 }
};
static const StateAssignmentValue g_stencilOpSAV[] = {
    { "KEEP", GL_KEEP },
    { "ZERO", GL_ZERO },
    { "REPLACE", GL_REPLACE },
    { "INCR_SAT", GL_INCR_WRAP },
    { "DECR_SAT", GL_DECR_WRAP },
    { "INVERT", GL_INVERT },
    { "INCR", GL_INCR },
    { "DECR", GL_DECR },
    { 0, 0 }
};
static const StateAssignmentValue g_blendOpSAV[] = {
    { "ADD", GL_ADD },
    { "SUBTRACT", GL_SUBTRACT },
    { "REV_SUBTRACT", GL_SUBTRACT },
    { "MIN", GL_MIN },
    { "MAX", GL_MAX },
    { 0, 0 }
};

static const StateAssignment g_stateAssignments[] = {
    { "RasterizerState",           kPassStateAssignment,         kRasterizer,               1, 1, false, 0 },
    { "DepthStencilState",         kPassStateAssignment,         kDepthStencil,             1, 1, false, 0 },
    { "BlendState",                kPassStateAssignment,         kBlend,                    1, 1, false, 0 },
    { "RenderTargetView",          kPassStateAssignment,         kRenderColorTarget,        1, 4, false, 0 },
    { "DepthStencilView",          kPassStateAssignment,         kRenderDepthStencilTarget, 1, 4, false, 0 },
    { "GenerateMips",              kPassStateAssignment,         kTexture,                  1, 4, false, 0 },
    { "VertexShader",              kPassStateAssignment,         kVertexShader,             1, 1, false, g_nullSAV },
    { "PixelShader",               kPassStateAssignment,         kPixelShader,              1, 1, false, g_nullSAV },
    { "GeometryShader",            kPassStateAssignment,         kGeometryShader,           1, 1, false, g_nullSAV },
    { "DS_StencilRef",             kPassStateAssignment,         kObjectTypeUInt32,         1, 1, false, 0 },
    { "AB_BlendFactor",            kPassStateAssignment,         kObjectTypeFloat,          4, 1, false, 0 },
    { "AB_SampleFactor",           kPassStateAssignment,         kObjectTypeUInt32,         1, 1, false, 0 },
    { "FillMode",                  kRasterizerStateAssignment,   kObjectTypeUInt32,         1, 1, false, 0 },
    { "CullMode",                  kRasterizerStateAssignment,   kObjectTypeUInt32,         1, 1, false, g_cullSAV },
    { "FrontCounterClockWise",     kRasterizerStateAssignment,   kObjectTypeBool,           1, 1, false, g_boolSAV },
    { "DepthBias",                 kRasterizerStateAssignment,   kObjectTypeUInt32,         1, 1, false, 0 },
    { "DepthBiasClamp",            kRasterizerStateAssignment,   kObjectTypeFloat,          1, 1, false, 0 },
    { "SlopeScaledDepthBial",      kRasterizerStateAssignment,   kObjectTypeFloat,          1, 1, false, 0 },
    { "DepthClipEnable",           kRasterizerStateAssignment,   kObjectTypeBool,           1, 1, false, g_boolSAV },
    { "ScissorEnable",             kRasterizerStateAssignment,   kObjectTypeBool,           1, 1, false, g_boolSAV },
    { "MultisampleEnable",         kRasterizerStateAssignment,   kObjectTypeBool,           1, 1, false, g_boolSAV },
    { "AntialiasedLineEnable",     kRasterizerStateAssignment,   kObjectTypeBool,           1, 1, false, g_boolSAV },
    { "DepthEnable",               kDepthStencilStateAssignment, kObjectTypeBool,           1, 1, false, g_boolSAV },
    { "DepthWriteMask",            kDepthStencilStateAssignment, kObjectTypeUInt32,         1, 1, false, g_depthWriteMaskSAV },
    { "DepthFunc",                 kDepthStencilStateAssignment, kObjectTypeUInt32,         1, 1, false, g_compareSAV },
    { "StencilEnable",             kDepthStencilStateAssignment, kObjectTypeBool,           1, 1, false, g_boolSAV },
    { "StencilReadMask",           kDepthStencilStateAssignment, kObjectTypeUInt8,          1, 1, false, 0 },
    { "StencilWriteMask",          kDepthStencilStateAssignment, kObjectTypeUInt8,          1, 1, false, 0 },
    { "FrontFaceStencilFail",      kDepthStencilStateAssignment, kObjectTypeUInt32,         1, 1, false, g_stencilOpSAV },
    { "FrontFaceStencilDepthFail", kDepthStencilStateAssignment, kObjectTypeUInt32,         1, 1, false, g_stencilOpSAV },
    { "FrontFaceStencilPass",      kDepthStencilStateAssignment, kObjectTypeUInt32,         1, 1, false, g_stencilOpSAV },
    { "FrontFaceStencilFunc",      kDepthStencilStateAssignment, kObjectTypeUInt32,         1, 1, false, g_compareSAV },
    { "BackFaceStencilFail",       kDepthStencilStateAssignment, kObjectTypeUInt32,         1, 1, false, g_stencilOpSAV },
    { "BackFaceStencilDepthFail",  kDepthStencilStateAssignment, kObjectTypeUInt32,         1, 1, false, g_stencilOpSAV },
    { "BackFaceStencilPass",       kDepthStencilStateAssignment, kObjectTypeUInt32,         1, 1, false, g_stencilOpSAV },
    { "BackFaceStencilFunc",       kDepthStencilStateAssignment, kObjectTypeUInt32,         1, 1, false, g_compareSAV },
    { "AlphaToCoverageEnable",     kBlendStateAssignment,        kObjectTypeBool,           1, 1, false, g_boolSAV },
    { "BlendEnable",               kBlendStateAssignment,        kObjectTypeBool,           1, 8, false, g_boolSAV },
    { "SrcBlend",                  kBlendStateAssignment,        kObjectTypeUInt32,         1, 8, true,  g_blendSAV },
    { "DestBlend",                 kBlendStateAssignment,        kObjectTypeUInt32,         1, 8, true,  g_blendSAV },
    { "BlendOp",                   kBlendStateAssignment,        kObjectTypeUInt32,         1, 8, true,  g_blendOpSAV },
    { "SrcBlendAlpha",             kBlendStateAssignment,        kObjectTypeUInt32,         1, 8, true,  g_blendSAV },
    { "DestBlendAlpha",            kBlendStateAssignment,        kObjectTypeUInt32,         1, 8, true,  g_blendSAV },
    { "BlendOpAlpha",              kBlendStateAssignment,        kObjectTypeUInt32,         1, 8, true,  g_blendOpSAV },
    { "RenderTargetWriteMask",     kBlendStateAssignment,        kObjectTypeUInt8,          1, 8, false, 0 },
    { "Filter",                    kSampleStateAssignment,       kObjectTypeUInt32,         1, 1, false, g_filterModeSAV },
    { "AddressU",                  kSampleStateAssignment,       kObjectTypeUInt32,         1, 1, false, g_textureAddressSAV },
    { "AddressV",                  kSampleStateAssignment,       kObjectTypeUInt32,         1, 1, false, g_textureAddressSAV },
    { "AddressW",                  kSampleStateAssignment,       kObjectTypeUInt32,         1, 1, false, 0 },
    { "MipLODBias",                kSampleStateAssignment,       kObjectTypeFloat,          1, 1, false, 0 },
    { "MaxAnisotropy",             kSampleStateAssignment,       kObjectTypeUInt32,         1, 1, false, 0 },
    { "ComparisonFunc",            kSampleStateAssignment,       kObjectTypeUInt32,         1, 1, false, g_compareSAV },
    { "BorderColor",               kSampleStateAssignment,       kObjectTypeFloat,          4, 1, false, 0 },
    { "MinLOD",                    kSampleStateAssignment,       kObjectTypeFloat,          1, 1, false, 0 },
    { "MaxLOD",                    kSampleStateAssignment,       kObjectTypeFloat,          1, 1, false, 0 },
    { "Texture",                   kSampleStateAssignment,       kTexture,                  1, 1, false, g_nullSAV },
    { "HullShader",                kPassStateAssignment,         kHullShader5,              1, 1, false, g_nullSAV },
    { "DomainShader",              kPassStateAssignment,         kDomainShader5,            1, 1, false, g_nullSAV },
    { "ComputeShader",             kPassStateAssignment,         kComputeShader5,           1, 1, false, g_nullSAV },
};
static const size_t g_numStateAssignments = sizeof(g_stateAssignments) / sizeof(g_stateAssignments[0]);

}

namespace vpvl2
{
namespace extensions
{
namespace fx
{

struct EffectFX5::Type {
    Type()
        : variable(kInvalidVariable),
          object(kInvalidObject),
          numElements(0),
          numTotalSize(0)
    {
    }
    ~Type() {
        variable = kInvalidVariable;
        object = kInvalidObject;
        numElements = 0;
        numTotalSize = 0;
    }
    bool isNumeric(ScalarType scalar, NumericLayoutType layout) const {
        return variable == kNumericVariable && numeric.scalarType == scalar && numeric.layout == layout;
    }
    bool isString() const {
        return variable == kObjectVariable && object == kString;
    }
    bool isTexture() const {
        if (variable == kObjectVariable) {
            switch (object) {
            case kTexture:
            case kTexture1D:
            case kTexture1DArray:
            case kTexture2D:
            case kTexture2DArray:
            case kTexture2DMultisample:
            case kTexture2DMultisampleArray:
            case kTexture3D:
            case kTextureCube:
            case kTextureCubeArray:
                return true;
            default:
                break;
            }
        }
        return false;
    }
    bool isSampler() const {
        if (variable == kObjectVariable) {
            switch (object) {
            case kSampler:
                return true;
            default:
                break;
            }
        }
        return false;
    }

    VariableType variable;
    ObjectType object;
    NumericType numeric;
    uint32_t numElements;
    uint32_t numTotalSize;
};

struct EffectFX5::Annotation : IEffect::IAnnotation {
    static const char *kEmpty;

    Annotation()
        : namePtr(0)
    {
    }
    ~Annotation() {
        strings.releaseAll();
        delete namePtr;
        namePtr = 0;
    }

    bool booleanValue() const {
        if (type.isNumeric(kBooleanScalar, kScalarLayout)) {
            return value.i != 0;
        }
        return false;
    }
    int integerValue() const {
        if (type.isNumeric(kIntegerScalar, kScalarLayout)) {
            return value.i;
        }
        else {
            return 0;
        }
    }
    const int *integerValues(int *size) const {
        if (type.isNumeric(kIntegerScalar, kVectorLayout)) {
            *size = 1;
            return static_cast<const int *>(&value.i);
        }
        else {
            *size = 0;
            return 0;
        }
    }
    float floatValue() const {
        if (type.isNumeric(kFloatScalar, kScalarLayout)) {
            return value.f;
        }
        else {
            return 0;
        }
    }
    const float *floatValues(int *size) const {
        if (type.isNumeric(kFloatScalar, kVectorLayout)) {
            *size = 1;
            return static_cast<const float *>(&value.f);
        }
        else {
            *size = 0;
            return 0;
        }
    }
    const char *stringValue() const {
        if (type.isString() && strings.count() > 0) {
            return reinterpret_cast<const char *>(strings[0]->toByteArray());
        }
        else {
            return kEmpty;
        }
    }

    void registerName(EffectFX5::String2AnnotationRefHash &value) {
        value.insert(namePtr->toHashString(), this);
    }

    EffectFX5::Type type;
    IString *namePtr;
    PointerArray<IString> strings;
    union {
        uint32_t alignment;
        int i;
        float f;
    } value;
};

const char *EffectFX5::Annotation::kEmpty = "";

struct EffectFX5::Assignable {
    enum AssignmentType {
        kAssignConstant,
        kAssignVariable,
        kAssignShader
    };
    struct Index {
        Index(const StateAssignment *sa, const AssignmentType t, const int i)
            : stateAssignmentRef(sa),
              type(t),
              index(i)
        {
        }
        const StateAssignment *stateAssignmentRef;
        const AssignmentType type;
        const int index;
    };
    struct Variable {
        Variable(IString *name)
            : namePtr(name),
              valueRef(0)
        {
        }
        ~Variable() {
            delete namePtr;
            namePtr = 0;
            valueRef = 0;
        }
        IString *namePtr;
        IEffect::IParameter *valueRef;
    };
    struct Shader {
        Shader(const uint8_t *ptr, size_t length, int shaderVersion) {
            Array<char> bytecode;
            bytecode.resize(length + 1);
            memcpy(&bytecode[0], ptr, length);
            bytecode[length] = 0;
            const GLLang lang = resolveShaderLanguageVersion(shaderVersion);
            internal::zerofill(&shaderPtr, sizeof(shaderPtr));
            TranslateHLSLFromMem(&bytecode[0], 0, lang, 0, &shaderPtr);
        }
        ~Shader() {
            FreeGLSLShader(&shaderPtr);
            internal::zerofill(&shaderPtr, sizeof(shaderPtr));
        }
        static GLLang resolveShaderLanguageVersion(int value) {
            switch (value) {
            case 430:
                return LANG_430;
            case 420:
                return LANG_420;
            case 410:
                return LANG_410;
            case 400:
                return LANG_400;
            case 330:
                return LANG_330;
            case 300:
                return LANG_ES_300;
            case 150:
                return LANG_150;
            case 140:
                return LANG_140;
            case 130:
                return LANG_130;
            case 120:
                return LANG_120;
            case 100:
                return LANG_ES_100;
            default:
                return LANG_DEFAULT;
            }
        }
        GLSLShader shaderPtr;
    };

    Assignable() {}
    virtual ~Assignable() {
        variables.releaseAll();
        shaders.releaseAll();
    }

    void addConstant(const StateAssignment *sa, const ConstantBlock &constant) {
        indices.append(Index(sa, kAssignConstant, constants.count()));
        constants.append(constant);
    }
    void addVariable(const StateAssignment *sa, IString *name) {
        indices.append(Index(sa, kAssignVariable, variables.count()));
        variables.append(new Variable(name));
    }
    void addShader(const StateAssignment *sa, const uint8_t *ptr, size_t length, int shaderVersion) {
        indices.append(Index(sa, kAssignShader, shaders.count()));
        shaders.append(new Shader(ptr, length, shaderVersion));
    }

    PointerArray<Shader> shaders;
    PointerArray<Variable> variables;
    Array<Index> indices;
    Array<ConstantBlock> constants;
};

struct EffectFX5::Parameter : IEffect::IParameter, Assignable {
    Parameter(EffectFX5 *p, EffectFX5::String2AnnotationRefHash *annotations)
        : effectRef(p),
          annotationRefs(annotations),
          namePtr(0),
          semanticPtr(0)
    {
    }
    ~Parameter() {
        effectRef = 0;
        annotationRefs = 0;
        delete namePtr;
        namePtr = 0;
        delete semanticPtr;
        semanticPtr = 0;
    }

    IEffect *parentEffectRef() const {
        return 0;
    }
    IEffect::IAnnotation *annotationRef(const char *name) const {
        EffectFX5::Annotation *const *annotation = annotationRefs->find(name);
        return annotation ? *annotation : 0;
    }
    const char *name() const {
        return reinterpret_cast<const char *>(namePtr->toByteArray());
    }
    const char *semantic() const {
        return reinterpret_cast<const char *>(semanticPtr->toByteArray());
    }
    Type type() const {
        if (parameterType.isTexture()) {
            return IParameter::kTexture;
        }
        else if (parameterType.isSampler()) {
            return IParameter::kSampler2D;
        }
        else if (parameterType.variable == kNumericVariable) {
            const NumericType &numeric = parameterType.numeric;
            switch (numeric.scalarType) {
            case kBooleanScalar:
                return IParameter::kBoolean;
            case kIntegerScalar:
            case kUIntegerScalar:
                return IParameter::kInteger;
            case kFloatScalar:
                switch (numeric.layout) {
                case kScalarLayout:
                    return IParameter::kFloat;
                case kVectorLayout:
                    return IParameter::kFloat4;
                case kMatrixLayout:
                    return IParameter::kFloat4x4;
                }
            default:
                return IParameter::kUnknown;
            }
        }
        else {
            return IParameter::kUnknown;
        }
    }
    void connect(IParameter *destinationParameter) {
        (void) destinationParameter;
    }
    void reset() {
    }
    void getValue(int &value) const {
        if (parameterType.isNumeric(kIntegerScalar, kScalarLayout)) {
            value = v.scalari;
        }
        else {
            value = 0;
        }
    }
    void getValue(float &value) const {
        if (parameterType.isNumeric(kFloatScalar, kScalarLayout)) {
            value = v.scalarf;
        }
        else {
            value = 0;
        }
    }
    void getValue(Vector3 &value) const {
        if (parameterType.isNumeric(kFloatScalar, kVectorLayout)) {
            value.setValue(v.vectorf[0], v.vectorf[1], v.vectorf[2]);
            value.setW(v.vectorf[3]);
        }
        else {
            value.setZero();
        }
    }
    void getValue(Vector4 &value) const {
        if (parameterType.isNumeric(kFloatScalar, kVectorLayout)) {
            value.setValue(v.vectorf[0], v.vectorf[1], v.vectorf[2], v.vectorf[3]);
        }
        else {
            value.setZero();
        }
    }
    void getMatrix(float *value) const {
        if (parameterType.isNumeric(kFloatScalar, kMatrixLayout)) {
            memcpy(value, v.matrix, sizeof(v.matrix));
        }
        else {
            memset(value, 0, sizeof(v.matrix));
            value[0] = value[4] = value[10] = value[15] = 1;
        }
    }
    void getArrayDimension(int &value) const {
        if (parameterType.numElements > 0) {
            value = 1;
        }
        else {
            value = 0;
        }
    }
    void getArrayTotalSize(int &value) const {
        value = parameterType.numTotalSize;
    }
    void getTextureRef(intptr_t &value) const {
        if (parameterType.isTexture()) {
            value = v.texture;
        }
    }
    void getSamplerStateRefs(Array<IEffect::ISamplerState *> &value) const {
        if (parameterType.isSampler()) {
            value.clear();
        }
    }
    void setValue(bool value) {
        if (parameterType.isNumeric(kBooleanScalar, kScalarLayout)) {
            v.scalarb = value;
        }
    }
    void setValue(int value) {
        if (parameterType.isNumeric(kIntegerScalar, kScalarLayout)) {
            v.scalari = value;
        }
    }
    void setValue(float value) {
        if (parameterType.isNumeric(kFloatScalar, kScalarLayout)) {
            v.scalarf = value;
        }
    }
    void setValue(const Vector3 &value) {
        if (parameterType.isNumeric(kFloatScalar, kVectorLayout)) {
            for (int i = 0; i < 3; i++) {
                v.vectorf[i] = value[i];
            }
            v.vectorf[3] = 0;
        }
    }
    void setValue(const Vector4 &value) {
        if (parameterType.isNumeric(kFloatScalar, kVectorLayout)) {
            for (int i = 0; i < 4; i++) {
                v.vectorf[i] = value[i];
            }
        }
    }
    void setValue(const Vector4 *value) {
    }
    void setMatrix(const float *value) {
        if (parameterType.isNumeric(kFloatScalar, kMatrixLayout)) {
            for (int i = 0; i < 16; i++) {
                v.matrix[i] = value[i];
            }
        }
    }
    void setSampler(const ITexture *value) {
        if (parameterType.isSampler()) {
            v.sampler = value->data();
        }
    }
    void setTexture(const ITexture *value) {
        setTexture(value->data());
    }
    void setTexture(intptr_t value) {
        if (parameterType.isTexture()) {
            v.texture = value;
        }
    }

    void registerName(EffectFX5::String2ParameterRefHash &value) {
        value.insert(namePtr->toHashString(), this);
    }
    void registerSemantic(EffectFX5::String2ParameterRefHash &value) {
        value.insert(semanticPtr->toHashString(), this);
    }

    EffectFX5 *effectRef;
    EffectFX5::String2AnnotationRefHash *annotationRefs;
    EffectFX5::Type parameterType;
    IString *namePtr;
    IString *semanticPtr;
    union {
        bool scalarb;
        int scalari;
        float scalarf;
        float vectorf[4];
        float matrix[16];
        intptr_t texture;
        intptr_t sampler;
    } v;
};

struct EffectFX5::Pass : IEffect::IPass, Assignable {
    Pass(EffectFX5 *p, IEffect::ITechnique *t, EffectFX5::String2AnnotationRefHash *annotations)
        : effectRef(p),
          annotationRefs(annotations),
          techniqueRef(t),
          namePtr(0)
    {
    }
    ~Pass() {
        effectRef = 0;
        annotationRefs = 0;
        techniqueRef = 0;
        delete namePtr;
        namePtr = 0;
    }

    IEffect::ITechnique *parentTechniqueRef() const {
        return techniqueRef;
    }
    IEffect::IAnnotation *annotationRef(const char *name) const {
        EffectFX5::Annotation *const * annotation = annotationRefs->find(name);
        return annotation ? *annotation : 0;
    }
    const char*name() const {
        return reinterpret_cast<const char *>(namePtr->toByteArray());
    }
    void setState() {
    }
    void resetState() {
    }

    void registerName(EffectFX5::String2PassRefHash &value) {
        value.insert(namePtr->toHashString(), this);
    }

    EffectFX5 *effectRef;
    EffectFX5::String2AnnotationRefHash *annotationRefs;
    IEffect::ITechnique *techniqueRef;
    IString *namePtr;
    ShaderProgram shaderProgram;
};

struct EffectFX5::Technique : IEffect::ITechnique {
    Technique(EffectFX5 *p, EffectFX5::String2AnnotationRefHash *annotations)
        : effectRef(p),
          annotationRefs(annotations),
          namePtr(0)
    {
    }
    ~Technique() {
        effectRef = 0;
        annotationRefs = 0;
        delete namePtr;
        namePtr = 0;
    }

    IEffect *parentEffectRef() const {
        return 0;
    }
    IEffect::IPass *findPass(const char *name) const {
        IEffect::IPass *const *passRef = name2PassRefs.find(name);
        return passRef ? *passRef : 0;
    }
    IEffect::IAnnotation *annotationRef(const char *name) const {
        EffectFX5::Annotation *const *annotation = annotationRefs->find(name);
        return annotation ? *annotation : 0;
    }
    const char *name() const {
        return reinterpret_cast<const char *>(namePtr->toByteArray());
    }
    void getPasses(Array<IEffect::IPass *> &passes) const {
        passes.copy(passRefs);
    }

    EffectFX5::Pass *addPass(PointerArray<EffectFX5::Pass> &passes) {
        EffectFX5::Pass *pass = passes.append(new EffectFX5::Pass(effectRef, this, annotationRefs));
        passRefs.append(pass);
        return pass;
    }
    void registerName(EffectFX5::String2TechniqueRefHash &value) {
        value.insert(namePtr->toHashString(), this);
    }

    EffectFX5 *effectRef;
    EffectFX5::String2AnnotationRefHash *annotationRefs;
    Array<IEffect::IPass *> passRefs;
    Hash<HashString, IEffect::IPass *> name2PassRefs;
    IString *namePtr;
};

struct EffectFX5::ParseData {
    ParseData(const uint8_t *base, uint8_t *ptr, const size_t size, size_t rest)
        : base(base),
          size(size),
          unstructured(ptr - base),
          ptr(ptr),
          nshaders(0),
          rest(rest)
    {
    }
    const uint8_t *base;
    const size_t size;
    const size_t unstructured;
    uint8_t *ptr;
    size_t nshaders;
    size_t rest;
};

#ifdef VPVL2_LINK_GLOG
std::ostream &operator<<(std::ostream &stream, const EffectFX5::ParseData &data)
{
    stream << "ptr=" << reinterpret_cast<const void *>(data.ptr) << " base=" << reinterpret_cast<const void *>(data.base) << " size=" << data.size << " rest=" << data.rest;
    return stream;
}
#endif

EffectFX5::EffectFX5(IEncoding *encoding)
    : m_encodingRef(encoding),
      m_shaderVersion(120)
{
}

EffectFX5::~EffectFX5()
{
    m_annotations.releaseAll();
    m_parameters.releaseAll();
    m_techniques.releaseAll();
    m_passes.releaseAll();
    m_encodingRef = 0;
    m_shaderVersion = 0;
}

bool EffectFX5::parse(const uint8_t *data, size_t size)
{
    if (!data || size == 0) {
        VPVL2_LOG(LOG(WARNING) << "Empty effect data is passed: ptr=" << reinterpret_cast<const void *>(data) << "size=" << size);
        return false;
    }

    Header header;
    internal::zerofill(&header, sizeof(header));
    uint8_t *ptr = const_cast<uint8_t *>(data);
    size_t rest = size;
    if (!internal::getTyped(ptr, rest, header)) {
        VPVL2_LOG(LOG(WARNING) << "Invalid header detected: ptr=" << reinterpret_cast<const void *>(ptr) << " rest=" << rest);
        return false;
    }
    if (!header.isValidSignature()) {
        VPVL2_LOG(LOG(WARNING) << "Invalid signature detected: data=" << header.signature << " rest=" << rest);
        return false;
    }
    if (!internal::validateSize(ptr, header.unstructuredSize, rest)) {
        VPVL2_LOG(LOG(WARNING) << "Invalid unstructured data detected: ptr=" << reinterpret_cast<const void *>(ptr) << " rest=" << rest);
        return false;
    }
    const uint32_t numConstants = header.effect.numConstants;
    EffectFX5::ParseData parseData(data + 0x60, ptr, size, rest);
    ConstantBufferElement constantBuffer;
    NumericVarableElement numericVariable;
    for (uint32_t i = 0; i < numConstants; i++) {
        if (!internal::getTyped(parseData.ptr, parseData.rest, constantBuffer)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid constant buffer detected: " << parseData);
            return false;
        }
        IString *name;
        if (!parseString(parseData, constantBuffer.offsetName, name)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid constant buffer name detected: " << parseData);
            return false;
        }
        VPVL2_LOG(VLOG(2) << "ConstantBuffer name=" << internal::cstr(name, "(null)") << " numVariables=" << constantBuffer.numVariables << " size=" << constantBuffer.size << " flags=" << constantBuffer.flags << " explicitBind=" << constantBuffer.explicitBindPoint);
        delete name;
        if (!parseAnnotation(parseData)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid annotation of constant buffer detected: " << parseData);
            return false;
        }
        const uint32_t nvariables = constantBuffer.numVariables;
        for (uint32_t j = 0; j < nvariables; j++) {
            if (!internal::getTyped(parseData.ptr, parseData.rest, numericVariable)) {
                VPVL2_LOG(LOG(WARNING) << "Invalid numeric variable detected: " << parseData);
                return false;
            }
            Parameter *parameter = m_parameters.append(new Parameter(this, &m_name2AnnotationRef));
            if (!parseString(parseData, numericVariable.offsetName, parameter->namePtr)) {
                VPVL2_LOG(LOG(WARNING) << "Invalid numeric variable name detected: " << parseData);
                return false;
            }
            if (!parseString(parseData, numericVariable.offsetSemantic, parameter->semanticPtr)) {
                VPVL2_LOG(LOG(WARNING) << "Invalid constant variable semantic detected: " << parseData);
                return false;
            }
            VPVL2_LOG(VLOG(2) << "NumericVariable name=" << internal::cstr(parameter->namePtr, "(null)") << " semantic=" << internal::cstr(parameter->semanticPtr, "(null)") << " flags=" << numericVariable.flags << " offsetSelf=" << numericVariable.offsetSelf);
            if (!parseAnnotation(parseData)) {
                VPVL2_LOG(LOG(WARNING) << "Invalid annotation of numeric variable detected: " << parseData);
                return false;
            }
            parameter->registerName(m_name2ParameterRef);
            parameter->registerSemantic(m_name2ParameterRef);
        }
    }

    const uint32_t numObjectVariables = header.effect.numObjectVariables;
    ObjectVariable objectVariable;
    for (uint32_t i = 0; i < numObjectVariables; i++) {
        if (!internal::getTyped(parseData.ptr, parseData.rest, objectVariable)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid object detected: " << parseData);
            return false;
        }
        Parameter *parameter = m_parameters.append(new Parameter(this, &m_name2AnnotationRef));
        if (!parseString(parseData, objectVariable.offsetName, parameter->namePtr)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid object name detected: " << parseData);
            return false;
        }
        if (!parseString(parseData, objectVariable.offsetSemantic, parameter->semanticPtr)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid object semantic detected: " << parseData);
            return false;
        }
        VPVL2_LOG(VLOG(2) << "ObjectVariable name=" << internal::cstr(parameter->namePtr, "(null)") << " semantic=" << internal::cstr(parameter->semanticPtr, "(null)") << " explicitBind=" << objectVariable.explicitBindPoint);
        if (!parseType(parseData, objectVariable.offsetType, parameter->parameterType)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid object type detected: " << parseData);
            return false;
        }
        const Type &type = parameter->parameterType;
        switch (static_cast<ObjectType>(type.object)) {
        case kBlend:
        case kDepthStencil:
        case kRasterizer:
        case kSampler:
        {
            uint32_t numElements = type.numElements;
            btSetMax(numElements, uint32_t(1));
            for (uint32_t j = 0; j < numElements; j++) {
                uint32_t numAssignments;
                if (!internal::getTyped(parseData.ptr, parseData.rest, numAssignments)) {
                    VPVL2_LOG(LOG(WARNING) << "Invalid assignment of object detected: " << parseData);
                    return false;
                }
                if (!parseAssignments(parseData, numAssignments, parameter)) {
                    VPVL2_LOG(LOG(WARNING) << "Invalid annotation of assignment of object detected: " << parseData);
                    return false;
                }
            }
            break;
        }
        case kString:
        case kPixelShader:
        case kVertexShader:
        case kGeometryShader:
        case kGeometryShaderStreamOutput:
        {
            uint32_t numElements = type.numElements;
            btSetMax(numElements, uint32_t(1));
            for (uint32_t j = 0; j < numElements; j++) {
                uint32_t offset;
                if (!internal::getTyped(parseData.ptr, parseData.rest, offset)) {
                    VPVL2_LOG(LOG(WARNING) << "Invalid string/shader of object detected: " << parseData);
                    return false;
                }
                IString *value;
                if (!parseString(parseData, offset, value)) {
                    VPVL2_LOG(LOG(WARNING) << "Invalid value of string/shader of object detected: " << parseData);
                    return false;
                }
                VPVL2_LOG(VLOG(2) << "ObjectVariable value=" << internal::cstr(value, "(null)"));
                delete value;
            }
            break;
        }
        default:
            break;
        }
        if (!parseAnnotation(parseData)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid annotation of object detected: " << parseData);
            return false;
        }
        parameter->registerName(m_name2ParameterRef);
        parameter->registerSemantic(m_name2ParameterRef);
    }

    /* NOT implemented */
    const uint32_t numInterfaces = header.numInterfaceVariableElements;
    if (numInterfaces > 0) {
        VPVL2_LOG(LOG(WARNING) << "Interface doesn't support yet: " << parseData);
        return false;
    }

    const uint32_t numGroups = header.numGroups;
    GroupElement groupElement;
    TechniqueElement techniqueElement;
    PassElement passElement;
    for (uint32_t i = 0; i < numGroups; i++) {
        if (!internal::getTyped(parseData.ptr, parseData.rest, groupElement)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid group detected: " << parseData);
            return false;
        }
        IString *name;
        if (!parseString(parseData, groupElement.offsetName, name)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid name of group detected: " << parseData);
            return false;
        }
        VPVL2_LOG(VLOG(2) << "Group name=" << internal::cstr(name, "(null)"));
        delete name;
        if (!parseAnnotation(parseData)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid annotation of group detected: " << parseData);
            return false;
        }
        const uint32_t numTechniques = groupElement.numTechniques;
        for (uint32_t j = 0; j < numTechniques; j++) {
            if (!internal::getTyped(parseData.ptr, parseData.rest, techniqueElement)) {
                VPVL2_LOG(LOG(WARNING) << "Invalid technique detected: " << parseData);
                return false;
            }
            Technique *technique = m_techniques.append(new Technique(this, &m_name2AnnotationRef));
            if (!parseString(parseData, techniqueElement.offsetName, technique->namePtr)) {
                VPVL2_LOG(LOG(WARNING) << "Invalid name of technique detected: " << parseData);
                return false;
            }
            VPVL2_LOG(VLOG(2) << "Technique name=" << internal::cstr(technique->namePtr, "(null)"));
            if (!parseAnnotation(parseData)) {
                VPVL2_LOG(LOG(WARNING) << "Invalid annotation of technique detected: " << parseData);
                return false;
            }
            technique->registerName(m_name2TechniqueRef);
            const uint32_t numPasses = techniqueElement.numPasses;
            for (uint32_t k = 0; k < numPasses; k++) {
                if (!internal::getTyped(parseData.ptr, parseData.rest, passElement)) {
                    VPVL2_LOG(LOG(WARNING) << "Invalid pass detected: " << parseData);
                    return false;
                }
                Pass *pass = technique->addPass(m_passes);
                if (!parseString(parseData, passElement.offsetName, pass->namePtr)) {
                    VPVL2_LOG(LOG(WARNING) << "Invalid name of pass detected: " << parseData);
                    return false;
                }
                VPVL2_LOG(VLOG(2) << "Pass name=" << internal::cstr(pass->namePtr, "(null)"));
                if (!parseAnnotation(parseData)) {
                    VPVL2_LOG(LOG(WARNING) << "Invalid annotation of pass detected: " << parseData);
                    return false;
                }
                if (!parseAssignments(parseData, passElement.numAssignments, pass)) {
                    VPVL2_LOG(LOG(WARNING) << "Invalid assignment of pass detected: " << parseData);
                    return false;
                }
                pass->registerName(m_name2PassRef);
            }
        }
    }

    return parseData.rest == 0;
}

bool EffectFX5::compile()
{
    const int npasses = m_passes.count();
    for (int i = 0; i < npasses; i++) {
        Pass *pass = m_passes[i];
        ShaderProgram &program = pass->shaderProgram;
        program.create();
        if (!program.isLinked()) {
            const int nshaders = pass->shaders.count();
            for (int j = 0; j < nshaders; j++) {
                const GLSLShader &shader = pass->shaders[j]->shaderPtr;
                if (!program.addShaderSource(shader.sourceCode, shader.shaderType)) {
                    VPVL2_LOG(LOG(WARNING) << "Shader in " << internal::cstr(pass->namePtr, "(null)") << " cannot be compiled: " << program.message());
                    return false;
                }
            }
            if (!program.link()) {
                VPVL2_LOG(LOG(WARNING) << "Program in " << internal::cstr(pass->namePtr, "(null)") << " cannot be linked: " << program.message());
                return false;
            }
        }
        resolveAssignableVariables(pass);
    }
    const int nparameters = m_parameters.count();
    for (int i = 0; i < nparameters; i++) {
        Parameter *parameter = m_parameters[i];
        resolveAssignableVariables(parameter);
    }
    return true;
}

void EffectFX5::setShaderVersion(int value)
{
    m_shaderVersion = value;
}

bool EffectFX5::lookup(const ParseData &data, size_t offset, uint32_t &value)
{
    if (offset + sizeof(uint32_t) > data.unstructured) {
        VPVL2_LOG(LOG(WARNING) << "Invalid offset detected: " << data);
        return false;
    }
    else {
        value = *reinterpret_cast<const uint32_t *>(data.base + offset);
        return true;
    }
}

bool EffectFX5::parseString(const ParseData &data, size_t offset, IString *&string)
{
    ssize_t rest = data.unstructured - offset;
    if (rest <= 0) {
        VPVL2_LOG(LOG(WARNING) << "Invalid offset of string detected: " << data);
        return false;
    }
    uint8_t *ptr = const_cast<uint8_t *>(data.base + offset), *ptr2 = ptr;
    while (*ptr2 && rest >= 0) {
        ptr2++;
        rest--;
    }
    if (rest <= 0 || !parseRawString(data, ptr, ptr2 - ptr, string)) {
        return false;
    }
    return true;
}

bool EffectFX5::parseRawString(const ParseData &data, const uint8_t *ptr, size_t size, IString *&string)
{
    if (size > data.unstructured) {
        VPVL2_LOG(LOG(WARNING) << "Invalid string length detected: " << data);
        return false;
    }
    string = m_encodingRef->toString(ptr, size, IString::kShiftJIS);
    return true;
}

bool EffectFX5::parseType(const ParseData &data, uint32_t offset, Type &type)
{
    TypeBlock typeElement;
    uint8_t *ptr = const_cast<uint8_t *>(data.base) + offset;
    size_t rest = data.unstructured - offset;
    if (!internal::getTyped(ptr, rest, typeElement)) {
        VPVL2_LOG(LOG(WARNING) << "Invalid type detected: " << data);
        return false;
    }
    type.numElements = typeElement.numElements;
    type.numTotalSize = typeElement.numTotalSize;
    IString *name;
    if (!parseString(data, typeElement.offsetTypeName, name)) {
        VPVL2_LOG(LOG(WARNING) << "Invalid name of type detected: " << data);
        return false;
    }
    VPVL2_LOG(VLOG(2) << "Type name=" << internal::cstr(name, "(null)"));
    delete name;
    type.variable = static_cast<VariableType>(typeElement.variableType);
    switch (type.variable) {
    case kNumericVariable: {
        if (!internal::getTyped(ptr, rest, type.numeric)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid numeric type detected: " << data);
            return false;
        }
        break;
    }
    case kObjectVariable: {
        uint32_t objectType;
        if (!internal::getTyped(ptr, rest, objectType)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid object type detected: " << data);
            return false;
        }
        type.object = static_cast<ObjectType>(objectType);
        break;
    }
    case kStructVariable: {
        uint32_t numMembers;
        if (!internal::getTyped(ptr, rest, numMembers)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid size of struct member detected: " << data);
            return false;
        }
        StructMemberBlock member;
        for (uint32_t i = 0; i < numMembers; i++) {
            if (!internal::getTyped(ptr, rest, member)) {
                VPVL2_LOG(LOG(WARNING) << "Invalid struct member detected: " << data);
                return false;
            }
            if (!parseString(data, member.offsetName, name)) {
                VPVL2_LOG(LOG(WARNING) << "Invalid name of struct member detected: " << data);
                return false;
            }
            delete name;
            IString *semantic;
            if (!parseString(data, member.offsetSemantic, semantic)) {
                VPVL2_LOG(LOG(WARNING) << "Invalid semantic of struct member detected: " << data);
                return false;
            }
            delete semantic;
            Type memberType;
            if (!parseType(data, member.offsetType, memberType)) {
                VPVL2_LOG(LOG(WARNING) << "Invalid type of struct member detected: " << data);
                return false;
            }
        }
        break;
    }
    default:
        VPVL2_LOG(LOG(WARNING) << "Invalid object type detected: value=" << type.variable << " " << data);
        return false;
    }
    return true;
}

bool EffectFX5::parseAnnotation(ParseData &data)
{
    uint32_t numAnnotations;
    if (!internal::getTyped(data.ptr, data.rest, numAnnotations)) {
        VPVL2_LOG(LOG(WARNING) << "Invalid type of annotation detected: " << data);
        return false;
    }
    AnnotationElement annotationElement;
    for (uint32_t i = 0; i < numAnnotations; i++) {
        if (!internal::getTyped(data.ptr, data.rest, annotationElement)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid annotation detected: " << data);
            return false;
        }
        Annotation *annotation = m_annotations.append(new Annotation());
        if (!parseString(data, annotationElement.offsetName, annotation->namePtr)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid name of annotation detected: " << data);
            return false;
        }
        VPVL2_LOG(VLOG(2) << "Annotation name=" << internal::cstr(annotation->namePtr, "(null)"));
        annotation->registerName(m_name2AnnotationRef);
        if (!parseType(data, annotationElement.offsetType, annotation->type)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid type of annotation detected: " << data);
            return false;
        }
        if (annotation->type.variable == kNumericVariable) {
            if (!internal::getTyped(data.ptr, data.rest, annotation->value.alignment)) {
                VPVL2_LOG(LOG(WARNING) << "Invalid numeric annotation detected: " << data);
                return false;
            }
        }
        else if (annotation->type.isString()) {
            uint32_t numElements = annotation->type.numElements;
            btSetMax(numElements, uint32_t(1));
            annotation->strings.reserve(numElements);
            for (uint32_t j = 0; j < numElements; j++) {
                uint32_t offsetString;
                if (!internal::getTyped(data.ptr, data.rest, offsetString)) {
                    VPVL2_LOG(LOG(WARNING) << "Invalid string annotation detected: " << data);
                    return false;
                }
                IString *value;
                if (!parseString(data, annotationElement.offsetName, value)) {
                    VPVL2_LOG(LOG(WARNING) << "Invalid value of string annotation detected: " << data);
                    return false;
                }
                annotation->strings.append(value);
                VPVL2_LOG(VLOG(2) << "String[" << j << "] name=" << internal::cstr(value, "(null)"));
            }
        }
    }
    return true;
}

bool EffectFX5::parseAssignments(ParseData &data, const uint32_t numAssignments, Assignable *assignable)
{
    AssignmentBlock assignment;
    for (uint32_t i = 0; i < numAssignments; i++) {
        if (!internal::getTyped(data.ptr, data.rest, assignment)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid type of assignment detected: " << data);
            return false;
        }
        if (assignment.keyIndex >= g_numStateAssignments) {
            VPVL2_LOG(LOG(WARNING) << "Invalid keyIndex of assignment detected: keyIndex=" << assignment.keyIndex << " " << data);
            return false;
        }
        const StateAssignment *stateAssignment = &g_stateAssignments[assignment.keyIndex];
        VPVL2_LOG(VLOG(2) << "Assignment[" << i << "] keyIndex=" << assignment.keyIndex << " valueIndex=" << assignment.valueIndex);
        switch (static_cast<AssignmentType>(assignment.type)) {
        case kConstantAssignment:
        {
            ConstantBlock constant;
            uint8_t *ptr = const_cast<uint8_t *>(data.base) + assignment.offsetInitializer;
            size_t rest = data.unstructured - assignment.offsetInitializer;
            if (!internal::getTyped(ptr, rest, constant)) {
                VPVL2_LOG(LOG(WARNING) << "Invalid type of constant assignment detected: " << data);
                return false;
            }
            assignable->addConstant(stateAssignment, constant);
            VPVL2_LOG(VLOG(2) << "ConstantAssignment type=" << constant.type << " value=" << constant.value);
            break;
        }
        case kVariableAssignment:
        {
            IString *value;
            if (!parseString(data, assignment.offsetInitializer, value)) {
                VPVL2_LOG(LOG(WARNING) << "Invalid variable of assignment detected: " << data);
                return false;
            }
            assignable->addVariable(stateAssignment, value);
            VPVL2_LOG(VLOG(2) << "VariableAssignment value=" << internal::cstr(value, "(null)"));
            break;
        }
        case kConstantIndexAssignment:
        {
            ConstantIndexBlock constantIndex;
            uint8_t *ptr = const_cast<uint8_t *>(data.base) + assignment.offsetInitializer;
            size_t rest = data.unstructured - assignment.offsetInitializer;
            if (!internal::getTyped(ptr, rest, constantIndex)) {
                VPVL2_LOG(LOG(WARNING) << "Invalid constant index of assignment detected: " << data);
                return false;
            }
            break;
        }
        case kVariableIndexAssignment:
        {
            VariableIndexBlock variableIndex;
            uint8_t *ptr = const_cast<uint8_t *>(data.base) + assignment.offsetInitializer;
            size_t rest = data.unstructured - assignment.offsetInitializer;
            if (!internal::getTyped(ptr, rest, variableIndex)) {
                VPVL2_LOG(LOG(WARNING) << "Invalid variable index of assignment detected: " << data);
                return false;
            }
            break;
        }
        case kExpressionAssignment:
        {
            VPVL2_LOG(LOG(WARNING) << "ExpressionAssignment is not supported yet: " << data);
            return false;
        }
        case kExpressionIndexAssignment:
        {
            VPVL2_LOG(LOG(WARNING) << "ExpressionIndexAssignment is not supported yet: " << data);
            return false;
        }
        case kInlineShaderAssignment:
        {
            InlineShaderBlock inlineShader;
            uint8_t *inlineShaderPtr = const_cast<uint8_t *>(data.base) + assignment.offsetInitializer;
            size_t inlineShaderRest = data.unstructured - assignment.offsetInitializer;
            if (!internal::getTyped(inlineShaderPtr, inlineShaderRest, inlineShader)) {
                VPVL2_LOG(LOG(WARNING) << "Invalid inline shader of assignment detected: " << data);
                return false;
            }
            uint8_t *shaderBodyPtr = const_cast<uint8_t *>(data.base) + inlineShader.offsetShader;
            size_t shaderBodyRest = data.unstructured - inlineShader.offsetShader;
            uint32_t length;
            if (!internal::getTyped(shaderBodyPtr, shaderBodyRest, length)) {
                VPVL2_LOG(LOG(WARNING) << "Invalid shader of assignment detected: " << data);
                return false;
            }
            if (length >= data.unstructured) {
                VPVL2_LOG(LOG(WARNING) << "Invalid shader of assignment detected: " << data);
                return false;
            }
            assignable->addShader(stateAssignment, shaderBodyPtr, length, m_shaderVersion);
            VPVL2_LOG(VLOG(2) << "InlineShaderAssignment size=" << length);
            break;
        }
        case kInvalidAssignment:
        default:
            VPVL2_LOG(LOG(WARNING) << "Invalid assignment type detected: type=" << assignment.type << " " << data);
            return false;
        }
    }
    return true;
}

void EffectFX5::resolveAssignableVariables(Assignable *value)
{
    const int nvariables = value->variables.count();
    for (int i = 0; i < nvariables; i++) {
        EffectFX5::Assignable::Variable *variable = value->variables[i];
        if (Parameter *const *parameterRef = m_name2ParameterRef.find(variable->namePtr->toHashString())) {
            variable->valueRef = *parameterRef;
        }
    }
}

} /* namespace fx */
} /* namespace extensions */
} /* namespace vpvl2 */
