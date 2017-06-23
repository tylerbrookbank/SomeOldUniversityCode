/***********************

Conway Game of Life

************************/

#include <stdio.h>

#include <stdlib.h>

#include <time.h>

#define NI 10
#define NJ 10
#define NSTEPS 1

int i, j, n, im, ip, jm, jp, ni, nj, nsum, isum;
int **old, **new;
float x;
FILE *outFile;

// doTimeStep(int n);

int readCSVFile(char *filename)
{
	FILE *file = fopen(filename,"r");
	if( file )
	{
		char buffer[BUFSIZ], *ptr;
		for( i=1; fgets(buffer, sizeof(buffer), file); ++i)
		{
			for( j=1, ptr = buffer; j<=NJ; ++j, ++ptr)
			{
				old[i][j] = (int)strtol(ptr, &ptr, 10);
			}
		}
		fclose(file);
        return 0;
	}
    else
    {
            printf("Error file not read.\n");
            return 1;
    }
}

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

int main(int argc, char *argv[]) {

  /* get array size
   * this is to make testing easier
   * so that I do not need to re-compile everytime I want to try new
   * array sizes and number of time steps.
   *
   * We start the timer after we gather the input so we are not counting
   * the user i/o*/
  
   
  /* start timer */
  time_t start_time;
  time_t end_time;
  double total_run_time;
  
  time(&start_time);
  
  /* allocate arrays */

  ni = NI + 2;  /* add 2 for left and right ghost cells */
  nj = NJ + 2;
  old = malloc(ni*sizeof(int*));
  new = malloc(ni*sizeof(int*));
  for(i=0; i<ni; i++){
    old[i] = malloc(nj*sizeof(int));
    new[i] = malloc(nj*sizeof(int));
  }

/*  initialize elements of old to 0 or 1 */

readCSVFile();

  /*  time steps */
  for(n=0; n<NSTEPS; n++){
	  doTimeStep(n);
  }

  /*  Iterations are done; sum the number of live cells */
  isum = 0;
  for(i=1; i<=NI; i++){
    for(j=1; j<=NJ; j++){
      isum = isum + new[i][j];
    }
  }
  printf("\nNumber of live cells = %d\n", isum);

  //getchar(); /*To get the console window to stay open!*/
  
  /* end timer */
  
  return 0;
}
