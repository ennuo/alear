#ifndef ALEAR_OPT_UI_H
#define ALEAR_OPT_UI_H

#include <filepath.h>
#include <GuidHash.h>

void InitAlearOptUiHooks();
void OnDatabaseFileChanged(CFilePath& fp);
void ReloadPendingDatabases();
void ProcessStartMenuNotifications();

class CFileDB;
void ReloadModifiedResources();
void ReloadModifiedResources(CFileDB* database, CFileDB* old_database);
void NotifyStartMenu(u32 flags);

enum EStartMenuNotification {
    NOTIFICATION_REFRESH_UI = 1,
    NOTIFICATION_RELOAD_FILESYSTEM = 2
};

enum ENodeType {
    E_LEVEL,
    E_FILE,
    E_FOLDER
};

class CBaseNode {
public:
    virtual ~CBaseNode();
protected:
    CBaseNode(const char* name, ENodeType type);
public:
    void ToggleDelete();
public:
    inline u32 GetNameHash() { return NameHash; }
    inline wchar_t* GetName() { return Name.c_str(); }
    inline ENodeType GetNodeType() { return Type; }
    inline CBaseNode* GetParent() { return Parent; }
    inline bool IsInRecyclingBin() { return InRecyclingBin; }
protected:
    MMString<wchar_t> Name;
    u32 NameHash;
    ENodeType Type;
    CBaseNode* Parent;
    bool InRecyclingBin;
};

class CFileNode : public CBaseNode {
friend class CFolderNode;
public:
    inline CFileNode(const char* name) : CBaseNode(name, E_FILE) {}
    inline CFileNode(const char* name, CGUID guid) : CBaseNode(name, E_FILE), GUID(guid) {}
public:
    inline CGUID GetGUID() { return GUID; }
protected:
    CGUID GUID;
};

class CFolderNode : public CBaseNode {
friend class CFileNode;
public:
    CFolderNode(const char* name);
    ~CFolderNode();
public:
    void DestroyHierarchy();
public:
    CFolderNode* AddFolder(const char* name);
    CBaseNode* AddFile(const char* name, CGUID guid);
public:
    CRawVector<CFolderNode*> Folders;
    CRawVector<CBaseNode*> Files;
    u64 LastSelectedUID;
};


#endif // ALEAR_OPT_UI_H