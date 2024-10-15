#include "serverswitcher.h"

#include "json.h"

#include "cell/DebugLog.h"
#include "vm/NativeRegistry.h"
#include "vm/NativeFunctionCracker.h"

CServerSwitcher* g_ServerSwitcher;

CServerSwitcher::CServerSwitcher()
{
    memset(Servers, 0, sizeof(Servers));

    // Use whatever is in the EBOOT as the primary server
    ServerIndex = DEFAULT_SERVER_INDEX;
    GetDefaultServerConfiguration(Servers[DEFAULT_SERVER_INDEX]);
    Size = 1;
}

bool CServerSwitcher::LoadFromFile(CFilePath& file)
{
    char* json = FileLoadText(file);
    if (json == NULL) return false;
    bool ret = LoadFromJSON(json);
    delete json;
    return ret;
}

bool CServerSwitcher::LoadFromJSON(char* json)
{
    if (json == NULL)
    {
        DebugLog("JSON configuration string was NULL!\n");
        return false;
    }

    const int MAX_SERVERS = 16;
    const int MAX_FIELDS = (4 + 1) * MAX_SERVERS; // 4 fields for each server object
    
    json_t pool[MAX_FIELDS];
    json_t const* root = json_create(json, pool, MAX_FIELDS);
    if (root == NULL)
    {
        DebugLog("Failed to load configuration file, JSON was invalid!\n");
        return false;
    }

    if (json_getType(root) != JSON_ARRAY)
    {
        DebugLog("Root of server configuration file must be a JSON array!\n");
        return false;
    }

    for (json_t const* element = json_getChild(root); element != NULL; element = json_getSibling(element))
    {
        if (Size == MAX_SERVERS)
        {
            DebugLog("No more space in server array, pruning remaining entries!\n");
            break;
        }

        if (json_getType(element) != JSON_OBJ)
        {
            DebugLog("Element in JSON array is invalid! (Not a JSON object), skipping!\n");
            continue;
        }

        const char* name = json_getPropertyValue(element, "name");
        const char* http = json_getPropertyValue(element, "http");

        if (name == NULL || http == NULL)
        {
            DebugLog("Server configuration in JSON is missing name or http field, skipping!\n");
            continue;
        }

        if (strlen(name) > SServerConfiguration::MAX_NAME_CHARS || 
            strlen(http) > SServerConfiguration::MAX_URL_CHARS)
        {
            DebugLog("Server configuration has name/http field that exceeds maximum characters, skipping!\n");
            continue;
        }

        const char* secure = json_getPropertyValue(element, "secure");
        // If no secure URL wasn't provided, default back to the http URL,
        // most servers right now only really seem to use HTTP anyway.
        if (secure == NULL) secure = http;

        if (strlen(secure) > SServerConfiguration::MAX_URL_CHARS)
        {
            DebugLog("Server configuration has https url that exceeds maximum characters, skipping!\n");
            continue;
        }

        const char* digest = json_getPropertyValue(element, "digest");
        // Pull the digest from the ELF if none was provided
        if (digest == NULL || strlen(digest) > MAX_DIGEST_CHARS) 
        {
            DebugLog("Digest was either not provided or invalid, defaulting to ELF digest key.\n");
            digest = g_ServerDigest;
        }
        
        SServerConfiguration& configuration = Servers[Size++];
        strcpy(configuration.Name, name);
        strcpy(configuration.HttpUrl, http);
        strcpy(configuration.SecureUrl, secure);
        strcpy(configuration.Digest, digest);

        DebugLog("Adding Server\n");
        DebugLog("\tName=%s\n", name);
        DebugLog("\tHTTP=%s\n", http);
        DebugLog("\tSecure=%s\n", secure);
        DebugLog("\tDigest=%s\n", digest);
    }

    return true;
}

bool CServerSwitcher::Switch(int index)
{
    // Don't switch if we're already connected to this server
    if (index == ServerIndex || index >= Size) return false;

    SServerConfiguration& config = Servers[index];
    ServerIndex = index;

    // Copy the server configuration data to the references,
    // might be better logically to override the GetURL functions?
    // But this definitely works
    g_ServerURL = config.HttpUrl;
    g_ServerSecureURL = config.SecureUrl;
    g_ServerDigest = config.Digest;

    return true;
}

void CServerSwitcher::GetDefaultServerConfiguration(SServerConfiguration& configuration)
{
    strcpy(configuration.Name, "Default");
    strcpy(configuration.HttpUrl, g_ServerURL);
    strcpy(configuration.SecureUrl, g_ServerSecureURL);
    strcpy(configuration.Digest, g_ServerDigest);
}

const char* CServerSwitcher::GetServerName(int index)
{
    if (index >= Size) return NULL;
    return Servers[index].Name;
}

const char* CServerSwitcher::GetServerURL(int index)
{
    if (index >= Size) return NULL;
    return Servers[index].HttpUrl;
}

namespace ServerSwitcherNativeFunctions
{
    int GetNumServers()
    {
        return g_ServerSwitcher->GetNumServers();
    }

    int GetServerIndex()
    {
        return g_ServerSwitcher->GetServerIndex();
    }

    const char* GetServerName(int index)
    {
        return g_ServerSwitcher->GetServerName(index);
    }

    const char* GetServerURL(int index)
    {
        return g_ServerSwitcher->GetServerURL(index);
    }

    void Switch(int index)
    {
        DebugLog("Beginning server switch...\n");
        if (g_ServerSwitcher->Switch(index)) DebugLog("Successfully switched servers!\n");
        else DebugLog("Failed to switch servers!\n");
    }

    void Register()
    {
        RegisterNativeFunction("ServerSwitcher", "GetNumServers__", true, NVirtualMachine::CNativeFunction0<int>::Call<GetNumServers>);
        RegisterNativeFunction("ServerSwitcher", "GetServerIndex__", true, NVirtualMachine::CNativeFunction0<int>::Call<GetServerIndex>);
        RegisterNativeFunction("ServerSwitcher", "GetServerName__i", true, NVirtualMachine::CNativeFunction1<const char*, int>::Call<GetServerName>);
        RegisterNativeFunction("ServerSwitcher", "GetServerURL__i", true, NVirtualMachine::CNativeFunction1<const char*, int>::Call<GetServerURL>);
        RegisterNativeFunction("ServerSwitcher", "Switch__", true, NVirtualMachine::CNativeFunction1V<int>::Call<Switch>);
    }
}

bool AlearInitServerSwitcher()
{
    DebugLog("Initializing Server Switcher\n");
    g_ServerSwitcher = new CServerSwitcher();
    CFilePath fp(FPR_GAMEDATA, "gamedata/alear/servers.json");
    if (FileExists(fp))
    {
        DebugLog("Loading server configuration file at %s\n", fp.c_str());
        g_ServerSwitcher->LoadFromFile(fp);

        // Switch to the first server providied in the configuration
        if (g_ServerSwitcher->GetNumServers() >= 2)
            g_ServerSwitcher->Switch(1);

    }
    else
    {
        DebugLog("No server configuration file exists at %s!\n", fp.c_str());
    }
    
    return true;
}