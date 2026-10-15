#ifndef CONFC_H
#define CONFC_H

#if !(defined (__GNUG__)&&defined(size_t))
typedef __SIZE_TYPE__ size_t;
#ifdef __BEOS__
typedef long ssize_t;
#endif
#endif

#include <stdio.h>


/* 在以 ':' 分隔的 path 中查找可执行的 target，路径项支持 ~、~用户名、$VAR_NAME、${VAR}；
   找到时返回 malloc 出来的完整路径（由调用者 free），否则返回 NULL */
char *confc_find_path(const char *path,const char *target);



typedef struct {
    char *key;
    char *value;
}ini_pair_t;

typedef struct {
    char *name;
    size_t pair_size;
    ini_pair_t *pairs;
}ini_section_t;

typedef struct {
    size_t section_size;
    ini_section_t *sections;
}ini_t;

ini_t ini_load(const char *filename);
ini_t ini_load_from_memory(const char *file);
void ini_unload(ini_t *file);

void ini_write(FILE *fp,ini_t *file);
void ini_write_fd(int fd,ini_t *file);
char *ini_write_to_memory(ini_t *file);

ini_section_t *ini_add_section(ini_t *file,ini_section_t *section);
void ini_add_pair(ini_section_t *section,ini_pair_t *pair);

int ini_del_section(ini_t *file,const char *section);
int ini_del_pair(ini_t *file,const char *section,const char *key);
int ini_del_pair_from_section(ini_section_t *section,const char *key);

const char *ini_lookup(ini_t *file,const char *section,const char *key);
ini_section_t *ini_lookup_section(ini_t *file,const char *name);
const char *ini_lookup_from_section(ini_section_t *section,const char *key);


typedef enum {
    TOML_STRING,
    TOML_INTEGER,
    TOML_FLOAT,
    TOML_BOOLEAN,
    TOML_DATETIME,
    TOML_TABLE,
    TOML_ARRAY
}toml_type_t;

typedef struct {
    char *key;
    toml_type_t *value;
}toml_pair_t;

typedef struct {
    toml_type_t tp;
    char *str;
}toml_string_t;

typedef struct {
    toml_type_t tp;
    long v;
}toml_integer_t;

typedef struct {
    toml_type_t tp;
    double v;
}toml_float_t;

typedef struct {
    toml_type_t tp;
    int v;
}toml_boolean_t;

typedef struct {
    toml_type_t tp;
    unsigned long time;
    long offset;
}toml_datetime_t;

typedef struct {
    toml_type_t tp;
    toml_pair_t *pairs;
    size_t size;
}toml_table_t;

typedef struct {
    toml_type_t tp;
    toml_type_t *items;
    size_t size;
}toml_array_t;



#ifdef CONFC_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/stat.h>

#ifdef CONFC_DEBUG_OUTPUT
#define ini_log_err(...) fprintf(stderr,__VA_ARGS__)
#else
#define ini_log_err(...) ((void)0)
#endif

#ifndef DARRAY_H

#define darray_t(Tp) \
struct{\
    Tp *arr;\
    size_t size,real;\
}

inline static int da_init(void *array);
inline static int da_add(size_t tp_size,void *array,const void *buf);
inline static int da_clear(void *array);

#endif

#ifndef DA_STR_T
#define DA_STR_T
typedef darray_t(char) da_str;
#endif



