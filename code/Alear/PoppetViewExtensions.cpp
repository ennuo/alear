#include "AlearShared.h"
#include "AlearHooks.h"
#include "AlearConfig.h"

#include <algorithm>

#include <ResourceLocalProfile.h>
#include <ResourceGame.h>
#include <Poppet.h>
#include <PoppetEnums.inl>
#include <Translate.h>
#include <InventoryItem.h>
#include <InventoryView.h>
#include <network/NetworkUtilsNP.h>

#include "customization/Emotes.h"

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

    if (type == E_TYPE_PLAYER_COLOUR) return false;
    
    switch (tool_type)
    {
        case TOOL_COSTUME_RESET:
        case TOOL_COSTUME_RANDOM:
        case TOOL_COSTUME_SAVE:
        case TOOL_COSTUME_WASH:
        {
            return (type & (E_TYPE_COSTUME_TOOL)) != 0 || (subtype & E_SUBTYPE_ANIMATION_STYLE) != 0;
        }
        
        case TOOL_STICKER_PICK:
        {
            return (type & (E_TYPE_STICKER | E_TYPE_DECORATION | E_TYPE_USER_STICKER | E_TYPE_PAINT | E_TYPE_EYETOY)) != 0;
        }

        case TOOL_DECORATION_TAKE_PHOTO:
        {
            return (type & (E_TYPE_STICKER | E_TYPE_DECORATION | E_TYPE_USER_STICKER | E_TYPE_PAINT | E_TYPE_EYETOY)) != 0;
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
            return (type & (E_TYPE_TOOL | E_TYPE_PLAN_TOOL)) != 0;
        }
        case TOOL_EYEDROPPER:
        {
            return ((type & (E_TYPE_TOOL)) != 0 && edit) || ((type & (E_TYPE_PLAN_TOOL)) != 0 && (type & (E_TYPE_PRIMITIVE_MATERIAL)) != 0 && edit);
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
        case TOOL_STICKER_CUTTER:
        {
            return (type & (E_TYPE_TOOL | E_TYPE_STICKER | E_TYPE_DECORATION | E_TYPE_USER_STICKER | E_TYPE_PAINT | E_TYPE_EYETOY)) != 0 && edit;
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
        case TOOL_UV_EDIT:
        case TOOL_GLUE:
        case TOOL_CURSOR:
        case TOOL_SHAPE_VERTEX_EDIT:
        case TOOL_SLICE_N_DICE:
        {
            return (type & E_TYPE_TOOL) != 0 && edit;
        }
        
        case TOOL_RANDOM_STICKER:
        {
            return (type & E_TYPE_STICKER) != 0;
        }
        case TOOL_RANDOM_DECORATION:
        {
            return (type & E_TYPE_DECORATION) != 0;
        }
        case TOOL_RANDOM_MATERIAL:
        {
            return (type & E_TYPE_PRIMITIVE_MATERIAL) != 0 && edit;
        }
        case TOOL_RANDOM_OBJECT:
        {
            return (type & E_TYPE_READYMADE) != 0 && edit;
        }
        
        case TOOL_POPIT_GRADIENT:
        case TOOL_MESH_CAPTURE:
        case TOOL_EXPLOSION:
        case TOOL_GENEALOGY:
        {
            return (type & E_TYPE_TOOL) != 0 && edit;
        }

        case TOOL_SHAPE_FLOOD_FILL:
        case TOOL_DOT_TO_DOT:
        {
            return (type & (E_TYPE_TOOL | E_TYPE_FLOOD_FILL)) != 0 && edit;
        }

        case TOOL_MORPH_RESET:
        case TOOL_MORPH_SAVE:
        case TOOL_MORPH_EDIT:
        {
            return (subtype & E_SUBTYPE_MORPH) != 0;
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
            if (
                tool == TOOL_DELETE_COMMUNITY_OBJECTS || 
                tool == TOOL_DELETE_COMMUNITY_STICKERS || 
                tool == TOOL_RANDOM_STICKER ||
                tool == TOOL_RANDOM_DECORATION ||
                tool == TOOL_RANDOM_MATERIAL ||
                tool == TOOL_RANDOM_OBJECT
            ) 
            {
                return false;
            }
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

    if (view->HeartedOnly && !item->IsHearted()) return false;
    if (item->Details.ToolType != TOOL_NOT_A_TOOL) return IsToolMatch(view, item->Details.ToolType);

    if (item_guid == 0x12981 || item_guid == 0x15351) return false;
    if ((item_type & view_type) == 0) return false;

    if ((item_type & E_TYPE_READYMADE) != 0)
    {
        if ((item_type & E_TYPE_TOOL) != 0) return true;

        if(!gUseToysPage)
        {
            return (item_type & view_type) != 0;
        }
        if (view_subtype == E_SUBTYPE_TOYS)
            return (item_subtype & E_SUBTYPE_TOYS) != 0;
        return (item_subtype & E_SUBTYPE_TOYS) == 0;
    }

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
                ? item->Details.IsCreatedBy(*owner)
                : (item_subtype & E_SUBTYPE_MADE_BY_OTHERS) == 0;
    
    if ((view_subtype & E_SUBTYPE_MADE_BY_ME) != 0 && owns)
        return true;
    
    if ((view_subtype & E_SUBTYPE_MADE_BY_OTHERS) != 0)
        return !owns;

    return false;
}

int MapInventoryItemToType(int type, int subtype)
{
    if (type == E_TYPE_TOOL)
        return 0;
    if (type == E_TYPE_PLAYER_COLOUR)
        return 1;
    if (type == E_TYPE_USER_POD)
        return 2;
    if (type == E_TYPE_BACKGROUND)
        return 3;
    if (type == E_TYPE_PRIMITIVE_SHAPE)
        return 4;
    if (type == E_TYPE_PRIMITIVE_MATERIAL)
        return 5;
    if (type == E_TYPE_READYMADE)
    {
        if (subtype == E_SUBTYPE_TOYS)
        {
            return 7;
        }
        return 6;
    }
    if (type == E_TYPE_USER_OBJECT)
    {
        if (type == E_SUBTYPE_MADE_BY_ME)
        {   
            return 8;
        }
        return 9;
    }
    if (type == E_TYPE_JOINT)
        return 10;
    if (type == E_TYPE_GADGET)
        return 11;
    if (type == E_TYPE_GAMEPLAY_KIT)
        return 12;
    if (type == E_TYPE_MUSIC)
        return 13;
    if (type == E_TYPE_SOUND)
        return 14;
    if (type == E_TYPE_COSTUME_MATERIAL || type == E_TYPE_COSTUME)
        return 15;
    if (type == E_TYPE_USER_COSTUME)
        return 16;
    if (type == E_TYPE_STICKER)
        return 17;
    if (type == E_TYPE_DECORATION)
        return 18;
    if (type == E_TYPE_PAINT)
        return 19;
    if (type == E_TYPE_EYETOY)
        return 20;
    if (type == E_TYPE_USER_STICKER)
    {
        if (type == E_SUBTYPE_MADE_BY_ME)
        {
            return 21;
        }
        return 22;
    }
    return 23;
}

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

struct SortByType
{
    bool operator()(CInventoryView::SInventoryItemData& a, CInventoryView::SInventoryItemData& z) const
    {
        int a_type = MapInventoryItemToType(a.Item->Details.Type, a.Item->Details.SubType);
        int z_type = MapInventoryItemToType(z.Item->Details.Type, z.Item->Details.SubType);

        if (a_type == z_type)
            return a.Item->UID < z.Item->UID;
        return a_type > z_type;
    }
};

struct SortByCurSortIndex
{
    bool operator()(const CInventoryView::SInventoryItemData& a, const CInventoryView::SInventoryItemData& z) const
    {
        if (a.CurSortIndex == z.CurSortIndex)
            return a.Item->UID < z.Item->UID;
        return a.CurSortIndex > z.CurSortIndex;
    }
};

void DoTypeSort(CInventoryView* view)
{
    std::sort(view->PageData.begin(), view->PageData.end(), SortByType());

    int key = 0xDEADBEEF;
    for (int i = 0; i < view->PageData.size(); ++i)
    {
        CInventoryItem* item = view->PageData[i].Item;
        int item_key = MapInventoryItemToType(item->Details.GetType(), item->Details.GetSubType());
        if (item_key != key)
        {
            const char* lams_key;
            switch (item->Details.GetType())
            {
                case E_TYPE_TOOL: lams_key = "POPIT_FUNCTIONS"; break;
                case E_TYPE_PLAYER_COLOUR: lams_key = "POPIT_PAGE_PLAYER_COLOUR"; break;
                case E_TYPE_USER_POD: lams_key = "TG_My_Pods"; break;
                case E_TYPE_BACKGROUND: lams_key = "TG_Backgrounds"; break;
                case E_TYPE_PRIMITIVE_SHAPE: lams_key = "TG_Shapes"; break;
                case E_TYPE_PRIMITIVE_MATERIAL: lams_key = "TG_Materials"; break;
                case E_TYPE_READYMADE:
                {
                    if (item->Details.GetSubType() == E_SUBTYPE_TOYS)
                        lams_key = "TG_Toys";
                    else
                        lams_key = "TG_Objects";
                    break;
                }
                case E_TYPE_USER_OBJECT:
                {
                    if (item->Details.GetSubType() == E_SUBTYPE_MADE_BY_ME)
                        lams_key = "POPIT_MY_PLANS";
                    else
                        lams_key = "POPIT_PAGE_COMMUNITY_OBJECTS";
                    break;
                }
                case E_TYPE_JOINT: lams_key = "TG_Links"; break;
                case E_TYPE_GADGET: lams_key = "POPIT_PAGE_GADGETS"; break;
                case E_TYPE_GAMEPLAY_KIT: lams_key = "TG_Gameplay_Kits"; break;
                case E_TYPE_MUSIC: lams_key = "CATEGORY_MUSIC"; break;
                case E_TYPE_SOUND: lams_key = "CATEGORY_SOUND_OBJECTS"; break;
                case E_TYPE_COSTUME_MATERIAL:
                case E_TYPE_COSTUME:
                case E_TYPE_USER_COSTUME:
                    lams_key = "TG_My_Costumes"; 
                    break;
                case E_TYPE_STICKER: lams_key = "TG_Stickers"; break;
                case E_TYPE_DECORATION: lams_key = "TG_Decorations"; break;
                case E_TYPE_PAINT: lams_key = "TG_Paint"; break;
                case E_TYPE_EYETOY: lams_key = "TG_Eyetoy"; break;
                case E_TYPE_USER_STICKER:
                {
                    if (item->Details.GetSubType() == E_SUBTYPE_MADE_BY_ME)
                        lams_key = "POPIT_MY_PHOTOS";
                    else
                        lams_key = "POPIT_PAGE_COMMUNITY_PHOTOS";
                    break;
                }
            }

            view->SortTermBoundaries.resize(view->SortTermBoundaries.size() + 1);
            SSortTermBoundary& boundary = view->SortTermBoundaries.back();
            boundary.Index = i;
            if (key != NULL)
                Translate(boundary.Name, lams_key);
            
            key = item_key;
        }
    }
}

void DoSubcategorySort(RLocalProfile* profile, CInventoryView* view)
{
    for (u32 i = 0; i < view->PageData.size(); ++i)
    {
        CInventoryView::SInventoryItemData& item = view->PageData[i];
        CInventoryItemDetails& details = item.Item->Details;
        if (details.IsATool()) item.CurSortIndex = -2;
        else item.CurSortIndex = profile->StringTable.GetSortIndex(details.GetSubcategoryIndex());
    }

    std::sort(view->PageData.begin(), view->PageData.end(), SortByCurSortIndex());

    int key = 0xDEADBEEF;
    for (int i = 0; i < view->PageData.size(); ++i)
    {
        CInventoryItem* item = view->PageData[i].Item;
        u32 subcategory = item->Details.GetSubcategoryIndex();
        if (subcategory != key)
        {
            view->SortTermBoundaries.resize(view->SortTermBoundaries.size() + 1);
            SSortTermBoundary& boundary = view->SortTermBoundaries.back();
            boundary.Index = i;
            profile->GetStringFromIndex(subcategory, boundary.Name);
            key = subcategory;
        }

    }
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
    if (view->Descriptor.Type & E_TYPE_PAINT) return true;
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
    TABLE[SORT_SUBCATEGORY] = (u32)&_global_subcategory_sort_hook - (u32)TABLE;
    TABLE[SORT_TYPE] = (u32)&_global_type_sort_hook - (u32)TABLE;

    // Switch out the pointer to the switch case in the TOC
    MH_Poke32(0x0091e06c, (u32)TABLE);
}
