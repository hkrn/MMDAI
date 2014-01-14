/**

 Copyright (c) 2010-2014  hkrn

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

#include <vpvl2/vpvl2.h>
#include <vpvl2/internal/util.h>
#include <vpvl2/extensions/fx/EffectFX2.h>

#include <mojoshader.h>

/*
 * effect parser based on mojoshader but compiles shader in the effect with mojoshader.
 * mojoshader: http://icculus.org/mojoshader/
 */

namespace vpvl2
{
namespace extensions
{
namespace fx
{

#pragma pack(push, 1)

struct Header {
    uint32_t signature;
    uint32_t offset;
};

struct ParameterHeader {
    uint32_t nparameters;
    uint32_t ntechniques;
    uint32_t unknown;
    uint32_t nobjects;
};

struct AnnotationIndexUnit {
    uint32_t type;
    uint32_t index;
};

struct ParameterUnit {
    struct {
        uint32_t type;
        uint32_t value;
    } offset;
    uint32_t flags;
    uint32_t nannotations;
};

struct ParameterType {
    uint32_t symtype;
    uint32_t symclass;
    struct {
        uint32_t name;
        uint32_t semantic;
    } offset;
};

struct ParameterValue {
    uint32_t symtype;
    uint32_t symclass;
};

struct TechniqueUnit {
    uint32_t offset;
    uint32_t nannotations;
    uint32_t npasses;
};

struct PassUnit {
    uint32_t offset;
    uint32_t nannotations;
    uint32_t nstates;
};

struct StateUnit {
    uint32_t type;
    uint32_t unknown;
    struct {
        uint32_t end;
        uint32_t index;
    } offset;
};

struct SizeUnit {
    uint32_t nannotations;
    uint32_t nobjects;
};

struct AnnotationUnit {
    uint32_t index;
    uint32_t value;
};

struct ShaderUnit {
    uint32_t technique;
    uint32_t pass;
    uint32_t unknown1;
    uint32_t unknown2;
    uint32_t unknown3;
    uint32_t size;
};

struct TextureUnit {
    uint32_t unknown1;
    uint32_t index;
    uint32_t unknown2;
    uint32_t unknown3;
    uint32_t type;
    uint32_t size;
};

#pragma pack(pop)

struct EffectFX2::Annotation : IEffect::IAnnotation {
    static const char *kEmpty;

    Annotation(AnnotationIndexUnit u)
        : symbolType(static_cast<MOJOSHADER_symbolType>(u.type)),
          index(u.index),
          valuePtr(0)
    {
    }
    ~Annotation() {
        delete valuePtr;
        valuePtr = 0;
    }

