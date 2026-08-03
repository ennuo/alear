#pragma once

namespace Ib
{
    bool IsEmulator();
    bool IsUsingRecompiler();
    int GeneratePatchYML(const char* patch_name, const char* game_name, const char* title_id, const char* output_path);
    bool NeedsRebuildPatch();
}
