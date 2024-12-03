#include "util.h"
#include "stix.h"
#include "cJSON.h"

GlobalT global;
char suffix[64];

int writeResult();
int writeName();
int writeUrl();
int writeDomain();
int writeEndPoint();
int writeMineEndPoint();
int writeRegex();
void printUsage();

int main(int argc, char *argv[])
{
  int opt;
  time_t t1, t2;
  char fileMine[256], filePublic[256], buf[1024];

  if(argc == 1){
    printUsage();
    return 0;
  }
  while ((opt = getopt(argc, argv, "hm:p:")) != EOF) {
    switch(opt){
      case 'h':
        printUsage();
        return 0;
      case 'm':
        strcpy(fileMine, optarg);
        break;
      case 'p':
        strcpy(filePublic, optarg);
        break;
    }
  }
  strcpy(suffix, "stix-file");
  if(initGlobal())
    return -1;
  printf("Load data ......\n");
  time(&t1);
  if(initStixEnviorment(filePublic, fileMine))
    return -1;
  time(&t2);
  printf("Complete, %ld second.\n", t2 - t1);
  sprintf(buf, "mkdir %s", suffix);
  system(buf);
  if(writeResult())
    return 0;
  if(writeName())
    return 0;
  if(writeEndPoint())
    return 0;
  if(writeMineEndPoint())
    return 0;
  if(writeUrl())
    return 0;
  if(writeDomain())
    return 0;
  if(writeRegex())
    return 0;
  printf("Success\n");
  return 0;
}

void printUsage()
{
  printf("    -h                          Print this help.\n");
  printf("    -m  <mine file>             Mine stix file.\n");
  printf("    -p  <public file>           Public stix file.\n");
}

int writeResult()
{
  int v;
  FILE *fp;
  char buf[1024];

  sprintf(buf, "%s/result", suffix);
  fp = fopen(buf, "w+");
  if(fp == 0){
    printf("Open result file failed\n");
    return -1;
  }

  fprintf(fp, "url count : %d\n", global.patternUrlInfo.cnt);
  fprintf(fp, "domain count : %d\n", global.patternDomainInfo.cnt);
  fprintf(fp, "endpoint count : %d\n", global.endPointInfo.cnt);
  fprintf(fp, "endpoint mine count : %d\n", global.mineEndPointInfo.cnt);
  fprintf(fp, "name count : %d\n", global.idNameInfo.cnt);
  fprintf(fp, "regex count : %d\n", global.regexInfo.cnt);

  v = global.urlStringInfo.len + 16 - (global.urlStringInfo.len % 16);
  fprintf(fp, "url string length : %d\n", v);
  v = global.domainStringInfo.len + 16 - (global.domainStringInfo.len % 16);
  fprintf(fp, "domain string length : %d\n", v);
  v = global.nameStringInfo.len + 16 - (global.nameStringInfo.len % 16);
  fprintf(fp, "name string length : %d\n", v);

  fprintf(fp, "url info : url-info\n");
  fprintf(fp, "domain info : domain-info\n");
  fprintf(fp, "endpoint info : endpoint-info\n");
  fprintf(fp, "endpoint mine info : endpoint-mine-info\n");
  fprintf(fp, "name info : name-info\n");
  fprintf(fp, "regex info : regex-info\n");

  fprintf(fp, "url string info : url-string-info\n");
  fprintf(fp, "domain string info : domain-string-info\n");
  fprintf(fp, "name string info : name-string-info\n");
  fclose(fp);
  return 0;
}

int writeName()
{
  int i, n, length;
  FILE *fp;
  char *p, buf[1024];

  length = global.nameStringInfo.len + 16 - (global.nameStringInfo.len % 16);
  sprintf(buf, "%s/name-string-info", suffix);
  fp = fopen(buf, "w+");
  if(fp == 0){
    printf("Open file %s failed\n", buf);
    return -1;
  }
  i = 0;
  p = global.nameStringInfo.pStr;
  while(1){
    n = fwrite(p + i, 1, length, fp);
    if(n == length)
      break;
    if(n == 0){
      printf("Write %s failed\n", buf);
      return -1;
    }
    length -= n;
    i += n;
  }
  fclose(fp);

  length = (global.idNameInfo.cnt + 1) * sizeof(IDNameT);
  sprintf(buf, "%s/name-info", suffix);
  fp = fopen(buf, "w+");
  if(fp == 0){
    printf("Open file %s failed\n", buf);
    return -1;
  }
  i = 0;
  p = (char*)global.idNameInfo.pIDName;
  while(1){
    n = fwrite(p + i, 1, length, fp);
    if(n == length)
      break;
    if(n == 0){
      printf("Write %s failed\n", buf);
      return -1;
    }
    length -= n;
    i += n;
  }
  fclose(fp);
  return 0;
}

