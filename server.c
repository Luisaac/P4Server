#include <pthread.h>
#include "cs537.h"
#include "request.h"

// 
// server.c: A very, very simple web server
//
// To run:
//  server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

// CS537: Parse the new arguments too
void getargs(int *port, int argc, char *argv[], int *threads, int *buffers)
{
    if (argc != 4) {
	fprintf(stderr, "Usage: %s <port> <threads> <buffers>\n", argv[0]);
	exit(1);
    }
    *port = atoi(argv[1]);
    *threads = atoi(argv[2]);
    *buffers = atoi(argv[3]);
}


int main(int argc, char *argv[])
{
    pthread_cond_t empty, fill;

    pthread_mutex_t* mutex;
    pthread_mutexattr_t mutexAttribute;

    int listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;

    int threads;
    int buffers;
    getargs(&port, argc, argv, &threads, &buffers);
    int buffer[buffers];
    int fill_ptr = 0;
    int use_ptr = 0;
    int count = 0;

    // 
    // CS537: Create some threads...
    //

    mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    pthread_mutexattr_init(&mutexAttribute);
    pthread_mutexattr_setpshared(&mutexAttribute, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(mutex, &mutexAttribute);


    listenfd = Open_listenfd(port);
//    while (1) {
    // client/producer
    for (int i = 0; i < threads; i++) {
        pthread_mutex_lock(mutex);
	    while (count == buffers)
            pthread_cond_wait(&empty, mutex);

        // put
        clientlen = sizeof(clientaddr);
	    connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
        buffer[fill_ptr] = connfd;
        fill_ptr = (fill_ptr +1) % buffers;
        count++; 
        
        pthread_cond_signal(&fill);
        pthread_mutex_unlock(mutex);
    }
	// 
	// CS537: In general, don't handle the request in the main thread.
	// Save the relevant info in a buffer and have one of the worker threads 
	// do the work. 
	// 

    // handle request after the lock
    while (1) {
        pthread_mutex_lock(mutex);
        while (count == 0) 
            pthread_cond_wait(&fill, mutex);
        int connfd = buffer[use_ptr];
        use_ptr = (use_ptr + 1) % buffers;
        count--;
        pthread_cond_signal(&empty);
        pthread_mutex_unlock(mutex);
        requestHandle(connfd);
    }
	

	Close(connfd);
 //   }

}


    


 
