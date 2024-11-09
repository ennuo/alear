#ifndef REFLECTION_FIND_DEPENDENCIES_H
#define REFLECTION_FIND_DEPENDENCIES_H

class CReflectionFindDependencies : public CReflectionBase, public CReflectionVisitSave {
public:
    inline bool GetSaving() { return false; }
    inline bool GetLoading() { return false; }
    inline u32 GetRevision() { return 0x272; }
    inline u32 GetBranchDescription() { return 0x4c440017; }
    inline u16 GetCustomVersion() { return ALEAR_LATEST_PLUS_ONE - 1; }
    inline u8 GetCompressionFlags() { return DEFAULT_COMPRESS_FLAGS; }
    inline u16 GetBranchID() { return 0x4c44; }
    inline u16 GetBranchRevision() { return 0x0017; }
    inline ReflectReturn ReadWrite(void* d, int size) 
    { 
        AddSize(size);
        return REFLECT_OK; 
    }
    inline void AddSize(u32 size) { Size += size; }
    inline u32 GetSize() { return Size; }
private:
    u32 Size;
};

#endif // REFLECTION_FIND_DEPENDENCIES_H