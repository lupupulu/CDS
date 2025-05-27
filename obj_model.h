#ifndef OBJ_MODEL_H
#define OBJ_MODEL_H

#include <stddef.h>

typedef struct {
    float *v,*vn,*vt;
    unsigned int *idx;
    size_t n;
    char *g;
    char *s;
    size_t mtl;
}OBJ_MESH;

typedef struct {
    char *name;
    float Ns;
    float Ka[3],Kd[3],Ks[3],Ke[3];
    float Ni;
    float d;
    int illum;
    char *map_Kd,*map_Ks,*map_Bump,*map_Ka;
    char *map_d;
}OBJ_MTL;

typedef struct OBJ_MTL_ARR OBJ_MTL_ARR;

OBJ_MTL_ARR *obj_mtl_create(void);
size_t obj_mtls_size(OBJ_MTL_ARR *mtl);
OBJ_MTL *obj_mtl_get(OBJ_MTL_ARR *mtl,size_t n);
void obj_mtl_delete(OBJ_MTL_ARR *mtl);

typedef struct {
    OBJ_MESH *groups;
    size_t n;
    char *name;
}OBJ_MODEL;

OBJ_MODEL *obj_load(const char *filename,size_t *n,OBJ_MTL_ARR *mtl);
void obj_unload(OBJ_MODEL *models,size_t n);

#endif