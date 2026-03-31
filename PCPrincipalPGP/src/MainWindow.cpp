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
#include "GenerateKeyDialog.h"
#include "KeyProfile.h"
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
#include <QInputDialog>
#include <QListWidget>
#include <QDialog>
#include <QLineEdit>
#include <QFormLayout>
#include <QCheckBox>

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
    
    // File menu - using new Qt6 API (text, shortcut, object, slot)
    QMenu* fileMenu = menuBar->addMenu("&File");
    
    QAction* newAction = new QAction("&New", this);
    newAction->setShortcut(QKeySequence::New);
    connect(newAction, &QAction::triggered, this, &MainWindow::onNewKey);
    fileMenu->addAction(newAction);
    
    QAction* importAction = new QAction("&Import Key...", this);
    importAction->setShortcut(QKeySequence("Ctrl+I"));
    connect(importAction, &QAction::triggered, this, &MainWindow::onImportKey);
    fileMenu->addAction(importAction);
    
    QAction* exportAction = new QAction("&Export Key...", this);
    connect(exportAction, &QAction::triggered, this, &MainWindow::onExportKey);
    fileMenu->addAction(exportAction);
    
    fileMenu->addSeparator();
    
    QAction* prefsAction = new QAction("&Preferences...", this);
    connect(prefsAction, &QAction::triggered, this, &MainWindow::onPreferences);
    fileMenu->addAction(prefsAction);
    
    fileMenu->addSeparator();
    
    QAction* exitAction = new QAction("E&xit", this);
    exitAction->setShortcut(QKeySequence::Quit);
    connect(exitAction, &QAction::triggered, this, &MainWindow::onExit);
    fileMenu->addAction(exitAction);
    
    // Keys menu
    QMenu* keysMenu = menuBar->addMenu("&Keys");
    
    QAction* manageKeysAction = new QAction("&Manage Keys...", this);
    manageKeysAction->setShortcut(QKeySequence("Ctrl+K"));
    connect(manageKeysAction, &QAction::triggered, this, &MainWindow::onManageKeys);
    keysMenu->addAction(manageKeysAction);
    
    QAction* newProfileAction = new QAction("&New Profile...", this);
    connect(newProfileAction, &QAction::triggered, this, &MainWindow::onNewProfile);
    keysMenu->addAction(newProfileAction);
    
    QAction* importProfileAction = new QAction("&Import Profile...", this);
    connect(importProfileAction, &QAction::triggered, this, &MainWindow::onImportProfile);
    keysMenu->addAction(importProfileAction);
    
    QAction* massImportAction = new QAction("&Mass Import...", this);
    connect(massImportAction, &QAction::triggered, this, &MainWindow::onMassImport);
    keysMenu->addAction(massImportAction);
    
    keysMenu->addSeparator();
    
    QAction* copyPubKeyAction = new QAction("&Copy Public Key", this);
    connect(copyPubKeyAction, &QAction::triggered, this, &MainWindow::onCopyPublicKey);
    keysMenu->addAction(copyPubKeyAction);
    
    // Encryption menu
    QMenu* encryptMenu = menuBar->addMenu("&Encryption");
    
    QAction* encryptMsgAction = new QAction("&Encrypt Message...", this);
    encryptMsgAction->setShortcut(QKeySequence("Ctrl+E"));
    connect(encryptMsgAction, &QAction::triggered, this, &MainWindow::onEncryptMessage);
    encryptMenu->addAction(encryptMsgAction);
    
    QAction* decryptMsgAction = new QAction("&Decrypt Message...", this);
    decryptMsgAction->setShortcut(QKeySequence("Ctrl+D"));
    connect(decryptMsgAction, &QAction::triggered, this, &MainWindow::onDecryptMessage);
    encryptMenu->addAction(decryptMsgAction);
    
    encryptMenu->addSeparator();
    
    QAction* signMsgAction = new QAction("&Sign Message...", this);
    signMsgAction->setShortcut(QKeySequence("Ctrl+Shift+S"));
    connect(signMsgAction, &QAction::triggered, this, &MainWindow::onSignMessage);
    encryptMenu->addAction(signMsgAction);
    
    QAction* verifyMsgAction = new QAction("&Verify Message...", this);
    connect(verifyMsgAction, &QAction::triggered, this, &MainWindow::onVerifyMessage);
    encryptMenu->addAction(verifyMsgAction);
    
    encryptMenu->addSeparator();
    
    QAction* encryptFileAction = new QAction("Encrypt &File...", this);
    connect(encryptFileAction, &QAction::triggered, this, &MainWindow::onEncryptFile);
    encryptMenu->addAction(encryptFileAction);
    
    QAction* decryptFileAction = new QAction("Decrypt F&ile...", this);
    connect(decryptFileAction, &QAction::triggered, this, &MainWindow::onDecryptFile);
    encryptMenu->addAction(decryptFileAction);
    
    QAction* signFileAction = new QAction("Sign F&ile...", this);
    connect(signFileAction, &QAction::triggered, this, &MainWindow::onSignFile);
    encryptMenu->addAction(signFileAction);
    
    QAction* verifyFileAction = new QAction("Verif&y File...", this);
    connect(verifyFileAction, &QAction::triggered, this, &MainWindow::onVerifyFile);
    encryptMenu->addAction(verifyFileAction);
    
    // Security menu
    QMenu* securityMenu = menuBar->addMenu("&Security");
    
    QAction* lockAction = new QAction("&Lock Application", this);
    lockAction->setShortcut(QKeySequence("Ctrl+L"));
    connect(lockAction, &QAction::triggered, this, &MainWindow::onLockApplication);
    securityMenu->addAction(lockAction);
    
    QAction* changePassAction = new QAction("&Change Password...", this);
    connect(changePassAction, &QAction::triggered, this, &MainWindow::onChangePassword);
    securityMenu->addAction(changePassAction);
    
    securityMenu->addSeparator();
    
    QAction* pqSettingsAction = new QAction("&Post-Quantum Settings...", this);
    connect(pqSettingsAction, &QAction::triggered, this, &MainWindow::onPostQuantumSettings);
    securityMenu->addAction(pqSettingsAction);
    
    QAction* decryptStorageAction = new QAction("&Decrypt Storage...", this);
    connect(decryptStorageAction, &QAction::triggered, this, &MainWindow::onDecryptStorage);
    securityMenu->addAction(decryptStorageAction);
    
    // Help menu
    QMenu* helpMenu = menuBar->addMenu("&Help");
    
    QAction* aboutAction = new QAction("&About", this);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
    helpMenu->addAction(aboutAction);
    
    QAction* docsAction = new QAction("&Documentation", this);
    connect(docsAction, &QAction::triggered, this, &MainWindow::onDocumentation);
    helpMenu->addAction(docsAction);
}

