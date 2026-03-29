#include "NotepadWidget.h"
#include "PGPManager.h"
#include "EncryptDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QTextEdit>
#include <QToolBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QClipboard>
#include <QApplication>
#include <QInputDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QPlainTextEdit>
#include <QPainter>

namespace PCPGP {

// CodeEditor implementation
class LineNumberArea : public QWidget {
public:
    LineNumberArea(CodeEditor* editor) : QWidget(editor), m_codeEditor(editor) {}
    
    QSize sizeHint() const override {
        return QSize(m_codeEditor->lineNumberAreaWidth(), 0);
    }
    
protected:
    void paintEvent(QPaintEvent* event) override {
        m_codeEditor->lineNumberAreaPaintEvent(event);
    }
    
private:
    CodeEditor* m_codeEditor;
};

CodeEditor::CodeEditor(QWidget* parent) : QTextEdit(parent) {
    setLineWrapMode(QTextEdit::NoWrap);
    setAcceptRichText(false);
    setFont(QFont("Consolas", 10));
    
    m_lineNumberArea = new LineNumberArea(this);
    
    updateLineNumberAreaWidth();
}

void CodeEditor::paintEvent(QPaintEvent* event) {
    QTextEdit::paintEvent(event);
}

void CodeEditor::resizeEvent(QResizeEvent* event) {
    QTextEdit::resizeEvent(event);
    
    QRect cr = contentsRect();
    m_lineNumberArea->setGeometry(QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void CodeEditor::updateLineNumberAreaWidth() {
    setViewportMargins(30, 0, 0, 0);
}

void CodeEditor::updateLineNumberArea(const QRect& rect, int dy) {
    if (dy) {
        m_lineNumberArea->scroll(0, dy);
    } else {
        m_lineNumberArea->update(0, rect.y(), m_lineNumberArea->width(), rect.height());
    }
}

void CodeEditor::lineNumberAreaPaintEvent(QPaintEvent* event) {
    QPainter painter(m_lineNumberArea);
    painter.fillRect(event->rect(), QColor(30, 30, 40));
    
    QTextBlock block = document()->firstBlock();
    int blockNumber = block.blockNumber();
    int top = static_cast<int>(blockBoundingGeometry(block).translated(contentOffset()).top());
    int bottom = top + static_cast<int>(blockBoundingRect(block).height());
    
    while (block.isValid() && top <= event->rect().bottom()) {
        if (block.isVisible() && bottom >= event->rect().top()) {
            QString number = QString::number(blockNumber + 1);
            painter.setPen(QColor(100, 100, 120));
            painter.drawText(0, top, m_lineNumberArea->width() - 5, fontMetrics().height(),
                           Qt::AlignRight, number);
        }
        
        block = block.next();
        top = bottom;
        bottom = top + static_cast<int>(blockBoundingRect(block).height());
        blockNumber++;
    }
}

int CodeEditor::lineNumberAreaWidth() {
    int digits = 1;
    int max = qMax(1, document()->blockCount());
    while (max >= 10) {
        max /= 10;
        digits++;
    }
    
    return 10 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
}

// NotepadWidget implementation
NotepadWidget::NotepadWidget(PGPManager* pgpManager, QWidget* parent)
    : QWidget(parent), m_pgpManager(pgpManager) {
    setupUI();
    setupToolbar();
    setupConnections();
    updateProfileList();
}

NotepadWidget::~NotepadWidget() {
}

void NotepadWidget::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    
    // Toolbar area
    m_toolBar = new QToolBar(this);
    m_toolBar->setMovable(false);
    mainLayout->addWidget(m_toolBar);
    
    // Profile selector
    QHBoxLayout* profileLayout = new QHBoxLayout();
    QLabel* profileLabel = new QLabel("Recipient/Signer:", this);
    profileLayout->addWidget(profileLabel);
    
    m_profileCombo = new QComboBox(this);
    m_profileCombo->setMinimumWidth(300);
    profileLayout->addWidget(m_profileCombo, 1);
    
    profileLayout->addStretch();
    mainLayout->addLayout(profileLayout);
    
    // Text editor
    m_textEdit = new CodeEditor(this);
    m_textEdit->setPlaceholderText("Enter your message here...\n\n"
                                   "Use the buttons below to encrypt, decrypt, sign, or verify messages.");
    mainLayout->addWidget(m_textEdit, 1);
    
    // Action buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    m_encryptBtn = new QPushButton("Encrypt", this);
    m_encryptBtn->setToolTip("Encrypt message for selected recipient");
    buttonLayout->addWidget(m_encryptBtn);
    
    m_decryptBtn = new QPushButton("Decrypt", this);
    m_decryptBtn->setToolTip("Decrypt PGP message");
    buttonLayout->addWidget(m_decryptBtn);
    
    buttonLayout->addSpacing(20);
    
    m_signBtn = new QPushButton("Sign", this);
    m_signBtn->setToolTip("Sign message with selected key");
    buttonLayout->addWidget(m_signBtn);
    
    m_verifyBtn = new QPushButton("Verify", this);
    m_verifyBtn->setToolTip("Verify signed message");
    buttonLayout->addWidget(m_verifyBtn);
    
    m_clearSignBtn = new QPushButton("Clear-sign", this);
    m_clearSignBtn->setToolTip("Create clear-signed message");
    buttonLayout->addWidget(m_clearSignBtn);
    
    buttonLayout->addSpacing(20);
    
    m_insertKeyBtn = new QPushButton("Insert Public Key", this);
    m_insertKeyBtn->setToolTip("Insert your public key into the message");
    buttonLayout->addWidget(m_insertKeyBtn);
    
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
    
    // Status label
    m_statusLabel = new QLabel("Ready", this);
    m_statusLabel->setStyleSheet("color: #888;");
    mainLayout->addWidget(m_statusLabel);
}

void NotepadWidget::setupToolbar() {
    m_toolBar->addAction("New", this, &NotepadWidget::clearText);
    m_toolBar->addAction("Open...", this, [this]() {
        QString path = QFileDialog::getOpenFileName(this, "Open File");
        if (!path.isEmpty()) {
            openFile(path);
        }
    });
    m_toolBar->addAction("Save...", this, [this]() {
        QString path = QFileDialog::getSaveFileName(this, "Save File");
        if (!path.isEmpty()) {
            saveFile(path);
        }
    });
    m_toolBar->addSeparator();
    m_toolBar->addAction("Cut", this, &NotepadWidget::onCut);
    m_toolBar->addAction("Copy", this, &NotepadWidget::onCopy);
    m_toolBar->addAction("Paste", this, &NotepadWidget::onPaste);
    m_toolBar->addSeparator();
    m_toolBar->addAction("Undo", this, &NotepadWidget::onUndo);
    m_toolBar->addAction("Redo", this, &NotepadWidget::onRedo);
}

void NotepadWidget::setupConnections() {
    connect(m_textEdit, &QTextEdit::textChanged, this, &NotepadWidget::onTextModified);
    
    connect(m_encryptBtn, &QPushButton::clicked, this, &NotepadWidget::onEncryptClicked);
    connect(m_decryptBtn, &QPushButton::clicked, this, &NotepadWidget::onDecryptClicked);
    connect(m_signBtn, &QPushButton::clicked, this, &NotepadWidget::onSignClicked);
    connect(m_verifyBtn, &QPushButton::clicked, this, &NotepadWidget::onVerifyClicked);
    connect(m_clearSignBtn, &QPushButton::clicked, this, &NotepadWidget::onClearSignClicked);
    connect(m_insertKeyBtn, &QPushButton::clicked, this, &NotepadWidget::onInsertPublicKey);
    
    connect(m_profileCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &NotepadWidget::onProfileChanged);
}

QString NotepadWidget::getText() const {
    return m_textEdit->toPlainText();
}

void NotepadWidget::setText(const QString& text) {
    m_textEdit->setPlainText(text);
    m_modified = false;
    emit textModified(false);
}

void NotepadWidget::appendText(const QString& text) {
    m_textEdit->append(text);
}

void NotepadWidget::clearText() {
    if (m_modified) {
        auto reply = QMessageBox::question(this, "Unsaved Changes",
            "You have unsaved changes. Clear anyway?",
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::No) {
            return;
        }
    }
    
    m_textEdit->clear();
    m_modified = false;
    emit textModified(false);
}

bool NotepadWidget::isModified() const {
    return m_modified;
}

void NotepadWidget::setModified(bool modified) {
    m_modified = modified;
    emit textModified(modified);
}

void NotepadWidget::encrypt(const QString& recipientKeyId) {
    QString text = getText();
    if (text.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Nothing to encrypt");
        return;
    }
    
    QByteArray encrypted = m_pgpManager->encrypt(text.toUtf8(), recipientKeyId);
    if (encrypted.isEmpty()) {
        QMessageBox::critical(this, "Error", 
            "Encryption failed: " + m_pgpManager->getLastError());
        return;
    }
    
    setText(QString::fromUtf8(encrypted));
    m_statusLabel->setText("Message encrypted successfully");
    emit statusMessage("Message encrypted");
}

void NotepadWidget::decrypt() {
    QString text = getText();
    if (text.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Nothing to decrypt");
        return;
    }
    
    QString keyId;
    QByteArray decrypted = m_pgpManager->decrypt(text.toUtf8(), keyId);
    if (decrypted.isEmpty()) {
        QMessageBox::critical(this, "Error", 
            "Decryption failed: " + m_pgpManager->getLastError());
        return;
    }
    
    setText(QString::fromUtf8(decrypted));
    m_statusLabel->setText("Message decrypted (Key: " + keyId + ")");
    emit statusMessage("Message decrypted");
}

void NotepadWidget::sign(const QString& keyId) {
    QString text = getText();
    if (text.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Nothing to sign");
        return;
    }
    
    QByteArray signature = m_pgpManager->sign(text.toUtf8(), keyId, true);
    if (signature.isEmpty()) {
        QMessageBox::critical(this, "Error", 
            "Signing failed: " + m_pgpManager->getLastError());
        return;
    }
    
    // Append signature to text
    appendText("\n\n-----BEGIN PGP SIGNATURE-----\n");
    appendText(QString::fromUtf8(signature));
    appendText("-----END PGP SIGNATURE-----");
    
    m_statusLabel->setText("Message signed successfully");
    emit statusMessage("Message signed");
}

bool NotepadWidget::verify(QString& outSigner) {
    QString text = getText();
    if (text.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Nothing to verify");
        return false;
    }
    
    // Extract signature from text
    QRegularExpression sigRegex(
        "-----BEGIN PGP SIGNATURE-----\\n(.*?)\\n-----END PGP SIGNATURE-----",
        QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match = sigRegex.match(text);
    
    if (!match.hasMatch()) {
        QMessageBox::warning(this, "Warning", "No signature found in text");
        return false;
    }
    
    QString signature = match.captured(0);
    QString message = text.left(match.capturedStart()).trimmed();
    
    QString signerKeyId;
    if (m_pgpManager->verify(message.toUtf8(), signature.toUtf8(), signerKeyId)) {
        outSigner = signerKeyId;
        QMessageBox::information(this, "Verification Successful",
            "Signature verified.\nSigned by: " + signerKeyId);
        m_statusLabel->setText("Signature verified (Signer: " + signerKeyId + ")");
        emit statusMessage("Signature verified");
        return true;
    } else {
        QMessageBox::critical(this, "Verification Failed",
            "Signature verification failed.");
        m_statusLabel->setText("Signature verification failed");
        emit statusMessage("Verification failed");
        return false;
    }
}

void NotepadWidget::clearSign(const QString& keyId) {
    QString text = getText();
    if (text.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Nothing to sign");
        return;
    }
    
    QByteArray signedText = m_pgpManager->cleartextSign(text.toUtf8(), keyId);
    if (signedText.isEmpty()) {
        QMessageBox::critical(this, "Error", 
            "Signing failed: " + m_pgpManager->getLastError());
        return;
    }
    
    setText(QString::fromUtf8(signedText));
    m_statusLabel->setText("Clear-signed message created");
    emit statusMessage("Message clear-signed");
}

bool NotepadWidget::openFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, "Error", "Failed to open file");
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    setText(QString::fromUtf8(data));
    m_statusLabel->setText("File opened: " + filePath);
    return true;
}

bool NotepadWidget::saveFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::critical(this, "Error", "Failed to save file");
        return false;
    }
    
    file.write(getText().toUtf8());
    file.close();
    
    m_modified = false;
    emit textModified(false);
    m_statusLabel->setText("File saved: " + filePath);
    return true;
}

