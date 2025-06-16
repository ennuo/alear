#include "AlearShared.h"
#include "AlearConfig.h"
#include "AlearDebugCamera.h"
#include "InventoryItemRequest.h"

#include <MMString.h>
#include <StringUtil.h>
#include <GuidHashMap.h>
#include <Fart.h>
#include <FartRO.h>
#include <vector.h>
#include <filepath.h>
#include <mem_stl_buckets.h>
#include <map>
#include <HTTPClient.h>

class MMOTextStreamA {
public:
    struct AsHex {
        AsHex(u32 value) { Value = value; }
        u32 Value;
    };

    struct FmtInt {
        FmtInt(const char* format, u32 value) { Format = format; Value = value; }
        const char* Format;
        u32 Value;
    };
public:
    virtual ~MMOTextStreamA() = 0;
public:

    inline MMOTextStreamA& operator<<(bool v)
    {
        return *this << (v ? "true" : "false");
    }

    inline MMOTextStreamA& operator<<(int v)
    {
        char fmt[64];
        FormatString<64>(fmt, "%d", v);
        return *this << fmt;
    }

    inline MMOTextStreamA& operator<<(v4 v)
    {
        char fmt[256];
        FormatString<256>(fmt, "[%f, %f, %f]", v.getX().getAsFloat(), v.getY().getAsFloat(), v.getZ().getAsFloat());
        return *this << fmt;
    }

    inline MMOTextStreamA& operator<<(char const* s)
    {
        OutputString(s);
        return *this;
    }

    inline MMOTextStreamA& operator<<(MMString<char>& s)
    {
        OutputString(s.c_str());
        return *this;
    }
public:
    virtual void OutputData(const void* data, u32 len) = 0;
private:
    virtual void OutputString(const char* s) = 0;
    virtual void OutputString(const wchar_t* s) = 0;
    virtual void OutputString(const tchar_t* s) = 0;
};

struct SCompareIgnoreCase {
	bool operator()(const MMString<char>& a, const MMString<char>& b) const
	{
		return StringICompare(a.c_str(), b.c_str());
	}
};

typedef std::map<MMString<char>, MMString<char>, SCompareIgnoreCase, STLBucketAlloc<MMString<char> > > ParameterMap;

class CRoute;
class CWebternate {
public:
    virtual ~CWebternate() = 0;
public:
    virtual void AddRoute(CRoute* page) = 0;
    virtual void Unknown_1() = 0;
    virtual int GetPageCount() const = 0;
    virtual CRoute* GetPage(int i) const;
};
class CRoute {
public:
    inline CRoute(CWebternate* owner) : Webternate(owner) {}
    virtual ~CRoute() {}
public:
    virtual MMString<char> GetFileName(ParameterMap& parameters) 
    {
        return "\0";
    };
    
    virtual const char* GetHref() { return NULL; }
    virtual const char* GetContentType() 
    { 
        return "text/html; charset=UTF-8"; 
    }
    virtual int GetContentLength() { return -1; }
    virtual void Write(MMOTextStreamA& stream, ParameterMap& parameters, TextRange<char>& body) {};
    virtual bool IsPage() { return true; }
    virtual bool Unknown_8() { return true; }
    virtual const char* GetTitle() { return NULL; }
    virtual int GetRefreshInterval() { return -1; }

    virtual void DoNavigation(MMOTextStreamA& stream) 
    {
        // stream << "<ul>";
        // for (int i = 0; i < Webternate->GetPageCount(); ++i)
        // {
        //     CRoute* page = Webternate->GetPage(i);
        //     if (!page->IsPage() || !page->Unknown_8()) continue;
        //     stream << "<li><a href=\"" << page->GetHref() << "\">";
        //     stream << page->GetTitle();
        //     stream << "</a></li>";
        // }
        // stream << "</ul>";

        stream << "<table><tr>";
        for (int i = 0; i < Webternate->GetPageCount(); ++i)
        {
            CRoute* page = Webternate->GetPage(i);
            if (!page->IsPage() || !page->Unknown_8()) continue;
            stream << "<td><a href=\"" << page->GetHref() << "\">";
            stream << page->GetTitle();
            stream << "</a></td>";
        }
        stream << "</tr></table><br>";
    }

    virtual void DoHeader(MMOTextStreamA& stream) 
    {
        stream << "<!DOCTYPE html><html lang=\"en-US\"><head>";
        if (GetRefreshInterval() != -1)
        {
            stream << "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"";
            stream << GetRefreshInterval();
            stream << ";URL=";
            stream << GetHref();
            stream << "\">";
        }

        stream << "<meta charset=\"utf-8\" />";
        stream << "<title>" << GetTitle() << "</title>";
        stream << "<link href=\"/r?path=gamedata/alear/web/base.css\" rel=\"stylesheet\" type=\"text/css\">";

        stream << "<body>";
        DoNavigation(stream);
    }