char *confc_find_path(const char *path,const char *target){
    if(target==NULL||target[0]=='\0'){
        return NULL;
    }

    /* target 带 '/' 时本身就是一个路径，不搜索 path */
    if(target[0]=='/'){
        struct stat status;
        if(stat(target,&status)!=0||!S_ISREG(status.st_mode)||access(target,X_OK)!=0){
            return NULL;
        }
        size_t size=strlen(target)+1;
        char *copy=malloc(size);
        if(!copy){
            ini_log_err("confc_find_path: malloc %zu bytes failed: %s\n",size,strerror(errno));
            return NULL;
        }
        memcpy(copy,target,size);
        return copy;
    }
    if(path==NULL){
        return NULL;
    }

    da_str dir;
    da_init(&dir);
    da_str candidate;
    da_init(&candidate);
    da_str name;
    da_init(&name);

    char *ret=NULL;
    const char *entry=path;
    int failed=0;

    while(ret==NULL&&!failed){
        const char *end=entry;
        while(*end!='\0'&&*end!=':'){
            end++;
        }

        /* 把当前路径项展开到 dir */
        const char *pos=entry;
        dir.size=0;

        if(pos<end&&*pos=='~'){
            const char *user=pos+1;
            const char *user_end=user;
            while(user_end<end&&*user_end!='/'){ user_end++; }

            const char *home=NULL;
            if(user_end==user){
                home=getenv("HOME");
                if(home==NULL||home[0]=='\0'){
                    struct passwd *pw=getpwuid(getuid());
                    home=pw?pw->pw_dir:NULL;
                }
            }else{
                name.size=0;
                for(const char *p=user;p<user_end;p++){
                    failed|=da_add(sizeof(char),&name,p);
                }
                failed|=da_add(sizeof(char),&name,"");
                if(!failed){
                    struct passwd *pw=getpwnam(name.arr);
                    home=pw?pw->pw_dir:NULL;
                }
            }

            if(home!=NULL&&home[0]!='\0'){
                for(const char *p=home;*p!='\0';p++){
                    failed|=da_add(sizeof(char),&dir,p);
                }
            }else{
                /* 取不到家目录（如用户名不存在）时原样保留 */
                failed|=da_add(sizeof(char),&dir,"~");
                for(const char *p=user;p<user_end;p++){
                    failed|=da_add(sizeof(char),&dir,p);
                }
            }
            pos=user_end;
        }

        while(pos<end){
            const char *var=NULL;
            size_t var_size=0;
            const char *next=pos;

            if(*pos=='$'&&pos+1<end){
                if(pos[1]=='{'){
                    const char *close=pos+2;
                    while(close<end&&*close!='}'){
                        close++;
                    }
                    if(close>pos+2&&close<end){
                        int ok=((pos[2]>='a'&&pos[2]<='z')||(pos[2]>='A'&&pos[2]<='Z')||pos[2]=='_');
                        for(const char *p=pos+3;ok&&p<close;p++){
                            char c=*p;
                            if(!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='_')){
                                ok=0;
                            }
                        }
                        if(ok){
                            var=pos+2;
                            var_size=close-pos-2;
                            next=close+1;
                        }
                    }
                }else{
                    char c=pos[1];
                    if((c>='a'&&c<='z')||(c>='A'&&c<='Z')||c=='_'){
                        const char *p=pos+2;
                        while(p<end){
                            c=*p;
                            if(!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='_')){
                                break;
                            }
                            p++;
                        }
                        var=pos+1;
                        var_size=p-pos-1;
                        next=p;
                    }
                }
            }

            if(var==NULL){
                /* 其他变量写法、单独的 '$' 一律原样保留 */
                failed|=da_add(sizeof(char),&dir,pos);
                pos++;
                continue;
            }

            name.size=0;
            for(const char *p=var;p<var+var_size;p++){
                failed|=da_add(sizeof(char),&name,p);
            }
            failed|=da_add(sizeof(char),&name,"");
            const char *value=failed?NULL:getenv(name.arr);
            if(value!=NULL){
                for(const char *p=value;*p!='\0';p++){
                    failed|=da_add(sizeof(char),&dir,p);
                }
            }
            pos=next;
        }

        if(dir.size==0){
            if(end!=entry){
                /* 展开后为空的路径项（如引用了未定义的变量）跳过 */
                if(*end=='\0'){
                    break;
                }
                entry=end+1;
                continue;
            }
            /* 空路径项表示当前目录 */
            failed|=da_add(sizeof(char),&dir,".");
        }

        if(!failed){
            candidate.size=0;
            for(size_t i=0;i<dir.size;i++){
                failed|=da_add(sizeof(char),&candidate,&dir.arr[i]);
            }
            if(dir.arr[dir.size-1]!='/'){
                failed|=da_add(sizeof(char),&candidate,"/");
            }
            for(const char *p=target;*p!='\0';p++){
                failed|=da_add(sizeof(char),&candidate,p);
            }
            failed|=da_add(sizeof(char),&candidate,"");

            if(!failed){
                struct stat status;
                if(stat(candidate.arr,&status)==0&&S_ISREG(status.st_mode)&&access(candidate.arr,X_OK)==0){
                    ret=candidate.arr;
                    candidate.arr=NULL;
                    candidate.size=0;
                    candidate.real=0;
                    break;
                }
            }
        }

        if(*end=='\0'){
            break;
        }
        entry=end+1;
    }

    da_clear(&dir);
    da_clear(&candidate);
    da_clear(&name);
    return ret;
}



