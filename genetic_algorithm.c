#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "genetic_algorithm.h"
#include <pthread.h>

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

int read_input(sack_object **objects, int *object_count, int *sack_capacity, int *generations_count, int *P, int argc, char *argv[])
{
	FILE *fp;

	if (argc < 4)
	{
		fprintf(stderr, "Usage:\n\t./tema1_par in_file generations_count nr_threads\n");
		return 0;
	}

	fp = fopen(argv[1], "r");
	if (fp == NULL)
	{
		return 0;
	}

	if (fscanf(fp, "%d %d", object_count, sack_capacity) < 2)
	{
		fclose(fp);
		return 0;
	}

	if (*object_count % 10)
	{
		fclose(fp);
		return 0;
	}

	sack_object *tmp_objects = (sack_object *)calloc(*object_count, sizeof(sack_object));

	for (int i = 0; i < *object_count; ++i)
	{
		if (fscanf(fp, "%d %d", &tmp_objects[i].profit, &tmp_objects[i].weight) < 2)
		{
			free(objects);
			fclose(fp);
			return 0;
		}
	}

	fclose(fp);

	*generations_count = (int)strtol(argv[2], NULL, 10);

	*P = (int)strtol(argv[3], NULL, 10);

	if (*generations_count == 0)
	{
		free(tmp_objects);

		return 0;
	}

	*objects = tmp_objects;

	if (*P == 0)
	{
		free(tmp_objects);

		return 0;
	}

	*objects = tmp_objects;

	return 1;
}

void print_objects(const sack_object *objects, int object_count)
{
	for (int i = 0; i < object_count; ++i)
	{
		printf("%d %d\n", objects[i].weight, objects[i].profit);
	}
}

void print_generation(const individual *generation, int limit)
{
	for (int i = 0; i < limit; ++i)
	{
		for (int j = 0; j < generation[i].chromosome_length; ++j)
		{
			printf("%d ", generation[i].chromosomes[j]);
		}

		printf("\n%d - %d\n", i, generation[i].fitness);
	}
}

void print_best_fitness(const individual *generation)
{
	printf("%d\n", generation[0].fitness);
}

void compute_fitness_function(const sack_object *objects, individual *generation, int start, int end, int sack_capacity)
{
	int weight;
	int profit;

	for (int i = start; i < end; ++i)
	{
		weight = 0;
		profit = 0;

		for (int j = 0; j < generation[i].chromosome_length; ++j)
		{
			if (generation[i].chromosomes[j])
			{
				weight += objects[j].weight;
				profit += objects[j].profit;
			}
		}

		generation[i].fitness = (weight <= sack_capacity) ? profit : 0;
	}
}

int cmpfunc(const void *a, const void *b)
{
	int i;
	individual *first = (individual *)a;
	individual *second = (individual *)b;

	int res = second->fitness - first->fitness; // decreasing by fitness
	if (res == 0)
	{
		int first_count = 0, second_count = 0;

		for (i = 0; i < first->chromosome_length && i < second->chromosome_length; ++i)
		{
			first_count += first->chromosomes[i];
			second_count += second->chromosomes[i];
		}

		res = first_count - second_count; // increasing by number of objects in the sack
		if (res == 0)
		{
			return second->index - first->index;
		}
	}

	return res;
}

void mutate_bit_string_1(const individual *ind, int generation_index)
{
	int i, mutation_size;
	int step = 1 + generation_index % (ind->chromosome_length - 2);

	if (ind->index % 2 == 0)
	{
		// for even-indexed individuals, mutate the first 40% chromosomes by a given step
		mutation_size = ind->chromosome_length * 4 / 10;
		for (i = 0; i < mutation_size; i += step)
		{
			ind->chromosomes[i] = 1 - ind->chromosomes[i];
		}
	}
	else
	{
		// for even-indexed individuals, mutate the last 80% chromosomes by a given step
		mutation_size = ind->chromosome_length * 8 / 10;
		for (i = ind->chromosome_length - mutation_size; i < ind->chromosome_length; i += step)
		{
			ind->chromosomes[i] = 1 - ind->chromosomes[i];
		}
	}
}

