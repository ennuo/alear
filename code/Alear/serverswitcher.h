#ifndef SERVER_SWITCHER_H
#define SERVER_SWITCHER_H


#include "filepath.h"

const int MAX_DIGEST_CHARS = 18;

extern const char* gServerURL;
extern const char* gServerSecureURL;
extern const char* gServerDigest;

struct __attribute__((packed, aligned(16))) SServerConfiguration
{
    static const int MAX_URL_CHARS = 127;
    static const int MAX_NAME_CHARS = 31;

    /// @brief Server instance name
    char Name[MAX_NAME_CHARS + 1];

    /// @brief HTTP connection URL
    char HttpUrl[MAX_URL_CHARS + 1];

    /// @brief HTTPS connection URL
    char SecureUrl[MAX_URL_CHARS + 1];

    /// @brief Digest key for request/response signatures
    char Digest[MAX_DIGEST_CHARS + 1];
};

class CServerSwitcher {
public:
    /// @brief The index used by the fallback server.
    static const int DEFAULT_SERVER_INDEX = 0;
    static const int MAX_SERVERS = 16;
public:
    /// @brief Constructs the server switcher with a default server
    CServerSwitcher();

    /// @brief Loads a server list from a JSON file on the disk.
    /// @param file JSON file to load
    /// @return Whether config was successfully loaded
    bool LoadFromFile(CFilePath& file);

    /// @brief Loads a server list form a JSON string
    /// @param json JSON string to load
    /// @return Whether JSON was successfully loaded
    bool LoadFromJSON(char* json);

    /// @brief Switches and connects to a new server by ID.
    /// @param index Server index to connect to
    /// @return Whether a server switch occurred
    bool Switch(int index);

    inline int GetServerIndex() { return ServerIndex; }
    inline int GetNumServers() { return Size; }

    const char* GetServerName(int index);
    const char* GetServerURL(int index);

private:
    /// @brief Fills in a server configuration with server details from the ELF
    /// @param config Configuration to fill
    static void GetDefaultServerConfiguration(SServerConfiguration& config);

private:
    SServerConfiguration Servers[MAX_SERVERS];
    int ServerIndex;
    int Size;
};

extern CServerSwitcher* gServerSwitcher;
bool AlearInitServerSwitcher();
namespace ServerSwitcherNativeFunctions
{
    void Register();
}

#endif // SERVER_SWITCHER_H