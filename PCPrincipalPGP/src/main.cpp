#include <QApplication>
#include <QStyleFactory>
#include <QPalette>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDir>
#include <QCommandLineParser>
#include <QSharedMemory>
#include <QSystemSemaphore>

#include "MainWindow.h"
#include "ConfigManager.h"
#include "SecureStorage.h"
#include "QuantumCrypto.h"

using namespace PCPGP;

void setupDarkTheme(QApplication& app) {
    // Set dark hacker theme
    app.setStyle(QStyleFactory::create("Fusion"));
    
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(15, 15, 20));
    darkPalette.setColor(QPalette::WindowText, QColor(200, 200, 210));
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 35));
    darkPalette.setColor(QPalette::AlternateBase, QColor(35, 35, 45));
    darkPalette.setColor(QPalette::ToolTipBase, QColor(45, 45, 55));
    darkPalette.setColor(QPalette::ToolTipText, QColor(220, 220, 230));
    darkPalette.setColor(QPalette::Text, QColor(200, 200, 210));
    darkPalette.setColor(QPalette::Button, QColor(45, 45, 55));
    darkPalette.setColor(QPalette::ButtonText, QColor(200, 200, 210));
    darkPalette.setColor(QPalette::BrightText, QColor(255, 255, 255));
    darkPalette.setColor(QPalette::Highlight, QColor(70, 130, 180));
    darkPalette.setColor(QPalette::HighlightedText, QColor(255, 255, 255));
    darkPalette.setColor(QPalette::Link, QColor(100, 160, 220));
    darkPalette.setColor(QPalette::LinkVisited, QColor(140, 100, 200));
    
    app.setPalette(darkPalette);
    
    // Set application stylesheet for additional styling
    app.setStyleSheet(R"(
        QMainWindow {
            background-color: #0f0f14;
        }
        QMenuBar {
            background-color: #1a1a24;
            color: #c8c8d2;
            border-bottom: 1px solid #2a2a3a;
        }
        QMenuBar::item:selected {
            background-color: #4682b4;
            color: #ffffff;
        }
        QMenu {
            background-color: #1a1a24;
            color: #c8c8d2;
            border: 1px solid #2a2a3a;
        }
        QMenu::item:selected {
            background-color: #4682b4;
            color: #ffffff;
        }
        QPushButton {
            background-color: #2d2d3d;
            color: #c8c8d2;
            border: 1px solid #3d3d4d;
            padding: 6px 16px;
            border-radius: 4px;
        }
        QPushButton:hover {
            background-color: #3d3d4d;
            border-color: #4682b4;
        }
        QPushButton:pressed {
            background-color: #4682b4;
        }
        QPushButton:disabled {
            background-color: #1a1a24;
            color: #5a5a6a;
        }
        QLineEdit, QTextEdit, QPlainTextEdit {
            background-color: #191923;
            color: #c8c8d2;
            border: 1px solid #2a2a3a;
            border-radius: 4px;
            padding: 4px;
            selection-background-color: #4682b4;
        }
        QLineEdit:focus, QTextEdit:focus {
            border-color: #4682b4;
        }
        QComboBox {
            background-color: #2d2d3d;
            color: #c8c8d2;
            border: 1px solid #3d3d4d;
            border-radius: 4px;
            padding: 4px 8px;
        }
        QComboBox:hover {
            border-color: #4682b4;
        }
        QComboBox::drop-down {
            border: none;
            width: 24px;
        }
        QComboBox QAbstractItemView {
            background-color: #1a1a24;
            color: #c8c8d2;
            border: 1px solid #2a2a3a;
            selection-background-color: #4682b4;
        }
        QTableWidget {
            background-color: #191923;
            color: #c8c8d2;
            border: 1px solid #2a2a3a;
            gridline-color: #2a2a3a;
        }
        QTableWidget::item:selected {
            background-color: #4682b4;
        }
        QHeaderView::section {
            background-color: #2d2d3d;
            color: #c8c8d2;
            padding: 6px;
            border: 1px solid #3d3d4d;
        }
        QTabWidget::pane {
            border: 1px solid #2a2a3a;
            background-color: #191923;
        }
        QTabBar::tab {
            background-color: #2d2d3d;
            color: #c8c8d2;
            padding: 8px 16px;
            border: 1px solid #3d3d4d;
            border-bottom: none;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
        }
        QTabBar::tab:selected {
            background-color: #4682b4;
            color: #ffffff;
        }
        QTabBar::tab:hover:!selected {
            background-color: #3d3d4d;
        }
        QGroupBox {
            color: #c8c8d2;
            border: 1px solid #2a2a3a;
            border-radius: 4px;
            margin-top: 8px;
            padding-top: 8px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 8px;
            padding: 0 4px;
        }
        QScrollBar:vertical {
            background-color: #1a1a24;
            width: 12px;
            border-radius: 6px;
        }
        QScrollBar::handle:vertical {
            background-color: #3d3d4d;
            border-radius: 6px;
            min-height: 20px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: #4682b4;
        }
        QScrollBar:horizontal {
            background-color: #1a1a24;
            height: 12px;
            border-radius: 6px;
        }
        QScrollBar::handle:horizontal {
            background-color: #3d3d4d;
            border-radius: 6px;
            min-width: 20px;
        }
        QScrollBar::handle:horizontal:hover {
            background-color: #4682b4;
        }
        QProgressBar {
            border: 1px solid #2a2a3a;
            border-radius: 4px;
            text-align: center;
            color: #c8c8d2;
        }
        QProgressBar::chunk {
            background-color: #4682b4;
            border-radius: 3px;
        }
        QToolBar {
            background-color: #1a1a24;
            border: 1px solid #2a2a3a;
            spacing: 4px;
            padding: 4px;
        }
        QToolButton {
            background-color: #2d2d3d;
            color: #c8c8d2;
            border: 1px solid #3d3d4d;
            border-radius: 4px;
            padding: 4px;
        }
        QToolButton:hover {
            background-color: #3d3d4d;
            border-color: #4682b4;
        }
        QStatusBar {
            background-color: #1a1a24;
            color: #c8c8d2;
            border-top: 1px solid #2a2a3a;
        }
        QDialog {
            background-color: #0f0f14;
        }
        QWizard {
            background-color: #0f0f14;
        }
        QWizardPage {
            background-color: #0f0f14;
        }
        QListWidget {
            background-color: #191923;
            color: #c8c8d2;
            border: 1px solid #2a2a3a;
            border-radius: 4px;
        }
        QListWidget::item:selected {
            background-color: #4682b4;
        }
        QCheckBox {
            color: #c8c8d2;
        }
        QCheckBox::indicator {
            width: 16px;
            height: 16px;
            border: 1px solid #3d3d4d;
            border-radius: 3px;
            background-color: #191923;
        }
        QCheckBox::indicator:checked {
            background-color: #4682b4;
            border-color: #4682b4;
        }
        QRadioButton {
            color: #c8c8d2;
        }
        QRadioButton::indicator {
            width: 16px;
            height: 16px;
            border: 1px solid #3d3d4d;
            border-radius: 8px;
            background-color: #191923;
        }
        QRadioButton::indicator:checked {
            background-color: #4682b4;
            border-color: #4682b4;
        }
    )");
}

