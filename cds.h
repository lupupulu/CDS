#ifndef CDS_H
#define CDS_H

#include <stddef.h>

#define cds_at(name,tp,a,n) (*(tp*)cds_##name##_at(a,n,sizeof(tp)))
#define cds_push(name,a,v) (cds_##name##_push(a,&(v),sizeof(v)))
#define cds_insert(name,t,k,v) (cds_##name##_insert(t,&(k),sizeof((k)),&(v),sizeof((v))))

typedef int(*CDS_COMPARE_FUNC)(const void*,const void*);
typedef void(*CDS_PRINT_FUNC)(const void*);
typedef void(*CDS_CLOSE_FUNC)(const void*);

typedef int(*CDS_CHAR_MAP_FUNC)(char);

int cds_compare_int(const void *a,const void *b);
int cds_compare_float(const void *a,const void *b);
int cds_compare_string(const void *a,const void *b);

int cds_equal_int(const void *a,const void *b);
int cds_equal_float(const void *a,const void *b);
int cds_equal_string(const void *a,const void *b);

void cds_print_int(const void *a);
void cds_print_float(const void *a);
void cds_print_string(const void *a);

int cds_char_map_func(char c);

#define CDS_PRINT_INT(num) \
    inline static void cds_print_int_##num(const void *a)\
    {char print[num+1];memset(print,0,sizeof(print));register int n=*(const int*)a;for(register int i=num-1;i>=0;i--){print[i]=n%10+'0';n/=10;}printf(print);}

void cds_swap(void *a,void *b,size_t size);

typedef struct{
    unsigned char *data;
    size_t size;
    size_t real_size;
}CDS_VECTOR;
#define CDS_VECTOR(tp) CDS_VECTOR

void cds_vector_init(CDS_VECTOR *a,size_t foot);
size_t cds_vector_get_size(CDS_VECTOR *a);
void cds_vector_set_size(CDS_VECTOR *a,size_t size);
void*cds_vector_at(CDS_VECTOR *a,size_t n,size_t foot);
void cds_vector_push_back(CDS_VECTOR *a,const void *data,size_t size);
#define cds_vector_push(a,data,size) cds_vector_push_back(a,data,size)
void cds_vector_pop_back(CDS_VECTOR *a,CDS_CLOSE_FUNC func,size_t size);
void cds_vector_close(CDS_VECTOR *a,CDS_CLOSE_FUNC func,size_t size);
void cds_vector_print(CDS_VECTOR *a,CDS_PRINT_FUNC func,size_t foot);

typedef CDS_VECTOR CDS_STRING;
void cds_string_init(CDS_STRING *s,const char *dst);
size_t cds_string_length(CDS_STRING *s);
void cds_string_add(CDS_STRING *s,const char *dst);
#define cds_string_close(s) cds_vector_close(s,NULL,NULL)



struct CDS_LINK_NODE{
    struct CDS_LINK_NODE *next;
    unsigned char *data;
};
typedef struct{
    struct CDS_LINK_NODE head;
}CDS_LINK;//Linked list
#define CDS_LINK(tp) CDS_LINK

void cds_link_init(CDS_LINK *a);
void*cds_link_at(CDS_LINK *a,size_t n,size_t foot);
void cds_link_push(struct CDS_LINK_NODE *node,const void *data,size_t data_size);
void cds_link_delete(struct CDS_LINK_NODE *node,CDS_CLOSE_FUNC func);
struct CDS_LINK_NODE *cds_link_get(struct CDS_LINK_NODE *node,size_t n);
void cds_link_close(CDS_LINK *a,CDS_CLOSE_FUNC func);
void cds_link_print(CDS_LINK *a,CDS_PRINT_FUNC func);

struct CDS_DOUBLYLINK_NODE{
    struct CDS_DOUBLYLINK_NODE *next;
    unsigned char *data;
    struct CDS_DOUBLYLINK_NODE *last;
};
typedef struct{
    struct CDS_DOUBLYLINK_NODE head;
}CDS_DOUBLYLINK;//Doubly linked list
#define CDS_DOUBLYLINK(tp) CDS_DOUBLYLINK
void cds_doublylink_init(CDS_DOUBLYLINK *a);
void*cds_doublylink_at(CDS_DOUBLYLINK *a,size_t n,size_t foot);
void cds_doublylink_push(struct CDS_DOUBLYLINK_NODE *node,const void *data,size_t data_size);
void cds_doublylink_push_before(struct CDS_DOUBLYLINK_NODE *node,const void *data,size_t data_size);
void cds_doublylink_delete(struct CDS_DOUBLYLINK_NODE *node,CDS_CLOSE_FUNC func);
void cds_doublylink_delete_before(struct CDS_DOUBLYLINK_NODE *node,CDS_CLOSE_FUNC func);
struct CDS_DOUBLYLINK_NODE *cds_doublylink_get(struct CDS_DOUBLYLINK_NODE *node,long long n);
void cds_doublylink_close(CDS_DOUBLYLINK *a,CDS_CLOSE_FUNC func);
void cds_doublylink_print(CDS_DOUBLYLINK *a,CDS_PRINT_FUNC func);

