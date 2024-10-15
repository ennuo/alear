#include "portal.h"
#include "cwlib.h"
#include "ppcasm.h"
#include "vector.h"
#include "hook.h"
#include "Camera.h"
#include "View.h"

#include "ResourceGame.h"
#include "ResourceLevel.h"
#include "ResourceGfxMaterial.h"

#include "SceneGraph.h"
#include "GFXApi.h"
#include "Clock.h"

#include <cell/gcm.h>

CPortalManager g_PortalManager;
CMemoryTexture CPortalManager::s_PortalTexture;
bool g_TextureInitialized = false;

MH_DefineFunc(CaptureRenderGoodBit, 0x001b8c60, TOC0, void, CVector<PWorld*>& world_list, CCamera* camera, u32 flags, bool square_mip_chain, bool gpu_downsample);
MH_DefineFunc(CopyRenderToMemory, 0x001bbaac, TOC0, void, u32 xsize, u32 ysize, u32 buffer_offset, u32 buffer_pitch, bool always_use_podcomputer, u32 transfer_type, bool square_mip_chain);

CPortalManager::CPortalManager()
{
    
}

void OnWorldRenderUpdate()
{
    RLevel* level = gGame->Level;
    if (level == NULL) return;
    CThing* thing = level->WorldThing;
    if (thing == NULL) return;
    PWorld* world = thing->GetPWorld();
    if (world == NULL) return;

    CMemoryTexture& texture = CPortalManager::s_PortalTexture;
    CellGcmTexture& gcm = texture.Texture;

    if (texture.Handle.Pool == NULL)
    {
        DebugLog("Allocating CPortalManager::s_PortalTexture...\n");
        texture.Create(gMemPools[MEM_POOL_LOCAL], 0x1, 640, 360, 1, 1, 0, 1, 1); // glFormat=1, should be A8R8G8B8
        g_TextureInitialized = true;
    }

    // CRawVector causes a crash, look into that
    // CVector<PWorld*> world_list;
    // world_list.push_back(world);
    // CaptureRenderGoodBit(world_list, gView.Camera, 4, false, true);
    // CopyRenderToMemory(gcm.width, gcm.height, gcm.offset, gcm.pitch, false, CELL_GCM_TRANSFER_LOCAL_TO_LOCAL, false);
}

extern "C" void _portal_hook_naked();
void AlearInitPortalHook()
{
    // MH_Poke32(0x00206174, B(&_portal_hook_naked, 0x00206174));
}
