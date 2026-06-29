#include "AlearShared.h"
#include "AlearConfig.h"


#include <refcount.h>
#include <Hash.h>

#include <PoppetChild.h>
#include <Poppet.h>
#include <PoppetEnums.inl>
#include <gooey/GooeyNodeManager.h>
#include <gooey/GooeyContainerStyles.inl>
#include <poppet/PoppetGooey.h>
#include <InventoryItem.h>
#include <inventoryView.h>
#include <InventoryCollection.h>
#include <Translate.h>
#include <ProfileCache.h>

#include <ResourceGFXTexture.h>
#include <ResourceLocalProfile.h>
#include <ResourceGame.h>
#include <PartYellowHead.h>
#include <Player.h>

#include <vm/VirtualMachine.h>

u64 GetSectionUID(u64 page_uid, u32 section_id)
{
    return (u64)section_id * 10000ULL + page_uid;
}

u64 GetItemUID(u64 frame_id, u32 i)
{
    return (u64)i + frame_id + 1 | 0xbeefface00000000ull;
}

u64 GooeyIDFromScriptInt(int v)
{
    return (u64)v | 0xbeefface00000000ull;
}

u64 GetPageUID(u32 cache_id, u32 page_id)
{
    return GooeyIDFromScriptInt(page_id * 1000000ul + cache_id * 100000000ul + 201000000ul);
}

MH_DefineFunc(CPoppetGooey_DoTitleBox, 0x00380e00, TOC1, void, CPoppetChild*, tchar_t* title);
u32 DoCustomInventoryPage(CPoppetChild* gooey, CInventoryCollection* current_cache, CInventoryView* page, v2 page_size, u32 page_number, u32 input_flags)
{
    CGooeyNodeManager* manager = *(CGooeyNodeManager**)((char*)gooey + 0x250);
    CScriptObject* so_gooey = *(CScriptObject**)((char*)gooey + 0x24c);
    CScriptObjectInstance* so_poppet = *(CScriptObjectInstance**)((char*)gooey + 0x238);

    u32 res = 0;

    if (!manager->StartFrame()) return res;

    manager->SetFrameLayoutMode(LM_CENTERED, LM_JUSTIFY_START);
    manager->SetFrameConstrainFocus(true);
    manager->SetFrameBorders(16.0f, 24.0f, 16.0f, 24.0f);
    manager->SetFrameSizing((float)page_size.getX(), (float)page_size.getY());
    manager->SetFrameWhollyVisibleWithChildren();

    CPoppetGooey_DoTitleBox(gooey, page->Descriptor.Title.c_str());

    if (manager->StartFrameNamed(GetPageUID(current_cache->CollectionID, page_number)))
    {
        manager->SetFrameLayoutMode(LM_CENTERED, LM_JUSTIFY_START);
        manager->SetFrameDefaultChildSpacing(16.0f, 16.0f);
        manager->SetFrameHighlightStyle((EGooeyHighlightStyle)(GHS_ROUNDED_RECT | GHS_DROP_SHADOW));
        manager->SetFrameBorders(16.0f, 8.0f, 16.0f, 8.0f);
        manager->SetFrameConstrainFocus(true);

        PWorld* world = gGame->GetWorld();
        
        CSignature signature;
        CScriptArguments args;
        args.AppendArg(so_gooey);

        if (page->CustomID == 0x4D525048) signature = CSignature("UpdateMorphUI__Q5Gooey");
        else
        {
            signature = CSignature("UpdateGlobalUI__Q5Gooeyi");
            args.AppendArg(page->CustomID);
        }

        so_poppet->InvokeSync(world, signature, args, NULL);

        manager->EndFrame();
    }

    return manager->EndFrame(input_flags);
}

