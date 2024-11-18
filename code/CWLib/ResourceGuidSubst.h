#ifndef RESOURCE_GUID_SUBST_H
#define RESOURCE_GUID_SUBST_H

#include <vector.h>
#include <refcount.h>
#include <GuidHash.h>

#include <Resource.h>

class CGuidSubst {
public:
    inline CGuidSubst() : From(), To() {}
    inline CGuidSubst(CGUID from, CGUID to) : From(from), To(to) {}
public:
    CGUID From;
    CGUID To;
};

class RGuidSubst : public CResource {
typedef CRawVector<CGuidSubst> V;
public:
    bool Get(CGUID g, CGUID& o) const;
public:
    V Substitutions;
};

namespace NGuidSubst {
    extern StaticCP<RGuidSubst> gRegionSubst;
    extern StaticCP<RGuidSubst> gButtonSubst;
    extern StaticCP<RGuidSubst> gLanguageSubst;
}

#endif // RESOURCE_GUID_SUBST_H