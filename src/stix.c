#include "util.h"
#include "stix.h"
#include "cJSON.h"

extern GlobalT global;

int initMemory();
cJSON* readFile(char *filename);
int analyzeJson(cJSON *pRoot, int type);
int getStixInfo(cJSON *pArr, char *pattern, int type);
int getStixUrlInfo(cJSON *pItem, char *pattern, int type);
int getStixDomainInfo(cJSON *pItem, char *pattern, int type);
int getEndPointInfo(cJSON *pItem, char *pattern);
int getMineEndPointInfo(cJSON *pItem, char *pattern);
void sortEndPointPattern(PatternEndPointT *pPatternEndPoint, int cnt);
void sortPattern(PatternT *pPattern, char *str, int cnt);
void sortName();

int initStixEnviorment(char *filename, char *minefilename)
{
  int ret;
  cJSON *pRoot;

  global.domainStringInfo.size = 65536;
  global.nameStringInfo.size = 65536;
  global.urlStringInfo.size = 65536;
  global.patternDomainInfo.size = 1024;
  global.patternUrlInfo.size = 1024;
  global.endPointInfo.size = 1024;
  global.mineEndPointInfo.size = 1024;
  global.idNameInfo.size = 1024;
  global.regexInfo.size = 1024;
  ret = initMemory();
  if(ret)
    return -1;

  pRoot = readFile(filename);
  if(pRoot == 0)
    return -1;
  ret = analyzeJson(pRoot, TYPE_STIX);
  if(ret)
    return -1;
  cJSON_Delete(pRoot);

  pRoot = readFile(minefilename);
  if(pRoot == 0)
    return -1;
  ret = analyzeJson(pRoot, TYPE_STIX_MINE);
  if(ret)
    return -1;
  cJSON_Delete(pRoot);

  sortEndPointPattern(global.endPointInfo.pPatternEndPoint, global.endPointInfo.cnt);
  sortEndPointPattern(global.mineEndPointInfo.pPatternEndPoint, global.mineEndPointInfo.cnt);
  sortPattern(global.patternUrlInfo.pPattern, global.urlStringInfo.pStr, global.patternUrlInfo.cnt);
  sortPattern(global.patternDomainInfo.pPattern, global.domainStringInfo.pStr, global.patternDomainInfo.cnt);
  sortName();
  return 0;
}

int initMemory()
{
  global.urlStringInfo.pStr = (char*)malloc(global.urlStringInfo.size);
  if(global.urlStringInfo.pStr == 0){
    printf("Memory error!\n");
    return -1;
  }
  memset(global.urlStringInfo.pStr, 0x00, global.urlStringInfo.size);

  global.domainStringInfo.pStr = (char*)malloc(global.domainStringInfo.size);
  if(global.domainStringInfo.pStr == 0){
    printf("Memory error!\n");
    return -1;
  }
  memset(global.domainStringInfo.pStr, 0x00, global.domainStringInfo.size);

  global.nameStringInfo.pStr = (char*)malloc(global.nameStringInfo.size);
  if(global.nameStringInfo.pStr == 0){
    printf("Memory error!\n");
    return -1;
  }
  memset(global.nameStringInfo.pStr, 0x00, global.nameStringInfo.size);

  global.patternUrlInfo.pPattern = (PatternT*)malloc(sizeof(PatternT) * global.patternUrlInfo.size);
  if(global.patternUrlInfo.pPattern == 0){
    printf("Memory error!\n");
    return -1;
  }
  memset(global.patternUrlInfo.pPattern, 0x00, sizeof(PatternT) * global.patternUrlInfo.size);

  global.patternDomainInfo.pPattern = (PatternT*)malloc(sizeof(PatternT) * global.patternDomainInfo.size);
  if(global.patternDomainInfo.pPattern == 0){
    printf("Memory error!\n");
    return -1;
  }
  memset(global.patternDomainInfo.pPattern, 0x00, sizeof(PatternT) * global.patternDomainInfo.size);

  global.endPointInfo.pPatternEndPoint = (PatternEndPointT*)malloc(sizeof(PatternEndPointT) * global.endPointInfo.size);
  if(global.endPointInfo.pPatternEndPoint == 0){
    printf("Memory error!\n");
    return -1;
  }
  memset(global.endPointInfo.pPatternEndPoint, 0x00, sizeof(PatternEndPointT) * global.endPointInfo.size);

  global.mineEndPointInfo.pPatternEndPoint = (PatternEndPointT*)malloc(sizeof(PatternEndPointT) * global.mineEndPointInfo.size);
  if(global.mineEndPointInfo.pPatternEndPoint == 0){
    printf("Memory error!\n");
    return -1;
  }
  memset(global.mineEndPointInfo.pPatternEndPoint, 0x00, sizeof(PatternEndPointT) * global.mineEndPointInfo.size);

  global.idNameInfo.pIDName = (IDNameT*)malloc(sizeof(IDNameT) * global.idNameInfo.size);
  if(global.idNameInfo.pIDName == 0){
    printf("Memory error!\n");
    return -1;
  }
  memset(global.idNameInfo.pIDName, 0x00, sizeof(IDNameT) * global.idNameInfo.size);

  global.regexInfo.pRegex = (RegexT*)malloc(sizeof(RegexT) * global.regexInfo.size);
  if(global.regexInfo.pRegex == 0){
    printf("Memory error!\n");
    return -1;
  }
  memset(global.regexInfo.pRegex, 0x00, sizeof(RegexT) * global.regexInfo.size);
  return 0;
}

