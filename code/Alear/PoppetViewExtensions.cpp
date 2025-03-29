#include "AlearShared.h"
#include "AlearHooks.h"

#include <algorithm>

#include <ResourceLocalProfile.h>
#include <ResourceGame.h>
#include <Poppet.h>
#include <PoppetEnums.inl>
#include <InventoryItem.h>
#include <InventoryView.h>
#include <network/NetworkUtilsNP.h>

#include "customization/Emotes.h"

const u32 gUserObjectMask = 0x40100480;
const u32 gPodMask = E_TYPE_USER_POD | E_TYPE_POD_TOOL;

bool IsPlayerCustomizationTool(u32 tool_type)
{
    return 
        tool_type == TOOL_COSTUME_RESET ||
        tool_type == TOOL_COSTUME_RANDOM ||
        tool_type == TOOL_COSTUME_SAVE ||
        tool_type == TOOL_COSTUME_WASH;
}

bool IsToolMatch(CInventoryView* view, u32 tool_type)
{
    bool edit = gGame->EditMode;
    u32 type = view->Descriptor.Type;
    u32 subtype = view->Descriptor.SubType;

    switch (tool_type)
    {
        case TOOL_COSTUME_RESET:
        case TOOL_COSTUME_RANDOM:
        case TOOL_COSTUME_SAVE:
        case TOOL_COSTUME_WASH:
        {
            return (type & (E_TYPE_COSTUME | E_TYPE_COSTUME_MATERIAL | E_TYPE_USER_COSTUME)) != 0;
        }

        case TOOL_STICKER_PICK:
        {
            return (type & (E_TYPE_STICKER | E_TYPE_DECORATION | E_TYPE_USER_STICKER | E_TYPE_EYETOY)) != 0;
        }

        case TOOL_DECORATION_TAKE_PHOTO:
        {
            return (type & (E_TYPE_STICKER | E_TYPE_DECORATION | E_TYPE_USER_STICKER | E_TYPE_EYETOY)) != 0;
        }

        case TOOL_DELETE_COMMUNITY_STICKERS:
        {
            return (type & E_TYPE_USER_STICKER) != 0 && (subtype & E_SUBTYPE_MADE_BY_OTHERS) != 0;
        }

        case TOOL_POD_RESET:
        case TOOL_POD_SAVE:
        {
            return (type & E_TYPE_USER_POD) != 0;
        }

        case TOOL_SHAPE_TAKE_PLAN:
        {
            return (type & (E_TYPE_PRIMITIVE_MATERIAL | E_TYPE_READYMADE | E_TYPE_USER_OBJECT | E_TYPE_TOOL)) != 0;
        }

        case TOOL_DELETE_COMMUNITY_OBJECTS:
        {
            return (type & E_TYPE_USER_OBJECT) != 0 && (subtype & E_SUBTYPE_MADE_BY_OTHERS) != 0;
        }

        case TOOL_DECORATION_IMPORT_PHOTO:
        {
            return (type & E_TYPE_USER_STICKER) != 0;
        }
        
        case TOOL_STICKER_WASH:
        {
            return (type & E_TYPE_STICKER | E_TYPE_DECORATION | E_TYPE_USER_STICKER | E_TYPE_EYETOY) != 0 && edit;
        }

        case TOOL_SHAPE_ELECTRIFY:
        case TOOL_SHAPE_BURNINATE:
        case TOOL_SHAPE_ICE:
        case TOOL_SHAPE_GAS:
        case TOOL_SHAPE_UNLETHAL:
        case TOOL_SHAPE_PLASMA:
        case TOOL_SHAPE_CRUSH:
        case TOOL_SHAPE_DROWNED:
        case TOOL_SHAPE_SPIKE:
        case TOOL_UNPHYSICS:
        case TOOL_SHAPE_VERTEX_EDIT:
        {
            return (type & E_TYPE_TOOL) != 0 && edit;
        }

        case TOOL_SHAPE_FLOOD_FILL:
        case TOOL_DOT_TO_DOT:
        {
            return (type & (E_TYPE_TOOL | E_TYPE_FLOOD_FILL)) != 0 && edit;
        }
        
        case TOOL_EYEDROPPER:
        {
            return (type & (E_TYPE_TOOL | E_TYPE_PRIMITIVE_MATERIAL)) != 0 && (type & (E_TYPE_TOOL | E_TYPE_PRIMITIVE_MATERIAL)) != 0 && edit;
        }

        default: return false;
    }
}

