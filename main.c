#include <stdlib.h>
#include "genetic_algorithm.h"
#include <pthread.h>
#include <stdio.h>

struct arguments
{
    int ID;
    individual *current_generation;
    individual *next_generation;
    individual *current_generation_New;
    individual *tmp;

    sack_object *objects;
    int object_count, generations_count, sack_capacity, P, object_count_New;

    pthread_barrier_t *barrier;
};

int main(int argc, char *argv[])
{
    pthread_barrier_t barrier;

    // array with all the objects that can be placed in the sack
    sack_object *objects = NULL;

    // number of objects
    int object_count = 0;

    // maximum weight that can be carried in the sack
    int sack_capacity = 0;

    // number of generations
    int generations_count = 0;

    //number of threads
    int P = 0;

    if (!read_input(&objects, &object_count, &sack_capacity, &generations_count, &P, argc, argv))
    {
        return 0;
    }

    pthread_t tid[P];
    int i;
    struct arguments argument_id[P];
    pthread_barrier_init(&barrier, NULL, P);

    int object_count_New = 1;
    while (object_count_New < object_count)
    {
        object_count_New = object_count_New * 2;
    }

    individual *current_generation = malloc( sizeof(individual)*object_count_New);
    individual *next_generation = malloc( sizeof(individual)*object_count_New);
    individual *current_generation_New = malloc(sizeof(individual)*object_count_New);
    individual *tmp = NULL;

    for (i = 0; i < P; i++)
    {

        argument_id[i].ID = i;
        argument_id[i].current_generation = current_generation;
        argument_id[i].next_generation = next_generation;
        argument_id[i].tmp = tmp;
        argument_id[i].current_generation_New = current_generation_New;
        //---------------------------------
        argument_id[i].objects = objects;
        argument_id[i].object_count_New = object_count_New;
        argument_id[i].object_count = object_count;
        argument_id[i].generations_count = generations_count;
        argument_id[i].sack_capacity = sack_capacity;

        argument_id[i].P = P;

        argument_id[i].barrier = &barrier;

        pthread_create(&tid[i], NULL, run_genetic_algorithm, &argument_id[i]);
    }

    for (i = 0; i < P; i++)
    {
        pthread_join(tid[i], NULL);
    }

    /*
    run_genetic_algorithm(objects, object_count, generations_count, sack_capacity, P);
*/


    // free_generation(current_generation);
	// free_generation(next_generation);
	// free_generation(current_generation_New);

	// // free resources
	// free(current_generation);
	// free(next_generation);
	// free(current_generation_New);
    
    free(objects);

    return 0;
}