u32 DoInventorySoundObjectButton(CPoppetChild* gooey, u64 uid, CInventoryItem* item, v4 col, bool wide_icons, bool roundy_bg)
{
    CGooeyNodeManager* manager = *(CGooeyNodeManager**)((char*)gooey + 0x250);
    u32 res = 0;

    manager->DoBreak();
    if (manager->StartFrame())
    {
        manager->SetFrameLayoutMode(LM_JUSTIFY_START, LM_JUSTIFY_START);
        manager->SetFrameSizing(SizingBehaviour::Contents(), 0.0f);
        manager->SetFrameHighlightStyle(GHS_ROUNDED_RECT);
        manager->SetFrameHighlightSizing((EGooeySizingType)648);
        if (manager->StartFrameNamed(uid))
        {
            manager->SetFrameLayoutMode(LM_CENTERED, LM_CENTERED);
            manager->SetFrameSizing(SizingBehaviour::Contents(), 0.0f);
            manager->SetFrameDefaultChildSpacing(64.0f, 0.0f);
            manager->SetFrameCornerRadius(64.0f);
            // manager->SetFrameOutlineWidth(48.0f);
            if (manager->StartFrame())
            {
                manager->SetFrameLayoutMode(LM_JUSTIFY_START, LM_JUSTIFY_START);
                manager->SetFrameSizing(0.0f, 0.0f);

                /* 0xa140200 does a loading indicator, not sure which bit */

                CP<RTexture> icon = item->Details.Icon.GetRef();
                manager->DoImageButtonNamed(manager->GetAnonymousUID(), icon, v2(128.0f), v4(1.0f), 0x0);

                manager->EndFrame();
            }

            if (manager->StartFrame())
            {
                manager->SetFrameSizing(SizingBehaviour::Contents(), SizingBehaviour::Contents());
                manager->SetFrameLayoutMode(LM_JUSTIFY_START, LM_CENTERED);
                manager->SetFrameBorders(8.0f, 8.0f);
                manager->SetFrameApplyClip(true, false);

                const tchar_t* text = item->Details.TranslateName();
                // if (!TryTranslate(item->Details.NameTranslationTag, text))
                //     text = (const tchar_t*)L"";
                manager->DoTitle((wchar_t*)text, GTS_B1, v2(-1.0f, 0.0f));
                
                manager->EndFrame();
            }

            res = manager->EndFrame(256);
        }

        manager->EndFrame();
    }

    manager->DoBreak();

    return res;
}

struct PageSection {
    SSortTermBoundary* Boundary;
    CRawVector<CInventoryItem*> ItemList;
};

struct SectionSettings {
    bool IsPlayerColourPage;
    u32 NumLocalPlayers;
    EGooeySizingType HighlightSizing;
    bool IsFirstSection;
};

struct EPoppetItemArgs {
    v2 size;
    u32 last_item_id;
    bool faded;
    bool is_costume_page;
    bool wide_icons;
};

static u32 gFunctionCategoryID = JenkinsHash((u8*)L"Functions", 18, 0);


MH_DefineFunc(CPoppetGooey_SetToolTip, 0x0037dd48, TOC1, void, CPoppetChild*, const wchar_t* tooltip, v4 rect, u64 uid, bool instant);
MH_DefineFunc(CPoppetGooey_ShowInfoBubble, 0x0037dd98, TOC1, void, CPoppetChild*, int mode);
MH_DefineFunc(CPoppetGooey_UpdateFocus, 0x0037dde0, TOC1, void, CPoppetChild*, u64 uid, bool);


