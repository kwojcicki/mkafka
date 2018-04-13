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
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/times.h>
#include <time.h>

#define BILLION 1E9

int main(int argc, char *argv[])
{

	int status = 0;

	char message[20];
	memset(message, 0, sizeof(message));
	strcpy(message, "test1");


	for(int i = 0; i < 7; i++){
		while(1){
			char destBuffer[MAX_MESSAGE_LEN];
			memset(destBuffer, 0, sizeof(destBuffer));
			int status = receive_message(destBuffer, sizeof(destBuffer), 1, 4);
			if((status == 0 || strlen(destBuffer) > 1) && strcmp(destBuffer, "test") == 0){
				printf("Got message %s\n", destBuffer);
				send_message(message, strlen(message) + 1, 1);
				break;
			} else {
				usleep(100000);
			}
		}
	}

	return 0;

}
