/* File mailbox.c */
/* Define system calls to the mailbox */

#include "mkafkalib.h"
#include <minix/syslib.h>
#include "glo.h"


static mkafka_t *mkafka;
static int mutex;


/*
 * Used for debugging purposes
 * Print all messages which are currently in the mailbox

int print_all_messages()
{

    int i;
    message_t *message_ptr = mailbox->head;

    for (i = 1; i <= mailbox->number_of_messages; i++)
    {
         message_ptr = message_ptr -> next;

         pid_node_t *pids = message_ptr-> recipients;
         char *message = message_ptr -> message;

         //printf("**Message number %d\n", i);
         //printf("**Message content %s\n", message);
         //printf("**Recipients: ");

         pids = pids -> next;

         while(pids->pid != -1)
         {
             printf(" %d, ", pids->pid);
             pids = pids -> next;
         }
         printf("\n");
    }

    return 0;
 }
*/

/* initialized mkafka  */
int init_mkafka (){
    mkafka = malloc (sizeof(mkafka_t));
    mkafka->head = NULL;
    mkafka->num_topics = 0; 

    mutex = 0;
    return OK;
}


/* tbd */
int remove_topic(char *id){
    return 1;
}

/* Used to create a new topic
 * Must be space
 * id must be unique
 */
topic_t* add_topic(int id){
    if (mkafka->num_topics < MAX_TOPICS){
        topic_t *tp = mkafka->head;
        if ( tp == NULL ){
            //no topics yet
            tp = malloc(sizeof(topic_t));
            tp->id = id; 
            tp->num_messages = 0; 
            tp->next = NULL;
            tp->head = NULL;
            tp->tail = NULL;
            mkafka->head = tp; 
            mkafka->num_topics = 1;
        }
        else {
            //find the next avalible slot,
            while(tp->next != NULL) {
               tp = tp->next;
            }
            topic_t *new_topic = malloc(sizeof(topic_t));
            tp->next = new_topic;
            new_topic->next = NULL;
            new_topic->id = id;
            new_topic->head = NULL; 
            new_topic->tail = NULL; 
            new_topic->num_messages = 0;
            tp = new_topic;
            mkafka->num_topics += 1;
        }
        //set up the listeners
        for (int i = 0; i < MAX_GROUPS; i++){
            tp->groups[i] = tp->head;
        }
        return tp;
    }
    else {
        printf ("Too Many topics\n");
        return NULL;
    }
}


/* Creates mailbox if it is missing
 * Add message to mailbox (if mailbox is not full)
 * Returns OK if message was successfully added
 * Returns ERROR if mailbox is full
 */
