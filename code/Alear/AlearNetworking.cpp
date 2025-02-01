#include "AlearNetworking.h"
#include "AlearHooks.h"

#include <vector.h>
#include <ResourceGame.h>
#include <network/NetworkManager.h>
#include <network/NetworkUtilsNP.h>
#include <ResourceDescriptor.h>
#include <ResourceSyncedProfile.h>
#include <ResourceLocalProfile.h>
#include <ResourceSystem.h>
#include <PlayerNumber.inl>
#include <PadEnums.h>
#include <PadIndex.inl>
#include <hook.h>
#include <padinput.h>

typedef unsigned int CellSysutilUserId;
typedef CellSysutilUserId LocalUserID;

class CNetworkPlayerInfoForJoin {
public:
    inline CNetworkPlayerInfoForJoin() : PlayerID(), ProfileDesc(), DisplayName(), PlayerNumber(E_PLAYER_NUMBER_NONE),
    PadIndex(), UserID(), CrossButtonIsEnter(true), ControllingPauseMenu(false)
    {

    }
public:
    NetworkPlayerID PlayerID;
    CResourceDescriptor<RSyncedProfile> ProfileDesc;
    MMString<wchar_t> DisplayName;
    EPlayerNumber PlayerNumber;
    EPadIndex PadIndex;
    LocalUserID UserID;
    bool CrossButtonIsEnter;
    bool ControllingPauseMenu;
};




bool gWaitForNetworkInput = false;
class CNetworkInput {
private:
    char CompressedData[255];
    u8 CompressedDataLength;
};

MH_DefineFunc(SortPadInputsIntoYellowHeads, PORT_RGAME_SORT_PAD_INPUTS_INTO_YELLOW_HEADS, TOC0, void, RGame*, CRawVector<CNetworkInput> const& player_inputs, bool predict);
MH_DefineFunc(AddPlayer, 0x000c28d4, TOC0, void, RGame*, CP<RLocalProfile> const&, CP<RSyncedProfile> const&, CNetworkPlayerInfoForJoin const&, SceNpId const&);


struct NetMessageInput {
    s8 LeftStickX;
    s8 LeftStickY;
    s8 RightStickX;
    s8 RightStickY;
    u16 SensorDataSmooth[4];
    u16 SensorData[2];
    u8 PressureL2R2[2];
    u8 PressureCross;
    u16 Buttons;
};

bool OnRunFrame(RGame* game, CRawVector<CNetworkInput> const& player_inputs, bool update_camera)
{
    // if (gPadData->ButtonsDown & PAD_BUTTON_TRIANGLE)
    // {
    //     CP<RSyncedProfile> prf = AllocateNewResource<RSyncedProfile>(0);
    //     CP<RLocalProfile> ipr = AllocateNewResource<RLocalProfile>(0);
    //     CNetworkPlayerInfoForJoin info;
    //     info.PlayerNumber = (EPlayerNumber)-2;
    //     info.DisplayName = L"test player";
    //     strcpy("test player", info.PlayerID.handle.data);

    //     AddPlayer(gGame, ipr, prf, info, info.PlayerID);

    // }

    if (gWaitForNetworkInput) return false;
    SortPadInputsIntoYellowHeads(game, player_inputs, false);
    return true;
}

void AttachNetworkingHooks()
{
    // MH_PokeBranch(0x000b1324, &_run_frame_hook);
}

// bool AllowedToApplySyncedUpdates(u32 num_inputs_applied_this_frame, u32 num_inputs_applied_last_frame)
// {
//     if (!gNetworkManager.GameDataManager.LevelDataSynced() || IsTimeToSwitchLevel()) return false;

//     bool need_to_apply_an_input_to_keep_ticking_over;

//     u32 players_to_pack_bits = gNetworkManager.InputManager.GetLocalPadsInGameBits(); 
//     if (players_to_pack_bits == 0)
//     {
//         CRawVector<unsigned int>& inputs_available_history = gNetworkManager.Messaging.GetInputAvailableHistory();
//         if (input_available_history.size() == 0)
//             return true;
                
//         need_to_apply_an_input_to_keep_ticking_over = true;    
//         u32 inputs_available = inputs_available_history.front();
//         bool have_enough_predicted_inputs_left = inputs_available >= 2;
//         if (!have_enough_predicted_inputs_left)
//             need_to_apply_an_input_to_keep_ticking_over = false;
//     }
//     else
//     {
//         need_to_apply_an_input_to_keep_ticking_over = gNetworkManager.InputManager.GetNumPredictedFrames() > 0;
//     }

//     if (!need_to_apply_an_input_to_keep_ticking_over && (num_inputs_applied_this_frame | num_inputs_applied_last_frame) != 0)
//         return !IsGamePaused();
    
//     return true;
// }