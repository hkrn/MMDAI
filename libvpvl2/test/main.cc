#include <QtCore/QtCore>
#include <gtest/gtest.h>

using namespace ::testing;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Q_UNUSED(a)
    InitGoogleTest(&argc, argv);

    return RUN_ALL_TESTS();
}
