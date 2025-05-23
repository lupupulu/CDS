#include "cds.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#define CDS_CMP_FUNC(tp) *(const tp*)a<*(const tp*)b
int cds_compare_int(const void *a,const void *b){
    return CDS_CMP_FUNC(int);
}
int cds_compare_float(const void *a,const void *b){
    return CDS_CMP_FUNC(float);
}
int cds_compare_string(const void *a,const void *b){
    return strcmp(a,b)<0;
}
#define CDS_EQU_FUNC(tp) *(const tp*)a==*(const tp*)b
int cds_equal_int(const void *a,const void *b){
    return CDS_EQU_FUNC(int);
}
int cds_equal_float(const void *a,const void *b){
    return CDS_EQU_FUNC(float);
}
int cds_equal_string(const void *a,const void *b){
    return strcmp(a,b)==0;
}

void cds_print_int(const void *a){
    printf("%d",*(const int*)a);
}
void cds_print_float(const void *a){
    printf("%f",*(const float*)a);
}
void cds_print_string(const void *a){
    printf(a);
}

#define ABS(a) ((a)>0?(a):-(a))
#define MAX(a,b) ((a)>(b)?(a):(b))


void cds_vector_init(CDS_VECTOR *a,size_t foot){
    a->data=malloc(foot);
    a->real_size=foot;
    a->size=0;
}
void cds_vector_set_size(CDS_VECTOR *a,size_t size){
    a->data=realloc(a->data,size);
    a->real_size=size;
    if(a->size>size){
        a->size=size;
    }
}
void *cds_vector_at(CDS_VECTOR *a,size_t n,size_t foot){
    if(n>=a->size){
        return NULL;
    }
    return a->data+n*foot;
}
void cds_vector_push_back(CDS_VECTOR *a,const void *data,size_t size){
    if((a->size+1)*size>=a->real_size){
        cds_vector_set_size(a,a->real_size*2);
    }
    memcpy(a->data+a->size*size,data,size);
    a->size++;
}
void cds_vector_pop_back(CDS_VECTOR *a,CDS_CLOSE_FUNC func,size_t size){
    if(func){
        func(a->data+size*(a->size-1));
    }
    a->size--;
}
void cds_vector_close(CDS_VECTOR *a,CDS_CLOSE_FUNC func,size_t size){
    if(func){
        while(a->size){
            cds_vector_pop_back(a,func,size);
        }
    }
    free(a->data);
    a->data=NULL;
    a->real_size=0;
    a->size=0;
}
void cds_vector_print(CDS_VECTOR *a,CDS_PRINT_FUNC func,size_t foot){
    for(register size_t i=0;i<a->size;i++){
        func(a->data+i*foot);
        printf(" ");
    }
}

void cds_string_init(CDS_STRING *s,const char *dst){
    cds_vector_init(s,sizeof(char));
    if(dst){
        s->size=1;
        cds_string_add(s,dst);
    }
}

size_t cds_string_length(CDS_STRING *s){
    return s->size-1;
}

void cds_string_add(CDS_STRING *s,const char *dst){
    register size_t i=0;
    s->size--;
    for(i;dst[i]!='\0';i++){
        cds_vector_push_back(s,&dst[i],sizeof(char));
    }
    cds_vector_push_back(s,&dst[i],sizeof(char));
}



void cds_link_init(CDS_LINK *a){
    a->head.next=NULL;
    a->head.data=NULL;
}

void*cds_link_at(CDS_LINK *a,size_t n,size_t foot){
    return cds_link_get(&a->head,n);
}

void cds_link_push(struct CDS_LINK_NODE *node,const void *data,size_t data_size){
    struct CDS_LINK_NODE *new=malloc(sizeof(struct CDS_LINK_NODE));
    new->data=malloc(data_size);
    memcpy(new->data,data,data_size);
    new->next=node->next;
    node->next=new;
}

void cds_link_delete(struct CDS_LINK_NODE *node,CDS_CLOSE_FUNC func){
    if(!node->next){
        return ;
    }
    struct CDS_LINK_NODE *next=node->next;
    node->next=next->next;
    if(func){
        func(next->data);
    }
    free(next->data);
    free(next);
}

