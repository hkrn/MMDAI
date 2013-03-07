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

#pragma once
#ifndef VPVL2_EXTENSIONS_POSE_H_
#define VPVL2_EXTENSIONS_POSE_H_

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

class Pose {
public:
    Pose(IEncoding *encoding)
        : m_encoding(encoding)
    {
    }
    ~Pose() {
        m_bones.releaseAll();
        m_morphs.releaseAll();
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
    void save(std::ostringstream & /* stream */) {
        // NOT IMPLEMENTED HERE
    }
    void bind(IModel *model) {
        if (model) {
            const int nbones = m_bones.count();
            for (int i = 0; i < nbones; i++) {
                Bone *target = m_bones[i];
                if (IBone *bone = model->findBone(target->name)) {
                    bone->setLocalPosition(target->position);
                    bone->setLocalRotation(target->rotation);
                }
            }
            const int nmorphs = m_morphs.count();
            for (int i = 0; i < nmorphs; i++) {
                Morph *target = m_morphs[i];
                if (IMorph *morph = model->findMorph(target->name)) {
                    morph->setWeight(target->weight);
                }
            }
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
    struct Bone {
        Bone(IString *n, const Vector3 &p, const Quaternion &r)
            : name(n),
              position(p),
              rotation(r)
        {
        }
        ~Bone() {
            delete name;
            name = 0;
        }
        IString *name;
        const Vector3 position;
        const Quaternion rotation;
    };
    struct Morph {
        Morph(IString *n, const IMorph::WeightPrecision w)
            : name(n),
              weight(w)
        {
        }
        ~Morph() {
            delete name;
            name = 0;
        }
        IString *name;
        const IMorph::WeightPrecision weight;
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
            IString *n = m_encoding->toString(reinterpret_cast<const uint8_t *>(name.c_str()),
                                              name.length(), IString::kUTF8);
            m_bones.append(new Bone(n, position, rotation));
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
            std::getline(s, index, '{'); // Bone[0-9]*
            if (strncmp(index.c_str(), kMorphName, strlen(kMorphName)) != 0) {
                break;
            }
            std::getline(s, name);
            if (name.empty()) {
                return false;
            }
            getLine(stream, wstr);
            getValue(wstr, weight);
            IString *n = m_encoding->toString(reinterpret_cast<const uint8_t *>(name.c_str()),
                                              name.length(), IString::kUTF8);
            m_morphs.append(new Morph(n, weight));
            getLine(stream, unused); // }
            getLine(stream, m_currentLine);
            if (m_currentLine.empty()) {
                break;
            }
        }
        return true;
    }

    IEncoding *m_encoding;
    PointerArray<Bone> m_bones;
    PointerArray<Morph> m_morphs;
    std::string m_currentLine;
};

} /* namespace extensions */
} /* namespace vpvl2 */

#endif
