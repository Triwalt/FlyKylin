/**
 * @file main.cpp
 * @brief FlyKylin P2P聊天应用入口
 */

#include <QApplication>
#include "ui/windows/MainWindow.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // 创建并显示主窗口
    flykylin::ui::MainWindow mainWindow;
    mainWindow.show();
    
    return app.exec();
}