void mutate_bit_string_2(const individual *ind, int generation_index)
{
	int step = 1 + generation_index % (ind->chromosome_length - 2);

	// mutate all chromosomes by a given step
	for (int i = 0; i < ind->chromosome_length; i += step)
	{
		ind->chromosomes[i] = 1 - ind->chromosomes[i];
	}
}

void crossover(individual *parent1, individual *child1, int generation_index)
{
	individual *parent2 = parent1 + 1;
	individual *child2 = child1 + 1;
	int count = 1 + generation_index % parent1->chromosome_length;

	memcpy(child1->chromosomes, parent1->chromosomes, count * sizeof(int));
	memcpy(child1->chromosomes + count, parent2->chromosomes + count, (parent1->chromosome_length - count) * sizeof(int));

	memcpy(child2->chromosomes, parent2->chromosomes, count * sizeof(int));
	memcpy(child2->chromosomes + count, parent1->chromosomes + count, (parent1->chromosome_length - count) * sizeof(int));
}

void copy_individual(const individual *from, const individual *to)
{
	memcpy(to->chromosomes, from->chromosomes, from->chromosome_length * sizeof(int));
}
int min(int a, int b)
{
	return a < b ? a : b;
}
void merge(individual *source, int start, int mid, int end, individual *destination)
{

	int iA = start;
	int iB = mid;
	int i;
	for (i = start; i < end; i++)
	{

		if (end == iB || (iA < mid && source[iA].fitness >= source[iB].fitness))
		{

			destination[i] = source[iA];
			iA++;
		}
		else
		{
			destination[i] = source[iB];
			iB++;
		}
	}
}
void free_generation(individual *generation)
{
	int i;

	for (i = 0; i < generation->chromosome_length; ++i)
	{
		free(generation[i].chromosomes);
		generation[i].chromosomes = NULL;
		generation[i].fitness = 0;
	}
}

