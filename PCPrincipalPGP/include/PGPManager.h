#ifndef PGPMANAGER_H
#define PGPMANAGER_H

#include <QString>
#include <QList>
#include <QByteArray>
#include <QDateTime>
#include <memory>

// GPGME headers
#include <gpgme.h>

namespace PCPGP {

struct PGPKey {
    QString keyId;
    QString fingerprint;
    QString userId;
    QString email;
    QDateTime creationDate;
    QDateTime expirationDate;
    bool isPrivate;
    bool isPublic;
    int keyLength;
    QString algorithm;
    bool canEncrypt;
    bool canSign;
    bool canCertify;
    bool isRevoked;
    bool isExpired;
    bool isDisabled;
    QString subkeys;
};

struct PGPProfile {
    QString name;
    QString email;
    QString keyId;
    QString fingerprint;
    QString publicKeyPath;
    QString privateKeyPath;
    QDateTime created;
    QString notes;
};

class PGPManager {
public:
    PGPManager();
    ~PGPManager();
    
    // Initialization
    bool initialize();
    bool isInitialized() const;
    QString getLastError() const;
    
    // Key generation
    bool generateKeyPair(const QString& name, const QString& email, 
                         const QString& passphrase, int keyLength = 4096,
                         int expireDays = 0);
    
    // Key import/export
    bool importKey(const QString& keyData);
    bool importKeyFromFile(const QString& filePath);
    bool importPublicKey(const QByteArray& keyData);
    bool importPrivateKey(const QByteArray& keyData);
    
    QByteArray exportPublicKey(const QString& keyId);
    QByteArray exportPrivateKey(const QString& keyId);
    bool exportPublicKeyToFile(const QString& keyId, const QString& filePath);
    bool exportPrivateKeyToFile(const QString& keyId, const QString& filePath);
    
    // Key listing
    QList<PGPKey> listKeys(bool includeSecret = false);
    QList<PGPKey> listPublicKeys();
    QList<PGPKey> listSecretKeys();
    PGPKey getKeyById(const QString& keyId);
    PGPKey getKeyByFingerprint(const QString& fingerprint);
    PGPKey getKeyByEmail(const QString& email);
    
    // Key operations
    bool deleteKey(const QString& keyId, bool deleteSecret = false);
    bool revokeKey(const QString& keyId, const QString& reason);
    bool changePassphrase(const QString& keyId, const QString& oldPass, const QString& newPass);
    bool setKeyTrust(const QString& keyId, int trustLevel);
    
    // Encryption/Decryption
    QByteArray encrypt(const QByteArray& data, const QString& recipientKeyId);
    QByteArray encryptForMultipleRecipients(const QByteArray& data, const QStringList& recipientKeyIds);
    QByteArray decrypt(const QByteArray& data, QString& outKeyId);
    
    bool encryptFile(const QString& inputPath, const QString& outputPath, 
                     const QString& recipientKeyId);
    bool decryptFile(const QString& inputPath, const QString& outputPath,
                     QString& outKeyId);
    
    // Signing
    QByteArray sign(const QByteArray& data, const QString& keyId, 
                    bool detached = false, const QString& passphrase = QString());
    bool verify(const QByteArray& data, const QByteArray& signature,
                QString& outSignerKeyId);
    bool signFile(const QString& inputPath, const QString& outputPath,
                  const QString& keyId, bool detached = false);
    bool verifyFile(const QString& filePath, const QString& signaturePath,
                    QString& outSignerKeyId);
    
    // Cleartext signing (for messages)
    QByteArray cleartextSign(const QByteArray& data, const QString& keyId,
                             const QString& passphrase = QString());
    bool verifyCleartext(const QByteArray& signedData, QString& outSignerKeyId);
    
    // Mass import
    int importKeysFromDirectory(const QString& dirPath, bool importPrivate);
    
    // Key validation
    bool validateKeyPair(const QString& publicKeyPath, const QString& privateKeyPath);
    bool keysMatch(const PGPKey& publicKey, const PGPKey& privateKey);
    
    // Profiles
    bool saveProfile(const PGPProfile& profile);
    bool loadProfile(const QString& name, PGPProfile& outProfile);
    QList<PGPProfile> listProfiles();
    bool deleteProfile(const QString& name);
    
    // Context menu operations
    bool encryptFileForProfile(const QString& filePath, const QString& profileName);
    bool decryptFileToPath(const QString& filePath, const QString& outputPath);
    QByteArray getPublicKeyArmored(const QString& keyId);
    
    // Refresh key list from GPG
    void refreshKeyCache();

private:
    gpgme_ctx_t m_ctx;
    bool m_initialized;
    QString m_lastError;
    QList<PGPKey> m_keyCache;
    
    bool checkError(gpgme_error_t err);
    PGPKey parseKey(gpgme_key_t key);
    QByteArray readGPGData(gpgme_data_t data);
    bool writeGPGData(gpgme_data_t data, const QByteArray& content);
    void updateKeyCache();
};

} // namespace PCPGP

#endif // PGPMANAGER_H
