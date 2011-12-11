/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2011  hkrn                                    */
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

#include "vpvl/vpvl.h"
#include "vpvl/internal/util.h"
#include "vpvl/Project.h"

#include <string>
#include <sstream>

namespace vpvl
{

struct Parser {
public:
    enum State {
        kInitial,
        kProject,
        kSettings,
        kPhysics,
        kModels,
        kModel,
        kAssets,
        kAsset,
        kMotions,
        kIKMotion,
        kAssetMotion,
        kAnimation,
        kBoneMotion,
        kVerticesMotion,
        kCameraMotion,
        kLightMotion
    };
    typedef Hash<HashString, std::string> StringHash;
    typedef Hash<HashPtr, StringHash *> PtrHash;
    const static int kBufferSize = 32;

    Parser()
        : state(kInitial),
          depth(0),
          currentAsset(0),
          currentModel(0),
          currentMotion(0)
    {
        internal::zerofill(key, sizeof(key));
    }
    ~Parser() {
        state = kInitial;
        depth = 0;
        assets.releaseAll();
        models.releaseAll();
        motions.releaseAll();
        keys.releaseArrayAll();
        localModelSettingValues.releaseAll();
        localAssetSettingValues.releaseAll();
        delete currentAsset;
        currentAsset = 0;
        delete currentModel;
        currentModel = 0;
        delete currentMotion;
        currentMotion = 0;
    }

    void pushState(State s) {
        state = s;
        depth++;
        // fprintf(stderr, "PUSH: depth = %d, state = %s\n", depth, toString(state));
    }
    void popState(State s) {
        state = s;
        depth--;
        // fprintf(stderr, "POP:  depth = %d, state = %s\n", depth, toString(state));
    }
    const char *copyKey() {
        return copyKey(key);
    }
    const char *copyKey(const char *k) {
        if (k && k[0] != 0) {
            size_t len = strlen(k);
            char *newKey = new char[len + 1];
            strncpy(newKey, k, len);
            keys.add(newKey);
            return newKey;
        }
        else {
            return 0;
        }
    }

    static const char *toString(State s) {
        switch (s) {
        case kInitial:
            return "kInitial";
        case kProject:
            return "kProject";
        case kSettings:
            return "kSettings";
        case kPhysics:
            return "kPhycis";
        case kModels:
            return "kModels";
        case kModel:
            return "kModel";
        case kAssets:
            return "kAssets";
        case kAsset:
            return "kAsset";
        case kMotions:
            return "kMotions";
        case kIKMotion:
            return "kIKMotion";
        case kAssetMotion:
            return "kAssetMotion";
        case kAnimation:
            return "kAnimation";
        case kBoneMotion:
            return "kBoneMotion";
        case kVerticesMotion:
            return "kVerticesMotion";
        case kCameraMotion:
            return "kCameraMotion";
        case kLightMotion:
            return "kLightMotion";
        default:
            return "kUnknown";
        }
    }

    static bool equals(const xmlChar *prefix, const xmlChar *localname, const char *dst) {
        return equals(prefix, "vpvm") && equals(localname, dst);
    }

