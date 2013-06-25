/**

 Copyright (c) 2010-2013  hkrn

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

/* This class no longer will be used by nvFX */

#include "vpvl2/vpvl2.h"
#include "vpvl2/internal/util.h"

#include "vpvl2/fx/Effect.h"

#include <map>
#include "tinyxml2.h"

using namespace tinyxml2;

namespace vpvl2
{
namespace fx
{

struct Effect::PrivateContext {
    union TypeUnion {
        TypeUnion(long v)
            : ival(v)
        {
        }
        TypeUnion(double v)
            : fval(v)
        {
        }
        int ival;
        float fval;
    };

    struct CFAnnotation;
    struct CFParameter;
    struct CFPass;
    struct CFTechnique;
    typedef std::map<std::string, const std::string *> CFSourceRefMap;
    typedef std::map<std::string, const CFAnnotation *> CFAnnotationRefMap;
    typedef std::map<std::string, const CFParameter *> CFParameterRefMap;
    typedef std::map<std::string, const CFPass *> CFPassRefMap;
    typedef std::map<std::string, const CFTechnique *> CFTechniqueRefMap;

    struct CFAnnotation : IEffect::Annotation {
        CFAnnotation(const std::string &name)
            : nameString(name),
              base(IEffect::Parameter::kUnknownParameterType),
              full(IEffect::Parameter::kUnknownParameterType)
        {
        }
        ~CFAnnotation() {
        }

        bool booleanValue() const {
            if (base == IEffect::Parameter::kBoolean) {
                return valueString == "true";
            }
            return false;
        }
        int integerValue() const {
            if (base == IEffect::Parameter::kInteger) {
                return std::strtol(valueString.c_str(), 0, 10);
            }
            return 0;
        }
        const int *integerValues(int *size) const {
            if (base == IEffect::Parameter::kInteger) {
                std::istringstream stream(valueString);
                std::string token;
                values.clear();
                while (std::getline(stream, token, ' ')) {
                    values.push_back(std::strtol(token.c_str(), 0, 10));
                }
                *size = values.size();
                return reinterpret_cast<const int *>(values.data());
            }
            size = 0;
            return 0;
        }
        float floatValue() const {
            if (base == IEffect::Parameter::kFloat) {
                return std::strtod(valueString.c_str(), 0);
            }
            return 0;
        }
        const float *floatValues(int *size) const {
            if (base == IEffect::Parameter::kFloat) {
                std::istringstream stream(valueString);
                std::string token;
                values.clear();
                while (std::getline(stream, token, ' ')) {
                    values.push_back(std::strtod(token.c_str(), 0));
                }
                *size = values.size();
                return reinterpret_cast<const float *>(values.data());
            }
            size = 0;
            return 0;
        }
        const char *stringValue() const {
            return valueString.c_str();
        }

        mutable std::vector<TypeUnion> values;
        std::string nameString;
        std::string valueString;
        IEffect::Parameter::Type base;
        IEffect::Parameter::Type full;
    };
    struct CFParameter : IEffect::Parameter {
        CFParameter(const std::string &sid, Effect *effect)
            : parentEffect(effect),
              symbolString(sid),
              base(IEffect::Parameter::kUnknownParameterType),
              full(IEffect::Parameter::kUnknownParameterType)
        {
        }
        ~CFParameter() {
        }

        IEffect *parentEffectRef() const {
            return parentEffect;
        }
        const IEffect::Annotation *annotationRef(const char *name) const {
            CFAnnotationRefMap::const_iterator it = annotations.find(name);
            return it != annotations.end() ? it->second : 0;
        }
        const char *name() const {
            return symbolString.c_str();
        }
        const char *semantic() const {
            return semanticString.c_str();
        }
        Type type() const {
            return full;
        }
        VariableType variableType() const {
            return kUniform;
        }
        void connect(IEffect::Parameter *destinationParameter) {
        }
        void reset() {
        }
        void getValue(int &value) const {
        }
        void getValue(float &value) const {
        }
        void getValue(Vector3 &value) const {
        }
        void getValue(Vector4 &value) const {
        }
        void getMatrix(float *value) const {
        }
        void getArrayDimension(int &value) const {
            value = 1;
        }
        void getArrayTotalSize(int &value) const {
            value = 1;
        }
        void getTextureRef(intptr_t &value) const {
            value = 0;
        }
        void getSamplerStateRefs(Array<IEffect::SamplerState *> &value) const {
        }
        void setValue(bool value) {
        }
        void setValue(int value) {
        }
        void setValue(float value) {
        }
        void setValue(const Vector3 &value) {
        }
        void setValue(const Vector4 &value) {
        }
        void setValue(const Vector4 *value) {
        }
        void setMatrix(const float *value) {
        }
        void setSampler(const ITexture *value) {
        }
        void setTexture(const ITexture *value) {
        }
        void setTexture(intptr_t value) {
        }
        void setPointer(const void *ptr, vsize size, vsize stride, Type type) {
        }

