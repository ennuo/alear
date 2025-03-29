#include <StringUtil.h>
#include <cell/DebugLog.h>


#include "AlearSerialization.h"
#include "mmalex.h"
#include "hook.h"

tGatherElementMap* gGatherElementMap;
void SetGatherIndexLookup(tGatherElementMap* map)
{
    gGatherElementMap = map;
}

/*
 * Adding a callback function for loading text-based assets
 * that allows changing how certain strings are mapped
 * to array indices, so for example
 * 
 * In an array in a text-based resource, you could have [PART_TYPE_TRIGGER] automatically
 * evaluate to [5].
*/
int RemapGatherVariableString(const char* str)
{
    int radix = 10;

    if (gGatherElementMap != NULL)
    {
        typename tGatherElementMap::iterator it = gGatherElementMap->find(str);
        if (it != gGatherElementMap->end())
            return it->second;
    }

    // Skip over the g prefix if it's used for GUIDs
    int len = StringLength(str);
    if (len >= 1 && str[0] == 'g')
        str += 1;
    
    // Allow parsing hexadecimal strings if applicable
    if (strstr(str, "0x") != NULL ) radix = 16;
    int value = mmalex::strtoll(str, NULL, radix);

    return value;
}

int RemapGatherVariableScanF(const char* str, const char* format, int& variable)
{
    variable = RemapGatherVariableString(str);

    return 1;
}

void AttachSerializationHooks()
{
    MH_PokeCall(0x003fa6ec, RemapGatherVariableString);
    MH_PokeCall(0x003f9dd8, RemapGatherVariableString);
    MH_PokeCall(0x003f9db8, RemapGatherVariableString);
    MH_PokeCall(0x003f9d98, RemapGatherVariableString);
    MH_PokeCall(0x003fa0a0, RemapGatherVariableScanF);
}