MH_DefineFunc(DoPoppetItem, 0x0037ffb0, TOC1, void, CPoppetChild*, u64, CInventoryItem*, CInventoryCollection*, u32, const EPoppetItemArgs&, bool&, const CP<RLocalProfile>&);
MH_DefineFunc(DoSectionBreak, 0x00380c78, TOC1, void, CPoppetChild*, const tchar_t*, bool);
void CustomDoPoppetSection(
    CPoppetChild* gooey, PageSection* page_section, SectionSettings& settings, bool hide_section_titles,
    u64 frame_id, CInventoryCollection* current_cache, u32 page_number, EPoppetItemArgs item_args,
    u32& uid_offset, bool& first_section_break, bool& filler, const tchar_t* override_title
)
{
    if (page_section->ItemList.empty()) return;

    CPoppet* poppet = gooey->GetParent();
    const CP<RLocalProfile>& prf = poppet != NULL ? poppet->GetLocalProfile() : RLocalProfile::EMPTY;

    CGooeyNodeManager* manager = *(CGooeyNodeManager**)((char*)gooey + 0x250);
    u64 section_uid = GetSectionUID(frame_id, uid_offset++);

    if (!manager->StartFrame(GBS_NO_STYLE)) return;
    manager->SetFrameSizing(SizingBehaviour::Contents(), 0.0f);
    manager->SetFrameDefaultChildSpacing(16.0f, 16.0f);

    manager->SetFrameHighlightStyle(GHS_DROP_SHADOW);
    manager->SetFrameScrollStyle(GSS_DEFAULT, GSS_PROPORTIONAL);
    manager->SetFrameDrawScrollArrows(true, false, false, false);
    if (settings.IsFirstSection)
    {
        manager->SetFrameWhollyVisibleWithChildren();
        settings.IsFirstSection = false;
    }

    manager->SetFrameHighlightSizing(settings.HighlightSizing);

    if (settings.IsPlayerColourPage)
    {
        SizingBehaviour behaviour = SizingBehaviour::Relative(0.5f);
        if (settings.NumLocalPlayers < 2)
        {
            behaviour = SizingBehaviour::Relative(0.33f);
            manager->AddFrameColumn(behaviour, LM_CENTERED);   
        }

        manager->AddFrameColumn(behaviour, LM_CENTERED);
        manager->AddFrameColumn(behaviour, LM_CENTERED);
    }

    const tchar_t* title = override_title != NULL ? override_title : (const tchar_t*)page_section->Boundary->Name.c_str();
    u32 category_id = JenkinsHash((u8*)title, StringLength(title) * sizeof(tchar_t), 0);

    bool is_hidden = false;
    bool can_hide = category_id != gFunctionCategoryID;
    if (!gCanCollapseCategories) 
        can_hide = false;
    
    bool backgrounds = current_cache->InventoryViews[page_number]->Descriptor.Type == E_TYPE_BACKGROUND;
    bool sound_objects = current_cache->InventoryViews[page_number]->Descriptor.Type == E_TYPE_SOUND;
    hide_section_titles = settings.IsPlayerColourPage || backgrounds || sound_objects;
    
    if (hide_section_titles) can_hide = false;

    if (can_hide)
    {
        for (int i = 0; i < prf->HiddenCategories.size(); ++i)
        {
            if (prf->HiddenCategories[i] == category_id)
            {
                is_hidden = true;
                break;
            }
        }
    }
    

    if (!hide_section_titles)
    {
        // float height = is_hidden ? 2.0f : 24.0f;
        float height = 24.0f;
        manager->SetFrameBorders(0.0f, height, 0.0f, height);
        manager->DoBreak();
        if (manager->StartFrame(GBS_NO_STYLE))
        {
            manager->SetFrameLayoutMode(LM_JUSTIFY_START, LM_CENTERED);
            manager->SetFrameSizing(SizingBehaviour::Contents(), 0.0f);
            manager->SetFrameBorders(0.0f, 0.0f, 0.0f, 16.0f);

            u64 item_uid = GetItemUID(section_uid, 0);
            u32 result = manager->DoFancyButtonNamed(item_uid, (wchar_t*)title, GTS_T5, GBS_LABEL, STATE_NORMAL, can_hide ? 0x100 : 0x0, NULL);
            if (result & 0x100)
            {
                if (is_hidden)
                {
                    for (u32* it = prf->HiddenCategories.begin(); it != prf->HiddenCategories.end(); ++it)
                    {
                        if (*it == category_id)
                        {
                            prf->HiddenCategories.erase(it);
                            break;
                        }
                    }
                }
                else
                {
                    prf->HiddenCategories.push_back(category_id);
                }

                is_hidden = !is_hidden;
            }

            if (result & 0x400000)
            {
                CRect rect = manager->GetLastItemScreenRect();
                CPoppetGooey_SetToolTip(gooey, L"", v4(rect.tl.getX(), rect.tl.getY(), rect.br.getX(), rect.br.getY()), item_uid, false);
            }

            manager->DoBreak(GBS_DASHED, v2(1.2f, 0.5f), 20.0f);
            manager->EndFrame();
        }

        manager->DoBreak();
    } 
    
    first_section_break = false;

    if (!is_hidden)
    {
        for (int i = 0; i < page_section->ItemList.size(); ++i)
        {
            CInventoryItem* item = page_section->ItemList[i];
            u64 item_uid = GetItemUID(section_uid, i + 1);
            DoPoppetItem(gooey, item_uid, item, current_cache, page_number, item_args, filler, prf);
        }
    }

    manager->EndFrame();
}

