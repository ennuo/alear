#ifndef SACK_BOY_ANIM_H
#define SACK_BOY_ANIM_H

class RMesh;

class CSackBoyAnim {
private:
    void FreezeSolid();
    void SetEffectMesh(RMesh* mesh);

private:
    char Pad[0x548];
public:
    int ExpressionState; // honestly forgot which one this was


};

#endif // SACK_BOY_ANIM_H