ini_t ini_load(const char *filename){
    ini_t ret={};
    int fd=open(filename,O_RDONLY);
    if(fd==-1){
        return ret;
    }

    struct stat status;
    if(fstat(fd,&status)!=0){
        close(fd);
        return ret;
    }
    char *mem=malloc(status.st_size+1);
    if(!mem){
        ini_log_err("ini_load: malloc %zu bytes failed: %s\n",(size_t)(status.st_size+1),strerror(errno));
        close(fd);
        return ret;
    }
    ssize_t got=0;
    while(got<(ssize_t)status.st_size){
        ssize_t n=read(fd,mem+got,status.st_size-got);
        if(n<=0){
            if(n==-1&&errno==EINTR){
                continue;
            }
            break;
        }
        got+=n;
    }
    close(fd);

    mem[got]='\0';
    memset(mem+got+1,0,status.st_size-got);
    ret=ini_load_from_memory(mem);

    free(mem);
    return ret;
}

inline static void ini_init_section(ini_t *file,ini_section_t **now_section){
    ini_section_t section={.name=""};
    *now_section=ini_add_section(file,&section);
}
static void ini_parse_per_line(ini_t *ret,ini_section_t **now_section,const char *file,size_t *i){
#define i (*i)

    da_str key,value;
    da_init(&key);
    da_init(&value);

    while(file[i]==' '){
        i++;
    }
    if(file[i]=='\n'||file[i]=='\r'||file[i]=='\0'){
        return ;
    }

    if(file[i]=='['){
        i++;
        while(file[i]==' '){
            i++;
        }
        while(file[i]!=']'&&file[i]!='\r'&&file[i]!='\0'){
            da_add(sizeof(char),&key,&file[i]);
            i++;
        }
        if(file[i]=='\r'||file[i]=='\0'){
            da_clear(&key);
            return ;
        }
        while(key.size&&key.arr[key.size-1]==' '){
            key.size--;
        }

        da_add(sizeof(char),&key,"");
        ini_section_t section={.name=key.arr};
        *now_section=ini_add_section(ret,&section);
        da_clear(&key);
        while(file[i]!='\n'&&file[i]!='\0'){
            i++;
        }
        return ;
    }else if(file[i]=='#'){
        while(file[i]!='\n'&&file[i]!='\0'){
            i++;
        }
        return ;
    }

    while(file[i]!='='&&file[i]!='#'&&file[i]!='\r'&&file[i]!='\n'&&file[i]!='\0'){
        da_add(sizeof(char),&key,&file[i]);
        i++;
    }
    if(key.size==0){
        da_clear(&key);
        return ;
    }
    while(key.arr[key.size-1]==' '){
        key.size--;
    }
    da_add(sizeof(char),&key,"");

    if(file[i]!='='){
        goto L_ADD_PAIR;
    }

    i++;
    while(file[i]==' '){
        i++;
    }
    while(file[i]!='#'&&file[i]!='\r'&&file[i]!='\n'&&file[i]!='\0'){
        da_add(sizeof(char),&value,&file[i]);
        i++;
    }
    while(value.size&&value.arr[value.size-1]==' '){
        value.size--;
    }
    L_ADD_PAIR:
    da_add(sizeof(char),&value,"");

    ini_pair_t pair={.key=key.arr,.value=value.arr};
    if(*now_section==NULL){
        ini_init_section(ret,now_section);
    }
    if(*now_section!=NULL){
        ini_add_pair(*now_section,&pair);
    }
    da_clear(&key);
    da_clear(&value);

    if(file[i]=='#'){
        while(file[i]!='\n'&&file[i]!='\0'){
            i++;
        }
    }

#undef i
}
ini_t ini_load_from_memory(const char *file){
    ini_t ret={};
    ini_section_t *now_section=NULL;

    size_t i=0;
    while(file[i]!='\0'){
        ini_parse_per_line(&ret,&now_section,file,&i);
        if(file[i]!='\0'){
            i++;
        }
    }

    return ret;
}

void ini_unload(ini_t *file){
    for(size_t i=0;i<file->section_size;i++){
        free(file->sections[i].name);
        for(size_t j=0;j<file->sections[i].pair_size;j++){
            free(file->sections[i].pairs[j].key);
            free(file->sections[i].pairs[j].value);
        }
        free(file->sections[i].pairs);
    }
    free(file->sections);
    file->sections=NULL;
    file->section_size=0;
}