#include <ResourceSystem.h>

static v2 icon_move_x;
static v2 icon_move_y;
static v2 icon_size;

void CPoppetGooey::DoItemInfoIcon(u64 uid, v2& offset, v2 item_size, v4 col, int icon)
{
    CGooeyNodeManager* gooey = GetManager();

    offset += icon_move_x;
    if ((float)offset.getX() > (float)item_size.getX()) // not dealing with vector shit right now
        offset += icon_move_y;
    
    gooey->DoIcon((EGooeyIcon)icon, icon_size, col);
    gooey->SetLastItemAsRelative(uid, item_size - offset);
}

void CPoppetGooey::DoInventoryItemInfoIcons(u64 uid, CInventoryItem* item, v2 size)
{
    CGooeyNodeManager* manager = GetManager();
    if (manager == NULL) return;

    manager->SetFrameApplyClip(false, false);

    icon_move_x = v2(64.0f, 0.0f);
    icon_move_y = v2(-128.0f, 64.0f);
    icon_size = v2(64.0f);

    v2 offset = v2(0.0f, 64.0f);
    

    const CPlayer* player = GetPlayer()->GetPYellowHead()->GetPlayer();
    const CSlotID& id = item->Details.GetLevelUnlockSlotID();

    v2 top_row_offset = v2(4.0f, 4.0f);
    v2 top_row_icon_size = v2(42.0f);

    if(gShowColorableItems)
    {
        if (item->Details.IsColorable())
        {
            manager->DoImageButton(
                LoadResourceByKey<RTexture>(2807377854ul),
                top_row_icon_size
            );
            
            manager->SetLastItemAsRelative(uid, top_row_offset);
            top_row_offset += icon_move_x;
        }

        if (item->Details.IsAnimated())
        {
            manager->DoIcon(GI_RATING_STAR, top_row_icon_size, v4(1.0f));
            manager->SetLastItemAsRelative(uid, top_row_offset);
            top_row_offset += icon_move_x;
        }
    }

    if (!id.Empty())
    {
        offset += icon_move_x;
        if ((float)offset.getX() > (float)size.getX()) // not dealing with vector shit right now
            offset += icon_move_y;

        const CP<RLocalProfile>& profile = ProfileCache::GetOrCreateMainUserProfile();
        const CSlot* slot = profile->FindSlot(id);
        if (slot != NULL && slot->Icon.IsValid() && slot->Icon.GetType() == RTYPE_TEXTURE)
        {
            manager->DoImageButton(
                LoadResource(slot->Icon),
                icon_size
            );

            manager->SetLastItemAsRelative(uid, size - offset);
        }
    }

    if(!item->Details.IsUserCreated())
    {
        const tchar_t* text;
        if (gShowItemsWithLore && item->Details.GetLore() != 0)
            DoItemInfoIcon(uid, offset, size, v4(0.0, 1.0, 0.0, 1.0), GI_CHAT);
        if (gShowMissingDesc && !TryTranslate(item->Details.GetDescription(), text))
            DoItemInfoIcon(uid, offset, size, v4(0.0, 0.0, 1.0, 1.0), GI_CHAT);
        if (gShowMissingName && !TryTranslate(item->Details.GetName(), text))
            DoItemInfoIcon(uid, offset, size, v4(1.0, 0.0, 0.0, 1.0), GI_CHAT);
        if (gShowEmittableItems && item->Details.AllowEmit)
            DoItemInfoIcon(uid, offset, size, v4(0.0, 0.0, 1.0, 1.0), GI_STAR);
    }
    if (gShowCheatedItems && item->IsCheat())
        DoItemInfoIcon(uid, offset, size, v4(1.0), GI_COPYRIGHT_MINE);
    if (item->IsErrored())
        DoItemInfoIcon(uid, offset, size, v4(1.0, 0.0, 0.0, 1.0), GI_UNSAVED);
    if (item->IsHearted())
        DoItemInfoIcon(uid, offset, size, v4(1.0), GI_HEART);
    if (item->IsUploaded())
        DoItemInfoIcon(uid, offset, size, v4(1.0), GI_UPLOADED);
    if (item->Details.IsUserCreated() && item->Details.GetCopyrightFlag() && !item->Details.IsACostume())
        DoItemInfoIcon(uid, offset, size, v4(1.0), GI_COPYRIGHT);
}