void *run_genetic_algorithm(void *arg)
{

	struct arguments argu = *(struct arguments *)arg;

	individual *current_generation = argu.current_generation;
	individual *next_generation = argu.next_generation;
	individual *tmp = argu.tmp;
	individual *current_generation_New = argu.current_generation_New;

	int object_count = argu.object_count;
	int generations_count = argu.generations_count;
	int sack_capacity = argu.sack_capacity;
	sack_object *objects = argu.objects;
	int object_count_New = argu.object_count_New;

	int count, cursor;
	int start_local, end_local;
	int thread_id = argu.ID;
	int width;
	int P = argu.P;
	int start, end;

	pthread_barrier_t *barrier = argu.barrier;

	start = thread_id * (double)object_count / P;

	end = min((thread_id + 1) * (double)object_count / P, object_count);

	// set initial generation (composed of object_count individuals with a single item in the sack)
	for (int i = start; i < end; ++i)
	{

		current_generation[i].fitness = 0;
		current_generation[i].chromosomes = (int *)calloc(object_count, sizeof(int));
		current_generation[i].chromosomes[i] = 1;
		current_generation[i].index = i;
		current_generation[i].chromosome_length = object_count;

		next_generation[i].fitness = 0;
		next_generation[i].chromosomes = (int *)calloc(object_count, sizeof(int));
		next_generation[i].index = i;
		next_generation[i].chromosome_length = object_count;
	}

	// iterate for each generation

	for (int k = 0; k < generations_count; ++k)
	{
		int start, end;

		cursor = 0;

		// compute fitness and sort by it
		start = thread_id * (double)object_count / P;
		end = min((thread_id + 1) * (double)object_count / P, object_count);
		compute_fitness_function(objects, current_generation, start, end, sack_capacity);

		pthread_barrier_wait(barrier);

		//------------------

		start = thread_id * (double)object_count_New / P;
		end = min((thread_id + 1) * (double)object_count_New / P, object_count_New);
		individual *aux;

		for (width = 1; width < object_count_New; width = 2 * width)
		{

			start_local = (start / (2 * width)) * (2 * width);
			end_local = min(object_count_New, (end / (2 * width)) * (2 * width));

			for (int i = start_local; i < end_local; i = i + 2 * width)
			{
				merge(current_generation, i, i + width, i + 2 * width, current_generation_New);
			}

			pthread_barrier_wait(barrier);

			aux = current_generation;
			current_generation = current_generation_New;
			current_generation_New = aux;

			pthread_barrier_wait(barrier);
		}
		// 		pthread_barrier_wait(barrier);
		// print_generation(current_generation,object_count);

		// pthread_barrier_wait(barrier);

		// pthread_barrier_wait(barrier);
		// printf("----------------\n");
		// print_generation(current_generation, object_count);
		// pthread_barrier_wait(barrier);

		//----------------------------

		// keep first 30% children (elite children selection)

		count = object_count * 3 / 10;

		start = thread_id * (double)count / P;
		end = min((thread_id + 1) * (double)count / P, count);

		for (int i = start; i < end; ++i)
		{

			copy_individual(current_generation + i, next_generation + i);
		}

		cursor = count;

		// mutate first 20% children with the first version of bit string mutation

		count = object_count * 2 / 10;

		start = thread_id * (double)count / P;
		end = min((thread_id + 1) * (double)count / P, count);

		for (int i = start; i < end; ++i)
		{
			copy_individual(current_generation + i, next_generation + cursor + i);
			mutate_bit_string_1(next_generation + cursor + i, k);
		}

		cursor += count;

		// mutate next 20% children with the second version of bit string mutation
		count = object_count * 2 / 10;
		start = thread_id * (double)count / P;
		end = min((thread_id + 1) * (double)count / P, count);

		for (int i = start; i < end; ++i)
		{
			copy_individual(current_generation + i + count, next_generation + cursor + i);
			mutate_bit_string_2(next_generation + cursor + i, k);
		}
		cursor += count;

		// crossover first 30% parents with one-point crossover
		// (if there is an odd number of parents, the last one is kept as such)

		count = object_count * 3 / 10;

		if (count % 2 == 1)
		{
			copy_individual(current_generation + object_count - 1, next_generation + cursor + count - 1);

			count--;
		}

		start = thread_id * (double)count / P;
		if (start % 2 != 0)
		{
			start = start + 1;
		}
		end = min((thread_id + 1) * (double)count / P, count);

		for (int i = start; i < end; i += 2)
		{
			crossover(current_generation + i, next_generation + cursor + i, k);
		}

		pthread_barrier_wait(barrier);

		// switch to new generation
		tmp = current_generation;
		current_generation = next_generation;
		next_generation = tmp;

		pthread_barrier_wait(barrier);

		start = thread_id * (double)object_count / P;
		end = min((thread_id + 1) * (double)object_count / P, object_count);
		for (int i = start; i < end; ++i)
		{
			current_generation[i].index = i;
		}

		if (thread_id == 0)
		{
			if (k % 5 == 0)
			{
				print_best_fitness(current_generation);
			}
		}
	}
	start = thread_id * (double)object_count / P;
	end = min((thread_id + 1) * (double)object_count / P, object_count);

	compute_fitness_function(objects, current_generation, start, end, sack_capacity);

	pthread_barrier_wait(barrier);

	start = thread_id * (double)object_count_New / P;
	end = min((thread_id + 1) * (double)object_count_New / P, object_count_New);

	for (width = 1; width < object_count_New; width = 2 * width)
	{

		start_local = (start / (2 * width)) * (2 * width);
		end_local = min(object_count_New, (end / (2 * width)) * (2 * width));

		for (int i = start_local; i < end_local; i = i + 2 * width)
		{
			merge(current_generation, i, i + width, i + 2 * width, current_generation_New);
		}

		pthread_barrier_wait(barrier);

		tmp = current_generation;
		current_generation = current_generation_New;
		current_generation_New = tmp;

		pthread_barrier_wait(barrier);
	}

	if (thread_id == 0)
	{
		print_best_fitness(current_generation);
	}
	// free resources for old generation
	//  free_generation(current_generation);
	// free_generation(next_generation);
	

	// // // free resources
	// free(current_generation);
	// free(next_generation);
	

	return NULL;
}