cJSON* readFile(char *filename)
{
  int size, n, len;
  FILE *fp;
  cJSON *pRoot;
  char *pText, *pTmp;

  size = 65536;
  pText = (char*)malloc(size);
  if(pText == 0){
    printf("Memory error!\n");
    return 0;
  }
  fp = fopen(filename, "r");
  if(fp == 0){
    printf("Open file %s failed!\n", filename);
    return 0;
  }
  len = 0;
  while(1){
    if((len+1024) >= size){
      pTmp = (char*)malloc(size*2);
      if(pTmp == 0)
        return 0;
      memcpy(pTmp, pText, len);
      size = size * 2;
      free(pText);
      pText = pTmp;
      pTmp = 0;
    }
    n = fread(pText+len, 1, 1024, fp);
    if(n <= 0)
      break;
    len += n;
  }
  pRoot = cJSON_Parse(pText);
  free(pText);
  if(pRoot == 0){
    printf("Parse json failed!\n");
    return 0;
  }
  return pRoot;
}

int analyzeJson(cJSON *pRoot, int type)
{
  int i, j, find, cnt, ret, len, iStart;
  cJSON *pChild, *pArr, *pItem;
  char *p, buf[1024];
  IDNameT *pIDName;

  pChild = pRoot->child;
  while(pChild != 0){
    if(strstr("objects", pChild->string) && (pChild->type == cJSON_Array)){
      cnt = cJSON_GetArraySize(pChild);
      for(i = 0; i < cnt; i++){
        pArr = cJSON_GetArrayItem(pChild, i);
        if(pArr->type == cJSON_Object){
          pItem = cJSON_GetObjectItem(pArr, "pattern");
          if(pItem != 0){
            strcpy(buf, pItem->valuestring);
            ret = getStixInfo(pArr, buf, type);
            if(ret)
              return -1;
          }
          pItem = cJSON_GetObjectItem(pArr, "name");
          if(pItem == 0)
            continue;
          strcpy(buf, pItem->valuestring);
          find = 0;
          for(j = 0; j < global.idNameInfo.cnt; j++){
            pIDName = global.idNameInfo.pIDName + j;
            if(!strcmp(buf, global.nameStringInfo.pStr + pIDName->iStart)){
              find = 1;
              break;
            }
          }
          if(find)
            continue;
          trim(buf);
          len = strlen(buf);

          iStart = global.nameStringInfo.len;
          if((iStart + len + 5) >= global.nameStringInfo.size){
            p = (char*)malloc(global.nameStringInfo.size*2);
            if(p == 0){
              printf("Memory error!\n");
              return -1;
            }
            memset(p, 0x00, global.nameStringInfo.size*2);
            memcpy(p, global.nameStringInfo.pStr, global.nameStringInfo.size);
            global.nameStringInfo.size = global.nameStringInfo.size * 2;
            free(global.nameStringInfo.pStr);
            global.nameStringInfo.pStr = p;
            p = 0;
          }
          strcpy(global.nameStringInfo.pStr + iStart, buf);
          global.nameStringInfo.len = iStart + len + 5;

          if(global.idNameInfo.cnt >= global.idNameInfo.size){
            global.idNameInfo.size = global.idNameInfo.size * 2;
            pIDName = (IDNameT*)malloc(sizeof(IDNameT) * global.idNameInfo.size);
            if(pIDName == 0){
              printf("Memory error!\n");
              return -1;
            }
            memset(pIDName, 0x00, sizeof(IDNameT) * global.idNameInfo.size);
            memcpy(pIDName, global.idNameInfo.pIDName, sizeof(IDNameT) * global.idNameInfo.cnt);
            free(global.idNameInfo.pIDName);
            global.idNameInfo.pIDName = pIDName;
            pIDName = 0;
          }
          pIDName = global.idNameInfo.pIDName + global.idNameInfo.cnt;
          global.idNameInfo.cnt++;
          pIDName->iStart = iStart;
          pIDName->id = global.idNameInfo.cnt;
        }
      }
    }
    pChild = pChild -> next;
  }
  return 0;
}

int getStixInfo(cJSON *pItem, char *pattern, int type)
{
  int ret;
  char *p;

  p = strstr(pattern, "url:value");
  if(p){
    ret = getStixUrlInfo(pItem, pattern, type);
    return ret;
  }
  p = strstr(pattern, "domain-name");
  if(p){
    ret = getStixDomainInfo(pItem, pattern, type);
    return ret;
  }
  p = strstr(pattern, "dst_port IN");
  if(p){
    if(type == TYPE_STIX_MINE)
      ret = getMineEndPointInfo(pItem, pattern);
    else
      ret = getEndPointInfo(pItem, pattern);
    return ret;
  }
  p = strstr(pattern, "dst_port=");
  if(p){
    if(type == TYPE_STIX_MINE)
      ret = getMineEndPointInfo(pItem, pattern);
    else
      ret = getEndPointInfo(pItem, pattern);
    return ret;
  }
  return 0;
}

