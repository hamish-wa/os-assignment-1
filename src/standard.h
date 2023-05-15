
#ifndef OS_ASSIGNMENT_20183622_STANDARD_H
#define OS_ASSIGNMENT_20183622_STANDARD_H

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h> /* For sleep() */
#include <string.h>
#include <signal.h> /* To catch ctrl+c */
#include <time.h> /* For time() */

#define TRUE 1
#define FALSE 0

/* If DEBUG is defined, then the program will print out debug messages by setting it to 1, if not defined DEBUG will be set to FALSE  */
#ifdef DEBUG
#undef DEBUG
#define DEBUG TRUE
#else
#define DEBUG FALSE
#endif


/*************************************************************************
 *                                Structs                                *
 *************************************************************************/

/*************************************************************************
 * Struct for a teller.
 *
 * This struct contains the teller number, the number of customers served
 * by the teller, the time the teller thread started, and the time the
 * teller thread ended.
 *
 * @param teller_number - The teller number.
 * @param customers_served - The number of customers served by the teller.
 * @param start_time - The time the teller thread started.
 * @param end_time - The time the teller thread ended.
 *************************************************************************/
typedef struct teller {
    int teller_number;
    int customers_served;
    char start_time[9];
    char end_time[9];
} teller_t; /* Teller struct. */

/*************************************************************************
 * Struct for a customer.
 *
 * This struct contains the customer number, the service type, the time
 * the customer arrived in the queue, and the time the customer was
 * finished being served by a teller.
 *
 * @param customer_number - The customer number.
 * @param service_type - The service type.
 * @param arrival_time - The time the customer arrived in the queue.
 *************************************************************************/
typedef struct customer {
    int customer_number;
    char service_type;
    char arrival_time[9];
} customer_t; /* Customer struct. */

/*************************************************************************
 * Struct for a customer queue.
 *
 * This struct contains the customer queue, the size of the queue, the
 * number of customers in the queue, the position to insert the next
 * customer, and the position to remove the next customer.
 *
 * @param c_queue - The customer queue.
 * @param c_queue.size - The size of the queue.
 * @param c_queue.count - The number of customers in the queue.
 * @param c_queue.in - The position to insert the next customer.
 * @param c_queue.out - The position to remove the next customer.
 *************************************************************************/
typedef struct customer_queue {
    customer_t *q;
    int size;
    int count;
    int in;
    int out;
    pthread_mutex_t mutex;
    pthread_cond_t empty;
    pthread_cond_t full;
} customer_queue_t; /* Customer queue struct. */

/*************************************************************************
*                          Function Prototypes                          *
*************************************************************************/

/* Helper functions. */
void wrt_log(const char *msg);

void debug(const char *msg);

void get_time(char *time_str);

int is_empty();

int is_full();

void sig_handler(int signo);

/* Thread functions. */
void *teller(void *arg);

void *customer(void *arg);


#endif /*OS_ASSIGNMENT_20183622_STANDARD_H*/
