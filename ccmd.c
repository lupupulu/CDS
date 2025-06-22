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

inline static int str_to_int(const char *str,int *r){
    int l=strlen(str);
    *r=0;
    for(int i=0;i<l;i++){
        if(str[i]<'0'||str[i]>'9'){
            *r=0;
            return 1;
        }
        *r*=10;
        *r+=str[i]-'0';
    }
    return 0;
}

inline static void find_required(struct CDS_BSTREE_NODE *node,int *times){
    if(node->l){
        find_required(node->l,times);
    }
    if(node->r){
        find_required(node->r,times);
    }
    CCMD_PARAM *para=(CCMD_PARAM*)(node->data);
    if(!(para->flg&CCMD_OPTIONAL)&&!para->sw){
        if(!(*times)){
            printf("%s:\n",argv[0]);
        }
        printf("    [%s]:is not defined.\n",node->key);
        (*times)++;
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
    return ;
}



int ccmd_deal(int exit){
    struct CDS_BSTREE_NODE **nd;
    char buf[128],*key,ch[2];
    int var;
    int errn=0;
    for(int i=1;i<argc;i++){
        nd=NULL;
        strcpy(buf,argv[i]);
        key=buf;
        if(key[0]=='/'||key[0]=='-'){
            key=key+1;
        }
        var=0;

        ch[0]=key[0];
        ch[1]='\0';
        nd=cds_brtree_find(&params,ch);
        if(nd){
            var=1;
            if(key[1]=='='){
                key[1]='\0';
                var=2;
            }
        }
        if(!nd){
            int len=strlen(key);
            for(int j=0;j<len;j++){
                if(key[j]=='='){
                    key[j]='\0';
                    var=j+1;
                    break;
                }
            }
            nd=cds_brtree_find(&params,key);
        }
        if(!nd){
            ch[0]='\0';
            nd=cds_brtree_find(&params,ch);
        }
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
                if(str_to_int(&key[var],&inte)){
                    printf("%s:[%s]:\"%s\" is not a int type\n",argv[0],&key[var]);
                    if(exit){
                        return 1;
                    }
                    errn+1;
                }
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
                if(str_to_int(&key[var],node->data)){
                    printf("%s:[%s]:\"%s\" is not a int type\n",argv[0],&key[var]);
                    if(exit){
                        return 1;
                    }
                    errn+1;
                }
            }else if(node->flg&CCMD_FUNCTION){
                cds_vector_push_back(&funcs,&node->data,sizeof(CCMD_FUNC));
            }else{
                (*(int*)node->data)=1;
            }
        }
    }

    int r=0;
    find_required((struct CDS_BSTREE_NODE*)params.root,&r);
    if(r){
        if(r==1){
            printf("%d param is not defined.\n",r);
        }else{
            printf("%d params are not defined.\n",r);
        }
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
