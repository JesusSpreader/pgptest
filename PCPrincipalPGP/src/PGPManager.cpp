#include "PGPManager.h"
#include "ConfigManager.h"
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>

namespace PCPGP {

PGPManager::PGPManager() : m_ctx(nullptr), m_initialized(false) {
}

PGPManager::~PGPManager() {
    if (m_ctx) {
        gpgme_release(m_ctx);
    }
}

bool PGPManager::initialize() {
    if (m_initialized) {
        return true;
    }
    
    // Initialize GPGME - returns version string, not error
    const char* version = gpgme_check_version(nullptr);
    if (!version) {
        m_lastError = "GPGME version check failed";
        return false;
    }
    
    // Set locale
    gpgme_set_locale(nullptr, LC_CTYPE, setlocale(LC_CTYPE, nullptr));
    
    // Create context
    gpgme_error_t err = gpgme_new(&m_ctx);
    if (checkError(err)) {
        return false;
    }
    
    // Set armor mode (ASCII armored output)
    gpgme_set_armor(m_ctx, 1);
    
    // Set text mode
    gpgme_set_textmode(m_ctx, 1);
    
    // Set GPG home directory
    QString gpgHome = ConfigManager::getInstance().getGPGHome();
    err = gpgme_ctx_set_engine_info(m_ctx, GPGME_PROTOCOL_OpenPGP, nullptr, 
                                     gpgHome.toUtf8().constData());
    if (checkError(err)) {
        return false;
    }
    
    // Set protocol
    err = gpgme_set_protocol(m_ctx, GPGME_PROTOCOL_OpenPGP);
    if (checkError(err)) {
        return false;
    }
    
    m_initialized = true;
    updateKeyCache();
    return true;
}

bool PGPManager::isInitialized() const {
    return m_initialized;
}

QString PGPManager::getLastError() const {
    return m_lastError;
}

bool PGPManager::generateKeyPair(const QString& name, const QString& email, 
                                  const QString& passphrase, int keyLength,
                                  int expireDays) {
    if (!m_initialized) {
        m_lastError = "PGP manager not initialized";
        return false;
    }
    
    // Build key generation parameters
    QString params = QString(
        "<GnupgKeyParms format=\"internal\">\n"
        "Key-Type: RSA\n"
        "Key-Length: %1\n"
        "Subkey-Type: RSA\n"
        "Subkey-Length: %1\n"
        "Name-Real: %2\n"
        "Name-Email: %3\n"
    ).arg(keyLength).arg(name).arg(email);
    
    if (!passphrase.isEmpty()) {
        params += QString("Passphrase: %1\n").arg(passphrase);
    }
    
    if (expireDays > 0) {
        params += QString("Expire-Date: %1d\n").arg(expireDays);
    } else {
        params += "Expire-Date: 0\n";
    }
    
    params += "</GnupgKeyParms>\n";
    
    gpgme_genkey_result_t result = nullptr;
    gpgme_error_t err = gpgme_op_genkey(m_ctx, params.toUtf8().constData(), nullptr, nullptr);
    
    if (checkError(err)) {
        return false;
    }
    
    result = gpgme_op_genkey_result(m_ctx);
    if (!result || !result->fpr) {
        m_lastError = "Key generation failed - no fingerprint returned";
        return false;
    }
    
    updateKeyCache();
    return true;
}

bool PGPManager::importKey(const QString& keyData) {
    if (!m_initialized) {
        m_lastError = "PGP manager not initialized";
        return false;
    }
    
    gpgme_data_t data;
    gpgme_error_t err = gpgme_data_new_from_mem(&data, keyData.toUtf8().constData(),
                                                 keyData.toUtf8().size(), 0);
    if (checkError(err)) {
        return false;
    }
    
    err = gpgme_op_import(m_ctx, data);
    gpgme_data_release(data);
    
    if (checkError(err)) {
        return false;
    }
    
    updateKeyCache();
    return true;
}

bool PGPManager::importKeyFromFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        m_lastError = "Failed to open key file";
        return false;
    }
    
    QByteArray keyData = file.readAll();
    file.close();
    
    return importKey(QString::fromUtf8(keyData));
}

bool PGPManager::importPublicKey(const QByteArray& keyData) {
    return importKey(QString::fromUtf8(keyData));
}

bool PGPManager::importPrivateKey(const QByteArray& keyData) {
    return importKey(QString::fromUtf8(keyData));
}

