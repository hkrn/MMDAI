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

#pragma once
#ifndef VPVL2_EXTENSIONS_POSE_H_
#define VPVL2_EXTENSIONS_POSE_H_

#include <string.h> /* for strlen */
#include <string>
#include <sstream>

#include <vpvl2/IBone.h>
#include <vpvl2/IEncoding.h>
#include <vpvl2/IModel.h>
#include <vpvl2/IMorph.h>
#include <vpvl2/IString.h>

namespace vpvl2
{
namespace extensions
{

class Pose VPVL2_DECL_FINAL {
public:
    static const IString::Codec kDefaultCodec = IString::kShiftJIS;
    class Bone {
    public:
        virtual ~Bone() {}
        virtual const IString *name() const = 0;
        virtual Vector3 translation() const = 0;
        virtual Quaternion rotation() const = 0;
    };
    class Morph {
    public:
        virtual ~Morph() {}
        virtual const IString *name() const = 0;
        virtual IMorph::WeightPrecision weight() const = 0;
    };

    Pose(IEncoding *encoding)
        : m_encoding(encoding)
    {
    }
    ~Pose() {
        m_bones.releaseAll();
        m_morphs.releaseAll();
    }

    Pose *clone() const {
        Pose *pose = new Pose(m_encoding);
        const int nbones = m_bones.count();
        for (int i = 0; i < nbones; i++) {
            BoneImpl *bone = m_bones[i];
            pose->m_bones.append(bone->clone());
        }
        const int nmorphs = m_morphs.count();
        for (int i = 0; i < nmorphs; i++) {
            MorphImpl *morph = m_morphs[i];
            pose->m_morphs.append(morph->clone());
        }
        return pose;
    }
    bool load(std::istringstream &stream) {
        getLine(stream, m_currentLine);
        if (m_currentLine != "Vocaloid Pose Data file") { // signature
            return false;
        }
        getLine(stream, m_currentLine); // model name
        getLine(stream, m_currentLine); // bone count
        if (!parseBones(stream)) {
            return false;
        }
        if (!parseMorph(stream)) {
            return false;
        }
        return true;
    }
    void save(std::ostringstream &stream, const IModel *model) const {
        if (model) {
            if (const IString *name = model->name(IEncoding::kJapanese)) {
                stream << "Vocaloid Pose Data file\r\n\r\n";
                uint8 *modelName = m_encoding->toByteArray(name, kDefaultCodec);
                stream << modelName << "\r\n";
                m_encoding->disposeByteArray(modelName);
                writeBones(stream, model);
                writeMorphs(stream, model);
            }
        }
    }
    void bind(IModel *model) {
        if (model) {
            const int nbones = m_bones.count();
            for (int i = 0; i < nbones; i++) {
                BoneImpl *target = m_bones[i];
                if (IBone *bone = model->findBoneRef(target->name())) {
                    bone->setLocalTranslation(target->translation());
                    bone->setLocalOrientation(target->rotation());
                }
            }
            const int nmorphs = m_morphs.count();
            for (int i = 0; i < nmorphs; i++) {
                MorphImpl *target = m_morphs[i];
                if (IMorph *morph = model->findMorphRef(target->name())) {
                    morph->setWeight(target->weight());
                }
            }
        }
    }
    void getBones(Array<const Bone *> &bones) const {
        const int nbones = m_bones.count();
        bones.clear();
        bones.reserve(nbones);
        for (int i = 0; i < nbones; i++) {
            BoneImpl *bone = m_bones[i];
            bones.append(bone);
        }
    }
    void getMorphs(Array<const Morph *> &morphs) const {
        const int nmorphs = m_morphs.count();
        morphs.clear();
        morphs.reserve(nmorphs);
        for (int i = 0; i < nmorphs; i++) {
            MorphImpl *morph = m_morphs[i];
            morphs.append(morph);
        }
    }

    static std::string trim(const std::string &value) {
        std::string::const_iterator stringFrom = value.begin(), stringTo = value.end() - 1;
        while (isspace(*stringFrom) && (stringFrom != value.end()))
            ++stringFrom;
        while (isspace(*stringTo) && (stringTo != value.begin()))
            --stringTo;
        return (stringTo - stringFrom >= 0) ? std::string(stringFrom, ++stringTo) : std::string();
    }
    static void getLine(std::istringstream &stream, std::string &nextLine) {
        std::string line;
        do {} while (std::getline(stream, line) && (line.empty() || line[0] == '\r'));
        if (line.length() > 0) {
            std::string::size_type lastPosition = line.length() - 1;
            if (line.at(lastPosition) == '\r') {
                line.erase(lastPosition);
            }
            nextLine.assign(trim(line));
        }
        else {
            nextLine.assign(std::string());
        }
    }
    template<typename T>
    static void getValue(const std::string &line, T &value) {
        std::istringstream stream(line);
        stream >> value;
    }

private:
    class BoneImpl : public Bone {
    public:
        BoneImpl(IString *name, const Vector3 &position, const Quaternion &rotation)
            : m_name(name),
              m_position(position),
              m_rotation(rotation)
        {
        }
        ~BoneImpl() {
            delete m_name;
            m_name = 0;
        }
        BoneImpl *clone() const {
            return new BoneImpl(m_name->clone(), m_position, m_rotation);
        }
        const IString *name() const { return m_name; }
        Vector3 translation() const { return m_position; }
        Quaternion rotation() const { return m_rotation; }
    private:
        IString *m_name;
        const Vector3 m_position;
        const Quaternion m_rotation;
    };
    class MorphImpl : public Morph {
    public:
        MorphImpl(IString *name, const IMorph::WeightPrecision weight)
            : m_name(name),
              m_weight(weight)
        {
        }
        ~MorphImpl() {
            delete m_name;
            m_name = 0;
        }
        MorphImpl *clone() const {
            return new MorphImpl(m_name->clone(), m_weight);
        }
        const IString *name() const { return m_name; }
        IMorph::WeightPrecision weight() const { return m_weight; }
    private:
        IString *m_name;
        const IMorph::WeightPrecision m_weight;
    };

