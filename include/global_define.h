#ifndef _GLOBAL_DEFINE_H_
#define _GLOBAL_DEFINE_H_

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <pcre.h>

#define  SEND_MODE_GELF                       1
#define  SEND_MODE_SYSLOG                     2

#define  TYPE_STIX                            1
#define  TYPE_STIX_MINE                       2

typedef struct {
  int id, iStart;
} IDNameT;

typedef struct {
  int port, type;
  u_int32_t address;
  unsigned long ulv;
  char name[128];
} PatternEndPointT;

typedef struct {
  int iStart, type;
  char name[128];
} PatternT;

typedef struct {
  int len, size;
  char *pStr;
} StringInfoT;

typedef struct {
  int cnt, size;
  IDNameT *pIDName;
} IDNameInfoT;

typedef struct {
  int cnt, size;
  PatternEndPointT *pPatternEndPoint;
} PatternEndPointInfoT;

typedef struct {
  int cnt, size;
  PatternT *pPattern;
} PatternInfoT;

typedef struct {
  int iStart;
  pcre *re;
  char pattern[256], name[256];
} RegexT;

typedef struct {
  int cnt, size;
  RegexT *pRegex;
} RegexInfoT;

typedef struct {
  u_int32_t src, dst, forward;
  int sport, dport, ret_code, type;
  time_t stamp;
  char strID[128], name[128], source[64], host[64];
  char method[64], strTime[64], strClient[64], strServer[64];
  char strForward[64], strVersion[64], domain[128];
  char message[512], request_header[512], response_header[512];
  char request_body[1024], response_body[1024], url[1024];
} HttpInfoT;

typedef struct {
  int runFlag, recvFD, listenPort, reqFlag, pid, sendFlag;
  int msgID, expPort, sendFD, tcpFlag, memFileFlag, keyFlag;
  struct sockaddr_in expAddress;
  char str_begin_time[32], str_src_ip[32], str_dst_ip[32];
  char str_forward[32], str_retcode[32], str_src_port[32];
  char str_dst_port[32], str_req_header[32], str_req_body[32];
  char str_rsp_header[32], str_rsp_body[32], str_url[32];
  char str_method[32], str_message[32], str_domain[32];
  char str_host[32], str_source[32], progName[128];
  char hostname[128], expDomain[128];
  IDNameInfoT idNameInfo;
  StringInfoT nameStringInfo, urlStringInfo, domainStringInfo;
  PatternEndPointInfoT endPointInfo, mineEndPointInfo;
  PatternInfoT patternUrlInfo, patternDomainInfo;
  RegexInfoT regexInfo;
} GlobalT;

#endif
