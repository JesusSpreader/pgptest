#include "ConfigManager.h"
#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QDebug>

namespace PCPGP {

ConfigManager& ConfigManager::getInstance() {
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::initialize() {
    if (m_initialized) {
        return true;
    }
    
    // Get application directory
    m_appDir = QCoreApplication::applicationDirPath();
    
    // Ensure directories exist
    ensureDirectoriesExist();
    
    // Load existing configuration
    if (QFile::exists(getConfigPath())) {
        if (!loadConfig()) {
            qWarning() << "Failed to load config, using defaults";
        }
    }
    
    m_initialized = true;
    return true;
}

bool ConfigManager::isFirstRun() const {
    return !QFile::exists(getConfigPath());
}

void ConfigManager::setPortableMode(PortableMode mode) {
    m_mode = mode;
}

PortableMode ConfigManager::getPortableMode() const {
    return m_mode;
}

QString ConfigManager::getAppDirectory() const {
    return m_appDir;
}

QString ConfigManager::getKeysDirectory() const {
    return QDir(m_appDir).filePath("keys");
}

QString ConfigManager::getPublicKeysDir() const {
    if (m_mode == PortableMode::PARTIAL_PORTABLE && !m_customPublicDir.isEmpty()) {
        return m_customPublicDir;
    }
    return QDir(getKeysDirectory()).filePath("public");
}

QString ConfigManager::getPrivateKeysDir() const {
    if (m_mode == PortableMode::PARTIAL_PORTABLE && !m_customPrivateDir.isEmpty()) {
        return m_customPrivateDir;
    }
    return QDir(getKeysDirectory()).filePath("private");
}

QString ConfigManager::getProfilesDir() const {
    return QDir(m_appDir).filePath("profiles");
}

QString ConfigManager::getTempDir() const {
    QString tempPath = QDir(m_appDir).filePath("temp");
    QDir().mkpath(tempPath);
    return tempPath;
}

void ConfigManager::setCustomPublicKeysDir(const QString& path) {
    m_customPublicDir = path;
    QDir().mkpath(path);
}

void ConfigManager::setCustomPrivateKeysDir(const QString& path) {
    m_customPrivateDir = path;
    QDir().mkpath(path);
}

QString ConfigManager::getCustomPublicKeysDir() const {
    return m_customPublicDir;
}

QString ConfigManager::getCustomPrivateKeysDir() const {
    return m_customPrivateDir;
}

void ConfigManager::setPostQuantumEnabled(bool enabled) {
    m_postQuantumEnabled = enabled;
}

bool ConfigManager::isPostQuantumEnabled() const {
    return m_postQuantumEnabled;
}

void ConfigManager::setEncryptPrivateKeysOnly(bool only) {
    m_encryptPrivateOnly = only;
}

bool ConfigManager::isEncryptPrivateKeysOnly() const {
    return m_encryptPrivateOnly;
}

bool ConfigManager::saveConfig() {
    QJsonObject config;
    config["portableMode"] = (m_mode == PortableMode::DIRECTORY_MODE) ? "directory" : "partial";
    config["customPublicDir"] = m_customPublicDir;
    config["customPrivateDir"] = m_customPrivateDir;
    config["postQuantumEnabled"] = m_postQuantumEnabled;
    config["encryptPrivateOnly"] = m_encryptPrivateOnly;
    config["passwordProtected"] = m_passwordProtected;
    config["version"] = "1.0.0";
    
    QJsonDocument doc(config);
    QFile file(getConfigPath());
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open config file for writing:" << getConfigPath();
        return false;
    }
    
    file.write(doc.toJson());
    file.close();
    return true;
}

bool ConfigManager::loadConfig() {
    QFile file(getConfigPath());
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open config file:" << getConfigPath();
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || !doc.isObject()) {
        qWarning() << "Invalid config file format";
        return false;
    }
    
    QJsonObject config = doc.object();
    
    QString mode = config["portableMode"].toString();
    m_mode = (mode == "partial") ? PortableMode::PARTIAL_PORTABLE : PortableMode::DIRECTORY_MODE;
    
    m_customPublicDir = config["customPublicDir"].toString();
    m_customPrivateDir = config["customPrivateDir"].toString();
    m_postQuantumEnabled = config["postQuantumEnabled"].toBool();
    m_encryptPrivateOnly = config["encryptPrivateOnly"].toBool();
    m_passwordProtected = config["passwordProtected"].toBool();
    
    m_config = config;
    return true;
}

bool ConfigManager::isPasswordProtected() const {
    return m_passwordProtected;
}

void ConfigManager::setPasswordProtected(bool protected_) {
    m_passwordProtected = protected_;
}

QString ConfigManager::getGPGHome() const {
    // Use a portable GPG home in the app directory
    QString gpgHome = QDir(m_appDir).filePath("gpg");
    QDir().mkpath(gpgHome);
    return gpgHome;
}

QString ConfigManager::getConfigPath() const {
    return QDir(m_appDir).filePath("config.json");
}

void ConfigManager::ensureDirectoriesExist() {
    QDir dir;
    
    // Create key directories
    dir.mkpath(getPublicKeysDir());
    dir.mkpath(getPrivateKeysDir());
    dir.mkpath(getProfilesDir());
    dir.mkpath(getTempDir());
    dir.mkpath(getGPGHome());
}

} // namespace PCPGP
