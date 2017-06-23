/***********************

Conway Game of Life

************************/

#include <stdio.h>

#include <stdlib.h>

#include <time.h>

int NI, NJ, NSTEPS, timer;
int i, j, n, im, ip, jm, jp, ni, nj, nsum, isum;
int **old, **new;
float x;
FILE *outFile;

int counter = 0;
time_t start_timer, end_timer, average_start_timer, average_timer, average_new_timer;

// readInput();

void readInput(){
	
  printf("Please enter the size of NI: ");
  scanf("%d",&NI);
  printf("Please enter the size of NJ: ");
  scanf("%d",&NJ);
  printf("Please enter the size of NSTEPS: ");
  scanf("%d",&NSTEPS);
	
}

// stopWatch();
/* stop watch function
 * the first timethis funtion is called it starts a timer 
 * when the function is called again it stops the timer and
 * returns the total time that the program took to run
 * on the first call the function returns -1*/
double stopWatch(){
  double total_time=0;
  
  if(timer == 0){ //timer is not running yet
    timer = 1;
	time(&start_timer);
	return -1;
  } 
  else { //timer is currently running
  	timer = 0;
	time(&end_timer);
	total_time = difftime(end_timer, start_timer);
  }
  return total_time;
  
}

// allocateArrays();

double allocateArrays(){
  
  stopWatch();
  
  ni = NI + 2;  /* add 2 for left and right ghost cells */
  nj = NJ + 2;
  old = malloc(ni*sizeof(int*));
  new = malloc(ni*sizeof(int*));

  for(i=0; i<ni; i++){
    old[i] = malloc(nj*sizeof(int));
    new[i] = malloc(nj*sizeof(int));
  }

  return stopWatch();
}

// initializeElements();
double initializeElements(){
  
  stopWatch();
  
  for(i=1; i<=NI; i++){
    for(j=1; j<=NJ; j++){
      x = rand()/((float)RAND_MAX + 1);
      if(x<0.5){
	old[i][j] = 0;
      } else {
	old[i][j] = 1;
      }
    }
  }
  
  return stopWatch();
}

// sumLiveCells();

double sumLiveCells(){
  
  stopWatch();
  
  isum = 0;
  for(i=1; i<=NI; i++){
    for(j=1; j<=NJ; j++){
      isum = isum + new[i][j];
    }
  }
  printf("Number of live cells = %d\n", isum);
  
  return stopWatch();
}

// doTimeStep(int n);

void doTimeStep(int n){

	/* corner boundary conditions */
	old[0][0] = old[NI][NJ];
	old[0][NJ + 1] = old[NI][1];
	old[NI + 1][NJ + 1] = old[1][1];
	old[NI + 1][0] = old[1][NJ];

	/* left-right boundary conditions */

	for (i = 1; i <= NI; i++){
		old[i][0] = old[i][NJ];
		old[i][NJ + 1] = old[i][1];
	}

	/* top-bottom boundary conditions */

	for (j = 1; j <= NJ; j++){
		old[0][j] = old[NI][j];
		old[NI + 1][j] = old[1][j];
	}

	for (i = 1; i <= NI; i++){
		for (j = 1; j <= NJ; j++){
			im = i - 1;
			ip = i + 1;
			jm = j - 1;
			jp = j + 1;

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

	/* copy new state into old state */

	for (i = 1; i <= NI; i++){
		for (j = 1; j <= NJ; j++){
			old[i][j] = new[i][j];
		}
	}
}

// timeStep();

double timeSteps(){
  
  stopWatch();
  
  for(n=0; n<NSTEPS; n++){
	  doTimeStep(n);
  }
  
  return stopWatch();
}

int main(int argc, char *argv[]) {

  /* get array size
   * this is to make testing easier
   * so that I do not need to re-compile everytime I want to try new
   * array sizes and number of time steps.*/
  readInput();
  
  /* allocate arrays */
  double allocate_time_took = allocateArrays();
  printf("Allocating the arrays took %f seconds.\n",allocate_time_took);

/*  initialize elements of old to 0 or 1 */

  double initialize_time_took = initializeElements();
  printf("Initializinf the Elements of the grid took %f seconds.\n",initialize_time_took);

  /*  time steps */
  double timesteps_time_took = timeSteps();
  printf("The time steps took %f seconds.\n",timesteps_time_took);

  /*  Iterations are done; sum the number of live cells */
  double sum_time_took = sumLiveCells();
  printf("Summing the live cells took %f seconds.\n",sum_time_took);

  double total_time = sum_time_took+timesteps_time_took+initialize_time_took+allocate_time_took;
  printf("Total program run time is %f seconds.",total_time);
  
  //getchar(); /*To get the console window to stay open!*/
  printf("\a");//alert
  return 0;
}
