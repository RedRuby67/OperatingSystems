/*
 * File: multi-lookup.c
 * Author: Jonathan Song
 * Project: CSCI 3753 Programming Assignment 2
 * Create Date: 2/28/2014
 * Modify Date: 2/28/2014
 * Description:
 * 	This file contains the reference threaded
 *      solution to this assignment.
 *  
 */

#include "queue.h"
#include "multi-lookup.h"

void* InputThread (void* p) {
    thread_request_arg_t* args = (thread_request_arg_t*) p;

    /* Local Vars */
    FILE* inputfp = NULL;
   
    /* Open Input File */  
    inputfp = fopen(args->fname, "r");
    if(!inputfp){
        char errorstr[SBUFSIZE];
        sprintf(errorstr, "Error Opening Input File: %s", args->fname);
        perror(errorstr);
        return NULL;
    }	
    
    /* Read File and Process*/
    char hostname[SBUFSIZE];
    //On success, the function returns the number of items of the argument list successfully filled
    // int fscanf ( FILE * stream, const char * format, ... )
    while(fscanf(inputfp, INPUTFS, hostname) > 0){

        // wait until queue is available
        while (1) {

            // lock the request queue mutex while queue is not available
            pthread_mutex_lock(args->mutex_queue);
            
            // if queue is full, unlock mutex
            if (queue_is_full(args->request_queue)) {
                pthread_mutex_unlock(args->mutex_queue);
				// sleep for random time between 0 and 100 microseconds
				// use usleep for microseconds
				// will sleep for random amount of microseconds 
				// rand() % 100 between 0 and 99
                usleep(rand() % 100);
			// exit otherwise
            } else { 
				break; 
			}
        }

        // use malloc to free space for hostname
        char* tocopy = malloc(sizeof(hostname));
        strncpy(tocopy, hostname, sizeof(hostname));

        // push hostname to request queue
        queue_push(args->request_queue, tocopy);

        // unlock the request queue mutex
        pthread_mutex_unlock(args->mutex_queue);
    
    }
    
    /* Close Input File */
    fclose(inputfp);

    return NULL;
}

void* OutputThread (void* p) {
    thread_resolve_arg_t* args = (thread_resolve_arg_t*) p;

    // need to pop items off of queue while true
    // queue in use
    while (1) {
    
        // lock queue mutex
        pthread_mutex_lock(args->mutex_queue);
        // pop item off of queue
        char* hostnamepop = queue_pop(args->rqueue);
        // unlock queue mutex
        pthread_mutex_unlock(args->mutex_queue);

        // check that queue is non-empty as looks up hostname and frees
        // it
        if (hostnamepop) {
            char hostname[SBUFSIZE];
            sprintf(hostname, "%s", hostnamepop);
            free(hostnamepop);

            /* Lookup hostname and get IP string */

            char firstipstr[INET6_ADDRSTRLEN];
            if (dnslookup(hostname, firstipstr, sizeof(firstipstr))
               == UTIL_FAILURE){
            fprintf(stderr, "dnslookup error: %s\n", hostname);
            strncpy(firstipstr, "", sizeof(firstipstr));
            }
            
            // lock output file mutex
            pthread_mutex_lock(args->mutex_ofile);
            // write output file
            fprintf(args->outputfp, "%s,%s\n", hostname, firstipstr);
            // unlock the output file
            pthread_mutex_unlock(args->mutex_ofile);
    
        // check for empty queue otherwise, no hostname
        // make sure reader threads are done
        } 
        // if queue is still in use
        // there are still jobs in request queue to do
        else if (!request_queue_finished) {
			// sleep for random time between 0 and 100 microseconds
			usleep(rand() % 100);
		}
		else {
			// otherwise exit out
			break;
		}
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    
    /* Check Arguments */
    if(argc < MINARGS){
		fprintf(stderr, "Not enough arguments: %d\n", (argc - 1));
		fprintf(stderr, "Usage:\n %s %s\n", argv[0], USAGE);
		return EXIT_FAILURE;
    }

    /* Local Vars */
    FILE* outputfp = NULL;
    queue request_queue;
    int request_queue_size = QUEUEMAXSIZE;
    pthread_t threads_request[argc - 1];
    pthread_t threads_resolve[MAX_RESOLVER_THREADS];
    int i;

    /* Open Output File */
    outputfp = fopen(argv[(argc - 1)], "w");
    if(!outputfp){
		perror("Error Opening Output File");
		return EXIT_FAILURE;
    }

    // initialize request queue
    queue_init(&request_queue, request_queue_size);

    // create mutex queue and mutex output file
    pthread_mutex_t mutex_queue = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t mutex_ofile = PTHREAD_MUTEX_INITIALIZER;

    // create requester thread for each requester file or input file
    thread_request_arg_t req_args[argc - 2];
    // will open input files in an array
    for(i = 1; i < (argc - 1); i++){
		// create requester threads with names, request queue, and
		// mutex
        req_args[i - 1].fname = argv[i];
        req_args[i - 1].request_queue = &request_queue;
        req_args[i - 1].mutex_queue = &mutex_queue;
    // use pthread_create to create a new thread with default attributes
    // with NULL
    // successful call will store pid of thread
    // failed call will result in undefined thread
	int rc = pthread_create(&(threads_request[i - 1]), NULL, InputThread, &(req_args[i - 1]));
	if (rc){
		printf("Error creating request thread: return code from pthread_create() is %d\n", rc);
		exit(EXIT_FAILURE);
	}
   }

    // create resolver threads with MAX_RESOLVER_THREADS as limit
    thread_resolve_arg_t res_args;
    // again create resolver threads with name, request queue, output 
    // file, mutex files
    res_args.rqueue = &request_queue;
    res_args.outputfp = outputfp;
    res_args.mutex_ofile = &mutex_ofile;
    res_args.mutex_queue = &mutex_queue;
    for(i = 0; i < MAX_RESOLVER_THREADS; i++) {
		int rc = pthread_create(&(threads_resolve[i]), NULL, OutputThread, &res_args);
		if (rc){
			printf("Error creating resolver thread: return code from pthread_create() is %d\n", rc);
			exit(EXIT_FAILURE);
	}
   }

    // merge requester threads
    // wait for requester threads to finish
    for(i = 0; i < (argc - 2); i++){
		// use pthread_join to wait for request threads to complete
		// successsful => 0
		// otherwise error due to deadlock, etc.
		int rv = pthread_join(threads_request[i], NULL);
	if (rv) {
        fprintf(stderr, "Error: pthread_join on requester thread returned %d\n", rv);
    }
   }
   
   
   // at this point, the queue is done, so set to true
   // no more jobs
    request_queue_finished = true;
    
	// create output writing thread and output parameter structs
    for(i = 0; i < MAX_RESOLVER_THREADS; i++){
		int rv = pthread_join(threads_resolve[i], NULL);
	if (rv) {
        fprintf(stderr, "Error: pthread_join on resolver thread returned %d\n", rv);
    }
   }
   

    // cleanup request queue
    queue_cleanup(&request_queue);

    /* Close output file */
    fclose(outputfp);

    // destroy mutex queue and mutex out files
    pthread_mutex_destroy(&mutex_queue);
    pthread_mutex_destroy(&mutex_ofile);

    return EXIT_SUCCESS;
}
