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

#include "vpvl2/vpvl2.h"
#include "vpvl2/internal/util.h"

#include "vpvl2/extensions/XMLProject.h"
#include "vpvl2/mvd/AssetKeyframe.h"
#include "vpvl2/mvd/AssetSection.h"
#include "vpvl2/mvd/BoneKeyframe.h"
#include "vpvl2/mvd/BoneSection.h"
#include "vpvl2/mvd/CameraKeyframe.h"
#include "vpvl2/mvd/CameraSection.h"
#include "vpvl2/mvd/EffectKeyframe.h"
#include "vpvl2/mvd/EffectSection.h"
#include "vpvl2/mvd/LightKeyframe.h"
#include "vpvl2/mvd/LightSection.h"
#include "vpvl2/mvd/ModelKeyframe.h"
#include "vpvl2/mvd/ModelSection.h"
#include "vpvl2/mvd/MorphKeyframe.h"
#include "vpvl2/mvd/MorphSection.h"
#include "vpvl2/mvd/ProjectKeyframe.h"
#include "vpvl2/mvd/ProjectSection.h"
#include "vpvl2/mvd/Motion.h"
#include "vpvl2/vmd/BoneAnimation.h"
#include "vpvl2/vmd/BoneKeyframe.h"
#include "vpvl2/vmd/CameraAnimation.h"
#include "vpvl2/vmd/CameraKeyframe.h"
#include "vpvl2/vmd/LightAnimation.h"
#include "vpvl2/vmd/LightKeyframe.h"
#include "vpvl2/vmd/MorphAnimation.h"
#include "vpvl2/vmd/MorphKeyframe.h"
#include "vpvl2/vmd/Motion.h"

#include <libxml/SAX2.h>
#include <libxml/xmlwriter.h>
#include <set>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>

#define VPVL2_CAST_XC(str) reinterpret_cast<const xmlChar *>(str)
#define VPVL2_XML_RC(rc) { if (rc < 0) { VPVL2_LOG(WARNING, "rc = " << rc << " at " << __FILE__ << ":" << __LINE__); return false; } }

namespace
{

__attribute__((format(printf, 3, 4)))
static inline int StringPrintf(uint8_t *buffer, size_t size, const char *format, ...)
{
    assert(buffer && size > 0);
    va_list ap;
    va_start(ap, format);
    int ret = ::vsnprintf(reinterpret_cast<char *>(buffer), size, format, ap);
    va_end(ap);
    return ret;
}

static inline int StringToInt(const std::string &value)
{
    char *p = 0;
    return int(::strtoul(value.c_str(), &p, 10));
}

static inline double StringToDouble(const std::string &value)
{
    char *p = 0;
    return ::strtod(value.c_str(), &p);
}

static inline float StringToFloat(const std::string &value)
{
    return float(StringToDouble(value));
}

static inline bool StringToBool(const std::string &value)
{
    return value == "true";
}

}

namespace vpvl2
{
namespace extensions
{

struct XMLProject::PrivateContext {
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
        kAnimation,
        kVMDBoneMotion,
        kVMDMorphMotion,
        kVMDCameraMotion,
        kVMDLightMotion,
        kMVDAssetMotion,
        kMVDBoneMotion,
        kMVDCameraMotion,
        kMVDEffectMotion,
        kMVDLightMotion,
        kMVDModelMotion,
        kMVDMorphMotion,
        kMVDProjectMotion
    };
    static const int kElementContentBufferSize = 128;
    static const std::string kEmpty;
    typedef std::map<XMLProject::UUID, IModel *> ModelMap;
    typedef std::map<XMLProject::UUID, IMotion *> MotionMap;

    static inline const xmlChar *projectPrefix() {
        return reinterpret_cast<const xmlChar *>("vpvm");
    }
    static inline const xmlChar *projectNamespaceURI() {
        return reinterpret_cast<const xmlChar *>("https://github.com/hkrn/MMDAI/");
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
        case kAnimation:
            return "kAnimation";
        case kVMDBoneMotion:
            return "kVMDBoneMotion";
        case kVMDMorphMotion:
            return "kVMDMorphMotion";
        case kVMDCameraMotion:
            return "kVMDCameraMotion";
        case kVMDLightMotion:
            return "kVMDLightMotion";
        case kMVDAssetMotion:
            return "kMVDAssetMotion";
        case kMVDBoneMotion:
            return "kMVDBoneMotion";
        case kMVDCameraMotion:
            return "kMVDCameraMotion";
        case kMVDEffectMotion:
            return "kMVDEffectMotion";
        case kMVDLightMotion:
            return "kMVDLightMotion";
        case kMVDModelMotion:
            return "kMVDModelMotion";
        case kMVDMorphMotion:
            return "kMVDMorphMotion";
        case kMVDProjectMotion:
            return "kMVDProjectMotion";
        default:
            return "kUnknown";
        }
    }
    static inline bool equals(const xmlChar *prefix, const xmlChar *localname, const char *dst) {
        return xmlStrcmp(prefix, projectPrefix()) == 0 && equals(localname, dst);
    }
    static inline bool equals(const xmlChar *name, const char *dst) {
        return xmlStrcmp(name, reinterpret_cast<const xmlChar *>(dst)) == 0;
    }
    static void splitString(const std::string &value, Array<std::string> &tokens) {
        const std::string &delimiter = ",";
        std::string item(value);
        for (size_t pos = item.find(delimiter);
             pos != std::string::npos;
             pos = item.find(delimiter, pos)) {
            item.replace(pos, delimiter.size(), " ");
        }
        tokens.clear();
        std::stringstream stream(item);
        while (stream >> item) {
            tokens.append(item);
        }
    }
    static void setQuadWordValues(const Array<std::string> &tokens, QuadWord &value, int offset) {
        value.setX(StringToFloat(tokens.at(offset + 0).c_str()));
        value.setY(StringToFloat(tokens.at(offset + 1).c_str()));
        value.setZ(StringToFloat(tokens.at(offset + 2).c_str()));
        value.setW(StringToFloat(tokens.at(offset + 3).c_str()));
    }
    static bool tryCreateVector3(const Array<std::string> &tokens, Vector3 &value) {
        if (tokens.count() == 3) {
            value.setX(StringToFloat(tokens.at(0).c_str()));
            value.setY(StringToFloat(tokens.at(1).c_str()));
            value.setZ(StringToFloat(tokens.at(2).c_str()));
            return true;
        }
        return false;
    }
    static bool tryCreateVector4(const Array<std::string> &tokens, Vector4 &value) {
        if (tokens.count() == 4) {
            value.setX(StringToFloat(tokens.at(0).c_str()));
            value.setY(StringToFloat(tokens.at(1).c_str()));
            value.setZ(StringToFloat(tokens.at(2).c_str()));
            value.setW(StringToFloat(tokens.at(3).c_str()));
            return true;
        }
        return false;
    }

    PrivateContext(Scene *scene, XMLProject::IDelegate *delegate, Factory *factory)
        : delegateRef(delegate),
          sceneRef(scene),
          factoryRef(factory),
          currentString(0),
          currentMotion(0),
          currentMotionType(IMotion::kVMDMotion),
          state(kInitial),
          depth(0),
          dirty(false)
    {
        internal::zerofill(&saxHandler, sizeof(saxHandler));
        saxHandler.initialized = XML_SAX2_MAGIC;
        saxHandler.startElementNs = &PrivateContext::startElement;
        saxHandler.endElementNs = &PrivateContext::endElement;
        saxHandler.cdataBlock = &PrivateContext::cdataBlock;
        saxHandler.warning = &PrivateContext::warning;
        saxHandler.error = &PrivateContext::error;
    }
    ~PrivateContext() {
        internal::zerofill(&saxHandler, sizeof(saxHandler));
        /* delete models/assets/motions instances at Scene class */
        assetRefs.clear();
        modelRefs.clear();
        motionRefs.clear();
        delete currentString;
        currentString = 0;
        delete currentMotion;
        currentMotion = 0;
        state = kInitial;
        depth = 0;
        dirty = false;
        sceneRef = 0;
        delegateRef = 0;
        factoryRef = 0;
    }