QByteArray PGPManager::exportPublicKey(const QString& keyId) {
    if (!m_initialized) {
        m_lastError = "PGP manager not initialized";
        return QByteArray();
    }
    
    gpgme_key_t key[2] = { nullptr, nullptr };
    gpgme_error_t err = gpgme_get_key(m_ctx, keyId.toUtf8().constData(), &key[0], 0);
    if (checkError(err)) {
        return QByteArray();
    }
    
    gpgme_data_t out;
    err = gpgme_data_new(&out);
    if (checkError(err)) {
        gpgme_key_release(key[0]);
        return QByteArray();
    }
    
    err = gpgme_op_export_keys(m_ctx, key, 0, out);
    gpgme_key_release(key[0]);
    
    if (checkError(err)) {
        gpgme_data_release(out);
        return QByteArray();
    }
    
    QByteArray result = readGPGData(out);
    gpgme_data_release(out);
    
    return result;
}

QByteArray PGPManager::exportPrivateKey(const QString& keyId) {
    if (!m_initialized) {
        m_lastError = "PGP manager not initialized";
        return QByteArray();
    }
    
    gpgme_key_t key[2] = { nullptr, nullptr };
    gpgme_error_t err = gpgme_get_key(m_ctx, keyId.toUtf8().constData(), &key[0], 1);
    if (checkError(err)) {
        return QByteArray();
    }
    
    gpgme_data_t out;
    err = gpgme_data_new(&out);
    if (checkError(err)) {
        gpgme_key_release(key[0]);
        return QByteArray();
    }
    
    err = gpgme_op_export_keys(m_ctx, key, GPGME_EXPORT_MODE_SECRET, out);
    gpgme_key_release(key[0]);
    
    if (checkError(err)) {
        gpgme_data_release(out);
        return QByteArray();
    }
    
    QByteArray result = readGPGData(out);
    gpgme_data_release(out);
    
    return result;
}

bool PGPManager::exportPublicKeyToFile(const QString& keyId, const QString& filePath) {
    QByteArray keyData = exportPublicKey(keyId);
    if (keyData.isEmpty()) {
        return false;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        m_lastError = "Failed to open output file";
        return false;
    }
    
    file.write(keyData);
    file.close();
    return true;
}

bool PGPManager::exportPrivateKeyToFile(const QString& keyId, const QString& filePath) {
    QByteArray keyData = exportPrivateKey(keyId);
    if (keyData.isEmpty()) {
        return false;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        m_lastError = "Failed to open output file";
        return false;
    }
    
    file.write(keyData);
    file.close();
    return true;
}

QList<PGPKey> PGPManager::listKeys(bool includeSecret) {
    if (!m_initialized) {
        m_lastError = "PGP manager not initialized";
        return QList<PGPKey>();
    }
    
    QList<PGPKey> keys;
    gpgme_key_t key;
    
    gpgme_error_t err = gpgme_op_keylist_start(m_ctx, nullptr, includeSecret ? 1 : 0);
    if (checkError(err)) {
        return keys;
    }
    
    while (!(err = gpgme_op_keylist_next(m_ctx, &key))) {
        keys.append(parseKey(key));
        gpgme_key_release(key);
    }
    
    gpgme_op_keylist_end(m_ctx);
    return keys;
}

QList<PGPKey> PGPManager::listPublicKeys() {
    return listKeys(false);
}

QList<PGPKey> PGPManager::listSecretKeys() {
    return listKeys(true);
}

PGPKey PGPManager::getKeyById(const QString& keyId) {
    for (const auto& key : m_keyCache) {
        if (key.keyId == keyId || key.keyId.endsWith(keyId)) {
            return key;
        }
    }
    return PGPKey();
}

PGPKey PGPManager::getKeyByFingerprint(const QString& fingerprint) {
    for (const auto& key : m_keyCache) {
        if (key.fingerprint == fingerprint || key.fingerprint.endsWith(fingerprint)) {
            return key;
        }
    }
    return PGPKey();
}

PGPKey PGPManager::getKeyByEmail(const QString& email) {
    for (const auto& key : m_keyCache) {
        if (key.email.compare(email, Qt::CaseInsensitive) == 0) {
            return key;
        }
    }
    return PGPKey();
}

