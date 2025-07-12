#ifndef _CCMD_H
#define _CCMD_H

void ccmd_init(int argc,const char **argv);
#define CCMD_REQUIRED  0b000000
#define CCMD_OPTIONAL  0b000001
#define CCMD_STR_PARAM 0b000010
#define CCMD_INT_PARAM 0b000100
#define CCMD_VARIABLE  0b001000
#define CCMD_FUNCTION  0b010000
#define CCMD_NEXT      0b100000
typedef void(*CCMD_FUNC)(void);
void ccmd_set_para(const char *sw,int flg,void *data);
/// @param exit if param unknown,0:continue the function;1:break the function.
/// @return the count of unknown params.
int ccmd_deal(int exit);
int ccmd_param_size(void *data);
void *ccmd_str_param_at(void *data,int n);
int ccmd_int_param_at(void *data,int n);
void ccmd_close(void);

#endif