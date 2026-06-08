#include <QApplication>
#include <QScreen>
#include "mainwindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setStyle("Fusion");

    MainWindow win;
    auto* screen = QGuiApplication::primaryScreen();
    win.move(screen->geometry().center() - win.rect().center());
    win.show();

    return app.exec();
}