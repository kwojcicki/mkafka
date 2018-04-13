#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/poll.h>
#include <mkafkalib.h>
#include <netinet/tcp.h>

#define myPORT 3505
#define BACKLOG 1
#define MAX_MEMORY 6
#define MAX_TOPICS 16

void doProcessing(int new_fd){
	printf("in here?");
	int pid;
	int topics[MAX_MEMORY];
	char *nullMessage;
	char messages[MAX_MEMORY][1024];
	char buf[1024];
	char buf1[1024];
	struct pollfd fds[1];
	char destBuffer[1024];
	char* topicBuffer = malloc(50 * sizeof(char));

	memset(fds, 0, sizeof(fds));

	for(int i = 0; i < MAX_MEMORY; i++){
		strcpy(messages[i], "");
		topics[i] = -1;
	}

	printf("forking\n");
	pid = fork();
	printf("done forking\n");
	if(pid < 0){
		perror("error forking\n");
		exit(1);
	}

	if(pid == 0){
		printf("in the child process\n");
		while(1){
			while(1){
				fds[0].fd = new_fd;
				fds[0].events = POLLIN;

				int ret = poll(fds, 1, 10 * 1000);

				if(ret == -1){
					perror("poll\n");
					return;
				}

				if(!ret){
					printf("timeout\n");
					break;
				}

				if(fds[0].revents & POLLIN){
					printf("can read\n");
				}

				bzero(buf, 1024);
				read(new_fd,buf,1023);
				if(strcmp(buf, "") == 0){
					break;
				}
				char *pt;
				pt = strtok(buf, ":");
				printf("topic: %s\n", buf);
				// topic
				int topic = atoi(pt);
				printf("topic: %d\n", topic);

				pt = strtok(NULL, "");
				printf("pt %s\n", pt);
				//strcpy(buf, pt);

				if(strcmp(pt, "") == 0){
					break;
				}

				int i = 0;
				for(int m = 0; m < MAX_MEMORY; m++){
					if(strcmp(messages[m],"") == 0){
						strcpy(messages[m], pt);
						topics[m] = topic;
						i = 1;
						printf("set message: %d to %s\n", m, pt);
						send_message(pt, strlen(pt) + 1, topic);
						break;
					}
				}
				if(i == 0){
					int randomNumber = rand() % (MAX_MEMORY);
					strcpy(messages[randomNumber], pt);
					topics[randomNumber] = topic;
					printf("set message: %d to %s\n", randomNumber, pt);
					send_message(pt, strlen(pt) + 1, topic);
				}
			}

			for(int i = 1; i < 2; i++){
				while(1){
					printf("checking topic: %d\n", i);
					// get message from mkafka
					memset(destBuffer, 0, sizeof(destBuffer));
					int status = receive_message(destBuffer, sizeof(destBuffer), i, 3);
					printf("status code: %d message: %s\n", status, destBuffer);
					// message was there waiting
					if(status == 0 || strlen(destBuffer) > 1){
						printf("Got message %s\n", destBuffer);
						// check if we have sent that message
						int flag = 0;
						for(int m = 0; m < MAX_MEMORY; m++){
							printf("Comparing %s to %s, size %d %d\n", messages[m], destBuffer, strlen(messages[m]), strlen(destBuffer));
							if(strcmp(messages[m], "") != 0 &&
									strcmp(messages[m], destBuffer) == 0 &&
									i == topics[m]){
								// messages are the same not sending this one over
								strcpy(messages[m], "");
								topics[m] = -1;
								printf("Message in memory: %s\n", messages[m]);
								flag = 1;
								break;
							}
						}
						if(flag == 0){
							// have not sent that message so will send
							printf("Message not in memory so sending: %s\n", destBuffer);
							snprintf(topicBuffer,50,"%d: %s",i, destBuffer);
							int n = write(new_fd, topicBuffer, strlen(topicBuffer));
							printf("Wrote %d\n", n);
							printf("Wrote %s\n", topicBuffer);
							//n = write(new_fd, destBuffer, strlen(destBuffer));
							//							printf("Wrote %d\n", n);
							//							strcpy(destBuffer, "");
							//							n = write(new_fd, destBuffer, strlen(destBuffer));
							int flagA = 0;
							setsockopt(new_fd, IPPROTO_TCP, TCP_NODELAY, (char*) &flagA, sizeof(int));
						}
					} else {
						break;
					}
				}
			}
			sleep(5);
		}
	} else {
		printf("In the parent process");
	}
}

void doConnect(char* server){
	int sockfd,nbytes;
	char buf[1024];
	struct hostent *he;
	struct sockaddr_in servaddr;

	he=gethostbyname(server);

	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(myPORT);
	servaddr.sin_addr= * ((struct in_addr *)he->h_addr);

	if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0){
		perror("create socket error");
		exit(1);
	}

	if(connect(sockfd,(struct sockaddr *)&servaddr,sizeof(struct sockaddr))==-1){
		perror("connect error\n");
		exit(1);
	}

	printf("do processing client");
	doProcessing(sockfd);
	close(sockfd);
}

int main(int argc, char *argv[]){
	if(argc == 2){
		doConnect(argv[1]);
		return 1;
	}

	int sockfd,new_fd;
	struct sockaddr_in cliaddr;
	struct sockaddr_in servaddr;
	int sin_size;

	if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0){
		perror("create socket failed\n");
		exit(1);
	}

	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family=AF_INET;
	servaddr.sin_port=htons(myPORT);
	servaddr.sin_addr.s_addr= htonl(INADDR_ANY);

	if(bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr))==-1){
		perror("bind error\n");
		exit(1);
	}

	if((listen(sockfd,BACKLOG)==-1)){
		perror("listen error \n");
		exit(1);
	}

	sin_size=sizeof(cliaddr);
	if((new_fd=accept(sockfd,(struct sockaddr *)&cliaddr, (socklen_t *)
			&sin_size))==-1){
		perror("accept error\n");
		exit(1);
	}

	printf("server:got connect from %s\n",inet_ntoa(cliaddr.sin_addr));
	doProcessing(new_fd);

	//close(new_fd);
	//close(sockfd);
}
