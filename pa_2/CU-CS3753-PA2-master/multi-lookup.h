/*
 * File: multi-lookup.h
 * Author: Jonathan Song <jonathan.song@colorado.edu>
 * Project: Programming Assignment 2: DNS Name Resolution Engine
 * Date: 2/28/2014
 * Description:
 * 	This is the header file containing the prototypes for functions used
 * 	in program multi-lookup.c
 * 
 */
 
// first need to define the multi-lookup
#ifndef MULTI_LOOKUP_H
#define MULTI_LOOKUP_H

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
// include for pthread_join, pthread_create, etc. 
#include <pthread.h>

// include for sleep command
#include <unistd.h>

// include queue.h and util.h to use functions
// queue.h implements FIFO queue data structure
// util.h contains DNS utility functions
#include "queue.h"
#include "util.h"

// Define some constants for multi-lookup
// can copy some values from the lookup.c file
#define MINARGS 3 
#define USAGE "<inputFilePath> <outputFilePath>"
#define SBUFSIZE 1025
#define INPUTFS "%1024s"

// need to implement limits
#define MAX_INPUT_FILES 10
#define MAX_RESOLVER_THREADS 10
#define MIN_RESOLVER_THREADS 2
#define MAX_NAME_LENGTH 1025
#define MAX_IP_LENGTH INET6_ADDRSTRLEN

// define variable to see if input queue is empty/done
extern bool request_queue_finished;
// initialize variable as false to indicate there are still jobs
// in request queue to complete
bool request_queue_finished = false;


// need to create queue to hold requests
// queue max size is 50 in included queue.h
#define QUEUE_SIZE 50

typedef struct inputFunctionParameter{
	// hold file name
    char* fname;
    queue* request_queue;
    // mutex to lock
    pthread_mutex_t* mutex_queue;
} thread_request_arg_t ;

typedef struct outputFunctionParameter{
    queue* rqueue;
    FILE* outputfp;
    // mutual exclusion locks (mutexes) used to serialize execution
    // of threads through critical sections of code that access
    // shared data
    // two mutex to lock
    // for queue and out file
    pthread_mutex_t* mutex_ofile;
    pthread_mutex_t* mutex_queue;
} thread_resolve_arg_t ;


// input and output thread functions
void* InputThread (void* p);
void* OutputThread (void* p);

#endif
