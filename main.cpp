#include <QApplication>
#include <QScreen>
#include "mainwindow.h"
#include "database.h"   // Add this

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setStyle("Fusion");

    // Initialize database
    if (!Database::instance().initialize()) {
        qDebug() << "Failed to initialize database!";
        return -1;
    }

    MainWindow win;
    auto* screen = QGuiApplication::primaryScreen();
    win.move(screen->geometry().center() - win.rect().center());
    win.show();

    return app.exec();
}