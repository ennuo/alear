#pragma once

#include <filepath.h>
#include <map>
#include <MMString.h>
#include <mem_stl_buckets.h>

struct SCompareIgnoreCase 
{
    inline bool operator()(const MMString<char>& lhs, const MMString<char>& rhs) const
    {
        return StringICompare(lhs.c_str(), rhs.c_str()) < 0;
    }
};

typedef std::map<MMString<char>, MMString<char>, SCompareIgnoreCase, STLBucketAlloc<std::pair<MMString<char>, MMString<char > > > > SettingsMap;
typedef std::map<MMString<char>, SettingsMap, SCompareIgnoreCase, STLBucketAlloc<std::pair<MMString<char>, SettingsMap> > > CategorySettingsMap; 

const char* copyupto(char* dst, const char* src, bool allowspace);

class CIniSettings {
public:
    CIniSettings();
public:
    bool ReadIniFile(const CFilePath& fpath);
    float GetFloat(const char* v, float d = 0.0f) const;
    int GetInt(const char* v, int d = 0) const;
    bool GetBool(const char* v, bool d = false) const;
    const char* GetString(const char* v, const char* d = NULL) const;
private:
    SettingsMap Settings;
    CategorySettingsMap CategorisedSettings;
    bool Iniitalised;
};