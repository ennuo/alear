#ifndef GOOEY_NODE_MANAGER_H
#define GOOEY_NODE_MANAGER_H

#include <vector.h>
#include <TextRange.h>

#include "gooey/GooeyNodeManagerEnums.h"
#include "gooey/GooeyNodeContainer.h"
#include "gooey/GooeyNode.h"
#include "gooey/GooeyContainerStyles.inl"
#include "gooey/GooeySizing.h"
#include "gooey/GooeyRender.h"

#include "InputMode.h"
#include "PlayerNumber.inl"
#include "PoppetEnums.inl"


class CGooeyNodeManager {
struct SNode {
    u64 UID;
    CGooeyNode* Node;
};
struct SRecentlyHighlighted {
    u64 HighlightedUID;
    u32 Timestamp;
};
typedef CRawVector<SNode> NodeVec;
typedef CRawVector<CGooeyNodeContainer*> ContainerVec;
public:
    CGooeyNodeManager(EInputMode input_type, EGooeyNodeManagerNetworkID network_id);
public:
    bool StartFrameNamed(u64 uid, EGooeyBubbleStyle bubble_style);
    void SetFrameSizing(SizingBehaviour behaviour_w, SizingBehaviour behaviour_h);
    void SetFrameLayoutMode(ELayoutMode layout_w, ELayoutMode layout_h);
    void SetFrameBorders(float x, float y);
    void SetFrameDefaultChildSpacing(float, float);
    void SetFrameStyle(EGooeyContainerStyle container_style);

    void EndFrame();
    u32 EndFrame(u32 accepted_input);

    u32 DoFancyButtonNamed(u64 uid, wchar_t* text, EGooeyTextStyle text_style, EGooeyButtonStyle button_style, EGooeyButtonState button_state, u32 accepted_input, CSDFIconParams* icon);
    u32 DoTextNamed(u64 uid, TextRange<wchar_t> text, EGooeyTextStyle text_style, v4 text_colour);
    u32 DoButton(u64 uid, wchar_t* text, EGooeyTextStyle text_style, EGooeyButtonState button_state, CSDFIconParams* icon, u32 accepted_input);
    u32 DoInline(u64 uid, wchar_t* text, EGooeyTextStyle text_style, EGooeyButtonState button_state, CSDFIconParams* icon, u32 accepted_input);

    void DoBreak();
    void DoHorizontalBreak(EGooeyBreakStyle style, v2 tex_scale);

    void PerFrameUpdate(EPlayerNumber player_number, EPoppetMode mode, u32 pad_index);
    void RenderToTexture(u32 screen_width, u32 screen_height, int texture_width, int texture_height, bool stencil_clip, bool respect_fade);
    u64 GetAnonymousUID();
public:
    inline u32 DoInline(wchar_t* text, EGooeyTextStyle text_style, EGooeyButtonState button_state, CSDFIconParams* icon, u32 accepted_input)
    {
        return DoInline(GetAnonymousUID(), text, text_style, button_state, icon, accepted_input);
    }

    inline u32 DoButton(wchar_t* text, EGooeyTextStyle text_style, EGooeyButtonState button_state, CSDFIconParams* icon, u32 accepted_input)
    {
        return DoButton(GetAnonymousUID(), text, text_style, button_state, icon, accepted_input);
    }

    inline u32 DoText(wchar_t* text, EGooeyTextStyle text_style)
    {
        return DoTextNamed(GetAnonymousUID(), text, text_style, v4(1.0f, 1.0f, 1.0f, 1.0f));
    }

    inline u32 DoText(wchar_t* text, EGooeyTextStyle text_style, v4 text_colour)
    {
        return DoTextNamed(GetAnonymousUID(), text, text_style, text_colour);
    }

    inline u32 DoTextNamed(u64 uid, wchar_t* text, EGooeyTextStyle text_style, v4 text_colour)
    {
        return DoTextNamed(uid, TextRange<wchar_t>(text), text_style, text_colour);
    }

    inline bool StartFrame(EGooeyBubbleStyle bubble_style)
    {
        return StartFrameNamed(GetAnonymousUID(), bubble_style);
    }
    
    inline bool StartFrame()
    {
        return StartFrameNamed(GetAnonymousUID());
    }

    inline bool StartFrameNamed(u64 uid)
    {
        return StartFrameNamed(uid, GBS_NO_STYLE);
    }

    inline void SetStylesheetScalingFactor(float factor) { StylesheetScalingFactor = factor; }
public:
    char Pad0[0x210];
    c32 PrimaryColour;
    c32 TertiaryColour;
    c32 HighlightColour;
    c32 EmphasisColour;
    char Pad1[0x30];
    float StylesheetScalingFactor;
    char Pad2[0x8c];
};


#endif // GOOEY_NODE_MANAGER_H