#include "obj_to_mesh.h"

static Edge make_edge(uint32_t a, uint32_t b) {
    if (a < b) {
        return (Edge){a, b};
    }

    return (Edge){b, a};
}

static int compare_edges(const void *lhs, const void *rhs) {
    const Edge *a = (const Edge *)lhs;
    const Edge *b = (const Edge *)rhs;

    if (a->v1 < b->v1) {
        return -1;
    }

    if (a->v1 > b->v1) {
        return 1;
    }

    if (a->v2 < b->v2) {
        return -1;
    }

    if (a->v2 > b->v2) {
        return 1;
    }

    return 0;
}

static bool build_wireframe_edges(Mesh *mesh) {
    if (!mesh || !mesh->triangles || mesh->triangle_count == 0) {
        return true;
    }

    if (mesh->triangle_count > SIZE_MAX / 3) {
        return false;
    }

    const size_t candidate_count = mesh->triangle_count * 3;
    Edge *edges = (Edge *)malloc(candidate_count * sizeof(Edge));
    if (!edges) {
        return false;
    }

    for (size_t i = 0; i < mesh->triangle_count; ++i) {
        const Triangle triangle = mesh->triangles[i];
        const size_t base = i * 3;

        edges[base + 0] = make_edge(triangle.v1, triangle.v2);
        edges[base + 1] = make_edge(triangle.v2, triangle.v3);
        edges[base + 2] = make_edge(triangle.v3, triangle.v1);
    }

    qsort(edges, candidate_count, sizeof(*edges), compare_edges);

    size_t unique_count = 0;
    for (size_t i = 0; i < candidate_count; ++i) {
        if (unique_count > 0 &&
            compare_edges(&edges[unique_count - 1], &edges[i]) == 0) {
            continue;
        }

        edges[unique_count++] = edges[i];
    }

    if (unique_count == 0) {
        free(edges);
        return true;
    }

    Edge *shrunk_edges = (Edge *)realloc(edges, unique_count * sizeof(*edges));
    mesh->edges = shrunk_edges ? shrunk_edges : edges;
    mesh->edge_count = unique_count;

    return true;
}

void mesh_free(Mesh *mesh) {
    if (!mesh)
        return;

    free(mesh->vertices);
    free(mesh->triangles);
    free(mesh->edges);

    mesh->vertices = NULL;
    mesh->triangles = NULL;
    mesh->edges = NULL;
    mesh->vertex_count = 0;
    mesh->triangle_count = 0;
    mesh->edge_count = 0;
}

bool fastobj_index_valid(const fastObjMesh *src, unsigned int p) {
    return p != 0 && p < src->position_count;
}

bool convert_fastobj_to_mesh(const fastObjMesh *src, Mesh *dst) {
    if (!src || !dst) {
        return false;
    }

    memset(dst, 0, sizeof(*dst));

    if (!src->positions || !src->indices || !src->face_vertices) {
        return false;
    }

    if (src->position_count <= 1) {
        return true;
    }

    const size_t vertex_count = (size_t)src->position_count - 1;

    Vertex *vertices = (Vertex *)malloc(vertex_count * sizeof(Vertex));
    if (!vertices) {
        return false;
    }

    for (size_t i = 0; i < vertex_count; ++i) {
        const size_t base = (i + 1) * 3;

        vertices[i].x = src->positions[base + 0];
        vertices[i].y = src->positions[base + 1];
        vertices[i].z = src->positions[base + 2];
    }

    size_t triangle_count = 0;
    size_t index_offset = 0;

    for (unsigned int face = 0; face < src->face_count; ++face) {
        const unsigned int n = src->face_vertices[face];
        const bool is_line =
            src->face_lines ? (src->face_lines[face] != 0) : false;

        if (!is_line && n >= 3) {
            triangle_count += (size_t)(n - 2);
        }

        index_offset += n;
    }

    Triangle *triangles = NULL;
    if (triangle_count > 0) {
        triangles = (Triangle *)malloc(triangle_count * sizeof(Triangle));
        if (!triangles) {
            free(vertices);
            return false;
        }
    }

    size_t out_tri = 0;
    index_offset = 0;

    for (unsigned int face = 0; face < src->face_count; ++face) {
        const unsigned int n = src->face_vertices[face];
        const bool is_line =
            src->face_lines ? (src->face_lines[face] != 0) : false;

        if (is_line || n < 3) {
            index_offset += n;
            continue;
        }

        const fastObjIndex i0 = src->indices[index_offset + 0];
        if (!fastobj_index_valid(src, i0.p)) {
            free(vertices);
            free(triangles);
            return false;
        }

        for (unsigned int k = 1; k + 1 < n; ++k) {
            const fastObjIndex i1 = src->indices[index_offset + k];
            const fastObjIndex i2 = src->indices[index_offset + k + 1];

            if (!fastobj_index_valid(src, i1.p) ||
                !fastobj_index_valid(src, i2.p)) {
                free(vertices);
                free(triangles);
                return false;
            }

            triangles[out_tri].v1 = (uint32_t)(i0.p - 1);
            triangles[out_tri].v2 = (uint32_t)(i1.p - 1);
            triangles[out_tri].v3 = (uint32_t)(i2.p - 1);
            ++out_tri;
        }

        index_offset += n;
    }

    dst->vertices = vertices;
    dst->vertex_count = vertex_count;
    dst->triangles = triangles;
    dst->triangle_count = triangle_count;

    if (!build_wireframe_edges(dst)) {
        mesh_free(dst);
        return false;
    }

    return true;
}
