#include <QtGui/QApplication>
#include <QtOpenGL/QtOpenGL>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <unicode/udata.h>
#include "vpvl2/Scene.h"

using namespace ::testing;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv); Q_UNUSED(a);
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QGLWidget widget;
    widget.show();
    widget.update();
    widget.hide();
    QFile file(":data/icu.dat");
    static QByteArray g_commonDataBytes;
    if (file.open(QFile::ReadOnly | QFile::Unbuffered)) {
        g_commonDataBytes = file.readAll();
        UErrorCode err = U_ZERO_ERROR;
        udata_setCommonData(g_commonDataBytes.constData(), &err);
    }
    if (!vpvl2::Scene::initialize(0)) {
        qFatal("Cannot initialize GLEW");
    }
    InitGoogleTest(&argc, argv);
    InitGoogleMock(&argc, argv);

    return RUN_ALL_TESTS();
}
