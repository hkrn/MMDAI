/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2010-2012  hkrn                                    */
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

#include <libxml/xmlwriter.h>
#include <set>
#include <string>
#include <sstream>
#include <map>

#define VPVL_XML_RC(rc) { if (rc < 0) { fprintf(stderr, "Failed at %s:%d\n", __FILE__, __LINE__); return false; } }
#define VPVL_CAST_XC(str) reinterpret_cast<const xmlChar *>(str)

namespace vpvl
{

struct Handler {
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
        kAssetMotion,
        kAnimation,
        kBoneMotion,
        kMorphMotion,
        kCameraMotion,
        kLightMotion
    };
    typedef std::map<std::string, std::string> StringMap;
    const static int kAttributeBufferSize = 32;
    const static int kElementContentBufferSize = 128;
    const static std::string kEmpty;
    typedef std::map<Project::UUID, Asset *> AssetMap;
    typedef std::map<Project::UUID, PMDModel *> PMDModelMap;
    typedef std::map<Project::UUID, VMDMotion *> VMDMotionMap;

    Handler(Project::IDelegate *delegate)
        : delegate(delegate),
          currentAsset(0),
          currentModel(0),
          currentMotion(0),
          state(kInitial),
          depth(0)
    {
    }
    ~Handler() {
        state = kInitial;
        depth = 0;
        for (AssetMap::const_iterator it = assets.begin(); it != assets.end(); it++)
            delete (*it).second;
        assets.clear();
        for (PMDModelMap::const_iterator it = models.begin(); it != models.end(); it++)
            delete (*it).second;
        models.clear();
        for (VMDMotionMap::const_iterator it = motions.begin(); it != motions.end(); it++)
            delete (*it).second;
        motions.clear();
        delete currentAsset;
        currentAsset = 0;
        delete currentModel;
        currentModel = 0;
        delete currentMotion;
        currentMotion = 0;
    }

