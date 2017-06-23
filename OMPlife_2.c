/***********************

Conway Game of Life

************************/

#include <stdio.h>
#include <omp.h>
#include <time.h>
#include <stdlib.h>

#define NI 1000  /* array sizes */

#define NJ 1000 

#define NUM_THREADS 8/* number of threads running on */

#define NSTEPS 500   /* number of time steps */

int i, j, n, im, ip, jm, jp, nsum, isum, offset;
int **old, **new;
float x;
int id, iBound;

clock_t timer, start_timer, end_timer;

double stopWatch(){
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

/* doTimeSteps */
void doTimeStep(int n){
	omp_set_num_threads(NUM_THREADS);
	//private varibles needed for each loop interation
#pragma omp parallel private(i, j, im, ip, jm, jp, id, nsum)
	{
		id = omp_get_thread_num(); //get thread id
		for (i = 0; i < NI; i++){
#pragma omp for schedule(static)//threads are to be split up over the j values
			for (j = 0; j < NJ; j++){
				im = i - 1;
				ip = i + 1;
				jm = j - 1;
				jp = j + 1;
				if (im == -1)//loop around to the bottom
					im = NI - 1;

				if (ip == NI)//loop around to the top
					ip = 0;

				if (jm == -1)//loop around to the right
					jm = NJ - 1;

				if (jp == NJ)//loop arounf to the left
					jp = 0;

				nsum = old[im][jp] + old[i][jp] + old[ip][jp]
					+ old[im][j] + old[ip][j]
					+ old[im][jm] + old[i][jm] + old[ip][jm];

				switch (nsum){

				case 3:
					new[i][j] = 1;
					break;

				case 2:
					new[i][j] = old[i][j];
					break;

				default:
					new[i][j] = 0;
				}
			}
		}
		for (i = 0; i < NI; i++){//set the new grid values
#pragma omp for schedule(static)//loop over the j values
			for (j = 0; j < NJ; j++){
				old[i][j] = new[i][j];
			}
		}
	}
}

int main(int argc, char *argv[]) {

	/* allocate arrays */
	old = malloc(NI*sizeof(int*));
	new = malloc(NJ*sizeof(int*));

	time_t start_time;
	time_t end_time;
	double total_run_time;

	stopWatch();

	omp_set_num_threads(NUM_THREADS);

#pragma omp parallel for schedule(static)//allocate memory in parallel
	for (i = 0; i<NI; i++){
		old[i] = malloc(NJ*sizeof(int));
		new[i] = malloc(NJ*sizeof(int));
	}

#pragma omp parallel private(i,j,id)//get random board
	{
		id = omp_get_thread_num();
		srand(id);//make sure the rand number will be different for each thread
		for (i = 0; i < NI; i++){
#pragma omp for schedule(static) private(x)
			for (j = 0; j < NJ; j++){
				x = rand() / ((float)RAND_MAX + 1);
				if (x < 0.5){
					old[i][j] = 0;
				}
				else {
					old[i][j] = 1;
				}
			}
		}
	}

	/*  time steps */

	for (n = 0; n < NSTEPS; n++){
		doTimeStep(n);
	}

	printf("Total running time: %.20f.\n", stopWatch()/CLOCKS_PER_SEC);
	getchar(); /*To get the console window to stay open!*/

	return 0;
}