typedef struct{
    CDS_LINK linked;
    struct CDS_LINK_NODE *last;
}CDS_QUEUE;
#define CDS_QUEUE(tp) CDS_QUEUE

void cds_queue_init(CDS_QUEUE *q);
void cds_queue_push(CDS_QUEUE *q,const void *data,size_t data_size);
void cds_queue_pop(CDS_QUEUE *q,CDS_CLOSE_FUNC func);
void *cds_queue_front(CDS_QUEUE *q);
#define cds_queue_at(q,n,t) cds_queue_front(q)
int cds_queue_empty(CDS_QUEUE *q);
void cds_queue_close(CDS_QUEUE *q,CDS_CLOSE_FUNC func);
void cds_queue_print(CDS_QUEUE *q,CDS_PRINT_FUNC func);



struct CDS_HEAP_NODE{
    void *key;
    void *value;
};
typedef struct{
    CDS_VECTOR data;
    CDS_COMPARE_FUNC cmp;
}CDS_HEAP;


void cds_heap_init(CDS_HEAP *heap,CDS_COMPARE_FUNC cmp);
void cds_heap_push(CDS_HEAP *heap,void *key,size_t key_size,void *value,size_t value_size);
void cds_heap_pop(CDS_HEAP *heap,CDS_CLOSE_FUNC key_f,CDS_CLOSE_FUNC value_f);
struct CDS_HEAP_NODE *cds_heap_front(CDS_HEAP *heap);
#define cds_heap_at(h,n,t) cds_heap_front(q)
int cds_heap_empty(CDS_HEAP *heap);
void cds_heap_close(CDS_HEAP *heap,CDS_CLOSE_FUNC key_f,CDS_CLOSE_FUNC value_f);


typedef struct {
    size_t cnt;
    CDS_COMPARE_FUNC cmp;
    unsigned char *data;
}CDS_HEAP_STACK;
void cds_heap_stack_init(CDS_HEAP_STACK *heap,CDS_COMPARE_FUNC cmp,void *data);
void cds_heap_stack_push(CDS_HEAP_STACK *heap,void *node,size_t size);
void cds_heap_stack_pop(CDS_HEAP_STACK *heap,CDS_CLOSE_FUNC node_f,size_t size);
void *cds_heap_stack_front(CDS_HEAP_STACK *heap);
int cds_heap_stack_empty(CDS_HEAP_STACK *heap);



typedef struct{
    void *data;
    unsigned char *end;
    size_t node_cnt;
    size_t ch_n,nodes_n;
}CDS_TRIE;

#define CDS_TRIE_STATE_OK       0b000
#define CDS_TRIE_STATE_REPEAT   0b001
#define CDS_TRIE_STATE_ERROR_CH 0b010
#define CDS_TRIE_STATE_NOT_END  0b100

void cds_trie_init(CDS_TRIE *trie,size_t ch_n,size_t nodes_n);
size_t cds_trie_get_next(CDS_TRIE *trie,size_t ch,size_t node);
size_t cds_trie_add(CDS_TRIE *trie,const char *str,CDS_CHAR_MAP_FUNC func,int *state);
size_t cds_trie_find(CDS_TRIE *trie,const char *str,CDS_CHAR_MAP_FUNC func,int *state);
int cds_trie_get_end(CDS_TRIE *trie,size_t node);
void cds_trie_close(CDS_TRIE *trie);


typedef struct{
    void *data;
    unsigned char *end;
    size_t node_cnt;
    size_t ch_n,nodes_n;
    int bytes;
}CDS_TRIE_STACK;
void cds_trie_stack_init(CDS_TRIE_STACK *trie,size_t ch_n,size_t nodes_n,int bytes,void *data,void *end);
size_t cds_trie_stack_get_next(CDS_TRIE_STACK *trie,size_t ch,size_t node);
size_t cds_trie_stack_add(CDS_TRIE_STACK *trie,const char *str,CDS_CHAR_MAP_FUNC func,int *state);
size_t cds_trie_stack_find(CDS_TRIE_STACK *trie,const char *str,CDS_CHAR_MAP_FUNC func,int *state);
int cds_trie_stack_get_end(CDS_TRIE_STACK *trie,size_t node);





