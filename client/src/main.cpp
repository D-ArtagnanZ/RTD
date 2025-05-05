#include <QApplication>
#include <QLabel>
#include <QMainWindow>

#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("芯片测试派工系统");
    app.setOrganizationName("芯片测试厂");

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