        Effect *parentEffect;
        CFAnnotationRefMap annotations;
        std::vector<TypeUnion> values;
        std::string symbolString;
        std::string semanticString;
        std::string value;
        Type base;
        Type full;
    };
    struct CFTechnique : IEffect::Technique {
        CFTechnique(const std::string &sid, Effect *effect)
            : parentEffect(effect),
              nameString(sid)
        {
        }
        ~CFTechnique() {
        }

        IEffect *parentEffectRef() const {
            return parentEffect;
        }
        IEffect::Pass *findPass(const char *name) const {
            CFPassRefMap::const_iterator it = passes.find(name);
            return it != passes.end() ? const_cast<CFPass *>(it->second) : 0;
        }
        const IEffect::Annotation *annotationRef(const char *name) const {
            CFAnnotationRefMap::const_iterator it = annotations.find(name);
            return it != annotations.end() ? it->second : 0;
        }
        const char *name() const {
            return nameString.c_str();
        }
        void getPasses(Array<IEffect::Pass *> &value) const {
            value.clear();
            for (CFPassRefMap::const_iterator it = passes.begin(); it != passes.end(); it++) {
                value.append(const_cast<CFPass *>(it->second));
            }
        }

        Effect *parentEffect;
        CFAnnotationRefMap annotations;
        CFPassRefMap passes;
        std::string nameString;
    };
    struct CFState : IEffect::SamplerState {
        CFState(const std::string &name, const std::string &value)
            : nameString(name),
              valueString(value)
        {
        }
        ~CFState() {
        }

        const char *name() const {
            return nameString.c_str();
        }
        IEffect::Parameter::Type type() const {
            return IEffect::Parameter::kUnknownParameterType;
        }
        IEffect::Parameter *parameterRef() const {
            return 0;
        }
        void getValue(int &value) const {
            value = std::strtol(valueString.c_str(), 0, 10);
        }

        std::string nameString;
        std::string valueString;
    };
    struct CFPass : IEffect::Pass {
        CFPass(CFTechnique *t)
            : technique(t)
        {
        }
        ~CFPass() {
            technique = 0;
        }

        IEffect::Technique *parentTechniqueRef() const {
            return technique;
        }
        const IEffect::Annotation *annotationRef(const char *name) const {
            CFAnnotationRefMap::const_iterator it = annotations.find(name);
            return it != annotations.end() ? it->second : 0;
        }
        const char *name() const {
            return nameString.c_str();
        }
        void setState() {
        }
        void resetState() {
        }

        CFSourceRefMap sources;
        CFAnnotationRefMap annotations;
        std::vector<CFParameter *> parameters;
        std::vector<CFState> states;
        std::string nameString;
        CFTechnique *technique;
    };

    typedef std::map<std::string, std::string> SourceMap;
    typedef std::map<std::string, CFAnnotation> AnnotationMap;
    typedef std::map<std::string, CFParameter> ParameterMap;
    typedef std::map<std::string, CFTechnique> TechniqueMap;
    typedef std::map<std::string, CFPass> PassMap;

