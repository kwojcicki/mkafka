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
	struct tms time1, time2;

	char message[20];
	long start,end;
	struct timespec timecheck, timecheck1;
	memset(message, 0, sizeof(message));
	strcpy(message, "test");

	clock_gettime(CLOCK_REALTIME, &timecheck);

	send_message(message, strlen(message) + 1, 1);

	printf("finished sending\n");

	for(int i = 0; i < 2; i++){
		while(1){
			char destBuffer[MAX_MESSAGE_LEN];
			memset(destBuffer, 0, sizeof(destBuffer));
			int status = receive_message(destBuffer, sizeof(destBuffer), 1, 4);
			if((status == 0 || strlen(destBuffer) > 1)){
				printf("Received message %d %s\n", i, destBuffer);
				break;
			} else {
				usleep(100000);
			}
		}
	}

	clock_gettime(CLOCK_REALTIME, &timecheck1);
	double accum = (timecheck1.tv_sec - timecheck.tv_sec) + (timecheck1.tv_nsec - timecheck.tv_nsec) / BILLION;
	printf("nsec before: %ld\n", timecheck.tv_nsec);
	printf("nsec after: %ld\n", timecheck1.tv_nsec);
	printf("sec before: %lld\n", timecheck.tv_sec);
	printf("sec after: %lld\n", timecheck1.tv_sec);
	printf("total time in seconds %lf\n", accum);
//	end = (long) timecheck1.tv_sec * 1000 + (long)timecheck1.tv_usec / 1000;
//	printf("\n%ld milliseconds elapsed\n", (end - start));
//	printf("usec: %d", timecheck.tv_usec);
//	printf("usec: %d", timecheck1.tv_usec);
//	printf("sec: %lld", timecheck.tv_sec);
//	printf("sec: %lld", timecheck1.tv_sec);

	return 0;

}