QString NotepadWidget::getSelectedText() const {
    return m_textEdit->textCursor().selectedText();
}

void NotepadWidget::replaceSelectedText(const QString& text) {
    m_textEdit->textCursor().insertText(text);
}

void NotepadWidget::onTextModified() {
    m_modified = true;
    emit textModified(true);
}

void NotepadWidget::onEncryptClicked() {
    QString recipient = m_profileCombo->currentData().toString();
    if (recipient.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please select a recipient");
        return;
    }
    
    encrypt(recipient);
    emit encryptionRequested();
}

void NotepadWidget::onDecryptClicked() {
    decrypt();
    emit decryptionRequested();
}

void NotepadWidget::onSignClicked() {
    QString signer = m_profileCombo->currentData().toString();
    if (signer.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please select a signing key");
        return;
    }
    
    sign(signer);
    emit signingRequested();
}

void NotepadWidget::onVerifyClicked() {
    QString signer;
    verify(signer);
    emit verificationRequested();
}

void NotepadWidget::onClearSignClicked() {
    QString signer = m_profileCombo->currentData().toString();
    if (signer.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please select a signing key");
        return;
    }
    
    clearSign(signer);
}

void NotepadWidget::onProfileChanged(int index) {
    // Update UI based on selected profile
}

void NotepadWidget::onInsertPublicKey() {
    QString keyId = m_profileCombo->currentData().toString();
    if (keyId.isEmpty()) {
        QMessageBox::warning(this, "Warning", "Please select a profile");
        return;
    }
    
    QByteArray keyData = m_pgpManager->getPublicKeyArmored(keyId);
    if (keyData.isEmpty()) {
        QMessageBox::critical(this, "Error", "Failed to get public key");
        return;
    }
    
    appendText("\n\n" + QString::fromUtf8(keyData));
    m_statusLabel->setText("Public key inserted");
}

