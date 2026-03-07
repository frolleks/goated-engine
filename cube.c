#include "mesh.h"

static const Vertex cube_vertices[] = {
    {0.25f, 0.25f, 0.25f},    {-0.25f, 0.25f, 0.25f},  {-0.25f, -0.25f, 0.25f},
    {0.25f, -0.25f, 0.25f},   {0.25f, 0.25f, -0.25f},  {-0.25f, 0.25f, -0.25f},
    {-0.25f, -0.25f, -0.25f}, {0.25f, -0.25f, -0.25f},
};

static const Triangle cube_triangles[] = {
    {0, 1, 2}, {0, 2, 3}, {4, 7, 6}, {4, 6, 5}, {0, 3, 7}, {0, 7, 4},
    {1, 5, 6}, {1, 6, 2}, {0, 4, 5}, {0, 5, 1}, {3, 2, 6}, {3, 6, 7},
};

const Mesh cube_mesh = {
    .vertices = cube_vertices,
    .vertex_count = sizeof(cube_vertices) / sizeof(cube_vertices[0]),
    .triangles = cube_triangles,
    .triangle_count = sizeof(cube_triangles) / sizeof(cube_triangles[0]),
};
