#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <pthread.h>

struct thread_global_arguments
{
    double *pi;
    pthread_mutex_t lock;
    __uint128_t max_iterations, current_iterations;
    __uint128_t threads_number;
};

struct thread_information
{
    pthread_t *thread_id;
    __uint128_t partial_iteration_start, partial_iteration_end;
    int thread_number;
    double current_sum;
    struct thread_global_arguments *thread_global_arguments;
};

void *logger(void *args)
{
    FILE *log = fopen("log.txt", "a+");
    struct thread_information *thread_information = (struct thread_information*)args;

    fprintf(log, "THREAD %d DONE / PARTIAL PI CALCULATED: %.18lf\n", thread_information->thread_number, *(thread_information->thread_global_arguments->pi) * 4);
    
    fclose(log);
    pthread_exit(NULL);
}

void *function(void *args)
{
    struct thread_information thread_information;
    memcpy(&thread_information, args, sizeof(struct thread_information));

    for(__uint128_t n = thread_information.partial_iteration_start; n < thread_information.partial_iteration_end; n++)
    {
        thread_information.current_sum += pow(-1, n)/((2 * n) + 1);
    }

    pthread_mutex_lock(&thread_information.thread_global_arguments->lock);
    *(thread_information.thread_global_arguments->pi) += thread_information.current_sum;
    pthread_mutex_unlock(&thread_information.thread_global_arguments->lock);

    pthread_t logger_id;

    if(pthread_create(&logger_id, NULL, logger, (void*)&thread_information) == 0)
    {
        fprintf(stdout, "Success: Thread logger created.\n");
    }
    else
    {
        fprintf(stderr, "Error: Creating thread logger.\n");
        fprintf(stderr, "Exiting...\n");
        exit(EXIT_FAILURE);
    }

    pthread_join(logger_id, NULL);

    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    // if(argc != 3)
    // {
    //     fprintf(stderr, "Error: Needed Arguments.\n");
    //     exit(EXIT_FAILURE);
    // }

    remove("log.txt");

    __uint128_t max_iterations = 100000000000;
    __uint128_t threads_number = 15;

    struct thread_global_arguments *thread_global_arguments = calloc(1, sizeof(struct thread_global_arguments));
    thread_global_arguments->pi = calloc(1, sizeof(double));

    thread_global_arguments->max_iterations = max_iterations;
    thread_global_arguments->current_iterations = 0;
    thread_global_arguments->threads_number = threads_number;
    
    pthread_t threads_identifiers[threads_number];

    struct thread_information threads_infos[threads_number];

    for(__uint128_t i = 0; i < threads_number; i++)
    {
        threads_infos[i].thread_global_arguments = thread_global_arguments;
        threads_infos[i].thread_id = &threads_identifiers[i];
        threads_infos[i].thread_number = (i+1);
        threads_infos[i].current_sum = 0;
        threads_infos[i].partial_iteration_start = (thread_global_arguments->max_iterations / thread_global_arguments->threads_number) * i;
        threads_infos[i].partial_iteration_end = (threads_infos[i].partial_iteration_start + (thread_global_arguments->max_iterations / thread_global_arguments->threads_number));
        
        if(pthread_create(&threads_identifiers[i], NULL, function, (void*)&threads_infos[i]) == 0)
        {
            fprintf(stdout, "Success: Thread %d created.\n", threads_infos[i].thread_number);
        }
        else
        {
            fprintf(stderr, "Error: Creating thread %d.\n", threads_infos[i].thread_number);
            fprintf(stderr, "Exiting...\n");
            exit(EXIT_FAILURE);
        }
    }

    for(__uint128_t i = 0; i < threads_number; i++)
    {
        pthread_join(threads_identifiers[i], NULL);
    }

    FILE *log = fopen("log.txt", "a+");

    fprintf(log, "PI       : %.18lf\n", M_PI);
    fprintf(log, "PI RESULT: %.18lf\n", (*(thread_global_arguments->pi) * 4));

    free(thread_global_arguments->pi);
    free(thread_global_arguments);
    fclose(log);
    return(0);
}