    static inline std::string toStringSafe(const char *value) {
        return value ? value : std::string();
    }
    static inline bool equalsConstant(const char *left, const char *const right) {
        return left && std::strncmp(left, right, std::strlen(right)) == 0;
    }
    static inline bool equalsToElement(const XMLElement *element, const char *const name) {
        return element && equalsConstant(element->Name(), name);
    }
    static bool equalsToTypedElement(const XMLElement *element, IEffect::Parameter::Type &type, IEffect::Parameter::Type &base) {
        type = IEffect::Parameter::kUnknownParameterType;
        base = IEffect::Parameter::kUnknownParameterType;
        if (element) {
            const char *name = element->Name();
            const char firstLetter = name[0];
            switch (firstLetter) {
            case 'b':
                base = IEffect::Parameter::kBoolean;
                return handleBoolType(name, type);
            case 'i':
                base = IEffect::Parameter::kInteger;
                return handleIntType(name, type);
            case 'f':
                base = IEffect::Parameter::kFloat;
                return handleFloatType(name, type);
            case 's': {
                static const char kTypeSamplerString[] = "sampler";
                if (equalsConstant(name, "string")) {
                    base = IEffect::Parameter::kString;
                    type = IEffect::Parameter::kString;
                    return true;
                }
                else if (equalsConstant(name, "surface")) {
                    base = IEffect::Parameter::kTexture;
                    type = IEffect::Parameter::kTexture;
                    return true;
                }
                else if (equalsConstant(name, kTypeSamplerString)) {
                    const char *ptr = name + sizeof(kTypeSamplerString) - 1;
                    base = IEffect::Parameter::kSampler;
                    switch (ptr[0]) {
                    case '1':
                        type = IEffect::Parameter::kSampler1D;
                        return true;
                    case '2':
                        type = IEffect::Parameter::kSampler2D;
                        return true;
                    case '3':
                        type = IEffect::Parameter::kSampler3D;
                        return true;
                    case 'C':
                        type = IEffect::Parameter::kSamplerCube;
                        return true;
                    }
                }
                break;
            }
            default:
                break;
            }
        }
        return false;
    }
    static bool handleBoolType(const char *name, IEffect::Parameter::Type &type) {
        static const char kTypeBoolString[] = "bool";
        if (equalsConstant(name, kTypeBoolString)) {
            int nelements = strtoul(name + sizeof(kTypeBoolString) - 1, 0, 10);
            switch (nelements) {
            case 4:
                type = IEffect::Parameter::kBool4;
                return true;
            case 3:
                type = IEffect::Parameter::kBool3;
                return true;
            case 2:
                type = IEffect::Parameter::kBool2;
                return true;
            case 1:
            default:
                type = IEffect::Parameter::kBool1;
                return true;
            }
        }
        return false;
    }
    static bool handleIntType(const char *name, IEffect::Parameter::Type &type) {
        static const char kTypeIntString[] = "int";
        if (equalsConstant(name, kTypeIntString)) {
            int nelements = strtoul(name + sizeof(kTypeIntString) - 1, 0, 10);
            switch (nelements) {
            case 4:
                type = IEffect::Parameter::kInt4;
                return true;
            case 3:
                type = IEffect::Parameter::kInt3;
                return true;
            case 2:
                type = IEffect::Parameter::kInt2;
                return true;
            case 1:
            default:
                type = IEffect::Parameter::kInt1;
                return true;
            }
        }
        return false;
    }
    static bool handleFloatType(const char *name, IEffect::Parameter::Type &type) {
        static const char kTypeFloatString[] = "float";
        if (equalsConstant(name, kTypeFloatString)) {
            const char *ptr = name + sizeof(kTypeFloatString) - 1;
            int nelements = strtoul(ptr, 0, 10);
            switch (nelements) {
            case 4:
                type = IEffect::Parameter::kFloat4;
                return true;
            case 3:
                type = IEffect::Parameter::kFloat3;
                return true;
            case 2:
                type = IEffect::Parameter::kFloat2;
                return true;
            case 1:
                type = IEffect::Parameter::kFloat1;
                return true;
            default:
                if (*ptr == 0) {
                    type = IEffect::Parameter::kFloat1;
                }
                else if (equalsConstant(ptr, "4x4")) {
                    type = IEffect::Parameter::kFloat4x4;
                }
                else if (equalsConstant(ptr, "3x3")) {
                    type = IEffect::Parameter::kFloat3x3;
                }
                else if (equalsConstant(ptr, "2x2")) {
                    type = IEffect::Parameter::kFloat2x2;
                }
                else {
                    type = IEffect::Parameter::kFloat1;
                }
                return true;
            }
        }
        return false;
    }

