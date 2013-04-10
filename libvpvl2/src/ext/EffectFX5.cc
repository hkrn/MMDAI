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

#include <mojoshader.h>

namespace {


#pragma pack(push, 1)

enum VariableType {
    kInvalidVariable,
    kNumericVariable,
    kObjectVariable,
    kStructVariable,
    kInterfaceVariable
};

enum ScalarType {
    kInvalidScalar,
    kFloat,
    kInt,
    kUInt,
    kBool,
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
    kCountObject
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

struct VariableCounter {
    uint32_t numConstants;
    uint32_t numNumericVariables;
    uint32_t numObjectVariables;
};

struct Header {
    uint32_t signature;
    VariableCounter effect;
    VariableCounter pool;
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

struct ConstantBuffer {
    uint32_t offsetName;
    uint32_t size;
    uint32_t flags;
    uint32_t numVariables;
    uint32_t explicitBindPoint;
};

struct BaseElement {
    uint32_t offsetName;
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

struct Type {
    uint32_t offsetTypeName;
    uint32_t variableType;
    uint32_t numElements;
    uint32_t numTotalSize;
    uint32_t stride;
    uint32_t packedSize;
};

struct StructMember {
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
    uint32_t isPackedArary : 1;
};

struct TypeInheritance {
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

struct Assignment {
    uint32_t index;
    uint32_t assignIndex;
    uint32_t type;
    uint32_t offsetInitializer;
};

struct InlineShader {
    uint32_t offsetShader;
    uint32_t offsetStreamOutputDeclaration;
};

struct Constant {
    uint32_t type;
    uint32_t value;
};

struct ConstantIndex {
    uint32_t offsetArrayName;
    uint32_t index;
};

struct VariableIndex {
    uint32_t offsetArrayName;
    uint32_t offsetIndexVariableName;
};

#pragma pack(pop)

}

namespace vpvl2
{
namespace extensions
{
namespace fx
{

struct EffectFX5::Annotation : IEffect::IAnnotation {
    static const char *kEmpty;

    Annotation()
        : symbolType(MOJOSHADER_SYMTYPE_VOID),
          index(0),
          valuePtr(0)
    {
    }
    ~Annotation() {
        delete valuePtr;
        valuePtr = 0;
    }

    bool booleanValue() const {
        if (symbolType == MOJOSHADER_SYMTYPE_BOOL) {
            return strcmp(cstring(), "true") == 0;
        }
        return false;
    }
    int integerValue() const {
        if (symbolType == MOJOSHADER_SYMTYPE_INT) {
            return strtol(cstring(), 0, 10);
        }
        else {
            return 0;
        }
    }
    const int *integerValues(int *size) const {
        if (symbolType == MOJOSHADER_SYMTYPE_INT) {
            return 0;
        }
        else {
            *size = 0;
            return 0;
        }
    }
    float floatValue() const {
        if (symbolType == MOJOSHADER_SYMTYPE_FLOAT) {
            return strtof(cstring(), 0);
        }
        else {
            return 0;
        }
    }
    const float *floatValues(int *size) const {
        if (symbolType == MOJOSHADER_SYMTYPE_FLOAT) {
            return 0;
        }
        else {
            *size = 0;
            return 0;
        }
    }
    const char *stringValue() const {
        if (symbolType == MOJOSHADER_SYMTYPE_STRING) {
            return cstring();
        }
        else {
            return kEmpty;
        }
    }
    const char *cstring() const {
        return reinterpret_cast<const char *>(valuePtr->toByteArray());
    }

    void registerName(EffectFX5::String2AnnotationRefHash &value) {
        value.insert(valuePtr->toHashString(), this);
    }

