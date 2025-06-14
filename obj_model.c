#include "obj_model.h"
#include "cds.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct{
    size_t v,vt,vn;
}vec3;

inline static void skip_line(FILE *fp){
    int c;
    while ((c = getc(fp)) != '\n' && c != EOF);
}
inline static void skip_no_mean(FILE *fp){
    register int c=getc(fp);
    while(c==' '||c=='\n'){
        c=getc(fp);
    }
    c=ungetc(c,fp);
}
inline static void read_vertex_data(FILE *fp,vec3 *vec){
    memset(vec,0,sizeof(vec3));
    register int c=0;

    fscanf(fp,"%zu",&vec->v);
    vec->v-=1;
    if(getc(fp)!='/')return;

    fscanf(fp,"%zu",&vec->vt);
    vec->vt-=1;
    if(getc(fp)!='/')return;

    fscanf(fp,"%zu",&vec->vn);
    vec->vn-=1;
}

typedef struct {
    CDS_VECTOR v,vt,vn;
    CDS_VECTOR idx;
    size_t n;
    CDS_BRTREE map;
    char *g;
    char *s;
    size_t mtl;
}OBJ_MESH_BUFFER;

static int cmp_vec3(const void *k1, const void *k2) {
    const vec3 *a = k1, *b = k2;
    if (a->v != b->v) return a->v < b->v;
    if (a->vt != b->vt) return a->vt < b->vt;
    return a->vn < b->vn;
}
static int equal_vec3(const void *k1,const void *k2){
    const vec3 *a = k1, *b = k2;
    return (a->v==b->v)&&(a->vt==b->vt)&&(a->vn==b->vn);
}

inline static void buf2buf(CDS_VECTOR *obj_buf,OBJ_MESH_BUFFER *buf){
    OBJ_MESH goal;
    goal.v=(float*)buf->v.data;
    goal.vt=(float*)buf->vt.data;
    goal.vn=(float*)buf->vn.data;
    goal.idx=(unsigned int *)buf->idx.data;
    cds_vector_init(&buf->v,sizeof(float[3]));
    cds_vector_init(&buf->vt,sizeof(float[2]));
    cds_vector_init(&buf->vn,sizeof(float[3]));
    cds_vector_init(&buf->idx,sizeof(unsigned int[3]));
    goal.n=buf->n;
    goal.g=buf->g;
    goal.s=buf->s;
    goal.mtl=buf->mtl;
    buf->n=0;
    buf->g=NULL;
    buf->s=NULL;
    buf->mtl=0;
    cds_brtree_close(&buf->map,NULL,NULL);
    cds_brtree_init(&buf->map,cmp_vec3,equal_vec3);
    cds_vector_push_back(obj_buf,&goal,sizeof(goal));
}

#define KW_V 1
#define KW_VT 2
#define KW_VN 3
#define KW_F 4
#define KW_S 5
#define KW_G 6
#define KW_O 7
#define KW_USEMTL 8
#define KW_MTLLIB 9

#define KW_NEWMTL 10
#define KW_NS 11
#define KW_KA 12
#define KW_KD 13
#define KW_KS 14
#define KW_KE 15
#define KW_NI 16
#define KW_D 17
#define KW_ILLUM 18
#define KW_MAP_KD 19
#define KW_MAP_KS 20
#define KW_MAP_KA 21
#define KW_MAP_BUMP 22
#define KW_MAP_D 23

static CDS_AVLTREE keyword_map;
static int init_map;
static int cmp_keyword(const void *k1,const void *k2){
    const char *kw1=k1,*kw2=k2;
    return strcmp(kw1,kw2)>0;
}
static int equal_keyword(const void *k1,const void *k2){
    const char *kw1=k1,*kw2=k2;
    return strcmp(kw1,kw2)==0;
}
inline static void init_keywordmap(void){
    cds_avltree_init(&keyword_map,cmp_keyword,equal_keyword);
    unsigned int value;
    #define insert(k,v) value=v,cds_avltree_insert(&keyword_map,k,sizeof(k),&value,sizeof(value))
    insert("v",KW_V);
    insert("vt",KW_VT);
    insert("vn",KW_VN);
    insert("f",KW_F);
    insert("s",KW_S);
    insert("g",KW_G);
    insert("o",KW_O);
    insert("usemtl",KW_USEMTL);
    insert("mtllib",KW_MTLLIB);
    
    insert("newmtl",KW_NEWMTL);
    insert("Ns",KW_NS);
    insert("Ka",KW_KA);
    insert("Kd",KW_KD);
    insert("Ks",KW_KS);
    insert("Ke",KW_KE);
    insert("Ni",KW_NI);
    insert("d",KW_D);
    insert("illum",KW_ILLUM);
    insert("map_Kd",KW_MAP_KD);
    insert("map_Ks",KW_MAP_KS);
    insert("map_Ka",KW_MAP_KA);
    insert("map_Bump",KW_MAP_BUMP);
    insert("map_d",KW_MAP_D);
    #undef insert
}
inline static unsigned int str2keyword(char *str){
    struct CDS_BSTREE_NODE **r=cds_avltree_find(&keyword_map,str);
    if(!r){
        return 0;
    }
    struct CDS_BSTREE_NODE *nd=*r;
    return *((unsigned int*)nd->data);
}

