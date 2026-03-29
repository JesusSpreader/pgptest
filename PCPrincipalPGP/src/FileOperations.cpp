#include "FileOperations.h"
#include "PGPManager.h"
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QDebug>

namespace PCPGP {

FileOperations::FileOperations(PGPManager* pgpManager, QObject* parent)
    : QObject(parent), m_pgpManager(pgpManager) {
}

FileOperations::~FileOperations() {
}

bool FileOperations::encryptFile(const QString& inputPath, const QString& outputPath,
                                  const QString& recipientKeyId) {
    emit operationStarted("Encrypting " + QFileInfo(inputPath).fileName());
    
    bool result = m_pgpManager->encryptFile(inputPath, outputPath, recipientKeyId);
    
    emit operationCompleted("Encrypt", result);
    if (!result) {
        emit errorOccurred(m_pgpManager->getLastError());
    }
    
    return result;
}

bool FileOperations::encryptFileForMultiple(const QString& inputPath, const QString& outputPath,
                                             const QStringList& recipientKeyIds) {
    emit operationStarted("Encrypting for multiple recipients");
    
    // For now, just encrypt for the first recipient
    // Full implementation would handle multiple recipients
    bool result = false;
    if (!recipientKeyIds.isEmpty()) {
        result = m_pgpManager->encryptFile(inputPath, outputPath, recipientKeyIds.first());
    }
    
    emit operationCompleted("Encrypt", result);
    return result;
}

bool FileOperations::decryptFile(const QString& inputPath, const QString& outputPath,
                                  QString& outKeyId) {
    emit operationStarted("Decrypting " + QFileInfo(inputPath).fileName());
    
    bool result = m_pgpManager->decryptFile(inputPath, outputPath, outKeyId);
    
    emit operationCompleted("Decrypt", result);
    if (!result) {
        emit errorOccurred(m_pgpManager->getLastError());
    }
    
    return result;
}

bool FileOperations::signFile(const QString& inputPath, const QString& outputPath,
                               const QString& keyId, bool detached) {
    emit operationStarted("Signing " + QFileInfo(inputPath).fileName());
    
    bool result = m_pgpManager->signFile(inputPath, outputPath, keyId, detached);
    
    emit operationCompleted("Sign", result);
    if (!result) {
        emit errorOccurred(m_pgpManager->getLastError());
    }
    
    return result;
}

bool FileOperations::verifyFile(const QString& filePath, const QString& signaturePath,
                                 QString& outSignerKeyId) {
    emit operationStarted("Verifying signature");
    
    bool result = m_pgpManager->verifyFile(filePath, signaturePath, outSignerKeyId);
    
    emit operationCompleted("Verify", result);
    if (!result) {
        emit errorOccurred("Signature verification failed");
    }
    
    return result;
}

bool FileOperations::verifyDetachedSignature(const QString& filePath, const QString& sigPath,
                                              QString& outSignerKeyId) {
    return verifyFile(filePath, sigPath, outSignerKeyId);
}

bool FileOperations::encryptDirectory(const QString& dirPath, const QString& outputDir,
                                       const QString& recipientKeyId) {
    QDir dir(dirPath);
    if (!dir.exists()) {
        emit errorOccurred("Directory does not exist");
        return false;
    }
    
    QDir outDir(outputDir);
    if (!outDir.exists()) {
        QDir().mkpath(outputDir);
    }
    
    QStringList files = dir.entryList(QDir::Files);
    int total = files.size();
    int current = 0;
    
    for (const QString& file : files) {
        QString inputFile = dir.filePath(file);
        QString outputFile = outDir.filePath(file + ".gpg");
        
        if (!encryptFile(inputFile, outputFile, recipientKeyId)) {
            return false;
        }
        
        current++;
        emit progressUpdated((current * 100) / total);
    }
    
    return true;
}

bool FileOperations::decryptDirectory(const QString& dirPath, const QString& outputDir) {
    QDir dir(dirPath);
    if (!dir.exists()) {
        emit errorOccurred("Directory does not exist");
        return false;
    }
    
    QDir outDir(outputDir);
    if (!outDir.exists()) {
        QDir().mkpath(outputDir);
    }
    
    QStringList filters;
    filters << "*.gpg" << "*.asc" << "*.pgp";
    QStringList files = dir.entryList(filters, QDir::Files);
    int total = files.size();
    int current = 0;
    
    for (const QString& file : files) {
        QString inputFile = dir.filePath(file);
        QString outputFile = outDir.filePath(file.left(file.lastIndexOf('.')));
        
        QString keyId;
        if (!decryptFile(inputFile, outputFile, keyId)) {
            return false;
        }
        
        current++;
        emit progressUpdated((current * 100) / total);
    }
    
    return true;
}

bool FileOperations::signDirectory(const QString& dirPath, const QString& outputDir,
                                    const QString& keyId) {
    QDir dir(dirPath);
    if (!dir.exists()) {
        emit errorOccurred("Directory does not exist");
        return false;
    }
    
    QDir outDir(outputDir);
    if (!outDir.exists()) {
        QDir().mkpath(outputDir);
    }
    
    QStringList files = dir.entryList(QDir::Files);
    int total = files.size();
    int current = 0;
    
    for (const QString& file : files) {
        QString inputFile = dir.filePath(file);
        QString outputFile = outDir.filePath(file + ".sig");
        
        if (!signFile(inputFile, outputFile, keyId, true)) {
            return false;
        }
        
        current++;
        emit progressUpdated((current * 100) / total);
    }
    
    return true;
}

bool FileOperations::encryptFileFromContext(const QString& filePath, const QString& profileName) {
    QFileInfo info(filePath);
    QString outputPath = info.path() + "/" + info.completeBaseName() + ".gpg";
    
    // Get key ID from profile
    // For now, use the profile name as key ID
    return encryptFile(filePath, outputPath, profileName);
}

bool FileOperations::decryptFileFromContext(const QString& filePath) {
    QFileInfo info(filePath);
    QString outputPath = info.path() + "/" + info.completeBaseName() + "_decrypted";
    
    if (!info.suffix().isEmpty()) {
        outputPath += "." + info.suffix();
    }
    
    QString keyId;
    return decryptFile(filePath, outputPath, keyId);
}

bool FileOperations::signFileFromContext(const QString& filePath, const QString& keyId) {
    QFileInfo info(filePath);
    QString outputPath = filePath + ".sig";
    
    return signFile(filePath, outputPath, keyId, true);
}

bool FileOperations::verifyFileFromContext(const QString& filePath) {
    QString sigPath = filePath + ".sig";
    
    if (!QFile::exists(sigPath)) {
        // Try .asc extension
        sigPath = filePath + ".asc";
        if (!QFile::exists(sigPath)) {
            emit errorOccurred("Signature file not found");
            return false;
        }
    }
    
    QString signerKeyId;
    return verifyFile(filePath, sigPath, signerKeyId);
}

bool FileOperations::handleDroppedFile(const QString& filePath, QString& outAction) {
    QFileInfo info(filePath);
    QString suffix = info.suffix().toLower();
    
    if (suffix == "gpg" || suffix == "asc" || suffix == "pgp") {
        // Encrypted/signed file - decrypt
        outAction = "decrypt";
        return decryptFileFromContext(filePath);
    } else if (suffix == "sig") {
        // Signature file - verify
        outAction = "verify";
        QString dataFile = info.path() + "/" + info.completeBaseName();
        QString signerKeyId;
        return verifyFile(dataFile, filePath, signerKeyId);
    } else if (suffix == "pub" || suffix == "key" || suffix == "asc") {
        // Key file - import
        outAction = "import";
        return m_pgpManager->importKeyFromFile(filePath);
    } else {
        // Regular file - encrypt
        outAction = "encrypt";
        // Need to get recipient from user
        return false;
    }
}

bool FileOperations::handleDroppedFiles(const QStringList& filePaths) {
    bool allSuccess = true;
    
    for (const QString& path : filePaths) {
        QString action;
        if (!handleDroppedFile(path, action)) {
            allSuccess = false;
        }
    }
    
    return allSuccess;
}

bool FileOperations::isPGPEncrypted(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QByteArray header = file.read(50);
    file.close();
    
    return header.contains("-----BEGIN PGP MESSAGE-----") ||
           header.contains("\x85\x01"); // Binary PGP packet header
}

bool FileOperations::isPGPSigned(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QByteArray header = file.read(100);
    file.close();
    
    return header.contains("-----BEGIN PGP SIGNED MESSAGE-----") ||
           header.contains("-----BEGIN PGP SIGNATURE-----");
}

bool FileOperations::isPGPKey(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QByteArray header = file.read(100);
    file.close();
    
    return header.contains("-----BEGIN PGP PUBLIC KEY BLOCK-----") ||
           header.contains("-----BEGIN PGP PRIVATE KEY BLOCK-----") ||
           header.contains("-----BEGIN PGP SECRET KEY BLOCK-----");
}

QString FileOperations::getPGPFileType(const QString& filePath) {
    if (isPGPEncrypted(filePath)) return "encrypted";
    if (isPGPSigned(filePath)) return "signed";
    if (isPGPKey(filePath)) return "key";
    return "unknown";
}

QString FileOperations::suggestOutputName(const QString& inputPath, const QString& operation) {
    QFileInfo info(inputPath);
    QString baseName = info.completeBaseName();
    
    if (operation == "encrypt") {
        return info.path() + "/" + baseName + ".gpg";
    } else if (operation == "decrypt") {
        if (info.suffix() == "gpg" || info.suffix() == "asc" || info.suffix() == "pgp") {
            return info.path() + "/" + baseName;
        }
        return inputPath + ".decrypted";
    } else if (operation == "sign") {
        return inputPath + ".sig";
    } else if (operation == "verify") {
        return inputPath;
    }
    
    return inputPath;
}

bool FileOperations::processFile(const QString& inputPath, const QString& outputPath,
                                  const std::function<bool(const QByteArray&, QByteArray&)>& processor) {
    QByteArray inputData;
    if (!readFile(inputPath, inputData)) {
        return false;
    }
    
    QByteArray outputData;
    if (!processor(inputData, outputData)) {
        return false;
    }
    
    return writeFile(outputPath, outputData);
}

bool FileOperations::readFile(const QString& path, QByteArray& outData) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        emit errorOccurred("Failed to read file: " + path);
        return false;
    }
    
    outData = file.readAll();
    file.close();
    return true;
}

bool FileOperations::writeFile(const QString& path, const QByteArray& data) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        emit errorOccurred("Failed to write file: " + path);
        return false;
    }
    
    file.write(data);
    file.close();
    return true;
}

} // namespace PCPGP
