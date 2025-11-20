#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setApplicationName("STS-SaveEditor");
    a.setOrganizationName("Moonshot");
    
    MainWindow w;
    w.show();
    return a.exec();
}