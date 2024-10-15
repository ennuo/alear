#ifndef SERIALISEENUMS_H
#define SERIALISEENUMS_H

enum ReflectReturn { // file: 20
    REFLECT_OK=0,
    REFLECT_GENERIC_ERROR=1000,
    REFLECT_EXCESSIVE_DATA=1001,
    REFLECT_INSUFFICIENT_DATA=1002,
    REFLECT_EXCESSIVE_ALLOCATIONS=1003,
    REFLECT_FORMAT_TOO_NEW=1004,
    REFLECT_FORMAT_TOO_OLD=1005,
    REFLECT_COULDNT_OPEN_FILE=1006,
    REFLECT_FILEIO_FAILURE=1007,
    REFLECT_NETWORK_FAILURE=1008,
    REFLECT_NOT_IMPLEMENTED=1009,
    REFLECT_COULDNT_GET_GUID=1010,
    REFLECT_UNINITIALISED=1011,
    REFLECT_NAN=1012,
    REFLECT_INVALID=1013,
    REFLECT_RESOURCE_IN_WRONG_STATE=1014,
    REFLECT_OUT_OF_GFX_MEM=1015,
    REFLECT_OUT_OF_SYNC=1016,
    REFLECT_DECOMPRESSION_FAIL=1017,
    REFLECT_COMPRESSION_FAIL=1018,
    REFLECT_APPLICATION_QUITTING=1019,
    REFLECT_OUT_OF_MEM=1020
};

enum EStreamPriority { // file: 46
    STREAM_IMMEDIATE=1,
    STREAM_NO_STREAMING=2,
    STREAM_PRIORITY_HI=3,
    STREAM_PRIORITY_MED=4,
    STREAM_PRIORITY_LOW=5,
    STREAM_PRIORITY_DONT_DESERIALISE=65536,
    STREAM_PRIORITY_DONT_LOAD=131072,
    STREAM_PRIORITY_DONT_LOAD_CPS=262144,
    STREAM_PRIORITY_DEFAULT=1048576
};

class CStreamPriority {
public:
    inline CStreamPriority() : Value(STREAM_PRIORITY_DEFAULT) {};
    inline CStreamPriority(EStreamPriority prio) : Value(prio) {}; 
    inline CStreamPriority(int bits) : Value((EStreamPriority) bits) {};

    inline bool FlagIsSet(EStreamPriority flag_to_test)
    {
        return Value & flag_to_test;
    }

    inline void SetFlag(EStreamPriority flag_to_set, bool value)
    {
        if (value)
        {
            Value = (EStreamPriority) (Value | flag_to_set);
        }
        else
        {
            Value = (EStreamPriority) (Value & (~flag_to_set));
        }
    }

    inline EStreamPriority GetPriority()
    {
        return (EStreamPriority) (Value & 0xFFFF);
    }

    inline void SetPriority(EStreamPriority prio)
    {
        Value = prio;
    }

    inline EStreamPriority GetRawBits()
    {
        return Value;
    }

private:
    EStreamPriority Value;
};



#endif