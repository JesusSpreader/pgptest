#ifndef PROFILEMANAGER_H
#define PROFILEMANAGER_H

#include <QObject>
#include <QList>
#include <QMap>
#include "KeyProfile.h"

namespace PCPGP {

class PGPManager;

class ProfileManager : public QObject {
    Q_OBJECT

public:
    explicit ProfileManager(PGPManager* pgpManager, QObject* parent = nullptr);
    ~ProfileManager();
    
    bool initialize();
    
    // Profile operations
    bool addProfile(const KeyProfile& profile);
    bool updateProfile(const KeyProfile& profile);
    bool removeProfile(const QString& name);
    KeyProfile getProfile(const QString& name) const;
    QList<KeyProfile> getAllProfiles() const;
    QList<KeyProfile> getProfilesByEmail(const QString& email) const;
    
    // Import/Export
    bool importProfileFromKey(const QString& keyId, const QString& name = QString());
    bool importProfileFromFile(const QString& filePath, const QString& name = QString());
    bool exportProfileToFile(const QString& name, const QString& filePath);
    
    // Key association
    bool associateKeyWithProfile(const QString& profileName, const QString& keyId);
    bool dissociateKeyFromProfile(const QString& profileName);
    QString getKeyIdForProfile(const QString& profileName) const;
    
    // Trust management
    bool setProfileTrust(const QString& name, bool trusted);
    bool setProfileTrustLevel(const QString& name, int level);
    
    // Search
    QList<KeyProfile> searchProfiles(const QString& query) const;
    
    // Current profile
    void setCurrentProfile(const QString& name);
    QString getCurrentProfile() const;
    KeyProfile getCurrentProfileData() const;
    
    // Persistence
    bool saveProfiles();
    bool loadProfiles();
    
    // Refresh from PGP keys
    void syncWithPGPKeys();

signals:
    void profileAdded(const QString& name);
    void profileRemoved(const QString& name);
    void profileUpdated(const QString& name);
    void currentProfileChanged(const QString& name);

private:
    QString getProfilesPath() const;
    bool profileExists(const QString& name) const;
    
    PGPManager* m_pgpManager;
    QMap<QString, KeyProfile> m_profiles;
    QString m_currentProfile;
    QString m_profilesDir;
};

} // namespace PCPGP

#endif // PROFILEMANAGER_H
