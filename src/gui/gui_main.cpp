#include "mainwindow.h"
#include <QApplication>
#include <iostream>
#include <QDebug>

int main(int argc, char *argv[])
{
    try {
        std::cout << "[STSSaveEditor] main() start" << std::endl;
        QApplication a(argc, argv);
        std::cout << "[STSSaveEditor] after QApplication" << std::endl;
        a.setApplicationName("STS-SaveEditor");
        a.setOrganizationName("Moonshot");

        qDebug() << "[STSSaveEditor] Creating MainWindow";
        MainWindow w;
        qDebug() << "[STSSaveEditor] MainWindow constructed";
        w.show();
        qDebug() << "[STSSaveEditor] MainWindow shown, entering event loop";
        return a.exec();
    } catch (const std::exception &e) {
        qCritical() << "Unhandled std::exception in main():" << e.what();
        return 1;
    } catch (...) {
        qCritical() << "Unhandled unknown exception in main()";
        return 1;
    }
}