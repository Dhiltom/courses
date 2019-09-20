
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <stdbool.h>


//structure of SBCP Attribute
struct SBCP_attribute{
	uint16_t type;
	uint16_t length;
	char payload[512];
};

//structure of SBCP Message
struct SBCP_message{
	uint8_t vrsn; 
	uint8_t type; 
	uint16_t length;
	struct SBCP_attribute attribute[2];
};


//to have a mapping of client fd and name
struct client_data {
	int fd;
	char client_name[16];
};

bool check_username (char name[], char *usernames[], int client_count) {

	for (int i = 0; i <= client_count; i++ ) {
		
		if (strcmp(name,usernames[i]) == 0 ) {
			return false;
		}
	}

	return true;
}


void main (int argc, char* argv[]) {
	
	// Checking for 3 command-line arguments
	
    if(argc != 4) {
        fprintf(stderr, "Usage: ./server Server_IP Server_Port Max_Clients \n");
        exit(1);
    }           
	
	int max_clients = atoi(argv[3]);
	int client_count = 0;
	char *usernames[max_clients];
	int x,k;
	struct client_data client[max_clients];
	struct addrinfo hints;
    struct addrinfo *servinfo; 
    memset(&hints, 0, sizeof hints); // to make sure that the struct is empty
    hints.ai_family = AF_UNSPEC;     // IPv4 or IPv6 address
    hints.ai_socktype = SOCK_STREAM; // for TCP 
	hints.ai_flags = AI_PASSIVE; 
	
	fd_set master;
    fd_set read_set;  
    FD_ZERO (&master);// Clear all entries in master
    FD_ZERO (&read_set);// Clear all entries in read_set
	
	if ((x = getaddrinfo(argv[1], argv[2] , &hints, &servinfo)) != 0) {
        fprintf(stderr, " getaddrinfo error : %s\n", gai_strerror(x));
        exit(1);
    }
    
	//socket inititialization
	
	int sockfd;
	if ( (sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) < 0 ) {
		fprintf(stderr, "%s\n",strerror(errno));
        exit(1);
	} else {
		printf("\n Socket created ! \n");
	}
	
	int r = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &r, sizeof(int)) < 0) {
        fprintf(stderr, " %s\n",strerror(errno));
        exit(1);
    }
	
	//binding socket to port & IP
	
	if( bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) < 0){
		fprintf(stderr, " %s\n",strerror(errno));
        exit(1);
    } else {	
		printf(" Socket binding done ! \n");
	}
    
	//listening for connections
	
	if (listen(sockfd , max_clients) < 0) {
        fprintf(stderr, " %s\n",strerror(errno));
        exit(1);
	} else {	
		printf(" Listening for clients ! \n");
	}
	
	
	FD_SET(sockfd,&master);
	int fdmax = sockfd;
	
	struct sockaddr_storage client_addr; //client's address
	//client_addr = (struct sockaddr *)malloc(sizeof(struct sockaddr));
	
	while(1) {
		
		read_set = master;
		
		if(select(fdmax + 1, &read_set, NULL, NULL, NULL) < 0 ){
            fprintf(stderr, " %s\n",strerror(errno));
			exit(1);
        }
	
		int t;
		
		for (int i = 0; i <= fdmax; i++) { // looping through all the file descriptors
			printf(" %d \n",i);
			if(FD_ISSET(i, &read_set)) {  // getting one file descriptor
				
				if ( i == sockfd ) {  // server socket, check for new connections
					printf(" i == sockfd \n\n\n");
					socklen_t client_size = sizeof client_addr;
					int client_fd;
					
					if ((client_fd = accept(sockfd, (struct sockaddr *)&client_addr , &client_size )) < 0 ) {
						printf(" %d \n\n\n",client_fd);
                        fprintf(stderr, " %s\n",strerror(errno));
                        //exit(1);
						
					} else {	
						printf(" accept else %d \n\n\n",client_fd);
						FD_SET(client_fd, &master);
                        
						t = fdmax;
						
                        if(client_fd > fdmax) { 
                            fdmax = client_fd;
                        }
						
						client_count++;
						printf(" client_count: %d Max : %d\n\n\n",client_count, max_clients);
						
						if ( client_count > max_clients ) {  //reject client connection when count exceeded
							printf(" client_count > max_clients ");
							fdmax = t;
							printf(" Maximum client limit (%d) reached ! \n", max_clients);
							FD_CLR(client_fd, &master);
							//NAK(client_fd);
							client_count--;
							
						} else {
							
							printf(" client_count > max_clients else \n ");
							printf(" client_count: %d Max : %d\n\n\n",client_count, max_clients);
							struct SBCP_message join;
							char name[16];
							
							read(client_fd,(struct SBCP_message *) &join,sizeof(join));
							printf(" Read         done ! %s , %d\n ",join.attribute[0].payload,join.type);
							
							if ( join.type == 2 ) { //checking if it is Join message
							
								strcpy(name,join.attribute[0].payload);  //copy username to name
							
							}
							printf(" Name : %s \n",name);
							if (check_username(name, usernames, client_count)) {
							
								printf(" %d Client Accepted ! \n", client_fd);
								//client_count++;
								strcpy(usernames[client_count],name);
								//ACK(client_fd);
								
								struct SBCP_message online;
								online.vrsn = 3;
								online.type = 8;
								online.attribute[0].type = 2;
								strcpy(online.attribute[0].payload, name);
								
								//sending online message to all clients except current one and server
								
								for (k = 0; k <= fdmax; k++) {
								
									if (FD_ISSET(k,&master)) {
										
										if (k != client_fd && k != sockfd) {
										
											if ((write(k,(void *) &online,sizeof(online))) < 0 ){
												
            	                            	fprintf(stderr, " %s\n",strerror(errno));
												
            	                            }
										
										}
										
									}
								
								}
							
								client[client_count].fd = i;
								strcpy(client[client_count].client_name,online.attribute[0].payload);
							
							} else {
							
								printf(" User with name : %s already exists, use another name...", name);
								//NAK(client_fd);
								client_count--;
							
							}
												
						
						}
						
					
					}
					
				} else {  // one of the client sockets, handle the data from it 
						
					struct SBCP_message reads;
					int num_bytes;
					num_bytes = read(i,(struct SBCP_message *) &reads,sizeof(reads));
					
					if (num_bytes < 0) {
						
						for (k = i; k < client_count; k++) {
							
							client[k] = client[k+1];
							
						}
						
						FD_CLR(i,&master);
						close(i);
						client_count--;
						
					} else if (num_bytes == 0) {
						
						struct SBCP_message offline;
						offline.vrsn = 3;
						offline.type = 6;
						offline.attribute[0].type = 2;
						
						for (k = 0; k <= client_count; k++) {
							
							if (i == client[k].fd) {
								
								strcpy(offline.attribute[0].payload, client[k].client_name);
							
							}
							
						}
						
						printf("User : %s has left the chat \n\n",offline.attribute[0].payload);
						
						//sending offline message to all clients except current one and server
								
						for (k = 0; k <= fdmax; k++) {
					
							if (FD_ISSET(k,&master)) {
										
								if (k != i && k != sockfd) {
										
									if ((write(k,(void *) &offline,sizeof(offline))) < 0 ){
												
                                       	fprintf(stderr, " %s\n",strerror(errno));
												
									}
										
								}
										
							}
								
						}
						
						for (k = i; k < client_count; k++) {
							
							client[k] = client[k+1];
							
						}
						
						FD_CLR(i,&master);
						close(i);
						client_count--;
							
					} else { //num_bytes > 0, so we forward it to all other clients
						
						struct SBCP_message fwd;
						fwd.vrsn = 3;
						fwd.type = 3;
						fwd.attribute[0].type = 4;
						fwd.attribute[1].type = 2;
						
						strcpy(fwd.attribute[0].payload, reads.attribute[0].payload);
						
						for (k = 0; k <= client_count; k++) {
							
							if (i == client[k].fd) {
								
								strcpy(fwd.attribute[1].payload, client[k].client_name);
							
							}
							
						}
						
						//forwarding message to all clients except current one and server
						
						for (k = 0; k <= fdmax; k++) {
					
							if (FD_ISSET(k,&master)) {
										
								if (k != i && k != sockfd) {
										
									if ((write(k,(void *) &fwd,sizeof(fwd))) < 0 ){
												
                                       	fprintf(stderr, " %s\n",strerror(errno));
												
									}
										
								}
										
							}
								
						}
						
						
					} //end of forwarding data from client
					
				} //end of handling data from client
				
			} //end of getting file descriptor
			
		} // end of for loop 
	
	} //end of while
	
	
	close(sockfd);
	
} //main