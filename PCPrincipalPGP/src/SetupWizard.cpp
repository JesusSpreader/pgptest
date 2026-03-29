#include "SetupWizard.h"
#include "ConfigManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QRadioButton>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QFileDialog>
#include <QProgressBar>
#include <QMessageBox>
#include <QRegularExpression>

namespace PCPGP {

SetupWizard::SetupWizard(QWidget* parent) : QWizard(parent) {
    setWindowTitle("PC Principal PGP - Setup");
    setMinimumSize(600, 500);
    
    setupPages();
}

SetupWizard::~SetupWizard() {
}

void SetupWizard::setupPages() {
    setPage(0, new WelcomePage(this));
    setPage(1, new ModePage(this));
    setPage(2, new PathsPage(this));
    setPage(3, new SecurityPage(this));
    setPage(4, new SummaryPage(this));
    
    setStartId(0);
}

bool SetupWizard::runSetup() {
    return exec() == QDialog::Accepted;
}

PortableMode SetupWizard::getPortableMode() const {
    return m_mode;
}

QString SetupWizard::getPublicKeysPath() const {
    return m_publicPath;
}

QString SetupWizard::getPrivateKeysPath() const {
    return m_privatePath;
}

bool SetupWizard::isPostQuantumEnabled() const {
    return m_postQuantum;
}

bool SetupWizard::isEncryptPrivateOnly() const {
    return m_encryptPrivate;
}

QString SetupWizard::getPassword() const {
    return m_password;
}

// IMPLEMENTED: Missing validateCurrentPage() override
bool SetupWizard::validateCurrentPage() {
    // Basic validation - can be extended as needed
    return QWizard::validateCurrentPage();
}

// IMPLEMENTED: Missing slot functions
void SetupWizard::onModeChanged() {
    // Handle mode change - update UI accordingly
    ModePage* modePage = qobject_cast<ModePage*>(page(1));
    if (modePage) {
        m_mode = modePage->getSelectedMode();
    }
}

void SetupWizard::onBrowsePublic() {
    QString path = QFileDialog::getExistingDirectory(this, "Select Public Keys Directory");
    if (!path.isEmpty()) {
        m_publicPath = path;
        PathsPage* pathsPage = qobject_cast<PathsPage*>(page(2));
        if (pathsPage) {
            // Update the path edit if accessible
        }
    }
}

void SetupWizard::onBrowsePrivate() {
    QString path = QFileDialog::getExistingDirectory(this, "Select Private Keys Directory");
    if (!path.isEmpty()) {
        m_privatePath = path;
    }
}

void SetupWizard::onPQToggled(bool checked) {
    m_postQuantum = checked;
}

void SetupWizard::onPasswordChanged() {
    SecurityPage* secPage = qobject_cast<SecurityPage*>(page(3));
    if (secPage) {
        m_password = secPage->getPassword();
    }
}

void SetupWizard::accept() {
    // Store settings before accepting
    SecurityPage* secPage = qobject_cast<SecurityPage*>(page(3));
    if (secPage) {
        m_postQuantum = secPage->isPostQuantumEnabled();
        m_password = secPage->getPassword();
    }
    
    QWizard::accept();
}

// Welcome Page
WelcomePage::WelcomePage(QWidget* parent) : QWizardPage(parent) {
    setTitle("Welcome to PC Principal PGP");
    setSubTitle("Let's set up your portable PGP environment.");
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    QLabel* iconLabel = new QLabel(this);
    QPixmap icon(":/icon.png");
    if (!icon.isNull()) {
        iconLabel->setPixmap(icon.scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    iconLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(iconLabel);
    
    QLabel* welcomeText = new QLabel(
        "<h2>PC Principal PGP</h2>"
        "<p>A secure, portable PGP encryption application.</p>"
        "<p>This wizard will help you set up:</p>"
        "<ul>"
        "<li>Portable mode (USB or custom directories)</li>"
        "<li>Post-quantum encryption options</li>"
        "<li>Password protection</li>"
        "</ul>"
        "<p>Click <b>Next</b> to continue.</p>", this);
    welcomeText->setWordWrap(true);
    welcomeText->setAlignment(Qt::AlignCenter);
    layout->addWidget(welcomeText);
    
    layout->addStretch();
}

// Mode Page
ModePage::ModePage(QWidget* parent) : QWizardPage(parent) {
    setTitle("Portable Mode");
    setSubTitle("Choose how you want to store your keys.");
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    QGroupBox* group = new QGroupBox("Storage Mode", this);
    QVBoxLayout* groupLayout = new QVBoxLayout(group);
    
    QRadioButton* dirModeRadio = new QRadioButton(
        "&Directory Mode (Fully Portable)\n"
        "Keys are stored in folders next to the application. "
        "Perfect for USB drives.", this);
    dirModeRadio->setChecked(true);
    dirModeRadio->setObjectName("dirModeRadio");
    groupLayout->addWidget(dirModeRadio);
    
    QRadioButton* partialModeRadio = new QRadioButton(
        "&Partial Portable\n"
        "You choose where to store public and private keys separately. "
        "Useful for keeping private keys on encrypted storage.", this);
    partialModeRadio->setObjectName("partialModeRadio");
    groupLayout->addWidget(partialModeRadio);
    
    layout->addWidget(group);
    layout->addStretch();
}

bool ModePage::validatePage() {
    return true;
}

PortableMode ModePage::getSelectedMode() const {
    QRadioButton* dirMode = findChild<QRadioButton*>("dirModeRadio");
    if (dirMode && dirMode->isChecked()) {
        return PortableMode::DIRECTORY_MODE;
    }
    return PortableMode::PARTIAL_PORTABLE;
}

// Paths Page
PathsPage::PathsPage(QWidget* parent) : QWizardPage(parent) {
    setTitle("Key Storage Paths");
    setSubTitle("Choose where to store your keys (only for Partial Portable mode).");
    
    QGridLayout* layout = new QGridLayout(this);
    
    layout->addWidget(new QLabel("Public Keys Directory:", this), 0, 0);
    m_publicPathEdit = new QLineEdit(this);
    layout->addWidget(m_publicPathEdit, 0, 1);
    QPushButton* browsePublicBtn = new QPushButton("Browse...", this);
    layout->addWidget(browsePublicBtn, 0, 2);
    
    layout->addWidget(new QLabel("Private Keys Directory:", this), 1, 0);
    m_privatePathEdit = new QLineEdit(this);
    layout->addWidget(m_privatePathEdit, 1, 1);
    QPushButton* browsePrivateBtn = new QPushButton("Browse...", this);
    layout->addWidget(browsePrivateBtn, 1, 2);
    
    connect(browsePublicBtn, &QPushButton::clicked, this, [this]() {
        QString path = QFileDialog::getExistingDirectory(this, "Select Public Keys Directory");
        if (!path.isEmpty()) {
            m_publicPathEdit->setText(path);
        }
    });
    
    connect(browsePrivateBtn, &QPushButton::clicked, this, [this]() {
        QString path = QFileDialog::getExistingDirectory(this, "Select Private Keys Directory");
        if (!path.isEmpty()) {
            m_privatePathEdit->setText(path);
        }
    });
}

void PathsPage::initializePage() {
    ModePage* modePage = qobject_cast<ModePage*>(wizard()->page(1));
    if (modePage) {
        bool isPartial = (modePage->getSelectedMode() == PortableMode::PARTIAL_PORTABLE);
        setEnabled(isPartial);
        
        if (!isPartial) {
            wizard()->next();
        }
    }
}

bool PathsPage::validatePage() {
    ModePage* modePage = qobject_cast<ModePage*>(wizard()->page(1));
    if (modePage && modePage->getSelectedMode() == PortableMode::PARTIAL_PORTABLE) {
        if (m_publicPathEdit->text().isEmpty() || m_privatePathEdit->text().isEmpty()) {
            QMessageBox::warning(this, "Validation Error", 
                "Please select both public and private key directories.");
            return false;
        }
    }
    return true;
}

QString PathsPage::getPublicPath() const {
    return m_publicPathEdit->text();
}

QString PathsPage::getPrivatePath() const {
    return m_privatePathEdit->text();
}

// Security Page
SecurityPage::SecurityPage(QWidget* parent) : QWizardPage(parent) {
    setTitle("Security Settings");
    setSubTitle("Configure post-quantum encryption and password protection.");
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    // Post-quantum group
    QGroupBox* pqGroup = new QGroupBox("Post-Quantum Encryption", this);
    QVBoxLayout* pqLayout = new QVBoxLayout(pqGroup);
    
    m_pqCheck = new QCheckBox("&Enable post-quantum encryption (XChaCha20-Poly1305)", this);
    m_pqCheck->setChecked(true);
    pqLayout->addWidget(m_pqCheck);
    
    m_encryptPrivateCheck = new QCheckBox("&Encrypt only private keys (recommended)", this);
    m_encryptPrivateCheck->setChecked(true);
    pqLayout->addWidget(m_encryptPrivateCheck);
    
    m_encryptAllCheck = new QCheckBox("&Encrypt all data (higher security, slower)", this);
    pqLayout->addWidget(m_encryptAllCheck);
    
    layout->addWidget(pqGroup);
    
    // Password group
    QGroupBox* passGroup = new QGroupBox("Password Protection", this);
    QGridLayout* passLayout = new QGridLayout(passGroup);
    
    passLayout->addWidget(new QLabel("Password:", this), 0, 0);
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("24+ characters recommended");
    passLayout->addWidget(m_passwordEdit, 0, 1);
    
    passLayout->addWidget(new QLabel("Confirm:", this), 1, 0);
    m_confirmEdit = new QLineEdit(this);
    m_confirmEdit->setEchoMode(QLineEdit::Password);
    passLayout->addWidget(m_confirmEdit, 1, 1);
    
    QHBoxLayout* strengthLayout = new QHBoxLayout();
    m_strengthLabel = new QLabel("Strength:", this);
    strengthLayout->addWidget(m_strengthLabel);
    m_strengthBar = new QProgressBar(this);
    m_strengthBar->setRange(0, 100);
    m_strengthBar->setValue(0);
    m_strengthBar->setTextVisible(false);
    m_strengthBar->setFixedHeight(8);
    strengthLayout->addWidget(m_strengthBar, 1);
    passLayout->addLayout(strengthLayout, 2, 1);
    
    layout->addWidget(passGroup);
    layout->addStretch();
    
    connect(m_passwordEdit, &QLineEdit::textChanged, this, [this]() {
        QString pwd = m_passwordEdit->text();
        int strength = 0;
        
        // Simple strength calculation
        strength += qMin(pwd.length() * 3, 40);
        if (pwd.contains(QRegularExpression("[a-z]"))) strength += 10;
        if (pwd.contains(QRegularExpression("[A-Z]"))) strength += 10;
        if (pwd.contains(QRegularExpression("[0-9]"))) strength += 10;
        if (pwd.contains(QRegularExpression("[^a-zA-Z0-9]"))) strength += 15;
        if (pwd.length() >= 24) strength += 15;
        
        strength = qMin(strength, 100);
        m_strengthBar->setValue(strength);
        
        if (strength < 30) {
            m_strengthBar->setStyleSheet("QProgressBar::chunk { background-color: #f44336; }");
        } else if (strength < 60) {
            m_strengthBar->setStyleSheet("QProgressBar::chunk { background-color: #ff9800; }");
        } else if (strength < 80) {
            m_strengthBar->setStyleSheet("QProgressBar::chunk { background-color: #ffc107; }");
        } else {
            m_strengthBar->setStyleSheet("QProgressBar::chunk { background-color: #4caf50; }");
        }
    });
}

bool SecurityPage::validatePage() {
    if (m_passwordEdit->text() != m_confirmEdit->text()) {
        QMessageBox::warning(this, "Validation Error", "Passwords do not match.");
        return false;
    }
    
    if (m_passwordEdit->text().length() > 0 && m_passwordEdit->text().length() < 8) {
        auto reply = QMessageBox::warning(this, "Weak Password",
            "Your password is less than 8 characters. This is not secure.\n\n"
            "Do you want to continue anyway?",
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No) {
            return false;
        }
    }
    
    return true;
}

bool SecurityPage::isPostQuantumEnabled() const {
    return m_pqCheck->isChecked();
}

bool SecurityPage::isEncryptPrivateOnly() const {
    return m_encryptPrivateCheck->isChecked();
}

QString SecurityPage::getPassword() const {
    return m_passwordEdit->text();
}

// Summary Page
SummaryPage::SummaryPage(QWidget* parent) : QWizardPage(parent) {
    setTitle("Summary");
    setSubTitle("Review your settings before finishing.");
    
    QVBoxLayout* layout = new QVBoxLayout(this);
    
    m_summaryLabel = new QLabel(this);
    m_summaryLabel->setWordWrap(true);
    layout->addWidget(m_summaryLabel);
    
    layout->addStretch();
}

void SummaryPage::initializePage() {
    ModePage* modePage = qobject_cast<ModePage*>(wizard()->page(1));
    PathsPage* pathsPage = qobject_cast<PathsPage*>(wizard()->page(2));
    SecurityPage* secPage = qobject_cast<SecurityPage*>(wizard()->page(3));
    
    QString summary = "<h3>Configuration Summary</h3><ul>";
    
    if (modePage) {
        PortableMode mode = modePage->getSelectedMode();
        summary += QString("<li><b>Mode:</b> %1</li>")
            .arg(mode == PortableMode::DIRECTORY_MODE ? "Fully Portable" : "Partial Portable");
    }
    
    if (pathsPage && modePage && modePage->getSelectedMode() == PortableMode::PARTIAL_PORTABLE) {
        summary += QString("<li><b>Public Keys:</b> %1</li>"
                          "<li><b>Private Keys:</b> %2</li>")
            .arg(pathsPage->getPublicPath())
            .arg(pathsPage->getPrivatePath());
    }
    
    if (secPage) {
        summary += QString("<li><b>Post-Quantum:</b> %1</li>"
                          "<li><b>Encrypt Private Keys Only:</b> %2</li>")
            .arg(secPage->isPostQuantumEnabled() ? "Enabled" : "Disabled")
            .arg(secPage->isEncryptPrivateOnly() ? "Yes" : "No");
        
        QString pwd = secPage->getPassword();
        summary += QString("<li><b>Password Protection:</b> %1</li>")
            .arg(pwd.isEmpty() ? "Disabled" : "Enabled");
    }
    
    summary += "</ul><p>Click <b>Finish</b> to apply these settings.</p>";
    
    m_summaryLabel->setText(summary);
}

} // namespace PCPGP
