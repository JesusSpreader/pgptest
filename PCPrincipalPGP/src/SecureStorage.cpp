#include "SecureStorage.h"
#include "ConfigManager.h"
#include <QFile>
#include <QDir>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
#include <QCryptographicHash>

namespace PCPGP {

SecureStorage& SecureStorage::getInstance() {
    static SecureStorage instance;
    return instance;
}

bool SecureStorage::initialize(const QString& password) {
    if (m_initialized) {
        return true;
    }
    
    m_storagePath = QDir(ConfigManager::getInstance().getAppDirectory())
                    .filePath("secure_storage.bin");
    
    if (storageExists()) {
        // Load existing storage
        if (!loadFromFile(m_storagePath)) {
            return false;
        }
        
        // Verify password
        if (!verifyPassword(password)) {
            return false;
        }
        
        m_password = password;
        m_locked = false;
    } else {
        // Create new storage
        m_password = password;
        m_data.clear();
    }
    
    m_initialized = true;
    return true;
}

bool SecureStorage::isInitialized() const {
    return m_initialized;
}

bool SecureStorage::changePassword(const QString& oldPassword, const QString& newPassword) {
    if (!verifyPassword(oldPassword)) {
        return false;
    }
    
    m_password = newPassword;
    return saveStorage();
}

bool SecureStorage::storeData(const QString& key, const QByteArray& data) {
    if (m_locked) {
        return false;
    }
    
    m_data[key] = data;
    return saveStorage();
}

bool SecureStorage::storeString(const QString& key, const QString& value) {
    return storeData(key, value.toUtf8());
}

bool SecureStorage::storeInt(const QString& key, int value) {
    QByteArray data;
    QDataStream stream(&data, QIODevice::WriteOnly);
    stream << value;
    return storeData(key, data);
}

bool SecureStorage::storeBool(const QString& key, bool value) {
    return storeData(key, QByteArray(1, value ? 1 : 0));
}

QByteArray SecureStorage::retrieveData(const QString& key) {
    if (m_locked || !m_data.contains(key)) {
        return QByteArray();
    }
    
    return m_data[key];
}

QString SecureStorage::retrieveString(const QString& key, const QString& defaultValue) {
    QByteArray data = retrieveData(key);
    if (data.isEmpty()) {
        return defaultValue;
    }
    return QString::fromUtf8(data);
}

int SecureStorage::retrieveInt(const QString& key, int defaultValue) {
    QByteArray data = retrieveData(key);
    if (data.isEmpty()) {
        return defaultValue;
    }
    
    QDataStream stream(data);
    int value;
    stream >> value;
    return value;
}

bool SecureStorage::retrieveBool(const QString& key, bool defaultValue) {
    QByteArray data = retrieveData(key);
    if (data.isEmpty()) {
        return defaultValue;
    }
    return data.at(0) != 0;
}

bool SecureStorage::hasKey(const QString& key) {
    return m_data.contains(key);
}

bool SecureStorage::removeKey(const QString& key) {
    if (!m_data.contains(key)) {
        return false;
    }
    
    m_data.remove(key);
    return saveStorage();
}

QList<QString> SecureStorage::listKeys() {
    return m_data.keys();
}

bool SecureStorage::storePGPPublicKey(const QString& keyId, const QByteArray& keyData) {
    QString key = "pgp_public_" + keyId;
    
    if (m_encryptAll) {
        QByteArray encrypted = m_crypto.encrypt(keyData, m_password);
        return storeData(key, encrypted);
    }
    
    return storeData(key, keyData);
}

bool SecureStorage::storePGPPrivateKey(const QString& keyId, const QByteArray& keyData) {
    QString key = "pgp_private_" + keyId;
    
    // Always encrypt private keys
    QByteArray encrypted = m_crypto.encrypt(keyData, m_password);
    return storeData(key, encrypted);
}

QByteArray SecureStorage::retrievePGPPublicKey(const QString& keyId) {
    QString key = "pgp_public_" + keyId;
    QByteArray data = retrieveData(key);
    
    if (data.isEmpty()) {
        return QByteArray();
    }
    
    if (m_encryptAll) {
        return m_crypto.decrypt(data, m_password);
    }
    
    return data;
}

QByteArray SecureStorage::retrievePGPPrivateKey(const QString& keyId) {
    QString key = "pgp_private_" + keyId;
    QByteArray data = retrieveData(key);
    
    if (data.isEmpty()) {
        return QByteArray();
    }
    
    // Private keys are always encrypted
    return m_crypto.decrypt(data, m_password);
}

bool SecureStorage::deletePGPKey(const QString& keyId) {
    bool result = true;
    result &= removeKey("pgp_public_" + keyId);
    result &= removeKey("pgp_private_" + keyId);
    return result;
}

QList<QString> SecureStorage::listStoredKeyIds() {
    QList<QString> keyIds;
    
    for (const QString& key : m_data.keys()) {
        if (key.startsWith("pgp_public_")) {
            keyIds.append(key.mid(11)); // Remove "pgp_public_" prefix
        }
    }
    
    return keyIds;
}

bool SecureStorage::storeProfile(const QString& name, const QByteArray& profileData) {
    QString key = "profile_" + name;
    
    if (m_encryptAll) {
        QByteArray encrypted = m_crypto.encrypt(profileData, m_password);
        return storeData(key, encrypted);
    }
    
    return storeData(key, profileData);
}

QByteArray SecureStorage::retrieveProfile(const QString& name) {
    QString key = "profile_" + name;
    QByteArray data = retrieveData(key);
    
    if (data.isEmpty()) {
        return QByteArray();
    }
    
    if (m_encryptAll) {
        return m_crypto.decrypt(data, m_password);
    }
    
    return data;
}

bool SecureStorage::deleteProfile(const QString& name) {
    return removeKey("profile_" + name);
}

QList<QString> SecureStorage::listProfiles() {
    QList<QString> profiles;
    
    for (const QString& key : m_data.keys()) {
        if (key.startsWith("profile_")) {
            profiles.append(key.mid(8)); // Remove "profile_" prefix
        }
    }
    
    return profiles;
}

bool SecureStorage::saveToFile(const QString& filePath) {
    return saveStorage();
}

bool SecureStorage::loadFromFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QByteArray encryptedData = file.readAll();
    file.close();
    
