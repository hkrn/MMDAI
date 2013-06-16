/* this is no longer used and replaced this to main.cc (OpenCOLLADA based) */

#include <QCoreApplication>
#include <QtCore>

#include <dae.h>
#include <dom/domCOLLADA.h>

#include <Cg/cg.h>
#include <Cg/cgGL.h>

namespace {

struct Deleter {
    static void cleanup(CGcontext context) {
        cgDestroyContext(context);
    }
    static void cleanup(CGeffect effect) {
        cgDestroyEffect(effect);
    }
private:
    Deleter();
    ~Deleter();
};

template<typename T>
static void SetDataToElement(const T *values, int nvalues, daeElement *valueElement, QStringList &format)
{
    for (int i = 0; i < nvalues; i++) {
        T value = values[i];
        format << QVariant(value).toString();
    }
    if (valueElement) {
        valueElement->setCharData(qPrintable(format.join(" ")));
    }
}

static void SetBooleanValues(const CGbool *values, int nvalues, daeElement *valueElement, QStringList &format)
{
    for (int i = 0; i < nvalues; i++) {
        const char *value = values[i] == CG_TRUE ? "true" : "false";
        format << QString(value);
    }
    if (valueElement) {
        valueElement->setCharData(qPrintable(format.join(" ")));
    }
}

static void PrintAnnotations(CGannotation annotation, daeElement *parentElement)
{
    QStringList format;
    while (annotation) {
        CGtype type = cgGetAnnotationType(annotation);
        daeElement *valueElement = 0;
        if (parentElement) {
            daeElement *annotationElement = parentElement->add("annotate");
            annotationElement->setAttribute("name", cgGetAnnotationName(annotation));
            valueElement = annotationElement->add(cgGetTypeString(type));
        }
        int nvalues = 0;
        switch (type) {
        case CG_BOOL: {
            if (const CGbool *values = cgGetBoolAnnotationValues(annotation, &nvalues)) {
                SetBooleanValues(values, nvalues, valueElement, format);
            }
            break;
        }
        case CG_INT: {
            if (const int *values = cgGetIntAnnotationValues(annotation, &nvalues)) {
                SetDataToElement(values, nvalues, valueElement, format);
            }
            break;
        }
        case CG_FLOAT:
        case CG_DOUBLE: {
            if (const float *values = cgGetFloatAnnotationValues(annotation, &nvalues)) {
                SetDataToElement(values, nvalues, valueElement, format);
            }
            break;
        }
        case CG_STRING: {
            if (const char *const *values = cgGetStringAnnotationValues(annotation, &nvalues)) {
                SetDataToElement(values, nvalues, valueElement, format);
            }
            break;
        }
        default:
            break;
        }
        qDebug("ANNOTATION: name=\"%s\" type=\"%s\" value=\"%s\"",
               cgGetAnnotationName(annotation),
               cgGetTypeString(type),
               qPrintable(format.join(" ")));
        format.clear();
        annotation = cgGetNextAnnotation(annotation);
    }
}

static void PrintParameters(CGparameter parameter, daeElement *parentElement)
{
    QStringList format;
    while (parameter) {
        CGtype type = cgGetParameterType(parameter);
        daeElement *parameterElement = parentElement->add("newparam");
        parameterElement->setAttribute("sid", cgGetParameterName(parameter));
        parameterElement->add("semantic")->setCharData(cgGetParameterSemantic(parameter));
        daeElement *valueElement = parameterElement->add(cgGetTypeString(type));
        int ntotal = cgGetParameterRows(parameter) * cgGetParameterColumns(parameter);
        int asize = cgGetArrayTotalSize(parameter);
        if (asize > 0) {
            ntotal *= asize;
        }
        switch (cgGetParameterBaseType(parameter)) {
        case CG_BOOL: {
            QVarLengthArray<int> values(ntotal);
            cgGetParameterDefaultValueir(parameter, ntotal, &values[0]);
            SetDataToElement(&values[0], values.size(), valueElement, format);
            break;
        }
        case CG_INT: {
            QVarLengthArray<int> values(ntotal);
            cgGetParameterDefaultValueir(parameter, ntotal, &values[0]);
            SetDataToElement(&values[0], values.size(), valueElement, format);
            break;
        }
        case CG_FLOAT: {
            QVarLengthArray<float> values(ntotal);
            cgGetParameterDefaultValuefr(parameter, ntotal, &values[0]);
            SetDataToElement(&values[0], values.size(), valueElement, format);
            break;
        }
        case CG_DOUBLE: {
            QVarLengthArray<double> values(ntotal);
            cgGetParameterDefaultValuedr(parameter, ntotal, &values[0]);
            SetDataToElement(&values[0], values.size(), valueElement, format);
            break;
        }
        case CG_SAMPLER2D: {
            CGstateassignment assignment = cgGetFirstSamplerStateAssignment(parameter);
            int nvalues = 0;
            while (assignment) {
                CGstate state = cgGetSamplerStateAssignmentState(assignment);
                const QString &name = QString(cgGetStateName(state)).toLower();
                if (name == "texture") {
                    const char *value = cgGetParameterName(cgGetTextureStateAssignmentValue(assignment));
                    //valueElement->add("source")->setCharData(value);
                }
                else {
                    cgGetIntStateAssignmentValues(assignment, &nvalues);
                    if (nvalues > 0) {
                        valueElement->add(qPrintable(name));
                    }
                }
                qDebug("%s", cgGetStateName(state));
                assignment = cgGetNextStateAssignment(assignment);
            }
            break;
        }
        default:
            break;
        }
        qDebug("PARAMETER: name=\"%s\" semantic=\"%s\" base=\"%s\" type=\"%s\" value=\"%s\"",
               cgGetParameterName(parameter),
               cgGetParameterSemantic(parameter),
               cgGetTypeString(cgGetParameterBaseType(parameter)),
               cgGetTypeString(type),
               qPrintable(format.join(" ")));
        format.clear();
        PrintAnnotations(cgGetFirstParameterAnnotation(parameter), parameterElement);
        parameter = cgGetNextParameter(parameter);
    }
}

static void PrintStateAssignments(CGstateassignment assignment, daeElement *passElement)
{
    QStringList format;
    daeElement *statesElement = passElement->add("states"); Q_UNUSED(statesElement);
    while (assignment) {
        CGstate state = cgGetStateAssignmentState(assignment);
        CGtype type = cgGetStateType(state);
        if (type != CG_PROGRAM_TYPE) {
            int nvalues = 0;
            switch (type) {
            case CG_BOOL: {
                if (const CGbool *values = cgGetBoolStateAssignmentValues(assignment, &nvalues)) {
                    SetBooleanValues(values, nvalues, statesElement, format);
                }
                break;
            }
            case CG_INT: {
                if (const int *values = cgGetIntStateAssignmentValues(assignment, &nvalues)) {
                    SetDataToElement(values, nvalues, statesElement, format);
                }
                break;
            }
            case CG_STRING: {
                if (const char *value = cgGetStringStateAssignmentValue(assignment)) {
                    SetDataToElement(&value, 1, statesElement, format);
                }
                break;
            }
            default:
                break;
            }
            qDebug("STATE: name=\"%s\" type=\"%s\"", cgGetStateName(state), cgGetTypeString(type));
            format.clear();
        }
        assignment = cgGetNextStateAssignment(assignment);
    }
}

static void PrintPasses(CGpass pass, daeElement *techniqueElement, daeElement *profileElement)
{
    while (pass) {
        daeElement *passElement = techniqueElement->add("pass");
        PrintStateAssignments(cgGetFirstStateAssignment(pass), passElement);
        passElement->setAttribute("sid", qPrintable(QUuid::createUuid().toString()));
        CGprogram vertexProgram = cgGetPassProgram(pass, CG_VERTEX_DOMAIN);
        CGprogram fragmentProgram = cgGetPassProgram(pass, CG_FRAGMENT_DOMAIN);
        cgCompileProgram(vertexProgram);
        cgCompileProgram(fragmentProgram);
        const QUuid &vertexId = QUuid::createUuid(), &fragmentId = QUuid::createUuid();
        daeElement *vertexShaderCodeElement = profileElement->add("code");
        vertexShaderCodeElement->setAttribute("sid", qPrintable(vertexId.toString()));
        //vertexShaderCodeElement->setCharData(cgGetProgramString(vertexProgram, CG_COMPILED_PROGRAM));
        daeElement *fragmentShaderCodeElement = profileElement->add("code");
        fragmentShaderCodeElement->setAttribute("sid", qPrintable(fragmentId.toString()));
        //fragmentShaderCodeElement->setCharData(cgGetProgramString(fragmentProgram, CG_COMPILED_PROGRAM));
        daeElement *programElement = passElement->add("program");
        daeElement *vertexShaderElement = programElement->add("shader");
        vertexShaderElement->setAttribute("stage", "VERTEX");
        daeElement *vertexSourceElement = vertexShaderElement->add("sources");
        vertexSourceElement->setAttribute("entry", "main");
        vertexSourceElement->add("import")->setAttribute("ref", qPrintable(vertexId.toString()));
        daeElement *fragmentShaderElement = programElement->add("shader");
        fragmentShaderElement->setAttribute("stage", "FRAGMENT");
        daeElement *fragmentSourceElement = fragmentShaderElement->add("sources");
        fragmentSourceElement->setAttribute("entry", "main");
        fragmentSourceElement->add("import")->setAttribute("ref", qPrintable(fragmentId.toString()));
        PrintAnnotations(cgGetFirstPassAnnotation(pass), passElement);
        qDebug("PASS: name=\"%s\"", cgGetPassName(pass));
        pass = cgGetNextPass(pass);
    }
}

static void PrintTechniques(CGtechnique technique, daeElement *profileElement)
{
    while (technique) {
        daeElement *techniqueElement = profileElement->add("technique");
        techniqueElement->setAttribute("id", cgGetTechniqueName(technique));
        techniqueElement->setAttribute("sid", qPrintable(QUuid::createUuid().toString()));
        qDebug("TECHNIQUE: name=\"%s\"", cgGetTechniqueName(technique));
        PrintPasses(cgGetFirstPass(technique), techniqueElement, profileElement);
        PrintAnnotations(cgGetFirstTechniqueAnnotation(technique), techniqueElement);
        technique = cgGetNextTechnique(technique);
    }
}

}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv); Q_UNUSED(a);

    if (argc != 2) {
        qDebug("usage: cgfx2dae: [filename]");
        exit(-1);
    }

    QFileInfo finfo(argv[1]);
    QScopedPointer<struct _CGcontext, Deleter> context(cgCreateContext());
    cgGLRegisterStates(context.data());
    qDebug("Loading an effect %s", qPrintable(finfo.fileName()));
    QScopedPointer<struct _CGeffect, Deleter> effect(cgCreateEffectFromFile(context.data(), qPrintable(finfo.absoluteFilePath()), 0));
    if (!effect.data()) {
        qWarning("Cannot compile the loaded effect: %s" , cgGetLastListing(context.data()));
    }

    DAE dae;
    QString filePath(QDir::current().absoluteFilePath(finfo.baseName() + ".dae"));
    domCOLLADA *root = dynamic_cast<domCOLLADA *>(dae.add(filePath.toStdString()));
    daeElement *assetElement = root->add("asset");
    assetElement->add("contributor authoring_tool")->setCharData("cgfx2dae");
    const std::string &datetime = QDateTime::currentDateTimeUtc().toString(Qt::ISODate).toStdString();
    assetElement->add("created")->setCharData(datetime);
    assetElement->add("modified")->setCharData(datetime);
    daeElement *effectElement = root->add("library_effects effect");
    effectElement->setAttribute("id", qPrintable(finfo.baseName()));
    effectElement->setAttribute("sid", qPrintable(QUuid::createUuid().toString()));
    daeElement *profileElement = effectElement->add("profile_GLSL");
    profileElement->setAttribute("platform", "PC");
    PrintParameters(cgGetFirstEffectParameter(effect.data()), profileElement);
    PrintTechniques(cgGetFirstTechnique(effect.data()), profileElement);
    PrintAnnotations(cgGetFirstEffectAnnotation(effect.data()), effectElement);
    dae.writeAll();

    return 0;
}
