#ifndef JSON_EXT_H
#define JSON_EXT_H

#include "json.h"
#include "mmalex.h"

#include <cell/DebugLog.h>

static inline char const* json_getStringProperty(json_t const* obj, char const* property, char const* def)
{
    const char* value = json_getPropertyValue(obj, property);
    if (value == NULL) return def;
    return value;
}

static inline bool json_getBoolProperty(json_t const* obj, char const* property, bool def)
{
    json_t const* prop = json_getProperty(obj, property);
    if (prop == NULL || json_getType(prop) != JSON_BOOLEAN) return def;
    return json_getBoolean(prop);
}

static inline int json_getIntProperty(json_t const* obj, char const* property, int def)
{
    json_t const* prop = json_getProperty(obj, property);
    if (prop == NULL || json_getType(prop) != JSON_INTEGER) return def;
    return mmalex::strtoll(prop->u.value,(char**)NULL, 10);
}

static inline float json_getFloatProperty(json_t const* obj, char const* property, float def)
{
    json_t const* prop = json_getProperty(obj, property);
    if (prop == NULL || (json_getType(prop) != JSON_REAL && json_getType(prop) != JSON_INTEGER)) return def;
    return mmalex::strtod(prop->u.value, (char**)NULL);
}

static inline v4 json_getVectorProperty(json_t const* obj, char const* property, v4 def)
{
    json_t const* prop = json_getProperty(obj, property);
    if (prop == NULL || json_getType(prop) != JSON_ARRAY) return def;

    int index = 0;
    v4 dst;
    json_t const* element = json_getChild(prop);
    while (element != NULL && index < 4)
    {
        if (json_getType(element) != JSON_REAL && json_getType(element) != JSON_INTEGER)
            return def;

        dst[index] = mmalex::strtod(element->u.value, (char**)NULL);
        element = json_getSibling(element);
        index++;
    }
    
    return dst;
}

#endif // JSON_EXT_H