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

    char* topic = argv[1];
    char* group = argv[2];

    char tp [10];
    char grp [10];

    strncpy (tp, topic, strlen(topic));
    strncpy (grp, group, strlen(group));
    char* message = malloc(20*sizeof(char));
    int rtn =0;
    int i = 0;
    // Parent will send the message every second
    while (1) {
        // Child will receive the message every 5 seconds
        char destBuffer[MAX_MESSAGE_LEN];
        rtn = receive_message(destBuffer, sizeof(destBuffer), topic, group);
        
        printf ("rtn: %d rcv: %s\n",rtn,  destBuffer);
        sleep(2); 
    }
    return 0;
}

