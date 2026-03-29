#ifndef KEYMANAGERDIALOG_H
#define KEYMANAGERDIALOG_H

#include <QDialog>
#include <QList>
#include "PGPManager.h"

QT_BEGIN_NAMESPACE
class QTableWidget;
class QPushButton;
class QLineEdit;
class QLabel;
class QTabWidget;
QT_END_NAMESPACE

namespace PCPGP {

class KeyManagerDialog : public QDialog {
    Q_OBJECT

public:
    explicit KeyManagerDialog(PGPManager* pgpManager, QWidget* parent = nullptr);
    ~KeyManagerDialog();
    
    void refreshKeys();

private slots:
    void onGenerateKey();
    void onImportKey();
    void onExportPublic();
    void onExportPrivate();
    void onDeleteKey();
    void onViewDetails();
    void onSetTrust();
    void onChangePassphrase();
    void onSearchTextChanged(const QString& text);
    void onKeySelectionChanged();
    void onRefresh();
    void onMassImport();
    void onTabChanged(int index);

private:
    void setupUI();
    void setupConnections();
    void populatePublicKeys();
    void populatePrivateKeys();
    void updateButtonStates();
    PGPKey getSelectedKey();
    void showKeyDetails(const PGPKey& key);
    
    PGPManager* m_pgpManager;
    QTabWidget* m_tabWidget;
    QTableWidget* m_publicTable;
    QTableWidget* m_privateTable;
    QLineEdit* m_searchEdit;
    QPushButton* m_generateBtn;
    QPushButton* m_importBtn;
    QPushButton* m_exportPublicBtn;
    QPushButton* m_exportPrivateBtn;
    QPushButton* m_deleteBtn;
    QPushButton* m_detailsBtn;
    QPushButton* m_trustBtn;
    QPushButton* m_passphraseBtn;
    QPushButton* m_refreshBtn;
    QPushButton* m_massImportBtn;
    QLabel* m_infoLabel;
    
    QList<PGPKey> m_publicKeys;
    QList<PGPKey> m_privateKeys;
};

} // namespace PCPGP

#endif // KEYMANAGERDIALOG_H
