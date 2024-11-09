#ifndef ALEAR_CAMERA_H
#define ALEAR_CAMERA_H

#include <MMString.h>
#include <vector.h>
#include <PlayerNumber.inl>
#include <DebugCamera.h>

extern bool gShowCameraHelper;
extern bool gShowOutlines;

void InitCameraHooks();

enum ECameraShotType {
    SHOT_CUT,
    SHOT_ORBIT_2D,
    SHOT_ORBIT_3D,
    SHOT_OFFSET
};

enum ECameraTargetType {
    TARGET_PLAYER,
    TARGET_OBJECT,
    TARGET_POSITION
};

enum ECinematicState {
    STATE_IDLE,
    STATE_PLAYING,
    STATE_TRANSITION
};

struct SCameraData {
    inline SCameraData() : Pos(), Foc()
    {

    }

    inline SCameraData(v4 pos, v4 foc)
    {
        Pos = pos;
        Foc = foc;
    }

    v4 Pos;
    v4 Foc;
};

class CCameraKeyframe {
public:
    inline CCameraKeyframe()
    {
        memset(this, 0, sizeof(CCameraKeyframe));

        ShotType = SHOT_CUT;
        TargetType = TARGET_POSITION;
    }

    inline bool HasTransition()
    {
        return TransitionTime > 0.0f;
    }
    
    float GetOrbitAngle(float time);
    SCameraData GetCameraData(float time);
public:
    // Position of the camera when using SHOT_CUT
    v4 Position;

    // Target focus of the camera, used when target type is TARGET_POSITION
    v4 Focus;

    // Transition between keyframes
    f32 TransitionTime;
    f32 HoldTime;

    struct SOrbitConfig {
        f32 Distance;
        f32 Radius;
        f32 Period;
        f32 AngleOffset;
        bool Flipped;
    } Orbit;

    union
    {
        u32 Target;
        EPlayerNumber PlayerNumber;
    };

    ECameraShotType ShotType;
    ECameraTargetType TargetType;
};

class CCameraClip {
public:
    MMString<wchar_t> Name;
    CVector<CCameraKeyframe> Keyframes;
    bool IsLooping;
};

class CCinemachine {
public:
    inline CCinemachine()
    {
        memset(this, 0, sizeof(CCinemachine));
        State = STATE_IDLE;
    }
public:
    void Play(CCameraClip* clip);
    void Stop();
    void Update();
public:
    inline void Apply(CCamera* camera)
    {
        Camera.Apply(camera);
    }
    inline CCameraClip* GetActiveClip() { return ActiveClip; }
    inline bool IsPlaying() { return State != STATE_IDLE; }
private:
    inline void Apply(SCameraData& data)
    {
        Camera.Pos = data.Pos;
        Camera.Foc = data.Foc;
    }

    inline void Apply(SCameraData& start, SCameraData& end, float factor)
    {
        Camera.Pos = start.Pos * (1.0f - factor) + end.Pos * factor;
        Camera.Foc = start.Foc * (1.0f - factor) + end.Foc * factor;
    }

    inline void SetState(ECinematicState state)
    {
        State = state;
        StateTimer = 0.0f;
    }

    inline bool HasValidClip()
    {
        return ActiveClip != NULL && ActiveClip->Keyframes.size() > 0;
    }

    inline bool HasFramesRemaining()
    {
        if (!HasValidClip()) return false;
        if (ActiveClip->IsLooping) return true;
        return Frame + 1 < ActiveClip->Keyframes.size();
    }

    inline void SetFrame(u32 frame)
    {
        SetState(STATE_PLAYING);
        Frame = frame;
    }

    inline void NextFrame()
    {
        if (!HasValidClip())
        {
            Stop();
            return;
        }

        int next_frame = Frame + 1;
        int num_frames = ActiveClip->Keyframes.size();

        // We've finished the clip
        if (next_frame == num_frames)
        {
            // If we're looping just reset back to the first frame
            if (ActiveClip->IsLooping)
            {
                SetFrame(0);
                return;
            }

            // Otherwise end the playback here
            Stop();
            return;
        }

        SetFrame(next_frame);
    }

    inline CCameraKeyframe* GetNextFrame()
    {
        if (!HasValidClip()) return NULL;
        return &ActiveClip->Keyframes[(Frame + 1) % ActiveClip->Keyframes.size()];
    }
public:
    CDebugCamera Camera;
private:
    SCameraData LerpStart;
    SCameraData LerpTarget;
    ECinematicState State;
    CCameraClip* ActiveClip;
    f32 StateTimer;
    u32 Frame;
};

extern CVector<CCameraClip*> gClips;
extern CCameraClip gStandardClip;
extern CCinemachine gCinemachine;
bool LoadCameraClips();
void UpdateDebugCameraNotInUse();

#endif // ALEAR_CAMERA_H