    void parseLibraryEffects(const XMLElement *element) {
        for (const XMLNode *node = element->FirstChild(); node; node = node->NextSibling()) {
            const XMLElement *element = node->ToElement();
            if (equalsToElement(element, "effect")) {
                parseEffect(element);
            }
        }
    }
    void parseEffect(const XMLElement *element) {
        for (const XMLNode *node = element->FirstChild(); node; node = node->NextSibling()) {
            const XMLElement *element = node->ToElement();
            if (equalsToElement(element, "profile_GLSL")) {
                parseEffectProfile(element);
            }
        }
    }
    void parseEffectProfile(const XMLElement *element) {
        for (const XMLNode *node = element->FirstChild(); node; node = node->NextSibling()) {
            const XMLElement *element = node->ToElement();
            if (equalsToElement(element, "annotate")) {
                parseAnnotate(element);
            }
            else if (equalsToElement(element, "code")) {
                parseCode(element);
            }
            else if (equalsToElement(element, "newparam")) {
                parseNewParam(element);
            }
            else if (equalsToElement(element, "technique")) {
                parseTechnique(element);
            }
        }
    }
    const CFAnnotation *parseAnnotate(const XMLElement *parentElement) {
        const std::string &name = toStringSafe(parentElement->Name());
        AnnotationMap::const_iterator it = annotations.find(name), it2;
        if (it != annotations.end()) {
            return &it->second;
        }
        else {
            CFAnnotation annotation(name);
            for (const XMLNode *node = parentElement->FirstChild(); node; node = node->NextSibling()) {
                const XMLElement *element = node->ToElement();
                if (equalsToTypedElement(element, annotation.full, annotation.base)) {
                    annotation.valueString.assign(toStringSafe(element->GetText()));
                }
            }
            it2 = annotations.insert(std::make_pair(name, annotation)).first;
            return &it2->second;
        }
    }
    const std::string *parseCode(const XMLElement *parentElement) {
        const std::string &sid = toStringSafe(parentElement->Attribute("sid"));
        SourceMap::const_iterator it = sources.find(sid), it2;
        if (it != sources.end()) {
            return &it->second;
        }
        else {
            const std::string &code = toStringSafe(parentElement->GetText());
            it2 = sources.insert(std::make_pair(sid, code)).first;
            return &it2->second;
        }
    }
    const CFParameter *parseNewParam(const XMLElement *parentElement) {
        const std::string &sid = toStringSafe(parentElement->Attribute("sid"));
        ParameterMap::const_iterator it = parameters.find(sid), it2;
        if (it != parameters.end()) {
            return &it->second;
        }
        else {
            CFParameter parameter(sid, 0);
            for (const XMLNode *node = parentElement->FirstChild(); node; node = node->NextSibling()) {
                const XMLElement *element = node->ToElement();
                if (equalsToElement(element, "annotate")) {
                    const CFAnnotation *annotationRef = parseAnnotate(element);
                    parameter.annotations.insert(std::make_pair(annotationRef->nameString, annotationRef));
                }
                if (equalsToElement(element, "semantic")) {
                    parameter.semanticString.assign(toStringSafe(element->GetText()));
                }
                else if (equalsToTypedElement(element, parameter.full, parameter.base)) {
                    parameter.value.assign(toStringSafe(element->GetText()));
                }
            }
            it2 = parameters.insert(std::make_pair(sid, parameter)).first;
            return &it2->second;
        }
    }
    const CFTechnique *parseTechnique(const XMLElement *parentElement) {
        const std::string &sid = toStringSafe(parentElement->Attribute("sid"));
        TechniqueMap::const_iterator it = techniques.find(sid), it2;
        if (it != techniques.end()) {
            return &it->second;
        }
        else {
            CFTechnique technique(sid, 0);
            for (const XMLNode *node = parentElement->FirstChild(); node; node = node->NextSibling()) {
                const XMLElement *element = node->ToElement();
                if (equalsToElement(element, "annotate")) {
                    const CFAnnotation *annotationRef = parseAnnotate(element);
                    technique.annotations.insert(std::make_pair(annotationRef->nameString, annotationRef));
                }
                else if (equalsToElement(element, "pass")) {
                    const CFPass *passRef = parsePasses(element, &technique);
                    technique.passes.insert(std::make_pair(passRef->nameString, passRef));
                }
            }
            it2 = techniques.insert(std::make_pair(sid, technique)).first;
            return &it2->second;
        }
    }
    void parseStates(const XMLElement *parentElement, CFPass *pass) {
        IEffect::Parameter::Type type, base;
        for (const XMLNode *node = parentElement->FirstChild(); node; node = node->NextSibling()) {
            const XMLElement *element = node->ToElement();
            const XMLElement *typedElement = element->FirstChildElement();
            if (equalsToTypedElement(typedElement, type, base)) {
                CFState state(toStringSafe(element->Name()), toStringSafe(element->Attribute("value")));
                pass->states.push_back(state);
            }
        }
    }
    const CFPass *parsePasses(const XMLElement *parentElement, CFTechnique *parentTechnique) {
        const std::string &sid = toStringSafe(parentElement->Attribute("sid"));
        const std::string &key = parentTechnique->nameString + "/" + sid;
        PassMap::const_iterator it = passes.find(key), it2;
        if (it != passes.end()) {
            return &it->second;
        }
        else {
            CFPass pass(parentTechnique);
            for (const XMLNode *node = parentElement->FirstChild(); node; node = node->NextSibling()) {
                const XMLElement *element = node->ToElement();
                if (equalsToElement(element, "annotate")) {
                    const CFAnnotation *annotationRef = parseAnnotate(element);
                    pass.annotations.insert(std::make_pair(annotationRef->nameString, annotationRef));
                }
                else if (equalsToElement(element, "program")) {
                    parseProgram(element, &pass);
                }
                else if (equalsToElement(element, "states")) {
                    parseStates(element, &pass);
                }
            }
            it2 = passes.insert(std::make_pair(key, pass)).first;
            return &it2->second;
        }
    }
    void parseProgram(const XMLElement *parentElement, CFPass *pass) {
        for (const XMLNode *node = parentElement->FirstChild(); node; node = node->NextSibling()) {
            const XMLElement *element = node->ToElement();
            if (equalsToElement(element, "shader")) {
                const std::string &stage = toStringSafe(element->Attribute("stage"));
                if (const XMLElement *sourceElement = element->FirstChildElement("source")) {
                    if (const XMLElement *importElement = sourceElement->FirstChildElement("import")) {
                        const std::string &ref = toStringSafe(importElement->Attribute("ref"));
                        SourceMap::const_iterator it = sources.find(ref);
                        if (it != sources.end()) {
                            pass->sources.insert(std::make_pair(stage, &it->second));
                        }
                    }
                }
            }
            else if (equalsToElement(element, "bind_uniform")) {
                const std::string &symbol = toStringSafe(element->Attribute("symbol"));
                if (const XMLElement *parameterElement = element->FirstChildElement("param")) {
                    const std::string &ref = toStringSafe(parameterElement->Attribute("ref"));
                    ParameterMap::iterator it = parameters.find(ref);
                    if (it != parameters.end()) {
                        CFParameter *parameter = &it->second;
                        parameter->symbolString.assign(symbol);
                        pass->parameters.push_back(parameter);
                    }
                }
            }
        }
    }

