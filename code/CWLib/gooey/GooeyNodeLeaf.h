#ifndef GOOEY_NODE_LEAF_H
#define GOOEY_NODE_LEAF_H

#include <refcount.h>

#include "ResourceGFXTexture.h"
#include "gooey/GooeyNode.h"

class CGooeyNodeLeaf : public CGooeyNode {
protected:
    CP<RTexture> SelectionSDFTexture;
    bool HasLayout;
};

#endif // GOOEY_NODE_LEAF_H