#ifndef POPIT_STYLES_H
#define POPIT_STYLES_H

class CGooeyNodeManager;
class CPoppet;

class CInfoBubble {
private:
    char Pad[0x280];
public:
    v2 BubbleMax;
    v2 BubbleMin;
};

void OnFillInfoBubbleBackground(CInfoBubble* bubble, CGooeyNodeManager* manager);
void OnFillPoppetBackground(CPoppet* poppet, float alpha);

#endif // POPIT_STYLES_H