char *ini_write_to_memory(ini_t *file){
    if(file==NULL){
        ini_log_err("ini_write_to_memory: file is nullptr\n");
        return NULL;
    }

    da_str mem;
    da_init(&mem);

    int failed=0;
    
    for(size_t i=0;i<file->section_size;i++){
        ini_section_t *section=&file->sections[i];
        if(section->name!=NULL&&section->name[0]!='\0'){
            failed|=da_add(sizeof(char),&mem,"[");
            for(const char *p=section->name;*p!='\0';p++){
                failed|=da_add(sizeof(char),&mem,p);
            }
            failed|=da_add(sizeof(char),&mem,"]");
            failed|=da_add(sizeof(char),&mem,"\n");
        }
        for(size_t j=0;j<section->pair_size;j++){
            ini_pair_t *pair=&section->pairs[j];
            if(pair->key!=NULL){
                for(const char *p=pair->key;*p!='\0';p++){
                    failed|=da_add(sizeof(char),&mem,p);
                }
            }
            failed|=da_add(sizeof(char),&mem,"=");
            if(pair->value!=NULL){
                for(const char *p=pair->value;*p!='\0';p++){
                    failed|=da_add(sizeof(char),&mem,p);
                }
            }
            failed|=da_add(sizeof(char),&mem,"\n");
        }
    }
    
    failed|=da_add(sizeof(char),&mem,"");

    if(failed){
        ini_log_err("ini_write_to_memory: out of memory\n");
        da_clear(&mem);
        return NULL;
    }
    return mem.arr;
}

void ini_write(FILE *fp,ini_t *file){
    if(fp==NULL||file==NULL){
        ini_log_err("ini_write: nullptr argument\n");
        return;
    }

    for(size_t i=0;i<file->section_size;i++){
        ini_section_t *section=&file->sections[i];
        if(section->name!=NULL&&section->name[0]!='\0'){
            fputc('[',fp);
            fputs(section->name,fp);
            fputc(']',fp);
            fputc('\n',fp);
        }
        for(size_t j=0;j<section->pair_size;j++){
            ini_pair_t *pair=&section->pairs[j];
            if(pair->key!=NULL){
                fputs(pair->key,fp);
            }
            fputc('=',fp);
            if(pair->value!=NULL){
                fputs(pair->value,fp);
            }
            fputc('\n',fp);
        }
    }
}

void ini_write_fd(int fd,ini_t *file){
    if(file==NULL){
        ini_log_err("ini_write_fd: file is nullptr\n");
        return;
    }

    for(size_t i=0;i<file->section_size;i++){
        ini_section_t *section=&file->sections[i];

        if(section->name!=NULL&&section->name[0]!='\0'){
            const char *piece[3]={"[",section->name,"]\n"};
            size_t size[3]={1,strlen(section->name),2};

            for(size_t k=0,off=0;k<3;){
                if(off==size[k]){
                    k++;
                    off=0;
                    continue;
                }
                ssize_t got=write(fd,piece[k]+off,size[k]-off);
                if(got<=0){
                    if(got==-1&&errno==EINTR){
                        continue;
                    }
                    return;
                }
                off+=got;
            }
        }

        for(size_t j=0;j<section->pair_size;j++){
            ini_pair_t *pair=&section->pairs[j];
            const char *piece[4]={pair->key?pair->key:"","=",pair->value?pair->value:"","\n"};
            size_t size[4]={strlen(piece[0]),1,strlen(piece[2]),1};

            for(size_t k=0,off=0;k<4;){
                if(off==size[k]){
                    k++;
                    off=0;
                    continue;
                }
                ssize_t got=write(fd,piece[k]+off,size[k]-off);
                if(got<=0){
                    if(got==-1&&errno==EINTR){
                        continue;
                    }
                    return;
                }
                off+=got;
            }
        }
    }
}

