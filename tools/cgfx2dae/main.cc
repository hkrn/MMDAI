#include <QCoreApplication>
#include <QtCore>

#include <GL/osmesa.h>
#include <GL/glext.h>

#include <Cg/cg.h>
#include <Cg/cgGL.h>

#include <COLLADABU.h>
#include <COLLADASWAnnotation.h>
#include <COLLADASWCode.h>
#include <COLLADASWConstants.h>
#include <COLLADASWEffectProfile.h>
#include <COLLADASWLibraryImages.h>
#include <COLLADASWLibraryEffects.h>
#include <COLLADASWOpenGLConstants.h>
#include <COLLADASWParamTemplate.h>
#include <COLLADASWPass.h>
#include <COLLADASWShader.h>
#include <COLLADASWRenderState.h>
#include <COLLADASWTechniqueFX.h>
#include <COLLADASWStreamWriter.h>
#include <COLLADASWParamBase.h>

#include "tinyxml2.h"

using namespace COLLADASW;

namespace {

class EffectExporter : protected LibraryEffects {
public:
    static const std::string kVertexDomain;
    static const std::string kFragmentDomain;
    static ValueType::ColladaType selectBooleanType(int value) {
        switch (value) {
        case 1:
        default:
            return ValueType::BOOL;
        case 2:
            return ValueType::BOOL2;
        case 3:
            return ValueType::BOOL3;
        case 4:
            return ValueType::BOOL4;
        }
    }
    static ValueType::ColladaType selectIntType(int value) {
        switch (value) {
        case 1:
        default:
            return ValueType::INT;
        case 2:
            return ValueType::INT2;
        case 3:
            return ValueType::INT3;
        case 4:
            return ValueType::INT4;
        }
    }
    static ValueType::ColladaType selectFloatType(int value, CGtype type) {
        switch (value) {
        case 1:
        default:
            return ValueType::FLOAT;
        case 2:
            return ValueType::FLOAT2;
        case 3:
            return ValueType::FLOAT3;
        case 4:
            return type == CG_FLOAT2x2 ? ValueType::FLOAT2x2 : ValueType::FLOAT4;
        case 9:
            return ValueType::FLOAT3x3;
        case 16:
            return ValueType::FLOAT4x4;
        }
    }
    static void setParameter(ParamBase &parameterElement, CGparameter parameter) {
        parameterElement.openParam(cgGetParameterName(parameter));
        parameterElement.addSemantic(cgGetParameterSemantic(parameter));
        parameterElement.openValuesElement();
    }
    static void cleanup(CGeffect effect) {
        if (cgIsEffect(effect)) {
            cgDestroyEffect(effect);
        }
    }
    static Sampler::WrapMode wrapModeFromStateAssignment(CGstateassignment assignment) {
        int nvalues = 0;
        CGstate state = cgGetSamplerStateAssignmentState(assignment);
        const int *values = cgGetIntStateAssignmentValues(assignment, &nvalues);
        return OPEN_GL::getWrapModeFromOpenGL(cgGetStateEnumerantName(state, values[0]));
    }
    static Sampler::SamplerFilter filterModeFromStateAssignment(CGstateassignment assignment) {
        int nvalues = 0;
        CGstate state = cgGetSamplerStateAssignmentState(assignment);
        const int *values = cgGetIntStateAssignmentValues(assignment, &nvalues);
        return OPEN_GL::getSamplerFilterFromOpenGL(cgGetStateEnumerantName(state, values[0]));
    }
    static std::string getShaderSid(const std::string &domain, const CGpass pass, std::ostringstream &stream) {
        stream.str(std::string());
        stream << cgGetPassName(pass);
        stream << domain;
        stream << pass;
        return stream.str();
    }

    struct ProgramShader {
        String sid;
        String source;
    };
    typedef  std::vector<Annotation> AnnotationList;
    typedef std::vector<ProgramShader> ProgramShaderList;

    EffectExporter(StreamWriter *writerRef)
        : LibraryEffects(writerRef),
          m_writerRef(writerRef)
    {
    }
    ~EffectExporter()
    {
        m_writerRef = 0;
    }

