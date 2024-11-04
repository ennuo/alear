#ifndef SCRIPT_OBJECT_CUSTOM_H
#define SCRIPT_OBJECT_CUSTOM_H

#include <map>

#include "vm/ScriptObject.h"

template <typename T>
class CScriptObjectCustom : public CScriptObjectInstance {
    typedef std::map<CScriptObjectCustom<T>*, T*> tNativeObjectMap;
public:
    inline T* GetNativeObject() 
    {
        typename tNativeObjectMap::iterator it = NativeObjectMap.find(this);
        if (it != NativeObjectMap.end())
            return it->second;
        return NULL;
    }
protected:
    static tNativeObjectMap NativeObjectMap;
};

#endif // SCRIPT_OBJECT_CUSTOM_H