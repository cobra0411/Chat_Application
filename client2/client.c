#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h> //defines hostent structure is defined here (info about host stored)
#include<unistd.h> //require for read write and close function
#include<ctype.h>


void error(const char* msg){
	perror(msg);
	exit(1);
}

int main(int argc, char* argv[]){
	int sockfd, portno, n;

	struct sockaddr_in serv_addr;
	struct hostent *server;

	char buffer[255];

	if(argc < 3){
		fprintf(stderr,"usage %s hostname port\n", argv[0]);
	}

	portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if(sockfd < 0)
		error("ERROR opening socket");

	//get server if available with given ip address
	server  = gethostbyname(argv[1]);

	if(server == NULL){
		fprintf(stderr, "%s\n", "Error, no such host");
		exit(1);
	}

	bzero((char *)&serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;

	//bcopy function copies from server to serv_addr
	bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
	
	serv_addr.sin_port = htons(portno);

	//connect to server
	if(connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0){
		error("Connection Failed");
	}

	//greeting message
	//bzero(buffer, 255);
	//n = read(sockfd, buffer,255);
	//if(n < 0){
	//	error("Error on read");
	//}
	//printf("Server says: %s\n",buffer);


	printf("enter your choice:\n ");
	printf("1. send\n");
	printf("2. receive\n");
	printf("3. upload\n");
	printf("4. download\n");
	printf("5. exit\n");

	while(1){
		//reply to server
		
		int choice;
		scanf("%d",&choice);

		send(sockfd, &choice, sizeof(choice), 0);
		fflush(stdin);

		switch(choice){
		case 1:
			bzero(buffer,255);
			fgets(buffer, 255, stdin);
			fgets(buffer, 255, stdin);
			send(sockfd, &buffer, 255,0);
			/*if(n < 0)
				error("Error on send");*/
			break;
		case 2:
			bzero(buffer,255);
			recv(sockfd, &buffer, 255,0);
			/*if(n < 0){
				error("Error on recieve");
			}*/
			printf("Server: %s\n",buffer);
			break;
		case 3:
			/* write upload logic here*/
			{	FILE *f;
				int words = 0;

				char c;

				char filename[255];
				bzero(filename,255);
				printf("Enter file name: ");
				scanf("%s", filename);

				send(sockfd, &filename, 255, 0); //send file name to be ulploaded

				f = fopen(filename, "r"); //open the file

				if(f == NULL){
					//if file not found
					words = -1;
					send(sockfd, &words, sizeof(int), 0);
				}
				else{
					//count number of words
					rewind(f);
					while((c = getc(f)) != EOF){
						fscanf(f,"%s",buffer);
						if(isspace(c) || c == '\t')
						words++;
					}

					send(sockfd, &words, sizeof(int), 0);
					rewind(f); //bring file pointer to front

					char ch;
					while(ch != EOF){
						fscanf(f, "%s", buffer);
						send(sockfd, &buffer, 255, 0);
						ch = fgetc(f);
					}

					fclose(f);
				}

				printf("successfully uploaded file %s\n",filename);
		
			break;
		}
		case 4:
			/* write download logic here*/
			{
				FILE *f;
				int words = 0;

				int ch = 0;

				char filename[255];

				printf("Enter filename to be downloaded: ");
				bzero(filename,255);
				scanf("%s",filename);
				send(sockfd, &filename, 255, 0); //send the file name to be downloaded

					
				recv(sockfd, &words, sizeof(int), 0); //recieve the number of words

				if(words == -1){
					printf("File not found\n");
				}

				else{
					f = fopen(filename, "w"); //open file in write mode
					rewind(f);
					while(ch != words){
						recv(sockfd, &buffer, 255,0);
						fprintf(f, "%s ", buffer);
						ch++;
					}
					fclose(f);
					printf("successfully downloaded file %s\n",filename);
				}
			break;
			}
		case 5:
			close(sockfd);
			return 0;
			break;
		default:
			printf("wrong choice\n");

		}
	}
	return 0;
}