    virtual void Unknown_13() {}

    virtual void DoFooter(MMOTextStreamA& stream) 
    {
        stream << "<p><a href=\"/\">Index</a></body></html>\n";
    }
protected:
    void DoFileContents(MMOTextStreamA& stream, const char* path)
    {
        CFilePath fp(path);
        CFileDBRow* row = FileDB::FindByPath(fp, false);
        if (row == NULL) return;

        ByteArray b;
        if (!GetFileDataFromCaches(row->FileHash, b))
        {
            CHash hash;
            fp.Assign(FPR_GAMEDATA, row->FilePathX);
            FileLoad(fp, b, hash);
        }

        if (b.empty()) return;
        stream.OutputData((const void*)b.begin(), b.size());
    }
private:
    CWebternate* Webternate;
};

class CFaviconEndpoint : public CRoute {
public:
    inline CFaviconEndpoint(CWebternate* owner) : CRoute(owner) {}
public:
    const char* GetHref() { return "favicon.ico"; }
    const char* GetContentType() { return "image/x-icon"; }
    bool IsPage() { return false; }
    void Write(MMOTextStreamA& stream, ParameterMap& parameters, TextRange<char>& body)
    {
        DoFileContents(stream, "gamedata/alear/web/favicon.ico");
    }
};

class CLookupResourceEndpoint : public CRoute {
public:
    inline CLookupResourceEndpoint(CWebternate* owner) : CRoute(owner) {}
public:
    const char* GetHref() { return "r"; }

    MMString<char> GetFileName(ParameterMap& parameters) 
    {
        // webternate is single threaded, so its fine to do this
        CachedContentType = "application/octet-stream";

        typename ParameterMap::iterator it = parameters.find("path");
        if (it != parameters.end())
        {
            const char* path = it->second.c_str();

            if (strstr(path, ".css") != NULL) CachedContentType = "text/css";
            else if (strstr(path, ".json") != NULL) CachedContentType = "application/json";
            else if (strstr(path, ".js") != NULL) CachedContentType = "text/javascript";
            else if (strstr(path, ".html") != NULL) CachedContentType = "text/html; charset=UTF-8";
            // only really need to return the filename if we're using binary content, i think?
            else return path;
        }

        return "\0";
    };

    const char* GetContentType() { return CachedContentType; }
    bool IsPage() { return false; }
    void Write(MMOTextStreamA& stream, ParameterMap& parameters, TextRange<char>& body)
    {
        typename ParameterMap::iterator it = parameters.find("path");
        if (it != parameters.end())
            DoFileContents(stream, it->second.c_str());
    }
private:
    const char* CachedContentType;
};

class CInventoryEndpoint : public CRoute {
public:
    inline CInventoryEndpoint(CWebternate* owner) : CRoute(owner) {}
public:
    const char* GetHref() { return "api/inventory"; }
    const char* GetContentType() { return "application/json"; }
    bool IsPage() { return false; }

    const char* GetItemResultCode(EItemRequestResult result)
    {
        switch (result)
        {
            case E_ITEM_NOT_FOUND: 
                return "itemNotFound";
            case E_ITEM_NO_PLAYER: 
                return "yellowheadNotFound";
            case E_ITEM_ADDED: 
                return "itemAdded"; 
            case E_ITEM_ALREADY_EXISTS: 
                return "itemAlreadyExists"; 
            case E_ITEM_FAILED_LOAD: 
                return "itemLoadFail"; 
            case E_ITEM_ALREADY_IN_PROGRESS:
                return "itemRequestPending"; 
            case E_ITEM_NO_DATA_SOURCE:
                return "noDataSource"; 
            default: 
                return "invalidState"; 
        }
    }

    void Write(MMOTextStreamA& stream, ParameterMap& parameters, TextRange<char>& body)
    {
        EPostOrGet method = (EPostOrGet)(body.Begin == NULL);
        if (method == E_HTTP_GET) 
        {
            stream << "{\"result\":\"invalidMethod\"}";
            return;
        }

        char json[256];
        EItemRequestResult result;

        typename ParameterMap::iterator it = parameters.find("path");
        if (it != parameters.end())
        {
            result = RequestItem(it->second.c_str());
        }
        else
        {
            result = RequestItem((const void*)body.Begin, body.Length());
        }

        const char* code = GetItemResultCode(result);
        FormatString<256>(json, "{\"result\":\"%s\"}", code);
        stream << json;
    }
};