    void getAnnotations(CGannotation annotation, AnnotationList &annotationList) {
        while (annotation) {
            CGtype type = cgGetAnnotationType(annotation);
            int nvalues = 0;
            switch (type) {
            case CG_BOOL:
            case CG_BOOL1:
            case CG_BOOL2:
            case CG_BOOL3:
            case CG_BOOL4:
            {
                if (const CGbool *values = cgGetBoolAnnotationValues(annotation, &nvalues)) {
                    QVarLengthArray<bool> v;
                    v.reserve(nvalues);
                    for (int i = 0; i < nvalues; i++) {
                        v.append(values[0] == CG_TRUE ? true : false);
                    }
                    Annotation annotationElement(m_writerRef, cgGetAnnotationName(annotation), selectBooleanType(nvalues), &values[0], nvalues);
                    annotationList.push_back(annotationElement);
                }
                break;
            }
            case CG_INT:
            case CG_INT1:
            case CG_INT2:
            case CG_INT3:
            case CG_INT4:
            {
                if (const int *values = cgGetIntAnnotationValues(annotation, &nvalues)) {
                    Annotation annotationElement(m_writerRef, cgGetAnnotationName(annotation), selectIntType(nvalues), values, nvalues);
                    annotationList.push_back(annotationElement);
                }
                break;
            }
            case CG_FLOAT:
            case CG_FLOAT1:
            case CG_FLOAT2:
            case CG_FLOAT3:
            case CG_FLOAT4:
            case CG_FLOAT4x4:
            {
                if (const float *values = cgGetFloatAnnotationValues(annotation, &nvalues)) {
                    Annotation annotationElement(m_writerRef, cgGetAnnotationName(annotation), selectIntType(nvalues), values, nvalues);
                    annotationList.push_back(annotationElement);
                }
                break;
            }
            case CG_STRING: {
                if (const char *const *values = cgGetStringAnnotationValues(annotation, &nvalues)) {
                    Annotation annotationElement(m_writerRef, cgGetAnnotationName(annotation), ValueType::STRING, values[0], 1);
                    annotationList.push_back(annotationElement);
                }
                break;
            }
            default:
                break;
            }
            qDebug("ANNOTATION: name=\"%s\" type=\"%s\"",
                   cgGetAnnotationName(annotation),
                   cgGetTypeString(type));
            annotation = cgGetNextAnnotation(annotation);
        }
    }
    void exportAnnotations(CGannotation annotation) {
        AnnotationList annotationList;
        getAnnotations(annotation, annotationList);
        for (AnnotationList::iterator it = annotationList.begin(); it != annotationList.end(); it++) {
            it->add();
        }
    }