    bool isDuplicatedUUID(const XMLProject::UUID &uuid, std::set<XMLProject::UUID> &set) const {
        if (set.find(uuid) != set.end()) {
            return true;
        }
        set.insert(uuid);
        return false;
    }
    bool checkDuplicateUUID() const {
        std::set<XMLProject::UUID> set;
        for (ModelMap::const_iterator it = assetRefs.begin(); it != assetRefs.end(); it++) {
            if (isDuplicatedUUID(it->first, set)) {
                return false;
            }
        }
        for (ModelMap::const_iterator it = modelRefs.begin(); it != modelRefs.end(); it++) {
            if (isDuplicatedUUID(it->first, set)) {
                return false;
            }
        }
        for (MotionMap::const_iterator it = motionRefs.begin(); it != motionRefs.end(); it++) {
            if (isDuplicatedUUID(it->first, set)) {
                return false;
            }
        }
        return true;
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
    IModel *findModel(const XMLProject::UUID &value) const {
        if (value == XMLProject::kNullUUID) {
            return 0;
        }
        ModelMap::const_iterator it = assetRefs.find(value);
        if (it != assetRefs.end()) {
            return it->second;
        }
        it = modelRefs.find(value);
        if (it != modelRefs.end()) {
            return it->second;
        }
        return 0;
    }
    const XMLProject::UUID &findModelUUID(const IModel *value) const {
        if (!value) {
            return XMLProject::kNullUUID;
        }
        for (ModelMap::const_iterator it = assetRefs.begin(); it != assetRefs.end(); it++) {
            if (it->second == value) {
                return it->first;
            }
        }
        for (ModelMap::const_iterator it = modelRefs.begin(); it != modelRefs.end(); it++) {
            if (it->second == value) {
                return it->first;
            }
        }
        return XMLProject::kNullUUID;
    }
    IMotion *findMotion(const XMLProject::UUID &value) const {
        if (value == XMLProject::kNullUUID) {
            return 0;
        }
        MotionMap::const_iterator it = motionRefs.find(value);
        if (it != motionRefs.end()) {
            return it->second;
        }
        return 0;
    }
    const XMLProject::UUID &findMotionUUID(const IMotion *value) const {
        if (!value) {
            return XMLProject::kNullUUID;
        }
        for (MotionMap::const_iterator it = motionRefs.begin(); it != motionRefs.end(); it++) {
            if (it->second == value) {
                return it->first;
            }
        }
        return XMLProject::kNullUUID;
    }
    bool removeModel(IModel *model) {
        for (ModelMap::iterator it = assetRefs.begin(); it != assetRefs.end(); it++) {
            if (it->second == model) {
                assetRefs.erase(it);
                sceneRef->removeModel(model);
                return true;
            }
        }
        for (ModelMap::iterator it = modelRefs.begin(); it != modelRefs.end(); it++) {
            if (it->second == model) {
                modelRefs.erase(it);
                sceneRef->removeModel(model);
                return true;
            }
        }
        return false;
    }
    bool removeMotion(const IMotion *motion) {
        for (MotionMap::iterator it = motionRefs.begin(); it != motionRefs.end(); it++) {
            if (it->second == motion) {
                motionRefs.erase(it);
                return true;
            }
        }
        return false;
    }

    bool writeXml(xmlTextWriterPtr writer) const {
        uint8_t buffer[kElementContentBufferSize];
        if (!writer) {
            return false;
        }
        VPVL2_XML_RC(xmlTextWriterSetIndent(writer, 1));
        VPVL2_XML_RC(xmlTextWriterStartDocument(writer, 0, "UTF-8", 0));
        VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("project"), projectNamespaceURI()));
        StringPrintf(buffer, sizeof(buffer), "%.1f", XMLProject::formatVersion());
        VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("version"), VPVL2_CAST_XC(buffer)));
        if (!writeSettings(writer)) {
            return false;
        }
        if (!writeModels(writer)) {
            return false;
        }
        if (!writeAssets(writer)) {
            return false;
        }
        VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("motions"), 0));
        for (MotionMap::const_iterator it = motionRefs.begin(); it != motionRefs.end(); it++) {
            const std::string &motionUUID = it->first;
            if (IMotion *motionPtr = it->second) {
                IMotion::Type motionType = motionPtr->type();
                if (motionType == IMotion::kVMDMotion) {
                    const vmd::Motion *motion = static_cast<const vmd::Motion *>(motionPtr);
                    VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("motion"), 0));
                    VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("type"), VPVL2_CAST_XC("vmd")));
                    const std::string &modelUUID = findModelUUID(motion->parentModelRef());
                    if (modelUUID != XMLProject::kNullUUID) {
                        VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("model"), VPVL2_CAST_XC(modelUUID.c_str())));
                    }
                    VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("uuid"), VPVL2_CAST_XC(motionUUID.c_str())));
                    if (!writeVMDBoneKeyframes(writer, motion)) {
                        return false;
                    }
                    if (!writeVMDMorphKeyframes(writer, motion)) {
                        return false;
                    }
                    if (!writeVMDCameraKeyframes(writer, motion)) {
                        return false;
                    }
                    if (!writeVMDLightKeyframes(writer, motion)) {
                        return false;
                    }
                    VPVL2_XML_RC(xmlTextWriterEndElement(writer)); /* vpvl:motion */
                }
                else if (motionType == IMotion::kMVDMotion) {
                    const mvd::Motion *motion = static_cast<const mvd::Motion *>(motionPtr);
                    VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("motion"), 0));
                    VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("type"), VPVL2_CAST_XC("mvd")));
                    const std::string &modelUUID = findModelUUID(motion->parentModelRef());
                    if (modelUUID != XMLProject::kNullUUID) {
                        VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("model"), VPVL2_CAST_XC(modelUUID.c_str())));
                    }
                    VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("uuid"), VPVL2_CAST_XC(motionUUID.c_str())));
                    if (!writeMVDBoneKeyframes(writer, motion)) {
                        return false;
                    }
                    if (!writeMVDMorphKeyframes(writer, motion)) {
                        return false;
                    }
                    if (!writeMVDCameraKeyframes(writer, motion)) {
                        return false;
                    }
                    if (!writeMVDLightKeyframes(writer, motion)) {
                        return false;
                    }
                    if (!writeMVDEffectKeyframes(writer, motion)) {
                        return false;
                    }
                    if (!writeMVDProjectKeyframes(writer, motion)) {
                        return false;
                    }
                    if (!writeMVDModelKeyframes(writer, motion)) {
                        return false;
                    }
                    VPVL2_XML_RC(xmlTextWriterEndElement(writer)); /* vpvl:motion */
                }
            }
        }
        VPVL2_XML_RC(xmlTextWriterEndElement(writer)); /* vpvl:motions */
        VPVL2_XML_RC(xmlTextWriterEndElement(writer)); /* vpvl:project */
        VPVL2_XML_RC(xmlTextWriterEndDocument(writer));
        return true;
    }
    bool writeSettings(xmlTextWriterPtr writer) const {
        VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("settings"), 0));
        if(!writeStringMap(projectPrefix(), globalSettings, writer)) {
            return false;
        }
        VPVL2_XML_RC(xmlTextWriterEndElement(writer)); /* vpvl:setting */
        return true;
    }
    void getNewModelSettings(const IModel *model, const StringMap &modelSettings, StringMap &newModelSettings) const {
        const IBone *parentBoneRef = model->parentBoneRef();
        newModelSettings = modelSettings;
        newModelSettings["state.opacity"] = XMLProject::toStringFromFloat32(model->opacity());
        newModelSettings["state.scale"] = XMLProject::toStringFromFloat32(model->scaleFactor());
        newModelSettings["state.offset.position"] = XMLProject::toStringFromVector3(model->worldPosition());
        newModelSettings["state.offset.rotation"] = XMLProject::toStringFromQuaternion(model->worldRotation());
        newModelSettings["state.edge.color"] = XMLProject::toStringFromVector3(model->edgeColor());
        newModelSettings["state.edge.offset"] = XMLProject::toStringFromFloat32(float32_t(model->edgeWidth()));
        newModelSettings["state.parent.model"] = findModelUUID(model->parentModelRef());
        newModelSettings["state.parent.bone"] = parentBoneRef ? delegateRef->toStdFromString(parentBoneRef->name()) : "";
    }
    bool writeModels(xmlTextWriterPtr writer) const {
        StringMap newModelSettings;
        VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("models"), 0));
        for (ModelMap::const_iterator it = modelRefs.begin(); it != modelRefs.end(); it++) {
            const XMLProject::UUID &uuid = it->first;
            const IModel *model = it->second;
            VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("model"), 0));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("uuid"), VPVL2_CAST_XC(uuid.c_str())));
            ModelSettings::const_iterator it2 = localModelSettings.find(uuid);
            if (it2 != localModelSettings.end()) {
                getNewModelSettings(model, it2->second, newModelSettings);
                if(!writeStringMap(projectPrefix(), newModelSettings, writer)) {
                    return false;
                }
            }
            VPVL2_XML_RC(xmlTextWriterEndElement(writer)); /* vpvl:model */
        }
        VPVL2_XML_RC(xmlTextWriterEndElement(writer)); /* vpvl:models */
        return true;
    }
    bool writeAssets(xmlTextWriterPtr writer) const {
        StringMap newAssetSettings;
        VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("assets"), 0));
        for (ModelMap::const_iterator it = assetRefs.begin(); it != assetRefs.end(); it++) {
            const XMLProject::UUID &uuid = it->first;
            const IModel *asset = it->second;
            VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("asset"), 0));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("uuid"), VPVL2_CAST_XC(uuid.c_str())));
            ModelSettings::const_iterator it2 = localAssetSettings.find(uuid);
            if (it2 != localAssetSettings.end()) {
                getNewModelSettings(asset, it2->second, newAssetSettings);
                if(!writeStringMap(projectPrefix(), newAssetSettings, writer)) {
                    return false;
                }
            }
            VPVL2_XML_RC(xmlTextWriterEndElement(writer)); /* vpvl:asset */
        }
        VPVL2_XML_RC(xmlTextWriterEndElement(writer)); /* vpvl:asset */
        return true;
    }
    bool writeVMDBoneKeyframes(xmlTextWriterPtr writer, const vmd::Motion *motion) const {
        Quaternion ix, iy, iz, ir;
        uint8_t buffer[kElementContentBufferSize];
        VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("animation"), 0));
        VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("type"), VPVL2_CAST_XC("bone")));
        const vmd::BoneAnimation &ba = motion->boneAnimation();
        int nkeyframes = ba.countKeyframes();
        for (int i = 0; i < nkeyframes; i++) {
            const vmd::BoneKeyframe *keyframe = static_cast<const vmd::BoneKeyframe *>(ba.findKeyframeAt(i));
            const std::string &name = delegateRef->toStdFromString(keyframe->name());
            VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("keyframe"), 0));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("name"), VPVL2_CAST_XC(name.c_str())));
            StringPrintf(buffer, sizeof(buffer), "%d", static_cast<int>(keyframe->timeIndex()));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("index"), VPVL2_CAST_XC(buffer)));
            const Vector3 &position = keyframe->localTranslation();
            StringPrintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f", position.x(), position.y(), -position.z());
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("position"), VPVL2_CAST_XC(buffer)));
            const Quaternion &rotation = keyframe->localRotation();
            StringPrintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f,%.8f",
                         -rotation.x(), -rotation.y(), rotation.z(), rotation.w());
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("rotation"), VPVL2_CAST_XC(buffer)));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("ik"), VPVL2_CAST_XC(keyframe->isIKEnabled() ? "true" : "false")));
            keyframe->getInterpolationParameter(IBoneKeyframe::kBonePositionX, ix);
            keyframe->getInterpolationParameter(IBoneKeyframe::kBonePositionY, iy);
            keyframe->getInterpolationParameter(IBoneKeyframe::kBonePositionZ, iz);
            keyframe->getInterpolationParameter(IBoneKeyframe::kBoneRotation, ir);
            StringPrintf(buffer, sizeof(buffer),
                         "%.f,%.f,%.f,%.f,"
                         "%.f,%.f,%.f,%.f,"
                         "%.f,%.f,%.f,%.f,"
                         "%.f,%.f,%.f,%.f"
                         , ix.x(), ix.y(), ix.z(), ix.w()
                         , iy.x(), iy.y(), iy.z(), iy.w()
                         , iz.x(), iz.y(), iz.z(), iz.w()
                         , ir.x(), ir.y(), ir.z(), ir.w()
                         );
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("interpolation"), VPVL2_CAST_XC(buffer)));
            VPVL2_XML_RC(xmlTextWriterEndElement(writer));
        }
        VPVL2_XML_RC(xmlTextWriterEndElement(writer)); /* animation */
        return true;
    }
    bool writeVMDCameraKeyframes(xmlTextWriterPtr writer, const vmd::Motion *motion) const {
        Quaternion ix, iy, iz, ir, ifv, idt;
        uint8_t buffer[kElementContentBufferSize];
        VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("animation"), 0));
        VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("type"), VPVL2_CAST_XC("camera")));
        const vmd::CameraAnimation &ca = motion->cameraAnimation();
        int nkeyframes = ca.countKeyframes();
        for (int i = 0; i < nkeyframes; i++) {
            const vmd::CameraKeyframe *keyframe = static_cast<const vmd::CameraKeyframe *>(ca.findKeyframeAt(i));
            VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("keyframe"), 0));
            StringPrintf(buffer, sizeof(buffer), "%d", static_cast<int>(keyframe->timeIndex()));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("index"), VPVL2_CAST_XC(buffer)));
            const Vector3 &position = keyframe->lookAt();
            StringPrintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f", position.x(), position.y(), -position.z());
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("position"), VPVL2_CAST_XC(buffer)));
            const Vector3 &angle = keyframe->angle();
            StringPrintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f",
                         btRadians(-angle.x()), btRadians(-angle.y()), btRadians(-angle.z()));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("angle"), VPVL2_CAST_XC(buffer)));
            StringPrintf(buffer, sizeof(buffer), "%.8f", keyframe->fov());
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("fovy"), VPVL2_CAST_XC(buffer)));
            StringPrintf(buffer, sizeof(buffer), "%.8f", keyframe->distance());
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("distance"), VPVL2_CAST_XC(buffer)));
            keyframe->getInterpolationParameter(ICameraKeyframe::kCameraLookAtX, ix);
            keyframe->getInterpolationParameter(ICameraKeyframe::kCameraLookAtY, iy);
            keyframe->getInterpolationParameter(ICameraKeyframe::kCameraLookAtZ, iz);
            keyframe->getInterpolationParameter(ICameraKeyframe::kCameraAngle, ir);
            keyframe->getInterpolationParameter(ICameraKeyframe::kCameraFov, ifv);
            keyframe->getInterpolationParameter(ICameraKeyframe::kCameraDistance, idt);
            StringPrintf(buffer, sizeof(buffer),
                         "%.f,%.f,%.f,%.f,"
                         "%.f,%.f,%.f,%.f,"
                         "%.f,%.f,%.f,%.f,"
                         "%.f,%.f,%.f,%.f,"
                         "%.f,%.f,%.f,%.f,"
                         "%.f,%.f,%.f,%.f"
                         , ix.x(), ix.y(), ix.z(), ix.w()
                         , iy.x(), iy.y(), iy.z(), iy.w()
                         , iz.x(), iz.y(), iz.z(), iz.w()
                         , ir.x(), ir.y(), ir.z(), ir.w()
                         , idt.x(), idt.y(), idt.z(), idt.w()
                         , ifv.x(), ifv.y(), ifv.z(), ifv.w()
                         );
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("interpolation"), VPVL2_CAST_XC(buffer)));
            VPVL2_XML_RC(xmlTextWriterEndElement(writer));
        }
        VPVL2_XML_RC(xmlTextWriterEndElement(writer)); /* animation */
        return true;
    }
    bool writeVMDLightKeyframes(xmlTextWriterPtr writer, const vmd::Motion *motion) const {
        uint8_t buffer[kElementContentBufferSize];
        VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("animation"), 0));
        VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("type"), VPVL2_CAST_XC("light")));
        const vmd::LightAnimation &la = motion->lightAnimation();
        int nkeyframes = la.countKeyframes();
        for (int i = 0; i < nkeyframes; i++) {
            const vmd::LightKeyframe *keyframe = static_cast<vmd::LightKeyframe *>(la.findKeyframeAt(i));
            VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("keyframe"), 0));
            StringPrintf(buffer, sizeof(buffer), "%d", static_cast<int>(keyframe->timeIndex()));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("index"), VPVL2_CAST_XC(buffer)));
            const Vector3 &color = keyframe->color();
            StringPrintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f", color.x(), color.y(), color.z());
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("color"), VPVL2_CAST_XC(buffer)));
            const Vector3 &direction = keyframe->direction();
            StringPrintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f", direction.x(), direction.y(), direction.z());
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("direction"), VPVL2_CAST_XC(buffer)));
            VPVL2_XML_RC(xmlTextWriterEndElement(writer));
        }
        VPVL2_XML_RC(xmlTextWriterEndElement(writer)); /* animation */
        return true;
    }
    bool writeVMDMorphKeyframes(xmlTextWriterPtr writer, const vmd::Motion *motion) const {
        uint8_t buffer[kElementContentBufferSize];
        VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("animation"), 0));
        VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("type"), VPVL2_CAST_XC("morph")));
        const vmd::MorphAnimation &fa = motion->morphAnimation();
        int nkeyframes = fa.countKeyframes();
        for (int i = 0; i < nkeyframes; i++) {
            const vmd::MorphKeyframe *keyframe = static_cast<vmd::MorphKeyframe *>(fa.findKeyframeAt(i));
            const std::string &name = delegateRef->toStdFromString(keyframe->name());
            VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("keyframe"), 0));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("name"), VPVL2_CAST_XC(name.c_str())));
            StringPrintf(buffer, sizeof(buffer), "%d", static_cast<int>(keyframe->timeIndex()));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("index"), VPVL2_CAST_XC(buffer)));
            StringPrintf(buffer, sizeof(buffer), "%.4f", keyframe->weight());
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("weight"), VPVL2_CAST_XC(buffer)));
            VPVL2_XML_RC(xmlTextWriterEndElement(writer));
        }
        VPVL2_XML_RC(xmlTextWriterEndElement(writer)); /* animation */
        return true;
    }
    bool writeMVDBoneKeyframes(xmlTextWriterPtr writer, const mvd::Motion *motion) const {
        Quaternion ix, iy, iz, ir;
        uint8_t buffer[kElementContentBufferSize];
        VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("animation"), 0));
        VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("type"), VPVL2_CAST_XC("bone")));
        int nkeyframes = motion->countKeyframes(IKeyframe::kBoneKeyframe);
        for (int i = 0; i < nkeyframes; i++) {
            const mvd::BoneKeyframe *keyframe = static_cast<const mvd::BoneKeyframe *>(motion->findBoneKeyframeRefAt(i));
            const std::string &name = delegateRef->toStdFromString(keyframe->name());
            VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("keyframe"), 0));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("name"), VPVL2_CAST_XC(name.c_str())));
            StringPrintf(buffer, sizeof(buffer), "%ld", static_cast<long>(keyframe->timeIndex()));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("index"), VPVL2_CAST_XC(buffer)));
            StringPrintf(buffer, sizeof(buffer), "%d", static_cast<int>(keyframe->layerIndex()));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("layer"), VPVL2_CAST_XC(buffer)));
            const Vector3 &position = keyframe->localTranslation();
            StringPrintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f", position.x(), position.y(), -position.z());
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("position"), VPVL2_CAST_XC(buffer)));
            const Quaternion &rotation = keyframe->localRotation();
            StringPrintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f,%.8f",
                         -rotation.x(), -rotation.y(), rotation.z(), rotation.w());
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("rotation"), VPVL2_CAST_XC(buffer)));
            keyframe->getInterpolationParameter(IBoneKeyframe::kBonePositionX, ix);
            keyframe->getInterpolationParameter(IBoneKeyframe::kBonePositionY, iy);
            keyframe->getInterpolationParameter(IBoneKeyframe::kBonePositionZ, iz);
            keyframe->getInterpolationParameter(IBoneKeyframe::kBoneRotation, ir);
            StringPrintf(buffer, sizeof(buffer),
                         "%.f,%.f,%.f,%.f,"
                         "%.f,%.f,%.f,%.f,"
                         "%.f,%.f,%.f,%.f,"
                         "%.f,%.f,%.f,%.f"
                         , ix.x(), ix.y(), ix.z(), ix.w()
                         , iy.x(), iy.y(), iy.z(), iy.w()
                         , iz.x(), iz.y(), iz.z(), iz.w()
                         , ir.x(), ir.y(), ir.z(), ir.w()
                         );
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("interpolation"), VPVL2_CAST_XC(buffer)));
            VPVL2_XML_RC(xmlTextWriterEndElement(writer));
        }
        VPVL2_XML_RC(xmlTextWriterEndElement(writer)); /* animation */
        return true;
    }
    bool writeMVDCameraKeyframes(xmlTextWriterPtr writer, const mvd::Motion *motion) const {
        Quaternion ix, ir, ifv, idt;
        uint8_t buffer[kElementContentBufferSize];
        VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("animation"), 0));
        VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("type"), VPVL2_CAST_XC("camera")));
        int nkeyframes = motion->countKeyframes(IKeyframe::kCameraKeyframe);
        for (int i = 0; i < nkeyframes; i++) {
            const mvd::CameraKeyframe *keyframe = static_cast<const mvd::CameraKeyframe *>(motion->findCameraKeyframeRefAt(i));
            VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("keyframe"), 0));
            StringPrintf(buffer, sizeof(buffer), "%ld", static_cast<long>(keyframe->timeIndex()));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("index"), VPVL2_CAST_XC(buffer)));
            StringPrintf(buffer, sizeof(buffer), "%d", static_cast<int>(keyframe->layerIndex()));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("layer"), VPVL2_CAST_XC(buffer)));
            const Vector3 &position = keyframe->lookAt();
            StringPrintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f", position.x(), position.y(), -position.z());
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("position"), VPVL2_CAST_XC(buffer)));
            const Vector3 &angle = keyframe->angle();
            StringPrintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f",
                         btRadians(-angle.x()), btRadians(-angle.y()), btRadians(-angle.z()));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("angle"), VPVL2_CAST_XC(buffer)));
            StringPrintf(buffer, sizeof(buffer), "%.8f", keyframe->fov());
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("fovy"), VPVL2_CAST_XC(buffer)));
            StringPrintf(buffer, sizeof(buffer), "%.8f", keyframe->distance());
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("distance"), VPVL2_CAST_XC(buffer)));
            keyframe->getInterpolationParameter(ICameraKeyframe::kCameraLookAtX, ix);
            keyframe->getInterpolationParameter(ICameraKeyframe::kCameraAngle, ir);
            keyframe->getInterpolationParameter(ICameraKeyframe::kCameraFov, ifv);
            keyframe->getInterpolationParameter(ICameraKeyframe::kCameraDistance, idt);
            StringPrintf(buffer, sizeof(buffer),
                         "%.f,%.f,%.f,%.f,"
                         "%.f,%.f,%.f,%.f,"
                         "%.f,%.f,%.f,%.f,"
                         "%.f,%.f,%.f,%.f"
                         , ix.x(), ix.y(), ix.z(), ix.w()
                         , ir.x(), ir.y(), ir.z(), ir.w()
                         , idt.x(), idt.y(), idt.z(), idt.w()
                         , ifv.x(), ifv.y(), ifv.z(), ifv.w()
                         );
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("interpolation"), VPVL2_CAST_XC(buffer)));
            VPVL2_XML_RC(xmlTextWriterEndElement(writer));
        }
        VPVL2_XML_RC(xmlTextWriterEndElement(writer)); /* animation */
        return true;
    }
    bool writeMVDEffectKeyframes(xmlTextWriterPtr writer, const mvd::Motion *motion) const {
        uint8_t buffer[kElementContentBufferSize];
        VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("animation"), 0));
        VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("type"), VPVL2_CAST_XC("effect")));
        int nkeyframes = motion->countKeyframes(IKeyframe::kEffectKeyframe);
        for (int i = 0; i < nkeyframes; i++) {
            const mvd::EffectKeyframe *keyframe = static_cast<const mvd::EffectKeyframe *>(motion->findEffectKeyframeRefAt(i));
            VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("keyframe"), 0));
            StringPrintf(buffer, sizeof(buffer), "%ld", static_cast<long>(keyframe->timeIndex()));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("index"), VPVL2_CAST_XC(buffer)));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("visible"), VPVL2_CAST_XC(keyframe->isVisible() ? "true" : "false")));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("addblend"), VPVL2_CAST_XC(keyframe->isAddBlendEnabled() ? "true" : "false")));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("shadow"), VPVL2_CAST_XC(keyframe->isShadowEnabled() ? "true" : "false")));
            StringPrintf(buffer, sizeof(buffer), "%.4f", keyframe->scaleFactor());
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("scale"), VPVL2_CAST_XC(buffer)));
            StringPrintf(buffer, sizeof(buffer), "%.4f", keyframe->opacity());
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("opacity"), VPVL2_CAST_XC(buffer)));
            VPVL2_XML_RC(xmlTextWriterEndElement(writer));
        }
        VPVL2_XML_RC(xmlTextWriterEndElement(writer)); /* animation */
        return true;
    }
    bool writeMVDLightKeyframes(xmlTextWriterPtr writer, const mvd::Motion *motion) const {
        uint8_t buffer[kElementContentBufferSize];
        VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("animation"), 0));
        VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("type"), VPVL2_CAST_XC("light")));
        int nkeyframes = motion->countKeyframes(IKeyframe::kLightKeyframe);
        for (int i = 0; i < nkeyframes; i++) {
            const mvd::LightKeyframe *keyframe = static_cast<const mvd::LightKeyframe *>(motion->findLightKeyframeRefAt(i));
            VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("keyframe"), 0));
            StringPrintf(buffer, sizeof(buffer), "%ld", static_cast<long>(keyframe->timeIndex()));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("index"), VPVL2_CAST_XC(buffer)));
            const Vector3 &color = keyframe->color();
            StringPrintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f", color.x(), color.y(), color.z());
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("color"), VPVL2_CAST_XC(buffer)));
            const Vector3 &direction = keyframe->direction();
            StringPrintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f", direction.x(), direction.y(), direction.z());
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("direction"), VPVL2_CAST_XC(buffer)));
            VPVL2_XML_RC(xmlTextWriterEndElement(writer));
        }
        VPVL2_XML_RC(xmlTextWriterEndElement(writer)); /* animation */
        return true;
    }
    bool writeMVDModelKeyframes(xmlTextWriterPtr writer, const mvd::Motion *motion) const {
        uint8_t buffer[kElementContentBufferSize];
        VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("animation"), 0));
        VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("type"), VPVL2_CAST_XC("model")));
        int nkeyframes = motion->countKeyframes(IKeyframe::kModelKeyframe);
        for (int i = 0; i < nkeyframes; i++) {
            const mvd::ModelKeyframe *keyframe = static_cast<const mvd::ModelKeyframe *>(motion->findModelKeyframeRefAt(i));
            VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("keyframe"), 0));
            StringPrintf(buffer, sizeof(buffer), "%ld", static_cast<long>(keyframe->timeIndex()));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("index"), VPVL2_CAST_XC(buffer)));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("visible"), VPVL2_CAST_XC(keyframe->isVisible() ? "true" : "false")));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("addblend"), VPVL2_CAST_XC(keyframe->isAddBlendEnabled() ? "true" : "false")));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("physics.enable"), VPVL2_CAST_XC(keyframe->isPhysicsEnabled() ? "true" : "false")));
            StringPrintf(buffer, sizeof(buffer), "%d", keyframe->physicsStillMode());
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("physics.mode"), VPVL2_CAST_XC(buffer)));
            StringPrintf(buffer, sizeof(buffer), "%.4f", keyframe->edgeWidth());
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("edge.width"), VPVL2_CAST_XC(buffer)));
            const Color &edgeColor = keyframe->edgeColor();
            StringPrintf(buffer, sizeof(buffer), "%.4f,%.4f,%.4f,%.4f", edgeColor.x(), edgeColor.y(), edgeColor.z(), edgeColor.w());
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("edge.color"), VPVL2_CAST_XC(buffer)));
            // TODO: implement writing IK state
            VPVL2_XML_RC(xmlTextWriterEndElement(writer));
        }
        VPVL2_XML_RC(xmlTextWriterEndElement(writer)); /* animation */
        return true;
    }
    bool writeMVDMorphKeyframes(xmlTextWriterPtr writer, const mvd::Motion *motion) const {
        uint8_t buffer[kElementContentBufferSize];
        VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("animation"), 0));
        VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("type"), VPVL2_CAST_XC("morph")));
        int nkeyframes = motion->countKeyframes(IKeyframe::kMorphKeyframe);
        for (int i = 0; i < nkeyframes; i++) {
            const mvd::MorphKeyframe *keyframe = static_cast<const mvd::MorphKeyframe *>(motion->findMorphKeyframeRefAt(i));
            const std::string &name = delegateRef->toStdFromString(keyframe->name());
            VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("keyframe"), 0));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("name"), VPVL2_CAST_XC(name.c_str())));
            StringPrintf(buffer, sizeof(buffer), "%ld", static_cast<long>(keyframe->timeIndex()));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("index"), VPVL2_CAST_XC(buffer)));
            StringPrintf(buffer, sizeof(buffer), "%.4f", keyframe->weight());
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("weight"), VPVL2_CAST_XC(buffer)));
            VPVL2_XML_RC(xmlTextWriterEndElement(writer));
        }
        VPVL2_XML_RC(xmlTextWriterEndElement(writer)); /* animation */
        return true;
    }
    bool writeMVDProjectKeyframes(xmlTextWriterPtr writer, const mvd::Motion *motion) const {
        uint8_t buffer[kElementContentBufferSize];
        VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("animation"), 0));
        VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("type"), VPVL2_CAST_XC("project")));
        int nkeyframes = motion->countKeyframes(IKeyframe::kProjectKeyframe);
        for (int i = 0; i < nkeyframes; i++) {
            const mvd::ProjectKeyframe *keyframe = static_cast<const mvd::ProjectKeyframe *>(motion->findProjectKeyframeRefAt(i));
            VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("keyframe"), 0));
            StringPrintf(buffer, sizeof(buffer), "%ld", static_cast<long>(keyframe->timeIndex()));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("index"), VPVL2_CAST_XC(buffer)));
            StringPrintf(buffer, sizeof(buffer), "%.8f", keyframe->gravityFactor());
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("gravity.factor"), VPVL2_CAST_XC(buffer)));
            const Vector3 &direction = keyframe->gravityDirection();
            StringPrintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f", direction.x(), direction.y(), direction.z());
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("gravity.direction"), VPVL2_CAST_XC(buffer)));
            StringPrintf(buffer, sizeof(buffer), "%d", keyframe->shadowMode());
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("shadow.mode"), VPVL2_CAST_XC(buffer)));
            StringPrintf(buffer, sizeof(buffer), "%.8f", keyframe->shadowDepth());
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("shadow.depth"), VPVL2_CAST_XC(buffer)));
            StringPrintf(buffer, sizeof(buffer), "%.8f", keyframe->shadowDistance());
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("shadow.distance"), VPVL2_CAST_XC(buffer)));
            VPVL2_XML_RC(xmlTextWriterEndElement(writer));
        }
        VPVL2_XML_RC(xmlTextWriterEndElement(writer)); /* animation */
        return true;
    }
    bool writeStringMap(const xmlChar *prefix, const StringMap &map, xmlTextWriterPtr writer) const {
        for (StringMap::const_iterator it = map.begin(); it != map.end(); it++) {
            if (it->first.empty() || it->second.empty()) {
                continue;
            }
            VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, prefix, VPVL2_CAST_XC("value"), 0));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("name"), VPVL2_CAST_XC(it->first.c_str())));
            VPVL2_XML_RC(xmlTextWriterWriteCDATA(writer, VPVL2_CAST_XC(it->second.c_str())));
            VPVL2_XML_RC(xmlTextWriterEndElement(writer)); /* vpvl:value */
        }
        return true;
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
        PrivateContext *self = static_cast<PrivateContext *>(context);
        if (self->depth == 0 && equals(prefix, localname, "project")) {
            self->readVersion(attributes, nattributes);
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
            if (self->state == kSettings && equals(prefix, localname, "value")) {
                self->readGlobalSettingKey(attributes, nattributes);
            }
            if (self->state == kModels && equals(prefix, localname, "model")) {
                self->readModel(attributes, nattributes);
            }
            else if (self->state == kAssets && equals(prefix, localname, "asset")) {
                self->readAsset(attributes, nattributes);
            }
            else if (self->state == kMotions && equals(prefix, localname, "motion")) {
                self->readMotion(attributes, nattributes);
            }
        }
        else if (self->depth == 3) {
            if ((self->state == kModel || self->state == kAsset) && equals(prefix, localname, "value")) {
                self->readLocalSettingKey(attributes, nattributes);
            }
            else if (self->state == kAnimation && equals(prefix, localname, "animation")) {
                self->readMotionType(attributes, nattributes);
            }
            else if (equals(prefix, localname, "keyframe")) {
#if 0
                // currently do nothing
                switch (self->state) {
                case kAssetMotion:
                    break;
                }
#endif
            }
        }
        else if (self->depth == 4 && equals(localname, "keyframe")) {
            switch (self->state) {
            case kVMDBoneMotion:
                self->readVMDBoneKeyframe(attributes, nattributes);
                break;
            case kVMDMorphMotion:
                self->readVMDMorphKeyframe(attributes, nattributes);
                break;
            case kVMDCameraMotion:
                self->readVMDCameraKeyframe(attributes, nattributes);
                break;
            case kVMDLightMotion:
                self->readVMDLightKeyframe(attributes, nattributes);
                break;
            case kMVDAssetMotion:
                self->readMVDAssetKeyframe(attributes, nattributes);
                break;
            case kMVDBoneMotion:
                self->readMVDBoneKeyframe(attributes, nattributes);
                break;
            case kMVDCameraMotion:
                self->readMVDCameraKeyframe(attributes, nattributes);
                break;
            case kMVDEffectMotion:
                self->readMVDEffectKeyframe(attributes, nattributes);
                break;
            case kMVDLightMotion:
                self->readMVDLightKeyframe(attributes, nattributes);
                break;
            case kMVDModelMotion:
                self->readMVDModelKeyframe(attributes, nattributes);
                break;
            case kMVDMorphMotion:
                self->readMVDMorphKeyframe(attributes, nattributes);
                break;
            case kMVDProjectMotion:
                self->readMVDProjectKeyframe(attributes, nattributes);
                break;
            case kInitial:
            case kProject:
            case kSettings:
            case kPhysics:
            case kModels:
            case kModel:
            case kAssets:
            case kAsset:
            case kMotions:
            case kAnimation:
            default:
                break;
            }
        }
    }
    static void cdataBlock(void *context,
                           const xmlChar *cdata,
                           int len)
    {
        PrivateContext *self = static_cast<PrivateContext *>(context);
        if (self->state == kSettings) {
            std::string value(reinterpret_cast<const char *>(cdata), len);
            self->globalSettings[self->settingKey] = value;
        }
        else if (self->state == kModel) {
            std::string value(reinterpret_cast<const char *>(cdata), len);
            self->localModelSettings[self->uuid][self->settingKey] = value;
        }
        else if (self->state == kAsset) {
            std::string value(reinterpret_cast<const char *>(cdata), len);
            self->localAssetSettings[self->uuid][self->settingKey] = value;
        }
    }
    static void endElement(void *context,
                           const xmlChar *localname,
                           const xmlChar *prefix,
                           const xmlChar * /* URI */)
    {
        PrivateContext *self = static_cast<PrivateContext *>(context);
        if (self->depth == 4 && !equals(localname, "keyframe")) {
            self->popState(kAnimation);
        }
        if (self->depth == 3) {
            switch (self->state) {
            case kAsset:
                if (equals(prefix, localname, "asset")) {
                    self->addAsset();
                }
                self->settingKey.clear();
                break;
            case kModel:
                if (equals(prefix, localname, "model")) {
                    self->addModel();
                }
                self->settingKey.clear();
                break;
            case kAnimation:
                if (equals(prefix, localname, "motion")) {
                    self->addMotion();
                }
                break;
            case kInitial:
            case kProject:
            case kSettings:
            case kPhysics:
            case kModels:
            case kAssets:
            case kMotions:
            case kVMDBoneMotion:
            case kVMDMorphMotion:
            case kVMDCameraMotion:
            case kVMDLightMotion:
            case kMVDAssetMotion:
            case kMVDBoneMotion:
            case kMVDCameraMotion:
            case kMVDEffectMotion:
            case kMVDLightMotion:
            case kMVDModelMotion:
            case kMVDMorphMotion:
            case kMVDProjectMotion:
            default:
                break;
            }
        }
        else if (self->depth == 2) {
            switch (self->state) {
            case kAssets:
                if (equals(prefix, localname, "assets")) {
                    self->popState(kProject);
                }
                break;
            case kModels:
                if (equals(prefix, localname, "models")) {
                    self->popState(kProject);
                }
                break;
            case kMotions:
                if (equals(prefix, localname, "motions")) {
                    self->popState(kProject);
                }
                break;
            case kSettings:
                if (equals(prefix, localname, "settings")) {
                    self->popState(kProject);
                }
                self->settingKey.clear();
                break;
            case kPhysics:
                if (equals(prefix, localname, "physics")) {
                    self->popState(kProject);
                }
                break;
            case kInitial:
            case kProject:
            case kModel:
            case kAsset:
            case kAnimation:
            case kVMDBoneMotion:
            case kVMDMorphMotion:
            case kVMDCameraMotion:
            case kVMDLightMotion:
            case kMVDAssetMotion:
            case kMVDBoneMotion:
            case kMVDCameraMotion:
            case kMVDEffectMotion:
            case kMVDLightMotion:
            case kMVDModelMotion:
            case kMVDMorphMotion:
            case kMVDProjectMotion:
            default:
                break;
            }
        }
        else if (self->depth == 1 && self->state == kProject && equals(prefix, localname, "project")) {
            self->depth--;
        }
    }

    void readVersion(const xmlChar **attributes, int nattributes) {
        std::string key, value;
        for (int i = 0, index = 0; i < nattributes; i++, index += 5) {
            readAttributeString(attributes, index, key, value);
            if (key == "version") {
                version = value;
            }
        }
        pushState(kProject);
    }
    void readGlobalSettingKey(const xmlChar **attributes, int nattributes) {
        std::string key, value;
        for (int i = 0, index = 0; i < nattributes; i++, index += 5) {
            readAttributeString(attributes, index, key, value);
            if (key == "name") {
                settingKey = value;
            }
        }
    }
    void readModel(const xmlChar **attributes, int nattributes) {
        XMLProject::UUID modelUUID;
        std::string key, value;
        for (int i = 0, index = 0; i < nattributes; i++, index += 5) {
            readAttributeString(attributes, index, key, value);
            if (key == "uuid") {
                modelUUID.assign(value);
            }
        }
        uuid.assign(modelUUID);
        pushState(kModel);
    }
    void readAsset(const xmlChar **attributes, int nattributes) {
        XMLProject::UUID assetUUID;
        std::string key, value;
        for (int i = 0, index = 0; i < nattributes; i++, index += 5) {
            readAttributeString(attributes, index, key, value);
            if (key ==  "uuid") {
                assetUUID.assign(value);
            }
        }
        uuid.assign(assetUUID);
        pushState(kAsset);
    }
    void readMotion(const xmlChar **attributes, int nattributes) {
        std::string key, value;
        currentMotionType = IMotion::kVMDMotion;
        for (int i = 0, index = 0; i < nattributes; i++, index += 5) {
            readAttributeString(attributes, index, key, value);
            if (key == "uuid") {
                uuid.assign(value);
                continue;
            }
            else if (key == "model") {
                parentModel.assign(value);
                continue;
            }
            else if (key != "type") {
                continue;
            }
            if (value == "mvd") {
                currentMotionType = IMotion::kMVDMotion;
            }
        }
        delete currentMotion;
        currentMotion = factoryRef->newMotion(currentMotionType, 0);
        if (!parentModel.empty()) {
            ModelMap::const_iterator it = modelRefs.find(parentModel);
            if (it != modelRefs.end()) {
                currentMotion->setParentModelRef(it->second);
            }
            else {
                ModelMap::const_iterator it2 = assetRefs.find(parentModel);
                if (it2 != assetRefs.end()) {
                    currentMotion->setParentModelRef(it2->second);
                }
            }
        }
        pushState(kAnimation);
    }
    void readLocalSettingKey(const xmlChar **attributes, int nattributes) {
        readGlobalSettingKey(attributes, nattributes);
    }
    void readMotionType(const xmlChar **attributes, int nattributes) {
        std::string key, value;
        bool isMVD = currentMotionType == IMotion::kMVDMotion;
        for (int i = 0, index = 0; i < nattributes; i++, index += 5) {
            readAttributeString(attributes, index, key, value);
            if (key != "type") {
                continue;
            }
            if (isMVD) {
                if (value == "bone") {
                    pushState(kMVDBoneMotion);
                }
                else if (value == "asset") {
                    pushState(kMVDAssetMotion);
                }
                if (value == "model") {
                    pushState(kMVDModelMotion);
                }
                else if (value == "light") {
                    pushState(kMVDLightMotion);
                }
                else if (value == "morph") {
                    pushState(kMVDMorphMotion);
                }
                else if (value == "camera") {
                    pushState(kMVDCameraMotion);
                }
                else if (value == "effect") {
                    pushState(kMVDEffectMotion);
                }
                else if (value == "project") {
                    pushState(kMVDProjectMotion);
                }
            }
            else {
                if (value == "bone") {
                    pushState(kVMDBoneMotion);
                }
                else if (value == "light") {
                    pushState(kVMDLightMotion);
                }
                else if (value == "morph") {
                    pushState(kVMDMorphMotion);
                }
                else if (value == "camera") {
                    pushState(kVMDCameraMotion);
                }
            }
        }
    }
    void readVMDBoneKeyframe(const xmlChar **attributes, int nattributes) {
        IBoneKeyframe *keyframe = factoryRef->createBoneKeyframe(currentMotion);
        if (keyframe) {
            Array<std::string> tokens;
            Vector4 vec4(kZeroV4);
            Vector3 vec3(kZeroV3);
            QuadWord qw(0, 0, 0, 0);
            std::string key, value;
            keyframe->setDefaultInterpolationParameter();
            for (int i = 0, index = 0; i < nattributes; i++, index += 5) {
                readAttributeString(attributes, index, key, value);
                if (key == "ik") {
                    // keyframe->setIKEnable(value == "true");
                }
                else if (key == "name") {
                    delete currentString;
                    currentString = delegateRef->toStringFromStd(value);
                    keyframe->setName(currentString);
                }
                else if (key == "index") {
                    keyframe->setTimeIndex(StringToFloat(value));
                }
                else if (key == "position") {
                    splitString(value, tokens);
                    if (tryCreateVector3(tokens, vec3)) {
#ifdef VPVL2_COORDINATE_OPENGL
                        vec3.setValue(vec3.x(), vec3.y(), -vec3.z());
#else
                        vec3.setValue(vec3.x(), vec3.y(), vec3.z());
#endif
                        keyframe->setLocalTranslation(vec3);
                    }
                }
                else if (key == "rotation") {
                    splitString(value, tokens);
                    if (tryCreateVector4(tokens, vec4)) {
                        Quaternion rotation;
#ifdef VPVL2_COORDINATE_OPENGL
                        rotation.setValue(-vec4.x(), -vec4.y(), vec4.z(), vec4.w());
#else
                        rotation.setValue(vec4.x(), vec4.y(), vec4.z(), vec4.w());
#endif
                        keyframe->setLocalRotation(rotation);
                    }
                }
                else if (key == "interpolation") {
                    splitString(value, tokens);
                    if (tokens.count() == 16) {
                        for (int i = 0; i < 4; i++) {
                            setQuadWordValues(tokens, qw, i * 4);
                            keyframe->setInterpolationParameter(static_cast<IBoneKeyframe::InterpolationType>(i), qw);
                        }
                    }
                }
            }
            currentMotion->addKeyframe(keyframe);
        }
    }
    void readVMDCameraKeyframe(const xmlChar **attributes, int nattributes) {
        ICameraKeyframe *keyframe = factoryRef->createCameraKeyframe(currentMotion);
        if (keyframe) {
            Vector3 vec3(kZeroV3);
            QuadWord qw(0, 0, 0, 0);
            Array<std::string> tokens;
            std::string key, value;
            keyframe->setDefaultInterpolationParameter();
            for (int i = 0, index = 0; i < nattributes; i++, index += 5) {
                readAttributeString(attributes, index, key, value);
                if (key == "fovy") {
                    keyframe->setFov(StringToFloat(value));
                }
                else if (key == "index") {
                    keyframe->setTimeIndex(StringToFloat(value));
                }
                else if (key == "angle") {
                    splitString(value, tokens);
                    if (tryCreateVector3(tokens, vec3)) {
#ifdef VPVL2_COORDINATE_OPENGL
                        vec3.setValue(-btDegrees(vec3.x()), -btDegrees(vec3.y()), -btDegrees(vec3.z()));
#else
                        vec3.setValue(btDegrees(vec3.x()), btDegrees(vec3.y()), -btDegrees(vec3.z()));
#endif
                        keyframe->setAngle(vec3);
                    }
                }
                else if (key == "position") {
                    splitString(value, tokens);
                    if (tryCreateVector3(tokens, vec3)) {
#ifdef VPVL2_COORDINATE_OPENGL
                        vec3.setValue(vec3.x(), vec3.y(), -vec3.z());
#else
                        vec3.setValue(vec3.x(), vec3.y(), vec3.z());
#endif
                        keyframe->setLookAt(vec3);
                    }
                }
                else if (key == "distance") {
                    keyframe->setDistance(StringToFloat(value));
                }
                else if (key == "interpolation") {
                    splitString(value, tokens);
                    if (tokens.count() == 24) {
                        for (int i = 0; i < 6; i++) {
                            setQuadWordValues(tokens, qw, i * 4);
                            keyframe->setInterpolationParameter(static_cast<ICameraKeyframe::InterpolationType>(i), qw);
                        }
                    }
                }
            }
            currentMotion->addKeyframe(keyframe);
        }
    }
    void readVMDLightKeyframe(const xmlChar **attributes, int nattributes) {
        ILightKeyframe *keyframe = factoryRef->createLightKeyframe(currentMotion);
        if (keyframe) {
            Array<std::string> tokens;
            Vector3 vec3(kZeroV3);
            std::string key, value;
            for (int i = 0, index = 0; i < nattributes; i++, index += 5) {
                readAttributeString(attributes, index, key, value);
                if (key == "index") {
                    keyframe->setTimeIndex(StringToFloat(value));
                }
                else if (key == "color") {
                    splitString(value, tokens);
                    if (tryCreateVector3(tokens, vec3)) {
                        keyframe->setColor(vec3);
                    }
                }
                else if (key == "direction") {
                    splitString(value, tokens);
                    if (tryCreateVector3(tokens, vec3)) {
                        keyframe->setDirection(vec3);
                    }
                }
            }
            currentMotion->addKeyframe(keyframe);
        }
    }
    void readVMDMorphKeyframe(const xmlChar **attributes, int nattributes) {
        IMorphKeyframe *keyframe = factoryRef->createMorphKeyframe(currentMotion);
        if (keyframe) {
            std::string key, value;
            for (int i = 0, index = 0; i < nattributes; i++, index += 5) {
                readAttributeString(attributes, index, key, value);
                if (key == "name") {
                    delete currentString;
                    currentString = delegateRef->toStringFromStd(value);
                    keyframe->setName(currentString);
                }
                else if (key == "index") {
                    keyframe->setTimeIndex(StringToFloat(value));
                }
                else if (key == "weight") {
                    keyframe->setWeight(StringToFloat(value));
                }
            }
            currentMotion->addKeyframe(keyframe);
        }
    }
    void readMVDAssetKeyframe(const xmlChar **attributes, int nattributes) {
        // FIXME: add createAssetKeyframe
        IMorphKeyframe *keyframe = factoryRef->createMorphKeyframe(currentMotion);
        if (keyframe) {
            std::string key, value;
            for (int i = 0, index = 0; i < nattributes; i++, index += 5) {
                readAttributeString(attributes, index, key, value);
            }
            currentMotion->addKeyframe(keyframe);
        }
    }
    void readMVDBoneKeyframe(const xmlChar **attributes, int nattributes) {
        IBoneKeyframe *keyframe = factoryRef->createBoneKeyframe(currentMotion);
        if (keyframe) {
            Array<std::string> tokens;
            Vector4 vec4(kZeroV4);
            Vector3 vec3(kZeroV3);
            QuadWord qw(0, 0, 0, 0);
            std::string key, value;
            for (int i = 0, index = 0; i < nattributes; i++, index += 5) {
                readAttributeString(attributes, index, key, value);
                if (key == "name") {
                    delete currentString;
                    currentString = delegateRef->toStringFromStd(value);
                    keyframe->setName(currentString);
                }
                else if (key == "index") {
                    keyframe->setTimeIndex(StringToDouble(value));
                }
                else if (key == "layer") {
                    keyframe->setLayerIndex(StringToInt(value));
                }
                else if (key == "position") {
                    splitString(value, tokens);
                    if (tryCreateVector3(tokens, vec3)) {
#ifdef VPVL2_COORDINATE_OPENGL
                        vec3.setValue(vec3.x(), vec3.y(), -vec3.z());
#else
                        vec3.setValue(vec3.x(), vec3.y(), vec3.z());
#endif
                        keyframe->setLocalTranslation(vec3);
                    }
                }
                else if (key == "rotation") {
                    splitString(value, tokens);
                    if (tryCreateVector4(tokens, vec4)) {
                        Quaternion rotation;
#ifdef VPVL2_COORDINATE_OPENGL
                        rotation.setValue(-vec4.x(), -vec4.y(), vec4.z(), vec4.w());
#else
                        rotation.setValue(vec4.x(), vec4.y(), vec4.z(), vec4.w());
#endif
                        keyframe->setLocalRotation(rotation);
                    }
                }
                else if (key == "interpolation") {
                    splitString(value, tokens);
                    if (tokens.count() == 16) {
                        for (int i = 0; i < 4; i++) {
                            setQuadWordValues(tokens, qw, i * 4);
                            keyframe->setInterpolationParameter(static_cast<IBoneKeyframe::InterpolationType>(i), qw);
                        }
                    }
                }
            }
            currentMotion->addKeyframe(keyframe);
        }
    }
    void readMVDCameraKeyframe(const xmlChar **attributes, int nattributes) {
        ICameraKeyframe *keyframe = factoryRef->createCameraKeyframe(currentMotion);
        if (keyframe) {
            Array<std::string> tokens;
            Vector3 vec3(kZeroV3);
            QuadWord qw(0, 0, 0, 0);
            std::string key, value;
            for (int i = 0, index = 0; i < nattributes; i++, index += 5) {
                readAttributeString(attributes, index, key, value);
                if (key == "fovy") {
                    keyframe->setFov(StringToFloat(value));
                }
                else if (key == "index") {
                    keyframe->setTimeIndex(StringToDouble(value));
                }
                else if (key == "layer") {
                    keyframe->setLayerIndex(StringToInt(value));
                }
                else if (key == "angle") {
                    splitString(value, tokens);
                    if (tryCreateVector3(tokens, vec3)) {
#ifdef VPVL2_COORDINATE_OPENGL
                        vec3.setValue(-btDegrees(vec3.x()), -btDegrees(vec3.y()), -btDegrees(vec3.z()));
#else
                        vec3.setValue(btDegrees(vec3.x()), btDegrees(vec3.y()), -btDegrees(vec3.z()));
#endif
                        keyframe->setAngle(vec3);
                    }
                }
                else if (key == "position") {
                    splitString(value, tokens);
                    if (tryCreateVector3(tokens, vec3)) {
#ifdef VPVL2_COORDINATE_OPENGL
                        vec3.setValue(vec3.x(), vec3.y(), -vec3.z());
#else
                        vec3.setValue(vec3.x(), vec3.y(), vec3.z());
#endif
                        keyframe->setLookAt(vec3);
                    }
                }
                else if (key == "distance") {
                    keyframe->setDistance(StringToFloat(value));
                }
                else if (key == "interpolation") {
                    splitString(value, tokens);
                    if (tokens.count() == 16) {
                        for (int i = 0; i < 4; i++) {
                            setQuadWordValues(tokens, qw, i * 4);
                            keyframe->setInterpolationParameter(static_cast<ICameraKeyframe::InterpolationType>(i + 2), qw);
                        }
                    }
                }
            }
            currentMotion->addKeyframe(keyframe);
        }
    }
    void readMVDEffectKeyframe(const xmlChar **attributes, int nattributes) {
        IEffectKeyframe *keyframe = factoryRef->createEffectKeyframe(currentMotion);
        if (keyframe) {
            std::string key, value;
            for (int i = 0, index = 0; i < nattributes; i++, index += 5) {
                readAttributeString(attributes, index, key, value);
                if (key == "index") {
                    keyframe->setTimeIndex(StringToDouble(value));
                }
                else if (key == "visible") {
                    keyframe->setVisible(StringToBool(value));
                }
                else if (key == "addblend") {
                    keyframe->setAddBlendEnable(StringToBool(value));
                }
                else if (key == "shadow") {
                    keyframe->setShadowEnable(StringToBool(value));
                }
                else if (key == "scale") {
                    keyframe->setScaleFactor(StringToFloat(value));
                }
                else if (key == "opacity") {
                    keyframe->setOpacity(StringToFloat(value));
                }
                else if (key == "model") {
                }
                else if (key == "bone") {
                }
            }
            currentMotion->addKeyframe(keyframe);
        }
    }
    void readMVDLightKeyframe(const xmlChar **attributes, int nattributes) {
        ILightKeyframe *keyframe = factoryRef->createLightKeyframe(currentMotion);
        if (keyframe) {
            Array<std::string> tokens;
            Vector3 vec3(kZeroV3);
            std::string key, value;
            for (int i = 0, index = 0; i < nattributes; i++, index += 5) {
                readAttributeString(attributes, index, key, value);
                if (key == "index") {
                    keyframe->setTimeIndex(StringToDouble(value));
                }
                else if (key == "color") {
                    splitString(value, tokens);
                    if (tryCreateVector3(tokens, vec3)) {
                        keyframe->setColor(vec3);
                    }
                }
                else if (key == "direction") {
                    splitString(value, tokens);
                    if (tryCreateVector3(tokens, vec3)) {
                        keyframe->setDirection(vec3);
                    }
                }
            }
            currentMotion->addKeyframe(keyframe);
        }
    }
    void readMVDModelKeyframe(const xmlChar **attributes, int nattributes) {
        IModelKeyframe *keyframe = factoryRef->createModelKeyframe(currentMotion);
        if (keyframe) {
            Array<std::string> tokens;
            Vector4 vec4(kZeroV4);
            std::string key, value;
            for (int i = 0, index = 0; i < nattributes; i++, index += 5) {
                readAttributeString(attributes, index, key, value);
                if (key == "index") {
                    keyframe->setTimeIndex(StringToDouble(value));
                }
                else if (key == "visible") {
                    keyframe->setVisible(StringToBool(value));
                }
                else if (key == "addblend") {
                    keyframe->setAddBlendEnable(StringToBool(value));
                }
                else if (key == "shadow") {
                    keyframe->setShadowEnable(StringToBool(value));
                }
                else if (key == "physics.enable") {
                    keyframe->setPhysicsEnable(StringToBool(value));
                }
                else if (key == "physics.mode") {
                    keyframe->setPhysicsStillMode(StringToInt(value));
                }
                else if (key == "edge.width") {
                    keyframe->setEdgeWidth(StringToFloat(value));
                }
                else if (key == "edge.color") {
                    splitString(value, tokens);
                    if (tryCreateVector4(tokens, vec4))
                        keyframe->setEdgeColor(vec4);
                }
            }
            currentMotion->addKeyframe(keyframe);
        }
    }
    void readMVDMorphKeyframe(const xmlChar **attributes, int nattributes) {
        IMorphKeyframe *keyframe = factoryRef->createMorphKeyframe(currentMotion);
        if (keyframe) {
            std::string key, value;
            for (int i = 0, index = 0; i < nattributes; i++, index += 5) {
                readAttributeString(attributes, index, key, value);
                if (key == "name") {
                    delete currentString;
                    currentString = delegateRef->toStringFromStd(value);
                    keyframe->setName(currentString);
                }
                else if (key == "index") {
                    keyframe->setTimeIndex(StringToDouble(value));
                }
                else if (key == "weight") {
                    keyframe->setWeight(StringToFloat(value));
                }
            }
            currentMotion->addKeyframe(keyframe);
        }
    }
    void readMVDProjectKeyframe(const xmlChar **attributes, int nattributes) {
        IProjectKeyframe *keyframe = factoryRef->createProjectKeyframe(currentMotion);
        if (keyframe) {
            Array<std::string> tokens;
            Vector3 vec3(kZeroV3);
            std::string key, value;
            for (int i = 0, index = 0; i < nattributes; i++, index += 5) {
                readAttributeString(attributes, index, key, value);
                if (key == "index") {
                    keyframe->setTimeIndex(StringToDouble(value));
                }
                else if (key == "gravity.factor") {
                    keyframe->setGravityFactor(StringToFloat(value));
                }
                else if (key == "gravity.direction") {
                    splitString(value, tokens);
                    if (tryCreateVector3(tokens, vec3)) {
                        keyframe->setGravityDirection(vec3);
                    }
                }
                else if (key == "shadow.mode") {
                    keyframe->setShadowMode(StringToInt(value));
                }
                else if (key == "shadow.depth") {
                    keyframe->setShadowDepth(StringToFloat(value));
                }
                else if (key == "shadow.distance") {
                    keyframe->setShadowDistance(StringToFloat(value));
                }
            }
            currentMotion->addKeyframe(keyframe);
        }
    }

    void addAsset() {
        if (!uuid.empty()) {
            if (uuid != XMLProject::kNullUUID) {
                /* delete the previous asset before assigning to prevent memory leak */
                ModelMap::iterator it = assetRefs.find(uuid);
                if (it != assetRefs.end()) {
                    assetRefs.erase(it);
                    sceneRef->removeModel(it->second);
                    delete it->second;
                }
                IModel *assetPtr = 0;
                IRenderEngine *enginePtr = 0;
                int priority = 0;
                if (delegateRef->loadModel(uuid, localAssetSettings[uuid], IModel::kAssetModel, assetPtr, enginePtr, priority)) {
                    assetRefs.insert(std::make_pair(uuid, assetPtr));
                    sceneRef->addModel(assetPtr, enginePtr, priority);
                }
            }
            else {
                localAssetSettings.erase(uuid);
            }
            uuid.clear();
        }
        popState(kAssets);
        uuid.clear();
    }
    void addModel() {
        if (!uuid.empty()) {
            if (uuid != XMLProject::kNullUUID) {
                /* delete the previous model before assigning to prevent memory leak */
                ModelMap::iterator it = modelRefs.find(uuid);
                if (it != modelRefs.end()) {
                    modelRefs.erase(it);
                    sceneRef->removeModel(it->second);
                    delete it->second;
                }
                IModel *modelPtr = 0;
                IRenderEngine *enginePtr = 0;
                int priority = 0;
                if (delegateRef->loadModel(uuid, localModelSettings[uuid], IModel::kPMDModel, modelPtr, enginePtr, priority)) {
                    modelRefs.insert(std::make_pair(uuid, modelPtr));
                    sceneRef->addModel(modelPtr, enginePtr, priority);
                }
            }
            else {
                localModelSettings.erase(uuid);
            }
            uuid.clear();
        }
        popState(kModels);
        uuid.clear();
    }
    void addMotion() {
        if (!uuid.empty()) {
            if (uuid != XMLProject::kNullUUID && currentMotion) {
                MotionMap::iterator it = motionRefs.find(uuid);
                if (it != motionRefs.end()) {
                    motionRefs.erase(it);
                    sceneRef->removeMotion(it->second);
                    delete it->second;
                }
                if (!parentModel.empty()) {
                    ModelMap::const_iterator it2 = modelRefs.find(parentModel);
                    if (it2 != modelRefs.end()) {
                        currentMotion->setParentModelRef(it2->second);
                    }
                }
                motionRefs.insert(std::make_pair(uuid, currentMotion));
                sceneRef->addMotion(currentMotion);
            }
            else {
                delete currentMotion;
            }
            currentMotion = 0;
        }
        uuid.clear();
        parentModel.clear();
        popState(kMotions);
    }

    bool save(xmlTextWriterPtr ptr) {
        const ICamera *camera = sceneRef->camera();
        globalSettings["state.camera.angle"] = XMLProject::toStringFromVector3(camera->angle());
        globalSettings["state.camera.distance"] = XMLProject::toStringFromFloat32(camera->distance());
        globalSettings["state.camera.fov"] = XMLProject::toStringFromFloat32(camera->fov());
        globalSettings["state.camera.lookat"] = XMLProject::toStringFromVector3(camera->lookAt());
        const ILight *light = sceneRef->light();
        globalSettings["state.light.color"] = XMLProject::toStringFromVector3(light->color());
        globalSettings["state.light.direction"] = XMLProject::toStringFromVector3(light->direction());
        bool ret = writeXml(ptr);
        xmlFreeTextWriter(ptr);
        if (ret) {
            dirty = false;
        }
        return ret;
    }
    bool validate(bool result) {
        return result && depth == 0 && checkDuplicateUUID();
    }
    void restoreModelStates(const ModelMap &modelMap, const ModelSettings &settings) {
        std::string value;
        for (ModelMap::const_iterator it = modelMap.begin(); it != modelMap.end(); it++) {
            IModel *model = it->second;
            ModelSettings::const_iterator it2 = settings.find(it->first);
            if (it2 != settings.end()) {
                const StringMap &settings = it2->second;
                if (settings.tryGetValue("state.opacity", value) ||
                        settings.tryGetValue("opacity", value)) {
                    model->setOpacity(XMLProject::toFloat32FromString(value));
                }
                if (settings.tryGetValue("state.scale", value) ||
                        settings.tryGetValue("factor", value)) {
                    model->setScaleFactor(XMLProject::toFloat32FromString(value));
                }
                if (settings.tryGetValue("state.offset.position", value) ||
                        settings.tryGetValue("offset.position", value)) {
                    model->setWorldPosition(XMLProject::toVector3FromString(value));
                }
                if (settings.tryGetValue("state.offset.rotation", value) ||
                        settings.tryGetValue("offset.rotation", value)) {
                    model->setWorldRotation(XMLProject::toQuaternionFromString(value));
                }
                if (settings.tryGetValue("state.edge.color", value) ||
                        settings.tryGetValue("edge.color", value)) {
                    model->setEdgeColor(XMLProject::toVector3FromString(value));
                }
                if (settings.tryGetValue("state.edge.offset", value) ||
                        settings.tryGetValue("edge.offset", value)) {
                    model->setEdgeWidth(XMLProject::toFloat32FromString(value));
                }
                IModel *parentModelRef = 0;
                if (settings.tryGetValue("state.parent.model", value)) {
                    parentModelRef = findModel(value);
                    if (parentModelRef) {
                        model->setParentModelRef(parentModelRef);
                    }
                }
                if (settings.tryGetValue("state.parent.bone", value) && parentModelRef) {
                    const IString *name = delegateRef->toStringFromStd(value);
                    if (IBone *bone = parentModelRef->findBoneRef(name)) {
                        model->setParentBoneRef(bone);
                    }
                }
            }
        }
    }
    void restoreSceneStates() {
        ICamera *camera = sceneRef->camera();
        std::string value;
        if (globalSettings.tryGetValue("state.camera.angle", value)) {
            camera->setAngle(XMLProject::toVector3FromString(value));
        }
        if (globalSettings.tryGetValue("state.camera.distance", value)) {
            camera->setDistance(XMLProject::toFloat32FromString(value));
        }
        if (globalSettings.tryGetValue("state.camera.fov", value)) {
            camera->setFov(XMLProject::toFloat32FromString(value));
        }
        if (globalSettings.tryGetValue("state.camera.lookat", value)) {
            camera->setLookAt(XMLProject::toVector3FromString(value));
        }
        ILight *light = sceneRef->light();
        if (globalSettings.tryGetValue("state.light.color", value)) {
            light->setColor(XMLProject::toVector3FromString(value));
        }
        if (globalSettings.tryGetValue("state.light.direction", value)) {
            light->setDirection(XMLProject::toVector3FromString(value));
        }
    }
    void restoreStates() {
        restoreSceneStates();
        restoreModelStates(modelRefs, localModelSettings);
        restoreModelStates(assetRefs, localAssetSettings);
    }

    typedef std::pair<XMLProject::UUID, IModel *> ModelPair;
    typedef std::vector<ModelPair> ModelList;
    class ModelPredication {
    public:
        ModelPredication(PrivateContext *context)
            : m_context(context)
        {
        }
        bool operator()(const ModelPair &left, const ModelPair &right) const {
            return order(left.first) < order(right.first);
        }
        int order(const XMLProject::UUID &uuid) const {
            const ModelSettings::const_iterator &it = m_context->localModelSettings.find(uuid);
            if (it != m_context->localModelSettings.end()) {
                std::string value;
                if (it->second.tryGetValue(XMLProject::kSettingOrderKey, value)) {
                    return StringToInt(value);
                }
            }
            return -1;
        }

    private:
        const PrivateContext *m_context;
    };
    class AssetPredication {
    public:
        AssetPredication(PrivateContext *context)
            : m_context(context)
        {
        }
        bool operator()(const ModelPair &left, const ModelPair &right) const {
            return order(left.first) < order(right.first);
        }
        int order(const XMLProject::UUID &uuid) const {
            const ModelSettings::const_iterator &it = m_context->localAssetSettings.find(uuid);
            if (it != m_context->localAssetSettings.end()) {
                std::string value;
                if (it->second.tryGetValue(XMLProject::kSettingOrderKey, value)) {
                    return StringToInt(value);
                }
            }
            return -1;
        }

    private:
        const PrivateContext *m_context;
    };

    void sort() {
        ModelList modelList, assetList;
        for (ModelMap::const_iterator it = modelRefs.begin(); it != modelRefs.end(); ++it) {
            modelList.push_back(*it);
        }
        for (ModelMap::const_iterator it = assetRefs.begin(); it != assetRefs.end(); ++it) {
            assetList.push_back(*it);
        }
        modelRefs.clear(); assetRefs.clear();
        std::sort(modelList.begin(), modelList.end(), ModelPredication(this));
        std::sort(assetList.begin(), assetList.end(), AssetPredication(this));
        for (ModelList::const_iterator it = modelList.begin(); it != modelList.end(); ++it) {
            modelRefs.insert(*it);
        }
        for (ModelList::const_iterator it = assetList.begin(); it != assetList.end(); ++it) {
            assetRefs.insert(*it);
        }
    }

    static inline void readAttributeString(const xmlChar **attributes,
                                           int index,
                                           std::string &key,
                                           std::string &value) {
        key.assign(reinterpret_cast<const char *>(attributes[index]));
        value.assign(reinterpret_cast<const char *>(attributes[index + 3]),
                reinterpret_cast<const char *>(attributes[index + 4]));
    }

    __attribute__((format(printf, 2, 3)))
    static void error(void * /* context */, const char *format, ...) {
        char bufsiz[1024];
        va_list ap;
        va_start(ap, format);
        ::vsnprintf(bufsiz, sizeof(bufsiz), format, ap);
        va_end(ap);
        VPVL2_LOG(ERROR, bufsiz);
    }

    __attribute__((format(printf, 2, 3)))
    static void warning(void * /* context */, const char *format, ...) {
        char bufsiz[1024];
        va_list ap;
        va_start(ap, format);
        ::vsnprintf(bufsiz, sizeof(bufsiz), format, ap);
        va_end(ap);
        VPVL2_LOG(WARNING, bufsiz);
    }

    xmlSAXHandler saxHandler;
    XMLProject::IDelegate *delegateRef;
    Scene *sceneRef;
    Factory *factoryRef;
    ModelMap assetRefs;
    ModelMap modelRefs;
    MotionMap motionRefs;
    StringMap globalSettings;
    ModelSettings localAssetSettings;
    ModelSettings localModelSettings;
    std::string version;
    std::string settingKey;
    std::string parentModel;
    XMLProject::UUID uuid;
    const IString *currentString;
    IMotion *currentMotion;
    IMotion::Type currentMotionType;
    State state;
    int depth;
    bool dirty;
};

