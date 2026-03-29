#include "EncryptDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QCheckBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QFileDialog>
#include <QMessageBox>
#include <QGroupBox>

namespace PCPGP {

EncryptDialog::EncryptDialog(EncryptMode mode, QWidget* parent) 
    : QDialog(parent), m_mode(mode) {
    
    setupUI();
    setupConnections();
    updateUIForMode();
    populateKeys();
}

EncryptDialog::~EncryptDialog() {
}

void EncryptDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    
    // Tab widget for text/file modes
    m_tabWidget = new QTabWidget(this);
    
    // Text tab
    QWidget* textTab = new QWidget(this);
    QVBoxLayout* textLayout = new QVBoxLayout(textTab);
    
    textLayout->addWidget(new QLabel("Input:", this));
    m_inputTextEdit = new QTextEdit(this);
    m_inputTextEdit->setPlaceholderText("Enter or paste your message here...");
    textLayout->addWidget(m_inputTextEdit);
    
    textLayout->addWidget(new QLabel("Output:", this));
    m_outputTextEdit = new QTextEdit(this);
    m_outputTextEdit->setReadOnly(true);
    m_outputTextEdit->setPlaceholderText("Result will appear here...");
    textLayout->addWidget(m_outputTextEdit);
    
    m_tabWidget->addTab(textTab, "Text");
    
    // File tab
    QWidget* fileTab = new QWidget(this);
    QVBoxLayout* fileLayout = new QVBoxLayout(fileTab);
    
    QHBoxLayout* inputFileLayout = new QHBoxLayout();
    inputFileLayout->addWidget(new QLabel("Input File:", this));
    m_inputFileEdit = new QLineEdit(this);
    inputFileLayout->addWidget(m_inputFileEdit, 1);
    m_browseInputBtn = new QPushButton("Browse...", this);
    inputFileLayout->addWidget(m_browseInputBtn);
    fileLayout->addLayout(inputFileLayout);
    
    QHBoxLayout* outputFileLayout = new QHBoxLayout();
    outputFileLayout->addWidget(new QLabel("Output File:", this));
    m_outputFileEdit = new QLineEdit(this);
    outputFileLayout->addWidget(m_outputFileEdit, 1);
    m_browseOutputBtn = new QPushButton("Browse...", this);
    outputFileLayout->addWidget(m_browseOutputBtn);
    fileLayout->addLayout(outputFileLayout);
    
    m_tabWidget->addTab(fileTab, "File");
    
    mainLayout->addWidget(m_tabWidget);
    
    // Options group
    QGroupBox* optionsGroup = new QGroupBox("Options", this);
    QVBoxLayout* optionsLayout = new QVBoxLayout(optionsGroup);
    
    // Signer/Recipient selection
    QHBoxLayout* signerLayout = new QHBoxLayout();
    signerLayout->addWidget(new QLabel("Your Key:", this));
    m_signerCombo = new QComboBox(this);
    signerLayout->addWidget(m_signerCombo, 1);
    optionsLayout->addLayout(signerLayout);
    
    // Recipients list (for encryption)
    optionsLayout->addWidget(new QLabel("Recipients:", this));
    m_recipientList = new QListWidget(this);
    m_recipientList->setSelectionMode(QAbstractItemView::MultiSelection);
    m_recipientList->setMaximumHeight(100);
    optionsLayout->addWidget(m_recipientList);
    
    QHBoxLayout* recipientBtnLayout = new QHBoxLayout();
    m_availableKeysCombo = new QComboBox(this);
    recipientBtnLayout->addWidget(m_availableKeysCombo, 1);
    m_addRecipientBtn = new QPushButton("Add", this);
    recipientBtnLayout->addWidget(m_addRecipientBtn);
    m_removeRecipientBtn = new QPushButton("Remove", this);
    recipientBtnLayout->addWidget(m_removeRecipientBtn);
    optionsLayout->addLayout(recipientBtnLayout);
    
    // Checkboxes
    m_armorCheck = new QCheckBox("ASCII Armor (text output)", this);
    m_armorCheck->setChecked(true);
    optionsLayout->addWidget(m_armorCheck);
    
    m_signCheck = new QCheckBox("Sign message", this);
    optionsLayout->addWidget(m_signCheck);
    
    // Passphrase (for signing)
    QHBoxLayout* passLayout = new QHBoxLayout();
    passLayout->addWidget(new QLabel("Passphrase:", this));
    m_passphraseEdit = new QLineEdit(this);
    m_passphraseEdit->setEchoMode(QLineEdit::Password);
    passLayout->addWidget(m_passphraseEdit, 1);
    m_showPassphraseCheck = new QCheckBox("Show", this);
    passLayout->addWidget(m_showPassphraseCheck);
    optionsLayout->addLayout(passLayout);
    
    mainLayout->addWidget(optionsGroup);
    
