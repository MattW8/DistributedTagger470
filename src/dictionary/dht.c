
#include <mpi.h>
#include <pthread.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "../tagger/tags.h"
#include "dictionary_generator.h"
#include "../corpus/corpus_io.h"
#include "dht.h"

//message types
#define INIT 0
#define PUT 1
#define GET 2
#define ACCEPTED 4
#define ERROR 5
#define TERMINATE 6

//tag definitions
#define SERVER 7
#define CLIENT 8

//struct to hold messages
struct msg
{
    int type; //type of message sent
    char key[MAX_KEYLEN]; //key, if applicable
    int value; //tag, if applicable
};

static int pid; //current process id
int nprocs; //current number of processes
pthread_t *thread_handle; //server thread handle
static pthread_mutex_t local_mutex = PTHREAD_MUTEX_INITIALIZER; //mutex to protect local functions

void* server(void* rank)
{   
    MPI_Status status; //status of messages recieved, used to send messages back

    struct msg response_msg; //message that is sent back
   
    struct msg server_msg; //message received
    server_msg.type = INIT;

    //loop infinitely untill i is changed
    int i = 1;
    while( i == 1 )
    {
        //recieve next message
        MPI_Recv( &server_msg, sizeof(struct msg), MPI_BYTE, MPI_ANY_SOURCE, SERVER, MPI_COMM_WORLD, &status );
        
        if( server_msg.type == PUT )
        {
            //if type is put
            pthread_mutex_lock(&local_mutex);
            local_put(server_msg.key, server_msg.value);
            pthread_mutex_unlock(&local_mutex);

            //prepare response
            response_msg.type = ACCEPTED;

            MPI_Ssend( &response_msg, sizeof(struct msg), MPI_BYTE, status.MPI_SOURCE, CLIENT, MPI_COMM_WORLD );
        }
        else if( server_msg.type == GET )
        {
            //if message is GET, retrieve the value associated with the key and return to sender

            long result; //value to send

            //retrieve local result
            pthread_mutex_lock(&local_mutex);
            result = local_get(server_msg.key);
            pthread_mutex_unlock(&local_mutex);

            //prepare and send response
            response_msg.type = ACCEPTED;
            response_msg.value = result;

            MPI_Ssend( &response_msg, sizeof(struct msg), MPI_BYTE, status.MPI_SOURCE, CLIENT, MPI_COMM_WORLD );
        }
        else if( server_msg.type == TERMINATE)
        {
            //if the message is TERMINATE, change value of i to allow thread to return
            i = 0; 
        }
            
    }
    
    return NULL;
}



/*
 * Determines the proper process to deal with a key.
 */
int hash_name(const char *name)
{
    unsigned hash = 5381;
    while (*name != '\0') {
        hash = ((hash << 5) + hash) + (unsigned)(*name++);
    }
    return hash % nprocs;
}

/*
 * Intitalizes dht
 */
int dht_init()
{
    local_init();

    //intialize mpi
    int provided;
    MPI_Init_thread(NULL, NULL, MPI_THREAD_MULTIPLE, &provided);
    if (provided != MPI_THREAD_MULTIPLE) {
        printf("ERROR: Cannot initialize MPI in THREAD_MULTIPLE mode.\n");
        exit(EXIT_FAILURE);
    }

    //spawn server thread
    thread_handle = (pthread_t *)malloc(sizeof(pthread_t));
    pthread_create(&thread_handle[0], NULL, server, (void*)0);

    //save mpi variables
    MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs); 

    return pid;
}

void dht_barrier()
{
    MPI_Barrier(MPI_COMM_WORLD);
}
/*
 * Enters key and value into hash table
 */
void dht_put(const char *key, long value)
{
    //get process that should store key
    int proc = hash_name( key );

    struct msg recv_msg; //message recieved if applicable

    //if key should be stored locally, do so, else send to server that should
    if(proc == pid )
    {
        pthread_mutex_lock(&local_mutex);
        local_put(key, value);
        pthread_mutex_unlock(&local_mutex);
    }
    else
    {
        //prepare and send put request
        struct msg put_msg;
        put_msg.type = 1;
        strncpy(put_msg.key, key, MAX_KEYLEN);
        put_msg.value = value;

        MPI_Ssend( &put_msg, sizeof(struct msg), MPI_BYTE, proc, SERVER, MPI_COMM_WORLD );

        //recieve confirmation that put was successful
        MPI_Recv( &recv_msg, sizeof(struct msg), MPI_BYTE, proc, CLIENT, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
        if( recv_msg.type != ACCEPTED)
        {
            printf("Client(%d): Error receiving put confirmation from server(%d)", pid, proc);
            exit(EXIT_FAILURE);
        }
    }
}

/*
 * Retrieves value at given key
 */
long dht_get(const char *key)
{
    long result = -1;
 
    //retrieve process that should hold key
    int proc = hash_name( key );

    struct msg recv_msg; //message to recieve response if needed

    //retrieve from local table if possible, else send request
    if(proc == pid )
    {
        pthread_mutex_lock(&local_mutex);
        result = local_get(key);
        pthread_mutex_unlock(&local_mutex);
    }
    else
    {
        //prepare request for key value
        struct msg put_msg;
        put_msg.type = GET;
        strncpy(put_msg.key, key, MAX_KEYLEN);

        //send request
        MPI_Ssend( &put_msg, sizeof(struct msg), MPI_BYTE, proc, SERVER, MPI_COMM_WORLD );

        //recieve response
        MPI_Recv( &recv_msg, sizeof(struct msg), MPI_BYTE, proc, CLIENT, MPI_COMM_WORLD, MPI_STATUS_IGNORE );
        if( recv_msg.type != ACCEPTED)
        {
            printf("Client(%d): Error receiving key value from server(%d)", pid, proc);
            exit(EXIT_FAILURE);
        }

        //update result
        result = recv_msg.value;
    }
    return result;
}

/*
 * performs cleanup and ends threads
 */ 
void dht_destroy()
{
    //barrier, wait until all processes are ready to finish
    MPI_Barrier(MPI_COMM_WORLD);

    //prepare terminate message
    struct msg term_msg;
    term_msg.type = TERMINATE;

    //send TERMINATE to own server thread
    MPI_Ssend( &term_msg, sizeof(struct msg), MPI_BYTE, pid, SERVER, MPI_COMM_WORLD );    
    pthread_join(thread_handle[0], NULL);
    
    MPI_Barrier(MPI_COMM_WORLD);

    //finalize MPI
    MPI_Finalize();

    //destroy mutex and free thread handle
    pthread_mutex_destroy(&local_mutex);
    free(thread_handle);
}

