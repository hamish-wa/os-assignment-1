#include "standard.h"

/*************************************************************************
 *                            Macro Definitions                          *
 *************************************************************************/

#define TRUE 1
#define FALSE 0
#define T_THREADS 4 /* Number of teller threads to be created. */
#define C_THREADS 1 /* Number of customer threads to be created. */
#define DEBUG_FILE "debug" /* The name of the debug file. */
#define LOG_FILE "r_log" /* The name of the log file. */
#define CUSTOMER_FILE "c_file" /* The name of the customer file. */

/*************************************************************************
 *                            Global Variables                           *
 *************************************************************************/

FILE *log_file = NULL;
FILE *debug_file = NULL;
FILE *c_file = NULL;

pthread_mutex_t debug_mutex = PTHREAD_MUTEX_INITIALIZER; /* Mutex for the debug access. */
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER; /* Mutex for the log access. */
pthread_mutex_t file_mutex = PTHREAD_MUTEX_INITIALIZER; /* Mutex for the file access. */

int end_of_file = FALSE; /* Flag to indicate if the end of the file has been reached. */
customer_queue_t c_queue; /* The customer queue. */

int t_I; /* The time duration of an information query. */
int t_C; /* The customer arrival period. */
int t_W; /* The time duration of a withdrawal. */
int t_D; /* The time duration of a deposit. */

teller_t *tellers; /* Array of tellers. */

pthread_t t_threads[T_THREADS]; /* Array of teller threads. */
pthread_t c_threads[C_THREADS]; /* Array of customer threads. */

int t_thread_numbers[T_THREADS]; /* Number of teller threads created (e.g., [ 1, 2, 3, 4 ]). */
int c_thread_numbers[C_THREADS]; /* Number of customer threads created. */

/*************************************************************************
 *                             Main Function                             *
 *************************************************************************/

/*************************************************************************
 * Main Function.
 *
 * This function is the entry point of the program. It creates the teller and
 * customer threads and then waits for them to finish.
 *
 * @param argc - The number of arguments passed to the program.
 * @param argv - The arguments passed to the program.
 *           argv[0] - The file name of the program.
 *           argv[1] - The size/length of the customer queue (m).
 *           argv[2] - The customer arrival period (t_C).
 *           argv[3] - The time duration of a withdrawal (t_W).
 *           argv[4] - The time duration of a deposit (t_D).
 *           argv[5] - The time duration of an information query (t_I).
 * @return int - The exit code of the program.
 *************************************************************************/
