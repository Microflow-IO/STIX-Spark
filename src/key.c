#include "util.h"

extern GlobalT global;

int readKeyFile(char *filename)
{
  int len;
  char *p, buf[1024];
  FILE *fp;

  fp = fopen(filename, "r");
  if(fp == 0)
    return -1;
  while(1){
    p = fgets(buf, 1024, fp);
    if(p == 0)
      break;
    len = strlen(buf);
    if(buf[len-1] == '\n'){
      buf[len-1] = 0;
      len--;
    }
    if(buf[len-1] == '\r'){
      buf[len-1] = 0;
      len--;
    }
    p = strstr(buf, ":");
    if(p == 0)
      continue;
    *p = 0;
    p++;
    trim(buf);
    toLowerCase(buf);
    trim(p);
    if(!strcmp(buf, "begin_time"))
      strcpy(global.str_begin_time, p);
    if(!strcmp(buf, "src_ip"))
      strcpy(global.str_src_ip, p);
    if(!strcmp(buf, "dst_ip"))
      strcpy(global.str_dst_ip, p);
    if(!strcmp(buf, "forward"))
      strcpy(global.str_forward, p);
    if(!strcmp(buf, "retcode"))
      strcpy(global.str_retcode, p);
    if(!strcmp(buf, "src_port"))
      strcpy(global.str_src_port, p);
    if(!strcmp(buf, "dst_port"))
      strcpy(global.str_dst_port, p);
    if(!strcmp(buf, "req_header"))
      strcpy(global.str_req_header, p);
    if(!strcmp(buf, "req_body"))
      strcpy(global.str_req_body, p);
    if(!strcmp(buf, "rsp_header"))
      strcpy(global.str_rsp_header, p);
    if(!strcmp(buf, "rsp_body"))
      strcpy(global.str_rsp_body, p);
    if(!strcmp(buf, "url"))
      strcpy(global.str_url, p);
    if(!strcmp(buf, "method"))
      strcpy(global.str_method, p);
    if(!strcmp(buf, "message"))
      strcpy(global.str_message, p);
    if(!strcmp(buf, "domain"))
      strcpy(global.str_domain, p);
    if(!strcmp(buf, "host"))
      strcpy(global.str_host, p);
    if(!strcmp(buf, "source"))
      strcpy(global.str_source, p);
  }
  fclose(fp);
  return 0;
}