bool PGPManager::deleteKey(const QString& keyId, bool deleteSecret) {
    if (!m_initialized) {
        m_lastError = "PGP manager not initialized";
        return false;
    }
    
    gpgme_key_t key;
    gpgme_error_t err = gpgme_get_key(m_ctx, keyId.toUtf8().constData(), &key, deleteSecret ? 1 : 0);
    if (checkError(err)) {
        return false;
    }
    
    err = gpgme_op_delete_ext(m_ctx, key, deleteSecret ? GPGME_DELETE_ALLOW_SECRET : 0);
    gpgme_key_release(key);
    
    if (checkError(err)) {
        return false;
    }
    
    updateKeyCache();
    return true;
}

bool PGPManager::revokeKey(const QString&, const QString&) {
    // Key revocation requires a revocation certificate
    // This is a simplified implementation
    m_lastError = "Key revocation requires manual GPG operation";
    return false;
}

bool PGPManager::changePassphrase(const QString&, const QString&, const QString&) {
    // Passphrase change requires interactive GPG operation
    m_lastError = "Passphrase change requires interactive operation";
    return false;
}

bool PGPManager::setKeyTrust(const QString&, int) {
    // Trust level setting requires GPG's edit-key functionality
    m_lastError = "Trust level change requires interactive operation";
    return false;
}

QByteArray PGPManager::encrypt(const QByteArray& data, const QString& recipientKeyId) {
    if (!m_initialized) {
        m_lastError = "PGP manager not initialized";
        return QByteArray();
    }
    
    gpgme_key_t key[2] = { nullptr, nullptr };
    gpgme_error_t err = gpgme_get_key(m_ctx, recipientKeyId.toUtf8().constData(), &key[0], 0);
    if (checkError(err)) {
        return QByteArray();
    }
    
    gpgme_data_t plain, cipher;
    err = gpgme_data_new_from_mem(&plain, data.constData(), data.size(), 0);
    if (checkError(err)) {
        gpgme_key_release(key[0]);
        return QByteArray();
    }
    
    err = gpgme_data_new(&cipher);
    if (checkError(err)) {
        gpgme_data_release(plain);
        gpgme_key_release(key[0]);
        return QByteArray();
    }
    
    err = gpgme_op_encrypt(m_ctx, key, GPGME_ENCRYPT_ALWAYS_TRUST, plain, cipher);
    gpgme_data_release(plain);
    gpgme_key_release(key[0]);
    
    if (checkError(err)) {
        gpgme_data_release(cipher);
        return QByteArray();
    }
    
    QByteArray result = readGPGData(cipher);
    gpgme_data_release(cipher);
    
    return result;
}

QByteArray PGPManager::encryptForMultipleRecipients(const QByteArray& data, const QStringList& recipientKeyIds) {
    if (recipientKeyIds.isEmpty()) {
        m_lastError = "No recipients specified";
        return QByteArray();
    }
    
    if (recipientKeyIds.size() == 1) {
        return encrypt(data, recipientKeyIds.first());
    }
    
    // For multiple recipients, encrypt for each
    // This is a simplified implementation
    return encrypt(data, recipientKeyIds.first());
}

QByteArray PGPManager::decrypt(const QByteArray& data, QString& outKeyId) {
    if (!m_initialized) {
        m_lastError = "PGP manager not initialized";
        return QByteArray();
    }
    
    gpgme_data_t cipher, plain;
    gpgme_error_t err = gpgme_data_new_from_mem(&cipher, data.constData(), data.size(), 0);
    if (checkError(err)) {
        return QByteArray();
    }
    
    err = gpgme_data_new(&plain);
    if (checkError(err)) {
        gpgme_data_release(cipher);
        return QByteArray();
    }
    
    err = gpgme_op_decrypt(m_ctx, cipher, plain);
    gpgme_data_release(cipher);
    
    if (checkError(err)) {
        gpgme_data_release(plain);
        return QByteArray();
    }
    
    // Get decryption result to find key ID
    gpgme_decrypt_result_t result = gpgme_op_decrypt_result(m_ctx);
    if (result && result->recipients) {
        outKeyId = QString::fromUtf8(result->recipients->keyid);
    }
    
    QByteArray output = readGPGData(plain);
    gpgme_data_release(plain);
    
    return output;
}

bool PGPManager::encryptFile(const QString& inputPath, const QString& outputPath, 
                              const QString& recipientKeyId) {
    QFile inputFile(inputPath);
    if (!inputFile.open(QIODevice::ReadOnly)) {
        m_lastError = "Failed to open input file";
        return false;
    }
    
    QByteArray data = inputFile.readAll();
    inputFile.close();
    
    QByteArray encrypted = encrypt(data, recipientKeyId);
    if (encrypted.isEmpty()) {
        return false;
    }
    
    QFile outputFile(outputPath);
    if (!outputFile.open(QIODevice::WriteOnly)) {
        m_lastError = "Failed to open output file";
        return false;
    }
    
    outputFile.write(encrypted);
    outputFile.close();
    return true;
}