const std::string XMLProject::PrivateContext::kEmpty = "";
const XMLProject::UUID XMLProject::kNullUUID = "{00000000-0000-0000-0000-000000000000}";
const std::string XMLProject::kSettingNameKey = "name";
const std::string XMLProject::kSettingURIKey = "uri";
const std::string XMLProject::kSettingArchiveURIKey = "uri.archive";
const std::string XMLProject::kSettingOrderKey = "order";

float32_t XMLProject::formatVersion()
{
    return 2.1f;
}

bool XMLProject::isReservedSettingKey(const std::string &key)
{
    return key.find(kSettingNameKey) == 0 || key.find(kSettingURIKey) == 0 || key.find(kSettingOrderKey) == 0;
}

std::string XMLProject::toStringFromFloat32(float32_t value)
{
    uint8_t buffer[16];
    StringPrintf(buffer, sizeof(buffer), "%.5f", value);
    return reinterpret_cast<const char *>(buffer);
}

std::string XMLProject::toStringFromVector3(const Vector3 &value)
{
    uint8_t buffer[64];
    StringPrintf(buffer, sizeof(buffer), "%.5f,%.5f,%.5f", value.x(), value.y(), value.z());
    return reinterpret_cast<const char *>(buffer);
}

std::string XMLProject::toStringFromVector4(const Vector4 &value)
{
    uint8_t buffer[80];
    StringPrintf(buffer, sizeof(buffer), "%.5f,%.5f,%.5f,%.5f", value.x(), value.y(), value.z(), value.w());
    return reinterpret_cast<const char *>(buffer);
}

