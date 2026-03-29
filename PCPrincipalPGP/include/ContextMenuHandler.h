#ifndef CONTEXTMENUHANDLER_H
#define CONTEXTMENUHANDLER_H

#include <QObject>
#include <QString>
#include <QStringList>

namespace PCPGP {

class PGPManager;
class ProfileManager;

// Handles Windows context menu integration
class ContextMenuHandler : public QObject {
    Q_OBJECT

public:
    explicit ContextMenuHandler(PGPManager* pgpManager, 
                                ProfileManager* profileManager,
                                QObject* parent = nullptr);
    ~ContextMenuHandler();
    
    // Register/unregister context menu
    bool registerContextMenu();
    bool unregisterContextMenu();
    bool isContextMenuRegistered() const;
    
    // Handle context menu commands
    bool handleCommand(const QString& command, const QStringList& files);
    
    // Context menu actions
    bool encryptFile(const QString& filePath);
    bool encryptFileTo(const QString& filePath, const QString& profileName);
    bool decryptFile(const QString& filePath);
    bool signFile(const QString& filePath);
    bool verifyFile(const QString& filePath);
    bool importKey(const QString& filePath);
    bool getPublicKey(const QString& keyId);

signals:
    void actionTriggered(const QString& action, const QString& file);
    void errorOccurred(const QString& error);

private:
    bool registerShellExtension();
    bool unregisterShellExtension();
    QString getMenuText(const QString& action);
    QString getCommandLine(const QString& action);
    
    PGPManager* m_pgpManager;
    ProfileManager* m_profileManager;
    bool m_registered = false;
};

// Windows registry helper
class WinRegistry {
public:
    static bool writeString(const QString& key, const QString& valueName, 
                            const QString& value);
    static bool writeDword(const QString& key, const QString& valueName, 
                           uint32_t value);
    static QString readString(const QString& key, const QString& valueName);
    static bool deleteKey(const QString& key);
    static bool keyExists(const QString& key);
};

} // namespace PCPGP

#endif // CONTEXTMENUHANDLER_H
