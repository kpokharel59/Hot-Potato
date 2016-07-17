#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h> 
 #include <sys/ioctl.h>
#include <errno.h>


int masterPortNumber;
int numberOfPlayers;
int hops;

struct Potato{
	int remainingHops;
	int ** playerList;
};

typedef struct Player{
	int id;
	int socketDescriptorOfPlayer;
	int peerPortNumber;
   char *playerHostName;
}playerStruct;


int getRandomPlayer(int n){
	int randm=rand();
	return randm%n;
}


void startGame(int sock,char * str){
	char buffer[512];
	bzero(buffer,512);
	strcpy(buffer,str);
	int n;
	
	//send to first randomly selected player
	if ((send(sock,buffer, strlen(buffer),0))== -1) {
       perror("Failure Sending Message\n");
       close(sock);
       exit(1);
   }
   // return;
   /// receive
  /* bzero(buffer,512);
   n = read(sock,buffer,511);
   if (n < 0) {
      perror("ERROR reading from socket");
      exit(1);
   }
   
   if ( !strcmp("close", buffer) )
       exit(1);
   else
	    printf("%s\n", buffer);*/
}

int main(int argc, char *argv[]){

	if(argc<4 || argc>4){
		printf("Syntax: master <port-number> <number-of-players> <hops> \n");
      exit(1);
	}
	//printf("Master\n");
	masterPortNumber=atoi(argv[1]);
	numberOfPlayers=atoi(argv[2]);
	hops=atoi(argv[3]);
	char host[64];
	static int playerIndex=0;
    playerStruct* playerList[numberOfPlayers];
	char buf[512];
	fd_set readfds,readfds1;
	int max_sd=-1;

		
	srand(5);
	
   //create master as server socket	
	int socketDescriptor,pid;
	
	/* use address family INET and STREAMing sockets (TCP) */
	// int socket (int family, int type, int protocol);
	socketDescriptor=socket(AF_INET,SOCK_STREAM,0);
	if(socketDescriptor<0){
		perror("socket:");
		exit(socketDescriptor);
	}
	
	struct hostent *hp,*ihp;
	  /* fill in hostent struct for self */
    gethostname(host, sizeof host);
    hp = gethostbyname(host);
    if ( hp == NULL ) {
      fprintf(stderr, "%s: host not found (%s)\n", argv[0], host);
      exit(1);
    }

    printf("Potato Master on %s\n", host);
	printf("Players = %d\n", numberOfPlayers);
	printf("Hops = %d\n", hops);
	
	struct sockaddr_in masterAddress,incomingPlayer;
	bzero((char*)&masterAddress,sizeof(masterAddress));
	masterAddress.sin_family=AF_INET;
	masterAddress.sin_port=htons(masterPortNumber);
  masterAddress.sin_addr.s_addr=htonl(INADDR_ANY);
    memcpy(&masterAddress.sin_addr, hp->h_addr_list[0], hp->h_length);	
	
	/* bind socket socketDescriptor to address sin */
	//int bind(int sockfd, struct sockaddr *my_addr,int addrlen);
	int bindSocket = bind(socketDescriptor, (struct sockaddr *)&masterAddress, sizeof(masterAddress));
	if(bindSocket<0){
		perror("bind:");
		exit(bindSocket);
	}
	
	/* listen 
   int listen(int sockfd,int backlog); */
   int listenConnection;
   listenConnection=listen(socketDescriptor,numberOfPlayers);
   if(listenConnection<0){
   	perror("listen:");
   	exit(listenConnection);
   }
   
   /* accept connections
   int accept (int sockfd, struct sockaddr *cliaddr, socklen_t *addrlen)
   put accept in loop to accept multiple connections*/
  //FD_ZERO(&readfds);
   int len,p,i,selectPByMaster;
   for(i=0;i<numberOfPlayers;i++) {
   	len=sizeof(incomingPlayer);
   	// can store the p value in array to maintain all socket connections
   	//printf("Here\n");
   	p=accept(socketDescriptor,(struct sockaddr *)&incomingPlayer, &len);
   	if(p<0){
   		perror("accept:");
   		exit(p);
   	}
   	ihp = gethostbyaddr((char *)&incomingPlayer.sin_addr, 
		sizeof(struct in_addr), AF_INET);
 
      
      FD_SET( p, &readfds);
      FD_SET( p, &readfds1);
      if(p>max_sd)
      	max_sd=p;
                      
      playerStruct * temp=malloc(sizeof(struct Player));
      temp->id=playerIndex;
      temp->socketDescriptorOfPlayer=p;
      temp->playerHostName=malloc(sizeof(char)*100);
      // send peer connect port to master
       bzero(buf,512);
       sprintf(buf, "%d", temp->id);
       len = send(p, buf, strlen(buf),0);
       if (len < 0) {
           perror("ERROR writing to socket");
           exit(1);
       }
      
      bzero(buf,512);
      len = recv(p, buf, 32, 0);
      if ( len < 0 ) {
	       perror("recv");
	       exit(1);
      }
      buf[len] = '\0';
	   
	   int m=0,foundpos=0;
	   while(m++<strlen(buf)){
           if(buf[m]==' '){
           	 temp->peerPortNumber=atoi(strndup(buf,m));
           	 foundpos=m;
           }
      }
	  
	   strcpy(temp->playerHostName,strndup(buf+foundpos+1, m-foundpos-1));
     printf("player %d is on %s\n", playerIndex,temp->playerHostName);
	   //printf("%s   %d\n",,temp->peerPortNumber);
	   playerList[playerIndex]=temp;
      playerIndex++;
	
      //close(p);
      //printf(">> Connection closed\n");

   }
   int k;
   char tempbuf[512];
   //send each player info about their left peer to connect
   // make sure that player count is greater then two/////////////////////////////////////////
   for(k=0;k<numberOfPlayers;k++){
   	// send peer machine name
      bzero(buf,512);
      strcpy(buf,playerList[k]->playerHostName);
      sprintf(tempbuf, "%d",numberOfPlayers); //player id
      strcat(buf," ");
      strcat(buf,tempbuf);
      strcat(buf," ");
      bzero(tempbuf,512);
      sprintf(tempbuf, "%d", playerList[k]->peerPortNumber);//peer port number
      strcat(buf,tempbuf);
            
      //printf("check host name=%s\n",buf);
   	if ((send(playerList[(k+1)%numberOfPlayers]->socketDescriptorOfPlayer,buf, strlen(buf),0))== -1) {
       perror("Failure Sending Message\n");
       //close(sock);
       exit(1);
   }
 
   }
	   
    int okCount=0;
   //waiting for acknowledgement from all Players as ring is formed
   while(1){
   	 int j1,sd1,n1;
   	 int max_sd1=-1;
   	 FD_ZERO(&readfds1);
   	 for(j1=0;j1< numberOfPlayers;j1++){
   	 	FD_SET(playerList[j1]->socketDescriptorOfPlayer,&readfds1);
   	 	if(playerList[j1]->socketDescriptorOfPlayer > max_sd1) 
   	 		max_sd1= playerList[j1]->socketDescriptorOfPlayer;
   	 }
		int activity1=  select( max_sd + 1 , &readfds1 , NULL , NULL , NULL);
   	if ((activity1 < 0) && (errno!=EINTR)) 
        	{
            printf("select error");
       	 }

   	 //printf("Inside select ack\n");
   	 for(j1=0;j1< numberOfPlayers;j1++){
   		sd1=playerList[j1]->socketDescriptorOfPlayer;
   		if(FD_ISSET(sd1,&readfds1)){
   			char bufz[512];
   		   bzero(bufz,512);
   			n1 = read(sd1,bufz,511);
   			if (n1 < 0) {
      			perror("ERROR reading from socket");
      			exit(1);
   			}
   			//printf(" %d buf=%s\n",j1,bufz);
   			if ( strcmp("ok", bufz)==0 )
     			  okCount=okCount+1;
	   		//break;
   		}
   	}
   	//printf("okCount=%d\n",okCount);
   	if(okCount==numberOfPlayers ) 
   		break;
   } 
   
   if(hops>0) {
   selectPByMaster=getRandomPlayer(numberOfPlayers);
   printf("All players present, sending potato to player %d\n",selectPByMaster);
   
   //Create Potato to be passed to Player
   char str[10];
   char potatostr[400]="potato ";
   sprintf(str, "%d", hops);
   strcat(potatostr,str);
   //printf(">> String Start:  %s \n",potatostr );
   startGame(playerList[selectPByMaster]->socketDescriptorOfPlayer,potatostr);
      
   //printf("After Start\n");
    
    char *endMsg;
    int msgSize;
   
	// waiting to receive potato from player to end game
	int activity=  select( max_sd + 1 , &readfds , NULL , NULL , NULL);
   if ((activity < 0) && (errno!=EINTR)) 
        {
            printf("select error");
        }
    int j,sd,n;
    for(j=0;j< numberOfPlayers;j++){
   	sd=playerList[j]->socketDescriptorOfPlayer;
   	if(FD_ISSET(sd,&readfds)){

       if(ioctl(sd,FIONREAD,&msgSize)<0){
          perror("ioctl");
          exit(1);
        }
   		   endMsg=malloc(sizeof(char)*msgSize+1);
   			n = read(sd,endMsg,msgSize);
   			if (n < 0) {
      			perror("ERROR reading from socket");
      			exit(1);
   			}
        endMsg[n]='\0';
   			if ( !strcmp("close", buf) )
     			  exit(1);
   			else{
	   			 //printf("%s", buf);
	   			 break;
	   		}
	   		break;
   	}
   }
   
//Decrypt the message received by master
       int foundPosition=0;
       i=0;
       char *pot;
       char *hopCont=malloc(sizeof(char)*20);
       char *pIdentity=malloc(sizeof(char)*msgSize-strlen("potat  o"));
       bzero(pIdentity,msgSize-strlen("potat  o"));
       while(i++<strlen(endMsg)){
           if(endMsg[i]==' '){
               if(foundPosition==0)
                  pot=strndup(endMsg,i);
               else
                   hopCont=strndup(endMsg+foundPosition+1, i-foundPosition-1);
           foundPosition=i;
           }
      
        }
        pIdentity=strndup(endMsg+foundPosition+1, i-foundPosition-1);
        //printf("%s\n",pIdentity);
        printf("Trace of potato:\n");
        
        i=0;
		foundPosition=0;
        while(i++<strlen(pIdentity)){
   	     if(pIdentity[i]==':'){
               if(foundPosition==0)
                  printf("%s",strndup(pIdentity,i));
               else
                   printf(",%s",strndup(pIdentity+foundPosition+1, i-foundPosition-1));
           foundPosition=i;
           }
        }
        if(foundPosition!=0) 
        		printf(",%s\n",strndup(pIdentity+foundPosition+1, i-foundPosition-1));
        else 
           	printf("%s\n",strndup(pIdentity+foundPosition, i-foundPosition-1));
   }
   
   //send close to all player to close their sockets at end
      for(k=0;k<numberOfPlayers;k++){
            
      //printf("check host name=%s\n",buf);
   	if ((send(playerList[k]->socketDescriptorOfPlayer,"close", 5,0))== -1) {
       perror("Failure Sending Message\n");
       //close(sock);
       exit(1);
   }
 	
 	close(playerList[k]->socketDescriptorOfPlayer);
  // printf("Socket closed for%d\n",k);
   }
   
   printf(">> Master closed\n");
   close(socketDescriptor);
   exit(0);	
}