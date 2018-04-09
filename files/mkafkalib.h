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
#define NO_GROUP -6
#define MAX_MESSAGE_LEN 1024
#define MAX_TOPICS 16
#define MAX_GROUPS 4
#define ID_LEN 5

void mkafka_error(int error){
    switch (error){
        case ERROR:
            printf("Unspecified error occured\n");
            break;
        case NO_TOPIC:
            printf("Error no topic\n");
            break;
        case MAX_TOPICS:
            printf("Error max topics reached\n");
            break;
        case MESSAGE_TOO_LONG:
            printf("Error message too long\n");
            break;
        case NO_MESSAGE:
            printf("Error no message\n");
            break;
        case NO_GROUP:
            printf("Error no group\n");
            break;
    }
}


/* Message
 * message - the message, null terminated max 1024
 */

typedef struct message_struct {
    char *message;
    struct message_struct *prev;
    struct message_struct *next;
} message_t;

typedef struct group_struct{
    char *id;
    message_t *message;
    struct group_struct *next;
} group_t;


/* topic
 * number_of_messages - current number of messages in the mailbox (limit is 16)
 * head - pointer to list of messages (head)
 * id - identifier for the topic
 */

typedef struct topic_struct {
  group_t *groups;
  char *id;
  int num_messages;
  message_t *head;
  message_t *tail;
  int max_messages;
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

int send_message(char *messageData, size_t messageLen, char* topic_id)
{
    message m;
    //char recipientsString[128];  // 6 [5 from pid + 1 from separator] * 16, will be always lower than 128
    //snprintf(recipientsString, 128, "%d", recipients[recipientsLen-1]);
    //printf("User: Mapping message %s to pids %s\n", messageData, recipientsString);

    m.m1_p1 = messageData;
    m.m1_p2 = topic_id;
    m.m1_i1 = (int) messageLen + 1;
    return(_syscall(PM_PROC_NR, PM_MK_PUSH, &m));
}



int receive_message(char *destBuffer, size_t bufferSize ,char* topic_id, char* group)
{
    message m;
    m.m1_p1 = destBuffer;
    m.m1_p2 = group;
    m.m1_p3 = topic_id;
    m.m1_i1 = (int) bufferSize;
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