int getStixUrlInfo(cJSON *pItem, char *pattern, int type)
{
  int i, len, iStart;
  char *p, *str;
  cJSON *pChild;
  PatternT *pInfo;
  char buf[1024];

  str = 0;
  strcpy(buf, pattern);
  p = strstr(buf, "url:value");
  if(p == 0)
    return 0;

  p = strstr(buf, "http://");
  if(p){
    str = p + 6;
  }else{
    p = strstr(buf, "url:value");
    str = p + 10;
  }
  len = strlen(str);
  for(i = 0; i < len; i++){
    if(!strncmp(str+i, "IN ", 3)){
      str[i] = ' ';
      str[i+1] = ' ';
      i += 2;
      continue;
    }
    if(str[i] == '(')
      str[i] = ' ';
    if(str[i] == ')')
      str[i] = ' ';
    if(str[i] == '[')
      str[i] = ' ';
    if(str[i] == ']')
      str[i] = ' ';
    if(str[i] == 0x27)
      str[i] = ' ';
    if(str[i] == '=')
      str[i] = ' ';
  }
  trim(str);
  len = strlen(str);
  iStart = global.urlStringInfo.len;
  if((iStart + len + 5) >= global.urlStringInfo.size){
    p = (char*)malloc(global.urlStringInfo.size*2);
    memset(p, 0x00, global.urlStringInfo.size*2);
    if(p == 0){
      printf("Memory error!\n");
      return -1;
    }
    memcpy(p, global.urlStringInfo.pStr, global.urlStringInfo.size);
    global.urlStringInfo.size = global.urlStringInfo.size * 2;
    free(global.urlStringInfo.pStr);
    global.urlStringInfo.pStr = p;
    p = 0;
  }
  strcpy(global.urlStringInfo.pStr + iStart, str);
  global.urlStringInfo.len = iStart + len + 5;

  if(global.patternUrlInfo.cnt >= global.patternUrlInfo.size){
    global.patternUrlInfo.size = global.patternUrlInfo.size * 2;
    pInfo = (PatternT*)malloc(sizeof(PatternT) * global.patternUrlInfo.size);
    if(pInfo == 0){
      printf("Memory error!\n");
      return -1;
    }
    memset(pInfo, 0x00, sizeof(PatternT) * global.patternUrlInfo.size);
    memcpy(pInfo, global.patternUrlInfo.pPattern, sizeof(PatternT) * global.patternUrlInfo.cnt);
    free(global.patternUrlInfo.pPattern);
    global.patternUrlInfo.pPattern = pInfo;
    pInfo = 0;
  }
  pInfo = global.patternUrlInfo.pPattern + global.patternUrlInfo.cnt;
  global.patternUrlInfo.cnt++;
  pInfo->iStart = iStart;
  pChild = cJSON_GetObjectItem(pItem, "name");
  if(pChild)
    strcpy(pInfo->name, pChild->valuestring);
  pInfo->type = type;
  return 0;
}

int getStixRegexDomainInfo(cJSON *pItem, char *pattern)
{
  int i, len;
  char *p, *str;
  char buf[1024];
  cJSON *pChild;
  RegexT *pRegex;

  str = 0;
  strcpy(buf, pattern);
  p = strstr(buf, "MATCHES");
  if(p == 0)
    return 0;
  str = p + 8;
  len = strlen(p);
  for(i = 0; i < len; i++){
    if(!strncmp(str + i, "AND", 3)){
      str[i] = 0;
      break;
    }
    if(str[i] == ']')
      str[i] = ' ';
    if(str[i] == 0x27)
      str[i] = ' ';
  }
  trim(str);
  if(global.regexInfo.cnt >= global.regexInfo.size){
    pRegex = (RegexT*)malloc(sizeof(RegexT) * global.regexInfo.size*2);
    if(pRegex == 0){
      printf("Memory error!\n");
      return -1;
    }
    memset(pRegex, 0x00, sizeof(RegexT) * global.regexInfo.size*2);
    memcpy(pRegex, global.regexInfo.pRegex, sizeof(RegexT) * global.regexInfo.size);
    global.regexInfo.size = global.regexInfo.size * 2;
    free(global.regexInfo.pRegex);
    global.regexInfo.pRegex = pRegex;
    pRegex = 0;
  }
  pRegex = global.regexInfo.pRegex + global.regexInfo.cnt;
  pChild = cJSON_GetObjectItem(pItem, "name");
  if(pChild)
    strcpy(pRegex->name, pChild->valuestring);
  strcpy(pRegex->pattern, str);
  global.regexInfo.cnt++;
  return 0;
}

