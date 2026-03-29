#include "PasswordDialog.h"
#include "SecureStorage.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QProgressBar>
#include <QCheckBox>
#include <QMessageBox>

namespace PCPGP {

PasswordDialog::PasswordDialog(PasswordMode mode, QWidget* parent) 
    : QDialog(parent), m_mode(mode) {
    
    setupUI();
    setupConnections();
    
    switch (mode) {
        case PasswordMode::VERIFY:
            setWindowTitle("Verify Password");
            m_titleLabel->setText("Enter your password to continue");
            m_newPasswordEdit->hide();
            m_confirmEdit->hide();
            break;
        case PasswordMode::SET_NEW:
            setWindowTitle("Set Password");
            m_titleLabel->setText("Set a password for your secure storage");
            m_passwordEdit->hide();
            break;
        case PasswordMode::CHANGE:
            setWindowTitle("Change Password");
            m_titleLabel->setText("Enter your current and new password");
            break;
        case PasswordMode::UNLOCK:
            setWindowTitle("Unlock Application");
            m_titleLabel->setText("Enter your password to unlock");
            m_newPasswordEdit->hide();
            m_confirmEdit->hide();
            break;
    }
}

PasswordDialog::~PasswordDialog() {
}

void PasswordDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // Title
    m_titleLabel = new QLabel(this);
    m_titleLabel->setStyleSheet("font-size: 14px; font-weight: bold;");
    mainLayout->addWidget(m_titleLabel);
    
    // Info
    m_infoLabel = new QLabel(this);
    m_infoLabel->setWordWrap(true);
    m_infoLabel->setStyleSheet("color: #888;");
    mainLayout->addWidget(m_infoLabel);
    
    // Current password
    mainLayout->addWidget(new QLabel("Current Password:", this));
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_passwordEdit->setPlaceholderText("Enter current password");
    mainLayout->addWidget(m_passwordEdit);
    
    // New password
    mainLayout->addWidget(new QLabel("New Password:", this));
    m_newPasswordEdit = new QLineEdit(this);
    m_newPasswordEdit->setEchoMode(QLineEdit::Password);
    m_newPasswordEdit->setPlaceholderText("Enter new password (24+ chars recommended)");
    mainLayout->addWidget(m_newPasswordEdit);
    
    // Confirm password
    mainLayout->addWidget(new QLabel("Confirm Password:", this));
    m_confirmEdit = new QLineEdit(this);
    m_confirmEdit->setEchoMode(QLineEdit::Password);
    m_confirmEdit->setPlaceholderText("Confirm new password");
    mainLayout->addWidget(m_confirmEdit);
    
    // Show password checkbox
    m_showPasswordCheck = new QCheckBox("Show passwords", this);
    mainLayout->addWidget(m_showPasswordCheck);
    
    // Strength indicator (for new password)
    QHBoxLayout* strengthLayout = new QHBoxLayout();
    m_strengthLabel = new QLabel("Strength:", this);
    strengthLayout->addWidget(m_strengthLabel);
    m_strengthBar = new QProgressBar(this);
    m_strengthBar->setRange(0, 100);
    m_strengthBar->setValue(0);
    m_strengthBar->setTextVisible(false);
    m_strengthBar->setFixedHeight(8);
    strengthLayout->addWidget(m_strengthBar, 1);
    mainLayout->addLayout(strengthLayout);
    
    mainLayout->addStretch();
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_cancelBtn = new QPushButton("Cancel", this);
    buttonLayout->addWidget(m_cancelBtn);
    
    m_okBtn = new QPushButton("OK", this);
    m_okBtn->setDefault(true);
    buttonLayout->addWidget(m_okBtn);
    
    mainLayout->addLayout(buttonLayout);
}

void PasswordDialog::setupConnections() {
    connect(m_okBtn, &QPushButton::clicked, this, &PasswordDialog::onOKClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &PasswordDialog::onCancelClicked);
    connect(m_newPasswordEdit, &QLineEdit::textChanged, this, &PasswordDialog::onPasswordChanged);
    connect(m_showPasswordCheck, &QCheckBox::toggled, this, &PasswordDialog::onShowPasswordToggled);
}

void PasswordDialog::onOKClicked() {
    if (!validateInput()) {
        return;
    }
    accept();
}

void PasswordDialog::onCancelClicked() {
    reject();
}

void PasswordDialog::onPasswordChanged() {
    updateStrengthIndicator();
}

