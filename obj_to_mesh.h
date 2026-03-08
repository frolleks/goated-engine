#ifndef OBJ_TO_MESH_H
#define OBJ_TO_MESH_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "mesh.h"
#include <fast_obj/fast_obj.h>

void mesh_free(Mesh *mesh);
bool fastobj_index_valid(const fastObjMesh *src, unsigned int p);
bool convert_fastobj_to_mesh(const fastObjMesh *src, Mesh *dst);

#endif