    ParamBase *createFloatParam(int value) const {
        switch (value) {
        case 1:
            return new NewParamFloat(m_writerRef);
        case 2:
            return new NewParamFloat2(m_writerRef);
        case 3:
            return new NewParamFloat3(m_writerRef);
        case 4:
            return new NewParamFloat4(m_writerRef);
        case 16:
            return new NewParamFloat4x4(m_writerRef);
        default:
            return 0;
        }
    }
    void exportSamplerParameter(CGparameter samplerParameter, CGtype type) {
        CGparameter textureParameter = 0;
        if (CGstateassignment assignment = cgGetNamedSamplerStateAssignment(samplerParameter, CSWC::CSW_ELEMENT_TEXTURE.c_str())) {
            textureParameter = cgGetTextureStateAssignmentValue(assignment);
        }
        if (!textureParameter) {
            qWarning("Sampler %s does not contains texture parameter", cgGetParameterName(samplerParameter));
            return;
        }
        qDebug("SAMPLER: sampler=%s texture=%s", cgGetParameterName(samplerParameter), cgGetParameterName(textureParameter));
        Sampler sampler(Sampler::SAMPLER_TYPE_UNSPECIFIED, cgGetParameterName(samplerParameter), cgGetParameterName(textureParameter));
        if (CGstateassignment assignment = cgGetNamedSamplerStateAssignment(samplerParameter, CSWC::CSW_ELEMENT_WRAP_S.c_str())) {
            sampler.setWrapS(wrapModeFromStateAssignment(assignment));
        }
        if (CGstateassignment assignment = cgGetNamedSamplerStateAssignment(samplerParameter, CSWC::CSW_ELEMENT_WRAP_T.c_str())) {
            sampler.setWrapT(wrapModeFromStateAssignment(assignment));
        }
        if (CGstateassignment assignment = cgGetNamedSamplerStateAssignment(samplerParameter, CSWC::CSW_ELEMENT_WRAP_P.c_str())) {
            sampler.setWrapP(wrapModeFromStateAssignment(assignment));
        }
        if (CGstateassignment assignment = cgGetNamedSamplerStateAssignment(samplerParameter, CSWC::CSW_ELEMENT_MINFILTER.c_str())) {
            sampler.setMinFilter(filterModeFromStateAssignment(assignment));
        }
        if (CGstateassignment assignment = cgGetNamedSamplerStateAssignment(samplerParameter, CSWC::CSW_ELEMENT_MAGFILTER.c_str())) {
            sampler.setMagFilter(filterModeFromStateAssignment(assignment));
        }
        if (CGstateassignment assignment = cgGetNamedSamplerStateAssignment(samplerParameter, CSWC::CSW_ELEMENT_MINFILTER.c_str())) {
            sampler.setMipFilter(filterModeFromStateAssignment(assignment));
        }
        if (CGstateassignment assignment = cgGetNamedSamplerStateAssignment(samplerParameter, CSWC::CSW_ELEMENT_MIPMAP_MAXLEVEL.c_str())) {
            int nvalues = 0;
            const int *values = cgGetIntStateAssignmentValues(assignment, &nvalues);
            sampler.setMipmapMaxlevel(values[0]);
        }
        if (CGstateassignment assignment = cgGetNamedSamplerStateAssignment(samplerParameter, CSWC::CSW_ELEMENT_MIPMAP_BIAS.c_str())) {
            int nvalues = 0;
            const float *values = cgGetFloatStateAssignmentValues(assignment, &nvalues);
            sampler.setMipmapBias(values[0]);
        }
        if (CGannotation anntoation = cgGetNamedParameterAnnotation(samplerParameter, CSWC::CSW_FX_ANNOTATION_RESOURCE_TYPE.c_str())) {
            String resourceType(cgGetStringAnnotationValue(anntoation));
            if (Utils::equalsIgnoreCase(resourceType, CSWC::CSW_SURFACE_TYPE_1D)) {
                sampler.setSamplerType(Sampler::SAMPLER_TYPE_1D);
            }
            else if (Utils::equalsIgnoreCase(resourceType, CSWC::CSW_SURFACE_TYPE_2D)) {
                sampler.setSamplerType(Sampler::SAMPLER_TYPE_2D);
            }
            else if (Utils::equalsIgnoreCase(resourceType, CSWC::CSW_SURFACE_TYPE_3D)) {
                sampler.setSamplerType(Sampler::SAMPLER_TYPE_3D);
            }
            else if (Utils::equalsIgnoreCase(resourceType, CSWC::CSW_SURFACE_TYPE_CUBE)) {
                sampler.setSamplerType(Sampler::SAMPLER_TYPE_CUBE);
            }
        }
        else {
            switch (type) {
            case CG_SAMPLER1D:
                sampler.setSamplerType(Sampler::SAMPLER_TYPE_1D);
                break;
            case CG_SAMPLER2D:
                sampler.setSamplerType(Sampler::SAMPLER_TYPE_2D);
                break;
            case CG_SAMPLER3D:
                sampler.setSamplerType(Sampler::SAMPLER_TYPE_3D);
                break;
            case CG_SAMPLERCUBE:
                sampler.setSamplerType(Sampler::SAMPLER_TYPE_CUBE);
                break;
            default:
                sampler.setSamplerType(Sampler::SAMPLER_TYPE_2D);
                break;
            }
        }
        AnnotationList textureAnnotations, samplerAnnotations;
        getAnnotations(cgGetFirstParameterAnnotation(textureParameter), textureAnnotations);
        getAnnotations(cgGetFirstParameterAnnotation(samplerParameter), samplerAnnotations);
        sampler.addInNewParam(m_writerRef, &textureAnnotations, &samplerAnnotations);
    }
    void exportParameters(CGparameter parameter) {
        while (parameter) {
            CGtype type = cgGetParameterType(parameter);
            int ntotal = cgGetParameterRows(parameter) * cgGetParameterColumns(parameter);
            int asize = cgGetArrayTotalSize(parameter);
            if (asize > 0) {
                ntotal *= asize;
            }
            switch (cgGetParameterBaseType(parameter)) {
            case CG_BOOL: {
                if (ntotal == 1) {
                    int value;
                    cgGetParameterDefaultValueic(parameter, ntotal, &value);
                    NewParamBool parameterElement(m_writerRef);
                    setParameter(parameterElement, parameter);
                    parameterElement.appendValues(value != 0 ? true : false);
                    parameterElement.closeParam();
                }
                break;
            }
            case CG_INT: {
                if (ntotal == 1) {
                    int value;
                    cgGetParameterDefaultValueic(parameter, ntotal, &value);
                    NewParamInt parameterElement(m_writerRef);
                    setParameter(parameterElement, parameter);
                    parameterElement.appendValues(value);
                    parameterElement.closeParam();
                }
                break;
            }
            case CG_FLOAT: {
                QScopedPointer<ParamBase> parameterElement(createFloatParam(ntotal));
                if (parameter) {
                    std::vector<float> values(ntotal);
                    cgGetParameterDefaultValuefc(parameter, ntotal, &values[0]);
                    setParameter(*parameterElement.data(), parameter);
                    parameterElement->appendValues(values);
                    parameterElement->closeParam();
                }
                break;
            }
            case CG_TEXTURE: {
                NewParamSurface parameterElement(m_writerRef);
                setParameter(parameterElement, parameter);
                parameterElement.closeParam();
                break;
            }
            case CG_SAMPLER1D:
            case CG_SAMPLER2D:
            case CG_SAMPLER3D:
            case CG_SAMPLERCUBE:
            {
                exportSamplerParameter(parameter, type);
                break;
            }
            default:
                break;
            }
            qDebug("PARAMETER: name=\"%s\" semantic=\"%s\" base=\"%s\" type=\"%s\"",
                   cgGetParameterName(parameter),
                   cgGetParameterSemantic(parameter),
                   cgGetTypeString(cgGetParameterBaseType(parameter)),
                   cgGetTypeString(type));
            exportAnnotations(cgGetFirstParameterAnnotation(parameter));
            parameter = cgGetNextParameter(parameter);
        }
    }
    template<typename T>
    void exportStateAssignmentValues(const std::string &typeName, T *values, int nvalues) {
        ParamBase base(m_writerRef, &typeName);
        base.openParam();
        for (int i = 0; i < nvalues; i++) {
            const T &value = values[i];
            base.appendAttribute(CSWC::CSW_ATTRIBUTE_VALUE, Utils::toString(value));
        }
        base.closeParam();
    }
    void exportStateAssignments(CGstateassignment assignment) {
        static const String kElementStates("states");
        String stateName, typeName;
        ParamBase statesTag(m_writerRef, &kElementStates);
        statesTag.openParam();
        while (assignment) {
            CGstate state = cgGetStateAssignmentState(assignment);
            const RenderState::PassState &passState = RenderState::getRenderStateFromCgName(cgGetStateName(state));
            if (passState == RenderState::PASS_STATE_INVALID) {
                assignment = cgGetNextStateAssignment(assignment);
                continue;
            }
            stateName.assign(RenderState::getColladaRenderStateName(passState));
            ParamBase stateTag(m_writerRef, &stateName);
            CGtype type = cgGetStateType(state);
            int nvalues = 0;
            stateTag.openParam();
            switch (type) {
            case CG_BOOL:
            case CG_BOOL1:
            case CG_BOOL2:
            case CG_BOOL3:
            case CG_BOOL4:
            {
                if (const CGbool *values = cgGetBoolStateAssignmentValues(assignment, &nvalues)) {
                    typeName.assign(ValueType::getColladaTypeString(selectBooleanType(nvalues)));
                    exportStateAssignmentValues(typeName, values, nvalues);
                }
                break;
            }
            case CG_INT:
            case CG_INT1:
            case CG_INT2:
            case CG_INT3:
            case CG_INT4:
            {
                if (const int *values = cgGetIntStateAssignmentValues(assignment, &nvalues)) {
                    typeName.assign(ValueType::getColladaTypeString(selectIntType(nvalues)));
                    exportStateAssignmentValues(typeName, values, nvalues);
                }
                break;
            }
            case CG_STRING: {
                if (const char *value = cgGetStringStateAssignmentValue(assignment)) {
                    typeName.assign(ValueType::getColladaTypeString(ValueType::STRING));
                    exportStateAssignmentValues(typeName, &value, 1);
                }
                break;
            }
            case CG_TEXTURE: {
                CGparameter parameter = cgGetTextureStateAssignmentValue(assignment);
                const char *name = cgGetParameterName(parameter);
                stateTag.appendAttribute(CSWC::CSW_ATTRIBUTE_VALUE, name);
                break;
            }
            case CG_SAMPLER1D:
            case CG_SAMPLER2D:
            case CG_SAMPLER3D:
            case CG_SAMPLERCUBE: {
                CGparameter parameter = cgGetSamplerStateAssignmentParameter(assignment);
                const char *name = cgGetParameterName(parameter);
                stateTag.appendAttribute(CSWC::CSW_ATTRIBUTE_VALUE, name);
                break;
            }
            default:
                break;
            }
            qDebug("STATE: name=\"%s\" type=\"%s\"", cgGetStateName(state), cgGetTypeString(type));
            stateTag.closeParam();
            assignment = cgGetNextStateAssignment(assignment);
        }
        statesTag.closeParam();
    }
    void exportShaderParameters(CGprogram shader, const std::string &source) {
        QString s = QString::fromStdString(source);
        QTextStream stream(&s, QIODevice::ReadOnly);
        static const String kBindUniformElement("bind_uniform");
        while (!stream.atEnd()) {
            QString line = stream.readLine();
            if (line.startsWith("//var")) {
                QStringList tokens = line.split(" : ");
                QStringList targetSymbol = tokens.at(2).trimmed().split(", ");
                QString destinationParameterRef = tokens.first().split(" ").at(2).trimmed();
                CGparameter parameter = cgGetNamedParameter(shader, qPrintable(destinationParameterRef));
                qDebug() << destinationParameterRef << targetSymbol.first()
                         << (targetSymbol.size() > 1 ? targetSymbol.at(1).toInt() : 1)
                         << parameter
                         << cgGetParameterName(parameter);
                m_writerRef->openElement(kBindUniformElement);
                m_writerRef->appendAttribute(CSWC::CSW_ATTRIBUTE_SYMBOL, targetSymbol.first().toStdString());
                ParamBase param(m_writerRef);
                param.openParam(destinationParameterRef.toStdString());
                param.closeParam();
                m_writerRef->closeElement();
            }
        }
    }
    void exportPassProgramParameters(CGpass pass) {
        std::string vertexShaderSource, fragmentShaderSource;
        CGprogram vertexShader = cgGetPassProgram(pass, CG_VERTEX_DOMAIN);
        CGprogram fragmentShader = cgGetPassProgram(pass, CG_FRAGMENT_DOMAIN);
        vertexShaderSource.assign(cgGetProgramString(vertexShader, CG_COMPILED_PROGRAM));
        fragmentShaderSource.assign(cgGetProgramString(vertexShader, CG_COMPILED_PROGRAM));
        exportShaderParameters(vertexShader, vertexShaderSource);
        exportShaderParameters(fragmentShader, fragmentShaderSource);
    }
    void exportPassPrograms(CGpass pass) {
        static const String kElementProgram("program"), kElementImport("import");
        std::ostringstream stream;
        m_writerRef->openElement(kElementProgram);
        Shader vertexShader(m_writerRef, Shader::SCOPE_GLSL, Shader::STAGE_VERTEX);
        vertexShader.openShader();
        m_writerRef->openElement(CSWC::CSW_ELEMENT_SOURCE);
        m_writerRef->openElement(kElementImport);
        m_writerRef->appendAttribute("ref", getShaderSid(kVertexDomain, pass, stream));
        m_writerRef->closeElement();
        m_writerRef->closeElement();
        vertexShader.closeShader();
        Shader fragmentShader(m_writerRef, Shader::SCOPE_GLSL, Shader::STAGE_FRAGMENT);
        fragmentShader.openShader();
        m_writerRef->openElement(CSWC::CSW_ELEMENT_SOURCE);
        m_writerRef->openElement(kElementImport);
        m_writerRef->appendAttribute("ref", getShaderSid(kVertexDomain, pass, stream));
        m_writerRef->closeElement();
        m_writerRef->closeElement();
        fragmentShader.closeShader();
        exportPassProgramParameters(pass);
        m_writerRef->closeElement();
    }
    void exportPasses(CGpass pass) {
        while (pass) {
            Pass passElement(m_writerRef);
            exportStateAssignments(cgGetFirstStateAssignment(pass));
            passElement.openPass(cgGetPassName(pass));
            exportPassPrograms(pass);
            exportAnnotations(cgGetFirstPassAnnotation(pass));
            qDebug("PASS: name=\"%s\"", cgGetPassName(pass));
            passElement.closePass();
            pass = cgGetNextPass(pass);
        }
    }
    void getPassProgramCodes(CGpass pass, ProgramShaderList &shaders) {
        std::ostringstream stream;
        ProgramShader shader;
        while (pass) {
            CGprogram vertexShader = cgGetPassProgram(pass, CG_VERTEX_DOMAIN);
            cgCompileProgram(vertexShader);
            shader.sid.assign(getShaderSid(kVertexDomain, pass, stream));
            shader.source.assign(cgGetProgramString(vertexShader, CG_COMPILED_PROGRAM));
            shaders.push_back(shader);
            qDebug("VERTEX: name=%s", shader.sid.c_str());
            CGprogram fragmentProgram = cgGetPassProgram(pass, CG_FRAGMENT_DOMAIN);
            cgCompileProgram(fragmentProgram);
            shader.sid.assign(getShaderSid(kFragmentDomain, pass, stream));
            shader.source.assign(cgGetProgramString(fragmentProgram, CG_COMPILED_PROGRAM));
            shaders.push_back(shader);
            qDebug("FRAGMENT: name=%s", shader.sid.c_str());
            pass = cgGetNextPass(pass);
        }
    }
    void exportTechniques(CGtechnique technique, EffectProfile &profile) {
        ProgramShaderList codes;
        CGtechnique techniquePtr = technique;
        while (techniquePtr) {
            getPassProgramCodes(cgGetFirstPass(techniquePtr), codes);
            techniquePtr = cgGetNextTechnique(techniquePtr);
        }
        for (ProgramShaderList::const_iterator it = codes.begin(); it != codes.end(); it++) {
            m_writerRef->openElement(CSWC::CSW_ELEMENT_CODE);
            m_writerRef->appendAttribute(CSWC::CSW_ATTRIBUTE_SID, it->sid);
            m_writerRef->appendText(it->source);
            m_writerRef->closeElement();
        }
        while (technique) {
            profile.openTechnique(cgGetTechniqueName(technique));
            qDebug("TECHNIQUE: name=\"%s\"", cgGetTechniqueName(technique));
            exportPasses(cgGetFirstPass(technique));
            exportAnnotations(cgGetFirstTechniqueAnnotation(technique));
            profile.closeTechnique();
            technique = cgGetNextTechnique(technique);
        }
    }

