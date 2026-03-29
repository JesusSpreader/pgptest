#include "MassImportDialog.h"
#include "PGPManager.h"
#include "ConfigManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QListWidget>
#include <QCheckBox>
#include <QRadioButton>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QFileInfo>
#include <QApplication>

namespace PCPGP {

MassImportDialog::MassImportDialog(PGPManager* pgpManager, QWidget* parent)
    : QDialog(parent), m_pgpManager(pgpManager) {
    
    setWindowTitle("Mass Import Keys");
    setMinimumSize(700, 500);
    
    setupUI();
    setupConnections();
}

MassImportDialog::~MassImportDialog() {
}

void MassImportDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    
    // Directory selection
    QGroupBox* dirGroup = new QGroupBox("Key Directories", this);
    QGridLayout* dirLayout = new QGridLayout(dirGroup);
    
    dirLayout->addWidget(new QLabel("Public Keys Directory:", this), 0, 0);
    m_publicDirEdit = new QLineEdit(this);
    dirLayout->addWidget(m_publicDirEdit, 0, 1);
    m_browsePublicBtn = new QPushButton("Browse...", this);
    dirLayout->addWidget(m_browsePublicBtn, 0, 2);
    
    dirLayout->addWidget(new QLabel("Private Keys Directory:", this), 1, 0);
    m_privateDirEdit = new QLineEdit(this);
    dirLayout->addWidget(m_privateDirEdit, 1, 1);
    m_browsePrivateBtn = new QPushButton("Browse...", this);
    dirLayout->addWidget(m_browsePrivateBtn, 1, 2);
    
    mainLayout->addWidget(dirGroup);
    
    // Options
    QGroupBox* optionsGroup = new QGroupBox("Import Options", this);
    QVBoxLayout* optionsLayout = new QVBoxLayout(optionsGroup);
    
    m_importPublicCheck = new QCheckBox("Import public keys", this);
    m_importPublicCheck->setChecked(true);
    optionsLayout->addWidget(m_importPublicCheck);
    
    m_importPrivateCheck = new QCheckBox("Import private keys", this);
    m_importPrivateCheck->setChecked(true);
    optionsLayout->addWidget(m_importPrivateCheck);
    
    m_matchPairsCheck = new QCheckBox("Match key pairs (import only matching public/private)", this);
    m_matchPairsCheck->setChecked(true);
    optionsLayout->addWidget(m_matchPairsCheck);
    
    QHBoxLayout* modeLayout = new QHBoxLayout();
    m_copyRadio = new QRadioButton("Copy keys to app directory", this);
    m_copyRadio->setChecked(true);
    modeLayout->addWidget(m_copyRadio);
    m_referenceRadio = new QRadioButton("Reference original locations", this);
    modeLayout->addWidget(m_referenceRadio);
    modeLayout->addStretch();
    optionsLayout->addLayout(modeLayout);
    
    mainLayout->addWidget(optionsGroup);
    
    // File list
    mainLayout->addWidget(new QLabel("Keys to Import:", this));
    m_fileList = new QListWidget(this);
    m_fileList->setSelectionMode(QAbstractItemView::MultiSelection);
    mainLayout->addWidget(m_fileList, 1);
    
    // Progress
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    mainLayout->addWidget(m_progressBar);
    
    m_statusLabel = new QLabel("Ready", this);
    m_statusLabel->setStyleSheet("color: #888;");
    mainLayout->addWidget(m_statusLabel);
    
    m_summaryLabel = new QLabel(this);
    m_summaryLabel->setStyleSheet("color: #4caf50;");
    mainLayout->addWidget(m_summaryLabel);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    m_selectAllBtn = new QPushButton("Select All", this);
    buttonLayout->addWidget(m_selectAllBtn);
    
    m_deselectAllBtn = new QPushButton("Deselect All", this);
    buttonLayout->addWidget(m_deselectAllBtn);
    
    m_validateBtn = new QPushButton("Validate Pairs", this);
    buttonLayout->addWidget(m_validateBtn);
    
    buttonLayout->addStretch();
    
    m_startBtn = new QPushButton("Start Import", this);
    m_startBtn->setStyleSheet("background-color: #4caf50;");
    buttonLayout->addWidget(m_startBtn);
    
    m_cancelBtn = new QPushButton("Close", this);
    buttonLayout->addWidget(m_cancelBtn);
    
