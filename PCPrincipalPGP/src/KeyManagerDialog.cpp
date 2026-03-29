#include "KeyManagerDialog.h"
#include "GenerateKeyDialog.h"
#include "MassImportDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QTabWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QClipboard>
#include <QApplication>
#include <QMenu>
#include <QInputDialog>

namespace PCPGP {

KeyManagerDialog::KeyManagerDialog(PGPManager* pgpManager, QWidget* parent)
    : QDialog(parent), m_pgpManager(pgpManager) {
    
    setWindowTitle("Key Manager");
    setMinimumSize(900, 600);
    resize(1000, 700);
    
    setupUI();
    setupConnections();
    refreshKeys();
}

KeyManagerDialog::~KeyManagerDialog() {
}

void KeyManagerDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    
    // Search bar
    QHBoxLayout* searchLayout = new QHBoxLayout();
    QLabel* searchLabel = new QLabel("Search:", this);
    searchLayout->addWidget(searchLabel);
    
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("Search by name, email, or key ID...");
    searchLayout->addWidget(m_searchEdit, 1);
    
    mainLayout->addLayout(searchLayout);
    
    // Tab widget for public/private keys
    m_tabWidget = new QTabWidget(this);
    
    // Public keys table
    m_publicTable = new QTableWidget(this);
    m_publicTable->setColumnCount(6);
    m_publicTable->setHorizontalHeaderLabels(
        QStringList() << "Name" << "Email" << "Key ID" << "Algorithm" << "Created" << "Status");
    m_publicTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_publicTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_publicTable->setAlternatingRowColors(true);
    m_publicTable->setContextMenuPolicy(Qt::CustomContextMenu);
    m_publicTable->horizontalHeader()->setStretchLastSection(true);
    m_publicTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    
    m_tabWidget->addTab(m_publicTable, "Public Keys");
    
    // Private keys table
    m_privateTable = new QTableWidget(this);
    m_privateTable->setColumnCount(6);
    m_privateTable->setHorizontalHeaderLabels(
        QStringList() << "Name" << "Email" << "Key ID" << "Algorithm" << "Created" << "Status");
    m_privateTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_privateTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_privateTable->setAlternatingRowColors(true);
    m_privateTable->setContextMenuPolicy(Qt::CustomContextMenu);
    m_privateTable->horizontalHeader()->setStretchLastSection(true);
    m_privateTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    
    m_tabWidget->addTab(m_privateTable, "Private Keys");
    
    mainLayout->addWidget(m_tabWidget, 1);
    
    // Button row
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    m_generateBtn = new QPushButton("Generate New Key", this);
    buttonLayout->addWidget(m_generateBtn);
    
    m_importBtn = new QPushButton("Import Key", this);
    buttonLayout->addWidget(m_importBtn);
    
    m_massImportBtn = new QPushButton("Mass Import", this);
    buttonLayout->addWidget(m_massImportBtn);
    
    buttonLayout->addSpacing(20);
    
    m_exportPublicBtn = new QPushButton("Export Public", this);
    m_exportPublicBtn->setEnabled(false);
    buttonLayout->addWidget(m_exportPublicBtn);
    
    m_exportPrivateBtn = new QPushButton("Export Private", this);
    m_exportPrivateBtn->setEnabled(false);
    buttonLayout->addWidget(m_exportPrivateBtn);
    
    buttonLayout->addSpacing(20);
    
    m_deleteBtn = new QPushButton("Delete", this);
    m_deleteBtn->setEnabled(false);
    buttonLayout->addWidget(m_deleteBtn);
    
    m_detailsBtn = new QPushButton("Details", this);
    m_detailsBtn->setEnabled(false);
    buttonLayout->addWidget(m_detailsBtn);
    
    m_refreshBtn = new QPushButton("Refresh", this);
    buttonLayout->addWidget(m_refreshBtn);
    
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
    
    // Info label
    m_infoLabel = new QLabel(this);
    m_infoLabel->setStyleSheet("color: #888;");
    mainLayout->addWidget(m_infoLabel);
}

