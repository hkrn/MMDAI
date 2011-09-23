#include "shared.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    UI ui; ui.show();
    return app.exec();
}
