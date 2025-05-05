#pragma once

#include <QApplication>
#include <QString>

// 确保没有隐藏的命名空间
class StyleHelper {
    public:
        // 应用样式文件
        static bool applyStylesheet(const QString &path);

        // 应用亮色/暗色主题
        static bool applyLightTheme();
        static bool applyDarkTheme();

    private:
        StyleHelper() = delete;    // 禁止实例化
};
