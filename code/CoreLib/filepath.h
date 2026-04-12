#pragma once

#include <GuidHash.h>
#include <vector.h>
#include <TextRange.h>
#include <MMString.h>

#define MAX_PATH (255)
#define INVALID_FILE_HANDLE (-1)

#define FILE_BEGIN (0)
#define FILE_CURRENT (1)
#define FILE_END (2)

typedef int FileHandle;
typedef int DirHandle;

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

extern bool gGameDataReady;

class CFilePath {
public:
    static CFilePath Empty;
public:
    CFilePath();
    CFilePath(const char* filename);
    CFilePath(EFilePathRootDir root_dir, const char* filename);
    CFilePath(const CFilePath& rhs);
    
    void Assign(const char* filename);
    void Assign(EFilePathRootDir root_dir, const char* filename);

    void Append(const char* str);
    void AppendRaw(const char* str);

    const char* GetExtension() const;
    const char* GetFilename() const;
    
    void FixSlashesAndCase();
    void StripExtension();
    void StripTrailingSlash();
    void Clear();

    
    operator char const*() const { return Filepath; }
    
    inline bool IsEmpty() const { return *Filepath == '\0'; }
    inline int Length() const { return strlen(Filepath); }

    inline bool operator==(const CFilePath& rhs) { return strcmp(Filepath, rhs.Filepath) == 0; }
    inline bool operator!=(const CFilePath& rhs) { return strcmp(Filepath, rhs.Filepath) != 0; }

    inline bool IsValid() const { return !Invalid; }

    CFilePath& operator=(const CFilePath& rhs);
    CFilePath& operator=(const char* rhs);
    
    inline const char* c_str() const { return Filepath; }
private:
    char Filepath[MAX_PATH];
    bool Invalid;
};

bool FileExists(const CFilePath& fp);
bool FileStat(FileHandle h, u64& modtime, u64& size);
bool FileStat(const CFilePath& fp, u64& modtime, u64& size);
u64 FileSize(const CFilePath& fp);
u64 FileSize(FileHandle h);

void FileClose(FileHandle& h);
bool FileOpen(const CFilePath& fp, FileHandle& fd, EOpenMode mode);
u64 FileRead(FileHandle h, void* out, u64 count);
u64 FileWrite(FileHandle h, const void* bin, u64 count);
u64 FileSeek(FileHandle h, s64 newpos, u32 whence);
bool FileResize(FileHandle h, u32 newsize);
bool FileResizeNoZeroFill(const CFilePath& fp, u32 newsize);
bool FileSync(FileHandle h);

bool DirectoryOpen(const CFilePath& fp, DirHandle& fd);
void DirectoryClose(DirHandle& fd);
bool DirectoryRead(DirHandle fd, char* out, u32 outsize);

int FileAttributes(const CFilePath& fp);

bool FileUnlink(const CFilePath& fp);


typedef bool (*ParseFn)(TextRange<char>&);
bool StripAndIgnoreFileHash(TextRange<char>& range);

bool LinesLoad(const ByteArray& bytes, CVector<MMString<char> >& out, ParseFn parsefunc = &StripAndIgnoreFileHash);
bool FileLoad(const CFilePath& f, ByteArray& out, CHash* out_hash = NULL);
bool FileLoad(const CFilePath& path, CVector<MMString<char> >& out, ParseFn parsefunc = &StripAndIgnoreFileHash);

bool FileHash(const CFilePath& fp, CHash* out_hash);

// not a function in the game, just figure it'd be useful to have for JSON
char* FileLoadText(CFilePath& fp);