    void setEffect(CGeffect effect) {
        m_effect.reset(effect);
    }
    void save() {
        openEffect();
        EffectProfile profile(m_writerRef);
        profile.setProfileType(EffectProfile::GLSL);
        profile.openProfile();
        exportParameters(cgGetFirstEffectParameter(m_effect.data()));
        exportTechniques(cgGetFirstTechnique(m_effect.data()), profile);
        profile.closeProfile();
        exportAnnotations(cgGetFirstEffectAnnotation(m_effect.data()));
        closeEffect();
    }

private:
    QScopedPointer<struct _CGeffect, EffectExporter> m_effect;
    QList<Image> m_images;
    StreamWriter *m_writerRef;
};

const std::string EffectExporter::kVertexDomain = std::string("VertexDomain");
const std::string EffectExporter::kFragmentDomain = std::string("FragmentDomain");

class ImageExporter : protected LibraryImages {
public:
    ImageExporter(StreamWriter *writerRef)
        : LibraryImages(writerRef),
          m_writerRef(writerRef)
    {
    }
    ~ImageExporter() {
        m_writerRef = 0;
    }

    void save() {
        Image image(URI::INVALID, "", "");
        addImage(image);
    }

private:
    StreamWriter *m_writerRef;
};

class Exporter {
public:
    static void cleanup(CGcontext context) {
        if (cgIsContext(context)) {
            cgDestroyContext(context);
        }
    }
    static void handleError(CGcontext /* context */, CGerror error, void * /* userData */) {
        qWarning("cgGetErrorString: %s", cgGetErrorString(error));
    }

