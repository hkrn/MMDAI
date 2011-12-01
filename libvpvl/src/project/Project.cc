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

namespace vpvl
{

struct Parser {
public:
    enum State {
        kInitial,
        kProject,
        kPreferences,
        kPhysics,
        kModels,
        kModel,
        kAssets,
        kAsset,
        kMotions,
        kBoneMotion,
        kVerticesMotion,
        kCameraMotion,
        kLightMotion,
        kIKMotion,
        kAssetMotion
    };

    Parser()
        : state(kInitial),
          depth(0),
          currentAsset(0),
          currentModel(0),
          currentMotion(0)
    {
    }
    ~Parser() {
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
    }
    void popState(State s) {
        state = s;
        depth--;
    }

    static bool equals(const xmlChar *prefix, const xmlChar *localname, const char *dst) {
        return equals(prefix, "vpvm") && equals(localname, dst);
    }

    static bool equals(const xmlChar *name, const char *dst) {
        return xmlStrcmp(name, reinterpret_cast<const xmlChar *>(dst));
    }

    static void startElement(void *context,
                             const xmlChar *localname,
                             const xmlChar *prefix,
                             const xmlChar * /* URI */,
                             int /* nNamespaces */,
                             const xmlChar ** /* namespaces */,
                             int nAttributes,
                             int /* nDefaulted */,
                             const xmlChar **attributes)
    {
        Parser *self = static_cast<Parser *>(context);
        if (self->depth == 0 && equals(prefix, localname, "project")) {
            self->pushState(kProject);
        }
        else if (self->depth == 1 && self->state == kProject) {
            if (equals(prefix, localname, "preferences")) {
                self->pushState(kPreferences);
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
            if (self->state == kPreferences) {
            }
            if (self->state == kModels && equals(prefix, localname, "model")) {
                self->currentModel = new vpvl::PMDModel();
                self->pushState(kModel);
            }
            else if (self->state == kAssets && equals(prefix, localname, "asset")) {
                self->currentAsset = new vpvl::Asset();
                self->pushState(kAsset);
            }
            else if (self->state == kMotions && equals(prefix, localname, "motion")) {
                self->currentMotion = new vpvl::VMDMotion();
                for (int i = 0; i < nAttributes; i++) {
                    const xmlChar *attribute = attributes[i];
                    if (equals(attribute, "bone")) {
                        self->pushState(kBoneMotion);
                    }
                    else if (equals(attribute, "vertices")) {
                        self->pushState(kVerticesMotion);
                    }
                    else if (equals(attribute, "camera")) {
                        self->pushState(kCameraMotion);
                    }
                    else if (equals(attribute, "light")) {
                        self->pushState(kLightMotion);
                    }
                    else if (equals(attribute, "ik")) {
                        self->pushState(kIKMotion);
                    }
                    else if (equals(attribute, "asset")) {
                        self->pushState(kAssetMotion);
                    }
                }
            }
        }
        else if (self->depth == 3) {
            if (self->state == kModel) {
            }
            else if (self->state = kAsset) {
            }
            else if (equals(localname, "keyframe")) {
                switch (self->state) {
                case kBoneMotion:
                    break;
                case kVerticesMotion:
                    break;
                case kCameraMotion:
                    break;
                case kLightMotion:
                    break;
                case kIKMotion:
                    break;
                case kAssetMotion:
                    break;
                }
            }
        }
    }
    static void endElement(void *context,
                           const xmlChar * /* localname */,
                           const xmlChar * /* prefix */,
                           const xmlChar * /* URI */)
    {
        Parser *self = static_cast<Parser *>(context);
        if (self->depth == 3) {
            switch (self->state) {
            case kAsset:
                self->assets.add(self->currentAsset);
                self->currentAsset = 0;
                self->popState(kAssets);
                break;
            case kModel:
                self->models.add(self->currentModel);
                self->currentModel = 0;
                self->popState(kModels);
                break;
            case kBoneMotion:
            case kVerticesMotion:
            case kCameraMotion:
            case kLightMotion:
            case kIKMotion:
            case kAssetMotion:
                self->motions.add(self->currentMotion);
                self->currentMotion = 0;
                self->popState(kMotions);
                break;
            default:
                break;
            }
        }
        else if (self->depth == 2) {
            switch (self->state) {
            case kAssets:
                self->popState(kProject);
                break;
            case kModels:
                self->popState(kProject);
                break;
            case kMotions:
                self->popState(kProject);
                break;
            case kPreferences:
            case kPhysics:
                self->popState(kProject);
                break;
            default:
                break;
            }
        }
        else if (self->depth == 1 && self->state == kProject) {
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

    Array<Asset *> assets;
    Array<PMDModel *> models;
    Array<VMDMotion *> motions;
    Hash<HashString, char *> globalPreferences;
    Hash<btHashPtr, char *> localModelPreference;
    Hash<btHashPtr, char *> localAssetPreference;
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
    return xmlSAXUserParseFile(&m_handler, m_parser, path) == 0;
}

bool Project::load(const uint8_t *data, size_t size)
{
    return xmlSAXUserParseMemory(&m_handler, m_parser, reinterpret_cast<const char *>(data), size) == 0;
}

void Project::save(const char * /* path */)
{
}

} /* namespace vpvl */

