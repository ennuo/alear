#include "mmalex.h"

m44 GetUniformMatrix(const m44& transform)
{
    m33 mat3 = transform.getUpper3x3();
    mat3[0] /= Vectormath::Aos::length(mat3[0]);
    mat3[0] *= Vectormath::Aos::length(mat3[1]);
    m44 mat4 = m44::identity();
    mat4.setUpper3x3(mat3);
    mat4.setCol3(transform.getCol3());
    
    return mat4;
}

float Normalize(v3& col)
{
    float d = Vectormath::Aos::dot(col, col);
    if (d > 1.0e-35f)
    {
        d = mmalex::sqrtf(d);
        col *= 1.0f / d;
    }
    else
    {
        col = v3(0.0f);
        d = 0.0f;
    }

    return d;
}

void Decompose(v4& translation, m44& rotation, v3& scale, const m44& transform)
{
    m33 mat3 = transform.getUpper3x3();
    
    scale.setX(Normalize(mat3[0]));
    scale.setY(Normalize(mat3[1]));
    scale.setZ(Normalize(mat3[2]));

    if (Vectormath::Aos::determinant(rotation) < 0.0f)
    {
        mat3 *= -1.0f;
        scale *= -1.0f;
    }

    rotation = m44::identity();
    rotation.setUpper3x3(mat3);

    translation = transform.getCol3();
}

void Decompose(v4& translation, q4& rotation, v3& scale, const m44& transform)
{
    m33 mat3 = transform.getUpper3x3();

    scale.setX(Normalize(mat3[0]));
    scale.setY(Normalize(mat3[1]));
    scale.setZ(Normalize(mat3[2]));
    if (Vectormath::Aos::determinant(mat3) < 0.0f)
    {
        mat3 *= -1.0f;
        scale *= -1.0f;
    }

    rotation = q4(mat3);
    translation = transform.getCol3();
}