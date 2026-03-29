#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QString>
#include <QDir>
#include <QJsonObject>
#include <QJsonDocument>
#include <memory>

namespace PCPGP {

enum class PortableMode {
    DIRECTORY_MODE,      // Fully portable - keys next to exe
    PARTIAL_PORTABLE     // User picks key folders
};

class ConfigManager {
public:
    static ConfigManager& getInstance();
    
    // Initialization
    bool initialize();
    bool isFirstRun() const;
    
    // Mode settings
    void setPortableMode(PortableMode mode);
    PortableMode getPortableMode() const;
    
    // Directory paths
    QString getAppDirectory() const;
    QString getKeysDirectory() const;
    QString getPublicKeysDir() const;
    QString getPrivateKeysDir() const;
    QString getProfilesDir() const;
    QString getTempDir() const;
    
    // Custom paths (for partial portable)
    void setCustomPublicKeysDir(const QString& path);
    void setCustomPrivateKeysDir(const QString& path);
    QString getCustomPublicKeysDir() const;
    QString getCustomPrivateKeysDir() const;
    
    // Configuration
    void setPostQuantumEnabled(bool enabled);
    bool isPostQuantumEnabled() const;
    void setEncryptPrivateKeysOnly(bool only);
    bool isEncryptPrivateKeysOnly() const;
    
    // Save/Load
    bool saveConfig();
    bool loadConfig();
    
    // Security
    bool isPasswordProtected() const;
    void setPasswordProtected(bool protected_);
    
    // GPG Home
    QString getGPGHome() const;
    
private:
    ConfigManager() = default;
    ~ConfigManager() = default;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;
    
    QString getConfigPath() const;
    void ensureDirectoriesExist();
    
    PortableMode m_mode = PortableMode::DIRECTORY_MODE;
    QString m_customPublicDir;
    QString m_customPrivateDir;
    QString m_appDir;
    bool m_postQuantumEnabled = false;
    bool m_encryptPrivateOnly = true;
    bool m_passwordProtected = false;
    bool m_initialized = false;
    QJsonObject m_config;
};

} // namespace PCPGP

#endif // CONFIGMANAGER_H
