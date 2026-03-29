#ifndef GENERATEKEYDIALOG_H
#define GENERATEKEYDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QComboBox;
class QSpinBox;
class QCheckBox;
class QPushButton;
class QLabel;
class QProgressBar;
QT_END_NAMESPACE

namespace PCPGP {

class GenerateKeyDialog : public QDialog {
    Q_OBJECT

public:
    explicit GenerateKeyDialog(QWidget* parent = nullptr);
    ~GenerateKeyDialog();
    
    QString getName() const;
    QString getEmail() const;
    QString getPassphrase() const;
    QString getConfirmPassphrase() const;
    int getKeyLength() const;
    int getExpirationDays() const;
    bool isExpirationEnabled() const;
    bool passphrasesMatch() const;

private slots:
    void onGenerateClicked();
    void onCancelClicked();
    void onPassphraseChanged();
    void onShowPassphraseToggled(bool checked);
    void onGenerateProgress();

private:
    void setupUI();
    void setupConnections();
    bool validateInput();
    void updateStrengthIndicator();
    int calculatePasswordStrength(const QString& password);
    
    QLineEdit* m_nameEdit;
    QLineEdit* m_emailEdit;
    QLineEdit* m_passphraseEdit;
    QLineEdit* m_confirmEdit;
    QComboBox* m_keyLengthCombo;
    QSpinBox* m_expirationSpin;
    QCheckBox* m_expirationCheck;
    QCheckBox* m_showPassphraseCheck;
    QPushButton* m_generateBtn;
    QPushButton* m_cancelBtn;
    QLabel* m_strengthLabel;
    QProgressBar* m_strengthBar;
    QLabel* m_infoLabel;
};

} // namespace PCPGP

#endif // GENERATEKEYDIALOG_H