    bool isDuplicatedUUID(const Project::UUID &uuid, std::set<Project::UUID> &set) const {
        if (set.find(uuid) != set.end())
            return true;
        set.insert(uuid);
        return false;
    }
    bool checkDuplicateUUID() const {
        std::set<Project::UUID> set;
        for (AssetMap::const_iterator it = assets.begin(); it != assets.end(); it++) {
            if (isDuplicatedUUID((*it).first, set))
                return false;
        }
        for (PMDModelMap::const_iterator it = models.begin(); it != models.end(); it++) {
            if (isDuplicatedUUID((*it).first, set))
                return false;
        }
        for (VMDMotionMap::const_iterator it = motions.begin(); it != motions.end(); it++) {
            if (isDuplicatedUUID((*it).first, set))
                return false;
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
    Asset *findAsset(const Project::UUID &value) const {
        if (value == Project::kNullUUID)
            return 0;
        AssetMap::const_iterator it = assets.find(value);
        if (it != assets.end())
            return (*it).second;
        return 0;
    }
    const Project::UUID &findAssetUUID(const Asset *value) const {
        if (!value)
            return Project::kNullUUID;
        for (AssetMap::const_iterator it = assets.begin(); it != assets.end(); it++) {
            if ((*it).second == value)
                return (*it).first;
        }
        return Project::kNullUUID;
    }
    PMDModel *findModel(const Project::UUID &value) const {
        if (value == Project::kNullUUID)
            return 0;
        PMDModelMap::const_iterator it = models.find(value);
        if (it != models.end())
            return (*it).second;
        return 0;
    }
    const Project::UUID &findModelUUID(const PMDModel *value) const {
        if (!value)
            return Project::kNullUUID;
        for (PMDModelMap::const_iterator it = models.begin(); it != models.end(); it++) {
            if ((*it).second == value)
                return (*it).first;
        }
        return Project::kNullUUID;
    }
    VMDMotion *findMotion(const Project::UUID &value) const {
        if (value == Project::kNullUUID)
            return 0;
        VMDMotionMap::const_iterator it = motions.find(value);
        if (it != motions.end())
            return (*it).second;
        return 0;
    }
    const Project::UUID &findMotionUUID(const VMDMotion *value) const {
        if (!value)
            return Project::kNullUUID;
        for (VMDMotionMap::const_iterator it = motions.begin(); it != motions.end(); it++) {
            if ((*it).second == value)
                return (*it).first;
        }
        return Project::kNullUUID;
    }
    void removeAsset(const Asset *asset) {
        for (AssetMap::iterator it = assets.begin(); it != assets.end(); it++) {
            if ((*it).second == asset) {
                assets.erase(it);
                break;
            }
        }
    }
    void removeModel(const PMDModel *model) {
        for (PMDModelMap::iterator it = models.begin(); it != models.end(); it++) {
            if ((*it).second == model) {
                models.erase(it);
                break;
            }
        }
    }
    void removeMotion(const VMDMotion *motion) {
        for (VMDMotionMap::iterator it = motions.begin(); it != motions.end(); it++) {
            if ((*it).second == motion) {
                motions.erase(it);
                break;
            }
        }
    }
    bool writeStringMap(const xmlChar *prefix, const StringMap &map, xmlTextWriterPtr &writer) {
        for (StringMap::const_iterator it = map.begin(); it != map.end(); it++) {
            if (it->first.empty() || it->second.empty())
                continue;
            VPVL_XML_RC(xmlTextWriterStartElementNS(writer, prefix, VPVL_CAST_XC("value"), 0));
            VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("name"), VPVL_CAST_XC(it->first.c_str())));
            VPVL_XML_RC(xmlTextWriterWriteCDATA(writer, VPVL_CAST_XC(it->second.c_str())));
            VPVL_XML_RC(xmlTextWriterEndElement(writer)); /* vpvl:value */
        }
        return true;
    }
    const std::string modelName(PMDModel *model) const {
        return delegate->toUnicode(std::string(reinterpret_cast<const char *>(model->name())));
    }
    bool save(xmlTextWriterPtr &writer) {
        uint8_t buffer[kElementContentBufferSize];
        if (!writer)
            return false;
        static const xmlChar *kPrefix = reinterpret_cast<const xmlChar *>(projectPrefix());
        static const xmlChar *kNSURI = reinterpret_cast<const xmlChar *>(projectNamespaceURI());
        VPVL_XML_RC(xmlTextWriterSetIndent(writer, 1));
        VPVL_XML_RC(xmlTextWriterStartDocument(writer, 0, "UTF-8", 0));
        VPVL_XML_RC(xmlTextWriterStartElementNS(writer, kPrefix, VPVL_CAST_XC("project"), kNSURI));
        internal::snprintf(buffer, sizeof(buffer), "%s", libraryVersionString());
        VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("version"), VPVL_CAST_XC(buffer)));
        VPVL_XML_RC(xmlTextWriterStartElementNS(writer, kPrefix, VPVL_CAST_XC("settings"), 0));
        if(!writeStringMap(kPrefix, globalSettings, writer))
            return false;
        VPVL_XML_RC(xmlTextWriterEndElement(writer)); /* vpvl:setting */
        VPVL_XML_RC(xmlTextWriterStartElementNS(writer, kPrefix, VPVL_CAST_XC("models"), 0));
        for (PMDModelMap::const_iterator it = models.begin(); it != models.end(); it++) {
            const Project::UUID &uuid = (*it).first;
            const PMDModel *model = (*it).second;
            VPVL_XML_RC(xmlTextWriterStartElementNS(writer, kPrefix, VPVL_CAST_XC("model"), 0));
            VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("uuid"), VPVL_CAST_XC(uuid.c_str())));
            if(!writeStringMap(kPrefix, localModelSettings[model], writer))
                return false;
            VPVL_XML_RC(xmlTextWriterEndElement(writer)); /* vpvl:model */
        }
        VPVL_XML_RC(xmlTextWriterEndElement(writer)); /* vpvl:models */
        VPVL_XML_RC(xmlTextWriterStartElementNS(writer, kPrefix, VPVL_CAST_XC("assets"), 0));
        for (AssetMap::const_iterator it = assets.begin(); it != assets.end(); it++) {
            const Project::UUID &uuid = (*it).first;
            const Asset *asset = (*it).second;
            VPVL_XML_RC(xmlTextWriterStartElementNS(writer, kPrefix, VPVL_CAST_XC("asset"), 0));
            VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("uuid"), VPVL_CAST_XC(uuid.c_str())));
            if(!writeStringMap(kPrefix, localAssetSettings[asset], writer))
                return false;
            VPVL_XML_RC(xmlTextWriterEndElement(writer)); /* vpvl:asset */
        }
        VPVL_XML_RC(xmlTextWriterEndElement(writer)); /* vpvl:asset */
        VPVL_XML_RC(xmlTextWriterStartElementNS(writer, kPrefix, VPVL_CAST_XC("motions"), 0));
        int nframes = 0;
        Quaternion ix, iy, iz, ir, ifv, idt;
        for (VMDMotionMap::const_iterator it = motions.begin(); it != motions.end(); it++) {
            const std::string &motionUUID = (*it).first;
            const VMDMotion *motion = (*it).second;
            VPVL_XML_RC(xmlTextWriterStartElementNS(writer, kPrefix, VPVL_CAST_XC("motion"), 0));
            const std::string &modelUUID = this->findModelUUID(motion->parentModel());
            if (modelUUID != Project::kNullUUID)
                VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("model"), VPVL_CAST_XC(modelUUID.c_str())));
            VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("uuid"), VPVL_CAST_XC(motionUUID.c_str())));
            VPVL_XML_RC(xmlTextWriterStartElementNS(writer, kPrefix, VPVL_CAST_XC("animation"), 0));
            VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("type"), VPVL_CAST_XC("bone")));
            const BoneAnimation &ba = motion->boneAnimation();
            nframes = ba.countKeyframes();
            for (int j = 0; j < nframes; j++) {
                const BoneKeyframe *frame = ba.frameAt(j);
                const std::string &name = delegate->toUnicode(std::string(reinterpret_cast<const char *>(frame->name())));
                VPVL_XML_RC(xmlTextWriterStartElementNS(writer, kPrefix, VPVL_CAST_XC("keyframe"), 0));
                VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("name"), VPVL_CAST_XC(name.c_str())));
                internal::snprintf(buffer, sizeof(buffer), "%d", static_cast<int>(frame->frameIndex()));
                VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("index"), VPVL_CAST_XC(buffer)));
                const Vector3 &position = frame->position();
                internal::snprintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f", position.x(), position.y(), -position.z());
                VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("position"), VPVL_CAST_XC(buffer)));
                const Quaternion &rotation = frame->rotation();
                internal::snprintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f,%.8f",
                                   -rotation.x(), -rotation.y(), rotation.z(), rotation.w());
                VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("rotation"), VPVL_CAST_XC(buffer)));
                VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("ik"), VPVL_CAST_XC(frame->isIKEnabled() ? "true" : "false")));
                frame->getInterpolationParameter(BoneKeyframe::kX, ix);
                frame->getInterpolationParameter(BoneKeyframe::kY, iy);
                frame->getInterpolationParameter(BoneKeyframe::kZ, iz);
                frame->getInterpolationParameter(BoneKeyframe::kRotation, ir);
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
                VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("interpolation"), VPVL_CAST_XC(buffer)));
                VPVL_XML_RC(xmlTextWriterEndElement(writer));
            }
            VPVL_XML_RC(xmlTextWriterEndElement(writer)); /* animation */
            VPVL_XML_RC(xmlTextWriterStartElementNS(writer, kPrefix, VPVL_CAST_XC("animation"), 0));
            VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("type"), VPVL_CAST_XC("morph")));
            const FaceAnimation &fa = motion->faceAnimation();
            nframes = fa.countKeyframes();
            for (int j = 0; j < nframes; j++) {
                const FaceKeyframe *frame = fa.frameAt(j);
                const std::string &name = delegate->toUnicode(std::string(reinterpret_cast<const char *>(frame->name())));
                VPVL_XML_RC(xmlTextWriterStartElementNS(writer, kPrefix, VPVL_CAST_XC("keyframe"), 0));
                VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("name"), VPVL_CAST_XC(name.c_str())));
                internal::snprintf(buffer, sizeof(buffer), "%d", static_cast<int>(frame->frameIndex()));
                VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("index"), VPVL_CAST_XC(buffer)));
                internal::snprintf(buffer, sizeof(buffer), "%.4f", frame->weight());
                VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("weight"), VPVL_CAST_XC(buffer)));
                VPVL_XML_RC(xmlTextWriterEndElement(writer));
            }
            VPVL_XML_RC(xmlTextWriterEndElement(writer)); /* animation */
            VPVL_XML_RC(xmlTextWriterStartElementNS(writer, kPrefix, VPVL_CAST_XC("animation"), 0));
            VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("type"), VPVL_CAST_XC("camera")));
            const CameraAnimation &ca = motion->cameraAnimation();
            nframes = ca.countKeyframes();
            for (int j = 0; j < nframes; j++) {
                const CameraKeyframe *frame = ca.frameAt(j);
                VPVL_XML_RC(xmlTextWriterStartElementNS(writer, kPrefix, VPVL_CAST_XC("keyframe"), 0));
                internal::snprintf(buffer, sizeof(buffer), "%d", static_cast<int>(frame->frameIndex()));
                VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("index"), VPVL_CAST_XC(buffer)));
                const Vector3 &position = frame->position();
                internal::snprintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f", position.x(), position.y(), -position.z());
                VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("position"), VPVL_CAST_XC(buffer)));
                const Vector3 &angle = frame->angle();
                internal::snprintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f",
                                   vpvl::radian(-angle.x()), vpvl::radian(-angle.y()), vpvl::radian(-angle.z()));
                VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("angle"), VPVL_CAST_XC(buffer)));
                internal::snprintf(buffer, sizeof(buffer), "%.8f", frame->fovy());
                VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("fovy"), VPVL_CAST_XC(buffer)));
                internal::snprintf(buffer, sizeof(buffer), "%.8f", frame->distance());
                VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("distance"), VPVL_CAST_XC(buffer)));
                frame->getInterpolationParameter(CameraKeyframe::kX, ix);
                frame->getInterpolationParameter(CameraKeyframe::kY, iy);
                frame->getInterpolationParameter(CameraKeyframe::kZ, iz);
                frame->getInterpolationParameter(CameraKeyframe::kRotation, ir);
                frame->getInterpolationParameter(CameraKeyframe::kFovy, ifv);
                frame->getInterpolationParameter(CameraKeyframe::kDistance, idt);
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
                VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("interpolation"), VPVL_CAST_XC(buffer)));
                VPVL_XML_RC(xmlTextWriterEndElement(writer));
            }
            VPVL_XML_RC(xmlTextWriterEndElement(writer)); /* animation */
            VPVL_XML_RC(xmlTextWriterStartElementNS(writer, kPrefix, VPVL_CAST_XC("animation"), 0));
            VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("type"), VPVL_CAST_XC("light")));
            const LightAnimation &la = motion->lightAnimation();
            nframes = la.countKeyframes();
            for (int j = 0; j < nframes; j++) {
                const LightKeyframe *frame = la.frameAt(j);
                VPVL_XML_RC(xmlTextWriterStartElementNS(writer, kPrefix, VPVL_CAST_XC("keyframe"), 0));
                internal::snprintf(buffer, sizeof(buffer), "%d", static_cast<int>(frame->frameIndex()));
                VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("index"), VPVL_CAST_XC(buffer)));
                const Vector3 &color = frame->color();
                internal::snprintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f", color.x(), color.y(), color.z());
                VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("color"), VPVL_CAST_XC(buffer)));
                const Vector3 &direction = frame->direction();
                internal::snprintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f", direction.x(), direction.y(), direction.z());
                VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("direction"), VPVL_CAST_XC(buffer)));
                VPVL_XML_RC(xmlTextWriterEndElement(writer));
            }
            VPVL_XML_RC(xmlTextWriterEndElement(writer)); /* animation */
            VPVL_XML_RC(xmlTextWriterEndElement(writer)); /* vpvl:motion */
        }
        VPVL_XML_RC(xmlTextWriterEndElement(writer)); /* vpvl:motions */
        VPVL_XML_RC(xmlTextWriterEndElement(writer)); /* vpvl:project */
        VPVL_XML_RC(xmlTextWriterEndDocument(writer));
        return true;
    }

    static const char *projectPrefix() {
        return "vpvm";
    }
    static const char *projectNamespaceURI() {
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
        case kAssetMotion:
            return "kAssetMotion";
        case kAnimation:
            return "kAnimation";
        case kBoneMotion:
            return "kBoneMotion";
        case kMorphMotion:
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
        static const char *kPrefix = projectPrefix();
        return equals(prefix, kPrefix) && equals(localname, dst);
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
        Handler *self = static_cast<Handler *>(context);
        char attributeName[kAttributeBufferSize];
        std::string value;
        int index = 0;
        if (self->depth == 0 && equals(prefix, localname, "project")) {
            for (int i = 0; i < nattributes; i++, index += 5) {
                if (equals(attributes[index], "version")) {
                    newString(attributes, index, value);
                    self->version = value;
                }
            }
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
            if (self->state == kSettings && equals(prefix, localname, "value")) {
                for (int i = 0; i < nattributes; i++, index += 5) {
                    if (equals(attributes[index], "name")) {
                        newString(attributes, index, value);
                        self->key = value;
                    }
                }
            }
            if (self->state == kModels && equals(prefix, localname, "model")) {
                self->currentModel = new PMDModel();
                for (int i = 0; i < nattributes; i++, index += 5) {
                    if (equals(attributes[index], "uuid")) {
                        newString(attributes, index, value);
                        self->uuid = value;
                    }
                }
                self->pushState(kModel);
            }
            else if (self->state == kAssets && equals(prefix, localname, "asset")) {
                self->currentAsset = new Asset();
                for (int i = 0; i < nattributes; i++, index += 5) {
                    if (equals(attributes[index], "uuid")) {
                        newString(attributes, index, value);
                        self->uuid = value;
                    }
                }
                self->pushState(kAsset);
            }
            else if (self->state == kMotions && equals(prefix, localname, "motion")) {
                bool found = false;
                for (int i = 0; i < nattributes; i++, index += 5) {
                    const xmlChar *name = attributes[index];
                    if (equals(name, "uuid")) {
                        newString(attributes, index, value);
                        self->uuid = value;
                        continue;
                    }
                    else if (equals(name, "model")) {
                        newString(attributes, index, value);
                        self->parentModel = value;
                        continue;
                    }
                    else if (!equals(name, "type")) {
                        continue;
                    }
                    strncpy(attributeName, reinterpret_cast<const char *>(attributes[index + 3]), sizeof(attributeName));
                    attributeName[sizeof(attributeName) - 1] = 0;
                    if (strncmp(attributeName, "asset", 5) == 0) {
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
            if ((self->state == kModel || self->state == kAsset) && equals(prefix, localname, "value")) {
                for (int i = 0; i < nattributes; i++, index += 5) {
                    if (equals(attributes[index], "name")) {
                        newString(attributes, index, value);
                        self->key = value;
                    }
                }
            }
            else if (self->state == kAnimation && equals(prefix, localname, "animation")) {
                for (int i = 0; i < nattributes; i++, index += 5) {
                    if (!equals(attributes[index], "type"))
                        continue;
                    strncpy(attributeName, reinterpret_cast<const char *>(attributes[index + 3]), sizeof(attributeName));
                    attributeName[sizeof(attributeName) - 1] = 0;
                    if (strncmp(attributeName, "bone", 4) == 0) {
                        self->pushState(kBoneMotion);
                    }
                    else if (strncmp(attributeName, "light", 5) == 0) {
                        self->pushState(kLightMotion);
                    }
                    else if (strncmp(attributeName, "morph", 5) == 0) {
                        self->pushState(kMorphMotion);
                    }
                    else if (strncmp(attributeName, "camera", 6) == 0) {
                        self->pushState(kCameraMotion);
                    }
                }
            }
            else if (equals(prefix, localname, "keyframe")) {
                switch (self->state) {
                case kAssetMotion:
                    break;
                }
            }
        }
        else if (self->depth == 4 && equals(localname, "keyframe")) {
            Array<std::string> tokens;
            Vector3 vec3(0, 0, 0);
            Vector4 vec4(0, 0, 0, 0);
            QuadWord qw(0, 0, 0, 0);
            switch (self->state) {
            case kBoneMotion:
            {
                BoneKeyframe *keyframe = new BoneKeyframe();
                keyframe->setDefaultInterpolationParameter();
                for (int i = 0; i < nattributes; i++, index += 5) {
                    strncpy(attributeName, reinterpret_cast<const char *>(attributes[index]), sizeof(attributeName));
                    attributeName[sizeof(attributeName) - 1] = 0;
                    if (strncmp(attributeName, "ik", 2) == 0) {
                        newString(attributes, index, value);
                        keyframe->setIKEnable(value == "true");
                    }
                    else if (strncmp(attributeName, "name", 4) == 0) {
                        newString(attributes, index, value);
                        keyframe->setName(reinterpret_cast<const uint8_t *>(self->delegate->fromUnicode(value).c_str()));
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
                                keyframe->setInterpolationParameter(static_cast<BoneKeyframe::InterpolationType>(i), qw);
                            }
                        }
                    }
                }
                self->currentMotion->mutableBoneAnimation()->addKeyframe(keyframe);
                break;
            }
            case kMorphMotion:
            {
                FaceKeyframe *keyframe = new FaceKeyframe();
                for (int i = 0; i < nattributes; i++, index += 5) {
                    strncpy(attributeName, reinterpret_cast<const char *>(attributes[index]), sizeof(attributeName));
                    attributeName[sizeof(attributeName) - 1] = 0;
                    if (strncmp(attributeName, "name", 4) == 0) {
                        newString(attributes, index, value);
                        keyframe->setName(reinterpret_cast<const uint8_t *>(self->delegate->fromUnicode(value).c_str()));
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
                self->currentMotion->mutableFaceAnimation()->addKeyframe(keyframe);
                break;
            }
            case kCameraMotion:
            {
                CameraKeyframe *keyframe = new CameraKeyframe();
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
                            reinterpret_cast<CameraKeyframe *>(keyframe)->setAngle(vec3);
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
                                keyframe->setInterpolationParameter(static_cast<CameraKeyframe::InterpolationType>(i), qw);
                            }
                        }
                    }
                }
                self->currentMotion->mutableCameraAnimation()->addKeyframe(keyframe);
                break;
            }
            case kLightMotion:
            {
                LightKeyframe *keyframe = new LightKeyframe();
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
                self->currentMotion->mutableLightAnimation()->addKeyframe(keyframe);
                break;
            }
            }
        }
    }
    static void cdataBlock(void *context,
                           const xmlChar *cdata,
                           int len)
    {
        Handler *self = static_cast<Handler *>(context);
        if (self->state == kSettings) {
            std::string value(reinterpret_cast<const char *>(cdata), len);
            self->globalSettings[self->key] = value;
        }
        else if (self->state == kModel) {
            std::string value(reinterpret_cast<const char *>(cdata), len);
            self->localModelSettings[self->currentModel][self->key] = value;
        }
        else if (self->state == kAsset) {
            std::string value(reinterpret_cast<const char *>(cdata), len);
            self->localAssetSettings[self->currentAsset][self->key] = value;
        }
    }
    static void endElement(void *context,
                           const xmlChar *localname,
                           const xmlChar *prefix,
                           const xmlChar * /* URI */)
    {
        Handler *self = static_cast<Handler *>(context);
        if (self->depth == 4 && !equals(localname, "keyframe")) {
            self->popState(kAnimation);
        }
        if (self->depth == 3) {
            switch (self->state) {
            case kAsset:
                if (equals(prefix, localname, "asset")) {
                    if (!self->uuid.empty()) {
                        if (self->uuid != Project::kNullUUID)
                            self->assets[self->uuid] = self->currentAsset;
                        else
                            delete self->currentAsset;
                        self->currentAsset = 0;
                    }
                    self->popState(kAssets);
                    self->uuid = "";
                }
                self->key = "";
                break;
            case kModel:
                if (equals(prefix, localname, "model")) {
                    if (!self->uuid.empty()) {
                        if (self->uuid != Project::kNullUUID)
                            self->models[self->uuid] = self->currentModel;
                        else
                            delete self->currentModel;
                        self->currentModel = 0;
                    }
                    self->popState(kModels);
                    self->uuid = "";
                }
                self->key = "";
                break;
            case kAnimation:
                if (equals(prefix, localname, "motion")) {
                    if (!self->uuid.empty()) {
                        if (self->uuid != Project::kNullUUID) {
                            self->motions[self->uuid] = self->currentMotion;
                            const std::string &parentModel = self->parentModel;
                            if (!parentModel.empty()) {
                                PMDModelMap::const_iterator it = self->models.find(parentModel);
                                if (it != self->models.end()) {
                                    if (PMDModel *model = self->models[parentModel])
                                        model->addMotion(self->currentMotion);
                                }
                            }
                        }
                        else {
                            delete self->currentMotion;
                        }
                        self->currentMotion = 0;
                    }
                    self->uuid = self->parentModel = "";
                    self->popState(kMotions);
                }
                break;
            case kAssetMotion:
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
                self->key = "";
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
        Handler *self = static_cast<Handler *>(context);
        va_list ap;
        va_start(ap, format);
        self->delegate->error(format, ap);
        va_end(ap);
    }
    static void warning(void *context, const char *format, ...)
    {
        Handler *self = static_cast<Handler *>(context);
        va_list ap;
        va_start(ap, format);
        self->delegate->warning(format, ap);
        va_end(ap);
    }

    Project::IDelegate *delegate;
    AssetMap assets;
    PMDModelMap models;
    VMDMotionMap motions;
    StringMap globalSettings;
    std::map<const Asset *, StringMap> localAssetSettings;
    std::map<const PMDModel *, StringMap> localModelSettings;
    std::string version;
    std::string key;
    std::string parentModel;
    Project::UUID uuid;
    Asset *currentAsset;
    PMDModel *currentModel;
    VMDMotion *currentMotion;
    State state;
    int depth;
};

const std::string Handler::kEmpty = "";
const Project::UUID Project::kNullUUID = "{00000000-0000-0000-0000-000000000000}";
const std::string Project::kSettingNameKey = "name";
const std::string Project::kSettingURIKey = "uri";

bool Project::isReservedSettingKey(const std::string &key)
{
    return key.find(kSettingNameKey) == 0 || key.find(kSettingURIKey) == 0;
}

Project::Project(IDelegate *delegate)
    : m_handler(0),
      m_dirty(false)
{
    internal::zerofill(&m_sax, sizeof(m_sax));
    m_handler = new Handler(delegate);
    m_sax.initialized = XML_SAX2_MAGIC;
    m_sax.startElementNs = &Handler::startElement;
    m_sax.endElementNs = &Handler::endElement;
    m_sax.cdataBlock = &Handler::cdataBlock;
    m_sax.warning = &Handler::warning;
    m_sax.error = &Handler::error;
}

Project::~Project()
{
    internal::zerofill(&m_sax, sizeof(m_sax));
    delete m_handler;
    m_handler = 0;
    m_dirty = false;
}

bool Project::load(const char *path)
{
    return validate(xmlSAXUserParseFile(&m_sax, m_handler, path) == 0);
}

bool Project::load(const uint8_t *data, size_t size)
{
    return validate(xmlSAXUserParseMemory(&m_sax, m_handler, reinterpret_cast<const char *>(data), size) == 0);
}

bool Project::save(const char *path)
{
    return save0(xmlNewTextWriterFilename(path, 0));
}

bool Project::save(xmlBufferPtr &buffer)
{
    return save0(xmlNewTextWriterMemory(buffer, 0));
}

const std::string &Project::version() const
{
    return m_handler->version;
}

const std::string &Project::globalSetting(const std::string &key) const
{
    return m_handler->globalSettings[key];
}

const std::string &Project::assetSetting(const Asset *asset, const std::string &key) const
{
    return containsAsset(asset) ? m_handler->localAssetSettings[asset][key] : Handler::kEmpty;
}

const std::string &Project::modelSetting(const PMDModel *model, const std::string &key) const
{
    return containsModel(model) ? m_handler->localModelSettings[model][key] : Handler::kEmpty;
}

const Project::UUIDList Project::assetUUIDs() const
{
    const Handler::AssetMap &assets = m_handler->assets;
    Project::UUIDList uuids;
    for (Handler::AssetMap::const_iterator it = assets.begin(); it != assets.end(); it++)
        uuids.push_back((*it).first);
    return uuids;
}

const Project::UUIDList Project::modelUUIDs() const
{
    const Handler::PMDModelMap &models = m_handler->models;
    Project::UUIDList uuids;
    for (Handler::PMDModelMap::const_iterator it = models.begin(); it != models.end(); it++)
        uuids.push_back((*it).first);
    return uuids;
}

const Project::UUIDList Project::motionUUIDs() const
{
    const Handler::VMDMotionMap &motions = m_handler->motions;
    Project::UUIDList uuids;
    for (Handler::VMDMotionMap::const_iterator it = motions.begin(); it != motions.end(); it++)
        uuids.push_back((*it).first);
    return uuids;
}

Asset *Project::asset(const UUID &uuid) const
{
    return m_handler->findAsset(uuid);
}

PMDModel *Project::model(const UUID &uuid) const
{
    return m_handler->findModel(uuid);
}

VMDMotion *Project::motion(const UUID &uuid) const
{
    return m_handler->findMotion(uuid);
}

const Project::UUID &Project::assetUUID(const Asset *asset) const
{
    return m_handler->findAssetUUID(asset);
}

const Project::UUID &Project::modelUUID(const PMDModel *model) const
{
    return m_handler->findModelUUID(model);
}

const Project::UUID &Project::motionUUID(const VMDMotion *motion) const
{
    return m_handler->findMotionUUID(motion);
}

bool Project::containsAsset(const Asset *asset) const
{
    return assetUUID(asset) != kNullUUID;
}

bool Project::containsModel(const PMDModel *model) const
{
    return modelUUID(model) != kNullUUID;
}

bool Project::containsMotion(const VMDMotion *motion) const
{
    return motionUUID(motion) != kNullUUID;
}

void Project::setDirty(bool value)
{
    m_dirty = value;
}

void Project::addAsset(Asset *asset, const UUID &uuid)
{
    if (!containsAsset(asset)) {
        m_handler->assets[uuid] = asset;
        setDirty(true);
    }
}

void Project::addModel(PMDModel *model, const UUID &uuid)
{
    if (!containsModel(model)) {
        m_handler->models[uuid] = model;
        setDirty(true);
    }
}

void Project::addMotion(VMDMotion *motion, PMDModel *model, const UUID &uuid)
{
    if (!containsMotion(motion)) {
        m_handler->motions[uuid] = motion;
        if (model)
            model->addMotion(motion);
        setDirty(true);
    }
}

void Project::deleteAsset(Asset *&asset)
{
    if (containsAsset(asset)) {
        m_handler->removeAsset(asset);
        delete asset;
        asset = 0;
        setDirty(true);
    }
}

void Project::deleteModel(PMDModel *&model)
{
    if (containsModel(model)) {
        m_handler->removeModel(model);
        delete model;
        model = 0;
        setDirty(true);
    }
}

void Project::deleteMotion(VMDMotion *&motion, PMDModel *model)
{
    if (containsMotion(motion)) {
        m_handler->removeMotion(motion);
        if (model) {
            model->deleteMotion(motion);
        }
        else {
            delete motion;
            motion = 0;
        }
        setDirty(true);
    }
}

void Project::removeAsset(Asset *asset)
{
    m_handler->removeAsset(asset);
    setDirty(true);
}

void Project::removeModel(PMDModel *model)
{
    m_handler->removeModel(model);
    setDirty(true);
}

void Project::removeMotion(VMDMotion *motion, PMDModel *model)
{
    m_handler->removeMotion(motion);
    if (model)
        model->removeMotion(motion);
}

void Project::setGlobalSetting(const std::string &key, const std::string &value)
{
    m_handler->globalSettings[key] = value;
    setDirty(true);
}

void Project::setAssetSetting(const Asset *asset, const std::string &key, const std::string &value)
{
    if (containsAsset(asset)) {
        m_handler->localAssetSettings[asset][key] = value;
        setDirty(true);
    }
}

void Project::setModelSetting(const PMDModel *model, const std::string &key, const std::string &value)
{
    if (containsModel(model)) {
        m_handler->localModelSettings[model][key] = value;
        setDirty(true);
    }
}

bool Project::save0(xmlTextWriterPtr ptr)
{
    bool ret = m_handler->save(ptr);
    xmlFreeTextWriter(ptr);
    if (ret)
        m_dirty = false;
    return ret;
}

bool Project::validate(bool result)
{
    return result && m_handler->depth == 0 && m_handler->checkDuplicateUUID();
}

} /* namespace vpvl */
