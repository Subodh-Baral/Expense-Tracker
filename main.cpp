#include <QApplication>
#include "mainwindow.h"
#include "database.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setStyle("Fusion");

    if (!Database::instance().initialize()) {
        return 1;
    }

    MainWindow win;
    win.show();

    return app.exec();
}