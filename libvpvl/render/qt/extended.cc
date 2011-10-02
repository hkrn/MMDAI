#include "shared.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
#if 1
    UI ui; ui.show();
    return app.exec();
#else
    UI *ui = new UI();
    ui->show();
    delete ui;
    return 0;
#endif
}

