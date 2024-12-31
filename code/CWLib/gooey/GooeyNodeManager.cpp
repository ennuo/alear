#include "gooey/GooeyNodeManager.h"

#include <hook.h>

MH_DefineFunc(CGooeyNodeManager_CGooeyNodeManager, 0x00301ce0, TOC0, void, CGooeyNodeManager*, EInputMode, EGooeyNodeManagerNetworkID);
CGooeyNodeManager::CGooeyNodeManager(EInputMode input_type, EGooeyNodeManagerNetworkID network_id)
{
    CGooeyNodeManager_CGooeyNodeManager(this, input_type, network_id);
}

MH_DefineFunc(CGooeyNodeManager_PerFrameUpdate, 0x0030548c, TOC0, void, CGooeyNodeManager*, EPlayerNumber, EPoppetMode, u32);
void CGooeyNodeManager::PerFrameUpdate(EPlayerNumber player_number, EPoppetMode mode, u32 pad_index)
{
    CGooeyNodeManager_PerFrameUpdate(this, player_number, mode, pad_index);
}

MH_DefineFunc(CGooeyNodeManager_StartFrameNamed, 0x0030118c, TOC0, bool, CGooeyNodeManager*, u64, EGooeyBubbleStyle);
bool CGooeyNodeManager::StartFrameNamed(u64 uid, EGooeyBubbleStyle bubble_style)
{
    return CGooeyNodeManager_StartFrameNamed(this, uid, bubble_style);
}

MH_DefineFunc(CGooeyNodeManager_EndFrame, 0x002fcb1c, TOC0, void, CGooeyNodeManager*);
void CGooeyNodeManager::EndFrame()
{
    CGooeyNodeManager_EndFrame(this);
}

MH_DefineFunc(CGooeyNodeManager_EndFrameInput, 0x002fcae0, TOC0, u32, CGooeyNodeManager*, u32);
u32 CGooeyNodeManager::EndFrame(u32 accepted_input)
{
    return CGooeyNodeManager_EndFrameInput(this, accepted_input);
}

MH_DefineFunc(CGooeyNodeManager_GetAnonymousUID, 0x002f9ae8, TOC0, u64, CGooeyNodeManager*);
u64 CGooeyNodeManager::GetAnonymousUID()
{
    return CGooeyNodeManager_GetAnonymousUID(this);
}

MH_DefineFunc(CGooeyNodeManager_GetLastItemScreenRect, 0x002fc580, TOC0, CRect, CGooeyNodeManager*);
CRect CGooeyNodeManager::GetLastItemScreenRect()
{
    return CGooeyNodeManager_GetLastItemScreenRect(this);
}

MH_DefineFunc(CGooeyNodeManager_EnsureNodeOrDescendantHasFocus, 0x00305900, TOC0, bool, CGooeyNodeManager*, u64);
bool CGooeyNodeManager::EnsureNodeOrDescendantHasFocus(u64 uid)
{
    return CGooeyNodeManager_EnsureNodeOrDescendantHasFocus(this, uid);
}

MH_DefineFunc(CGooeyNodeManager_NodeOrDescendantHasFocus, 0x002f9a60, TOC0, bool, CGooeyNodeManager*, u64);
bool CGooeyNodeManager::NodeOrDescendantHasFocus(u64 uid)
{
    return CGooeyNodeManager_NodeOrDescendantHasFocus(this, uid);
}

MH_DefineFunc(CGooeyNodeManager_SetFrameSizing, 0x002f8a28, TOC0, void, CGooeyNodeManager*, SizingBehaviour, SizingBehaviour);
void CGooeyNodeManager::SetFrameSizing(SizingBehaviour behaviour_w, SizingBehaviour behaviour_h)
{
    CGooeyNodeManager_SetFrameSizing(this, behaviour_w, behaviour_h);
}

MH_DefineFunc(CGooeyNodeManager_SetFrameLayoutMode, 0x002fa34c, TOC0, void, CGooeyNodeManager*, ELayoutMode, ELayoutMode);
void CGooeyNodeManager::SetFrameLayoutMode(ELayoutMode layout_w, ELayoutMode layout_h)
{
    CGooeyNodeManager_SetFrameLayoutMode(this, layout_w, layout_h);
}

MH_DefineFunc(CGooeyNodeManager_SetFrameCornerRadius, 0x002fa068, TOC0, void, CGooeyNodeManager*, float);
void CGooeyNodeManager::SetFrameCornerRadius(float radius)
{
    CGooeyNodeManager_SetFrameCornerRadius(this, radius);
}