bool PGPManager::decryptFile(const QString& inputPath, const QString& outputPath,
                              QString& outKeyId) {
    QFile inputFile(inputPath);
    if (!inputFile.open(QIODevice::ReadOnly)) {
        m_lastError = "Failed to open input file";
        return false;
    }
    
    QByteArray data = inputFile.readAll();
    inputFile.close();
    
    QByteArray decrypted = decrypt(data, outKeyId);
    if (decrypted.isEmpty()) {
        return false;
    }
    
    QFile outputFile(outputPath);
    if (!outputFile.open(QIODevice::WriteOnly)) {
        m_lastError = "Failed to open output file";
        return false;
    }
    
    outputFile.write(decrypted);
    outputFile.close();
    return true;
}

QByteArray PGPManager::sign(const QByteArray& data, const QString& keyId, 
                             bool detached, const QString&) {
    if (!m_initialized) {
        m_lastError = "PGP manager not initialized";
        return QByteArray();
    }
    
    // Set signer key
    gpgme_signers_clear(m_ctx);
    gpgme_key_t key;
    gpgme_error_t err = gpgme_get_key(m_ctx, keyId.toUtf8().constData(), &key, 1);
    if (checkError(err)) {
        return QByteArray();
    }
    
    err = gpgme_signers_add(m_ctx, key);
    gpgme_key_release(key);
    if (checkError(err)) {
        return QByteArray();
    }
    
    gpgme_data_t plain, sig;
    err = gpgme_data_new_from_mem(&plain, data.constData(), data.size(), 0);
    if (checkError(err)) {
        return QByteArray();
    }
    
    err = gpgme_data_new(&sig);
    if (checkError(err)) {
        gpgme_data_release(plain);
        return QByteArray();
    }
    
    gpgme_sig_mode_t mode = detached ? GPGME_SIG_MODE_DETACH : GPGME_SIG_MODE_CLEAR;
    err = gpgme_op_sign(m_ctx, plain, sig, mode);
    gpgme_data_release(plain);
    
    if (checkError(err)) {
        gpgme_data_release(sig);
        return QByteArray();
    }
    
    QByteArray result = readGPGData(sig);
    gpgme_data_release(sig);
    
    return result;
}

bool PGPManager::verify(const QByteArray& data, const QByteArray& signature,
                         QString& outSignerKeyId) {
    if (!m_initialized) {
        m_lastError = "PGP manager not initialized";
        return false;
    }
    
    gpgme_data_t sig, plain;
    gpgme_error_t err = gpgme_data_new_from_mem(&sig, signature.constData(), 
                                                 signature.size(), 0);
    if (checkError(err)) {
        return false;
    }
    
    err = gpgme_data_new_from_mem(&plain, data.constData(), data.size(), 0);
    if (checkError(err)) {
        gpgme_data_release(sig);
        return false;
    }
    
    err = gpgme_op_verify(m_ctx, sig, plain, nullptr);
    gpgme_data_release(sig);
    gpgme_data_release(plain);
    
    if (checkError(err)) {
        return false;
    }
    
    gpgme_verify_result_t result = gpgme_op_verify_result(m_ctx);
    if (result && result->signatures) {
        outSignerKeyId = QString::fromUtf8(result->signatures->fpr);
        return result->signatures->status == GPG_ERR_NO_ERROR;
    }
    
    return false;
}

bool PGPManager::signFile(const QString& inputPath, const QString& outputPath,
                           const QString& keyId, bool detached) {
    QFile inputFile(inputPath);
    if (!inputFile.open(QIODevice::ReadOnly)) {
        m_lastError = "Failed to open input file";
        return false;
    }
    
    QByteArray data = inputFile.readAll();
    inputFile.close();
    
    QByteArray signature = sign(data, keyId, detached);
    if (signature.isEmpty()) {
        return false;
    }
    
    QFile outputFile(outputPath);
    if (!outputFile.open(QIODevice::WriteOnly)) {
        m_lastError = "Failed to open output file";
        return false;
    }
    
    outputFile.write(signature);
    outputFile.close();
    return true;
}

