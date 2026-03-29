#ifndef NOTEPADWIDGET_H
#define NOTEPADWIDGET_H

#include <QWidget>
#include <QTextEdit>
#include <memory>

QT_BEGIN_NAMESPACE
class QPushButton;
class QComboBox;
class QLabel;
class QToolBar;
class QLineEdit;
QT_END_NAMESPACE

namespace PCPGP {

class PGPManager;

// Custom text edit with line numbers
class CodeEditor : public QTextEdit {
    Q_OBJECT
public:
    CodeEditor(QWidget* parent = nullptr);
    
    void lineNumberAreaPaintEvent(QPaintEvent* event);
    int lineNumberAreaWidth();
    
protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    
private:
    void updateLineNumberAreaWidth();
    void updateLineNumberArea(const QRect& rect, int dy);
    QWidget* m_lineNumberArea;
};

// Main notepad widget for encryption/decryption
class NotepadWidget : public QWidget {
    Q_OBJECT

public:
    explicit NotepadWidget(PGPManager* pgpManager, QWidget* parent = nullptr);
    ~NotepadWidget();
    
    QString getText() const;
    void setText(const QString& text);
    void appendText(const QString& text);
    void clearText();
    bool isModified() const;
    void setModified(bool modified);
    
    // Operations
    void encrypt(const QString& recipientKeyId);
    void decrypt();
    void sign(const QString& keyId);
    bool verify(QString& outSigner);
    void clearSign(const QString& keyId);
    
    // File operations
    bool openFile(const QString& filePath);
    bool saveFile(const QString& filePath);
    
    // Get selected text
    QString getSelectedText() const;
    void replaceSelectedText(const QString& text);

signals:
    void textChanged();
    void textModified(bool modified);
    void encryptionRequested();
    void decryptionRequested();
    void signingRequested();
    void verificationRequested();
    void statusMessage(const QString& message);

public slots:
    void onCut();
    void onCopy();
    void onPaste();
    void onSelectAll();
    void onUndo();
    void onRedo();
    void onFind();
    void onReplace();
    void onZoomIn();
    void onZoomOut();
    void onResetZoom();
    void onWordWrap(bool wrap);
    
    // These are called by MainWindow
    void onEncryptClicked();
    void onDecryptClicked();
    void onSignClicked();
    void onVerifyClicked();
    void onClearSignClicked();

private slots:
    void onTextModified();
    void onProfileChanged(int index);
    void onInsertPublicKey();
    void onFormatPGPBlock();

private:
    void setupUI();
    void setupToolbar();
    void setupConnections();
    void updateProfileList();
    void showResult(const QString& title, const QString& content);
    
    QString formatPGPMessage(const QString& message);
    QString extractPGPMessage(const QString& text);
    bool isPGPMessage(const QString& text);
    bool isSignedMessage(const QString& text);
    
    PGPManager* m_pgpManager;
    CodeEditor* m_textEdit;
    QComboBox* m_profileCombo;
    QToolBar* m_toolBar;
    QLabel* m_statusLabel;
    QPushButton* m_encryptBtn;
    QPushButton* m_decryptBtn;
    QPushButton* m_signBtn;
    QPushButton* m_verifyBtn;
    QPushButton* m_clearSignBtn;
    QPushButton* m_insertKeyBtn;
    
    bool m_modified = false;
    float m_zoomLevel = 1.0f;
};

} // namespace PCPGP

#endif // NOTEPADWIDGET_H