#define CDS_TREE_NODE(name) \
    struct CDS_##name##_NODE *l,*r;\
    unsigned char *key; \
    unsigned char *data;
#define CDS_TREE(name) \
    struct CDS_##name##_NODE *root; \
    CDS_COMPARE_FUNC cmp,equal;


struct CDS_BSTREE_NODE{
    CDS_TREE_NODE(BSTREE)
};
typedef struct{
    CDS_TREE(BSTREE)
}CDS_BSTREE;

#define CDS_TO_BSTREE(a) ((CDS_BSTREE*)a)
#define CDS_TO_BSTREE_NODE(a) ((struct CDS_BSTREE_NODE*)a)

void cds_bstree_init(CDS_BSTREE *t,CDS_COMPARE_FUNC cmp,CDS_COMPARE_FUNC equal);
void cds_bstree_insert(CDS_BSTREE *t,void *key,size_t key_size,void *value,size_t value_size);
struct CDS_BSTREE_NODE **cds_bstree_find(CDS_BSTREE *t,void *key);
void*cds_bstree_at(CDS_BSTREE *t,void *key,size_t key_size);
void cds_bstree_delete(struct CDS_BSTREE_NODE **node,CDS_CLOSE_FUNC key_f,CDS_CLOSE_FUNC value_f);
void cds_bstree_close(CDS_BSTREE *t,CDS_CLOSE_FUNC key_f,CDS_CLOSE_FUNC value_f);
void cds_bstree_print(CDS_BSTREE *t,CDS_PRINT_FUNC key_print,size_t key_length,CDS_PRINT_FUNC value_print,size_t value_length,CDS_PRINT_FUNC extra_print,size_t extra_length);
size_t cds_bstree_get_node_num(struct CDS_BSTREE_NODE *node);



struct CDS_AVLTREE_NODE{
    CDS_TREE_NODE(AVLTREE)
    size_t height;
    int lazy_tag:1;
};
typedef struct{
    CDS_TREE(AVLTREE)
}CDS_AVLTREE;

void cds_avltree_init(CDS_AVLTREE *t,CDS_COMPARE_FUNC cmp,CDS_COMPARE_FUNC equal);
void cds_avltree_insert(CDS_AVLTREE *t,void *key,unsigned key_size,void *value,unsigned value_size);
#define cds_avltree_find(t,k) (cds_bstree_find((CDS_BSTREE*)t,k))
#define cds_avltree_at(t,k,sz) cds_bstree_at((CDS_BSTREE*)t,k,sz)
void cds_avltree_delete(struct CDS_AVLTREE_NODE **node,CDS_CLOSE_FUNC key_f,CDS_CLOSE_FUNC value_f);
void cds_avltree_close(CDS_AVLTREE *t,CDS_CLOSE_FUNC key_f,CDS_CLOSE_FUNC value_f);
#define cds_avltree_print(t,kf,kl,vf,vl,ef,el) cds_bstree_print((CDS_BSTREE*)t,kf,kl,vf,vl,ef,el)
#define CDS_PRINT_HEIGHT_INT(num)  \
    inline static void cds_avltree_print_int_##num(const void *a)\
    {const struct CDS_AVLTREE_NODE *nd=a;cds_print_int_##num(&nd->height);}




struct CDS_BRTREE_NODE{
    CDS_TREE_NODE(BRTREE)
    struct CDS_BRTREE_NODE *parent;
    int color:1;
};
typedef struct{
    CDS_TREE(BRTREE)
}CDS_BRTREE;

void cds_brtree_init(CDS_BRTREE *t,CDS_COMPARE_FUNC cmp,CDS_COMPARE_FUNC equal);
void cds_brtree_insert(CDS_BRTREE* t,void* key,unsigned key_size,void* value,unsigned value_size);
#define cds_brtree_find(t,k) (cds_bstree_find((CDS_BSTREE*)t,k))
#define cds_brtree_at(t,k,sz) cds_bstree_at((CDS_BSTREE*)t,k,sz)
void cds_brtree_delete(struct CDS_BRTREE_NODE** node);
void cds_brtree_close(CDS_BRTREE* t,CDS_CLOSE_FUNC key_f,CDS_CLOSE_FUNC value_f);
#define cds_brtree_print(t,kf,kl,vf,vl,ef,el) cds_bstree_print((CDS_BSTREE*)(t),kf,kl,vf,vl,ef,el)

#endif
