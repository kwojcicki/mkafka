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

    /* printf("Enter your message (Max %d chars) : ", MAX_MESSAGE_LEN); */
    /* gets( message ); */

    int i = 0;
    if (fork() != 0) {
      // Parent will send the message every second
      while (1) {
        i++;
        snprintf(message,10,"%d",i);
        printf("mesage: %s len: %d\n",message, strlen(message));
        send_message(message, strlen(message) + 1, 1);
        sleep(1);
      }
    }
    else {
      // Child will receive the message every 5 seconds
      while (1) {
        char destBuffer[MAX_MESSAGE_LEN];
        receive_message(destBuffer, sizeof(destBuffer), 1, 1);
        printf ("rcv: %s\n", destBuffer);
        sleep(5);
      }
    }

    return 0;
}