void NotepadWidget::onFormatPGPBlock() {
    // Format the current text as a proper PGP block
}

void NotepadWidget::onCut() {
    m_textEdit->cut();
}

void NotepadWidget::onCopy() {
    m_textEdit->copy();
}

void NotepadWidget::onPaste() {
    m_textEdit->paste();
}

void NotepadWidget::onSelectAll() {
    m_textEdit->selectAll();
}

void NotepadWidget::onUndo() {
    m_textEdit->undo();
}

void NotepadWidget::onRedo() {
    m_textEdit->redo();
}

void NotepadWidget::onFind() {
    // Open find dialog
}

void NotepadWidget::onReplace() {
    // Open replace dialog
}

void NotepadWidget::onZoomIn() {
    m_zoomLevel *= 1.1f;
    m_textEdit->setStyleSheet(QString("font-size: %1pt;").arg(10 * m_zoomLevel));
}

void NotepadWidget::onZoomOut() {
    m_zoomLevel /= 1.1f;
    m_textEdit->setStyleSheet(QString("font-size: %1pt;").arg(10 * m_zoomLevel));
}

void NotepadWidget::onResetZoom() {
    m_zoomLevel = 1.0f;
    m_textEdit->setStyleSheet("font-size: 10pt;");
}

void NotepadWidget::onWordWrap(bool wrap) {
    m_textEdit->setLineWrapMode(wrap ? QTextEdit::WidgetWidth : QTextEdit::NoWrap);
}

