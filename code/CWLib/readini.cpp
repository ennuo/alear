#include <ReadINI.h>
#include <vector.h>
#include <mmalex.h>

const char* copyupto(char* dst, const char* src, bool allowspace)
{
    char c;
    while ((c = *src) != '\0' && IsWhiteSpace(c)) src++;

    bool quotes = false;
    while ((c = *src) != '\0')
    {
        if (c == '[' || c == ']' || c == '\n' || c == '\r') break;
        if (!allowspace && !quotes && IsWhiteSpace(c)) break;
        if (c == '#' || c == ';' || c == '=') break;

        src += 1;
        if (c == '"' && !allowspace)
        {
            quotes = !quotes;
            continue;
        }
        
        *dst++ = c;
    }

    *dst = '\0';

    while (*src != '\0' && IsWhiteSpace(*src)) src++;
    return src;
}

float CIniSettings::GetFloat(const char* v, float d) const
{
    typename SettingsMap::const_iterator it = Settings.find(v);
    if (it != Settings.end())
        return mmalex::strtod(it->second.c_str(), NULL);
    return d;
}

int CIniSettings::GetInt(const char* v, int d) const
{
    typename SettingsMap::const_iterator it = Settings.find(v);
    if (it != Settings.end())
        return mmalex::strtoll(it->second.c_str(), NULL, 10);
    return d;
}

bool CIniSettings::GetBool(const char* v, bool d) const
{
    typename SettingsMap::const_iterator it = Settings.find(v);
    if (it != Settings.end())
        return strcmp(it->second.c_str(), "false") ? true : false;
    return d;
}

const char* CIniSettings::GetString(const char* v, const char* d) const
{
    typename SettingsMap::const_iterator it = Settings.find(v);
    if (it != Settings.end())
        return it->second.c_str();
    return d;
}

bool CIniSettings::ReadIniFile(const CFilePath& fpath)
{
    CVector<MMString<char> > lines;
    if (!FileLoad(fpath, lines))
    {
        Iniitalised = true;
        return false;
    }

    char key[256] = {0};
    char val[256] = {0};
    char valspaces[256] = {0};
    char path[256] = {0};

    CVector<MMString<char> >::iterator itr = lines.begin();
    CVector<MMString<char> >::iterator end = lines.end();
    for (; itr != end; ++itr)
    {
        const char* s = itr->c_str();

        if (*s == '[') s = copyupto(path, s + 1, false);

        s = copyupto(key, s, false);

        *valspaces = '\0';
        if (*s == '=')
            s = copyupto(valspaces, s + 1, true);
        
        copyupto(val, valspaces, false);

        if (StringICompare(path, "pc") == 0) continue; 

        if (*key != '\0')
            Settings[key] = val;
    }

    Iniitalised = true;
    return true;
}


CIniSettings::CIniSettings() : Settings(), CategorisedSettings(), Iniitalised()
{
}

