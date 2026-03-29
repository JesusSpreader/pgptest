#ifndef SETUPWIZARD_H
#define SETUPWIZARD_H

#include <QWizard>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QProgressBar>
#include "ConfigManager.h"

QT_BEGIN_NAMESPACE
class QRadioButton;
QT_END_NAMESPACE

namespace PCPGP {

class SetupWizard : public QWizard {
    Q_OBJECT

public:
    explicit SetupWizard(QWidget* parent = nullptr);
    ~SetupWizard();
    
    bool runSetup();
    
    // Get settings
    PortableMode getPortableMode() const;
    QString getPublicKeysPath() const;
    QString getPrivateKeysPath() const;
    bool isPostQuantumEnabled() const;
    bool isEncryptPrivateOnly() const;
    QString getPassword() const;

protected:
    bool validateCurrentPage() override;
    void accept() override;

private slots:
    void onModeChanged();
    void onBrowsePublic();
    void onBrowsePrivate();
    void onPQToggled(bool checked);
    void onPasswordChanged();

private:
    void setupPages();
    void createWelcomePage();
    void createModePage();
    void createPathsPage();
    void createSecurityPage();
    void createSummaryPage();
    
    // Page widgets
    QRadioButton* m_dirModeRadio;
    QRadioButton* m_partialModeRadio;
    QLineEdit* m_publicPathEdit;
    QLineEdit* m_privatePathEdit;
    QPushButton* m_browsePublicBtn;
    QPushButton* m_browsePrivateBtn;
    QCheckBox* m_pqCheck;
    QCheckBox* m_encryptPrivateCheck;
    QCheckBox* m_encryptAllCheck;
    QLineEdit* m_passwordEdit;
    QLineEdit* m_confirmPasswordEdit;
    QProgressBar* m_strengthBar;
    QLabel* m_strengthLabel;
    QLabel* m_summaryLabel;
    
    // Settings
    PortableMode m_mode = PortableMode::DIRECTORY_MODE;
    QString m_publicPath;
    QString m_privatePath;
    bool m_postQuantum = true;
    bool m_encryptPrivate = true;
    bool m_encryptAll = false;
    QString m_password;
};

// Individual pages
class WelcomePage : public QWizardPage {
    Q_OBJECT
public:
    WelcomePage(QWidget* parent = nullptr);
};

class ModePage : public QWizardPage {
    Q_OBJECT
public:
    ModePage(QWidget* parent = nullptr);
    bool validatePage() override;
    PortableMode getSelectedMode() const;
};

class PathsPage : public QWizardPage {
    Q_OBJECT
public:
    PathsPage(QWidget* parent = nullptr);
    void initializePage() override;
    bool validatePage() override;
    QString getPublicPath() const;
    QString getPrivatePath() const;
    
private:
    QLineEdit* m_publicPathEdit;
    QLineEdit* m_privatePathEdit;
};

class SecurityPage : public QWizardPage {
    Q_OBJECT
public:
    SecurityPage(QWidget* parent = nullptr);
    bool validatePage() override;
    bool isPostQuantumEnabled() const;
    bool isEncryptPrivateOnly() const;
    QString getPassword() const;
    
private:
    QCheckBox* m_pqCheck;
    QCheckBox* m_encryptPrivateCheck;
    QCheckBox* m_encryptAllCheck;
    QLineEdit* m_passwordEdit;
    QLineEdit* m_confirmEdit;
    QProgressBar* m_strengthBar;
    QLabel* m_strengthLabel;
};

class SummaryPage : public QWizardPage {
    Q_OBJECT
public:
    SummaryPage(QWidget* parent = nullptr);
    void initializePage() override;
    
private:
    QLabel* m_summaryLabel;
};

} // namespace PCPGP

#endif // SETUPWIZARD_H
