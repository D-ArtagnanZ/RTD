#pragma once

#include <QAction>
#include <QLabel>
#include <QMainWindow>
#include <QTabWidget>
#include <QTimer>

class ApiClient;
class DispatchView;
class DispatchModel;
class LotModel;
class EquipmentModel;
class LotListView;
class EquipmentListView;

class MainWindow: public QMainWindow {
        Q_OBJECT

    public:
        explicit MainWindow(QWidget *parent = nullptr);
        ~MainWindow();

    public slots:
        void refreshAllData();

    private slots:
        void switchTheme();
        void showSettings();
        void showAbout();
        void handleApiConnectionStatus(bool connected);

    private:
        void setupUi();
        void createActions();
        void createMenus();
        void setupModels();

        // API客户端
        ApiClient *m_apiClient;

        // 数据模型
        DispatchModel  *m_dispatchModel;
        LotModel       *m_lotModel;
        EquipmentModel *m_equipmentModel;

        // UI组件
        QTabWidget        *m_tabWidget;
        DispatchView      *m_dispatchView;
        LotListView       *m_lotListView;
        EquipmentListView *m_equipmentView;

        // 刷新定时器
        QTimer *m_refreshTimer;

        // 状态栏组件
        QLabel *m_timeLabel;

        // 菜单动作
        QAction *m_refreshAction;
        QAction *m_exitAction;
        QAction *m_themeAction;
        QAction *m_settingsAction;
        QAction *m_aboutAction;

        bool m_darkTheme;
};
