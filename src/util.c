#include <zlib.h>
#include "util.h"
#include "cJSON.h"

extern GlobalT global;

int initGlobal()
{
  memset(&global, 0x00, sizeof(GlobalT));
  return 0;
}

void trim(char *str)
{
  int len, i;
  char tmp[8192], *p;

  strcpy(tmp, str);
  len = strlen(tmp);
  p = 0;
  for(i = 0; i < len; i++){
    if(tmp[i] != ' '){
      if(tmp[i] == '\r')
        continue;
      if(tmp[i] == '\n')
        continue;
      p = tmp + i;
      break;
    }
  }
  if(p)
    strcpy(str, p);
  len = strlen(str);
  for(i = len - 1; i >= 0; i--){
    if(tmp[i] == '\r'){
      str[i] = 0;
      continue;
    }
    if(tmp[i] == '\n'){
      str[i] = 0;
      continue;
    }
    if(str[i] != ' ')
      break;
    str[i] = 0;
  }
}
int isEmptyString(char *str)
{
  if(str == 0)
    return 1;
  if(str[0] == 0)
    return 1;
  if(!strcmp(str, "null"))
    return 1;
  if(!strcmp(str, "NULL"))
    return 1;
  return 0;
}
void toLowerCase(char *str)
{
  int i, len;

  len = strlen(str);
  for (i = 0; i < len; i++) {
    if((str[i] >= 'A') && (str[i] <= 'Z'))
      str[i] = str[i] + 32;
  }
}

int isZIP(const u_char *str)
{
  if(str[0] != 0x1F)
    return 0;
  if(str[1] != 0x8B)
    return 0;
  if(str[2] != 0x08)
    return 0;
  return 1;
}

int checkText(const u_char *str)
{
  int i;

  for(i = 0; i < 10; i++){
    if(str[i] < 0x20)
      return 0;
  }
  return 1;
}

int unzip(char *src, int srcLen, char *dst, int dstLen)
{
  int ret;
  z_stream stream;

  memset(&stream, 0x00, sizeof(z_stream));
  stream.next_in = (void*)src;
  stream.avail_in = srcLen;
  stream.next_out = (void*)dst;
  stream.avail_out = dstLen;

  if(inflateInit2(&stream, 47) != Z_OK)
    return -1;

  ret = inflate(&stream, Z_NO_FLUSH);
  if((ret != Z_OK) && (ret != Z_STREAM_END)){
    inflateEnd(&stream);
    return -2;
  }

  if (inflate(&stream, Z_FINISH) != Z_STREAM_END) {
    inflateEnd(&stream);
    return -3;
  }
  inflateEnd(&stream);
  return 0;
}

