#pragma once

#include "apiclient.h"
#include "dispatchwidget.h"
#include "equipmentwidget.h"
#include "lotwidget.h"
#include <QButtonGroup>
#include <QDialog>
#include <QFile>    // 添加QFile头文件
#include <QFrame>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace Ui {
class MainWindow;
}

class MainWindow: public QMainWindow {
        Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = nullptr);
        ~MainWindow();

    private slots:
        void onNavButtonClicked(int id);
        void onServerSettingsClicked();
        void handleNetworkError(const QString &errorMessage);

    private:
        Ui::MainWindow *ui;
        ApiClient      *m_apiClient;
        QButtonGroup   *m_navButtons;

        EquipmentWidget *m_equipmentWidget;
        LotWidget       *m_lotWidget;
        DispatchWidget  *m_dispatchWidget;

        void setupUi();
        void setupConnections();
        void loadStyleSheet();
};
