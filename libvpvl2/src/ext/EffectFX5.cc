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
    kInvalid,
    kNumeric,
    kObject,
    kStruct,
    kInterface
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

struct Constant {
    uint32_t type;
    uint32_t value;
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

    void dump() {
        const int nerrors = data->error_count;
        VPVL2_LOG(VLOG(2) << "Effect::Shader: nattrs=" << data->attribute_count);
        VPVL2_LOG(VLOG(2) << "Effect::Shader: nconstants=" << data->constant_count);
        VPVL2_LOG(VLOG(2) << "Effect::Shader: nerrors=" << nerrors);
        VPVL2_LOG(VLOG(2) << "Effect::Shader: ninstructions=" << data->instruction_count);
        VPVL2_LOG(VLOG(2) << "Effect::Shader: major=" << data->major_ver);
        VPVL2_LOG(VLOG(2) << "Effect::Shader: minor=" << data->minor_ver);
        VPVL2_LOG(VLOG(2) << "Effect::Shader: noutputs=" << data->output_count);
        VPVL2_LOG(VLOG(2) << "Effect::Shader: output=" << data->output_len);
        VPVL2_LOG(VLOG(2) << "Effect::Shader: nsamplers=" << data->sampler_count);
        VPVL2_LOG(VLOG(2) << "Effect::Shader: nswizzles=" << data->swizzle_count);
        VPVL2_LOG(VLOG(2) << "Effect::Shader: nsymbols=" << data->symbol_count);
        VPVL2_LOG(VLOG(2) << "Effect::Shader: nuniforms=" << data->uniform_count);
        for (int i = 0; i < nerrors; i++) {
            const MOJOSHADER_error *err = &data->errors[i];
            VPVL2_LOG(LOG(WARNING) << "Effect::Shader error: index=" << i << ": filename=" << (err->filename ? err->filename : "(null)") << " position=" << err->error_position << " message=" << err->error);
        }
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
        VPVL2_LOG(LOG(WARNING) << "Invalid header detected: ptr=" << reinterpret_cast<const void *>(ptr) << " size=" << rest);
        return false;
    }
    if (!header.isValidSignature()) {
        VPVL2_LOG(LOG(WARNING) << "Invalid signature detected: data=" << header.signature);
        return false;
    }
    if (!internal::validateSize(ptr, header.unstructuredSize, rest)) {
        VPVL2_LOG(LOG(WARNING) << "Invalid unstructured data detected: ptr=" << reinterpret_cast<const void *>(ptr) << " size=" << rest);
        return false;
    }
    const uint32_t numConstants = header.effect.numConstants;
    EffectFX5::ParseData parseData(data + 0x60, ptr, size, rest);
    ConstantBuffer constantBuffer;
    NumericVarableElement numericVariable;
    for (uint32_t i = 0; i < numConstants; i++) {
        if (!internal::getTyped(parseData.ptr, parseData.rest, constantBuffer)) {
            return false;
        }
        IString *name;
        if (!parseString(parseData, constantBuffer.offsetName, name)) {
            return false;
        }
        VPVL2_LOG(VLOG(2) << "ConstantBuffer name=" << internal::cstr(name, "(null)") << " numVariables=" << constantBuffer.numVariables << " size=" << constantBuffer.size << " flags=" << constantBuffer.flags << " explicitBind=" << constantBuffer.explicitBindPoint);
        delete name;
        if (!parseAnnotation(parseData)) {
            return false;
        }
        const uint32_t nvariables = constantBuffer.numVariables;
        for (uint32_t j = 0; j < nvariables; j++) {
            if (!internal::getTyped(parseData.ptr, parseData.rest, numericVariable)) {
                return false;
            }
            if (!parseString(parseData, numericVariable.offsetName, name)) {
                return false;
            }
            IString *semantic;
            if (!parseString(parseData, numericVariable.offsetSemantic, semantic)) {
                return false;
            }
            VPVL2_LOG(VLOG(2) << "NumericVariable name=" << internal::cstr(name, "(null)") << " semantic=" << internal::cstr(semantic, "(null)") << " flags=" << numericVariable.flags << " offsetSelf=" << numericVariable.offsetSelf);
            delete name;
            delete semantic;
            if (!parseAnnotation(parseData)) {
                return false;
            }
        }
    }

