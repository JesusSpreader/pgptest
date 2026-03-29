#ifndef QUANTUMCRYPTO_H
#define QUANTUMCRYPTO_H

#include <QByteArray>
#include <QString>
#include <memory>

// Forward declaration for libsodium
extern "C" {
    #include <sodium.h>
}

namespace PCPGP {

// Post-quantum encryption using XChaCha20-Poly1305 + Argon2id
class QuantumCrypto {
public:
    QuantumCrypto();
    ~QuantumCrypto();
    
    // Initialize libsodium
    static bool initialize();
    static bool isInitialized();
    
    // Key derivation using Argon2id
    QByteArray deriveKey(const QString& password, const QByteArray& salt);
    
    // Encryption/Decryption
    QByteArray encrypt(const QByteArray& data, const QString& password);
    QByteArray decrypt(const QByteArray& data, const QString& password);
    
    // File encryption/decryption
    bool encryptFile(const QString& inputPath, const QString& outputPath, const QString& password);
    bool decryptFile(const QString& inputPath, const QString& outputPath, const QString& password);
    
    // Key encryption (for PGP private keys)
    QByteArray encryptKey(const QByteArray& keyData, const QString& password);
    QByteArray decryptKey(const QByteArray& encryptedData, const QString& password);
    
    // Generate random salt
    QByteArray generateSalt();
    
    // Verify password against encrypted data
    bool verifyPassword(const QByteArray& encryptedData, const QString& password);
    
    // Constants
    static constexpr size_t SALT_SIZE = 32;
    static constexpr size_t KEY_SIZE = 32;
    static constexpr size_t NONCE_SIZE = 24;
    static constexpr size_t TAG_SIZE = 16;

private:
    static bool s_initialized;
    
    struct EncryptedHeader {
        unsigned char salt[SALT_SIZE];
        unsigned char nonce[NONCE_SIZE];
        unsigned char tag[TAG_SIZE];
        uint64_t dataSize;
    };
};

} // namespace PCPGP

#endif // QUANTUMCRYPTO_H
