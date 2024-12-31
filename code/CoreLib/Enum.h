#ifndef ENUM_H
#define ENUM_H

struct EnumInfo {
    int Value;
    const char* Name;
};

template <typename T>
class Enum {
public:
    typedef const EnumInfo* const_iterator;
    const_iterator begin();
    const_iterator end();
};

#endif // ENUM_H