    bool booleanValue() const {
        if (symbolType == MOJOSHADER_SYMTYPE_BOOL) {
            return std::strcmp(cstring(), "true") == 0;
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

    void registerName(EffectFX2::String2AnnotationRefHash &value) {
        value.insert(valuePtr->toHashString(), this);
    }

    MOJOSHADER_symbolType symbolType;
    uint32_t index;
    IString *valuePtr;
};

const char *EffectFX2::Annotation::kEmpty = "";

struct EffectFX2::Annotateable {
    virtual ~Annotateable() {}
    Array<EffectFX2::Annotation *> annotationRefs;
};

struct EffectFX2::Parameter : EffectFX2::Annotateable, IEffect::IParameter {
    Parameter(EffectFX2 *p, EffectFX2::String2AnnotationRefHash *annotations)
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
        EffectFX2::Annotation *const *annotation = annotationRefs->find(name);
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

    void registerName(EffectFX2::String2ParameterRefHash &value) {
        value.insert(namePtr->toHashString(), this);
    }
    void registerSemantic(EffectFX2::String2ParameterRefHash &value) {
        value.insert(semanticPtr->toHashString(), this);
    }

    EffectFX2 *effectRef;
    EffectFX2::String2AnnotationRefHash *annotationRefs;
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

struct EffectFX2::Pass : EffectFX2::Annotateable, IEffect::IPass {
    Pass(EffectFX2 *p, IEffect::ITechnique *t, EffectFX2::String2AnnotationRefHash *annotations)
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
        EffectFX2::Annotation *const * annotation = annotationRefs->find(name);
        return annotation ? *annotation : 0;
    }
    const char*name() const {
        return reinterpret_cast<const char *>(namePtr->toByteArray());
    }
    void setState() {
    }
    void resetState() {
    }

    void registerName(EffectFX2::String2PassRefHash &value) {
        value.insert(namePtr->toHashString(), this);
    }

    EffectFX2 *effectRef;
    EffectFX2::String2AnnotationRefHash *annotationRefs;
    IEffect::ITechnique *techniqueRef;
    IString *namePtr;
};

struct EffectFX2::Technique : EffectFX2::Annotateable, IEffect::ITechnique {
    Technique(EffectFX2 *p, EffectFX2::String2AnnotationRefHash *annotations)
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
        EffectFX2::Annotation *const *annotation = annotationRefs->find(name);
        return annotation ? *annotation : 0;
    }
    const char *name() const {
        return reinterpret_cast<const char *>(namePtr->toByteArray());
    }
    void getPasses(Array<IEffect::IPass *> &passes) const {
        passes.copy(passRefs);
    }

    EffectFX2::Pass *addPass(PointerArray<EffectFX2::Pass> &passes) {
        EffectFX2::Pass *pass = passes.append(new EffectFX2::Pass(effectRef, this, annotationRefs));
        passRefs.append(pass);
        return pass;
    }
    void registerName(EffectFX2::String2TechniqueRefHash &value) {
        value.insert(namePtr->toHashString(), this);
    }

    EffectFX2 *effectRef;
    EffectFX2::String2AnnotationRefHash *annotationRefs;
    Array<IEffect::IPass *> passRefs;
    Hash<HashString, IEffect::IPass *> name2PassRefs;
    IString *namePtr;
};

struct EffectFX2::State {
    State(const StateUnit &unit)
        : type(unit.type)
    {
    }
    const uint32_t type;
};

struct EffectFX2::Texture {
    Texture(EffectFX2 *p, const TextureUnit &unit)
        : parentEffectRef(p),
          name(0),
          index(unit.index),
          type(unit.type)
    {
    }
    ~Texture() {
        delete name;
        name = 0;
        index = 0;
        type = 0;
    }

    void registerName(EffectFX2::String2TextureRefHash &value) {
        value.insert(name->toHashString(), this);
    }

    EffectFX2 *parentEffectRef;
    IString *name;
    uint32_t index;
    uint32_t type;
};

struct EffectFX2::Shader {
    Shader(EffectFX2 *p, const ShaderUnit &unit, const uint8_t *ptr)
        : parentEffectRef(p),
          data(MOJOSHADER_parse("glsl120", ptr, unit.size, 0, 0, 0, 0, 0, 0, this)),
          technique(unit.technique),
          pass(unit.pass)
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

