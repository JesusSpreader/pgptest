#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMenu>
#include <memory>

QT_BEGIN_NAMESPACE
class QTextEdit;
class QComboBox;
class QPushButton;
class QLabel;
class QTabWidget;
class QListWidget;
class QSplitter;
QT_END_NAMESPACE

namespace PCPGP {

class PGPManager;
class NotepadWidget;
class KeyManagerDialog;
class ProfileManager;
class SetupWizard;
class EncryptDialog;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    
    bool initialize();

protected:
    void closeEvent(QCloseEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;

private slots:
    // Menu actions
    void onNewKey();
    void onImportKey();
    void onExportKey();
    void onManageKeys();
    void onManageProfiles();
    void onPreferences();
    void onExit();
    
    // Encryption actions
    void onEncryptMessage();
    void onDecryptMessage();
    void onSignMessage();
    void onVerifyMessage();
    void onEncryptFile();
    void onDecryptFile();
    void onSignFile();
    void onVerifyFile();
    
    // Profile actions
    void onNewProfile();
    void onImportProfile();
    void onSelectProfile();
    void onMassImport();
    
    // Security actions
    void onLockApplication();
    void onChangePassword();
    void onPostQuantumSettings();
    void onDecryptStorage();
    
    // Help actions
    void onAbout();
    void onDocumentation();
    
    // Tray icon
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onShowWindow();
    void onHideWindow();
    
    // Notepad signals
    void onTextChanged();
    void onEncryptionRequested();
    void onDecryptionRequested();
    void onSigningRequested();
    void onVerificationRequested();
    
    // Quick actions
    void onQuickEncrypt();
    void onQuickDecrypt();
    void onQuickSign();
    void onQuickVerify();
    void onCopyPublicKey();

private:
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupTrayIcon();
    void setupStyle();
    void setupConnections();
    
    void checkFirstRun();
    void loadProfiles();
    void updateProfileCombo();
    void updateStatusBar();
    void updateWindowTitle();
    
    bool ensureUnlocked();
    bool confirmOperation(const QString& title, const QString& message);
    void showError(const QString& message);
    void showSuccess(const QString& message);
    
    void handleDroppedFile(const QString& filePath);
    void processCommandLine();
    
    // Core components
    std::unique_ptr<PGPManager> m_pgpManager;
    std::unique_ptr<ProfileManager> m_profileManager;
    
    // UI Components
    QTabWidget* m_tabWidget;
    NotepadWidget* m_notepad;
    QComboBox* m_profileCombo;
    QLabel* m_statusLabel;
    QLabel* m_securityLabel;
    QSystemTrayIcon* m_trayIcon;
    QMenu* m_trayMenu;
    
    // Dialogs
    KeyManagerDialog* m_keyManagerDialog = nullptr;
    SetupWizard* m_setupWizard = nullptr;
    EncryptDialog* m_encryptDialog = nullptr;
    
    // State
    bool m_locked = false;
    bool m_modified = false;
    QString m_currentFile;
};

} // namespace PCPGP

#endif // MAINWINDOW_H