int makeHttpInfo(HttpInfoT *pHttpInfo, cJSON *pRoot)
{
  int i, j, v, len;
  char *p;
  cJSON *pChild;

  v = 0;
  pChild = cJSON_GetObjectItem(pRoot, "BEGIN_TIME");
  if((pChild == 0) && global.keyFlag)
    pChild = cJSON_GetObjectItem(pRoot, global.str_begin_time);
  if(pChild){
    v = 1;
    strcpy(pHttpInfo->strTime, pChild->valuestring);
    getTimeFromStr(pHttpInfo->strTime, &(pHttpInfo->stamp));
  }else{
    pChild = cJSON_GetObjectItem(pRoot, "timestamp");
    if(pChild){
      v = 1;
      pHttpInfo->stamp = pChild->valueint;;
    }
  }
  if(v == 0)
    return 0;

  pChild = cJSON_GetObjectItem(pRoot, "SRC_IP");
  if((pChild == 0) && global.keyFlag)
    pChild = cJSON_GetObjectItem(pRoot, global.str_src_ip);
  if(pChild == 0)
    return 0;
  strcpy(pHttpInfo->strClient, pChild->valuestring);
  getIPFromStr(pHttpInfo->strClient, &(pHttpInfo->src));

  pChild = cJSON_GetObjectItem(pRoot, "DST_IP");
  if((pChild == 0) && global.keyFlag)
    pChild = cJSON_GetObjectItem(pRoot, global.str_dst_ip);
  if(pChild == 0)
    return 0;
  strcpy(pHttpInfo->strServer, pChild->valuestring);
  getIPFromStr(pHttpInfo->strServer, &(pHttpInfo->dst));

  pChild = cJSON_GetObjectItem(pRoot, "FORWARD");
  if((pChild == 0) && global.keyFlag)
    pChild = cJSON_GetObjectItem(pRoot, global.str_forward);
  if(pChild){
    strcpy(pHttpInfo->strForward, pChild->valuestring);
    getIPFromStr(pHttpInfo->strForward, &(pHttpInfo->forward));
  }

  pChild = cJSON_GetObjectItem(pRoot, "RETCODE");
  if((pChild == 0) && global.keyFlag)
    pChild = cJSON_GetObjectItem(pRoot, global.str_retcode);
  if(pChild)
    pHttpInfo->ret_code = atoi(pChild->valuestring);

  pChild = cJSON_GetObjectItem(pRoot, "SRC_PORT");
  if((pChild == 0) && global.keyFlag)
    pChild = cJSON_GetObjectItem(pRoot, global.str_src_port);
  if(pChild == 0)
    return 0;
  pHttpInfo->sport = atoi(pChild->valuestring);

  pChild = cJSON_GetObjectItem(pRoot, "DST_PORT");
  if((pChild == 0) && global.keyFlag)
    pChild = cJSON_GetObjectItem(pRoot, global.str_dst_port);
  if(pChild == 0)
    return 0;
  pHttpInfo->dport = atoi(pChild->valuestring);

  pChild = cJSON_GetObjectItem(pRoot, "REQ_HEADER");
  if((pChild == 0) && global.keyFlag)
    pChild = cJSON_GetObjectItem(pRoot, global.str_req_header);
  if(pChild)
    strcpy(pHttpInfo->request_header, pChild->valuestring);

  pChild = cJSON_GetObjectItem(pRoot, "REQ_BODY");
  if((pChild == 0) && global.keyFlag)
    pChild = cJSON_GetObjectItem(pRoot, global.str_req_body);
  if(pChild)
    strcpy(pHttpInfo->request_body, pChild->valuestring);

  pChild = cJSON_GetObjectItem(pRoot, "RSP_HEADER");
  if((pChild == 0) && global.keyFlag)
    pChild = cJSON_GetObjectItem(pRoot, global.str_rsp_header);
  if(pChild)
    strcpy(pHttpInfo->response_header, pChild->valuestring);

  pChild = cJSON_GetObjectItem(pRoot, "RSP_BODY");
  if((pChild == 0) && global.keyFlag)
    pChild = cJSON_GetObjectItem(pRoot, global.str_rsp_body);
  if(pChild)
    strcpy(pHttpInfo->response_body, pChild->valuestring);

  pChild = cJSON_GetObjectItem(pRoot, "URL");
  if((pChild == 0) && global.keyFlag)
    pChild = cJSON_GetObjectItem(pRoot, global.str_url);
  if(pChild)
    strcpy(pHttpInfo->url, pChild->valuestring);

  pChild = cJSON_GetObjectItem(pRoot, "METHOD");
  if((pChild == 0) && global.keyFlag)
    pChild = cJSON_GetObjectItem(pRoot, global.str_method);
  if(pChild)
    strcpy(pHttpInfo->method, pChild->valuestring);

  pChild = cJSON_GetObjectItem(pRoot, "message");
  if((pChild == 0) && global.keyFlag)
    pChild = cJSON_GetObjectItem(pRoot, global.str_message);
  if(pChild)
    strcpy(pHttpInfo->message, pChild->valuestring);

  pChild = cJSON_GetObjectItem(pRoot, "host");
  if((pChild == 0) && global.keyFlag)
    pChild = cJSON_GetObjectItem(pRoot, global.str_host);
  if(pChild)
    strcpy(pHttpInfo->host, pChild->valuestring);

  pChild = cJSON_GetObjectItem(pRoot, "source");
  if((pChild == 0) && global.keyFlag)
    pChild = cJSON_GetObjectItem(pRoot, global.str_source);
  if(pChild)
    strcpy(pHttpInfo->source, pChild->valuestring);

  pChild = cJSON_GetObjectItem(pRoot, "DOMAIN");
  if((pChild == 0) && global.keyFlag)
    pChild = cJSON_GetObjectItem(pRoot, global.str_domain);
  if(pChild)
    strcpy(pHttpInfo->domain, pChild->valuestring);

  p = pHttpInfo->response_header;
  len = strlen(p);
  j = 0;
  for(i = 0; i < len; i++){
    if(p[i] == ' ')
      break;
    pHttpInfo->strVersion[j] = p[i];
    j++;
    pHttpInfo->strVersion[j] = 0;
    if((p[i] >= 'A') && (p[i] <= 'Z')){
      j = 0;
      pHttpInfo->strVersion[j] = 0;
    }
    if(p[i] == '/'){
      j = 0;
      pHttpInfo->strVersion[j] = 0;
    }
  }
  return 1;
}

