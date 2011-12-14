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

#include <libxml2/libxml/xmlwriter.h>

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
        kIKMotion,
        kAssetMotion,
        kAnimation,
        kBoneMotion,
        kVerticesMotion,
        kCameraMotion,
        kLightMotion
    };
    typedef std::map<std::string, std::string> StringMap;
    const static int kAttributeBufferSize = 32;
    const static int kElementContentBufferSize = 128;
    const static std::string kEmpty;

    Handler(Project::IDelegate *delegate)
        : delegate(delegate),
          currentAsset(0),
          currentModel(0),
          currentMotion(0),
          state(kInitial),
          depth(0),
          version(0.0f),
          enablePhysics(false)
    {
    }
    ~Handler() {
        state = kInitial;
        depth = 0;
        assets.releaseAll();
        models.releaseAll();
        motions.releaseAll();
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
    bool containsAsset(Asset *asset) const {
        int nassets = assets.count();
        for (int i = 0; i < nassets; i++) {
            if (assets.at(i) == asset)
                return true;
        }
        return false;
    }
    bool containsModel(PMDModel *model) const {
        int nassets = models.count();
        for (int i = 0; i < nassets; i++) {
            if (models.at(i) == model)
                return true;
        }
        return false;
    }
    bool containsMotion(VMDMotion *motion) const {
        int nmotions = motions.count();
        for (int i = 0; i < nmotions; i++) {
            if (motions.at(i) == motion)
                return true;
        }
        return false;
    }
    bool writeStringMap(const xmlChar *prefix, const StringMap &map, xmlTextWriterPtr &writer) {
        for (StringMap::const_iterator it = map.begin(); it != map.end(); it++) {
            if (it->first.empty() || it->second.empty())
                continue;
            VPVL_XML_RC(xmlTextWriterStartElementNS(writer, prefix, VPVL_CAST_XC("value"), 0));
            VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("name"), VPVL_CAST_XC(it->first.c_str())));
            VPVL_XML_RC(xmlTextWriterWriteString(writer, VPVL_CAST_XC(it->second.c_str())))
                    VPVL_XML_RC(xmlTextWriterEndElement(writer)); /* vpvl:value */
        }
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
        internal::snprintf(buffer, sizeof(buffer), "%.1f", Project::kCurrentVersion);
        VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("version"), VPVL_CAST_XC(buffer)));
        VPVL_XML_RC(xmlTextWriterStartElementNS(writer, kPrefix, VPVL_CAST_XC("settings"), 0));
        if(!writeStringMap(kPrefix, globalSettings, writer))
            return false;
        VPVL_XML_RC(xmlTextWriterEndElement(writer)); /* vpvl:setting */
        VPVL_XML_RC(xmlTextWriterStartElementNS(writer, kPrefix, VPVL_CAST_XC("physics"), 0));
        const xmlChar *isPhysicsEnabled = enablePhysics ? VPVL_CAST_XC("true") : VPVL_CAST_XC("false");
        VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("enabled"), isPhysicsEnabled));
        VPVL_XML_RC(xmlTextWriterEndElement(writer)); /* vpvl:physics */
        VPVL_XML_RC(xmlTextWriterStartElementNS(writer, kPrefix, VPVL_CAST_XC("models"), 0));
        int nmodels = models.count();
        for (int i = 0; i < nmodels; i++) {
            PMDModel *model = models.at(i);
            const std::string &name = delegate->toUnicode(std::string(reinterpret_cast<const char *>(model->name())));
            VPVL_XML_RC(xmlTextWriterStartElementNS(writer, kPrefix, VPVL_CAST_XC("model"), 0));
            VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("name"), VPVL_CAST_XC(name.c_str())));
            if(!writeStringMap(kPrefix, localModelSettings[model], writer))
                return false;
            VPVL_XML_RC(xmlTextWriterEndElement(writer)); /* vpvl:model */
        }
        VPVL_XML_RC(xmlTextWriterEndElement(writer)); /* vpvl:models */
        VPVL_XML_RC(xmlTextWriterStartElementNS(writer, kPrefix, VPVL_CAST_XC("assets"), 0));
        int nassets = assets.count();
        for (int i = 0; i < nassets; i++) {
            Asset *asset = assets.at(i);
            const std::string &name = delegate->toUnicode(std::string(asset->name() ? asset->name() : ""));
            VPVL_XML_RC(xmlTextWriterStartElementNS(writer, kPrefix, VPVL_CAST_XC("asset"), 0));
            VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("name"), VPVL_CAST_XC(name.c_str())));
            if(!writeStringMap(kPrefix, localAssetSettings[asset], writer))
                return false;
            VPVL_XML_RC(xmlTextWriterEndElement(writer)); /* vpvl:asset */
        }
        VPVL_XML_RC(xmlTextWriterEndElement(writer)); /* vpvl:asset */
        VPVL_XML_RC(xmlTextWriterStartElementNS(writer, kPrefix, VPVL_CAST_XC("motions"), 0));
        int nmotions = motions.count(), nframes = 0;
        Quaternion ix, iy, iz, ir, ifv, idt;
        for (int i = 0; i < nmotions; i++) {
            VMDMotion *motion = motions.at(i);
            VPVL_XML_RC(xmlTextWriterStartElementNS(writer, kPrefix, VPVL_CAST_XC("motion"), 0));
            VPVL_XML_RC(xmlTextWriterStartElementNS(writer, kPrefix, VPVL_CAST_XC("animation"), 0));
            VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("type"), VPVL_CAST_XC("bone")));
            const BoneAnimation &ba = motion->boneAnimation();
            nframes = ba.countKeyFrames();
            for (int j = 0; j < nframes; j++) {
                const BoneKeyFrame *frame = ba.frameAt(j);
                const std::string &name = delegate->toUnicode(std::string(reinterpret_cast<const char *>(frame->name())));
                VPVL_XML_RC(xmlTextWriterStartElementNS(writer, kPrefix, VPVL_CAST_XC("keyframe"), 0));
                VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("name"), VPVL_CAST_XC(name.c_str())));
                internal::snprintf(buffer, sizeof(buffer), "%d", static_cast<int>(frame->frameIndex()));
                VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("index"), VPVL_CAST_XC(buffer)));
                const Vector3 &position = frame->position();
                internal::snprintf(buffer, sizeof(buffer), "%.f,%.f,%.f", position.x(), position.y(), -position.z());
                VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("position"), VPVL_CAST_XC(buffer)));
                const Quaternion &rotation = frame->rotation();
                internal::snprintf(buffer, sizeof(buffer), "%.f,%.f,%.f,%.f",
                                   -rotation.x(), -rotation.y(), rotation.z(), rotation.w());
                VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("rotation"), VPVL_CAST_XC(buffer)));
                frame->getInterpolationParameter(BoneKeyFrame::kX, ix);
                frame->getInterpolationParameter(BoneKeyFrame::kY, iy);
                frame->getInterpolationParameter(BoneKeyFrame::kZ, iz);
                frame->getInterpolationParameter(BoneKeyFrame::kRotation, ir);
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
            VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("type"), VPVL_CAST_XC("vertices")));
            const FaceAnimation &fa = motion->faceAnimation();
            nframes = fa.countKeyFrames();
            for (int j = 0; j < nframes; j++) {
                const FaceKeyFrame *frame = fa.frameAt(j);
                const std::string &name = delegate->toUnicode(std::string(reinterpret_cast<const char *>(frame->name())));
                VPVL_XML_RC(xmlTextWriterStartElementNS(writer, kPrefix, VPVL_CAST_XC("keyframe"), 0));
                VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("name"), VPVL_CAST_XC(name.c_str())));
                internal::snprintf(buffer, sizeof(buffer), "%d", static_cast<int>(frame->frameIndex()));
                VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("index"), VPVL_CAST_XC(buffer)));
                internal::snprintf(buffer, sizeof(buffer), "%.2f", frame->weight());
                VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("weight"), VPVL_CAST_XC(buffer)));
                VPVL_XML_RC(xmlTextWriterEndElement(writer));
            }
            VPVL_XML_RC(xmlTextWriterEndElement(writer)); /* animation */
            VPVL_XML_RC(xmlTextWriterStartElementNS(writer, kPrefix, VPVL_CAST_XC("animation"), 0));
            VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("type"), VPVL_CAST_XC("camera")));
            const CameraAnimation &ca = motion->cameraAnimation();
            nframes = ca.countKeyFrames();
            for (int j = 0; j < nframes; j++) {
                const CameraKeyFrame *frame = ca.frameAt(j);
                VPVL_XML_RC(xmlTextWriterStartElementNS(writer, kPrefix, VPVL_CAST_XC("keyframe"), 0));
                internal::snprintf(buffer, sizeof(buffer), "%d", static_cast<int>(frame->frameIndex()));
                VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("index"), VPVL_CAST_XC(buffer)));
                const Vector3 &position = frame->position();
                internal::snprintf(buffer, sizeof(buffer), "%.f,%.f,%.f", position.x(), position.y(), -position.z());
                VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("position"), VPVL_CAST_XC(buffer)));
                const Vector3 &angle = frame->angle();
                internal::snprintf(buffer, sizeof(buffer), "%.f,%.f,%.f",
                                   vpvl::radian(-angle.x()), vpvl::radian(-angle.y()), vpvl::radian(-angle.z()));
                VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("angle"), VPVL_CAST_XC(buffer)));
                internal::snprintf(buffer, sizeof(buffer), "%.f", frame->fovy());
                VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("fovy"), VPVL_CAST_XC(buffer)));
                internal::snprintf(buffer, sizeof(buffer), "%.f", frame->distance());
                VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("distance"), VPVL_CAST_XC(buffer)));
                frame->getInterpolationParameter(CameraKeyFrame::kX, ix);
                frame->getInterpolationParameter(CameraKeyFrame::kY, iy);
                frame->getInterpolationParameter(CameraKeyFrame::kZ, iz);
                frame->getInterpolationParameter(CameraKeyFrame::kRotation, ir);
                frame->getInterpolationParameter(CameraKeyFrame::kFovy, ifv);
                frame->getInterpolationParameter(CameraKeyFrame::kDistance, idt);
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
            nframes = la.countKeyFrames();
            for (int j = 0; j < nframes; j++) {
                const LightKeyFrame *frame = la.frameAt(j);
                VPVL_XML_RC(xmlTextWriterStartElementNS(writer, kPrefix, VPVL_CAST_XC("keyframe"), 0));
                internal::snprintf(buffer, sizeof(buffer), "%d", static_cast<int>(frame->frameIndex()));
                VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("index"), VPVL_CAST_XC(buffer)));
                const Vector3 &color = frame->color();
                internal::snprintf(buffer, sizeof(buffer), "%.f,%.f,%.f", color.x(), color.y(), color.z());
                VPVL_XML_RC(xmlTextWriterWriteAttribute(writer, VPVL_CAST_XC("color"), VPVL_CAST_XC(buffer)));
                const Vector3 &direction = frame->direction();
                internal::snprintf(buffer, sizeof(buffer), "%.f,%.f,%.f", direction.x(), direction.y(), direction.z());
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
                    newString(attributes, i, value);
                    self->version = internal::stringToFloat(value.c_str());
                }
            }
            self->pushState(kProject);
        }
        else if (self->depth == 1 && self->state == kProject) {
            if (equals(prefix, localname, "settings")) {
                self->pushState(kSettings);
            }
            else if (equals(prefix, localname, "physics")) {
                for (int i = 0; i < nattributes; i++, index += 5) {
                    if (equals(attributes[index], "enabled")) {
                        newString(attributes, i, value);
                        self->enablePhysics = value == "true";
                    }
                }
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
                        newString(attributes, i, value);
                        self->key = value;
                    }
                }
            }
            if (self->state == kModels && equals(prefix, localname, "model")) {
                self->currentModel = new PMDModel();
                self->pushState(kModel);
            }
            else if (self->state == kAssets && equals(prefix, localname, "asset")) {
                self->currentAsset = new Asset();
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
            if ((self->state == kModel || self->state == kAsset) && equals(prefix, localname, "value")) {
                for (int i = 0; i < nattributes; i++, index += 5) {
                    if (equals(attributes[index], "name")) {
                        newString(attributes, i, value);
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
            else if (equals(prefix, localname, "keyframe")) {
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
            Vector3 vec3(0, 0, 0);
            Vector4 vec4(0, 0, 0, 0);
            QuadWord qw(0, 0, 0, 0);
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
        Handler *self = static_cast<Handler *>(context);
        if (self->state == kSettings) {
            std::string value(reinterpret_cast<const char *>(character), len);
            self->globalSettings[self->key] = value;
        }
        else if (self->state == kModel) {
            std::string value(reinterpret_cast<const char *>(character), len);
            self->localModelSettings[self->currentModel][self->key] = value;
        }
        else if (self->state == kAsset) {
            std::string value(reinterpret_cast<const char *>(character), len);
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
                    self->assets.add(self->currentAsset);
                    self->currentAsset = 0;
                    self->popState(kAssets);
                }
                self->key = "";
                break;
            case kModel:
                if (equals(prefix, localname, "model")) {
                    self->models.add(self->currentModel);
                    self->currentModel = 0;
                    self->popState(kModels);
                }
                self->key = "";
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
    Array<Asset *> assets;
    Array<PMDModel *> models;
    Array<VMDMotion *> motions;
    StringMap globalSettings;
    std::map<PMDModel *, StringMap> localModelSettings;
    std::map<Asset *, StringMap> localAssetSettings;
    std::string key;
    Asset *currentAsset;
    PMDModel *currentModel;
    VMDMotion *currentMotion;
    State state;
    float version;
    int depth;
    bool enablePhysics;
};

const std::string Handler::kEmpty = "";
const float Project::kCurrentVersion = 0.1f;

Project::Project(IDelegate *delegate)
    : m_handler(0)
{
    internal::zerofill(&m_sax, sizeof(m_sax));
    m_handler = new Handler(delegate);
    m_sax.initialized = XML_SAX2_MAGIC;
    m_sax.startElementNs = &Handler::startElement;
    m_sax.endElementNs = &Handler::endElement;
    m_sax.characters = &Handler::characters;
    m_sax.warning = &Handler::warning;
    m_sax.error = &Handler::error;
}

Project::~Project()
{
    internal::zerofill(&m_sax, sizeof(m_sax));
    delete m_handler;
    m_handler = 0;
}

bool Project::load(const char *path)
{
    return xmlSAXUserParseFile(&m_sax, m_handler, path) == 0 && m_handler->depth == 0;
}

bool Project::load(const uint8_t *data, size_t size)
{
    return xmlSAXUserParseMemory(&m_sax, m_handler, reinterpret_cast<const char *>(data), size) == 0;
}

bool Project::save(const char *path)
{
    xmlTextWriterPtr ptr = xmlNewTextWriterFilename(path, 0);
    bool ret = m_handler->save(ptr);
    xmlFreeTextWriter(ptr);
    return ret;
}

bool Project::save(xmlBufferPtr &buffer)
{
    xmlTextWriterPtr ptr = xmlNewTextWriterMemory(buffer, 0);
    bool ret = m_handler->save(ptr);
    xmlFreeTextWriter(ptr);
    return ret;
}

float Project::version() const
{
    return m_handler->version;
}

bool Project::isPhysicsEnabled() const
{
    return m_handler->enablePhysics;
}

const Array<Asset *> &Project::assets() const
{
    return m_handler->assets;
}

const Array<PMDModel *> &Project::models() const
{
    return m_handler->models;
}

const Array<VMDMotion *> &Project::motions() const
{
    return m_handler->motions;
}

const std::string &Project::globalSetting(const std::string &key) const
{
    return m_handler->globalSettings[key];
}

const std::string &Project::localAssetSetting(Asset *asset, const std::string &key) const
{
    return containsAsset(asset) ? m_handler->localAssetSettings[asset][key] : Handler::kEmpty;
}

const std::string &Project::localModelSetting(PMDModel *model, const std::string &key) const
{
    return containsModel(model) ? m_handler->localModelSettings[model][key] : Handler::kEmpty;
}

bool Project::containsAsset(Asset *asset) const
{
    return m_handler->containsAsset(asset);
}

bool Project::containsModel(PMDModel *model) const
{
    return m_handler->containsModel(model);
}

bool Project::containsMotion(VMDMotion *motion) const
{
    return m_handler->containsMotion(motion);
}

void Project::addAsset(Asset *asset)
{
    m_handler->assets.add(asset);
}

void Project::addModel(PMDModel *model)
{
    m_handler->models.add(model);
}

void Project::addMotion(VMDMotion *motion)
{
    m_handler->motions.add(motion);
}

void Project::removeAsset(Asset *&asset)
{
    m_handler->assets.remove(asset);
    delete asset;
    asset = 0;
}

void Project::removeModel(PMDModel *&model)
{
    m_handler->models.remove(model);
    delete model;
    model = 0;
}

void Project::removeMotion(VMDMotion *&motion)
{
    m_handler->motions.remove(motion);
    delete motion;
    motion = 0;
}

void Project::setPhysicsEnable(bool value)
{
    m_handler->enablePhysics = value;
}

void Project::setGlobalSetting(const std::string &key, std::string &value)
{
    m_handler->globalSettings[key] = value;
}

void Project::setLocalAssetSetting(Asset *asset, const std::string &key, const std::string &value)
{
    if (containsAsset(asset))
        m_handler->localAssetSettings[asset][key] = value;
}

void Project::setLocalModelSetting(PMDModel *model, const std::string &key, const std::string &value)
{
    if (containsModel(model))
        m_handler->localModelSettings[model][key] = value;
}

} /* namespace vpvl */
