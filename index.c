#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>//for lstat

#define BUFSIZE 100
#define ERRORMESSAGE "E,ERROR"
#define ACKMESSAGE "A,Ack"

struct pdu {
char type;
char data[100];
} RequestPDU, ResponsePDU;

struct RPDU{ //register PDU structure
	char type;
    	char peerName[10];
    	char contentName[10];
    	char address[80];
};

struct TPDU{ //De Register PDU structure
	char type;
    	char peerName[10];
    	char contentName[10];
};

//changes RPDU string back to RPDU pdu format
struct RPDU deGenRpduString(char *stringRPDU,struct RPDU*rpdu){
    bzero(rpdu->peerName,10);
    bzero(rpdu->contentName,10);
    bzero(rpdu->address,80);

    strcat(stringRPDU,",");
    rpdu->type=stringRPDU[0];
    char *token=strtok(stringRPDU,",");
    token=strtok(NULL,",");
    strcpy(rpdu->peerName,token);
    token=strtok(NULL,",");
    strcpy(rpdu->contentName,token);
    token=strtok(NULL,",");
    strcpy(rpdu->address,token);

    return *rpdu;
};

//changes TPDU string back to TPDU pdu format
struct TPDU deGenTpduString(char *stringTPDU,struct TPDU*tpdu){
    bzero(tpdu->peerName,10);
    bzero(tpdu->contentName,10);

    strcat(stringTPDU,",");
    tpdu->type=stringTPDU[0];
    char *token=strtok(stringTPDU,",");
    token=strtok(NULL,",");
    strcpy(tpdu->peerName,token);
    token=strtok(NULL,",");
    strcpy(tpdu->contentName,token);

    return *tpdu;
};


//check to see if content is registered and return its index
int findDContent(char *data[20][4],char*contentName){
    int i;
    int index=-1;
    for(i=0;i<20;i++){
        if((strcmp(contentName,data[i][1])==0)&&index==-1){
            index=i;
        }
        else if(index!=-1&&(strcmp(contentName,data[i][1])==0)){
            if(atoi(data[i][3])<atoi(data[index][3])){
                index=i;
            }
        }
    }
    return(index);
};


//check if there is an existing content name matching to peer name
//in the data array, if there is returns 0, if unique then returns 1
int checkData(char *data[20][4],char*peerName,char*contentName){
    int i;
    int flag=1;
    for(i=0;i<20;i++){
        if((strcmp(contentName,data[i][1])==0)&&(strcmp(peerName,data[i][0])==0)){
            flag=0;
        }
    }
    return(flag);
};

//searches data array to see if there is an entry that matches the
//provided peerName and contentName, if there is it returns the index
//otherwise it returns -1
int findContent(char *data[20][4],char*peerName,char*contentName){
    int i;
    int index=-1;
    for(i=0;i<20;i++){
        if((strcmp(contentName,data[i][1])==0)&&(strcmp(peerName,data[i][0])==0)){
            index=i;
        }
    }
    return(index);
};


/*------------------------------------------------------------------------
 * main - Iterative UDP server for TIME service
 *------------------------------------------------------------------------
 */
