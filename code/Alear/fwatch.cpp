#include "fwatch.h"

#include <System.h>

CFileWatcher* gFileWatcher;

bool CFileWatcher::AddFile(CFilePath& fp, FileWatchCallback cb)
{
    CCSLock _the_lock(&CS, __FILE__, __LINE__);
    
    CWatchFile file;
    if (FileStat(fp, &file.LastModified, &file.Size))
    {
        file.Path = fp;
        file.Callback = cb;

        Files.push_back(file);
    }

    return false;
}

void CFileWatcher::WorkerThreadFunctionStatic(u64 arg)
{
    ((CFileWatcher*)arg)->WorkerThreadFunction();
    ExitPPUThread(0);
}

void CFileWatcher::WorkerThreadFunction()
{
    while (!WantQuitOrWantQuitRequested())
    {
        CS.Enter(__FILE__, __LINE__);

        CWatchFile* iter = Files.begin();
        for (; iter != Files.end(); iter++)
        {
            u64 modtime, size;
            if (FileStat(iter->Path, &modtime, &size))
            {
                if (iter->LastModified != modtime || iter->Size != size)
                {
                    iter->Callback(iter->Path);
                    iter->LastModified = modtime;
                    iter->Size = size;
                }
            }
        }

        CS.Leave();
        ThreadSleep(500);
    }
}

bool InitFileWatcher()
{
    gFileWatcher = new CFileWatcher();
    return true;
}

void CloseFileWatcher()
{
    delete gFileWatcher;
}