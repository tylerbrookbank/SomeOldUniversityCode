#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "mpi.h"

int NI, NJ, NSTEPS, outputCheck;
char *fileName;
FILE *output;
int i,j,ni,nj,k,im,ip,jm,jp,n,nsum,isum,offset,boardType;
int **old, **new, *section, **boarder;
int my_rank, p, source, dest, tag;
time_t timer, start_timer, end_timer;
MPI_Status status;
float x;

/*function for timing the program running time
 *function is called twice, once to star the timer and a second time to end it*/
float stopWatch(){
    float total_time=0.0;
  
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

/*simple function that opens the output file*/
void openOutput()
{
    output = fopen("output.csv", "w+");
}

/*gets a randomized board for the game of life*/
void randomBoard()
{
    for(i=0; i<NI; i++){
    for(j=0; j<NJ; j++){
      x = rand()/((float)RAND_MAX + 1);
      if(x<0.5){
	old[i][j] = 0;
      } else {
	old[i][j] = 1;
      }
    }
  }
}

/* code provided by Parsa*/
int readCSVFile(char *filename)
{
	FILE *file = fopen(filename,"r");
	if( file )
	{
		char buffer[BUFSIZ], *ptr;
		for( i=0; fgets(buffer, sizeof(buffer), file); ++i)
		{
			for( j=0, ptr = buffer; j<NJ; ++j, ++ptr)
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
/* end of code provided by Parsa */

/*file that outputs the current state of the board to the output file*/
void writeFile()
{
	for(i=0;i<NI;i++)
	{
		for(j=0;j<NJ;j++)
		{
			fprintf(output,"%d,",old[i][j]);
		}
		fprintf(output,"\n");
	}
    fprintf(output,"\n\n\n");
    
}

/*this function simply initializes all the grids needed for computation*/
void initGrids()
{
    if(my_rank==0)//master process needs a larger board then other processes 
    {
        old = malloc(sizeof(int*)*NI);
        new = malloc(sizeof(int*)*NI);
        
        for(i=0;i<NI;i++)
        {
            old[i] = malloc(sizeof(int)*NJ);
            new[i] = malloc(sizeof(int)*NJ);
        }
    }
    else
    {
        old = malloc(sizeof(int*)*NI/p);
        new = malloc(sizeof(int*)*NI/p);
        
        for(i=0;i<NI/p;i++)
        {
            old[i] = malloc(sizeof(int)*NJ);
            new[i] = malloc(sizeof(int)*NJ);
        }
    }
    
    //section is used to pass each row of the grid the processes are in charge of
    section = malloc(sizeof(int)*NJ);
    
    //top and bottom boarder
    boarder = malloc(sizeof(int*)*2);
    
    for(i=0;i<2;i++)
        boarder[i] = malloc(sizeof(int)*NJ);
}

/*deallocate memory*/
void cleanup()
{
    free(old);
    free(new);
    free(boarder);
    free(section);
}

/*this function sends all the input read from the user to processes 1-p*/
void sendInput()
{
    if(my_rank==0)
    {
        for(i=1;i<p;i++)
        {
            MPI_Send(&NI, sizeof(int), MPI_INT, i, 0 ,  MPI_COMM_WORLD);
            MPI_Send(&NJ, sizeof(int), MPI_INT, i, 0 ,  MPI_COMM_WORLD);
            MPI_Send(&NSTEPS, sizeof(int), MPI_INT, i, 0 ,  MPI_COMM_WORLD);
            MPI_Send(&outputCheck, sizeof(int), MPI_INT, i, 0 ,  MPI_COMM_WORLD);
        }
    }
    else
    {
        MPI_Recv(&NI, sizeof(int), MPI_INT, 0, 0 ,  MPI_COMM_WORLD, &status);
        MPI_Recv(&NJ, sizeof(int), MPI_INT, 0, 0 ,  MPI_COMM_WORLD, &status);
        MPI_Recv(&NSTEPS, sizeof(int), MPI_INT, 0, 0 ,  MPI_COMM_WORLD, &status);
        MPI_Recv(&outputCheck, sizeof(int), MPI_INT, 0, 0 ,  MPI_COMM_WORLD, &status);
    }
}

/*this function is for communicating the sections of the board from p0 to p1-n*/
void sendSections()
{
    
    if(my_rank==0)
    {
        for(i=1;i<p;i++)
        {
            for(j=0;j<NI/p;j++)
            {
                tag=j;
                MPI_Send(old[(NI/p*i) + offset + j],sizeof(int)*NJ,MPI_BYTE,i,tag,MPI_COMM_WORLD);
            }
        }
    }
    else
    {
        for(i=0;i<NI/p;i++)
        {
            tag=i;
            dest=0;
            MPI_Recv(section,sizeof(int)*NJ,MPI_BYTE,dest,tag,MPI_COMM_WORLD,&status);
            
            for(j=0;j<NJ;j++)
            {
                old[i][j] = section[j];
            }
        }
    }
}

/*this function is for sending the sections of the board from p1-n to p0
  p0 then puts the sections back in place into the full board*/
void gatherSections()
{
    if(my_rank==0)
    {
        for(i=1;i<p;i++)
        {
            for(j=0;j<NI/p;j++)
            {
                tag=j;
                MPI_Recv(section,sizeof(int)*NJ,MPI_BYTE,i,tag,MPI_COMM_WORLD,&status);
                
                for(k=0;k<NJ;k++)
                {
                    old[(NI/p*i + offset)+j][k] = section[k];
                }
            }
        }
    }
    else
    {
        for(i=0;i<NI/p;i++)
        {
            tag=i;
            dest=0;
            MPI_Send(old[i],sizeof(int)*NJ,MPI_BYTE,dest,tag,MPI_COMM_WORLD);
        }
    }
}

/*function for communicating the boarders to each process
  this is to calculate the values along the section boarders*/
void getBoarders()
{
    int top, bottom, temp;
    int first, last;
    
    first = 0;
    if(my_rank == 0)
        last = NI/p + offset;
    else
        last = NI/p;
    
    top = my_rank - 1;
    bottom = my_rank + 1;
    
    if(top == -1)
        top = p - 1;
     if(bottom == p)
         bottom = 0;
    
    int above = 0;//above boarder
    int bellow = 1;//bellow boarder
    
    if(p==2 || p==1)
    {
        //this is here because when there is only 1 process or 2 processes
        //the top and bottom seem to swap do to the top and bottom being 
        //an equal process
        above = 1;
        bellow = 0;
    }
    
    MPI_Send(old[first],sizeof(int)*NJ,MPI_BYTE,top,0,MPI_COMM_WORLD);
    MPI_Send(old[last-1], sizeof(int) * NJ, MPI_BYTE, bottom, 0, MPI_COMM_WORLD);

    MPI_Recv(boarder[above],sizeof(int)*NJ,MPI_BYTE,top,0,MPI_COMM_WORLD,&status);
    MPI_Recv(boarder[bellow],sizeof(int)*NJ,MPI_BYTE,bottom,0,MPI_COMM_WORLD,&status);

}

/*doTimeStep is similar to the sequential version
this version just utilizes the boarder sections that it gets from getBoarders()
and the NJ loop is divided by p*/
void doTimeStep()
{
    int iBound,top,bottom;
    top = 0;
    bottom =1;
    
    if(my_rank==0)
        iBound = NI/p + offset;
    else
        iBound = NI/p;
    getBoarders();
    for(i=0;i<iBound;i++)
    {
        for(j=0;j<NJ;j++)
        {
            if(j==0) 
                jm = NJ-1;
            else 
                jm = j - 1;
            
            if(j==NJ-1) 
                jp = 0;
            else 
                jp = j + 1;
            
            if(iBound == 1)
            {
                nsum = boarder[top][jp] + old[i][jp] + boarder[bottom][jp] + boarder[top][j] + boarder[bottom][j] 
                     + boarder[top][jm] + old[i][jm] + boarder[bottom][jm];
            }
            else if(i==0)
            {
                ip = i + 1;
                nsum = boarder[top][jp] + old[i][jp] + old[ip][jp] + boarder[top][j] + old[ip][j] 
                     + boarder[top][jm] + old[i][jm] + old[ip][jm];
            }
            else if(i==iBound - 1)
            {
                im = i - 1;
                nsum = old[im][jp] + old[i][jp] + boarder[bottom][jp] + old[im][j] + boarder[bottom][j]
                     + old[im][jm] + old[i][jm] + boarder[bottom][jm];
            }
            else
            {
                im = i - 1;
                ip = i + 1;
                nsum = old[im][jp] + old[i][jp] + old[ip][jp] + old[im][j] + old[ip][j]
                     + old[im][jm] + old[i][jm] + old[ip][jm];
            }
            
            switch (nsum)
            {
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
    
    for(i=0;i<iBound;i++)
    {
        for(j=0;j<NJ;j++)
        {
            old[i][j] = new[i][j];
        }
    }
}

/*this function runs the time steps as well as keeps track of the output file*/
void runTimeSteps()
{
    for(n=0;n<NSTEPS;n++)
    {
        doTimeStep();
        
        if(outputCheck!=0)
        {
            if(n%outputCheck==0)
            {
                gatherSections();
                if(my_rank==0)
                    writeFile();
            }
        }
    }
}

/*sum live cells counts the number of live cells left in the board*/
void sumLiveCells(){
  isum = 0;
  for(i=0; i<NI; i++){
    for(j=0; j<NJ; j++){
      isum = isum + old[i][j];
    }
  }
  printf("Number of live cells = %d\n", isum);
}

/*main function gets the user input and runs the above functions*/
int main(int argc, char* argv[])
{
    MPI_Init(&argc,&argv);
    
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    
    if(my_rank==0)
    {
        int buffer = 1024;
        fileName = malloc(sizeof(buffer));
        printf("Please enter grid N M k and m\n");
        scanf("%d %d %d %d", &NI, &NJ, &NSTEPS, &outputCheck);
        printf("Do you want a random board or a board from a file? 0 for random 1 for file.\n");
        scanf("%d",&boardType);
        if(boardType)
        {
            printf("Please enter the file name containing N x M grid.\n");
            scanf("%s",fileName);
        }
        if(outputCheck != 0) openOutput();
        stopWatch();
    }
    
    sendInput();
    
   // if(my_rank==0) printf("\nInitialize grids.\n");
    initGrids();
    
    offset = NI - (NI/p) * p;
    
    if(my_rank==0) printf("\nGetting board.\n");
    if(my_rank==0)
    {
        if(boardType)
        {
            if(readCSVFile(fileName))
            {
                cleanup();
                MPI_Finalize();
                exit(1);
            }
        }
        else
        {
            randomBoard();
        }
    }
    
    
    if(my_rank==0) printf("\nSending board sections.\n");
    sendSections();
    
    if(my_rank==0) printf("\nRunning time steps.\n");
    runTimeSteps();
    
    if(my_rank==0) printf("\nGathering board sections.\n");
    gatherSections();
    
    if(my_rank == 0) sumLiveCells();
    
    if(my_rank == 0 && outputCheck!= 0) fclose(output);
    
   // if(my_rank==0) printf("\nFree memory.\n");
   if(my_rank == 0) printf("Program total time = %.10f seconds.\n",stopWatch());
    cleanup();
    
    MPI_Finalize();
	
	return 0;
}