void CustomDoEmptyPageMessage(CPoppetGooey* gooey, CInventoryView* view, u32 num_items)
{
    CGooeyNodeManager* manager = gooey->GetManager();
    CPoppet* poppet = gooey->GetParent();

    if (manager == NULL || poppet == NULL || view == NULL) return;

    manager->DoBreak();
    if (manager->StartFrame())
    {
        manager->SetFrameSizing(SizingBehaviour::Contents(), 0.0f);
        manager->SetFrameLayoutMode(LM_CENTERED, LM_JUSTIFY_START);
        manager->SetFrameHighlightStyle(GHS_NONE);
        manager->SetFrameBorders(16.0f, 64.0f);

        const char* prefix = num_items == 0 ? "POPIT_PAGE_NO_" : "POPIT_PAGE_FEW_";
        const char* suffix;
        CInventoryView* page = poppet->GetCurrentInventoryPage();
        if (page != NULL)
        {
            if (!page->HeartedOnly)
            {
                const u32 type = page->Descriptor.Type;
                const u32 subtype = page->Descriptor.SubType;

                if (type & E_TYPE_USER_COSTUME)
                    suffix = "SAVED_COSTUMES";
                else if (type == E_TYPE_PLAYER_COLOUR)
                    suffix = "COLOURS";
                else if (type == E_TYPE_COSTUME_MATERIAL)
                    suffix = "COSTUME_MATERIALS";
                else if (type == E_TYPE_COSTUME)
                {
                    if (subtype == E_SUBTYPE_MORPH)
                        suffix = "MORPHS";
                    else
                        suffix = "COSTUMES";
                }
                else if (type == E_TYPE_STICKER)
                    suffix = "STICKERS";
                else if (type == E_TYPE_DECORATION)
                    suffix = "DECORATIONS";
                else if (type == E_TYPE_EYETOY)
                    suffix = "EYETOY";
                else if (type == E_TYPE_PAINT)
                    suffix = "PAINT";
                else if ((type & E_TYPE_USER_STICKER) == 0)
                {
                    if (type == E_TYPE_PRIMITIVE_SHAPE)
                        suffix = "SHAPES";
                    else if (type == E_TYPE_PRIMITIVE_MATERIAL)
                        suffix = "MATERIALS";
                    else if ((type & E_TYPE_USER_POD) == 0)
                    {
                        if ((type & E_TYPE_USER_OBJECT) == 0)
                        {
                            if (subtype == E_SUBTYPE_TOYS)
                                suffix = "TOYS";
                            else
                                suffix = "OBJECTS";
                        }
                        else if ((subtype & E_SUBTYPE_MADE_BY_OTHERS) == 0)
                            suffix = "PLANS";
                        else
                            suffix = "OTHERSPLANS";
                    }
                    else 
                    {
                        if (subtype == E_SUBTYPE_POD_CONTROLLER)
                            suffix = "POD_CONTROLLERS";
                        else if (subtype == E_SUBTYPE_POD_MESH)
                            suffix = "POD_MESHES";
                        else
                            suffix = "PODS";
                    }
                }
                else if ((subtype & E_SUBTYPE_MADE_BY_OTHERS) == 0)
                    suffix = "PHOTOS";
                else suffix = "OTHERSPHOTOS";
            }
            else
            {
                suffix = "HEARTED";
            }

            manager->DoText((wchar_t*)Translate(MakeLamsKeyID(prefix, suffix)), GTS_T5);
        }

        manager->EndFrame();
    }
}

void AttachPoppetInterfaceExtensionHooks()
{
    MH_PokeHook(0x00382818, DoCustomInventoryPage);
    MH_PokeHook(0x00380944, CustomDoEmptyPageMessage);
    MH_PokeMemberHook(0x002ed5b8, CInventoryItemDetails::TranslateName);
    MH_PokeMemberHook(0x0037f2bc, CPoppetGooey::DoInventoryItemInfoIcons);
}