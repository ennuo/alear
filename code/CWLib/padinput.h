#ifndef PAD_INPUT_H
#define PAD_INPUT_H

#include "PadEnums.h"

class PadData {
public:
    u16 Reserved[2];
    u32 Buttons;
    u16 RightStickX;
    u16 RightStickY;
    u16 LeftStickX;
    u16 LeftStickY;
    u16 PressureData[12];
    u16 SensorData[4];
    float SensorDataSmooth[4];
    v4 SensorDir;
    u16 SensorDataZeroCalib[4];
    float SensorDataTempCalib[4];
    u16 SensorHistory[15][4];
    u16 SensorHistoryHead;
    float *DebugHistory;
    u16 DPadStickX;
    u16 DPadStickY;
    u16 Padding;
    bool Connected;
    bool OldConnected;
    u8 NewlyConnected;
    u8 NewlyRemoved;
    u8 StateChanged;
    u8 SupportPressure;
    u8 SupportSensor;
    u8 SupportAccuator;
    u8 SensorCalibrated;
    u32 ButtonsDown;
    u32 ButtonsUp;
    u32 ButtonSupress;
    u32 OldButtons;
};

extern PadData gPadData[7];

#endif // PAD_INPUT_H