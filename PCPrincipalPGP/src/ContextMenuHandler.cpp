#include "ContextMenuHandler.h"
#include "PGPManager.h"
#include "ProfileManager.h"
#include <QSettings>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace PCPGP {

ContextMenuHandler::ContextMenuHandler(PGPManager* pgpManager, 
                                        ProfileManager* profileManager,
                                        QObject* parent)
    : QObject(parent), m_pgpManager(pgpManager), 
      m_profileManager(profileManager), m_registered(false) {
}

ContextMenuHandler::~ContextMenuHandler() {
}

bool ContextMenuHandler::registerContextMenu() {
#ifdef Q_OS_WIN
    return registerShellExtension();
#else
    // Context menu registration is Windows-specific
    return false;
#endif
}

bool ContextMenuHandler::unregisterContextMenu() {
#ifdef Q_OS_WIN
    return unregisterShellExtension();
#else
    return false;
#endif
}

bool ContextMenuHandler::isContextMenuRegistered() const {
    return m_registered;
}

bool ContextMenuHandler::handleCommand(const QString& command, const QStringList& files) {
    if (files.isEmpty()) {
        return false;
    }
    
    if (command == "encrypt") {
        return encryptFile(files.first());
    } else if (command == "decrypt") {
        return decryptFile(files.first());
    } else if (command == "sign") {
        return signFile(files.first());
    } else if (command == "verify") {
        return verifyFile(files.first());
    } else if (command == "import") {
        return importKey(files.first());
    } else if (command.startsWith("encrypt_to:")) {
        QString profile = command.mid(11);
        return encryptFileTo(files.first(), profile);
    }
    
    return false;
}

bool ContextMenuHandler::encryptFile(const QString& filePath) {
    // Show profile selection dialog and encrypt
    Q_UNUSED(filePath)
    return false;
}

bool ContextMenuHandler::encryptFileTo(const QString& filePath, const QString& profileName) {
    // Encrypt file for specific profile
    QString outputPath = filePath + ".gpg";
    return m_pgpManager->encryptFileForProfile(filePath, profileName);
}

bool ContextMenuHandler::decryptFile(const QString& filePath) {
    QString outputPath = filePath;
    // Remove .gpg extension if present
    if (outputPath.endsWith(".gpg")) {
        outputPath = outputPath.left(outputPath.length() - 4);
    } else if (outputPath.endsWith(".asc")) {
        outputPath = outputPath.left(outputPath.length() - 4);
    } else if (outputPath.endsWith(".pgp")) {
        outputPath = outputPath.left(outputPath.length() - 4);
    } else {
        outputPath += ".decrypted";
    }
    
    QString keyId;
    return m_pgpManager->decryptFile(filePath, outputPath, keyId);
}

bool ContextMenuHandler::signFile(const QString& filePath) {
    QString outputPath = filePath + ".sig";
    
    // Get default signing key
    QString keyId = m_profileManager->getCurrentProfileData().getKeyId();
    if (keyId.isEmpty()) {
        return false;
    }
    
    return m_pgpManager->signFile(filePath, outputPath, keyId, true);
}

bool ContextMenuHandler::verifyFile(const QString& filePath) {
    QString sigPath = filePath + ".sig";
    
    if (!QFile::exists(sigPath)) {
        // Try .asc extension
        sigPath = filePath + ".asc";
        if (!QFile::exists(sigPath)) {
            return false;
        }
    }
    
    QString signerKeyId;
    return m_pgpManager->verifyFile(filePath, sigPath, signerKeyId);
}

bool ContextMenuHandler::importKey(const QString& filePath) {
    return m_pgpManager->importKeyFromFile(filePath);
}

bool ContextMenuHandler::getPublicKey(const QString& keyId) {
    // Copy public key to clipboard
    QByteArray keyData = m_pgpManager->getPublicKeyArmored(keyId);
    if (!keyData.isEmpty()) {
        // Copy to clipboard
        return true;
    }
    return false;
}

#ifdef Q_OS_WIN
bool ContextMenuHandler::registerShellExtension() {
    // Register context menu entries in Windows registry
    QString appPath = QCoreApplication::applicationFilePath();
    
    try {
        // Register for all files
        QString baseKey = "HKEY_CURRENT_USER\\Software\\Classes\\*\\shell\\PCPrincipalPGP";
        WinRegistry::writeString(baseKey, "", "PC Principal PGP");
        WinRegistry::writeString(baseKey, "Icon", appPath + ",0");
        WinRegistry::writeString(baseKey + "\\command", "", 
            QString("\"%1\" \"--context=%2\" \"%3\"").arg(appPath).arg("%1"));
        
        // Submenu items
        QString encryptKey = baseKey + "\\shell\\encrypt";
        WinRegistry::writeString(encryptKey, "", "Encrypt...");
        WinRegistry::writeString(encryptKey + "\\command", "", 
            QString("\"%1\" \"--encrypt\" \"%2\"").arg(appPath).arg("%1"));
        
        QString decryptKey = baseKey + "\\shell\\decrypt";
        WinRegistry::writeString(decryptKey, "", "Decrypt");
        WinRegistry::writeString(decryptKey + "\\command", "", 
            QString("\"%1\" \"--decrypt\" \"%2\"").arg(appPath).arg("%1"));
        
        QString signKey = baseKey + "\\shell\\sign";
        WinRegistry::writeString(signKey, "", "Sign");
        WinRegistry::writeString(signKey + "\\command", "", 
            QString("\"%1\" \"--sign\" \"%2\"").arg(appPath).arg("%1"));
        
        QString verifyKey = baseKey + "\\shell\\verify";
        WinRegistry::writeString(verifyKey, "", "Verify Signature");
        WinRegistry::writeString(verifyKey + "\\command", "", 
            QString("\"%1\" \"--verify\" \"%2\"").arg(appPath).arg("%1"));
        
        m_registered = true;
        return true;
    } catch (...) {
        return false;
    }
}

