#include "stdio.h"
#include <stdlib.h>
#include "netinet/in.h"
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <unistd.h>   //close  
#include <arpa/inet.h>    //close  
#include <sys/types.h>  
#include <sys/socket.h> 
#include <limits.h> 
#include <netinet/in.h>  
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros 
#define rep(i,a,b) for(int i=a;i<b;i++)

typedef struct item    //We maintain 2 array of linked lists for buy_orders and sell_orders.The index of the array+1 indicates the item code 
{						//So in the node we store userid,quantity,price only
        int userid;
        int qty;
        int price;
        struct item *next;
}item;

typedef struct order  //After successful purchasing of an item the transaction details are stored in a linked list which buyer,seller_id,quantity 
{					//itemid,price
    int buyid;
    int sellid;
    int qty;
    int itemid;
    int price;
    struct order *next;
}order;

item *buy_items[10]; //buy_orders array of linked list
item *sell_items[10]; //sell_orders array of linked list
order *trades;        //linked list for storing the transaction details

void intialise()
{
	//In each linked list the first node is dummy entry,the actual entries are made from second place 
	//Therefore we initialise ecah linked list and insert dummy variables into the corr node
    rep(i,0,10)
    {
        buy_items[i]=(item*)malloc(sizeof(item));      
        sell_items[i]=(item*)malloc(sizeof(item));     
        buy_items[i]->userid=0;buy_items[i]->price=0;
        buy_items[i]->qty=0;sell_items[i]->qty=0;
        sell_items[i]->userid=0;sell_items[i]->price=0;
        buy_items[i]->next=NULL;sell_items[i]->next=NULL;
    }
    trades=(order*)malloc(sizeof(order));
    trades->buyid=0;trades->sellid=0;trades->qty=0;
    trades->price=0;trades->itemid=0;trades->next=NULL;
}

int searcher_ind(char buffer[])
{
	//While giving input from client to server we end the input using delimiter(;).So this func returns the index of delimiter in buffer which
	//denotes the client message has ended here
    rep(i,0,1024)
    {
        if(buffer[i]==';')return i;
    }
    return 1024;
}

int auth(char* logindetails)
{
	//The login details are stored in a file login.txt line by line.So we are given logindetails in format username:passwd we open file and
	//each line by comparing with logindetails and simultaneoulsy increment the index i for each mismatch.When match found retrn the index i

    FILE *fp=fopen("login.txt","r");
    char *dataToBeRead; dataToBeRead = (char*) malloc(64*sizeof(char));
    int i=0;int f=0;
    if ( fp != NULL ) 
    {  
    
    while(fscanf(fp,"%s\n",dataToBeRead) != EOF) 
    { i++;
       if(strcmp(logindetails,dataToBeRead)==0)
            {f=1;break;}
    } 
      
    fclose(fp);
    }
    else
    {
        printf("Login File not exitsing\n");
    }
    if(f) return i;
    return 0;
}

void add(int itemcode,int qty,int price,int sellid,int buyid)
{
	//for successful transaction between a buyer and seller we store the transaction details in trades linked list
	//we go to the end of linked list and add the new node which contain the transaction details at the end
    order *temp=trades;
    while(temp->next)
        temp=temp->next;
    order *p;p=(order*)malloc(sizeof(order));
    p->itemid=itemcode;p->qty=qty;p->sellid=sellid;p->price=price;
    p->buyid=buyid;p->next=NULL;temp->next=p;
}


