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

int cds_char_map_func(char c){
    if(c>='A'&&c<='Z'){
        return c-'A';
    }else if(c>='a'&&c<='z'){
        return c-'a'+26;
    }else if(c>='0'&&c<='9'){
        return c-'0'+52;
    }else if(c=='_'){
        return 62;
    }
    return -1;
}

void cds_swap(void *a,void *b,size_t size){
    void *swap=malloc(size);
    memcpy(swap,a,size);
    memcpy(a,b,size);
    memcpy(b,swap,size);
}

#define ABS(a) ((a)>0?(a):-(a))
#define MAX(a,b) ((a)>(b)?(a):(b))


void cds_vector_init(CDS_VECTOR *a,size_t foot){
    a->data=malloc(foot);
    a->real_size=foot;
    a->size=0;
}
size_t cds_vector_get_size(CDS_VECTOR *a){
    return a->size;
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
    return cds_link_get(&a->head,n)->data;
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
    for(register size_t i=0;i<n;i++){
        if(!now){
            break;
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
    printf("\n");
}

void cds_doublylink_init(CDS_DOUBLYLINK *a){
    a->head.data=NULL;
    a->head.last=NULL;
    a->head.next=NULL;
}
void*cds_doublylink_at(CDS_DOUBLYLINK *a,size_t n,size_t foot){
    return cds_doublylink_get(&a->head,n)->data;
}
void cds_doublylink_push(struct CDS_DOUBLYLINK_NODE *node,const void *data,size_t data_size){
    struct CDS_DOUBLYLINK_NODE *new=malloc(sizeof(struct CDS_DOUBLYLINK_NODE));
    new->data=malloc(data_size);
    memcpy(new->data,data,data_size);
    new->next=node->next;
    new->last=node;

    if(node->next){
        node->next->last=new;
    }
    node->next=new;
}
void cds_doublylink_push_before(struct CDS_DOUBLYLINK_NODE *node,const void *data,size_t data_size){
    struct CDS_DOUBLYLINK_NODE *new=malloc(sizeof(struct CDS_DOUBLYLINK_NODE));
    new->data=malloc(data_size);
    memcpy(new->data,data,data_size);
    new->next=node;
    new->last=node->last;

    if(node->last){
        node->last->next=new;
    }
    node->last=new;
}
void cds_doublylink_delete(struct CDS_DOUBLYLINK_NODE *node,CDS_CLOSE_FUNC func){
    if(!node->next){
        return ;
    }
    struct CDS_DOUBLYLINK_NODE *next=node->next;
    node->next=node->next->next;
    if(node->next){
        node->next->last=node;
    }
    if(func){
        func(next->data);
    }
    free(next->data);
    free(next);
}
void cds_doublylink_delete_before(struct CDS_DOUBLYLINK_NODE *node,CDS_CLOSE_FUNC func){
    if(!node->last){
        return ;
    }
    struct CDS_DOUBLYLINK_NODE *last=node->last;
    node->last=node->last->last;
    if(node->last){
        node->last->next=node;
    }
    if(func){
        func(last->data);
    }
    free(last->data);
    free(last);
}
struct CDS_DOUBLYLINK_NODE *cds_doublylink_get(struct CDS_DOUBLYLINK_NODE *node,long long n){
    register int s=0;
    if(n<0){
        s=1;
        n=-n;
    }
    struct CDS_DOUBLYLINK_NODE *now=node;
    for(register long long i=0;i<n;i++){
        if(!now){
            break;
        }
        if(s){
            now=now->next;
        }else{
            now=now->last;
        }
    }
    return now;
}
void cds_doublylink_close(CDS_DOUBLYLINK *a,CDS_CLOSE_FUNC func){
    while(a->head.last){
        cds_doublylink_delete_before(&a->head,func);
    }
    while(a->head.next){
        cds_doublylink_delete(&a->head,func);
    }
}
void cds_doublylink_print(CDS_DOUBLYLINK *a,CDS_PRINT_FUNC func){
    printf("last:");
    struct CDS_DOUBLYLINK_NODE *now=a->head.last;
    while(now){
        func(now->data);
        printf(" ");
        now=now->last;
    }
    now=a->head.next;
    printf("\nnext:");
    while(now){
        func(now->data);
        printf(" ");
        now=now->next;
    }
    printf("\n");
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


void cds_heap_init(CDS_HEAP *heap,CDS_COMPARE_FUNC cmp){
    cds_vector_init(&heap->data,sizeof(struct CDS_HEAP_NODE));
    heap->cmp=cmp;
}
inline static void cds_heap_node_swap(struct CDS_HEAP_NODE *a,struct CDS_HEAP_NODE *b){
    struct CDS_HEAP_NODE swap={.key=a->key,.value=a->value};
    a->key=b->key;
    a->value=b->value;
    b->key=swap.key;
    b->value=swap.value;
}
void cds_heap_push(CDS_HEAP *heap,void *key,size_t key_size,void *value,size_t value_size){
    struct CDS_HEAP_NODE node;
    node.key=malloc(key_size);
    memcpy(node.key,key,key_size);
    node.value=malloc(value_size);
    memcpy(node.value,value,value_size);
    cds_vector_push_back(&heap->data,&node,sizeof(node));
    size_t l=heap->data.size,i=0;
    struct CDS_HEAP_NODE *now,*last;
    while(l!=1){
        if(l%2){
            i=(l-1)/2;
        }else{
            i=l/2;
        }
        last=cds_vector_at(&heap->data,l-1,sizeof(struct CDS_HEAP_NODE));
        now=cds_vector_at(&heap->data,i-1,sizeof(struct CDS_HEAP_NODE));
        if(heap->cmp(now->key,last->key)){
            cds_heap_node_swap(now,last);
        }
        l=i;
    }
}
static CDS_CLOSE_FUNC cds_heap_key_close_f,cds_heap_value_close_f;
static void cds_heap_node_close_func(const void *p){
    struct CDS_HEAP_NODE *node=(void*)p;
    if(cds_heap_key_close_f){
        cds_heap_key_close_f(node->key);
    }
    if(cds_heap_value_close_f){
        cds_heap_value_close_f(node->value);
    }
}
void cds_heap_pop(CDS_HEAP *heap,CDS_CLOSE_FUNC key_f,CDS_CLOSE_FUNC value_f){
    struct CDS_HEAP_NODE *last,*nowl,*nowr,*nowj;
    last=cds_vector_at(&heap->data,0,sizeof(struct CDS_HEAP_NODE));
    nowl=cds_vector_at(&heap->data,heap->data.size-1,sizeof(struct CDS_HEAP_NODE));
    cds_heap_node_swap(last,nowl);
    cds_heap_key_close_f=key_f;
    cds_heap_value_close_f=value_f;
    cds_vector_pop_back(&heap->data,cds_heap_node_close_func,sizeof(struct CDS_HEAP_NODE));
    size_t lt=1,l,r,j;
    while(lt*2<=heap->data.size&&lt){
        last=cds_vector_at(&heap->data,lt-1,sizeof(struct CDS_HEAP_NODE));
        l=lt*2;
        nowl=cds_vector_at(&heap->data,l-1,sizeof(struct CDS_HEAP_NODE));
        if(lt*2+1<=heap->data.size){
            r=lt*2+1;
        }else{
            if(heap->cmp(last->key,nowl->key)){
                cds_heap_node_swap(nowl,last);
            }
            break;
        }
        nowr=cds_vector_at(&heap->data,r-1,sizeof(struct CDS_HEAP_NODE));
        if(heap->cmp(nowl->key,nowr->key)){
            j=r;
            nowj=nowr;
        }else{
            j=l;
            nowj=nowl;
        }
        if(heap->cmp(last->key,nowj->key)){
            cds_heap_node_swap(last,nowj);
        }
        lt=j;
    }
}
struct CDS_HEAP_NODE *cds_heap_front(CDS_HEAP *heap){
    return (struct CDS_HEAP_NODE *)heap->data.data;
}
int cds_heap_empty(CDS_HEAP *heap){
    if(!heap->data.size){
        return 1;
    }
    return 0;
}
void cds_heap_close(CDS_HEAP *heap,CDS_CLOSE_FUNC key_f,CDS_CLOSE_FUNC value_f){
    cds_heap_key_close_f=key_f;
    cds_heap_value_close_f=value_f;
    cds_vector_close(&heap->data,cds_heap_key_close_f,sizeof(struct CDS_HEAP_NODE));
}


void cds_heap_stack_init(CDS_HEAP_STACK *heap,CDS_COMPARE_FUNC cmp,void *data){
    heap->cnt=0;
    heap->cmp=cmp;
    heap->data=data;
}
void cds_heap_stack_push(CDS_HEAP_STACK *heap,void *node,size_t size){
    memcpy(heap->data+size*heap->cnt,node,size);
    heap->cnt++;
    size_t l=heap->cnt,i=0;
    void *now,*last;
    while(l!=1){
        if(l%2){
            i=(l-1)/2;
        }else{
            i=l/2;
        }
        last=heap->data+(l-1)*size;
        // last=cds_vector_at(&heap->data,l-1,sizeof(struct CDS_HEAP_NODE));
        now=heap->data+(i-1)*size;
        // now=cds_vector_at(&heap->data,i-1,sizeof(struct CDS_HEAP_NODE));
        if(heap->cmp(now,last)){
            unsigned char tmp[size];
            memcpy(tmp,now,size);
            memcpy(now,last,size);
            memcpy(last,tmp,size);
        }
        l=i;
    }
}
void cds_heap_stack_pop(CDS_HEAP_STACK *heap,CDS_CLOSE_FUNC node_f,size_t size){
    void *last=heap->data;
    void *nowl=heap->data+(heap->cnt-1)*size;
    void *nowr;
    void *nowj;

    char tmp[size];
    memcpy(tmp,last,size);
    memcpy(last,nowl,size);
    memcpy(nowl,tmp,size);

    heap->cnt--;
    if(node_f){
        node_f(heap->data+heap->cnt*size);
    }

    size_t lt=1,l,r,j;
    while(lt*2<=heap->cnt&&lt){
        last=heap->data+(lt-1)*size;
        l=lt*2;
        nowl=heap->data+(l-1)*size;
        if(lt*2+1<=heap->cnt){
            r=lt*2+1;
        }else{
            if(heap->cmp(last,nowl)){
                memcpy(tmp,last,size);
                memcpy(last,nowl,size);
                memcpy(nowl,tmp,size);
            }
            break;
        }
        nowr=heap->data+(r-1)*size;
        if(heap->cmp(nowl,nowr)){
            j=r;
            nowj=nowr;
        }else{
            j=l;
            nowj=nowl;
        }
        if(heap->cmp(last,nowj)){
            memcpy(tmp,last,size);
            memcpy(last,nowj,size);
            memcpy(nowj,tmp,size);
        }
        lt=j;
    }
}
void *cds_heap_stack_front(CDS_HEAP_STACK *heap){
    return heap->data;
}
int cds_heap_stack_empty(CDS_HEAP_STACK *heap){
    return !heap->cnt;
}



inline static size_t cds_trie_get_sizeof(size_t nodes_n){
    if(nodes_n<1<<8){
        return 1;
    }else if(nodes_n<1<<16){
        return 2;
    }else if(nodes_n<(size_t)1<<32){
        return 4;
    }else{
        return 8;
    }
    return 0;
}

inline static size_t cds_trie_get_node(CDS_TRIE *trie,size_t ch,size_t node,int stride){
    void *data=trie->data+(size_t)(trie->ch_n*node+ch)*stride;
    switch(stride){
    case 1:return *(unsigned char*)  data;
    case 2:return *(unsigned short*) data;
    case 4:return *(unsigned int*)   data;
    case 8:return *(size_t*)         data;
    }
    return 0;
}

inline static void cds_trie_set_node(CDS_TRIE *trie,size_t ch,size_t node,int stride,size_t value){
    void *data=trie->data+(trie->ch_n*node+ch)*stride;
    switch(stride){
    case 1:*(unsigned char*)  data = value; break;
    case 2:*(unsigned short*) data = value; break;
    case 4:*(unsigned int*)   data = value; break;
    case 8:*(size_t*)         data = value; break;
    }
    return ;
}

void cds_trie_init(CDS_TRIE *trie,size_t ch_n,size_t nodes_n){
    size_t size=cds_trie_get_sizeof(nodes_n)*nodes_n;
    trie->end=malloc(nodes_n);
    memset(trie->end,0,nodes_n);
    trie->data=malloc(size*ch_n);
    memset(trie->data,0,size*ch_n);
    trie->node_cnt=1;
    trie->ch_n=ch_n;
}

size_t cds_trie_get_next(CDS_TRIE *trie,size_t ch,size_t node){
    return cds_trie_get_node(trie,ch,node,cds_trie_get_sizeof(trie->node_cnt));
}

size_t cds_trie_add(CDS_TRIE *trie,const char *str,CDS_CHAR_MAP_FUNC func,int *state){
    size_t ch=0;
    int stride=cds_trie_get_sizeof(trie->nodes_n);
    size_t node=0;

    size_t len=strlen(str);
    for(size_t i=0;i<len;i++){
        ch=func(str[i]);
        if(ch>trie->ch_n){
            if(state){
                *state=CDS_TRIE_STATE_ERROR_CH;
            }
            return -1;
        }

        if(!cds_trie_get_node(trie,ch,node,stride)){
            cds_trie_set_node(trie,ch,node,stride,trie->node_cnt++);
        }
        node=cds_trie_get_node(trie,ch,node,stride);
    }
    if(trie->end[node]&&state){
        *state=CDS_TRIE_STATE_REPEAT;
    }
    trie->end[node]++;
    return node;
}

size_t cds_trie_find(CDS_TRIE *trie,const char *str,CDS_CHAR_MAP_FUNC func,int *state){
    size_t ch=0;
    int stride=cds_trie_get_sizeof(trie->nodes_n);
    size_t node=0;

    size_t len=strlen(str),i=0;
    for(i=0;i<len;i++){
        ch=func(str[i]);
        if(ch>trie->ch_n){
            if(state){
                *state=CDS_TRIE_STATE_ERROR_CH;
            }
            return -1;
        }

        if(!cds_trie_get_node(trie,ch,node,stride)){
            break;
        }
        node=cds_trie_get_node(trie,ch,node,stride);
    }
    if(trie->end[node]){
        return node;
    }
    if(i==len-1&&state){
        *state=CDS_TRIE_STATE_NOT_END;
        return node;
    }
    return -1;
}

int cds_trie_get_end(CDS_TRIE *trie,size_t node){
    if(node>trie->node_cnt){
        return -1;
    }
    return trie->end[node];
}

void cds_trie_close(CDS_TRIE *trie){
    free(trie->end);
    free(trie->data);
    memset(trie,0,sizeof(CDS_TRIE));
}



void cds_trie_stack_init(CDS_TRIE_STACK *trie,size_t ch_n,size_t nodes_n,int bytes,void *data,void *end){
    trie->bytes=bytes;
    trie->ch_n=ch_n;
    trie->nodes_n=nodes_n;
    trie->node_cnt=1;
    trie->data=data;
    trie->end=end;
}

inline static size_t cds_trie_stack_get_node(CDS_TRIE_STACK *trie,size_t ch,size_t node){
    void *data=trie->data+(size_t)(trie->ch_n*node+ch)*trie->bytes;
    switch(trie->bytes){
    case 1:return *(unsigned char*)  data;
    case 2:return *(unsigned short*) data;
    case 4:return *(unsigned int*)   data;
    case 8:return *(size_t*)         data;
    }
    return 0;
}

inline static void cds_trie_stack_set_node(CDS_TRIE_STACK *trie,size_t ch,size_t node,size_t value){
    void *data=trie->data+(trie->ch_n*node+ch)*trie->bytes;
    switch(trie->bytes){
    case 1:*(unsigned char*)  data = value; break;
    case 2:*(unsigned short*) data = value; break;
    case 4:*(unsigned int*)   data = value; break;
    case 8:*(size_t*)         data = value; break;
    }
    return ;
}

size_t cds_trie_stack_get_next(CDS_TRIE_STACK *trie,size_t ch,size_t node){
    return cds_trie_stack_get_node(trie,ch,node);
}

size_t cds_trie_stack_add(CDS_TRIE_STACK *trie,const char *str,CDS_CHAR_MAP_FUNC func,int *state){
    size_t ch=0;
    size_t node=0;

    size_t len=strlen(str);
    for(size_t i=0;i<len;i++){
        ch=func(str[i]);
        if(ch>trie->ch_n){
            if(state){
                *state=CDS_TRIE_STATE_ERROR_CH;
            }
            return -1;
        }

        if(!cds_trie_stack_get_node(trie,ch,node)){
            cds_trie_stack_set_node(trie,ch,node,trie->node_cnt++);
        }
        node=cds_trie_stack_get_node(trie,ch,node);
    }
    if(trie->end[node]&&state){
        *state=CDS_TRIE_STATE_REPEAT;
    }
    trie->end[node]++;
    return node;
}

size_t cds_trie_stack_find(CDS_TRIE_STACK *trie,const char *str,CDS_CHAR_MAP_FUNC func,int *state){
    size_t ch=0;
    size_t node=0;

    size_t len=strlen(str),i=0;
    for(i=0;i<len;i++){
        ch=func(str[i]);
        if(ch>trie->ch_n){
            if(state){
                *state=CDS_TRIE_STATE_ERROR_CH;
            }
            return -1;
        }

        if(!cds_trie_stack_get_node(trie,ch,node)){
            break;
        }
        node=cds_trie_stack_get_node(trie,ch,node);
    }
    if(trie->end[node]){
        return node;
    }
    if(i==len-1&&state){
        *state=CDS_TRIE_STATE_NOT_END;
        return node;
    }
    return -1;
}

int cds_trie_stack_get_end(CDS_TRIE_STACK *trie,size_t node){
    if(node>trie->node_cnt){
        return -1;
    }
    return trie->end[node];
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

inline static void cds_brtree_rotate_left(struct CDS_BRTREE_NODE **nd){
    struct CDS_BRTREE_NODE *r=(*nd)->r;
    
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

inline static void cds_brtree_rotate_right(struct CDS_BRTREE_NODE **nd){
    struct CDS_BRTREE_NODE *l=(*nd)->l;
    
    l->parent=(*nd)->parent;
    (*nd)->parent=l;
    
    (*nd)->l=l->r;
    if(l->r){
        l->r->parent=*nd;
    }
    l->r=*nd;
    
    l->color=(*nd)->color;
    (*nd)->color=CDS_BRTREE_RED;
    
    *nd=l;
}


inline static void cds_brtree_flip_color(struct CDS_BRTREE_NODE *nd){
    nd->color=!nd->color;
    nd->l->color=!nd->l->color;
    nd->r->color=!nd->r->color;
}

inline static int cds_brtree_is_red(struct CDS_BRTREE_NODE *nd){
    return nd&&nd->color==CDS_BRTREE_RED;
}

inline static void cds_brtree_fix_insert(struct CDS_BRTREE_NODE **nd){
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

inline static void cds_brtree_insert_recursion(struct CDS_BRTREE_NODE **nd,CDS_COMPARE_FUNC cmp,void *key,void *value,unsigned value_size,struct CDS_BRTREE_NODE *p){
    if(!*nd){
        cds_brtree_make_node(nd,key,value,value_size,p);
        return;
    }

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
void cds_brtree_insert(CDS_BRTREE *t,void *key,unsigned key_size,void *value,unsigned value_size){
    void *_key=malloc(key_size);
    memcpy(_key,key,key_size);
    cds_brtree_insert_recursion(&t->root,t->cmp,_key,value,value_size,NULL);
    t->root->color=CDS_BRTREE_BLACK;
}

inline static void cds_brtree_fix_delete(struct CDS_BRTREE_NODE **node){
    while(*node&&(*node)->color==CDS_BRTREE_BLACK){
        struct CDS_BRTREE_NODE *sibling;
        
        if(node==&(*node)->parent->l){
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
    if(*node){
        (*node)->color=CDS_BRTREE_BLACK;
    }
}

void cds_brtree_delete(struct CDS_BRTREE_NODE **node){
    struct CDS_BRTREE_NODE *nd=*node;
    struct CDS_BRTREE_NODE *child;
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
        struct CDS_BRTREE_NODE **max=&nd->l;
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

inline static void cds_brtree_close_recursion(struct CDS_BRTREE_NODE *node,CDS_CLOSE_FUNC key_f,CDS_CLOSE_FUNC value_f){
    if(!node){
        return;
    }
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

void cds_brtree_close(CDS_BRTREE *t,CDS_CLOSE_FUNC key_f,CDS_CLOSE_FUNC value_f){
    cds_brtree_close_recursion(t->root,key_f,value_f);
    t->root=NULL;
}
