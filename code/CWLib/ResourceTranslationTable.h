#ifndef RESOURCE_TRANSLATION_TABLE_H
#define RESOURCE_TRANSLATION_TABLE_H


#include "Resource.h"

class RTranslationTable : public CResource {
struct Index {
    u32 Hash;
    union {
        u32 Offset;
        tchar_t* Text;
    };
};
public:
    bool GetText(u32 key, tchar_t const*& out);
private:
    ByteArray Data;
    Index* IndexStart;
    Index* IndexEnd;
};

extern StaticCP<RTranslationTable> gPatchTrans;
extern StaticCP<RTranslationTable> gTranslationTable;


extern u32 (*MakeLamsKeyID)(const char* prefix, const char* suffix);


#endif // RESOURCE_TRANSLATION_TABLE_H