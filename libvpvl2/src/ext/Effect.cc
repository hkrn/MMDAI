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
#include <vpvl2/extensions/fx/Effect.h>

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
    uint32_t type;
    uint32_t typeClass;
    struct {
        uint32_t name;
        uint32_t semantic;
    } offset;
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
        uint32_t start;
    } offset;
};

struct SizeUnit {
    uint32_t nannotations;
    uint32_t nobjects;
};

struct AnnotationUnit {
    uint32_t index;
    uint32_t name;
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

struct Effect::Annotation {
    Annotation(AnnotationIndexUnit u)
        : unit(u),
          name(0)
    {
    }
    ~Annotation() {
        delete name;
        name = 0;
    }
    AnnotationIndexUnit unit;
    IString *name;
};

struct Effect::Annotateable {
    virtual ~Annotateable() {}
    Array<Annotation *> annotationRefs;
};

struct Effect::Parameter : public Effect::Annotateable {
    Parameter()
        : name(0),
          semantic(0)
    {
    }
    ~Parameter() {
        delete name;
        name = 0;
        delete semantic;
        semantic = 0;
    }

    IString *name;
    IString *semantic;
};

struct Effect::Pass : public Effect::Annotateable {
    Pass(Effect::Technique *t)
        : techniqueRef(t),
          name(0)
    {
    }
    ~Pass() {
        techniqueRef = 0;
        delete name;
        name = 0;
    }

    Effect::Technique *techniqueRef;
    IString *name;
};

struct Effect::Technique : public Effect::Annotateable {
    Technique()
        : name(0)
    {
    }
    ~Technique() {
        delete name;
        name = 0;
    }

    Effect::Pass *addPass(PointerArray<Effect::Pass> &passes) {
        Effect::Pass *pass = passes.append(new Effect::Pass(this));
        passRefs.append(pass);
        return pass;
    }

    Array<Effect::Pass *> passRefs;
    IString *name;
};

struct Effect::State {
    State(const StateUnit &unit)
        : type(unit.type)
    {
    }
    const uint32_t type;
};

struct Effect::Texture {
    Texture(const TextureUnit &unit)
        : name(0),
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

    IString *name;
    uint32_t index;
    uint32_t type;
};

struct Effect::Shader {
    Shader(const ShaderUnit &unit, const uint8_t *ptr)
        : data(MOJOSHADER_parse("glsl", ptr, unit.size, 0, 0, 0, 0, 0, 0, this)),
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

    const MOJOSHADER_parseData *data;
    uint32_t technique;
    uint32_t pass;
};

Effect::Effect(IEncoding *encoding)
    : m_encoding(encoding)
{
}

Effect::~Effect()
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

bool Effect::parse(const uint8_t *data, size_t size)
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

bool Effect::lookup(const ParseData &data, size_t offset, uint32_t &value)
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

uint32_t Effect::paddingSize(uint32_t size)
{
    return ((size + 3) / 4) * 4;
}

bool Effect::parseString(const ParseData &data, size_t offset, IString *&string)
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

bool Effect::parseRawString(const ParseData &data, const uint8_t *ptr, size_t size, IString *&string)
{
    if (size > data.rest) {
        VPVL2_LOG(LOG(WARNING) << "Invalid string length detected: size=" << size << " rest=" << data.rest);
        return false;
    }
    string = m_encoding->toString(ptr, size, IString::kShiftJIS);
    return true;
}

bool Effect::parseAnnotationIndices(ParseData &data, Annotateable *annotate, const int nannotations)
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

bool Effect::parseParameters(ParseData &data, const int nparameters)
{
    ParameterUnit unit;
    ParameterType type;
    for (int i = 0; i < nparameters; i++) {
        if (!internal::getTyped(data.ptr, data.rest, unit)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid parameter unit detected: ptr=" << reinterpret_cast<const void *>(data.ptr) << " rest=" << data.rest);
            return false;
        }
        Parameter *parameter = m_parameters.append(new Parameter());
        if (!parseAnnotationIndices(data, parameter, unit.nannotations)) {
            return false;
        }
        uint8_t *typeptr = const_cast<uint8_t *>(data.base) + unit.offset.type;
        size_t rest = data.rest;
        if (!internal::getTyped(typeptr, rest, type)) {
            return false;
        }
        if (!parseString(data, type.offset.name, parameter->name)) {
            return false;
        }
        if (!parseString(data, type.offset.semantic, parameter->semantic)) {
            return false;
        }
        VPVL2_LOG(VLOG(2) << "Parameter: class=" << type.typeClass << " type=" << type.type << " flags=" << unit.flags << " annotations=" << unit.nannotations << " name=" << internal::cstr(parameter->name, "(null)") << " semantic=" << internal::cstr(parameter->semantic, "(null)"));
    }
    return true;
}

bool Effect::parseTechniques(ParseData &data, const int ntechniques)
{
    TechniqueUnit unit;
    for (int i = 0; i < ntechniques; i++) {
        if (!internal::getTyped(data.ptr, data.rest, unit)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid technique unit detected: ptr=" << reinterpret_cast<const void *>(data.ptr) << " rest=" << data.rest);
            return false;
        }
        Technique *technique = m_techniques.append(new Technique());
        if (!parseAnnotationIndices(data, technique, unit.nannotations)) {
            return false;
        }
        if (!parseString(data, unit.offset, technique->name)) {
            return false;
        }
        if (!parsePasses(data, technique, unit.npasses)) {
            return false;
        }
        VPVL2_LOG(VLOG(2) << "Technique: passes=" << unit.npasses << " annotations=" << unit.nannotations << " name=" << internal::cstr(technique->name, "(null)"));
    }
    return true;
}

bool Effect::parsePasses(ParseData &data, Technique *technique, const int npasses)
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
        if (!parseString(data, unit.offset, pass->name)) {
            return false;
        }
        if (!parseStates(data, unit.nstates)) {
            return false;
        }
        VPVL2_LOG(VLOG(2) << "Pass: states=" << unit.nstates << " annotations=" << unit.nannotations << " name=" << internal::cstr(pass->name, "(null)"));
    }
    return true;
}

