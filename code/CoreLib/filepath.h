#ifndef FILE_PATH_H
#define FILE_PATH_H


#include "hook.h"

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

extern bool g_GameDataReady;

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
        shkOpd _shk_prx_opd = { (void*)0x0057bba0, (void*) TOC1 };
        ((void*(*)(CFilePath*, EFilePathRootDir, char*) )&_shk_prx_opd )(this, root_dir, filename);
    }

    inline void Assign(char const* filename)
    {
        int len = strlen(filename);
        if (0xfe > len)
        {
            Invalid = false;
            strcpy(Filepath, filename);
            Filepath[len] = '\0';
        }
        else Invalid = true;
    }
    
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

// not a function in the game, just figure it'd be useful to have for JSON
char* FileLoadText(CFilePath& fp);

#endif // FILE_PATH_H