MH_DefineFunc(CGooeyNodeManager_RenderToTexture, 0x00303c58, TOC0, void, CGooeyNodeManager*, u32, u32, int, int, bool, bool);
void CGooeyNodeManager::RenderToTexture(u32 screen_width, u32 screen_height, int texture_width, int texture_height, bool stencil_clip, bool respect_fade)
{
    CGooeyNodeManager_RenderToTexture(this, screen_width, screen_height, texture_width, texture_height, stencil_clip, respect_fade);
}

MH_DefineFunc(CGooeyNodeManager_DoImageButtonNamed, 0x003007e8, TOC0, u32, CGooeyNodeManager*, u64, CP<RTexture> const&, v2, v4, u32);
u32 CGooeyNodeManager::DoImageButtonNamed(u64 uid, CP<RTexture> const& texture, v2 size, v4 colour, u32 accepted_input)
{
    return CGooeyNodeManager_DoImageButtonNamed(this, uid, texture, size, colour, accepted_input);
}

MH_DefineFunc(CGooeyNodeManager_DoTextNamed, 0x002ffb48, TOC0, u32, CGooeyNodeManager*, u64, TextRange<wchar_t>, EGooeyTextStyle, v4);
u32 CGooeyNodeManager::DoTextNamed(u64 uid, TextRange<wchar_t> text, EGooeyTextStyle text_style, v4 text_colour)
{
    return CGooeyNodeManager_DoTextNamed(this, uid, text, text_style, text_colour);
}

MH_DefineFunc(CGooeyNodeManager_DoTitleNamed, 0x002ff608, TOC0, u32, CGooeyNodeManager*, u64, wchar_t*, EGooeyTextStyle, v2);
u32 CGooeyNodeManager::DoTitleNamed(u64 uid, wchar_t* text, EGooeyTextStyle text_style, v2 size)
{
    return CGooeyNodeManager_DoTitleNamed(this, uid, text, text_style, size);
}

MH_DefineFunc(CGooeyNodeManager_DoFancyButtonNamed, 0x002ffd10, TOC0, u32, CGooeyNodeManager*, u64, wchar_t*, EGooeyTextStyle, EGooeyButtonStyle, EGooeyButtonState, u32, CSDFIconParams*);
u32 CGooeyNodeManager::DoFancyButtonNamed(u64 uid, wchar_t* text, EGooeyTextStyle text_style, EGooeyButtonStyle button_style, EGooeyButtonState button_state, u32 accepted_input, CSDFIconParams* icon)
{
    CGooeyNodeManager_DoFancyButtonNamed(this, uid, text, text_style, button_style, button_state, accepted_input, icon);
}

u32 CGooeyNodeManager::DoButton(u64 uid, wchar_t* text, EGooeyTextStyle text_style, EGooeyButtonState button_state, CSDFIconParams* icon, u32 accepted_input)
{
    return DoFancyButtonNamed(uid, text, text_style, GBS_BUTTON, button_state, accepted_input, icon);
}

u32 CGooeyNodeManager::DoInline(u64 uid, wchar_t* text, EGooeyTextStyle text_style, EGooeyButtonState button_state, CSDFIconParams* icon, u32 accepted_input)
{
    return DoFancyButtonNamed(uid, text, text_style, GBS_INLINE, button_state, accepted_input, icon);
}

MH_DefineFunc(CGooeyNodeManager_DoScrollingTextChangerNamed, 0x002ff514, TOC0, u32, CGooeyNodeManager*, u64, wchar_t*, EGooeyTextStyle, v2);
u32 CGooeyNodeManager::DoScrollingTextChangerNamed(u64 uid, wchar_t* text, EGooeyTextStyle text_style, v2 size)
{
    return CGooeyNodeManager_DoScrollingTextChangerNamed(this, uid, text, text_style, size);
}

MH_DefineFunc(CGooeyNodeManager_DoRectangleNamed, 0x00300ae4, TOC0, u32, CGooeyNodeManager*, u64, v2, v4);
u32 CGooeyNodeManager::DoRectangleNamed(u64 uid, v2 size, v4 colour)
{
    return CGooeyNodeManager_DoRectangleNamed(this, uid, size, colour);
}

MH_DefineFunc(CGooeyNodeManager_DoBreak_TexScale_Float, 0x00300d44, TOC0, void, CGooeyNodeManager*, EGooeyBreakStyle, v2, float);
void CGooeyNodeManager::DoBreak(EGooeyBreakStyle style, v2 tex_scale, float f)
{
    CGooeyNodeManager_DoBreak_TexScale_Float(this, style, tex_scale, f);
}