int makeGelfHttpInfo(HttpInfoT *pHttpInfo, cJSON *pRoot)
{
  int i, j, v, len;
  char *p;
  cJSON *pChild;

  pChild = cJSON_GetObjectItem(pRoot, "_BEGIN_TIME");
  if((pChild == 0) && global.keyFlag)
    pChild = cJSON_GetObjectItem(pRoot, global.str_begin_time);
  if(pChild){
    v = 1;
    strcpy(pHttpInfo->strTime, pChild->valuestring);
    getTimeFromStr(pHttpInfo->strTime, &(pHttpInfo->stamp));
  }else{
    pChild = cJSON_GetObjectItem(pRoot, "timestamp");
    if(pChild){
      v = 1;
      pHttpInfo->stamp = pChild->valueint;
    }
  }
  if(v == 0)
    return 0;

  pChild = cJSON_GetObjectItem(pRoot, "_SRC_IP");
  if((pChild == 0) && global.keyFlag)
    pChild = cJSON_GetObjectItem(pRoot, global.str_src_ip);
  if(pChild == 0)
    return 0;
  strcpy(pHttpInfo->strClient, pChild->valuestring);
  getIPFromStr(pHttpInfo->strClient, &(pHttpInfo->src));

  pChild = cJSON_GetObjectItem(pRoot, "_DST_IP");
  if((pChild == 0) && global.keyFlag)
    pChild = cJSON_GetObjectItem(pRoot, global.str_dst_ip);
  if(pChild == 0)
    return 0;
  strcpy(pHttpInfo->strServer, pChild->valuestring);
  getIPFromStr(pHttpInfo->strServer, &(pHttpInfo->dst));

  pChild = cJSON_GetObjectItem(pRoot, "_FORWARD");
  if((pChild == 0) && global.keyFlag)
    pChild = cJSON_GetObjectItem(pRoot, global.str_forward);
  if(pChild){
    strcpy(pHttpInfo->strForward, pChild->valuestring);
    getIPFromStr(pHttpInfo->strForward, &(pHttpInfo->forward));
  }

  pChild = cJSON_GetObjectItem(pRoot, "_RETCODE");
  if((pChild == 0) && global.keyFlag)
    pChild = cJSON_GetObjectItem(pRoot, global.str_retcode);
  if(pChild)
    pHttpInfo->ret_code = atoi(pChild->valuestring);

  pChild = cJSON_GetObjectItem(pRoot, "_SRC_PORT");
  if((pChild == 0) && global.keyFlag)
    pChild = cJSON_GetObjectItem(pRoot, global.str_src_port);
  if(pChild == 0)
    return 0;
  pHttpInfo->sport = atoi(pChild->valuestring);

  pChild = cJSON_GetObjectItem(pRoot, "_DST_PORT");
  if((pChild == 0) && global.keyFlag)
    pChild = cJSON_GetObjectItem(pRoot, global.str_dst_port);
  if(pChild == 0)
    return 0;
  pHttpInfo->dport = atoi(pChild->valuestring);

  pChild = cJSON_GetObjectItem(pRoot, "_REQ_HEADER");
  if((pChild == 0) && global.keyFlag)
    pChild = cJSON_GetObjectItem(pRoot, global.str_req_header);
  if(pChild){
    strncpy(pHttpInfo->request_header, pChild->valuestring, 510);
    pHttpInfo->request_header[510] = 0;
  }

  pChild = cJSON_GetObjectItem(pRoot, "_REQ_BODY");
  if((pChild == 0) && global.keyFlag)
    pChild = cJSON_GetObjectItem(pRoot, global.str_req_body);
  if(pChild){
    strncpy(pHttpInfo->request_body, pChild->valuestring, 1020);
    pHttpInfo->request_body[1020] = 0;
  }

  pChild = cJSON_GetObjectItem(pRoot, "_RSP_HEADER");
  if((pChild == 0) && global.keyFlag)
    pChild = cJSON_GetObjectItem(pRoot, global.str_rsp_header);
  if(pChild){
    strncpy(pHttpInfo->response_header, pChild->valuestring, 510);
    pHttpInfo->response_header[510] = 0;
  }

  pChild = cJSON_GetObjectItem(pRoot, "_RSP_BODY");
  if((pChild == 0) && global.keyFlag)
    pChild = cJSON_GetObjectItem(pRoot, global.str_rsp_body);
  if(pChild){
    strncpy(pHttpInfo->response_body, pChild->valuestring, 1020);
    pHttpInfo->response_body[1020] = 0;
  }

  pChild = cJSON_GetObjectItem(pRoot, "_URL");
  if((pChild == 0) && global.keyFlag)
    pChild = cJSON_GetObjectItem(pRoot, global.str_url);
  if(pChild)
    strcpy(pHttpInfo->url, pChild->valuestring);

  pChild = cJSON_GetObjectItem(pRoot, "_METHOD");
  if((pChild == 0) && global.keyFlag)
    pChild = cJSON_GetObjectItem(pRoot, global.str_method);
  if(pChild)
    strcpy(pHttpInfo->method, pChild->valuestring);

  pChild = cJSON_GetObjectItem(pRoot, "_message");
  if((pChild == 0) && global.keyFlag)
    pChild = cJSON_GetObjectItem(pRoot, global.str_message);
  if(pChild)
    strcpy(pHttpInfo->message, pChild->valuestring);

  pChild = cJSON_GetObjectItem(pRoot, "host");
  if((pChild == 0) && global.keyFlag)
    pChild = cJSON_GetObjectItem(pRoot, global.str_host);
  if(pChild)
    strcpy(pHttpInfo->host, pChild->valuestring);

  pChild = cJSON_GetObjectItem(pRoot, "_source");
  if((pChild == 0) && global.keyFlag)
    pChild = cJSON_GetObjectItem(pRoot, global.str_source);
  if(pChild)
    strcpy(pHttpInfo->source, pChild->valuestring);

  pChild = cJSON_GetObjectItem(pRoot, "_DOMAIN");
  if((pChild == 0) && global.keyFlag)
    pChild = cJSON_GetObjectItem(pRoot, global.str_domain);
  if(pChild)
    strcpy(pHttpInfo->domain, pChild->valuestring);

  p = pHttpInfo->response_header;
  len = strlen(p);
  j = 0;
  for(i = 0; i < len; i++){
    if(p[i] == ' ')
      break;
    pHttpInfo->strVersion[j] = p[i];
    j++;
    pHttpInfo->strVersion[j] = 0;
    if((p[i] >= 'A') && (p[i] <= 'Z')){
      j = 0;
      pHttpInfo->strVersion[j] = 0;
    }
    if(p[i] == '/'){
      j = 0;
      pHttpInfo->strVersion[j] = 0;
    }
  }
  return 1;
}