struct CDS_LINK_NODE *cds_link_get(struct CDS_LINK_NODE *node,size_t n){
    struct CDS_LINK_NODE *now=node;
    if(!now->next){
        return NULL;
    }
    for(register size_t i=0;i<n;i++){
        if(!now->next){
            return NULL;
        }
        now=now->next;
    }
    return now;
}

void cds_link_close(CDS_LINK *a,CDS_CLOSE_FUNC func){
    while(a->head.next){
        cds_link_delete(&a->head,func);
    }
}

void cds_link_print(CDS_LINK *a,CDS_PRINT_FUNC func){
    struct CDS_LINK_NODE *now=a->head.next;
    while(now){
        func(now->data);
        printf(" ");
        now=now->next;
    }
}


void cds_queue_init(CDS_QUEUE *q){
    cds_link_init(&q->linked);
    q->last=&q->linked.head;
}
void cds_queue_push(CDS_QUEUE *q,const void *data,size_t data_size){
    cds_link_push(q->last,data,data_size);
    q->last=q->last->next;
}
void cds_queue_pop(CDS_QUEUE *q,CDS_CLOSE_FUNC func){
    cds_link_delete(&q->linked.head,func);
    if(cds_queue_empty(q)){
        q->last=&q->linked.head;
    }
}
void*cds_queue_front(CDS_QUEUE *q){
    if(q->linked.head.next){
        return q->linked.head.next->data;
    }
    return NULL;
}
int cds_queue_empty(CDS_QUEUE *q){
    return !(q->linked.head.next);
}
void cds_queue_close(CDS_QUEUE *q,CDS_CLOSE_FUNC func){
    cds_link_close(&q->linked,func);
    q->last=NULL;
}
void cds_queue_print(CDS_QUEUE *q,CDS_PRINT_FUNC func){
    cds_link_print(&q->linked,func);
}







#define CDS_TREE_L 0
#define CDS_TREE_R 1

inline static void cds_bstree_make_node(struct CDS_BSTREE_NODE **nd,size_t nd_size,void *key,void *value,size_t value_size){
    *nd=malloc(nd_size);
    struct CDS_BSTREE_NODE *node=*nd;
    node->data=malloc(value_size);
    memcpy(node->data,value,value_size);
    node->key=key;
    node->l=NULL;
    node->r=NULL;
}

typedef void(*CDS_TREE_NODE_FUNC)(void*);

inline static struct CDS_BSTREE_NODE **cds_bstree_recursion(struct CDS_BSTREE_NODE **nd,CDS_COMPARE_FUNC cmp,CDS_COMPARE_FUNC equal,void *key,CDS_TREE_NODE_FUNC f){
    if(f){
        f(nd);
    }
    if(equal((*nd)->key,(unsigned char *)key)){
        return nd;
    }
    if(cmp((unsigned char *)key,(*nd)->key)){
        if((*nd)->l){
            return cds_bstree_recursion(&(*nd)->l,cmp,equal,key,f);
        }else{
            return nd;
        }
    }else{
        if((*nd)->r){
            return cds_bstree_recursion(&(*nd)->r,cmp,equal,key,f);
        }else{
            return nd;
        }
    }
    return nd;
}

inline static void cds_bstree_close_recursion(struct CDS_BSTREE_NODE *node,CDS_CLOSE_FUNC key_f,CDS_CLOSE_FUNC value_f){
    if(node->l){
        cds_bstree_close_recursion(node->l,key_f,value_f);
    }
    if(node->r){
        cds_bstree_close_recursion(node->r,key_f,value_f);
    }
    if(key_f){
        key_f(node->key);
    }
    free(node->key);
    if(value_f){
        value_f(node->data);
    }
    free(node->data);
    free(node);
}

inline static struct CDS_BSTREE_NODE *cds_bstree_delete_recursion(struct CDS_BSTREE_NODE **node,int dir,CDS_TREE_NODE_FUNC f){
    if(f){
        f(node);
    }
    struct CDS_BSTREE_NODE *tmp;
    if(dir){
        if(!((*node)->r)){
            tmp=*node;
            *node=(*node)->l;
            return tmp;
        }else{
            return cds_bstree_delete_recursion(&((*node)->r),dir,f);
        }
    }else{
        if(!((*node)->l)){
            tmp=*node;
            *node=(*node)->r;
            return tmp;
        }else{
            return cds_bstree_delete_recursion(&((*node)->l),dir,f);
        }
    }
    return NULL;
}


