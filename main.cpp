#include "ScreenController.h"
#include <QApplication>
#include "core/DatabaseManager.h"
#include "core/LogManager.h"
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Apply dark mode stylesheet globally
    QString darkStylesheet = R"(
        QWidget {
            background-color: #1c1c1e;
            color: #f5f5f7;
        }
        QMainWindow {
            background-color: #1c1c1e;
        }
        QLabel {
            color: #f5f5f7;
            background-color: transparent;
        }
        QLineEdit {
            background-color: rgba(255, 255, 255, 0.08);
            color: #f5f5f7;
            border: 1px solid rgba(255, 255, 255, 0.12);
            border-radius: 8px;
            padding: 8px;
            selection-background-color: #0a84ff;
        }
        QLineEdit:focus {
            border: 1px solid rgba(10, 132, 255, 0.8);
            background-color: rgba(255, 255, 255, 0.10);
        }
        QPushButton {
            background-color: #0a84ff;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 8px;
            font-weight: 500;
            min-width: 80px;
            min-height: 40px;
        }
        QPushButton:hover {
            background-color: #0d7cff;
        }
        QPushButton:pressed {
            background-color: #0966d2;
        }
        QPushButton:disabled {
            background-color: #5a5a5e;
            color: #8e8e93;
        }
        QComboBox {
            background-color: rgba(255, 255, 255, 0.08);
            color: #f5f5f7;
            border: 1px solid rgba(255, 255, 255, 0.12);
            border-radius: 8px;
            padding: 6px;
            min-height: 35px;
        }
        QComboBox:focus {
            border: 1px solid rgba(10, 132, 255, 0.8);
        }
        QComboBox::drop-down {
            border: none;
            background: transparent;
            width: 30px;
        }
        QComboBox::down-arrow {
            image: none;
            border: none;
        }
        QListView, QTableView {
            background-color: rgba(255, 255, 255, 0.05);
            color: #f5f5f7;
            gridline-color: rgba(255, 255, 255, 0.1);
            border: 1px solid rgba(255, 255, 255, 0.12);
            border-radius: 8px;
        }
        QListView::item:selected, QTableView::item:selected {
            background-color: #0a84ff;
        }
        QListView::item:hover, QTableView::item:hover {
            background-color: rgba(10, 132, 255, 0.3);
        }
        QHeaderView::section {
            background-color: rgba(255, 255, 255, 0.05);
            color: #f5f5f7;
            padding: 6px;
            border: none;
            border-right: 1px solid rgba(255, 255, 255, 0.1);
        }
        QScrollBar:vertical {
            background-color: rgba(255, 255, 255, 0.03);
            width: 12px;
            border: none;
        }
        QScrollBar::handle:vertical {
            background-color: rgba(255, 255, 255, 0.2);
            border-radius: 6px;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: rgba(255, 255, 255, 0.3);
        }
        QScrollBar:horizontal {
            background-color: rgba(255, 255, 255, 0.03);
            height: 12px;
            border: none;
        }
        QScrollBar::handle:horizontal {
            background-color: rgba(255, 255, 255, 0.2);
            border-radius: 6px;
            min-width: 20px;
        }
        QScrollBar::handle:horizontal:hover {
            background-color: rgba(255, 255, 255, 0.3);
        }
        QGroupBox {
            color: #f5f5f7;
            border: 1px solid rgba(255, 255, 255, 0.12);
            border-radius: 8px;
            margin-top: 12px;
            padding-top: 12px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 12px;
            padding: 0 4px;
        }
        QCheckBox, QRadioButton {
            color: #f5f5f7;
            spacing: 8px;
        }
        QCheckBox::indicator, QRadioButton::indicator {
            width: 18px;
            height: 18px;
        }
        QCheckBox::indicator:unchecked {
            background-color: rgba(255, 255, 255, 0.08);
            border: 1px solid rgba(255, 255, 255, 0.12);
            border-radius: 4px;
        }
        QCheckBox::indicator:checked {
            background-color: #0a84ff;
            border: 1px solid #0a84ff;
            border-radius: 4px;
        }
        QTabBar::tab {
            background-color: rgba(255, 255, 255, 0.05);
            color: #f5f5f7;
            padding: 8px 16px;
            border: none;
            border-bottom: 2px solid transparent;
        }
        QTabBar::tab:selected {
            background-color: rgba(255, 255, 255, 0.08);
            border-bottom: 2px solid #0a84ff;
        }
        QTabWidget::pane {
            border: none;
        }
    )";
    
    app.setStyle("Fusion");
    app.setStyleSheet(darkStylesheet);

    // Initialize database and ensure default admin user exists
    if (!DatabaseManager::instance().initialize()) {
        qWarning() << "Failed to initialize database";
    } else {
        // ensure an admin user exists
        if (!DatabaseManager::instance().userExists("admin")) {
            QString err;
            DatabaseManager::instance().addUser("admin", "admin", true, &err);
        }
        // add a sample user if none
        if (DatabaseManager::instance().listUsers().size() <= 1) {
            QString err;
            DatabaseManager::instance().addUser("analyst1", "testpass", false, &err);
        }
    }

    // Initialize log manager
    if (!LogManager::instance().initialize()) {
        qWarning() << "Failed to initialize log manager";
    }

    ScreenController controller;
    controller.showFullScreen();

    return app.exec();
}
