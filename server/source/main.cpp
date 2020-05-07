#include "MainWindow/MainWindow.h"

#include "ClientDialog/ClientDialog.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow w;
    w.show();

    ClientDialog x;
    x.show();

    return app.exec();
}
