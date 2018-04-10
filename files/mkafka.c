/* File mailbox.c */
/* Define system calls to the mailbox */

#include "mkafkalib.h"
#include <minix/syslib.h>
#include "glo.h"
#include <string.h>

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
int create_topic(){
    return 1;
}

/* tbd */
int remove_topic(){
    return 1;
}

/* Used to create a new topic
 * Must be space
 * id must be unique
 */
topic_t* add_topic(char* t_id){
    if (mkafka->num_topics < MAX_TOPICS){
        topic_t *tp = mkafka->head;
        if ( tp == NULL ){
            //no topics yet
            tp = malloc(sizeof(topic_t));
            tp->num_messages = 0; 
            tp->next = NULL;
            tp->head = NULL;
            tp->tail = NULL;
            mkafka->head = tp; 
            mkafka->num_topics = 1;

            tp->id = malloc(sizeof(char)*ID_LEN);
            strncpy(tp->id,t_id,strlen(t_id));
            tp->max_messages = MAX_MESSAGE_COUNT;
            //set up the listeners
            tp->groups = malloc(sizeof(group_t));
            tp->groups->message = tp->head;
            tp->groups->next = NULL;
            tp->groups->id = malloc(sizeof(char)*ID_LEN);
            strncpy(tp->groups->id,"tmp",strlen("temp"));
            return tp;
        }
        else {
            //find the next avalible slot,
            while(tp->next != NULL) {
               tp = tp->next;
            }
            topic_t *new_topic = malloc(sizeof(topic_t));
            tp->next = new_topic;
            new_topic->next = NULL;
            new_topic->head = NULL; 
            new_topic->tail = NULL; 
            new_topic->num_messages = 0;
            mkafka->num_topics += 1;

            new_topic->id = malloc(sizeof(char)*ID_LEN);
            strncpy(new_topic->id,t_id,strlen(t_id));
            new_topic->max_messages = MAX_MESSAGE_COUNT;
            //set up the listeners
            new_topic->groups = malloc(sizeof(group_t));
            new_topic->groups->message = new_topic->head;
            new_topic->groups->next = NULL;
            new_topic->groups->id = malloc(sizeof(char)*ID_LEN);
            strncpy(new_topic->groups->id,"tmp",strlen("temp"));
            return new_topic;
        }
    }
    else {
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
    char topic_id[ID_LEN];
    messageLen = (int) m_in.m1_i1;
    // If message size > MAX_MESSAGE_LEN return error
    if (messageLen > MAX_MESSAGE_LEN)
    {
        mutex = 0;
        return MESSAGE_TOO_LONG;
    }

    int messageBytes = messageLen * sizeof(char);
    message = malloc(messageBytes);
    sys_datacopy(who_e, (vir_bytes)m_in.m1_p1, SELF, (vir_bytes)message, messageBytes);
    sys_datacopy(who_e, (vir_bytes)m_in.m1_p2, SELF, (vir_bytes)topic_id, ID_LEN);

    //printf("Mailbox: New message sent. Message content with %d bytes: %s\n", messageBytes, message);

    //If the mkafka doesn't exist, we create it first, then add the topic
    if (!mkafka){
        init_mkafka();
        add_topic (topic_id);
    }

        
    topic_t *tp = mkafka->head;
    while (tp != NULL){
        if (strcmp(tp->id, topic_id) == 0){
            break;
        }
        if (tp->next == NULL){
            if (mkafka->num_topics >= MAX_TOPICS) {
                mutex = 0;
                return MAX_TOPICS;
            }
        }
        tp = tp->next;
    }
    if (tp == NULL){
        tp = add_topic(topic_id);
        if (tp == NULL){
            //max topics reached
            mutex = 0;
            return MAX_TOPICS;
         }
    }
    
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
    else if (tp->num_messages == tp->max_messages){
        //push forwawrds by 1
        group_t *group_iter = tp->groups;
        while (group_iter != NULL){
            if (group_iter->message == tp->tail) group_iter->message = tp->tail->prev;
            group_iter = group_iter->next;
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
    
    //new messages, move up the groups that are empty
    group_t *group_iter = tp->groups;
    while (group_iter != NULL) {
        if (group_iter->message == NULL) group_iter->message = tp->head;
        group_iter = group_iter->next;
    }

    mutex = 0;
    return OK;
}

/* Retrieve a process' messages from the mailbox
 */
int pull_from_topic()
{
    while (mutex==1);
    if (!mkafka){
        return -100;
    }
    mutex = 1; 


    char topic_id[ID_LEN];
    int bufferSize = m_in.m1_i1;
    char group[ID_LEN]; 
    //int group_num = m_in.m1_i3;

    sys_datacopy(who_e, (vir_bytes)m_in.m1_p2, SELF, (vir_bytes)group, ID_LEN);
    sys_datacopy(who_e, (vir_bytes)m_in.m1_p3, SELF, (vir_bytes)topic_id, ID_LEN);
    
    if (bufferSize < MAX_MESSAGE_LEN)
    {
        mutex = 0;
        return MESSAGE_TOO_LONG;
    }
    topic_t *tp = mkafka->head;

    do{
        if (strcmp(tp->id, topic_id) == 0) break; 
        else if (tp->next == NULL) {
            mutex = 0;
            return -20;
        }
        tp = tp->next;
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
        group_t *grp = tp->groups;
        while (grp != NULL){ 
            if (strcmp(grp->id,group) == 0) break;
            if (grp->next == NULL) {
                group_t *new_group = malloc(sizeof(group_t));
                grp->next = new_group;
                new_group->next = NULL;
                new_group->id = malloc(sizeof(char)*ID_LEN);
                strncpy(tp->groups->id,group,strlen(group));
                new_group->message = tp->tail;
                break;
            }
            grp = grp->next; 
        } 

        if (grp->message != NULL) { 
            message_t *m_ptr = grp->message;
            char* temp = "\0";
            int messageBytes = strlen(m_ptr->message) * sizeof(char);
            sys_datacopy(SELF, (vir_bytes)m_ptr->message, who_e, (vir_bytes)m_in.m1_p1, messageBytes);
            sys_datacopy(SELF, (vir_bytes)temp, who_e, (vir_bytes)(m_in.m1_p1 + messageBytes), 1);
            grp->message = grp->message->prev;
        }
        else {
            mutex = 0; 
            return NO_MESSAGE; 
        }
        group_t *grp_itr = tp->groups;
        while (grp_itr != NULL) {
            if (grp_itr->message == tp->tail) break;
            if (grp_itr->next == NULL){
                //remove head
                message_t *new_tail = tp->tail->prev; 
                free(tp->tail->message); 
                free(tp->tail); 
                tp->tail = new_tail; 
                new_tail->next = NULL;
            }
            grp_itr = grp_itr->next; 
        }

    }
    mutex = 0;
    return OK;
}