    // Status
    m_statusLabel = new QLabel(this);
    m_statusLabel->setStyleSheet("color: #888;");
    mainLayout->addWidget(m_statusLabel);
    
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

void EncryptDialog::setupConnections() {
    connect(m_okBtn, &QPushButton::clicked, this, &EncryptDialog::onOKClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &EncryptDialog::onCancelClicked);
    connect(m_browseInputBtn, &QPushButton::clicked, this, &EncryptDialog::onBrowseInput);
    connect(m_browseOutputBtn, &QPushButton::clicked, this, &EncryptDialog::onBrowseOutput);
    connect(m_addRecipientBtn, &QPushButton::clicked, this, &EncryptDialog::onAddRecipient);
    connect(m_removeRecipientBtn, &QPushButton::clicked, this, &EncryptDialog::onRemoveRecipient);
    connect(m_showPassphraseCheck, &QCheckBox::toggled, this, &EncryptDialog::onShowPasswordToggled);
}

void EncryptDialog::populateKeys() {
    // This would be populated from PGPManager
    // For now, placeholder
}

void EncryptDialog::updateUIForMode() {
    switch (m_mode) {
        case EncryptMode::ENCRYPT:
            setWindowTitle("Encrypt");
            m_okBtn->setText("Encrypt");
            m_recipientList->setEnabled(true);
            m_signCheck->setEnabled(true);
            break;
        case EncryptMode::DECRYPT:
            setWindowTitle("Decrypt");
            m_okBtn->setText("Decrypt");
            m_recipientList->setEnabled(false);
            m_signCheck->setEnabled(false);
            break;
        case EncryptMode::SIGN:
            setWindowTitle("Sign");
            m_okBtn->setText("Sign");
            m_recipientList->setEnabled(false);
            m_signCheck->setEnabled(false);
            break;
        case EncryptMode::VERIFY:
            setWindowTitle("Verify");
            m_okBtn->setText("Verify");
            m_recipientList->setEnabled(false);
            m_signCheck->setEnabled(false);
            break;
    }
}

void EncryptDialog::setInputText(const QString& text) {
    m_inputTextEdit->setPlainText(text);
}

void EncryptDialog::setInputFile(const QString& filePath) {
    m_inputFileEdit->setText(filePath);
    m_tabWidget->setCurrentIndex(1);
}

QString EncryptDialog::getOutputText() const {
    return m_resultText;
}

QString EncryptDialog::getOutputFile() const {
    return m_resultFile;
}

QStringList EncryptDialog::getRecipientKeys() const {
    QStringList keys;
    for (int i = 0; i < m_recipientList->count(); i++) {
        keys.append(m_recipientList->item(i)->data(Qt::UserRole).toString());
    }
    return keys;
}

QString EncryptDialog::getSignerKey() const {
    return m_signerCombo->currentData().toString();
}

bool EncryptDialog::isArmorEnabled() const {
    return m_armorCheck->isChecked();
}

bool EncryptDialog::isSignEnabled() const {
    return m_signCheck->isChecked();
}

QString EncryptDialog::getInputFile() const {
    return m_inputFileEdit->text();
}

QString EncryptDialog::getOutputFilePath() const {
    return m_outputFileEdit->text();
}

void EncryptDialog::onOKClicked() {
    if (!validateInput()) {
        return;
    }
    
    performOperation();
}

void EncryptDialog::onCancelClicked() {
    reject();
}

void EncryptDialog::onBrowseInput() {
    QString path = QFileDialog::getOpenFileName(this, "Select Input File");
    if (!path.isEmpty()) {
        m_inputFileEdit->setText(path);
    }
}

void EncryptDialog::onBrowseOutput() {
    QString path = QFileDialog::getSaveFileName(this, "Select Output File");
    if (!path.isEmpty()) {
        m_outputFileEdit->setText(path);
    }
}

void EncryptDialog::onAddRecipient() {
    QString keyId = m_availableKeysCombo->currentData().toString();
    QString name = m_availableKeysCombo->currentText();
    
    if (keyId.isEmpty()) return;
    
    // Check if already added
    for (int i = 0; i < m_recipientList->count(); i++) {
        if (m_recipientList->item(i)->data(Qt::UserRole).toString() == keyId) {
            return;
        }
    }
    
    QListWidgetItem* item = new QListWidgetItem(name);
    item->setData(Qt::UserRole, keyId);
    m_recipientList->addItem(item);
}

void EncryptDialog::onRemoveRecipient() {
    qDeleteAll(m_recipientList->selectedItems());
}

void EncryptDialog::onRecipientSelectionChanged() {
    // Update UI
}

void EncryptDialog::onModeChanged(int index) {
    Q_UNUSED(index)
}

void EncryptDialog::onInputTypeChanged(bool isFile) {
    m_tabWidget->setCurrentIndex(isFile ? 1 : 0);
}

void EncryptDialog::onShowPasswordToggled(bool checked) {
    m_passphraseEdit->setEchoMode(checked ? QLineEdit::Normal : QLineEdit::Password);
}

bool EncryptDialog::validateInput() {
    if (m_tabWidget->currentIndex() == 0) {
        // Text mode
        if (m_inputTextEdit->toPlainText().isEmpty()) {
            QMessageBox::warning(this, "Validation Error", "Please enter text to process.");
            return false;
        }
    } else {
        // File mode
        if (m_inputFileEdit->text().isEmpty()) {
            QMessageBox::warning(this, "Validation Error", "Please select an input file.");
            return false;
        }
        if (m_outputFileEdit->text().isEmpty()) {
            QMessageBox::warning(this, "Validation Error", "Please select an output file.");
            return false;
        }
    }
    
    if (m_mode == EncryptMode::ENCRYPT && getRecipientKeys().isEmpty()) {
        QMessageBox::warning(this, "Validation Error", "Please add at least one recipient.");
        return false;
    }
    
    return true;
}

void EncryptDialog::performOperation() {
    // This would perform the actual encryption/decryption/signing/verification
    // For now, just accept the dialog
    accept();
}

} // namespace PCPGP
