#include <QApplication>
#include <QDebug>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    // Run GUI as user
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}
