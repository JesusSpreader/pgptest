#include "MainWindow.h"
#include "PGPManager.h"
#include "NotepadWidget.h"
#include "KeyManagerDialog.h"
#include "ProfileManager.h"
#include "SetupWizard.h"
#include "EncryptDialog.h"
#include "ConfigManager.h"
#include "SecureStorage.h"
#include "PasswordDialog.h"
#include "MassImportDialog.h"
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QAction>
#include <QMessageBox>
#include <QCloseEvent>
#include <QApplication>
#include <QSplitter>
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QFileDialog>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QPixmap>
#include <QTabWidget>
#include <QClipboard>
#include <QTimer>

namespace PCPGP {

MainWindow::MainWindow(QWidget* parent) 
    : QMainWindow(parent), m_locked(false), m_modified(false) {
    
    setWindowTitle("PC Principal PGP");
    setMinimumSize(1000, 700);
    resize(1200, 800);
    
    // Set window icon
    QPixmap iconPixmap(":/icon.png");
    if (!iconPixmap.isNull()) {
        setWindowIcon(iconPixmap);
    }
}

MainWindow::~MainWindow() {
}

bool MainWindow::initialize() {
    // Create PGP manager
    m_pgpManager = std::make_unique<PGPManager>();
    if (!m_pgpManager->initialize()) {
        QMessageBox::critical(this, "Error", 
            "Failed to initialize PGP manager: " + m_pgpManager->getLastError());
        return false;
    }
    
    // Create profile manager
    m_profileManager = std::make_unique<ProfileManager>(m_pgpManager.get(), this);
    if (!m_profileManager->initialize()) {
        qWarning() << "Failed to initialize profile manager";
    }
    
    // Setup UI
    setupUI();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    setupTrayIcon();
    setupConnections();
    
    // Check first run
    checkFirstRun();
    
    // Load profiles
    loadProfiles();
    
    // Update UI
    updateStatusBar();
    updateWindowTitle();
    
    return true;
}

void MainWindow::setupUI() {
    // Central widget
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // Header with icon and title
    QWidget* headerWidget = new QWidget(this);
    headerWidget->setFixedHeight(80);
    headerWidget->setStyleSheet("background-color: #1a1a24; border-bottom: 2px solid #2a2a3a;");
    
    QHBoxLayout* headerLayout = new QHBoxLayout(headerWidget);
    headerLayout->setContentsMargins(20, 10, 20, 10);
    
    // Icon
    QLabel* iconLabel = new QLabel(headerWidget);
    QPixmap iconPixmap(":/icon.png");
    if (!iconPixmap.isNull()) {
        iconLabel->setPixmap(iconPixmap.scaled(60, 60, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    headerLayout->addWidget(iconLabel);
    
    // Title
    QLabel* titleLabel = new QLabel("PC Principal PGP", headerWidget);
    titleLabel->setStyleSheet("font-size: 24px; font-weight: bold; color: #4682b4; border: none;");
    headerLayout->addWidget(titleLabel);
    
    headerLayout->addStretch();
    
    // Profile selector
    QLabel* profileLabel = new QLabel("Profile:", headerWidget);
    profileLabel->setStyleSheet("color: #c8c8d2; border: none;");
    headerLayout->addWidget(profileLabel);
    
    m_profileCombo = new QComboBox(headerWidget);
    m_profileCombo->setFixedWidth(250);
    headerLayout->addWidget(m_profileCombo);
    
    mainLayout->addWidget(headerWidget);
    
    // Tab widget
    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setDocumentMode(true);
    
    // Notepad tab
    m_notepad = new NotepadWidget(m_pgpManager.get(), this);
    m_tabWidget->addTab(m_notepad, "Notepad");
    
    // Key Manager tab (placeholder - opens dialog)
    QWidget* keyWidget = new QWidget(this);
    QVBoxLayout* keyLayout = new QVBoxLayout(keyWidget);
    QLabel* keyLabel = new QLabel("Use Key Manager from the Keys menu to manage your PGP keys.", keyWidget);
    keyLabel->setAlignment(Qt::AlignCenter);
    keyLayout->addWidget(keyLabel);
    
    QPushButton* openKeyManagerBtn = new QPushButton("Open Key Manager", keyWidget);
    connect(openKeyManagerBtn, &QPushButton::clicked, this, &MainWindow::onManageKeys);
    keyLayout->addWidget(openKeyManagerBtn, 0, Qt::AlignCenter);
    keyLayout->addStretch();
    
    m_tabWidget->addTab(keyWidget, "Keys");
    
    // Profiles tab (placeholder)
    QWidget* profileWidget = new QWidget(this);
    QVBoxLayout* profileLayout = new QVBoxLayout(profileWidget);
    QLabel* profileInfoLabel = new QLabel("Manage your contact profiles here.", profileWidget);
    profileInfoLabel->setAlignment(Qt::AlignCenter);
    profileLayout->addWidget(profileInfoLabel);
    
    QPushButton* openProfileManagerBtn = new QPushButton("Open Profile Manager", profileWidget);
    connect(openProfileManagerBtn, &QPushButton::clicked, this, &MainWindow::onManageProfiles);
    profileLayout->addWidget(openProfileManagerBtn, 0, Qt::AlignCenter);
    profileLayout->addStretch();
    
    m_tabWidget->addTab(profileWidget, "Profiles");
    
    mainLayout->addWidget(m_tabWidget, 1);
}

void MainWindow::setupMenuBar() {
    QMenuBar* menuBar = this->menuBar();
    
    // File menu
    QMenu* fileMenu = menuBar->addMenu("&File");
    
    fileMenu->addAction("&New", this, &MainWindow::onNewKey, QKeySequence::New);
    fileMenu->addAction("&Import Key...", this, &MainWindow::onImportKey, QKeySequence("Ctrl+I"));
    fileMenu->addAction("&Export Key...", this, &MainWindow::onExportKey);
    fileMenu->addSeparator();
    fileMenu->addAction("&Preferences...", this, &MainWindow::onPreferences);
    fileMenu->addSeparator();
    fileMenu->addAction("E&xit", this, &MainWindow::onExit, QKeySequence::Quit);
    
    // Keys menu
    QMenu* keysMenu = menuBar->addMenu("&Keys");
    
    keysMenu->addAction("&Manage Keys...", this, &MainWindow::onManageKeys, QKeySequence("Ctrl+K"));
    keysMenu->addAction("&New Profile...", this, &MainWindow::onNewProfile);
    keysMenu->addAction("&Import Profile...", this, &MainWindow::onImportProfile);
    keysMenu->addAction("&Mass Import...", this, &MainWindow::onMassImport);
    keysMenu->addSeparator();
    keysMenu->addAction("&Copy Public Key", this, &MainWindow::onCopyPublicKey);
    
    // Encryption menu
    QMenu* encryptMenu = menuBar->addMenu("&Encryption");
    
    encryptMenu->addAction("&Encrypt Message...", this, &MainWindow::onEncryptMessage, QKeySequence("Ctrl+E"));
    encryptMenu->addAction("&Decrypt Message...", this, &MainWindow::onDecryptMessage, QKeySequence("Ctrl+D"));
    encryptMenu->addSeparator();
    encryptMenu->addAction("&Sign Message...", this, &MainWindow::onSignMessage, QKeySequence("Ctrl+Shift+S"));
    encryptMenu->addAction("&Verify Message...", this, &MainWindow::onVerifyMessage);
    encryptMenu->addSeparator();
    encryptMenu->addAction("Encrypt &File...", this, &MainWindow::onEncryptFile);
    encryptMenu->addAction("Decrypt F&ile...", this, &MainWindow::onDecryptFile);
    encryptMenu->addAction("Sign F&ile...", this, &MainWindow::onSignFile);
    encryptMenu->addAction("Verif&y File...", this, &MainWindow::onVerifyFile);
    
    // Security menu
    QMenu* securityMenu = menuBar->addMenu("&Security");
    
    securityMenu->addAction("&Lock Application", this, &MainWindow::onLockApplication, QKeySequence("Ctrl+L"));
    securityMenu->addAction("&Change Password...", this, &MainWindow::onChangePassword);
    securityMenu->addSeparator();
    securityMenu->addAction("&Post-Quantum Settings...", this, &MainWindow::onPostQuantumSettings);
    securityMenu->addAction("&Decrypt Storage...", this, &MainWindow::onDecryptStorage);
    
    // Help menu
    QMenu* helpMenu = menuBar->addMenu("&Help");
    
    helpMenu->addAction("&About", this, &MainWindow::onAbout);
    helpMenu->addAction("&Documentation", this, &MainWindow::onDocumentation);
}

void MainWindow::setupToolBar() {
    QToolBar* toolBar = addToolBar("Main");
    toolBar->setMovable(false);
    
    toolBar->addAction("Encrypt", this, &MainWindow::onQuickEncrypt);
    toolBar->addAction("Decrypt", this, &MainWindow::onQuickDecrypt);
    toolBar->addSeparator();
    toolBar->addAction("Sign", this, &MainWindow::onQuickSign);
    toolBar->addAction("Verify", this, &MainWindow::onQuickVerify);
    toolBar->addSeparator();
    toolBar->addAction("Keys", this, &MainWindow::onManageKeys);
    toolBar->addAction("Profiles", this, &MainWindow::onManageProfiles);
}

void MainWindow::setupStatusBar() {
    QStatusBar* statusBar = this->statusBar();
    
    m_statusLabel = new QLabel("Ready", this);
    statusBar->addWidget(m_statusLabel);
    
    // Use permanent widget instead of addStretch
    m_securityLabel = new QLabel("Secure", this);
    m_securityLabel->setStyleSheet("color: #4CAF50;");
    statusBar->addPermanentWidget(m_securityLabel);
}

void MainWindow::setupTrayIcon() {
    m_trayIcon = new QSystemTrayIcon(this);
    QPixmap iconPixmap(":/icon.png");
    if (!iconPixmap.isNull()) {
        m_trayIcon->setIcon(iconPixmap);
    }
    m_trayIcon->setToolTip("PC Principal PGP");
    
    m_trayMenu = new QMenu(this);
    m_trayMenu->addAction("Show", this, &MainWindow::onShowWindow);
    m_trayMenu->addAction("Hide", this, &MainWindow::onHideWindow);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction("Exit", this, &MainWindow::onExit);
    
    m_trayIcon->setContextMenu(m_trayMenu);
    
    connect(m_trayIcon, &QSystemTrayIcon::activated, 
            this, &MainWindow::onTrayIconActivated);
    
    m_trayIcon->show();
}

void MainWindow::setupConnections() {
    connect(m_notepad, &NotepadWidget::textModified, this, [this](bool modified) {
        m_modified = modified;
        updateWindowTitle();
    });
    
    connect(m_notepad, &NotepadWidget::statusMessage, this, [this](const QString& msg) {
        m_statusLabel->setText(msg);
    });
    
    connect(m_profileCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onSelectProfile);
}

void MainWindow::checkFirstRun() {
    ConfigManager& config = ConfigManager::getInstance();
    
    if (config.isFirstRun()) {
        SetupWizard wizard(this);
        if (wizard.exec() == QDialog::Accepted) {
            // Apply settings from wizard
            config.setPortableMode(wizard.getPortableMode());
            if (wizard.getPortableMode() == PortableMode::PARTIAL_PORTABLE) {
                config.setCustomPublicKeysDir(wizard.getPublicKeysPath());
                config.setCustomPrivateKeysDir(wizard.getPrivateKeysPath());
            }
            config.setPostQuantumEnabled(wizard.isPostQuantumEnabled());
            config.setEncryptPrivateKeysOnly(wizard.isEncryptPrivateOnly());
            
            QString password = wizard.getPassword();
            if (!password.isEmpty()) {
                config.setPasswordProtected(true);
                SecureStorage::getInstance().initialize(password);
            }
            
            config.saveConfig();
        }
    } else if (config.isPasswordProtected()) {
        // Ask for password
        if (!PasswordDialog::unlock(this)) {
            QMessageBox::critical(this, "Access Denied", 
                "Incorrect password. Application will close.");
            QApplication::quit();
        }
    }
}

void MainWindow::loadProfiles() {
    if (m_profileManager) {
        m_profileManager->loadProfiles();
        updateProfileCombo();
    }
}

void MainWindow::updateProfileCombo() {
    m_profileCombo->clear();
    m_profileCombo->addItem("-- Select Profile --");
    
    auto profiles = m_profileManager->getAllProfiles();
    for (const auto& profile : profiles) {
        m_profileCombo->addItem(profile.getDisplayName(), profile.getKeyId());
    }
}

void MainWindow::updateStatusBar() {
    ConfigManager& config = ConfigManager::getInstance();
    
    QString mode = (config.getPortableMode() == PortableMode::DIRECTORY_MODE) 
                   ? "Fully Portable" : "Partial Portable";
    
    QString pqStatus = config.isPostQuantumEnabled() ? "PQ Enabled" : "Standard";
    
    m_statusLabel->setText(QString("Mode: %1 | %2").arg(mode, pqStatus));
}

void MainWindow::updateWindowTitle() {
    QString title = "PC Principal PGP";
    if (m_modified) {
        title = "* " + title;
    }
    if (m_locked) {
        title = "[LOCKED] " + title;
    }
    setWindowTitle(title);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (m_modified) {
        auto reply = QMessageBox::question(this, "Unsaved Changes",
            "You have unsaved changes. Do you want to save before exiting?",
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        
        if (reply == QMessageBox::Save) {
            // Save logic here
        } else if (reply == QMessageBox::Cancel) {
            event->ignore();
            return;
        }
    }
    
    event->accept();
}

void MainWindow::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent* event) {
    const QMimeData* mimeData = event->mimeData();
    
    if (mimeData->hasUrls()) {
        for (const QUrl& url : mimeData->urls()) {
            QString filePath = url.toLocalFile();
            if (!filePath.isEmpty()) {
                handleDroppedFile(filePath);
            }
        }
    }
}

void MainWindow::handleDroppedFile(const QString& filePath) {
    // Determine file type and handle accordingly
    QFileInfo fileInfo(filePath);
    QString suffix = fileInfo.suffix().toLower();
    
    if (suffix == "asc" || suffix == "gpg" || suffix == "pgp") {
        // PGP file - ask what to do
        auto reply = QMessageBox::question(this, "PGP File Detected",
            "Import this PGP file or decrypt/verify it?",
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
            QMessageBox::Yes);
        
        if (reply == QMessageBox::Yes) {
            m_pgpManager->importKeyFromFile(filePath);
        } else if (reply == QMessageBox::No) {
            // Decrypt/verify
        }
    } else {
        // Regular file - encrypt
        auto reply = QMessageBox::question(this, "File Dropped",
            "Encrypt this file?",
            QMessageBox::Yes | QMessageBox::No);
        
        if (reply == QMessageBox::Yes) {
            onEncryptFile();
        }
    }
}

// Menu action handlers
void MainWindow::onNewKey() {
    // Open generate key dialog
}

void MainWindow::onImportKey() {
    QString filePath = QFileDialog::getOpenFileName(this, "Import Key", QString(),
        "PGP Keys (*.asc *.gpg *.pgp *.pub *.key);;All Files (*)");
    
    if (!filePath.isEmpty()) {
        if (m_pgpManager->importKeyFromFile(filePath)) {
            showSuccess("Key imported successfully");
        } else {
            showError("Failed to import key: " + m_pgpManager->getLastError());
        }
    }
}

void MainWindow::onExportKey() {
    // Open export dialog
}

void MainWindow::onManageKeys() {
    if (!m_keyManagerDialog) {
        m_keyManagerDialog = new KeyManagerDialog(m_pgpManager.get(), this);
    }
    m_keyManagerDialog->show();
    m_keyManagerDialog->raise();
    m_keyManagerDialog->activateWindow();
}

void MainWindow::onManageProfiles() {
    // Open profile manager
}

void MainWindow::onPreferences() {
    // Open preferences dialog
}

void MainWindow::onExit() {
    close();
}

void MainWindow::onEncryptMessage() {
    m_notepad->onEncryptClicked();
}

void MainWindow::onDecryptMessage() {
    m_notepad->onDecryptClicked();
}

void MainWindow::onSignMessage() {
    m_notepad->onSignClicked();
}

void MainWindow::onVerifyMessage() {
    m_notepad->onVerifyClicked();
}

void MainWindow::onEncryptFile() {
    QString inputPath = QFileDialog::getOpenFileName(this, "Select File to Encrypt");
    if (inputPath.isEmpty()) return;
    
    QString outputPath = QFileDialog::getSaveFileName(this, "Save Encrypted File",
        inputPath + ".gpg", "PGP Files (*.gpg *.asc *.pgp)");
    if (outputPath.isEmpty()) return;
    
    // Get recipient
    QString recipient = m_profileCombo->currentData().toString();
    if (recipient.isEmpty()) {
        showError("Please select a recipient profile");
        return;
    }
    
    if (m_pgpManager->encryptFile(inputPath, outputPath, recipient)) {
        showSuccess("File encrypted successfully");
    } else {
        showError("Failed to encrypt file: " + m_pgpManager->getLastError());
    }
}

void MainWindow::onDecryptFile() {
    QString inputPath = QFileDialog::getOpenFileName(this, "Select File to Decrypt",
        QString(), "PGP Files (*.gpg *.asc *.pgp)");
    if (inputPath.isEmpty()) return;
    
    QString outputPath = QFileDialog::getSaveFileName(this, "Save Decrypted File");
    if (outputPath.isEmpty()) return;
    
    QString keyId;
    if (m_pgpManager->decryptFile(inputPath, outputPath, keyId)) {
        showSuccess("File decrypted successfully");
    } else {
        showError("Failed to decrypt file: " + m_pgpManager->getLastError());
    }
}

void MainWindow::onSignFile() {
    QString inputPath = QFileDialog::getOpenFileName(this, "Select File to Sign");
    if (inputPath.isEmpty()) return;
    
    QString outputPath = QFileDialog::getSaveFileName(this, "Save Signature",
        inputPath + ".sig", "Signature Files (*.sig *.asc)");
    if (outputPath.isEmpty()) return;
    
    QString keyId = m_profileCombo->currentData().toString();
    if (keyId.isEmpty()) {
        showError("Please select a signing profile");
        return;
    }
    
    if (m_pgpManager->signFile(inputPath, outputPath, keyId, true)) {
        showSuccess("File signed successfully");
    } else {
        showError("Failed to sign file: " + m_pgpManager->getLastError());
    }
}

void MainWindow::onVerifyFile() {
    QString filePath = QFileDialog::getOpenFileName(this, "Select File to Verify");
    if (filePath.isEmpty()) return;
    
    QString sigPath = QFileDialog::getOpenFileName(this, "Select Signature File",
        QString(), "Signature Files (*.sig *.asc)");
    if (sigPath.isEmpty()) return;
    
    QString signerKeyId;
    if (m_pgpManager->verifyFile(filePath, sigPath, signerKeyId)) {
        showSuccess("Signature verified. Signed by: " + signerKeyId);
    } else {
        showError("Signature verification failed");
    }
}

void MainWindow::onNewProfile() {
    // Open new profile dialog
}

void MainWindow::onImportProfile() {
    QString filePath = QFileDialog::getOpenFileName(this, "Import Profile",
        QString(), "Profile Files (*.json);;All Files (*)");
    
    if (!filePath.isEmpty()) {
        // Import profile logic
    }
}

void MainWindow::onSelectProfile() {
    QString keyId = m_profileCombo->currentData().toString();
    if (!keyId.isEmpty() && m_profileManager) {
        m_profileManager->setCurrentProfile(m_profileCombo->currentText());
    }
}

void MainWindow::onMassImport() {
    MassImportDialog dialog(m_pgpManager.get(), this);
    dialog.exec();
}

void MainWindow::onLockApplication() {
    SecureStorage::getInstance().lock();
    m_locked = true;
    updateWindowTitle();
    
    // Disable sensitive UI
    m_notepad->setEnabled(false);
    
    m_securityLabel->setText("Locked");
    m_securityLabel->setStyleSheet("color: #f44336;");
}

void MainWindow::onChangePassword() {
    QString newPassword;
    if (PasswordDialog::changePassword(this, newPassword)) {
        showSuccess("Password changed successfully");
    }
}

void MainWindow::onPostQuantumSettings() {
    // Open PQ settings dialog
}

void MainWindow::onDecryptStorage() {
    auto reply = QMessageBox::warning(this, "Decrypt Storage",
        "This will decrypt all stored data. Are you sure?",
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        // Decrypt storage logic
    }
}

void MainWindow::onAbout() {
    QMessageBox::about(this, "About PC Principal PGP",
        "<h2>PC Principal PGP 1.0.0</h2>"
        "<p>A portable, secure PGP encryption application.</p>"
        "<p>Features:</p>"
        "<ul>"
        "<li>100% Portable - works from USB</li>"
        "<li>Post-quantum encryption support</li>"
        "<li>Profile management for contacts</li>"
        "<li>Message and file encryption/signing</li>"
        "</ul>");
}

void MainWindow::onDocumentation() {
    // Open documentation
    QMessageBox::information(this, "Documentation",
        "Documentation is available in the README file.");
}

// Tray icon handlers
void MainWindow::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason == QSystemTrayIcon::DoubleClick) {
        onShowWindow();
    }
}

void MainWindow::onShowWindow() {
    show();
    raise();
    activateWindow();
}

void MainWindow::onHideWindow() {
    hide();
}

// Quick action handlers
void MainWindow::onQuickEncrypt() {
    onEncryptMessage();
}

void MainWindow::onQuickDecrypt() {
    onDecryptMessage();
}

void MainWindow::onQuickSign() {
    onSignMessage();
}

void MainWindow::onQuickVerify() {
    onVerifyMessage();
}

void MainWindow::onCopyPublicKey() {
    QString keyId = m_profileCombo->currentData().toString();
    if (keyId.isEmpty()) {
        showError("Please select a profile");
        return;
    }
    
    QByteArray keyData = m_pgpManager->getPublicKeyArmored(keyId);
    if (!keyData.isEmpty()) {
        QApplication::clipboard()->setText(QString::fromUtf8(keyData));
        showSuccess("Public key copied to clipboard");
    } else {
        showError("Failed to get public key");
    }
}

// Notepad signal handlers - IMPLEMENTED (were missing)
void MainWindow::onTextChanged() {
    // Handle text changed signal from notepad
    m_modified = true;
    updateWindowTitle();
}

void MainWindow::onEncryptionRequested() {
    // Handle encryption requested signal from notepad
    m_statusLabel->setText("Encryption completed");
}

void MainWindow::onDecryptionRequested() {
    // Handle decryption requested signal from notepad
    m_statusLabel->setText("Decryption completed");
}

void MainWindow::onSigningRequested() {
    // Handle signing requested signal from notepad
    m_statusLabel->setText("Signing completed");
}

void MainWindow::onVerificationRequested() {
    // Handle verification requested signal from notepad
    m_statusLabel->setText("Verification completed");
}

void MainWindow::showError(const QString& message) {
    QMessageBox::critical(this, "Error", message);
    m_statusLabel->setText("Error: " + message);
}

void MainWindow::showSuccess(const QString& message) {
    m_statusLabel->setText(message);
    m_statusLabel->setStyleSheet("color: #4CAF50;");
    QTimer::singleShot(3000, [this]() {
        m_statusLabel->setStyleSheet("");
    });
}

} // namespace PCPGP
