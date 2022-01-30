
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <unistd.h>
//#include <errno.h>


#define BUFSIZE 100
#define ACK_PDU_TYPE 'A'
#define ERROR_PDU_TYPE 'E'

struct pdu {
char type;
char data[100];
} NamePDU, IndexPDU, SendPDU,TCPPDU;

struct RPDU { //Register PDU structure
	char type;
	char peerName[10];
	char contentName[10];
	char address[80];
};

struct TPDU { //De Register PDU structure
	char type;
	char peerName[10];
	char contentName[10];
};

void options() {
	printf("\nChoose one of the following options:\nR - Register Content\nD - Content Download Request\nT - Content De-Registration\nO - List of Online Registered Content\nS - Search for Content\nQ - Quit\n\n");
}

char * genRpduString(struct RPDU pdu, char * stringRPDU){ //convert RPDU to string for transmission
    bzero(stringRPDU, sizeof(101));
    sprintf(stringRPDU, "%c", pdu.type);
    strcat(stringRPDU, ",");
    strcat(stringRPDU, pdu.peerName);
    strcat(stringRPDU, ",");
    strcat(stringRPDU, pdu.contentName);
    strcat(stringRPDU, ",");
    strcat(stringRPDU, pdu.address);

    return stringRPDU;
};

char * genTpduString(struct TPDU pdu, char * stringTPDU){ //convert TPDU to string for transmission
    bzero(stringTPDU, sizeof(21));
    sprintf(stringTPDU, "%c", pdu.type);
    strcat(stringTPDU, ",");
    strcat(stringTPDU, pdu.peerName);
    strcat(stringTPDU, ",");
    strcat(stringTPDU, pdu.contentName);

    return stringTPDU;
};
void TCPConnection(int new_sd){
	char BUFF[100];
	char Con[101];
	char Mess[101];
	memset(BUFF, 0, strlen(BUFF));
	memset(Con, 0, strlen(Con));
	memset(Mess, 0, strlen(Mess));
	memset(TCPPDU.data, 0, strlen(TCPPDU.data));
	FILE*fp;

	read(new_sd, BUFF,100);
	TCPPDU.type = BUFF[0];
	for (int i = 0; i < 100; i++) {
		TCPPDU.data[i] = BUFF[i + 1];
	}
	if(TCPPDU.type == 'D'){
		fp =fopen(TCPPDU.data, "r");
		if(fp==NULL){
			printf("Error: finding file\n");
			strcat(Mess, "E");
			strcat(Mess, "Error: finding file\n");
		}
		else{
			fread(Con,1, 100,fp);
			strcat(Mess, "C");
			strcat(Mess, Con);
		}
		write(new_sd, Mess, 100);
		fclose(fp);
	}
	else{
		printf("There was an error receiving TCP socket from peer\n");
	}
	close(new_sd);
	//options();
}

