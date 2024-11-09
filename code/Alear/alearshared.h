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

#endif // ALEAR_UNITY_H