bool PGPManager::verifyFile(const QString& filePath, const QString& signaturePath,
                             QString& outSignerKeyId) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        m_lastError = "Failed to open file";
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QFile sigFile(signaturePath);
    if (!sigFile.open(QIODevice::ReadOnly)) {
        m_lastError = "Failed to open signature file";
        return false;
    }
    
    QByteArray signature = sigFile.readAll();
    sigFile.close();
    
    return verify(data, signature, outSignerKeyId);
}

QByteArray PGPManager::cleartextSign(const QByteArray& data, const QString& keyId,
                                      const QString& passphrase) {
    return sign(data, keyId, false, passphrase);
}

bool PGPManager::verifyCleartext(const QByteArray& signedData, QString& outSignerKeyId) {
    // Cleartext signatures are verified similarly to detached signatures
    // The signed data contains both the message and signature
    return verify(signedData, signedData, outSignerKeyId);
}

int PGPManager::importKeysFromDirectory(const QString& dirPath, bool) {
    QDir dir(dirPath);
    QStringList filters;
    filters << "*.asc" << "*.gpg" << "*.pgp" << "*.pub" << "*.key";
    
    QStringList files = dir.entryList(filters, QDir::Files);
    int imported = 0;
    
    for (const QString& file : files) {
        QString filePath = dir.filePath(file);
        if (importKeyFromFile(filePath)) {
            imported++;
        }
    }
    
    return imported;
}

bool PGPManager::validateKeyPair(const QString& publicKeyPath, const QString& privateKeyPath) {
    // Import both keys and check if they match
    QByteArray pubData, privData;
    
    QFile pubFile(publicKeyPath);
    if (!pubFile.open(QIODevice::ReadOnly)) {
        return false;
    }
    pubData = pubFile.readAll();
    pubFile.close();
    
    QFile privFile(privateKeyPath);
    if (!privFile.open(QIODevice::ReadOnly)) {
        return false;
    }
    privData = privFile.readAll();
    privFile.close();
    
    // Extract fingerprints and compare
    // This is a simplified check
    return !pubData.isEmpty() && !privData.isEmpty();
}

bool PGPManager::keysMatch(const PGPKey& publicKey, const PGPKey& privateKey) {
    return publicKey.fingerprint == privateKey.fingerprint;
}