    Exporter(StreamWriter *writerRef)
        : m_context(cgCreateContext()),
          m_effectExporter(writerRef),
          m_imageExporter(writerRef),
          m_writerRef(writerRef)
    {
        cgSetErrorHandler(&Exporter::handleError, this);
        cgGLRegisterStates(m_context.data());
    }
    ~Exporter() {
        m_writerRef = 0;
    }

    void load(const char *path) {
        QFileInfo finfo(path);
        m_effectExporter.setEffect(cgCreateEffectFromFile(m_context.data(), qPrintable(finfo.absoluteFilePath()), 0));
    }
    void save() {
        m_writerRef->startDocument();
        m_effectExporter.save();
        m_imageExporter.save();
        m_writerRef->endDocument();
    }

private:
    QScopedPointer<struct _CGcontext, Exporter> m_context;
    EffectExporter m_effectExporter;
    ImageExporter m_imageExporter;
    StreamWriter *m_writerRef;
};

using namespace tinyxml2;

class EffectLoader {
public:
    enum Type {
        kUnknown,
        kBool,
        kBool1,
        kBool2,
        kBool3,
        kBool4,
        kInt,
        kInt1,
        kInt2,
        kInt3,
        kInt4,
        kFloat,
        kFloat1,
        kFloat2,
        kFloat3,
        kFloat4,
        kFloat2x2,
        kFloat3x3,
        kFloat4x4,
        kString,
        kTexture,
        kSampler,
        kSampler1D,
        kSampler2D,
        kSampler3D,
        kSamplerCube,
        kMaxType
    };

