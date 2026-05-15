#include "mainwindow.h"
#include <QApplication>
#include <QFont>
#include <QFontDatabase>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Banker's Algorithm Simulator");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("OS Project");

    // Load a clean system font
    QFont defaultFont("Segoe UI", 10);
    defaultFont.setStyleHint(QFont::SansSerif);
    app.setFont(defaultFont);

    MainWindow w;
    w.show();
    return app.exec();
}