MH_DefineFunc(CGooeyNodeManager_DoHorizontalBreak_TexScale, 0x00300dc8, TOC0, void, CGooeyNodeManager*, EGooeyBreakStyle, v2);
void CGooeyNodeManager::DoHorizontalBreak(EGooeyBreakStyle style, v2 tex_scale)
{
    CGooeyNodeManager_DoHorizontalBreak_TexScale(this, style, tex_scale);
}

MH_DefineFunc(CGooeyNodeManager_SetFrameBorders, 0x002f8a90, TOC0, void, CGooeyNodeManager*, float, float);
void CGooeyNodeManager::SetFrameBorders(float x, float y)
{
    CGooeyNodeManager_SetFrameBorders(this, x, y);
}

MH_DefineFunc(CGooeyNodeManager_SetFrameBorders4, 0x002fa274, TOC0, void, CGooeyNodeManager*, float, float, float, float);
void CGooeyNodeManager::SetFrameBorders(float x0, float y0, float x1, float y1)
{
    CGooeyNodeManager_SetFrameBorders4(this, x0, y0, x1, y1);
}

MH_DefineFunc(CGooeyNodeManager_SetFrameDefaultChildSpacing, 0x002fa20c, TOC0, void, CGooeyNodeManager*, float, float)
void CGooeyNodeManager::SetFrameDefaultChildSpacing(float x, float y)
{
    CGooeyNodeManager_SetFrameDefaultChildSpacing(this, x, y);
}

MH_DefineFunc(CGooeyNodeManager_DoBreak, 0x00300f1c, TOC0, void, CGooeyNodeManager*);
void CGooeyNodeManager::DoBreak()
{
    CGooeyNodeManager_DoBreak(this);
}

MH_DefineFunc(CGooeyNodeManager_SetFrameStyle, 0x002fa58c, TOC0, void, CGooeyNodeManager*, EGooeyContainerStyle);
void CGooeyNodeManager::SetFrameStyle(EGooeyContainerStyle container_style)
{
    CGooeyNodeManager_SetFrameStyle(this, container_style);
}

MH_DefineFunc(CGooeyNodeManager_AddFrameColumn, 0x002f9bb4, TOC0, void, CGooeyNodeManager*, SizingBehaviour, ELayoutMode);
void CGooeyNodeManager::AddFrameColumn(SizingBehaviour behaviour, ELayoutMode layout)
{
    CGooeyNodeManager_AddFrameColumn(this, behaviour, layout);
}

MH_DefineFunc(CGooeyNodeManager_SetFrameHighlightStyle, 0x002f9f94, TOC0, void, CGooeyNodeManager*, EGooeyHighlightStyle);
void CGooeyNodeManager::SetFrameHighlightStyle(EGooeyHighlightStyle highlight_style)
{
    CGooeyNodeManager_SetFrameHighlightStyle(this, highlight_style);
}

MH_DefineFunc(CGooeyNodeManager_SetFrameScrollStyle, 0x002f9eb8, TOC0, void, CGooeyNodeManager*, EGooeyScrollStyle, EGooeyScrollStyle);
void CGooeyNodeManager::SetFrameScrollStyle(EGooeyScrollStyle h_style, EGooeyScrollStyle v_style)
{
    CGooeyNodeManager_SetFrameScrollStyle(this, h_style, v_style);
}

MH_DefineFunc(CGooeyNodeManager_SetFrameDrawScrollArrows, 0x002f9e40, TOC0, void, CGooeyNodeManager*, bool, bool, bool, bool);
void CGooeyNodeManager::SetFrameDrawScrollArrows(bool d, bool highlight_left, bool highlight_right, bool push_out)
{
    CGooeyNodeManager_SetFrameDrawScrollArrows(this, d, highlight_left, highlight_right, push_out);
}

MH_DefineFunc(CGooeyNodeManager_SetFrameWhollyVisibleWithChildren, 0x002fa1a0, TOC0, void, CGooeyNodeManager*);
void CGooeyNodeManager::SetFrameWhollyVisibleWithChildren()
{
    CGooeyNodeManager_SetFrameWhollyVisibleWithChildren(this);
}

MH_DefineFunc(CGooeyNodeManager_SetFrameHighlightSizing, 0x002f9f28, TOC0, void, CGooeyNodeManager*, EGooeySizingType);
void CGooeyNodeManager::SetFrameHighlightSizing(EGooeySizingType sizing)
{
    CGooeyNodeManager_SetFrameHighlightSizing(this, sizing);
}