    static bool equals(const xmlChar *name, const char *dst) {
        return xmlStrcmp(name, reinterpret_cast<const xmlChar *>(dst)) == 0;
    }
    static void newString(const xmlChar **attributes, int index, std::string &value) {
        value = std::string(
                    reinterpret_cast<const char *>(attributes[index + 3]),
                    reinterpret_cast<const char *>(attributes[index + 4]));
    }
    static void splitString(const std::string &value, Array<std::string> &tokens) {
        const std::string &delimiter = ",";
        std::string item(value);
        for(size_t pos = item.find(delimiter);
            pos != std::string::npos;
            pos = item.find(delimiter, pos))
            item.replace(pos, delimiter.size(), " ");
        tokens.clear();
        std::stringstream stream(item);
        while (stream >> item)
            tokens.add(item);
    }
    static void setQuadWordValues(const Array<std::string> &tokens, QuadWord &value, int offset) {
        value.setX(internal::stringToFloat(tokens.at(offset + 0).c_str()));
        value.setY(internal::stringToFloat(tokens.at(offset + 1).c_str()));
        value.setZ(internal::stringToFloat(tokens.at(offset + 2).c_str()));
        value.setW(internal::stringToFloat(tokens.at(offset + 3).c_str()));
    }
    static bool createVector3(const Array<std::string> &tokens, Vector3 &value) {
        if (tokens.count() == 3) {
            value.setX(internal::stringToFloat(tokens.at(0).c_str()));
            value.setY(internal::stringToFloat(tokens.at(1).c_str()));
            value.setZ(internal::stringToFloat(tokens.at(2).c_str()));
            return true;
        }
        return false;
    }
    static bool createVector4(const Array<std::string> &tokens, Vector4 &value) {
        if (tokens.count() == 4) {
            value.setX(internal::stringToFloat(tokens.at(0).c_str()));
            value.setY(internal::stringToFloat(tokens.at(1).c_str()));
            value.setZ(internal::stringToFloat(tokens.at(2).c_str()));
            value.setW(internal::stringToFloat(tokens.at(3).c_str()));
            return true;
        }
        return false;
    }

