#include "utils/StyleHelper.h"

#include <QApplication>
#include <QFile>

bool StyleHelper::applyStylesheet(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }

    QString styleSheet = QLatin1String(file.readAll());
    qApp->setStyleSheet(styleSheet);

    return true;
}

bool StyleHelper::applyLightTheme()
{
    return applyStylesheet(":/styles/light.qss");
}

bool StyleHelper::applyDarkTheme()
{
    return applyStylesheet(":/styles/dark.qss");
}
