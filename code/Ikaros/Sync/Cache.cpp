#include <Sync/Cache.h>

#include <algorithm>
#include <functional>

CMutableCache::CMutableCache(const CFilePath& _fp) :
CFartRO(_fp, true, false, true)
{
    
}

bool CMutableCache::Put(CHash& hash_in_out, const void* bin, u32 size)
{
    CCSLock lock(&Mutex, __FILE__, __LINE__);

    CFAT fat;
    fat.hash = hash_in_out;
    fat.size = size;
    fat.offset = 0;

    for (CFAT* it = FAT.begin(); it != FAT.end(); ++it)
    {
        fat.offset = MAX(fat.offset, it->offset + it->size);
        
        // Hash is already in the FAT, we can just ignore this data.
        if (it->hash == fat.hash) return true;
    }

    FileHandle fd;
    if (!FileOpen(fp, fd, OPEN_RDWR))
        return false;

    FAT.push_back(fat);
    std::sort(FAT.begin(), FAT.end(), std::less<CFAT>());

    Footer footer;
    footer.count = FAT.size();
    footer.magic = FARC;

    FileSeek(fd, fat.offset, FILE_BEGIN);
    FileWrite(fd, bin, size);
    FileWrite(fd, (void*)FAT.begin(), FAT.size() * sizeof(CFAT));
    FileWrite(fd, (void*)&footer, sizeof(Footer));
    FileClose(fd);

    return true;
}