int main(int argc, char *argv[]) {
    int i; /* Loop counter. */
    int teller_total = 0; /* Total number of customers served by all tellers. */
    char *msg = malloc(sizeof(char) * 100);

    /* Open the debug and log file. */
    debug_file = fopen(DEBUG_FILE, "w");
    log_file = fopen(LOG_FILE, "w");

    /* Initialize the log and debug mutexes. */
    pthread_mutex_init(&log_mutex, NULL);
    pthread_mutex_init(&debug_mutex, NULL);

    /* Register the SIGINT signal handler. */
    if (signal(SIGINT, sig_handler) == SIG_ERR) {
        printf("Error: Failed to register the SIGINT signal handler.\n");
    }

    /*************************************************************************
     * Check the arguments passed to the program.
     *************************************************************************/

    /* Check if the correct number of arguments were passed. */
    if (argc != 6) {
        printf("Usage: %s <m> <t_C> <t_W> <t_D> <t_I>\n", argv[0]);
        printf("  m  - The size/length of the customer queue.\n");
        printf("  t_C - The customer arrival period.\n");
        printf("  t_W - The time duration of a withdrawal.\n");
        printf("  t_D - The time duration of a deposit.\n");
        printf("  t_I - The time duration of an information query.\n");
        printf("\nExample: %s 100 5 2 2 1\n", argv[0]);
        return 1; /* Exit with error code 1. */
    } else {
        sprintf(msg, "The number of arguments passed: %d", argc);
        sprintf(msg, "The arguments passed to the program:");
        for (i = 0; i < argc; i++) {
            sprintf(msg, "  argv[%d] = %s", i, argv[i]);
        }
    }

    /* Get the queue size. */
    c_queue.size = atoi(argv[1]);

    /* Get the time periods. */
    t_C = atoi(argv[2]);
    t_W = atoi(argv[3]);
    t_D = atoi(argv[4]);
    t_I = atoi(argv[5]);

    /* Check that the queue size is greater than 0. */
    if (c_queue.size <= 0) {
        printf("Error: The queue size must be greater than 0.\n");
        printf("Entered queue size: %d\n", c_queue.size);
        printf("Usage: %s <m> <t_C> <t_W> <t_D> <t_I>\n", argv[0]);
        return 1; /* Exit with error code 1. */
    }

    /* Check that the time periods are all greater or equal to 1 */
    if (t_C < 1 || t_W < 1 || t_D < 1 || t_I < 1) {
        printf("Error: All time periods must be greater than or equal to 1.\n");
        printf("Entered time periods: t_C=%d, t_W=%d, t_D=%d, t_I=%d\n", t_C, t_W, t_D, t_I);
        printf("Usage: %s <m> <t_C> <t_W> <t_D> <t_I>\n", argv[0]);
        return 1; /* Exit with error code 1. */
    }

    /*************************************************************************
     * Initialize the customer queue.
     *************************************************************************/
    
    /* Initialize the customer queue mutex. */
    if (pthread_mutex_init(&c_queue.mutex, NULL) != 0) {
        printf("Error: Failed to initialize the customer queue mutex.\n");
        return 1; /* Exit with error code 1. */
    }

    /* Initialize the customer queue condition variables. */
    if (pthread_cond_init(&c_queue.empty, NULL) != 0) {
        printf("Error: Failed to initialize the customer queue empty condition variable.\n");
        return 1; /* Exit with error code 1. */
    }

    if (pthread_cond_init(&c_queue.full, NULL) != 0) {
        printf("Error: Failed to initialize the customer queue full condition variable.\n");
        return 1; /* Exit with error code 1. */
    }

    /* Initialize the customer queue variables. */
    c_queue.in = 0;
    c_queue.out = 0;
    c_queue.count = 0;
    c_queue.q = malloc(sizeof(struct customer) * c_queue.size);

    /*************************************************************************
     * Initialize the mutexes.
     *************************************************************************/


    if (pthread_mutex_init(&c_queue.mutex, NULL) != 0) {
        printf("Error: Failed to initialize the queue mutex.\n");
        return 1; /* Exit with error code 1. */
    }


    if (pthread_mutex_init(&file_mutex, NULL) != 0) {
        printf("Error: Failed to initialize the teller mutex.\n");
        return 1; /* Exit with error code 1. */
    }

    /*************************************************************************
     * Initialize the condition variables.
     *************************************************************************/

    /* Initialize the condition variables. */
    if (pthread_cond_init(&c_queue.full, NULL) != 0) {
        printf("Error: Failed to initialize the full condition variable.\n");
        return 1; /* Exit with error code 1. */
    }

    if (pthread_cond_init(&c_queue.empty, NULL) != 0) {
        printf("Error: Failed to initialize the empty condition variable.\n");
        return 1; /* Exit with error code 1. */
    }

    /* Create the teller threads. */
    tellers = malloc(sizeof(struct teller) * T_THREADS);
    for (i = 0; i < T_THREADS; i++) {

        /* Allocate memory for the teller struct. */

        /* Set the teller thread numbers. */
        t_thread_numbers[i] = i;
        sprintf(msg, "Creating teller thread %d...", i + 1);

        /* Allocate memory for the teller struct. */

        tellers[i].teller_number = i + 1;
        tellers[i].customers_served = 0;

        /* Set the teller start time. */
        get_time(tellers[i].start_time);

        /* The below line is initializing the teller thread with the teller function.
         * pthread_create(<address of thread>, <thread attributes>, <teller function to run>, <teller struct>)
         */
        pthread_create(&t_threads[i], NULL, teller, (void *) &t_thread_numbers[i]);
        sprintf(msg, "Created teller thread %d.", i + 1);
    }

    /* Create the customer threads. */
    for (i = 0; i < C_THREADS; i++) {
        c_thread_numbers[i] = i + 1;

        /* The below line is initializing the customer thread with the customer function.
         * pthread_create(<address of thread>, <thread attributes>, <customer function to run>, <filename>)
         */
        pthread_create(&c_threads[i], NULL, customer, (void *) CUSTOMER_FILE);

        sprintf(msg, "Created customer thread %d.", i + 1);
    }

    /* Wait for the customer threads to finish. */
    for (i = 0; i < C_THREADS; i++) {
        pthread_join(c_threads[i], NULL);
        sprintf(msg, "Joined customer thread %d.", i + 1);
    }

    /* Wait for the teller threads to finish. */
    for (i = 0; i < T_THREADS; i++) {
        pthread_join(t_threads[i], NULL);
        sprintf(msg, "Joined teller thread %d.", i + 1);
    }

    /* Print the teller stats. */
    wrt_log("Teller Statistic");
    teller_total = 0;
    for (i = 0; i < T_THREADS; i++) {
        sprintf(msg, "Teller-%d serves %d customers.", tellers[i].teller_number, tellers[i].customers_served);
        wrt_log(msg);
        teller_total += tellers[i].customers_served;
    }
    sprintf(msg, "\nTotal customers served: %d\n", teller_total);
    wrt_log(msg);

    /* Close the file. */
    fclose(c_file);

    /*************************************************************************
     * Print the results.
     *************************************************************************/

    free(tellers);

    /* Free the memory. */
    free(msg); /* Free the message string. */

    /* Free the arrays. */
    free(c_queue.q); /* Free the queue. */

    /* Destroy the mutexes. */
    pthread_mutex_destroy(&c_queue.mutex);

    /* Destroy the condition variables. */
    pthread_cond_destroy(&c_queue.full);
    pthread_cond_destroy(&c_queue.empty);

    /* Close the files. */
    fclose(log_file);
    fclose(debug_file);

    return 0;
}

