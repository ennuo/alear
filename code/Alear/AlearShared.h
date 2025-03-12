#ifndef ALEAR_UNITY_H
#define ALEAR_UNITY_H

#include <printf.h>
#include <set>
#include <filepath.h>
#include <ResourceDescriptor.h>
#include <mem_stl_buckets.h>
#include <network/NetworkUtilsNP.h>

typedef std::set<CResourceDescriptor<RPlan>, std::less<CResourceDescriptor<RPlan> >, STLBucketAlloc<CResourceDescriptor<RPlan> > > PlanDescriptorSet;
extern PlanDescriptorSet gUsedPlanDescriptors;

class CInventoryView;
class CInventoryItem;
class RLocalProfile;

void OnUpdateLevel();
void OnReleaseLevel();
void OnRunPipelinePostProcessing();
void OnPredictionOrRenderUpdate();
void OnWorldRenderUpdate();
void OnUpdateHttpTasks();
bool CustomTryTranslate(u32 key, tchar_t const*& out);
bool CustomItemMatch(CInventoryView* view, CInventoryItem* item, NetworkPlayerID* owner);
void GatherUsedPlanDescriptors();

void InitSharedHooks();
void InitLogicSystemHooks();
bool IsItemSelected(RLocalProfile* profile, CInventoryItem* item);


void AttachCustomPoppetMessages();
void AttachCustomToolTypes();
void AttachCustomSortModes();
void LoadCursorSprites();
void LoadRecordingShaders();
void LoadPostProcessingShaders();

#define E_GSUB_RLST 3536200819u
#define E_TRANSLATIONS_RLST 4014013721u

class CPoppet;
extern std::set<CPoppet*, std::less<CPoppet*>, STLBucketAlloc<CPoppet*> > gPoppetBloomHack;

class CVideoRecording {
public:
    inline CVideoRecording() : Filepath(), Handle(-1), NumFrames(0)
    {

    }
public:
    void StartRecording(char* path);
    void StopRecording();
    void ToggleRecording(char* path);
    bool IsRecording();
    void AppendVideoFrame();
public:
    CFilePath Filepath;
    FileHandle Handle;
    u32 NumFrames;    
};

extern CVideoRecording gBloomRecording;
extern CVideoRecording gPoppetRecording;

#endif // ALEAR_UNITY_H