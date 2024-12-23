#ifndef GOOEY_NODE_CONTAINER_H
#define GOOEY_NODE_CONTAINER_H

#include "gooey/GooeyNode.h"

class CGooeyNodeContainer : public CGooeyNode {
struct ContainerAttributes : public NodeAttributes {
    v4 Tint;
};
protected:
    CRawVector<CGooeyNode*> Children;
    m44 ViewMatrix;
    u32 UniqueContainerID;
    u32 NextUniqueChildID;
    u32 CurrentInsertionIdx;
    u32 DescendantAcceptedInput;
    u32 AllowChildrenInput;
    bool UpdatingContents;
    bool NeedsLayout;
public:
    // sneaking these fields in here because there's extra padding
    bool ClipX;
    bool ClipY;
};



class CGooeySimpleFrame : public CGooeyNodeContainer {

};

#endif // GOOEY_NODE_CONTAINER_H