int getStixDomainInfo(cJSON *pItem, char *pattern, int type)
{
  int i, len, iStart;
  char *p, *p1, *str;
  cJSON *pChild;
  PatternT *pInfo;
  char buf[1024];

  str = 0;
  strcpy(buf, pattern);
  p = strstr(buf, "dst_ref.value");
  if(p == 0)
    return 0;
  p1 = strstr(buf, "MATCHES");
  if(p1)
    return getStixRegexDomainInfo(pItem, pattern);

  str = p + 14;
  len = strlen(str);
  for(i = 0; i < len; i++){
    if(str[i] == ' '){
      str[i] = 0;
      break;
    }
    if(str[i] == '[')
      str[i] = ' ';
    if(str[i] == ']')
      str[i] = ' ';
    if(str[i] == 0x27)
      str[i] = ' ';
    if(str[i] == '=')
      str[i] = ' ';
    if(!strncmp(str + i, "AND", 3)){
      str[i] = 0;
      break;
    }
  }
  trim(str);
  len = strlen(str);
  iStart = global.domainStringInfo.len;
  if((iStart + len + 5) >= global.domainStringInfo.size){
    p = (char*)malloc(global.domainStringInfo.size*2);
    if(p == 0){
      printf("Memory error!\n");
      return -1;
    }
    memset(p, 0x00, global.domainStringInfo.size*2);
    memcpy(p, global.domainStringInfo.pStr, global.domainStringInfo.size);
    global.domainStringInfo.size = global.domainStringInfo.size * 2;
    free(global.domainStringInfo.pStr);
    global.domainStringInfo.pStr = p;
    p = 0;
  }
  strcpy(global.domainStringInfo.pStr + iStart, str);
  global.domainStringInfo.len = iStart + len + 5;

  if(global.patternDomainInfo.cnt >= global.patternDomainInfo.size){
    global.patternDomainInfo.size = global.patternDomainInfo.size * 2;
    pInfo = (PatternT*)malloc(sizeof(PatternT) * global.patternDomainInfo.size);
    if(pInfo == 0){
      printf("Memory error!\n");
      return -1;
    }
    memset(pInfo, 0x00, sizeof(PatternT) * global.patternDomainInfo.size);
    memcpy(pInfo, global.patternDomainInfo.pPattern, sizeof(PatternT) * global.patternDomainInfo.cnt);
    free(global.patternDomainInfo.pPattern);
    global.patternDomainInfo.pPattern = pInfo;
    pInfo = 0;
  }
  pInfo = global.patternDomainInfo.pPattern + global.patternDomainInfo.cnt;
  global.patternDomainInfo.cnt++;
  pInfo->iStart = iStart;
  pChild = cJSON_GetObjectItem(pItem, "name");
  if(pChild)
    strcpy(pInfo->name, pChild->valuestring);
  pInfo->type = type;
  return 0;
}

int getEndPointInfo(cJSON *pItem, char *pattern)
{
  int i, len, port;
  u_int32_t uv;
  char *p, *str;
  cJSON *pChild;
  unsigned long ulv;
  PatternEndPointT *pInfo;
  char buf[1024];

  str = 0;
  port = 0;
  uv = 0;

  strcpy(buf, pattern);
  p = strstr(buf, "dst_port IN");
  if(p)
    str = p + 11;
  if(p == 0){
    p = strstr(buf, "dst_port=");
    if(p)
      str = p + 9;
  }
  if(str == 0)
    return 0;
  len = strlen(str);
  for(i = 0; i < len; i++){
    if(str[i] == '(')
      str[i] = ' ';
    if(str[i] == 0x27)
      str[i] = ' ';
    if(str[i] == ']')
      str[i] = ' ';
    if(str[i] == ')'){
      str[i] = 0;
      break;
    }
  }
  trim(str);
  port = atoi(str);

  strcpy(buf, pattern);
  p = strstr(buf, "dst_ref.value");
  if(p == 0)
    return 0;
  str = p + 13;
  len = strlen(str);
  for(i = 0; i < len; i++){
    if(str[i] == ' '){
      str[i] = 0;
      break;
    }
    if(str[i] == '=')
      str[i] = ' ';
    if(str[i] == 0x27)
      str[i] = ' ';
    if(!strncmp(str + i, "AND", 3)){
      str[i] = 0;
      break;
    }
  }
  trim(str);
  getIPFromStr(str, &uv);

  if(port == 0)
    return 0;
  if(uv == 0)
    return 0;
  ulv = uv;
  ulv = ulv * 65536 + port;

  if(global.endPointInfo.cnt >= global.endPointInfo.size){
    global.endPointInfo.size = global.endPointInfo.size * 2;
    pInfo = (PatternEndPointT*)malloc(sizeof(PatternEndPointT) * global.endPointInfo.size);
    if(pInfo == 0){
      printf("Memory error!\n");
      return -1;
    }
    memset(pInfo, 0x00, sizeof(PatternEndPointT) * global.endPointInfo.size);
    memcpy(pInfo, global.endPointInfo.pPatternEndPoint, sizeof(PatternEndPointT) * global.endPointInfo.cnt);
    free(global.endPointInfo.pPatternEndPoint);
    global.endPointInfo.pPatternEndPoint = pInfo;
    pInfo = 0;
  }
  pInfo = global.endPointInfo.pPatternEndPoint + global.endPointInfo.cnt;
  global.endPointInfo.cnt++;
  pChild = cJSON_GetObjectItem(pItem, "name");
  if(pChild)
    strcpy(pInfo->name, pChild->valuestring);
  pInfo->ulv = ulv;
  pInfo->type = TYPE_STIX;
  return 0;
}