void buy(char *temp,int userid)
{
	//The temp sent into this function has format itemcode:qty:price so we use strtok to retrieve each element 
    char *t=strtok(temp,":");
    int itemcode=atoi(t)-1;
    t=strtok(NULL,":");
    int qty=atoi(t);
    t=strtok(NULL,":");
    int price=atoi(t);
    item * head=sell_items[itemcode]->next; 

    //when a buyer keeps order of an item we first check in the sell_orders linked list of the item for a seller who sells the item for a price 
     //less than or equal to the price for which the buyer wants to buy

    if(head==NULL)							
    {										
        item *t=buy_items[itemcode]->next;						//if the head is NULL=>no seller is available,Now insert this order into buy_items[itemcode] linked list
        item *p;p=(item*)malloc(sizeof(item));p->userid=userid;
        p->qty=qty;p->price=price;p->next=NULL;
        if(t==NULL)
        {
            buy_items[itemcode]->next=p;
        }
        else{
            while(t->next!=NULL)
                t=t->next;
            t->next=p;
        }
    }
    else
    {						
        while(1)
        {
            item *min=NULL;int m=INT_MAX;item *prev=sell_items[itemcode],*prevm=sell_items[itemcode];

        while(head!=NULL)
        {
            if(head->price<=price && head->price<m )		//else check for seller who is selling at the minimum price
            {												// and also less the price at which the buyer wishes to buy
                m=head->price;min=head;prevm=prev;
            }
            prev=head;head=head->next;
        }
        if(min==NULL)										//if min=NULL=>no such seller is available so insert the order into buy_items
        {
            item *t=buy_items[itemcode]->next;
            item *p;p=(item*)malloc(sizeof(item));
            p->userid=userid;
            p->qty=qty;p->price=price;p->next=NULL;
            if(t==NULL)
            {
                buy_items[itemcode]->next=p;
            }
            else{
                while(t->next!=NULL)
                    t=t->next;
                t->next=p;
            }

            break;
        }
        else                                               //if seller is found and has qty >quantity the buyer wishes to buy 
        												//subtract from the sellers item quantity ,the quantity the user wishes to buy
        {												//and the transaction details into trades linked list
            if(min->qty>qty)
            {
                min->qty-=qty;
                add(itemcode,qty,min->price,min->userid,userid);
                break;
            }
            else
            {										//if seller is found and has qty =quantity the buyer wishes to buy add the transaction details
            										//into trades and the min node should be removed from sell_items[item] linked list because
                if(min->qty==qty)					// all the items seller wants to send are sold
                {
                    prevm->next=min->next;
                    add(itemcode,qty,min->price,min->userid,userid);
                    break;
                }
                add(itemcode,min->qty,min->price,min->userid,userid);	//if seller is found and has qty < quantity the buyer wishes to buy the qty available with seller  
                qty=qty-min->qty;										//is kept as qty in transaction details and from buyers required qty this is subtracted 
                //printf("%d  ",qty);									//and min is removed from linked list and this process continues until all the quantity buyer
                prevm->next=min->next;									//wants is satisfied or a case when his request cannot be processed further
                head=sell_items[itemcode]->next;
            }
        }
    }
}

}


void sell(char *temp,int userid)
{
	//The temp sent into this function has format itemcode:qty:price so we use strtok to retrieve each element 
    char *t=strtok(temp,":");
    int itemcode=atoi(t)-1;
    t=strtok(NULL,":");
    int qty=atoi(t);
    t=strtok(NULL,":");
    int price=atoi(t);
    item * head=buy_items[itemcode]->next;
     //when a seller keeps order of an item we first check in the buy_orders linked list of the item for a buyer who wants to buy the item for a price 
     //greater than or equal to the price for which the seller wants to sell
    if(head==NULL)
    {										//if the head is NULL=>no buyer for the item  is available,Now insert this order into sell_items[itemcode] linked list
        item *t=sell_items[itemcode]->next;
        item *p;p=(item*)malloc(sizeof(item));p->userid=userid;
        p->qty=qty;p->price=price;p->next=NULL;
        if(t==NULL)
        {
            sell_items[itemcode]->next=p;
        }
        else{
            while(t->next!=NULL)
                t=t->next;
            t->next=p;
        }
    }
    else
    {
    
        while(1)												//else check for buyer who wanna buy at the maximum price amon all such buyers
        {														// and also greater than or equal to the price at which the seller wishes to sell
            item *min=NULL;int m=0;item *prev=buy_items[itemcode],*prevm=buy_items[itemcode];
        while(head!=NULL)
        {
            if(head->price>=price && head->price>=m )
            {
                m=head->price;min=head;prevm=prev;
            }
            prev=head;head=head->next;
        }
        if(min==NULL)							//if min=NULL=>no such buyer is available so insert the order into sell_items
        {
            item *t=sell_items[itemcode]->next;
            item *p;p=(item*)malloc(sizeof(item));
            p->userid=userid;
            p->qty=qty;p->price=price;p->next=NULL;
            if(t==NULL)
            {
                sell_items[itemcode]->next=p;
            }
            else{
                while(t->next!=NULL)
                    t=t->next;
                t->next=p;
            }
            break;
        }
        else 
        {												   //if buyer is found and has qty >quantity the seller wishes to sell 
        												//subtract from the buyers item quantity ,the quantity the seller wishes to sell
        											//and the transaction details into trades linked list
            if(min->qty>qty)
            {
                min->qty-=qty;
                add(itemcode,qty,min->price,userid,min->userid);
                break;
            }
            else
            {										//if buyer is found and has qty =quantity the seller wishes to sell add the transaction details
            										//into trades and the min node should be removed from buy_items[item] linked list because
            										// the quantity the buyer wants to buy are bought.
                if(min->qty==qty)
                {
                    prevm->next=min->next; add(itemcode,qty,min->price,userid,min->userid);
                    break;
                }
                //if seller is found and has qty < quantity the buyer wishes to buy the qty available with seller  
              	//is kept as qty in transaction details and from buyers required qty this is subtracted 
               	//and min is removed from linked list and this process continues until all the quantity buyer
               	//wants is satisfied or a case when his request cannot be processed further

                add(itemcode,min->qty,min->price,userid,min->userid);
                qty=qty-min->qty;
                prevm->next=min->next;
                head=buy_items[itemcode]->next;
            }
        }
    }
}

}