typedef struct OBJ_MTL_ARR{
    CDS_VECTOR v;
}OBJ_MTL_ARR;

OBJ_MTL_ARR *obj_mtl_create(void){
    OBJ_MTL_ARR *r=malloc(sizeof(OBJ_MTL_ARR));
    cds_vector_init(&r->v,sizeof(OBJ_MTL));
    return r;
}

size_t obj_mtls_size(OBJ_MTL_ARR *mtl){
    return mtl->v.size;
}

OBJ_MTL *obj_mtl_get(OBJ_MTL_ARR *mtl,size_t n){
    return cds_vector_at(&mtl->v,n,sizeof(OBJ_MTL));
}

static void delete_mtl(void *m){
    OBJ_MTL *mtl=m;
    free(mtl->map_d);
    free(mtl->map_Bump);
    free(mtl->map_Ka);
    free(mtl->map_Kd);
    free(mtl->map_Ks);
}
void obj_mtl_delete(OBJ_MTL_ARR *mtl){
    cds_vector_close(&mtl->v,(CDS_CLOSE_FUNC)delete_mtl,sizeof(OBJ_MTL));
    free(mtl);
}

inline static char *change_path(const char *origin,const char *addition){
    char *target=malloc(128);
    strcpy(target,origin);
    size_t i=strlen(target)-1;
    while(i){
        if(target[i]=='/'||target[i]=='\\'){
            break;
        }
        target[i]=0;
        i--;
    }
    strcat(target,addition);
    return target;
}

inline static void mtl_load(char *filename,OBJ_MTL_ARR *mtl,CDS_BRTREE *mtl_map){
    FILE *fp=fopen(filename,"r");
    if(!fp){
        printf("Error file %s\n",filename);
        return ;
    }
    char buf[128];
    unsigned int key;
    size_t tmp=0;

    char c;
    OBJ_MTL mtl_buf={};

    while(!feof(fp)){
        fscanf(fp,"%c",&c);
        ungetc(c,fp);
        if(c=='#'){
            skip_line(fp);
            continue;
        }
        fscanf(fp,"%s",buf);
        key=str2keyword(buf);

        switch(key){
        case KW_NEWMTL:
            if(mtl_buf.name){
                cds_vector_push_back(&mtl->v,&mtl_buf,sizeof(OBJ_MTL));
                tmp=mtl->v.size-1;
                cds_brtree_insert(mtl_map,mtl_buf.name,strlen(mtl_buf.name)+1,&tmp,sizeof(size_t));
                free(mtl_buf.name);
            }
            memset(&mtl_buf,0,sizeof(OBJ_MTL));
            fscanf(fp,"%s",buf);
            mtl_buf.name=malloc(strlen(buf)+1);
            strcpy(mtl_buf.name,buf);
            break;
        case KW_NS:
            fscanf(fp,"%f",&mtl_buf.Ns);
            break;
        case KW_KA:
            fscanf(fp,"%f %f %f",&mtl_buf.Ka[0],&mtl_buf.Ka[1],&mtl_buf.Ka[2]);
            break;
        case KW_KD:
            fscanf(fp,"%f %f %f",&mtl_buf.Kd[0],&mtl_buf.Kd[1],&mtl_buf.Kd[2]);
            break;
        case KW_KS:
            fscanf(fp,"%f %f %f",&mtl_buf.Ks[0],&mtl_buf.Ks[1],&mtl_buf.Ks[2]);
            break;
        case KW_KE:
            fscanf(fp,"%f %f %f",&mtl_buf.Ke[0],&mtl_buf.Ke[1],&mtl_buf.Ke[2]);
            break;
        case KW_NI:
            fscanf(fp,"%f",&mtl_buf.Ni);
            break;
        case KW_D:
            fscanf(fp,"%f",&mtl_buf.d);
            break;
        case KW_ILLUM:
            fscanf(fp,"%f",&mtl_buf.illum);
            break;
        case KW_MAP_KD:
            fscanf(fp,"%s\n",buf);
            mtl_buf.map_Kd=change_path(filename,buf);
            break;
        case KW_MAP_KS:
            fscanf(fp,"%s\n",buf);
            mtl_buf.map_Ks=change_path(filename,buf);
            break;
        case KW_MAP_KA:
            fscanf(fp,"%s\n",buf);
            mtl_buf.map_Ka=change_path(filename,buf);
            break;
        case KW_MAP_BUMP:
            fscanf(fp,"%s\n",buf);
            mtl_buf.map_Bump=change_path(filename,buf);
            break;
        case KW_MAP_D:
            fscanf(fp,"%s\n",buf);
            mtl_buf.map_d=change_path(filename,buf);
            break;
        default:
            printf("%s: Keyword %s is not surpported\n",filename,buf);
            skip_line(fp);
            break;
        }
    }

    cds_vector_push_back(&mtl->v,&mtl_buf,sizeof(OBJ_MTL));
    tmp=mtl->v.size-1;
    cds_brtree_insert(mtl_map,mtl_buf.name,strlen(mtl_buf.name)+1,&tmp,sizeof(size_t));
    free(mtl_buf.name);

    free(filename);
    fclose(fp);
}