int push_to_topic()
{
    while (mutex==1);
    mutex = 1; 

    char* message;
    int messageLen;
    int topic_id;
    // If message size > MAX_MESSAGE_LEN return error
    messageLen = m_in.m1_i1;
    topic_id = m_in.m1_i2;
    printf("message: %s, topic: %d, len: %d\n",message,topic_id,messageLen);
    if (messageLen > MAX_MESSAGE_LEN)
    {
        printf("Error: received message size exceeds %d chars\n", MAX_MESSAGE_LEN);
        mutex = 0;
        return MESSAGE_TOO_LONG;
    }

    int messageBytes = messageLen * sizeof(char);
    message = malloc(messageBytes);
    sys_datacopy(who_e, (vir_bytes)m_in.m1_p1, SELF, (vir_bytes)message, messageBytes);

    //printf("Mailbox: New message sent. Message content with %d bytes: %s\n", messageBytes, message);

    //If the mkafka doesn't exist, we create it first, then add the topic
    if (!mkafka){
        init_mkafka();
        add_topic (topic_id);
        printf("created mkafka\n");
    }

        
    topic_t *tp = mkafka->head;
    while (tp != NULL){
        if (tp->id == topic_id){
            break;
        }
        if (tp->next == NULL){
            if (mkafka->num_topics >= MAX_TOPICS) {
                printf ("Max topics reached, can't add new topic/n");
                return MAX_TOPICS;
            }
        }
        tp = tp->next;
    }
    if (tp == NULL){
        tp = add_topic(topic_id);
        if (tp == NULL){
            //max topics reached
            return MAX_TOPICS;
         }
    }
    printf("Found topic\n");
    
    message_t *new_message = malloc(sizeof(message_t));
    new_message->message = message;


    new_message->next = tp->head;
    new_message->prev = NULL;
    //no messages in queue
    if (tp->num_messages == 0){
        tp->tail = new_message;
        tp->head = new_message;
        tp->tail->next = NULL;
        tp->head->next = NULL;
        tp->tail->prev = NULL;
        tp->head->prev = NULL;
    }
    //1 message in queue
    else if ( tp->num_messages == 1) { 
        tp->head = new_message; 
        tp->head->next = tp->tail; 
        tp->tail->prev = tp->head;
    }
    //max messages in queue (have to move all indexs back by one)
    else if (tp->num_messages == MAX_MESSAGE_COUNT){
        //push forwawrds by 1
        for (int i = 0; i < MAX_GROUPS; i++){
            if (tp->groups[i] == tp->tail ) tp->groups[i] = tp->tail->prev;
        }
        message_t *new_tail = tp->tail->prev;
        tp->tail->prev->next = NULL;

        free(tp->tail->message);
        free(tp->tail);
        tp->tail = new_tail;
        tp->num_messages -=1;
        tp->head->prev = new_message;
    }
    //some messages in queue
    else {
        tp->head->prev = new_message;
        new_message->next = tp->head;
    }
    tp->head = new_message;
    tp->num_messages += 1;

    printf("Message Added\n");
    for (int i = 0; i < MAX_GROUPS; i++){
        //if a group is NULL then it has seen all of the older messages -> point to newest
        if (tp->groups[i] == NULL ) tp->groups[i] = tp->head;
    }
    printf ("Message sent\n");

    mutex = 0;
    return OK;
}

/* Retrieve a process' messages from the mailbox
 */
int pull_from_topic()
{
    while (mutex==1);
    if (!mkafka){
        return ERROR;
    }
    mutex = 1; 


    int topic_id = m_in.m1_i1;
    int bufferSize = m_in.m1_i2;
    int group_num = m_in.m1_i3;
    
    if (bufferSize < MAX_MESSAGE_LEN)
    {
        printf("Error: insufficient buffer size, should be %d chars\n", MAX_MESSAGE_LEN);
        mutex = 0;
        return MESSAGE_TOO_LONG;
    }
    topic_t *tp = mkafka->head;

    do{
        if (tp->id == topic_id) break; 
        else if (tp->next == NULL) {
            printf ("Couldn't find topic\n");
            return ERROR;
        }
    } while (tp != NULL);
    // Return error if there are no messages in the mailbox
    if (!tp || tp->num_messages == 0)
    {
        //printf("Error: topic is empty or has not been created\n");
        mutex = 0;
        return NO_TOPIC;
    }
    else
    {
        if (tp->groups[group_num] != NULL) { 
            message_t *m_ptr = tp->groups[group_num];
            char* temp = "\0";
            int messageBytes = strlen(m_ptr->message) * sizeof(char);
            sys_datacopy(SELF, (vir_bytes)m_ptr->message, who_e, (vir_bytes)m_in.m1_p1, messageBytes);
            sys_datacopy(SELF, (vir_bytes)temp, who_e, (vir_bytes)(m_in.m1_p1 + messageBytes), 1);
            tp->groups[group_num] = tp->groups[group_num]->prev;

        }
        else {
            mutex = 0; 
            return NO_MESSAGE; 
        }
        for (int i = 0; i < MAX_GROUPS; i++) {
            if (tp->groups[i] == tp->tail) break;
            if (i == MAX_GROUPS -1){
                //remove head
                message_t *new_tail = tp->tail->prev; 
                free(tp->tail->message); 
                free(tp->tail); 
                tp->tail = new_tail; 
                new_tail->next = NULL;
            }
        }

    }
    // In case of not find a message for the recipient return error
    mutex = 0;
    return ERROR;
}
