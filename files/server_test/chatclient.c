#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <net/hton.h>
#include <sys/socket.h>
#include <unistd.h>
#include <mkafkalib.h>
#include <sys/wait.h>
#include <sys/poll.h>

int main(int argc, char *argv[])
{

    int topic = atoi(argv[1]);
    int group = atoi(argv[2]);

	char message[20];
	int i = 1;
	struct pollfd input[1] = {{fd: 0, events: POLLIN}};
	int ret_pol = -1;
	while(1){

    	memset(message, 0, sizeof(message));
		ret_pol = poll(input, 1, 100);
		if(ret_pol){
			read(0, message, 20);
			if(strcmp(message, "") != 0 && strlen(message) > 1){
				message[strcspn(message, "\n")] = 0;
				printf("sending %s\n", message);
				send_message(message, strlen(message) + 1, topic);
			}
		}
    	memset(message, 0, sizeof(message));

		char destBuffer[MAX_MESSAGE_LEN];
    	memset(destBuffer, 0, sizeof(destBuffer));
		// get message from mkafka
		int status = receive_message(destBuffer, sizeof(destBuffer), topic, group);
		if(status == 0 || strlen(destBuffer) > 1){
			printf("Got message %s\n", destBuffer);
			printf("Status %d\n", status);
		}

	}
}
