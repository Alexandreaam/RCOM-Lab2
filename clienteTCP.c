#define SIZE 512
#include <stdio.h>
#include "clienteTCP.h"
#define SERVER_PORT 21

//Establish a new ftp connection
int connect_ftp(char * url,int porta)
{
	int	socketfd;//socket
	struct	sockaddr_in server_addr;//estrutura a preencher com os dados da host
	
	/*server address handling*/
	bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(getHostIp(url));	/*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(porta);		/*server TCP port must be network byte ordered */
    
	/*open an TCP socket*/
	if ((socketfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
    		perror("socket()");
        	exit(0);
    }
	/*connect to the server*/
    if(connect(socketfd, (struct sockaddr *)&server_addr,  sizeof(server_addr)) < 0){
        perror("connect()");
		exit(0);
	}
	return socketfd;
}

//Gets the IP from the URL
char* getHostIp(char * url)
{
	struct hostent *h;
    if ((h=gethostbyname(url)) == NULL) {  
        herror("gethostbyname");
        exit(1);
    }
    printf("Host name  : %s\n", h->h_name);
    printf("IP Address : %s\n",inet_ntoa(*((struct in_addr *)h->h_addr)));
    return inet_ntoa(*((struct in_addr *)h->h_addr));
}

//Verifies if the code is correct or not
int verifyCode(int code)
{
	if(code==220 || code==331 || code==230 ||code==227 ||code==150 ||code==220 ||code==226 ||code==221){
		printf("OK\n");
		return 1;
	}else {
		printf("Error\n");
		exit( -1);
	}
}

void printAnswer(int fd)
{
	char rbuffer[SIZE];
	int res=0;
	while(1){
		res=read(fd, rbuffer, SIZE);
		rbuffer[res]='\0';
		printf("%s",rbuffer);
		if(endReached(rbuffer)==1)
			break;
	}
	printf("\n");
}

void sendCommand(int port,char * command)
{
	int res;
	res=write(port,command,strlen(command));
	if(res<strlen(command))
		printf("Error Sending Command:");
	printf("#>>%s",command);
}


int saveFile(int socket_data,char * path,int socketfd){


	//Gets file name from path
	char * name;
	int i;
	for(i=strlen(path);i>=0;i--){
		if(path[i]=='/'){
			break;
		}
	}
	name=path+(i+1); 

	FILE * fp=fopen(name,"w+");//Create new file
	char c;	
	int error=0;
	while(1){
		error=read(socket_data,&c,1);//Reading a char at a time
		
		if(error==0){
			break;}
		fprintf(fp, "%c", c);
		
	}
	fclose(fp);
	close(socket_data); //Close Data connection after the file was received
	return 0;
}

int getParameter(char limit,char * argv,char * res,int i)
{
	int j=0;
	while(1){
		if(argv[i]==limit)
			break;
		res[j]=argv[i];
		i++;
		j++;
	}
	i++;
	res[j]='\0';
	j=0;
	
	return i;
}

//Computes new port
int computePortNumber(int socketfd)
{
	char rbuffer[SIZE];
	int code,res=0,p2,tmp,p1;
	res=read(socketfd, rbuffer, SIZE);
	rbuffer[res]='\0';
	printf("%s\n",rbuffer);
	
	sscanf(rbuffer,"%d Entering Passive Mode (%d,%d,%d,%d,%d,%d)",&code,&tmp,&tmp,&tmp,&tmp,&p1,&p2);
	verifyCode(code);//Verifies if an error has occured
	if(res<=0)
		return -1;
	return p1*256+p2;//port number
}

//Verifies if it's the last package from the answer to a command
int endReached(char * rbuffer){
	int code,i=0;
	char *p = rbuffer;
    while (*p) {// procurar o code, pois pode vir no meio de uma string, em caso de respostas com multiplas linhas
	    if (isdigit(*p)) {// se encontrar o 1º digito
		    code=strtol(p, &p, 10);//obter o code
		    if(rbuffer[i+3]==' '){//se depois dos 3 digitos tiver espaço, é a ultima linha da mensagem
			   	verifyCode(code);
			   	return 1;	
			}
		    break;
	    } else {
	 	   p++;
	 	   i++;
	    }
    }
	return 0;
}

int main(int argc, char** argv){
	char user[SIZE],password[SIZE],host[SIZE],path[SIZE],buffer[SIZE];
	int	socketfd, socket_data,nport;
	int i =6;   //para ignorar a 'ftp://'
	if(argc!=2){
		printf("usage: %s ftp://<user>:<password>@<host>/<url-path>\n",argv[0]); // RFC1738
		return -1;
	}

	i = getParameter(':',argv[1],user,i);
	i = getParameter('@',argv[1],password,i);
	i = getParameter('/',argv[1],host,i);
	i = getParameter('\0',argv[1],path,i);
	
	//Establish a Control Connection
	socketfd=connect_ftp(host,21);
	printAnswer(socketfd);

	//Send Username
	sprintf(buffer,"user %s\r\n",user);
	sendCommand(socketfd,buffer);
	printAnswer(socketfd);

	//Send Password
	sprintf(buffer,"pass %s\r\n",password);
	sendCommand(socketfd,buffer);
	printAnswer(socketfd);

	//Passive Mode
	sendCommand(socketfd,"pasv\r\n");
	nport=computePortNumber(socketfd); //we need to compute the new portnumber after we enter Passive Mode
	printf("port \t : %d\n",nport);

	//Establish a Data Connection
	socket_data=connect_ftp(host,nport);
	sprintf(buffer,"retr %s\r\n",path);
	sendCommand(socketfd,buffer);
	printAnswer(socketfd);

	//Save File
	saveFile(socket_data,path,socketfd);
	printAnswer(socketfd);
	
	//END
	sendCommand(socketfd,"quit\r\n");
	printAnswer(socketfd);
	close(socketfd);
	return 1;
}