std::string XMLProject::toStringFromQuaternion(const Quaternion &value)
{
    uint8_t buffer[80];
    StringPrintf(buffer, sizeof(buffer), "%.5f,%.5f,%.5f,%.5f", value.x(), value.y(), value.z(), value.w());
    return reinterpret_cast<const char *>(buffer);
}

int XMLProject::toIntFromString(const std::string &value)
{
    int ret = 0;
    ::sscanf(value.c_str(), "%i", &ret);
    return ret;
}

float32_t XMLProject::toFloat32FromString(const std::string &value)
{
    float32_t ret = 0;
    ::sscanf(value.c_str(), "%f", &ret);
    return ret;
}

Vector3 XMLProject::toVector3FromString(const std::string &value)
{
    Vector3 ret;
    float32_t x = 0, y = 0, z = 0;
    ::sscanf(value.c_str(), "%f,%f,%f", &x, &y, &z);
    ret.setValue(x, y, z);
    return ret;
}

Vector4 XMLProject::toVector4FromString(const std::string &value)
{
    Vector4 ret;
    float32_t x = 0, y = 0, z = 0, w = 0;
    ::sscanf(value.c_str(), "%f,%f,%f,%f", &x, &y, &z, &w);
    ret.setValue(x, y, z, w);
    return ret;
}