/*************************************************************************
 *                           Thread Functions                            *
 *************************************************************************/

/*************************************************************************
 * Customer Producer Function.
 *
 * This function reads the c_file and creates the customers
 * based on the data in the file. It then adds the customers to the
 * shared queue and then sleep for t_C seconds.
 *
 * @param arg - The argument passed to the thread.
 * @return void* - The return value of the thread.
 *************************************************************************/
void *customer(void *arg) {
    char line[10] = {0}; /* The line read from the file. */
    int customer_number;
    char service_type;
    char *arrive_time;
    char *msg = malloc(sizeof(char) * 100);
    c_file = fopen((char *) arg, "r"); /* Open the file. */

    if (c_file == NULL) {
        printf("Error opening file.\n");
        exit(1);
    }

    do {
        /* Sleep for t_C seconds. */
        sleep(t_C);

        if (fgets(line, 10, c_file) != NULL) {
            /* Lock the queue. */
            pthread_mutex_lock(&c_queue.mutex);

            /*************************************************************************
             *                           Critical Section                            *
             *************************************************************************/

            end_of_file = FALSE;

            line[strlen(line) - 1] = '\0'; /* Remove the newline character. */

            /* Read the customer number. */
            sscanf(line, "%d %c", &customer_number, &service_type);

            /* Wait for the queue to not be full. */
            while (is_full() == TRUE) {
                pthread_cond_wait(&c_queue.full, &c_queue.mutex);
            }

            /* Set the customer values. */
            c_queue.q[c_queue.in].customer_number = customer_number;
            c_queue.q[c_queue.in].service_type = service_type;

            /* Get the current time. */
            get_time(c_queue.q[c_queue.in].arrival_time);
            arrive_time = c_queue.q[c_queue.in].arrival_time;

            /* Print the customer information. */
            wrt_log("-----------------------------------------------------------------------");
            sprintf(msg, "Customer %d: %c\nArrival time: %s", customer_number, service_type, arrive_time);
            wrt_log(msg);
            wrt_log("-----------------------------------------------------------------------\n");


            /* Increment the queue size. */
            c_queue.count++;

            /* Increment the queue in. */
            c_queue.in = (c_queue.in + 1) % c_queue.size;

            /*************************************************************************
             *                        End of Critical Section                        *
             *************************************************************************/

            /* Signal that the queue is not empty. */
            pthread_cond_signal(&c_queue.empty);

            /* Unlock the queue. */
            pthread_mutex_unlock(&c_queue.mutex);

        } else if (feof(c_file) != 0) { /* The end of the file has been reached. */
            pthread_mutex_lock(&c_queue.mutex);
            end_of_file = TRUE;
            /* Broadcast that the queue is not empty. This will wake up the tellers, and they will shut down. */
            pthread_cond_broadcast(&c_queue.empty);
            pthread_mutex_unlock(&c_queue.mutex);
        } else if (ferror(c_file) != 0) { /* There was an error reading the file. */
            printf("Error reading file.\n");
            pthread_mutex_lock(&c_queue.mutex);
            end_of_file = TRUE;
            /* Broadcast that the queue is not empty. This will wake up the tellers, and they will shut down. */
            pthread_cond_broadcast(&c_queue.empty);
            pthread_mutex_unlock(&c_queue.mutex);
            exit(1);
        }

    } while (end_of_file == FALSE);

    /* Free the memory. */
    free(msg);

    /* Exit the thread. */
    pthread_exit(NULL);
    return NULL; /* This line is only here to prevent a warning. */
}