ini_section_t *ini_add_section(ini_t *file,ini_section_t *section){

    const char *src_name=section->name?section->name:"";
    char *name=malloc(strlen(src_name)+1);
    if(!name){
        ini_log_err("ini_add_section: malloc %zu bytes failed: %s\n",(size_t)(strlen(src_name)+1),strerror(errno));
        return NULL;
    }
    strcpy(name,src_name);
    ini_section_t sec=*section;
    sec.name=name;

    if(file->section_size==0){
        file->sections=malloc(sizeof(ini_section_t));
        if(!file->sections){
            ini_log_err("ini_add_section: malloc %zu bytes failed: %s\n",sizeof(ini_section_t),strerror(errno));
            free(name);
            return NULL;
        }
        memcpy(&file->sections[0],&sec,sizeof(ini_section_t));
        file->section_size++;
        return &file->sections[0];
    }

    size_t l=0,r=file->section_size-1;
    size_t mid=l+(r-l)/2;
    while(l<=r&&r!=(size_t)-1){
        mid=l+(r-l)/2;
        int res=strcmp(file->sections[mid].name,sec.name);
        if(res==0){
            for(size_t i=0;i<sec.pair_size;i++){
                ini_add_pair(&file->sections[mid],&sec.pairs[i]);
            }
            free(name);
            return &file->sections[mid];
        }else if(res<0){
            l=mid+1;
        }else{
            r=mid-1;
        }
    }
    ini_section_t *sections=realloc(file->sections,(file->section_size+1)*sizeof(ini_section_t));
    if(!sections){
        ini_log_err("ini_add_section: realloc %zu bytes failed: %s\n",(size_t)((file->section_size+1)*sizeof(ini_section_t)),strerror(errno));
        free(name);
        return NULL;
    }
    file->sections=sections;
    memmove(&file->sections[l+1],&file->sections[l],(file->section_size-l)*sizeof(ini_section_t));
    memcpy(&file->sections[l],&sec,sizeof(ini_section_t));
    file->section_size++;

    return &file->sections[l];
}

void ini_add_pair(ini_section_t *section,ini_pair_t *pair){

    const char *src_key=pair->key?pair->key:"";
    const char *src_value=pair->value?pair->value:"";
    ini_pair_t p=*pair;
    p.key=malloc(strlen(src_key)+1);
    if(!p.key){
        ini_log_err("ini_add_pair: malloc %zu bytes failed: %s\n",(size_t)(strlen(src_key)+1),strerror(errno));
        return ;
    }
    strcpy(p.key,src_key);
    p.value=malloc(strlen(src_value)+1);
    if(!p.value){
        ini_log_err("ini_add_pair: malloc %zu bytes failed: %s\n",(size_t)(strlen(src_value)+1),strerror(errno));
        free(p.key);
        return ;
    }
    strcpy(p.value,src_value);

    if(section->pair_size==0){
        section->pairs=malloc(sizeof(ini_pair_t));
        if(!section->pairs){
            ini_log_err("ini_add_pair: malloc %zu bytes failed: %s\n",sizeof(ini_pair_t),strerror(errno));
            free(p.key);
            free(p.value);
            return ;
        }
        memcpy(&section->pairs[0],&p,sizeof(ini_pair_t));
        section->pair_size++;
        return ;
    }

    size_t l=0,r=section->pair_size-1;
    size_t mid=l+(r-l)/2;
    while(l<=r&&r!=(size_t)-1){
        mid=l+(r-l)/2;
        int res=strcmp(section->pairs[mid].key,p.key);
        if(res==0){
            free(section->pairs[mid].key);
            free(section->pairs[mid].value);
            memcpy(&section->pairs[mid],&p,sizeof(ini_pair_t));
            return ;
        }else if(res<0){
            l=mid+1;
        }else{
            r=mid-1;
        }
    }
    ini_pair_t *pairs=realloc(section->pairs,(section->pair_size+1)*sizeof(ini_pair_t));
    if(!pairs){
        ini_log_err("ini_add_pair: realloc %zu bytes failed: %s\n",(size_t)((section->pair_size+1)*sizeof(ini_pair_t)),strerror(errno));
        free(p.key);
        free(p.value);
        return ;
    }
    section->pairs=pairs;
    memmove(&section->pairs[l+1],&section->pairs[l],(section->pair_size-l)*sizeof(ini_pair_t));
    memcpy(&section->pairs[l],&p,sizeof(ini_pair_t));
    section->pair_size++;
}

