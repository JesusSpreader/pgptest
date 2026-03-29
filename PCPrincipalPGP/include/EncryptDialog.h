#ifndef ENCRYPTDIALOG_H
#define ENCRYPTDIALOG_H

#include <QDialog>
#include <QStringList>

QT_BEGIN_NAMESPACE
class QComboBox;
class QPushButton;
class QLabel;
class QListWidget;
class QCheckBox;
class QLineEdit;
class QTextEdit;
class QTabWidget;
QT_END_NAMESPACE

namespace PCPGP {

enum class EncryptMode {
    ENCRYPT,
    DECRYPT,
    SIGN,
    VERIFY
};

class EncryptDialog : public QDialog {
    Q_OBJECT

public:
    explicit EncryptDialog(EncryptMode mode, QWidget* parent = nullptr);
    ~EncryptDialog();
    
    void setInputText(const QString& text);
    void setInputFile(const QString& filePath);
    QString getOutputText() const;
    QString getOutputFile() const;
    
    // For encryption
    QStringList getRecipientKeys() const;
    QString getSignerKey() const;
    bool isArmorEnabled() const;
    bool isSignEnabled() const;
    
    // For file operations
    QString getInputFile() const;
    QString getOutputFilePath() const;

private slots:
    void onOKClicked();
    void onCancelClicked();
    void onBrowseInput();
    void onBrowseOutput();
    void onAddRecipient();
    void onRemoveRecipient();
    void onRecipientSelectionChanged();
    void onModeChanged(int index);
    void onInputTypeChanged(bool isFile);
    void onShowPasswordToggled(bool checked);

private:
    void setupUI();
    void setupConnections();
    void populateKeys();
    bool validateInput();
    void updateUIForMode();
    void performOperation();
    
    EncryptMode m_mode;
    QTabWidget* m_tabWidget;
    
    // Text tab
    QTextEdit* m_inputTextEdit;
    QTextEdit* m_outputTextEdit;
    
    // File tab
    QLineEdit* m_inputFileEdit;
    QLineEdit* m_outputFileEdit;
    QPushButton* m_browseInputBtn;
    QPushButton* m_browseOutputBtn;
    
    // Options
    QComboBox* m_signerCombo;
    QListWidget* m_recipientList;
    QComboBox* m_availableKeysCombo;
    QPushButton* m_addRecipientBtn;
    QPushButton* m_removeRecipientBtn;
    QCheckBox* m_armorCheck;
    QCheckBox* m_signCheck;
    QCheckBox* m_fileModeCheck;
    QLineEdit* m_passphraseEdit;
    QCheckBox* m_showPassphraseCheck;
    
    // Buttons
    QPushButton* m_okBtn;
    QPushButton* m_cancelBtn;
    
    QLabel* m_statusLabel;
    
    QString m_resultText;
    QString m_resultFile;
};

} // namespace PCPGP

#endif // ENCRYPTDIALOG_H
