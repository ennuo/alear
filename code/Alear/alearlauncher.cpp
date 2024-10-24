#include "alearlauncher.h"

#include <hook.h>
#include <vector.h>
#include <MMString.h>
#include <ppcasm.h>

#include <gfxcore.h>
#include <padinput.h>
#include <cell/DebugLog.h>

#include <ResourceGFXFont.h>
#include <View.h>
#include <OverlayUI.h>
#include <LoadingScreen.h>
#include <gooey/GooeyNodeManager.h>

enum ENodeType {
    E_LEVEL,
    E_FOLDER
};

class CBaseNode {
protected:
    inline CBaseNode(const wchar_t* name, ENodeType type)
    {
        Name = name;
        Type = type;
    }
public:
    inline const wchar_t* GetName() { return Name; }
    inline ENodeType GetNodeType() { return Type; }
protected:
    const wchar_t* Name;
    ENodeType Type;
};


class CLevelNode : public CBaseNode {
public:
    inline CLevelNode(const wchar_t* name, const char* level_path) : CBaseNode(name, E_LEVEL)
    {
        LevelPath = level_path;
        Played = false;
    }
private:
    const char* LevelPath;
    bool Played;
};

class CFolderNode : public CBaseNode {
public:
    inline CFolderNode(const wchar_t* name) : CBaseNode(name, E_FOLDER), Folders(), Levels()
    {

    }
public:
    CVector<CFolderNode> Folders;
    CVector<CLevelNode> Levels;
};

CFolderNode gRootFolder(NULL);
CGooeyNodeManager* gLauncherGooey;

void UpdateLauncherUI(CGooeyNodeManager* manager)
{
    if (!manager->StartFrameNamed(1000)) return;

    manager->SetFrameSizing(gResX, gResY);
    manager->SetFrameLayoutMode(LM_JUSTIFY_START, LM_JUSTIFY_START);
    if (manager->StartFrame())
    {
        manager->SetFrameSizing(SizingBehaviour::Contents(), SizingBehaviour::Relative(0.1f));



        manager->EndFrame();
    }

    if (manager->StartFrame())
    {
        manager->SetFrameSizing(SizingBehaviour::Contents(), SizingBehaviour::Relative(0.8f));
        manager->SetFrameLayoutMode(LM_CENTERED, LM_CENTERED);
        manager->SetFrameDefaultChildSpacing(32.0f, 32.0f);

        for (int i = 0; i < 30; ++i)
        {
            manager->DoInline(L"Button", GTS_SMALL_PRINT, STATE_NORMAL, NULL, 256);
            manager->DoBreak();
        }

        manager->EndFrame();
    }

    if (manager->StartFrame())
    {
        manager->SetFrameSizing(SizingBehaviour::Contents(), SizingBehaviour::Relative(0.1f));



        manager->EndFrame();
    }

    manager->EndFrame();
}

void UpdateAlearLauncher()
{
    if (gLauncherGooey == NULL) 
        gLauncherGooey = new CGooeyNodeManager(INPUT_ALL, E_GOOEY_MANAGER_NETWORK_NONE);
    
    // gLauncherGooey->RenderToTexture(gResX, gResY, gResX, gResY, false, false);
}