void terminal(char* user_name, int s, char* Tport, char* Thost){

    char  type;
    int n,TCPport;
    char data[101];
    FILE*fp;
    char content[1000];

    char buffer[101];
    struct RPDU rpdu; //declare register pdu
    struct TPDU tpdu; //declare de register pdu


    char ER [10];
    char Tadd[80];
    char peerName[10];
    char contentName[10];
    char*registerPDU;
    char*deRegisterPDU;
    registerPDU=malloc(101);
    deRegisterPDU=malloc(101);
    char SinChar;

    	int 	STCP;
	struct	sockaddr_in serverTCP;
//	char *host;
	struct hostent *phe;


//    options();




    //get a single character from stdin
    type=getchar();
    switch(type){
    	case 'D':
    //get the file name of interest
    	printf("Enter the name of the Content: \n");
    	memset(NamePDU.data, 0, sizeof(NamePDU.data));
    	n=read(0, NamePDU.data, BUFSIZE);
    	NamePDU.type= 'S';
    	NamePDU.data[n-1]='\0';

    //send the file name to Index
	if( write(s, &NamePDU, n+1) <0){
		printf("\nError sending file name to Index\n");
		options();
    		break;
    	}
    //get the response from Index
    	memset(data, 0, sizeof(data));
    	n=read(s, data, BUFSIZE);
    	IndexPDU.type = data[0];
    	memset(IndexPDU.data, 0, sizeof(IndexPDU.data));
    	for(int i=0; i<n; i++){
    		IndexPDU.data[i] = data[i+1];
    	}
    	IndexPDU.data[n-1] ='\0';
    	TCPport = atoi(IndexPDU.data);
    //check if there was an error
    	if(IndexPDU.type == 'E'){
    		printf("Error Content Doesn't Exist\n");
    		options();
    		break;
    	}
    //Set up TCP connection with the Peer
    //empty out the Server structure and set default Values
    memset(&serverTCP, 0 , sizeof(serverTCP));
    serverTCP.sin_family = AF_INET;
    serverTCP.sin_port = TCPport;

    // Make sure everything good with IP and copy into server
    if  (phe = gethostbyname(Thost)) {
        memcpy(&serverTCP.sin_addr, phe->h_addr, phe->h_length);
    } else if ((serverTCP.sin_addr.s_addr = inet_addr(Thost)) == INADDR_NONE) {
        fprintf(stderr, "TCP:Cant get host entry \n");
    }

    // Create a stream socket
    if ((STCP= socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "TCP:Can't create socket\n");
//        exit(1);
    }


    	printf("Connected to Port: %d\n", serverTCP.sin_port );
    // Connect the socket
    if (connect(STCP, (struct sockaddr *)&serverTCP, sizeof(serverTCP)) < 0)
        fprintf(stderr, "TCP:Can't connect to %s \n", Thost);

    //sending the file name to the peer that has the content
    NamePDU.type='D';
    write(STCP, &NamePDU, sizeof(NamePDU.data) + 1);
    //recieving the reponse from the peer
    memset(data, 0, sizeof(data));
    bzero(content,1000);
    while( n=read(STCP, data, BUFSIZE) ){
	    SendPDU.type= data[0];
	    for(int i=0; i<n; i++){
	    	SendPDU.data[i] = data[i+1];
	    }
	    SendPDU.data[n-1]='\0';
	    strcat(content, SendPDU.data);
	    memset(SendPDU.data, 0, sizeof(SendPDU.data));
    }

    //check if there was an error
    if(SendPDU.type == 'E'){
    	printf("Error receiving from peer");
    	options();
    	break;
    }
    //else create file and store contents inside
    else{
    	fp=fopen(NamePDU.data,"w");
    	fputs( content,fp);
    	fclose(fp);
    	memset(content, 0, sizeof(content));
    	printf("%s DOWNLOADED!",NamePDU.data);

    }
    close(STCP);


     	rpdu.type = 'R';
    //save info to rpdu
    	strcpy(rpdu.peerName,user_name);
    	//issue lies here
    	strcpy(rpdu.contentName,NamePDU.data);
    	sprintf(Tadd, "%d",serverTCP.sin_port);
    	strcpy(rpdu.address,Tport);
    //convert Rpdu to string
    	registerPDU = genRpduString(rpdu, registerPDU);
    //write the register PDU string to index server
    	write(s,registerPDU,101);
    //get response from index
    	int receive=0;
        receive = read(s, buffer,101);
    //if sucessfully registered, index server returns an Ack PDU, so we let the user know and clear all variables
	if(buffer[0]==ACK_PDU_TYPE){
	    	printf("\nCONTENT REGISTERD!\n");
	    	bzero(rpdu.contentName,10);
	    	bzero(buffer,101);
	}
    //if invalid peer name, index server returns an error PDU
    	else if (buffer[0]==ERROR_PDU_TYPE){
	    	printf("\nError Registering At Index,Change Peer Name!\n");
	    	bzero(rpdu.contentName,10);
	    	bzero(buffer,101);
    	}


    	options();
    	break;
  	case 'S':
    //get the file name of interest
    	printf("Enter the name of the Content: \n");
    	memset(NamePDU.data, 0, sizeof(NamePDU.data));
    	n=read(0, NamePDU.data, BUFSIZE);
    	NamePDU.type= 'S';
    	NamePDU.data[n-1]='\0';
    //send the file name to Index
	if( write(s, &NamePDU, n+1) <0){
		printf("\nError sending file name to Index\n");
		options();
		break;
    	}
    //get the response from Index
    	memset(data, 0, sizeof(data));
    	n=read(s, data, BUFSIZE);
    	IndexPDU.type = data[0];
    	memset(IndexPDU.data, 0, sizeof(IndexPDU.data));
    	for(int i=0; i<n; i++){
    		IndexPDU.data[i] = data[i+1];
    	}
    	IndexPDU.data[n-1] ='\0';
    	TCPport = atoi(IndexPDU.data);
    //check if there was an error
    	if(IndexPDU.type == 'E'){
    		printf("Error Content Doesn't Exist\n");
    		options();
    		break;
    	}
    //If the server has the content
    	else if( IndexPDU.type = 'S'){
    		printf("\nContent Can Be Found At Port: %d\n",TCPport);
    	}
    	options();
    	break;
    case 'O':

	SinChar='O';
    //send the file name to Index
	if( write(s,&SinChar, 1) <0){
		printf("\nError sending file name to Index\n");
		options();
		break;
    	}
    //get the response from Index
    	memset(data, 0, sizeof(data));
    	n=read(s, data, BUFSIZE);
    	IndexPDU.type = data[0];
    	memset(IndexPDU.data, 0, sizeof(IndexPDU.data));
    	for(int i=0; i<n; i++){
    		IndexPDU.data[i] = data[i+1];
    	}
    	IndexPDU.data[n-1] ='\0';
    //check if there was an error
    	if(IndexPDU.type == 'E'){
    		printf("\nError receiving from Index\n");
    		options();
    		break;
    	}
    //If the server has the content
    	else if( IndexPDU.type = 'O'){
    		printf("Online Content:\n%s\n",IndexPDU.data);
    	}
    	options();
    	break;
    case 'R':
    	rpdu.type = 'R';
    	fflush(stdout);
    	fgets(ER,10, stdin);
    //get required info from stdin
    	printf("Enter Content Name: \n");
    	fflush(stdout);
    	fgets(contentName,10, stdin);
    	strtok(contentName,"\n");
    //save info to rpdu
    	strcpy(rpdu.peerName,user_name);
    	strcpy(rpdu.contentName,contentName);
    	strcpy(rpdu.address,Tport);
    //convert Rpdu to string
    	registerPDU = genRpduString(rpdu, registerPDU);
    //write the register PDU string to index server
    	write(s,registerPDU,101);
    //get response from index
        receive = read(s, buffer,101);
    //if sucessfully registered, index server returns an Ack PDU, so we let the user know and clear all variables
	if(buffer[0]==ACK_PDU_TYPE){
	    	printf("CONTENT REGISTERD!\n");
	    	bzero(rpdu.contentName,10);
	    	bzero(buffer,101);
	}
    //if invalid peer name, index server returns an error PDU
    	else if (buffer[0]==ERROR_PDU_TYPE){
	    	printf("\nError Registering At Index,Change Peer Name!\n");
	    	bzero(rpdu.contentName,10);
	    	bzero(buffer,101);
    	}

    	options();
    	break;
    case 'T':
    	tpdu.type = 'T';
    	fflush(stdout);
    	fgets(ER,10, stdin);
    //get required info from stdin
    	printf("Enter Content Name to De Register: \n");
    	fflush(stdout);
    	fgets(contentName,10, stdin);
    	strtok(contentName,"\n");
    //save info to rpdu
    	strcpy(tpdu.peerName,user_name);
    	strcpy(tpdu.contentName,contentName);
    //convert Rpdu to string
 	deRegisterPDU = genTpduString(tpdu, deRegisterPDU);
    //send to index server
    	write(s,deRegisterPDU,101);
    //get response from index
    	receive = read(s, buffer,101);
    //if sucessfully registered, index server returns an Ack PDU, so we let the user know and clear all variables
    	if(buffer[0]==ACK_PDU_TYPE){
		printf("CONTENT SUCESSFULLY DE REGISTERED!\n");
    	}
    	else{
    		printf("ERROR, UNABLE TO DE-REGISTER CONTENT!\n");
    	}
    	options();
    	break;
    case 'Q':

    memset(NamePDU.data, 0, sizeof(NamePDU.data));
    	NamePDU.type= 'Q';
    	strcpy(NamePDU.data,user_name);
    	n=strlen(NamePDU.data);
    //send the file name to Index
	if( write(s, &NamePDU, n+1) <0){
		printf("\nError sending file name to Index\n");
		options();
		break;
    	}
    	printf("De-registering all content\n");
    //get the response from Index
    	memset(data, 0, sizeof(data));
    	n=read(s, data, BUFSIZE);
    	IndexPDU.type = data[0];
    	memset(IndexPDU.data, 0, sizeof(IndexPDU.data));
    	for(int i=0; i<n; i++){
    		IndexPDU.data[i] = data[i+1];
    	}
    	IndexPDU.data[n-1] ='\0';
    	if( IndexPDU.type = 'A'){
    		printf("ACKNOWLEDGED!\n");
    	}
    	exit(0);
  }

}