//This function returns for each item the maximum price at which a buyer wants to buy and minimum price at which a seller wanna sell which are curenntly in the order state 
void orders(int sd)
{
    char buffer[1024];bzero(buffer,1024);
    strcat(buffer,"BUY\n");
    item *head,*temp;
    int min;
    rep(i,0,10)
    {
        head=buy_items[i]->next;			//for each items go into its buy_items and check for the maximum price
        temp=NULL;min=-1;
        char buff[64];bzero(buff,64);
        while(head!=NULL)
        {
            if(head->price>min)
            {
                temp=head;min=head->price;
            }
            head=head->next;
        }
        if(temp==NULL)
        {
            sprintf(buff,"ITEM %d:price: %d qty: %d\n",i+1,-1,0);	//temp=NULL =>no one is currently buying the product 
        }
        else
        {
            sprintf(buff,"ITEM %d:price: %d qty: %d\n",i+1,temp->price,temp->qty); 
        }
        strcat(buffer,buff);
    }
    strcat(buffer,"SELL\n");
    rep(i,0,10)									//for each items go into its sell_items and check for the minimum price
    {
        head=sell_items[i]->next;
        temp=NULL;min=INT_MAX;
        char buff[64];bzero(buff,64);
        while(head!=NULL)					
        {
            if(head->price<min)
            {
                temp=head;min=head->price;
            }
            head=head->next;
        }
        if(temp==NULL)							//temp=NULL =>no one is currently selling  the product 
        {
            sprintf(buff,"ITEM %d:price: %d qty: %d\n",i+1,-1,0);
        }
        else  									//else store the item:price:qty in buffer
        {
            sprintf(buff,"ITEM %d:price: %d qty: %d\n",i+1,temp->price,temp->qty);
        }

        strcat(buffer,buff);

    }

    int n=write(sd,buffer,sizeof(buffer));// write all this buffer to the client sd
    
}

//This function returns all the succesfull transactions of a user(the cases in which he is seller and in which he is a buyer)
void trade(int sd,int userid)
{
    order *head=trades->next;
    char buffer[1024];bzero(buffer,1024);
    char buff[64],buff1[512];bzero(buff,64);bzero(buff1,512);	
    strcat(buffer,"BUY ORDERS\n");
    strcat(buff1,"SELL ORDERS\n");
    while(head!=NULL)
    {
        if(head->buyid==userid)						//in trades linked list,if buyer_id=userid store the info in buffer
        {
            sprintf(buff,"Seller: %d Qty: %d Price :%d\n",head->sellid,head->qty,head->price);
            strcat(buffer,buff);
            bzero(buff,64);
        }
        if(head->sellid==userid)					//in trades linked list,if seller_id=userid store the info in buff1
        {
            sprintf(buff,"Buyer: %d Qty: %d Price :%d\n",head->buyid,head->qty,head->price);
            strcat(buff1,buff);
            bzero(buff,64);
        }
        head=head->next;
    }
    strcat(buffer,buff1);					//concatenate both buffer and buff1;
    int n=write(sd,buffer,sizeof(buffer));		
}