void DoBooleanResponse(bool& b, const char* name, const char* key, const char* value)
{
    if (strcmp(key, name) != 0) return;
    if (strcmp(value, "toggle") == 0) b = !b;
    else b = strcmp(value, "true") == 0;
}

class CGameVarsEndpoint : public CRoute {
public:
    inline CGameVarsEndpoint(CWebternate* owner) : CRoute(owner) {}
public:
    const char* GetHref() { return "api/gamevars"; }
    const char* GetContentType() { return "application/json"; }
    bool IsPage() { return false; }

    void Write(MMOTextStreamA& stream, ParameterMap& parameters, TextRange<char>& body)
    {
        EPostOrGet method = (EPostOrGet)(body.Begin == NULL);
        
        if (method == E_HTTP_GET)
        {
            stream << "{";
                stream << "\"isSimPaused\":" << (gPauseGameSim ? "true" : "false");
            stream << "}";

            return;
        }

        const char* key = NULL;
        const char* value = NULL;

        typename ParameterMap::iterator it = parameters.find("key");
        if (it != parameters.end()) key = it->second.c_str();
        else return;

        it = parameters.find("value");
        if (it != parameters.end()) value = it->second.c_str();
        else return;

        DoBooleanResponse(gPauseGameSim, "isSimPaused", key, value);
    }
};

class CCinemachineEndpoint : public CRoute {
public:
    inline CCinemachineEndpoint(CWebternate* owner) : CRoute(owner) {}
public:
    const char* GetHref() { return "api/cinemachine"; }
    const char* GetContentType() { return "application/json"; }
    bool IsPage() { return false; }
    void Write(MMOTextStreamA& stream, ParameterMap& parameters, TextRange<char>& body)
    {
        EPostOrGet method = (EPostOrGet)(body.Begin == NULL);

        if (method == E_HTTP_POST)
        {
            const char* key = NULL;
            const char* value = NULL;

            typename ParameterMap::iterator it = parameters.find("key");
            if (it != parameters.end()) key = it->second.c_str();
            else return;

            it = parameters.find("value");
            if (it != parameters.end()) value = it->second.c_str();
            else return;

            DoBooleanResponse(gCinemachineDisableYellowHead, "intercept", key, value);
        }

        stream << "{";
            stream << "\"pos\":" << GetCameraPosition() << ",";
            stream << "\"foc\":" << GetCameraFocus() << ",";
            stream << "\"intercept\":" << gCinemachineDisableYellowHead;
        stream << "}";
    }
};

class CAurienPage : public CRoute {
public:
    inline CAurienPage(CWebternate* owner) : CRoute(owner) {}
public:
    const char* GetHref() { return "machinima"; }
    const char* GetTitle() { return "Machinima"; }
    void Write(MMOTextStreamA& stream, ParameterMap& parameters, TextRange<char>& body)
    {
        EPostOrGet method = (EPostOrGet)(body.Begin == NULL);
        if (method == E_HTTP_POST)
        {
            typename ParameterMap::iterator it = parameters.find("action");
            if (it != parameters.end())
            {
                const char* action = it->second.c_str();
                if (StringICompare(action, "record") == 0)
                {
                    gBloomRecording.ToggleRecording("output/poppet_bloom.avi");
                    gPoppetRecording.ToggleRecording("output/poppet.avi");
                }
            }

            return;
        }

        DoHeader(stream);
        DoFileContents(stream, "gamedata/alear/web/aurien.html");
        DoFooter(stream);

        return;
    }
};

class CInventoryPage : public CRoute {
public:
    inline CInventoryPage(CWebternate* owner) : CRoute(owner) {}
public:
    const char* GetHref() { return "inventory"; }
    const char* GetTitle() { return "Inventory"; }
    void Write(MMOTextStreamA& stream, ParameterMap& parameters, TextRange<char>& body)
    {
        // dumb hack, the range will always be set if we're in a POST request
        EPostOrGet method = (EPostOrGet)(body.Begin == NULL);
        if (method == E_HTTP_POST) return;

        DoHeader(stream);
        DoFileContents(stream, "gamedata/alear/web/inventory.html");
        DoFooter(stream);
    }
};


void OnWebternateSetup(CWebternate* webternate)
{
    webternate->AddRoute(new CInventoryPage(webternate));
    webternate->AddRoute(new CAurienPage(webternate));
    webternate->AddRoute(new CLookupResourceEndpoint(webternate));
    webternate->AddRoute(new CGameVarsEndpoint(webternate));
    webternate->AddRoute(new CCinemachineEndpoint(webternate));
    webternate->AddRoute(new CInventoryEndpoint(webternate));
    webternate->AddRoute(new CFaviconEndpoint(webternate));
}