bool CustomItemMatch(CInventoryView* view, CInventoryItem* item, NetworkPlayerID* owner)
{
    if (view->Descriptor.Type == gEmotesCustomId)
    {
        return IsEmoteItem(item->Plan.GetGUID()) || IsPlayerCustomizationTool(item->Details.ToolType);
    }
    
    if (view->Descriptor.Type == gUsedItemsCustomId)
    {
        u32 tool = item->Details.ToolType;

        if (tool != TOOL_NOT_A_TOOL)
        {
            if (tool == TOOL_DELETE_COMMUNITY_OBJECTS || tool == TOOL_DELETE_COMMUNITY_STICKERS || tool == TOOL_UNPHYSICS) return false;
            return true;
        }

        return gUsedPlanDescriptors.find(item->Plan) != gUsedPlanDescriptors.end();
    }

    CGUID item_guid = item->Plan.GetGUID();
    // if (IsEmoteItem(item_guid)) return false;

    u32 item_type = item->Details.Type;
    u32 view_type = view->Descriptor.Type;

    u32 item_subtype = item->Details.SubType;
    u32 view_subtype = view->Descriptor.SubType;

    if (view->HeartedOnly && (item->Flags & E_IIF_HEARTED) == 0) return false;
    if (item->Details.ToolType != TOOL_NOT_A_TOOL) return IsToolMatch(view, item->Details.ToolType);

    if (item_guid == 0x12981 || item_guid == 0x15351) return false;
    if ((item_type & view_type) == 0) return false;

    if (view_type == gPodMask)
    {
        if ((item_type & E_TYPE_POD_TOOL) != 0) return true;

        if ((item_subtype & E_SUBTYPE_POD_CONTROLLER) != 0 || (item_subtype & E_SUBTYPE_POD_MESH) != 0)
            return (item_subtype & view_subtype) != 0;
        
        return (view_subtype & E_SUBTYPE_POD_CONTROLLER) == 0 && (view_subtype & E_SUBTYPE_POD_MESH) == 0;
    }

    if ((item_type & gUserObjectMask) == 0)
    {
        if (item_subtype != 0)
            return (item_subtype & view_subtype) != 0;

        return true;
    }
    
    bool owns = item->Details.ToolType == TOOL_NOT_A_TOOL 
                ? item->Details.IsCreatedBy(owner)
                : (item_subtype & E_SUBTYPE_MADE_BY_OTHERS) == 0;
    
    if ((view_subtype & E_SUBTYPE_MADE_BY_ME) != 0 && owns)
        return true;
    
    if ((view_subtype & E_SUBTYPE_MADE_BY_OTHERS) != 0)
        return !owns;

    return false;
}

struct BaseGroupBy {
    RLocalProfile* LocalProfile;
    CInventoryView* Page;
};

struct SortByName
{
    bool operator()(CInventoryView::SInventoryItemData& a, CInventoryView::SInventoryItemData& z) const
    {
        const tchar_t* name_a = a.Item->Details.TranslateName();
        const tchar_t* name_b = z.Item->Details.TranslateName();

        int cmp = wcscmp((const wchar_t*)name_a, (const wchar_t*)name_b);
        if (cmp == 0) return a.Item->UID < z.Item->UID;

        return cmp < 0;
    }
};

struct SortByArtist
{
    bool operator()(CInventoryView::SInventoryItemData& a, CInventoryView::SInventoryItemData& z) const
    {
        if (a.Item->Details.CustomSortKey != z.Item->Details.CustomSortKey)
        {
            return a.Item->Details.CustomSortKey > z.Item->Details.CustomSortKey;
        }

        return a.Item->UID > z.Item->UID;
    }
};

void DoArtistSort(CInventoryView* view)
{
    std::sort(view->PageData.begin(), view->PageData.end(), SortByArtist());

    int key = 0xDEADBEEF;
    for (int i = 0; i < view->PageData.size(); ++i)
    {
        CInventoryItem* item = view->PageData[i].Item;
        if (item->Details.CustomSortKey != key)
        {
            SSortTermBoundary boundary;
            boundary.Index = i;
            MultiByteToWChar(boundary.Name, "No Artist", NULL);
            view->SortTermBoundaries.push_back(boundary);

            key = item->Details.CustomSortKey;
        }

    }
}