void KeyManagerDialog::setupConnections() {
    connect(m_searchEdit, &QLineEdit::textChanged, this, &KeyManagerDialog::onSearchTextChanged);
    
    connect(m_generateBtn, &QPushButton::clicked, this, &KeyManagerDialog::onGenerateKey);
    connect(m_importBtn, &QPushButton::clicked, this, &KeyManagerDialog::onImportKey);
    connect(m_massImportBtn, &QPushButton::clicked, this, &KeyManagerDialog::onMassImport);
    connect(m_exportPublicBtn, &QPushButton::clicked, this, &KeyManagerDialog::onExportPublic);
    connect(m_exportPrivateBtn, &QPushButton::clicked, this, &KeyManagerDialog::onExportPrivate);
    connect(m_deleteBtn, &QPushButton::clicked, this, &KeyManagerDialog::onDeleteKey);
    connect(m_detailsBtn, &QPushButton::clicked, this, &KeyManagerDialog::onViewDetails);
    connect(m_refreshBtn, &QPushButton::clicked, this, &KeyManagerDialog::onRefresh);
    
    connect(m_publicTable, &QTableWidget::itemSelectionChanged, 
            this, &KeyManagerDialog::onKeySelectionChanged);
    connect(m_privateTable, &QTableWidget::itemSelectionChanged,
            this, &KeyManagerDialog::onKeySelectionChanged);
    
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &KeyManagerDialog::onTabChanged);
    
    // Context menus
    connect(m_publicTable, &QTableWidget::customContextMenuRequested,
            this, [this](const QPoint& pos) {
        QMenu menu(this);
        menu.addAction("Copy Key ID", this, [this]() {
            auto key = getSelectedKey();
            if (!key.keyId.isEmpty()) {
                QApplication::clipboard()->setText(key.keyId);
            }
        });
        menu.addAction("Copy Fingerprint", this, [this]() {
            auto key = getSelectedKey();
            if (!key.fingerprint.isEmpty()) {
                QApplication::clipboard()->setText(key.fingerprint);
            }
        });
        menu.addAction("Export to Clipboard", this, [this]() {
            onExportPublic();
        });
        menu.exec(m_publicTable->viewport()->mapToGlobal(pos));
    });
}

void KeyManagerDialog::refreshKeys() {
    m_publicKeys = m_pgpManager->listPublicKeys();
    m_privateKeys = m_pgpManager->listSecretKeys();
    
    populatePublicKeys();
    populatePrivateKeys();
    
    m_infoLabel->setText(QString("Total: %1 public keys, %2 private keys")
                         .arg(m_publicKeys.size()).arg(m_privateKeys.size()));
}

void KeyManagerDialog::populatePublicKeys() {
    m_publicTable->setRowCount(0);
    
    for (const auto& key : m_publicKeys) {
        int row = m_publicTable->rowCount();
        m_publicTable->insertRow(row);
        
        m_publicTable->setItem(row, 0, new QTableWidgetItem(key.userId));
        m_publicTable->setItem(row, 1, new QTableWidgetItem(key.email));
        m_publicTable->setItem(row, 2, new QTableWidgetItem(key.keyId.right(8)));
        m_publicTable->setItem(row, 3, new QTableWidgetItem(key.algorithm + " " + QString::number(key.keyLength)));
        m_publicTable->setItem(row, 4, new QTableWidgetItem(key.creationDate.toString("yyyy-MM-dd")));
        
        QString status;
        if (key.isRevoked) status = "Revoked";
        else if (key.isExpired) status = "Expired";
        else if (key.isDisabled) status = "Disabled";
        else status = "Valid";
        
        QTableWidgetItem* statusItem = new QTableWidgetItem(status);
        if (status == "Valid") {
            statusItem->setForeground(QColor(76, 175, 80));
        } else {
            statusItem->setForeground(QColor(244, 67, 54));
        }
        m_publicTable->setItem(row, 5, statusItem);
        
        // Store full key ID as data
        m_publicTable->item(row, 0)->setData(Qt::UserRole, key.keyId);
    }
}

void KeyManagerDialog::populatePrivateKeys() {
    m_privateTable->setRowCount(0);
    
    for (const auto& key : m_privateKeys) {
        int row = m_privateTable->rowCount();
        m_privateTable->insertRow(row);
        
        m_privateTable->setItem(row, 0, new QTableWidgetItem(key.userId));
        m_privateTable->setItem(row, 1, new QTableWidgetItem(key.email));
        m_privateTable->setItem(row, 2, new QTableWidgetItem(key.keyId.right(8)));
        m_privateTable->setItem(row, 3, new QTableWidgetItem(key.algorithm + " " + QString::number(key.keyLength)));
        m_privateTable->setItem(row, 4, new QTableWidgetItem(key.creationDate.toString("yyyy-MM-dd")));
        
        QString status;
        if (key.isRevoked) status = "Revoked";
        else if (key.isExpired) status = "Expired";
        else if (key.isDisabled) status = "Disabled";
        else status = "Valid";
        
        QTableWidgetItem* statusItem = new QTableWidgetItem(status);
        if (status == "Valid") {
            statusItem->setForeground(QColor(76, 175, 80));
        } else {
            statusItem->setForeground(QColor(244, 67, 54));
        }
        m_privateTable->setItem(row, 5, statusItem);
        
        // Store full key ID as data
        m_privateTable->item(row, 0)->setData(Qt::UserRole, key.keyId);
    }
}