Quaternion XMLProject::toQuaternionFromString(const std::string &value)
{
    Quaternion ret;
    float32_t x = 0, y = 0, z = 0, w = 0;
    ::sscanf(value.c_str(), "%f,%f,%f,%f", &x, &y, &z, &w);
    ret.setValue(x, y, z, w);
    return ret;
}

XMLProject::XMLProject(IDelegate *delegate, Factory *factory, bool ownMemory)
    : Scene(ownMemory),
      m_context(0)
{
    m_context = new PrivateContext(this, delegate, factory);
}

XMLProject::~XMLProject()
{
    delete m_context;
    m_context = 0;
}

bool XMLProject::load(const char *path)
{
    bool ret = m_context->validate(xmlSAXUserParseFile(&m_context->saxHandler, m_context, path) == 0);
    m_context->sort();
    m_context->restoreStates();
    return ret;
}

bool XMLProject::load(const uint8_t *data, size_t size)
{
    bool ret = m_context->validate(xmlSAXUserParseMemory(&m_context->saxHandler, m_context, reinterpret_cast<const char *>(data), size) == 0);
    m_context->sort();
    m_context->restoreStates();
    return ret;
}

bool XMLProject::save(const char *path)
{
    return m_context->save(xmlNewTextWriterFilename(path, 0));
}

