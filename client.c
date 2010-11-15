#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

char* server_ip;
int prio;
char* command;
char* arg1;
int arg2;

void* control (void* args);
void marshall (int sockfd);
int main(int argc, char** argv)
{
  if (argc != 6) {
    printf("Usage: client <server_ip> <prio> <command> <arg1> <arg2>\n");
    exit(1);
  }
  server_ip = argv[1];
  prio = atoi(argv[2]);
  command = argv[3];
  arg1 = argv[4];
  arg2 = atoi(argv[5]);
  control(NULL);
}
void* control (void* args)
{
  int sockfd,connection,i;
  char buf[1000];
  char ret[1000];
  struct sockaddr_in clientaddr;
  sockfd = socket(PF_INET,SOCK_STREAM,0);
  clientaddr.sin_family = AF_INET;
  clientaddr.sin_port = htons(5000);
  clientaddr.sin_addr.s_addr = inet_addr(server_ip);
  connection = connect(sockfd,(struct sockaddr *)&clientaddr,sizeof(clientaddr));
  marshall(sockfd);
}

void marshall(int sockfd)
{
  char* packet = malloc(2000);
  char* host = malloc(1000);
  char* ret = malloc(20000);
  int i;
  gethostname(host,1000);
  sprintf(packet,"%d%s:%d:%s:%s:%d",10,host,prio,command,arg1,arg2);
  send(sockfd,packet,strlen(packet),0);
  while(recv(sockfd,ret,19999,0) != 0)
    {
      ret[19999] = '\0';
      printf("%s",ret);
    }
}

