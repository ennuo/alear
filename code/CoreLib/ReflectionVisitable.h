#ifndef REFLECTION_VISITABLE_H
#define REFLECTION_VISITABLE_H



class CReflectionVisitable {
public:
    u32 VisitationStamp;
    void* Visited;
};

class CDependencyWalkable {
public:
    inline CDependencyWalkable() {
        DependencyWalkStamp = -1;
        LimitsDependencyWalkStamp = -1;
    }

    u32 DependencyWalkStamp; 
    u32 LimitsDependencyWalkStamp;
};

#endif // REFLECTION_VISITABLE_H