    const uint32_t numObjectVariables = header.effect.numObjectVariables;
    ObjectVariable objectVariable;
    for (uint32_t i = 0; i < numObjectVariables; i++) {
        if (!internal::getTyped(parseData.ptr, parseData.rest, objectVariable)) {
            return false;
        }
        IString *name;
        if (!parseString(parseData, objectVariable.offsetName, name)) {
            return false;
        }
        IString *semantic;
        if (!parseString(parseData, objectVariable.offsetSemantic, semantic)) {
            return false;
        }
        VPVL2_LOG(VLOG(2) << "ObjectVariable name=" << internal::cstr(name, "(null)") << " semantic=" << internal::cstr(semantic, "(null)") << " explicitBind=" << objectVariable.explicitBindPoint);
        delete name;
        delete semantic;
        uint32_t varType, objectType, numElemenets;
        if (!parseType(parseData, objectVariable.offsetType, varType, objectType, numElemenets)) {
            return false;
        }
        // EOT_Blend, EOT_DepthStencil, EOT_Rasterizer, EOT_Sampler
        switch (objectType) {
        case 2:  // Blend
        case 3:  // DepthStencil
        case 4:  // Rasterizer
        case 21: // Sampler
        {
            btSetMax(numElemenets, uint32_t(1));
            for (uint32_t j = 0; j < numElemenets; j++) {
                uint32_t numAssignments;
                if (!internal::getTyped(parseData.ptr, parseData.rest, numAssignments)) {
                    return false;
                }
                Assignment assignment;
                for (uint32_t k = 0; k < numAssignments; k++) {
                    if (!internal::getTyped(parseData.ptr, parseData.rest, assignment)) {
                        return false;
                    }
                }
            }
            break;
        }
        case 1:
        case 5:
        case 6:
        case 7:
        case 8:
        case 25:
        case 26:
        case 27:
        case 28:
        case 29:
        case 30:
        {
            btSetMax(numElemenets, uint32_t(1));
            for (uint32_t j = 0; j < numElemenets; j++) {
                uint32_t offset;
                if (!internal::getTyped(parseData.ptr, parseData.rest, offset)) {
                    return false;
                }
                IString *value;
                if (!parseString(parseData, offset, value)) {
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
            return false;
        }
    }

    /* NOT implemented but parse */
    const uint32_t numInterfaces = header.numInterfaceVariableElements;
    InterfaceVariableElement interfaceVariable;
    for (uint32_t i = 0; i < numInterfaces; i++) {
        if (!internal::getTyped(parseData.ptr, parseData.rest, interfaceVariable)) {
            return false;
        }
        if (!parseAnnotation(parseData)) {
            return false;
        }
    }

    const uint32_t numGroups = header.numGroups;
    GroupElement group;
    TechniqueElement technique;
    PassElement pass;
    for (uint32_t i = 0; i < numGroups; i++) {
        if (!internal::getTyped(parseData.ptr, parseData.rest, group)) {
            return false;
        }
        IString *name;
        if (!parseString(parseData, group.offsetName, name)) {
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
                return false;
            }
            if (!parseString(parseData, technique.offsetName, name)) {
                return false;
            }
            VPVL2_LOG(VLOG(2) << "Technique name=" << internal::cstr(name, "(null)"));
            delete name;
            const uint32_t numPasses = technique.numPasses;
            for (uint32_t k = 0; k < numPasses; k++) {
                if (!internal::getTyped(parseData.ptr, parseData.rest, pass)) {
                    return false;
                }
                if (!parseString(parseData, pass.offsetName, name)) {
                    return false;
                }
                VPVL2_LOG(VLOG(2) << "Pass name=" << internal::cstr(name, "(null)"));
                delete name;
            }
        }
    }

    return true;
}

bool EffectFX5::lookup(const ParseData &data, size_t offset, uint32_t &value)
{
    if (offset + sizeof(uint32_t) > data.unstructured) {
        VPVL2_LOG(LOG(WARNING) << "Invalid offset detected: offset=" << offset << " size=" << data.size);
        return false;
    }
    else {
        value = *reinterpret_cast<const uint32_t *>(data.base + offset);
        return true;
    }
}

bool EffectFX5::parseString(const ParseData &data, size_t offset, IString *&string)
{
    if (offset >= data.unstructured) {
        return false;
    }
    uint8_t *ptr = const_cast<uint8_t *>(data.base + offset), *ptr2 = ptr;
    ssize_t rest = data.unstructured;
    while (*ptr2 && rest >= 0) {
        ptr2++;
        rest--;
    }
    if (rest == 0 || !parseRawString(data, ptr, ptr2 - ptr, string)) {
        return false;
    }
    return true;
}

bool EffectFX5::parseRawString(const ParseData &data, const uint8_t *ptr, size_t size, IString *&string)
{
    if (size > data.unstructured) {
        VPVL2_LOG(LOG(WARNING) << "Invalid string length detected: size=" << size << " rest=" << data.rest);
        return false;
    }
    string = m_encoding->toString(ptr, size, IString::kShiftJIS);
    return true;
}

bool EffectFX5::parseType(const ParseData &data, uint32_t offset, uint32_t &varType, uint32_t &objectType, uint32_t &nelements)
{
    Type type;
    uint8_t *ptr = const_cast<uint8_t *>(data.base) + offset;
    size_t rest = data.unstructured;
    if (!internal::getTyped(ptr, rest, type)) {
        return false;
    }
    nelements = type.numElements;
    IString *name;
    if (!parseString(data, type.offsetTypeName, name)) {
        return false;
    }
    VPVL2_LOG(VLOG(2) << "Type name=" << internal::cstr(name, "(null)"));
    delete name;
    varType = type.variableType;
    objectType = 0;
    switch (static_cast<VariableType>(varType)) {
    case kNumeric: {
        uint32_t numericType;
        if (!internal::getTyped(ptr, rest, numericType)) {
            return false;
        }
        break;
    }
    case kObject: {
        if (!internal::getTyped(ptr, rest, objectType)) {
            return false;
        }
        break;
    }
    case kStruct: {
        uint32_t numMembers;
        if (!internal::getTyped(ptr, rest, numMembers)) {
            return false;
        }
        StructMember member;
        for (uint32_t i = 0; i < numMembers; i++) {
            if (!internal::getTyped(ptr, rest, member)) {
                return false;
            }
            if (!parseString(data, member.offsetName, name)) {
                return false;
            }
            delete name;
            IString *semantic;
            if (!parseString(data, member.offsetSemantic, semantic)) {
                return false;
            }
            delete semantic;
            if (!parseType(data, member.offsetType, varType, objectType, nelements)) {
                return false;
            }
        }
        break;
    }
    default:
        break;
    }
    return true;
}

bool EffectFX5::parseAnnotation(ParseData &data)
{
    uint32_t numAnnotations;
    if (!internal::getTyped(data.ptr, data.rest, numAnnotations)) {
        return false;
    }
    AnnotationElement annotation;
    for (uint32_t i = 0; i < numAnnotations; i++) {
        if (!internal::getTyped(data.ptr, data.rest, annotation)) {
            return false;
        }
        IString *name;
        if (!parseString(data, annotation.offsetName, name)) {
            return false;
        }
        VPVL2_LOG(VLOG(2) << "Annotation name=" << internal::cstr(name, "(null)"));
        delete name;
        uint32_t varType, objectType, numElements;
        if (!parseType(data, annotation.offsetType, varType, objectType, numElements)) {
            return false;
        }
        if (varType == 1) { // VarType::kNumeric
            uint32_t defaultValue;
            if (!internal::getTyped(data.ptr, data.rest, defaultValue)) {
                return false;
            }
        }
        else if (objectType == 1) { // ObjectType::kString
            btSetMax(numElements, uint32_t(1));
            for (uint32_t j = 0; j < numElements; j++) {
                uint32_t offsetString;
                if (!internal::getTyped(data.ptr, data.rest, offsetString)) {
                    return false;
                }
                IString *value;
                if (!parseString(data, annotation.offsetName, value)) {
                    return false;
                }
                VPVL2_LOG(VLOG(2) << "String[" << j << "] name=" << internal::cstr(value, "(null)"));
                delete value;
            }
        }
    }
    return true;
}

} /* namespace fx */
} /* namespace extensions */
} /* namespace vpvl2 */