    template<typename T>
    void getValue(std::istringstream &stream, T &value) {
        std::string token;
        std::getline(stream, token, ',');
        getValue(token, value);
    }
    bool parseBones(std::istringstream &stream) {
        std::string index, name, pstr, rstr, unused;
        Vector3 position;
        Quaternion rotation;
        getLine(stream, m_currentLine);
        while (true) {
            static const char *const kBoneName = "Bone";
            std::istringstream s(m_currentLine);
            std::getline(s, index, '{'); // Bone[0-9]*
            if (strncmp(index.c_str(), kBoneName, strlen(kBoneName)) != 0) {
                break;
            }
            std::getline(s, name);
            if (name.empty()) {
                return false;
            }
            getLine(stream, pstr);
            getLine(stream, rstr);
            std::istringstream pstream(pstr), rstream(rstr);
            float x = 0, y = 0, z = 0, w = 0;
            getValue(pstream, x);
            getValue(pstream, y);
            getValue(pstream, z);
#ifdef VPVL2_COORDINATE_OPENGL
            position.setValue(x, y, -z);
#else
            position.setValue(x, y, z);
#endif
            getValue(rstream, x);
            getValue(rstream, y);
            getValue(rstream, z);
            getValue(rstream, w);
#ifdef VPVL2_COORDINATE_OPENGL
            rotation.setValue(-x, -y, z, w);
#else
            rotation.setValue(x, y, z, w);
#endif
            IString *n = m_encoding->toString(reinterpret_cast<const uint8 *>(name.c_str()),
                                              name.length(), kDefaultCodec);
            m_bones.append(new BoneImpl(n, position, rotation));
            getLine(stream, unused); // }
            getLine(stream, m_currentLine);
            if (m_currentLine.empty()) {
                break;
            }
        }
        return true;
    }
    bool parseMorph(std::istringstream &stream) {
        std::string index, name, wstr, unused;
        IMorph::WeightPrecision weight;
        while (true) {
            static const char *const kMorphName = "Morph";
            std::istringstream s(m_currentLine);
            std::getline(s, index, '{'); // Morph[0-9]*
            if (strncmp(index.c_str(), kMorphName, strlen(kMorphName)) != 0) {
                break;
            }
            std::getline(s, name);
            if (name.empty()) {
                return false;
            }
            getLine(stream, wstr);
            getValue(wstr, weight);
            IString *n = m_encoding->toString(reinterpret_cast<const uint8 *>(name.c_str()),
                                              name.length(), kDefaultCodec);
            m_morphs.append(new MorphImpl(n, weight));
            getLine(stream, unused); // }
            getLine(stream, m_currentLine);
            if (m_currentLine.empty()) {
                break;
            }
        }
        return true;
    }
    void writeBones(std::ostringstream &stream, const IModel *model) const {
        Array<IBone *> rawBones;
        Array<const IBone *> bones;
        model->getBoneRefs(rawBones);
        const int nRawBones = rawBones.count();
        bones.reserve(nRawBones);
        for (int i = 0; i < nRawBones; i++) {
            const IBone *bone = rawBones[i];
            if (bone->isVisible()) {
                bones.append(bone);
            }
        }
        const int nbones = bones.count();
        stream << nbones << "\r\n\r\n";
        for (int i = 0; i < nbones; i++) {
            const IBone *bone = bones[i];
            if (const IString *name = bone->name(IEncoding::kJapanese)) {
                uint8 *boneName = m_encoding->toByteArray(name, kDefaultCodec);
                stream << "Bone" << i << "{" << boneName << "\r\n";
                m_encoding->disposeByteArray(boneName);
                const Vector3 &position = bone->localTranslation();
                stream << "  " << position.x() << "," << position.y() << ","
          #ifdef VPVL2_COORDINATE_OPENGL
                       << -position.z()
          #else
                       << position.z()
          #endif
                       << "\r\n";
                const Quaternion &rotation = bone->localOrientation();
                stream << "  "
          #ifdef VPVL2_COORDINATE_OPENGL
                       << -rotation.x() << "," << -rotation.y()
          #else
                       << rotation.x() << "," << rotation.y()
          #endif
                       << "," << rotation.z() << "," << rotation.w()
                       << "\r\n}\r\n\r\n";
            }
        }
    }
    void writeMorphs(std::ostringstream &stream, const IModel *model) const {
        Array<IMorph *> morphs;
        model->getMorphRefs(morphs);
        const int nmorphs = morphs.count();
        for (int i = 0, morphIndex = 0; i < nmorphs; i++) {
            const IMorph *morph = morphs[i];
            if (const IString *name = morph->name(IEncoding::kJapanese)) {
                uint8 *morphName = m_encoding->toByteArray(name, kDefaultCodec);
                stream << "Morph" << morphIndex << "{" << morphName << "\r\n";
                m_encoding->disposeByteArray(morphName);
                stream << "  " << morph->weight() << ";\r\n}\r\n\r\n";
                morphIndex++;
            }
        }
    }

    IEncoding *m_encoding;
    PointerArray<BoneImpl> m_bones;
    PointerArray<MorphImpl> m_morphs;
    std::string m_currentLine;
};

} /* namespace extensions */
} /* namespace vpvl2 */

#endif
