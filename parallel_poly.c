
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

/*global varible declarations*/

#define N_THREAD   8
#define N_POLY     4
#define SIZE       257
#define SIZE_SQ    SIZE*SIZE
#define SIZE_CU    SIZE*SIZE_SQ
#define LEGNTH     32
#define middle	   128

typedef struct param{
	double weight;
	int *history;
	int iteration;
	int x, y, z;
	int current_length;
	int face;
} path;

int **space;
path **p;
int **polymers;
int poly_count;
int *sorted_weights;
omp_lock_t poly_lock;
omp_lock_t copy_lock;

int my_id;

void sort()
{
	my_id = omp_get_thread_num();
	//printf("start sort %d\n", my_id);
	int id;
	int j;
	int swapped = 1;
	while (swapped)
	{
		swapped = 0;
		for (j = 1; j<N_THREAD; j++)
		{
			if (p[sorted_weights[j - 1]]->weight > p[sorted_weights[j]]->weight)
			{
				id = sorted_weights[j];
				sorted_weights[j] = sorted_weights[j - 1];
				sorted_weights[j - 1] = id;
				swapped = 1;
			}
		}
	}
	//printf("end sort %d\n", my_id);
}

int getNewParam(){

	int i;

	int randomBest;

	my_id = omp_get_thread_num();
	
	while ((randomBest = rand() % 3) != my_id);

	//printf("Start param %d\n", my_id);
	
	//printf("Random best: ");

	//for (i = 0; i < p[my_id]->current_length; i++)
	//	printf("%d,%d: %d\n", my_id,i,p[my_id]->history[i]);

	omp_set_lock(&copy_lock);

	p[my_id]->weight = p[sorted_weights[randomBest]]->weight;
	//printf("%d New weight: %f\n",my_id,p[my_id]->weight);

	p[my_id]->x = p[sorted_weights[randomBest]]->x;
	//printf("%d New x: %d\n", my_id, p[my_id]->x);

	p[my_id]->y = p[sorted_weights[randomBest]]->y;
	//printf("%d New y: %d\n", my_id, p[my_id]->y);

	p[my_id]->z = p[sorted_weights[randomBest]]->z;
	//printf("%d New z: %d\n", my_id, p[my_id]->z);

	p[my_id]->face = p[sorted_weights[randomBest]]->face;
	//printf("%d New face: %d\n", my_id, p[my_id]->face);

	p[my_id]->current_length = p[sorted_weights[randomBest]]->current_length;
	//printf("%d New current length: %d\n", my_id, p[my_id]->current_length);

	for (i = 0; i < p[my_id]->current_length; i++)
	{
		p[my_id]->history[i] = p[sorted_weights[randomBest]]->history[i];
		//printf("%d History Copy %d: %d\n", i, my_id, p[sorted_weights[randomBest]]->history[i]);
	}

	omp_unset_lock(&copy_lock);
	//printf("%d: FACE %d\n", my_id, p[my_id]->face);

	//printf("end Param %d\n", my_id);

	return 0;
}

int offset(int x, int y, int z){
	return (x + (y*SIZE) + (z*SIZE_SQ));
}

int run(){

	int i;
	int moveCount = 0;
	my_id = omp_get_thread_num();
	printf("start run %d\n", my_id);
	//printf("%d: %d\n",my_id,p[my_id]->iteration);

	switch (p[my_id]->face)
	{
	case 0:
		p[my_id]->x++;
		break;
	case 1:
		p[my_id]->y++;
		break;
	case 2:
		p[my_id]->z++;
		break;
	case 3:
		p[my_id]->x--;
		break;
	case 4:
		p[my_id]->y--;
		break;
	case 5:
		p[my_id]->z--;
		break;
	default:
		printf("Something went wrong.\n");
		break;
	}

	if (space[my_id][offset(p[my_id]->x, p[my_id]->y, p[my_id]->z)] == p[my_id]->iteration){
		//printf("\n\nID;: %d   FAILED\n\n", my_id);
		//printf("%d\t%d\t%d\n\n", p[my_id]->x, p[my_id]->y, p[my_id]->z);
		return 0;
	}
	else{
		space[my_id][offset(p[my_id]->x, p[my_id]->y, p[my_id]->z)] = p[my_id]->iteration;
	}
	
	if (space[my_id][offset(p[my_id]->x + 1, p[my_id]->y, p[my_id]->z)] != p[my_id]->iteration)
		moveCount++;
	if (space[my_id][offset(p[my_id]->x - 1, p[my_id]->y, p[my_id]->z)] != p[my_id]->iteration)
		moveCount++;
	if (space[my_id][offset(p[my_id]->x, p[my_id]->y + 1, p[my_id]->z)] != p[my_id]->iteration)
		moveCount++;
	if (space[my_id][offset(p[my_id]->x, p[my_id]->y - 1, p[my_id]->z)] != p[my_id]->iteration)
		moveCount++;
	if (space[my_id][offset(p[my_id]->x, p[my_id]->y, p[my_id]->z + 1)] != p[my_id]->iteration)
		moveCount++;
	if (space[my_id][offset(p[my_id]->x, p[my_id]->y, p[my_id]->z - 1)] != p[my_id]->iteration)
		moveCount++;

	//printf("%d: FACE %d\n\n", my_id, p[my_id]->face);
	//stores the valuce of face so we made rebuild the path after sucess
	omp_set_lock(&copy_lock);
	p[my_id]->history[p[my_id]->current_length] = p[my_id]->face;
	p[my_id]->current_length++;
	//dosent allow the path to try and double back on itself

	p[my_id]->face += (rand() % 5) - 2;
	p[my_id]->face = p[my_id]->face % 6;
	if (p[my_id]->face < 0)
		p[my_id]->face = p[my_id]->face + 6;
	//printf("%d: FACE %d\n", my_id, p[my_id]->face);

	p[my_id]->weight *= (moveCount / 6.0);
	omp_unset_lock(&copy_lock);
	printf("end run %d\n", my_id);
	return 1;
}

