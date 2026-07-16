#pragma once

#include <CollisionQuery.h>
#include <CompactCollision.h>
#include <PartShape.h>
#include <PartPhysicsWorld.h>
#include <thing.h>

template <typename T>
void IterateCollisionsOnConvex(u32 n, const PShape* shape, const T& accumulator)
{
    const PWorld* w = shape->GetThing()->GetWorld();
    CCollisionQuery* q = w->GetCollisionQuery();
    
    const CCompactConvex* convex = shape->GetConvexes().begin() + n;
    u16 c_number = convex->NewNumber;
    u32 pair = c_number << 16;

    const u32* begin = q->SortedConvexCollisionPairs;
    const u32* end = begin + q->CollisionPairCount;
    const u32* match = std::lower_bound(begin, end, pair);
    while (match != end && ((*match >> 16) == c_number))
        accumulator(convex, q->Collisions[match++ - begin]);

    begin = q->SwappedSortedConvexCollisionPairs;
    end = begin + q->CollisionPairCount;
    match = std::lower_bound(begin, end, pair);
    while (match != end && ((*match >> 16) == c_number))
        accumulator(convex, q->Collisions[q->SwappedSortedConvexCollisionPairsIndex[match++ - begin]]);
}