void cds_bstree_init(CDS_BSTREE *t,CDS_COMPARE_FUNC cmp,CDS_COMPARE_FUNC equal){
    t->cmp=cmp;
    t->equal=equal;
    t->root=NULL;
}
void cds_bstree_insert(CDS_BSTREE *t,void *key,size_t key_size,void *value,size_t value_size){
    void *_key=malloc(key_size);
    memcpy(_key,key,key_size);

    if(!t->root){
        cds_bstree_make_node(&t->root,sizeof(struct CDS_BSTREE_NODE),_key,value,value_size);
        return ;
    }

    struct CDS_BSTREE_NODE *node=*cds_bstree_recursion(&t->root,t->cmp,t->equal,_key,NULL);
    if(t->equal(_key,node->key)){
        return ;
    }
    if(t->cmp(_key,node->key)){
        cds_bstree_make_node(&node->l,sizeof(struct CDS_BSTREE_NODE),_key,value,value_size);
    }else{
        cds_bstree_make_node(&node->r,sizeof(struct CDS_BSTREE_NODE),_key,value,value_size);
    }
} 
struct CDS_BSTREE_NODE **cds_bstree_find(CDS_BSTREE *t,void *key){
    if(!t->root){
        return NULL;
    }
    struct CDS_BSTREE_NODE **node=cds_bstree_recursion(&t->root,t->cmp,t->equal,key,NULL);
    if(t->equal(key,(*node)->key)){
        return node;
    }
    return NULL;
}
void*cds_bstree_at(CDS_BSTREE *t,void *key,size_t key_size){
    struct CDS_BSTREE_NODE **r=cds_bstree_find(t,key);
    if(r){
        return (*r)->data;
    }
    return NULL;
}
void cds_bstree_delete(struct CDS_BSTREE_NODE **node,CDS_CLOSE_FUNC key_f,CDS_CLOSE_FUNC value_f){
    if(key_f){
        key_f((*node)->key);
    }
    free((*node)->key);
    if(value_f){
        value_f((*node)->data);
    }
    free((*node)->data);
    if(!(*node)->l){
        *node=(*node)->r;
        return ;
    }else if(!(*node)->r){
        *node=(*node)->l;
        return ;
    }
    int dir=rand()%2;
    struct CDS_BSTREE_NODE *r;
    if((dir&&!(*node)->l)||(!dir&&!(*node)->r)){
        dir=!dir;
    }
    if(dir){
        r=cds_bstree_delete_recursion(&(*node)->l,dir,NULL);
    }else{
        r=cds_bstree_delete_recursion(&(*node)->r,dir,NULL);
    }
    r->r=(*node)->r;
    r->l=(*node)->l;
    free(*node);
    *node=r;
}
void cds_bstree_close(CDS_BSTREE *t,CDS_CLOSE_FUNC key_f,CDS_CLOSE_FUNC value_f){
    if(!t->root){
        return ;
    }
    cds_bstree_close_recursion(t->root,key_f,value_f);
    t->root=NULL;
}


inline static size_t cds_bstree_get_deep_recursion(struct CDS_BSTREE_NODE *node){
    size_t n=1;
    if(node->l){
        n=cds_bstree_get_deep_recursion(node->l)+1;
    }
    if(node->r){
        n=MAX(cds_bstree_get_deep_recursion(node->r)+1,n);
    }
    return n;
}

