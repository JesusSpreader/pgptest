#include "GenerateKeyDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>
#include <QProgressBar>
#include <QGroupBox>
#include <QMessageBox>

namespace PCPGP {

GenerateKeyDialog::GenerateKeyDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Generate New Key Pair");
    setMinimumWidth(450);
    
    setupUI();
    setupConnections();
}

GenerateKeyDialog::~GenerateKeyDialog() {
}

void GenerateKeyDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // Info label
    m_infoLabel = new QLabel("Generate a new PGP key pair for encryption and signing.", this);
    m_infoLabel->setWordWrap(true);
    m_infoLabel->setStyleSheet("color: #888;");
    mainLayout->addWidget(m_infoLabel);
    
    // Form group
    QGroupBox* formGroup = new QGroupBox("Key Information", this);
    QGridLayout* formLayout = new QGridLayout(formGroup);
    
    // Name
    formLayout->addWidget(new QLabel("Full Name:*", this), 0, 0);
    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText("Your full name");
    formLayout->addWidget(m_nameEdit, 0, 1);
    
    // Email
    formLayout->addWidget(new QLabel("Email:*", this), 1, 0);
    m_emailEdit = new QLineEdit(this);
    m_emailEdit->setPlaceholderText("your.email@example.com");
    formLayout->addWidget(m_emailEdit, 1, 1);
    
    // Key length
    formLayout->addWidget(new QLabel("Key Length:", this), 2, 0);
    m_keyLengthCombo = new QComboBox(this);
    m_keyLengthCombo->addItem("4096 bits (Recommended)", 4096);
    m_keyLengthCombo->addItem("3072 bits", 3072);
    m_keyLengthCombo->addItem("2048 bits", 2048);
    formLayout->addWidget(m_keyLengthCombo, 2, 1);
    
    // Expiration
    formLayout->addWidget(new QLabel("Expiration:", this), 3, 0);
    QHBoxLayout* expirationLayout = new QHBoxLayout();
    m_expirationCheck = new QCheckBox("Expires in", this);
    m_expirationCheck->setChecked(false);
    expirationLayout->addWidget(m_expirationCheck);
    m_expirationSpin = new QSpinBox(this);
    m_expirationSpin->setRange(1, 3650);
    m_expirationSpin->setValue(365);
    m_expirationSpin->setSuffix(" days");
    m_expirationSpin->setEnabled(false);
    expirationLayout->addWidget(m_expirationSpin);
    expirationLayout->addStretch();
    formLayout->addLayout(expirationLayout, 3, 1);
    
    mainLayout->addWidget(formGroup);
    
    // Passphrase group
    QGroupBox* passGroup = new QGroupBox("Passphrase (Optional)", this);
    QGridLayout* passLayout = new QGridLayout(passGroup);
    
    passLayout->addWidget(new QLabel("Passphrase:", this), 0, 0);
    m_passphraseEdit = new QLineEdit(this);
    m_passphraseEdit->setEchoMode(QLineEdit::Password);
    m_passphraseEdit->setPlaceholderText("Leave empty for no passphrase");
    passLayout->addWidget(m_passphraseEdit, 0, 1);
    
    passLayout->addWidget(new QLabel("Confirm:", this), 1, 0);
    m_confirmEdit = new QLineEdit(this);
    m_confirmEdit->setEchoMode(QLineEdit::Password);
    passLayout->addWidget(m_confirmEdit, 1, 1);
    
    m_showPassphraseCheck = new QCheckBox("Show passphrase", this);
    passLayout->addWidget(m_showPassphraseCheck, 2, 1);
    
    // Strength indicator
    QHBoxLayout* strengthLayout = new QHBoxLayout();
    m_strengthLabel = new QLabel("Strength:", this);
    strengthLayout->addWidget(m_strengthLabel);
    m_strengthBar = new QProgressBar(this);
    m_strengthBar->setRange(0, 100);
    m_strengthBar->setValue(0);
    m_strengthBar->setTextVisible(false);
    m_strengthBar->setFixedHeight(8);
    strengthLayout->addWidget(m_strengthBar, 1);
    passLayout->addLayout(strengthLayout, 3, 1);
    
    mainLayout->addWidget(passGroup);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    m_cancelBtn = new QPushButton("Cancel", this);
    buttonLayout->addWidget(m_cancelBtn);
    
    m_generateBtn = new QPushButton("Generate", this);
    m_generateBtn->setDefault(true);
    buttonLayout->addWidget(m_generateBtn);
    
    mainLayout->addLayout(buttonLayout);
}

