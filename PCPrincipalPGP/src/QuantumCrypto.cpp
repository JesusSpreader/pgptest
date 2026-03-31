#include "QuantumCrypto.h"
#include <QDebug>
#include <QFile>
#include <QCryptographicHash>
#include <cstring>

namespace PCPGP {

bool QuantumCrypto::s_initialized = false;

QuantumCrypto::QuantumCrypto() {
}

QuantumCrypto::~QuantumCrypto() {
}

bool QuantumCrypto::initialize() {
    if (s_initialized) {
        return true;
    }
    
    if (sodium_init() < 0) {
        qCritical() << "Failed to initialize libsodium";
        return false;
    }
    
    s_initialized = true;
    return true;
}

bool QuantumCrypto::isInitialized() {
    return s_initialized;
}

QByteArray QuantumCrypto::deriveKey(const QString& password, const QByteArray& salt) {
    if (!s_initialized) {
        return QByteArray();
    }
    
    QByteArray key(KEY_SIZE, 0);
    QByteArray passwordBytes = password.toUtf8();
    
    // Use Argon2id for key derivation
    if (crypto_pwhash(
            reinterpret_cast<unsigned char*>(key.data()),
            KEY_SIZE,
            passwordBytes.constData(),
            passwordBytes.size(),
            reinterpret_cast<const unsigned char*>(salt.constData()),
            crypto_pwhash_OPSLIMIT_MODERATE,
            crypto_pwhash_MEMLIMIT_MODERATE,
            crypto_pwhash_ALG_ARGON2ID13) != 0) {
        qCritical() << "Key derivation failed";
        return QByteArray();
    }
    
    return key;
}

QByteArray QuantumCrypto::encrypt(const QByteArray& data, const QString& password) {
    if (!s_initialized || data.isEmpty()) {
        return QByteArray();
    }
    
    // Generate random salt and nonce
    QByteArray salt = generateSalt();
    QByteArray nonce(NONCE_SIZE, 0);
    randombytes_buf(nonce.data(), NONCE_SIZE);
    
    // Derive key from password
    QByteArray key = deriveKey(password, salt);
    if (key.isEmpty()) {
        return QByteArray();
    }
    
    // Prepare output buffer (ciphertext + tag)
    QByteArray ciphertext(data.size() + TAG_SIZE, 0);
    unsigned long long ciphertextLen;
    
    // Encrypt using XChaCha20-Poly1305
    if (crypto_aead_xchacha20poly1305_ietf_encrypt(
            reinterpret_cast<unsigned char*>(ciphertext.data()),
            &ciphertextLen,
            reinterpret_cast<const unsigned char*>(data.constData()),
            data.size(),
            nullptr,  // additional data
            0,        // additional data length
            nullptr,  // secret nonce (unused)
            reinterpret_cast<const unsigned char*>(nonce.constData()),
            reinterpret_cast<const unsigned char*>(key.constData())) != 0) {
        qCritical() << "Encryption failed";
        return QByteArray();
    }
    
    // Build output: [salt(32)][nonce(24)][ciphertext_len(8)][ciphertext+tag]
    QByteArray result;
    result.append(salt);
    result.append(nonce);
    
    // Append ciphertext length as 8 bytes (uint64_t)
    quint64 len = static_cast<quint64>(ciphertextLen);
    result.append(reinterpret_cast<const char*>(&len), sizeof(len));
    
    result.append(ciphertext);
    
    // Clear sensitive data
    sodium_memzero(key.data(), key.size());
    
    return result;
}

QByteArray QuantumCrypto::decrypt(const QByteArray& data, const QString& password) {
    // Fix sign-compare warning by casting size_t to qsizetype for comparison
    const qsizetype minSize = static_cast<qsizetype>(SALT_SIZE + NONCE_SIZE + sizeof(quint64) + TAG_SIZE);
    if (!s_initialized || data.size() < minSize) {
        return QByteArray();
    }
    
    // Extract components
    qsizetype pos = 0;
    QByteArray salt = data.mid(pos, static_cast<qsizetype>(SALT_SIZE));
    pos += static_cast<qsizetype>(SALT_SIZE);
    
    QByteArray nonce = data.mid(pos, static_cast<qsizetype>(NONCE_SIZE));
    pos += static_cast<qsizetype>(NONCE_SIZE);
    
    quint64 ciphertextLen;
    memcpy(&ciphertextLen, data.constData() + pos, sizeof(ciphertextLen));
    pos += sizeof(ciphertextLen);
    
    QByteArray ciphertext = data.mid(pos);
    
    if (static_cast<quint64>(ciphertext.size()) != ciphertextLen) {
        qWarning() << "Ciphertext length mismatch";
        return QByteArray();
    }
    
    // Derive key
    QByteArray key = deriveKey(password, salt);
    if (key.isEmpty()) {
        return QByteArray();
    }
    
    // Decrypt
    QByteArray plaintext(ciphertext.size() - static_cast<qsizetype>(TAG_SIZE), 0);
    unsigned long long plaintextLen;
    
    if (crypto_aead_xchacha20poly1305_ietf_decrypt(
            reinterpret_cast<unsigned char*>(plaintext.data()),
            &plaintextLen,
            nullptr,  // secret nonce (unused)
            reinterpret_cast<const unsigned char*>(ciphertext.constData()),
            ciphertext.size(),
            nullptr,  // additional data
            0,        // additional data length
            reinterpret_cast<const unsigned char*>(nonce.constData()),
            reinterpret_cast<const unsigned char*>(key.constData())) != 0) {
        // Decryption failed - likely wrong password
        sodium_memzero(key.data(), key.size());
        return QByteArray();
    }
    
    // Resize to actual plaintext length
    plaintext.resize(static_cast<qsizetype>(plaintextLen));
    
    // Clear sensitive data
    sodium_memzero(key.data(), key.size());
    
    return plaintext;
}

bool QuantumCrypto::encryptFile(const QString& inputPath, const QString& outputPath, 
                                const QString& password) {
    QFile inputFile(inputPath);
    if (!inputFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open input file:" << inputPath;
        return false;
    }
    
    QByteArray data = inputFile.readAll();
    inputFile.close();
    
    QByteArray encrypted = encrypt(data, password);
    if (encrypted.isEmpty()) {
        return false;
    }
    
    QFile outputFile(outputPath);
    if (!outputFile.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open output file:" << outputPath;
        return false;
    }
    
    outputFile.write(encrypted);
    outputFile.close();
    
    return true;
}

bool QuantumCrypto::decryptFile(const QString& inputPath, const QString& outputPath,
                                const QString& password) {
    QFile inputFile(inputPath);
    if (!inputFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open input file:" << inputPath;
        return false;
    }
    
    QByteArray data = inputFile.readAll();
    inputFile.close();
    
    QByteArray decrypted = decrypt(data, password);
    if (decrypted.isEmpty()) {
        return false;
    }
    
    QFile outputFile(outputPath);
    if (!outputFile.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open output file:" << outputPath;
        return false;
    }
    
    outputFile.write(decrypted);
    outputFile.close();
    
    return true;
}

QByteArray QuantumCrypto::encryptKey(const QByteArray& keyData, const QString& password) {
    return encrypt(keyData, password);
}

QByteArray QuantumCrypto::decryptKey(const QByteArray& encryptedData, const QString& password) {
    return decrypt(encryptedData, password);
}

QByteArray QuantumCrypto::generateSalt() {
    QByteArray salt(SALT_SIZE, 0);
    randombytes_buf(salt.data(), SALT_SIZE);
    return salt;
}

bool QuantumCrypto::verifyPassword(const QByteArray& encryptedData, const QString& password) {
    // Try to decrypt - if it succeeds, password is correct
    QByteArray result = decrypt(encryptedData, password);
    bool valid = !result.isEmpty();
    sodium_memzero(result.data(), result.size());
    return valid;
}

} // namespace PCPGP
