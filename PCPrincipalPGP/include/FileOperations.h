#ifndef FILEOPERATIONS_H
#define FILEOPERATIONS_H

#include <QString>
#include <QStringList>
#include <QObject>

namespace PCPGP {

class PGPManager;

class FileOperations : public QObject {
    Q_OBJECT

public:
    explicit FileOperations(PGPManager* pgpManager, QObject* parent = nullptr);
    ~FileOperations();
    
    // File encryption/decryption
    bool encryptFile(const QString& inputPath, const QString& outputPath,
                     const QString& recipientKeyId);
    bool encryptFileForMultiple(const QString& inputPath, const QString& outputPath,
                                const QStringList& recipientKeyIds);
    bool decryptFile(const QString& inputPath, const QString& outputPath,
                     QString& outKeyId);
    
    // File signing
    bool signFile(const QString& inputPath, const QString& outputPath,
                  const QString& keyId, bool detached = false);
    bool verifyFile(const QString& filePath, const QString& signaturePath,
                    QString& outSignerKeyId);
    bool verifyDetachedSignature(const QString& filePath, const QString& sigPath,
                                  QString& outSignerKeyId);
    
    // Batch operations
    bool encryptDirectory(const QString& dirPath, const QString& outputDir,
                          const QString& recipientKeyId);
    bool decryptDirectory(const QString& dirPath, const QString& outputDir);
    bool signDirectory(const QString& dirPath, const QString& outputDir,
                       const QString& keyId);
    
    // Context menu operations
    bool encryptFileFromContext(const QString& filePath, const QString& profileName);
    bool decryptFileFromContext(const QString& filePath);
    bool signFileFromContext(const QString& filePath, const QString& keyId);
    bool verifyFileFromContext(const QString& filePath);
    
    // Drag and drop
    bool handleDroppedFile(const QString& filePath, QString& outAction);
    bool handleDroppedFiles(const QStringList& filePaths);
    
    // File utilities
    static bool isPGPEncrypted(const QString& filePath);
    static bool isPGPSigned(const QString& filePath);
    static bool isPGPKey(const QString& filePath);
    static QString getPGPFileType(const QString& filePath);
    static QString suggestOutputName(const QString& inputPath, const QString& operation);

signals:
    void progressUpdated(int percent);
    void operationStarted(const QString& operation);
    void operationCompleted(const QString& operation, bool success);
    void errorOccurred(const QString& error);

private:
    bool processFile(const QString& inputPath, const QString& outputPath,
                     const std::function<bool(const QByteArray&, QByteArray&)>& processor);
    bool readFile(const QString& path, QByteArray& outData);
    bool writeFile(const QString& path, const QByteArray& data);
    
    PGPManager* m_pgpManager;
};

} // namespace PCPGP

#endif // FILEOPERATIONS_H