void GenerateKeyDialog::setupConnections() {
    connect(m_generateBtn, &QPushButton::clicked, this, &GenerateKeyDialog::onGenerateClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &GenerateKeyDialog::onCancelClicked);
    connect(m_passphraseEdit, &QLineEdit::textChanged, this, &GenerateKeyDialog::onPassphraseChanged);
    connect(m_showPassphraseCheck, &QCheckBox::toggled, this, &GenerateKeyDialog::onShowPassphraseToggled);
    connect(m_expirationCheck, &QCheckBox::toggled, m_expirationSpin, &QSpinBox::setEnabled);
}

void GenerateKeyDialog::onGenerateClicked() {
    if (!validateInput()) {
        return;
    }
    
    accept();
}

void GenerateKeyDialog::onCancelClicked() {
    reject();
}

void GenerateKeyDialog::onPassphraseChanged() {
    updateStrengthIndicator();
}

void GenerateKeyDialog::onShowPassphraseToggled(bool checked) {
    m_passphraseEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
    m_confirmEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
}

void GenerateKeyDialog::onGenerateProgress() {
    // Update progress during key generation
}

bool GenerateKeyDialog::validateInput() {
    if (m_nameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Please enter your full name.");
        m_nameEdit->setFocus();
        return false;
    }
    
    if (m_emailEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Please enter your email address.");
        m_emailEdit->setFocus();
        return false;
    }
    
    if (!m_emailEdit->text().contains("@")) {
        QMessageBox::warning(this, "Validation Error", "Please enter a valid email address.");
        m_emailEdit->setFocus();
        return false;
    }
    
    if (!passphrasesMatch()) {
        QMessageBox::warning(this, "Validation Error", "Passphrases do not match.");
        m_confirmEdit->setFocus();
        return false;
    }
    
    return true;
}

void GenerateKeyDialog::updateStrengthIndicator() {
    int strength = calculatePasswordStrength(m_passphraseEdit->text());
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

int GenerateKeyDialog::calculatePasswordStrength(const QString& password) {
    if (password.isEmpty()) return 0;
    
    int score = 0;
    
    // Length
    score += qMin(password.length() * 4, 40);
    
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
    
    // Penalize repetition
    QSet<QChar> uniqueChars;
    for (const QChar& c : password) {
        uniqueChars.insert(c);
    }
    score += (uniqueChars.size() * 2);
    
    return qMin(score, 100);
}

QString GenerateKeyDialog::getName() const {
    return m_nameEdit->text().trimmed();
}

QString GenerateKeyDialog::getEmail() const {
    return m_emailEdit->text().trimmed();
}

QString GenerateKeyDialog::getPassphrase() const {
    return m_passphraseEdit->text();
}

QString GenerateKeyDialog::getConfirmPassphrase() const {
    return m_confirmEdit->text();
}

int GenerateKeyDialog::getKeyLength() const {
    return m_keyLengthCombo->currentData().toInt();
}

int GenerateKeyDialog::getExpirationDays() const {
    return m_expirationSpin->value();
}

bool GenerateKeyDialog::isExpirationEnabled() const {
    return m_expirationCheck->isChecked();
}

bool GenerateKeyDialog::passphrasesMatch() const {
    if (m_passphraseEdit->text().isEmpty() && m_confirmEdit->text().isEmpty()) {
        return true; // Both empty is valid (no passphrase)
    }
    return m_passphraseEdit->text() == m_confirmEdit->text();
}

} // namespace PCPGP
