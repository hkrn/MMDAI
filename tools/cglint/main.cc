#include <QCoreApplication>

#include <Cg/cg.h>
#include <Cg/cgGL.h> /* for cgGLRegisterStates */

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

}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv); Q_UNUSED(a);
    if (argc != 2) {
        qDebug("usage: cglint: [filename]");
        exit(-1);
    }
    const char *filename = argv[1];

    QScopedPointer<struct _CGcontext, Deleter> context(cgCreateContext());
    cgGLRegisterStates(context.data());

    qDebug("Loading an effect %s", filename);
    QScopedPointer<struct _CGeffect, Deleter> effect(cgCreateEffectFromFile(context.data(), filename, 0));
    if (!effect.data()) {
        qWarning("Cannot compile the loaded effect: %s" , cgGetLastListing(context.data()));
    }

    CGtechnique technique = cgGetFirstTechnique(effect.data());
    while (technique) {
        qDebug("Technique: %s", cgGetTechniqueName(technique));
        CGpass pass = cgGetFirstPass(technique);
        while (pass) {
            CGprogram vertexProgarm = cgGetPassProgram(pass, CG_VERTEX_DOMAIN);
            CGprogram fragmentProgarm = cgGetPassProgram(pass, CG_FRAGMENT_DOMAIN);
            cgSetPassState(pass);
            const char *v = cgGetProgramString(vertexProgarm, CG_COMPILED_PROGRAM);
            const char *f = cgGetProgramString(fragmentProgarm, CG_COMPILED_PROGRAM);
            bool vok = strlen(v) > 0,  fok = strlen(f) > 0;
            qDebug("Pass: %s vertexShader=%s fragmentShader=%s",
                   cgGetPassName(pass), vok ? "OK" : "NG", fok ? "OK" : "NG");
            if (!vok || !fok) {
                if (const char *message = cgGetLastListing(context.data())) {
                    qWarning("%s", message);
                }
                technique = 0;
                pass = 0;
                break;
            }
            cgResetPassState(pass);
            pass = cgGetNextPass(pass);
        }
        technique = cgGetNextTechnique(technique);
    }

    return 0;
}