int main(int argc,char* argv[])
{
    int new_socket,listening_socket,client_socket[5]={0};int user[5]={0};//we are allowing only 5 users to login ata time user[5] stores the userid when a client is logged in
    int input_read; 
    int max_sd,no_of_clients=0,activity,sd,opt=1;
    struct sockaddr_in server_add,client_add;
    fd_set clients;
    char buffer[1025];
    bzero(buffer,1025);

    if(argc<2 || argc>2)			//The command line arguements sent =2 (./server portnumber)
    {
        printf("Incorrect number of arguments\n");
        return 0;
    }
    int port=atoi(argv[1]); //getting port number which is 2nd arguement
    
    listening_socket = socket(AF_INET, SOCK_STREAM, 0);//creation of socket
    if (listening_socket  == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    }
    intialise(); //intialising buy_items,sell_items,trades linked lists

    //master socket is bein set for allowing multiple connections ,  
    if( setsockopt(listening_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,  sizeof(opt)) < 0 )   
    {   
    perror("setsockopt");   
    exit(EXIT_FAILURE);   
    } 
    server_add.sin_family = AF_INET; //setting family,port,addr in the server_addr structure defined earlier 
    server_add.sin_port = htons(port);
    server_add.sin_addr.s_addr=INADDR_ANY;
    if (bind(listening_socket, (struct sockaddr*)&server_add, sizeof(server_add))<0) //binding the socket(listening socket) with server address
    { 
        perror("binding failed"); 
        exit(EXIT_FAILURE); 
    } 
    // if listen is successful 0 returned else error maybe some other socket using the port
    if (listen(listening_socket, 3) < 0) 
    { 
        perror("listening failed"); 
        exit(EXIT_FAILURE); 
    } 
    char *message = "Connection established"; 
    int addrlen=sizeof(server_add);
    while(1)   
    {   
        //Each time in this loop socket set defined by clients is cleared 
        FD_ZERO(&clients);   
     
        //master socket added to sockket set clients  
        FD_SET(listening_socket, &clients);   
        max_sd = listening_socket;   //max_sd denotes the highest file descriptor ,we should update this each time a socket is added  to socket list 
    								//which is used in the select function         
        //add child sockets to set  
        rep(i,0,5)  
        {   
           //if valid socket descriptor then add socket list  
            if(client_socket[i] > 0)   
                FD_SET( client_socket[i] , &clients);   
                 
        
            if(client_socket[i] > max_sd)   
                max_sd = client_socket[i];;   
        }   
     
        //wait for an activity on one of the sockets , timeout is NULL ,  
        //so wait indefinitely  
        activity = select( max_sd + 1 , &clients , NULL , NULL , NULL);   
       
        if ((activity < 0) && (errno!=EINTR))   
        {   
            printf("select error");   
        }   
             
        //If something happened on the master socket ,  
        //then its an incoming connection  
        if (FD_ISSET(listening_socket, &clients))   
        {   
            if ((new_socket = accept(listening_socket, (struct sockaddr *)&client_add, (socklen_t*)&addrlen))<0)   
            {   
                perror("accept failed");   
                exit(EXIT_FAILURE);   
            }   

            if(no_of_clients==5)	//we are allowing only 5 connections at a time so if this is reached and another user wants to connection send msg connections exceeded and close the socket
            {
                char *msg="Connections Exceeded"; 
                 int n=write(new_socket, msg, strlen(msg)); 
                 close(new_socket);
            }
            else{ 
                no_of_clients++;   //Else increment no of clients
                printf("New connection established\n");// , socket fd is %d , ip is : %s , port : %d \n" , new_socket , inet_ntoa(client_add.sin_addr) , ntohs(client_add.sin_port));   
                  if( send(new_socket, message, strlen(message), 0) != strlen(message) )   
            {   
                perror("sending failed");   
            }
                  
            }
            //add new socket to array of sockets  
            rep(i,0,5)   
            {   
                //if position is empty  
                if( client_socket[i] == 0 )   
                {   
                    client_socket[i] = new_socket;   
                    printf("Adding to list of sockets as %d\n" , i);   
                         
                    break;   
                }   
            }   
        }   
   
        //else its some I/O operation on some other socket 
        rep(i,0,5)  
        {   
            sd = client_socket[i];   
                
            if (FD_ISSET( sd , &clients))   
            {    
               //if input_read==0 indicated client has sent 0 bytes of dates which means he wants to terminate the connection
                if ((input_read = read( sd , buffer, 1024)) == 0)   
                {   
                    //terminate the connection(close the socket)and print the details
                    getpeername(sd , (struct sockaddr*)&client_add,(socklen_t*)&addrlen);   
                    printf("Host disconnected , ip %s , port %d,client no:%d ,user  id :%d\n" ,inet_ntoa(client_add.sin_addr) , ntohs(client_add.sin_port),i,user[i]);  
                    close( sd );   
                    client_socket[i] = 0;user[i]=0;
                    no_of_clients--; //connection closed so decrease the number of clients
                   
                }   
                      
                else 
                {   
                    char *in=(char*) malloc(1025*sizeof(char));
                    strcpy(in,buffer);
                    bzero(buffer,1025);
                    int size=searcher_ind(in); //searching for the end of input delimiter(;)
                    if(size!=1024)in[size]='\0';    
                    printf("Client No %d User Id %d message Received:%s\n",i,user[i],in);
                    char* temp=strtok(in,"#");//Number before # indicates the function client wanna use
                    char t=(*temp);
                    switch(t)
                    {
                        case '1': { temp=strtok(NULL,"#");       //t=="1"=>LOGIN
                                  int index=auth(temp);			//checking for user authentication 
                                  if(index==0)					//0 indicates invalid authentication
                                  {
                                      char *msg="Invalid Username or Password";
                                    write(sd , msg , strlen(msg)  );                                    
                                  }
                                  else
                                  {								//checking if already logged in
                                      int f=0;
                                      rep(j,0,5)
                                      {
                                          if(user[j]==index)
                                          {
                                              f=1;break;
                                          }
                                      }
                                      if(!f)				//f=0=>user not logged in
                                      {user[i]=index;
                                        char msg[1024];bzero(msg,1024);
                                        sprintf(msg,"User ID:%d Logged In",index);
                                        write(sd , msg , strlen(msg) );
                                      } 
                                      else
                                      {
                                        char *msg="Already Logged In"; //f==1 => userdid already present in user print meassge to user that already logged in and close the socket
                                        write(sd , msg , strlen(msg) );
                                        getpeername(sd , (struct sockaddr*)&client_add,(socklen_t*)&addrlen);   
                                        printf("Host disconnected , ip %s , port %d,client no:%d \n" ,inet_ntoa(client_add.sin_addr) , ntohs(client_add.sin_port),i);  
                                        close(sd);  
                                        client_socket[i] = 0;
                                        no_of_clients--; 
                                      }
                                  }
                                  break;
                                }
                        case '2':{
                                    temp=strtok(NULL,"#");  //2=>BUY_order
                                    buy(temp,user[i]); 
                                    char *msg="Message Received\n";
                                    write(sd , msg , strlen(msg) );     
                                    break;
                                }
                        case '3':{
                                temp=strtok(NULL,"#");		//3=>SELL_ORDER
                                sell(temp,user[i]);
                                char *msg="Message Received\n";
                                write(sd , msg , strlen(msg) );
                                break;
                                 }
                        case '4':{
                                    orders(sd);            //4=>ORDER_STATUS
                                    break;
                                }
                        case '5':{
                                    trade(sd,user[i]);		//5=>TRADE_STATUS
                                    break;
                                 }
                    }
                    
                } 
                
            }   
        }
           
    }   
    return 0;
}
