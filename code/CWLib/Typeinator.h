#ifndef TYPEINATOR_H
#define TYPEINATOR_H

#include <refcount.h>
#include <ResourceGFXTexture.h>

class CTypeinator {
    CP<RTexture> Background;
    CP<RTexture> ChatIcon;
    CP<RTexture> Tail;
    float CurAlpha;
    float DstAlpha;
    float DstCursorAlpha;
    float CurCursorAlpha;
    float DstOverlayRenderWidth;
    float CurOverlayRenderWidth;
    float CurCandidateOffset;
    float SrcCandidateOffset;
    float DstCandidateOffset;
    float CandidateMoveTime;
    int LastCandidateBase;
};

#endif // TYPEINATOR_H