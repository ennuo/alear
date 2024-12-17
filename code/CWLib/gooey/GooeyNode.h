#ifndef GOOEY_NODE_H
#define GOOEY_NODE_H

#include "gooey/GooeyContainerStyles.inl"
#include "PlayerNumber.inl"

class CGooeyNodeContainer;
class CGooeyNodeManager;

template<typename T>
class StateZoomer {
public:
    T Current;
    T Target;
};

class CGooeyNode {
public:
    struct SUpdateSettings {
        EPlayerNumber PlayerNumber;
        void* Navigator;
        v2 GlobalOffset;
        bool NavigationInvalidated;
    };

    class NodeExtra {
    public:
        v4 LocalOffsetSizeVel;
    };

    class NodeAttributes {
    public:
        v4 LocalOffsetSize;
        v4 Colour;
        v4 TransitionTime;
        float Validness;
        float Selected;
    };
public:
    virtual ~CGooeyNode();
public:
    virtual int GetType();
    virtual bool IsType(int type);
    virtual bool HasTransitionedOut();
    virtual void Begin();
    virtual void End(u32);
    virtual bool PerFrameUpdate(void* settings);
protected:
    v4 RelativeOffset;
public:
    u64 UID;
    CGooeyNodeManager* Manager;
    CGooeyNodeContainer* Parent;
    u32 AcceptedInput;
protected:
    u64 RelativeToItemUID;
    bool SnapAttributes;
private:
    bool NewlyCreated;
    bool Valid;
    u8 RenderTag;
};

#endif // GOOEY_NODE_H