    // Try to decrypt with empty password (unencrypted storage)
    if (encryptedData.startsWith("{")) {
        // Unencrypted JSON
        return deserializeData(encryptedData);
    }
    
    // Encrypted storage - will need password to decrypt
    m_storagePath = filePath;
    return true;
}

QString SecureStorage::getStoragePath() const {
    return m_storagePath;
}

bool SecureStorage::storageExists() const {
    return QFile::exists(m_storagePath);
}

void SecureStorage::setEncryptAll(bool encryptAll) {
    m_encryptAll = encryptAll;
}

void SecureStorage::setEncryptPrivateOnly(bool privateOnly) {
    m_encryptPrivateOnly = privateOnly;
}

bool SecureStorage::isEncryptAll() const {
    return m_encryptAll;
}

bool SecureStorage::isEncryptPrivateOnly() const {
    return m_encryptPrivateOnly;
}

void SecureStorage::lock() {
    m_locked = true;
    
    // Clear sensitive data from memory
    for (auto it = m_data.begin(); it != m_data.end(); ++it) {
        sodium_memzero(it.value().data(), it.value().size());
    }
    m_data.clear();
    
    sodium_memzero(m_password.data(), m_password.size() * sizeof(QChar));
    m_password.clear();
}

bool SecureStorage::unlock(const QString& password) {
    if (!m_initialized) {
        return false;
    }
    
    if (!verifyPassword(password)) {
        return false;
    }
    
    m_password = password;
    m_locked = false;
    
    // Reload data
    return loadStorage();
}

bool SecureStorage::isLocked() const {
    return m_locked;
}

void SecureStorage::clear() {
    m_data.clear();
    saveStorage();
}

bool SecureStorage::backup(const QString& backupPath) {
    return QFile::copy(m_storagePath, backupPath);
}

bool SecureStorage::restore(const QString& backupPath, const QString& password) {
    if (!QFile::exists(backupPath)) {
        return false;
    }
    
    // Verify password works with backup
    SecureStorage tempStorage;
    tempStorage.m_storagePath = backupPath;
    
    if (!tempStorage.loadFromFile(backupPath)) {
        return false;
    }
    
    if (!tempStorage.verifyPassword(password)) {
        return false;
    }
    
    // Copy backup to current storage
    return QFile::copy(backupPath, m_storagePath);
}

bool SecureStorage::verifyPassword(const QString& password) {
    QFile file(m_storagePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QByteArray encryptedData = file.readAll();
    file.close();
    
    // Check if it's unencrypted JSON
    if (encryptedData.startsWith("{")) {
        return true; // No password needed
    }
    
    // Try to decrypt
    QByteArray decrypted = m_crypto.decrypt(encryptedData, password);
    return !decrypted.isEmpty();
}

bool SecureStorage::loadStorage() {
    QFile file(m_storagePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    // Check if encrypted
    if (data.startsWith("{")) {
        // Unencrypted JSON
        return deserializeData(data);
    }
    
    // Decrypt
    QByteArray decrypted = m_crypto.decrypt(data, m_password);
    if (decrypted.isEmpty()) {
        return false;
    }
    
    return deserializeData(decrypted);
}

bool SecureStorage::saveStorage() {
    QByteArray data = serializeData();
    if (data.isEmpty()) {
        return false;
    }
    
    QByteArray output;
    
    if (m_encryptAll || m_passwordProtected) {
        output = m_crypto.encrypt(data, m_password);
    } else {
        output = data;
    }
    
    QFile file(m_storagePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    file.write(output);
    file.close();
    return true;
}

QByteArray SecureStorage::serializeData() {
    QJsonObject root;
    
    for (auto it = m_data.begin(); it != m_data.end(); ++it) {
        root[it.key()] = QString::fromLatin1(it.value().toBase64());
    }
    
    QJsonDocument doc(root);
    return doc.toJson();
}

bool SecureStorage::deserializeData(const QByteArray& data) {
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        return false;
    }
    
    QJsonObject root = doc.object();
    m_data.clear();
    
    for (auto it = root.begin(); it != root.end(); ++it) {
        m_data[it.key()] = QByteArray::fromBase64(it.value().toString().toLatin1());
    }
    
    return true;
}

// SecureBuffer implementation
SecureBuffer::SecureBuffer(size_t size) : m_size(size) {
    m_data = static_cast<unsigned char*>(sodium_malloc(size));
    if (m_data) {
        memset(m_data, 0, size);
    }
}

SecureBuffer::~SecureBuffer() {
    clear();
    sodium_free(m_data);
}

unsigned char* SecureBuffer::data() {
    return m_data;
}

const unsigned char* SecureBuffer::data() const {
    return m_data;
}

size_t SecureBuffer::size() const {
    return m_size;
}

void SecureBuffer::clear() {
    if (m_data) {
        sodium_memzero(m_data, m_size);
    }
}

QByteArray SecureBuffer::toByteArray() const {
    return QByteArray(reinterpret_cast<const char*>(m_data), static_cast<int>(m_size));
}

} // namespace PCPGP
