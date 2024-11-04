#ifndef FILE_PATH_H
#define FILE_PATH_H


#include "hook.h"
#include "vector.h"
#include "TextRange.h"
#include "MMString.h"
#include "StringUtil.h"
#include "GuidHash.h"

enum EFilePathRootDir 
{
	FPR_GAMEDATA,
	FPR_BLURAY,
	FPR_SYSCACHE
};

enum EOpenMode 
{
	OPEN_READ,
	OPEN_WRITE,
	OPEN_APPEND,
	OPEN_RDWR,
	OPEN_SECURE
};

typedef int FileHandle;
typedef int DirHandle;

extern bool gGameDataReady;

class CFilePath {
public:
    inline CFilePath()
    {
        Invalid = true;
    }
    
    inline CFilePath(EFilePathRootDir root_dir, char* filename)
    {
        Assign(root_dir, filename);
    }

    inline void Assign(EFilePathRootDir root_dir, char* filename)
    {
        static shkOpd _shk_prx_opd = { (void*)0x0057bba0, (void*) TOC1 };
        ((void*(*)(CFilePath*, EFilePathRootDir, char*) )&_shk_prx_opd )(this, root_dir, filename);
    }

    inline void Assign(char const* filename)
    {
        int len = StringCopy<char, 255>(Filepath, filename);
        Invalid = 0xfe < len;
    }

    inline void StripExtension()
    {
        char* ptr = strrchr(Filepath, '.');
        if (ptr != NULL) *ptr = '\0';
    }

    inline void StripTrailingSlash()
    {
        int len = strlen(Filepath);
        if (len == 0) return;
        char& c = Filepath[len - 1];
        if (c == '/' || c == '\\')
            c = '\0';
    }

    operator char const*() const { return Filepath; }

    inline bool operator==(CFilePath& rhs) { return strcmp(Filepath, rhs.Filepath) == 0; }
    inline bool operator!=(CFilePath& rhs) { return strcmp(Filepath, rhs.Filepath) != 0; }
    
    inline const char* c_str() { return Filepath; }
private:
    char Filepath[255];
    bool Invalid;
};

bool FileExists(CFilePath& fp);
bool FileStat(FileHandle h, u64* modtime, u64* size);
bool FileStat(CFilePath& fp, u64* modtime, u64* size);

void FileClose(FileHandle* h);
bool FileOpen(CFilePath& fp, FileHandle* fd, EOpenMode mode);
u64 FileRead(FileHandle h, void* out, u64 count);
u64 FileWrite(FileHandle h, void* bin, u64 count);

typedef bool (*ParseFn)(TextRange<char>&);
bool FileLoad(CFilePath const& fp, ByteArray& bufout, CHash& hash_out);
bool FileLoad(CFilePath const& filename, ByteArray& out, ParseFn parsefunc);

// not a function in the game, just figure it'd be useful to have for JSON
char* FileLoadText(CFilePath& fp);

#endif // FILE_PATH_H