int getMineEndPointInfo(cJSON *pItem, char *pattern)
{
  int i, len, port;
  u_int32_t uv;
  char *p, *str;
  cJSON *pChild;
  unsigned long ulv;
  PatternEndPointT *pInfo;
  char buf[1024];

  str = 0;
  port = 0;
  uv = 0;

  strcpy(buf, pattern);
  p = strstr(buf, "dst_port IN");
  if(p)
    str = p + 11;
  if(p == 0){
    p = strstr(buf, "dst_port=");
    if(p)
      str = p + 9;
  }
  if(str == 0)
    return 0;
  len = strlen(str);
  for(i = 0; i < len; i++){
    if(str[i] == '(')
      str[i] = ' ';
    if(str[i] == 0x27)
      str[i] = ' ';
    if(str[i] == ']')
      str[i] = ' ';
    if(str[i] == ')'){
      str[i] = 0;
      break;
    }
  }
  trim(str);
  port = atoi(str);

  strcpy(buf, pattern);
  p = strstr(buf, "dst_ref.value");
  if(p == 0)
    return 0;
  str = p + 13;
  len = strlen(str);
  for(i = 0; i < len; i++){
    if(str[i] == ' '){
      str[i] = 0;
      break;
    }
    if(str[i] == '=')
      str[i] = ' ';
    if(str[i] == 0x27)
      str[i] = ' ';
    if(!strncmp(str + i, "AND", 3)){
      str[i] = 0;
      break;
    }
  }
  trim(str);
  getIPFromStr(str, &uv);

  if(port == 0)
    return 0;
  if(uv == 0)
    return 0;
  ulv = uv;
  ulv = ulv * 65536 + port;

  if(global.mineEndPointInfo.cnt >= global.mineEndPointInfo.size){
    global.mineEndPointInfo.size = global.mineEndPointInfo.size * 2;
    pInfo = (PatternEndPointT*)malloc(sizeof(PatternEndPointT) * global.mineEndPointInfo.size);
    if(pInfo == 0){
      printf("Memory error!\n");
      return -1;
    }
    memset(pInfo, 0x00, sizeof(PatternEndPointT) * global.mineEndPointInfo.size);
    memcpy(pInfo, global.mineEndPointInfo.pPatternEndPoint, sizeof(PatternEndPointT) * global.mineEndPointInfo.cnt);
    free(global.mineEndPointInfo.pPatternEndPoint);
    global.mineEndPointInfo.pPatternEndPoint = pInfo;
    pInfo = 0;
  }
  pInfo = global.mineEndPointInfo.pPatternEndPoint + global.mineEndPointInfo.cnt;
  global.mineEndPointInfo.cnt++;
  pChild = cJSON_GetObjectItem(pItem, "name");
  if(pChild)
    strcpy(pInfo->name, pChild->valuestring);
  pInfo->ulv = ulv;
  pInfo->type = TYPE_STIX_MINE;
  return 0;
}

void sortName()
{
  int i, j, ind, min_iStart, cnt;
  char *str;
  IDNameT *pIDName, entity;

  pIDName = global.idNameInfo.pIDName;
  cnt = global.idNameInfo.cnt;
  str = global.nameStringInfo.pStr;
  for(i = 0; i < cnt; i++){
    ind = i+1;
    min_iStart = pIDName[i+1].iStart;
    for(j = i+2; j < cnt; j++){
      if(strcmp(str + min_iStart, str + pIDName[j].iStart) > 0){
        min_iStart = pIDName[j].iStart;
        ind = j;
      }
    }
    if(strcmp(str + pIDName[i].iStart, str + min_iStart) > 0){
      memcpy(&entity, pIDName+i, sizeof(IDNameT));
      memcpy(pIDName+i, pIDName+ind, sizeof(IDNameT));
      memcpy(pIDName+ind, &entity, sizeof(IDNameT));
    }
  }
}

void sortPattern(PatternT *pPattern, char *str, int cnt)
{
  int i, j, ind, min_iStart;
  PatternT entity;

  for(i = 0; i < cnt; i++){
    ind = i+1;
    min_iStart = pPattern[i+1].iStart;
    for(j = i+2; j < cnt; j++){
      if(strcmp(str + min_iStart, str + pPattern[j].iStart) > 0){
        min_iStart = pPattern[j].iStart;
        ind = j;
      }
    }
    if(strcmp(str + pPattern[i].iStart, str + min_iStart) > 0){
      memcpy(&entity, pPattern+i, sizeof(PatternT));
      memcpy(pPattern+i, pPattern+ind, sizeof(PatternT));
      memcpy(pPattern+ind, &entity, sizeof(PatternT));
    }
  }
}

void sortEndPointPattern(PatternEndPointT *pPatternEndPoint, int cnt)
{
  int i, j, ind;
  unsigned long min_ulv;
  PatternEndPointT entity;

  for(i = 0; i < cnt; i++){
    ind = i+1;
    min_ulv = pPatternEndPoint[i+1].ulv;
    for(j = i+2; j < cnt; j++){
      if(pPatternEndPoint[j].ulv < min_ulv){
        min_ulv = pPatternEndPoint[j].ulv;
        ind = j;
      }
    }
    if(min_ulv < pPatternEndPoint[i].ulv){
      memcpy(&entity, pPatternEndPoint+i, sizeof(PatternEndPointT));
      memcpy(pPatternEndPoint+i, pPatternEndPoint+ind, sizeof(PatternEndPointT));
      memcpy(pPatternEndPoint+ind, &entity, sizeof(PatternEndPointT));
    }
  }
}

int findIDByName(char *str)
{
  int cnt, start, end, middle;
  char *strs;
  IDNameT *pIDName;

  cnt = global.idNameInfo.cnt;
  strs = global.nameStringInfo.pStr;
  pIDName = global.idNameInfo.pIDName;
  if(cnt <= 2){
    if(!strcmp(str, strs + pIDName[0].iStart))
      return pIDName[0].id;
    if(cnt == 1)
      return 0;
    if(!strcmp(str, strs + pIDName[1].iStart))
      return pIDName[1].id;
    return 0;
  }
  start = 0;
  end = cnt;
  while(1){
    if(start > end)
      break;
    if(end - start <= 2){
      if(!strcmp(str, strs + pIDName[start].iStart))
        return pIDName[start].id;
      if(start == end)
        return 0;
      if(!strcmp(str, strs + pIDName[start+1].iStart))
        return pIDName[start+1].id;
      return 0;
    }
    middle = (start + end) / 2;
    if(!strcmp(str, strs + pIDName[middle].iStart))
      return pIDName[middle].id;
    if(strcmp(str, strs + pIDName[middle].iStart) < 0){
      end = middle;
      continue;
    }
    if(strcmp(str, strs + pIDName[middle].iStart) > 0){
      start = middle + 1;
      continue;
    }
  }
  return 0;
}

