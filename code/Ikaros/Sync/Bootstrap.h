#pragma once

#include <thread.h>

namespace sync
{
    extern int gServerPort;
    extern const char* gServerAddress;

    bool Open();
    void Close();
    void Update();
}