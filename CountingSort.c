#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>

#define RANGE 100000
#define ALENGTH 1000000
#define NUM_THREADS 8

clock_t timer, start_timer, end_timer;

typedef struct Element{

	int key;

} element;

double stopWatch(){//timer function
	double total_time = 0.0;

	if (timer == 0){ //timer is not running yet
		timer = 1;
		start_timer = clock();
		return -1;
	}
	else { //timer is currently running
		timer = 0;
		end_timer = clock();
		total_time = (double)(end_timer - start_timer);
	}

	return total_time;

}

/* Counting Sort */
void countingSort(element *A, element *B, int range)
{
	int i;
	int *C;
	C = malloc(sizeof(int)*range);
	omp_lock_t lock[RANGE];//create a lock for each index of C

	omp_set_num_threads(NUM_THREADS);
#pragma omp parallel for schedule(static)
	for (i = 0; i < range; i++)
	{
		omp_init_lock(&lock[i]);//initailize locks
		C[i] = 0;//initialize C indexs to 0
	}

#pragma omp parallel for schedule(static)
	for (i = 0; i < ALENGTH; i++)
	{
#pragma omp atomic
		C[A[i].key]++;//count number of occurances of index of C
	}

	for (i = 1; i < range; i++)
	{
		C[i] = C[i] + C[i - 1];//keep a running total
	}

#pragma omp parallel for schedule(static)
	for (i = 0; i < ALENGTH; i++)
	{
		omp_set_lock(&lock[A[i].key]);//lock this index
		B[C[A[i].key] - 1] = A[i];
		C[A[i].key] = C[A[i].key] - 1;
		omp_unset_lock(&lock[A[i].key]);//unlock this index
	}
}

int main()
{
	int i;
	element *a;
	element *b;

	omp_set_num_threads(NUM_THREADS);

	a = (element*)malloc(sizeof(element)*ALENGTH);
	b = (element*)malloc(sizeof(element)*ALENGTH);


	for (i = 0; i < ALENGTH; i++)
	{
		a[i].key = rand() % RANGE;
	}

	printf("Starting sort.\n");
	stopWatch();
	countingSort(a, b, RANGE);
	printf("Done %.20f.\n", stopWatch() / CLOCKS_PER_SEC);


	getchar();
	return 0;
}