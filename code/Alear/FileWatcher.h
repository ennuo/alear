#ifndef FWATCH_H
#define FWATCH_H

#include <CritSec.h>
#include <cell/thread.h>
#include <filepath.h>
#include <vector.h>


typedef void (*FileWatchCallback)(CFilePath&);

class CFileWatcher {
public:
    struct CWatchFile {
        CFilePath Path;
        u64 LastModified;
        u64 Size;
        FileWatchCallback Callback;
    };
public:
    inline CFileWatcher() : Thread(), CS("fwatch"), Files() 
    {
        Thread = CreatePPUThread(&WorkerThreadFunctionStatic, (u64)this, "fwatch", 1000, 0x10000, true);
    }
public:
    bool AddFile(CFilePath& fp, FileWatchCallback cb);
private:
    static void WorkerThreadFunctionStatic(u64 arg);
private:
    void WorkerThreadFunction();
private:
    THREAD Thread;
    CCriticalSec CS;
    CVector<CWatchFile> Files;
};

extern CFileWatcher* gFileWatcher;
bool InitFileWatcher();
void CloseFileWatcher();

#endif // FWATCH_H