/*************************************************************************
 * Teller Consumer Function.
 *
 * This function is the entry point of the teller threads. It simulates
 * the teller serving customers. It is the consumer thread.
 *
 * @param arg - The teller struct.
 * @return void* - The teller struct.
 *************************************************************************/
void *teller(void *arg) {
    char *msg;
    char response_time[9];
    char completion_time[9];
    int sleep_time;
    int customer_number;
    customer_t current_customer;
    char service_type;

    teller_t t; /* The teller struct. */

    /* Allocate memory for the message string. */
    msg = malloc(sizeof(char) * 100);

    /* Get the teller struct. */
    t = tellers[*((int *) arg)];

    /* Loop until the end of the file has been reached and the queue is empty. */
    do {
        /* Lock the queue. */
        pthread_mutex_lock(&c_queue.mutex);

        /*************************************************************************
         *                           Critical Section                            *
         *************************************************************************/

        /* Wait for the queue to not be empty. */
        while (is_empty() == TRUE && end_of_file == FALSE) {
            pthread_cond_wait(&c_queue.empty, &c_queue.mutex);
        }

        /* Check if the end of the file has been reached. */
        if (end_of_file == TRUE && is_empty() == TRUE) {
            /* Unlock the queue. */
            pthread_mutex_unlock(&c_queue.mutex);
            break;
        } else {

            /* Set the current customer. */
            current_customer = c_queue.q[c_queue.out];

            /* Set the customer values. */
            customer_number = current_customer.customer_number; /* Get the customer number. */
            service_type = current_customer.service_type; /* Get the service type. */

            /* Get the current time. */
            get_time(response_time);

            /* Log the customer. */
            sprintf(msg, "Teller: %d\nCustomer: %d\nService: %c\nArrival Time: %s\nResponse Time: %s\n",
                    t.teller_number, customer_number, service_type, current_customer.arrival_time, response_time);
            wrt_log(msg);

            /* Increment the queue out. */
            c_queue.out = (c_queue.out + 1) % c_queue.size;

            /* Decrement the queue size. */
            c_queue.count--;

            /*************************************************************************
             *                        End of Critical Section                        *
             *************************************************************************/

            /* Signal that the queue is no longer full. */
            pthread_cond_signal(&c_queue.full);

            /* Unlock the queue. */
            pthread_mutex_unlock(&c_queue.mutex);

            /* Get the sleep time. */
            if (service_type == 'D') {
                sleep_time = t_D; /* Set the sleep time to deposit. */
            } else if (service_type == 'W') {
                sleep_time = t_W; /* Set the sleep time to withdraw. */
            } else { /* service_type == 'I' */
                sleep_time = t_I; /* Set the sleep time to information. */
            }

            /* Sleep for t_C seconds. */
            sleep(sleep_time);

            /* Increment the number of customers served. */
            t.customers_served++;

            /* Get the current time. */
            get_time(completion_time);

            /* Log the customer served. */
            sprintf(msg, "Teller: %d\nCustomer: %d\nArrival Time: %s\nCompletion Time: %s\n", t.teller_number, customer_number, current_customer.arrival_time, completion_time);
            wrt_log(msg);
        }
    } while (end_of_file == FALSE || (end_of_file == TRUE && is_empty() == FALSE));

    /* Set the teller end time. */
    get_time(t.end_time);

    /* Write the teller log exit message. */
    sprintf(msg, "Termination: teller-%d\n#served customers: %d\nStart time: %s\nTermination time: %s\n", t.teller_number, t.customers_served, t.start_time, t.end_time);
    wrt_log(msg);

    free(msg); /* Free the memory. */

    tellers[*((int *) arg)] = t; /* Set the teller struct. This is to ensure that the new values are saved. */

    /* Exit the thread. */
    pthread_exit(NULL);
    return NULL; /* To avoid warnings. */
}