int getIPFromStr(const char *str, u_int32_t *address)
{
  char buf[1024];
  u_int32_t a, b, c, d;

  strcpy(buf, str);
  if(sscanf(buf, "%d.%d.%d.%d", &a, &b, &c, &d) != 4){
    return -1;
  }
  if(a > 255)
    return -1;
  if(b > 255)
    return -1;
  if(c > 255)
    return -1;
  if(d > 255)
    return -1;
  *address = ((a & 0xff) << 24) + ((b & 0xff) << 16) + ((c & 0xff) << 8) + (d & 0xff);
  return 0;
}

void getStrIP(char *str, u_int32_t address)
{
  u_int32_t a, b, c, d;

  a = address / 256 / 256 / 256;
  b = address / 256 / 256 % 256;
  c = address / 256 % 256;
  d = address % 256;
  sprintf(str, "%u.%u.%u.%u", a, b, c, d);
}

int getTimeFromStr(const char *str, time_t *tt)
{
  int i, len, v, y, m, d, h, mi, s, split;
  char *p, buf[1024];
  time_t tcurr;
  struct tm stm;

  h = 0, mi = 0, s = 0;
  y = 0, m = 0, d = 0;
  strcpy(buf, str);
  len = strlen(buf);
  buf[len] = ':';
  len++;
  buf[len] = 0;
  p = buf;
  v = 0;
  for(i = 0; i < len; i++){
    split = 0;
    if(buf[i] == ':')
      split = 1;
    if(buf[i] == '-')
      split = 1;
    if(buf[i] == ' ')
      split = 1;
    if(split == 0)
      continue;
    v++;
    buf[i] = 0;
    if(v == 1)
      y = atoi(p);
    if(v == 2)
      m = atoi(p);
    if(v == 3)
      d = atoi(p);
    if(v == 4)
      h = atoi(p);
    if(v == 5)
      mi = atoi(p);
    if(v == 6)
      s = atoi(p);
    p = buf + i + 1;
  }
  if(v < 6)
    return -1;
  if(y < 1900)
    return -1;
  if((m <= 0) || (m > 12))
    return -1;
  if((d <= 0) || (d > 31))
    return -1;
  time(&tcurr);
  localtime_r(&tcurr, &stm);
  stm.tm_year = y - 1900;
  stm.tm_mon = m - 1;
  stm.tm_mday = d;
  stm.tm_hour = h;
  stm.tm_min = mi;
  stm.tm_sec = s;
  *tt = mktime(&stm);
  return 0;
}

