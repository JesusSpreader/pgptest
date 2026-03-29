#include "ProfileManager.h"
#include "PGPManager.h"
#include "ConfigManager.h"
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

namespace PCPGP {

ProfileManager::ProfileManager(PGPManager* pgpManager, QObject* parent)
    : QObject(parent), m_pgpManager(pgpManager) {
    m_profilesDir = ConfigManager::getInstance().getProfilesDir();
}

ProfileManager::~ProfileManager() {
}

bool ProfileManager::initialize() {
    QDir().mkpath(m_profilesDir);
    return loadProfiles();
}

bool ProfileManager::addProfile(const KeyProfile& profile) {
    if (profile.getName().isEmpty()) {
        return false;
    }
    
    if (profileExists(profile.getName())) {
        return false;
    }
    
    m_profiles[profile.getName()] = profile;
    saveProfiles();
    
    emit profileAdded(profile.getName());
    return true;
}

bool ProfileManager::updateProfile(const KeyProfile& profile) {
    if (!profileExists(profile.getName())) {
        return false;
    }
    
    m_profiles[profile.getName()] = profile;
    saveProfiles();
    
    emit profileUpdated(profile.getName());
    return true;
}

bool ProfileManager::removeProfile(const QString& name) {
    if (!profileExists(name)) {
        return false;
    }
    
    m_profiles.remove(name);
    saveProfiles();
    
    emit profileRemoved(name);
    return true;
}

KeyProfile ProfileManager::getProfile(const QString& name) const {
    return m_profiles.value(name);
}

QList<KeyProfile> ProfileManager::getAllProfiles() const {
    return m_profiles.values();
}

QList<KeyProfile> ProfileManager::getProfilesByEmail(const QString& email) const {
    QList<KeyProfile> result;
    
    for (const auto& profile : m_profiles) {
        if (profile.getEmail().compare(email, Qt::CaseInsensitive) == 0) {
            result.append(profile);
        }
    }
    
    return result;
}

bool ProfileManager::importProfileFromKey(const QString& keyId, const QString& name) {
    PGPKey key = m_pgpManager->getKeyById(keyId);
    if (key.keyId.isEmpty()) {
        return false;
    }
    
    KeyProfile profile;
    profile.setName(name.isEmpty() ? key.userId : name);
    profile.setEmail(key.email);
    profile.setKeyId(key.keyId);
    profile.setFingerprint(key.fingerprint);
    
    return addProfile(profile);
}

bool ProfileManager::importProfileFromFile(const QString& filePath, const QString& name) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        return false;
    }
    
    KeyProfile profile = KeyProfile::fromJson(doc.object());
    if (!name.isEmpty()) {
        profile.setName(name);
    }
    
    return addProfile(profile);
}

