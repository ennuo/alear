#ifndef RESOURCE_FILE_OF_BYTES_H
#define RESOURCE_FILE_OF_BYTES_H

#include <vector.h>
#include <Resource.h>
#include <filepath.h>

class RFilename : public CResource {
public:
    const CFilePath& GetPath() const { return Path; }
protected:
    CFilePath Path;
    CFilePath SysCachePath;
};

class RFileOfBytes : public CResource {
public:
    inline ByteArray& GetData() { return Data; }
protected:
    ByteArray Data;
};

#endif // RESOURCE_FILE_OF_BYTES_H