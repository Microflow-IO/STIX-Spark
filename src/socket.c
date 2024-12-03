#include "util.h"
#include "stix.h"

extern GlobalT global;

int initUdpSocket();
int initTcpSocket();
void processStr(const char *str);

int initSocket()
{
  int fd, n;
  socklen_t slen;

  if(global.tcpFlag)
    n = initTcpSocket();
  else
    n = initUdpSocket();
  if(n < 0)
    return -1;

  memset(&(global.expAddress), 0x00, sizeof(struct sockaddr_in));
  global.expAddress.sin_family = AF_INET;
  global.expAddress.sin_port = htons(global.expPort);
  global.expAddress.sin_addr.s_addr = inet_addr(global.expDomain);
  slen = sizeof(struct sockaddr_in);
  fd = socket(AF_INET, SOCK_DGRAM, 0);
  if(fd < 0){
    printf("socket failed!\n");
    close(global.recvFD);
    return -1;
  }
  if (connect(fd, (struct sockaddr*)&global.expAddress, slen) < 0){
    printf("connect export server %s:%d failed: %s\n", global.expDomain, global.expPort, strerror(errno));
    close(fd);
    close(global.recvFD);
    return -1;
  }
  global.sendFD = fd;
  return n;
}

int initUdpSocket()
{
  int fd;
  struct sockaddr_in inaddr;

  memset(&inaddr, 0x00, sizeof(struct sockaddr_in));
  inaddr.sin_family = AF_INET;
  inaddr.sin_port = htons(global.listenPort);
  inaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  fd = socket(AF_INET, SOCK_DGRAM, 0);
  if(fd < 0){
    printf("socket failed!\n");
    return -1;
  }
  if(bind(fd, (struct sockaddr*)&inaddr, sizeof(struct sockaddr_in)) < 0){
    printf("Bind failed!\n");
    return -1;
  }
  global.recvFD = fd;
  return 0;
}

int initTcpSocket()
{
  int fd;
  struct sockaddr_in inaddr;

  memset(&inaddr, 0x00, sizeof(struct sockaddr_in));
  inaddr.sin_family = AF_INET;
  inaddr.sin_port = htons(global.listenPort);
  inaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  fd = socket(AF_INET, SOCK_STREAM, 0);
  if(fd < 0){
    printf("socket failed!\n");
    return -1;
  }
  if(bind(fd, (struct sockaddr*)&inaddr, sizeof(struct sockaddr)) < 0){
    printf("open socket failed, %s\n", strerror(errno));
    return -1;
  }
  if(listen(fd, 5) < 0){
    printf("open socket failed, %s\n", strerror(errno));
    return -1;
  }
  global.recvFD = fd;
  return 0;
}

void runTcpMainLoop()
{
  int fd, n, iStart, len;
  socklen_t socklen;
  struct sockaddr_in clientAddr;
  char *p, buf[65536], str[65536];

  while(global.runFlag){
    socklen = sizeof(struct sockaddr);
    fd = accept(global.recvFD, (struct sockaddr*)&clientAddr, &socklen);
    if(fd < 0)
      continue;
    iStart = 0;
    n = 0;
    while(1){
      n = 60000 - iStart;
      len = recv(fd, buf+iStart, n, 0);
      if(len <= 0){
        close(fd);
        break;
      }
      len += iStart;
      buf[len] = 0;
      buf[len+1] = 0;
      buf[len+2] = 0;
      buf[len+3] = 0;
      p = buf;
      while(1){
        n = strlen(p);
        if(n <= 0){
          iStart = 0;
          break;
        }
        if(p[n-1] == '}'){
          processStr(p);
          p = p + n;
          if(*p == 0) p++;
          if(*p == 0) p++;
          if(*p == 0) p++;
          if((p-buf) >= len){
            iStart = 0;
            break;
          }
        }else{
          strcpy(str, p);
          strcpy(buf, str);
          iStart = strlen(buf);
          break;
        }
      }
    }
  }
  return;
}

void processStr(const char *str)
{
  int v, n, len;
  char *p;
  cJSON *pRoot;
  HttpInfoT httpInfo;

  pRoot = cJSON_Parse(str);
  if(pRoot == 0)
    return;
  memset(&httpInfo, 0x00, sizeof(HttpInfoT));
  v = makeHttpInfo(&httpInfo, pRoot);
  if(v == 0){
    v = makeGelfHttpInfo(&httpInfo, pRoot);
    if(v == 0){
      cJSON_Delete(pRoot);
      return;
    }
  }
  cJSON_Delete(pRoot);
  v = processHttpInfo(&httpInfo);
  if(v){
    pRoot = makeAlertJson(&httpInfo);
    p = cJSON_PrintUnformatted(pRoot);
    len = strlen(p);
    v = sendto(global.sendFD, (char*)p, len, 0, (struct sockaddr *)&global.expAddress, sizeof(struct sockaddr));
    if(v < 0){
      n = 3;
      while(n > 0){
        v = sendto(global.sendFD, (char*)p, len, 0, (struct sockaddr *)&global.expAddress, sizeof(struct sockaddr));
        n--;
        if(v > 0)
          break;
      }
    }
    free(p);
    cJSON_Delete(pRoot);
  }
  return;
}