int ini_del_section(ini_t *file,const char *section){
    if(file->section_size==0){
        return 1;
    }

    size_t l=0,r=file->section_size-1;
    size_t mid=l+(r-l)/2;
    while(l<=r&&r!=(size_t)-1){
        mid=l+(r-l)/2;
        int res=strcmp(file->sections[mid].name,section);
        if(res==0){
            break;
        }else if(res<0){
            l=mid+1;
        }else{
            r=mid-1;
        }
    }
    if(l>r||r==(size_t)-1){
        return 1;
    }

    ini_section_t *now=&file->sections[mid];
    for(size_t i=0;i<now->pair_size;i++){
        free(now->pairs[i].key);
        free(now->pairs[i].value);
    }
    free(now->pairs);
    free(now->name);

    memmove(&file->sections[mid],&file->sections[mid+1],(file->section_size-mid-1)*sizeof(ini_section_t));
    file->section_size--;
    ini_section_t *sections=realloc(file->sections,file->section_size*sizeof(ini_section_t));
    if(!sections&&file->section_size){
        ini_log_err("ini_del_section: realloc %zu bytes failed: %s\n",(size_t)(file->section_size*sizeof(ini_section_t)),strerror(errno));
    }else{
        file->sections=sections;
    }

    return 0;
}

int ini_del_pair(ini_t *file,const char *section,const char *key){
    ini_section_t *sec=ini_lookup_section(file,section);
    if(sec==NULL){
        return 1;
    }
    return ini_del_pair_from_section(sec,key);
}

int ini_del_pair_from_section(ini_section_t *section,const char *key){
    if(section->pair_size==0){
        return 2;
    }

    size_t l=0,r=section->pair_size-1;
    size_t mid=l+(r-l)/2;
    while(l<=r&&r!=(size_t)-1){
        mid=l+(r-l)/2;
        int res=strcmp(section->pairs[mid].key,key);
        if(res==0){
            break;
        }else if(res<0){
            l=mid+1;
        }else{
            r=mid-1;
        }
    }
    if(l>r||r==(size_t)-1){
        return 2;
    }

    free(section->pairs[mid].key);
    free(section->pairs[mid].value);

    memmove(&section->pairs[mid],&section->pairs[mid+1],(section->pair_size-mid-1)*sizeof(ini_pair_t));
    section->pair_size--;
    ini_pair_t *pairs=realloc(section->pairs,section->pair_size*sizeof(ini_pair_t));
    if(!pairs&&section->pair_size){
        ini_log_err("ini_del_pair_from_section: realloc %zu bytes failed: %s\n",(size_t)(section->pair_size*sizeof(ini_pair_t)),strerror(errno));
    }else{
        section->pairs=pairs;
    }

    return 0;
}

const char *ini_lookup(ini_t *file,const char *section,const char *key){
    ini_section_t *sec=ini_lookup_section(file,section);
    if(sec==NULL){
        return NULL;
    }
    return ini_lookup_from_section(sec,key);
}

ini_section_t *ini_lookup_section(ini_t *file,const char *name){
    if(file->section_size==0){
        return NULL;
    }

    size_t l=0,r=file->section_size-1;
    while(l<=r&&r!=(size_t)-1){
        size_t mid=l+(r-l)/2;
        int res=strcmp(file->sections[mid].name,name);
        if(res==0){
            return &file->sections[mid];
        }else if(res<0){
            l=mid+1;
        }else{
            r=mid-1;
        }
    }
    return NULL;
}

const char *ini_lookup_from_section(ini_section_t *section,const char *key){
    if(section->pair_size==0){
        return NULL;
    }

    size_t l=0,r=section->pair_size-1;
    size_t mid=l+(r-l)/2;
    while(l<=r&&r!=(size_t)-1){
        mid=l+(r-l)/2;
        int res=strcmp(section->pairs[mid].key,key);
        if(res==0){
            return section->pairs[mid].value;
        }else if(res<0){
            l=mid+1;
        }else{
            r=mid-1;
        }
    }
    return NULL;
}

#ifndef DARRAY_H

int da_init(void *array){
    memset(array,0,sizeof(darray_t(void)));
    return 0;
}

int da_add(size_t tp_size,void *array,const void *buf){
    darray_t(void) *arr=array;
    if(arr->size+1>=arr->real){
        void *p=realloc(arr->arr,(arr->real*3/2+1)*tp_size);
        if(!p){
            ini_log_err("da_add: realloc %zu bytes failed: %s\n",(size_t)((arr->real*3/2+1)*tp_size),strerror(errno));
            return 127;
        }
        arr->arr=p;
        arr->real=arr->real*3/2+1;
    }
    memcpy(arr->arr+arr->size*tp_size,buf,tp_size);
    arr->size++;
    return 0;
}

int da_clear(void *array){
    darray_t(void) *arr=array;
    free(arr->arr);
    memset(arr,0,sizeof(darray_t(void)));
    return 0;
}

#endif

#undef ini_log_err

#endif

#endif