void NotepadWidget::updateProfileList() {
    m_profileCombo->clear();
    m_profileCombo->addItem("-- Select Profile --", QString());
    
    auto keys = m_pgpManager->listPublicKeys();
    for (const auto& key : keys) {
        QString display = key.userId;
        if (display.isEmpty()) display = key.email;
        if (display.isEmpty()) display = key.keyId;
        
        m_profileCombo->addItem(display, key.keyId);
    }
}

void NotepadWidget::showResult(const QString& title, const QString& content) {
    QDialog dialog(this);
    dialog.setWindowTitle(title);
    dialog.setMinimumSize(600, 400);
    
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    
    QTextEdit* textEdit = new QTextEdit(&dialog);
    textEdit->setPlainText(content);
    textEdit->setReadOnly(true);
    layout->addWidget(textEdit);
    
    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Save, &dialog);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout->addWidget(buttons);
    
    dialog.exec();
}

QString NotepadWidget::formatPGPMessage(const QString& message) {
    // Format as proper PGP message block
    return message;
}

QString NotepadWidget::extractPGPMessage(const QString& text) {
    // Extract PGP message from text
    QRegularExpression msgRegex(
        "-----BEGIN PGP MESSAGE-----\\n(.*?)\\n-----END PGP MESSAGE-----",
        QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatch match = msgRegex.match(text);
    
    if (match.hasMatch()) {
        return match.captured(0);
    }
    
    return text;
}

bool NotepadWidget::isPGPMessage(const QString& text) {
    return text.contains("-----BEGIN PGP MESSAGE-----") &&
           text.contains("-----END PGP MESSAGE-----");
}

bool NotepadWidget::isSignedMessage(const QString& text) {
    return text.contains("-----BEGIN PGP SIGNED MESSAGE-----") ||
           text.contains("-----BEGIN PGP SIGNATURE-----");
}

} // namespace PCPGP
