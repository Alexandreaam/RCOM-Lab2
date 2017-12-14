#include <stdio.h>
#include <stdlib.h> 
#include <ctype.h>
#include <errno.h> 
#include <netdb.h> 
#include <sys/types.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#define h_addr h_addr_list[0] //  The first address in h_addr_list. 

char* getHostIp(char * url);

int connect_ftp(char * url,int porta);

void printAnswer(int socket);

void sendCommand(int port,char * cmd);

int computePortNumber(int sockfd);

int verifyCode(int code);

int saveFile(int sock,char * path,int sockControl);

int getParam(char limit,char * argv,char * res,int i);

int endReached(char * rbuf);
