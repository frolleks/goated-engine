#ifndef MESH_H
#define MESH_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
    float x;
    float y;
    float z;
} Vertex;

typedef struct {
    uint32_t v1;
    uint32_t v2;
    uint32_t v3;
} Triangle;

typedef struct {
    uint32_t v1;
    uint32_t v2;
} Edge;

typedef struct {
    Vertex *vertices;
    size_t vertex_count;

    Triangle *triangles;
    size_t triangle_count;

    Edge *edges;
    size_t edge_count;
} Mesh;

#endif
