#include <getopt.h>
#include <signal.h>
#include "util.h"
#include "key.h"
#include "stix.h"
#include "socket.h"
#include "cJSON.h"

GlobalT global;

void terminateProcess(int signo);
void daemonize();
void printUsage();
void runMainLoop();

int main(int argc, char *argv[])
{
  int dFlag, opt, dport, stixFlag, memFlag;
  char *p, keyFile[128], stixFile[256], stixMineFile[256], memFile[256], buf[1024];

  dFlag = 1;
  if(initGlobal())
    return -1;
  dport = 0;
  stixFile[0] = 0;
  stixFlag = 0;
  memFlag = 0;

  while ((opt = getopt(argc, argv, "ahrtf:l:m:s:p:o:k:u:")) != EOF) {
    switch(opt){
      case 'a':
        global.reqFlag = 1;
        break;
      case 'h':
        printUsage();
        return 0;
      case 't':
        global.tcpFlag = 1;
        break;
      case 'r':
        dFlag = 0;
        break;
      case 'l':
        global.listenPort = atoi(optarg);
        break;
      case 'f':
        strcpy(stixFile, optarg);
        stixFlag = 1;
        break;
      case 'o':
        strcpy(stixMineFile, optarg);
        stixFlag = 1;
        break;
      case 'm':
        strcpy(memFile, optarg);
        memFlag = 1;
        break;
      case 'p':
        strcpy(global.hostname, optarg);
        break;
      case 'k':
        strcpy(keyFile, optarg);
        global.keyFlag = 1;
        break;
      case 's':
        strcpy(buf, optarg);
        p = strstr(buf, ":");
        if(p){
          *p = 0;
          p++;
          dport = atoi(p);
        }
        strcpy(global.expDomain, buf);
        global.expPort = dport;
        break;
      case 'u':
        strcpy(buf, optarg);
        toLowerCase(buf);
        if(!strcmp(buf, "gelf"))
          global.sendFlag = SEND_MODE_GELF;
        if(!strcmp(buf, "syslog"))
          global.sendFlag = SEND_MODE_SYSLOG;
        break;
    }
  }
  if(global.sendFlag == 0)
    global.sendFlag = SEND_MODE_GELF;
  getSelfProcessName(argv[0]);
  if(stixFlag && memFlag){
    printf("Stix file and Memory file cannot be all specified!\n");
    return 0;
  }
  if(global.listenPort == 0){
    printf("Please input listening port!\n");
    return 0;
  }
  if(global.expDomain[0] == 0){
    printf("Please input export address!\n");
    return 0;
  }
  if(global.expPort == 0){
    printf("Please input export port!\n");
    return 0;
  }
  if((stixFile[0] == 0) && (memFile[0] == 0)){
    printf("No stix file.\n");
    return 0;
  }
  if(global.keyFlag && readKeyFile(keyFile)){
    printf("Read key file %s failed!\n", keyFile);
    return 0;
  }

  if(dFlag)
    daemonize();
  global.runFlag = 1;
  if(stixFlag && initStixEnviorment(stixFile, stixMineFile))
    return -1;
  if(memFlag && initStixEnviormentByMemFile(memFile))
    return -1;
  initRegex();
  if(initSocket()){
    printf("Init socket failed!\n");
    return 0;
  }
  signal(SIGINT, terminateProcess);
  signal(SIGTERM, terminateProcess);

  if(global.tcpFlag)
    runTcpMainLoop();
  else
    runMainLoop();
  return 0;
}

void printUsage()
{
  printf("    -h                          Print this help.\n");
  printf("    -l  <listening port>        Listening port, the service process receives data on this port.\n");
  printf("    -f  <stix public file>      Public stix file.\n");
  printf("    -o  <stix mine file>        Mine stix file.\n");
  printf("    -m  <memory file>           Stix memory file.\n");
  printf("    -r                          Run in front mode.\n");
  printf("    -t                          Receiving data using TCP.\n");
  printf("    -s  <export json address>   The alert JSON data will be sent to this address in real time.\n");
  printf("    -p  <ip address>            Host IP address, output to JSON string.\n");
  printf("    -k  <key file>              The string corresponding to the key in JSON.\n");
  printf("    -u  <send mode>             Method of sending data.\n");
}

void terminateProcess(int signo)
{
  global.runFlag = 0;
  sleep(1);
  exit(1);
}

void daemonize()
{
  int i;
  pid_t pid;

  signal(SIGHUP ,SIG_IGN);
  pid = fork();
  if (pid < 0) {
    printf("fork error!\n");
    exit(-1);
  }
  if (pid > 0)
    exit(-1);
  pid = setsid();
  umask(0);
  if (pid == -1) {
    printf("setsid error!\n");
    exit(-1);
  }
  pid = fork();
  if (pid)
    exit(-1);
  chdir("/tmp");
  for(i = 0; i < 3; i++)
    close (i);
  open("/dev/null", O_RDONLY);
  open("/dev/null", O_WRONLY);
  open("/dev/null", O_WRONLY);
  return;
}

void runMainLoop()
{
  int v, n, len;
  time_t tt;
  cJSON *pRoot;
  socklen_t flen;
  struct sockaddr cliaddr;
  HttpInfoT httpInfo;
  struct tm stm;
  char *p, *p2, timebuf[256], tmp[65536], buf[65536], str[65536];

  while(global.runFlag){
    len = recvfrom(global.recvFD, buf, 65536, 0, &cliaddr, &flen);
    buf[len] = 0;
    p = 0;
    pRoot = 0;
    if(isZIP((u_char*)buf)){
      v = unzip(buf, len, str, 65536);
      if(v >= 0)
        p = str;
    }else{
      v = checkText((u_char*)buf);
      if(v)
        p = buf;
    }
    if(p){
      pRoot = cJSON_Parse(p);
    }
    if(pRoot == 0)
      continue;
    memset(&httpInfo, 0x00, sizeof(HttpInfoT));
    v = makeHttpInfo(&httpInfo, pRoot);
    if(v == 0){
      v = makeGelfHttpInfo(&httpInfo, pRoot);
      if(v == 0){
        cJSON_Delete(pRoot);
        continue;
      }
    }
    v = processHttpInfo(&httpInfo);
    cJSON_Delete(pRoot);
    if(v){
      pRoot = makeAlertJson(&httpInfo);
      p = cJSON_PrintUnformatted(pRoot);
      p2 = p;
      len = strlen(p);
      if(global.sendFlag == SEND_MODE_SYSLOG){
        time(&tt);
        localtime_r(&tt, &stm);
        strftime(timebuf, 128, "%Y-%m-%dT%H:%M:%SZ", &stm);
        global.msgID++;
        sprintf(tmp, "<165>1 %s %s %s %d %d %s", timebuf, global.hostname, global.progName, global.pid, global.msgID, p);
        len = strlen(tmp);
        p2 = tmp;
      }
      v = sendto(global.sendFD, (char*)p2, len, 0, (struct sockaddr *)&global.expAddress, sizeof(struct sockaddr));
      if(v < 0){
        n = 3;
        while(n > 0){
          v = sendto(global.sendFD, (char*)p2, len, 0, (struct sockaddr *)&global.expAddress, sizeof(struct sockaddr));
          n--;
          if(v > 0)
            break;
        }
      }
      free(p);
      cJSON_Delete(pRoot);
    }
  }
  return;
}