    static void startElement(void *context,
                             const xmlChar *localname,
                             const xmlChar *prefix,
                             const xmlChar * /* URI */,
                             int /* nnamespaces */,
                             const xmlChar ** /* namespaces */,
                             int nattributes,
                             int /* ndefaulted */,
                             const xmlChar **attributes)
    {
        Parser *self = static_cast<Parser *>(context);
        char attributeName[kBufferSize];
        int index = 0;
        if (self->depth == 0 && equals(prefix, localname, "project")) {
            self->pushState(kProject);
        }
        else if (self->depth == 1 && self->state == kProject) {
            if (equals(prefix, localname, "settings")) {
                self->pushState(kSettings);
            }
            else if (equals(prefix, localname, "physics")) {
                self->pushState(kPhysics);
            }
            else if (equals(prefix, localname, "models")) {
                self->pushState(kModels);
            }
            else if (equals(prefix, localname, "assets")) {
                self->pushState(kAssets);
            }
            else if (equals(prefix, localname, "motions")) {
                self->pushState(kMotions);
            }
        }
        else if (self->depth == 2) {
            if (self->state == kSettings) {
                strncpy(self->key, reinterpret_cast<const char *>(localname), sizeof(self->key) - 1);
            }
            if (self->state == kModels && equals(prefix, localname, "model")) {
                self->currentModel = new PMDModel();
                StringHash *values = new StringHash();
                self->localModelSettingValues.add(values);
                self->localModelSettings.insert(self->currentModel, values);
                self->pushState(kModel);
            }
            else if (self->state == kAssets && equals(prefix, localname, "asset")) {
                self->currentAsset = new Asset();
                StringHash *values = new StringHash();
                self->localAssetSettingValues.add(values);
                self->localAssetSettings.insert(self->currentAsset, values);
                self->pushState(kAsset);
            }
            else if (self->state == kMotions && equals(prefix, localname, "motion")) {
                bool found = false;
                for (int i = 0; i < nattributes; i++, index += 5) {
                    if (!equals(attributes[index], "type"))
                        continue;
                    strncpy(attributeName, reinterpret_cast<const char *>(attributes[index + 3]), sizeof(attributeName));
                    attributeName[sizeof(attributeName) - 1] = 0;
                    if (strncmp(attributeName, "ik", 2) == 0) {
                        self->pushState(kIKMotion);
                        found = true;
                    }
                    else if (strncmp(attributeName, "asset", 5) == 0) {
                        self->pushState(kAssetMotion);
                        found = true;
                    }
                }
                if (!found) {
                    self->currentMotion = new VMDMotion();
                    self->pushState(kAnimation);
                }
            }
        }
        else if (self->depth == 3) {
            if (self->state == kModel || self->state == kAsset) {
                strncpy(self->key, reinterpret_cast<const char *>(localname), sizeof(self->key) - 1);
            }
            else if (self->state == kAnimation && equals(localname, "animation")) {
                for (int i = 0; i < nattributes; i++, index += 5) {
                    if (!equals(attributes[index], "type"))
                        continue;
                    strncpy(attributeName, reinterpret_cast<const char *>(attributes[index + 3]), sizeof(attributeName));
                    attributeName[sizeof(attributeName) - 1] = 0;
                    if (strncmp(attributeName, "bone", 4) == 0) {
                        self->pushState(kBoneMotion);
                    }
                    else if (strncmp(attributeName, "vertices", 8) == 0) {
                        self->pushState(kVerticesMotion);
                    }
                    else if (strncmp(attributeName, "camera", 6) == 0) {
                        self->pushState(kCameraMotion);
                    }
                    else if (strncmp(attributeName, "light", 5) == 0) {
                        self->pushState(kLightMotion);
                    }
                }
            }
            else if (equals(localname, "keyframe")) {
                switch (self->state) {
                case kIKMotion:
                    break;
                case kAssetMotion:
                    break;
                }
            }
        }
        else if (self->depth == 4 && equals(localname, "keyframe")) {
            Array<std::string> tokens;
            Vector3 vec3;
            Vector4 vec4;
            QuadWord qw;
            std::string value;
            switch (self->state) {
            case kBoneMotion:
            {
                BoneKeyFrame *keyframe = new BoneKeyFrame();
                keyframe->setDefaultInterpolationParameter();
                for (int i = 0; i < nattributes; i++, index += 5) {
                    strncpy(attributeName, reinterpret_cast<const char *>(attributes[index]), sizeof(attributeName));
                    attributeName[sizeof(attributeName) - 1] = 0;
                    if (strncmp(attributeName, "name", 4) == 0) {
                        newString(attributes, index, value);
                        keyframe->setName(reinterpret_cast<const uint8_t *>(value.c_str()));
                    }
                    else if (strncmp(attributeName, "index", 5) == 0) {
                        newString(attributes, index, value);
                        keyframe->setFrameIndex(internal::stringToFloat(value.c_str()));
                    }
                    else if (strncmp(attributeName, "position", 8) == 0) {
                        newString(attributes, index, value);
                        splitString(value, tokens);
                        if (createVector3(tokens, vec3)) {
#ifdef VPVL_COORDINATE_OPENGL
                            vec3.setValue(vec3.x(), vec3.y(), -vec3.z());
#else
                            vec3.setValue(vec3.x(), vec3.y(), vec3.z());
#endif
                            keyframe->setPosition(vec3);
                        }
                    }
                    else if (strncmp(attributeName, "rotation", 8) == 0) {
                        newString(attributes, index, value);
                        splitString(value, tokens);
                        if (createVector4(tokens, vec4)) {
                            Quaternion rotation;
#ifdef VPVL_COORDINATE_OPENGL
                            rotation.setValue(-vec4.x(), -vec4.y(), vec4.z(), vec4.w());
#else
                            rotation.setValue(vec4.x(), vec4.y(), vec4.z(), vec4.w());
#endif
                            keyframe->setRotation(rotation);
                        }
                    }
                    else if (strncmp(attributeName, "interpolation", 12) == 0) {
                        newString(attributes, index, value);
                        splitString(value, tokens);
                        if (tokens.count() == 16) {
                            for (int i = 0; i < 4; i++) {
                                setQuadWordValues(tokens, qw, i * 4);
                                keyframe->setInterpolationParameter(static_cast<BoneKeyFrame::InterpolationType>(i), qw);
                            }
                        }
                    }
                }
                self->currentMotion->mutableBoneAnimation()->addKeyFrame(keyframe);
                break;
            }
            case kVerticesMotion:
            {
                FaceKeyFrame *keyframe = new FaceKeyFrame();
                for (int i = 0; i < nattributes; i++, index += 5) {
                    strncpy(attributeName, reinterpret_cast<const char *>(attributes[index]), sizeof(attributeName));
                    attributeName[sizeof(attributeName) - 1] = 0;
                    if (strncmp(attributeName, "name", 4) == 0) {
                        newString(attributes, index, value);
                        keyframe->setName(reinterpret_cast<const uint8_t *>(value.c_str()));
                    }
                    else if (strncmp(attributeName, "index", 5) == 0) {
                        newString(attributes, index, value);
                        keyframe->setFrameIndex(internal::stringToFloat(value.c_str()));
                    }
                    else if (strncmp(attributeName, "weight", 6) == 0) {
                        newString(attributes, index, value);
                        keyframe->setWeight(internal::stringToFloat(value.c_str()));
                    }
                }
                self->currentMotion->mutableFaceAnimation()->addKeyFrame(keyframe);
                break;
            }
            case kCameraMotion:
            {
                CameraKeyFrame *keyframe = new CameraKeyFrame();
                keyframe->setDefaultInterpolationParameter();
                for (int i = 0; i < nattributes; i++, index += 5) {
                    strncpy(attributeName, reinterpret_cast<const char *>(attributes[index]), sizeof(attributeName));
                    attributeName[sizeof(attributeName) - 1] = 0;
                    if (strncmp(attributeName, "fovy", 4) == 0) {
                        newString(attributes, index, value);
                        keyframe->setFovy(internal::stringToFloat(value.c_str()));
                    }
                    else if (strncmp(attributeName, "index", 5) == 0) {
                        newString(attributes, index, value);
                        keyframe->setFrameIndex(internal::stringToFloat(value.c_str()));
                    }
                    else if (strncmp(attributeName, "angle", 5) == 0) {
                        newString(attributes, index, value);
                        splitString(value, tokens);
                        if (createVector3(tokens, vec3)) {
#ifdef VPVL_COORDINATE_OPENGL
                            vec3.setValue(-degree(vec3.x()), -degree(vec3.y()), -degree(vec3.z()));
#else
                            vec3.setValue(degree(vec3.x()), degree(vec3.y()), -degree(vec3.z()));
#endif
                            reinterpret_cast<CameraKeyFrame *>(keyframe)->setAngle(vec3);
                        }
                    }
                    else if (strncmp(attributeName, "position", 8) == 0) {
                        newString(attributes, index, value);
                        splitString(value, tokens);
                        if (createVector3(tokens, vec3)) {
#ifdef VPVL_COORDINATE_OPENGL
                            vec3.setValue(vec3.x(), vec3.y(), -vec3.z());
#else
                            vec3.setValue(vec3.x(), vec3.y(), vec3.z());
#endif
                            keyframe->setPosition(vec3);
                        }
                    }
                    else if (strncmp(attributeName, "distance", 8) == 0) {
                        newString(attributes, index, value);
                        keyframe->setDistance(internal::stringToFloat(value.c_str()));
                    }
                    else if (strncmp(attributeName, "interpolation", 12) == 0) {
                        newString(attributes, index, value);
                        splitString(value, tokens);
                        if (tokens.count() == 24) {
                            for (int i = 0; i < 6; i++) {
                                setQuadWordValues(tokens, qw, i * 4);
                                keyframe->setInterpolationParameter(static_cast<CameraKeyFrame::InterpolationType>(i), qw);
                            }
                        }
                    }
                }
                self->currentMotion->mutableCameraAnimation()->addKeyFrame(keyframe);
                break;
            }
            case kLightMotion:
            {
                LightKeyFrame *keyframe = new LightKeyFrame();
                for (int i = 0; i < nattributes; i++, index += 5) {
                    strncpy(attributeName, reinterpret_cast<const char *>(attributes[index]), sizeof(attributeName));
                    attributeName[sizeof(attributeName) - 1] = 0;
                    if (strncmp(attributeName, "index", 5) == 0) {
                        newString(attributes, index, value);
                        keyframe->setFrameIndex(internal::stringToFloat(value.c_str()));
                    }
                    else if (strncmp(attributeName, "color", 5) == 0) {
                        newString(attributes, index, value);
                        splitString(value, tokens);
                        if (createVector3(tokens, vec3))
                            keyframe->setColor(vec3);
                    }
                    else if (strncmp(attributeName, "direction", 9) == 0) {
                        newString(attributes, index, value);
                        splitString(value, tokens);
                        if (createVector3(tokens, vec3))
                            keyframe->setDirection(vec3);
                    }
                }
                self->currentMotion->mutableLightAnimation()->addKeyFrame(keyframe);
                break;
            }
            }
        }
    }
    static void characters(void *context,
                           const xmlChar *character,
                           int len)
    {
        Parser *self = static_cast<Parser *>(context);
        const char *key = 0;
        if (self->state == kSettings) {
            std::string value(reinterpret_cast<const char *>(character), len);
            if (key = self->copyKey())
                self->globalSettings.insert(key, value);
        }
        else if (self->state == kModel) {
            std::string value(reinterpret_cast<const char *>(character), len);
            StringHash **values = self->localModelSettings[self->currentModel];
            if (values && (key = self->copyKey()))
                (*values)->insert(key, value);
        }
        else if (self->state == kAsset) {
            std::string value(reinterpret_cast<const char *>(character), len);
            StringHash **values = self->localAssetSettings[self->currentAsset];
            if (values && (key = self->copyKey()))
                (*values)->insert(key, value);
        }
    }
    static void endElement(void *context,
                           const xmlChar *localname,
                           const xmlChar *prefix,
                           const xmlChar * /* URI */)
    {
        Parser *self = static_cast<Parser *>(context);
        if (self->depth == 4 && !equals(localname, "keyframe")) {
            self->popState(kAnimation);
        }
        if (self->depth == 3) {
            switch (self->state) {
            case kAsset:
                if (equals(prefix, localname, "asset")) {
                    self->assets.add(self->currentAsset);
                    self->currentAsset = 0;
                    self->popState(kAssets);
                }
                internal::zerofill(self->key, sizeof(self->key));
                break;
            case kModel:
                if (equals(prefix, localname, "model")) {
                    self->models.add(self->currentModel);
                    self->currentModel = 0;
                    self->popState(kModels);
                }
                internal::zerofill(self->key, sizeof(self->key));
                break;
            case kAnimation:
                if (equals(prefix, localname, "motion")) {
                    self->motions.add(self->currentMotion);
                    self->currentMotion = 0;
                    self->popState(kMotions);
                }
                break;
            case kAssetMotion:
            case kIKMotion:
                if (equals(prefix, localname, "motion")) {
                    self->popState(kMotions);
                }
                break;
            default:
                break;
            }
        }
        else if (self->depth == 2) {
            switch (self->state) {
            case kAssets:
                if (equals(prefix, localname, "assets"))
                    self->popState(kProject);
                break;
            case kModels:
                if (equals(prefix, localname, "models"))
                    self->popState(kProject);
                break;
            case kMotions:
                if (equals(prefix, localname, "motions"))
                    self->popState(kProject);
                break;
            case kSettings:
                if (equals(prefix, localname, "settings"))
                    self->popState(kProject);
                internal::zerofill(self->key, sizeof(self->key));
                break;
            case kPhysics:
                if (equals(prefix, localname, "physics"))
                    self->popState(kProject);
                break;
            default:
                break;
            }
        }
        else if (self->depth == 1 && self->state == kProject && equals(prefix, localname, "project")) {
            self->depth--;
        }
    }
    static void error(void *context, const char *format, ...)
    {
        Parser *self = static_cast<Parser *>(context);
        (void) self;
    }
    static void warning(void *context, const char *format, ...)
    {
        Parser *self = static_cast<Parser *>(context);
        (void) self;
    }