    MOJOSHADER_symbolType symbolType;
    uint32_t index;
    IString *valuePtr;
};

const char *EffectFX5::Annotation::kEmpty = "";

struct EffectFX5::Annotateable {
    virtual ~Annotateable() {}
    Array<EffectFX5::Annotation *> annotationRefs;
};

struct EffectFX5::Parameter : EffectFX5::Annotateable, IEffect::IParameter {
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
        switch (symbolType) {
        case MOJOSHADER_SYMTYPE_BOOL:
            return kBoolean;
        case MOJOSHADER_SYMTYPE_INT:
            return kInteger;
        case MOJOSHADER_SYMTYPE_FLOAT:
            switch (symbolClass) {
            case MOJOSHADER_SYMCLASS_SCALAR:
                return kFloat;
            case MOJOSHADER_SYMCLASS_VECTOR:
                return kFloat4;
            case MOJOSHADER_SYMCLASS_MATRIX_COLUMNS:
            case MOJOSHADER_SYMCLASS_MATRIX_ROWS:
                return kFloat4x4;
            default:
                return kUnknown;
            }
            break;
        case MOJOSHADER_SYMTYPE_TEXTURE:
        case MOJOSHADER_SYMTYPE_TEXTURE1D:
        case MOJOSHADER_SYMTYPE_TEXTURE2D:
        case MOJOSHADER_SYMTYPE_TEXTURE3D:
        case MOJOSHADER_SYMTYPE_TEXTURECUBE:
            return kTexture;
        case MOJOSHADER_SYMTYPE_SAMPLER:
        case MOJOSHADER_SYMTYPE_SAMPLER2D:
            return kSampler2D;
        case MOJOSHADER_SYMTYPE_SAMPLER3D:
            return kSampler3D;
        case MOJOSHADER_SYMTYPE_SAMPLERCUBE:
            return kSamplerCube;
        default:
            return kUnknown;
        }
    }
    void connect(IParameter *destinationParameter) {
        (void) destinationParameter;
    }
    void reset() {
    }
    void getValue(int &value) const {
        if (symbolClass == MOJOSHADER_SYMCLASS_SCALAR && symbolType == MOJOSHADER_SYMTYPE_INT) {
            value = v.scalari;
        }
        else {
            value = 0;
        }
    }
    void getValue(float &value) const {
        if (symbolClass == MOJOSHADER_SYMCLASS_SCALAR && symbolType == MOJOSHADER_SYMTYPE_FLOAT) {
            value = v.scalarf;
        }
        else {
            value = 0;
        }
    }
    void getValue(Vector3 &value) const {
        if (symbolClass == MOJOSHADER_SYMCLASS_VECTOR && symbolType == MOJOSHADER_SYMTYPE_FLOAT) {
            value.setValue(v.vectorf[0], v.vectorf[1], v.vectorf[2]);
            value.setW(v.vectorf[3]);
        }
        else {
            value.setZero();
        }
    }
    void getValue(Vector4 &value) const {
        if (symbolClass == MOJOSHADER_SYMCLASS_VECTOR && symbolType == MOJOSHADER_SYMTYPE_FLOAT) {
            value.setValue(v.vectorf[0], v.vectorf[1], v.vectorf[2], v.vectorf[3]);
        }
        else {
            value.setZero();
        }
    }
    void getMatrix(float *value) const {
        if ((symbolClass == MOJOSHADER_SYMCLASS_MATRIX_COLUMNS ||
             symbolClass == MOJOSHADER_SYMCLASS_MATRIX_ROWS) &&
                symbolType == MOJOSHADER_SYMTYPE_FLOAT) {
            memcpy(value, v.matrix, sizeof(v.matrix));
        }
        else {
            memset(value, 0, sizeof(v.matrix));
            value[0] = value[4] = value[10] = value[15] = 1;
        }
    }
    void getArrayDimension(int &value) const {
        value = 1;
    }
    void getArrayTotalSize(int &value) const {
        value = 1;
    }
    void getTextureRef(intptr_t &value) const {
        if (symbolClass == MOJOSHADER_SYMCLASS_OBJECT && symbolType == MOJOSHADER_SYMTYPE_TEXTURE) {
            value = v.texture;
        }
    }
    void getSamplerStateRefs(Array<IEffect::ISamplerState *> &value) const {
    }
    void setValue(bool value) {
        if (symbolClass == MOJOSHADER_SYMCLASS_SCALAR && symbolType == MOJOSHADER_SYMTYPE_BOOL) {
            v.scalarb = value;
        }
    }
    void setValue(int value) {
        if (symbolClass == MOJOSHADER_SYMCLASS_SCALAR && symbolType == MOJOSHADER_SYMTYPE_INT) {
            v.scalari = value;
        }
    }
    void setValue(float value) {
        if (symbolClass == MOJOSHADER_SYMCLASS_SCALAR && symbolType == MOJOSHADER_SYMTYPE_FLOAT) {
            v.scalarf = value;
        }
    }
    void setValue(const Vector3 &value) {
        if (symbolClass == MOJOSHADER_SYMCLASS_VECTOR && symbolType == MOJOSHADER_SYMTYPE_FLOAT) {
            for (int i = 0; i < 3; i++) {
                v.vectorf[i] = value[i];
            }
            v.vectorf[3] = 0;
        }
    }
    void setValue(const Vector4 &value) {
        if (symbolClass == MOJOSHADER_SYMCLASS_VECTOR && symbolType == MOJOSHADER_SYMTYPE_FLOAT) {
            for (int i = 0; i < 4; i++) {
                v.vectorf[i] = value[i];
            }
        }
    }
    void setValue(const Vector4 *value) {
    }
    void setMatrix(const float *value) {
        if ((symbolClass == MOJOSHADER_SYMCLASS_MATRIX_COLUMNS ||
             symbolClass == MOJOSHADER_SYMCLASS_MATRIX_ROWS) &&
                symbolType == MOJOSHADER_SYMTYPE_FLOAT) {
            for (int i = 0; i < 16; i++) {
                v.matrix[i] = value[i];
            }
        }
    }
    void setSampler(const ITexture *value) {
        if (symbolClass == MOJOSHADER_SYMCLASS_OBJECT &&
                (symbolType == MOJOSHADER_SYMTYPE_SAMPLER ||
                 symbolType == MOJOSHADER_SYMTYPE_SAMPLER1D ||
                 symbolType == MOJOSHADER_SYMTYPE_SAMPLER2D ||
                 symbolType == MOJOSHADER_SYMTYPE_SAMPLER3D ||
                 symbolType == MOJOSHADER_SYMTYPE_SAMPLERCUBE)) {
            v.sampler = value->data();
        }
        else {
        }
    }
    void setTexture(const ITexture *value) {
        setTexture(value->data());
    }
    void setTexture(intptr_t value) {
        if (symbolClass == MOJOSHADER_SYMCLASS_OBJECT &&
                (symbolType == MOJOSHADER_SYMTYPE_TEXTURE ||
                 symbolType == MOJOSHADER_SYMTYPE_TEXTURE1D ||
                 symbolType == MOJOSHADER_SYMTYPE_TEXTURE2D ||
                 symbolType == MOJOSHADER_SYMTYPE_TEXTURE3D ||
                 symbolType == MOJOSHADER_SYMTYPE_TEXTURECUBE)) {
            v.texture = value;
        }
        else {
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
    MOJOSHADER_symbolType symbolType;
    MOJOSHADER_symbolClass symbolClass;
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

struct EffectFX5::Pass : EffectFX5::Annotateable, IEffect::IPass {
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
};

struct EffectFX5::Technique : EffectFX5::Annotateable, IEffect::ITechnique {
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

struct EffectFX5::State {
    State()
        : type(0)
    {
    }
    const uint32_t type;
};

struct EffectFX5::Texture {
    Texture(EffectFX5 *p)
        : parentEffectRef(p),
          name(0),
          index(0),
          type(0)
    {
    }
    ~Texture() {
        delete name;
        name = 0;
        index = 0;
        type = 0;
    }

    void registerName(EffectFX5::String2TextureRefHash &value) {
        value.insert(name->toHashString(), this);
    }

    EffectFX5 *parentEffectRef;
    IString *name;
    uint32_t index;
    uint32_t type;
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

std::ostream &operator<<(std::ostream &stream, const EffectFX5::ParseData &data)
{
    stream << "ptr=" << reinterpret_cast<const char *>(data.ptr) << " base=" << reinterpret_cast<const char *>(data.base) << " size=" << data.size << " rest=" << data.rest;
    return stream;
}

struct EffectFX5::Shader {
    Shader(EffectFX5 *p, const uint8_t *ptr)
        : parentEffectRef(p),
          data(MOJOSHADER_parse("glsl120", ptr, 0, 0, 0, 0, 0, 0, 0, this)),
          technique(0),
          pass(0)
    {
    }
    ~Shader() {
        MOJOSHADER_freeParseData(data);
        technique = 0;
        pass = 0;
    }

    EffectFX5 *parentEffectRef;
    const MOJOSHADER_parseData *data;
    uint32_t technique;
    uint32_t pass;
};

EffectFX5::EffectFX5(IEncoding *encoding)
    : m_encoding(encoding)
{
}

EffectFX5::~EffectFX5()
{
    m_encoding = 0;
    m_annotations.releaseAll();
    m_parameters.releaseAll();
    m_techniques.releaseAll();
    m_passes.releaseAll();
    m_states.releaseAll();
    m_textures.releaseAll();
    m_shaders.releaseAll();
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
    ConstantBuffer constantBuffer;
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
            if (!parseString(parseData, numericVariable.offsetName, name)) {
                VPVL2_LOG(LOG(WARNING) << "Invalid numeric variable name detected: " << parseData);
                return false;
            }
            IString *semantic;
            if (!parseString(parseData, numericVariable.offsetSemantic, semantic)) {
                VPVL2_LOG(LOG(WARNING) << "Invalid constant variable semantic detected: " << parseData);
                return false;
            }
            VPVL2_LOG(VLOG(2) << "NumericVariable name=" << internal::cstr(name, "(null)") << " semantic=" << internal::cstr(semantic, "(null)") << " flags=" << numericVariable.flags << " offsetSelf=" << numericVariable.offsetSelf);
            delete name;
            delete semantic;
            if (!parseAnnotation(parseData)) {
                VPVL2_LOG(LOG(WARNING) << "Invalid annotation of numeric variable detected: " << parseData);
                return false;
            }
        }
    }

    const uint32_t numObjectVariables = header.effect.numObjectVariables;
    ObjectVariable objectVariable;
    for (uint32_t i = 0; i < numObjectVariables; i++) {
        if (!internal::getTyped(parseData.ptr, parseData.rest, objectVariable)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid object detected: " << parseData);
            return false;
        }
        IString *name;
        if (!parseString(parseData, objectVariable.offsetName, name)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid object name detected: " << parseData);
            return false;
        }
        IString *semantic;
        if (!parseString(parseData, objectVariable.offsetSemantic, semantic)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid object semantic detected: " << parseData);
            return false;
        }
        VPVL2_LOG(VLOG(2) << "ObjectVariable name=" << internal::cstr(name, "(null)") << " semantic=" << internal::cstr(semantic, "(null)") << " explicitBind=" << objectVariable.explicitBindPoint);
        delete name;
        delete semantic;
        uint32_t varType, objectType, numElements;
        if (!parseType(parseData, objectVariable.offsetType, varType, objectType, numElements)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid object type detected: " << parseData);
            return false;
        }
        switch (static_cast<ObjectType>(objectType)) {
        case kBlend:
        case kDepthStencil:
        case kRasterizer:
        case kSampler:
        {
            btSetMax(numElements, uint32_t(1));
            for (uint32_t j = 0; j < numElements; j++) {
                uint32_t numAssignments;
                if (!internal::getTyped(parseData.ptr, parseData.rest, numAssignments)) {
                    VPVL2_LOG(LOG(WARNING) << "Invalid assignment of object detected: " << parseData);
                    return false;
                }
                if (!parseAssignments(parseData, numAssignments)) {
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
    }

    /* NOT implemented */
    const uint32_t numInterfaces = header.numInterfaceVariableElements;
    if (numInterfaces > 0) {
        VPVL2_LOG(LOG(WARNING) << "Interface doesn't support yet: " << parseData);
        return false;
    }

    const uint32_t numGroups = header.numGroups;
    GroupElement group;
    TechniqueElement technique;
    PassElement pass;
    for (uint32_t i = 0; i < numGroups; i++) {
        if (!internal::getTyped(parseData.ptr, parseData.rest, group)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid group detected: " << parseData);
            return false;
        }
        IString *name;
        if (!parseString(parseData, group.offsetName, name)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid name of group detected: " << parseData);
            return false;
        }
        VPVL2_LOG(VLOG(2) << "Group name=" << internal::cstr(name, "(null)"));
        delete name;
        if (!parseAnnotation(parseData)) {
            return false;
        }
        const uint32_t numTechniques = group.numTechniques;
        for (uint32_t j = 0; j < numTechniques; j++) {
            if (!internal::getTyped(parseData.ptr, parseData.rest, technique)) {
                VPVL2_LOG(LOG(WARNING) << "Invalid technique detected: " << parseData);
                return false;
            }
            if (!parseString(parseData, technique.offsetName, name)) {
                VPVL2_LOG(LOG(WARNING) << "Invalid name of technique detected: " << parseData);
                return false;
            }
            VPVL2_LOG(VLOG(2) << "Technique name=" << internal::cstr(name, "(null)"));
            delete name;
            if (!parseAnnotation(parseData)) {
                VPVL2_LOG(LOG(WARNING) << "Invalid annotation of technique detected: " << parseData);
                return false;
            }
            const uint32_t numPasses = technique.numPasses;
            for (uint32_t k = 0; k < numPasses; k++) {
                if (!internal::getTyped(parseData.ptr, parseData.rest, pass)) {
                    VPVL2_LOG(LOG(WARNING) << "Invalid pass detected: " << parseData);
                    return false;
                }
                if (!parseString(parseData, pass.offsetName, name)) {
                    VPVL2_LOG(LOG(WARNING) << "Invalid name of pass detected: " << parseData);
                    return false;
                }
                VPVL2_LOG(VLOG(2) << "Pass name=" << internal::cstr(name, "(null)"));
                delete name;
                if (!parseAnnotation(parseData)) {
                    VPVL2_LOG(LOG(WARNING) << "Invalid annotation of pass detected: " << parseData);
                    return false;
                }
                if (!parseAssignments(parseData, pass.numAssignments)) {
                    VPVL2_LOG(LOG(WARNING) << "Invalid assignment of pass detected: " << parseData);
                    return false;
                }
            }
        }
    }

    return parseData.rest == 0;
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
    string = m_encoding->toString(ptr, size, IString::kShiftJIS);
    return true;
}

bool EffectFX5::parseType(const ParseData &data, uint32_t offset, uint32_t &varType, uint32_t &objectType, uint32_t &nelements)
{
    Type type;
    uint8_t *ptr = const_cast<uint8_t *>(data.base) + offset;
    size_t rest = data.unstructured - offset;
    if (!internal::getTyped(ptr, rest, type)) {
        VPVL2_LOG(LOG(WARNING) << "Invalid type detected: " << data);
        return false;
    }
    nelements = type.numElements;
    IString *name;
    if (!parseString(data, type.offsetTypeName, name)) {
        VPVL2_LOG(LOG(WARNING) << "Invalid name of type detected: " << data);
        return false;
    }
    VPVL2_LOG(VLOG(2) << "Type name=" << internal::cstr(name, "(null)"));
    delete name;
    varType = type.variableType;
    objectType = 0;
    switch (static_cast<VariableType>(varType)) {
    case kNumericVariable: {
        uint32_t numericType;
        if (!internal::getTyped(ptr, rest, numericType)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid numeric type detected: " << data);
            return false;
        }
        break;
    }
    case kObjectVariable: {
        if (!internal::getTyped(ptr, rest, objectType)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid object type detected: " << data);
            return false;
        }
        break;
    }
    case kStructVariable: {
        uint32_t numMembers;
        if (!internal::getTyped(ptr, rest, numMembers)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid size of struct member detected: " << data);
            return false;
        }
        StructMember member;
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
            if (!parseType(data, member.offsetType, varType, objectType, nelements)) {
                VPVL2_LOG(LOG(WARNING) << "Invalid type of struct member detected: " << data);
                return false;
            }
        }
        break;
    }
    default:
        VPVL2_LOG(LOG(WARNING) << "Invalid object type detected: value=" << varType << " " << data);
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
    AnnotationElement annotation;
    for (uint32_t i = 0; i < numAnnotations; i++) {
        if (!internal::getTyped(data.ptr, data.rest, annotation)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid annotation detected: " << data);
            return false;
        }
        IString *name;
        if (!parseString(data, annotation.offsetName, name)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid name of annotation detected: " << data);
            return false;
        }
        VPVL2_LOG(VLOG(2) << "Annotation name=" << internal::cstr(name, "(null)"));
        delete name;
        uint32_t varType, objectType, numElements;
        if (!parseType(data, annotation.offsetType, varType, objectType, numElements)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid type of annotation detected: " << data);
            return false;
        }
        if (static_cast<VariableType>(varType) == kNumericVariable) { // VarType::kNumeric
            uint32_t defaultValue;
            if (!internal::getTyped(data.ptr, data.rest, defaultValue)) {
                VPVL2_LOG(LOG(WARNING) << "Invalid numeric annotation detected: " << data);
                return false;
            }
        }
        else if (static_cast<ObjectType>(objectType) == kString) {
            btSetMax(numElements, uint32_t(1));
            for (uint32_t j = 0; j < numElements; j++) {
                uint32_t offsetString;
                if (!internal::getTyped(data.ptr, data.rest, offsetString)) {
                    VPVL2_LOG(LOG(WARNING) << "Invalid string annotation detected: " << data);
                    return false;
                }
                IString *value;
                if (!parseString(data, annotation.offsetName, value)) {
                    VPVL2_LOG(LOG(WARNING) << "Invalid value of string annotation detected: " << data);
                    return false;
                }
                VPVL2_LOG(VLOG(2) << "String[" << j << "] name=" << internal::cstr(value, "(null)"));
                delete value;
            }
        }
    }
    return true;
}

bool EffectFX5::parseAssignments(ParseData &data, const uint32_t numAssignments)
{
    Assignment assignment;
    for (uint32_t j = 0; j < numAssignments; j++) {
        if (!internal::getTyped(data.ptr, data.rest, assignment)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid type of assignment detected: " << data);
            return false;
        }
        switch (static_cast<AssignmentType>(assignment.type)) {
        case kConstantAssignment:
        {
            Constant constant;
            uint8_t *ptr = const_cast<uint8_t *>(data.base) + assignment.offsetInitializer;
            size_t rest = data.unstructured - assignment.offsetInitializer;
            if (!internal::getTyped(ptr, rest, constant)) {
                VPVL2_LOG(LOG(WARNING) << "Invalid type of constant assignment detected: " << data);
                return false;
            }
            break;
        }
        case kVariableAssignment:
        {
            IString *value;
            if (!parseString(data, assignment.offsetInitializer, value)) {
                VPVL2_LOG(LOG(WARNING) << "Invalid variable of assignment detected: " << data);
                return false;
            }
            VPVL2_LOG(VLOG(2) << "Assignment value=" << internal::cstr(value, "(null)"));
            delete value;
            break;
        }
        case kConstantIndexAssignment:
        {
            ConstantIndex constantIndex;
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
            VariableIndex variableIndex;
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
            InlineShader inlineShader;
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
            IString *value;
            if (!parseRawString(data, shaderBodyPtr, length, value)) {
                VPVL2_LOG(LOG(WARNING) << "Invalid shader of assignment detected: " << data);
                return false;
            }
            VPVL2_LOG(VLOG(2) << "InlineShader size=" << length);
            delete value;
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

} /* namespace fx */
} /* namespace extensions */
} /* namespace vpvl2 */
