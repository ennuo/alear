#ifndef MMALEX_H
#define MMALEX_H

namespace mmalex {
    extern double (*__stod)(const char *, char **, long); 
    extern long long (*__stoll)(const char *, char **, int);
    extern float (*__FSin)(float, unsigned int);

    inline float sin(float f) { return __FSin(f, 0); }
    inline float cos(float f) { return __FSin(f, 1); }
    inline float fmod(float x, float y) { return x - trunc(x / y) * y; }
    inline double strtod(const char *_Restrict _Str, char **_Restrict _Endptr)
    {
        return __stod(_Str, _Endptr, 0);
    }

    inline long long strtoll(const char *_Restrict _Str, char **_Restrict _Endptr, int _Base)
	{
        return (__stoll(_Str, _Endptr, _Base));
	}
}



#endif // MMALEX_H