bool XMLProject::save(xmlBufferPtr &buffer)
{
    return m_context->save(xmlNewTextWriterMemory(buffer, 0));
}

std::string XMLProject::version() const
{
    return m_context->version;
}

std::string XMLProject::globalSetting(const std::string &key) const
{
    return m_context->globalSettings.value(key);
}

std::string XMLProject::modelSetting(const IModel *model, const std::string &key) const
{
    if (model) {
        switch (model->type()) {
        case IModel::kAssetModel: {
            const UUID &uuid = modelUUID(model);
            if (uuid != kNullUUID) {
                const std::string &value = m_context->localAssetSettings[uuid][key];
                return value;
            }
            return PrivateContext::kEmpty;
        }
        case IModel::kPMDModel:
        case IModel::kPMXModel: {
            const UUID &uuid = modelUUID(model);
            if (uuid != kNullUUID) {
                const std::string &value = m_context->localModelSettings[uuid][key];
                return value;
            }
            return PrivateContext::kEmpty;
        }
        default:
            return PrivateContext::kEmpty;
        }
    }
    return PrivateContext::kEmpty;
}

const XMLProject::UUIDList XMLProject::modelUUIDs() const
{
    XMLProject::UUIDList uuids;
    const PrivateContext::ModelMap &assets = m_context->assetRefs;
    for (PrivateContext::ModelMap::const_iterator it = assets.begin(); it != assets.end(); it++) {
        uuids.push_back(it->first);
    }
    const PrivateContext::ModelMap &models = m_context->modelRefs;
    for (PrivateContext::ModelMap::const_iterator it = models.begin(); it != models.end(); it++) {
        uuids.push_back(it->first);
    }
    return uuids;
}

