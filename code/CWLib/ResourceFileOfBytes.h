#ifndef RESOURCE_FILE_OF_BYTES_H
#define RESOURCE_FILE_OF_BYTES_H

#include <vector.h>
#include <Resource.h>

class RFileOfBytes : public CResource {
public:
    inline ByteArray& GetData() { return Data; }
protected:
    ByteArray Data;
};

#endif // RESOURCE_FILE_OF_BYTES_H