inline static void cds_bstree_print_space(size_t n){
    for(register size_t i=0;i<n;i++){
        printf(" ");
    }
}
inline static void cds_bstree_print_line(size_t n){
    for(register size_t i=0;i<n;i++){
        printf("-");
    }
}
void cds_bstree_print(CDS_BSTREE *t,CDS_PRINT_FUNC key_print,size_t key_length,CDS_PRINT_FUNC value_print,size_t value_length,CDS_PRINT_FUNC extra_print,size_t extra_length){
    if(!t->root){
        return ;
    }
    size_t deep=cds_bstree_get_deep_recursion(t->root);
    size_t len=key_length+value_length+extra_length+1;
    if(extra_length&&extra_print){
        ++len;
    }
    //x*x-x+1
    #define f(x) ((x)*(x)-(x)+1)
    CDS_QUEUE q[2];
    cds_queue_init(&q[0]);
    cds_queue_init(&q[1]);

    struct CDS_BSTREE_NODE *nd=NULL;

    cds_queue_push(&q[(deep-1)&1],&t->root,sizeof(t->root));
    while(deep--){
        if(deep){
            cds_bstree_print_space(f(deep)*len);
        }
        while(!cds_queue_empty(&q[deep&1])){
            struct CDS_BSTREE_NODE *node=cds_at(queue,struct CDS_BSTREE_NODE *,&q[deep&1],0);

            if(node==NULL){
                cds_queue_push(&q[!(deep&1)],&nd,sizeof(nd));
                cds_queue_push(&q[!(deep&1)],&nd,sizeof(nd));
                cds_bstree_print_line((len-3)/2);
                printf("NUL");
                cds_bstree_print_line(len-3-(len-3)/2);
                goto POP;
            }

            key_print(node->key);
            printf(":");
            key_print(node->data);
            if(extra_print&&extra_length){
                printf(";");
                extra_print(node);
            }

            if(node->l){
                cds_queue_push(&q[!(deep&1)],&node->l,sizeof(node->l));
            }else{
                cds_queue_push(&q[!(deep&1)],&nd,sizeof(nd));
            }

            if(node->r){
                cds_queue_push(&q[!(deep&1)],&node->r,sizeof(node->r));
            }else{
                cds_queue_push(&q[!(deep&1)],&nd,sizeof(nd));
            }


            POP:
            if(deep){
                cds_bstree_print_space(f(deep)*len*2+len);
            }else{
                cds_bstree_print_space(f(deep)*(len));
            }
            cds_queue_pop(&q[deep&1],NULL);
        }
        printf("\n");
    }
    #undef f
    cds_queue_close(&q[0],NULL);
    cds_queue_close(&q[1],NULL);

}

size_t cds_bstree_get_node_num(struct CDS_BSTREE_NODE *node){
    if(!node){
        return 0;
    }
    return cds_bstree_get_node_num(node->l)+cds_bstree_get_node_num(node->r)+1;
}






inline static size_t cds_avltree_get_height(struct CDS_AVLTREE_NODE *nd){
    if(!nd){
        return 0;
    }
    return nd->height;
}

inline static void cds_avltree_rotate_left(struct CDS_AVLTREE_NODE **nd){
    struct CDS_AVLTREE_NODE *r=(*nd)->r,*now=*nd;
    *nd=r;
    now->r=r->l;
    r->l=now;
    now->height=MAX(cds_avltree_get_height(now->l)+1,cds_avltree_get_height(now->r)+1);
    r->height=MAX(now->height+1,r->height);
}
inline static void cds_avltree_rotate_right(struct CDS_AVLTREE_NODE **nd){
    struct CDS_AVLTREE_NODE *l=(*nd)->l,*now=*nd;
    *nd=l;
    now->l=l->r;
    l->r=now;
    now->height=MAX(cds_avltree_get_height(now->l)+1,cds_avltree_get_height(now->r)+1);
    l->height=MAX(now->height+1,l->height);
}





typedef struct {
    struct CDS_AVLTREE_NODE **node;
    size_t height;
}CDS_AVLTREE_RETURN_T;

inline static void cds_avltree_balance(struct CDS_AVLTREE_NODE **nd){
    if(!(*nd)->l){
        cds_avltree_rotate_left(nd);
        return ;
    }
    if(!(*nd)->r){
        cds_avltree_rotate_right(nd);
        return ;
    }

    if((*nd)->l->height>(*nd)->r->height){
        if(cds_avltree_get_height((*nd)->l->r)>cds_avltree_get_height((*nd)->l->l)){
            cds_avltree_rotate_left(&(*nd)->l);
            cds_avltree_rotate_right(nd);
        }else{
            cds_avltree_rotate_right(nd);
        }
    }else{
        if(cds_avltree_get_height((*nd)->r->l)>cds_avltree_get_height((*nd)->r->r)){
            cds_avltree_rotate_right(&(*nd)->r);
            cds_avltree_rotate_left(nd);
        }else{
            cds_avltree_rotate_left(nd);
        }
    }
}

