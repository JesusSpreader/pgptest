#ifndef KEYPROFILE_H
#define KEYPROFILE_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>
#include <QPixmap>

namespace PCPGP {

// Represents a contact/friend with their PGP key
class KeyProfile {
public:
    KeyProfile();
    KeyProfile(const QString& name, const QString& email, const QString& keyId);
    
    // Getters
    QString getName() const { return m_name; }
    QString getEmail() const { return m_email; }
    QString getKeyId() const { return m_keyId; }
    QString getFingerprint() const { return m_fingerprint; }
    QString getPublicKeyPath() const { return m_publicKeyPath; }
    QString getNotes() const { return m_notes; }
    QDateTime getCreated() const { return m_created; }
    QDateTime getLastUsed() const { return m_lastUsed; }
    bool isTrusted() const { return m_trusted; }
    int getTrustLevel() const { return m_trustLevel; }
    
    // Setters
    void setName(const QString& name) { m_name = name; }
    void setEmail(const QString& email) { m_email = email; }
    void setKeyId(const QString& keyId) { m_keyId = keyId; }
    void setFingerprint(const QString& fingerprint) { m_fingerprint = fingerprint; }
    void setPublicKeyPath(const QString& path) { m_publicKeyPath = path; }
    void setNotes(const QString& notes) { m_notes = notes; }
    void setTrusted(bool trusted) { m_trusted = trusted; }
    void setTrustLevel(int level) { m_trustLevel = level; }
    void updateLastUsed() { m_lastUsed = QDateTime::currentDateTime(); }
    
    // Serialization
    QJsonObject toJson() const;
    static KeyProfile fromJson(const QJsonObject& json);
    
    // Display
    QString getDisplayName() const;
    QString getShortFingerprint() const;
    
    // Validation
    bool isValid() const;
    bool matchesKey(const QString& fingerprint) const;

private:
    QString m_name;
    QString m_email;
    QString m_keyId;
    QString m_fingerprint;
    QString m_publicKeyPath;
    QString m_notes;
    QDateTime m_created;
    QDateTime m_lastUsed;
    bool m_trusted = false;
    int m_trustLevel = 0;
};

} // namespace PCPGP

#endif // KEYPROFILE_H
