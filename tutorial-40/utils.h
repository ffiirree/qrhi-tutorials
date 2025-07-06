#ifndef UTILS_H
#define UTILS_H

#include <assimp/matrix4x4.h>
#include <QMatrix4x4>

namespace utils
{
    inline QMatrix4x4 to_qmatrix4x4(aiMatrix4x4 aim)
    {
        return {
            aim.a1, aim.a2, aim.a3, aim.a4, aim.b1, aim.b2, aim.b3, aim.b4,
            aim.c1, aim.c2, aim.c3, aim.c4, aim.d1, aim.d2, aim.d3, aim.d4,
        };
    }
} // namespace utils

#endif //! UTILS_H