int writeUrl()
{
  int i, n, length;
  FILE *fp;
  char *p, buf[1024];

  length = global.urlStringInfo.len + 16 - (global.urlStringInfo.len % 16);
  sprintf(buf, "%s/url-string-info", suffix);
  fp = fopen(buf, "w+");
  if(fp == 0){
    printf("Open %s file failed\n", buf);
    return -1;
  }
  i = 0;
  p = global.urlStringInfo.pStr;
  while(1){
    n = fwrite(p + i, 1, length, fp);
    if(n == length)
      break;
    if(n == 0){
      printf("Write %s failed\n", buf);
      return -1;
    }
    length -= n;
    i += n;
  }
  fclose(fp);

  length = (global.patternUrlInfo.cnt + 1) * sizeof(PatternT);
  sprintf(buf, "%s/url-info", suffix);
  fp = fopen(buf, "w+");
  if(fp == 0){
    printf("Open file %s failed\n", buf);
    return -1;
  }
  i = 0;
  p = (char*)global.patternUrlInfo.pPattern;
  while(1){
    n = fwrite(p + i, 1, length, fp);
    if(n == length)
      break;
    if(n == 0){
      printf("Write %s failed\n", buf);
      return -1;
    }
    length -= n;
    i += n;
  }
  fclose(fp);
  return 0;
}

int writeDomain()
{
  int i, n, length;
  FILE *fp;
  char *p, buf[1024];

  length = global.domainStringInfo.len + 16 - (global.domainStringInfo.len % 16);
  sprintf(buf, "%s/domain-string-info", suffix);
  fp = fopen(buf, "w+");
  if(fp == 0){
    printf("Open %s file failed\n", buf);
    return -1;
  }
  i = 0;
  p = global.domainStringInfo.pStr;
  while(1){
    n = fwrite(p + i, 1, length, fp);
    if(n == length)
      break;
    if(n == 0){
      printf("Write %s failed\n", buf);
      return -1;
    }
    length -= n;
    i += n;
  }
  fclose(fp);

  length = (global.patternDomainInfo.cnt + 1) * sizeof(PatternT);
  sprintf(buf, "%s/domain-info", suffix);
  fp = fopen(buf, "w+");
  if(fp == 0){
    printf("Open file %s failed\n", buf);
    return -1;
  }
  i = 0;
  p = (char*)global.patternDomainInfo.pPattern;
  while(1){
    n = fwrite(p + i, 1, length, fp);
    if(n == length)
      break;
    if(n == 0){
      printf("Write %s failed\n", buf);
      return -1;
    }
    length -= n;
    i += n;
  }
  fclose(fp);
  return 0;
}

int writeEndPoint()
{
  int i, n, length;
  FILE *fp;
  char *p, buf[1024];

  length = (global.endPointInfo.cnt + 1) * sizeof(PatternEndPointT);
  sprintf(buf, "%s/endpoint-info", suffix);
  fp = fopen(buf, "w+");
  if(fp == 0){
    printf("Open %s file failed\n", buf);
    return -1;
  }
  i = 0;
  p = (char*)global.endPointInfo.pPatternEndPoint;
  while(1){
    n = fwrite(p + i, 1, length, fp);
    if(n == length)
      break;
    if(n == 0){
      printf("Write %s failed\n", buf);
      return -1;
    }
    length -= n;
    i += n;
  }
  fclose(fp);
  return 0;
}

int writeMineEndPoint()
{
  int i, n, length;
  FILE *fp;
  char *p, buf[1024];

  length = (global.mineEndPointInfo.cnt + 1) * sizeof(PatternEndPointT);
  sprintf(buf, "%s/endpoint-mine-info", suffix);
  fp = fopen(buf, "w+");
  if(fp == 0){
    printf("Open %s file failed\n", buf);
    return -1;
  }
  i = 0;
  p = (char*)global.mineEndPointInfo.pPatternEndPoint;
  while(1){
    n = fwrite(p + i, 1, length, fp);
    if(n == length)
      break;
    if(n == 0){
      printf("Write %s failed\n", buf);
      return -1;
    }
    length -= n;
    i += n;
  }
  fclose(fp);
  return 0;
}

int writeRegex()
{
  int i, n, length;
  FILE *fp;
  char *p, buf[1024];

  length = (global.regexInfo.cnt + 1) * sizeof(RegexT);
  sprintf(buf, "%s/regex-info", suffix);
  fp = fopen(buf, "w+");
  if(fp == 0){
    printf("Open %s file failed\n", buf);
    return -1;
  }
  i = 0;
  p = (char*)global.regexInfo.pRegex;
  while(1){
    n = fwrite(p + i, 1, length, fp);
    if(n == length)
      break;
    if(n == 0){
      printf("Write %s failed\n", buf);
      return -1;
    }
    length -= n;
    i += n;
  }
  fclose(fp);
  return 0;
}
