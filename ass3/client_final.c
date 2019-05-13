#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>

void login(int newsock);
void request(int i,int  newsock);


int main(int argc, char* argv[])
{
	
	if (argc < 3)//the number of command line arguements=3(./client ipaddress portno)
	{
		fprintf(stderr,"usage %s hostname port\n", argv[0]);
		exit(0);
	}

	int newsock;
	newsock=socket(AF_INET, SOCK_STREAM, 0);//creating a new socket
	if(newsock < 0)
	{
		fprintf(stderr,"ERROR opening socket\n");
		exit(0);
	}
	int portno = atoi(argv[2]);//retrieving port no from command line arguement

	struct hostent *server;
	struct sockaddr_in serveraddress;
	server=gethostbyname(argv[1]);  //getting server ip address
	if(server == NULL)
	{
		fprintf(stderr, "ERROR, no such host\n");
		exit(0);
	}
	bzero((char *)&serveraddress, sizeof(serveraddress));
	serveraddress.sin_family = AF_INET;		//adding family,port,ip_address to the sever _address previouly defined
	bcopy((char *) server->h_addr, (char *) &serveraddress.sin_addr.s_addr, server->h_length);
	serveraddress.sin_port= htons(portno);

	if(connect(newsock,(struct sockaddr *)&serveraddress,sizeof(serveraddress))<0) //attempting to connect to the server 
	{
		fprintf(stderr, "ERROR connecting\n");				//return value <0 =>there was an error in connecting to the server 
		exit(0);
	}

	login(newsock);		//on successfull newsock stores the socket_id used for communication 

	printf("To buy press 1\n");
	printf("To sell press 2\n");
	printf("To view order status press 3\n");
	printf("To view your trade status press 4\n");
	printf("To exit press ctlr C\n");
	int n = 0;
	while(1)
	{
		printf("Enter your option:");
		scanf("%d",&n);
		switch(n)
		{
			case 1: request(2,newsock);
					break;
			case 2: request(3,newsock);
					break;
			case 3: request(4,newsock);
					break;
			case 4: request(5,newsock);
					break;
			default:printf("enter valid option \n"); 
		}
	}
	return 0;
}

void login(int newsock)
{
	char buffer[1024],temp1[512],temp2[512];
	bzero(buffer,1025);
    int n = read(newsock,buffer,1025);
	if(n < 0)
		fprintf(stderr, "ERROR reading from socket\n"); 
	if(n==0)
	{
		printf("Server Closed\n");                //if n==0 bytes then it is an indication that server has closed so we close the client also
		exit(0);
	}	
	printf("%s\n",buffer);
	if(strcmp(buffer,"Connections Exceeded")==0)  //When no of clinets=max_clients server sends message Connections exceeded then we close the client program
		exit(0);
	bzero(buffer,1025);
	while(1)
	{
		printf("Enter username:");            
		bzero(temp1,512);
		bzero(temp2,512);
		bzero(buffer,1024);
		fgets(temp1,255,stdin);				// storing the username provided by user into temp1
		temp1[strlen(temp1)-1] = '\0';
		printf("Enter password:");
		fgets(temp2,255,stdin);             //storing password into temp2
		temp2[strlen(temp2)-1] = '\0';
		strcat(buffer,"1#");              	//in buffer initially we put 1# because this is login information server understands that this login information by seeing this 1
		strcat(buffer,temp1);				//next we put username:passwd into the buffer which the same format client login detils are stored in login.txt in the server side
		strcat(buffer,":");
		strcat(buffer,temp2);
		strcat(buffer,";\0");
		n=write(newsock,buffer,strlen(buffer));		//we write this into newsock we got after connecting
		bzero(buffer,1025);
		n = read(newsock,buffer,1025);
		if(n < 0)
			fprintf(stderr, "ERROR reading from socket\n"); 
		if(n==0)
		{
			printf("Server Closed\n");                //if n==0 bytes then it is an indication that server has closed so we close the client also
			exit(0);
		}
		printf("%s\n",buffer);
		
		int v=strcmp(buffer,"Already Logged In\0");  //if alreading logged in aslo we should close the client
		
		if(v==0)
		{
			exit(0);
		}
		int h=strcmp(buffer,"Invalid Username or Password\0"); //if invalid username or password we do not close the client we print try again and this goes in a loop
		if(h!=0)
		{
			break;
		}
		bzero(buffer,1025);
	}
}

void request(int i,int  newsock)
{
	if(i==2 || i==3)				//i=2=>BUY and i=3=>SELL
	{
		char buffer[1025];
		int n,code,qty,price;
		printf("Enter item code:");
		while(1)
		{
			scanf("%d",&code);
			if(code>=0 && code<=10)   //there are 10 items out of which the user should select if selected otherwise print error and ask to try again
			{
				break;
			}
			printf("Enter valid item code :");
		}
		printf("Enter item quantity:");//quntity of the item to be entered
		while(1)
		{
			scanf("%d",&qty);
			if(qty>0)
			{
				break;
			}
			printf("Enter valid item quantity :");
		}
		printf("Enter item price:");//price for which he wants to buys or sell should be printed
		while(1)
		{
			scanf("%d",&price);
			if(price>=0)
			{
				break;
			}
			printf("Enter valid price :");
		}
		sprintf(buffer,"%d#%d:%d:%d;",i,code,qty,price);//write to the socket in forma 2#itemcode:quantity:price which is format required to be processed by the server
		n=write(newsock,buffer,strlen(buffer));
		if(n < 0){
			fprintf(stderr, "Error in writing to socket\n");
			exit(0);
		}
		bzero(buffer,1024);
		n=read(newsock,buffer,1024);
		if(n<0)
		{
			fprintf(stderr, "Error in reading from socket\n");
			exit(0);
		}
		if(n==0)
		{
			printf("Server Closed\n");	//n==0=>sever is closed so the client program also should be closed
			exit(0);
		}
		printf("%s",buffer);
	}
	else
	{
		int n=0;            //4# => ORDER_STATUS AND 5# => TRADE_STATUS
		char buffer[1024];
		if(i==4)
			n=write(newsock,"4#",2);
		else
			n=write(newsock,"5#",2);
		if(n<0)
		{
			fprintf(stderr, "Error in writing to socket\n");
			exit(0);
		}
		bzero(buffer,1024);
		n=read(newsock,buffer,1024);
		if(n<0)
		{
			fprintf(stderr, "Error in reading from socket\n");
			exit(0);
		}
		if(n==0)
		{
			printf("Server Closed\n");//n==0=>sever is closed so the client program also should be closed
			exit(0);
		}
		printf("%s",buffer);
	}
}
