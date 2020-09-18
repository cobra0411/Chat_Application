/*
	Author : Ajit Kumar
*/
#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<errno.h>
#include<string.h>
#include<sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include<ctype.h>

#define TRUE 1
#define FALSE 0

void error(char* str){
	perror(str);
	exit(1);
}

int main(int argc, char* argv[]){
	if(argc < 2){
		fprintf(stderr, "%s\n", "PORT NUMBER REQUIRED. PROGRAM TERMINATED");
	}

	int portno = atoi(argv[1]);
	char message[255] = "HELLO CLIENT";
	int master_socket; //server socket descriptor
	int new_socket; //store new file descriptor after accrpt
	socklen_t clilen;

	int client_socket[30]; //socket descriptor for 30 clients
	int max_clients = 30; //maximum clients

	int opt = TRUE;

	struct sockaddr_in serv_addr, cli_addr;
	int n;

	char buffer[255]; //to store messages

	//set of socket descriptors
	fd_set readfds;

	int activity, i, valread, sd, max_sd;

	//initialise all client sockets to 0 so not checked
	for(int i = 0;i< max_clients;i++){
		client_socket[i] = 0;
	}

	master_socket = socket(AF_INET, SOCK_STREAM, 0);

	if(master_socket < 0){
		error("Socket not created");
	}

	//clear ser_addr
	bzero((char *)&serv_addr,sizeof(serv_addr));

	//set the master socket to allow multiple connections
	if(setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0){
		error("setsockpot");
	}

	//set socket attributes
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	//bind
	if(bind(master_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
		error("bind error");
	}

	printf("Listener on port %d \n", portno);

	//listen
	if(listen(master_socket, 10) < 0){
		error("Listen error");
	}

	//accept incloming call

	clilen = sizeof(cli_addr);

	while(TRUE){
		//clear the socket set
		FD_ZERO(&readfds);

		//add master socket to set
		FD_SET(master_socket, &readfds);
		max_sd = master_socket;

		//add child socket to set
		for(i = 0; i< max_clients; i++){
			sd = client_socket[i];

			//if a valid socket descriptor then add to read list
			if(sd > 0){
				FD_SET(sd, &readfds);
			}

			//highest file descriptor number, need it for the select function
			if(sd > max_sd)
				max_sd = sd;
		}


		//wait for an activity on one of the sockets, timeout is NULL
		//so wait indefinitely

		activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

		if((activity < 0) && (errno != EINTR)){
			fprintf(stderr, "%s\n", "select error");
		}


		//if something happened on the master socket
		//then its an incoming connection

		if(FD_ISSET(master_socket, &readfds))
		{

			if((new_socket = accept(master_socket, (struct sockaddr *)&cli_addr,&clilen)) < 0){
				error("accept error");
			}

			//inform user of socket number- used send and recieve commands
			printf("New Connection, socket fd is %d, ip is : %s, port : %d \n", new_socket, inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

			//send new connection greeting message
			/*bzero(buffer,255);
			fgets(buffer,255,stdin);
			if(write(new_socket, buffer, 255) < 0){
				error("write error");
			}*/


			//add new socket to array of sockets
			for(i =0;i<max_clients;i++){
				if(client_socket[i] == 0){
					client_socket[i] = new_socket;
					break;
				}
			}
		}
		//if ends

		//else its some of IO operations on some other socket
		for(i =0;i<max_clients; i++){
			sd = client_socket[i];

			if(FD_ISSET(sd, &readfds)) //if client is in the set
			{
				//check if it was for closing and read the incoming message
				//get the message from the client

				int choice;
				recv(sd,&choice,sizeof(choice),0);
				//printf("%d\n",choice);
				switch(choice){
				case 1:
					bzero(buffer, 255);
					recv(sd, &buffer, 255,0);

					/*if(n < 0){
						error("Error on recieve");
					}*/
					puts(buffer);
					printf("Client %d says: %s \n",i+1, buffer);
					break;
				case 2:
					send(sd,&buffer,255,0);
					/*if(n < 0){
						error("error on send");
					}*/
					break;
				case 3:
					/* file upload logic */
					{FILE *f;
					int words = 0;

					int ch = 0;

					char filename[255];
					bzero(filename,255);
					recv(sd, &filename, 255, 0); //receive the file name to be uploaded

					recv(sd, &words, sizeof(int), 0); //recieve the number of words

					if(words == -1){
						printf("File can not be uploaded (file doesn't exist)\n");
					}
					else{
						f = fopen(filename,"w");
						rewind(f);
						while(ch != words){
						recv(sd, &buffer, 255,0);
						fprintf(f, "%s ", buffer);
						ch++;
						}
						fclose(f);
						printf("Client %d successfully uploaded file %s\n",i+1,filename);
				
					}
					break;
				}
				case 4:
					/* file download logic */
				{	FILE *f;
					int words = 0;

					char c;

					char filename[255];
					bzero(filename,255);

					recv(sd, &filename, 255, 0); //receive file name to be downloaded

					f = fopen(filename, "r"); //open the file

					if(f == NULL){
						//if file not found
						words = -1;
						send(sd, &words, sizeof(int), 0);
					}
					else{
						//count number of words
						rewind(f);
						while((c = getc(f)) != EOF){
							fscanf(f,"%s",buffer);
							if(isspace(c) || c == '\t')
								words++;
						}

						send(sd, &words, sizeof(int), 0);
						rewind(f); //bring file pointer to front

						char ch;
						while(ch != EOF){
							fscanf(f, "%s", buffer);
							send(sd, &buffer, 255, 0);
							ch = fgetc(f);
						}

						fclose(f);
					}

					printf("Client %d successfully downloaded file %s\n",i+1,filename);
					break;
				}

				case 5:
					getpeername(sd, (struct sockaddr *)&cli_addr, &clilen);
					printf("Client %d disconnected\n",i+1);
					printf("Host disconnected, ip %s, port %d \n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));


					//close the socket and mark as 0 in the list for reuse
					close(sd);
					client_socket[i] = 0;
					break;
				}
			}
		}
	}
	close(master_socket);
	return 0;
}
