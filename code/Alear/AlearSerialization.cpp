#include "AlearSerialization.h"
#include "mmalex.h"
#include "hook.h"

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
    return mmalex::strtoll(str, NULL, 10);
}

void AttachSerializationHooks()
{
    
}