PatternT* findPattern(PatternT *pPattern, char *strs, int cnt, char *str)
{
  int start, end, middle;

  if(cnt <= 2){
    if(!strcmp(str, strs + pPattern[0].iStart))
      return pPattern;
    if(cnt == 1)
      return 0;
    if(!strcmp(str, strs + pPattern[1].iStart))
      return pPattern + 1;
    return 0;
  }
  start = 0;
  end = cnt;
  while(1){
    if(start > end)
      break;
    if(end - start <= 2){
      if(!strcmp(str, strs + pPattern[start].iStart))
        return pPattern + start;
      if(start == end)
        return 0;
      if(!strcmp(str, strs + pPattern[start+1].iStart))
        return pPattern + start + 1;
      return 0;
    }
    middle = (start + end) / 2;
    if(!strcmp(str, strs + pPattern[middle].iStart))
      return pPattern + middle;
    if(strcmp(str, strs + pPattern[middle].iStart) < 0){
      end = middle;
      continue;
    }
    if(strcmp(str, strs + pPattern[middle].iStart) > 0){
      start = middle + 1;
      continue;
    }
  }
  return 0;
}

PatternEndPointT* findEndPointPattern(u_int32_t address, int port, int type)
{
  int start, end, middle, cnt;
  unsigned long ulv;
  PatternEndPointT *pPatternEndPoint;

  ulv = address;
  ulv = ulv * 65536 + port;
  if(type == TYPE_STIX_MINE){
    cnt = global.mineEndPointInfo.cnt;
    pPatternEndPoint = global.mineEndPointInfo.pPatternEndPoint;
  }else{
    cnt = global.endPointInfo.cnt;
    pPatternEndPoint = global.endPointInfo.pPatternEndPoint;
  }
  if(cnt <= 2){
    if(ulv == pPatternEndPoint[0].ulv)
      return pPatternEndPoint;
    if(cnt == 1)
      return 0;
    if(ulv == pPatternEndPoint[1].ulv)
      return pPatternEndPoint + 1;
    return 0;
  }
  start = 0;
  end = cnt;
  while(1){
    if(start > end)
      break;
    if(end - start <= 2){
      if(ulv == pPatternEndPoint[start].ulv)
        return pPatternEndPoint + start;
      if(start == end)
        return 0;
      if(ulv == pPatternEndPoint[start+1].ulv)
        return pPatternEndPoint + start + 1;
      return 0;
    }
    middle = (start + end) / 2;
    if(ulv == pPatternEndPoint[middle].ulv)
      return pPatternEndPoint + middle;
    if(ulv < pPatternEndPoint[middle].ulv){
      end = middle;
      continue;
    }
    if(ulv > pPatternEndPoint[middle].ulv){
      start = middle + 1;
      continue;
    }
  }
  return 0;
}