bool ContextMenuHandler::unregisterShellExtension() {
    try {
        QString baseKey = "HKEY_CURRENT_USER\\Software\\Classes\\*\\shell\\PCPrincipalPGP";
        WinRegistry::deleteKey(baseKey);
        
        m_registered = false;
        return true;
    } catch (...) {
        return false;
    }
}
#endif

QString ContextMenuHandler::getMenuText(const QString& action) {
    if (action == "encrypt") return "Encrypt...";
    if (action == "decrypt") return "Decrypt";
    if (action == "sign") return "Sign";
    if (action == "verify") return "Verify Signature";
    if (action == "import") return "Import Key";
    return action;
}

QString ContextMenuHandler::getCommandLine(const QString& action) {
    return QString("--%1").arg(action);
}

// WinRegistry implementation
#ifdef Q_OS_WIN
bool WinRegistry::writeString(const QString& key, const QString& valueName, 
                               const QString& value) {
    HKEY hKey;
    QString path = key;
    path = path.replace("HKEY_CURRENT_USER\\", "");
    path = path.replace("HKEY_LOCAL_MACHINE\\", "");
    
    LONG result = RegCreateKeyExW(HKEY_CURRENT_USER, 
        reinterpret_cast<const wchar_t*>(path.utf16()),
        0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr);
    
    if (result != ERROR_SUCCESS) {
        return false;
    }
    
    result = RegSetValueExW(hKey, 
        valueName.isEmpty() ? nullptr : reinterpret_cast<const wchar_t*>(valueName.utf16()),
        0, REG_SZ,
        reinterpret_cast<const BYTE*>(value.utf16()),
        (value.length() + 1) * sizeof(wchar_t));
    
    RegCloseKey(hKey);
    return result == ERROR_SUCCESS;
}

bool WinRegistry::writeDword(const QString& key, const QString& valueName, 
                              uint32_t value) {
    HKEY hKey;
    QString path = key;
    path = path.replace("HKEY_CURRENT_USER\\", "");
    
    LONG result = RegCreateKeyExW(HKEY_CURRENT_USER,
        reinterpret_cast<const wchar_t*>(path.utf16()),
        0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr);
    
    if (result != ERROR_SUCCESS) {
        return false;
    }
    
    result = RegSetValueExW(hKey,
        valueName.isEmpty() ? nullptr : reinterpret_cast<const wchar_t*>(valueName.utf16()),
        0, REG_DWORD,
        reinterpret_cast<const BYTE*>(&value), sizeof(value));
    
    RegCloseKey(hKey);
    return result == ERROR_SUCCESS;
}

QString WinRegistry::readString(const QString& key, const QString& valueName) {
    HKEY hKey;
    QString path = key;
    path = path.replace("HKEY_CURRENT_USER\\", "");
    
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER,
        reinterpret_cast<const wchar_t*>(path.utf16()),
        0, KEY_READ, &hKey);
    
    if (result != ERROR_SUCCESS) {
        return QString();
    }
    
    wchar_t buffer[1024];
    DWORD bufferSize = sizeof(buffer);
    DWORD type;
    
    result = RegQueryValueExW(hKey,
        valueName.isEmpty() ? nullptr : reinterpret_cast<const wchar_t*>(valueName.utf16()),
        nullptr, &type, reinterpret_cast<BYTE*>(buffer), &bufferSize);
    
    RegCloseKey(hKey);
    
    if (result == ERROR_SUCCESS && type == REG_SZ) {
        return QString::fromWCharArray(buffer);
    }
    
    return QString();
}

bool WinRegistry::deleteKey(const QString& key) {
    QString path = key;
    path = path.replace("HKEY_CURRENT_USER\\", "");
    
    LONG result = RegDeleteTreeW(HKEY_CURRENT_USER,
        reinterpret_cast<const wchar_t*>(path.utf16()));
    
    return result == ERROR_SUCCESS;
}

bool WinRegistry::keyExists(const QString& key) {
    HKEY hKey;
    QString path = key;
    path = path.replace("HKEY_CURRENT_USER\\", "");
    
    LONG result = RegOpenKeyExW(HKEY_CURRENT_USER,
        reinterpret_cast<const wchar_t*>(path.utf16()),
        0, KEY_READ, &hKey);
    
    if (result == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return true;
    }
    
    return false;
}
#else
// Stub implementations for non-Windows platforms
bool WinRegistry::writeString(const QString& key, const QString& valueName, 
                               const QString& value) { Q_UNUSED(key) Q_UNUSED(valueName) Q_UNUSED(value) return false; }
bool WinRegistry::writeDword(const QString& key, const QString& valueName, 
                              uint32_t value) { Q_UNUSED(key) Q_UNUSED(valueName) Q_UNUSED(value) return false; }
QString WinRegistry::readString(const QString& key, const QString& valueName) { Q_UNUSED(key) Q_UNUSED(valueName) return QString(); }
bool WinRegistry::deleteKey(const QString& key) { Q_UNUSED(key) return false; }
bool WinRegistry::keyExists(const QString& key) { Q_UNUSED(key) return false; }
#endif

} // namespace PCPGP