    EffectFX2 *parentEffectRef;
    const MOJOSHADER_parseData *data;
    uint32_t technique;
    uint32_t pass;
};

EffectFX2::EffectFX2(IEncoding *encoding)
    : m_encoding(encoding)
{
}

EffectFX2::~EffectFX2()
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

bool EffectFX2::parse(const uint8_t *data, size_t size)
{
    if (!data || size == 0) {
        VPVL2_LOG(LOG(WARNING) << "Empty effect data is passed: ptr=" << reinterpret_cast<const void *>(data) << "size=" << size);
        return false;
    }

    Header header;
    uint8_t *ptr = const_cast<uint8_t *>(data);
    size_t rest = size;
    if (!internal::getTyped(ptr, rest, header)) {
        VPVL2_LOG(LOG(WARNING) << "Invalid header detected: ptr=" << reinterpret_cast<const void *>(ptr) << " size=" << rest);
        return false;
    }
    if (header.signature != 0xfeff0901) {
        VPVL2_LOG(LOG(WARNING) << "Invalid signature detected: data=" << header.signature);
        return false;
    }
    if (header.offset > size) {
        VPVL2_LOG(LOG(WARNING) << "Invalid offset detected: offset=" << header.offset << " size=" << size);
        return false;
    }
    ParseData parseData(ptr, size, rest);
    internal::drainBytes(header.offset, parseData.ptr, parseData.rest);
    VPVL2_LOG(VLOG(1) << "Base: ptr=" << reinterpret_cast<const void *>(parseData.base) << " size=" << parseData.size);

    ParameterHeader parameterHeader;
    if (!internal::getTyped(parseData.ptr, parseData.rest, parameterHeader)) {
        VPVL2_LOG(LOG(WARNING) << "Invalid parameter header detected: ptr=" << reinterpret_cast<const void *>(ptr) << " size=" << rest);
        return false;
    }
    if (!parseParameters(parseData, parameterHeader.nparameters)) {
        return false;
    }
    if (!parseTechniques(parseData, parameterHeader.ntechniques)) {
        return false;
    }

    SizeUnit sizeUnit;
    if (!internal::getTyped(parseData.ptr, parseData.rest, sizeUnit)) {
        VPVL2_LOG(LOG(WARNING) << "Invalid parameter header detected: ptr=" << reinterpret_cast<const void *>(ptr) << " size=" << rest);
        return false;
    }
    if (!parseAnnotations(parseData, sizeUnit.nannotations)) {
        return false;
    }
    if (!parseShaders(parseData, parseData.nshaders)) {
        return false;
    }
    if (!parseTextures(parseData, sizeUnit.nobjects - parseData.nshaders)) {
        return false;
    }

    return true;
}

bool EffectFX2::lookup(const ParseData &data, size_t offset, uint32_t &value)
{
    if (offset + sizeof(uint32_t) > data.size) {
        VPVL2_LOG(LOG(WARNING) << "Invalid offset detected: offset=" << offset << " size=" << data.size);
        return false;
    }
    else {
        value = *reinterpret_cast<const uint32_t *>(data.base + offset);
        return true;
    }
}

uint32_t EffectFX2::paddingSize(uint32_t size)
{
    return ((size + 3) / 4) * 4;
}

bool EffectFX2::parseString(const ParseData &data, size_t offset, IString *&string)
{
    uint32_t len;
    size_t rest = data.rest;
    if (offset >= data.size + sizeof(len)) {
        VPVL2_LOG(LOG(WARNING) << "String offset overflow detected: offset=" << offset << " size=" << data.size);
        return false;
    }
    uint8_t *ptr = const_cast<uint8_t *>(data.base) + offset;
    if (!internal::getTyped(ptr, rest, len)) {
        VPVL2_LOG(LOG(WARNING) << "Invalid string length detected: ptr=" << reinterpret_cast<const void *>(ptr) << " size=" << rest);
        return false;
    }
    if (!parseRawString(data, ptr, len, string)) {
        return false;
    }
    return true;
}

bool EffectFX2::parseRawString(const ParseData &data, const uint8_t *ptr, size_t size, IString *&string)
{
    if (size > data.rest) {
        VPVL2_LOG(LOG(WARNING) << "Invalid string length detected: size=" << size << " rest=" << data.rest);
        return false;
    }
    string = m_encoding->toString(ptr, size, IString::kShiftJIS);
    return true;
}

bool EffectFX2::parseAnnotationIndices(ParseData &data, Annotateable *annotate, const int nannotations)
{
    AnnotationIndexUnit unit, anno;
    for (int i = 0; i < nannotations; i++) {
        if (!internal::getTyped(data.ptr, data.rest, unit)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid annotation index detected: ptr=" << data.ptr << " size=" << data.rest);
            return false;
        }
        if (!lookup(data, unit.index, anno.index)) {
            return false;
        }
        if (!lookup(data, unit.type, anno.type)) {
            return false;
        }
        VPVL2_LOG(VLOG(2) << "AnnotationIndex: index=" << anno.index << " type=" << anno.type);
        annotate->annotationRefs.append(m_annotations.insert(anno.index, new Annotation(anno)));
    }
    return true;
}

bool EffectFX2::parseParameters(ParseData &data, const int nparameters)
{
    ParameterUnit unit;
    ParameterType type;
    for (int i = 0; i < nparameters; i++) {
        if (!internal::getTyped(data.ptr, data.rest, unit)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid parameter unit detected: ptr=" << reinterpret_cast<const void *>(data.ptr) << " rest=" << data.rest);
            return false;
        }
        Parameter *parameter = m_parameters.append(new Parameter(this, &m_name2AnnotationRef));
        if (!parseAnnotationIndices(data, parameter, unit.nannotations)) {
            return false;
        }
        uint8_t *typeptr = const_cast<uint8_t *>(data.base) + unit.offset.type;
        size_t rest = data.rest;
        if (!internal::getTyped(typeptr, rest, type)) {
            return false;
        }
        if (!parseString(data, type.offset.name, parameter->namePtr)) {
            return false;
        }
        parameter->registerName(m_name2ParameterRef);
        if (!parseString(data, type.offset.semantic, parameter->semanticPtr)) {
            return false;
        }
        parameter->registerSemantic(m_semantic2ParameterRef);
        parameter->symbolClass = static_cast<MOJOSHADER_symbolClass>(type.symclass);
        parameter->symbolType = static_cast<MOJOSHADER_symbolType>(type.symtype);
        VPVL2_LOG(VLOG(2) << "Parameter: class=" << type.symclass << " type=" << type.symtype << " flags=" << unit.flags << " annotations=" << unit.nannotations << " name=" << internal::cstr(parameter->namePtr, "(null)") << " semantic=" << internal::cstr(parameter->semanticPtr, "(null)"));
    }
    return true;
}

bool EffectFX2::parseTechniques(ParseData &data, const int ntechniques)
{
    TechniqueUnit unit;
    for (int i = 0; i < ntechniques; i++) {
        if (!internal::getTyped(data.ptr, data.rest, unit)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid technique unit detected: ptr=" << reinterpret_cast<const void *>(data.ptr) << " rest=" << data.rest);
            return false;
        }
        Technique *technique = m_techniques.append(new Technique(this, &m_name2AnnotationRef));
        if (!parseAnnotationIndices(data, technique, unit.nannotations)) {
            return false;
        }
        if (!parseString(data, unit.offset, technique->namePtr)) {
            return false;
        }
        technique->registerName(m_name2TechniqueRef);
        if (!parsePasses(data, technique, unit.npasses)) {
            return false;
        }
        VPVL2_LOG(VLOG(2) << "Technique: passes=" << unit.npasses << " annotations=" << unit.nannotations << " name=" << internal::cstr(technique->namePtr, "(null)"));
    }
    return true;
}

bool EffectFX2::parsePasses(ParseData &data, Technique *technique, const int npasses)
{
    PassUnit unit;
    for (int i = 0; i < npasses; i++) {
        if (!internal::getTyped(data.ptr, data.rest, unit)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid pass unit detected: ptr=" << reinterpret_cast<const void *>(data.ptr) << " rest=" << data.rest);
            return false;
        }
        Pass *pass = technique->addPass(m_passes);
        if (!parseAnnotationIndices(data, pass, unit.nannotations)) {
            return false;
        }
        if (!parseString(data, unit.offset, pass->namePtr)) {
            return false;
        }
        pass->registerName(m_name2PassRef);
        technique->name2PassRefs.insert(pass->namePtr->toHashString(), pass);
        if (!parseStates(data, unit.nstates)) {
            return false;
        }
        VPVL2_LOG(VLOG(2) << "Pass: states=" << unit.nstates << " annotations=" << unit.nannotations << " name=" << internal::cstr(pass->namePtr, "(null)"));
    }
    return true;
}

bool EffectFX2::parseStates(ParseData &data, const int nstates)
{
    StateUnit unit;
    for (int i = 0; i < nstates; i++) {
        if (!internal::getTyped(data.ptr, data.rest, unit)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid state unit detected: ptr=" << reinterpret_cast<const void *>(data.ptr) << " rest=" << data.rest);
            return false;
        }
        uint32_t index, end;
        lookup(data, unit.offset.index, index);
        lookup(data, unit.offset.end, end);
        m_states.append(new State(unit));
        if (unit.type == 0x92 || unit.type == 0x93) {
            data.nshaders++;
        }
        VPVL2_LOG(VLOG(2) << "State: type=" << unit.type << " index=" << index << " end=" << end);
    }
    return true;
}

bool EffectFX2::parseAnnotations(ParseData &data, const int nannotations)
{
    AnnotationUnit unit;
    for (int i = 0; i < nannotations; i++) {
        if (!internal::getTyped(data.ptr, data.rest, unit)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid annotation unit detected: ptr=" << reinterpret_cast<const void *>(data.ptr) << " rest=" << data.rest);
            return false;
        }
        IString *value = 0;
        if (!parseRawString(data, data.ptr, unit.value, value)) {
            return false;
        }
        if (Annotation *const *annotationPtr = m_annotations.find(unit.index)) {
            Annotation *annotationRef = (*annotationPtr);
            annotationRef->valuePtr = value;
            annotationRef->registerName(m_name2AnnotationRef);
            VPVL2_LOG(VLOG(2) << "Annotation: found=true index=" << unit.index << " value=" << internal::cstr(value, "(null)"));
        }
        else {
            VPVL2_LOG(VLOG(2) << "Annotation: found=false index=" << unit.index << " value=" << internal::cstr(value, "(null)"));
            delete value;
        }
        internal::drainBytes(paddingSize(unit.value), data.ptr, data.rest);
    }
    return true;
}

bool EffectFX2::parseShaders(ParseData &data, const int nshaders)
{
    ShaderUnit unit;
    for (int i = 0; i < nshaders; i++) {
        if (!internal::getTyped(data.ptr, data.rest, unit)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid shader unit detected: ptr=" << reinterpret_cast<const void *>(data.ptr) << " rest=" << data.rest);
            return false;
        }
        m_shaders.append(new Shader(this, unit, data.ptr));
        VPVL2_LOG(VLOG(2) << "Shader: technique=" << unit.technique << " pass=" << unit.pass << " size=" << unit.size);
        internal::drainBytes(unit.size, data.ptr, data.rest);
    }
    return true;
}

bool EffectFX2::parseTextures(ParseData &data, const int ntextures)
{
    TextureUnit unit;
    for (int i = 0; i < ntextures; i++) {
        if (!internal::getTyped(data.ptr, data.rest, unit)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid texture unit detected: ptr=" << reinterpret_cast<const void *>(data.ptr) << " rest=" << data.rest);
            return false;
        }
        Texture *texture = m_textures.append(new Texture(this, unit));
        if (unit.size > 0) {
            size_t size = paddingSize(unit.size);
            if (!parseRawString(data, data.ptr, unit.size, texture->name)) {
                return false;
            }
            texture->registerName(m_name2TextureRef);
            internal::drainBytes(size, data.ptr, data.rest);
        }
        VPVL2_LOG(VLOG(2) << "Texture: type=" << unit.type << " index=" << unit.index << " name=" << internal::cstr(texture->name, "(null)"));
    }
    return true;
}

} /* namespace fx */
} /* namespace extensions */
} /* namespace vpvl2 */
