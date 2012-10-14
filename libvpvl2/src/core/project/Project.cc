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

#include "vpvl2/vpvl2.h"
#include "vpvl2/internal/util.h"

#include "vpvl2/Project.h"
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
#include <map>

#define VPVL2_CAST_XC(str) reinterpret_cast<const xmlChar *>(str)
#define VPVL2_XML_RC(rc) { if (rc < 0) { fprintf(stderr, "Failed at %s:%d\n", __FILE__, __LINE__); return false; } }

namespace
{

static inline int StringPrintf(uint8_t *buffer, size_t size, const char *format, ...)
{
    assert(buffer && size > 0);
    va_list ap;
    va_start(ap, format);
    int ret = vsnprintf(reinterpret_cast<char *>(buffer), size, format, ap);
    va_end(ap);
    return ret;
}

static inline float StringToFloat(const char *str)
{
    assert(str);
    char *p = 0;
#if defined(WIN32)
    return static_cast<float>(strtod(str, &p));
#else
    return strtof(str, &p);
#endif
}

}

namespace vpvl2
{

struct Project::PrivateContext {
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
        kVMDBoneMotion,
        kVMDMorphMotion,
        kVMDCameraMotion,
        kVMDLightMotion
    };
    static const int kElementContentBufferSize = 128;
    static const std::string kEmpty;
    typedef std::map<std::string, std::string> StringMap;
    typedef std::map<Project::UUID, IModel *> ModelMap;
    typedef std::map<Project::UUID, IMotion *> MotionMap;

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
        case kAssetMotion:
            return "kAssetMotion";
        case kAnimation:
            return "kAnimation";
        case kVMDBoneMotion:
            return "kBoneMotion";
        case kVMDMorphMotion:
            return "kVerticesMotion";
        case kVMDCameraMotion:
            return "kCameraMotion";
        case kVMDLightMotion:
            return "kLightMotion";
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
        value.setX(StringToFloat(tokens.at(offset + 0).c_str()));
        value.setY(StringToFloat(tokens.at(offset + 1).c_str()));
        value.setZ(StringToFloat(tokens.at(offset + 2).c_str()));
        value.setW(StringToFloat(tokens.at(offset + 3).c_str()));
    }
    static bool createVector3(const Array<std::string> &tokens, Vector3 &value) {
        if (tokens.count() == 3) {
            value.setX(StringToFloat(tokens.at(0).c_str()));
            value.setY(StringToFloat(tokens.at(1).c_str()));
            value.setZ(StringToFloat(tokens.at(2).c_str()));
            return true;
        }
        return false;
    }
    static bool createVector4(const Array<std::string> &tokens, Vector4 &value) {
        if (tokens.count() == 4) {
            value.setX(StringToFloat(tokens.at(0).c_str()));
            value.setY(StringToFloat(tokens.at(1).c_str()));
            value.setZ(StringToFloat(tokens.at(2).c_str()));
            value.setW(StringToFloat(tokens.at(3).c_str()));
            return true;
        }
        return false;
    }

    PrivateContext(Scene *scene, Project::IDelegate *delegate, Factory *factory)
        : delegateRef(delegate),
          sceneRef(scene),
          factoryRef(factory),
          currentString(0),
          currentAsset(0),
          currentModel(0),
          currentMotion(0),
          state(kInitial),
          depth(0),
          dirty(false)
    {
        internal::zerofill(&saxHandler, sizeof(saxHandler));
    }
    ~PrivateContext() {
        internal::zerofill(&saxHandler, sizeof(saxHandler));
        for (ModelMap::const_iterator it = assets.begin(); it != assets.end(); it++) {
            IModel *model = (*it).second;
            sceneRef->deleteModel(model);
        }
        assets.clear();
        for (ModelMap::const_iterator it = models.begin(); it != models.end(); it++) {
            IModel *model = (*it).second;
            sceneRef->deleteModel(model);
        }
        models.clear();
        for (MotionMap::const_iterator it = motions.begin(); it != motions.end(); it++) {
            IMotion *motion = (*it).second;
            sceneRef->removeMotion(motion);
            delete motion;
        }
        motions.clear();
        delete currentString;
        currentString = 0;
        delete currentAsset;
        currentAsset = 0;
        delete currentModel;
        currentModel = 0;
        delete currentMotion;
        currentMotion = 0;
        state = kInitial;
        depth = 0;
        dirty = false;
        sceneRef = 0;
        delegateRef = 0;
        factoryRef = 0;
    }

    bool isDuplicatedUUID(const Project::UUID &uuid, std::set<Project::UUID> &set) const {
        if (set.find(uuid) != set.end())
            return true;
        set.insert(uuid);
        return false;
    }
    bool checkDuplicateUUID() const {
        std::set<Project::UUID> set;
        for (ModelMap::const_iterator it = assets.begin(); it != assets.end(); it++) {
            if (isDuplicatedUUID((*it).first, set))
                return false;
        }
        for (ModelMap::const_iterator it = models.begin(); it != models.end(); it++) {
            if (isDuplicatedUUID((*it).first, set))
                return false;
        }
        for (MotionMap::const_iterator it = motions.begin(); it != motions.end(); it++) {
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
    IModel *findModel(const Project::UUID &value) const {
        if (value == Project::kNullUUID)
            return 0;
        ModelMap::const_iterator it = assets.find(value);
        if (it != assets.end())
            return (*it).second;
        it = models.find(value);
        if (it != models.end())
            return (*it).second;
        return 0;
    }
    const Project::UUID &findModelUUID(const IModel *value) const {
        if (!value)
            return Project::kNullUUID;
        for (ModelMap::const_iterator it = assets.begin(); it != assets.end(); it++) {
            if ((*it).second == value)
                return (*it).first;
        }
        for (ModelMap::const_iterator it = models.begin(); it != models.end(); it++) {
            if ((*it).second == value)
                return (*it).first;
        }
        return Project::kNullUUID;
    }
    IMotion *findMotion(const Project::UUID &value) const {
        if (value == Project::kNullUUID)
            return 0;
        MotionMap::const_iterator it = motions.find(value);
        if (it != motions.end())
            return (*it).second;
        return 0;
    }
    const Project::UUID &findMotionUUID(const IMotion *value) const {
        if (!value)
            return Project::kNullUUID;
        for (MotionMap::const_iterator it = motions.begin(); it != motions.end(); it++) {
            if ((*it).second == value)
                return (*it).first;
        }
        return Project::kNullUUID;
    }
    bool removeModel(const IModel *model) {
        for (ModelMap::iterator it = assets.begin(); it != assets.end(); it++) {
            if ((*it).second == model) {
                assets.erase(it);
                return true;
            }
        }
        for (ModelMap::iterator it = models.begin(); it != models.end(); it++) {
            if ((*it).second == model) {
                models.erase(it);
                return true;
            }
        }
        return false;
    }
    bool removeMotion(const IMotion *motion) {
        for (MotionMap::iterator it = motions.begin(); it != motions.end(); it++) {
            if ((*it).second == motion) {
                motions.erase(it);
                return true;
            }
        }
        return false;
    }

    bool save(xmlTextWriterPtr writer) {
        uint8_t buffer[kElementContentBufferSize];
        if (!writer)
            return false;
        VPVL2_XML_RC(xmlTextWriterSetIndent(writer, 1));
        VPVL2_XML_RC(xmlTextWriterStartDocument(writer, 0, "UTF-8", 0));
        VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("project"), projectNamespaceURI()));
        StringPrintf(buffer, sizeof(buffer), "%s", libraryVersionString());
        VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("version"), VPVL2_CAST_XC(buffer)));
        if (!writeSettings(writer))
            return false;
        if (!writeModels(writer))
            return false;
        if (!writeAssets(writer))
            return false;
        VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("motions"), 0));
        for (MotionMap::const_iterator it = motions.begin(); it != motions.end(); it++) {
            const std::string &motionUUID = (*it).first;
            IMotion *motionPtr = (*it).second;
            if (motionPtr->type() != IMotion::kVMD)
                continue;
            const vmd::Motion *motion = reinterpret_cast<vmd::Motion *>(motionPtr);
            VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("motion"), 0));
            const std::string &modelUUID = this->findModelUUID(motion->parentModel());
            if (modelUUID != Project::kNullUUID)
                VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("model"), VPVL2_CAST_XC(modelUUID.c_str())));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("uuid"), VPVL2_CAST_XC(motionUUID.c_str())));
            if (!writeVMDBoneKeyframes(writer, motion))
                return false;
            if (!writeVMDMorphKeyframes(writer, motion))
                return false;
            if (!writeVMDCameraKeyframes(writer, motion))
                return false;
            if (!writeVMDLightKeyframes(writer, motion))
                return false;
            VPVL2_XML_RC(xmlTextWriterEndElement(writer)); /* vpvl:motion */
        }
        VPVL2_XML_RC(xmlTextWriterEndElement(writer)); /* vpvl:motions */
        VPVL2_XML_RC(xmlTextWriterEndElement(writer)); /* vpvl:project */
        VPVL2_XML_RC(xmlTextWriterEndDocument(writer));
        return true;
    }
    bool writeSettings(xmlTextWriterPtr writer) {
        VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("settings"), 0));
        if(!writeStringMap(projectPrefix(), globalSettings, writer))
            return false;
        VPVL2_XML_RC(xmlTextWriterEndElement(writer)); /* vpvl:setting */
        return true;
    }
    bool writeModels(xmlTextWriterPtr writer) {
        VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("models"), 0));
        for (ModelMap::const_iterator it = models.begin(); it != models.end(); it++) {
            const Project::UUID &uuid = (*it).first;
            const IModel *model = (*it).second;
            VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("model"), 0));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("uuid"), VPVL2_CAST_XC(uuid.c_str())));
            if(!writeStringMap(projectPrefix(), localModelSettings[model], writer))
                return false;
            VPVL2_XML_RC(xmlTextWriterEndElement(writer)); /* vpvl:model */
        }
        VPVL2_XML_RC(xmlTextWriterEndElement(writer)); /* vpvl:models */
        return true;
    }
    bool writeAssets(xmlTextWriterPtr writer) {
        VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("assets"), 0));
        for (ModelMap::const_iterator it = assets.begin(); it != assets.end(); it++) {
            const Project::UUID &uuid = (*it).first;
            const IModel *asset = (*it).second;
            VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("asset"), 0));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("uuid"), VPVL2_CAST_XC(uuid.c_str())));
            if(!writeStringMap(projectPrefix(), localAssetSettings[asset], writer))
                return false;
            VPVL2_XML_RC(xmlTextWriterEndElement(writer)); /* vpvl:asset */
        }
        VPVL2_XML_RC(xmlTextWriterEndElement(writer)); /* vpvl:asset */
        return true;
    }
    bool writeVMDBoneKeyframes(xmlTextWriterPtr writer, const vmd::Motion *motion) {
        Quaternion ix, iy, iz, ir;
        uint8_t buffer[kElementContentBufferSize];
        VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("animation"), 0));
        VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("type"), VPVL2_CAST_XC("bone")));
        const vmd::BoneAnimation &ba = motion->boneAnimation();
        int nkeyframes = ba.countKeyframes();
        for (int j = 0; j < nkeyframes; j++) {
            const vmd::BoneKeyframe *frame = static_cast<vmd::BoneKeyframe *>(ba.keyframeAt(j));
            const std::string &name = delegateRef->toStdFromString(frame->name());
            VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("keyframe"), 0));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("name"), VPVL2_CAST_XC(name.c_str())));
            StringPrintf(buffer, sizeof(buffer), "%d", static_cast<int>(frame->timeIndex()));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("index"), VPVL2_CAST_XC(buffer)));
            const Vector3 &position = frame->position();
            StringPrintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f", position.x(), position.y(), -position.z());
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("position"), VPVL2_CAST_XC(buffer)));
            const Quaternion &rotation = frame->rotation();
            StringPrintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f,%.8f",
                         -rotation.x(), -rotation.y(), rotation.z(), rotation.w());
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("rotation"), VPVL2_CAST_XC(buffer)));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("ik"), VPVL2_CAST_XC(frame->isIKEnabled() ? "true" : "false")));
            frame->getInterpolationParameter(vmd::BoneKeyframe::kX, ix);
            frame->getInterpolationParameter(vmd::BoneKeyframe::kY, iy);
            frame->getInterpolationParameter(vmd::BoneKeyframe::kZ, iz);
            frame->getInterpolationParameter(vmd::BoneKeyframe::kRotation, ir);
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
    bool writeVMDCameraKeyframes(xmlTextWriterPtr writer, const vmd::Motion *motion) {
        Quaternion ix, iy, iz, ir, ifv, idt;
        uint8_t buffer[kElementContentBufferSize];
        VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("animation"), 0));
        VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("type"), VPVL2_CAST_XC("camera")));
        const vmd::CameraAnimation &ca = motion->cameraAnimation();
        int nkeyframes = ca.countKeyframes();
        for (int j = 0; j < nkeyframes; j++) {
            const vmd::CameraKeyframe *frame = static_cast<vmd::CameraKeyframe *>(ca.frameAt(j));
            VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("keyframe"), 0));
            StringPrintf(buffer, sizeof(buffer), "%d", static_cast<int>(frame->timeIndex()));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("index"), VPVL2_CAST_XC(buffer)));
            const Vector3 &position = frame->position();
            StringPrintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f", position.x(), position.y(), -position.z());
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("position"), VPVL2_CAST_XC(buffer)));
            const Vector3 &angle = frame->angle();
            StringPrintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f",
                         vpvl2::radian(-angle.x()), vpvl2::radian(-angle.y()), vpvl2::radian(-angle.z()));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("angle"), VPVL2_CAST_XC(buffer)));
            StringPrintf(buffer, sizeof(buffer), "%.8f", frame->fov());
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("fovy"), VPVL2_CAST_XC(buffer)));
            StringPrintf(buffer, sizeof(buffer), "%.8f", frame->distance());
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("distance"), VPVL2_CAST_XC(buffer)));
            frame->getInterpolationParameter(vmd::CameraKeyframe::kX, ix);
            frame->getInterpolationParameter(vmd::CameraKeyframe::kY, iy);
            frame->getInterpolationParameter(vmd::CameraKeyframe::kZ, iz);
            frame->getInterpolationParameter(vmd::CameraKeyframe::kRotation, ir);
            frame->getInterpolationParameter(vmd::CameraKeyframe::kFov, ifv);
            frame->getInterpolationParameter(vmd::CameraKeyframe::kDistance, idt);
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
    bool writeVMDLightKeyframes(xmlTextWriterPtr writer, const vmd::Motion *motion) {
        uint8_t buffer[kElementContentBufferSize];
        VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("animation"), 0));
        VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("type"), VPVL2_CAST_XC("light")));
        const vmd::LightAnimation &la = motion->lightAnimation();
        int nkeyframes = la.countKeyframes();
        for (int j = 0; j < nkeyframes; j++) {
            const vmd::LightKeyframe *frame = static_cast<vmd::LightKeyframe *>(la.frameAt(j));
            VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("keyframe"), 0));
            StringPrintf(buffer, sizeof(buffer), "%d", static_cast<int>(frame->timeIndex()));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("index"), VPVL2_CAST_XC(buffer)));
            const Vector3 &color = frame->color();
            StringPrintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f", color.x(), color.y(), color.z());
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("color"), VPVL2_CAST_XC(buffer)));
            const Vector3 &direction = frame->direction();
            StringPrintf(buffer, sizeof(buffer), "%.8f,%.8f,%.8f", direction.x(), direction.y(), direction.z());
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("direction"), VPVL2_CAST_XC(buffer)));
            VPVL2_XML_RC(xmlTextWriterEndElement(writer));
        }
        VPVL2_XML_RC(xmlTextWriterEndElement(writer)); /* animation */
        return true;
    }
    bool writeVMDMorphKeyframes(xmlTextWriterPtr writer, const vmd::Motion *motion) {
        uint8_t buffer[kElementContentBufferSize];
        VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("animation"), 0));
        VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("type"), VPVL2_CAST_XC("morph")));
        const vmd::MorphAnimation &fa = motion->morphAnimation();
        int nkeyframes = fa.countKeyframes();
        for (int j = 0; j < nkeyframes; j++) {
            const vmd::MorphKeyframe *frame = static_cast<vmd::MorphKeyframe *>(fa.keyframeAt(j));
            const std::string &name = delegateRef->toStdFromString(frame->name());
            VPVL2_XML_RC(xmlTextWriterStartElementNS(writer, projectPrefix(), VPVL2_CAST_XC("keyframe"), 0));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("name"), VPVL2_CAST_XC(name.c_str())));
            StringPrintf(buffer, sizeof(buffer), "%d", static_cast<int>(frame->timeIndex()));
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("index"), VPVL2_CAST_XC(buffer)));
            StringPrintf(buffer, sizeof(buffer), "%.4f", frame->weight());
            VPVL2_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL2_CAST_XC("weight"), VPVL2_CAST_XC(buffer)));
            VPVL2_XML_RC(xmlTextWriterEndElement(writer));
        }
        VPVL2_XML_RC(xmlTextWriterEndElement(writer)); /* animation */
        return true;
    }
    bool writeStringMap(const xmlChar *prefix, const StringMap &map, xmlTextWriterPtr &writer) {
        for (StringMap::const_iterator it = map.begin(); it != map.end(); it++) {
            if (it->first.empty() || it->second.empty())
                continue;
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
            case kInitial:
            case kProject:
            case kSettings:
            case kPhysics:
            case kModels:
            case kModel:
            case kAssets:
            case kAsset:
            case kMotions:
            case kAssetMotion:
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
            if (self->settingKey == kSettingURIKey && value.find(".pmx") != std::string::npos) {
                StringMap values = self->localModelSettings[self->currentModel];
                self->localModelSettings.erase(self->currentModel);
                delete self->currentModel;
                self->currentModel = self->factoryRef->createModel(IModel::kPMX);
                self->localModelSettings[self->currentModel] = values;
            }
            self->localModelSettings[self->currentModel][self->settingKey] = value;
        }
        else if (self->state == kAsset) {
            std::string value(reinterpret_cast<const char *>(cdata), len);
            self->localAssetSettings[self->currentAsset][self->settingKey] = value;
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
            case kAssetMotion:
                if (equals(prefix, localname, "motion")) {
                    self->popState(kMotions);
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
                self->settingKey.clear();
                break;
            case kPhysics:
                if (equals(prefix, localname, "physics"))
                    self->popState(kProject);
                break;
            case kInitial:
            case kProject:
            case kModel:
            case kAsset:
            case kAssetMotion:
            case kAnimation:
            case kVMDBoneMotion:
            case kVMDMorphMotion:
            case kVMDCameraMotion:
            case kVMDLightMotion:
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
        std::string key, value;
        delete currentModel;
        currentModel = factoryRef->createModel(IModel::kPMD);
        for (int i = 0, index = 0; i < nattributes; i++, index += 5) {
            readAttributeString(attributes, index, key, value);
            if (key == "uuid") {
                uuid = value;
            }
        }
        pushState(kModel);
    }
    void readAsset(const xmlChar **attributes, int nattributes) {
        std::string key, value;
        delete currentAsset;
        currentAsset = factoryRef->createModel(IModel::kAsset);
        for (int i = 0, index = 0; i < nattributes; i++, index += 5) {
            readAttributeString(attributes, index, key, value);
            if (key ==  "uuid") {
                uuid = value;
            }
        }
        pushState(kAsset);
    }
    void readMotion(const xmlChar **attributes, int nattributes) {
        std::string key, value;
        bool found = false;
        for (int i = 0, index = 0; i < nattributes; i++, index += 5) {
            readAttributeString(attributes, index, key, value);
            if (key == "uuid") {
                uuid = value;
                continue;
            }
            else if (key == "model") {
                parentModel = value;
                continue;
            }
            else if (key != "type") {
                continue;
            }
            if (value == "asset") {
                pushState(kAssetMotion);
                found = true;
            }
        }
        if (!found) {
            delete currentMotion;
            currentMotion = factoryRef->createMotion(IMotion::kVMD, 0);
            pushState(kAnimation);
        }
    }
    void readLocalSettingKey(const xmlChar **attributes, int nattributes) {
        readGlobalSettingKey(attributes, nattributes);
    }
    void readMotionType(const xmlChar **attributes, int nattributes) {
        std::string key, value;
        for (int i = 0, index = 0; i < nattributes; i++, index += 5) {
            readAttributeString(attributes, index, key, value);
            if (key != "type")
                continue;
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
    void readVMDBoneKeyframe(const xmlChar **attributes, int nattributes) {
        IBoneKeyframe *keyframe = factoryRef->createBoneKeyframe(currentMotion);
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
                keyframe->setTimeIndex(StringToFloat(value.c_str()));
            }
            else if (key == "position") {
                splitString(value, tokens);
                if (createVector3(tokens, vec3)) {
#ifdef VPVL2_COORDINATE_OPENGL
                    vec3.setValue(vec3.x(), vec3.y(), -vec3.z());
#else
                    vec3.setValue(vec3.x(), vec3.y(), vec3.z());
#endif
                    keyframe->setPosition(vec3);
                }
            }
            else if (key == "rotation") {
                splitString(value, tokens);
                if (createVector4(tokens, vec4)) {
                    Quaternion rotation;
#ifdef VPVL2_COORDINATE_OPENGL
                    rotation.setValue(-vec4.x(), -vec4.y(), vec4.z(), vec4.w());
#else
                    rotation.setValue(vec4.x(), vec4.y(), vec4.z(), vec4.w());
#endif
                    keyframe->setRotation(rotation);
                }
            }
            else if (key == "interpolation") {
                splitString(value, tokens);
                if (tokens.count() == 16) {
                    for (int i = 0; i < 4; i++) {
                        setQuadWordValues(tokens, qw, i * 4);
                        keyframe->setInterpolationParameter(static_cast<vmd::BoneKeyframe::InterpolationType>(i), qw);
                    }
                }
            }
        }
        currentMotion->addKeyframe(keyframe);
    }
    void readVMDCameraKeyframe(const xmlChar **attributes, int nattributes) {
        vmd::CameraKeyframe *keyframe = new vmd::CameraKeyframe();
        Vector3 vec3(kZeroV3);
        QuadWord qw(0, 0, 0, 0);
        Array<std::string> tokens;
        std::string key, value;
        keyframe->setDefaultInterpolationParameter();
        for (int i = 0, index = 0; i < nattributes; i++, index += 5) {
            readAttributeString(attributes, index, key, value);
            if (key == "fovy") {
                keyframe->setFov(StringToFloat(value.c_str()));
            }
            else if (key == "index") {
                keyframe->setTimeIndex(StringToFloat(value.c_str()));
            }
            else if (key == "angle") {
                splitString(value, tokens);
                if (createVector3(tokens, vec3)) {
#ifdef VPVL2_COORDINATE_OPENGL
                    vec3.setValue(-degree(vec3.x()), -degree(vec3.y()), -degree(vec3.z()));
#else
                    vec3.setValue(degree(vec3.x()), degree(vec3.y()), -degree(vec3.z()));
#endif
                    reinterpret_cast<vmd::CameraKeyframe *>(keyframe)->setAngle(vec3);
                }
            }
            else if (key == "position") {
                splitString(value, tokens);
                if (createVector3(tokens, vec3)) {
#ifdef VPVL2_COORDINATE_OPENGL
                    vec3.setValue(vec3.x(), vec3.y(), -vec3.z());
#else
                    vec3.setValue(vec3.x(), vec3.y(), vec3.z());
#endif
                    keyframe->setPosition(vec3);
                }
            }
            else if (key == "distance") {
                keyframe->setDistance(StringToFloat(value.c_str()));
            }
            else if (key == "interpolation") {
                splitString(value, tokens);
                if (tokens.count() == 24) {
                    for (int i = 0; i < 6; i++) {
                        setQuadWordValues(tokens, qw, i * 4);
                        keyframe->setInterpolationParameter(static_cast<vmd::CameraKeyframe::InterpolationType>(i), qw);
                    }
                }
            }
        }
        currentMotion->addKeyframe(keyframe);
    }
    void readVMDLightKeyframe(const xmlChar **attributes, int nattributes) {
        vmd::LightKeyframe *keyframe = new vmd::LightKeyframe();
        Array<std::string> tokens;
        Vector3 vec3(kZeroV3);
        std::string key, value;
        for (int i = 0, index = 0; i < nattributes; i++, index += 5) {
            readAttributeString(attributes, index, key, value);
            if (key == "index") {
                keyframe->setTimeIndex(StringToFloat(value.c_str()));
            }
            else if (key == "color") {
                splitString(value, tokens);
                if (createVector3(tokens, vec3))
                    keyframe->setColor(vec3);
            }
            else if (key == "direction") {
                splitString(value, tokens);
                if (createVector3(tokens, vec3))
                    keyframe->setDirection(vec3);
            }
        }
        currentMotion->addKeyframe(keyframe);
    }
    void readVMDMorphKeyframe(const xmlChar **attributes, int nattributes) {
        IMorphKeyframe *keyframe = factoryRef->createMorphKeyframe(currentMotion);
        std::string key, value;
        for (int i = 0, index = 0; i < nattributes; i++, index += 5) {
            readAttributeString(attributes, index, key, value);
            if (key == "name") {
                delete currentString;
                currentString = delegateRef->toStringFromStd(value);
                keyframe->setName(currentString);
            }
            else if (key == "index") {
                keyframe->setTimeIndex(StringToFloat(value.c_str()));
            }
            else if (key == "weight") {
                keyframe->setWeight(StringToFloat(value.c_str()));
            }
        }
        currentMotion->addKeyframe(keyframe);
    }
    void addAsset() {
        if (!uuid.empty()) {
            if (uuid != Project::kNullUUID) {
                /* delete the previous asset before assigning to prevent memory leak */
                IModel *&assetPtr = assets[uuid];
                delete assetPtr;
                assetPtr = currentAsset;
            }
            else {
                delete currentAsset;
            }
            currentAsset = 0;
        }
        popState(kAssets);
        uuid.clear();
    }
    void addModel() {
        if (!uuid.empty()) {
            if (uuid != Project::kNullUUID) {
                /* delete the previous model before assigning to prevent memory leak */
                IModel *&modelPtr = models[uuid];
                delete modelPtr;
                modelPtr = currentModel;
            }
            else {
                delete currentModel;
            }
            currentModel = 0;
        }
        popState(kModels);
        uuid.clear();
    }
    void addMotion() {
        if (!uuid.empty()) {
            if (uuid != Project::kNullUUID) {
                motions[uuid] = currentMotion;
                if (!parentModel.empty()) {
                    ModelMap::const_iterator it = models.find(parentModel);
                    if (it != models.end()) {
                        if (IModel *model = models[parentModel])
                            currentMotion->setParentModel(model);
                    }
                }
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
    static inline void readAttributeString(const xmlChar **attributes,
                                           int index,
                                           std::string &key,
                                           std::string &value) {
        key.assign(reinterpret_cast<const char *>(attributes[index]));
        value.assign(reinterpret_cast<const char *>(attributes[index + 3]),
                     reinterpret_cast<const char *>(attributes[index + 4]));
    }

    static void error(void *context, const char *format, ...) {
        PrivateContext *self = static_cast<PrivateContext *>(context);
        va_list ap;
        va_start(ap, format);
        self->delegateRef->error(format, ap);
        va_end(ap);
    }
    static void warning(void *context, const char *format, ...) {
        PrivateContext *self = static_cast<PrivateContext *>(context);
        va_list ap;
        va_start(ap, format);
        self->delegateRef->warning(format, ap);
        va_end(ap);
    }

    xmlSAXHandler saxHandler;
    Project::IDelegate *delegateRef;
    Scene *sceneRef;
    Factory *factoryRef;
    ModelMap assets;
    ModelMap models;
    MotionMap motions;
    StringMap globalSettings;
    std::map<const IModel *, StringMap> localAssetSettings;
    std::map<const IModel *, StringMap> localModelSettings;
    std::string version;
    std::string settingKey;
    std::string parentModel;
    Project::UUID uuid;
    const IString *currentString;
    IModel *currentAsset;
    IModel *currentModel;
    IMotion *currentMotion;
    State state;
    int depth;
    bool dirty;
};

const std::string Project::PrivateContext::kEmpty = "";
const Project::UUID Project::kNullUUID = "{00000000-0000-0000-0000-000000000000}";
const std::string Project::kSettingNameKey = "name";
const std::string Project::kSettingURIKey = "uri";

bool Project::isReservedSettingKey(const std::string &key)
{
    return key.find(kSettingNameKey) == 0 || key.find(kSettingURIKey) == 0;
}

Project::Project(IDelegate *delegate, Factory *factory)
    : Scene(),
      m_context(0)
{
    m_context = new PrivateContext(this, delegate, factory);
    xmlSAXHandler &handler = m_context->saxHandler;
    handler.initialized = XML_SAX2_MAGIC;
    handler.startElementNs = &PrivateContext::startElement;
    handler.endElementNs = &PrivateContext::endElement;
    handler.cdataBlock = &PrivateContext::cdataBlock;
    handler.warning = &PrivateContext::warning;
    handler.error = &PrivateContext::error;
}

Project::~Project()
{
    delete m_context;
    m_context = 0;
}

bool Project::load(const char *path)
{
    return validate(xmlSAXUserParseFile(&m_context->saxHandler, m_context, path) == 0);
}

bool Project::load(const uint8_t *data, size_t size)
{
    return validate(xmlSAXUserParseMemory(&m_context->saxHandler, m_context, reinterpret_cast<const char *>(data), size) == 0);
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
    return m_context->version;
}

const std::string &Project::globalSetting(const std::string &key) const
{
    return m_context->globalSettings[key];
}

const std::string &Project::modelSetting(const IModel *model, const std::string &key) const
{
    if (model) {
        switch (model->type()) {
        case IModel::kAsset:
            return containsModel(model) ? m_context->localAssetSettings[model][key] : PrivateContext::kEmpty;
        case IModel::kPMD:
        case IModel::kPMX:
            return containsModel(model) ? m_context->localModelSettings[model][key] : PrivateContext::kEmpty;
        default:
            return PrivateContext::kEmpty;
        }
    }
    return PrivateContext::kEmpty;
}

const Project::UUIDList Project::modelUUIDs() const
{
    Project::UUIDList uuids;
    const PrivateContext::ModelMap &assets = m_context->assets;
    for (PrivateContext::ModelMap::const_iterator it = assets.begin(); it != assets.end(); it++)
        uuids.push_back((*it).first);
    const PrivateContext::ModelMap &models = m_context->models;
    for (PrivateContext::ModelMap::const_iterator it = models.begin(); it != models.end(); it++)
        uuids.push_back((*it).first);
    return uuids;
}

const Project::UUIDList Project::motionUUIDs() const
{
    const PrivateContext::MotionMap &motions = m_context->motions;
    Project::UUIDList uuids;
    for (PrivateContext::MotionMap::const_iterator it = motions.begin(); it != motions.end(); it++)
        uuids.push_back((*it).first);
    return uuids;
}

IModel *Project::model(const UUID &uuid) const
{
    return m_context->findModel(uuid);
}

IMotion *Project::motion(const UUID &uuid) const
{
    return m_context->findMotion(uuid);
}

const Project::UUID &Project::modelUUID(const IModel *model) const
{
    return m_context->findModelUUID(model);
}

const Project::UUID &Project::motionUUID(const IMotion *motion) const
{
    return m_context->findMotionUUID(motion);
}

bool Project::containsModel(const IModel *model) const
{
    return modelUUID(model) != kNullUUID;
}

bool Project::containsMotion(const IMotion *motion) const
{
    return motionUUID(motion) != kNullUUID;
}

bool Project::isDirty() const
{
    return m_context->dirty;
}

void Project::setDirty(bool value)
{
    m_context->dirty = value;
}

void Project::addModel(IModel *model, IRenderEngine *engine, const UUID &uuid)
{
    if (!containsModel(model)) {
        switch (model->type()) {
        case IModel::kAsset:
            m_context->assets[uuid] = model;
            break;
        case IModel::kPMD:
        case IModel::kPMX:
            m_context->models[uuid] = model;
            break;
        default:
            return;
        }
        Scene::addModel(model, engine);
        setDirty(true);
    }
}

void Project::addMotion(IMotion *motion, const UUID &uuid)
{
    if (!containsMotion(motion)) {
        m_context->motions[uuid] = motion;
        Scene::addMotion(motion);
        setDirty(true);
    }
}

void Project::removeModel(IModel *model)
{
    if (m_context->removeModel(model))
        setDirty(true);
}

void Project::removeMotion(IMotion *motion)
{
    if (m_context->removeMotion(motion)) {
        Scene::removeMotion(motion);
        setDirty(true);
    }
}

void Project::setGlobalSetting(const std::string &key, const std::string &value)
{
    m_context->globalSettings[key] = value;
    setDirty(true);
}

void Project::setModelSetting(const IModel *model, const std::string &key, const std::string &value)
{
    if (containsModel(model)) {
        switch (model->type()) {
        case IModel::kAsset:
            m_context->localAssetSettings[model][key] = value;
            break;
        case IModel::kPMD:
        case IModel::kPMX:
            m_context->localModelSettings[model][key] = value;
            break;
        default:
            return;
        }
        setDirty(true);
    }
}

bool Project::save0(xmlTextWriterPtr ptr)
{
    bool ret = m_context->save(ptr);
    xmlFreeTextWriter(ptr);
    if (ret)
        setDirty(false);
    return ret;
}

bool Project::validate(bool result)
{
    return result && m_context->depth == 0 && m_context->checkDuplicateUUID();
}

} /* namespace vpvl */