cJSON* makeAlertJson(HttpInfoT *pHttpInfo)
{
  int type;
  cJSON *pRoot;
  char buf[1024];

  pRoot = cJSON_CreateObject();
  cJSON_AddStringToObject(pRoot, "rule_id", pHttpInfo->strID);
  type = pHttpInfo->type;
  if(type == TYPE_STIX_MINE){
    cJSON_AddStringToObject(pRoot, "alert_engine", "mine");
  }else{
    cJSON_AddStringToObject(pRoot, "alert_engine", "threat");
  }

  makeTimeString(pHttpInfo->stamp, buf);
  cJSON_AddStringToObject(pRoot, "time", buf);

  cJSON_AddStringToObject(pRoot, "message", pHttpInfo->message);
  cJSON_AddStringToObject(pRoot, "forward", pHttpInfo->strForward);

  sprintf(buf, "%d", pHttpInfo->ret_code);
  cJSON_AddStringToObject(pRoot, "retcode", buf);

  cJSON_AddStringToObject(pRoot, "source", pHttpInfo->source);
  cJSON_AddStringToObject(pRoot, "src_ip", pHttpInfo->strClient);
  cJSON_AddStringToObject(pRoot, "dst_ip", pHttpInfo->strServer);
  cJSON_AddStringToObject(pRoot, "geoip", "");

  sprintf(buf, "%d", pHttpInfo->sport);
  cJSON_AddStringToObject(pRoot, "src_port", buf);

  sprintf(buf, "%d", pHttpInfo->dport);
  cJSON_AddStringToObject(pRoot, "dst_port", buf);

  cJSON_AddStringToObject(pRoot, "host", pHttpInfo->host);
  cJSON_AddStringToObject(pRoot, "domain", pHttpInfo->domain);
  cJSON_AddStringToObject(pRoot, "url", pHttpInfo->url);
  cJSON_AddStringToObject(pRoot, "method", pHttpInfo->method);
  cJSON_AddStringToObject(pRoot, "msg", pHttpInfo->name);

  cJSON_AddStringToObject(pRoot, "severity", "1");
  if(global.reqFlag){
    cJSON_AddStringToObject(pRoot, "req_header", pHttpInfo->request_header);
    cJSON_AddStringToObject(pRoot, "req_body", pHttpInfo->request_body);
    cJSON_AddStringToObject(pRoot, "rsp_header", pHttpInfo->response_header);
    cJSON_AddStringToObject(pRoot, "rsp_body", pHttpInfo->response_body);
  }
  return pRoot;
}

void makeTimeString(time_t tt, char *buf)
{
  int year, month, day, hour, min, sec;
  struct tm stm;

  localtime_r(&tt, &stm);
  year = stm.tm_year + 1900;
  month = stm.tm_mon + 1;
  day = stm.tm_mday;
  hour = stm.tm_hour;
  min = stm.tm_min;
  sec = stm.tm_sec;
  sprintf(buf, "%d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, min, sec);
}

char* getMemInfoFromFile(char *file, int size)
{
  int iStart, n, totalLen;
  FILE *fp;
  char *p;

  fp = fopen(file, "r");
  if(fp == 0){
    printf("Open file %s failed\n", file);
    return 0;
  }
  p = (char*)malloc(size + 32);
  if(p == 0){
    printf("Not enough memory\n");
    return 0;
  }
  iStart = 0;
  totalLen = size;
  while(1){
    n = fread(p + iStart, 1, totalLen, fp);
    if(n <= 0)
      break;
    totalLen -= n;
    iStart += n;
  }
  fclose(fp);
  return p;
}

void getSelfProcessName(char *str)
{
  int i, len;
  char buf[256], *p;

  strcpy(buf, str);
  len = strlen(buf);
  global.pid = getpid();
  for(i = 0; i < len; i++){
    if(buf[i] == '.')
      buf[i] = ' ';
    if(buf[i] == '/'){
      buf[i] = ' ';
      p = buf + i + 1;
    }
  }
  if(p){
    trim(p);
    p[31] = 0;
    strcpy(global.progName, p);
    return;
  }
  trim(buf);
  buf[31] = 0;
  strcpy(global.progName, buf);
}