int processHttpInfo(HttpInfoT *pHttpInfo)
{
  int i, v, cnt, ovector[32];
  char tmp[1024], buf[1024];
  RegexT *pRegex;
  PatternT *pPattern;
  PatternEndPointT *pPatternEndPoint;

  pPatternEndPoint = findEndPointPattern(pHttpInfo->src, pHttpInfo->sport, TYPE_STIX);
  if(pPatternEndPoint){
    v = findIDByName(pPatternEndPoint->name);
    sprintf(pHttpInfo->strID, "%d", v);
    strcpy(pHttpInfo->name, pPatternEndPoint->name);
    pHttpInfo->type = TYPE_STIX;
    return 1;
  }

  pPatternEndPoint = findEndPointPattern(pHttpInfo->src, pHttpInfo->sport, TYPE_STIX_MINE);
  if(pPatternEndPoint){
    v = findIDByName(pPatternEndPoint->name);
    sprintf(pHttpInfo->strID, "%d", v);
    strcpy(pHttpInfo->name, pPatternEndPoint->name);
    pHttpInfo->type = TYPE_STIX_MINE;
    return 1;
  }

  pPatternEndPoint = findEndPointPattern(pHttpInfo->dst, pHttpInfo->dport, TYPE_STIX);
  if(pPatternEndPoint){
    v = findIDByName(pPatternEndPoint->name);
    sprintf(pHttpInfo->strID, "%d", v);
    strcpy(pHttpInfo->name, pPatternEndPoint->name);
    pHttpInfo->type = TYPE_STIX;
    return 1;
  }

  pPatternEndPoint = findEndPointPattern(pHttpInfo->dst, pHttpInfo->dport, TYPE_STIX_MINE);
  if(pPatternEndPoint){
    v = findIDByName(pPatternEndPoint->name);
    sprintf(pHttpInfo->strID, "%d", v);
    strcpy(pHttpInfo->name, pPatternEndPoint->name);
    pHttpInfo->type = TYPE_STIX_MINE;
    return 1;
  }

  pPattern = findPattern(global.patternUrlInfo.pPattern, global.urlStringInfo.pStr, global.patternUrlInfo.cnt, pHttpInfo->url);
  if(pPattern){
    v = findIDByName(pPattern->name);
    sprintf(pHttpInfo->strID, "%d", v);
    strcpy(pHttpInfo->name, pPattern->name);
    pHttpInfo->type = pPattern->type;
    return 1;
  }

  pPattern = findPattern(global.patternDomainInfo.pPattern, global.domainStringInfo.pStr, global.patternDomainInfo.cnt, pHttpInfo->domain);
  if(pPattern){
    v = findIDByName(pPattern->name);
    sprintf(pHttpInfo->strID, "%d", v);
    strcpy(pHttpInfo->name, pPattern->name);
    pHttpInfo->type = pPattern->type;
    return 1;
  }

  getStrIP(tmp, pHttpInfo->dst);
  sprintf(buf, "/%s%s", tmp, pHttpInfo->url);
  pPattern = findPattern(global.patternUrlInfo.pPattern, global.urlStringInfo.pStr, global.patternUrlInfo.cnt, buf);
  if(pPattern){
    v = findIDByName(pPattern->name);
    sprintf(pHttpInfo->strID, "%d", v);
    strcpy(pHttpInfo->name, pPattern->name);
    pHttpInfo->type = pPattern->type;
    return 1;
  }

  sprintf(buf, "/%s:%d%s", tmp, pHttpInfo->dport, pHttpInfo->url);
  pPattern = findPattern(global.patternUrlInfo.pPattern, global.urlStringInfo.pStr, global.patternUrlInfo.cnt, buf);
  if(pPattern){
    v = findIDByName(pPattern->name);
    sprintf(pHttpInfo->strID, "%d", v);
    strcpy(pHttpInfo->name, pPattern->name);
    pHttpInfo->type = pPattern->type;
    return 1;
  }

  sprintf(buf, "/%s%s", pHttpInfo->domain, pHttpInfo->url);
  pPattern = findPattern(global.patternUrlInfo.pPattern, global.urlStringInfo.pStr, global.patternUrlInfo.cnt, buf);
  if(pPattern){
    v = findIDByName(pPattern->name);
    sprintf(pHttpInfo->strID, "%d", v);
    strcpy(pHttpInfo->name, pPattern->name);
    pHttpInfo->type = pPattern->type;
    return 1;
  }

  sprintf(buf, "/%s:%d%s", pHttpInfo->domain, pHttpInfo->dport, pHttpInfo->url);
  pPattern = findPattern(global.patternUrlInfo.pPattern, global.urlStringInfo.pStr, global.patternUrlInfo.cnt, buf);
  if(pPattern){
    v = findIDByName(pPattern->name);
    sprintf(pHttpInfo->strID, "%d", v);
    strcpy(pHttpInfo->name, pPattern->name);
    pHttpInfo->type = pPattern->type;
    return 1;
  }

  cnt = global.regexInfo.cnt;
  for(i = 0; i < cnt; i++){
    pRegex = global.regexInfo.pRegex + i;
    if(pRegex->re == 0)
      continue;
    v = pcre_exec(pRegex->re, NULL, pHttpInfo->domain, strlen(pHttpInfo->domain), 0, 0, ovector, 32);
    if(v >= 0){
      v = findIDByName(pRegex->name);
      sprintf(pHttpInfo->strID, "%d", v);
      strcpy(pHttpInfo->name, pRegex->name);
      pHttpInfo->type = TYPE_STIX_MINE;
      return 1;
    }
  }
  return 0;
}