bool ProfileManager::exportProfileToFile(const QString& name, const QString& filePath) {
    KeyProfile profile = getProfile(name);
    if (!profile.isValid()) {
        return false;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    QJsonDocument doc(profile.toJson());
    file.write(doc.toJson());
    file.close();
    
    return true;
}

bool ProfileManager::associateKeyWithProfile(const QString& profileName, const QString& keyId) {
    if (!profileExists(profileName)) {
        return false;
    }
    
    KeyProfile profile = m_profiles[profileName];
    profile.setKeyId(keyId);
    
    PGPKey key = m_pgpManager->getKeyById(keyId);
    if (!key.fingerprint.isEmpty()) {
        profile.setFingerprint(key.fingerprint);
    }
    
    m_profiles[profileName] = profile;
    saveProfiles();
    
    return true;
}

bool ProfileManager::dissociateKeyFromProfile(const QString& profileName) {
    if (!profileExists(profileName)) {
        return false;
    }
    
    KeyProfile profile = m_profiles[profileName];
    profile.setKeyId(QString());
    profile.setFingerprint(QString());
    
    m_profiles[profileName] = profile;
    saveProfiles();
    
    return true;
}

QString ProfileManager::getKeyIdForProfile(const QString& profileName) const {
    return m_profiles.value(profileName).getKeyId();
}

bool ProfileManager::setProfileTrust(const QString& name, bool trusted) {
    if (!profileExists(name)) {
        return false;
    }
    
    KeyProfile profile = m_profiles[name];
    profile.setTrusted(trusted);
    
    m_profiles[name] = profile;
    saveProfiles();
    
    emit profileUpdated(name);
    return true;
}

bool ProfileManager::setProfileTrustLevel(const QString& name, int level) {
    if (!profileExists(name)) {
        return false;
    }
    
    KeyProfile profile = m_profiles[name];
    profile.setTrustLevel(level);
    
    m_profiles[name] = profile;
    saveProfiles();
    
    emit profileUpdated(name);
    return true;
}

QList<KeyProfile> ProfileManager::searchProfiles(const QString& query) const {
    QList<KeyProfile> result;
    
    for (const auto& profile : m_profiles) {
        if (profile.getName().contains(query, Qt::CaseInsensitive) ||
            profile.getEmail().contains(query, Qt::CaseInsensitive) ||
            profile.getKeyId().contains(query, Qt::CaseInsensitive)) {
            result.append(profile);
        }
    }
    
    return result;
}

void ProfileManager::setCurrentProfile(const QString& name) {
    if (profileExists(name) || name.isEmpty()) {
        m_currentProfile = name;
        emit currentProfileChanged(name);
    }
}

QString ProfileManager::getCurrentProfile() const {
    return m_currentProfile;
}

KeyProfile ProfileManager::getCurrentProfileData() const {
    return getProfile(m_currentProfile);
}

bool ProfileManager::saveProfiles() {
    QJsonArray profilesArray;
    
    for (const auto& profile : m_profiles) {
        profilesArray.append(profile.toJson());
    }
    
    QJsonObject root;
    root["profiles"] = profilesArray;
    root["currentProfile"] = m_currentProfile;
    
    QString filePath = QDir(m_profilesDir).filePath("profiles.json");
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to save profiles";
        return false;
    }
    
    QJsonDocument doc(root);
    file.write(doc.toJson());
    file.close();
    
    return true;
}

bool ProfileManager::loadProfiles() {
    QString filePath = QDir(m_profilesDir).filePath("profiles.json");
    QFile file(filePath);
    
    if (!file.exists()) {
        return true; // No profiles yet
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to load profiles";
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        return false;
    }
    
    QJsonObject root = doc.object();
    QJsonArray profilesArray = root["profiles"].toArray();
    
    m_profiles.clear();
    for (const auto& value : profilesArray) {
        KeyProfile profile = KeyProfile::fromJson(value.toObject());
        if (profile.isValid()) {
            m_profiles[profile.getName()] = profile;
        }
    }
    
    m_currentProfile = root["currentProfile"].toString();
    
    return true;
}

void ProfileManager::syncWithPGPKeys() {
    // Sync profiles with available PGP keys
    auto keys = m_pgpManager->listPublicKeys();
    
    for (const auto& key : keys) {
        // Check if we have a profile for this key
        bool found = false;
        for (auto& profile : m_profiles) {
            if (profile.matchesKey(key.fingerprint)) {
                found = true;
                // Update profile info if needed
                if (profile.getEmail().isEmpty() && !key.email.isEmpty()) {
                    profile.setEmail(key.email);
                }
                break;
            }
        }
        
        // Create profile for key if not found
        if (!found) {
            KeyProfile newProfile;
            newProfile.setName(key.userId.isEmpty() ? key.email : key.userId);
            newProfile.setEmail(key.email);
            newProfile.setKeyId(key.keyId);
            newProfile.setFingerprint(key.fingerprint);
            
            addProfile(newProfile);
        }
    }
    
    saveProfiles();
}

QString ProfileManager::getProfilesPath() const {
    return QDir(m_profilesDir).filePath("profiles.json");
}

bool ProfileManager::profileExists(const QString& name) const {
    return m_profiles.contains(name);
}

} // namespace PCPGP
