#ifndef ALEAR_UNITY_H
#define ALEAR_UNITY_H

#include <network/NetworkUtilsNP.h>

class CInventoryView;
class CInventoryItem;

void OnUpdateLevel();
void OnReleaseLevel();
void OnRunPipelinePostProcessing();
void OnPredictionOrRenderUpdate();
void OnWorldRenderUpdate();
void OnUpdateHttpTasks();
bool CustomTryTranslate(u32 key, tchar_t const*& out);
bool CustomItemMatch(CInventoryView* view, CInventoryItem* item, NetworkPlayerID* owner);

void InitSharedHooks();

#define E_GSUB_RLST 3536200819u
#define E_TRANSLATIONS_RLST 4014013721u

#endif // ALEAR_UNITY_H