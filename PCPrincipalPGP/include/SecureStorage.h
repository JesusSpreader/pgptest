#ifndef SECURESTORAGE_H
#define SECURESTORAGE_H

#include <QString>
#include <QByteArray>
#include <QMap>
#include <memory>
#include "QuantumCrypto.h"

namespace PCPGP {

// Secure encrypted storage for application data
class SecureStorage {
public:
    static SecureStorage& getInstance();
    
    // Initialize with password
    bool initialize(const QString& password);
    bool isInitialized() const;
    bool changePassword(const QString& oldPassword, const QString& newPassword);
    
    // Data storage
    bool storeData(const QString& key, const QByteArray& data);
    bool storeString(const QString& key, const QString& value);
    bool storeInt(const QString& key, int value);
    bool storeBool(const QString& key, bool value);
    
    // Data retrieval
    QByteArray retrieveData(const QString& key);
    QString retrieveString(const QString& key, const QString& defaultValue = QString());
    int retrieveInt(const QString& key, int defaultValue = 0);
    bool retrieveBool(const QString& key, bool defaultValue = false);
    
    // Key operations
    bool hasKey(const QString& key);
    bool removeKey(const QString& key);
    QList<QString> listKeys();
    
    // PGP Key storage (encrypted)
    bool storePGPPublicKey(const QString& keyId, const QByteArray& keyData);
    bool storePGPPrivateKey(const QString& keyId, const QByteArray& keyData);
    QByteArray retrievePGPPublicKey(const QString& keyId);
    QByteArray retrievePGPPrivateKey(const QString& keyId);
    bool deletePGPKey(const QString& keyId);
    QList<QString> listStoredKeyIds();
    
    // Profile storage
    bool storeProfile(const QString& name, const QByteArray& profileData);
    QByteArray retrieveProfile(const QString& name);
    bool deleteProfile(const QString& name);
    QList<QString> listProfiles();
    
    // File operations
    bool saveToFile(const QString& filePath);
    bool loadFromFile(const QString& filePath);
    
    // Master storage file
    QString getStoragePath() const;
    bool storageExists() const;
    
    // Encryption modes
    void setEncryptAll(bool encryptAll);
    void setEncryptPrivateOnly(bool privateOnly);
    bool isEncryptAll() const;
    bool isEncryptPrivateOnly() const;
    
    // Lock/Unlock
    void lock();
    bool unlock(const QString& password);
    bool isLocked() const;
    
    // Clear all data
    void clear();
    
    // Backup/Restore
    bool backup(const QString& backupPath);
    bool restore(const QString& backupPath, const QString& password);
    
    // Verify password
    bool verifyPassword(const QString& password);

private:
    SecureStorage() = default;
    ~SecureStorage() = default;
    SecureStorage(const SecureStorage&) = delete;
    SecureStorage& operator=(const SecureStorage&) = delete;
    
    bool loadStorage();
    bool saveStorage();
    QByteArray serializeData();
    bool deserializeData(const QByteArray& data);
    
    QuantumCrypto m_crypto;
    QMap<QString, QByteArray> m_data;
    QString m_password;
    QString m_storagePath;
    bool m_initialized = false;
    bool m_locked = true;
    bool m_encryptAll = false;
    bool m_encryptPrivateOnly = true;
};

// Secure memory clearing
class SecureBuffer {
public:
    SecureBuffer(size_t size);
    ~SecureBuffer();
    
    unsigned char* data();
    const unsigned char* data() const;
    size_t size() const;
    
    void clear();
    QByteArray toByteArray() const;
    
private:
    unsigned char* m_data;
    size_t m_size;
};

} // namespace PCPGP

#endif // SECURESTORAGE_H
