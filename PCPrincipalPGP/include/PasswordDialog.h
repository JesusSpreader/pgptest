#ifndef PASSWORDDIALOG_H
#define PASSWORDDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QPushButton;
class QLabel;
class QProgressBar;
class QCheckBox;
QT_END_NAMESPACE

namespace PCPGP {

enum class PasswordMode {
    VERIFY,         // Just verify existing password
    SET_NEW,        // Set new password
    CHANGE,         // Change existing password
    UNLOCK          // Unlock application
};

class PasswordDialog : public QDialog {
    Q_OBJECT

public:
    explicit PasswordDialog(PasswordMode mode, QWidget* parent = nullptr);
    ~PasswordDialog();
    
    QString getPassword() const;
    QString getNewPassword() const;
    QString getConfirmPassword() const;
    
    static bool verifyPassword(QWidget* parent);
    static QString setPassword(QWidget* parent);
    static bool changePassword(QWidget* parent, QString& outNewPassword);
    static bool unlock(QWidget* parent);

private slots:
    void onOKClicked();
    void onCancelClicked();
    void onPasswordChanged();
    void onShowPasswordToggled(bool checked);

private:
    void setupUI();
    void setupConnections();
    bool validateInput();
    void updateStrengthIndicator();
    int calculatePasswordStrength(const QString& password);
    
    PasswordMode m_mode;
    QLineEdit* m_passwordEdit;
    QLineEdit* m_newPasswordEdit;
    QLineEdit* m_confirmEdit;
    QCheckBox* m_showPasswordCheck;
    QPushButton* m_okBtn;
    QPushButton* m_cancelBtn;
    QLabel* m_titleLabel;
    QLabel* m_infoLabel;
    QLabel* m_strengthLabel;
    QProgressBar* m_strengthBar;
};

} // namespace PCPGP

#endif // PASSWORDDIALOG_H
