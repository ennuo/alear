#include <Constraints.h>

#include <thing.h>

#include <PartPhysicsJoint.h>
#include <PartPhysicsBody.h>
#include <PartRenderPosition.h>

void CCompactConstraints::ReadJoint(PJoint* const* joint_list, unsigned int joint_count, float frames) // 1701
// ~200 lines?
{
    // 1705 = const float DAMPENING_FOR_SPRINGS
    // 1706 = const float DAMPENING_FOR_ANGULAR_SPRINGS
    // 1708 = const float DAMPENING_FOR_QUANTIZED 
    // 1709 = const float MIN_LENGTH

    CCompactMass* base = VecCCompactMass.begin(); // 1711
    PJoint* const* t; // 1712
    PJoint* const* next_t; // 1714

    for (t = joint_list, next_t = joint_list + 1; t != joint_list + joint_count; t++, next_t++)
    {
        const PJoint* Joint = *t;
        v2 accumulated_impulse;

        if (Joint->CurrentlyEditing) continue;
        if (!Joint->GetA() || !Joint->GetB()) continue;

        const PBody* a_body = Joint->GetA()->GetBodyRoot();
        const PBody* b_body = Joint->GetB()->GetBodyRoot();

        if (!a_body || !b_body) continue;
        if (!a_body->IsMovable() && !b_body->IsMovable()) continue;

        v2 a_force_point = Joint->GetContactPointA();
        v2 b_force_point = Joint->GetContactPointB();
        v2 rel_pos = b_force_point - a_force_point;

        switch (Joint->Type)
        {
            case JOINT_TYPE_LEGACY:
            {
                break;
            }
            case JOINT_TYPE_ELASTIC:
            {
                break;
            }
            case JOINT_TYPE_SPRING:
            {
                break;
            }
            case JOINT_TYPE_CHAIN:
            {
                break;
            }
            case JOINT_TYPE_PISTON:
            {
                break;
            }
            case JOINT_TYPE_STRING:
            {
                break;
            }
            case JOINT_TYPE_ROD:
            {
                break;
            }
            case JOINT_TYPE_BOLT:
            {
                break;
            }
            case JOINT_TYPE_SPRING_ANGULAR:
            {
                break;
            }
            case JOINT_TYPE_MOTOR:
            {
                if (Joint->AnimationPattern == JOINT_PATTERN_FLIPPER)
                {
                    
                }

                break;
            }
            case JOINT_TYPE_QUANTIZED:
            {
                break;
            }
        }

    }

    // 1725 const PJoint* Joint
    // 1727 v2 accumulated_impulse
    // 1731 const PBody* a_body
    // 1732 const PBody* b_body
    // 1737 v2 a_force_point
    // 1738 v2 b_force_point
    // 1739 v2 rel_pos

    // 1924 const PShape* a_shape
    // 1925 const PShape* b_shape
    // 1929 const CCompactConvex* a
    // 1930 const CCompactConvex* b

    // 1935 v2 a_normal
    // 1936 v2 b_normal

    // 1950 SLocalCollisionInfo lciab
    // 1951 SLocalCollisionInfo lciba

}