bool ensureSingleInstance() {
    // Use shared memory to ensure single instance
    static QSharedMemory sharedMemory("PCPrincipalPGP_SingleInstance");
    
    if (sharedMemory.attach()) {
        // Another instance is running
        return false;
    }
    
    sharedMemory.create(1);
    return true;
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    // Application info
    app.setApplicationName("PCPrincipalPGP");
    app.setApplicationDisplayName("PC Principal PGP");
    app.setOrganizationName("PCPrincipalPGP");
    app.setApplicationVersion("1.0.0");
    
    // Setup dark theme
    setupDarkTheme(app);
    
    // Command line parser
    QCommandLineParser parser;
    parser.setApplicationDescription("PC Principal PGP - Portable PGP Encryption Tool");
    parser.addHelpOption();
    parser.addVersionOption();
    
    QCommandLineOption encryptOption(QStringList() << "e" << "encrypt",
        "Encrypt a file", "file");
    QCommandLineOption decryptOption(QStringList() << "d" << "decrypt",
        "Decrypt a file", "file");
    QCommandLineOption signOption(QStringList() << "s" << "sign",
        "Sign a file", "file");
    QCommandLineOption verifyOption(QStringList() << "v" << "verify",
        "Verify a signature", "file");
    QCommandLineOption recipientOption(QStringList() << "r" << "recipient",
        "Recipient key ID for encryption", "keyid");
    QCommandLineOption keyOption(QStringList() << "k" << "key",
        "Key ID for signing", "keyid");
    QCommandLineOption outputOption(QStringList() << "o" << "output",
        "Output file", "file");
    QCommandLineOption importOption(QStringList() << "i" << "import",
        "Import a key file", "file");
    QCommandLineOption profileOption(QStringList() << "p" << "profile",
        "Use profile for operation", "profile");
    QCommandLineOption setupOption(QStringList() << "setup",
        "Run setup wizard");
    QCommandLineOption noGuiOption(QStringList() << "nogui",
        "Run without GUI (command line mode)");
    
    parser.addOption(encryptOption);
    parser.addOption(decryptOption);
    parser.addOption(signOption);
    parser.addOption(verifyOption);
    parser.addOption(recipientOption);
    parser.addOption(keyOption);
    parser.addOption(outputOption);
    parser.addOption(importOption);
    parser.addOption(profileOption);
    parser.addOption(setupOption);
    parser.addOption(noGuiOption);
    
    parser.process(app);
    
    // Check for single instance (only in GUI mode)
    bool guiMode = !parser.isSet(noGuiOption);
    if (guiMode && !ensureSingleInstance()) {
        QMessageBox::warning(nullptr, "PC Principal PGP",
            "Another instance of PC Principal PGP is already running.");
        return 1;
    }
    
    // Initialize libsodium
    if (!QuantumCrypto::initialize()) {
        QMessageBox::critical(nullptr, "Initialization Error",
            "Failed to initialize cryptographic library.");
        return 1;
    }
    
    // Initialize configuration
    ConfigManager& config = ConfigManager::getInstance();
    if (!config.initialize()) {
        QMessageBox::critical(nullptr, "Configuration Error",
            "Failed to initialize configuration.");
        return 1;
    }
    
    // Check if first run or setup requested
    bool firstRun = config.isFirstRun() || parser.isSet(setupOption);
    
    // Create and show main window
    MainWindow window;
    
    if (!window.initialize()) {
        QMessageBox::critical(nullptr, "Initialization Error",
            "Failed to initialize PGP manager.");
        return 1;
    }
    
    // Handle command line operations
    if (parser.isSet(encryptOption) || parser.isSet(decryptOption) ||
        parser.isSet(signOption) || parser.isSet(verifyOption) ||
        parser.isSet(importOption)) {
        // Command line mode operations would go here
        // For now, just show the GUI
    }
    
    window.show();
    
    return app.exec();
}