int initStixEnviormentByMemFile(const char *filename)
{
  int i, len;
  FILE *fp;
  char *p1, *p2, buf[256], tmp[256], dir[256];
  char urlFile[256], nameFile[256], regexFile[256];
  char endPointFile[256], domainFile[256];
  char urlStringFile[256], domainStringFile[256];
  char nameStringFile[256], mineEndPointFile[256];

  strcpy(buf, filename);
  len = strlen(buf);
  dir[0] = 0;
  urlFile[0] = 0;
  domainFile[0] = 0;
  endPointFile[0] = 0;
  nameFile[0] = 0;
  urlStringFile[0] = 0;
  domainStringFile[0] = 0;
  nameStringFile[0] = 0;
  for(i = 0; i < len; i++){
    if(buf[i] == '/'){
      buf[i] = 0;
      strcpy(dir, buf);
      break;
    }
  }
  fp = fopen(filename, "r");
  if(fp == 0){
    printf("open file %s failed\n", filename);
    return -1;
  }
  while(1){
    p1 = fgets(buf, 1024, fp);
    if(p1 == 0)
      break;
    len = strlen(buf);

    if(buf[len-1] == '\n')
      buf[len-1] = 0;
    if(buf[len-1] == '\r')
      buf[len-1] = 0;
    len--;
    if(buf[len-1] == '\n')
      buf[len-1] = 0;
    if(buf[len-1] == '\r')
      buf[len-1] = 0;

    p2 = strstr(buf, ":");
    if(p2 == 0)
      continue;
    p2++;
    //v = atoi(p2);
    if(strstr(buf, "url count"))
      global.patternUrlInfo.cnt = atoi(p2);
    if(strstr(buf, "domain count"))
      global.patternDomainInfo.cnt = atoi(p2);
    if(strstr(buf, "endpoint count"))
      global.endPointInfo.cnt = atoi(p2);
    if(strstr(buf, "endpoint mine count"))
      global.mineEndPointInfo.cnt = atoi(p2);
    if(strstr(buf, "name count"))
      global.idNameInfo.cnt = atoi(p2);
    if(strstr(buf, "regex count"))
      global.regexInfo.cnt = atoi(p2);

    if(strstr(buf, "url string length")){
      global.urlStringInfo.len = atoi(p2);
      global.urlStringInfo.size = atoi(p2);
    }
    if(strstr(buf, "domain string length")){
      global.domainStringInfo.len = atoi(p2);
      global.domainStringInfo.size = atoi(p2);
    }
    if(strstr(buf, "name string length")){
      global.nameStringInfo.len = atoi(p2);
      global.nameStringInfo.size = atoi(p2);
    }

    if(strstr(buf, "url info")){
      strcpy(urlFile, p2);
      trim(urlFile);
    }
    if(strstr(buf, "domain info")){
      strcpy(domainFile, p2);
      trim(domainFile);
    }
    if(strstr(buf, "endpoint info")){
      strcpy(endPointFile, p2);
      trim(endPointFile);
    }
    if(strstr(buf, "endpoint mine info")){
      strcpy(mineEndPointFile, p2);
      trim(mineEndPointFile);
    }
    if(strstr(buf, "name info")){
      strcpy(nameFile, p2);
      trim(nameFile);
    }
    if(strstr(buf, "regex info")){
      strcpy(regexFile, p2);
      trim(regexFile);
    }
    if(strstr(buf, "url string info")){
      strcpy(urlStringFile, p2);
      trim(urlStringFile);
    }
    if(strstr(buf, "domain string info")){
      strcpy(domainStringFile, p2);
      trim(domainStringFile);
    }
    if(strstr(buf, "name string info")){
      strcpy(nameStringFile, p2);
      trim(nameStringFile);
    }
  }
  fclose(fp);
  if(isEmptyString(urlFile)){
    printf("Not found url info file\n");
    return -1;
  }
  if(isEmptyString(domainFile)){
    printf("Not found domain info file\n");
    return -1;
  }
  if(isEmptyString(endPointFile)){
    printf("Not found end point info file\n");
    return -1;
  }
  if(isEmptyString(mineEndPointFile)){
    printf("Not found mine end point info file\n");
    return -1;
  }
  if(isEmptyString(nameFile)){
    printf("Not found name info file\n");
    return -1;
  }
  if(isEmptyString(regexFile)){
    printf("Not found regex info file\n");
    return -1;
  }
  if(isEmptyString(urlStringFile)){
    printf("Not found url info file\n");
    return -1;
  }
  if(isEmptyString(domainStringFile)){
    printf("Not found domain info file\n");
    return -1;
  }
  if(isEmptyString(nameStringFile)){
    printf("Not found name info file\n");
    return -1;
  }

  strcpy(tmp, urlFile);
  if(dir[0] != 0)
    sprintf(tmp, "%s/%s", dir, urlFile);
  global.patternUrlInfo.pPattern = (PatternT*)getMemInfoFromFile(tmp, global.patternUrlInfo.cnt * sizeof(PatternT));

  strcpy(tmp, domainFile);
  if(dir[0] != 0)
    sprintf(tmp, "%s/%s", dir, domainFile);
  global.patternDomainInfo.pPattern = (PatternT*)getMemInfoFromFile(tmp, global.patternDomainInfo.cnt * sizeof(PatternT));

  strcpy(tmp, endPointFile);
  if(dir[0] != 0)
    sprintf(tmp, "%s/%s", dir, endPointFile);
  global.endPointInfo.pPatternEndPoint = (PatternEndPointT*)getMemInfoFromFile(tmp, global.endPointInfo.cnt * sizeof(PatternEndPointT));

  strcpy(tmp, mineEndPointFile);
  if(dir[0] != 0)
    sprintf(tmp, "%s/%s", dir, mineEndPointFile);
  global.mineEndPointInfo.pPatternEndPoint = (PatternEndPointT*)getMemInfoFromFile(tmp, global.mineEndPointInfo.cnt * sizeof(PatternEndPointT));

  strcpy(tmp, nameFile);
  if(dir[0] != 0)
    sprintf(tmp, "%s/%s", dir, nameFile);
  global.idNameInfo.pIDName = (IDNameT*)getMemInfoFromFile(tmp, global.idNameInfo.cnt * sizeof(IDNameT));

  strcpy(tmp, regexFile);
  if(dir[0] != 0)
    sprintf(tmp, "%s/%s", dir, regexFile);
  global.regexInfo.pRegex = (RegexT*)getMemInfoFromFile(tmp, global.regexInfo.cnt * sizeof(RegexT));

  strcpy(tmp, urlStringFile);
  if(dir[0] != 0)
    sprintf(tmp, "%s/%s", dir, urlStringFile);
  global.urlStringInfo.pStr = getMemInfoFromFile(tmp, global.urlStringInfo.size);

  strcpy(tmp, domainStringFile);
  if(dir[0] != 0)
    sprintf(tmp, "%s/%s", dir, domainStringFile);
  global.domainStringInfo.pStr = getMemInfoFromFile(tmp, global.domainStringInfo.size);

  strcpy(tmp, nameStringFile);
  if(dir[0] != 0)
    sprintf(tmp, "%s/%s", dir, nameStringFile);
  global.nameStringInfo.pStr = getMemInfoFromFile(tmp, global.nameStringInfo.size);

  return 0;
}

int initRegex()
{
  int i, cnt, erroffset;
  const char *error;
  RegexT *pRegex;

  cnt = global.regexInfo.cnt;
  for(i = 0; i < cnt; i++){
    pRegex = global.regexInfo.pRegex + i;
    pRegex->re = pcre_compile(pRegex->pattern, 0, &error, &erroffset, NULL);
  }
  return 0;
}