const XMLProject::UUIDList XMLProject::motionUUIDs() const
{
    const PrivateContext::MotionMap &motions = m_context->motionRefs;
    XMLProject::UUIDList uuids;
    for (PrivateContext::MotionMap::const_iterator it = motions.begin(); it != motions.end(); it++) {
        uuids.push_back(it->first);
    }
    return uuids;
}

IModel *XMLProject::findModel(const UUID &uuid) const
{
    return m_context->findModel(uuid);
}

IMotion *XMLProject::findMotion(const UUID &uuid) const
{
    return m_context->findMotion(uuid);
}

XMLProject::UUID XMLProject::modelUUID(const IModel *model) const
{
    return m_context->findModelUUID(model);
}

XMLProject::UUID XMLProject::motionUUID(const IMotion *motion) const
{
    return m_context->findMotionUUID(motion);
}

bool XMLProject::containsModel(const IModel *model) const
{
    return modelUUID(model) != kNullUUID;
}

bool XMLProject::containsMotion(const IMotion *motion) const
{
    return motionUUID(motion) != kNullUUID;
}

bool XMLProject::isDirty() const
{
    return m_context->dirty;
}

void XMLProject::setDirty(bool value)
{
    m_context->dirty = value;
}

void XMLProject::addModel(IModel *model, IRenderEngine *engine, const UUID &uuid, int order)
{
    if (!containsModel(model)) {
        switch (model->type()) {
        case IModel::kAssetModel:
            m_context->assetRefs.insert(std::make_pair(uuid, model));
            break;
        case IModel::kPMDModel:
        case IModel::kPMXModel:
            m_context->modelRefs.insert(std::make_pair(uuid, model));
            break;
        default:
            return;
        }
        Scene::addModel(model, engine, order);
        std::ostringstream s; s << order;
        setModelSetting(model, kSettingOrderKey, s.str());
        setDirty(true);
    }
}