void setPoly(){
	int i;
	my_id = omp_get_thread_num();
	printf("start set %d\n", my_id);
	
	omp_set_lock(&copy_lock);
	for (i = 0; i < LEGNTH; i++){
		polymers[poly_count][i] = p[my_id]->history[i];
	}
	omp_unset_lock(&copy_lock);
	
	printf("end set %d\n", my_id);
}

void buildSpace(){
	int x, y, z, i;

	x = y = z = middle;

	my_id = omp_get_thread_num();
	printf("start build %d\n", my_id);
	for (i = 0; i < p[my_id]->current_length; i++)
	{
		switch (p[my_id]->history[i])
		{
		case 0:
			x++;
			break;
		case 1:
			y++;
			break;
		case 2:
			z++;
			break;
		case 3:
			x--;
			break;
		case 4:
			y--;
			break;
		case 5:
			z--;
			break;
		}
		space[my_id][offset(x, y, z)] = p[my_id]->iteration;
	}
	printf("end build %d\n", my_id);
}

void setdefaults()
{
	p[my_id]->weight = 1;
	p[my_id]->x = middle;
	p[my_id]->y = middle;
	p[my_id]->z = middle;
	p[my_id]->face = rand() % 6;
	p[my_id]->current_length = 0;

	//printf("%d: FACE %d\n", my_id, p[my_id]->face);
}

int getPoly(){

	my_id = omp_get_thread_num();

	while (poly_count < N_POLY)
	{
	//	printf("%d: %d\n", my_id, p[my_id]->iteration);
		if (run())
		{
			if (p[my_id]->current_length == LEGNTH)
			{
				omp_set_lock(&poly_lock);
				if (poly_count < N_POLY)
				{
					setPoly();
					poly_count++;
				}
				omp_unset_lock(&poly_lock);
				p[my_id]->iteration++;
				setdefaults();
			}
		}
		else
		{
			p[my_id]->iteration++;
			//getNewParam();
			setdefaults();
			//buildSpace();
		}

#pragma omp barrier
		if (my_id == 0)
		{
			//printf("sorting.\n");
			//sort();
		}
#pragma omp barrier
	}
	return 0;
}


void printPoly()
{
	FILE *f = fopen("output.txt","w");
	int i, j, x, y, z;
	for (i = 0; i < N_POLY; i++)
	{
		x = y = z = 0;
		fprintf(f,"Polymer #%d:\n\n", i + 1);
		for (j = 0; j < LEGNTH; j++)
		{
			
			switch (polymers[i][j])
			{
			case 0:
				x++;
				break;
			case 1:
				y++;
				break;
			case 2:
				z++;
				break;
			case 3:
				x--;
				break;
			case 4:
				y--;
				break;
			case 5:
				z--;
				break;
			}
			// print to stdout  should redirect flow with batch running
			fprintf(f,"%d\t%d\t%d \t", x, y, z);
			fprintf(f, "FACE = %d\n", polymers[i][j]);
		}
	}
}

void initialize(){

	int i;
	p = (path**)malloc(sizeof(path**)*N_THREAD);
	space = (int**)malloc(sizeof(int**)*N_THREAD);
	polymers = (int**)malloc(sizeof(int**)*N_POLY);
	poly_count = 0;
	sorted_weights = (int*)malloc(sizeof(int*)*N_THREAD);
	omp_init_lock(&poly_lock);
	omp_init_lock(&copy_lock);

	for (i = 0; i < N_THREAD; i++)
	{
		space[i] = (int*)malloc(sizeof(int*)*SIZE_CU);
		p[i] = (path*)malloc(sizeof(path*));

	}

	for (i = 0; i < N_POLY; i++)
	{
		polymers[i] = (int*)malloc(sizeof(int*)*LEGNTH);
	}

	for (i = 0; i<N_THREAD; i++)
	{
		int j;
		p[i]->history = (int*)malloc(sizeof(int*)*LEGNTH);
		for (j = 0; j < LEGNTH; j++)
			p[i]->history[i] = -1;
		p[i]->x = middle;
		p[i]->y = middle;
		p[i]->z = middle;
		p[i]->iteration = 1;
		p[i]->current_length = 0;
		p[i]->face = rand() % 6;
		//printf("%d: FACE %d\n",i,p[i]->face);
		p[i]->weight = 1;
		sorted_weights[i] = i;
		//printf("%d: %d %d %d %d %d\n", i, p[i]->x, p[i]->y, p[i]->z, p[i]->face, p[i]->current_length);
	}
}

int main(){
	srand((int)time(NULL));
	omp_set_num_threads(N_THREAD);
	initialize();
#pragma omp parallel private(my_id)// shared(p,space,sorted_weights,polymers,poly_count,copy_lock,poly_lock)
	{
		my_id = omp_get_thread_num();
		srand((int)time(NULL) + my_id);

		//printf("hello from %d. my current_length is %d, there are %d\n", my_id, p[my_id]->current_length,N_THREAD);
		getPoly();
	}
	printf("Done.\n");
	printPoly();
	getchar();
}