OBJ_MODEL *obj_load(const char *filename,size_t *n,OBJ_MTL_ARR *mtl){
    if(!init_map){
        init_keywordmap();
        init_map=1;
    }
    FILE *fp=fopen(filename,"r");
    if(!fp){
        printf("Error file %s\n",filename);
        return NULL;
    }
    char buf[128];
    unsigned int key,tmp,squ;
    struct CDS_BSTREE_NODE **tmpnd;
    CDS_VECTOR v,vt,vn;
    OBJ_MESH_BUFFER mesh_buf={};
    CDS_VECTOR model_buf,models;
    OBJ_MODEL model={};

    CDS_BRTREE mtl_map;


    cds_vector_init(&mesh_buf.v,sizeof(float[3]));
    cds_vector_init(&mesh_buf.vt,sizeof(float[2]));
    cds_vector_init(&mesh_buf.vn,sizeof(float[3]));
    cds_vector_init(&mesh_buf.idx,sizeof(unsigned int[3]));
    cds_brtree_init(&mesh_buf.map,cmp_vec3,equal_vec3);

    cds_brtree_init(&mtl_map,cmp_keyword,equal_keyword);



    float vertex[3]={0.0f,0.0f,0.0f};
    vec3 vec={0,0,0};
    cds_vector_init(&v,sizeof(float[3]));
    cds_vector_init(&vt,sizeof(float[2]));
    cds_vector_init(&vn,sizeof(float[3]));

    cds_vector_init(&model_buf,sizeof(OBJ_MESH));
    cds_vector_init(&models,sizeof(OBJ_MODEL));

    char c;

    // int kc=0;


    while(!feof(fp)){
        fscanf(fp,"%c",&c);
        ungetc(c,fp);
        if(c=='#'){
            skip_line(fp);
            continue;
        }
        fscanf(fp,"%s",buf);
        key=str2keyword(buf);
        // printf("%d %s\n",kc,buf);
        // kc++;

        switch(key){
        case KW_V:
            fscanf(fp,"%f",&vertex[0]);
            fscanf(fp,"%f",&vertex[1]);
            fscanf(fp,"%f",&vertex[2]);
            cds_vector_push_back(&v, vertex, sizeof(float[3]));
            break;
        case KW_VT:
            fscanf(fp,"%f",&vertex[0]);
            fscanf(fp,"%f",&vertex[1]);
            cds_vector_push_back(&vt, vertex, sizeof(float[2]));
            break;
        case KW_VN:
            fscanf(fp,"%f",&vertex[0]);
            fscanf(fp,"%f",&vertex[1]);
            fscanf(fp,"%f",&vertex[2]);
            cds_vector_push_back(&vn, vertex, sizeof(float[3]));
            break;
        case KW_F:
            for(register int i=0;;i++){
                read_vertex_data(fp,&vec);
                tmpnd=cds_avltree_find(&mesh_buf.map,&vec);
                if(!tmpnd){
                    cds_vector_push_back(&mesh_buf.v,cds_vector_at(&v,vec.v,sizeof(float[3])),sizeof(float[3]));
                    cds_vector_push_back(&mesh_buf.vt,cds_vector_at(&vt,vec.vt,sizeof(float[2])),sizeof(float[2]));
                    cds_vector_push_back(&mesh_buf.vn,cds_vector_at(&vn,vec.vn,sizeof(float[3])),sizeof(float[3]));
                    tmp=mesh_buf.v.size-1;
                    cds_brtree_insert(&mesh_buf.map,&vec,sizeof(vec),&tmp,sizeof(unsigned int));
                }else{
                    tmp=*(unsigned int*)(*tmpnd)->data;
                }
                cds_vector_push_back(&mesh_buf.idx,&tmp,sizeof(unsigned int));

                if(i>2){
                    cds_vector_push_back(&mesh_buf.idx,cds_vector_at(&mesh_buf.idx,mesh_buf.idx.size-2,sizeof(unsigned int)),sizeof(unsigned int));
                    cds_vector_push_back(&mesh_buf.idx,cds_vector_at(&mesh_buf.idx,mesh_buf.idx.size-i-1,sizeof(unsigned int)),sizeof(unsigned int));
                    // printf("%d %d\n",mesh_buf.idx.size-2,mesh_buf.idx.size-i,mesh_buf.idx.size-1);
                }

                fscanf(fp,"%c",&c);
                ungetc(c,fp);
                if(c=='\n'){
                    break;
                }
            }
            break;
        case KW_S:
            if(mesh_buf.s){
                buf2buf(&model_buf,&mesh_buf);
            }
            fscanf(fp,"%s",buf);
            mesh_buf.s=malloc(strlen(buf)+1);
            strcpy(mesh_buf.s,buf);
            break;
        case KW_G:
            if(mesh_buf.g){
                buf2buf(&model_buf,&mesh_buf);
            }
            fscanf(fp,"%s",buf);
            mesh_buf.g=malloc(strlen(buf)+1);
            strcpy(mesh_buf.g,buf);
            break;
        case KW_O:
            if(model.name){
                buf2buf(&model_buf,&mesh_buf);
                model.groups=(OBJ_MESH *)model_buf.data;
                model.n=model_buf.size;
                cds_vector_push_back(&models,&model,sizeof(model));
                cds_vector_init(&model_buf,sizeof(OBJ_MESH));
            }
            fscanf(fp,"%s",buf);
            model.name=malloc(strlen(buf)+1);
            break;
        case KW_USEMTL:
            if(mesh_buf.mtl){
                buf2buf(&model_buf,&mesh_buf);
            }
            fscanf(fp,"%s",buf);
            tmpnd=cds_brtree_find(&mtl_map,buf);
            if(!tmpnd){
                printf("%s: Wrong mtl name %s\n",filename,buf);
            }
            mesh_buf.mtl=*((size_t*)(*tmpnd)->data);
            break;
        case KW_MTLLIB:
            fscanf(fp,"%s",buf);
            mtl_load(change_path(filename,buf),mtl,&mtl_map);
            break;
        default:
            printf("%s: Keyword %s is not surpported\n",filename,buf);
            skip_line(fp);
            break;
        }
        skip_no_mean(fp);
    }
    fclose(fp);

    buf2buf(&model_buf,&mesh_buf);
    model.groups=(OBJ_MESH *)model_buf.data;
    model.n=model_buf.size;
    strcpy(model.name,buf);
    cds_vector_push_back(&models,&model,sizeof(model));

    cds_brtree_close(&mtl_map,NULL,NULL);

    *n=models.size;
    return (void*)models.data;
}

void obj_unload(OBJ_MODEL *models,size_t n){
    for(register size_t i=0;i<n;i++){
        for(register size_t j=0;j<models[i].n;j++){
            free(models[i].groups[j].g);
            free(models[i].groups[j].idx);
            free(models[i].groups[j].s);
            free(models[i].groups[j].v);
            free(models[i].groups[j].vn);
            free(models[i].groups[j].vt);
        }
        free(models[i].groups);
        free(models[i].name);
    }
    free(models);
}