void PasswordDialog::onShowPasswordToggled(bool checked) {
    m_passwordEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
    m_newPasswordEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
    m_confirmEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
}

bool PasswordDialog::validateInput() {
    switch (m_mode) {
        case PasswordMode::VERIFY:
        case PasswordMode::UNLOCK:
            if (m_passwordEdit->text().isEmpty()) {
                QMessageBox::warning(this, "Error", "Please enter your password.");
                return false;
            }
            break;
            
        case PasswordMode::SET_NEW:
            if (m_newPasswordEdit->text().isEmpty()) {
                QMessageBox::warning(this, "Error", "Please enter a password.");
                return false;
            }
            if (m_newPasswordEdit->text() != m_confirmEdit->text()) {
                QMessageBox::warning(this, "Error", "Passwords do not match.");
                return false;
            }
            break;
            
        case PasswordMode::CHANGE:
            if (m_passwordEdit->text().isEmpty()) {
                QMessageBox::warning(this, "Error", "Please enter your current password.");
                return false;
            }
            if (m_newPasswordEdit->text().isEmpty()) {
                QMessageBox::warning(this, "Error", "Please enter a new password.");
                return false;
            }
            if (m_newPasswordEdit->text() != m_confirmEdit->text()) {
                QMessageBox::warning(this, "Error", "New passwords do not match.");
                return false;
            }
            break;
    }
    
    return true;
}

void PasswordDialog::updateStrengthIndicator() {
    QString password = m_newPasswordEdit->text();
    int strength = calculatePasswordStrength(password);
    m_strengthBar->setValue(strength);
    
    if (strength < 30) {
        m_strengthBar->setStyleSheet("QProgressBar::chunk { background-color: #f44336; }");
        m_strengthLabel->setText("Strength: Weak");
    } else if (strength < 60) {
        m_strengthBar->setStyleSheet("QProgressBar::chunk { background-color: #ff9800; }");
        m_strengthLabel->setText("Strength: Fair");
    } else if (strength < 80) {
        m_strengthBar->setStyleSheet("QProgressBar::chunk { background-color: #ffc107; }");
        m_strengthLabel->setText("Strength: Good");
    } else {
        m_strengthBar->setStyleSheet("QProgressBar::chunk { background-color: #4caf50; }");
        m_strengthLabel->setText("Strength: Strong");
    }
}

int PasswordDialog::calculatePasswordStrength(const QString& password) {
    if (password.isEmpty()) return 0;
    
    int score = 0;
    
    // Length (max 40 points)
    score += qMin(password.length() * 2, 40);
    
    // Character variety
    bool hasLower = false, hasUpper = false, hasDigit = false, hasSpecial = false;
    for (const QChar& c : password) {
        if (c.isLower()) hasLower = true;
        else if (c.isUpper()) hasUpper = true;
        else if (c.isDigit()) hasDigit = true;
        else hasSpecial = true;
    }
    
    if (hasLower) score += 10;
    if (hasUpper) score += 10;
    if (hasDigit) score += 10;
    if (hasSpecial) score += 15;
    
    // Bonus for length > 24 (post-quantum recommendation)
    if (password.length() >= 24) score += 15;
    
    return qMin(score, 100);
}

QString PasswordDialog::getPassword() const {
    return m_passwordEdit->text();
}

QString PasswordDialog::getNewPassword() const {
    return m_newPasswordEdit->text();
}

QString PasswordDialog::getConfirmPassword() const {
    return m_confirmEdit->text();
}

bool PasswordDialog::verifyPassword(QWidget* parent) {
    PasswordDialog dialog(PasswordMode::VERIFY, parent);
    return dialog.exec() == QDialog::Accepted;
}

QString PasswordDialog::setPassword(QWidget* parent) {
    PasswordDialog dialog(PasswordMode::SET_NEW, parent);
    if (dialog.exec() == QDialog::Accepted) {
        return dialog.getNewPassword();
    }
    return QString();
}

bool PasswordDialog::changePassword(QWidget* parent, QString& outNewPassword) {
    PasswordDialog dialog(PasswordMode::CHANGE, parent);
    if (dialog.exec() == QDialog::Accepted) {
        outNewPassword = dialog.getNewPassword();
        return true;
    }
    return false;
}

bool PasswordDialog::unlock(QWidget* parent) {
    PasswordDialog dialog(PasswordMode::UNLOCK, parent);
    if (dialog.exec() == QDialog::Accepted) {
        return SecureStorage::getInstance().unlock(dialog.getPassword());
    }
    return false;
}

} // namespace PCPGP