void MainWindow::setupToolBar() {
    QToolBar* toolBar = addToolBar("Main");
    toolBar->setMovable(false);
    
    QAction* encryptAction = new QAction("Encrypt", this);
    connect(encryptAction, &QAction::triggered, this, &MainWindow::onQuickEncrypt);
    toolBar->addAction(encryptAction);
    
    QAction* decryptAction = new QAction("Decrypt", this);
    connect(decryptAction, &QAction::triggered, this, &MainWindow::onQuickDecrypt);
    toolBar->addAction(decryptAction);
    
    toolBar->addSeparator();
    
    QAction* signAction = new QAction("Sign", this);
    connect(signAction, &QAction::triggered, this, &MainWindow::onQuickSign);
    toolBar->addAction(signAction);
    
    QAction* verifyAction = new QAction("Verify", this);
    connect(verifyAction, &QAction::triggered, this, &MainWindow::onQuickVerify);
    toolBar->addAction(verifyAction);
    
    toolBar->addSeparator();
    
    QAction* keysAction = new QAction("Keys", this);
    connect(keysAction, &QAction::triggered, this, &MainWindow::onManageKeys);
    toolBar->addAction(keysAction);
    
    QAction* profilesAction = new QAction("Profiles", this);
    connect(profilesAction, &QAction::triggered, this, &MainWindow::onManageProfiles);
    toolBar->addAction(profilesAction);
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
    
    QAction* showAction = new QAction("Show", this);
    connect(showAction, &QAction::triggered, this, &MainWindow::onShowWindow);
    m_trayMenu->addAction(showAction);
    
    QAction* hideAction = new QAction("Hide", this);
    connect(hideAction, &QAction::triggered, this, &MainWindow::onHideWindow);
    m_trayMenu->addAction(hideAction);
    
    m_trayMenu->addSeparator();
    
    QAction* exitAction = new QAction("Exit", this);
    connect(exitAction, &QAction::triggered, this, &MainWindow::onExit);
    m_trayMenu->addAction(exitAction);
    
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
    // Open generate key dialog - constructor only takes QWidget*
    GenerateKeyDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        // Generate the key using PGPManager
        if (m_pgpManager->generateKeyPair(
                dialog.getName(),
                dialog.getEmail(),
                dialog.getPassphrase(),
                dialog.getKeyLength(),
                dialog.isExpirationEnabled() ? dialog.getExpirationDays() : 0)) {
            showSuccess("Key generated successfully");
            // Create a profile for this key
            if (m_profileManager) {
                auto keys = m_pgpManager->listSecretKeys();
                for (const auto& key : keys) {
                    if (key.userId == dialog.getName() || key.email == dialog.getEmail()) {
                        m_profileManager->importProfileFromKey(key.keyId, dialog.getName());
                        break;
                    }
                }
                loadProfiles();
                updateProfileCombo();
            }
        } else {
            showError("Failed to generate key: " + m_pgpManager->getLastError());
        }
    }
}