int main (int argc, char** argv) {

	int 	s, new_s, port;
	struct	sockaddr_in server;
	char *host;
	struct hostent *phe;


	int TCPs,alen, new_sd, client_len;
	struct	sockaddr_in TCPserver, client;
	struct hostent *TCPphe;

	int n;
	char user_name[10];
	char ER [10];
	char Tport [80];
	char Thost [80];
	memset(user_name, 0 , sizeof(user_name));
	memset(ER, 0 , sizeof(ER));
	memset(Tport, 0 , sizeof(Tport));
	memset(Thost, 0 , sizeof(Thost));



    //store the IP address and Port # from the terminal
    switch(argc) {
        case 3:
            host = argv[1];
            port = atoi(argv[2]);
            break;
        default:
            fprintf(stderr, "Usage: %s [port]\n", argv[0]);
            exit(1);
    }

    //empty out the Server structure and set default Values
    memset(&server, 0 , sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);


    // Make sure everything good with IP and copy into server
    if  (phe = gethostbyname(host)) {
        memcpy(&server.sin_addr, phe->h_addr, phe->h_length);
    } else if ((server.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE) {
        fprintf(stderr, "Cant get host entry \n");
    }

    // Create a stream socket
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        fprintf(stderr, "Can't create socket\n");
        exit(1);
    }

    // Connect the socket
    if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0)
        fprintf(stderr, "Can't connect ot %s \n", host);


    //Set up TCP connection
    //empty out the Server structure and set default Values
    memset(&TCPserver, 0 , sizeof(TCPserver));
    TCPserver.sin_family = AF_INET;
    TCPserver.sin_port = htons(0);
    TCPserver.sin_addr.s_addr=htonl(INADDR_ANY);

    // Create a stream socket
    if ((TCPs = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "TCP:Can't create socket\n");
    }

    if (bind(TCPs,(struct sockaddr *)&TCPserver, sizeof(TCPserver)) == -1){
		fprintf(stderr, "Can't bind name to socket\n");
		exit(1);
	}

	alen=sizeof(struct sockaddr_in);
	getsockname(TCPs,(struct sockaddr *)&TCPserver, &alen);


	listen(TCPs, 5);

	fd_set rfds, afds;

	printf("Choose a user name\n");
    	memset(user_name, 0, sizeof(user_name));
    	n=read(0, user_name, sizeof(user_name));
    	user_name[n-1]='\0';

    	sprintf(Thost, "%d",TCPserver.sin_addr.s_addr);
    	printf("\n%s's TCP port number: %d\n", user_name,TCPserver.sin_port);
    	sprintf(Tport, "%d",TCPserver.sin_port);
	options();

	while(1){
	//clear the file descriptor
		FD_ZERO(&afds);
	//gets TCPS value sets the corresponding afds bit to 1
		FD_SET(TCPs, &afds);
	//sets the 0th bit of afds to 1
		FD_SET(0, &afds);
		memcpy(&rfds, &afds, sizeof(rfds));
	// retrun is blocked until TCP connection estblished
	//or get someting from stdin then rfds will contain
	//the socket that needs to be serviced
		select(FD_SETSIZE, &rfds,NULL,NULL,NULL);

	//check if data arrived at stdin
		if(FD_ISSET(TCPs,&rfds)){
			client_len = sizeof(client);
	  		new_sd = accept(TCPs, (struct sockaddr *)&client, &client_len);
	  		if(new_sd < 0){
			    fprintf(stderr, "TCP Can't accept client \n");
			}
			else{
				TCPConnection(new_sd);
			}

		}
	//check if there is a pending TCP connection
		if(FD_ISSET(0,&rfds)){
			terminal(user_name,s,Tport,Thost);
		}
	}
}