int
main(int argc, char *argv[])
{
	struct  sockaddr_in fsin;	/* the from address of a client	*/


	char    *pts;
	int	sock;			/* server socket		*/
	time_t	now;			/* current time			*/
	int	alen;			/* from-address length		*/
	struct  sockaddr_in sin; /* an Internet endpoint address         */
        int     s, type;        /* socket descriptor and socket type    */
	int 	port=3000;

	char	*buf;
	buf = malloc(101);

    	//declare main register PDU and de-register PDU structures
    	struct RPDU rpdu;
    	struct TPDU tpdu;

//create a 3d array of 20 so index server can store up to 20
    	//entries with content name, peer name and peer address
    	//allocate 20 characters of space for each content name,peer name
    	//and address as well
    	char *data[20][4];
    	int i,j,in=-1;
    	for(i=0;i<20;i++){
    	    for(j=0;j<4;j++){
    	    	//data[i][j]='\0';
    	        data[i][j]=malloc(20);
    	        memset(data[i][j], 0, sizeof(data[i][j]));
    	    };
    	};


    	//declare index for main data array
    	int index=0;


	switch(argc){
		case 2:
			port = atoi(argv[1]);
			break;
		default:
			fprintf(stderr, "Usage: %s [port]\n", argv[0]);
			exit(1);
	}

        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = INADDR_ANY;
        sin.sin_port = htons(port);

    /* Allocate a socket */
        s = socket(AF_INET, SOCK_DGRAM, 0);
        if (s < 0)
		fprintf(stderr, "can't creat socket\n");

    /* Bind the socket */
        if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		fprintf(stderr, "can't bind to %d port\n",port);
        listen(s, 5);
	alen = sizeof(fsin);


	char pdu_type;
	char BUFF[101];
	memset(BUFF,0,strlen(BUFF));
	FILE * fp;
	struct stat st;
	int size;

	while (1) {
		memset(&BUFF, 0 , sizeof(BUFF));
		//receiving message from peer:
		recvfrom(s, BUFF, BUFSIZE, 0,(struct sockaddr *)&fsin, &alen);
		strcpy(buf,BUFF);
		fflush(stdout);

		memset(&RequestPDU.data, 0 , sizeof(RequestPDU.data));
		//coping the contents of data buffer into RequestPDU
		RequestPDU.type = BUFF[0];
		for (int i = 0; i < BUFSIZE; i++) {
			RequestPDU.data[i] = BUFF[i + 1];
		}
		switch(RequestPDU.type){
		case 'S':

			printf("Searching for %s\n", RequestPDU.data);
			//check registered content to see if
			//requested content is present
			//and if it is send back port number
			strcat(RequestPDU.data,"\n");
			in =findDContent(data,RequestPDU.data);
			if( in != -1){
				memset(&ResponsePDU.data, 0 , sizeof(ResponsePDU.data));
				ResponsePDU.type = 'S';
				strcpy(ResponsePDU.data, data[in][2]);
				sendto(s, &ResponsePDU, strlen(ResponsePDU.data)+1, 0,(struct sockaddr *)&fsin, sizeof(fsin));
				//increment entry usage by 1 after each time port details are requested
				int val=atoi(data[in][3]);
				val++;
				sprintf(data[in][3],"%d",val);
				}

		//if the content isnt found then there is an error
			else {
				ResponsePDU.type = 'E';
				strcpy(ResponsePDU.data, "File not found");
				sendto(s, &ResponsePDU, strlen(ResponsePDU.data)+1, 0,(struct sockaddr *)&fsin, sizeof(fsin));
			}
			break;
		case 'O':
			memset(&ResponsePDU.data, 0 , sizeof(ResponsePDU.data));
			ResponsePDU.type = 'O';
			printf("Sending Online-content list\n");
			// check if there are any content stored and
			// copy into responsePDU to send back to peer
			for(int i=0; i<20;i++){
				if( strcmp(data[i][1],"")!=0 ){
					strcat(ResponsePDU.data, data[i][1]);
				}
			}
			//check if anything was added to ResponsePDU
			if( strcmp(ResponsePDU.data,"")==0 ){
				strcpy(ResponsePDU.data, "Nothing is stored yet");
			}

			sendto(s, &ResponsePDU, strlen(ResponsePDU.data)+1, 0,(struct sockaddr *)&fsin, sizeof(fsin));


			break;
		case 'R':
		   	 //convert received string into RPDU format
		        rpdu=deGenRpduString(buf,&rpdu);
		        strcat(rpdu.contentName,"\n");
		        printf("%s requesting to register %s\n",rpdu.peerName,rpdu.contentName);
		        //check if recieved peer and content name combo is in data
		        //array already, if not allow to send ACK
		        if (checkData(data,rpdu.peerName,rpdu.contentName)==1){
		    	    sendto(s, ACKMESSAGE, strlen(ACKMESSAGE), 0, (struct sockaddr *) &fsin, sizeof(fsin));

		    	    //write peerName,contentName and address to array
		    	    memcpy(data[index][0],rpdu.peerName,sizeof(rpdu.peerName));
		    	    memcpy(data[index][1],rpdu.contentName,sizeof(rpdu.contentName));
		    	    memcpy(data[index][2],rpdu.address,sizeof(rpdu.address));

	    	     	    //increment index so next registered content is
	    	     	    //put into next array row
	    	     	    index++;
		    	 }
		    	 //if received name combo is in array already, send ERROR
		    	 else{
		    	     sendto(s, ERRORMESSAGE, strlen(ERRORMESSAGE), 0, (struct sockaddr *) &fsin, sizeof(fsin));

		    	 }

			break;
		case 'T':
		        //convert received string into TPDU structure
		        tpdu=deGenTpduString(buf,&tpdu);
		        strcat(tpdu.contentName,"\n");
		        printf("%s requesting to de-register %s\n",tpdu.peerName,tpdu.contentName);
		        //index to delete
		        int dIndex=findContent(data,tpdu.peerName,tpdu.contentName);
		        //if content is not found in array, return ERROR
		        if(dIndex==-1){
		            sendto(s, ERRORMESSAGE, strlen(ERRORMESSAGE), 0, (struct sockaddr *) &fsin, sizeof(fsin));
		        }
		        //if content is found in array, return ACK and zero out
		        //the entry in data array
		        else{
		            bzero(data[dIndex][0],sizeof(data[dIndex][0]));
		            bzero(data[dIndex][1],sizeof(data[dIndex][1]));
		            bzero(data[dIndex][2],sizeof(data[dIndex][2]));
		            sendto(s, ACKMESSAGE, strlen(ACKMESSAGE), 0, (struct sockaddr *) &fsin, sizeof(fsin));
		        }

			break;

			case 'Q':
			printf("%s Quiting\n",RequestPDU.data);
			// check if there are any content stored and
			// de register
			for(int i=0; i<20;i++){
				if( strcmp(data[i][1],"")!=0 && strcmp(data[i][0],RequestPDU.data)==0){
					bzero(data[i][0],sizeof(data[i][0]));
		            		bzero(data[i][1],sizeof(data[i][1]));
		            		bzero(data[i][2],sizeof(data[i][2]));
				}
			}
			memset(&ResponsePDU.data, 0 , sizeof(ResponsePDU.data));
			ResponsePDU.type = 'O';
			sendto(s, &ResponsePDU, strlen(ResponsePDU.data)+1, 0,(struct sockaddr *)&fsin, sizeof(fsin));


			break;

		}
	}
}
