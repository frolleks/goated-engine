#include "mesh.h"

static const Vertex object_vertices[] = {
    {0.25f, 0.25f, 0.25f},
    {-0.25f, 0.25f, 0.25f},
    {-0.25f, -0.25f, 0.25f},
    {0.25f, -0.25f, 0.25f},
    {0.25f, 0.25f, -0.25f},
    {-0.25f, 0.25f, -0.25f},
    {-0.25f, -0.25f, -0.25f},
    {0.25f, -0.25f, -0.25f},
};

static const Triangle object_triangles[] = {
    {0, 1, 2}, {0, 2, 3},
    {4, 7, 6}, {4, 6, 5},
    {0, 3, 7}, {0, 7, 4},
    {1, 5, 6}, {1, 6, 2},
    {0, 4, 5}, {0, 5, 1},
    {3, 2, 6}, {3, 6, 7},
};

const Mesh object_mesh = {
    .vertices = object_vertices,
    .vertex_count = sizeof(object_vertices) / sizeof(object_vertices[0]),
    .triangles = object_triangles,
    .triangle_count = sizeof(object_triangles) / sizeof(object_triangles[0]),
};