inline static void cds_avltree_recursion_func(void *p){
    struct CDS_AVLTREE_NODE **nd=p;
    (*nd)->lazy_tag=1;
}

inline static void cds_avltree_height_recursion(struct CDS_AVLTREE_NODE **nd){
    (*nd)->lazy_tag=0;
    if((*nd)->l&&(*nd)->l->lazy_tag){
        cds_avltree_height_recursion(&(*nd)->l);
        (*nd)->height=MAX((*nd)->height,(*nd)->l->height+1);
    }
    if((*nd)->r&&(*nd)->r->lazy_tag){
        cds_avltree_height_recursion(&(*nd)->r);
        (*nd)->height=MAX((*nd)->height,(*nd)->r->height+1);
    }
    if(!(*nd)->l&&!(*nd)->r){
        (*nd)->height=1;
    }
    if(ABS(cds_avltree_get_height((*nd)->l)-cds_avltree_get_height((*nd)->r))>1){
        cds_avltree_balance(nd);
    }
}


void cds_avltree_init(CDS_AVLTREE *t,CDS_COMPARE_FUNC cmp,CDS_COMPARE_FUNC equal){
    t->cmp=cmp;
    t->equal=equal;
    t->root=NULL;
}
void cds_avltree_insert(CDS_AVLTREE *t,void *key,unsigned key_size,void *value,unsigned value_size){
    void *_key=malloc(key_size);
    memcpy(_key,key,key_size);

    if(!t->root){
        cds_bstree_make_node((struct CDS_BSTREE_NODE **)&t->root,sizeof(struct CDS_AVLTREE_NODE),_key,value,value_size);
        t->root->lazy_tag=0;
        return ;
    }

    struct CDS_AVLTREE_NODE *node=(struct CDS_AVLTREE_NODE *)*cds_bstree_recursion((struct CDS_BSTREE_NODE **)&t->root,t->cmp,t->equal,_key,cds_avltree_recursion_func);
    if(t->equal(_key,node->key)){
        return ;
    }
    if(t->cmp(_key,node->key)){
        cds_bstree_make_node((struct CDS_BSTREE_NODE **)&node->l,sizeof(struct CDS_AVLTREE_NODE),_key,value,value_size);
        node->l->height=1;
        node->l->lazy_tag=0;
    }else{
        cds_bstree_make_node((struct CDS_BSTREE_NODE **)&node->r,sizeof(struct CDS_AVLTREE_NODE),_key,value,value_size);
        node->r->height=1;
        node->r->lazy_tag=0;
    }
    
    cds_avltree_height_recursion(&t->root);
}
void cds_avltree_delete(struct CDS_AVLTREE_NODE **node,CDS_CLOSE_FUNC key_f,CDS_CLOSE_FUNC value_f){
    if(key_f){
        key_f((*node)->key);
    }
    free((*node)->key);
    if(value_f){
        value_f((*node)->data);
    }
    free((*node)->data);
    if(!(*node)->l){
        *node=(*node)->r;
        return ;
    }else if(!(*node)->r){
        *node=(*node)->l;
        return ;
    }
    int dir=rand()%2;
    struct CDS_AVLTREE_NODE *r;
    if((dir&&!(*node)->l)||(!dir&&!(*node)->r)){
        dir=!dir;
    }
    if(dir){
        r=(struct CDS_AVLTREE_NODE *)cds_bstree_delete_recursion((struct CDS_BSTREE_NODE **)&(*node)->l,dir,cds_avltree_recursion_func);
    }else{
        r=(struct CDS_AVLTREE_NODE *)cds_bstree_delete_recursion((struct CDS_BSTREE_NODE **)&(*node)->r,dir,cds_avltree_recursion_func);
    }
    r->r=(*node)->r;
    r->l=(*node)->l;
    free(*node);
    *node=r;
    (*node)->height=MAX(cds_avltree_get_height((*node)->l)+1,cds_avltree_get_height((*node)->r)+1);
    cds_avltree_height_recursion(node);
}
void cds_avltree_close(CDS_AVLTREE *t,CDS_CLOSE_FUNC key_f,CDS_CLOSE_FUNC value_f){
    if(!t->root){
        return ;
    }
    cds_bstree_close_recursion((struct CDS_BSTREE_NODE *)t->root,key_f,value_f);
    t->root=NULL;
}



