#include <QtGui/QApplication>
#include <QtOpenGL/QtOpenGL>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace ::testing;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv); Q_UNUSED(a)
    QGLWidget widget;
    widget.show();
    widget.update();
    widget.hide();
    InitGoogleTest(&argc, argv);
    InitGoogleMock(&argc, argv);

    return RUN_ALL_TESTS();
}
