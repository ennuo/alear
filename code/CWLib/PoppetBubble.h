#ifndef POPPET_BUBBLE_H
#define POPPET_BUBBLE_H

#include "PoppetChild.h"
#include "PolyBubble.h"

class PShape;
class CPoppetBubble : public CPoppetChild {
public:
    bool IsTweakBubble();
    void SetBubbleSticker();
    void SetBubbleRect(float width, float height);
    void SetBubbleRoundedRect(float width, float height);
    void SetBubbleBalloon(float width, float height, float egginess);
    void SetBubbleShape(PShape* shape);
    void SetBubblePolygon(const CRawVector<v2, CAllocatorMMAligned128>& poly);
    void SetBubbleCircle(float radius);
    bool SetBubbleObject(CThing* thing, bool centre);
    void SetBubbleModeID(u32 id);
    bool FindBoundaryPosition(v2 direction, v2& pos_out) const;
    m44 CalcBubbleMatrix();
    void UpdateShape(bool in_use);
private:
    bool BubbleMatrixIsDocked;
    m44 BubbleMatrix;
    u32 BubbleModeID;
    float LastScaleFactor;
    CPolyBubble TheBubble;
};


#endif // POPPET_BUBBLE_H