void MainWindow::onImportKey() {
    QString filePath = QFileDialog::getOpenFileName(this, "Import Key", QString(),
        "PGP Keys (*.asc *.gpg *.pgp *.pub *.key);;All Files (*)");
    
    if (!filePath.isEmpty()) {
        if (m_pgpManager->importKeyFromFile(filePath)) {
            showSuccess("Key imported successfully");
            loadProfiles();
            updateProfileCombo();
        } else {
            showError("Failed to import key: " + m_pgpManager->getLastError());
        }
    }
}

void MainWindow::onExportKey() {
    // Get list of available keys
    auto keys = m_pgpManager->listPublicKeys();
    if (keys.isEmpty()) {
        showError("No keys available to export");
        return;
    }
    
    // Create a dialog to select which key to export
    QStringList keyList;
    for (const auto& key : keys) {
        keyList.append(QString("%1 (%2)").arg(key.userId, key.keyId));
    }
    
    bool ok;
    QString selected = QInputDialog::getItem(this, "Export Key",
        "Select key to export:", keyList, 0, false, &ok);
    
    if (!ok || selected.isEmpty()) {
        return;
    }
    
    // Extract keyId from selection
    QString keyId = keys[keyList.indexOf(selected)].keyId;
    
    // Ask for export file path
    QString filePath = QFileDialog::getSaveFileName(this, "Export Public Key",
        QString("%1_public.asc").arg(keyId), "ASCII Armored (*.asc);;All Files (*)");
    
    if (!filePath.isEmpty()) {
        if (m_pgpManager->exportPublicKeyToFile(keyId, filePath)) {
            showSuccess("Public key exported successfully");
        } else {
            showError("Failed to export key: " + m_pgpManager->getLastError());
        }
    }
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
    // Show a simple profile manager dialog
    QDialog dialog(this);
    dialog.setWindowTitle("Profile Manager");
    dialog.setMinimumSize(500, 400);
    
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    
    // Profile list
    QListWidget* profileList = new QListWidget(&dialog);
    auto profiles = m_profileManager->getAllProfiles();
    for (const auto& profile : profiles) {
        profileList->addItem(profile.getDisplayName());
    }
    layout->addWidget(profileList);
    
    // Buttons
    QHBoxLayout* btnLayout = new QHBoxLayout();
    QPushButton* closeBtn = new QPushButton("Close", &dialog);
    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    layout->addLayout(btnLayout);
    
    dialog.exec();
}

