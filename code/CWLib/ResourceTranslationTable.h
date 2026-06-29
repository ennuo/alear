#pragma once

#include <Resource.h>

class RTranslationTable : public CResource {
    struct Index 
    {
        u32 Hash;
        union {
            u32 Offset;
            tchar_t* Text;
        };
    };

    struct SSortByIndex
    {
        inline bool operator()(const RTranslationTable::Index& lhs, const RTranslationTable::Index& rhs) const
        {
            return lhs.Hash < rhs.Hash;
        }

        inline bool operator()(u32 lhs, const RTranslationTable::Index& rhs) const
        {
            return lhs < rhs.Hash;
        }

        inline bool operator()(const RTranslationTable::Index& lhs, u32 rhs) const
        {
            return lhs.Hash < rhs;
        }
    };

public:
    bool GetText(u32 key, tchar_t const*& out);
public:
    static void GetTranslationTables(CP<RTranslationTable>& table, CP<RTranslationTable>& patch);
private:
    ByteArray Data;
    Index* IndexStart;
    Index* IndexEnd;
};
