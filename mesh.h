#ifndef MESH_H
#define MESH_H

#include <stddef.h>
#include <stdint.h>

#include "texture_loader.h"

typedef struct {
    float x;
    float y;
    float z;
    float u;
    float v;
} Vertex;

typedef struct {
    uint32_t v1;
    uint32_t v2;
    uint32_t v3;
    uint32_t material_index;
} Triangle;

typedef struct {
    uint32_t v1;
    uint32_t v2;
} Edge;

typedef struct {
    char *diffuse_texture_path;
    uint32_t texture_index;
} Material;

typedef struct {
    Vertex *vertices;
    size_t vertex_count;

    Triangle *triangles;
    size_t triangle_count;

    Edge *edges;
    size_t edge_count;

    Material *materials;
    size_t material_count;
} Mesh;

#endif
