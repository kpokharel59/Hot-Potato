#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
 #include <sys/ioctl.h>
#include <errno.h>

#define LEN	64

char *masterMachineName;
int portNumber;

int getRandomPlayer(int n){
	int randm=rand();
	return randm%n;
}

int main(int argc, char *argv[]){

	if(argc<3||argc>3){
		printf("Syntax: player <master-machine-name> <port-number> \n");
      exit(1);
	}
	//printf("Player\n");
	masterMachineName=argv[1];
	portNumber=atoi(argv[2]);
	int s,s1,rc,len,peerSock;
	char str[LEN];
	char host[64];
	int peerConnectPort,peerPort;
	char *leftPeermName=malloc(sizeof(char)*100);
   char *iid=malloc(sizeof(char)*20);
	char *leftport=malloc(sizeof(char)*15);
	char *hopCont=malloc(sizeof(char)*20);
	char *pIdentity;
	int leftPlayerPortNo;
	int id,hopCount,numberOfPlayers;
	int lsd;
	int rsd;
	fd_set rfds;
	//printf("Master machine name: %s",masterMachineName);
	
	
	struct hostent *hp;
	struct hostent *lefthp;
	struct hostent *selfhp;
	struct sockaddr_in sin,sin1;
	peerConnectPort=portNumber+1;
	
  /* fill in hostent struct for self */
  gethostname(host, sizeof host);
  selfhp = gethostbyname(host);
  if (selfhp == NULL ) {
    fprintf(stderr, "%s: host not found (%s)\n", argv[0], host);
    exit(1);
  }

	/* fill in hostent struct */
   hp = gethostbyname(masterMachineName); 
   if ( hp == NULL ) {
       fprintf(stderr, "%s: host not found (%s)\n", argv[0], masterMachineName);
       exit(1);
   }
	
	//Creating socket to communicate with peer processes
	peerSock=socket(AF_INET,SOCK_STREAM,0);
   	if(peerSock<0){
		perror("socket:");
		exit(peerSock);
	} 
   
	struct sockaddr_in masterAddress,incomingPlayer;
	bzero((char*)&masterAddress,sizeof(masterAddress));
	masterAddress.sin_family=AF_INET;
   memcpy(&masterAddress.sin_addr, selfhp->h_addr_list[0], selfhp->h_length);	
	int bindSocket;
	
   do {
		masterAddress.sin_port=htons(peerConnectPort);
		masterAddress.sin_addr.s_addr=htonl(INADDR_ANY);
		bindSocket = bind(peerSock, (struct sockaddr *)&masterAddress, sizeof(masterAddress));
		if(bindSocket<0){
			 peerConnectPort++;
		}
		}while(bindSocket<0);
   
   //Listen
   int listenConnection;
   listenConnection=listen(peerSock,3);
   if(listenConnection<0){
   	perror("listen:");
   	exit(peerSock);
   }   
   
	/* create and connect to a socket */

   /* use address family INET and STREAMing sockets (TCP) */
   s = socket(AF_INET, SOCK_STREAM, 0);
   if ( s < 0 ) {
       perror("socket:");
       exit(s);
   }
   
   /* set up the address and port */
   bzero((char *)&sin,sizeof(sin));
   sin.sin_family = AF_INET;
   sin.sin_port = htons(portNumber);
   sin.sin_addr.s_addr=htonl(INADDR_ANY);
   memcpy(&sin.sin_addr, hp->h_addr_list[0], hp->h_length);
  
   /* connect to socket at above addr and port */
   rc = connect(s, (struct sockaddr *)&sin, sizeof(sin));
   if ( rc < 0 ) {
       perror("connect:");
       exit(rc);
   }
   
     /* read a string from the terminal and send on socket */
	int n;
   char buffer[512];
  // while(1) {
  	   // receive th id of the player from master
       bzero(buffer,512);
       n = recv(s,buffer,511,0);
   
       if (n < 0) {
          perror("ERROR reading from socket");
          exit(1);
       } 
       id=atoi(buffer); 	
       printf("Connected as player =%d\n",id);
  	
      // send peer connect port to master
       bzero(buffer,512);
       sprintf(buffer, "%d", peerConnectPort);
       strcat(buffer," ");
       strcat(buffer,host);
       n = write(s, buffer, strlen(buffer));
       if (n < 0) {
           perror("ERROR writing to socket");
           exit(1);
       }

       // Read left peer machine name, port number to connect
       // id received is players own id
       bzero(buffer,512);
       n = recv(s,buffer,511,0);
   
       if (n < 0) {
          perror("ERROR reading from socket");
          exit(1);
       }
       buffer[n]='\0';
       //strcpy(leftPeermName,buffer);
       //printf("peer Name =%s\n",leftPeermName);
       
       int foundPosition=0;
       int i=0;
       while(i++<strlen(buffer)){
           if(buffer[i]==' '){
               if(foundPosition==0)
                   leftPeermName=strndup(buffer,i);
               else
                   iid=strndup(buffer+foundPosition+1, i-foundPosition-1);
           foundPosition=i;
           }
      
        }
        strcpy(leftport,strndup(buffer+foundPosition+1, i-foundPosition-1));
        peerPort=atoi(leftport);
        numberOfPlayers=atoi(iid);
        lefthp=gethostbyname(leftPeermName);
        // printf("Number of player =%d\n",numberOfPlayers);
        
        
        // if id==0 accept first then connect otherwise connect first then accept
        int len1,p;
        if(id==0) {
        	    rsd=accept(peerSock,(struct sockaddr *)&incomingPlayer, &len1);
     	    if(rsd<0){
   		    perror("accept:");
   		    exit(rsd);
   	    }
 			 //printf("Connected to right\n");
        }
        //connect first
       s1 = socket(AF_INET, SOCK_STREAM, 0);
       if ( s1 < 0 ) {
           perror("socket:");
           exit(s);
       }
   
        /* set up the address and port */
        bzero((char *)&sin1,sizeof(sin1));
        sin1.sin_family = AF_INET;
        sin1.sin_port = htons(peerPort);
        sin1.sin_addr.s_addr=htonl(INADDR_ANY);
        memcpy(&sin1.sin_addr, lefthp->h_addr_list[0], lefthp->h_length);
        
        lsd=connect(s1, (struct sockaddr *)&sin1, sizeof(sin1));
        if ( lsd < 0 ) {
            perror("connect:");
            exit(lsd);
        }
        //printf("Player %d connected to left\n",id);
        
        //accept
        if(id!=0){
            rsd=accept(peerSock,(struct sockaddr *)&incomingPlayer, &len1);
     	         if(rsd<0){
   		          perror("accept:");
   		          exit(rsd);
   	         }
   	      //printf("Connected to right\n");
   	  }
   	  
   	  // send ok when connected to left and right peer
       bzero(buffer,512);
       strcpy(buffer,"ok");
       n = write(s, buffer, strlen(buffer));
       if (n < 0) {
           perror("ERROR writing to socket");
           exit(1);
       }
       //printf("ok send by player %d\n",id);
   	   char * messageBuf;
   	   	    
       // Acknowledge Master That ring has formed.
   	 //receive potato
	    FD_SET(s,&rfds);
	    FD_SET(s1,&rfds);
	    FD_SET(rsd,&rfds);	
	    
	    int maxfd= (s>s1?s:s1);
	    maxfd=maxfd>rsd?maxfd:rsd;  	       	 
       int retval;  
       srand(id);
       int msgSize; 	 
   	 
       while(1) {
       FD_ZERO(&rfds);
       FD_SET(s,&rfds);
	    FD_SET(s1,&rfds);
	    FD_SET(rsd,&rfds);
   	 retval=select(maxfd+1,&rfds,NULL,NULL,NULL);

       if ((retval < 0) && (errno!=EINTR)) 
       {
            printf("select error");
	    exit(1);
       }

	
       if(FD_ISSET(s,&rfds)) { 

		   if(ioctl(s,FIONREAD,&msgSize)<0){
        	perror("ioctl");
        	exit(1);
        } 
        messageBuf=malloc(msgSize+1);
		 bzero(messageBuf,msgSize+1);    
       	n = recv(s,messageBuf,msgSize,0);//////////////////////////////////look how to use select to listen all
   
       	if (n < 0) {
         	 perror("ERROR reading from socket");
         	 exit(1);
      	 }
      	// potato_received=1;
      	 messageBuf[n]='\0';
      	 if(!strcmp("close",messageBuf))
      	    break;
       }else if(FD_ISSET(s1,&rfds)) {  

        if(ioctl(s1,FIONREAD,&msgSize)<0){
        	perror("ioctl");
        	exit(1);
        }

        messageBuf=malloc(msgSize+1);
        bzero(messageBuf,msgSize+1);   
       	n = recv(s1,messageBuf,msgSize,0);//////////////////////////////////look how to use select to listen all
   
       	if (n < 0) {
         	 perror("ERROR reading from socket");
         	 exit(1);
      	 }
         // potato_received=1;
      	 messageBuf[n]='\0';
       }else if(FD_ISSET(rsd,&rfds)) { 
       	
        if(ioctl(rsd,FIONREAD,&msgSize)<0){
        	perror("ioctl");
        	exit(1);
        }

        messageBuf=malloc(msgSize+1);
         bzero(messageBuf,msgSize+1);     
       	n = recv(rsd,messageBuf,msgSize,0);//////////////////////////////////look how to use select to listen all
   
       	if (n < 0) {
         	 perror("ERROR reading from socket");
         	 exit(1);
      	 }
      	 //potato_received=1;
      	 messageBuf[n]='\0';
       }
       //printf("At player %d received String :%s\n",id,messageBuf);
       
       pIdentity=malloc(sizeof(char)*msgSize-strlen("potat  o"));
       bzero(pIdentity,msgSize-strlen("potat  o"));
       //process the received potato
       foundPosition=0;
       i=0;
       char *pot;
       while(i++<strlen(messageBuf)){
           if(messageBuf[i]==' '){
               if(foundPosition==0)
                  pot=strndup(messageBuf,i);
               else
                   hopCont=strndup(messageBuf+foundPosition+1, i-foundPosition-1);
           foundPosition=i;
           }
      
        }
        if(strchr(messageBuf,' ')==strrchr(messageBuf,' '))
            hopCont=strndup(messageBuf+foundPosition+1, i-foundPosition-1);
        else
            pIdentity=strndup(messageBuf+foundPosition+1, i-foundPosition-1);
       // printf(" pot=%s hopCont=%s pIdentity=%s\n",pot,hopCont,pIdentity);
        hopCount=atoi(hopCont);
        hopCount=hopCount-1;
		  char pId[5];
		  sprintf(pId,"%d",id);  
		  if(strlen(pIdentity))
		  strcat(pIdentity,":");   
		  strcat(pIdentity,pId); 
        
        char strI[10];
        char tempp[10];
        //sprintf()
        //printf("Hop Count after decrement :%d\n",hopCount);
        //printf("pIdentity :%s\n",pIdentity);
         sprintf(strI, "%d", hopCount);
        	  char *potatostr=malloc(sizeof(char)*msgSize +3);
        	  bzero(potatostr,(msgSize +3));
        	 // strcpy(potatostr,"potato ");
              //char *tem=malloc(sizeof(char)*msgSize + strlen(strI)2);
        	  //bzero(potatostr,(msgSize + strlen(strI)+ strlen("potato ")+2));

           sprintf(potatostr,"%s%s %s","potato ",strI,pIdentity);
          // strcat(potatostr,strI);
           //strcat(potatostr," ");
           //strcat(potatostr,pIdentity);
           //printf("Potato to be send  %s\n",potatostr);
           int selectPByMaster=getRandomPlayer(2);
           //char msg[512];
			//  bzero(msg,512);
			//  strcpy(msg,potatostr);
           if(hopCount>0 ){
           	//potato_received=0;
           if(selectPByMaster==0){
           	  printf("Sending potato to %d\n",((id-1)+numberOfPlayers)%numberOfPlayers);
	           //send to left if 0
	 			 if ((send(s1,potatostr, strlen(potatostr),0))== -1) {
       				perror("Failure Sending Message\n");
       				close(s1);
       				exit(1);
   			 }
           }
           else {
           	  
           		if ((send(rsd,potatostr, strlen(potatostr),0))== -1) {
       				perror("Failure Sending Message\n");
       				close(rsd);
       				exit(1);
   			 }
   			  printf("Sending potato to %d\n",(id+1)%numberOfPlayers);
           }
        }
        else if(hopCount==0){
         printf("I'm it %s\n",potatostr);
        	//send to the master
         if ((send(s,potatostr, strlen(potatostr),0))== -1) {
       			perror("Failure Sending Message\n");
       			close(s);
       			exit(1);
   		}
        }
        }
       
   close(s1);
   close(peerSock);
   close(rsd);
   close(s);
   exit(0);
   
}