void KeyManagerDialog::updateButtonStates() {
    bool hasSelection = !getSelectedKey().keyId.isEmpty();
    
    m_exportPublicBtn->setEnabled(hasSelection);
    m_exportPrivateBtn->setEnabled(hasSelection && m_tabWidget->currentIndex() == 1);
    m_deleteBtn->setEnabled(hasSelection);
    m_detailsBtn->setEnabled(hasSelection);
}

PGPKey KeyManagerDialog::getSelectedKey() {
    QTableWidget* table = (m_tabWidget->currentIndex() == 0) ? m_publicTable : m_privateTable;
    
    QList<QTableWidgetItem*> selected = table->selectedItems();
    if (selected.isEmpty()) {
        return PGPKey();
    }
    
    int row = selected.first()->row();
    QString keyId = table->item(row, 0)->data(Qt::UserRole).toString();
    
    // Find key in list
    const auto& keys = (m_tabWidget->currentIndex() == 0) ? m_publicKeys : m_privateKeys;
    for (const auto& key : keys) {
        if (key.keyId == keyId) {
            return key;
        }
    }
    
    return PGPKey();
}

void KeyManagerDialog::showKeyDetails(const PGPKey& key) {
    QString details = QString(
        "<h3>Key Details</h3>"
        "<table>"
        "<tr><td><b>Name:</b></td><td>%1</td></tr>"
        "<tr><td><b>Email:</b></td><td>%2</td></tr>"
        "<tr><td><b>Key ID:</b></td><td>%3</td></tr>"
        "<tr><td><b>Fingerprint:</b></td><td>%4</td></tr>"
        "<tr><td><b>Algorithm:</b></td><td>%5</td></tr>"
        "<tr><td><b>Key Length:</b></td><td>%6 bits</td></tr>"
        "<tr><td><b>Created:</b></td><td>%7</td></tr>"
        "<tr><td><b>Expires:</b></td><td>%8</td></tr>"
        "<tr><td><b>Status:</b></td><td>%9</td></tr>"
        "<tr><td><b>Capabilities:</b></td><td>%10</td></tr>"
        "</table>"
    ).arg(key.userId)
     .arg(key.email)
     .arg(key.keyId)
     .arg(key.fingerprint)
     .arg(key.algorithm)
     .arg(key.keyLength)
     .arg(key.creationDate.toString("yyyy-MM-dd hh:mm:ss"))
     .arg(key.expirationDate.isValid() ? key.expirationDate.toString("yyyy-MM-dd") : "Never")
     .arg(key.isRevoked ? "Revoked" : key.isExpired ? "Expired" : key.isDisabled ? "Disabled" : "Valid")
     .arg(QString("%1%2%3")
          .arg(key.canEncrypt ? "Encrypt " : "")
          .arg(key.canSign ? "Sign " : "")
          .arg(key.canCertify ? "Certify" : ""));
    
    QMessageBox::information(this, "Key Details", details);
}

void KeyManagerDialog::onGenerateKey() {
    GenerateKeyDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        if (m_pgpManager->generateKeyPair(
                dialog.getName(),
                dialog.getEmail(),
                dialog.getPassphrase(),
                dialog.getKeyLength(),
                dialog.isExpirationEnabled() ? dialog.getExpirationDays() : 0)) {
            QMessageBox::information(this, "Success", "Key pair generated successfully!");
            refreshKeys();
        } else {
            QMessageBox::critical(this, "Error", 
                "Failed to generate key: " + m_pgpManager->getLastError());
        }
    }
}

void KeyManagerDialog::onImportKey() {
    QString filePath = QFileDialog::getOpenFileName(this, "Import Key", QString(),
        "PGP Keys (*.asc *.gpg *.pgp *.pub *.key);;All Files (*)");
    
    if (!filePath.isEmpty()) {
        if (m_pgpManager->importKeyFromFile(filePath)) {
            QMessageBox::information(this, "Success", "Key imported successfully!");
            refreshKeys();
        } else {
            QMessageBox::critical(this, "Error", 
                "Failed to import key: " + m_pgpManager->getLastError());
        }
    }
}

