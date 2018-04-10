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

    if ( argc != 3) return -1;

    char* umessage = argv[1];
    char* topic = argv[2];
    char* message = malloc(20*sizeof(char));

    int i = 0;
    int rtn = 0;
    // Parent will send the message every second
    while (1) {
      i++;
      snprintf(message,20,"%d: %s",i, umessage);
      rtn = send_message(message, strlen(message) + 1, topic);
      printf("rtn: %d mesage: %s len: %d\n",rtn, message, strlen(message));
      sleep(1);
    }
    return 0;
}

