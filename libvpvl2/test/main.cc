#include <QApplication>
#include <QtOpenGL>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <unicode/udata.h>

#include "vpvl2/Scene.h"
#include "../../src/ext/ICUCommonData.inl"

using namespace ::testing;

int main(int argc, char *argv[])
{
#ifdef VPVL2_LINK_GLOG
    FLAGS_logtostderr = true;
    FLAGS_v = 1;
    google::InitGoogleLogging(argv[0]);
#endif
    QApplication a(argc, argv); Q_UNUSED(a);
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
#endif
    UErrorCode code = U_ZERO_ERROR;
    udata_setCommonData(g_icudt52l_dat, &code);
    QGLWidget widget;
    widget.show();
    widget.hide();
    if (!vpvl2::Scene::initialize(0)) {
        qFatal("Cannot initialize GLEW");
    }
    InitGoogleTest(&argc, argv);
    InitGoogleMock(&argc, argv);

    return RUN_ALL_TESTS();
}