    void load(const char *path) {
        XMLDocument document;
        if (document.LoadFile(path) == XML_NO_ERROR) {
            if (const XMLElement *collada = document.RootElement()) {
                for (XMLNode *node = const_cast<XMLNode *>(collada->FirstChild()); node; node = node->NextSibling()) {
                    const XMLElement *element = node->ToElement();
                    if (equalsToElement(element, "library_effects")) {
                        parseLibraryEffects(element);
                    }
                }
            }
        }
        else {
            qWarning("XMLError: id=%d path=%s", document.ErrorID(), path);
        }
    }

private:
    struct EffectPass;
    typedef std::map<std::string, std::string> EffectSourceMap;
    struct EffectAnnotation {
        std::string value;
        Type base;
        Type type;
    };
    typedef std::map<std::string, EffectAnnotation> EffectAnnotationMap;
    struct EffectParameter {
        std::string symbol;
        std::string semantic;
        std::string value;
        Type base;
        Type type;
    };
    typedef std::map<std::string, EffectParameter> EffectParameterMap;
    struct EffectTechnique {
        std::string name;
        std::vector<const EffectAnnotation *> annotations;
        std::vector<const EffectPass *> passes;
    };
    typedef std::map<std::string, EffectTechnique> EffectTechniqueMap;
    struct EffectState {
        std::string name;
        std::string value;
    };
    struct EffectPass {
        std::string name;
        EffectTechnique *technique;
        std::map<std::string, const std::string *> sources;
        std::vector<const EffectAnnotation *> annotations;
        std::vector<EffectParameter *> parameters;
        std::vector<EffectState> states;
    };
    typedef std::map<std::string, EffectPass> EffectPassMap;

    static inline std::string toStringSafe(const char *value) {
        return value ? value : std::string();
    }
    static bool equalsConstant(const char *left, const char *const right) {
        return left && strncmp(left, right, sizeof(right) - 1) == 0;
    }
    static inline bool equalsToElement(const XMLElement *element, const char *const name) {
        return element && equalsConstant(element->Name(), name);
    }
    static bool equalsToTypedElement(const XMLElement *element, Type &type, Type &base) {
        type = kUnknown;
        base = kUnknown;
        if (element) {
            const char *name = element->Name();
            const char firstLetter = name[0];
            switch (firstLetter) {
            case 'b':
                base = kBool;
                return handleBoolType(name, type);
            case 'i':
                base = kInt;
                return handleIntType(name, type);
            case 'f':
                base = kFloat;
                return handleFloatType(name, type);
            case 's': {
                static const char kTypeSamplerString[] = "sampler";
                if (equalsConstant(name, "string")) {
                    base = kString;
                    type = kString;
                    return true;
                }
                else if (equalsConstant(name, "surface")) {
                    base = kTexture;
                    type = kTexture;
                    return true;
                }
                else if (equalsConstant(name, kTypeSamplerString)) {
                    const char *ptr = name + sizeof(kTypeSamplerString) - 1;
                    base = kSampler;
                    switch (ptr[0]) {
                    case '1':
                        type = kSampler1D;
                        return true;
                    case '2':
                        type = kSampler2D;
                        return true;
                    case '3':
                        type = kSampler3D;
                        return true;
                    case 'C':
                        type = kSamplerCube;
                        return true;
                    }
                }
                break;
            }
            default:
                break;
            }
        }
        return false;
    }
    static bool handleBoolType(const char *name, Type &type) {
        static const char kTypeBoolString[] = "bool";
        if (equalsConstant(name, kTypeBoolString)) {
            int nelements = strtoul(name + sizeof(kTypeBoolString) - 1, 0, 10);
            switch (nelements) {
            case 4:
                type = kBool4;
                return true;
            case 3:
                type = kBool3;
                return true;
            case 2:
                type = kBool2;
                return true;
            case 1:
            default:
                type = kBool1;
                return true;
            }
        }
        return false;
    }
    static bool handleIntType(const char *name, Type &type) {
        static const char kTypeIntString[] = "int";
        if (equalsConstant(name, kTypeIntString)) {
            int nelements = strtoul(name + sizeof(kTypeIntString) - 1, 0, 10);
            switch (nelements) {
            case 4:
                type = kInt4;
                return true;
            case 3:
                type = kInt3;
                return true;
            case 2:
                type = kInt2;
                return true;
            case 1:
            default:
                type = kInt1;
                return true;
            }
        }
        return false;
    }
    static bool handleFloatType(const char *name, Type &type) {
        static const char kTypeFloatString[] = "float";
        if (equalsConstant(name, kTypeFloatString)) {
            const char *ptr = name + sizeof(kTypeFloatString) - 1;
            int nelements = strtoul(ptr, 0, 10);
            switch (nelements) {
            case 4:
                type = kFloat4;
                return true;
            case 3:
                type = kFloat3;
                return true;
            case 2:
                type = kFloat2;
                return true;
            case 1:
                type = kFloat1;
                return true;
            default:
                if (*ptr == 0) {
                    type = kFloat1;
                }
                else if (strcmp(ptr, "4x4") == 0) {
                    type = kFloat4x4;
                }
                else if (strcmp(ptr, "3x3") == 0) {
                    type = kFloat3x3;
                }
                else if (strcmp(ptr, "2x2") == 0) {
                    type = kFloat2x2;
                }
                else {
                    type = kFloat1;
                }
                return true;
            }
        }
        return false;
    }

