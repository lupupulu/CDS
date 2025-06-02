#include "ccmd.h"
#include "cds.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static int argc;
static const char **argv;

typedef struct {
    void *data;
    int size;
    int real_size;
    int flg;
    int sw;
}CCMD_PARAM;
static CDS_BRTREE params;
static CDS_VECTOR funcs;

static int cmp_str(const void *k1,const void *k2){
    const char *kw1=k1,*kw2=k2;
    return strcmp(kw1,kw2)>0;
}
static int equal_str(const void *k1,const void *k2){
    const char *kw1=k1,*kw2=k2;
    return strcmp(kw1,kw2)==0;
}

void ccmd_init(int c,const char **v){
    argc=c;
    argv=v;
    cds_brtree_init(&params,cmp_str,equal_str);
    cds_vector_init(&funcs,sizeof(CCMD_FUNC));
}

void ccmd_set_para(const char *sw,int flg,void *data){
    if(flg&CCMD_VARIABLE&&(!flg&CCMD_INT_PARAM&&!flg&CCMD_STR_PARAM)){
        flg|=CCMD_STR_PARAM;
    }
    CCMD_PARAM p={.data=data,.flg=flg,.sw=0,.size=0,.real_size=0};
    cds_brtree_insert(&params,(char*)sw,strlen(sw)+1,&p,sizeof(CCMD_PARAM));
}

inline static int str_to_int(const char *str){
    int r=0,l=strlen(str);
    for(int i=0;i<l;i++){
        r*=10;
        r+=str[i]-'0';
    }
    return r;
}

inline static int find_required(struct CDS_BSTREE_NODE *node){
    int l=0,r=0,k=0;
    if(node->l){
        l=find_required(node->l);
    }
    if(node->r){
        r=find_required(node->r);
    }
    CCMD_PARAM *para=(CCMD_PARAM*)(node->data);
    if(!(para->flg&CCMD_OPTIONAL)&&!para->sw){
        printf("    [%s]:is not defined.\n",node->key);
        k++;
    }
    CDS_VECTOR vec;
    void *i=NULL;
    if(para->flg&CCMD_OPTIONAL&&!para->sw&&para->flg&CCMD_VARIABLE){
        cds_vector_init(&vec,sizeof(void*));
        cds_vector_push_back(&vec,&i,sizeof(void*));
        (*(void**)para->data)=vec.data;
        para->size=vec.size;
        para->real_size=vec.real_size;
    }
    return l+r+k;
}



int ccmd_deal(int exit){
    struct CDS_BSTREE_NODE **nd;
    char buf[128],*key;
    int var;
    int errn=0;
    for(int i=1;i<argc;i++){
        strcpy(buf,argv[i]);
        key=buf;
        if(key[0]=='/'||key[0]=='-'){
            key=key+1;
        }
        var=0;
        int len=strlen(key);
        for(int j=0;j<len;j++){
            if(key[j]=='='){
                key[j]='\0';
                var=j+1;
                break;
            }
        }
        nd=cds_brtree_find(&params,key);
        if(!nd){
            printf("%s:[%s]:can not deal the param.\n",argv[0],key);
            if(exit){
                return 1;
            }
            errn++;
            continue;
        }
        CCMD_PARAM *node=(CCMD_PARAM*)((*nd)->data);
        CDS_VECTOR vec;
        char *str;
        int inte;
        int stride;
        if(node->flg&CCMD_VARIABLE){
            if(node->flg&CCMD_STR_PARAM){
                stride=sizeof(char*);
            }else{
                stride=sizeof(int);
            }
            if(!node->sw){
                inte=0;
                cds_vector_init(&vec,stride);
                cds_vector_push_back(&vec,&inte,sizeof(int));
                (*(void**)node->data)=vec.data;
                node->size=vec.size;
                node->real_size=vec.real_size;
                node->sw=1;
                if(node->flg&CCMD_FUNCTION){
                    cds_vector_push_back(&funcs,&node->data,sizeof(CCMD_FUNC));
                }
            }
            vec.data=(*(void**)node->data);
            vec.size=node->size;
            vec.real_size=node->real_size;
            if(node->flg&CCMD_STR_PARAM){
                str=malloc(strlen(&key[var])+1);
                strcpy(str,&key[var]);
                cds_vector_push_back(&vec,&str,sizeof(char*));
            }else if(node->flg&CCMD_INT_PARAM){
                inte=str_to_int(&key[var]);
                cds_vector_push_back(&vec,&inte,sizeof(int));
            }
            (*(void**)node->data)=vec.data;
            node->size=vec.size;
            node->real_size=vec.real_size;
            (*(int*)cds_vector_at(&vec,0,stride))++;
        }else{
            if(node->sw){
                printf("%s:[%s]:this switch is used too many times.\n",argv[0],key);
                if(exit){
                    return 1;
                }
                errn++;
                continue;
            }
            node->sw=1;
            if(node->flg&CCMD_STR_PARAM){
                (*(char**)node->data)=(char*)argv[i]+(&key[var]-buf);
            }else if(node->flg&CCMD_INT_PARAM){
                (*(int*)node->data)=str_to_int(&key[var]);
            }
            if(node->flg&CCMD_FUNCTION){
                cds_vector_push_back(&funcs,&node->data,sizeof(CCMD_FUNC));
            }
        }
    }
    int r=find_required((struct CDS_BSTREE_NODE*)params.root);
    if(r){
        printf("%s:%d param be not defined.\n",argv[0],r);
        errn+=r;
        return errn;
    }

    for(register size_t i=0;i<funcs.size;i++){
        (*(CCMD_FUNC*)cds_vector_at(&funcs,i,sizeof(CCMD_FUNC)))();
    }
    
    return errn;
}

int ccmd_param_size(void *data){
    return *(int*)data;
}
void *ccmd_str_param_at(void *data,int n){
    if(n>=ccmd_param_size(data)){
        return NULL;
    }
    return ((char**)data)[n+1];
}
int ccmd_int_param_at(void *data,int n){
    if(n>=ccmd_param_size(data)){
        return 0;
    }
    return ((int*)data)[n+1];
}

static size_t vec_size;
static void free_str(void *v){
    vec_size--;
    if(vec_size>0){
        free(*(char**)v);
    }
}
static void close_node(void *v){
    CCMD_PARAM *node=v;
    CDS_VECTOR vec;
    if(node->flg&CCMD_VARIABLE&&node->size){
        vec.data=(*(void**)node->data);
        vec.size=node->size;
        vec.real_size=node->real_size;
        if(node->flg&CCMD_STR_PARAM){
            vec_size=vec.size;
            cds_vector_close(&vec,(CDS_CLOSE_FUNC)free_str,sizeof(char*));
        }else{
            cds_vector_close(&vec,NULL,sizeof(int));
        }
    }
}
void ccmd_close(void){
    cds_brtree_close(&params,NULL,(CDS_CLOSE_FUNC)close_node);
    cds_vector_close(&funcs,NULL,sizeof(CCMD_FUNC));
}