#define CDS_BRTREE_BLACK 0
#define CDS_BRTREE_RED   1

inline static void cds_brtree_rotate_left(struct CDS_BRTREE_NODE** nd){
    struct CDS_BRTREE_NODE* r=(*nd)->r;
    
    // 更新父指针
    r->parent=(*nd)->parent;
    (*nd)->parent=r;
    
    (*nd)->r=r->l;
    if(r->l){
        r->l->parent=*nd;
    }
    r->l=*nd;
    
    r->color=(*nd)->color;
    (*nd)->color=CDS_BRTREE_RED;
    *nd=r;
}

inline static void cds_brtree_rotate_right(struct CDS_BRTREE_NODE** nd){
    struct CDS_BRTREE_NODE* l=(*nd)->l;
    
    // 更新父指针
    l->parent=(*nd)->parent;
    (*nd)->parent=l;
    
    // 调整子树连接
    (*nd)->l=l->r;
    if(l->r){
        l->r->parent=*nd;
    }
    l->r=*nd;
    
    // 颜色调整
    l->color=(*nd)->color;
    (*nd)->color=CDS_BRTREE_RED;
    
    // 更新节点指针
    *nd=l;
}


inline static void cds_brtree_flip_color(struct CDS_BRTREE_NODE* nd){
    nd->color=!nd->color;
    nd->l->color=!nd->l->color;
    nd->r->color=!nd->r->color;
}

inline static int cds_brtree_is_red(struct CDS_BRTREE_NODE* nd){
    return nd&&nd->color==CDS_BRTREE_RED;
}

inline static void cds_brtree_fix_insert(struct CDS_BRTREE_NODE** nd){
    if(cds_brtree_is_red((*nd)->r)&&!cds_brtree_is_red((*nd)->l)){
        cds_brtree_rotate_left(nd);
    }
    if(cds_brtree_is_red((*nd)->l)&&cds_brtree_is_red((*nd)->l->l)){
        cds_brtree_rotate_right(nd);
    }
    if(cds_brtree_is_red((*nd)->l)&&cds_brtree_is_red((*nd)->r)){
        cds_brtree_flip_color(*nd);
    }
}

inline static void cds_brtree_make_node(struct CDS_BRTREE_NODE **nd,void *key,void *value,unsigned value_size,struct CDS_BRTREE_NODE *parent){
    *nd=malloc(sizeof(struct CDS_BRTREE_NODE));
    struct CDS_BRTREE_NODE *node=*nd;
    node->data=malloc(value_size);
    memcpy(node->data,value,value_size);
    node->key=key;
    node->l=NULL;
    node->r=NULL;
    node->color=CDS_BRTREE_RED;
    node->parent=parent;
}



void cds_brtree_init(CDS_BRTREE *t,CDS_COMPARE_FUNC cmp,CDS_COMPARE_FUNC equal){
    t->cmp=cmp;
    t->equal=equal;
    t->root=NULL;
}

inline static void cds_brtree_insert_recursion(struct CDS_BRTREE_NODE** nd,CDS_COMPARE_FUNC cmp,void* key,void* value,unsigned value_size,struct CDS_BRTREE_NODE *p){
    if(!*nd){
        cds_brtree_make_node(nd,key,value,value_size,p);
        return;
    }

    // 处理重复键
    if(cds_equal_int(key,(*nd)->key)){
        return;
    }

    if(cmp(key,(*nd)->key)){
        cds_brtree_insert_recursion(&(*nd)->l,cmp,key,value,value_size,*nd);
    }else{
        cds_brtree_insert_recursion(&(*nd)->r,cmp,key,value,value_size,*nd);
    }
    
    cds_brtree_fix_insert(nd);
}
void cds_brtree_insert(CDS_BRTREE* t,void* key,unsigned key_size,void* value,unsigned value_size){
    void* _key=malloc(key_size);
    memcpy(_key,key,key_size);
    cds_brtree_insert_recursion(&t->root,t->cmp,_key,value,value_size,NULL);
    t->root->color=CDS_BRTREE_BLACK;
}