/*************************************************************************
 *                            Helper Functions                           *
 *************************************************************************/

/*************************************************************************
 * Log Function.
 *
 * This function writes the log file.
 *
 * @param msg - The message to be written to the log file.
 * @return void
 *************************************************************************/
void wrt_log(const char *msg) {
    /* Lock the log file. */
    pthread_mutex_lock(&log_mutex);

    if (log_file == NULL) {
        log_file = fopen(LOG_FILE, "w");
        if (log_file == NULL) {
            fprintf(stderr, "Error opening log file.\n");
            exit(EXIT_FAILURE);
        }
    }

    fprintf(log_file, "%s\n", msg);
    fflush(log_file); /* Print the message to the log file. */

    pthread_mutex_unlock(&log_mutex);
}

/*************************************************************************
 * Print Debug Function.
 *
 * This function prints a debug message if the DEBUG flag is set.
 *
 * @param msg - The message to be printed.
 * @return void
 *************************************************************************/
void debug(const char *msg) {

    /* Lock the debug file. */

    pthread_mutex_lock(&debug_mutex);
    if (debug_file == NULL) {
        debug_file = fopen(DEBUG_FILE, "w");
        if (debug_file == NULL) {
            fprintf(stderr, "Error opening debug file.\n");
            exit(EXIT_FAILURE);
        }
    }

    fprintf(debug_file, "%s\n", msg);
    fflush(debug_file);

    pthread_mutex_unlock(&debug_mutex);
}

/*************************************************************************
 * Time Function.
 *
 * This function fills a string with the current time.
 *
 * @param time_str - The pointer to the string to be filled with the current time.
 * @return void
 *************************************************************************/
void get_time(char *time_str) {
    time_t tt;
    struct tm *tm_info;

    tt = time(NULL);
    tm_info = localtime(&tt);
    strftime(time_str, 9, "%H:%M:%S", tm_info);
}

/*************************************************************************
 * Is Empty Function.
 *
 * This function checks if the queue is empty.
 *
 * @return int - TRUE if the queue is empty, FALSE otherwise.
 *************************************************************************/
int is_empty() {
    if (c_queue.count == 0) {
        return TRUE;
    }
    return FALSE;
}

/*************************************************************************
 * Is Full Function.
 *
 * This function checks if the queue is full.
 *
 * @return int - TRUE if the queue is full, FALSE otherwise.
 *************************************************************************/
int is_full() {
    if (c_queue.count == c_queue.size) {
        return TRUE;
    }
    return FALSE;
}

/*************************************************************************
 * Sig Handler Function.
 *
 * This function is called when the program receives a SIGINT signal. It
 * prints the number of customers served by each teller and the total number
 * of customers served.
 * The SIGINT signal is sent when the user presses CTRL+C.
 *
 * @param sig - The signal number.
 * @return void
 *************************************************************************/
void sig_handler(int signo) {
    if (signo == SIGINT) {
        printf("\n");
        printf("The program was interrupted by the user.\n");
        exit(0);
    }
}