    mainLayout->addLayout(buttonLayout);
}

void MassImportDialog::setupConnections() {
    connect(m_browsePublicBtn, &QPushButton::clicked, this, &MassImportDialog::onBrowsePublic);
    connect(m_browsePrivateBtn, &QPushButton::clicked, this, &MassImportDialog::onBrowsePrivate);
    connect(m_startBtn, &QPushButton::clicked, this, &MassImportDialog::onStartImport);
    connect(m_cancelBtn, &QPushButton::clicked, this, &MassImportDialog::onCancel);
    connect(m_selectAllBtn, &QPushButton::clicked, this, &MassImportDialog::onSelectAll);
    connect(m_deselectAllBtn, &QPushButton::clicked, this, &MassImportDialog::onDeselectAll);
    connect(m_validateBtn, &QPushButton::clicked, this, &MassImportDialog::onValidatePairs);
    // Added missing connection for item selection changed
    connect(m_fileList, &QListWidget::itemSelectionChanged, this, &MassImportDialog::onItemSelectionChanged);
}

void MassImportDialog::setPublicKeysDir(const QString& path) {
    m_publicDirEdit->setText(path);
    scanDirectories();
}

void MassImportDialog::setPrivateKeysDir(const QString& path) {
    m_privateDirEdit->setText(path);
    scanDirectories();
}

void MassImportDialog::onBrowsePublic() {
    QString path = QFileDialog::getExistingDirectory(this, "Select Public Keys Directory");
    if (!path.isEmpty()) {
        m_publicDirEdit->setText(path);
        scanDirectories();
    }
}

void MassImportDialog::onBrowsePrivate() {
    QString path = QFileDialog::getExistingDirectory(this, "Select Private Keys Directory");
    if (!path.isEmpty()) {
        m_privateDirEdit->setText(path);
        scanDirectories();
    }
}

void MassImportDialog::onStartImport() {
    if (m_importing) {
        m_cancelled = true;
        m_startBtn->setText("Cancelling...");
        return;
    }
    
    m_importing = true;
    m_cancelled = false;
    m_startBtn->setText("Cancel");
    m_startBtn->setStyleSheet("background-color: #f44336;");
    
    m_results.clear();
    
    // Import selected files
    QList<QListWidgetItem*> selected = m_fileList->selectedItems();
    int total = selected.size();
    int current = 0;
    int success = 0;
    
    for (QListWidgetItem* item : selected) {
        if (m_cancelled) break;
        
        QString filePath = item->data(Qt::UserRole).toString();
        ImportResult result = importKeyFile(filePath);
        m_results.append(result);
        
        if (result.success) success++;
        
        current++;
        m_progressBar->setValue((current * 100) / total);
        m_statusLabel->setText(QString("Importing %1/%2: %3")
            .arg(current).arg(total).arg(QFileInfo(filePath).fileName()));
        
        QApplication::processEvents();
    }
    
    m_importing = false;
    m_startBtn->setText("Start Import");
    m_startBtn->setStyleSheet("background-color: #4caf50;");
    
    showResults();
}

void MassImportDialog::onCancel() {
    if (m_importing) {
        m_cancelled = true;
    } else {
        reject();
    }
}

void MassImportDialog::onSelectAll() {
    m_fileList->selectAll();
}

void MassImportDialog::onDeselectAll() {
    m_fileList->clearSelection();
}

// IMPLEMENTED: Missing onItemSelectionChanged() slot
void MassImportDialog::onItemSelectionChanged() {
    int selectedCount = m_fileList->selectedItems().count();
    m_statusLabel->setText(QString("%1 items selected").arg(selectedCount));
}

void MassImportDialog::onValidatePairs() {
    // Validate that public and private keys match
    int matched = 0;
    int unmatched = 0;
    
    for (const QString& pubFile : m_publicFiles) {
        QString baseName = QFileInfo(pubFile).completeBaseName();
        bool foundMatch = false;
        
        for (const QString& privFile : m_privateFiles) {
            if (QFileInfo(privFile).completeBaseName() == baseName) {
                if (validateKeyPair(pubFile, privFile)) {
                    matched++;
                    foundMatch = true;
                }
                break;
            }
        }
        
        if (!foundMatch) unmatched++;
    }
    
    QMessageBox::information(this, "Validation Results",
        QString("Matched pairs: %1\nUnmatched keys: %2").arg(matched).arg(unmatched));
}

