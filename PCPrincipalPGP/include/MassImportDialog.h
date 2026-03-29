#ifndef MASSIMPORTDIALOG_H
#define MASSIMPORTDIALOG_H

#include <QDialog>
#include <QStringList>

QT_BEGIN_NAMESPACE
class QLineEdit;
class QPushButton;
class QLabel;
class QProgressBar;
class QListWidget;
class QCheckBox;
class QRadioButton;
QT_END_NAMESPACE

namespace PCPGP {

class PGPManager;

struct ImportResult {
    QString filePath;
    bool success;
    QString keyId;
    QString error;
    bool isPublic;
    bool isPrivate;
    bool matched;
};

class MassImportDialog : public QDialog {
    Q_OBJECT

public:
    explicit MassImportDialog(PGPManager* pgpManager, QWidget* parent = nullptr);
    ~MassImportDialog();
    
    void setPublicKeysDir(const QString& path);
    void setPrivateKeysDir(const QString& path);

private slots:
    void onBrowsePublic();
    void onBrowsePrivate();
    void onStartImport();
    void onCancel();
    void onSelectAll();
    void onDeselectAll();
    void onItemSelectionChanged();
    void onValidatePairs();

private:
    void setupUI();
    void setupConnections();
    void scanDirectories();
    void populateFileList();
    QList<QString> findKeyFiles(const QString& dir);
    bool isKeyFile(const QString& filePath);
    ImportResult importKeyFile(const QString& filePath);
    bool validateKeyPair(const QString& publicPath, const QString& privatePath);
    void updateProgress();
    void showResults();
    
    PGPManager* m_pgpManager;
    
    QLineEdit* m_publicDirEdit;
    QLineEdit* m_privateDirEdit;
    QPushButton* m_browsePublicBtn;
    QPushButton* m_browsePrivateBtn;
    QPushButton* m_startBtn;
    QPushButton* m_cancelBtn;
    QPushButton* m_selectAllBtn;
    QPushButton* m_deselectAllBtn;
    QPushButton* m_validateBtn;
    
    QListWidget* m_fileList;
    QProgressBar* m_progressBar;
    QLabel* m_statusLabel;
    QLabel* m_summaryLabel;
    
    QCheckBox* m_importPublicCheck;
    QCheckBox* m_importPrivateCheck;
    QCheckBox* m_matchPairsCheck;
    QRadioButton* m_copyRadio;
    QRadioButton* m_referenceRadio;
    
    QStringList m_publicFiles;
    QStringList m_privateFiles;
    QList<ImportResult> m_results;
    
    bool m_importing = false;
    bool m_cancelled = false;
};

} // namespace PCPGP

#endif // MASSIMPORTDIALOG_H
