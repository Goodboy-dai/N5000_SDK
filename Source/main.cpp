#include "launch.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    launch usrApp;
    usrApp.resize(500,700);
    usrApp.setWindowTitle("N5000");
    usrApp.show();
    return a.exec();

}