    void parseLibraryEffects(const XMLElement *element) {
        for (XMLNode *node = const_cast<XMLNode *>(element->FirstChild()); node; node = node->NextSibling()) {
            const XMLElement *element = node->ToElement();
            if (equalsToElement(element, "effect")) {
                parseEffect(element);
            }
        }
    }
    void parseEffect(const XMLElement *element) {
        for (XMLNode *node = const_cast<XMLNode *>(element->FirstChild()); node; node = node->NextSibling()) {
            const XMLElement *element = node->ToElement();
            if (equalsToElement(element, "profile_GLSL")) {
                parseEffectProfile(element);
            }
        }
    }
    void parseEffectProfile(const XMLElement *element) {
        for (XMLNode *node = const_cast<XMLNode *>(element->FirstChild()); node; node = node->NextSibling()) {
            const XMLElement *element = node->ToElement();
            if (equalsToElement(element, "annotate")) {
                parseAnnotate(element);
            }
            else if (equalsToElement(element, "code")) {
                parseCode(element);
            }
            else if (equalsToElement(element, "newparam")) {
                parseNewParam(element);
            }
            else if (equalsToElement(element, "technique")) {
                parseTechnique(element);
            }
        }
    }
    const EffectAnnotation *parseAnnotate(const XMLElement *parentElement) {
        const std::string &name = toStringSafe(parentElement->Name());
        EffectAnnotationMap::const_iterator it = m_annotations.find(name), it2;
        if (it != m_annotations.end()) {
            return &it->second;
        }
        else {
            EffectAnnotation annotation;
            for (XMLNode *node = const_cast<XMLNode *>(parentElement->FirstChild()); node; node = node->NextSibling()) {
                const XMLElement *element = node->ToElement();
                if (equalsToTypedElement(element, annotation.type, annotation.base)) {
                    annotation.value.assign(toStringSafe(element->GetText()));
                }
            }
            it2 = m_annotations.insert(std::make_pair(name, annotation)).first;
            return &it2->second;
        }
    }
    const std::string *parseCode(const XMLElement *parentElement) {
        const std::string &sid = toStringSafe(parentElement->Attribute("sid"));
        EffectSourceMap::const_iterator it = m_sources.find(sid), it2;
        if (it != m_sources.end()) {
            return &it->second;
        }
        else {
            const std::string &code = toStringSafe(parentElement->GetText());
            it2 = m_sources.insert(std::make_pair(sid, code)).first;
            return &it2->second;
        }
    }
    const EffectParameter *parseNewParam(const XMLElement *parentElement) {
        const std::string &sid = toStringSafe(parentElement->Attribute("sid"));
        EffectParameterMap::const_iterator it = m_parameters.find(sid), it2;
        if (it != m_parameters.end()) {
            return &it->second;
        }
        else {
            EffectParameter parameter;
            for (XMLNode *node = const_cast<XMLNode *>(parentElement->FirstChild()); node; node = node->NextSibling()) {
                const XMLElement *element = node->ToElement();
                if (equalsToElement(element, "semantic")) {
                    parameter.semantic.assign(toStringSafe(element->GetText()));
                }
                else if (equalsToTypedElement(element, parameter.type, parameter.base)) {
                    parameter.value.assign(toStringSafe(element->GetText()));
                }
            }
            it2 = m_parameters.insert(std::make_pair(sid, parameter)).first;
            return &it2->second;
        }
    }
    const EffectTechnique *parseTechnique(const XMLElement *parentElement) {
        const std::string &sid = toStringSafe(parentElement->Attribute("sid"));
        EffectTechniqueMap::const_iterator it = m_techniques.find(sid), it2;
        if (it != m_techniques.end()) {
            return &it->second;
        }
        else {
            EffectTechnique technique;
            for (XMLNode *node = const_cast<XMLNode *>(parentElement->FirstChild()); node; node = node->NextSibling()) {
                const XMLElement *element = node->ToElement();
                if (equalsToElement(element, "annotate")) {
                    technique.annotations.push_back(parseAnnotate(element));
                }
                else if (equalsToElement(element, "pass")) {
                    technique.passes.push_back(parsePasses(element, &technique));
                }
            }
            it2 = m_techniques.insert(std::make_pair(sid, technique)).first;
            return &it2->second;
        }
    }
    void parseStates(const XMLElement *parentElement, EffectPass *pass) {
        Type type, base;
        for (XMLNode *node = const_cast<XMLNode *>(parentElement->FirstChild()); node; node = node->NextSibling()) {
            const XMLElement *element = node->ToElement();
            const XMLElement *typedElement = element->FirstChildElement();
            if (equalsToTypedElement(typedElement, type, base)) {
                EffectState state;
                state.name.assign(toStringSafe(element->Name()));
                state.value.assign(toStringSafe(element->Attribute("value")));
                pass->states.push_back(state);
            }
        }
    }
    const EffectPass *parsePasses(const XMLElement *parentElement, EffectTechnique *parentTechnique) {
        const std::string &sid = toStringSafe(parentElement->Attribute("sid"));
        const std::string &key = parentTechnique->name + "/" + sid;
        EffectPassMap::const_iterator it = m_passes.find(key), it2;
        if (it != m_passes.end()) {
            return &it->second;
        }
        else {
            EffectPass pass;
            pass.technique = parentTechnique;
            for (XMLNode *node = const_cast<XMLNode *>(parentElement->FirstChild()); node; node = node->NextSibling()) {
                const XMLElement *element = node->ToElement();
                if (equalsToElement(element, "annotate")) {
                    pass.annotations.push_back(parseAnnotate(element));
                }
                else if (equalsToElement(element, "program")) {
                    parseProgram(element, &pass);
                }
                else if (equalsToElement(element, "states")) {
                    parseStates(element, &pass);
                }
            }
            it2 = m_passes.insert(std::make_pair(key, pass)).first;
            return &it2->second;
        }
    }
    void parseProgram(const XMLElement *parentElement, EffectPass *pass) {
        for (XMLNode *node = const_cast<XMLNode *>(parentElement->FirstChild()); node; node = node->NextSibling()) {
            const XMLElement *element = node->ToElement();
            if (equalsToElement(element, "shader")) {
                const std::string &stage = toStringSafe(element->Attribute("stage"));
                if (const XMLElement *sourceElement = element->FirstChildElement("source")) {
                    if (const XMLElement *importElement = sourceElement->FirstChildElement("import")) {
                        const std::string &ref = toStringSafe(importElement->Attribute("ref"));
                        EffectSourceMap::const_iterator it = m_sources.find(ref);
                        if (it != m_sources.end()) {
                            pass->sources.insert(std::make_pair(stage, &it->second));
                        }
                    }
                }
            }
            else if (equalsToElement(element, "bind_uniform")) {
                const std::string &symbol = toStringSafe(element->Attribute("symbol"));
                if (const XMLElement *parameterElement = element->FirstChildElement("param")) {
                    const std::string &ref = toStringSafe(parameterElement->Attribute("ref"));
                    EffectParameterMap::iterator it = m_parameters.find(ref);
                    if (it != m_parameters.end()) {
                        EffectParameter *parameter = &it->second;
                        parameter->symbol.assign(symbol);
                        pass->parameters.push_back(parameter);
                    }
                }
            }
        }
    }

    EffectAnnotationMap m_annotations;
    EffectTechniqueMap m_techniques;
    EffectPassMap m_passes;
    EffectSourceMap m_sources;
    EffectParameterMap m_parameters;
};

}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (argc != 2) {
        qDebug("usage: cgfx2dae: [filename]");
        exit(-1);
    }

    uint8_t dummy[4];
    OSMesaContext context = OSMesaCreateContextExt(OSMESA_RGBA, 24, 0, 0, 0);
    OSMesaMakeCurrent(context, &dummy[0], GL_UNSIGNED_BYTE, 1, 1);
    qDebug("GL_VENDOR   = %s", glGetString(GL_VENDOR));
    qDebug("GL_RENDERER = %s", glGetString(GL_RENDERER));
    qDebug("GL_VERSION  = %s", glGetString(GL_VERSION));
    NativeString filename(QDir(a.applicationDirPath()).absoluteFilePath("test.dae").toStdString());
    {
        StreamWriter writer(filename, false, StreamWriter::COLLADA_1_5_0);
        Exporter exporter(&writer);
        exporter.load(argv[1]);
        exporter.save();
    }
    EffectLoader loader;
    loader.load(filename.c_str());
    OSMesaDestroyContext(context);

    return 0;
}
