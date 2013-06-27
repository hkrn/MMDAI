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

#include <tinyxml2.h>
#include <set>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>

using namespace tinyxml2;

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

    static inline const char *projectPrefix() {
        return "vpvm";
    }
    static inline const char *projectNamespaceURI() {
        return "https://github.com/hkrn/MMDAI/";
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
    static inline bool equalsConstant(const char *left, const char *const right) {
        return left && std::strncmp(left, right, std::strlen(right)) == 0;
    }
    static inline bool equalsToAttribute(const XMLAttribute *attribute, const char *const name) {
        return attribute && equalsConstant(attribute->Name(), name);
    }
    static inline bool equalsToElement(const XMLElement &element, const char *const name) {
        return equalsConstant(element.Name(), name);
    }
    static void splitString(const std::string &value, Array<std::string> &tokens) {
        const std::string &delimiter = ",";
        std::string item(value);
        for (vsize pos = item.find(delimiter);
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
        float v = 0;
        for (int i = 0; i < 4; i++) {
            XMLUtil::ToFloat(tokens.at(offset + i).c_str(), &v);
            value[i] = v;
        }
    }
    static bool tryCreateVector3(const Array<std::string> &tokens, Vector3 &value) {
        if (tokens.count() == 3) {
            float v = 0;
            for (int i = 0; i < 3; i++) {
                XMLUtil::ToFloat(tokens.at(i).c_str(), &v);
                value[i] = v;
            }
            return true;
        }
        return false;
    }
    static bool tryCreateVector4(const Array<std::string> &tokens, Vector4 &value) {
        if (tokens.count() == 4) {
            float v = 0;
            for (int i = 0; i < 4; i++) {
                XMLUtil::ToFloat(tokens.at(i).c_str(), &v);
                value[i] = v;
            }
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
    }
    ~PrivateContext() {
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

    bool isUUIDDuplicated(const XMLProject::UUID &uuid, std::set<XMLProject::UUID> &set) const {
        if (set.find(uuid) != set.end()) {
            return true;
        }
        else {
            set.insert(uuid);
            return false;
        }
    }
    bool checkDuplicateUUID() const {
        std::set<XMLProject::UUID> set;
        for (ModelMap::const_iterator it = assetRefs.begin(); it != assetRefs.end(); it++) {
            if (isUUIDDuplicated(it->first, set)) {
                return false;
            }
        }
        for (ModelMap::const_iterator it = modelRefs.begin(); it != modelRefs.end(); it++) {
            if (isUUIDDuplicated(it->first, set)) {
                return false;
            }
        }
        for (MotionMap::const_iterator it = motionRefs.begin(); it != motionRefs.end(); it++) {
            if (isUUIDDuplicated(it->first, set)) {
                return false;
            }
        }
        return true;
    }
    void pushState(State s) {
        state = s;
        depth++;
    }
    void popState(State s) {
        state = s;
        depth--;
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

    bool writeXml(XMLPrinter &printer) const {
        char buffer[kElementContentBufferSize];
        printer.PushDeclaration("xml version=\"1.0\" encoding=\"UTF-8\"");
        printer.OpenElement("vpvm:project");
        internal::snprintf(buffer, sizeof(buffer), "%.1f", XMLProject::formatVersion());
        printer.PushAttribute("version", buffer);
        printer.PushAttribute("xmlns:vpvm", projectNamespaceURI());
        if (!writeSettings(printer)) {
            return false;
        }
        if (!writeModels(printer)) {
            return false;
        }
        if (!writeAssets(printer)) {
            return false;
        }
        printer.OpenElement("vpvm:motions");
        for (MotionMap::const_iterator it = motionRefs.begin(); it != motionRefs.end(); it++) {
            const std::string &motionUUID = it->first;
            if (IMotion *motionPtr = it->second) {
                IMotion::Type motionType = motionPtr->type();
                if (motionType == IMotion::kVMDMotion) {
                    const vmd::Motion *motion = static_cast<const vmd::Motion *>(motionPtr);
                    printer.OpenElement("vpvm:motion");
                    printer.PushAttribute("type", "vmd");
                    const std::string &modelUUID = findModelUUID(motion->parentModelRef());
                    if (modelUUID != XMLProject::kNullUUID) {
                        printer.PushAttribute("model", modelUUID.c_str());
                    }
                    printer.PushAttribute("uuid", motionUUID.c_str());
                    if (!writeVMDBoneKeyframes(motion, printer)) {
                        return false;
                    }
                    if (!writeVMDMorphKeyframes(motion, printer)) {
                        return false;
                    }
                    if (!writeVMDCameraKeyframes(motion, printer)) {
                        return false;
                    }
                    if (!writeVMDLightKeyframes(motion, printer)) {
                        return false;
                    }
                    printer.CloseElement(); /* vpvm:motion */
                }
                else if (motionType == IMotion::kMVDMotion) {
                    const mvd::Motion *motion = static_cast<const mvd::Motion *>(motionPtr);
                    printer.OpenElement("vpvm:motion");
                    printer.PushAttribute("type", "mvd");
                    const std::string &modelUUID = findModelUUID(motion->parentModelRef());
                    if (modelUUID != XMLProject::kNullUUID) {
                        printer.PushAttribute("model", modelUUID.c_str());
                    }
                    printer.PushAttribute("uuid", motionUUID.c_str());
                    if (!writeMVDBoneKeyframes(motion, printer)) {
                        return false;
                    }
                    if (!writeMVDMorphKeyframes(motion, printer)) {
                        return false;
                    }
                    if (!writeMVDCameraKeyframes(motion, printer)) {
                        return false;
                    }
                    if (!writeMVDLightKeyframes(motion, printer)) {
                        return false;
                    }
                    if (!writeMVDEffectKeyframes(motion, printer)) {
                        return false;
                    }
                    if (!writeMVDProjectKeyframes(motion, printer)) {
                        return false;
                    }
                    if (!writeMVDModelKeyframes(motion, printer)) {
                        return false;
                    }
                    printer.CloseElement(); /* vpvm:motion */
                }
            }
        }
        printer.CloseElement(); /* vpvm:motions */
        printer.CloseElement(); /* vpvm:project */
        return true;
    }
    bool writeSettings(XMLPrinter &printer) const {
        printer.OpenElement("vpvm:settings");
        if(!writeStringMap(globalSettings, printer)) {
            return false;
        }
        printer.CloseElement(); /* vpvm:setting */
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
        newModelSettings["state.edge.offset"] = XMLProject::toStringFromFloat32(float32(model->edgeWidth()));
        newModelSettings["state.parent.model"] = findModelUUID(model->parentModelRef());
        newModelSettings["state.parent.bone"] = parentBoneRef ? delegateRef->toStdFromString(parentBoneRef->name()) : "";
    }
    bool writeModels(XMLPrinter &printer) const {
        StringMap newModelSettings;
        printer.OpenElement("vpvm:models");
        for (ModelMap::const_iterator it = modelRefs.begin(); it != modelRefs.end(); it++) {
            const XMLProject::UUID &uuid = it->first;
            const IModel *model = it->second;
            printer.OpenElement("vpvm:model");
            printer.PushAttribute("uuid", uuid.c_str());
            ModelSettings::const_iterator it2 = localModelSettings.find(uuid);
            if (it2 != localModelSettings.end()) {
                getNewModelSettings(model, it2->second, newModelSettings);
                if(!writeStringMap(newModelSettings, printer)) {
                    return false;
                }
            }
            printer.CloseElement(); /* vpvm:model */
        }
        printer.CloseElement(); /* vpvm:models */
        return true;
    }
    bool writeAssets(XMLPrinter &printer) const {
        StringMap newAssetSettings;
        printer.OpenElement("vpvm:assets");
        for (ModelMap::const_iterator it = assetRefs.begin(); it != assetRefs.end(); it++) {
            const XMLProject::UUID &uuid = it->first;
            const IModel *asset = it->second;
            printer.OpenElement("vpvm:asset");
            printer.PushAttribute("uuid", uuid.c_str());
            ModelSettings::const_iterator it2 = localAssetSettings.find(uuid);
            if (it2 != localAssetSettings.end()) {
                getNewModelSettings(asset, it2->second, newAssetSettings);
                if(!writeStringMap(newAssetSettings, printer)) {
                    return false;
                }
            }
            printer.CloseElement(); /* vpvm:asset */
        }
        printer.CloseElement(); /* vpvm:asset */
        return true;
    }
    bool writeVMDBoneKeyframes(const vmd::Motion *motion, XMLPrinter &printer) const {
        Quaternion ix, iy, iz, ir;
        char buffer[kElementContentBufferSize];
        printer.OpenElement("vpvm:animation");
        printer.PushAttribute("type", "bone");
        const vmd::BoneAnimation &ba = motion->boneAnimation();
        int nkeyframes = ba.countKeyframes();
        for (int i = 0; i < nkeyframes; i++) {
            const vmd::BoneKeyframe *keyframe = static_cast<const vmd::BoneKeyframe *>(ba.findKeyframeAt(i));
            const std::string &name = delegateRef->toStdFromString(keyframe->name());
            printer.OpenElement("vpvm:keyframe");
            printer.PushAttribute("name", name.c_str());
            printer.PushAttribute("index", unsigned(keyframe->timeIndex()));
            const Vector3 &position = keyframe->localTranslation();
            internal::snprintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f", position.x(), position.y(), -position.z());
            printer.PushAttribute("position", buffer);
            const Quaternion &rotation = keyframe->localRotation();
            internal::snprintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f,%.8f",
                               -rotation.x(), -rotation.y(), rotation.z(), rotation.w());
            printer.PushAttribute("rotation", buffer);
            printer.PushAttribute("ik", keyframe->isIKEnabled());
            keyframe->getInterpolationParameter(IBoneKeyframe::kBonePositionX, ix);
            keyframe->getInterpolationParameter(IBoneKeyframe::kBonePositionY, iy);
            keyframe->getInterpolationParameter(IBoneKeyframe::kBonePositionZ, iz);
            keyframe->getInterpolationParameter(IBoneKeyframe::kBoneRotation, ir);
            internal::snprintf(buffer, sizeof(buffer),
                               "%.f,%.f,%.f,%.f,"
                               "%.f,%.f,%.f,%.f,"
                               "%.f,%.f,%.f,%.f,"
                               "%.f,%.f,%.f,%.f"
                               , ix.x(), ix.y(), ix.z(), ix.w()
                               , iy.x(), iy.y(), iy.z(), iy.w()
                               , iz.x(), iz.y(), iz.z(), iz.w()
                               , ir.x(), ir.y(), ir.z(), ir.w()
                               );
            printer.PushAttribute("interpolation", buffer);
            printer.CloseElement(); /* vpvm:keyframe */
        }
        printer.CloseElement(); /* vpvm:animation */
        return true;
    }
    bool writeVMDCameraKeyframes(const vmd::Motion *motion, XMLPrinter &printer) const {
        Quaternion ix, iy, iz, ir, ifv, idt;
        char buffer[kElementContentBufferSize];
        printer.OpenElement("vpvm:animation");
        printer.PushAttribute("type", "camera");
        const vmd::CameraAnimation &ca = motion->cameraAnimation();
        int nkeyframes = ca.countKeyframes();
        for (int i = 0; i < nkeyframes; i++) {
            const vmd::CameraKeyframe *keyframe = static_cast<const vmd::CameraKeyframe *>(ca.findKeyframeAt(i));
            printer.OpenElement("vpvm:keyframe");
            printer.PushAttribute("index", unsigned(keyframe->timeIndex()));
            const Vector3 &position = keyframe->lookAt();
            internal::snprintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f", position.x(), position.y(), -position.z());
            printer.PushAttribute("position", buffer);
            const Vector3 &angle = keyframe->angle();
            internal::snprintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f",
                               btRadians(-angle.x()), btRadians(-angle.y()), btRadians(-angle.z()));
            printer.PushAttribute("angle", buffer);
            printer.PushAttribute("fovy", keyframe->fov());
            printer.PushAttribute("distance", keyframe->distance());
            keyframe->getInterpolationParameter(ICameraKeyframe::kCameraLookAtX, ix);
            keyframe->getInterpolationParameter(ICameraKeyframe::kCameraLookAtY, iy);
            keyframe->getInterpolationParameter(ICameraKeyframe::kCameraLookAtZ, iz);
            keyframe->getInterpolationParameter(ICameraKeyframe::kCameraAngle, ir);
            keyframe->getInterpolationParameter(ICameraKeyframe::kCameraFov, ifv);
            keyframe->getInterpolationParameter(ICameraKeyframe::kCameraDistance, idt);
            internal::snprintf(buffer, sizeof(buffer),
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
            printer.PushAttribute("interpolation", buffer);
            printer.CloseElement(); /* vpvm:keyframe */
        }
        printer.CloseElement(); /* vpvm:animation */
        return true;
    }
    bool writeVMDLightKeyframes(const vmd::Motion *motion, XMLPrinter &printer) const {
        char buffer[kElementContentBufferSize];
        printer.OpenElement("vpvm:animation");
        printer.PushAttribute("type", "light");
        const vmd::LightAnimation &la = motion->lightAnimation();
        int nkeyframes = la.countKeyframes();
        for (int i = 0; i < nkeyframes; i++) {
            const vmd::LightKeyframe *keyframe = static_cast<vmd::LightKeyframe *>(la.findKeyframeAt(i));
            printer.OpenElement("vpvm:keyframe");
            printer.PushAttribute("index", unsigned(keyframe->timeIndex()));
            const Vector3 &color = keyframe->color();
            internal::snprintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f", color.x(), color.y(), color.z());
            printer.PushAttribute("color", buffer);
            const Vector3 &direction = keyframe->direction();
            internal::snprintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f", direction.x(), direction.y(), direction.z());
            printer.PushAttribute("direction", buffer);
            printer.CloseElement(); /* vpvm:keyframe */
        }
        printer.CloseElement(); /* vpvm:animation */
        return true;
    }
    bool writeVMDMorphKeyframes(const vmd::Motion *motion, XMLPrinter &printer) const {
        printer.OpenElement("vpvm:animation");
        printer.PushAttribute("type", "morph");
        const vmd::MorphAnimation &fa = motion->morphAnimation();
        int nkeyframes = fa.countKeyframes();
        for (int i = 0; i < nkeyframes; i++) {
            const vmd::MorphKeyframe *keyframe = static_cast<vmd::MorphKeyframe *>(fa.findKeyframeAt(i));
            const std::string &name = delegateRef->toStdFromString(keyframe->name());
            printer.OpenElement("vpvm:keyframe");
            printer.PushAttribute("name", name.c_str());
            printer.PushAttribute("index", unsigned(keyframe->timeIndex()));
            printer.PushAttribute("weight", keyframe->weight());
            printer.CloseElement(); /* vpvm:keyframe */
        }
        printer.CloseElement(); /* animation */
        return true;
    }
    bool writeMVDBoneKeyframes(const mvd::Motion *motion, XMLPrinter &printer) const {
        Quaternion ix, iy, iz, ir;
        char buffer[kElementContentBufferSize];
        printer.OpenElement("vpvm:animation");
        printer.PushAttribute("type", "bone");
        int nkeyframes = motion->countKeyframes(IKeyframe::kBoneKeyframe);
        for (int i = 0; i < nkeyframes; i++) {
            const mvd::BoneKeyframe *keyframe = static_cast<const mvd::BoneKeyframe *>(motion->findBoneKeyframeRefAt(i));
            const std::string &name = delegateRef->toStdFromString(keyframe->name());
            printer.OpenElement("vpvm:keyframe");
            printer.PushAttribute("name", name.c_str());
            printer.PushAttribute("index", unsigned(keyframe->timeIndex()));
            printer.PushAttribute("layer", keyframe->layerIndex());
            const Vector3 &position = keyframe->localTranslation();
            internal::snprintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f", position.x(), position.y(), -position.z());
            printer.PushAttribute("position", buffer);
            const Quaternion &rotation = keyframe->localRotation();
            internal::snprintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f,%.8f",
                               -rotation.x(), -rotation.y(), rotation.z(), rotation.w());
            printer.PushAttribute("rotation", buffer);
            keyframe->getInterpolationParameter(IBoneKeyframe::kBonePositionX, ix);
            keyframe->getInterpolationParameter(IBoneKeyframe::kBonePositionY, iy);
            keyframe->getInterpolationParameter(IBoneKeyframe::kBonePositionZ, iz);
            keyframe->getInterpolationParameter(IBoneKeyframe::kBoneRotation, ir);
            internal::snprintf(buffer, sizeof(buffer),
                               "%.f,%.f,%.f,%.f,"
                               "%.f,%.f,%.f,%.f,"
                               "%.f,%.f,%.f,%.f,"
                               "%.f,%.f,%.f,%.f"
                               , ix.x(), ix.y(), ix.z(), ix.w()
                               , iy.x(), iy.y(), iy.z(), iy.w()
                               , iz.x(), iz.y(), iz.z(), iz.w()
                               , ir.x(), ir.y(), ir.z(), ir.w()
                               );
            printer.PushAttribute("interpolation", buffer);
            printer.CloseElement(); /* vpvm:keyframe */
        }
        printer.CloseElement(); /* vpvm:animation */
        return true;
    }
    bool writeMVDCameraKeyframes(const mvd::Motion *motion, XMLPrinter &printer) const {
        Quaternion ix, ir, ifv, idt;
        char buffer[kElementContentBufferSize];
        printer.OpenElement("vpvm:animation");
        printer.PushAttribute("type", "camera");
        int nkeyframes = motion->countKeyframes(IKeyframe::kCameraKeyframe);
        for (int i = 0; i < nkeyframes; i++) {
            const mvd::CameraKeyframe *keyframe = static_cast<const mvd::CameraKeyframe *>(motion->findCameraKeyframeRefAt(i));
            printer.OpenElement("vpvm:keyframe");
            printer.PushAttribute("index", unsigned(keyframe->timeIndex()));
            printer.PushAttribute("layer", keyframe->layerIndex());
            const Vector3 &position = keyframe->lookAt();
            internal::snprintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f", position.x(), position.y(), -position.z());
            printer.PushAttribute("position", buffer);
            const Vector3 &angle = keyframe->angle();
            internal::snprintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f",
                               btRadians(-angle.x()), btRadians(-angle.y()), btRadians(-angle.z()));
            printer.PushAttribute("angle", buffer);
            printer.PushAttribute("fovy", keyframe->fov());
            printer.PushAttribute("distance", keyframe->distance());
            keyframe->getInterpolationParameter(ICameraKeyframe::kCameraLookAtX, ix);
            keyframe->getInterpolationParameter(ICameraKeyframe::kCameraAngle, ir);
            keyframe->getInterpolationParameter(ICameraKeyframe::kCameraFov, ifv);
            keyframe->getInterpolationParameter(ICameraKeyframe::kCameraDistance, idt);
            internal::snprintf(buffer, sizeof(buffer),
                               "%.f,%.f,%.f,%.f,"
                               "%.f,%.f,%.f,%.f,"
                               "%.f,%.f,%.f,%.f,"
                               "%.f,%.f,%.f,%.f"
                               , ix.x(), ix.y(), ix.z(), ix.w()
                               , ir.x(), ir.y(), ir.z(), ir.w()
                               , idt.x(), idt.y(), idt.z(), idt.w()
                               , ifv.x(), ifv.y(), ifv.z(), ifv.w()
                               );
            printer.PushAttribute("interpolation", buffer);
            printer.CloseElement(); /* vpvm:keyframe */
        }
        printer.CloseElement(); /* vpvm:animation */
        return true;
    }
    bool writeMVDEffectKeyframes(const mvd::Motion *motion, XMLPrinter &printer) const {
        printer.OpenElement("vpvm:animation");
        printer.PushAttribute("type", "effect");
        int nkeyframes = motion->countKeyframes(IKeyframe::kEffectKeyframe);
        for (int i = 0; i < nkeyframes; i++) {
            const mvd::EffectKeyframe *keyframe = static_cast<const mvd::EffectKeyframe *>(motion->findEffectKeyframeRefAt(i));
            printer.OpenElement("vpvm:keyframe");
            printer.PushAttribute("index", unsigned(keyframe->timeIndex()));
            printer.PushAttribute("visible", keyframe->isVisible());
            printer.PushAttribute("addblend", keyframe->isAddBlendEnabled());
            printer.PushAttribute("shadow", keyframe->isShadowEnabled());
            printer.PushAttribute("scale", keyframe->scaleFactor());
            printer.PushAttribute("opacity", keyframe->opacity());
            printer.CloseElement(); /* vpvm:keyframe */
        }
        printer.CloseElement(); /* vpvm:animation */
        return true;
    }
    bool writeMVDLightKeyframes(const mvd::Motion *motion, XMLPrinter &printer) const {
        char buffer[kElementContentBufferSize];
        printer.OpenElement("vpvm:animation");
        printer.PushAttribute("type", "light");
        int nkeyframes = motion->countKeyframes(IKeyframe::kLightKeyframe);
        for (int i = 0; i < nkeyframes; i++) {
            const mvd::LightKeyframe *keyframe = static_cast<const mvd::LightKeyframe *>(motion->findLightKeyframeRefAt(i));
            printer.OpenElement("vpvm:keyframe");
            printer.PushAttribute("index", unsigned(keyframe->timeIndex()));
            const Vector3 &color = keyframe->color();
            internal::snprintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f", color.x(), color.y(), color.z());
            printer.PushAttribute("color", buffer);
            const Vector3 &direction = keyframe->direction();
            internal::snprintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f", direction.x(), direction.y(), direction.z());
            printer.PushAttribute("direction", buffer);
            printer.CloseElement(); /* vpvm:keyframe */
        }
        printer.CloseElement(); /* vpvm:animation */
        return true;
    }
    bool writeMVDModelKeyframes(const mvd::Motion *motion, XMLPrinter &printer) const {
        char buffer[kElementContentBufferSize];
        printer.OpenElement("vpvm:animation");
        printer.PushAttribute("type", "model");
        int nkeyframes = motion->countKeyframes(IKeyframe::kModelKeyframe);
        for (int i = 0; i < nkeyframes; i++) {
            const mvd::ModelKeyframe *keyframe = static_cast<const mvd::ModelKeyframe *>(motion->findModelKeyframeRefAt(i));
            printer.OpenElement("vpvm:keyframe");
            printer.PushAttribute("index", unsigned(keyframe->timeIndex()));
            printer.PushAttribute("visible", keyframe->isVisible());
            printer.PushAttribute("addblend", keyframe->isAddBlendEnabled());
            printer.PushAttribute("physics.enable", keyframe->isPhysicsEnabled());
            printer.PushAttribute("physics.mode", keyframe->physicsStillMode());
            printer.PushAttribute("edge.width", keyframe->edgeWidth());
            const Color &edgeColor = keyframe->edgeColor();
            internal::snprintf(buffer, sizeof(buffer), "%.4f,%.4f,%.4f,%.4f", edgeColor.x(), edgeColor.y(), edgeColor.z(), edgeColor.w());
            printer.PushAttribute("edge.color", buffer);
            // TODO: implement writing IK state
            printer.CloseElement(); /* vpvm:keyframe */
        }
        printer.CloseElement(); /* vpvm:animation */
        return true;
    }
    bool writeMVDMorphKeyframes(const mvd::Motion *motion, XMLPrinter &printer) const {
        printer.OpenElement("vpvm:animation");
        printer.PushAttribute("type", "morph");
        int nkeyframes = motion->countKeyframes(IKeyframe::kMorphKeyframe);
        for (int i = 0; i < nkeyframes; i++) {
            const mvd::MorphKeyframe *keyframe = static_cast<const mvd::MorphKeyframe *>(motion->findMorphKeyframeRefAt(i));
            const std::string &name = delegateRef->toStdFromString(keyframe->name());
            printer.OpenElement("vpvm:keyframe");
            printer.PushAttribute("name", name.c_str());
            printer.PushAttribute("index", unsigned(keyframe->timeIndex()));
            printer.PushAttribute("weight", keyframe->weight());
            printer.CloseElement(); /* vpvm:keyframe */
        }
        printer.CloseElement(); /* vpvm:animation */
        return true;
    }
    bool writeMVDProjectKeyframes(const mvd::Motion *motion, XMLPrinter &printer) const {
        char buffer[kElementContentBufferSize];
        printer.OpenElement("vpvm:animation");
        printer.PushAttribute("type", "project");
        int nkeyframes = motion->countKeyframes(IKeyframe::kProjectKeyframe);
        for (int i = 0; i < nkeyframes; i++) {
            const mvd::ProjectKeyframe *keyframe = static_cast<const mvd::ProjectKeyframe *>(motion->findProjectKeyframeRefAt(i));
            printer.OpenElement("vpvm:keyframe");
            printer.PushAttribute("index", unsigned(keyframe->timeIndex()));
            printer.PushAttribute("gravity.factor", keyframe->gravityFactor());
            const Vector3 &direction = keyframe->gravityDirection();
            internal::snprintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f", direction.x(), direction.y(), direction.z());
            printer.PushAttribute("gravity.direction", buffer);
            printer.PushAttribute("shadow.mode", keyframe->shadowMode());
            printer.PushAttribute("shadow.depth", keyframe->shadowDepth());
            printer.PushAttribute("shadow.distance", keyframe->shadowDistance());
            printer.CloseElement();
        }
        printer.CloseElement();
        return true;
    }
    bool writeStringMap(const StringMap &map, XMLPrinter &printer) const {
        for (StringMap::const_iterator it = map.begin(); it != map.end(); it++) {
            if (it->first.empty() || it->second.empty()) {
                continue;
            }
            printer.OpenElement("vpvm:value");
            printer.PushAttribute("name", it->first.c_str());
            printer.PushText(it->second.c_str(), true);
            printer.CloseElement();
        }
        return true;
    }

    bool visitReadEnter(const XMLElement &element, const XMLAttribute *firstAttribute) {
        if (depth == 0 && equalsToElement(element, "vpvm:project")) {
            readVersion(firstAttribute);
        }
        else if (depth == 1 && state == kProject) {
            if (equalsToElement(element, "vpvm:settings")) {
                pushState(kSettings);
            }
            else if (equalsToElement(element, "vpvm:physics")) {
                pushState(kPhysics);
            }
            else if (equalsToElement(element, "vpvm:models")) {
                pushState(kModels);
            }
            else if (equalsToElement(element, "vpvm:assets")) {
                pushState(kAssets);
            }
            else if (equalsToElement(element, "vpvm:motions")) {
                pushState(kMotions);
            }
        }
        else if (depth == 2) {
            if (state == kSettings && equalsToElement(element, "vpvm:value")) {
                readGlobalSettingKey(firstAttribute);
            }
            if (state == kModels && equalsToElement(element, "vpvm:model")) {
                readModel(firstAttribute);
            }
            else if (state == kAssets && equalsToElement(element, "vpvm:asset")) {
                readAsset(firstAttribute);
            }
            else if (state == kMotions && equalsToElement(element, "vpvm:motion")) {
                readMotion(firstAttribute);
            }
        }
        else if (depth == 3) {
            if ((state == kModel || state == kAsset) && equalsToElement(element, "vpvm:value")) {
                readLocalSettingKey(firstAttribute);
            }
            else if (state == kAnimation && equalsToElement(element, "vpvm:animation")) {
                readMotionType(firstAttribute);
            }
            else if (equalsToElement(element, "vpvm:keyframe")) {
#if 0
                // currently do nothing
                switch (state) {
                case kAssetMotion:
                    break;
                }
#endif
            }
        }
        else if (depth == 4 && equalsToElement(element, "vpvm:keyframe")) {
            switch (state) {
            case kVMDBoneMotion:
                readVMDBoneKeyframe(firstAttribute);
                break;
            case kVMDMorphMotion:
                readVMDMorphKeyframe(firstAttribute);
                break;
            case kVMDCameraMotion:
                readVMDCameraKeyframe(firstAttribute);
                break;
            case kVMDLightMotion:
                readVMDLightKeyframe(firstAttribute);
                break;
            case kMVDAssetMotion:
                readMVDAssetKeyframe(firstAttribute);
                break;
            case kMVDBoneMotion:
                readMVDBoneKeyframe(firstAttribute);
                break;
            case kMVDCameraMotion:
                readMVDCameraKeyframe(firstAttribute);
                break;
            case kMVDEffectMotion:
                readMVDEffectKeyframe(firstAttribute);
                break;
            case kMVDLightMotion:
                readMVDLightKeyframe(firstAttribute);
                break;
            case kMVDModelMotion:
                readMVDModelKeyframe(firstAttribute);
                break;
            case kMVDMorphMotion:
                readMVDMorphKeyframe(firstAttribute);
                break;
            case kMVDProjectMotion:
                readMVDProjectKeyframe(firstAttribute);
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
        else {
            return false;
        }
        return true;
    }
    bool visitRead(const XMLText &text) {
        if (state == kSettings) {
            globalSettings[settingKey].assign(text.Value());
        }
        else if (state == kModel) {
            localModelSettings[uuid][settingKey].assign(text.Value());
        }
        else if (state == kAsset) {
            localAssetSettings[uuid][settingKey].assign(text.Value());
        }
        else {
            return false;
        }
        return true;
    }
    bool visitReadExit(const XMLElement &element) {
        if (depth == 4) {
            if (!equalsToElement(element, "vpvm:keyframe")) {
                popState(kAnimation);
            }
            /* else { conitnue reading keyframes of current motion } */
        }
        else if (depth == 3) {
            switch (state) {
            case kAsset:
                if (equalsToElement(element, "vpvm:asset")) {
                    addAsset();
                }
                settingKey.clear();
                break;
            case kModel:
                if (equalsToElement(element, "vpvm:model")) {
                    addModel();
                }
                settingKey.clear();
                break;
            case kAnimation:
                if (equalsToElement(element, "vpvm:motion")) {
                    addMotion();
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
        else if (depth == 2) {
            switch (state) {
            case kAssets:
                if (equalsToElement(element, "vpvm:assets")) {
                    popState(kProject);
                }
                break;
            case kModels:
                if (equalsToElement(element, "vpvm:models")) {
                    popState(kProject);
                }
                break;
            case kMotions:
                if (equalsToElement(element, "vpvm:motions")) {
                    popState(kProject);
                }
                break;
            case kSettings:
                if (equalsToElement(element, "vpvm:settings")) {
                    popState(kProject);
                }
                settingKey.clear();
                break;
            case kPhysics:
                if (equalsToElement(element, "vpvm:physics")) {
                    popState(kProject);
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
        else if (depth == 1 && state == kProject && equalsToElement(element, "vpvm:project")) {
            depth--;
        }
        else {
            return false;
        }
        return true;
    }

    void readVersion(const XMLAttribute *firstAttribute) {
        for (const XMLAttribute *attr = firstAttribute; attr; attr = attr->Next()) {
            if (equalsToAttribute(attr, "version")) {
                version.assign(attr->Value());
            }
        }
        pushState(kProject);
    }
    void readGlobalSettingKey(const XMLAttribute *firstAttribute) {
        for (const XMLAttribute *attr = firstAttribute; attr; attr = attr->Next()) {
            if (equalsToAttribute(attr, "name")) {
                settingKey.assign(attr->Value());
            }
        }
    }
    void readModel(const XMLAttribute *firstAttribute) {
        for (const XMLAttribute *attr = firstAttribute; attr; attr = attr->Next()) {
            if (equalsToAttribute(attr, "uuid")) {
                uuid.assign(attr->Value());
            }
        }
        pushState(kModel);
    }
    void readAsset(const XMLAttribute *firstAttribute) {
        for (const XMLAttribute *attr = firstAttribute; attr; attr = attr->Next()) {
            if (equalsToAttribute(attr, "uuid")) {
                uuid.assign(attr->Value());
            }
        }
        pushState(kAsset);
    }
    void readMotion(const XMLAttribute *firstAttribute) {
        currentMotionType = IMotion::kVMDMotion;
        for (const XMLAttribute *attr = firstAttribute; attr; attr = attr->Next()) {
            if (equalsToAttribute(attr, "uuid")) {
                uuid.assign(attr->Value());
            }
            else if (equalsToAttribute(attr, "model")) {
                parentModel.assign(attr->Value());
            }
            else if (equalsToAttribute(attr, "type") && equalsConstant(attr->Value(), "mvd")) {
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
    void readLocalSettingKey(const XMLAttribute *firstAttribute) {
        readGlobalSettingKey(firstAttribute);
    }
    void readMotionType(const XMLAttribute *firstAttribute) {
        bool isMVD = currentMotionType == IMotion::kMVDMotion;
        for (const XMLAttribute *attr = firstAttribute; attr; attr = attr->Next()) {
            if (!equalsToAttribute(attr, "type")) {
                continue;
            }
            const char *value = attr->Value();
            if (isMVD) {
                if (equalsConstant(value, "bone")) {
                    pushState(kMVDBoneMotion);
                }
                else if (equalsConstant(value, "asset")) {
                    pushState(kMVDAssetMotion);
                }
                else if (equalsConstant(value, "model")) {
                    pushState(kMVDModelMotion);
                }
                else if (equalsConstant(value, "light")) {
                    pushState(kMVDLightMotion);
                }
                else if (equalsConstant(value, "morph")) {
                    pushState(kMVDMorphMotion);
                }
                else if (equalsConstant(value, "camera")) {
                    pushState(kMVDCameraMotion);
                }
                else if (equalsConstant(value, "effect")) {
                    pushState(kMVDEffectMotion);
                }
                else if (equalsConstant(value, "project")) {
                    pushState(kMVDProjectMotion);
                }
            }
            else {
                if (equalsConstant(value, "bone")) {
                    pushState(kVMDBoneMotion);
                }
                else if (equalsConstant(value, "light")) {
                    pushState(kVMDLightMotion);
                }
                else if (equalsConstant(value, "morph")) {
                    pushState(kVMDMorphMotion);
                }
                else if (equalsConstant(value, "camera")) {
                    pushState(kVMDCameraMotion);
                }
            }
        }
    }
    void readVMDBoneKeyframe(const XMLAttribute *firstAttribute) {
        if (IBoneKeyframe *keyframe = factoryRef->createBoneKeyframe(currentMotion)) {
            Array<std::string> tokens;
            Vector4 vec4(kZeroV4);
            Vector3 vec3(kZeroV3);
            QuadWord qw(0, 0, 0, 0);
            keyframe->setDefaultInterpolationParameter();
            for (const XMLAttribute *attr = firstAttribute; attr; attr = attr->Next()) {
                if (equalsToAttribute(attr, "name")) {
                    delete currentString;
                    currentString = delegateRef->toStringFromStd(attr->Value());
                    keyframe->setName(currentString);
                }
                else if (equalsToAttribute(attr, "index")) {
                    keyframe->setTimeIndex(IKeyframe::TimeIndex(attr->DoubleValue()));
                }
                else if (equalsToAttribute(attr, "position")) {
                    splitString(attr->Value(), tokens);
                    if (tryCreateVector3(tokens, vec3)) {
#ifdef VPVL2_COORDINATE_OPENGL
                        vec3.setValue(vec3.x(), vec3.y(), -vec3.z());
#else
                        vec3.setValue(vec3.x(), vec3.y(), vec3.z());
#endif
                        keyframe->setLocalTranslation(vec3);
                    }
                }
                else if (equalsToAttribute(attr, "rotation")) {
                    splitString(attr->Value(), tokens);
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
                else if (equalsToAttribute(attr, "interpolation")) {
                    splitString(attr->Value(), tokens);
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
    void readVMDCameraKeyframe(const XMLAttribute *firstAttribute) {
        if (ICameraKeyframe *keyframe = factoryRef->createCameraKeyframe(currentMotion)) {
            Vector3 vec3(kZeroV3);
            QuadWord qw(0, 0, 0, 0);
            Array<std::string> tokens;
            keyframe->setDefaultInterpolationParameter();
            for (const XMLAttribute *attr = firstAttribute; attr; attr = attr->Next()) {
                if (equalsToAttribute(attr, "fovy")) {
                    keyframe->setFov(attr->FloatValue());
                }
                else if (equalsToAttribute(attr, "index")) {
                    keyframe->setTimeIndex(IKeyframe::TimeIndex(attr->DoubleValue()));
                }
                else if (equalsToAttribute(attr, "angle")) {
                    splitString(attr->Value(), tokens);
                    if (tryCreateVector3(tokens, vec3)) {
#ifdef VPVL2_COORDINATE_OPENGL
                        vec3.setValue(-btDegrees(vec3.x()), -btDegrees(vec3.y()), -btDegrees(vec3.z()));
#else
                        vec3.setValue(btDegrees(vec3.x()), btDegrees(vec3.y()), -btDegrees(vec3.z()));
#endif
                        keyframe->setAngle(vec3);
                    }
                }
                else if (equalsToAttribute(attr, "position")) {
                    splitString(attr->Value(), tokens);
                    if (tryCreateVector3(tokens, vec3)) {
#ifdef VPVL2_COORDINATE_OPENGL
                        vec3.setValue(vec3.x(), vec3.y(), -vec3.z());
#else
                        vec3.setValue(vec3.x(), vec3.y(), vec3.z());
#endif
                        keyframe->setLookAt(vec3);
                    }
                }
                else if (equalsToAttribute(attr, "distance")) {
                    keyframe->setDistance(attr->FloatValue());
                }
                else if (equalsToAttribute(attr, "interpolation")) {
                    splitString(attr->Value(), tokens);
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
    void readVMDLightKeyframe(const XMLAttribute *firstAttribute) {
        if (ILightKeyframe *keyframe = factoryRef->createLightKeyframe(currentMotion)) {
            Array<std::string> tokens;
            Vector3 vec3(kZeroV3);
            for (const XMLAttribute *attr = firstAttribute; attr; attr = attr->Next()) {
                if (equalsToAttribute(attr, "index")) {
                    keyframe->setTimeIndex(IKeyframe::TimeIndex(attr->DoubleValue()));
                }
                else if (equalsToAttribute(attr, "color")) {
                    splitString(attr->Value(), tokens);
                    if (tryCreateVector3(tokens, vec3)) {
                        keyframe->setColor(vec3);
                    }
                }
                else if (equalsToAttribute(attr, "direction")) {
                    splitString(attr->Value(), tokens);
                    if (tryCreateVector3(tokens, vec3)) {
                        keyframe->setDirection(vec3);
                    }
                }
            }
            currentMotion->addKeyframe(keyframe);
        }
    }
    void readVMDMorphKeyframe(const XMLAttribute *firstAttribute) {
        if (IMorphKeyframe *keyframe = factoryRef->createMorphKeyframe(currentMotion)) {
            for (const XMLAttribute *attr = firstAttribute; attr; attr = attr->Next()) {
                if (equalsToAttribute(attr, "name")) {
                    delete currentString;
                    currentString = delegateRef->toStringFromStd(attr->Value());
                    keyframe->setName(currentString);
                }
                else if (equalsToAttribute(attr, "index")) {
                    keyframe->setTimeIndex(IKeyframe::TimeIndex(attr->DoubleValue()));
                }
                else if (equalsToAttribute(attr, "weight")) {
                    keyframe->setWeight(IMorph::WeightPrecision(attr->DoubleValue()));
                }
            }
            currentMotion->addKeyframe(keyframe);
        }
    }
    void readMVDAssetKeyframe(const XMLAttribute *firstAttribute) {
        // FIXME: add createAssetKeyframe
        if (IMorphKeyframe *keyframe = factoryRef->createMorphKeyframe(currentMotion)) {
            for (const XMLAttribute *attr = firstAttribute; attr; attr = attr->Next()) {
            }
            currentMotion->addKeyframe(keyframe);
        }
    }
    void readMVDBoneKeyframe(const XMLAttribute *firstAttribute) {
        if (IBoneKeyframe *keyframe = factoryRef->createBoneKeyframe(currentMotion)) {
            Array<std::string> tokens;
            Vector4 vec4(kZeroV4);
            Vector3 vec3(kZeroV3);
            QuadWord qw(0, 0, 0, 0);
            for (const XMLAttribute *attr = firstAttribute; attr; attr = attr->Next()) {
                if (equalsToAttribute(attr, "name")) {
                    delete currentString;
                    currentString = delegateRef->toStringFromStd(attr->Value());
                    keyframe->setName(currentString);
                }
                else if (equalsToAttribute(attr, "index")) {
                    keyframe->setTimeIndex(IKeyframe::TimeIndex(attr->DoubleValue()));
                }
                else if (equalsToAttribute(attr, "layer")) {
                    keyframe->setLayerIndex(IKeyframe::LayerIndex(attr->IntValue()));
                }
                else if (equalsToAttribute(attr, "position")) {
                    splitString(attr->Value(), tokens);
                    if (tryCreateVector3(tokens, vec3)) {
#ifdef VPVL2_COORDINATE_OPENGL
                        vec3.setValue(vec3.x(), vec3.y(), -vec3.z());
#else
                        vec3.setValue(vec3.x(), vec3.y(), vec3.z());
#endif
                        keyframe->setLocalTranslation(vec3);
                    }
                }
                else if (equalsToAttribute(attr, "rotation")) {
                    splitString(attr->Value(), tokens);
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
                else if (equalsToAttribute(attr, "interpolation")) {
                    splitString(attr->Value(), tokens);
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
    void readMVDCameraKeyframe(const XMLAttribute *firstAttribute) {
        if (ICameraKeyframe *keyframe = factoryRef->createCameraKeyframe(currentMotion)) {
            Array<std::string> tokens;
            Vector3 vec3(kZeroV3);
            QuadWord qw(0, 0, 0, 0);
            for (const XMLAttribute *attr = firstAttribute; attr; attr = attr->Next()) {
                if (equalsToAttribute(attr, "fovy")) {
                    keyframe->setFov(attr->FloatValue());
                }
                else if (equalsToAttribute(attr, "index")) {
                    keyframe->setTimeIndex(IKeyframe::TimeIndex(attr->DoubleValue()));
                }
                else if (equalsToAttribute(attr, "layer")) {
                    keyframe->setLayerIndex(IKeyframe::LayerIndex(attr->IntValue()));
                }
                else if (equalsToAttribute(attr, "angle")) {
                    splitString(attr->Value(), tokens);
                    if (tryCreateVector3(tokens, vec3)) {
#ifdef VPVL2_COORDINATE_OPENGL
                        vec3.setValue(-btDegrees(vec3.x()), -btDegrees(vec3.y()), -btDegrees(vec3.z()));
#else
                        vec3.setValue(btDegrees(vec3.x()), btDegrees(vec3.y()), -btDegrees(vec3.z()));
#endif
                        keyframe->setAngle(vec3);
                    }
                }
                else if (equalsToAttribute(attr, "position")) {
                    splitString(attr->Value(), tokens);
                    if (tryCreateVector3(tokens, vec3)) {
#ifdef VPVL2_COORDINATE_OPENGL
                        vec3.setValue(vec3.x(), vec3.y(), -vec3.z());
#else
                        vec3.setValue(vec3.x(), vec3.y(), vec3.z());
#endif
                        keyframe->setLookAt(vec3);
                    }
                }
                else if (equalsToAttribute(attr, "distance")) {
                    keyframe->setDistance(attr->FloatValue());
                }
                else if (equalsToAttribute(attr, "interpolation")) {
                    splitString(attr->Value(), tokens);
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
    void readMVDEffectKeyframe(const XMLAttribute *firstAttribute) {
        if (IEffectKeyframe *keyframe = factoryRef->createEffectKeyframe(currentMotion)) {
            for (const XMLAttribute *attr = firstAttribute; attr; attr = attr->Next()) {
                if (equalsToAttribute(attr, "index")) {
                    keyframe->setTimeIndex(IKeyframe::TimeIndex(attr->DoubleValue()));
                }
                else if (equalsToAttribute(attr, "visible")) {
                    keyframe->setVisible(attr->BoolValue());
                }
                else if (equalsToAttribute(attr, "addblend")) {
                    keyframe->setAddBlendEnable(attr->BoolValue());
                }
                else if (equalsToAttribute(attr, "shadow")) {
                    keyframe->setShadowEnable(attr->BoolValue());
                }
                else if (equalsToAttribute(attr, "scale")) {
                    keyframe->setScaleFactor(attr->FloatValue());
                }
                else if (equalsToAttribute(attr, "opacity")) {
                    keyframe->setOpacity(attr->FloatValue());
                }
                else if (equalsToAttribute(attr, "model")) {
                }
                else if (equalsToAttribute(attr,  "bone")) {
                }
            }
            currentMotion->addKeyframe(keyframe);
        }
    }
    void readMVDLightKeyframe(const XMLAttribute *firstAttribute) {
        if (ILightKeyframe *keyframe = factoryRef->createLightKeyframe(currentMotion)) {
            Array<std::string> tokens;
            Vector3 vec3(kZeroV3);
            for (const XMLAttribute *attr = firstAttribute; attr; attr = attr->Next()) {
                if (equalsToAttribute(attr, "index")) {
                    keyframe->setTimeIndex(IKeyframe::TimeIndex(attr->DoubleValue()));
                }
                else if (equalsToAttribute(attr, "color")) {
                    splitString(attr->Value(), tokens);
                    if (tryCreateVector3(tokens, vec3)) {
                        keyframe->setColor(vec3);
                    }
                }
                else if (equalsToAttribute(attr, "direction")) {
                    splitString(attr->Value(), tokens);
                    if (tryCreateVector3(tokens, vec3)) {
                        keyframe->setDirection(vec3);
                    }
                }
            }
            currentMotion->addKeyframe(keyframe);
        }
    }
    void readMVDModelKeyframe(const XMLAttribute *firstAttribute) {
        if (IModelKeyframe *keyframe = factoryRef->createModelKeyframe(currentMotion)) {
            Array<std::string> tokens;
            Vector4 vec4(kZeroV4);
            for (const XMLAttribute *attr = firstAttribute; attr; attr = attr->Next()) {
                if (equalsToAttribute(attr, "index")) {
                    keyframe->setTimeIndex(IKeyframe::TimeIndex(attr->DoubleValue()));
                }
                else if (equalsToAttribute(attr, "visible")) {
                    keyframe->setVisible(attr->BoolValue());
                }
                else if (equalsToAttribute(attr, "addblend")) {
                    keyframe->setAddBlendEnable(attr->BoolValue());
                }
                else if (equalsToAttribute(attr, "shadow")) {
                    keyframe->setShadowEnable(attr->BoolValue());
                }
                else if (equalsToAttribute(attr, "physics.enable")) {
                    keyframe->setPhysicsEnable(attr->BoolValue());
                }
                else if (equalsToAttribute(attr, "physics.mode")) {
                    keyframe->setPhysicsStillMode(attr->IntValue());
                }
                else if (equalsToAttribute(attr, "edge.width")) {
                    keyframe->setEdgeWidth(IVertex::EdgeSizePrecision(attr->DoubleValue()));
                }
                else if (equalsToAttribute(attr, "edge.color")) {
                    splitString(attr->Value(), tokens);
                    if (tryCreateVector4(tokens, vec4)) {
                        keyframe->setEdgeColor(vec4);
                    }
                }
            }
            currentMotion->addKeyframe(keyframe);
        }
    }
    void readMVDMorphKeyframe(const XMLAttribute *firstAttribute) {
        if (IMorphKeyframe *keyframe = factoryRef->createMorphKeyframe(currentMotion)) {
            for (const XMLAttribute *attr = firstAttribute; attr; attr = attr->Next()) {
                if (equalsToAttribute(attr, "name")) {
                    delete currentString;
                    currentString = delegateRef->toStringFromStd(attr->Value());
                    keyframe->setName(currentString);
                }
                else if (equalsToAttribute(attr, "index")) {
                    keyframe->setTimeIndex(IKeyframe::TimeIndex(attr->DoubleValue()));
                }
                else if (equalsToAttribute(attr, "weight")) {
                    keyframe->setWeight(IMorph::WeightPrecision(attr->DoubleValue()));
                }
            }
            currentMotion->addKeyframe(keyframe);
        }
    }
    void readMVDProjectKeyframe(const XMLAttribute *firstAttribute) {
        if (IProjectKeyframe *keyframe = factoryRef->createProjectKeyframe(currentMotion)) {
            Array<std::string> tokens;
            Vector3 vec3(kZeroV3);
            for (const XMLAttribute *attr = firstAttribute; attr; attr = attr->Next()) {
                if (equalsToAttribute(attr, "index")) {
                    keyframe->setTimeIndex(IKeyframe::TimeIndex(attr->DoubleValue()));
                }
                else if (equalsToAttribute(attr, "gravity.factor")) {
                    keyframe->setGravityFactor(attr->FloatValue());
                }
                else if (equalsToAttribute(attr, "gravity.direction")) {
                    splitString(attr->Value(), tokens);
                    if (tryCreateVector3(tokens, vec3)) {
                        keyframe->setGravityDirection(vec3);
                    }
                }
                else if (equalsToAttribute(attr, "shadow.mode")) {
                    keyframe->setShadowMode(attr->IntValue());
                }
                else if (equalsToAttribute(attr, "shadow.depth")) {
                    keyframe->setShadowDepth(attr->FloatValue());
                }
                else if (equalsToAttribute(attr, "shadow.distance")) {
                    keyframe->setShadowDistance(attr->FloatValue());
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

    bool save(XMLPrinter &printer) {
        const ICamera *camera = sceneRef->camera();
        globalSettings["state.camera.angle"] = XMLProject::toStringFromVector3(camera->angle());
        globalSettings["state.camera.distance"] = XMLProject::toStringFromFloat32(camera->distance());
        globalSettings["state.camera.fov"] = XMLProject::toStringFromFloat32(camera->fov());
        globalSettings["state.camera.lookat"] = XMLProject::toStringFromVector3(camera->lookAt());
        const ILight *light = sceneRef->light();
        globalSettings["state.light.color"] = XMLProject::toStringFromVector3(light->color());
        globalSettings["state.light.direction"] = XMLProject::toStringFromVector3(light->direction());
        bool ret = writeXml(printer);
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
                    int index = 0;
                    XMLUtil::ToInt(value.c_str(), &index);
                    return index;
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
                    int index = 0;
                    XMLUtil::ToInt(value.c_str(), &index);
                    return index;
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

    struct Reader : XMLVisitor {
        Reader(PrivateContext *context)
            : contextRef(context)
        {
        }
        ~Reader() {
            contextRef = 0;
        }

        bool VisitEnter(const XMLElement &element, const XMLAttribute *firstAttribute) {
            return contextRef->visitReadEnter(element, firstAttribute);
        }
        bool Visit(const XMLText &text) {
            return contextRef->visitRead(text);
        }
        bool VisitExit(const XMLElement &element) {
            return contextRef->visitReadExit(element);
        }
        PrivateContext *contextRef;
    };

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

float32 XMLProject::formatVersion()
{
    return 2.1f;
}

bool XMLProject::isReservedSettingKey(const std::string &key)
{
    return key.find(kSettingNameKey) == 0 || key.find(kSettingURIKey) == 0 || key.find(kSettingOrderKey) == 0;
}

std::string XMLProject::toStringFromFloat32(float32 value)
{
    char buffer[16];
    internal::snprintf(buffer, sizeof(buffer), "%.5f", value);
    return reinterpret_cast<const char *>(buffer);
}

std::string XMLProject::toStringFromVector3(const Vector3 &value)
{
    char buffer[64];
    internal::snprintf(buffer, sizeof(buffer), "%.5f,%.5f,%.5f", value.x(), value.y(), value.z());
    return reinterpret_cast<const char *>(buffer);
}

std::string XMLProject::toStringFromVector4(const Vector4 &value)
{
    char buffer[80];
    internal::snprintf(buffer, sizeof(buffer), "%.5f,%.5f,%.5f,%.5f", value.x(), value.y(), value.z(), value.w());
    return reinterpret_cast<const char *>(buffer);
}

std::string XMLProject::toStringFromQuaternion(const Quaternion &value)
{
    char buffer[80];
    internal::snprintf(buffer, sizeof(buffer), "%.5f,%.5f,%.5f,%.5f", value.x(), value.y(), value.z(), value.w());
    return reinterpret_cast<const char *>(buffer);
}

int XMLProject::toIntFromString(const std::string &value)
{
    int ret = 0;
    ::sscanf(value.c_str(), "%i", &ret);
    return ret;
}

float32 XMLProject::toFloat32FromString(const std::string &value)
{
    float32 ret = 0;
    ::sscanf(value.c_str(), "%f", &ret);
    return ret;
}

Vector3 XMLProject::toVector3FromString(const std::string &value)
{
    Vector3 ret;
    float32 x = 0, y = 0, z = 0;
    ::sscanf(value.c_str(), "%f,%f,%f", &x, &y, &z);
    ret.setValue(x, y, z);
    return ret;
}

Vector4 XMLProject::toVector4FromString(const std::string &value)
{
    Vector4 ret;
    float32 x = 0, y = 0, z = 0, w = 0;
    ::sscanf(value.c_str(), "%f,%f,%f,%f", &x, &y, &z, &w);
    ret.setValue(x, y, z, w);
    return ret;
}

Quaternion XMLProject::toQuaternionFromString(const std::string &value)
{
    Quaternion ret;
    float32 x = 0, y = 0, z = 0, w = 0;
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
    tinyxml2::XMLDocument document;
    bool ret = false;
    if (document.LoadFile(path) == XML_NO_ERROR) {
        PrivateContext::Reader reader(m_context);
        ret = m_context->validate(document.Accept(&reader));
        if (ret) {
            m_context->sort();
            m_context->restoreStates();
        }
    }
    else {
        VPVL2_LOG(WARNING, "Cannot load project from file " << path << ": first=" << document.GetErrorStr1() << " second=" << document.GetErrorStr2());
    }
    return ret;
}

bool XMLProject::load(const uint8 *data, vsize size)
{
    tinyxml2::XMLDocument document;
    bool ret = false;
    if (document.Parse(reinterpret_cast<const char *>(data), size) == XML_NO_ERROR) {
        PrivateContext::Reader reader(m_context);
        ret = m_context->validate(document.Accept(&reader));
        if (ret) {
            m_context->sort();
            m_context->restoreStates();
        }
    }
    else {
        VPVL2_LOG(WARNING, "Cannot load project from memory: first=" << document.GetErrorStr1() << " second=" << document.GetErrorStr2());
    }
    return ret;
}

bool XMLProject::save(const char *path)
{
    if (FILE *fp = fopen(path, "w")) {
        XMLPrinter printer(fp);
        bool ret = m_context->save(printer);
        fclose(fp);
        return ret;
    }
    return false;
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
