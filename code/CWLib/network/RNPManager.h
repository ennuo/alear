#pragma once

class CRNPManager {
enum EPrepareStage {
    E_PREPARE_STAGE_INIT,
    E_PREPARE_STAGE_FULLY_CONNECTED
};
public:
    inline bool CanHandleCSR() const { return PrepareStage == E_PREPARE_STAGE_FULLY_CONNECTED; }
private:
    EPrepareStage PrepareStage;
};

extern CRNPManager gRNPManager;