    char key[kBufferSize];
    Array<Asset *> assets;
    Array<PMDModel *> models;
    Array<VMDMotion *> motions;
    StringHash globalSettings;
    Array<char *> keys;
    Array<StringHash *> localModelSettingValues;
    Array<StringHash *> localAssetSettingValues;
    PtrHash localModelSettings;
    PtrHash localAssetSettings;
    Asset *currentAsset;
    PMDModel *currentModel;
    VMDMotion *currentMotion;
    State state;
    int depth;
};

Project::Project()
    : m_parser(0)
{
    internal::zerofill(&m_handler, sizeof(m_handler));
    m_parser = new Parser();
    m_handler.initialized = XML_SAX2_MAGIC;
    m_handler.startElementNs = &Parser::startElement;
    m_handler.endElementNs = &Parser::endElement;
    m_handler.characters = &Parser::characters;
    m_handler.warning = &Parser::warning;
    m_handler.error = &Parser::error;
}

Project::~Project()
{
    internal::zerofill(&m_handler, sizeof(m_handler));
    delete m_parser;
    m_parser = 0;
}

bool Project::load(const char *path)
{
    return xmlSAXUserParseFile(&m_handler, m_parser, path) == 0 && m_parser->depth == 0;
}

bool Project::load(const uint8_t *data, size_t size)
{
    return xmlSAXUserParseMemory(&m_handler, m_parser, reinterpret_cast<const char *>(data), size) == 0;
}

void Project::save(const char * /* path */)
{
}

const Array<Asset *> &Project::assets() const
{
    return m_parser->assets;
}

const Array<PMDModel *> &Project::models() const
{
    return m_parser->models;
}

const Array<VMDMotion *> &Project::motions() const
{
    return m_parser->motions;
}

const std::string Project::globalSetting(const char *key) const
{
    std::string *value = const_cast<std::string *>(m_parser->globalSettings.find(key));
    return value ? *value : std::string();
}

const std::string Project::localAssetSetting(Asset *asset, const char *key) const
{
    Parser::StringHash **values = const_cast<Parser::StringHash **>(m_parser->localAssetSettings.find(asset));
    if (values) {
        std::string *value = const_cast<std::string *>((*values)->find(key));
        if (value)
            return *value;
    }
    return std::string();
}

const std::string Project::localModelSetting(PMDModel *model, const char *key) const
{
    Parser::StringHash **values = const_cast<Parser::StringHash **>(m_parser->localModelSettings.find(model));
    if (values) {
        std::string *value = const_cast<std::string *>((*values)->find(key));
        if (value)
            return *value;
    }
    return std::string();
}

Array<Asset *> *Project::mutableAssets()
{
    return &m_parser->assets;
}

Array<PMDModel *> *Project::mutableModels()
{
    return &m_parser->models;
}

Array<VMDMotion *> *Project::mutableMotions()
{
    return &m_parser->motions;
}

void Project::setGlobalSetting(const char *key, std::string &value)
{
    m_parser->globalSettings.insert(key, value);
}

void Project::setLocalAssetSetting(Asset *asset, const char *key, const std::string &value) const
{
    Parser::StringHash **values = const_cast<Parser::StringHash **>(m_parser->localAssetSettings.find(asset));
    if (!values) {
        Parser::StringHash *hash = new Parser::StringHash();
        const char *k = m_parser->copyKey(key);
        hash->insert(k, value);
        m_parser->localAssetSettingValues.add(hash);
        m_parser->localAssetSettings.insert(asset, hash);
    }
    else {
        (*values)->insert(key, value);
    }
}

void Project::setLocalModelSetting(PMDModel *model, const char *key, const std::string &value) const
{
    Parser::StringHash **values = const_cast<Parser::StringHash **>(m_parser->localModelSettings.find(model));
    if (!values) {
        Parser::StringHash *hash = new Parser::StringHash();
        const char *k = m_parser->copyKey(key);
        hash->insert(k, value);
        m_parser->localModelSettingValues.add(hash);
        m_parser->localModelSettings.insert(model, hash);
    }
    else {
        (*values)->insert(key, value);
    }
}

} /* namespace vpvl */
