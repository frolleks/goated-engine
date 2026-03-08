#include "math3d.h"

Vertex rotate_y(Vertex v, float angle) {
    const float c = cosf(angle);
    const float s = sinf(angle);

    return (Vertex){
        (v.x * c) - (v.z * s), v.y, (v.x * s) + (v.z * c), v.u, v.v,
    };
}
