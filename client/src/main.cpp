#include "mainwindow.h"
#include <QApplication>
#include <QFontDatabase>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 设置应用程序信息
    QCoreApplication::setOrganizationName("RTD+");
    QCoreApplication::setApplicationName("RTD+ Client");
    QCoreApplication::setApplicationVersion("1.0.0");

    // 设置高DPI缩放 - 使用Qt6兼容的方法
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    // 创建主窗口
    MainWindow w;
    w.show();

    return app.exec();
}
