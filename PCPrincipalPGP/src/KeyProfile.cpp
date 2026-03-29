#include "KeyProfile.h"
#include <QJsonObject>

namespace PCPGP {

KeyProfile::KeyProfile() : m_created(QDateTime::currentDateTime()), 
                            m_lastUsed(QDateTime::currentDateTime()),
                            m_trusted(false), m_trustLevel(0) {
}

KeyProfile::KeyProfile(const QString& name, const QString& email, const QString& keyId)
    : m_name(name), m_email(email), m_keyId(keyId),
      m_created(QDateTime::currentDateTime()),
      m_lastUsed(QDateTime::currentDateTime()),
      m_trusted(false), m_trustLevel(0) {
}

QJsonObject KeyProfile::toJson() const {
    QJsonObject obj;
    obj["name"] = m_name;
    obj["email"] = m_email;
    obj["keyId"] = m_keyId;
    obj["fingerprint"] = m_fingerprint;
    obj["publicKeyPath"] = m_publicKeyPath;
    obj["notes"] = m_notes;
    obj["created"] = m_created.toString(Qt::ISODate);
    obj["lastUsed"] = m_lastUsed.toString(Qt::ISODate);
    obj["trusted"] = m_trusted;
    obj["trustLevel"] = m_trustLevel;
    return obj;
}

KeyProfile KeyProfile::fromJson(const QJsonObject& json) {
    KeyProfile profile;
    profile.m_name = json["name"].toString();
    profile.m_email = json["email"].toString();
    profile.m_keyId = json["keyId"].toString();
    profile.m_fingerprint = json["fingerprint"].toString();
    profile.m_publicKeyPath = json["publicKeyPath"].toString();
    profile.m_notes = json["notes"].toString();
    profile.m_created = QDateTime::fromString(json["created"].toString(), Qt::ISODate);
    profile.m_lastUsed = QDateTime::fromString(json["lastUsed"].toString(), Qt::ISODate);
    profile.m_trusted = json["trusted"].toBool();
    profile.m_trustLevel = json["trustLevel"].toInt();
    return profile;
}

QString KeyProfile::getDisplayName() const {
    if (!m_name.isEmpty() && !m_email.isEmpty()) {
        return m_name + " <" + m_email + ">";
    } else if (!m_name.isEmpty()) {
        return m_name;
    } else if (!m_email.isEmpty()) {
        return m_email;
    } else {
        return m_keyId;
    }
}

QString KeyProfile::getShortFingerprint() const {
    if (m_fingerprint.length() >= 16) {
        return m_fingerprint.right(16);
    }
    return m_fingerprint;
}

bool KeyProfile::isValid() const {
    return !m_keyId.isEmpty() && (!m_name.isEmpty() || !m_email.isEmpty());
}

bool KeyProfile::matchesKey(const QString& fingerprint) const {
    return m_fingerprint.compare(fingerprint, Qt::CaseInsensitive) == 0 ||
           m_fingerprint.right(16).compare(fingerprint.right(16), Qt::CaseInsensitive) == 0;
}

} // namespace PCPGP