struct SortByPreference
{
    bool operator()(CInventoryView::SInventoryItemData& a, CInventoryView::SInventoryItemData& z) const
    {
        return a.Item->Details.NameTranslationTag == 1244778990;
    }
};

void DoPreferenceSort(CInventoryView* view)
{
    std::sort(view->PageData.begin(), view->PageData.end(), SortByPreference());

    SSortTermBoundary boundary;
    MultiByteToWChar(boundary.Name, "Good", NULL);
    view->SortTermBoundaries.push_back(boundary);

    boundary.Index = 1;
    MultiByteToWChar(boundary.Name, "Shit", NULL);
    view->SortTermBoundaries.push_back(boundary);
}

bool IsAlphabeticalCategory(CInventoryView* view)
{
    if (view->HeartedOnly || (view->Descriptor.SubType & E_SUBTYPE_MADE_BY_OTHERS) != 0) return false;
    if (view->Descriptor.Type & E_TYPE_USER_STICKER) return false;
    if (view->Descriptor.Type & E_TYPE_USER_OBJECT) return false;
    
    if (view->Descriptor.Type == E_TYPE_MUSIC || view->Descriptor.Type == E_TYPE_SOUND) return true;
    if (view->Descriptor.Type & (E_TYPE_COSTUME | E_TYPE_COSTUME_MATERIAL)) return true;
    if (view->Descriptor.Type & E_TYPE_READYMADE) return true;
    if (view->Descriptor.Type & E_TYPE_STICKER) return true;
    if (view->Descriptor.Type & E_TYPE_DECORATION) return true;
    if (view->Descriptor.Type & E_TYPE_PRIMITIVE_MATERIAL) return true;
    if (view->Descriptor.Type & E_TYPE_BACKGROUND) return true;

    return false;
}

bool HackIsToolBoundary(CInventoryView* view, SSortTermBoundary& boundary)
{
    CInventoryView::SInventoryItemData& data = view->PageData[boundary.Index];
    if (data.Item == NULL) return false;
    return data.Item->Details.ToolType != TOOL_NOT_A_TOOL;
}

void SortBoundariesAlphabetically(CInventoryView* view)
{
    if (!IsAlphabeticalCategory(view)) return;

    for (int i = 0; i < view->SortTermBoundaries.size(); ++i)
    {
        SSortTermBoundary& boundary = view->SortTermBoundaries[i];
        
        if (HackIsToolBoundary(view, boundary)) continue;

        int end = view->PageData.size();
        if (i + 1 < view->SortTermBoundaries.size())
            end = view->SortTermBoundaries[i + 1].Index;

        std::sort(view->PageData.begin() + boundary.Index, view->PageData.begin() + end, SortByName());
    }
}

void AttachCustomSortModes()
{
    MH_Poke32(0x000c2ca0, 0x2b990000 + (NUM_SORT_MODES - 1));

    // Initialise the switch table with the offsets to the invalid resource type case
    const int SWITCH_LABEL = 0x000c2cc4;
    const int NOP_LABEL = 0x000c2ce0;
    const int LABEL_COUNT = 0x7;
    static s32 TABLE[NUM_SORT_MODES];
    for (int i = 0; i < NUM_SORT_MODES; ++i)
        TABLE[i] = NOP_LABEL - (u32)TABLE;

    // Copy the old switch case into our new table and replace the offsets.
    MH_Read(SWITCH_LABEL, TABLE, LABEL_COUNT * sizeof(s32));
    for (int i = 0; i < LABEL_COUNT; ++i)
    {
        s32 target = SWITCH_LABEL + TABLE[i] - (u32)TABLE;
        TABLE[i] = target;
    }

    // Using the location sort for the artist sort
    TABLE[SORT_ARTIST] = (u32)&_global_artist_hook - (u32)TABLE;
    TABLE[SORT_MEME] = (u32)&_global_pref_hook - (u32)TABLE;

    // Switch out the pointer to the switch case in the TOC
    MH_Poke32(0x0091e06c, (u32)TABLE);
}