    AnnotationMap annotations;
    TechniqueMap techniques;
    PassMap passes;
    SourceMap sources;
    ParameterMap parameters;
};

Effect::Effect()
    : m_context(0)
{
    m_context = new PrivateContext();
}

Effect::~Effect()
{
    delete m_context;
    m_context = 0;
}

void Effect::load(const char *path)
{
    XMLDocument document;
    if (document.LoadFile(path) == XML_NO_ERROR) {
        if (const XMLElement *collada = document.RootElement()) {
            for (const XMLNode *node = collada->FirstChild(); node; node = node->NextSibling()) {
                const XMLElement *element = node->ToElement();
                if (PrivateContext::equalsToElement(element, "library_effects")) {
                    m_context->parseLibraryEffects(element);
                }
            }
        }
    }
    else {
        VPVL2_LOG(WARNING, "XMLError: id=" << document.ErrorID() << " path=" << path);
    }
}

void *Effect::internalContext() const
{
    return 0;
}

void *Effect::internalPointer() const
{
    return 0;
}

void Effect::getOffscreenRenderTargets(Array<OffscreenRenderTarget> &value) const
{
}

void Effect::getInteractiveParameters(Array<IEffect::Parameter *> &value) const
{
}

IEffect *Effect::parentEffectRef() const
{
    return 0;
}

void Effect::setParentEffectRef(IEffect *value)
{
}

extensions::gl::FrameBufferObject *Effect::parentFrameBufferObject() const
{
    return 0;
}

void Effect::createFrameBufferObject()
{
}

IEffect::ScriptOrderType Effect::scriptOrderType() const
{
    return kStandard;
}

void Effect::addOffscreenRenderTarget(ITexture *textureRef, Parameter *textureParameterRef, Parameter *samplerParameterRef)
{
}

void Effect::addInteractiveParameter(IEffect::Parameter *value)
{
}

void Effect::addRenderColorTargetIndex(int targetIndex)
{
}

void Effect::removeRenderColorTargetIndex(int targetIndex)
{
}

void Effect::clearRenderColorTargetIndices()
{
}

void Effect::setScriptOrderType(ScriptOrderType value)
{
}

bool Effect::hasRenderColorTargetIndex(int targetIndex) const
{
}

IEffect::Parameter *Effect::findVaryingParameter(const char *name) const
{
    return 0;
}

IEffect::Parameter *Effect::findUniformParameter(const char *name) const
{
    return 0;
}

IEffect::Technique *Effect::findTechnique(const char *name) const
{
    return 0;
}

void Effect::getParameterRefs(Array<Parameter *> &parameters) const
{
}

void Effect::getTechniqueRefs(Array<Technique *> &techniques) const
{
}

} /* namespace fx */
} /* namespace vpvl2 */
