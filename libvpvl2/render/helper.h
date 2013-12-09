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

#include <vpvl2/vpvl2.h>
#include <vpvl2/extensions/Archive.h>
#include <vpvl2/extensions/BaseApplicationContext.h>
#include <vpvl2/extensions/World.h>
#include <vpvl2/extensions/StringMap.h>
#include <vpvl2/extensions/icu4c/Encoding.h>

#ifdef VPVL2_OS_OSX
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>

namespace vpvl2 {
namespace extensions {
namespace icu4c {
VPVL2_MAKE_SMARTPTR(Encoding);
}
}
}

using namespace vpvl2;
using namespace vpvl2::extensions;

namespace ui {

static void drawScreen(const Scene &scene)
{
    Array<IRenderEngine *> enginesForPreProcess, enginesForStandard, enginesForPostProcess;
    Hash<HashPtr, IEffect *> nextPostEffects;
    scene.getRenderEnginesByRenderOrder(enginesForPreProcess,
                                        enginesForStandard,
                                        enginesForPostProcess,
                                        nextPostEffects);
    for (int i = enginesForPostProcess.count() - 1; i >= 0; i--) {
        IRenderEngine *engine = enginesForPostProcess[i];
        engine->preparePostProcess();
    }
    for (int i = 0, nengines = enginesForPreProcess.count(); i < nengines; i++) {
        IRenderEngine *engine = enginesForPreProcess[i];
        engine->performPreProcess();
    }
    for (int i = 0, nengines = enginesForStandard.count(); i < nengines; i++) {
        IRenderEngine *engine = enginesForStandard[i];
        engine->renderModel();
        engine->renderEdge();
        if (!scene.shadowMapRef()) {
            engine->renderShadow();
        }
    }
    for (int i = 0, nengines = enginesForPostProcess.count(); i < nengines; i++) {
        IRenderEngine *engine = enginesForPostProcess[i];
        IEffect *const *nextPostEffect = nextPostEffects[engine];
        engine->performPostProcess(*nextPostEffect);
    }
}

static void loadSettings(const std::string &path, StringMap &settings)
{
    std::ifstream stream(path.c_str());
    std::string line;
    UnicodeString k, v;
    while (stream && std::getline(stream, line)) {
        if (line.empty() || line.find_first_of("#;") != std::string::npos) {
            continue;
        }
        std::istringstream ss(line);
        std::string key, value;
        std::getline(ss, key, '=');
        std::getline(ss, value);
        k.setTo(UnicodeString::fromUTF8(key));
        v.setTo(UnicodeString::fromUTF8(value));
        settings[icu4c::String::toStdString(k.trim())] = icu4c::String::toStdString(v.trim());
    }
}

static void initializeDictionary(const StringMap &settings, icu4c::Encoding::Dictionary &dictionary)
{
    dictionary.insert(IEncoding::kArm, icu4c::String::create(settings.value("encoding.constant.arm", std::string())));
    dictionary.insert(IEncoding::kAsterisk, icu4c::String::create(settings.value("encoding.constant.asterisk", std::string("*"))));
    dictionary.insert(IEncoding::kCenter, icu4c::String::create(settings.value("encoding.constant.center", std::string())));
    dictionary.insert(IEncoding::kElbow, icu4c::String::create(settings.value("encoding.constant.elbow", std::string())));
    dictionary.insert(IEncoding::kFinger, icu4c::String::create(settings.value("encoding.constant.finger", std::string())));
    dictionary.insert(IEncoding::kLeft, icu4c::String::create(settings.value("encoding.constant.left", std::string())));
    dictionary.insert(IEncoding::kLeftKnee, icu4c::String::create(settings.value("encoding.constant.leftknee", std::string())));
    dictionary.insert(IEncoding::kOpacityMorphAsset, icu4c::String::create(settings.value("encoding.constant.opacityMorphAsset", std::string())));
    dictionary.insert(IEncoding::kRight, icu4c::String::create(settings.value("encoding.constant.right", std::string())));
    dictionary.insert(IEncoding::kRightKnee, icu4c::String::create(settings.value("encoding.constant.rightknee", std::string())));
    dictionary.insert(IEncoding::kRootBone, icu4c::String::create(settings.value("encoding.constant.rootBoneAsset", std::string())));
    dictionary.insert(IEncoding::kScaleBoneAsset, icu4c::String::create(settings.value("encoding.constant.scaleBoneAsset", std::string())));
    dictionary.insert(IEncoding::kSPAExtension, icu4c::String::create(settings.value("encoding.constant.spa", std::string(".spa"))));
    dictionary.insert(IEncoding::kSPHExtension, icu4c::String::create(settings.value("encoding.constant.sph", std::string(".sph"))));
    dictionary.insert(IEncoding::kWrist, icu4c::String::create(settings.value("encoding.constant.wrist", std::string())));
}

static bool loadModel(const UnicodeString &path,
                      BaseApplicationContext *applicationContextRef,
                      Factory *factoryRef,
                      IEncoding *encodingRef,
                      ArchiveSmartPtr &archive,
                      IModelSmartPtr &model)
{
    static const UnicodeString kPMDExtension(".pmd"), kPMXExtension(".pmx");
    BaseApplicationContext::MapBuffer buffer(applicationContextRef);
    bool ok = false;
    if (path.endsWith(".zip")) {
        archive.reset(new Archive(encodingRef));
        vpvl2::extensions::Archive::EntryNames entries;
        icu4c::String s(path);
        if (archive->open(&s, entries)) {
            for (Archive::EntryNames::const_iterator it = entries.begin(); it != entries.end(); it++) {
                const UnicodeString &filename = UnicodeString::fromUTF8(*it);
                if (filename.endsWith(kPMDExtension) || filename.endsWith(kPMXExtension)) {
                    archive->uncompressEntry(*it);
                    int offset = filename.lastIndexOf('/');
                    const std::string *bytes = archive->dataRef(*it);
                    const uint8 *data = reinterpret_cast<const uint8 *>(bytes->data());
                    archive->setBasePath(icu4c::String::toStdString(filename.tempSubString(0, offset)));
                    model.reset(factoryRef->createModel(data, bytes->size(), ok));
                    break;
                }
            }
        }
    }
    else if (applicationContextRef->mapFile(icu4c::String::toStdString(path), &buffer)) {
        model.reset(factoryRef->createModel(buffer.address, buffer.size, ok));
    }
    return ok && model.get() != 0;
}

static void loadAllModels(const StringMap &settings,
                          BaseApplicationContext *applicationContextRef,
                          Scene *sceneRef,
                          Factory *factoryRef,
                          IEncoding *encodingRef)
{
    const std::string &globalMotionPath = settings.value("file.motion", std::string());
    int nmodels = settings.value("models/size", 0);
    bool parallel = settings.value("enable.parallel", true), ok = false;
    ArchiveSmartPtr archive;
    IModelSmartPtr model;
    std::ostringstream stream;
    if (settings.value("enable.vss", false)) {
        sceneRef->setAccelerationType(Scene::kVertexShaderAccelerationType1);
    }
    else if (settings.value("enable.opencl", false)) {
        sceneRef->setAccelerationType(Scene::kOpenCLAccelerationType1);
    }
    for (int i = 0; i < nmodels; i++) {
        stream.str(std::string());
        stream << "models/" << (i + 1);
        const std::string &prefix = stream.str(), &path = settings.value(prefix + "/path", std::string());
        const UnicodeString &modelPath = UnicodeString::fromUTF8(path);
        archive.reset();
        model.reset();
        VPVL2_VLOG(2, "Loading a model from " << path);
        int flags = settings.value(prefix + "/enable.effects", true) ? Scene::kEffectCapable : 0;
        int indexOf = modelPath.lastIndexOf("/");
        icu4c::String dir(modelPath.tempSubString(0, indexOf));
        if (loadModel(modelPath, applicationContextRef, factoryRef, encodingRef, archive, model)) {
            BaseApplicationContext::ModelContext modelContext(applicationContextRef, archive.get(), &dir);
            IRenderEngineSmartPtr engine(sceneRef->createRenderEngine(applicationContextRef, model.get(), flags));
            IEffect *effectRef = 0;
            /*
             * BaseRenderContext#addModelPath() must be called before BaseRenderContext#createEffectRef()
             * because BaseRenderContext#createEffectRef() depends on BaseRenderContext#addModelPath() result
             * by BaseRenderContext#findModelPath() via BaseRenderContext#effectFilePath()
             */
            applicationContextRef->addModelFilePath(model.get(), icu4c::String::toStdString(modelPath));
            if ((flags & Scene::kEffectCapable) != 0) {
                //effectRef = applicationContextRef->createEffectRef(model.get(), &dir);
                //if (effectRef) {
                //    effectRef->createFrameBufferObject();
                engine->setEffect(effectRef, IEffect::kAutoDetection, &modelContext);
                //}
                effectRef = engine->effectRef(IEffect::kDefault);
            }
            if (engine->upload(&modelContext)) {
                engine->setUpdateOptions(parallel ? IRenderEngine::kParallelUpdate : IRenderEngine::kNone);
                model->setEdgeWidth(settings.value(prefix + "/edge.width", 1.0f));
                model->setPhysicsEnable(settings.value(prefix + "/enable.physics", true));
                sceneRef->addModel(model.get(), engine.release(), i);
                BaseApplicationContext::MapBuffer motionBuffer(applicationContextRef);
                const std::string &modelMotionPath = settings.value(prefix + "/motion", std::string());
                if (applicationContextRef->mapFile(!modelMotionPath.empty() ? modelMotionPath : globalMotionPath, &motionBuffer)) {
                    IMotionSmartPtr motion(factoryRef->createMotion(motionBuffer.address,
                                                                    motionBuffer.size,
                                                                    model.get(), ok));
                    sceneRef->addMotion(motion.release());
                }
                applicationContextRef->setCurrentModelRef(model.get());
                model.release();
            }
        }
        else {
            icu4c::String s(UnicodeString::fromUTF8(settings.value(prefix + "/effect", std::string())));
            if (!s.value().isEmpty()) {
                if (IEffect *effectRef = applicationContextRef->createEffectRef(s.toStdString())) {
                    applicationContextRef->parseOffscreenSemantic(effectRef, 0);
                    model.reset(factoryRef->newModel(IModel::kPMXModel));
                    model->setName(effectRef->name(), IEncoding::kDefaultLanguage);
                    IRenderEngineSmartPtr engine(sceneRef->createRenderEngine(applicationContextRef, model.get(), flags));
                    BaseApplicationContext::ModelContext modelContext(applicationContextRef, 0, &dir);
                    engine->setEffect(effectRef, IEffect::kAutoDetection, &modelContext);
                    applicationContextRef->createEffectParameterUIWidgets(effectRef);
                    sceneRef->addModel(model.release(), engine.release(), i);
                }
            }
        }
    }
}

}