bool Effect::parseStates(ParseData &data, const int nstates)
{
    StateUnit unit;
    for (int i = 0; i < nstates; i++) {
        if (!internal::getTyped(data.ptr, data.rest, unit)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid state unit detected: ptr=" << reinterpret_cast<const void *>(data.ptr) << " rest=" << data.rest);
            return false;
        }
        /*
        IString *s = 0;
        if (parseRawString(data, data.base, unit.offset.end - unit.offset.start, s)) {
            return false;
        }
        */
        m_states.append(new State(unit));
        if (unit.type == 0x92 || unit.type == 0x93) {
            data.nshaders++;
        }
        VPVL2_LOG(VLOG(2) << "State: type=" << unit.type);
    }
    return true;
}

bool Effect::parseAnnotations(ParseData &data, const int nannotations)
{
    AnnotationUnit unit;
    for (int i = 0; i < nannotations; i++) {
        if (!internal::getTyped(data.ptr, data.rest, unit)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid annotation unit detected: ptr=" << reinterpret_cast<const void *>(data.ptr) << " rest=" << data.rest);
            return false;
        }
        IString *name = 0;
        if (!parseRawString(data, data.ptr, unit.name, name)) {
            return false;
        }
        if (Annotation *const *anno = m_annotations.find(unit.index)) {
            (*anno)->name = name;
            VPVL2_LOG(VLOG(2) << "Annotation: found=true index=" << unit.index << " name=" << internal::cstr(name, "(null)"));
        }
        else {
            VPVL2_LOG(VLOG(2) << "Annotation: found=false index=" << unit.index << " name=" << internal::cstr(name, "(null)"));
            delete name;
        }
        internal::drainBytes(paddingSize(unit.name), data.ptr, data.rest);
    }
    return true;
}

bool Effect::parseShaders(ParseData &data, const int nshaders)
{
    ShaderUnit unit;
    for (int i = 0; i < nshaders; i++) {
        if (!internal::getTyped(data.ptr, data.rest, unit)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid shader unit detected: ptr=" << reinterpret_cast<const void *>(data.ptr) << " rest=" << data.rest);
            return false;
        }
        m_shaders.append(new Shader(unit, data.ptr));
        VPVL2_LOG(VLOG(2) << "Shader: technique=" << unit.technique << " pass=" << unit.pass << " size=" << unit.size);
        internal::drainBytes(unit.size, data.ptr, data.rest);
    }
    return true;
}

bool Effect::parseTextures(ParseData &data, const int ntextures)
{
    TextureUnit unit;
    for (int i = 0; i < ntextures; i++) {
        if (!internal::getTyped(data.ptr, data.rest, unit)) {
            VPVL2_LOG(LOG(WARNING) << "Invalid texture unit detected: ptr=" << reinterpret_cast<const void *>(data.ptr) << " rest=" << data.rest);
            return false;
        }
        Texture *texture = m_textures.append(new Texture(unit));
        if (unit.size > 0) {
            size_t size = paddingSize(unit.size);
            if (!parseRawString(data, data.ptr, unit.size, texture->name)) {
                return false;
            }
            internal::drainBytes(size, data.ptr, data.rest);
        }
        VPVL2_LOG(VLOG(2) << "Texture: type=" << unit.type << " index=" << unit.index << " name=" << internal::cstr(texture->name, "(null)"));
    }
    return true;
}

} /* namespace fx */
} /* namespace extensions */
} /* namespace vpvl2 */
