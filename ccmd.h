#ifndef _CCMD_H
#define _CCMD_H

void ccmd_init(int argc,const char **argv);
#define CCMD_REQUIRED  0b00000
#define CCMD_OPTIONAL  0b00001
#define CCMD_STR_PARAM 0b00010
#define CCMD_INT_PARAM 0b00100
#define CCMD_VARIABLE  0b01000
#define CCMD_FUNCTION  0b10000
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