void XMLProject::addMotion(IMotion *motion, const UUID &uuid)
{
    if (!containsMotion(motion)) {
        m_context->motionRefs.insert(std::make_pair(uuid, motion));
        Scene::addMotion(motion);
        setDirty(true);
    }
}

void XMLProject::removeModel(IModel *model)
{
    if (m_context->removeModel(model)) {
        setDirty(true);
    }
}

void XMLProject::removeMotion(IMotion *motion)
{
    if (m_context->removeMotion(motion)) {
        Scene::removeMotion(motion);
        setDirty(true);
    }
}

void XMLProject::setGlobalSetting(const std::string &key, const std::string &value)
{
    m_context->globalSettings[key] = value;
    setDirty(true);
}

void XMLProject::setModelSetting(const IModel *model, const std::string &key, const std::string &value)
{
    const UUID &uuid = modelUUID(model);
    if (uuid != kNullUUID) {
        switch (model->type()) {
        case IModel::kAssetModel:
            m_context->localAssetSettings[uuid][key] = value;
            break;
        case IModel::kPMDModel:
        case IModel::kPMXModel:
            m_context->localModelSettings[uuid][key] = value;
            break;
        default:
            return;
        }
        setDirty(true);
    }
}

} /* namespace extensions */
} /* namespace vpvl */
