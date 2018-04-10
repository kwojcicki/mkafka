/* ================================================= *
 *     Test: Testing multiple senders and receivers
 *     Sender: sends message every second
 *     Receiver: receives message every 5 seconds
 * ================================================= */

#include <stdlib.h>
#include <stdio.h>
#include <lib.h>
#include <string.h>
#include <mkafkalib.h>
#include <unistd.h>

int main(int argc, char* argv[])
{

    char *message = malloc(sizeof(char)*10);
    message = "TEST";

    char tid [10] = "TEST\0";
    char *gid = malloc(sizeof(char)*10);
    //char *tid = malloc(sizeof(char)*10);
    gid = "tess";
    //tid = "ok";
    /* printf("Enter your message (Max %d chars) : ", MAX_MESSAGE_LEN); */
    /* gets( message ); */

    int i = 0;
    if (fork() != 0) {
      // Parent will send the message every second
      while (1) {
        i++;
        int rtn =0;
        snprintf(message,10,"%d",i);
        rtn = send_message(message, strlen(message) + 1, tid);
        printf("rtn: %d mesage: %s len: %d\n",rtn, message, strlen(message));
        sleep(1);
      }
    }
    else {
      // Child will receive the message every 5 seconds
      while (1) {
        int rtr = 0; 
        char destBuffer[MAX_MESSAGE_LEN];
        rtr = receive_message(destBuffer, sizeof(destBuffer), tid, gid);
        printf ("rtn = %d rcv: %s\n",rtr,  destBuffer);
        sleep(5);
      }
    }

    return 0;
}

