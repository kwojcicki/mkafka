/* ================================================= *
 * Define prototypes for message and mailbox structs *
 * ================================================= */

#ifndef _MKAFKALIB_H_
#define _MKAFKALIB_H_

#include <stdlib.h>
#include <stdio.h>
#include <lib.h>
#include <string.h>

#define MAX_MESSAGE_COUNT 16
#define OK 0
#define ERROR -1
#define NO_TOPIC -2
#define MAX_TOPICS -3
#define MESSAGE_TOO_LONG -4
#define NO_MESSAGE -5
#define MAX_MESSAGE_LEN 1024
#define MAX_TOPICS 16
#define MAX_GROUPS 16

void mkafka_error(int error){
}


/* Message
 * message - the message, null terminated max 1024
 */

typedef struct message_struct {
    char *message;
    struct message_struct *prev;
    struct message_struct *next;
} message_t;

/* topic
 * number_of_messages - current number of messages in the mailbox (limit is 16)
 * head - pointer to list of messages (head)
 * id - identifier for the topic
 */

typedef struct topic_struct {
  message_t *groups [MAX_GROUPS];
  int id;
  int num_messages;
  message_t *head;
  message_t *tail;
  struct topic_struct *next;
} topic_t;

/* mkafka
 * nume_topics - current number of topics, (limit is 16) 
 */

typedef struct {
  topic_t *head;
  int num_topics;
} mkafka_t;

int init_mkafka();
int remove_topic();
topic_t* add_topic();
int init_msg_pid_list(message_t *m);

int send_message(char *messageData, size_t messageLen, int topic_id)
{
    message m;
    //char recipientsString[128];  // 6 [5 from pid + 1 from separator] * 16, will be always lower than 128
    //snprintf(recipientsString, 128, "%d", recipients[recipientsLen-1]);
    //printf("User: Mapping message %s to pids %s\n", messageData, recipientsString);

    m.m1_p1 = messageData;
    m.m1_i2 = topic_id;
    m.m1_i1 = (int) messageLen + 1;
    return(_syscall(PM_PROC_NR, PM_MK_PUSH, &m));
}



int receive_message(char *destBuffer, size_t bufferSize ,int topic_id, int group)
{
    message m;
    m.m1_p1 = destBuffer;
    m.m1_i3 = group;
    m.m1_i1 = topic_id;
    m.m1_i2 = (int) bufferSize;
    int status = _syscall(PM_PROC_NR, PM_MK_PULL, &m);
    /*
    if (status == ERROR)
    {
        printf("ERROR: There is no message for the topic/group\n");
    }
    else
    {
        printf("User: message (%d bytes) \"%s\" received\n", m.m1_i2, destBuffer);
    }
    */
    return status;
}

#endif