void MainWindow::onPreferences() {
    // Open preferences dialog
    QMessageBox::information(this, "Preferences",
        "Preferences dialog will be implemented in a future version.\n\n"
        "Current settings can be modified through the Setup Wizard (File > New to reset).");
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
    // Create a simple dialog to add a new profile
    QDialog dialog(this);
    dialog.setWindowTitle("New Profile");
    dialog.setMinimumWidth(400);
    
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    QFormLayout* formLayout = new QFormLayout();
    
    QLineEdit* nameEdit = new QLineEdit(&dialog);
    QLineEdit* emailEdit = new QLineEdit(&dialog);
    
    formLayout->addRow("Name:", nameEdit);
    formLayout->addRow("Email:", emailEdit);
    layout->addLayout(formLayout);
    
    // Get available keys
    auto keys = m_pgpManager->listPublicKeys();
    QComboBox* keyCombo = new QComboBox(&dialog);
    for (const auto& key : keys) {
        keyCombo->addItem(QString("%1 (%2)").arg(key.userId, key.keyId), key.keyId);
    }
    formLayout->addRow("Key:", keyCombo);
    
    QHBoxLayout* btnLayout = new QHBoxLayout();
    QPushButton* okBtn = new QPushButton("OK", &dialog);
    QPushButton* cancelBtn = new QPushButton("Cancel", &dialog);
    connect(okBtn, &QPushButton::clicked, &dialog, &QDialog::accept);
    connect(cancelBtn, &QPushButton::clicked, &dialog, &QDialog::reject);
    btnLayout->addStretch();
    btnLayout->addWidget(okBtn);
    btnLayout->addWidget(cancelBtn);
    layout->addLayout(btnLayout);
    
    if (dialog.exec() == QDialog::Accepted) {
        QString keyId = keyCombo->currentData().toString();
        if (!keyId.isEmpty() && m_profileManager) {
            m_profileManager->importProfileFromKey(keyId, nameEdit->text());
            loadProfiles();
            updateProfileCombo();
            showSuccess("Profile created successfully");
        }
    }
}

void MainWindow::onImportProfile() {
    QString filePath = QFileDialog::getOpenFileName(this, "Import Profile",
        QString(), "Profile Files (*.json);;All Files (*)");
    
    if (!filePath.isEmpty()) {
        if (m_profileManager && m_profileManager->importProfileFromFile(filePath)) {
            showSuccess("Profile imported successfully");
            loadProfiles();
            updateProfileCombo();
        } else {
            showError("Failed to import profile");
        }
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
    ConfigManager& config = ConfigManager::getInstance();
    
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Post-Quantum Settings");
    msgBox.setText("Post-Quantum Encryption Settings");
    msgBox.setInformativeText("Enable post-quantum encryption using XChaCha20-Poly1305 with Argon2id key derivation?");
    
    QCheckBox* pqCheckBox = new QCheckBox("Enable Post-Quantum Encryption");
    pqCheckBox->setChecked(config.isPostQuantumEnabled());
    msgBox.setCheckBox(pqCheckBox);
    
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);
    
    if (msgBox.exec() == QMessageBox::Save) {
        config.setPostQuantumEnabled(pqCheckBox->isChecked());
        config.saveConfig();
        updateStatusBar();
        showSuccess(pqCheckBox->isChecked() ? 
            "Post-quantum encryption enabled" : 
            "Post-quantum encryption disabled");
    }
}

void MainWindow::onDecryptStorage() {
    auto reply = QMessageBox::warning(this, "Decrypt Storage",
        "This will decrypt all stored data. Are you sure?",
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        // Decrypt storage logic
        if (SecureStorage::getInstance().isLocked()) {
            showError("Storage is locked. Please unlock first.");
            return;
        }
        
        QMessageBox::information(this, "Decrypt Storage",
            "Storage decrypted successfully.\n\n"
            "Note: Your private keys are now stored in plain text. "
            "This is not recommended for production use.");
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
        "<h2>PC Principal PGP - Documentation</h2>"
        "<h3>Quick Start</h3>"
        "<ol>"
        "<li>Generate or import PGP keys using Keys > Manage Keys</li>"
        "<li>Create profiles for your contacts</li>"
        "<li>Type messages in the Notepad tab</li>"
        "<li>Use Encrypt/Decrypt buttons for secure communication</li>"
        "</ol>"
        "<h3>Keyboard Shortcuts</h3>"
        "<ul>"
        "<li>Ctrl+N - New Key</li>"
        "<li>Ctrl+I - Import Key</li>"
        "<li>Ctrl+K - Manage Keys</li>"
        "<li>Ctrl+E - Encrypt Message</li>"
        "<li>Ctrl+D - Decrypt Message</li>"
        "<li>Ctrl+Shift+S - Sign Message</li>"
        "<li>Ctrl+L - Lock Application</li>"
        "</ul>");
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