inline static void cds_brtree_fix_delete(struct CDS_BRTREE_NODE** node){
    while(*node&&(*node)->color==CDS_BRTREE_BLACK){
        struct CDS_BRTREE_NODE* sibling;
        
        if(node==&(*node)->parent->l){
            /* 左子树处理 */
            sibling=(*node)->parent->r;
            if(cds_brtree_is_red(sibling)){
                sibling->color=CDS_BRTREE_BLACK;
                (*node)->parent->color=CDS_BRTREE_RED;
                cds_brtree_rotate_left(&(*node)->parent);
                sibling=(*node)->parent->r;
            }
            if(!cds_brtree_is_red(sibling->l)&&!cds_brtree_is_red(sibling->r)){
                sibling->color=CDS_BRTREE_RED;
                node=&(*node)->parent;
            }else{
                if(!cds_brtree_is_red(sibling->r)){
                    sibling->l->color=CDS_BRTREE_BLACK;
                    sibling->color=CDS_BRTREE_RED;
                    cds_brtree_rotate_right(&sibling);
                    sibling=(*node)->parent->r;
                }
                sibling->color=(*node)->parent->color;
                (*node)->parent->color=CDS_BRTREE_BLACK;
                sibling->r->color=CDS_BRTREE_BLACK;
                cds_brtree_rotate_left(&(*node)->parent);
                node=&(*node)->parent;
            }
        }else{
            /* 右子树处理 */
            sibling=(*node)->parent->l;
            if(cds_brtree_is_red(sibling)){
                sibling->color=CDS_BRTREE_BLACK;
                (*node)->parent->color=CDS_BRTREE_RED;
                cds_brtree_rotate_right(&(*node)->parent);
                sibling=(*node)->parent->l;
            }
            if(!cds_brtree_is_red(sibling->r)&&!cds_brtree_is_red(sibling->l)){
                sibling->color=CDS_BRTREE_RED;
                node=&(*node)->parent;
            }else{
                if(!cds_brtree_is_red(sibling->l)){
                    sibling->r->color=CDS_BRTREE_BLACK;
                    sibling->color=CDS_BRTREE_RED;
                    cds_brtree_rotate_left(&sibling);
                    sibling=(*node)->parent->l;
                }
                sibling->color=(*node)->parent->color;
                (*node)->parent->color=CDS_BRTREE_BLACK;
                sibling->l->color=CDS_BRTREE_BLACK;
                cds_brtree_rotate_right(&(*node)->parent);
                node=&(*node)->parent;
            }
        }
    }
    if(*node)(*node)->color=CDS_BRTREE_BLACK;
}

void cds_brtree_delete(struct CDS_BRTREE_NODE** node){
    struct CDS_BRTREE_NODE* nd=*node;
    struct CDS_BRTREE_NODE* child;
    int original_color=nd->color;

    if(!nd->l){
        child=nd->r;
        *node=child;
        if(child)child->parent=nd->parent;
    }else if(!nd->r){
        child=nd->l;
        *node=child;
        if(child)child->parent=nd->parent;
    }else{
        struct CDS_BRTREE_NODE** max=&nd->l;
        while((*max)->r)max=&(*max)->r;
        original_color=(*max)->color;
        child=(*max)->l;

        (*max)->color=nd->color;
        (*max)->r=nd->r;
        (*max)->r->parent=*max;
        *node=*max;
        (*max)->parent=nd->parent;
        *max=child;
    }

    free(nd->key);
    free(nd->data);
    free(nd);

    if(original_color==CDS_BRTREE_BLACK){
        cds_brtree_fix_delete(node);
    }
}

inline static void cds_brtree_close_recursion(struct CDS_BRTREE_NODE* node,CDS_CLOSE_FUNC key_f,CDS_CLOSE_FUNC value_f){
    if(!node)return;
    cds_brtree_close_recursion(node->l,key_f,value_f);
    cds_brtree_close_recursion(node->r,key_f,value_f);
    if(key_f){
        key_f(node->key);
    }
    free(node->key);
    if(value_f){
        value_f(node->data);
    }
    free(node->data);
    free(node);
}

void cds_brtree_close(CDS_BRTREE* t,CDS_CLOSE_FUNC key_f,CDS_CLOSE_FUNC value_f){
    cds_brtree_close_recursion(t->root,key_f,value_f);
    t->root=NULL;
}