#ifndef MATRIX_UTILS_H
#define MATRIX_UTILS_H

m44 GetUniformMatrix(const m44& transform);
float Normalize(v3& col);
void Decompose(v4& translation, m44& rotation, v3& scale, const m44& transform);
void Decompose(v4& translation, q4& rotation, v3& scale, const m44& transform);

#endif // MATRIX_UTILS_H