#ifndef _STIX_H_
#define _STIX_H_

int initStixEnviorment(char *filename, char *minefilename);
PatternEndPointT* findEndPointPattern(u_int32_t address, int port, int type);
PatternT* findPattern(PatternT *pPattern, char *strs, int cnt, char *str);
int processHttpInfo(HttpInfoT *pHttpInfo);
int findIDByName(char *str);
int initStixEnviormentByMemFile(const char *filename);
int initRegex();

#endif
