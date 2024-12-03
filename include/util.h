#ifndef _UTIL_H_
#define _UTIL_H_

#include "global_define.h"
#include "cJSON.h"

int initGlobal();
void trim(char *str);
int isEmptyString(char *str);
void toLowerCase(char *str);
int isZIP(const u_char *str);
int checkText(const u_char *str);
int unzip(char *src, int srcLen, char *dst, int dstLen);
int makeHttpInfo(HttpInfoT *pHttpInfo, cJSON *pRoot);
int makeGelfHttpInfo(HttpInfoT *pHttpInfo, cJSON *pRoot);
int getIPFromStr(const char *str, u_int32_t *address);
void getStrIP(char *str, u_int32_t address);
int getTimeFromStr(const char *str, time_t *tt);
cJSON* makeAlertJson(HttpInfoT *pHttpInfo);
void makeTimeString(time_t tt, char *buf);
char* getMemInfoFromFile(char *file, int size);
void getSelfProcessName(char *str);

#endif