void MassImportDialog::scanDirectories() {
    m_publicFiles.clear();
    m_privateFiles.clear();
    m_fileList->clear();
    
    QString publicDir = m_publicDirEdit->text();
    QString privateDir = m_privateDirEdit->text();
    
    if (!publicDir.isEmpty() && QDir(publicDir).exists()) {
        m_publicFiles = findKeyFiles(publicDir);
    }
    
    if (!privateDir.isEmpty() && QDir(privateDir).exists()) {
        m_privateFiles = findKeyFiles(privateDir);
    }
    
    populateFileList();
}

void MassImportDialog::populateFileList() {
    m_fileList->clear();
    
    // Add public keys
    for (const QString& file : m_publicFiles) {
        QFileInfo info(file);
        QListWidgetItem* item = new QListWidgetItem(
            QString("[PUBLIC] %1").arg(info.fileName()));
        item->setData(Qt::UserRole, file);
        item->setForeground(QColor(100, 160, 220));
        m_fileList->addItem(item);
    }
    
    // Add private keys
    for (const QString& file : m_privateFiles) {
        QFileInfo info(file);
        QListWidgetItem* item = new QListWidgetItem(
            QString("[PRIVATE] %1").arg(info.fileName()));
        item->setData(Qt::UserRole, file);
        item->setForeground(QColor(220, 100, 100));
        m_fileList->addItem(item);
    }
    
    m_statusLabel->setText(QString("Found %1 public and %2 private keys")
        .arg(m_publicFiles.size()).arg(m_privateFiles.size()));
}

QList<QString> MassImportDialog::findKeyFiles(const QString& dir) {
    QList<QString> files;
    QDir directory(dir);
    QStringList filters;
    filters << "*.asc" << "*.gpg" << "*.pgp" << "*.pub" << "*.key" << "*.sec";
    
    for (const QString& file : directory.entryList(filters, QDir::Files)) {
        files.append(directory.filePath(file));
    }
    
    return files;
}

bool MassImportDialog::isKeyFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QByteArray header = file.read(100);
    file.close();
    
    return header.contains("-----BEGIN PGP");
}

ImportResult MassImportDialog::importKeyFile(const QString& filePath) {
    ImportResult result;
    result.filePath = filePath;
    
    if (!isKeyFile(filePath)) {
        result.success = false;
        result.error = "Not a valid PGP key file";
        return result;
    }
    
    // Import the key
    if (m_pgpManager->importKeyFromFile(filePath)) {
        result.success = true;
        
        // Determine if public or private
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly)) {
            QByteArray content = file.readAll();
            file.close();
            
            result.isPublic = content.contains("PUBLIC KEY");
            result.isPrivate = content.contains("PRIVATE KEY") || content.contains("SECRET KEY");
        }
        
        // Copy to app directory if requested
        if (m_copyRadio->isChecked()) {
            QString destDir = result.isPublic 
                ? ConfigManager::getInstance().getPublicKeysDir()
                : ConfigManager::getInstance().getPrivateKeysDir();
            
            QString destPath = QDir(destDir).filePath(QFileInfo(filePath).fileName());
            QFile::copy(filePath, destPath);
        }
    } else {
        result.success = false;
        result.error = m_pgpManager->getLastError();
    }
    
    return result;
}

bool MassImportDialog::validateKeyPair(const QString& publicPath, const QString& privatePath) {
    return m_pgpManager->validateKeyPair(publicPath, privatePath);
}

void MassImportDialog::updateProgress() {
    // Progress is updated during import
}

void MassImportDialog::showResults() {
    int success = 0;
    int failed = 0;
    
    for (const auto& result : m_results) {
        if (result.success) success++;
        else failed++;
    }
    
    m_summaryLabel->setText(QString("Import complete: %1 succeeded, %2 failed")
        .arg(success).arg(failed));
    
    if (failed > 0) {
        QString errors;
        for (const auto& result : m_results) {
            if (!result.success) {
                errors += QFileInfo(result.filePath).fileName() + ": " + result.error + "\n";
            }
        }
        
        QMessageBox::warning(this, "Import Results",
            QString("Successfully imported: %1\nFailed: %2\n\nErrors:\n%3")
            .arg(success).arg(failed).arg(errors));
    } else {
        QMessageBox::information(this, "Import Complete",
            QString("Successfully imported %1 keys.").arg(success));
    }
}

} // namespace PCPGP
