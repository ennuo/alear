#include <Translate.h>
#include <ResourceTranslationTable.h>
#include <ResourceSystem.h>
#include <ResourceFileOfBytes.h>
#include <vector.h>
#include <filepath.h>
#include <DebugLog.h>
#include <Slot.h>

enum
{
    E_KEY_ENGLISH_TRANS = 31131,
    E_KEY_ENGLISH_PATCH_TRANS = 69914,
    E_KEY_TRANSLATIONS_RLST = 4014013721ul,
    NUM_TRANSLATION_FILES = 2
};

extern StaticCP<RTranslationTable> gPatchTrans;
extern StaticCP<RTranslationTable> gTranslationTable;

MH_DefineFunc(HashTextRangesA, 0x0009f3b0, TOC0, u32, const TextRangeA&, const TextRangeA&);

const tchar_t* CSlot::GetTranslatedName() const
{
    switch (SlotID.Type)
    {
        case LEVEL_TYPE_DEVELOPER_CREATED:
        case LEVEL_TYPE_DEVELOPER_GROUP:
        case LEVEL_TYPE_DLC_LEVEL:
        case LEVEL_TYPE_DLC_PACK:
            return Translate(MakeLamsKeyID(TranslationTag.c_str(), "_NAME"));
        default:
        {
            return (const tchar_t*)Name.c_str();
        }
    }
}

u32 HashTextRangeA(const TextRangeA& r)
{
    u32 h1 = 0, h2 = 0xc8509800ul;
    const char* c;

    for (c = r.Begin + 31; c >= r.Begin; --c)
        h1 = h1 * 0x1b + ((c < r.End) ? *c : 0x20);

    if (r.Length() > 32)
    {
        h2 = 0;
        for (c = r.Begin + 63; c >= r.Begin + 32; --c)
            h2 = h2 * 0x1b + ((c < r.End) ? *c : 0x20);
    }

    return h1 + h2 * 0xDEADBEEFul;
}

u32 MakeLamsKeyID(TextRangeA lams_key)
{
    return HashTextRangeA(lams_key);
}

u32 MakeLamsKeyID(const char* prefix, const char* suffix)
{
    return HashTextRangesA(prefix, suffix);
}

const tchar_t* Translate(u32 key)
{
    const tchar_t* out;
    if (TryTranslate(key, out))
        return out;
    return (const tchar_t*)L"";
}

bool Translate(MMString<wchar_t>& dst, const char* lams_key)
{
    const u32 key = MakeLamsKeyID(lams_key);
    const tchar_t* text = (const tchar_t*)L"";
    bool rv = TryTranslate(key, text);
    dst = (const wchar_t*)text;
    return rv;
}

CVector<CP<RTranslationTable> > gTranslations;
void RTranslationTable::GetTranslationTables(CP<RTranslationTable>& table, CP<RTranslationTable>& patch)
{
    table = LoadResourceByKey<RTranslationTable>(E_KEY_ENGLISH_TRANS);
    patch = LoadResourceByKey<RTranslationTable>(E_KEY_ENGLISH_PATCH_TRANS);

    CP<RFileOfBytes> rlst = LoadResourceByKey<RFileOfBytes>(E_KEY_TRANSLATIONS_RLST);
    rlst->BlockUntilLoaded();

    if (rlst->IsLoaded())
    {
        CVector<MMString<char> > lines;
        LinesLoad(rlst->GetData(), lines);

        for (u32 i = 0; i < lines.size(); ++i)
        {
            const char* line = lines[i].c_str();
            MMLog("%s\n", CFilePath(line).GetFilename());
            gTranslations.push_back(LoadResourceByFilename<RTranslationTable>(line));
        }
    }
    else
    {
        MMLog("couldn't find translations.rlst, not loading extra translations\n");

    }

    gTranslations.push_back(table);
    gTranslations.push_back(patch);
}

bool TryTranslate(u32 key, tchar_t const*& out)
{
    for (int i = 0; i < gTranslations.size(); ++i)
    {
        RTranslationTable* table = gTranslations[i];
        if (table == NULL || !table->IsLoaded()) continue;
        if (table->GetText(key, out)) return true;
    }

    
    out = (const tchar_t*)L" ";
    return false;
}