void KeyManagerDialog::onExportPublic() {
    PGPKey key = getSelectedKey();
    if (key.keyId.isEmpty()) return;
    
    QString filePath = QFileDialog::getSaveFileName(this, "Export Public Key",
        key.userId + "_public.asc", "ASCII Armored (*.asc);;PGP Files (*.gpg *.pgp)");
    
    if (!filePath.isEmpty()) {
        if (m_pgpManager->exportPublicKeyToFile(key.keyId, filePath)) {
            QMessageBox::information(this, "Success", "Public key exported successfully!");
        } else {
            QMessageBox::critical(this, "Error", 
                "Failed to export key: " + m_pgpManager->getLastError());
        }
    }
}

void KeyManagerDialog::onExportPrivate() {
    PGPKey key = getSelectedKey();
    if (key.keyId.isEmpty()) return;
    
    // Warning about private key export
    auto reply = QMessageBox::warning(this, "Warning",
        "You are about to export a PRIVATE key.\n\n"
        "Keep this file secure and never share it with anyone!\n\n"
        "Do you want to continue?",
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply != QMessageBox::Yes) return;
    
    QString filePath = QFileDialog::getSaveFileName(this, "Export Private Key",
        key.userId + "_private.asc", "ASCII Armored (*.asc);;PGP Files (*.gpg *.pgp)");
    
    if (!filePath.isEmpty()) {
        if (m_pgpManager->exportPrivateKeyToFile(key.keyId, filePath)) {
            QMessageBox::information(this, "Success", 
                "Private key exported successfully!\n\n"
                "Keep this file in a secure location!");
        } else {
            QMessageBox::critical(this, "Error", 
                "Failed to export key: " + m_pgpManager->getLastError());
        }
    }
}

void KeyManagerDialog::onDeleteKey() {
    PGPKey key = getSelectedKey();
    if (key.keyId.isEmpty()) return;
    
    bool isPrivate = m_tabWidget->currentIndex() == 1;
    
    QString message = isPrivate 
        ? QString("Are you sure you want to delete the private key for '%1'?\n\n"
                  "This action cannot be undone!")
                  .arg(key.userId)
        : QString("Are you sure you want to delete the public key for '%1'?")
                  .arg(key.userId);
    
    auto reply = QMessageBox::warning(this, "Confirm Delete", message,
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        if (m_pgpManager->deleteKey(key.keyId, isPrivate)) {
            QMessageBox::information(this, "Success", "Key deleted successfully!");
            refreshKeys();
        } else {
            QMessageBox::critical(this, "Error", 
                "Failed to delete key: " + m_pgpManager->getLastError());
        }
    }
}

void KeyManagerDialog::onViewDetails() {
    PGPKey key = getSelectedKey();
    if (!key.keyId.isEmpty()) {
        showKeyDetails(key);
    }
}

void KeyManagerDialog::onSetTrust() {
    // Open trust dialog
}

void KeyManagerDialog::onChangePassphrase() {
    // Open passphrase change dialog
}

void KeyManagerDialog::onSearchTextChanged(const QString& text) {
    // Filter tables based on search text
    for (int i = 0; i < m_publicTable->rowCount(); i++) {
        bool match = false;
        for (int j = 0; j < m_publicTable->columnCount(); j++) {
            QTableWidgetItem* item = m_publicTable->item(i, j);
            if (item && item->text().contains(text, Qt::CaseInsensitive)) {
                match = true;
                break;
            }
        }
        m_publicTable->setRowHidden(i, !match);
    }
    
    for (int i = 0; i < m_privateTable->rowCount(); i++) {
        bool match = false;
        for (int j = 0; j < m_privateTable->columnCount(); j++) {
            QTableWidgetItem* item = m_privateTable->item(i, j);
            if (item && item->text().contains(text, Qt::CaseInsensitive)) {
                match = true;
                break;
            }
        }
        m_privateTable->setRowHidden(i, !match);
    }
}

void KeyManagerDialog::onKeySelectionChanged() {
    updateButtonStates();
}

void KeyManagerDialog::onRefresh() {
    refreshKeys();
}

void KeyManagerDialog::onMassImport() {
    MassImportDialog dialog(m_pgpManager, this);
    dialog.exec();
    refreshKeys();
}

void KeyManagerDialog::onTabChanged(int index) {
    updateButtonStates();
}

} // namespace PCPGP