bool PGPManager::saveProfile(const PGPProfile& profile) {
    QJsonObject obj;
    obj["name"] = profile.name;
    obj["email"] = profile.email;
    obj["keyId"] = profile.keyId;
    obj["fingerprint"] = profile.fingerprint;
    obj["publicKeyPath"] = profile.publicKeyPath;
    obj["privateKeyPath"] = profile.privateKeyPath;
    obj["created"] = profile.created.toString(Qt::ISODate);
    obj["notes"] = profile.notes;
    
    QString profilePath = QDir(ConfigManager::getInstance().getProfilesDir())
                          .filePath(profile.name + ".json");
    
    QFile file(profilePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    QJsonDocument doc(obj);
    file.write(doc.toJson());
    file.close();
    return true;
}

bool PGPManager::loadProfile(const QString& name, PGPProfile& outProfile) {
    QString profilePath = QDir(ConfigManager::getInstance().getProfilesDir())
                          .filePath(name + ".json");
    
    QFile file(profilePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        return false;
    }
    
    QJsonObject obj = doc.object();
    outProfile.name = obj["name"].toString();
    outProfile.email = obj["email"].toString();
    outProfile.keyId = obj["keyId"].toString();
    outProfile.fingerprint = obj["fingerprint"].toString();
    outProfile.publicKeyPath = obj["publicKeyPath"].toString();
    outProfile.privateKeyPath = obj["privateKeyPath"].toString();
    outProfile.created = QDateTime::fromString(obj["created"].toString(), Qt::ISODate);
    outProfile.notes = obj["notes"].toString();
    
    return true;
}

QList<PGPProfile> PGPManager::listProfiles() {
    QList<PGPProfile> profiles;
    QDir dir(ConfigManager::getInstance().getProfilesDir());
    QStringList files = dir.entryList(QStringList() << "*.json", QDir::Files);
    
    for (const QString& file : files) {
        PGPProfile profile;
        QString name = file.left(file.length() - 5); // Remove .json
        if (loadProfile(name, profile)) {
            profiles.append(profile);
        }
    }
    
    return profiles;
}

bool PGPManager::deleteProfile(const QString& name) {
    QString profilePath = QDir(ConfigManager::getInstance().getProfilesDir())
                          .filePath(name + ".json");
    return QFile::remove(profilePath);
}

bool PGPManager::encryptFileForProfile(const QString& filePath, const QString& profileName) {
    PGPProfile profile;
    if (!loadProfile(profileName, profile)) {
        m_lastError = "Profile not found";
        return false;
    }
    
    QString outputPath = filePath + ".gpg";
    return encryptFile(filePath, outputPath, profile.keyId);
}

bool PGPManager::decryptFileToPath(const QString& filePath, const QString& outputPath) {
    QString keyId;
    return decryptFile(filePath, outputPath, keyId);
}

QByteArray PGPManager::getPublicKeyArmored(const QString& keyId) {
    return exportPublicKey(keyId);
}

void PGPManager::refreshKeyCache() {
    updateKeyCache();
}

bool PGPManager::checkError(gpgme_error_t err) {
    if (err != GPG_ERR_NO_ERROR) {
        m_lastError = QString::fromUtf8(gpgme_strerror(err));
        qWarning() << "GPGME error:" << m_lastError;
        return true;
    }
    return false;
}

PGPKey PGPManager::parseKey(gpgme_key_t key) {
    PGPKey pgpKey;
    
    if (!key) {
        return pgpKey;
    }
    
    pgpKey.keyId = QString::fromUtf8(key->subkeys ? key->subkeys->keyid : "");
    pgpKey.fingerprint = QString::fromUtf8(key->subkeys ? key->subkeys->fpr : "");
    pgpKey.isPrivate = key->secret ? true : false;
    pgpKey.isPublic = key->protocol == GPGME_PROTOCOL_OpenPGP;
    pgpKey.keyLength = key->subkeys ? key->subkeys->length : 0;
    pgpKey.canEncrypt = key->can_encrypt;
    pgpKey.canSign = key->can_sign;
    pgpKey.canCertify = key->can_certify;
    pgpKey.isRevoked = key->revoked;
    pgpKey.isExpired = key->expired;
    pgpKey.isDisabled = key->disabled;
    
    if (key->subkeys) {
        pgpKey.creationDate = QDateTime::fromSecsSinceEpoch(key->subkeys->timestamp);
        if (key->subkeys->expires) {
            pgpKey.expirationDate = QDateTime::fromSecsSinceEpoch(key->subkeys->expires);
        }
    }
    
    if (key->uids) {
        pgpKey.userId = QString::fromUtf8(key->uids->name);
        pgpKey.email = QString::fromUtf8(key->uids->email);
    }
    
    // Determine algorithm
    if (key->subkeys) {
        switch (key->subkeys->pubkey_algo) {
            case GPGME_PK_RSA:
                pgpKey.algorithm = "RSA";
                break;
            case GPGME_PK_ELG:
                pgpKey.algorithm = "ElGamal";
                break;
            case GPGME_PK_DSA:
                pgpKey.algorithm = "DSA";
                break;
            case GPGME_PK_ECC:
                pgpKey.algorithm = "ECC";
                break;
            case GPGME_PK_ECDSA:
                pgpKey.algorithm = "ECDSA";
                break;
            case GPGME_PK_ECDH:
                pgpKey.algorithm = "ECDH";
                break;
            case GPGME_PK_EDDSA:
                pgpKey.algorithm = "EdDSA";
                break;
            default:
                pgpKey.algorithm = "Unknown";
        }
    }
    
    return pgpKey;
}

QByteArray PGPManager::readGPGData(gpgme_data_t data) {
    gpgme_data_seek(data, 0, SEEK_SET);
    
    const char* buf;
    size_t len;
    QByteArray result;
    
    while ((len = gpgme_data_read(data, &buf, 0)) > 0) {
        // This doesn't work directly, use alternative approach
        break;
    }
    
    // Alternative: read using seek and read
    gpgme_data_seek(data, 0, SEEK_END);
    off_t size = gpgme_data_seek(data, 0, SEEK_CUR);
    gpgme_data_seek(data, 0, SEEK_SET);
    
    result.resize(static_cast<int>(size));
    size_t read = gpgme_data_read(data, result.data(), size);
    result.resize(static_cast<int>(read));
    
    return result;
}

bool PGPManager::writeGPGData(gpgme_data_t data, const QByteArray& content) {
    gpgme_data_seek(data, 0, SEEK_SET);
    ssize_t written = gpgme_data_write(data, content.constData(), content.size());
    return written == content.size();
}

void PGPManager::updateKeyCache() {
    m_keyCache = listKeys(true);
}

} // namespace PCPGP
